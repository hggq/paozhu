# 生成候选修复版驱动副本 temp/sqlite_real_1022/sqlite_conn_fix.cpp
# 产品源码不动：只在 temp/ 里改副本，用来证明每条候选补丁恰好翻转对应判据。
import os
import sys

# 源码在 tests/orm/，生成物落 temp/；ROOT 由脚本位置往上三层推出，换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
GEN = os.path.join(ROOT, "temp/sqlite_real_1022")
SRC = os.path.join(ROOT, "vendor/httpserver/src/sqlite_conn.cpp")
OUT = os.path.join(GEN, "sqlite_conn_fix.cpp")

src = open(SRC, encoding="utf-8").read()
base_md5 = __import__("hashlib").md5(src.encode()).hexdigest()
applied = []


def sub(tag, anchor, repl, expect=1):
    global src
    n = src.count(anchor)
    assert n == expect, "%s 锚点命中 %d 次，预期 %d" % (tag, n, expect)
    src = src.replace(anchor, repl)
    applied.append(tag)


# ---- F-3 exec_bound_impl 入口清错误(改后 A19c 绿) ----
sub("F-3", """bool sqlite_conn_base::exec_bound_impl(const std::string &sql, const std::vector<sqlite_bind_param> &params)
{
    if (!db_ || isclose_.load())""", """bool sqlite_conn_base::exec_bound_impl(const std::string &sql, const std::vector<sqlite_bind_param> &params)
{
    this->clear_error();

    if (!db_ || isclose_.load())""")

# ---- F-2b exec_bound 失败路径同步事务窗口 ----
sub("F-2b", """    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(sqlite3_errmsg(db_));
        this->check_and_attempt_recovery(sqlite_err_code);
        return false;
    }""", """    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(sqlite3_errmsg(db_));
        this->check_and_attempt_recovery(sqlite_err_code);
        this->reconcile_transaction_state_after_error();
        return false;
    }""")

# ---- F-2e fetch_directly 失败路径同步事务窗口 ----
sub("F-2e", """        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(sqlite3_errmsg(db_));
        this->check_and_attempt_recovery(sqlite_err_code);
        if (rc == SQLITE_SCHEMA)""", """        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(sqlite3_errmsg(db_));
        this->check_and_attempt_recovery(sqlite_err_code);
        this->reconcile_transaction_state_after_error();
        if (rc == SQLITE_SCHEMA)""")

# ---- F-5 fetch_directly 空 BLOB 与 NULL 形状区分 ----
sub("F-5", """            const void *blob = sqlite3_column_blob(stmt, col_idx);
            int len          = sqlite3_column_bytes(stmt, col_idx);
            if (blob && len > 0)
            {
                return {const_cast<unsigned char *>(static_cast<const unsigned char *>(blob)), static_cast<size_t>(len)};
            }
            return {nullptr, 0};""", """            const void *blob = sqlite3_column_blob(stmt, col_idx);
            int len          = sqlite3_column_bytes(stmt, col_idx);
            if (blob && len > 0)
            {
                return {const_cast<unsigned char *>(static_cast<const unsigned char *>(blob)), static_cast<size_t>(len)};
            }
            if (blob)
            {
                return {const_cast<unsigned char *>(static_cast<const unsigned char *>(blob)), 0};
            }
            // 长度为 0 的 BLOB，column_blob 可能给空指针；column_text 仍给有效指针，
            // 用它把"空值"和 NULL 分开（NULL 才是 {nullptr,0}）
            const unsigned char *eb = sqlite3_column_text(stmt, col_idx);
            if (eb)
            {
                return {const_cast<unsigned char *>(eb), 0};
            }
            return {nullptr, 0};""")

# ---- F-4 二进制读路：空 TEXT/BLOB 给空 string_view，不再等同 NULL ----
sub("F-4a", """                const char *p = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_idx));
                if (!p)
                    return std::monostate{};
                int len = sqlite3_column_bytes(stmt, col_idx);""", """                const char *p = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_idx));
                if (!p)
                    return std::string_view();// 列类型是 TEXT，空值不是 NULL
                int len = sqlite3_column_bytes(stmt, col_idx);""")
sub("F-4b", """                const void *b = sqlite3_column_blob(stmt, col_idx);
                if (!b)
                    return std::monostate{};
                int len = sqlite3_column_bytes(stmt, col_idx);""", """                const void *b = sqlite3_column_blob(stmt, col_idx);
                if (!b)
                    return std::string_view();// 列类型是 BLOB，空值不是 NULL
                int len = sqlite3_column_bytes(stmt, col_idx);""")

# ---- F-2c 两条 prepared 读路失败路径同步事务窗口 ----
sub("F-2c", """        if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
            this->set_error(sqlite3_errmsg(db_));
            return row_num;
        }""", """        if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
            this->set_error(sqlite3_errmsg(db_));
            this->reconcile_transaction_state_after_error();
            return row_num;
        }""", expect=2)

# ---- F-6 预编译通路拒绝多语句 + F-1 取完行再判错 + F-2a 失败路径恢复/同步 ----
helper = """// prepare 的 tail 指针非空白即说明文本里还有第二条语句；
// 预编译通路只执行第一条，多语句必须显式拒绝，不能静默少执行
static bool sqlite_sql_has_second_statement(sqlite3 *db, const std::string &sql)
{
    sqlite3_stmt *tmp = nullptr;
    const char *tail  = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &tmp, &tail) != SQLITE_OK)
    {
        if (tmp)
        {
            sqlite3_finalize(tmp);
        }
        return false;
    }
    if (tmp)
    {
        sqlite3_finalize(tmp);
    }
    while (tail && *tail)
    {
        unsigned char ch = (unsigned char)*tail;
        if (std::isspace(ch) || ch == ';')
        {
            tail++;
            continue;
        }
        return true;
    }
    return false;
}

unsigned int sqlite_conn_base::exec_dml_prepared_impl("""
sub("F-6h", "unsigned int sqlite_conn_base::exec_dml_prepared_impl(", helper, expect=1)

sub("F-6g", """    sqlite3_int64 total_before = sqlite3_total_changes(db_);
    auto bind_params           = _obj_vals_to_bind_params(params);""", """    if (sql.find(';') != std::string::npos && sqlite_sql_has_second_statement(db_, sql))
    {
        this->set_error("prepared statement must contain a single statement");
        return (unsigned int)-1;
    }

    sqlite3_int64 total_before = sqlite3_total_changes(db_);
    auto bind_params           = _obj_vals_to_bind_params(params);""")

sub("F-1", """        int rc = sqlite3_step(stmt);
        sqlite3_reset(stmt);

        if (rc == SQLITE_SCHEMA)""", """        int rc = sqlite3_step(stmt);
        // 行返回型语句(INSERT/UPDATE/DELETE ... RETURNING)会先给行再结束，
        // 取完剩余行才判错，否则语句已写入却被报成失败
        while (rc == SQLITE_ROW)
        {
            rc = sqlite3_step(stmt);
        }
        sqlite3_reset(stmt);

        if (rc == SQLITE_SCHEMA)""")

sub("F-2a", """        if (rc != SQLITE_DONE)
        {
            this->set_error(sqlite3_errmsg(db_));
            return (unsigned int)-1;
        }""", """        if (rc != SQLITE_DONE)
        {
            int sqlite_err_code = sqlite3_errcode(db_);
            this->set_error(sqlite3_errmsg(db_));
            this->check_and_attempt_recovery(sqlite_err_code);
            this->reconcile_transaction_state_after_error();
            return (unsigned int)-1;
        }""")

sub("F-6i", "#include <climits>", "#include <cctype>\n#include <climits>")

# ---- F-7 内联执行仍须单执行者(头文件模板 + 驱动回退点) ----
# 该补丁往类里加了一枚成员，对象体积随之变化：只有把链接行上每一枚构造/内联
# 该类的对象都用补丁头重编才可比。当前台子只换了驱动 + 连接池两枚对象，混合布局
# 会在 os_unfair_lock 上 abort，所以默认不进测量产物，只作为候选代码交审核。
APPLY_F7 = os.environ.get("APPLY_F7", "0") == "1"
HDR = os.path.join(ROOT, "vendor/httpserver/include/sqlite_conn.h")
FIXINC = os.path.join(os.path.dirname(OUT), "fixinc")
OUTH = os.path.join(FIXINC, "sqlite_conn.h")
os.makedirs(FIXINC, exist_ok=True)


def hsub(tag, anchor, repl, expect=1):
    global h
    n = h.count(anchor)
    assert n == expect, "%s 头文件锚点命中 %d 次，预期 %d" % (tag, n, expect)
    h = h.replace(anchor, repl)
    applied.append(tag)


if APPLY_F7:
    h = open(HDR, encoding="utf-8").read()
    hsub("F-7m", """    std::mutex error_mutex_;
    orm_conn_t conn_config_;
};""", """    std::mutex error_mutex_;
    orm_conn_t conn_config_;
    // worker 停止后任务改在调用线程执行，这把锁保证同一连接仍然只有一个执行者。
    // 放在成员末尾：只增对象大小、不移动既有成员偏移
    std::mutex inline_exec_mutex_;
};""")
    hsub("F-7s", """    if (!worker_running_.load())
    {
        try
        {
            return fn();
        }""", """    if (!worker_running_.load())
    {
        std::lock_guard<std::mutex> inline_lock(inline_exec_mutex_);
        try
        {
            return fn();
        }""")
    hsub("F-7r", """    if (!worker_running_.load())
        co_return fn();""", """    if (!worker_running_.load())
    {
        std::lock_guard<std::mutex> inline_lock(inline_exec_mutex_);
        co_return fn();
    }""")
    open(OUTH, "w", encoding="utf-8").write(h)

    sub("F-7c", """        // worker 已停止（程序关闭/停服）：直接在调用线程执行任务，
        // 否则 submit_sync 的 future.get() 将永久阻塞
        task();""", """        // worker 已停止（程序关闭/停服）：直接在调用线程执行任务，
        // 否则 submit_sync 的 future.get() 将永久阻塞
        std::lock_guard<std::mutex> inline_lock(inline_exec_mutex_);
        task();""")
else:
    # 未打补丁时删掉旧副本，避免 build_probe.py 误判成"头文件已改布局"而去换池对象
    if os.path.exists(OUTH):
        os.remove(OUTH)

open(OUT, "w", encoding="utf-8").write(src)
sys.stdout.write("base_md5=%s applied=%s out=%s\n" % (base_md5, ",".join(applied), OUT))
