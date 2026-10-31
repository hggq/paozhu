/*
 * prepfix_probe.cpp — 预编译通路占位符/尾串修复的真驱动探针（2026-10-31 第 8 条）
 *
 * 口径: 不走 conf/orm.conf，自建一枚 orm::sqlite_conn_base 直连夹具库文件，
 *       调用产品里的 fetch_prepared / exec_dml_prepared 本体（不是同形复刻）。
 * 用法: 从仓库根启动；PAOZHU_PROBE_TMP 指夹具目录，默认 ./temp
 * 输出: PASS/FAIL 判定行 + READ 读数行
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <asio.hpp>
// sqlite_conn.h 必须排在其它框架头之前：同名 include guard 由先展开者占有
#include "sqlite_conn.h"
#include "orm_conn_pool.h"

using conn_ptr   = std::shared_ptr<orm::sqlite_conn_base>;
using objval_vec = std::vector<http::obj_val>;

static unsigned int g_pass = 0, g_fail = 0;

static void CHECK(bool ok, const std::string &id, const std::string &msg)
{
    ok ? g_pass++ : g_fail++;
    std::printf("%s %s %s\n", ok ? "PASS" : "FAIL", id.c_str(), msg.c_str());
}

static void READ(const std::string &id, const std::string &val)
{
    std::printf("READ %s %s\n", id.c_str(), val.c_str());
}

// 把 error_msg 里的控制字符压掉，便于当作读数打印
static std::string esc(const std::string &in)
{
    std::string out;
    for (char ch : in)
        out.push_back((unsigned char)ch < 32 ? ' ' : ch);
    return out;
}

static orm::orm_conn_t lite_conf(const std::string &path)
{
    orm::orm_conn_t conf;
    conf.host   = path;
    conf.dbtype = "sqlite";
    conf.tag    = "lite";
    return conf;
}

// 抓第一列文本
struct one_val
{
    std::string text;
    unsigned int rows = 0;
};

static one_val fetch_one_col(conn_ptr &c, const std::string &sql, const objval_vec &pp)
{
    one_val out;
    unsigned int n = c->fetch_prepared(
        sql, pp,
        [&out](int, char **, auto get) -> bool
        {
            auto [p, l] = get(0);
            if (p)
                out.text = std::string(reinterpret_cast<char *>(p), l);
            out.rows++;
            return true;
        });
    out.rows = n;
    return out;
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);

    const char *tmpenv = std::getenv("PAOZHU_PROBE_TMP");
    std::string dir    = tmpenv && *tmpenv ? tmpenv : "./temp";
    std::string f      = dir + "/pfx1031.db";
    std::remove(f.c_str());
    std::remove((f + "-wal").c_str());
    std::remove((f + "-shm").c_str());

    auto c = std::make_shared<orm::sqlite_conn_base>();
    if (!c->connect(lite_conf(f)))
    {
        CHECK(false, "P0", "自建连接失败: " + esc(c->error_msg));
        std::printf("SUMMARY pass=%u fail=%u\n", g_pass, g_fail);
        return 1;
    }
    c->exec_dml("CREATE TABLE p (id INTEGER PRIMARY KEY, k TEXT)");
    c->exec_dml("INSERT INTO p VALUES (1,'a1'),(2,'a2'),(3,'a3')");
    READ("P0setup", "rows=" + std::to_string(fetch_one_col(c, "SELECT count(*) FROM p", objval_vec{}).rows) +
                        " err=" + esc(c->error_msg));

    // ---- 8-a: ?NNN 是匿名占位符的编号写法，必须放行且真绑上位 ----
    auto bare = fetch_one_col(c, "SELECT k FROM p WHERE id = ?", objval_vec{http::obj_val((long long)1)});
    CHECK(bare.rows == 1 && bare.text == "a1" && c->error_msg.empty(), "P1a",
          "裸 ? 仍然可用 rows=" + std::to_string(bare.rows) + " val=" + bare.text + " err=" + esc(c->error_msg));

    auto nn = fetch_one_col(c, "SELECT k FROM p WHERE id = ?1", objval_vec{http::obj_val((long long)1)});
    CHECK(nn.rows == 1 && nn.text == "a1" && c->error_msg.empty(), "P1b",
          "?NNN 不再被命名占位符守卫误拒 rows=" + std::to_string(nn.rows) + " val=" + nn.text +
              " err=" + esc(c->error_msg));

    auto n2 = fetch_one_col(c, "SELECT k FROM p WHERE id = ?2 AND k = ?1",
                            objval_vec{http::obj_val(std::string("a2")), http::obj_val((long long)2)});
    READ("P1c", "two_named_numbered_rows=" + std::to_string(n2.rows) + " val=" + n2.text +
                     " err=" + esc(c->error_msg));
    CHECK(n2.rows == 1 && n2.text == "a2" && c->error_msg.empty(), "P1c",
          "两个 ?NNN 按编号顺序绑定（槽1=第1个值, 槽2=第2个值）");

    unsigned int upd = c->exec_dml_prepared("UPDATE p SET k = ?2 WHERE id = ?1",
                                            objval_vec{http::obj_val((long long)3), http::obj_val(std::string("u3"))});
    auto after = fetch_one_col(c, "SELECT k FROM p WHERE id = 3", objval_vec{});
    READ("P1d", "effect=" + std::to_string(upd) + " now=" + after.text + " err=" + esc(c->error_msg));
    CHECK(upd == 1 && after.text == "u3" && c->error_msg.empty(), "P1d",
          "?NNN 走 DML 通路真实写库(值落在编号对应槽位)");

    // ---- 8-c: 命名占位符仍拒绝，且给可读文本 ----
    // 文案既列出三种命名形状（:name/@name/$name），也复述调用方实际写的那个名字。
    for (const char *form : {":iid", "@iid", "$iid"})
    {
        std::string sql = std::string("SELECT k FROM p WHERE id = ") + form;
        auto r          = fetch_one_col(c, sql, objval_vec{http::obj_val((long long)1)});
        bool text_ok    = c->error_msg.find("named placeholder") != std::string::npos &&
                       c->error_msg.find("use ? or ?NNN") != std::string::npos &&
                       c->error_msg.find(std::string("'") + form + "'") != std::string::npos;
        CHECK(r.rows == 0 && text_ok, std::string("P2") + form[0],
              std::string(form) + " 被拒且文案可读: rows=" + std::to_string(r.rows) + " err=" + esc(c->error_msg));
    }

    // ---- 8-b: 尾串里的注释不构成第二条语句 ----
    struct tail_case
    {
        const char *sql;
        const char *id;
        int want;  // 1=取到 1 行, 0=只要求"没被当多语句拒"
        bool reject;
    };
    const tail_case cases[] = {
        { "SELECT k FROM p WHERE id = ?; -- 行尾注释", "P3a", 1, false },
        { "SELECT k FROM p WHERE id = ?; /* 块注释 */", "P3b", 1, false },
        { "SELECT k FROM p WHERE id = ?; -- 只有行注释\n", "P3c", 1, false },
        { "SELECT k FROM p WHERE id = ?;", "P3d", 1, false },
        { "SELECT k FROM p WHERE id = ?; -- 注释\nDELETE FROM p", "P4a", 0, true },
        { "SELECT k FROM p WHERE id = ?; DELETE FROM p", "P4b", 0, true },
        { "SELECT k FROM p WHERE id = ?; /* 未闭合", "P4c", 0, false },  // 交给 prepare 报语法错
    };
    for (const auto &tc : cases)
    {
        objval_vec pp{http::obj_val((long long)1)};
        unsigned int n = c->fetch_prepared(
            tc.sql, pp, [](int, char **, auto) -> bool { return true; });
        std::string err = esc(c->error_msg);
        bool multi      = err.find("must contain a single statement") != std::string::npos;
        if (tc.reject)
        {
            CHECK(multi, tc.id, std::string("仍拒多语句: rows=") + std::to_string(n) + " err=" + err + " sql=" + tc.sql);
        }
        else
        {
            bool ok = !multi && (tc.want == 0 || n == 1);
            CHECK(ok, tc.id,
                  std::string("不误判为多语句: rows=") + std::to_string(n) + " err=" + err + " sql=" + tc.sql);
        }
    }
    auto left = fetch_one_col(c, "SELECT count(*) FROM p", objval_vec{});
    READ("P4rows", "count_col_rows=" + std::to_string(left.rows) + " first_value=" + left.text);

    // ---- 个数不匹配仍是 RANGE 文案（8-c 泛化文本不再吃掉可识别错误）----
    // 故意用裸 ?：这条臂不能依赖 ?NNN，否则突变臂里它会因"被命名守卫拒掉"而红，
    // 归因就落在别的臂的原因上（读数是 named placeholder 而不是 count mismatch）。
    fetch_one_col(c, "SELECT k FROM p WHERE id = ? AND k = ?", objval_vec{http::obj_val((long long)1)});
    bool range_ok = c->error_msg.find("count mismatch") != std::string::npos &&
                    c->error_msg.find("2 placeholder(s)") != std::string::npos &&
                    c->error_msg.find("1 value(s)") != std::string::npos;
    CHECK(range_ok, "P5", "个数不匹配给明细文案 err=" + esc(c->error_msg));

    // ---- 引擎错误不被私有文案覆盖（守卫没有吞掉真正的语法错文本）----
    fetch_one_col(c, "SELECT k FROM nosuchtable_zz WHERE id = ?", objval_vec{http::obj_val((long long)1)});
    bool engine_ok = c->error_msg.find("named placeholder") == std::string::npos && !c->error_msg.empty();
    CHECK(engine_ok, "P6", " prepare 失败时沿用引擎文本 err=" + esc(c->error_msg));

    c->close();
    std::remove(f.c_str());
    std::remove((f + "-wal").c_str());
    std::remove((f + "-shm").c_str());

    std::printf("SUMMARY pass=%u fail=%u\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
