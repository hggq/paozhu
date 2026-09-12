/*
 *  @dest schema-createdb  — CLI: paozhu_cli schema-createdb <dbtag> [-u <admin_user>]
 *  1. 从 conf/orm.conf 解析 dbtag 配置
 *  2. MySQL/PG: admin 连系统库 → CREATE DATABASE → CREATE USER → GRANT
 *     SQLite:  跳过（文件系统）
 *  3. 交互式询问是否导入 schema 表结构
 *  4. read schema/<dbtag>/tables/ directory *.sql → FK Topological sorting → Execute table by table DDL
 *  5. same name schema/<dbtag>/datainit/ directory *.sql auto INSERT
 */
#pragma once

#include "dbtypes.hpp"
#include "dbscheme_check.hpp"// parse_any_ddl, to_dbtypes_dbtype
#include "parse_ini.h"
#include "orm_common.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <termios.h>
#include <unordered_map>
#include <unordered_set>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

namespace dbcreatedb
{

// ========== 颜色 ==========
#define C_RED "\033[1m\033[31m"
#define C_GREEN "\033[1m\033[32m"
#define C_YELLOW "\033[1m\033[33m"
#define C_CYAN "\033[1m\033[36m"
#define C_RESET "\033[0m"

// ========== 工具函数 ==========

inline std::string trim(const std::string &s)
{
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a])))
        a++;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1])))
        b--;
    return s.substr(a, b - a);
}

inline std::string getpass(const char *prompt)
{
    std::cout << prompt;
    std::cout.flush();

    // stdin 非 TTY（重定向/管道）时 tcgetattr 会失败，此时不做回显屏蔽，
    // 避免使用未初始化 termios 结构
    struct termios old{};
    bool have_tty = (tcgetattr(STDIN_FILENO, &old) == 0);
    if (have_tty)
    {
        struct termios nw = old;
        nw.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &nw);
    }

    std::string pw;
    std::getline(std::cin, pw);

    if (have_tty)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        std::cout << std::endl;
    }
    return pw;
}

inline bool is_safe_identifier(const std::string &s)
{
    if (s.empty())
        return false;
    for (char c : s)
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
            return false;
    return true;
}

// ---------- SQL 字面量 / 标识符转义 ----------
// conf 中的 user/password/dbname 属可信源，但仍可能含引号或反斜杠，
// 直接拼接会破坏语句甚至造成注入，这里统一转义后再拼。

// MySQL 字符串字面量：默认模式（NO_BACKSLASH_ESCAPES 关闭）下 '\' 也是转义符
inline std::string mysql_string_literal(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 2);
    out += '\'';
    for (char c : s)
    {
        if (c == '\\' || c == '\'')
            out += '\\';
        out += c;
    }
    out += '\'';
    return out;
}

// MySQL 标识符：反引号包裹，内部反引号翻倍
inline std::string mysql_ident(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 2);
    out += '`';
    for (char c : s)
    {
        if (c == '`')
            out += '`';
        out += c;
    }
    out += '`';
    return out;
}

// PostgreSQL 字符串字面量：standard_conforming_strings=on 时反斜杠为普通字符，仅单引号翻倍
inline std::string pg_string_literal(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 2);
    out += '\'';
    for (char c : s)
    {
        if (c == '\'')
            out += '\'';
        out += c;
    }
    out += '\'';
    return out;
}

// PostgreSQL 标识符：双引号包裹，内部双引号翻倍
inline std::string pg_ident(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 2);
    out += '"';
    for (char c : s)
    {
        if (c == '"')
            out += '"';
        out += c;
    }
    out += '"';
    return out;
}

// =============================================================
// SQL 注释剥离 + 分号拆分
// =============================================================

inline std::string strip_sql_comments(const std::string &sql)
{
    std::string out;
    out.reserve(sql.size());
    char in_quote = 0;// 0=' " `
    for (size_t i = 0; i < sql.size(); i++)
    {
        char c = sql[i];
        if (in_quote)
        {
            out += c;
            if (c == '\\' && i + 1 < sql.size())
            {
                out += sql[++i];
                continue;
            }
            if (c == in_quote)
                in_quote = 0;
            continue;
        }
        if (c == '\'' || c == '"' || c == '`')
        {
            in_quote = c;
            out += c;
            continue;
        }
        if (c == '-' && i + 1 < sql.size() && sql[i + 1] == '-')
        {
            // -- 行注释: 跳到行尾
            while (i < sql.size() && sql[i] != '\n')
                i++;
            out += '\n';
            continue;
        }
        if (c == '/' && i + 1 < sql.size() && sql[i + 1] == '*')
        {
            // /* 块注释 */
            i += 2;
            while (i + 1 < sql.size() && !(sql[i] == '*' && sql[i + 1] == '/'))
                i++;
            if (i + 1 < sql.size())
                i++;// 跳过 '/'
            continue;
        }
        out += c;
    }
    return out;
}

inline std::vector<std::string> split_sql_by_semicolon(const std::string &sql)
{
    std::vector<std::string> result;
    std::string cleaned = strip_sql_comments(sql);
    std::string stmt;
    char in_quote = 0;
    for (size_t i = 0; i < cleaned.size(); i++)
    {
        char c = cleaned[i];
        if (in_quote)
        {
            stmt += c;
            if (c == '\\' && i + 1 < cleaned.size())
            {
                stmt += cleaned[++i];
                continue;
            }
            if (c == in_quote)
                in_quote = 0;
        }
        else
        {
            if (c == '\'' || c == '"' || c == '`')
            {
                in_quote = c;
                stmt += c;
            }
            else if (c == ';')
            {
                std::string t = trim(stmt);
                if (!t.empty())
                    result.push_back(t);
                stmt.clear();
            }
            else
                stmt += c;
        }
    }
    std::string last = trim(stmt);
    if (!last.empty())
        result.push_back(last);
    return result;
}

// =============================================================
// FK 拓扑排序 (Kahn's 算法)
// =============================================================

struct topo_result
{
    std::vector<std::string> ordered;     // 父→子顺序
    std::vector<std::string> missing_refs;// FK 引用不存在的表
    bool has_cycle = false;
};

inline topo_result topo_sort_fk(
    const std::vector<std::string> &table_names,
    const std::unordered_map<std::string, std::vector<dbtypes::db_foreign_key_info>> &fk_map)
{
    topo_result result;
    std::unordered_map<std::string, int> indegree;
    std::unordered_map<std::string, std::vector<std::string>> children;
    std::unordered_set<std::string> nodes(table_names.begin(), table_names.end());

    for (auto &t : table_names)
        indegree[t] = 0;

    for (auto &[child, fks] : fk_map)
    {
        for (auto &fk : fks)
        {
            auto &parent = fk.ref_table;
            // 自引用外键 (如 parentid → 同表) 不构成建表顺序约束，
            // 否则会因自身 indegree 永不归零而被误判为循环依赖
            if (parent == child)
                continue;
            if (nodes.find(parent) == nodes.end())
            {
                result.missing_refs.push_back(parent + " <- " + child);
                continue;
            }
            children[parent].push_back(child);
            indegree[child]++;
        }
    }

    std::queue<std::string> q;
    for (auto &[name, deg] : indegree)
        if (deg == 0)
            q.push(name);

    while (!q.empty())
    {
        auto t = q.front();
        q.pop();
        result.ordered.push_back(t);
        for (auto &c : children[t])
            if (--indegree[c] == 0)
                q.push(c);
    }

    if (result.ordered.size() != table_names.size())
    {
        result.has_cycle = true;
        std::vector<std::string> remaining;
        for (auto &[name, deg] : indegree)
            if (deg > 0)
                remaining.push_back(name);
        std::sort(remaining.begin(), remaining.end());
        for (auto &t : remaining)
            result.ordered.push_back(t);
    }
    return result;
}

// =============================================================
// 从 orm.conf 取 main 组配置
// =============================================================

inline bool load_dbtag_config(const std::string &dbtag, orm::orm_conn_t &out)
{
    std::vector<orm::orm_conn_t> cfgs = orm::get_orm_config_file("conf/orm.conf");
    if (cfgs.empty())
        return false;
    for (auto &c : cfgs)
    {
        if (c.link_type == 0 && c.tag == dbtag)
        {
            out = c;
            return true;
        }
    }
    return false;
}

inline void print_available_tags()
{
    std::vector<orm::orm_conn_t> cfgs = orm::get_orm_config_file("conf/orm.conf");
    std::cout << "  Available tags: ";
    bool first = true;
    for (auto &c : cfgs)
    {
        if (c.link_type == 0)
        {
            if (!first)
                std::cout << ", ";
            std::cout << C_CYAN << c.tag << C_RESET;
            first = false;
        }
    }
    std::cout << std::endl;
}

// =============================================================
// 执行单条 SQL (成功=true)
// =============================================================

struct exec_stats
{
    int ok   = 0;
    int fail = 0;
};

inline exec_stats exec_sql_list(
    const dbtypes::db_conn_variant &conn,
    dbtypes::DB_TYPE type,
    const std::vector<std::string> &stmts,
    std::string &last_error)
{
    exec_stats s;
    for (auto &sql : stmts)
    {
        std::string err;
        if (dbtypes::exec_ddl(conn, type, sql, err))
        {
            s.ok++;
        }
        else
        {
            s.fail++;
            if (!err.empty())
                last_error = err;
        }
    }
    return s;
}

// =============================================================
// MySQL: CREATE DATABASE + CREATE USER + GRANT
// =============================================================

inline int createdb_mysql(
    const std::string &host, const std::string &port, const std::string &admin_user, const std::string &admin_pwd, const std::string &dbname, const std::string &app_user, const std::string &app_pwd)
{
    asio::io_context ioc;

    orm::orm_conn_t admin_cfg;
    admin_cfg.host     = host;
    admin_cfg.port     = port.empty() ? "3306" : port;
    admin_cfg.user     = admin_user;
    admin_cfg.password = admin_pwd;
    admin_cfg.dbname   = "mysql";
    admin_cfg.db_type  = orm::DB_TYPE::MYSQL;

    auto conn = dbtypes::create_connection(admin_cfg, dbtypes::DB_TYPE::MYSQL, ioc);
    if (!conn.ok())
    {
        std::cerr << C_RED << "[FAIL]" << C_RESET << " MySQL connect failed: " << conn.error_msg << std::endl;
        return 1;
    }
    std::cout << C_GREEN << "[OK  ]" << C_RESET << " Connected to MySQL " << host << ":" << admin_cfg.port
              << " (admin: " << admin_user << ")" << std::endl;

    auto mysql = std::get<std::shared_ptr<orm::mysql_conn_base>>(conn.conn);

    // --- 1. CREATE DATABASE ---
    // 不用 IF NOT EXISTS：让"已存在"返回错误码 1007，从而与真正失败区分开
    std::string create_db_sql =
        "CREATE DATABASE " + mysql_ident(dbname) +
        " DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci";
    unsigned int rc = mysql->exec_dml(create_db_sql);
    if (rc == (unsigned int)-1)
    {
        // 1007 = DB already exists
        if (mysql->error_code == 1007)
        {
            std::cout << C_YELLOW << "[SKIP]" << C_RESET
                      << " Database [" << dbname << "] already exists" << std::endl;
        }
        else
        {
            std::cerr << C_RED << "[FAIL]" << C_RESET
                      << " CREATE DATABASE failed: " << mysql->error_msg << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << C_GREEN << "[OK  ]" << C_RESET << " Database [" << dbname << "] ready" << std::endl;
    }

    // --- 2. 检查用户 ---
    bool user_exists      = false;
    std::string check_sql = "SELECT COUNT(*) FROM mysql.user WHERE user=" +
                            mysql_string_literal(app_user) + " AND host='%'";
    mysql->fetch_directly(check_sql,
                          [&](int cc, char **cn, std::function<std::tuple<unsigned char *, size_t>(int)> gd) -> bool
                          {
                              (void)cc;
                              (void)cn;
                              auto [ptr, len] = gd(0);
                              std::string s(reinterpret_cast<const char *>(ptr), len);
                              user_exists = (s != "0");
                              return false;// 只需第一行，返回 false 后由实现内部把结果集读完
                          });

    if (user_exists)
    {
        std::cout << C_YELLOW << "[SKIP]" << C_RESET
                  << " User [" << app_user << "@%] already exists, skipping create" << std::endl;
    }
    else
    {
        // --- 3. CREATE USER ---
        std::string create_user_sql =
            "CREATE USER " + mysql_string_literal(app_user) + "@'%' IDENTIFIED BY " +
            mysql_string_literal(app_pwd);
        rc = mysql->exec_dml(create_user_sql);
        if (rc == (unsigned int)-1)
        {
            std::cerr << C_RED << "[FAIL]" << C_RESET
                      << " CREATE USER failed: " << mysql->error_msg << std::endl;
            return 1;
        }
        std::cout << C_GREEN << "[OK  ]" << C_RESET
                  << " User [" << app_user << "@%] created" << std::endl;
    }

    // --- 4. GRANT ---
    std::string grant_sql =
        "GRANT ALL PRIVILEGES ON " + mysql_ident(dbname) + ".* TO " +
        mysql_string_literal(app_user) + "@'%'";
    rc = mysql->exec_dml(grant_sql);
    if (rc == (unsigned int)-1)
    {
        std::cerr << C_RED << "[FAIL]" << C_RESET
                  << " GRANT failed: " << mysql->error_msg << std::endl;
        return 1;
    }
    std::cout << C_GREEN << "[OK  ]" << C_RESET << " Privileges granted" << std::endl;

    // --- 5. FLUSH ---
    mysql->exec_dml("FLUSH PRIVILEGES");

    return 0;
}

// =============================================================
// PostgreSQL: CREATE DATABASE + CREATE ROLE + GRANT
// =============================================================

inline int createdb_pg(
    const std::string &host, const std::string &port, const std::string &admin_user, const std::string &admin_pwd, const std::string &dbname, const std::string &app_user, const std::string &app_pwd)
{
    asio::io_context ioc;

    orm::orm_conn_t admin_cfg;
    admin_cfg.host     = host;
    admin_cfg.port     = port.empty() ? "5432" : port;
    admin_cfg.user     = admin_user;
    admin_cfg.password = admin_pwd;
    admin_cfg.dbname   = "postgres";
    admin_cfg.db_type  = orm::DB_TYPE::POSTGRESQL;

    auto conn = dbtypes::create_connection(admin_cfg, dbtypes::DB_TYPE::POSTGRESQL, ioc);
    if (!conn.ok())
    {
        std::cerr << C_RED << "[FAIL]" << C_RESET << " PostgreSQL connect failed: " << conn.error_msg << std::endl;
        return 1;
    }
    std::cout << C_GREEN << "[OK  ]" << C_RESET << " Connected to PostgreSQL " << host << ":" << admin_cfg.port
              << " (admin: " << admin_user << ")" << std::endl;

    auto pg = std::get<std::shared_ptr<orm::pg_conn_base>>(conn.conn);

    // 执行一条语句；失败(返回非 0 或 error_msg 非空)时打印并返回 false
    auto pg_exec = [&](const std::string &sql, const std::string &what) -> bool
    {
        std::vector<orm::field_info_t> f;
        std::vector<orm::pg_row_data_t> r;
        unsigned int a   = 0;
        unsigned int ret = pg->execute_and_fetch(sql, f, r, a);
        if (ret != 0 || !pg->error_msg.empty())
        {
            std::cerr << C_RED << "[FAIL]" << C_RESET << " " << what
                      << " failed: " << pg->error_msg << std::endl;
            return false;
        }
        return true;
    };

    // 查询是否有结果行 (用于存在性判断)
    auto pg_exists = [&](const std::string &sql) -> bool
    {
        std::vector<orm::field_info_t> f;
        std::vector<orm::pg_row_data_t> r;
        unsigned int a = 0;
        pg->execute_and_fetch(sql, f, r, a);
        return !r.empty();
    };

    // --- 1. 检查 DB + CREATE DATABASE ---
    bool db_exists = pg_exists("SELECT 1 FROM pg_database WHERE datname = " +
                               pg_string_literal(dbname));

    if (db_exists)
    {
        std::cout << C_YELLOW << "[SKIP]" << C_RESET
                  << " Database [" << dbname << "] already exists" << std::endl;
    }
    else
    {
        if (!pg_exec("CREATE DATABASE " + pg_ident(dbname) + " ENCODING 'UTF8'", "CREATE DATABASE"))
            return 1;
        std::cout << C_GREEN << "[OK  ]" << C_RESET << " Database [" << dbname << "] created" << std::endl;
    }

    // --- 2. 检查用户 + CREATE ROLE ---
    bool user_exists = pg_exists("SELECT 1 FROM pg_roles WHERE rolname = " +
                                 pg_string_literal(app_user));

    if (user_exists)
    {
        std::cout << C_YELLOW << "[SKIP]" << C_RESET
                  << " Role [" << app_user << "] already exists, skipping create" << std::endl;
    }
    else
    {
        if (!pg_exec("CREATE ROLE " + pg_ident(app_user) + " LOGIN PASSWORD " +
                         pg_string_literal(app_pwd),
                     "CREATE ROLE"))
            return 1;
        std::cout << C_GREEN << "[OK  ]" << C_RESET
                  << " Role [" << app_user << "] created" << std::endl;
    }

    // --- 3. GRANT DB 级权限 ---
    if (!pg_exec("GRANT ALL PRIVILEGES ON DATABASE " + pg_ident(dbname) + " TO " +
                     pg_ident(app_user),
                 "GRANT"))
        return 1;
    std::cout << C_GREEN << "[OK  ]" << C_RESET << " Privileges granted on database" << std::endl;

    // --- 4. 让 app 用户成为库 owner ---
    // PG 15+ 起 public schema 的 CREATE 权限只归库 owner (pg_database_owner)，
    // 库级 GRANT 不含 schema 权限；不转移 owner 的话，导入时 app 用户
    // 执行 CREATE TABLE 会报 "permission denied for schema public"
    if (app_user != admin_user)
    {
        if (!pg_exec("ALTER DATABASE " + pg_ident(dbname) + " OWNER TO " + pg_ident(app_user),
                     "ALTER DATABASE OWNER"))
            return 1;
        std::cout << C_GREEN << "[OK  ]" << C_RESET
                  << " Database owner set to [" << app_user << "]" << std::endl;
    }

    return 0;
}

// =============================================================
// SQLite: 仅创建文件
// =============================================================

inline int createdb_sqlite(const std::string &dbfile)
{
    asio::io_context ioc;

    orm::orm_conn_t cfg;
    cfg.host    = dbfile;
    cfg.dbname  = "lite";
    cfg.db_type = orm::DB_TYPE::SQLITE;

    auto conn = dbtypes::create_connection(cfg, dbtypes::DB_TYPE::SQLITE, ioc);
    if (!conn.ok())
    {
        std::cerr << C_RED << "[FAIL]" << C_RESET
                  << " SQLite open failed: " << conn.error_msg << std::endl;
        return 1;
    }
    std::cout << C_GREEN << "[OK  ]" << C_RESET
              << " SQLite database ready: " << dbfile << std::endl;
    return 0;
}

// =============================================================
// import_schema —  扫目录+排序, 执行 DDL+datainit, 汇总
// =============================================================

struct table_entry
{
    std::string table_name;
    std::string filename;
    std::string raw_sql;
    std::string data_sql;// "" 表示没有 datainit
};

inline int import_schema(
    const std::string &dbtag,
    dbtypes::DB_TYPE dbtype,
    const orm::orm_conn_t &conn_cfg)
{
    // ---- 连接目标库 ----
    asio::io_context ioc;
    auto conn_res = dbtypes::create_connection(conn_cfg, dbtype, ioc);
    if (!conn_res.ok())
    {
        std::cerr << C_RED << "[FAIL]" << C_RESET
                  << " App user connect failed: " << conn_res.error_msg << std::endl;
        return 1;
    }

    // ---- DDL 目录 ----
    std::string ddl_dir  = "schema/" + dbtag + "/tables/";
    std::string data_dir = "schema/" + dbtag + "/datainit/";

    if (!fs::exists(ddl_dir))
    {
        std::cout << C_YELLOW << "[WARN]" << C_RESET
                  << " schema/" << dbtag << "/tables/ not exist, skipping schema import" << std::endl;
        return 0;
    }

    auto dbtype_display = dbtypes::db_type_display(dbtype);

    // 全量解析 + 拓扑排序
    std::vector<table_entry> entries;
    std::unordered_map<std::string, std::vector<dbtypes::db_foreign_key_info>> fk_map;

    std::vector<fs::path> sql_files;
    for (auto &e : fs::directory_iterator(ddl_dir))
        if (e.path().extension() == ".sql")
            sql_files.push_back(e.path());
    std::sort(sql_files.begin(), sql_files.end());

    int parse_fail = 0;

    for (auto &path : sql_files)
    {
        std::ifstream f(path);
        std::stringstream ss;
        ss << f.rdbuf();
        std::string raw_sql = ss.str();

        dbtypes::db_table_info info;
        bool ok = dbscheme::parse_any_ddl(raw_sql, dbtype, info);
        if (!ok || info.table_name.empty())
        {
            std::cout << C_RED << "[FAIL]" << C_RESET
                      << " DDL parse fail: " << path.filename().string() << std::endl;
            parse_fail++;
            continue;
        }

        table_entry entry;
        entry.table_name = info.table_name;
        entry.filename   = path.filename().string();
        entry.raw_sql    = raw_sql;

        // datainit 匹配同名 .sql
        fs::path data_file = fs::path(data_dir) / (info.table_name + ".sql");
        if (fs::exists(data_file))
        {
            std::ifstream df(data_file);
            std::stringstream dss;
            dss << df.rdbuf();
            entry.data_sql = dss.str();
        }

        fk_map[info.table_name] = info.foreign_keys;
        entries.push_back(std::move(entry));
    }

    if (entries.empty())
    {
        std::cout << C_YELLOW << "[WARN]" << C_RESET << " No tables can be imported (parse failed completely)" << std::endl;
        return parse_fail > 0 ? 1 : 0;
    }

    // 拓扑排序
    std::vector<std::string> table_names;
    table_names.reserve(entries.size());
    for (auto &e : entries)
        table_names.push_back(e.table_name);

    auto topo = topo_sort_fk(table_names, fk_map);

    if (topo.has_cycle)
    {
        std::cout << C_YELLOW << "[WARN]" << C_RESET
                  << " FK circular dependency detected, fallback sorted by name" << std::endl;
    }
    for (auto &ref : topo.missing_refs)
    {
        std::cout << C_YELLOW << "[WARN]" << C_RESET
                  << " FK Reference to a non-existent parent table: " << ref << std::endl;
    }

    // 按 topo.ordered 重排 entries
    std::unordered_map<std::string, size_t> name_to_idx;
    for (size_t i = 0; i < entries.size(); i++)
        name_to_idx[entries[i].table_name] = i;

    std::vector<table_entry> ordered_entries;
    ordered_entries.reserve(entries.size());
    for (auto &name : topo.ordered)
        ordered_entries.push_back(std::move(entries[name_to_idx[name]]));
    entries = std::move(ordered_entries);

    // 按序执行

    std::cout << "\n===== Schema Import (" << dbtype_display
              << ", " << entries.size() << " tables, FK-sorted) =====" << std::endl;

    int ddl_ok_count    = 0;
    int ddl_fail_count  = 0;
    int data_with       = 0;
    int data_skip       = 0;
    int data_fail_count = 0;
    int total_data_ok   = 0;
    int total_data_fail = 0;

    for (auto &entry : entries)
    {
        std::cout << "  Creating table [" << entry.table_name << "]..." << std::endl;

        // 执行 DDL
        auto stmts = split_sql_by_semicolon(entry.raw_sql);
        std::string last_err;
        auto ddl_stats = exec_sql_list(conn_res.conn, dbtype, stmts, last_err);

        if (ddl_stats.fail > 0)
        {
            ddl_fail_count++;
            std::cout << "  " << C_RED << "[FAIL]" << C_RESET
                      << " " << entry.table_name
                      << " — DDL: " << ddl_stats.ok << " ok, " << ddl_stats.fail << " fail";
            if (!last_err.empty())
                std::cout << "  (" << last_err << ")";
            std::cout << std::endl;
            std::cout << "  " << C_YELLOW << "[SKIP]" << C_RESET << " datainit (DDL failed)" << std::endl;
            continue;
        }
        ddl_ok_count++;

        // DDL 成功 → 处理 datainit
        if (entry.data_sql.empty())
        {
            std::cout << "  " << C_GREEN << "[OK ]" << C_RESET
                      << " " << entry.table_name
                      << " — DDL: " << ddl_stats.ok << " ok, 0 fail | no datainit" << std::endl;
            data_skip++;
            continue;
        }

        data_with++;
        auto data_stmts = split_sql_by_semicolon(entry.data_sql);
        std::string data_err;
        auto data_stats = exec_sql_list(conn_res.conn, dbtype, data_stmts, data_err);

        total_data_ok += data_stats.ok;
        total_data_fail += data_stats.fail;

        if (data_stats.fail == 0)
        {
            std::cout << "  " << C_GREEN << "[OK ]" << C_RESET
                      << " " << entry.table_name
                      << " — DDL: " << ddl_stats.ok << " ok, 0 fail | DATA: "
                      << data_stats.ok << " ok, 0 fail" << std::endl;
        }
        else
        {
            data_fail_count++;
            std::cout << "  " << C_YELLOW << "[WARN]" << C_RESET
                      << " " << entry.table_name
                      << " — DDL: " << ddl_stats.ok << " ok, 0 fail | DATA: "
                      << data_stats.ok << " ok, " << data_stats.fail << " fail";
            if (!data_err.empty())
                std::cout << "  (" << data_err << ")";
            std::cout << std::endl;
        }
    }

    // 汇总
    std::cout << "\n===== Schema Import Summary =====" << std::endl;
    std::cout << "  Total: " << entries.size() << " tables"
              << "  |  DDL OK: " << C_GREEN << ddl_ok_count << C_RESET
              << "  FAIL: " << (ddl_fail_count > 0 ? C_RED : C_GREEN) << ddl_fail_count << C_RESET
              << "  |  DATA: " << data_with << " with, " << data_skip << " skip"
              << " (" << total_data_ok << " ok, " << total_data_fail << " fail)";
    if (data_fail_count > 0)
        std::cout << "  (" << C_YELLOW << data_fail_count << C_RESET << " tables have data errors)";
    std::cout << std::endl;
    std::cout << "  Parse errors: " << parse_fail
              << "  |  FK warnings: " << topo.missing_refs.size()
              << "  |  Cycle detected: ";
    if (topo.has_cycle)
        std::cout << C_YELLOW << "YES" << C_RESET << std::endl;
    else
        std::cout << C_GREEN << "NO" << C_RESET << std::endl;

    int hard_fail = ddl_fail_count + parse_fail;
    return hard_fail > 0 ? 1 : 0;
}

// =============================================================
// CLI Main Entrance
// =============================================================

inline int createdb_cli(const std::string &dbtag, const std::string &admin_user)
{
    // ---- 安全校验 ----
    if (!dbtypes::is_safe_dbtag(dbtag))
        return 1;
    if (!admin_user.empty() && !is_safe_identifier(admin_user))
    {
        std::cerr << C_RED << "[ERROR]" << C_RESET
                  << " admin_user only allows [a-zA-Z0-9_]" << std::endl;
        return 1;
    }

    // ---- 加载配置 ----
    orm::orm_conn_t cfg;
    if (!load_dbtag_config(dbtag, cfg))
    {
        std::cerr << C_RED << "[ERROR]" << C_RESET
                  << " Tag [" << dbtag << "] not found in conf/orm.conf" << std::endl;
        print_available_tags();
        return 1;
    }

    // 先校验 dbtype 字符串，再按解析出的枚举统一分派 (支持 pg/postgres/sqlite3 等别名)
    if (!dbtypes::is_valid_target_type_str(cfg.dbtype))
    {
        std::cerr << C_RED << "[ERROR]" << C_RESET
                  << " Unknown dbtype: " << cfg.dbtype << std::endl;
        return 1;
    }
    auto dbtype = dbtypes::parse_target_type(cfg.dbtype);

    std::cout << C_CYAN << "\n===== Database Setup =====" << C_RESET << std::endl;
    std::cout << "  Tag:      [" << C_CYAN << dbtag << C_RESET << "]" << std::endl;
    std::cout << "  Type:     " << dbtypes::db_type_display(dbtype) << std::endl;
    if (dbtype == dbtypes::DB_TYPE::SQLITE)
    {
        std::cout << "  DB File:  " << cfg.host << std::endl;
    }
    else
    {
        std::cout << "  Host:     " << cfg.host;
        if (!cfg.port.empty())
            std::cout << ":" << cfg.port;
        std::cout << std::endl;
        std::cout << "  Database: " << cfg.dbname << std::endl;
        std::cout << "  App User: " << (cfg.user.empty() ? "(none)" : cfg.user) << std::endl;
    }

    int rc = 0;

    if (dbtype == dbtypes::DB_TYPE::SQLITE)
    {
        if (cfg.host.empty())
        {
            std::cerr << C_RED << "[ERROR]" << C_RESET
                      << " SQLite configuration is missing the host (should be the database file path)" << std::endl;
            return 1;
        }
        rc = createdb_sqlite(cfg.host);
    }
    else
    {
        if (admin_user.empty())
        {
            std::cerr << C_RED << "[ERROR]" << C_RESET
                      << " MySQL/PostgreSQL need -u <admin_user> (Users with CREATE DATABASE permission)" << std::endl;
            return 1;
        }

        // app 身份
        std::string app_user = cfg.user;
        std::string app_pwd  = cfg.password;

        if (dbtype == dbtypes::DB_TYPE::MYSQL)
        {
            rc = createdb_mysql(cfg.host, cfg.port, admin_user, getpass("  Enter admin password: "), cfg.dbname, app_user, app_pwd);
        }
        else
        {
            rc = createdb_pg(cfg.host, cfg.port, admin_user, getpass("  Enter admin password: "), cfg.dbname, app_user, app_pwd);
        }
    }

    if (rc != 0)
        return rc;

    // ---- 交互式询问导入 schema ----
    std::cout << "\nImport schema tables? [y/N]: ";
    std::string ans;
    std::getline(std::cin, ans);
    if (ans != "y" && ans != "Y")
    {
        std::cout << C_GREEN << "Done." << C_RESET << std::endl;
        return 0;
    }

    // 用 app user 连目标库
    orm::orm_conn_t app_cfg = cfg;
    // SQLite 没有 user/password 概念，但 host 是文件路径
    // MySQL/PG 已有正确的 dbname/user/password/host/port

    rc = import_schema(dbtag, dbtype, app_cfg);

    std::cout << "\n===== Done! =====" << std::endl;
    return rc;
}

}// namespace dbcreatedb
