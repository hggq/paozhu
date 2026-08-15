/*
 *  @author 黄自权 huangziquan
 *  @date 2026-08-14
 *  @dest 数据库转换迁移工具 - MySQL <-> PostgreSQL
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
using dbtypes::db_field_info;
using dbtypes::db_index_info;
using dbtypes::db_table_info;
using dbtypes::escape_mysql_string;
using dbtypes::escape_pg_identifier;
using dbtypes::escape_pg_string;
using dbtypes::is_valid_mysql_charset;
using dbtypes::is_valid_mysql_engine;
using dbtypes::is_valid_sql_identifier;
using dbtypes::mysql_fetch_rows;
using dbtypes::mysql_get_table_schema;
using dbtypes::mysql_type_to_pg;
using dbtypes::parse_mysql_show_create;
using dbtypes::pg_fetch_rows;
using dbtypes::pg_get_table_schema;
using dbtypes::pg_type_to_mysql;
using dbtypes::remove_quotes;
using dbtypes::row_data_t;
using dbtypes::starts_with_icase;
using dbtypes::to_lower;
using dbtypes::trim;
using dbtypes::validate_database_name;
using dbtypes::validate_table_name;
using orm::DB_TYPE;

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

    DB_TYPE src_type = (to_lower(src_config.dbtype) == "postgresql") ? DB_TYPE::POSTGRESQL : DB_TYPE::MYSQL;
    DB_TYPE dst_type = (to_lower(dst_config.dbtype) == "postgresql") ? DB_TYPE::POSTGRESQL : DB_TYPE::MYSQL;

    std::cout << "  Source type: " << (src_type == DB_TYPE::POSTGRESQL ? "PostgreSQL" : "MySQL") << std::endl;
    std::cout << "  Target type: " << (dst_type == DB_TYPE::POSTGRESQL ? "PostgreSQL" : "MySQL") << std::endl;

    if (src_config.host == dst_config.host &&
        src_config.port == dst_config.port &&
        src_config.dbname == dst_config.dbname &&
        src_config.user == dst_config.user)
    {
        std::cerr << "  [WARN] Source and target are the same database, aborting." << std::endl;
        return 1;
    }

    asio::io_context io_context;

    std::shared_ptr<orm::mysql_conn_base> src_mysql;
    std::shared_ptr<orm::pg_conn_base> src_pg;

    if (src_type == DB_TYPE::MYSQL)
    {
        auto src_pool          = std::make_shared<orm::orm_conn_pool>();
        src_pool->io_context   = &io_context;
        src_pool->conf_data[0] = src_config;
        try
        {
            src_mysql = src_pool->add_mysql_edit_connect();
        }
        catch (const std::exception &e)
        {
            std::cerr << "  [ERROR] MySQL source connect failed: " << e.what() << std::endl;
            return 1;
        }
        std::cout << "  [OK] Source MySQL connected" << std::endl;
    }
    else
    {
        src_pg = std::make_shared<orm::pg_conn_base>(
            orm::orm_conn_link_t::create(io_context, orm::DB_TYPE::POSTGRESQL));
        orm::orm_conn_t pg_cfg = src_config;
        if (pg_cfg.port.empty())
            pg_cfg.port = "5432";
        if (!src_pg->connect(pg_cfg))
        {
            std::cerr << "  [ERROR] PG source connect failed: " << src_pg->error_msg << std::endl;
            return 1;
        }
        std::cout << "  [OK] Source PostgreSQL connected" << std::endl;
    }

    std::shared_ptr<orm::mysql_conn_base> dst_mysql;
    std::shared_ptr<orm::pg_conn_base> dst_pg;

    if (dst_type == DB_TYPE::MYSQL)
    {
        auto dst_pool          = std::make_shared<orm::orm_conn_pool>();
        dst_pool->io_context   = &io_context;
        dst_pool->conf_data[0] = dst_config;
        try
        {
            dst_mysql = dst_pool->add_mysql_edit_connect();
        }
        catch (const std::exception &e)
        {
            std::cerr << "  [ERROR] MySQL target connect failed: " << e.what() << std::endl;
            return 1;
        }
        std::cout << "  [OK] Target MySQL connected" << std::endl;
    }
    else
    {
        dst_pg = std::make_shared<orm::pg_conn_base>(
            orm::orm_conn_link_t::create(io_context, orm::DB_TYPE::POSTGRESQL));
        orm::orm_conn_t pg_cfg = dst_config;
        if (pg_cfg.port.empty())
            pg_cfg.port = "5432";
        if (!dst_pg->connect(pg_cfg))
        {
            std::cerr << "  [ERROR] PG target connect failed: " << dst_pg->error_msg << std::endl;
            return 1;
        }
        std::cout << "  [OK] Target PostgreSQL connected" << std::endl;
    }

    bool target_db_exists = false;
    if (!validate_database_name(dst_config.dbname))
    {
        return 1;
    }

    if (dst_type == DB_TYPE::MYSQL)
    {
        if (!dst_mysql)
        {
            std::cerr << "  [ERROR] Target MySQL connection failed (null)" << std::endl;
            return 1;
        }
        std::string check_sql = "SELECT SCHEMA_NAME FROM information_schema.SCHEMATA WHERE SCHEMA_NAME = '" +
                                dst_config.dbname + "'";
        std::vector<row_data_t> check_rows;
        if (mysql_fetch_rows(dst_mysql, check_sql, check_rows))
        {
            target_db_exists = !check_rows.empty();
        }
    }
    else
    {
        if (!dst_pg)
        {
            std::cerr << "  [ERROR] Target PostgreSQL connection failed (null)" << std::endl;
            return 1;
        }
        std::string check_sql = "SELECT 1 FROM pg_database WHERE datname = '" + dst_config.dbname + "'";
        std::vector<row_data_t> check_rows;
        if (pg_fetch_rows(dst_pg, check_sql, check_rows))
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

    std::vector<std::string> table_list;
    if (src_type == DB_TYPE::MYSQL)
    {
        if (!src_mysql)
        {
            std::cerr << "  [ERROR] Source MySQL connection failed (null)" << std::endl;
            return 1;
        }
        std::vector<row_data_t> rows;
        if (mysql_fetch_rows(src_mysql, "SHOW TABLES", rows))
        {
            for (auto &r : rows)
            {
                if (!r.values.empty())
                {
                    table_list.push_back(r.values[0]);
                }
            }
        }
    }
    else
    {
        if (!src_pg)
        {
            std::cerr << "  [ERROR] Source PostgreSQL connection failed (null)" << std::endl;
            return 1;
        }
        std::vector<row_data_t> rows;
        if (pg_fetch_rows(src_pg,
                          "SELECT tablename FROM pg_tables WHERE schemaname = 'public' ORDER BY tablename",
                          rows))
        {
            for (auto &r : rows)
            {
                if (!r.values.empty())
                {
                    table_list.push_back(r.values[0]);
                }
            }
        }
    }

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
        bool schema_ok = false;

        if (src_type == DB_TYPE::MYSQL)
        {
            schema_ok = mysql_get_table_schema(src_mysql, table_name, table_info);
        }
        else
        {
            schema_ok = pg_get_table_schema(src_pg, table_name, table_info);
        }

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
                    dst_type);
                f.field_type = target_type;
            }
        }
        table_info.source_db_type = src_type;

        bool table_exists = false;
        if (dst_type == DB_TYPE::MYSQL)
        {
            std::string check_sql = "SELECT 1 FROM information_schema.TABLES WHERE TABLE_SCHEMA='" +
                                    dst_config.dbname + "' AND TABLE_NAME='" + table_name + "'";
            std::vector<row_data_t> check_rows;
            if (mysql_fetch_rows(dst_mysql, check_sql, check_rows))
            {
                table_exists = !check_rows.empty();
            }
        }
        else
        {
            std::string check_sql = "SELECT 1 FROM information_schema.tables WHERE table_schema='public' AND table_name='" +
                                    table_name + "'";
            std::vector<row_data_t> check_rows;
            if (pg_fetch_rows(dst_pg, check_sql, check_rows))
            {
                table_exists = !check_rows.empty();
            }
        }

        if (table_exists && !force_overwrite)
        {
            std::cout << " \033[33m[SKIP]\033[0m (table already exists in target, use 'force' to overwrite)" << std::endl;
            continue;
        }

        std::string ddl = build_create_table(table_info, dst_type);

        bool create_ok = false;
        if (!table_exists)
        {
            if (dst_type == DB_TYPE::MYSQL)
            {
                if (dst_mysql->write_sql(ddl) > 0)
                {
                    unsigned int rn = dst_mysql->read_loop();
                    if (rn > 0)
                    {
                        orm::pack_info_t ddl_resp;
                        ddl_resp.seq_id      = 1;
                        unsigned int roffset = 0;
                        dst_mysql->read_field_pack(dst_mysql->_cache_data, rn, roffset, ddl_resp);
                        if (ddl_resp.error == 0 && ddl_resp.data.size() > 0 && (unsigned char)ddl_resp.data[0] == 0x00)
                        {
                            create_ok = true;
                        }
                        else if (ddl_resp.error == 0 && ddl_resp.data.size() > 0 && (unsigned char)ddl_resp.data[0] == 0xFF)
                        {
                            std::cerr << " \033[31m[FAIL]\033[0m DDL error: " << ddl_resp.data.substr(1) << std::endl;
                        }
                        else
                        {
                            create_ok = true;
                        }
                    }
                    else
                    {
                        std::cerr << " \033[33m[WARN]\033[0m CREATE TABLE read failed: " << dst_mysql->error_msg << std::endl;
                    }
                }
                else
                {
                    std::cerr << " \033[31m[FAIL]\033[0m CREATE TABLE write failed: " << dst_mysql->error_msg << std::endl;
                }
            }
            else
            {
                std::vector<orm::field_info_t> dummy_fields;
                std::vector<orm::pg_row_data_t> dummy_rows;
                unsigned int affected = 0;
                unsigned int err      = dst_pg->execute_and_fetch(ddl, dummy_fields, dummy_rows, affected);
                if (err == 0)
                {
                    create_ok = true;
                }
                else
                {
                    std::cerr << " \033[31m[FAIL]\033[0m DDL: " << dst_pg->error_msg << std::endl;
                }
            }
        }
        else
        {
            if (dst_type == DB_TYPE::MYSQL)
            {
                std::string drop_sql = gen_mysql_drop_table(table_info.table_name);
                if (dst_mysql->write_sql(drop_sql) > 0)
                {
                    unsigned int rn = dst_mysql->read_loop();
                    if (rn == 0)
                    {
                        std::cerr << " \033[33m[WARN]\033[0m DROP TABLE failed: " << dst_mysql->error_msg << std::endl;
                    }
                }
                else
                {
                    std::cerr << " \033[33m[WARN]\033[0m DROP TABLE write failed: " << dst_mysql->error_msg << std::endl;
                }
                if (dst_mysql->write_sql(ddl) > 0)
                {
                    unsigned int rn = dst_mysql->read_loop();
                    if (rn > 0)
                    {
                        orm::pack_info_t ddl_resp;
                        ddl_resp.seq_id      = 1;
                        unsigned int roffset = 0;
                        dst_mysql->read_field_pack(dst_mysql->_cache_data, rn, roffset, ddl_resp);
                        if (ddl_resp.error == 0 && ddl_resp.data.size() > 0 && (unsigned char)ddl_resp.data[0] == 0x00)
                        {
                            create_ok = true;
                        }
                        else if (ddl_resp.error == 0 && ddl_resp.data.size() > 0 && (unsigned char)ddl_resp.data[0] == 0xFF)
                        {
                            std::cerr << " \033[31m[FAIL]\033[0m DDL error: " << ddl_resp.data.substr(1) << std::endl;
                        }
                        else
                        {
                            create_ok = true;
                        }
                    }
                    else
                    {
                        std::cerr << " \033[33m[WARN]\033[0m CREATE TABLE read failed: " << dst_mysql->error_msg << std::endl;
                    }
                }
                else
                {
                    std::cerr << " \033[31m[FAIL]\033[0m CREATE TABLE write failed: " << dst_mysql->error_msg << std::endl;
                }
            }
            else
            {
                std::string drop_sql = gen_pg_drop_table(table_info.table_name);
                std::vector<orm::field_info_t> dummy_fields;
                std::vector<orm::pg_row_data_t> dummy_rows;
                unsigned int affected = 0;
                unsigned int drop_err = dst_pg->execute_and_fetch(drop_sql, dummy_fields, dummy_rows, affected);
                if (drop_err > 0)
                {
                    std::cerr << " \033[33m[WARN]\033[0m DROP TABLE failed: " << dst_pg->error_msg << std::endl;
                }

                unsigned int err = dst_pg->execute_and_fetch(ddl, dummy_fields, dummy_rows, affected);
                if (err == 0)
                {
                    create_ok = true;
                }
                else
                {
                    std::cerr << " \033[31m[FAIL]\033[0m DDL: " << dst_pg->error_msg << std::endl;
                }
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
            std::string count_sql;
            if (src_type == DB_TYPE::MYSQL)
            {
                count_sql = "SELECT COUNT(*) FROM `" + table_name + "`";
            }
            else
            {
                count_sql = "SELECT COUNT(*) FROM " + table_name;
            }
            std::vector<row_data_t> count_rows;
            bool count_ok = false;
            if (src_type == DB_TYPE::MYSQL)
            {
                count_ok = mysql_fetch_rows(src_mysql, count_sql, count_rows);
            }
            else
            {
                count_ok = pg_fetch_rows(src_pg, count_sql, count_rows);
            }
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
            std::string select_sql;
            if (src_type == DB_TYPE::MYSQL)
            {
                select_sql = "SELECT * FROM `" + table_name + "` LIMIT " +
                             std::to_string(batch_size) + " OFFSET " + std::to_string(offset);
            }
            else
            {
                select_sql = "SELECT * FROM " + table_name + " LIMIT " +
                             std::to_string(batch_size) + " OFFSET " + std::to_string(offset);
            }

            std::vector<row_data_t> batch_rows;
            bool read_ok = false;
            if (src_type == DB_TYPE::MYSQL)
            {
                read_ok = mysql_fetch_rows(src_mysql, select_sql, batch_rows);
            }
            else
            {
                read_ok = pg_fetch_rows(src_pg, select_sql, batch_rows);
            }

            if (!read_ok || batch_rows.empty())
                break;

            for (auto &row : batch_rows)
            {
                std::string insert_sql;
                if (dst_type == DB_TYPE::MYSQL)
                {
                    insert_sql = build_insert_sql_mysql(table_info.table_name, table_info.fields, row, 0, row.values.size());
                    if (dst_mysql->write_sql(insert_sql) > 0)
                    {
                        unsigned int rn = dst_mysql->read_loop();
                        if (rn == 0)
                        {
                            std::cerr << " \033[31m[WARN]\033[0m MySQL INSERT read failed: " << dst_mysql->error_msg << std::endl;
                        }
                    }
                    else
                    {
                        std::cerr << " \033[31m[WARN]\033[0m MySQL INSERT write failed: " << dst_mysql->error_msg << std::endl;
                    }
                }
                else
                {
                    insert_sql = build_insert_sql_pg(table_info.table_name, table_info.fields, row, 0, row.values.size());
                    std::vector<orm::field_info_t> dummy_fields;
                    std::vector<orm::pg_row_data_t> dummy_rows;
                    unsigned int affected = 0;
                    unsigned int err      = dst_pg->execute_and_fetch(insert_sql, dummy_fields, dummy_rows, affected);
                    if (err > 0)
                    {
                        std::cerr << " \033[31m[WARN]\033[0m PostgreSQL INSERT failed: " << dst_pg->error_msg << std::endl;
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
            else if (dst_type == DB_TYPE::MYSQL)
            {
                std::string alter_sql = "SELECT MAX(`" + table_info.auto_inc_field + "`) FROM `" +
                                        table_info.table_name + "`";
                std::vector<row_data_t> max_rows;
                if (mysql_fetch_rows(dst_mysql, alter_sql, max_rows) && !max_rows.empty() && !max_rows[0].values.empty())
                {
                    if (!max_rows[0].values[0].empty() && !max_rows[0].is_null[0])
                    {
                        try
                        {
                            unsigned long long max_val = std::stoull(max_rows[0].values[0]);
                            std::string reset_sql      = "ALTER TABLE `" + table_info.table_name +
                                                    "` AUTO_INCREMENT = " + std::to_string(max_val + 1);
                            if (dst_mysql->write_sql(reset_sql) > 0)
                            {
                                unsigned int rn = dst_mysql->read_loop();
                                if (rn == 0)
                                {
                                    std::cerr << " \033[33m[WARN]\033[0m Failed to reset AUTO_INCREMENT read: " << dst_mysql->error_msg << std::endl;
                                }
                            }
                            else
                            {
                                std::cerr << " \033[33m[WARN]\033[0m Failed to reset AUTO_INCREMENT write: " << dst_mysql->error_msg << std::endl;
                            }
                        }
                        catch (...)
                        {
                        }
                    }
                }
            }
            else if (dst_type == DB_TYPE::POSTGRESQL)
            {
                std::string seq_name  = table_info.table_name + "_" + table_info.auto_inc_field + "_seq";
                std::string alter_sql = "SELECT setval('" + seq_name + "', (SELECT MAX(" +
                                        table_info.auto_inc_field + ") FROM " +
                                        table_info.table_name + "))";
                std::vector<orm::field_info_t> dummy_fields;
                std::vector<orm::pg_row_data_t> dummy_rows;
                unsigned int affected = 0;
                unsigned int err      = dst_pg->execute_and_fetch(alter_sql, dummy_fields, dummy_rows, affected);
                if (err > 0)
                {
                    std::cerr << " \033[33m[WARN]\033[0m Failed to reset sequence: " << dst_pg->error_msg << std::endl;
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
