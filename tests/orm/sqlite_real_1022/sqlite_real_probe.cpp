/*
 * sqlite_real_probe.cpp — SQLite 驱动真库探针
 * 口径: 走 conf/orm.conf 的 [lite] 常驻连接(单条 edit + 单条 select, 同一 worker 线程串行),
 *       另用直连 sqlite_conn_base 做损坏库/跨写者锁/停 worker 等臂。
 * 用法: sqlite_real_probe [族列表]   例: sqlite_real_probe A,B   默认 all = A,B,C,D,E,F,R
 *       G 不在默认集合里：它把同一句柄放到多个线程上跑，允许崩溃，需显式传 G 且单独一次进程。
 *       从仓库根启动(库文件、夹具目录和 conf/orm.conf 都按当前目录解析)；
 *       PAOZHU_LITE_DB / PAOZHU_PROBE_TMP 可把库文件与夹具目录指到别处。
 * 输出: PASS/FAIL 判定行 + READ 读数行(真值, 不作判定)
 */
#include <algorithm>
#include <atomic>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>
// sqlite_conn.h 必须排在其它框架头之前：那些头引同名文件时命中的是它们自己的同级副本，
// 谁先被展开谁就占掉 include guard。
#include "sqlite_conn.h"
#include "orm_conn_pool.h"
#include "orm_query.h"
#include "request.h"
// 模型侧的臂(R6a/R6b)要用重新生成后的 lite 中间层，包含路径按仓库根解析
#include "orm/lite/include/fortune_base.h"
#include "orm/lite/include/fortune_opsql.h"

// 探针自带的模型绑定类：与 models/lite/Fortune.cpp 同形，默认构造按 fortune_base 里的
// _rmstag 绑到 [lite] 池。产品里那一枚的构造函数定义在 models/*.cpp，探针不链产品目标，
// 所以自己实例化一次生成模板；生成码的元组调用点是同一份，这正是本臂要验的东西。
namespace probe_model
{
    class Fortune : public orm::lite::fortune_opsql<Fortune, orm::lite::fortune_base>
    {
      public:
        Fortune() : orm::lite::fortune_opsql<Fortune, orm::lite::fortune_base>()
        {
            mod = this;
        }
    };
}  // namespace probe_model

using clk          = std::chrono::high_resolution_clock;
using conn_ptr     = std::shared_ptr<orm::sqlite_conn_base>;
using bind_vec     = std::vector<orm::sqlite_bind_param>;
using objval_vec   = std::vector<http::obj_val>;

// 库文件与夹具目录都按当前工作目录解析，和 main 里的 "conf/orm.conf" 是同一个前提：
// 从仓库根启动。两个环境变量各指一处，库在别处或想把夹具写到别的盘时不用改码。
static std::string env_or(const char *name, const char *fallback)
{
    const char *v = std::getenv(name);
    return (v && *v) ? std::string(v) : std::string(fallback);
}

static const std::string kLiteDb = env_or("PAOZHU_LITE_DB", "docs/sqlite_lite.db");
static const std::string kTmpDir = env_or("PAOZHU_PROBE_TMP", "temp/sqlite_real_1022");

static int g_pass = 0, g_fail = 0;
static std::set<std::string> g_families;

static bool fam(const char *f)
{
    return g_families.empty() || g_families.count(f) > 0;
}

static void CHECK(bool cond, const std::string &id, const std::string &name)
{
    if (cond)
    {
        g_pass++;
        std::printf("PASS %s %s\n", id.c_str(), name.c_str());
    }
    else
    {
        g_fail++;
        std::printf("FAIL %s %s\n", id.c_str(), name.c_str());
    }
}

static void CHECK_EQ(const std::string &got, const std::string &expect, const std::string &id, const std::string &name)
{
    bool ok = (got == expect);
    CHECK(ok, id, name);
    if (!ok)
    {
        std::printf("     got=[%s] expect=[%s]\n", got.c_str(), expect.c_str());
    }
}

static void READ(const std::string &id, const std::string &kv)
{
    std::printf("READ %s %s\n", id.c_str(), kv.c_str());
}

static std::string esc(const std::string &s)
{
    std::string o;
    for (unsigned char c : s)
    {
        if (c < 32 || c > 126)
        {
            char b[8];
            std::snprintf(b, sizeof(b), "\\x%02x", c);
            o += b;
        }
        else
        {
            o.push_back((char)c);
        }
    }
    return o;
}

// 引擎真实事务状态（驱动内部就以此为准），探针用它做"标记 vs 引擎"对照
static bool engine_in_txn(const conn_ptr &c)
{
    return c->get_db() != nullptr && sqlite3_get_autocommit(c->get_db()) == 0;
}

static std::string scalar_text(orm::sqlite_conn_base &c, const std::string &sql)
{
    orm::sqlite_scalar_result r;
    if (!c.query_scalar(sql, r))
    {
        return std::string("<ERR:") + c.error_msg + ">";
    }
    if (r.is_null)
    {
        return "<NULL>";
    }
    return r.value;
}

static unsigned int count_rows(orm::sqlite_conn_base &c, const std::string &sql)
{
    unsigned int n = 0;
    c.fetch_directly(sql, [&n](int, char **, auto) mutable -> bool
                     {
        n++;
        return true; });
    return n;
}

// 逐条回查 {rowid, 期望的 k}：wrong_owner=行不属于该调用方, duplicate=两个调用方拿到同一 id
// 表名/列名默认是探针夹具；模型侧的臂要用生成码认得的真实表(fortune.message)。
static void verify_rowids(const std::vector<std::pair<long long, std::string>> &items,
                          const conn_ptr &sel,
                          unsigned int &wrong_owner,
                          unsigned int &duplicate,
                          unsigned int &invalid,
                          const std::string &table = "_pz_probe_t1",
                          const std::string &col   = "k")
{
    wrong_owner = 0;
    duplicate   = 0;
    invalid     = 0;
    std::vector<long long> ids;
    for (auto &it : items)
    {
        if (it.first <= 0)
        {
            invalid++;
            continue;
        }
        ids.push_back(it.first);
        std::string got;
        orm::sqlite_scalar_result sr;
        if (sel->query_scalar("SELECT " + col + " FROM " + table + " WHERE id=" + std::to_string(it.first), sr) &&
            !sr.is_null)
            got = sr.value;
        if (got != it.second)
            wrong_owner++;
    }
    std::sort(ids.begin(), ids.end());
    for (size_t i = 1; i < ids.size(); i++)
        if (ids[i] == ids[i - 1])
            duplicate++;
}

// fetch_directly 首行各列 (ptr,len) 快照
struct ColSnap
{
    bool is_null = false;
    size_t len   = 0;
    std::string bytes;
};

static std::vector<ColSnap> first_row(orm::sqlite_conn_base &c, const std::string &sql)
{
    std::vector<ColSnap> out;
    c.fetch_directly(sql, [&out](int col_count, char **, auto get_data) mutable -> bool
                     {
        for (int i = 0; i < col_count; i++)
        {
            auto [ptr, len] = get_data(i);
            ColSnap s;
            s.is_null = (ptr == nullptr);
            s.len     = len;
            if (ptr && len)
                s.bytes.assign(reinterpret_cast<const char *>(ptr), len);
            out.push_back(std::move(s));
        }
        return false; });
    return out;
}

// 二进制版首行各列 typed 值
static std::vector<std::string> first_row_binary(orm::sqlite_conn_base &c, const std::string &sql,
                                                 const objval_vec &params)
{
    std::vector<std::string> out;
    c.fetch_prepared_binary(sql, params, [&out](int col_count, char **, auto get_data) mutable -> bool
                            {
        for (int i = 0; i < col_count; i++)
        {
            orm::col_value_variant v = get_data(i);
            std::string t            = std::visit(
                [](auto &&x) -> std::string
                {
                    using X = std::decay_t<decltype(x)>;
                    if constexpr (std::is_same_v<X, std::monostate>)
                        return "NULL";
                    else if constexpr (std::is_same_v<X, int64_t>)
                        return "int:" + std::to_string(x);
                    else if constexpr (std::is_same_v<X, uint64_t>)
                        return "uint:" + std::to_string(x);
                    else if constexpr (std::is_same_v<X, double>)
                    {
                        char b[64];
                        std::snprintf(b, sizeof(b), "dbl:%.17g", x);
                        return b;
                    }
                    else
                        return "sv:" + std::to_string(x.size()) + "/" + esc(std::string(x));
                },
                v);
            out.push_back(t);
        }
        return false; });
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

// ============ 族 A：连接 / PRAGMA / 池 / 空语句 / 错误文本 ============
static void fam_A(const conn_ptr &edit, const conn_ptr &sel)
{
    if (!fam("A"))
        return;
    std::printf("\n---- A: connect / pragma / pool ----\n");

    CHECK(edit->ping(), "A1", "edit ping");
    CHECK(sel->ping(), "A2", "select ping");
    CHECK(!edit->is_closed(), "A3", "edit is_closed=false");
    CHECK(edit->isclose == false, "A4", "isclose 成员与 is_closed() 同步");

    CHECK_EQ(scalar_text(*sel, "PRAGMA journal_mode"), "wal", "A5", "journal_mode=wal");
    CHECK_EQ(scalar_text(*sel, "PRAGMA busy_timeout"), "5000", "A6", "busy_timeout=5000");
    CHECK_EQ(scalar_text(*sel, "PRAGMA foreign_keys"), "1", "A7", "foreign_keys=ON");
    CHECK_EQ(scalar_text(*sel, "PRAGMA synchronous"), "1", "A8", "synchronous=NORMAL(1)");
    CHECK_EQ(scalar_text(*sel, "PRAGMA temp_store"), "2", "A9", "temp_store=MEMORY(2)");
    CHECK_EQ(scalar_text(*sel, "PRAGMA cache_size"), "10000", "A10", "cache_size=10000");

    // 池: 同一 tag 的 edit / select 各自常驻且共享
    auto pool = orm::get_orm_conn_pool_obj()["lite"];
    auto e2   = pool->get_sqlite_edit_conn();
    CHECK(e2.get() == edit.get(), "A11", "get_sqlite_edit_conn 两次同一对象(单连接常驻)");
    auto s2 = pool->get_sqlite_select_conn();
    CHECK(s2.get() == sel.get(), "A12", "get_sqlite_select_conn 两次同一对象");
    CHECK(edit.get() != sel.get(), "A13", "edit 与 select 是两条句柄");

    // 空串 / 纯注释: 各入口应"成功且零变更", 不留错误文本
    edit->exec_dml("SELECT 1/0 FROM _pz_no_such_table");
    READ("A14", "stale_error_before=" + esc(edit->error_msg));
    const char *empties[] = {"", "   \n\t", ";", "-- only comment", "/* unterminated",
                             "/* closed */ ;", "-- a\n-- b\n"};
    for (const char *q : empties)
    {
        std::string tag = std::string("\"") + q + "\"";
        int r1          = edit->exec_sql(q);
        unsigned int r2 = edit->exec_dml(q);
        orm::sqlite_query_result qr;
        bool r3         = edit->query_fetch(q, qr);
        bind_vec no_p;
        bool r4     = edit->exec_bound(q, no_p);
        unsigned int r5 = edit->fetch_prepared(q, objval_vec{}, [](int, char **, auto) { return true; });
        unsigned int r6 = edit->exec_dml_prepared(q, objval_vec{});
        unsigned int r7 = edit->fetch_prepared_binary(q, objval_vec{}, [](int, char **, auto) { return true; });
        std::ostringstream os;
        os << "sql=" << esc(q) << " exec_sql=" << r1 << " exec_dml=" << r2
           << " query_fetch=" << (r3 ? "T" : "F") << "/" << qr.rows.size() << "/" << qr.column_names.size()
           << " exec_bound=" << (r4 ? "T" : "F") << " fetch_prepared=" << r5 << " exec_dml_prepared=" << r6
           << " fetch_prepared_binary=" << r7 << " err=[" << esc(edit->error_msg) << "]";
        CHECK(r1 == 0 && r2 == 0 && r3 && qr.rows.empty() && r4 && r5 == 0 && r6 == 0 && r7 == 0 &&
                  edit->error_msg.empty(),
              "A15", "空/注释语句全入口零变更无错误 " + tag);
        READ("A15", os.str());
    }

    // A19 exec_bound 入口不 clear_error：成功语句之后仍带着上一条失败语句的错误文本
    {
        edit->exec_dml("SELEKT stale probe");
        std::string stale = edit->error_msg;
        CHECK(!stale.empty(), "A19a", "先制造一条失败语句的错误文本");
        bind_vec bp;
        orm::sqlite_bind_param a;
        a.type = SQLITE_TEXT;
        a.data = "stale_ok";
        bp.push_back(a);
        bool ok = edit->exec_bound("INSERT INTO _pz_probe_t1 (k) VALUES (?)", bp);
        READ("A19b", "exec_bound_ok=" + std::to_string(ok) + " err_still=[" + esc(edit->error_msg) + "]");
        CHECK(ok && edit->error_msg.empty(), "A19c", "成功的 exec_bound 应清掉陈旧 error_msg(与其它入口一致)");
        edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k='stale_ok'");
    }

    // DDL/表反射
    auto tables = sel->get_table_list();
    bool has_t1 = false;
    for (auto &t : tables)
        if (t == "_pz_probe_t1")
            has_t1 = true;
    CHECK(has_t1, "A16", "get_table_list 含探针表");
    auto cols = sel->get_table_info("_pz_probe_t1");
    std::string colnames;
    for (auto &c : cols)
    {
        colnames += c["name"];
        colnames += ",";
    }
    READ("A17", "table_info=" + colnames);
    CHECK(colnames.find("k,") != std::string::npos && colnames.find("num,") != std::string::npos,
          "A17", "get_table_info 列名/类型可读");

    auto idx = sel->get_index_list("_pz_probe_u");
    std::string idxs;
    for (auto &i : idx)
        idxs += i + ",";
    READ("A18", "index_list(_pz_probe_u)=" + idxs + " count=" + std::to_string(idx.size()));
    // 唯一约束(UNIQUE 自动索引)与手工索引都应在列表里
    CHECK(idxs.find("idx_pz_probe_u_k") != std::string::npos, "A18b", "手工索引出现在 index_list");
    if (!idx.empty())
    {
        auto info = sel->get_index_info(idx[0]);
        std::string s;
        for (auto &x : info)
            s += x + ",";
        READ("A18c", "index_info(" + idx[0] + ")=" + s);
    }
}

// ============ 族 B：读路径与值保真 ============
static void fam_B(const conn_ptr &edit, const conn_ptr &sel)
{
    if (!fam("B"))
        return;
    std::printf("\n---- B: read paths / value fidelity ----\n");

    // ---- B1: NULL vs 空文本 vs 空 BLOB ----
    {
        bind_vec bp;
        orm::sqlite_bind_param k;
        k.type = SQLITE_TEXT;
        k.data = "b1b";
        bp.push_back(k);
        orm::sqlite_bind_param t;
        t.type = SQLITE_TEXT;
        t.data = "";
        bp.push_back(t);
        orm::sqlite_bind_param b;
        b.type = SQLITE_BLOB;// 空 BLOB：有效指针 + 长度 0
        b.data = "";
        bp.push_back(b);
        orm::sqlite_bind_param n;
        n.type = SQLITE_NULL;
        bp.push_back(n);
        bool ok = edit->exec_bound("INSERT INTO _pz_probe_t1 (k, txt, blb, nul) VALUES (?,?,?,?)", bp);
        CHECK(ok, "B1a", "exec_bound 绑空文本/空 BLOB/NULL 成功");
        READ("B1a", "typeof=" + scalar_text(*sel, "SELECT typeof(txt)||'/'||typeof(blb)||'/'||typeof(nul) FROM _pz_probe_t1 WHERE k='b1b'"));

        auto snaps = first_row(*sel, "SELECT txt, blb, nul FROM _pz_probe_t1 WHERE k='b1b'");
        if (snaps.size() == 3)
        {
            std::ostringstream os;
            os << "txt{null=" << snaps[0].is_null << ",len=" << snaps[0].len << "} "
               << "blb{null=" << snaps[1].is_null << ",len=" << snaps[1].len << "} "
               << "nul{null=" << snaps[2].is_null << ",len=" << snaps[2].len << "}";
            READ("B1b", os.str());
            CHECK(!snaps[0].is_null && snaps[0].len == 0, "B1b", "fetch_directly: 空文本 ptr 非空 len 0(可与 NULL 区分)");
            CHECK(snaps[2].is_null, "B1c", "fetch_directly: NULL 列 ptr=nullptr");
            CHECK(!snaps[1].is_null, "B1d", "fetch_directly: 空 BLOB 不应与 NULL 同形");
        }
        else
        {
            CHECK(false, "B1b", "fetch_directly 取到 3 列");
        }

        orm::sqlite_query_result qr;
        sel->query_fetch("SELECT txt, blb, nul FROM _pz_probe_t1 WHERE k='b1b'", qr);
        if (!qr.rows.empty() && qr.is_null.size() == 1 && qr.is_null[0].size() == 3)
        {
            std::ostringstream os;
            os << "query_fetch is_null=[" << qr.is_null[0][0] << "," << qr.is_null[0][1] << "," << qr.is_null[0][2]
               << "] len=[" << qr.rows[0][0].size() << "," << qr.rows[0][1].size() << "," << qr.rows[0][2].size() << "]";
            READ("B1e", os.str());
            CHECK(!qr.is_null[0][0] && !qr.is_null[0][1] && qr.is_null[0][2],
                  "B1e", "query_fetch 位图: 空文本/空 BLOB 非 NULL, NULL 列是 NULL");
        }
        else
        {
            CHECK(false, "B1e", "query_fetch is_null 位图完整");
        }

        auto bin = first_row_binary(*sel, "SELECT txt, blb, nul FROM _pz_probe_t1 WHERE k='b1b'", objval_vec{});
        std::string flat;
        for (auto &bt : bin)
            flat += bt + " | ";
        READ("B1f", "binary=" + flat);
        CHECK(bin.size() == 3 && bin[2] == "NULL", "B1f", "二进制版 NULL 列 -> monostate");
        CHECK(bin.size() == 3 && bin[1].rfind("sv:0/", 0) == 0, "B1g", "二进制版空 BLOB 仍给 string_view(可与 NULL 区分)");
    }

    // ---- B2: 内嵌 NUL ----
    {
        std::string with_nul = std::string("a\0b", 3);
        bind_vec bp;
        orm::sqlite_bind_param a;
        a.type = SQLITE_TEXT;
        a.data = with_nul;
        bp.push_back(a);
        orm::sqlite_bind_param k;
        k.type = SQLITE_TEXT;
        k.data = "b2";
        bp.push_back(k);
        bool ok = edit->exec_bound("INSERT INTO _pz_probe_t1 (txt, k) VALUES (?,?)", bp);
        CHECK(ok, "B2a", "exec_bound 绑定含 NUL 的文本(显式长度)");

        auto s = first_row(*sel, "SELECT txt FROM _pz_probe_t1 WHERE k='b2'");
        READ("B2b", "len=" + std::to_string(s.empty() ? 0 : s[0].len) + " bytes=" + esc(s.empty() ? "" : s[0].bytes));
        CHECK(s.size() == 1 && s[0].len == 3 && s[0].bytes == with_nul, "B2b", "读回含 NUL 文本按字节长度完整返回");

        orm::sqlite_query_result q2;
        sel->query_fetch("SELECT txt FROM _pz_probe_t1 WHERE k='b2'", q2);
        CHECK(!q2.rows.empty() && q2.rows[0][0].size() == 3, "B2c", "query_fetch 保留内嵌 NUL");

        auto b2 = first_row_binary(*sel, "SELECT txt FROM _pz_probe_t1 WHERE k='b2'", objval_vec{});
        CHECK(b2.size() == 1 && b2[0].rfind("sv:3/", 0) == 0, "B2d", "二进制版保留内嵌 NUL");
    }
    // 文本通路(值直接拼进语句)遇 NUL: 引擎按 C 串取 SQL, 语句在 NUL 处截断
    {
        std::string stmt = "INSERT INTO _pz_probe_t1 (txt, k) VALUES ('x";
        stmt += std::string("\0y', 'b2t')", 11);
        unsigned int r = edit->exec_dml(stmt);
        READ("B2e", "text_path_nul_exec_dml=" + std::to_string((int)r) + " err=" + esc(edit->error_msg));
        unsigned int n = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='b2t'");
        READ("B2f", "text_path_nul_row_written=" + std::to_string(n));
        CHECK(r == (unsigned int)-1 || n == 0, "B2e", "文本通路含 NUL 的语句要么失败要么未写入(无静默半写)");
    }

    // ---- B3: REAL 精度往返 ----
    {
        struct Case
        {
            const char *name;
            double v;
        } cases[] = {
            {"0.1", 0.1}, {"1_3", 1.0 / 3.0}, {"1e300", 1e300}, {"mzero", -0.0},
            {"dblmax", DBL_MAX}, {"dblmin", DBL_MIN}, {"1em300", 1e-300}, {"p53", 9007199254740993.0},
        };
        int lossy = 0, lossy_zerofalse = 0;
        for (auto &c : cases)
        {
            objval_vec pp;
            pp.push_back(http::obj_val(c.v));
            pp.push_back(http::obj_val(std::string("r_") + c.name));
            unsigned int r = edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (num, k) VALUES (?,?)", pp);
            auto s         = first_row(*sel, "SELECT num FROM _pz_probe_t1 WHERE k='r_" + std::string(c.name) + "'");
            double back    = s.empty() ? 0.0 : std::strtod(s[0].bytes.c_str(), nullptr);
            bool same      = (r != (unsigned int)-1) && std::memcmp(&back, &c.v, sizeof(double)) == 0;
            if (!same)
            {
                // 引擎直读(double)也丢符号 => 上游 SQLite 存储规格化, 不是 %.17g 通路的问题
                auto bn    = first_row_binary(*sel, "SELECT num FROM _pz_probe_t1 WHERE k='r_" +
                                                 std::string(c.name) + "'", objval_vec{});
                bool false_also = !bn.empty() && bn[0].rfind("dbl:-", 0) != 0;
                if (std::string(c.name) == "mzero" && false_also)
                    lossy_zerofalse++;
                else
                    lossy++;
                std::printf("     精度丢失 case=%s wrote=%.17g read_text=%s rc=%d read_binary=%s\n",
                            c.name, c.v, s.empty() ? "<none>" : s[0].bytes.c_str(), (int)r,
                            bn.empty() ? "<none>" : bn[0].c_str());
            }
        }
        CHECK(lossy == 0, "B3a", "预编译 DOUBLE 绑定 -> %.17g 文本读回, 极端值逐位相同(符号零除外)");
        READ("B3a", "lossy_cases=" + std::to_string(lossy) + " sqlite_storage_normalized=" +
                        std::to_string(lossy_zerofalse) + "(仅 -0.0, 文本与 typed 两条读路同丢)");

        auto bv = first_row_binary(*sel, "SELECT num FROM _pz_probe_t1 WHERE k='r_1_3'", objval_vec{});
        double expect = 1.0 / 3.0;
        char want[64];
        std::snprintf(want, sizeof(want), "dbl:%.17g", expect);
        CHECK_EQ(bv.empty() ? "" : bv[0], want, "B3b", "二进制版 REAL 列以 double 原值返回");

        // 文本 DSL 通路的 double(obj_val::to_string 走 to_chars 最短往返)
        std::string sqltext = "INSERT INTO _pz_probe_t1 (num, k) VALUES (" +
                              http::obj_val(1.0 / 3.0).to_string() + ", 'r_txt')";
        edit->exec_dml(sqltext);
        auto ts    = first_row(*sel, "SELECT num FROM _pz_probe_t1 WHERE k='r_txt'");
        double tback = ts.empty() ? 0.0 : std::strtod(ts[0].bytes.c_str(), nullptr);
        CHECK(ts.size() == 1 && std::memcmp(&tback, &expect, sizeof(double)) == 0, "B3c",
              "文本 DSL 通路 double 字面量逐位往返");
        READ("B3c", "literal=" + http::obj_val(1.0 / 3.0).to_string() + " read=" + (ts.empty() ? "" : ts[0].bytes));

        // 单格 float_buf_ 契约: 立即拷贝 vs 延迟解引用
        {
            objval_vec pp;
            pp.push_back(http::obj_val(1.0 / 3.0));
            pp.push_back(http::obj_val(2.0 / 7.0));
            pp.push_back(http::obj_val("buf"));
            edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (num, num2, k) VALUES (?,?,?)", pp);
            std::string a_copy, b_copy;
            bool deferred_same = false;
            sel->fetch_directly("SELECT num, num2 FROM _pz_probe_t1 WHERE k='buf'",
                                [&](int, char **, auto get_data) -> bool
                                {
                                    auto [pa, la] = get_data(0);
                                    a_copy.assign(reinterpret_cast<const char *>(pa), la);
                                    auto [pb, lb] = get_data(1);
                                    b_copy.assign(reinterpret_cast<const char *>(pb), lb);
                                    // 延迟读: 两格指针都留着, 再回头解引用 a
                                    deferred_same = (la == lb && std::memcmp(pa, pb, la) == 0);
                                    return false;
                                });
            char wa[64], wb[64];
            std::snprintf(wa, sizeof(wa), "%.17g", 1.0 / 3.0);
            std::snprintf(wb, sizeof(wb), "%.17g", 2.0 / 7.0);
            CHECK(a_copy == wa && b_copy == wb, "B3d", "同一次 handler 内逐列立即拷贝: 两个 REAL 都保真");
            READ("B3e", "deferred_a_equals_b=" + std::to_string((int)deferred_same) + " a=" + a_copy + " b=" + b_copy);
        }
    }

    // ---- B4: 整数边界与越界 ----
    {
        objval_vec pp;
        pp.push_back(http::obj_val((long long)INT64_MIN));
        pp.push_back(http::obj_val("i_min"));
        CHECK(edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (ival, k) VALUES (?,?)", pp) != (unsigned int)-1,
              "B4a", "绑定 INT64_MIN 成功");
        auto s = first_row(*sel, "SELECT ival FROM _pz_probe_t1 WHERE k='i_min'");
        CHECK_EQ(s.empty() ? "" : s[0].bytes, std::to_string((long long)INT64_MIN), "B4b", "INT64_MIN 原样读回");

        // INT64_MAX 同界
        objval_vec pm;
        pm.push_back(http::obj_val((long long)INT64_MAX));
        pm.push_back(http::obj_val("i_max"));
        edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (ival, k) VALUES (?,?)", pm);
        auto sm = first_row(*sel, "SELECT ival FROM _pz_probe_t1 WHERE k='i_max'");
        CHECK_EQ(sm.empty() ? "" : sm[0].bytes, std::to_string((long long)INT64_MAX), "B4m", "INT64_MAX 原样读回");

        http::obj_val big;
        big.set_type(http::obj_type::ULONG);
        big.uval = 18446744073709551615ULL;// > INT64_MAX
        objval_vec pp2;
        pp2.push_back(big);
        pp2.push_back(http::obj_val("i_big"));
        unsigned int r = edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (ival, k) VALUES (?,?)", pp2);
        READ("B4c", "ULONG_MAX r=" + std::to_string((int)r) + " err=" + esc(edit->error_msg));
        CHECK(r == (unsigned int)-1 && edit->error_msg.find("invalid/out-of-range") != std::string::npos,
              "B4c", "ULONG 超 INT64_MAX 显式拒绝而非静默变负");
        CHECK(count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='i_big'") == 0, "B4n", "越界绑定未落库");

        objval_vec pp3;
        pp3.push_back(http::obj_val("only_one"));
        unsigned int r3 = edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (k, txt) VALUES (?,?)", pp3);
        READ("B4d", "count_mismatch r=" + std::to_string((int)r3) + " err=" + esc(edit->error_msg));
        CHECK(r3 == (unsigned int)-1 && edit->error_msg.find("count mismatch") != std::string::npos,
              "B4d", "参数个数与占位符不符显式拒绝");
        edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k LIKE 'i_%'");
    }

    // ---- B5: 影响行数语义 ----
    {
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        edit->exec_dml("INSERT INTO _pz_probe_t1 (k, txt) VALUES ('c1','x'),('c2','y'),('c3','z')");
        CHECK(edit->exec_dml("UPDATE _pz_probe_t1 SET txt='q' WHERE k='nope'") == 0, "B5a",
              "UPDATE 未命中返回 0(不是错误)");
        CHECK(edit->exec_dml("UPDATE _pz_probe_t1 SET txt='q' WHERE k IN ('c1','c2')") == 2, "B5b",
              "UPDATE 命中 2 行返回 2");
        CHECK(edit->exec_dml("SELECT count(*) FROM _pz_probe_t1") == 0, "B5c",
              "SELECT 经 exec_dml 返回 0(total_changes 无增量)");
        unsigned int del = edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k LIKE 'c%'");
        CHECK(del == 3, "B5d", "DELETE 3 行返回 3");
        READ("B5d", "del=" + std::to_string(del));
        CHECK(edit->exec_dml("SELECT 1") == 0, "B5e", "SELECT 不带回上次 DML 的陈旧计数");

        objval_vec pp;
        pp.push_back(http::obj_val("d1"));
        edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp);
        objval_vec pp2;
        pp2.push_back(http::obj_val("nope2"));
        CHECK(edit->exec_dml_prepared("UPDATE _pz_probe_t1 SET txt='w' WHERE k=?", pp2) == 0, "B5f",
              "预编译 UPDATE 未命中返回 0");
        unsigned int rg = edit->exec_dml_prepared("SELECT count(*) FROM _pz_probe_t1", objval_vec{});
        READ("B5g", "prepared_select_rc=" + std::to_string((int)rg) + " err=" + esc(edit->error_msg));
        READ("B5g", "text_path_select_rc=" + std::to_string((int)edit->exec_dml("SELECT 2")));
        CHECK(rg == 0, "B5g", "预编译 SELECT 返回 0(与文本通路 B5c/B5e 同口径)");

        // INSERT ... RETURNING 会step 出 SQLITE_ROW：两条通路是否都把"已写入"报成失败？
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        unsigned int rt = edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('ret_text') RETURNING id");
        unsigned int rtn = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='ret_text'");
        READ("B5h", "text_returning_rc=" + std::to_string((int)rt) + " rows=" + std::to_string(rtn) +
             " err=" + esc(edit->error_msg));
        CHECK(rt != (unsigned int)-1 || rtn == 0, "B5h", "文本通路 RETURNING 要么报成功要么未写入");

        objval_vec ppr;
        ppr.push_back(http::obj_val(std::string("ret_prep")));
        unsigned int rp = edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (k) VALUES (?) RETURNING id", ppr);
        unsigned int rpn = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='ret_prep'");
        READ("B5i", "prep_returning_rc=" + std::to_string((int)rp) + " rows=" + std::to_string(rpn) +
             " err=" + esc(edit->error_msg));
        CHECK(rp != (unsigned int)-1 || rpn == 0, "B5i", "预编译 RETURNING 报失败时不得已写入(无静默半写)");
    }

    // ---- B6: last_insert_rowid 单线程语义 ----
    {
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        unsigned int r = edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('lid1')");
        long long id1  = edit->last_insert_rowid();
        CHECK(r == 1 && id1 > 0, "B6a", "INSERT 后 last_insert_rowid 返回本次 rowid");
        READ("B6a", "id=" + std::to_string(id1));
        auto s = first_row(*sel, "SELECT k FROM _pz_probe_t1 WHERE id=" + std::to_string(id1));
        CHECK_EQ(s.empty() ? "" : s[0].bytes, "lid1", "B6b", "rowid 指向自己插入的行");
        edit->exec_dml("UPDATE _pz_probe_t1 SET txt='z' WHERE k='lid1'");
        CHECK(edit->last_insert_rowid() == id1, "B6c", "UPDATE 不改变 last_insert_rowid");
        // select 句柄上无插入时的读数
        READ("B6d", "select_handle_rowid=" + std::to_string(sel->last_insert_rowid()));
    }

    // ---- B7: 大文本(8MB / 20MB), SQLite 无 MySQL 那种 16MB 拆包上限 ----
    {
        for (size_t mb : {(size_t)8, (size_t)20})
        {
            std::string big(mb * 1024 * 1024, 'A');
            big[0]              = 'S';
            big[big.size() - 1] = 'E';
            bind_vec bp;
            orm::sqlite_bind_param a;
            a.type = SQLITE_TEXT;
            a.data = std::to_string(mb);
            bp.push_back(a);
            orm::sqlite_bind_param b;
            b.type = SQLITE_TEXT;
            b.data = big;
            bp.push_back(b);
            bool ok = edit->exec_bound("INSERT INTO _pz_probe_t1 (k, txt) VALUES (?,?)", bp);
            CHECK(ok, "B7a", "绑定 " + std::to_string(mb) + "MB 文本成功");
            auto s    = first_row(*sel, "SELECT txt FROM _pz_probe_t1 WHERE k='" + std::to_string(mb) + "'");
            bool same = !s.empty() && s[0].len == big.size() && !s[0].bytes.empty() &&
                        s[0].bytes.front() == 'S' && s[0].bytes.back() == 'E';
            CHECK(same, "B7b", std::to_string(mb) + "MB 文本原样读回(长度/首尾字节)");
            READ("B7b", "mb=" + std::to_string(mb) + " got_len=" + std::to_string(s.empty() ? 0 : s[0].len) +
                            " err=" + esc(sel->error_msg));
            edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k='" + std::to_string(mb) + "'");
        }
        // 超过 INT_MAX 长度的绑定应被显式拒绝(无法用 int 表示)
        READ("B7c", "bind_len_guard=INT_MAX bytes, 本轮不构造(需 >2GB 内存)");
    }
}

// ============ 族 C：事务 ============
static void fam_C(const conn_ptr &edit, const conn_ptr &sel)
{
    if (!fam("C"))
        return;
    std::printf("\n---- C: transactions ----\n");

    edit->exec_dml("DELETE FROM _pz_probe_t1");
    edit->exec_dml("DELETE FROM _pz_probe_abort");
    edit->exec_dml("DELETE FROM _pz_probe_roll");

    // C1 begin + commit 与 WAL 隔离
    CHECK(edit->begin_transaction(), "C1a", "begin_transaction 成功");
    CHECK(edit->in_transaction(), "C1b", "begin 后 in_transaction()=true");
    CHECK(engine_in_txn(edit), "C1c", "引擎真值与标记一致(autocommit==0)");
    edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('tx1')");
    CHECK(count_rows(*edit, "SELECT k FROM _pz_probe_t1 WHERE k='tx1'") == 1, "C1d", "写句柄事务内可见自己的未提交写");
    auto selvis = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='tx1'");
    READ("C1e", "select_handle_sees_uncommitted=" + std::to_string(selvis));
    CHECK(selvis == 0, "C1e", "另一条句柄读不到未提交写(WAL 隔离成立)");
    CHECK(edit->commit_transaction(), "C1f", "commit 成功");
    CHECK(!edit->in_transaction(), "C1g", "commit 后标记归位");
    CHECK(!engine_in_txn(edit), "C1h", "commit 后引擎 autocommit==1");
    CHECK(count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='tx1'") == 1, "C1i", "commit 后另一句柄立即可见");

    // C2 二次 begin 被拒; rollback 后事务权归还
    CHECK(edit->begin_transaction(), "C2a", "再次 begin 成功");
    bool second = edit->begin_transaction();
    READ("C2b", "second_begin_err=" + esc(edit->error_msg));
    CHECK(!second, "C2b", "事务中重复 begin 被拒");
    CHECK(edit->rollback_transaction(), "C2c", "rollback 成功");
    CHECK(edit->begin_transaction(), "C2d", "rollback 之后仍可 begin(事务权已归还)");
    CHECK(edit->rollback_transaction(), "C2e", "清理回滚");

    // C3 另一持有者(db_conn)的事务被本连接上的直接 commit 破坏
    {
        auto ulink = std::make_unique<orm::db_conn>("lite");
        bool began = ulink->begin_commit();
        CHECK(began, "C3a", "A(orm::db_conn) begin_commit 成功");
        READ("C3a", "same_handle=" + std::to_string(ulink->sqlite_edit_conn.get() == (orm::sqlite_conn_base *)edit.get()));
        ulink->sqlite_edit_conn->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('holder')");
        CHECK(engine_in_txn(edit), "C3b", "A 事务在引擎层打开");
        bool bcommit = edit->commit_transaction();
        READ("C3c", "nonowner_commit_returned=" + std::to_string(bcommit) +
                        " in_transaction=" + std::to_string(edit->in_transaction()));
        long long visible = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='holder'");
        READ("C3d", "holder_row_visible_after_other_commit=" + std::to_string(visible));
        CHECK(bcommit == false || visible == 0, "C3e",
              "另一持有者(orm::db_conn)的事务不应被直接 commit 破坏(头文件承诺)");
        if (edit->in_transaction())
            edit->rollback_transaction();
        else if (visible == 1)
            edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k='holder'");
        ulink->rollback();
    }

    // C4 裸 BEGIN/COMMIT 经 exec_sql 走同一把事务权
    {
        edit->exec_sql("BEGIN");
        CHECK(edit->in_transaction(), "C4a", "裸 BEGIN 后标记与引擎对齐(=true)");
        bool b = edit->begin_transaction();
        READ("C4b", "begin_after_bare_err=" + esc(edit->error_msg));
        CHECK(!b, "C4b", "裸 BEGIN 开启的事务同样占住事务权, 后续 begin 被拒");
        edit->exec_sql("COMMIT");
        CHECK(!edit->in_transaction(), "C4c", "裸 COMMIT 后标记归位");
        CHECK(edit->begin_transaction(), "C4d", "裸 COMMIT 归还了事务权");
        edit->rollback_transaction();

        edit->exec_sql("-- x\nBEGIN");
        CHECK(edit->in_transaction(), "C4e", "注释前缀的裸 BEGIN 也被追踪");
        edit->exec_sql("/* y */ ROLLBACK");
        CHECK(!edit->in_transaction(), "C4f", "注释前缀的裸 ROLLBACK 也被追踪");

        // 多语句批: BEGIN + 写入 + 不 COMMIT 收尾 -> 引擎留悬挂事务, 标记必须跟上
        edit->exec_sql("BEGIN;INSERT INTO _pz_probe_t1 (k) VALUES ('pend')");
        READ("C4g", "after_open_batch in_transaction=" + std::to_string(edit->in_transaction()) +
                        " engine=" + std::to_string((int)engine_in_txn(edit)));
        CHECK(edit->in_transaction() && engine_in_txn(edit), "C4g", "批次里开的裸事务被标记追踪");
        edit->exec_sql("ROLLBACK");
        CHECK(!edit->in_transaction(), "C4h", "收尾后事务权归还");
    }

    // C5 事务吞掉其它调用方的写
    {
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('B_before')");
        CHECK(edit->begin_transaction(), "C5a", "A begin");
        edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('A_in_txn')");
        unsigned int bw = edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('B_during')");
        READ("C5b", "B_during_exec_dml=" + std::to_string(bw));
        CHECK(edit->rollback_transaction(), "C5c", "A rollback");
        long long a_rows = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='A_in_txn'");
        long long b_rows = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='B_during'");
        long long bb     = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='B_before'");
        std::ostringstream os;
        os << "A_in_txn=" << a_rows << " B_during=" << b_rows << " B_before=" << bb;
        READ("C5d", os.str());
        CHECK(a_rows == 0 && bb == 1, "C5d", "A 回滚只影响自己的行");
        CHECK(b_rows == 1, "C5e", "B 报告成功(影响 1 行)的写, 不能因别人的事务回滚而消失");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // C6 exec_batch
    {
        std::vector<std::string> good = {
            "INSERT INTO _pz_probe_t1 (k) VALUES ('batch1')",
            "INSERT INTO _pz_probe_t1 (k) VALUES ('batch2')"};
        CHECK(edit->exec_batch(good) == 0, "C6a", "exec_batch 全成功返回 0");
        CHECK(count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'batch%'") == 2, "C6b", "批量 2 行落库");

        std::vector<std::string> bad = {
            "INSERT INTO _pz_probe_t1 (k) VALUES ('batch3')",
            "INSERT INTO _pz_probe_abort VALUES (1)",
            "INSERT INTO _pz_probe_abort VALUES (1)",// 唯一冲突 -> 整批回滚
            "INSERT INTO _pz_probe_t1 (k) VALUES ('batch5')"};
        int rb = edit->exec_batch(bad);
        READ("C6c", "exec_batch_bad_rc=" + std::to_string(rb) + " err=" + esc(edit->error_msg));
        long long left = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'batch%'");
        CHECK(rb == -1 && left == 2, "C6c", "exec_batch 中途失败整批回滚(不留 batch3/batch5)");
        CHECK(!edit->in_transaction(), "C6d", "失败批次结束后不残留事务");
        edit->exec_dml("DELETE FROM _pz_probe_abort");

        CHECK(edit->begin_transaction(), "C6e", "begin for batch gate");
        std::vector<std::string> one = {"INSERT INTO _pz_probe_t1 (k) VALUES ('gate')"};
        int rg                       = edit->exec_batch(one);
        CHECK(rg == -1, "C6f", "已有事务时 exec_batch 拒绝(不再裸发 BEGIN)");
        CHECK(edit->rollback_transaction(), "C6g", "回滚外层事务");
        CHECK(count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='gate'") == 0, "C6h", "被拒批次零写入");
        edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k LIKE 'batch%'");
    }

    // C7 语句错误只回滚语句, 事务仍在
    {
        CHECK(edit->begin_transaction(), "C7a", "begin");
        edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('keep')");
        edit->exec_dml("INSERT INTO _pz_probe_abort VALUES (5)");
        unsigned int r = edit->exec_dml("INSERT INTO _pz_probe_abort VALUES (5)");
        READ("C7b", "stmt_err=" + std::to_string((int)r) + " err=" + esc(edit->error_msg));
        CHECK(r == (unsigned int)-1, "C7b", "冲突语句报错");
        CHECK(edit->in_transaction() && engine_in_txn(edit), "C7c", "语句级失败后事务仍打开(标记与引擎一致)");
        CHECK(edit->commit_transaction(), "C7d", "后续 commit 仍可成功");
        CHECK(count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='keep'") == 1, "C7e", "事务内早前写入保留");
        CHECK(count_rows(*sel, "SELECT a FROM _pz_probe_abort WHERE a=5") == 1, "C7f",
              "ABORT 冲突只回滚失败语句本身(第一条成功的那条仍在)");
        edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k='keep'");
        edit->exec_dml("DELETE FROM _pz_probe_abort");
    }

    // C8 失败语句在引擎层结束事务 -> 事务权不得泄漏(四条通路各自验)
    {
        edit->exec_dml("DELETE FROM _pz_probe_roll");
        edit->exec_dml("INSERT INTO _pz_probe_roll VALUES (7)");

        struct Arm
        {
            const char *name;
            const char *id_ok;
            const char *id_gate;
        } arms[] = {{"tx exec_dml", "C8a", "C8b"},
                    {"pp exec_dml_prepared", "C8c", "C8d"},
                    {"pf fetch_prepared", "C8e", "C8f"},
                    {"bd exec_bound", "C8g", "C8h"},
                    {"fd fetch_directly", "C8i", "C8j"}};

        for (auto &ar : arms)
        {
            bool began = edit->begin_transaction();
            if (!began)
            {
                std::printf("     %s: begin 前置失败(上一臂事务权未归还) err=%s\n", ar.name,
                            esc(edit->error_msg).c_str());
                CHECK(false, ar.id_ok, "begin 前置可用");
                continue;
            }
            std::string err_before_clear;
            objval_vec pp;
            pp.push_back(http::obj_val((long long)7));
            if (std::strncmp(ar.name, "tx ", 3) == 0)
                edit->exec_dml("INSERT INTO _pz_probe_roll VALUES (7)");
            else if (std::strncmp(ar.name, "pp ", 3) == 0)
                edit->exec_dml_prepared("INSERT INTO _pz_probe_roll VALUES (?)", pp);
            else if (std::strncmp(ar.name, "pf ", 3) == 0)
                edit->fetch_prepared("INSERT INTO _pz_probe_roll VALUES (?)", pp,
                                     [](int, char **, auto) { return true; });
            else if (std::strncmp(ar.name, "fd ", 3) == 0)
                edit->fetch_directly("INSERT INTO _pz_probe_roll VALUES (7)",
                                     [](int, char **, auto) { return true; });
            else
            {
                bind_vec bp;
                orm::sqlite_bind_param a;
                a.type = SQLITE_INTEGER;
                a.data = "7";
                bp.push_back(a);
                edit->exec_bound("INSERT INTO _pz_probe_roll VALUES (?)", bp);
            }
            bool engine_open = engine_in_txn(edit);
            bool flag_open   = edit->in_transaction();
            bool can_begin   = edit->begin_transaction();
            std::ostringstream os;
            os << "arm=" << ar.name << " engine_in_txn=" << (int)engine_open << " flag_in_txn=" << flag_open
               << " can_begin_again=" << can_begin << " err=" << esc(edit->error_msg);
            READ(ar.id_gate, os.str());
            CHECK(!engine_open || flag_open, ar.id_ok, std::string(ar.name) + ": 引擎仍在事务时标记必须同步");
            CHECK(can_begin, ar.id_gate, std::string(ar.name) + ": 引擎已结束事务后事务权必须归还(begin 可用)");
            if (can_begin)
                edit->rollback_transaction();
            else
            {
                // 事务权泄漏: 验证是否只能靠 commit/rollback 自愈
                bool selfheal = edit->rollback_transaction();
                bool after    = edit->begin_transaction();
                READ(std::string(ar.id_gate) + "-leak",
                     "selfheal_by_rollback=" + std::to_string(selfheal) + " begin_after=" + std::to_string(after));
                if (after)
                    edit->rollback_transaction();
            }
            edit->exec_dml("DELETE FROM _pz_probe_roll");
            edit->exec_dml("INSERT INTO _pz_probe_roll VALUES (7)");
        }
        edit->exec_dml("DELETE FROM _pz_probe_roll");
        edit->exec_dml("INSERT INTO _pz_probe_roll VALUES (7)");
    }

    // C9 语句错误后事务不丢: 事务里失败一条, 后续语句仍可提交
    {
        CHECK(edit->begin_transaction(), "C9a", "begin");
        edit->exec_dml("SELEKT bad in txn");
        CHECK(edit->in_transaction(), "C9b", "语法错误后仍在事务中");
        edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('after_err')");
        CHECK(edit->commit_transaction(), "C9c", "错误之后的写入仍可提交");
        CHECK(count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='after_err'") == 1, "C9d", "提交结果可见");
        edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k='after_err'");
    }
}

// ============ 族 D：错误 / 恢复 / 重连 ============
static void fam_D(const conn_ptr &edit, const conn_ptr &sel)
{
    if (!fam("D"))
        return;
    std::printf("\n---- D: errors / recovery / reconnect ----\n");

    // D1 语法错误后连接可用
    {
        unsigned int r = edit->exec_dml("SELEKT bad syntax");
        CHECK(r == (unsigned int)-1, "D1a", "语法错误返回错误哨兵");
        READ("D1a", "err=" + esc(edit->error_msg));
        CHECK(!edit->is_closed(), "D1b", "语法错误不关闭连接");
        CHECK(edit->ping(), "D1c", "错误后 ping 正常");
        CHECK(edit->exec_dml("SELECT 1") != (unsigned int)-1, "D1d", "错误后仍可执行");
    }

    // D2 未连接对象
    {
        orm::sqlite_conn_base fresh;
        unsigned int r = fresh.exec_dml("SELECT 1");
        CHECK(r == (unsigned int)-1 && fresh.error_msg == "Database not connected", "D2a",
              "未连接时 exec_dml 明确失败");
        CHECK(fresh.begin_transaction() == false, "D2b", "未连接 begin 失败");
        orm::sqlite_query_result qr;
        CHECK(fresh.query_fetch("SELECT 1", qr) == false, "D2c", "未连接 query_fetch 失败");
        CHECK(fresh.exec_dml_prepared("SELECT ?", objval_vec{}) == (unsigned int)-1, "D2d",
              "未连接预编译写失败");
        CHECK(fresh.fetch_directly("SELECT 1", [](int, char **, auto) { return true; }) == 0, "D2e",
              "未连接读返回 0 行");
    }

    // D3 跨写者锁: 另一条句柄占住写锁, busy_timeout 生效
    {
        orm::sqlite_conn_base other;
        bool c = other.connect(lite_conf(kLiteDb));
        CHECK(c, "D3a", "直连第二条句柄成功");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        other.exec_sql("BEGIN IMMEDIATE");
        other.exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('locker')");
        auto t0        = clk::now();
        unsigned int r = edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('blocked')");
        auto ms        = std::chrono::duration_cast<std::chrono::milliseconds>(clk::now() - t0).count();
        std::ostringstream os;
        os << "r=" << (int)r << " waited_ms=" << ms << " err=" << esc(edit->error_msg);
        READ("D3b", os.str());
        CHECK(r == (unsigned int)-1 && edit->error_msg.find("locked") != std::string::npos, "D3b",
              "写锁冲突在 busy_timeout 后报 database is locked");
        CHECK(ms >= 4000 && ms < 12000, "D3c", "等待约 busy_timeout(5s), 不是立即返回也没跑整库体检");
        other.exec_sql("COMMIT");
        CHECK(edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('after_release')") == 1, "D3d",
              "锁释放后写入成功");
        CHECK(count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='locker'") == 1, "D3e",
              "对方提交的行立即可见(读快照未钉住)");
        edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k IN ('locker','after_release')");
        other.close();
    }

    // D4 数据页损坏: 文本 / 预编译两条通路处理是否一致
    {
        std::string src  = std::string(kTmpDir) + "/d4_src.db";
        std::string badf = std::string(kTmpDir) + "/d4_bad.db";
        std::remove(src.c_str());
        std::remove(badf.c_str());
        std::remove((src + "-wal").c_str());
        std::remove((src + "-shm").c_str());
        {
            orm::sqlite_conn_base mk;
            if (mk.connect(lite_conf(src)))
            {
                // 回滚日志模式, 让全部数据落在主文件里(便于按页破坏)
                mk.exec_dml("PRAGMA journal_mode=DELETE");
                mk.exec_dml("CREATE TABLE t (a INTEGER PRIMARY KEY, b TEXT)");
                for (int i = 0; i < 800; i++)
                    mk.exec_dml("INSERT INTO t (b) VALUES ('row" + std::to_string(i) + "')");
                std::string chk = scalar_text(mk, "SELECT count(*) FROM t");
                READ("D4setup", "rows_before_corrupt=" + chk);
                mk.close();
            }
            else
            {
                READ("D4setup", "create_fixture_failed=" + esc(mk.error_msg));
            }
        }
        {
            std::ifstream in(src, std::ios::binary);
            std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            std::ofstream out(badf, std::ios::binary);
            if (data.size() > 8192)
            {
                for (size_t page = 2; page * 4096 < data.size() && page <= 6; page++)
                {
                    size_t off = page * 4096 + 8;
                    for (size_t i = off; i < off + 300 && i < data.size(); i++)
                        data[i] = (char)0xAB;
                    data[page * 4096] = (char)0xFF;// 非法 b-tree 页类型
                }
            }
            out.write(data.data(), (std::streamsize)data.size());
            READ("D4setup", "file_bytes=" + std::to_string(data.size()));
        }

        orm::sqlite_conn_base bad;
        bool c = bad.connect(lite_conf(badf));
        READ("D4a", "connect_ok_on_data_page_corruption=" + std::to_string(c) + " err=" + esc(bad.error_msg));
        if (c)
        {
            auto t0 = clk::now();
            unsigned int last = 0;
            for (int i = 0; i < 6; i++)
                last = bad.exec_dml("SELECT * FROM t");
            auto ms      = std::chrono::duration_cast<std::chrono::milliseconds>(clk::now() - t0).count();
            std::string text_err = bad.error_msg;
            READ("D4b", "text_6errs last_rc=" + std::to_string((int)last) + " ms=" + std::to_string(ms) +
                            " err=" + esc(text_err));

            auto t1 = clk::now();
            for (int i = 0; i < 6; i++)
                bad.exec_dml_prepared("SELECT * FROM t", objval_vec{});
            auto ms2      = std::chrono::duration_cast<std::chrono::milliseconds>(clk::now() - t1).count();
            std::string prep_err = bad.error_msg;
            READ("D4c", "prepared_6errs ms=" + std::to_string(ms2) + " err=" + esc(prep_err));

            // 恢复流程会跑 PRAGMA integrity_check / VACUUM, 结束文本与耗时随之不同
            CHECK(text_err == prep_err && std::abs((long long)ms - (long long)ms2) < 100, "D4d",
                  "文本与预编译通路对损坏库的处理一致(同一恢复动作)");
            std::printf("     text_err=[%s] prepared_err=[%s] ms=%lld/%lld\n", esc(text_err).c_str(),
                        esc(prep_err).c_str(), (long long)ms, (long long)ms2);
            bad.close();
        }
        else
        {
            std::printf("     数据页损坏在 connect 阶段即被拒, 无法比较两条通路\n");
        }
        std::remove(src.c_str());
        std::remove(badf.c_str());
    }

    // D5 头部魔数损坏 -> 拒绝当作可用连接
    {
        std::string f = std::string(kTmpDir) + "/d5_notdb.db";
        std::remove(f.c_str());
        {
            std::ofstream out(f, std::ios::binary);
            // 去掉结尾 NUL 的魔数 -> 非法数据库文件
            out << "SQLite format 2" << std::string(8000, 'x');
        }
        orm::sqlite_conn_base c;
        bool ok = c.connect(lite_conf(f));
        READ("D5a", "connect_on_non_database=" + std::to_string(ok) + " err=" + esc(c.error_msg));
        CHECK(!ok, "D5a", "非库文件被存活探测拒绝(不把半死连接当成功)");
        if (ok)
            c.close();
        std::remove(f.c_str());
    }

    // D6 路径为空 / 目录不存在
    {
        orm::sqlite_conn_base c1;
        CHECK(!c1.connect(lite_conf("")) && c1.error_msg.find("path is empty") != std::string::npos, "D6a",
              "空路径明确失败");
        if (c1.error_msg.find("path is empty") == std::string::npos)
            READ("D6a", "err=" + esc(c1.error_msg));
        orm::sqlite_conn_base c2;
        bool ok = c2.connect(lite_conf(std::string(kTmpDir) + "/no_such_dir/x.db"));
        READ("D6b", "missing_dir_connect=" + std::to_string(ok) + " err=" + esc(c2.error_msg));
        if (ok)
            c2.close();
    }

    // D7 rollback_unfinished_transactions
    {
        edit->exec_sql("BEGIN");
        edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('hang')");
        CHECK(engine_in_txn(edit), "D7a", "引擎确在事务中");
        bool rb = edit->rollback_unfinished_transactions();
        CHECK(rb && !edit->in_transaction() && !engine_in_txn(edit), "D7b", "rollback_unfinished_transactions 收掉悬挂事务");
        CHECK(count_rows(*edit, "SELECT k FROM _pz_probe_t1 WHERE k='hang'") == 0, "D7c", "悬挂事务的写被丢弃");
        CHECK(edit->begin_transaction(), "D7d", "收干净后 begin 可用(无事务权泄漏)");
        edit->rollback_transaction();
    }

    // D8 reconnect 语义
    {
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        objval_vec pp;
        pp.push_back(http::obj_val("cache_before"));
        edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp);// 已提交 + 进语句缓存
        CHECK(edit->begin_transaction(), "D8a", "reconnect 前打开一个事务");
        edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('cache_open')");
        bool r = edit->reconnect(3);
        READ("D8b", "reconnect=" + std::to_string(r) + " err=" + esc(edit->error_msg));
        CHECK(r, "D8b", "同文件重连成功");
        CHECK(!edit->in_transaction() && !engine_in_txn(edit), "D8c", "重连后无事务");
        CHECK_EQ(scalar_text(*edit, "PRAGMA busy_timeout"), "5000", "D8d", "重连后 PRAGMA 重新生效");
        CHECK_EQ(scalar_text(*edit, "PRAGMA foreign_keys"), "1", "D8e", "重连后 foreign_keys 重新生效");
        objval_vec pp2;
        pp2.push_back(http::obj_val("cache_after"));
        CHECK(edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp2) == 1, "D8f",
              "缓存清空后同一 SQL 重新 prepare 正常");
        CHECK(count_rows(*edit, "SELECT k FROM _pz_probe_t1 WHERE k='cache_before'") == 1, "D8g",
              "重连不影响已提交数据");
        CHECK(count_rows(*edit, "SELECT k FROM _pz_probe_t1 WHERE k='cache_open'") == 0, "D8h",
              "重连明确丢弃未提交事务(不留半成品)");
        edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k IN ('cache_before','cache_after')");
    }

    // D9 integrity_check / repair_database 在健康库上
    {
        std::string report;
        bool ok = edit->integrity_check(report);
        CHECK(ok, "D9a", "健康库 integrity_check=true");
        READ("D9a", "report=" + esc(report));
        CHECK(edit->repair_database(), "D9b", "健康库 repair_database 返回 true");
        // repair 是否短路: 体检 ok 时不应做 VACUUM(读耗时/读文件大小变化)
        long long before = 0;
        {
            std::ifstream in(kLiteDb, std::ios::binary | std::ios::ate);
            before = (long long)in.tellg();
        }
        bool r2 = edit->repair_database();
        std::ifstream in2(kLiteDb, std::ios::binary | std::ios::ate);
        long long after = (long long)in2.tellg();
        READ("D9c", "repair_ok=" + std::to_string(r2) + " bytes=" + std::to_string(before) + "->" +
                        std::to_string(after));
    }

    // D10 文本 exec_sql 多语句批全部执行
    {
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        int r = edit->exec_sql(
            "INSERT INTO _pz_probe_t1 (k) VALUES ('m1');INSERT INTO _pz_probe_t1 (k) VALUES ('m2');INSERT INTO _pz_probe_t1 (k) VALUES ('m3')");
        long long n = count_rows(*edit, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'm%'");
        READ("D10a", "exec_sql_multi rc=" + std::to_string(r) + " rows_in_db=" + std::to_string(n));
        CHECK(n == 3, "D10a", "文本 exec_sql 多语句批全部执行");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // D11 连续瞬态错误不得触发整库体检(BUSY 类不累计)
    {
        orm::sqlite_conn_base other;
        if (other.connect(lite_conf(kLiteDb)))
        {
            other.exec_sql("BEGIN IMMEDIATE");
            auto t0 = clk::now();
            for (int i = 0; i < 6; i++)
                edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('busy')");
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(clk::now() - t0).count();
            other.exec_sql("ROLLBACK");
            std::string err = edit->error_msg;
            READ("D11a", "six_busy_round_ms=" + std::to_string(ms) + " err=" + esc(err));
            // 6 次 BUSY 各等 5s; 若混进整库体检/VACUUM 会显著超过 30s
            CHECK(ms < 34000 && err.find("locked") != std::string::npos, "D11a",
                  "BUSY 只等 busy_timeout, 不进恢复流程(不体检不 VACUUM)");
            other.close();
        }
        edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k='busy'");
    }
}

// ============ 族 E：语句缓存 / schema / 读快照 ============
static void fam_E(const conn_ptr &edit, const conn_ptr &sel)
{
    if (!fam("E"))
        return;
    std::printf("\n---- E: stmt cache / schema / snapshot ----\n");

    // E1 超过缓存上限 128 条不同 SQL
    {
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        int bad = 0;
        for (int i = 0; i < 200; i++)
        {
            objval_vec pp;
            pp.push_back(http::obj_val((long long)i));
            pp.push_back(http::obj_val("lru" + std::to_string(i)));
            unsigned int r = edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (ival, k) VALUES (?,?)", pp);
            if (r != 1)
                bad++;
        }
        long long n = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'lru%'");
        READ("E1a", "bad=" + std::to_string(bad) + " rows=" + std::to_string(n));
        CHECK(bad == 0 && n == 200, "E1a", "200 条不同预编译语句(超 128 上限)全部正确执行");
        auto s = first_row(*sel, "SELECT k FROM _pz_probe_t1 WHERE ival=99");
        CHECK_EQ(s.empty() ? "<none>" : s[0].bytes, "lru99", "E1b", "LRU 驱逐后各条语句内容仍各自正确");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // E2 外部 DDL 之后，同一 SQL 文本命中缓存语句
    {
        edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_s");
        edit->exec_dml("CREATE TABLE _pz_probe_s (a INTEGER, b TEXT)");
        edit->exec_dml("INSERT INTO _pz_probe_s VALUES (1,'one'),(2,'two')");

        const std::string same = "SELECT a, b FROM _pz_probe_s WHERE a>=? ORDER BY a";
        objval_vec pp;
        pp.push_back(http::obj_val((long long)1));

        unsigned int rows1 = 0, calls1 = 0;
        int cols1          = -1;
        edit->fetch_prepared(same, pp,
                             [&](int col_count, char **, auto) { rows1++; calls1++; cols1 = col_count; return true; });
        CHECK(rows1 == 2 && calls1 == 2 && cols1 == 2, "E2a", "首建语句: 2 行 2 列, handler 恰好 2 次");

        edit->exec_dml("ALTER TABLE _pz_probe_s ADD COLUMN c TEXT");
        edit->exec_dml("UPDATE _pz_probe_s SET c='x'");

        unsigned int rows2 = 0, calls2 = 0, rows3 = 0, calls3 = 0;
        int cols2 = -1, cols3 = -1;
        edit->fetch_prepared(same, pp,
                             [&](int col_count, char **, auto) { rows2++; calls2++; cols2 = col_count; return true; });
        std::string err2 = edit->error_msg;
        edit->fetch_prepared(same, pp,
                             [&](int col_count, char **, auto) { rows3++; calls3++; cols3 = col_count; return true; });
        std::ostringstream os;
        os << "first{rows=" << rows2 << ",calls=" << calls2 << ",cols=" << cols2 << ",err=" << esc(err2) << "} "
           << "second{rows=" << rows3 << ",calls=" << calls3 << ",cols=" << cols3 << "}";
        READ("E2b", os.str());
        CHECK((rows2 == 2 && cols2 == 2) || (rows3 == 2 && cols3 == 2), "E2b",
              "同一 SQL 在 ADD COLUMN 后两次调用内必须自愈到 2 行 2 列");
        CHECK(calls2 <= 2 && calls3 <= 2, "E2c", "DDL 重试不得把同一行交付两遍(handler 副作用不重复)");

        // 列数变化方向：SELECT * 先按 1 列建缓存，再重建为 3 列，然后再回到 2 列
        const std::string star = "SELECT * FROM _pz_probe_s ORDER BY a";
        orm::sqlite_query_result prime;
        edit->query_fetch(star, prime);
        READ("E2d", "primed_cols=" + std::to_string(prime.column_names.size()));
        edit->exec_dml("DROP TABLE _pz_probe_s");
        edit->exec_dml("CREATE TABLE _pz_probe_s (a INTEGER, b TEXT, c TEXT)");
        edit->exec_dml("INSERT INTO _pz_probe_s VALUES (1,'x','y'),(2,'x','y')");
        orm::sqlite_query_result more;
        bool ok_more = edit->query_fetch(star, more);
        std::ostringstream os2;
        os2 << "ok=" << (ok_more ? "T" : "F") << " rows=" << more.rows.size()
            << " names=" << more.column_names.size()
            << " row0cols=" << (more.rows.empty() ? 0u : more.rows[0].size()) << " err=" << esc(edit->error_msg);
        READ("E2e", os2.str());
        CHECK(ok_more && more.rows.size() == 2 && more.column_names.size() == 3 && more.rows[0].size() == 3, "E2e",
              "列变多后 query_fetch 按新 schema 给列名/列数(不静默截断)");

        edit->exec_dml("DROP TABLE _pz_probe_s");
        edit->exec_dml("CREATE TABLE _pz_probe_s (a INTEGER)");
        edit->exec_dml("INSERT INTO _pz_probe_s VALUES (1),(2)");
        orm::sqlite_query_result less;
        bool ok_less = edit->query_fetch(star, less);
        std::ostringstream os3;
        os3 << "ok=" << (ok_less ? "T" : "F") << " rows=" << less.rows.size()
            << " names=" << less.column_names.size()
            << " row0cols=" << (less.rows.empty() ? 0u : less.rows[0].size())
            << " nulls=" << (less.is_null.empty() ? 0u : less.is_null[0].size()) << " err=" << esc(edit->error_msg);        READ("E2f", os3.str());
        CHECK(ok_less && less.rows.size() == 2 && less.column_names.size() == 1 && less.rows[0].size() == 1 &&
                  less.is_null.size() >= 1 && less.is_null[0].size() >= 1 && less.is_null[0][0] == false,
              "E2f", "列变少后 query_fetch 不越界读、不给首行补假的 NULL 列");

        edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_s");
    }

    // E3 读快照释放：读完 / handler 中止 / 失败查询 之后语句必须已 reset
    {
        edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_snap");
        edit->exec_dml("CREATE TABLE _pz_probe_snap (a INTEGER)");
        edit->exec_dml("INSERT INTO _pz_probe_snap VALUES (1),(2),(3)");

        struct SnapCase
        {
            const char *name;
            const char *id;
        } cases[] = {{"fetch_directly 中止", "E3a"},
                    {"fetch_prepared 中止", "E3b"},
                    {"fetch_prepared_binary 中止", "E3c"},
                    {"query_fetch 失败", "E3d"}};

        for (auto &cs : cases)
        {
            edit->exec_dml("DELETE FROM _pz_probe_snap");
            edit->exec_dml("INSERT INTO _pz_probe_snap VALUES (1),(2),(3)");
            if (cs.id == std::string("E3a"))
                edit->fetch_directly("SELECT a FROM _pz_probe_snap ORDER BY a",
                                     [](int, char **, auto) { return false; });
            else if (cs.id == std::string("E3b"))
                edit->fetch_prepared("SELECT a FROM _pz_probe_snap ORDER BY a", objval_vec{},
                                     [](int, char **, auto) { return false; });
            else if (cs.id == std::string("E3c"))
                edit->fetch_prepared_binary("SELECT a FROM _pz_probe_snap ORDER BY a", objval_vec{},
                                            [](int, char **, auto) { return false; });
            else
            {
                orm::sqlite_query_result bad;
                edit->query_fetch("SELECT a FROM _pz_probe_no_such", bad);
                edit->query_fetch("SELECT a FROM _pz_probe_snap ORDER BY a", bad);
            }
            // 另一条句柄提交一行，本句柄下一次读必须看到
            {
                orm::sqlite_conn_base other;
                if (other.connect(lite_conf(kLiteDb)))
                {
                    other.exec_dml("INSERT INTO _pz_probe_snap VALUES (99)");
                    other.exec_dml("DELETE FROM _pz_probe_snap WHERE a=99");
                    other.exec_dml("INSERT INTO _pz_probe_snap VALUES (98)");
                    other.close();
                }
                else
                {
                    READ(cs.id, "second_handle_connect_failed=" + esc(other.error_msg));
                }
            }
            unsigned int visible = count_rows(*edit, "SELECT a FROM _pz_probe_snap WHERE a=98");
            std::ostringstream os4;
            os4 << "arm=" << cs.name << " sees_other_commit=" << visible;
            READ(cs.id, os4.str());
            CHECK(visible == 1, cs.id, std::string(cs.name) + " 之后必须已 reset(不钉住 WAL 读快照)");
        }
        edit->exec_dml("DELETE FROM _pz_probe_snap");
        // 失败查询后连接仍可用
        CHECK(count_rows(*edit, "SELECT a FROM _pz_probe_snap") == 0, "E3e", "失败/中止读之后查询仍可用");
        edit->exec_dml("DROP TABLE _pz_probe_snap");
    }

    // E4 一条字符串里塞多条语句：文本通路全跑，预编译通路只跑第一条
    {
        edit->exec_dml("DELETE FROM _pz_probe_t1");
        std::string two = "INSERT INTO _pz_probe_t1 (k) VALUES ('m1');"
                          "INSERT INTO _pz_probe_t1 (k) VALUES ('m2')";
        unsigned int rt = edit->exec_dml(two);
        unsigned int tr = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'm%'");
        edit->exec_dml("DELETE FROM _pz_probe_t1");

        unsigned int rp = edit->exec_dml_prepared(two, objval_vec{});
        unsigned int pr = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'm%'");
        std::string errp = edit->error_msg;

        std::ostringstream os;
        os << "text{rc=" << (int)rt << ",rows=" << tr << "} prepared{rc=" << (int)rp << ",rows=" << pr
           << ",err=" << esc(errp) << "}";
        READ("E4a", os.str());
        CHECK(tr == 2, "E4a", "文本通路 exec_dml 逐条执行分号分隔的多语句(两条都写入)");
        // sqlite3_changes 只反映最后一条语句，多语句批的返回值天然偏小：只记读数不判缺陷
        READ("E4a", "text_rc_counts_last_statement_only=" + std::to_string((int)rt));
        CHECK(rp == (unsigned int)-1 || pr == tr, "E4b",
              "预编译通路要么拒绝多语句，要么与文本通路一样全部执行(不得静默只跑第一条)");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // E5 多语句 + 占位符：尾部语句的参数会不会被算进绑定个数
    {
        objval_vec pp;
        pp.push_back(http::obj_val("tp1"));
        pp.push_back(http::obj_val("tp2"));
        unsigned int r = edit->exec_dml_prepared(
            "INSERT INTO _pz_probe_t1 (k) VALUES (?);INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp);
        unsigned int rows = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'tp%'");
        std::ostringstream os;
        os << "rc=" << (int)r << " rows=" << rows << " err=" << esc(edit->error_msg);
        READ("E5a", os.str());
        CHECK(r == (unsigned int)-1 || rows == 2, "E5a", "带占位符的多语句同样不得静默半执行");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }
}

// ============ 族 R：多线程共用一条常驻句柄 ============
static void fam_R(const conn_ptr &edit, const conn_ptr &sel)
{
    if (!fam("R"))
        return;
    std::printf("\n---- R: threads on one shared handle ----\n");

    edit->exec_dml("DELETE FROM _pz_probe_t1");

    // R1 并发绑定写 + R2 记录每次 INSERT 之后本线程读到的 rowid
    const int T1 = 8, N1 = 200;
    struct Rec
    {
        long long rowid;
        std::string want;
    };
    std::vector<std::vector<Rec>> recs(T1);
    std::atomic<unsigned int> bad_insert{0};
    {
        std::vector<std::thread> ths;
        for (int t = 0; t < T1; t++)
        {
            ths.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < N1; i++)
                    {
                        std::string k = "rt" + std::to_string(t) + "-" + std::to_string(i);
                        objval_vec pp;
                        pp.push_back(http::obj_val(k));
                        if (edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp) != 1)
                        {
                            bad_insert++;
                            continue;
                        }
                        recs[t].push_back({edit->last_insert_rowid(), k});
                    }
                });
        }
        for (auto &th : ths)
            th.join();
    }

    unsigned int recorded = 0;
    for (auto &v : recs)
        recorded += (unsigned int)v.size();
    unsigned int present = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'rt%'");
    std::ostringstream os1;
    os1 << "bad_insert=" << bad_insert.load() << " recorded=" << recorded << " rows_in_db=" << present
        << " expect=" << (unsigned int)(T1 * N1);
    READ("R1a", os1.str());
    CHECK(bad_insert.load() == 0 && recorded == (unsigned int)(T1 * N1) && present == (unsigned int)(T1 * N1), "R1a",
          "8 线程 × 200 绑定 INSERT 全部成功且行数完整(单 worker 串行成立)");

    // R2 rowid 归属：静默期逐条回查，唯一变量是 INSERT 与 last_insert_rowid 之间的交错
    {
        unsigned int wrong_owner = 0, invalid = 0;
        std::vector<long long> ids;
        ids.push_back(-1);
        ids.pop_back();
        for (int t = 0; t < T1; t++)
        {
            for (auto &r : recs[t])
            {
                if (r.rowid <= 0)
                {
                    invalid++;
                    continue;
                }
                ids.push_back(r.rowid);
                std::string got;
                orm::sqlite_scalar_result sr;
                if (sel->query_scalar("SELECT k FROM _pz_probe_t1 WHERE id=" + std::to_string(r.rowid), sr) &&
                    !sr.is_null)
                    got = sr.value;
                if (got != r.want)
                    wrong_owner++;
            }
        }
        std::sort(ids.begin(), ids.end());
        unsigned int dup = 0;
        for (size_t i = 1; i < ids.size(); i++)
            if (ids[i] == ids[i - 1])
                dup++;
        std::ostringstream os2;
        os2 << "wrong_owner=" << wrong_owner << " duplicate_rowid=" << dup << " invalid=" << invalid
            << " of=" << recorded;
        READ("R2a", os2.str());
        CHECK(wrong_owner == 0 && dup == 0 && invalid == 0, "R2a",
              "INSERT 之后 last_insert_rowid 必须指向本线程刚插入的行");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // R3 绑定值不串：每线程写自己的 (ival, txt) 配对并逐行回读
    {
        const int T3 = 8, N3 = 500;
        std::atomic<unsigned int> failw{0}, mismatch{0};
        std::vector<std::thread> th3;
        for (int t = 0; t < T3; t++)
        {
            th3.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < N3; i++)
                    {
                        long long num = (long long)t * 100000 + i;
                        std::string txt = "p" + std::to_string(t) + "_" + std::to_string(i);
                        objval_vec pp;
                        pp.push_back(http::obj_val(num));
                        pp.push_back(http::obj_val(txt));
                        if (edit->exec_dml_prepared("INSERT INTO _pz_probe_t1 (ival, txt) VALUES (?,?)", pp) != 1)
                        {
                            failw++;
                            continue;
                        }
                        std::string back;
                        orm::sqlite_scalar_result sr;
                        if (sel->query_scalar("SELECT txt FROM _pz_probe_t1 WHERE ival=" + std::to_string(num), sr) &&
                            !sr.is_null)
                            back = sr.value;
                        if (back != txt)
                            mismatch++;
                    }
                });
        }
        for (auto &th : th3)
            th.join();
        unsigned int rows = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE txt LIKE 'p%'");
        std::ostringstream o3;
        o3 << "fail=" << failw.load() << " pair_mismatch=" << mismatch.load() << " rows=" << rows
           << " expect=" << (unsigned int)(T3 * N3);
        READ("R3a", o3.str());
        CHECK(failw.load() == 0 && mismatch.load() == 0 && rows == (unsigned int)(T3 * N3), "R3a",
              "8 线程 × 500 绑定写 + 逐行回读: 值对不错位(缓冲与绑定不串)");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // R4 并发 begin：只允许一个持有者，竞争后事务权必须归还
    {
        const int T4 = 6, N4 = 50;
        std::atomic<unsigned int> ok_begin{0}, refused{0}, commit_fail{0};
        std::vector<std::thread> th4;
        for (int t = 0; t < T4; t++)
        {
            th4.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < N4; i++)
                    {
                        if (!edit->begin_transaction())
                        {
                            refused++;
                            continue;
                        }
                        ok_begin++;
                        std::string k = "txn" + std::to_string(t) + "_" + std::to_string(i);
                        edit->exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('" + k + "')");
                        if (!edit->commit_transaction())
                        {
                            commit_fail++;
                            edit->rollback_transaction();
                        }
                    }
                });
        }
        for (auto &th : th4)
            th.join();
        unsigned int rows = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'txn%'");
        bool idle  = !edit->in_transaction();
        bool again = edit->begin_transaction();
        if (again)
            edit->rollback_transaction();
        std::ostringstream o4;
        o4 << "ok_begin=" << ok_begin.load() << " refused=" << refused.load()
           << " commit_fail=" << commit_fail.load() << " rows=" << rows << " idle_after=" << (int)idle
           << " begin_after=" << (int)again << " err=" << esc(edit->error_msg);
        READ("R4a", o4.str());
        CHECK(idle && again && rows + commit_fail.load() == ok_begin.load() && ok_begin.load() > 0, "R4a",
              "并发 begin 竞争后：行数与提交数一致、不留悬挂事务、事务权已归还");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // R5a 文本元组接口：8×200 并发下 INSERT 与自增主键在同一次提交里取完
    {
        const int T5 = 8, N5 = 200;
        std::vector<std::vector<std::pair<long long, std::string>>> own5(T5);
        std::atomic<unsigned int> aff_bad{0};
        std::vector<std::thread> th5;
        for (int t = 0; t < T5; t++)
        {
            th5.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < N5; i++)
                    {
                        std::string k = "tx" + std::to_string(t) + "-" + std::to_string(i);
                        auto one = edit->exec_dml_with_rowid("INSERT INTO _pz_probe_t1 (k) VALUES ('" + k + "')");
                        if (std::get<0>(one) != 1)
                        {
                            aff_bad++;
                            continue;
                        }
                        own5[t].push_back({std::get<1>(one), k});
                    }
                });
        }
        for (auto &th : th5)
            th.join();
        std::vector<std::pair<long long, std::string>> all5;
        for (auto &v : own5)
            all5.insert(all5.end(), v.begin(), v.end());
        unsigned int wo = 0, dp = 0, iv = 0;
        verify_rowids(all5, sel, wo, dp, iv);
        std::ostringstream o5;
        o5 << "wrong_owner=" << wo << " duplicate_rowid=" << dp << " invalid=" << iv << " aff_bad=" << aff_bad.load()
           << " of=" << (unsigned int)all5.size() << " expect=" << (unsigned int)(T5 * N5);
        READ("R5a", o5.str());
        CHECK(wo == 0 && dp == 0 && iv == 0 && aff_bad.load() == 0 && all5.size() == (unsigned int)(T5 * N5), "R5a",
              "exec_dml_with_rowid: 并发下自增主键必须指向本线程刚插入的行");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // R5b 预编译元组接口：同样的形状走绑定参数的写通路
    {
        const int T5 = 8, N5 = 200;
        std::vector<std::vector<std::pair<long long, std::string>>> own5(T5);
        std::atomic<unsigned int> aff_bad{0};
        std::vector<std::thread> th5;
        for (int t = 0; t < T5; t++)
        {
            th5.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < N5; i++)
                    {
                        std::string k = "pr" + std::to_string(t) + "-" + std::to_string(i);
                        objval_vec pp;
                        pp.push_back(http::obj_val(k));
                        auto one = edit->exec_dml_prepared_with_rowid("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp);
                        if (std::get<0>(one) != 1)
                        {
                            aff_bad++;
                            continue;
                        }
                        own5[t].push_back({std::get<1>(one), k});
                    }
                });
        }
        for (auto &th : th5)
            th.join();
        std::vector<std::pair<long long, std::string>> all5;
        for (auto &v : own5)
            all5.insert(all5.end(), v.begin(), v.end());
        unsigned int wo = 0, dp = 0, iv = 0;
        verify_rowids(all5, sel, wo, dp, iv);
        std::ostringstream o5;
        o5 << "wrong_owner=" << wo << " duplicate_rowid=" << dp << " invalid=" << iv << " aff_bad=" << aff_bad.load()
           << " of=" << (unsigned int)all5.size() << " expect=" << (unsigned int)(T5 * N5);
        READ("R5b", o5.str());
        CHECK(wo == 0 && dp == 0 && iv == 0 && aff_bad.load() == 0 && all5.size() == (unsigned int)(T5 * N5), "R5b",
              "exec_dml_prepared_with_rowid: 并发下自增主键必须指向本线程刚插入的行");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    // R6a 模型侧文本通路：写走重新生成后的 _opsql.h 调用点(生成码里 8 处同步元组接口)，
    // 8×50 并发下 insert() 返回的主键必须指向本线程刚写的行。
    // 这一臂是 R5a 的上层等价物：R5a 直调驱动，本臂证明产品里那 12 处调用点真的接到了元组接口上。
    {
        const int T6 = 8, N6 = 50;
        std::vector<std::vector<std::pair<long long, std::string>>> own6(T6);
        std::atomic<unsigned int> aff_bad{0};
        std::mutex err_mu;
        std::string first_err;
        std::vector<std::thread> th6;
        for (int t = 0; t < T6; t++)
        {
            th6.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < N6; i++)
                    {
                        std::string k = "mz" + std::to_string(t) + "-" + std::to_string(i);
                        probe_model::Fortune m;
                        m.data.message = k;
                        auto one = m.insert();
                        if (std::get<0>(one) != 1 || std::get<1>(one) == 0)
                        {
                            aff_bad++;
                            std::lock_guard<std::mutex> g(err_mu);
                            if (first_err.empty())
                                first_err = m.error_msg;
                            continue;
                        }
                        own6[t].push_back({(long long)std::get<1>(one), k});
                    }
                });
        }
        for (auto &th : th6)
            th.join();
        std::vector<std::pair<long long, std::string>> all6;
        for (auto &v : own6)
            all6.insert(all6.end(), v.begin(), v.end());
        unsigned int wo = 0, dp = 0, iv = 0;
        verify_rowids(all6, sel, wo, dp, iv, "fortune", "message");
        std::ostringstream o6;
        o6 << "wrong_owner=" << wo << " duplicate_rowid=" << dp << " invalid=" << iv
           << " aff_bad=" << aff_bad.load() << " of=" << (unsigned int)all6.size()
           << " expect=" << (unsigned int)(T6 * N6) << " err=" << esc(first_err);
        READ("R6a", o6.str());
        CHECK(wo == 0 && dp == 0 && iv == 0 && aff_bad.load() == 0 && all6.size() == (unsigned int)(T6 * N6), "R6a",
              "模型侧 insert(): 并发下返回的自增主键必须指向本线程刚插入的行");
        edit->exec_dml("DELETE FROM fortune WHERE message LIKE 'mz%'");
    }

    // R6b 模型侧预编译通路：同一形状走 exec_insert()(生成码里 4 处 prepared 元组接口)
    {
        const int T6 = 8, N6 = 50;
        std::vector<std::vector<std::pair<long long, std::string>>> own6(T6);
        std::atomic<unsigned int> aff_bad{0};
        std::mutex err_mu;
        std::string first_err;
        std::vector<std::thread> th6;
        for (int t = 0; t < T6; t++)
        {
            th6.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < N6; i++)
                    {
                        std::string k = "mp" + std::to_string(t) + "-" + std::to_string(i);
                        probe_model::Fortune m;
                        m.data.message = k;
                        auto one = m.exec_insert();
                        if (std::get<0>(one) != 1 || std::get<1>(one) == 0)
                        {
                            aff_bad++;
                            std::lock_guard<std::mutex> g(err_mu);
                            if (first_err.empty())
                                first_err = m.error_msg;
                            continue;
                        }
                        own6[t].push_back({(long long)std::get<1>(one), k});
                    }
                });
        }
        for (auto &th : th6)
            th.join();
        std::vector<std::pair<long long, std::string>> all6;
        for (auto &v : own6)
            all6.insert(all6.end(), v.begin(), v.end());
        unsigned int wo = 0, dp = 0, iv = 0;
        verify_rowids(all6, sel, wo, dp, iv, "fortune", "message");
        std::ostringstream o6;
        o6 << "wrong_owner=" << wo << " duplicate_rowid=" << dp << " invalid=" << iv
           << " aff_bad=" << aff_bad.load() << " of=" << (unsigned int)all6.size()
           << " expect=" << (unsigned int)(T6 * N6) << " err=" << esc(first_err);
        READ("R6b", o6.str());
        CHECK(wo == 0 && dp == 0 && iv == 0 && aff_bad.load() == 0 && all6.size() == (unsigned int)(T6 * N6), "R6b",
              "模型侧 exec_insert(): 并发下返回的自增主键必须指向本线程刚插入的行");
        edit->exec_dml("DELETE FROM fortune WHERE message LIKE 'mp%'");
    }
}

// ============ 族 F：协程通路 ============
struct FStats
{
    std::atomic<unsigned int> wrong{0};
    std::string notes;
    std::vector<std::pair<long long, std::string>> rowids;
};

static asio::awaitable<void> f_basic(conn_ptr edit, conn_ptr sel, std::shared_ptr<FStats> st)
{
    bool p = co_await edit->async_ping();
    int r  = co_await edit->async_exec_sql("SELECT 1");
    unsigned int d = co_await edit->async_exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('f_basic')");
    unsigned int n = co_await edit->async_fetch_directly(
        "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'f_%'", [](int, char **, auto) { return true; });
    orm::sqlite_query_result qr;
    bool qf = co_await edit->async_query_fetch("SELECT k FROM _pz_probe_t1 WHERE k='f_basic'", qr);
    orm::sqlite_scalar_result sr;
    bool sc = co_await sel->async_query_scalar("SELECT count(*) FROM _pz_probe_t1", sr);
    if (!p || r != 0 || d != 1 || n == 0 || !qf || qr.rows.size() != 1 || !sc || sr.is_null)
        st->wrong++;
    st->notes += "basic ping=" + std::to_string(p) + " exec_sql=" + std::to_string(r) +
                 " dml=" + std::to_string(d) + " fetchd=" + std::to_string(n) +
                 " qf_rows=" + std::to_string(qr.rows.size()) + " scalar=" + esc(sr.value) + " ";
    co_await edit->async_exec_dml("DELETE FROM _pz_probe_t1 WHERE k='f_basic'");
    co_return;
}

static asio::awaitable<void> f_prepared(conn_ptr edit, std::shared_ptr<FStats> st)
{
    objval_vec pp;
    pp.push_back(http::obj_val(std::string("f_prep_1")));
    unsigned int d = co_await edit->async_exec_dml_prepared("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp);
    unsigned int n = co_await edit->async_fetch_prepared(
        "SELECT k FROM _pz_probe_t1 WHERE k=?", pp, [](int, char **, auto) { return true; });
    unsigned int b = co_await edit->async_fetch_prepared_binary(
        "SELECT k FROM _pz_probe_t1 WHERE k=?", pp, [](int, char **, auto) { return true; });
    if (d != 1 || n != 1 || b != 1)
        st->wrong++;
    st->notes += "prepared dml=" + std::to_string(d) + " fetch=" + std::to_string(n) +
                 " binary=" + std::to_string(b) + " ";
    co_await edit->async_exec_dml("DELETE FROM _pz_probe_t1 WHERE k='f_prep_1'");
    co_return;
}

static asio::awaitable<void> f_many(conn_ptr edit, std::shared_ptr<FStats> st, int t, int n)
{
    for (int i = 0; i < n; i++)
    {
        std::string k = "fm" + std::to_string(t) + "_" + std::to_string(i);
        objval_vec pp;
        pp.push_back(http::obj_val(k));
        unsigned int d = co_await edit->async_exec_dml_prepared("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp);
        if (d != 1)
            st->wrong++;
    }
    co_return;
}

static asio::awaitable<void> f_rowid(conn_ptr edit, std::shared_ptr<FStats> st, std::string k, int delay_ms)
{
    objval_vec pp;
    pp.push_back(http::obj_val(k));
    unsigned int d = co_await edit->async_exec_dml_prepared("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp);
    asio::steady_timer timer(co_await asio::this_coro::executor);
    timer.expires_after(std::chrono::milliseconds(delay_ms));
    co_await timer.async_wait(asio::use_awaitable);
    long long id = co_await edit->async_last_insert_rowid();
    if (d == 1)
        st->rowids.push_back({id, k});
    else
        st->wrong++;
    co_return;
}

// 元组接口(两条异步 twin)：写语句与自增主键在同一次提交里取完，结果写进调用方自己的槽位
static asio::awaitable<void> f_rowid_tuple(conn_ptr edit,
                                           std::shared_ptr<FStats> st,
                                           std::string prefix,
                                           int ops,
                                           bool prepared,
                                           std::vector<std::pair<long long, std::string>> *out)
{
    for (int i = 0; i < ops; i++)
    {
        std::string k = prefix + "-" + std::to_string(i);
        long long id  = 0;
        unsigned int d = 0;
        if (prepared)
        {
            objval_vec pp;
            pp.push_back(http::obj_val(k));
            auto one = co_await edit->async_exec_dml_prepared_with_rowid("INSERT INTO _pz_probe_t1 (k) VALUES (?)", pp);
            d  = std::get<0>(one);
            id = std::get<1>(one);
        }
        else
        {
            auto one = co_await edit->async_exec_dml_with_rowid("INSERT INTO _pz_probe_t1 (k) VALUES ('" + k + "')");
            d  = std::get<0>(one);
            id = std::get<1>(one);
        }
        if (d == 1)
            out->push_back({id, k});
        else
            st->wrong++;
        // 让出执行权，把 4 条协程的提交交错开
        asio::steady_timer timer(co_await asio::this_coro::executor);
        timer.expires_after(std::chrono::milliseconds(1));
        co_await timer.async_wait(asio::use_awaitable);
    }
    co_return;
}

static asio::awaitable<void> f_txn_a(conn_ptr edit, std::shared_ptr<FStats> st)
{
    bool b = co_await edit->async_begin_transaction();
    st->notes += "txnA_begin=" + std::to_string(b) + " ";
    if (!b)
    {
        st->wrong++;
        co_return;
    }
    asio::steady_timer timer(co_await asio::this_coro::executor);
    timer.expires_after(std::chrono::milliseconds(50));
    co_await timer.async_wait(asio::use_awaitable);
    bool rb = co_await edit->async_rollback_transaction();
    st->notes += "txnA_rollback=" + std::to_string(rb) + " ";
    co_return;
}

static asio::awaitable<void> f_txn_b(conn_ptr edit, std::shared_ptr<FStats> st)
{
    asio::steady_timer timer(co_await asio::this_coro::executor);
    timer.expires_after(std::chrono::milliseconds(20));
    co_await timer.async_wait(asio::use_awaitable);
    unsigned int d = co_await edit->async_exec_dml("INSERT INTO _pz_probe_t1 (k) VALUES ('B_coro')");
    st->notes += "txnB_write=" + std::to_string(d) + " ";
    co_return;
}

static asio::awaitable<void> f_dbconn(std::shared_ptr<FStats> st)
{
    auto ulink = std::make_unique<orm::db_conn>("lite");
    bool b     = co_await ulink->async_begin_commit();
    if (!b)
    {
        st->notes += "dbconn_begin=F ";
        st->wrong++;
        co_return;
    }
    unsigned int d = co_await ulink->sqlite_edit_conn->async_exec_dml(
        "INSERT INTO _pz_probe_t1 (k) VALUES ('f_dbconn')");
    bool c = co_await ulink->async_commit();
    if (d != 1 || !c)
        st->wrong++;
    st->notes += "dbconn dml=" + std::to_string(d) + " commit=" + std::to_string(c) + " ";
    co_return;
}

static asio::awaitable<void> f_connlife(asio::io_context *ioc, std::string path, std::shared_ptr<FStats> st)
{
    auto c = std::make_shared<orm::sqlite_conn_base>();
    bool ok = co_await c->async_connect(lite_conf(path));
    if (!ok)
    {
        st->notes += "connlife connect=F err=" + esc(c->error_msg) + " ";
        st->wrong++;
        co_return;
    }
    c->start_worker_thread(*ioc, path);
    bool p          = co_await c->async_ping();
    unsigned int dd = co_await c->async_exec_dml("CREATE TABLE life (a INTEGER)");
    unsigned int e  = co_await c->async_exec_dml("INSERT INTO life VALUES (1)");
    bool cl         = co_await c->async_close();
    bool closed     = c->is_closed();
    unsigned int after = co_await c->async_exec_dml("INSERT INTO life VALUES (2)");
    std::string aerr   = c->error_msg;
    c->stop_worker_thread();
    st->notes += "connlife ping=" + std::to_string(p) + " ddl=" + std::to_string(dd) +
                 " ins=" + std::to_string(e) + " close=" + std::to_string(cl) +
                 " is_closed=" + std::to_string(closed) + " after_close=" + std::to_string((int)after) +
                 " err=" + esc(aerr) + " ";
    if (!p || dd != 0 || e != 1 || !cl || !closed || after != (unsigned int)-1)
        st->wrong++;
    co_return;
}

static void fam_F(asio::io_context &ioc, const conn_ptr &edit, const conn_ptr &sel)
{
    if (!fam("F"))
        return;
    std::printf("\n---- F: coroutine path ----\n");

    edit->exec_dml("DELETE FROM _pz_probe_t1");
    auto st = std::make_shared<FStats>();

    unsigned int before = st->wrong.load();
    asio::co_spawn(ioc, f_basic(edit, sel, st), asio::use_future).get();
    std::string detail = st->notes;
    st->notes.clear();
    CHECK(st->wrong.load() == before, "F1", "协程: ping/exec_sql/exec_dml/fetch_directly/query_fetch/query_scalar");
    READ("F1", detail);

    before = st->wrong.load();
    asio::co_spawn(ioc, f_prepared(edit, st), asio::use_future).get();
    CHECK(st->wrong.load() == before, "F2", "协程: 绑定写 + fetch_prepared + fetch_prepared_binary 各 1 行");
    READ("F2", st->notes);

    before = st->wrong.load();
    {
        std::vector<std::future<void>> futs;
        for (int t = 0; t < 40; t++)
            futs.push_back(asio::co_spawn(ioc, f_many(edit, st, t, 10), asio::use_future));
        for (auto &f : futs)
            f.get();
    }
    unsigned int rows = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'fm%'");
    std::ostringstream os3;
    os3 << "wrong_delta=" << (st->wrong.load() - before) << " rows=" << rows << " expect=400";
    READ("F3", os3.str());
    CHECK(st->wrong.load() == before && rows == 400, "F3", "40 个并发协程 × 10 绑定写: 行数完整且无错误");

    edit->exec_dml("DELETE FROM _pz_probe_t1");
    {
        st->rowids.clear();
        std::vector<std::future<void>> futs;
        futs.push_back(asio::co_spawn(ioc, f_rowid(edit, st, std::string("race_a"), 40), asio::use_future));
        futs.push_back(asio::co_spawn(ioc, f_rowid(edit, st, std::string("race_b"), 0), asio::use_future));
        for (auto &f : futs)
            f.get();
        unsigned int wrong_owner = 0;
        std::ostringstream os4;
        for (auto &rp : st->rowids)
        {
            std::string got;
            orm::sqlite_scalar_result sr;
            if (sel->query_scalar("SELECT k FROM _pz_probe_t1 WHERE id=" + std::to_string(rp.first), sr) &&
                !sr.is_null)
                got = sr.value;
            if (got != rp.second)
                wrong_owner++;
            os4 << rp.first << "->" << esc(got) << "(want " << esc(rp.second) << ") ";
        }
        os4 << "wrong_owner=" << wrong_owner;
        READ("F4", os4.str());
        CHECK(st->rowids.size() == 2 && wrong_owner == 0, "F4",
              "协程交错下 async_last_insert_rowid 必须指向本协程插入的行");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    edit->exec_dml("DELETE FROM _pz_probe_t1");
    {
        std::vector<std::future<void>> futs;
        futs.push_back(asio::co_spawn(ioc, f_txn_b(edit, st), asio::use_future));
        futs.push_back(asio::co_spawn(ioc, f_txn_a(edit, st), asio::use_future));
        for (auto &f : futs)
            f.get();
        unsigned int b_rows = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='B_coro'");
        READ("F5", st->notes + "B_coro_rows=" + std::to_string(b_rows) + " in_txn=" +
                                 std::to_string((int)edit->in_transaction()));
        CHECK(b_rows == 1, "F5", "协程 A 的事务回滚不得吞掉自报成功的协程 B 的写");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }

    edit->exec_dml("DELETE FROM _pz_probe_t1 WHERE k='f_dbconn'");
    {
        asio::co_spawn(ioc, f_dbconn(st), asio::use_future).get();
        unsigned int d_rows = count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k='f_dbconn'");
        before              = st->wrong.load();
        std::string note    = st->notes;
        st->notes.clear();
        READ("F6", "dbconn_rows=" + std::to_string(d_rows) + " " + note);
        CHECK(d_rows == 1 && !edit->in_transaction(), "F6",
              "orm::db_conn 协程事务: 提交结果可见且事务权归还");
    }

    {
        std::string path = std::string(kTmpDir) + "/f_connlife.db";
        std::remove(path.c_str());
        std::remove((path + "-wal").c_str());
        std::remove((path + "-shm").c_str());
        before = st->wrong.load();
        asio::co_spawn(ioc, f_connlife(&ioc, path, st), asio::use_future).get();
        std::string note = st->notes;
        st->notes.clear();
        READ("F7", note);
        CHECK(st->wrong.load() == before, "F7",
              "协程: 独立连接 connect/ping/DDL/DML/close 语义一致(关闭后写入明确失败)");
        std::remove(path.c_str());
        std::remove((path + "-wal").c_str());
        std::remove((path + "-shm").c_str());
    }

    edit->exec_dml("DELETE FROM _pz_probe_t1");
    {
        before = st->wrong.load();
        const int OPS = 200;
        std::vector<std::pair<long long, std::string>> c1, c2, c3, c4;
        std::vector<std::future<void>> futs;
        futs.push_back(asio::co_spawn(ioc, f_rowid_tuple(edit, st, std::string("tup_text_a"), OPS, false, &c1), asio::use_future));
        futs.push_back(asio::co_spawn(ioc, f_rowid_tuple(edit, st, std::string("tup_text_b"), OPS, false, &c2), asio::use_future));
        futs.push_back(asio::co_spawn(ioc, f_rowid_tuple(edit, st, std::string("tup_prep_a"), OPS, true, &c3), asio::use_future));
        futs.push_back(asio::co_spawn(ioc, f_rowid_tuple(edit, st, std::string("tup_prep_b"), OPS, true, &c4), asio::use_future));
        for (auto &f : futs)
            f.get();
        std::vector<std::pair<long long, std::string>> all;
        for (auto *v : {&c1, &c2, &c3, &c4})
            all.insert(all.end(), v->begin(), v->end());
        unsigned int wo = 0, dp = 0, iv = 0;
        verify_rowids(all, sel, wo, dp, iv);
        std::ostringstream os8;
        os8 << "wrong_owner=" << wo << " duplicate_rowid=" << dp << " invalid=" << iv
            << " rows=" << count_rows(*sel, "SELECT k FROM _pz_probe_t1 WHERE k LIKE 'tup_%'")
            << " of=" << (unsigned int)all.size() << " expect=" << (unsigned int)(4 * OPS);
        READ("F8", os8.str());
        CHECK(st->wrong.load() == before && all.size() == (unsigned int)(4 * OPS) && wo == 0 && dp == 0 && iv == 0, "F8",
              "协程交错下 async 元组接口(文本+预编译 twin)的自增主键必须指向本协程插入的行");
        edit->exec_dml("DELETE FROM _pz_probe_t1");
    }
    edit->exec_dml("DELETE FROM _pz_probe_t1");
}

// ============ 族 G：worker 停止后的内联回退（单 worker 串行契约）============
static void fam_G(asio::io_context &ioc)
{
    if (!fam("G"))
        return;
    std::printf("\n---- G: worker stopped -> inline execution ----\n");

    std::string f = std::string(kTmpDir) + "/g_serial.db";
    std::remove(f.c_str());
    std::remove((f + "-wal").c_str());
    std::remove((f + "-shm").c_str());

    auto conn = std::make_shared<orm::sqlite_conn_base>();
    if (!conn->connect(lite_conf(f)))
    {
        CHECK(false, "G0", "探针自建连接: " + conn->error_msg);
        return;
    }
    conn->start_worker_thread(ioc, f);
    CHECK(conn->is_worker_running(), "G1a", "worker 已启动(任务全部落在 worker 线程上串行)");

    conn->exec_dml("CREATE TABLE g (id INTEGER PRIMARY KEY, v REAL, t TEXT)");
    for (int i = 1; i <= 40; i++)
        conn->exec_dml("INSERT INTO g VALUES (" + std::to_string(i) + ", " + std::to_string(i * 1.5 + 0.125) +
                       ", 'v" + std::to_string(i) + "')");

    auto run_read = [&](int threads, int per_thread, std::atomic<unsigned int> &max_inside,
                        std::atomic<unsigned int> &mismatch, std::atomic<unsigned int> &completed)
    {
        std::atomic<int> inside{0};
        std::vector<std::thread> ths;
        for (int t = 0; t < threads; t++)
        {
            ths.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < per_thread; i++)
                    {
                        int row = (t * per_thread + i) % 40 + 1;
                        char want[64];
                        std::snprintf(want, sizeof(want), "%.17g", (double)(row * 1.5 + 0.125));
                        std::string got;
                        // inside 只在 handler 内部计数：外层"调用在进行"必然重叠，
                        // 要判的是"同一条连接的取数回调是否并发执行"
                        conn->fetch_directly("SELECT v, t FROM g WHERE id=" + std::to_string(row),
                                             [&](int, char **, auto get_data) -> bool
                                             {
                            int cur = ++inside;
                            if ((unsigned int)cur > max_inside.load())
                                max_inside.store((unsigned int)cur);
                            auto [p, len] = get_data(0);
                            if (p)
                                got.assign(reinterpret_cast<const char *>(p), len);
                            --inside;
                            return true; });
                        if (got != want)
                            mismatch++;
                        completed++;
                    }
                });
        }
        for (auto &th : ths)
            th.join();
    };

    std::atomic<unsigned int> max_c{0}, mism_c{0}, done_c{0};
    std::printf("MARK G1b-begin worker-on 4x50 fetch_directly\n");
    run_read(4, 50, max_c, mism_c, done_c);
    std::ostringstream os1;
    os1 << "max_inside=" << max_c.load() << " mismatch=" << mism_c.load() << " completed=" << done_c.load();
    READ("G1b", os1.str());
    CHECK(done_c.load() == 200 && max_c.load() == 1 && mism_c.load() == 0, "G1b",
          "控制臂(worker 在跑): 同一句柄的 handler 从不重叠, REAL 列还原缓冲不串值");

    std::printf("MARK G2a-begin stop_worker_thread\n");
    conn->stop_worker_thread();
    CHECK(!conn->is_worker_running(), "G2a", "worker 已停止(后续任务改在调用线程内联执行)");

    std::atomic<unsigned int> max_s{0}, mism_s{0}, done_s{0};
    std::printf("MARK G3a-begin worker-off 4x50 fetch_directly\n");
    run_read(4, 50, max_s, mism_s, done_s);
    std::ostringstream os2;
    os2 << "max_inside=" << max_s.load() << " mismatch=" << mism_s.load() << " completed=" << done_s.load()
        << " err=" << esc(conn->error_msg);
    READ("G3a", os2.str());
    CHECK(done_s.load() == 200 && max_s.load() == 1 && mism_s.load() == 0, "G3a",
          "worker 停止后仍须保持单执行者语义(不重叠、不串值、不丢任务)");

    std::printf("MARK G3c-begin 竞争后再发一条串行 DDL(崩溃点定位)\n");
    conn->exec_dml("DELETE FROM g");
    conn->exec_dml("CREATE TABLE w (id INTEGER PRIMARY KEY, t TEXT)");
    std::printf("MARK G3d-done 串行 DDL 返回, err=%s\n", esc(conn->error_msg).c_str());
    {
        std::vector<std::thread> ths;
        std::printf("MARK G4a-begin worker-off 4x25 exec_dml_prepared INSERT\n");
        for (int t = 0; t < 4; t++)
        {
            ths.emplace_back(
                [&, t]()
                {
                    for (int i = 0; i < 25; i++)
                    {
                        objval_vec pp;
                        pp.push_back(http::obj_val((long long)(t * 100 + i)));
                        pp.push_back(http::obj_val("w" + std::to_string(t) + "_" + std::to_string(i)));
                        conn->exec_dml_prepared("INSERT INTO w (id, t) VALUES (?,?)", pp);
                    }
                });
        }
        for (auto &th : ths)
            th.join();
    }
    unsigned int wrows = count_rows(*conn, "SELECT id FROM w");
    READ("G4a", "inline_write_rows=" + std::to_string(wrows) + " expect=100");
    CHECK(wrows == 100, "G4a", "worker 停止后内联并发写入行数完整(不丢不重)");

    conn->close();
    std::remove(f.c_str());
    std::remove((f + "-wal").c_str());
    std::remove((f + "-shm").c_str());
}

int main(int argc, char **argv)
{
    // G 族允许崩溃：块缓冲会把崩溃前的读数全部留在内存里丢掉
    setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc > 1)
    {
        std::string arg = argv[1];
        if (arg != "all")
        {
            std::string cur;
            for (char ch : arg)
            {
                if (ch == ',' || ch == ' ')
                {
                    if (!cur.empty())
                        g_families.insert(cur);
                    cur.clear();
                }
                else
                    cur.push_back((char)std::toupper((unsigned char)ch));
            }
            if (!cur.empty())
                g_families.insert(cur);
        }
    }
    if (g_families.empty())
    {
        // 默认 all 不含 G：那一族把同一句柄放到多个线程上跑，允许崩溃，
        // 必须显式传 G 且单独一次进程，否则一次崩溃就丢掉后面全部读数
        for (const char *f : {"A", "B", "C", "D", "E", "F", "R"})
            g_families.insert(f);
    }

    asio::io_context ioc;
    // 夹具目录先建好：D/G 族往里写独立库文件，目录不存在时 connect 会直接失败
    std::error_code mk_ec;
    std::filesystem::create_directories(kTmpDir, mk_ec);
    if (!std::filesystem::exists(kLiteDb))
    {
        std::printf("FAIL Z0 找不到 lite 库 %s (当前目录 %s)；从仓库根启动，或用 PAOZHU_LITE_DB 指过去\n",
                    kLiteDb.c_str(), std::filesystem::current_path().c_str());
        return 2;
    }
    std::string initlog = orm::init_orm_conn_pool(ioc, "conf/orm.conf");
    READ("Z0", "pool_init=" + esc(initlog));

    auto pool = orm::get_orm_conn_pool_obj()["lite"];
    if (!pool)
    {
        std::printf("FAIL Z0 conf/orm.conf 没有 [lite] 段(需 -DENABLE_SQLITE=ON)\n");
        return 2;
    }
    std::thread io_thread(
        [&ioc]()
        {
            auto guard = asio::make_work_guard(ioc);
            ioc.run();
        });

    auto edit = pool->get_sqlite_edit_conn();
    auto sel  = pool->get_sqlite_select_conn();
    if (!edit || !sel)
    {
        std::printf("FAIL Z0 连接池借出 sqlite edit/select 句柄失败\n");
        ioc.stop();
        io_thread.join();
        return 2;
    }
    CHECK(edit->is_closed() == false && sel->is_closed() == false, "Z0a", "两条常驻句柄均已连接");
    READ("Z1", "sqlite_version=" + std::string(orm::sqlite_conn_base::sqlite_version()) + " db=" +
                   edit->db_file_path() + " sel_db=" + sel->db_file_path());

    // 直连臂(损坏库/跨写者锁)和池里的 [lite] 必须是同一个库文件，否则两组读数不可比
    if (std::filesystem::weakly_canonical(edit->db_file_path()) != std::filesystem::weakly_canonical(kLiteDb))
    {
        std::printf("FATAL conf/orm.conf 的 [lite] host=%s，探针直连的是 %s，两者不是同一个库\n",
                    edit->db_file_path().c_str(), kLiteDb.c_str());
        return 4;
    }

    const char *fixture_stmts[] = {
        "CREATE TABLE IF NOT EXISTS _pz_probe_t1 (id INTEGER PRIMARY KEY AUTOINCREMENT, k TEXT, txt TEXT, blb BLOB, nul TEXT, num REAL, num2 REAL, ival INTEGER)",
        "CREATE TABLE IF NOT EXISTS _pz_probe_u (id INTEGER PRIMARY KEY AUTOINCREMENT, k TEXT, num REAL)",
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_pz_probe_u_k ON _pz_probe_u(k)",
        "CREATE TABLE IF NOT EXISTS _pz_probe_abort (a INTEGER UNIQUE ON CONFLICT ABORT)",
        "CREATE TABLE IF NOT EXISTS _pz_probe_roll (a INTEGER UNIQUE ON CONFLICT ROLLBACK)",
        "DELETE FROM _pz_probe_t1",
        "DELETE FROM _pz_probe_abort",
        "DELETE FROM _pz_probe_roll",
    };
    std::string fixture_err;
    for (const char *fs : fixture_stmts)
    {
        edit->exec_dml(fs);
        // 每条语句入口会 clear_error，只看最后一条会把中途的错误吞掉
        if (fixture_err.empty() && !edit->error_msg.empty())
            fixture_err = std::string(fs).substr(0, 24) + " -> " + edit->error_msg;
    }
    CHECK(fixture_err.empty(), "Z2", "夹具表建立无错误");
    if (!fixture_err.empty())
    {
        // 夹具没建起来时后面每一族都会红，先把这条归因到"库文件被别的进程占着"，
        // 而不是让级联红冒充驱动缺陷
        READ("Z2", "fixture_err=" + esc(fixture_err));
        std::printf("FATAL 夹具表建立失败，本轮读数全部作废(同一时刻只允许一个探针进程)\n");
        pool->back_sqlite_edit_conn(std::move(edit));
        pool->back_sqlite_select_conn(std::move(sel));
        ioc.stop();
        io_thread.join();
        orm::init_orm_conn_pool_release();
        return 3;
    }

    fam_A(edit, sel);
    fam_B(edit, sel);
    fam_C(edit, sel);
    fam_E(edit, sel);
    fam_R(edit, sel);
    fam_F(ioc, edit, sel);
    fam_D(edit, sel);
    fam_G(ioc);

    edit->exec_dml("DELETE FROM _pz_probe_t1");
    edit->exec_dml("DELETE FROM _pz_probe_u");
    edit->exec_dml("DELETE FROM _pz_probe_abort");
    edit->exec_dml("DELETE FROM _pz_probe_roll");
    edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_t1");
    edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_u");
    edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_abort");
    edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_roll");
    edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_s");
    edit->exec_dml("DROP TABLE IF EXISTS _pz_probe_snap");

    pool->back_sqlite_edit_conn(std::move(edit));
    pool->back_sqlite_select_conn(std::move(sel));
    ioc.stop();
    io_thread.join();
    orm::init_orm_conn_pool_release();

    std::printf("\nSUMMARY pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}