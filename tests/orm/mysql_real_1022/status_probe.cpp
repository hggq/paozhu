// OK / OK-via-EOF 包 status_flags 线级探针：只为回答"能不能从服务端状态位
// 得到 autocommit / 事务在途"，并测出该位在哪些响应包里、何时更新。
// 不动产品代码；raw 读包用 write_sql + read_loop 手工组帧（与 exec_dml_impl 同口径累积半包）。
#include <asio.hpp>

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "mysql_conn.h"
#include "orm_conn_pool.h"
#include "request.h"

using conn_t = std::shared_ptr<orm::mysql_conn_base>;
using pool_t = std::shared_ptr<orm::orm_conn_pool>;

static int g_pass = 0;
static int g_fail = 0;

static void expect(bool ok, const std::string &name, const std::string &detail = "")
{
    if (ok)
    {
        ++g_pass;
        std::printf("PASS %s\n", name.c_str());
    }
    else
    {
        ++g_fail;
        std::printf("FAIL %s %s\n", name.c_str(), detail.c_str());
    }
}

static void reading(const std::string &name, const std::string &v)
{
    std::printf("READ %s=%s\n", name.c_str(), v.c_str());
}

static std::string hex_of(const unsigned char *d, size_t n)
{
    static const char *hx = "0123456789abcdef";
    std::string r;
    for (size_t i = 0; i < n; ++i)
    {
        r.push_back(hx[d[i] >> 4]);
        r.push_back(hx[d[i] & 15]);
    }
    return r;
}

// 带缓冲的逐包读：read_some 会把服务端多个小包一次送上来，
// 所以必须按包切、剩下的字节留在缓冲区里，否则下一条 read 会永久阻塞。
struct packet_reader
{
    conn_t c;
    std::vector<unsigned char> buf;
    size_t pos = 0;

    explicit packet_reader(conn_t &in) : c(in) {}

    bool fill_at_least(size_t need)
    {
        for (int guard = 0; guard < 4096; ++guard)
        {
            if (have_packet(need))
                return true;
            unsigned int n = c->read_loop();
            if (n == 0)
                return have_packet(need);
            buf.insert(buf.end(), c->_cache_data, c->_cache_data + n);
        }
        return have_packet(need);
    }

    bool have_packet(size_t need)
    {
        if (buf.size() - pos < need)
            return false;
        if (need > 4)
            return true;
        // need==4：只要头里报的整包也在缓冲里就算够
        unsigned int pkt_len = buf[pos] | (buf[pos + 1] << 8) | (buf[pos + 2] << 16);
        return buf.size() - pos >= 4u + pkt_len;
    }

    // 返回一包（含 4B 头）；读不到返回空
    std::vector<unsigned char> next()
    {
        if (buf.size() - pos < 4 && !fill_at_least(4))
            return {};
        unsigned int pkt_len = buf[pos] | (buf[pos + 1] << 8) | (buf[pos + 2] << 16);
        if (!fill_at_least(4u + pkt_len))
            return {};
        std::vector<unsigned char> one(buf.begin() + pos, buf.begin() + pos + 4 + pkt_len);
        pos += 4 + pkt_len;
        return one;
    }
};

struct pkt_info
{
    unsigned int len      = 0;
    unsigned char type    = 0;
    bool have_status      = false;
    unsigned int status   = 0;
    std::string head_hex;
};

// lenenc 前缀长度（返回下一个字段起点）
static unsigned int skip_lenenc(const std::vector<unsigned char> &d, unsigned int off)
{
    if (off >= d.size())
        return (unsigned int)d.size();
    unsigned char f = d[off];
    if (f < 0xFB)
        return off + 1;
    if (f == 0xFB)
        return off + 1; // NULL
    if (f == 0xFC)
        return off + 3;
    if (f == 0xFD)
        return off + 4;
    if (f == 0xFE)
        return off + 9;
    return off + 1;
}

static pkt_info describe(const std::vector<unsigned char> &d)
{
    pkt_info p;
    if (d.size() < 5)
    {
        p.head_hex = hex_of(d.data(), d.size());
        return p;
    }
    p.len  = d[0] | (d[1] << 8) | (d[2] << 16);
    p.type = d[4];
    p.head_hex = hex_of(d.data(), d.size() < 24 ? d.size() : 24);
    if (p.type == 0x00)
    {
        // OK：[0x00][lenenc affected][lenenc lastinsert][2B status_flags]
        unsigned int off = skip_lenenc(d, 5);
        off              = skip_lenenc(d, off);
        if (off + 2 <= d.size())
        {
            p.status     = d[off] | (d[off + 1] << 8);
            p.have_status = true;
        }
    }
    else if (p.type == 0xFE && p.len < 0xFFFFFFu)
    {
        // OK-via-EOF 终结包：[0xFE][2B warning][2B status_flags]
        if (d.size() >= 9)
        {
            p.status      = d[7] | (d[8] << 8);
            p.have_status = true;
        }
    }
    return p;
}

static std::string hex4(unsigned int v)
{
    char tmp[8];
    std::snprintf(tmp, sizeof(tmp), "%04x", v);
    return std::string(tmp);
}

static std::string bits_of(const pkt_info &p)
{
    if (!p.have_status)
        return "no-status";
    std::string s;
    s += "0x" + hex4(p.status);
    s += (p.status & 0x0001) ? " IN_TRANS" : " not_in_trans";
    s += (p.status & 0x0002) ? " AUTOCOMMIT" : " no_autocommit";
    if (p.status & 0x0008)
        s += " MORE_RESULTS";
    if (p.status & 0x0010)
        s += " NO_GOOD_INDEX";
    if (p.status & 0x0020)
        s += " NO_INDEX_USED";
    if (p.status & 0x0040)
        s += " RAW_MODE";
    if (p.status & 0x0100)
        s += " PERSIST_READ_ONLY";
    if (p.status & 0x0200)
        s += " NO_BACKSLASH_ESCAPES";
    if (p.status & 0x0400)
        s += " TABLES_CHANGED";
    if (p.status & 0x0800)
        s += " EXPLICIT_COMMIT";
    if (p.status & 0x4000)
        s += " IN_READONLY_TXN";
    return s;
}

// 发一条语句，读完它的全部响应包，返回终结包
static pkt_info raw_query(packet_reader &r, const std::string &sql, std::string &trace)
{
    pkt_info last;
    r.c->error_msg.clear();
    r.c->error_code = 0;
    if (r.c->write_sql(sql) == 0)
    {
        trace = "write_failed:" + r.c->error_msg;
        return last;
    }
    trace.clear();
    for (int guard = 0; guard < 64; ++guard)
    {
        auto d = r.next();
        if (d.size() < 5)
        {
            trace += " short_read:" + r.c->error_msg;
            break;
        }
        pkt_info p = describe(d);
        trace += " [" + std::to_string(p.len) + "B type=0x" +
                 std::string(hex_of(d.data() + 4, 1)) + "]";
        if (p.type == 0x00 || p.type == 0xFF || p.type == 0xFE)
        {
            last = p;
            break;
        }
    }
    return last;
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    std::thread watchdog(
        []()
        {
            for (int i = 0; i < 400; ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::printf("WATCHDOG fired\n");
            std::exit(99);
        });
    watchdog.detach();
    asio::io_context ioc;
    orm::init_orm_conn_pool(ioc, "conf/orm.conf");
    pool_t pool = orm::get_orm_conn_pool_obj()["cms"];
    if (!pool)
    {
        std::printf("FAIL pool_cms\n");
        return 1;
    }
    std::thread io_thread(
        [&ioc]()
        {
            auto guard = asio::make_work_guard(ioc);
            ioc.run();
        });

    // 建一张自己的表，避免与主探针夹具耦合
    auto ddl = pool->get_mysql_edit_conn();
    if (!ddl)
    {
        std::printf("FAIL borrow_ddl\n");
        return 1;
    }
    ddl->exec_dml("DROP TABLE IF EXISTS _mysql_probe_sf");
    unsigned int made = ddl->exec_dml(
        "CREATE TABLE _mysql_probe_sf (id INT PRIMARY KEY, v INT) ENGINE=InnoDB");
    expect(made == 0, "create_sf_table", "r=" + std::to_string(made) + " err=" + ddl->error_msg);
    pool->back_mysql_edit_conn(std::move(ddl));

    struct capture
    {
        std::string v;
        bool operator()(int cc, char **, std::function<std::tuple<unsigned char *, size_t>(int)> get)
        {
            if (cc >= 1)
            {
                auto [p, l] = get(0);
                if (p)
                    v.assign(reinterpret_cast<char *>(p), l);
            }
            return true;
        }
    };

    auto read_var = [](conn_t &c, const std::string &sql) -> std::string
    {
        capture cap;
        c->fetch_directly(sql, std::ref(cap));
        return cap.v;
    };

    // ---------- 1. 握手包里的 status_flags 现值（(a) 的免费种子） ----------
    {
        auto c = pool->get_mysql_select_conn();
        reading("session_autocommit", read_var(c, "SELECT @@session.autocommit"));
        reading("global_autocommit", read_var(c, "SELECT @@global.autocommit"));
        reading("global_init_connect", read_var(c, "SELECT @@global.init_connect"));
        reading("session_tx_isolation", read_var(c, "SELECT @@session.transaction_isolation"));
        unsigned int hh = c->server_hello.status_flags;
        std::string hs;
        hs += "0x" + hex4(hh);
        hs += (hh & 0x0001) ? " IN_TRANS" : " not_in_trans";
        hs += (hh & 0x0002) ? " AUTOCOMMIT" : " no_autocommit";
        hs += (hh & 0x0200) ? " NO_BACKSLASH_ESCAPES" : "";
        reading("handshake_status_flags", hs);
        c->hard_close();
        pool->back_mysql_select_conn(std::move(c));
    }

    // ---------- 2. SET autocommit=0 自己的 OK 包里该位是否已经清零 ----------
    {
        auto c = pool->get_mysql_select_conn();
        packet_reader r(c);
        std::string tr;
        pkt_info p = raw_query(r, "SET autocommit=0", tr);
        reading("set_autocommit0_packet", bits_of(p) + tr);
        expect(p.have_status && !(p.status & 0x0002), "set0_clears_autocommit_bit_same_packet",
               bits_of(p));

        std::string tr2;
        pkt_info i1 = raw_query(r, "INSERT INTO _mysql_probe_sf VALUES (9001, 1)", tr2);
        reading("insert_after_set0", bits_of(i1) + tr2);
        expect(i1.have_status && (i1.status & 0x0001), "insert_reports_in_trans", bits_of(i1));

        std::string tr3;
        pkt_info sel = raw_query(r, "SELECT id FROM _mysql_probe_sf WHERE id=9001", tr3);
        reading("select_terminator_after_set0", bits_of(sel) + tr3);

        std::string tr4;
        pkt_info cm = raw_query(r, "COMMIT", tr4);
        reading("commit_after_set0", bits_of(cm) + tr4);
        expect(cm.have_status && !(cm.status & 0x0001) && !(cm.status & 0x0002),
               "commit_keeps_autocommit_cleared", bits_of(cm));

        std::string tr5;
        pkt_info back = raw_query(r, "SET autocommit=1", tr5);
        reading("set_autocommit1", bits_of(back) + tr5);
        expect(back.have_status && (back.status & 0x0002), "set1_restores_autocommit_bit",
               bits_of(back));

        c->hard_close();
        pool->back_mysql_select_conn(std::move(c));
    }

    // ---------- 3. 显式 BEGIN 事务里的状态位 ----------
    {
        auto c = pool->get_mysql_select_conn();
        packet_reader r(c);
        std::string tr;
        pkt_info b = raw_query(r, "BEGIN", tr);
        reading("begin_packet", bits_of(b) + tr);
        // BEGIN 那一刻事务尚未有语句，服务端可能仍报 not_in_trans；只记录不设判据

        std::string tr2;
        pkt_info i1 = raw_query(r, "INSERT INTO _mysql_probe_sf VALUES (9002, 2)", tr2);
        reading("insert_in_begin_txn", bits_of(i1) + tr2);
        expect(i1.have_status && (i1.status & 0x0001), "begin_txn_reports_in_trans", bits_of(i1));

        std::string tr3;
        pkt_info sel = raw_query(r, "SELECT id FROM _mysql_probe_sf WHERE id=9002", tr3);
        reading("select_terminator_in_begin_txn", bits_of(sel) + tr3);
        expect(sel.have_status && (sel.status & 0x0001), "select_terminator_reports_in_trans",
               bits_of(sel));

        std::string tr4;
        pkt_info rb = raw_query(r, "ROLLBACK", tr4);
        reading("rollback_packet", bits_of(rb) + tr4);
        expect(rb.have_status && !(rb.status & 0x0001), "rollback_clears_in_trans", bits_of(rb));

        c->hard_close();
        pool->back_mysql_select_conn(std::move(c));
    }

    // ---------- 4. 错误包不带 status_flags（(a) 的刷新盲区） ----------
    {
        auto c = pool->get_mysql_select_conn();
        packet_reader r(c);
        std::string tr;
        pkt_info e = raw_query(r, "INSERT INTO no_such_table_xyz VALUES (1)", tr);
        reading("err_packet_type", "0x" + std::string(hex_of(&e.type, 1)) + tr);
        expect(!e.have_status, "err_packet_has_no_status_flags");

        std::string tr2;
        pkt_info ok = raw_query(r, "SELECT 1", tr2);
        reading("next_packet_after_err", bits_of(ok) + tr2);
        c->hard_close();
        pool->back_mysql_select_conn(std::move(c));
    }

    // ---------- 5. DDL 隐式提交 / 多结果集标志 ----------
    {
        auto c = pool->get_mysql_select_conn();
        packet_reader r(c);
        std::string tr;
        raw_query(r, "BEGIN", tr);
        std::string tr2;
        pkt_info ddlp = raw_query(r, "CREATE TABLE _mysql_probe_sf2 (id INT PRIMARY KEY) ENGINE=InnoDB",
                                  tr2);
        reading("ddl_ok_packet", bits_of(ddlp) + tr2);
        raw_query(r, "DROP TABLE IF EXISTS _mysql_probe_sf2", tr);
        c->hard_close();
        pool->back_mysql_select_conn(std::move(c));
    }

    std::printf("SUMMARY pass=%d fail=%d\n", g_pass, g_fail);
    pool.reset();
    ioc.stop();
    io_thread.join();
    orm::init_orm_conn_pool_release();
    return g_fail == 0 ? 0 : 1;
}
