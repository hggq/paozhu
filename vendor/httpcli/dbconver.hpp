/*
 *  @author 黄自权 huangziquan
 *  @date 2026-08-14
 *  @dest 数据库转换迁移工具 - MySQL <-> PostgreSQL <-> SQLite
 *  Usage: paozhu_cli dbconver <dbtag1> <dbtag2>
 */

#ifndef _DBCONVER_HPP
#define _DBCONVER_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>
#include <memory>
#include <filesystem>
#include <cstring>
#include <cctype>
#include "dbtypes.hpp"

namespace fs = std::filesystem;
namespace dbconver
{

using dbtypes::build_insert_sql_mysql;
using dbtypes::build_insert_sql_pg;
using dbtypes::convert_type_for_target;
using dbtypes::create_connection;
using dbtypes::db_field_info;
using dbtypes::db_index_info;
using dbtypes::db_table_info;
using dbtypes::db_type_display;
using dbtypes::escape_mysql_string;
using dbtypes::escape_pg_identifier;
using dbtypes::escape_pg_string;
using dbtypes::exec_ddl;
using dbtypes::fetch_rows;
using dbtypes::gen_sqlite_create_table;
using dbtypes::get_tables;
using dbtypes::get_table_schema;
using dbtypes::insert_row;
using dbtypes::is_valid_mysql_charset;
using dbtypes::is_valid_mysql_engine;
using dbtypes::is_valid_sql_identifier;
using dbtypes::is_safe_dbtag;
using dbtypes::parse_target_type;
using dbtypes::quote_identifier;
using dbtypes::remove_quotes;
using dbtypes::reset_autoincrement;
using dbtypes::row_data_t;
using dbtypes::starts_with_icase;
using dbtypes::table_exists;
using dbtypes::to_lower;
using dbtypes::trim;
using dbtypes::validate_database_name;
using dbtypes::validate_table_name;
using orm::DB_TYPE;

// SQLite 数据库文件路径 (host 字段, 回退 dbname)
inline std::string sqlite_conn_path(const orm::orm_conn_t &cfg)
{
    return cfg.host.empty() ? cfg.dbname : cfg.host;
}

// ===================== DDL 生成 (迁移专用) =====================

inline std::string gen_mysql_drop_table(const std::string &table_name)
{
    return "DROP TABLE IF EXISTS `" + table_name + "`";
}

inline std::string build_mysql_create_table(const db_table_info &info)
{
    std::ostringstream oss;
    oss << "CREATE TABLE `" << info.table_name << "` (\n";

    for (size_t i = 0; i < info.fields.size(); i++)
    {
        const auto &f = info.fields[i];
        oss << "  `" << f.field_name << "` ";

        std::string type_str = f.field_type;

        if (f.is_auto_inc && f.is_pk)
        {
            if (to_lower(type_str) == "integer" || to_lower(type_str) == "int" || to_lower(type_str) == "serial")
            {
                type_str = "INT";
            }
            else if (to_lower(type_str) == "bigint" || to_lower(type_str) == "bigserial")
            {
                type_str = "BIGINT";
            }
            else if (to_lower(type_str) == "smallint" || to_lower(type_str) == "smallserial")
            {
                type_str = "SMALLINT";
            }
        }

        oss << type_str;

        std::string tl = to_lower(type_str);
        if (f.length > 0 && (tl == "varchar" || tl == "char" || tl == "character varying" || tl == "character"))
        {
            oss << "(" << f.length << ")";
        }
        else if (f.length > 0 && (tl == "decimal" || tl == "numeric"))
        {
            if (f.decimals > 0)
            {
                oss << "(" << f.length << "," << (unsigned int)f.decimals << ")";
            }
            else
            {
                oss << "(" << f.length << ")";
            }
        }

        if (f.is_unsigned)
            oss << " UNSIGNED";
        if (!f.is_nullable)
            oss << " NOT NULL";
        if (!f.default_value.empty())
        {
            std::string dl = to_lower(f.default_value);
            if (dl.find("nextval") == std::string::npos)
            {
                if (f.default_value == "CURRENT_TIMESTAMP" || f.default_value == "now()")
                {
                    oss << " DEFAULT " << f.default_value;
                }
                else if (f.default_value.size() >= 2 &&
                         (f.default_value.front() == '\'' || f.default_value.front() == '"'))
                {
                    oss << " DEFAULT " << f.default_value;
                }
                else
                {
                    oss << " DEFAULT '" << escape_mysql_string(f.default_value) << "'";
                }
            }
        }
        if (f.is_auto_inc)
            oss << " AUTO_INCREMENT";
        if (!f.comment.empty())
            oss << " COMMENT '" << escape_mysql_string(f.comment) << "'";

        if (i < info.fields.size() - 1)
            oss << ",";
        oss << "\n";
    }

    if (!info.pk_name.empty())
    {
        oss << ",  PRIMARY KEY (`" << info.pk_name << "`)\n";
    }

    oss << ")";

    if (!info.engine.empty() && is_valid_mysql_engine(info.engine))
        oss << " ENGINE=" << info.engine;
    else
        oss << " ENGINE=InnoDB";

    if (!info.charset.empty() && is_valid_mysql_charset(info.charset))
        oss << " DEFAULT CHARSET=" << info.charset;
    else
        oss << " DEFAULT CHARSET=utf8mb4";

    if (!info.table_comment.empty())
        oss << " COMMENT='" << escape_mysql_string(info.table_comment) << "'";

    oss << ";\n";
    return oss.str();
}

inline std::string gen_pg_drop_table(const std::string &table_name)
{
    return "DROP TABLE IF EXISTS " + escape_pg_identifier(table_name);
}

inline std::string build_pg_create_table(const db_table_info &info)
{
    std::ostringstream oss;
    oss << "CREATE TABLE " << escape_pg_identifier(info.table_name) << " (\n";

    for (size_t i = 0; i < info.fields.size(); i++)
    {
        const auto &f = info.fields[i];
        oss << "  " << escape_pg_identifier(f.field_name) << " ";

        if (f.is_auto_inc && f.is_pk)
        {
            std::string tl = to_lower(f.field_type);
            if (tl == "bigint" || tl == "bigserial")
            {
                oss << "BIGSERIAL PRIMARY KEY";
            }
            else if (tl == "smallint" || tl == "smallserial")
            {
                oss << "SMALLSERIAL PRIMARY KEY";
            }
            else
            {
                oss << "SERIAL PRIMARY KEY";
            }
        }
        else
        {
            std::string target_type = f.field_type;

            std::string tl = to_lower(f.field_type);
            if (tl == "serial" || tl == "smallserial" || tl == "bigserial")
            {
                target_type = "integer";
            }

            oss << target_type;

            if (!f.is_nullable)
                oss << " NOT NULL";
            if (!f.default_value.empty())
            {
                std::string dl = to_lower(f.default_value);
                if (dl.find("nextval") == std::string::npos)
                {
                    if (f.default_value == "CURRENT_TIMESTAMP" || f.default_value == "now()")
                    {
                        oss << " DEFAULT " << f.default_value;
                    }
                    else if (f.default_value.size() >= 2 &&
                             (f.default_value.front() == '\'' || f.default_value.front() == '"'))
                    {
                        oss << " DEFAULT " << f.default_value;
                    }
                    else
                    {
                        oss << " DEFAULT '" << escape_pg_string(f.default_value) << "'";
                    }
                }
            }
            if (f.is_pk)
                oss << " PRIMARY KEY";
        }

        if (i < info.fields.size() - 1)
            oss << ",";
        oss << "\n";
    }

    oss << ");\n";
    return oss.str();
}

inline std::string build_create_table(const db_table_info &info, DB_TYPE target_type)
{
    if (target_type == DB_TYPE::POSTGRESQL)
    {
        return build_pg_create_table(info);
    }
    if (target_type == DB_TYPE::SQLITE)
    {
        // 含 DROP TABLE IF EXISTS + CREATE TABLE + 索引, 一次 exec_sql 全部执行 (幂等)
        return gen_sqlite_create_table(info);
    }
    return build_mysql_create_table(info);
}

// ===================== 主迁移逻辑 =====================

inline int dbconvercli(const std::string &dbtag1 = "", const std::string &dbtag2 = "", const std::string &force_flag = "")
{
    if (dbtag1.empty() || dbtag2.empty())
    {
        std::cout << "Usage: paozhu_cli dbconver <dbtag1> <dbtag2> [force]" << std::endl;
        std::cout << "  dbtag1: source database tag (from conf/orm.conf)" << std::endl;
        std::cout << "  dbtag2: target database tag (from conf/orm.conf)" << std::endl;
        std::cout << "  force:  drop and recreate if table already exists (optional)" << std::endl;
        std::cout << "Example: paozhu_cli dbconver cms pg" << std::endl;
        std::cout << "         (migrate MySQL [cms] to PostgreSQL [pg], skip existing tables)" << std::endl;
        std::cout << "         paozhu_cli dbconver cms pg force" << std::endl;
        std::cout << "         (migrate and overwrite existing tables)" << std::endl;
        return 1;
    }

    // ---- 输入安全验证 ----
    if (!is_safe_dbtag(dbtag1))
    {
        return 1;
    }
    if (!is_safe_dbtag(dbtag2))
    {
        return 1;
    }
    if (!force_flag.empty() && force_flag != "force")
    {
        std::cerr << "  [ERROR] Invalid force flag: '" << force_flag << "'" << std::endl;
        std::cerr << "          Only 'force' or empty is allowed" << std::endl;
        return 1;
    }
    // ---- 安全验证完成 ----

    bool force_overwrite = (force_flag == "force");

    std::cout << "\033[36m 🍄 Database Converter\033[0m" << std::endl;
    std::cout << "  Source: \033[35m[" << dbtag1 << "]\033[0m  →  Target: \033[35m[" << dbtag2 << "]\033[0m" << std::endl;
    if (force_overwrite)
    {
        std::cout << "  Mode: \033[33m[FORCE]\033[0m (will overwrite existing tables)" << std::endl;
    }
    else
    {
        std::cout << "  Mode: \033[32m[SAFE]\033[0m (skip existing tables)" << std::endl;
    }

    std::vector<orm::orm_conn_t> myconfig = orm::get_orm_config_file("conf/orm.conf");
    if (myconfig.empty())
    {
        std::cerr << "  [ERROR] Cannot read conf/orm.conf" << std::endl;
        return 1;
    }

    orm::orm_conn_t src_config, dst_config;
    bool found_src = false, found_dst = false;

    for (auto &c : myconfig)
    {
        if (c.link_type == 0 && c.tag == dbtag1)
        {
            src_config = c;
            found_src  = true;
        }
        if (c.link_type == 0 && c.tag == dbtag2)
        {
            dst_config = c;
            found_dst  = true;
        }
    }

    if (!found_src)
    {
        std::cerr << "  [ERROR] Source tag [" << dbtag1 << "] not found in conf/orm.conf" << std::endl;
        return 1;
    }
    if (!found_dst)
    {
        std::cerr << "  [ERROR] Target tag [" << dbtag2 << "] not found in conf/orm.conf" << std::endl;
        return 1;
    }

    DB_TYPE src_type = parse_target_type(src_config.dbtype);
    DB_TYPE dst_type = parse_target_type(dst_config.dbtype);

    std::cout << "  Source type: " << db_type_display(src_type) << std::endl;
    std::cout << "  Target type: " << db_type_display(dst_type) << std::endl;

    // 同源同目标检查: SQLite 比较文件路径, 网络库比较 host/port/dbname/user
    bool same_db = false;
    if (src_type == DB_TYPE::SQLITE || dst_type == DB_TYPE::SQLITE)
    {
        same_db = (src_type == dst_type) && (sqlite_conn_path(src_config) == sqlite_conn_path(dst_config));
    }
    else
    {
        same_db = src_config.host == dst_config.host &&
                  src_config.port == dst_config.port &&
                  src_config.dbname == dst_config.dbname &&
                  src_config.user == dst_config.user;
    }
    if (same_db)
    {
        std::cerr << "  [WARN] Source and target are the same database, aborting." << std::endl;
        return 1;
    }

    asio::io_context io_context;

    auto src = create_connection(src_config, src_type, io_context);
    if (!src.ok())
    {
        std::cerr << "  [ERROR] " << src.error_msg << std::endl;
        return 1;
    }
    std::cout << "  [OK] Source " << db_type_display(src_type) << " connected" << std::endl;

    auto dst = create_connection(dst_config, dst_type, io_context);
    if (!dst.ok())
    {
        std::cerr << "  [ERROR] " << dst.error_msg << std::endl;
        return 1;
    }
    std::cout << "  [OK] Target " << db_type_display(dst_type) << " connected" << std::endl;

    bool target_db_exists = false;
    if (dst_type == DB_TYPE::SQLITE)
    {
        // SQLite: 文件不存在时 sqlite3_open 自动创建, connect 已成功即视为可用
        target_db_exists = true;
    }
    else
    {
        if (!validate_database_name(dst_config.dbname))
        {
            return 1;
        }
        std::string check_sql;
        if (dst_type == DB_TYPE::MYSQL)
            check_sql = "SELECT SCHEMA_NAME FROM information_schema.SCHEMATA WHERE SCHEMA_NAME = '" +
                        dst_config.dbname + "'";
        else
            check_sql = "SELECT 1 FROM pg_database WHERE datname = '" + dst_config.dbname + "'";
        std::vector<row_data_t> check_rows;
        if (fetch_rows(dst.conn, dst_type, check_sql, check_rows))
        {
            target_db_exists = !check_rows.empty();
        }
    }

    if (!target_db_exists)
    {
        std::cerr << "\n  [ERROR] Target database '" << dst_config.dbname
                  << "' does not exist! Please create it manually first:" << std::endl;
        if (dst_type == DB_TYPE::MYSQL)
        {
            std::cerr << "    MySQL: CREATE DATABASE `" << dst_config.dbname
                      << "` DEFAULT CHARSET utf8mb4 COLLATE utf8mb4_general_ci;" << std::endl;
        }
        else
        {
            std::cerr << "    PostgreSQL: CREATE DATABASE " << dst_config.dbname
                      << " WITH ENCODING 'UTF8';" << std::endl;
        }
        return 1;
    }

    std::cout << "  [OK] Target database '" << dst_config.dbname << "' exists" << std::endl;

    auto table_list = get_tables(src.conn, src_type);

    if (table_list.empty())
    {
        std::cout << "  [INFO] No tables found in source database." << std::endl;
        return 0;
    }

    std::cout << "\n  Found " << table_list.size() << " tables in source." << std::endl;
    std::cout << "  ===============================" << std::endl;

    unsigned int total_success = 0;
    unsigned int total_fail    = 0;
    unsigned int total_rows    = 0;
    unsigned int batch_size    = 500;

    for (size_t ti = 0; ti < table_list.size(); ti++)
    {
        const std::string &table_name = table_list[ti];

        if (!is_valid_sql_identifier(table_name))
        {
            std::cerr << " \033[33m[SKIP]\033[0m Invalid table name: '" << table_name << "'" << std::endl;
            total_fail++;
            continue;
        }

        std::cout << "\n  [" << (ti + 1) << "/" << table_list.size() << "] " << table_name << std::flush;

        db_table_info table_info;
        bool schema_ok = get_table_schema(src.conn, src_type, table_name, table_info);

        if (!schema_ok)
        {
            std::cout << " \033[31m[SKIP]\033[0m (read schema failed)" << std::endl;
            total_fail++;
            continue;
        }

        if (src_type != dst_type)
        {
            for (auto &f : table_info.fields)
            {
                std::string target_type = convert_type_for_target(
                    f.field_type,
                    f.length,
                    f.is_unsigned,
                    src_type,
                    dst_type);
                f.field_type = target_type;
                if (src_type == DB_TYPE::SQLITE)
                {
                    // SQLite 源声明自带参数 (如 varchar(255)), 清零避免目标 DDL 重复追加
                    f.length   = 0;
                    f.decimals = 0;
                }
                f.is_unsigned = false;
            }
        }
        table_info.source_db_type = src_type;

        bool tbl_exists = table_exists(dst.conn, dst_type, table_name, dst_config.dbname);

        if (tbl_exists && !force_overwrite)
        {
            std::cout << " \033[33m[SKIP]\033[0m (table already exists in target, use 'force' to overwrite)" << std::endl;
            continue;
        }

        std::string ddl = build_create_table(table_info, dst_type);

        bool create_ok = false;
        std::string ddl_err;
        if (!tbl_exists)
        {
            create_ok = exec_ddl(dst.conn, dst_type, ddl, ddl_err);
            if (!create_ok)
            {
                std::cerr << " \033[31m[FAIL]\033[0m DDL error: " << ddl_err << std::endl;
            }
        }
        else
        {
            // force_overwrite: 先 DROP 再 CREATE
            if (dst_type == DB_TYPE::MYSQL)
            {
                std::string drop_sql = gen_mysql_drop_table(table_info.table_name);
                std::string drop_err;
                if (!exec_ddl(dst.conn, dst_type, drop_sql, drop_err))
                {
                    std::cerr << " \033[33m[WARN]\033[0m DROP TABLE failed: " << drop_err << std::endl;
                }
            }
            else if (dst_type == DB_TYPE::POSTGRESQL)
            {
                std::string drop_sql = gen_pg_drop_table(table_info.table_name);
                std::string drop_err;
                if (!exec_ddl(dst.conn, dst_type, drop_sql, drop_err))
                {
                    std::cerr << " \033[33m[WARN]\033[0m DROP TABLE failed: " << drop_err << std::endl;
                }
            }
            // SQLite: DDL 自带 DROP TABLE IF EXISTS, 无需单独 DROP
            create_ok = exec_ddl(dst.conn, dst_type, ddl, ddl_err);
            if (!create_ok)
            {
                std::cerr << " \033[31m[FAIL]\033[0m DDL: " << ddl_err << std::endl;
            }
        }

        if (!create_ok)
        {
            total_fail++;
            continue;
        }

        std::cout << " \033[33m[CREATED]\033[0m " << std::flush;

        unsigned long long total_count = 0;
        {
            std::string count_sql = "SELECT COUNT(*) FROM " + quote_identifier(src_type, table_name);
            std::vector<row_data_t> count_rows;
            bool count_ok = fetch_rows(src.conn, src_type, count_sql, count_rows);
            if (count_ok && !count_rows.empty() && !count_rows[0].values.empty())
            {
                try
                {
                    total_count = std::stoull(count_rows[0].values[0]);
                }
                catch (...)
                {
                }
            }
        }

        unsigned long long total_rows_migrated = 0;
        unsigned long long offset              = 0;

        while (total_rows_migrated < total_count)
        {
            std::string select_sql = "SELECT * FROM " + quote_identifier(src_type, table_name) +
                                     " LIMIT " + std::to_string(batch_size) + " OFFSET " + std::to_string(offset);

            std::vector<row_data_t> batch_rows;
            bool read_ok = fetch_rows(src.conn, src_type, select_sql, batch_rows);

            if (!read_ok || batch_rows.empty())
                break;

            if (dst_type == DB_TYPE::SQLITE)
            {
                // SQLite 目标: 参数绑定插入 (二进制安全), 每批次包裹事务提速
                auto dst_sqlite = std::get<std::shared_ptr<orm::sqlite_conn_base>>(dst.conn);
                dst_sqlite->exec_sql("BEGIN");
                std::string ins_err;
                for (auto &row : batch_rows)
                {
                    if (!insert_row(dst.conn, dst_type, table_info.table_name, table_info.fields, row, ins_err))
                    {
                        std::cerr << "\n  \033[31m[WARN]\033[0m SQLite INSERT failed: " << ins_err << std::endl;
                    }
                }
                dst_sqlite->exec_sql("COMMIT");
            }
            else
            {
                std::string ins_err;
                for (auto &row : batch_rows)
                {
                    if (!insert_row(dst.conn, dst_type, table_info.table_name, table_info.fields, row, ins_err))
                    {
                        std::cerr << " \033[31m[WARN]\033[0m INSERT failed: " << ins_err << std::endl;
                    }
                }
            }

            total_rows_migrated += batch_rows.size();
            offset += batch_rows.size();

            std::cout << "\r  [" << (ti + 1) << "/" << table_list.size() << "] " << table_name
                      << " \033[33m[MIGRATING]\033[0m " << total_rows_migrated << "/" << total_count << " rows" << std::flush;
        }

        if (!table_info.auto_inc_field.empty())
        {
            if (!is_valid_sql_identifier(table_info.auto_inc_field))
            {
                std::cerr << " \033[33m[WARN]\033[0m Invalid auto increment field name: '"
                          << table_info.auto_inc_field << "', skip reset" << std::endl;
            }
            else
            {
                std::string ai_err;
                if (!reset_autoincrement(dst.conn, dst_type, table_info.table_name, table_info.auto_inc_field, ai_err))
                {
                    std::cerr << " \033[33m[WARN]\033[0m Failed to reset auto-increment: " << ai_err << std::endl;
                }
            }
        }

        std::cout << " \033[32m[OK]\033[0m " << total_rows_migrated << " rows migrated" << std::endl;
        total_success++;
        total_rows += total_rows_migrated;
    }

    std::cout << "\n  ===============================" << std::endl;
    std::cout << "  Migration Summary:" << std::endl;
    std::cout << "    Total tables:  " << table_list.size() << std::endl;
    std::cout << "    Success:       \033[32m" << total_success << "\033[0m" << std::endl;
    std::cout << "    Failed:        \033[31m" << total_fail << "\033[0m" << std::endl;
    std::cout << "    Total rows:    " << total_rows << std::endl;
    std::cout << "  ===============================" << std::endl;

    if (total_fail > 0)
    {
        return 1;
    }
    return 0;
}

}// namespace dbconver

#endif// _DBCONVER_HPP
