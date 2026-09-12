// ============================================================
// dbscheme_check.hpp  —  schema/<tag>/tables/*.sql 管理工具
//
// 功能:
//   1. load_schema_fallback()  — DB 不可用时从 SQL 文件加载表结构
//   2. schema_check_tag()      — 校验单个 tag 的 SQL 文件完整性
//   3. schema_check_all()      — 校验所有已存在的 tag
//   4. schema_regen_sql()      — 用 dbtypes::gen_ddl() 重写规范化 SQL
//
// 依赖: dbtypes.hpp (parse_pg_ddl / parse_sqlite_ddl / parse_mysql_show_create / gen_ddl)
// ============================================================
#pragma once

#include "dbtypes.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>

namespace fs = std::filesystem;

namespace dbscheme
{

// ============================================================
// 内部: 按 dbtype 选择对应的 DDL 解析器
// ============================================================
inline bool parse_any_ddl(const std::string &sql, dbtypes::DB_TYPE db_type, dbtypes::db_table_info &out)
{
    switch (db_type)
    {
    case dbtypes::DB_TYPE::MYSQL:
        return dbtypes::parse_mysql_show_create(sql, out);
    case dbtypes::DB_TYPE::POSTGRESQL:
        return dbtypes::parse_pg_ddl(sql, out);
    case dbtypes::DB_TYPE::SQLITE:
        return dbtypes::parse_sqlite_ddl(sql, out);
    }
    return false;
}

inline dbtypes::DB_TYPE to_dbtypes_dbtype(DBType t)
{
    switch (t)
    {
    case DBType::POSTGRESQL: return dbtypes::DB_TYPE::POSTGRESQL;
    case DBType::SQLITE: return dbtypes::DB_TYPE::SQLITE;
    default: return dbtypes::DB_TYPE::MYSQL;
    }
}

// ============================================================
// load_schema_fallback
//   DB 不可用时遍历 schema/<tag>/tables/*.sql，返回 table_lists 和按表分组的列信息
//   与 modelfun.hpp 里的版本逻辑相同，但放在这里方便复用
// ============================================================
inline bool load_schema_fallback(const std::string &tag, DBType db_type, std::vector<std::string> &out_table_lists, std::map<std::string, std::vector<dbtypes::db_field_info>> &out_cols_map)
{
    std::string schema_dir = "./schema/" + tag + "/tables/";
    if (!fs::exists(schema_dir))
    {
        std::cerr << "  Schema dir not found: " << schema_dir << std::endl;
        return false;
    }

    dbtypes::DB_TYPE sql_dbtype = to_dbtypes_dbtype(db_type);

    int ok = 0, fail = 0;
    for (auto &entry : fs::directory_iterator(schema_dir))
    {
        if (entry.path().extension() != ".sql")
            continue;

        std::ifstream f(entry.path());
        std::stringstream ss;
        ss << f.rdbuf();
        std::string sql = ss.str();

        dbtypes::db_table_info info;
        if (!parse_any_ddl(sql, sql_dbtype, info) || info.table_name.empty())
        {
            std::cerr << "  [WARN] parse fail: " << entry.path().filename().string() << std::endl;
            fail++;
            continue;
        }
        ok++;
        out_table_lists.push_back(info.table_name);
        out_cols_map[info.table_name] = info.fields;
    }
    std::cout << "  Schema fallback: " << ok << "/" << (ok + fail) << " tables from " << schema_dir << std::endl;
    return ok > 0;
}

// ============================================================
// schema-check: 单个 tag
//   返回值: 0 = 全部通过, 1 = 有 WARN, 2 = 有 FAIL
// ============================================================
struct check_report
{
    std::string filename;
    std::string table_name;
    int field_count = 0;
    int warn_count  = 0;
    int fail_count  = 0;
    std::vector<std::string> warnings;
    std::vector<std::string> failures;
};

inline int schema_check_tag(const std::string &tag, DBType db_type)
{
    std::string schema_dir = "./schema/" + tag + "/tables/";

    if (!fs::exists(schema_dir))
    {
        std::cout << " \033[1m\033[33m[SKIP]\033[0m schema/" << tag << "/tables/ does not exist" << std::endl;
        return 0;
    }

    dbtypes::DB_TYPE sql_dbtype = to_dbtypes_dbtype(db_type);

    std::vector<check_report> reports;
    int total_ok   = 0;
    int total_warn = 0;
    int total_fail = 0;

    // 收集所有 .sql 文件
    std::vector<fs::path> sql_files;
    for (auto &entry : fs::directory_iterator(schema_dir))
    {
        if (entry.path().extension() == ".sql")
            sql_files.push_back(entry.path());
    }

    if (sql_files.empty())
    {
        std::cout << " \033[1m\033[33m[WARN]\033[0m schema/" << tag << "/tables/ The directory is empty" << std::endl;
        return 1;
    }

    // 先打印 header
    std::cout << "\n\033[1m===== schema-check: " << tag << " ("
              << (sql_dbtype == dbtypes::DB_TYPE::MYSQL ? "MySQL" : sql_dbtype == dbtypes::DB_TYPE::POSTGRESQL ? "PostgreSQL" :
                                                                                                                 "SQLite")
              << ", " << sql_files.size() << " files) =====\033[0m" << std::endl;

    for (auto &path : sql_files)
    {
        check_report rep;
        rep.filename = path.filename().string();

        // 读文件
        std::ifstream fs(path);
        std::stringstream ss;
        ss << fs.rdbuf();
        std::string sql = ss.str();

        // 解析
        dbtypes::db_table_info info;
        bool parsed    = parse_any_ddl(sql, sql_dbtype, info);
        rep.table_name = info.table_name;

        if (!parsed || info.table_name.empty())
        {
            rep.fail_count++;
            rep.failures.push_back("DDL Parsing failed");
            reports.push_back(rep);
            total_fail++;
            continue;
        }

        rep.field_count = (int)info.fields.size();

        // === 检查项 ===

        // 1) 文件名 vs 表名
        std::string file_base = path.stem().string();
        if (file_base != info.table_name)
        {
            rep.warn_count++;
            rep.warnings.push_back("Filename [" + file_base + "] and table name [" + info.table_name + "] discrepancy");
        }

        // 2) 空表
        if (info.fields.empty())
        {
            rep.fail_count++;
            rep.failures.push_back("No field definitions");
        }

        // 3) 表没有 PK（允许是 WARN，因为有些表确实没 PK）
        bool has_pk = false;
        for (auto &f : info.fields)
            if (f.is_pk)
            {
                has_pk = true;
                break;
            }
        if (!has_pk)
        {
            rep.warn_count++;
            rep.warnings.push_back("Undefined PRIMARY KEY");
        }

        // 4) 字段级检查
        for (auto &f : info.fields)
        {
            // 类型为空
            if (f.field_type.empty())
            {
                rep.fail_count++;
                rep.failures.push_back("field [" + f.field_name + "] type is empty");
            }
            // mysql_type 未映射
            if (f.mysql_type == 0)
            {
                rep.warn_count++;
                rep.warnings.push_back("field [" + f.field_name + "] type [" + f.field_type + "] not mapped mysql_type");
            }
        }

        if (rep.fail_count > 0)
            total_fail++;
        else if (rep.warn_count > 0)
            total_warn++;
        else
            total_ok++;

        reports.push_back(rep);
    }

    // === 输出 ===
    for (auto &r : reports)
    {
        if (r.fail_count > 0)
        {
            std::cout << " \033[1m\033[31m[FAIL]\033[0m " << r.filename
                      << "  (" << r.table_name << ", " << r.field_count << " cols)" << std::endl;
            for (auto &w : r.warnings)
                std::cout << "         \033[33mWARN:  " << w << "\033[0m" << std::endl;
            for (auto &f : r.failures)
                std::cout << "         \033[31mFAIL:  " << f << "\033[0m" << std::endl;
        }
        else if (r.warn_count > 0)
        {
            std::cout << " \033[1m\033[33m[WARN]\033[0m " << r.filename
                      << "  (" << r.table_name << ", " << r.field_count << " cols)" << std::endl;
            for (auto &w : r.warnings)
                std::cout << "         \033[33mWARN:  " << w << "\033[0m" << std::endl;
        }
        else
        {
            std::cout << " \033[1m\033[32m[ OK ]\033[0m " << r.filename
                      << "  (" << r.table_name << ", " << r.field_count << " cols)" << std::endl;
        }
    }

    std::cout << " -----" << std::endl;
    std::cout << " \033[32mOK: " << total_ok << "\033[0m   "
              << "\033[33mWARN: " << total_warn << "\033[0m   "
              << "\033[31mFAIL: " << total_fail << "\033[0m" << std::endl;

    if (total_fail > 0)
        return 2;
    if (total_warn > 0)
        return 1;
    return 0;
}

// ============================================================
// schema-check: 所有 tag（自动扫描 schema/*/tables/）
// ============================================================
inline int schema_check_all()
{
    if (!fs::exists("schema"))
    {
        std::cout << "schema/ Directory does not exist" << std::endl;
        return 0;
    }

    int worst = 0;
    for (auto &entry : fs::directory_iterator("schema"))
    {
        if (!entry.is_directory())
            continue;
        std::string tag = entry.path().filename().string();

        // 从 schema 里只有 tables/ 子目录才能校验
        if (!fs::exists(entry.path() / "tables"))
            continue;

        // 默认按 MySQL 解析（保守选择）；用户如需精确可单独指定
        int rc = schema_check_tag(tag, DBType::MYSQL);
        if (rc > worst)
            worst = rc;
    }
    return worst;
}

// ============================================================
// schema-regen: 用 gen_ddl() 重写某个 tag 的所有 SQL 文件
//   先 parse 再 gen_ddl，规范化格式
// ============================================================
inline int schema_regen_tag(const std::string &tag, DBType db_type, bool dry_run = false)
{
    std::string schema_dir = "./schema/" + tag + "/tables/";
    if (!fs::exists(schema_dir))
    {
        std::cerr << "schema/" << tag << "/tables/ not exist" << std::endl;
        return 1;
    }

    dbtypes::DB_TYPE sql_dbtype = to_dbtypes_dbtype(db_type);

    int ok = 0, fail = 0;
    for (auto &entry : fs::directory_iterator(schema_dir))
    {
        if (entry.path().extension() != ".sql")
            continue;

        std::ifstream f(entry.path());
        std::stringstream ss;
        ss << f.rdbuf();
        std::string sql = ss.str();

        dbtypes::db_table_info info;
        if (!parse_any_ddl(sql, sql_dbtype, info))
        {
            std::cerr << "  [FAIL] parse: " << entry.path().filename().string() << std::endl;
            fail++;
            continue;
        }

        std::string gen = dbtypes::gen_ddl(info, sql_dbtype);
        if (gen.empty())
        {
            std::cerr << "  [FAIL] gen_ddl: " << info.table_name << std::endl;
            fail++;
            continue;
        }

        if (dry_run)
        {
            std::cout << "  [DRY] " << entry.path().filename().string()
                      << " -> " << gen.size() << " bytes" << std::endl;
        }
        else
        {
            std::ofstream out(entry.path());
            out << gen;
            std::cout << "  [REGEN] " << entry.path().filename().string() << std::endl;
        }
        ok++;
    }
    std::cout << "Regen: " << ok << " ok, " << fail << " fail" << std::endl;
    return fail > 0 ? 1 : 0;
}

// =================================================================
// CLI 入口（由 paozhu_cli.cpp 调用）
// =================================================================

// 内部: 从 orm::get_orm_config_file() 构建 tag → DBType 映射
inline std::map<std::string, orm::DB_TYPE> load_tag_dbtype_map()
{
    std::map<std::string, orm::DB_TYPE> m;
    auto cfgs = orm::get_orm_config_file("conf/orm.conf");
    for (const auto &c : cfgs)
        if (m.find(c.tag) == m.end())// 只取 main 组（第一组）
            m[c.tag] = c.db_type;
    return m;
}

// paozhu_cli schema-check [dbtag]
inline int dbschemacheckcli(int argc, char *argv[])
{
    auto tag_dbtype = load_tag_dbtype_map();

    auto dbtype_to_dbscheme = [](orm::DB_TYPE t) -> DBType
    {
        return (t == orm::DB_TYPE::POSTGRESQL) ? DBType::POSTGRESQL : (t == orm::DB_TYPE::SQLITE) ? DBType::SQLITE :
                                                                                                    DBType::MYSQL;
    };

    if (argc >= 3)
    {
        std::string tag = argv[2];
        DBType dt       = DBType::MYSQL;
        auto it         = tag_dbtype.find(tag);
        if (it != tag_dbtype.end())
            dt = dbtype_to_dbscheme(it->second);
        else
            std::cout << "  Tag not found in orm.conf [" << tag << "], parse according to MySQL)" << std::endl;
        return schema_check_tag(tag, dt);
    }

    std::cout << "Tag not specified, scanning schema/*/tables/ ..." << std::endl;
    int worst = 0;
    if (fs::exists("schema"))
    {
        for (auto &e : fs::directory_iterator("schema"))
        {
            if (!e.is_directory())
                continue;
            std::string t = e.path().filename().string();
            if (!fs::exists(e.path() / "tables"))
                continue;
            DBType dt = DBType::MYSQL;
            auto it   = tag_dbtype.find(t);
            if (it != tag_dbtype.end())
                dt = dbtype_to_dbscheme(it->second);
            int rc = schema_check_tag(t, dt);
            if (rc > worst)
                worst = rc;
        }
    }
    return worst;
}

// paozhu_cli schema-regen <dbtag> [-dry-run]
inline int dbschemaregencli(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: paozhu_cli schema-regen <dbtag> [-dry-run]" << std::endl;
        std::cout << "  use dbtypes::gen_ddl() Rewrte schema/<dbtag>/tables/*.sql for standardized format" << std::endl;
        return 1;
    }

    std::string tag = argv[2];
    bool dry        = false;
    for (int i = 3; i < argc; i++)
        if (std::string(argv[i]) == "-dry-run")
            dry = true;

    auto tag_dbtype         = load_tag_dbtype_map();
    auto dbtype_to_dbscheme = [](orm::DB_TYPE t) -> DBType
    {
        return (t == orm::DB_TYPE::POSTGRESQL) ? DBType::POSTGRESQL : (t == orm::DB_TYPE::SQLITE) ? DBType::SQLITE :
                                                                                                    DBType::MYSQL;
    };
    DBType dt = DBType::MYSQL;
    auto it   = tag_dbtype.find(tag);
    if (it != tag_dbtype.end())
        dt = dbtype_to_dbscheme(it->second);
    else
        std::cout << "  (Not in orm.conf found tag [" << tag << "], parse as MySQL)" << std::endl;

    return schema_regen_tag(tag, dt, dry);
}

}// namespace dbscheme
