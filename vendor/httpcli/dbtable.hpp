/*
 *  @author 黄自权 huangziquan
 *  @date 2026-08-14
 *  @dest 导出数据库表结构到SQL文件
 *  Usage: paozhu_cli dbtable <dbtag> <filename.sql> [-target=mysql|postgresql]
 */

#ifndef _DBTABLE_HPP
#define _DBTABLE_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <cctype>
#include "dbtypes.hpp"

namespace fs = std::filesystem;
namespace dbtable
{

using dbtypes::convert_table_for_target;
using dbtypes::convert_type_for_target;
using dbtypes::db_field_info;
using dbtypes::db_index_info;
using dbtypes::db_table_info;
using dbtypes::escape_mysql_string;
using dbtypes::escape_pg_string;
using dbtypes::gen_ddl;
using dbtypes::is_valid_sql_identifier;
using dbtypes::mysql_fetch_rows;
using dbtypes::mysql_get_table_schema;
using dbtypes::mysql_type_to_pg;
using dbtypes::parse_mysql_show_create;
using dbtypes::parse_target_type;
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

// ===================== 获取表列表 =====================

inline std::vector<std::string> get_mysql_tables(std::shared_ptr<orm::mysql_conn_base> mysql_conn)
{
    std::vector<std::string> tables;
    if (!mysql_conn)
    {
        std::cerr << "  [ERROR] MySQL connection is null" << std::endl;
        return tables;
    }
    std::vector<row_data_t> rows;
    if (mysql_fetch_rows(mysql_conn, "SHOW TABLES", rows))
    {
        for (auto &r : rows)
        {
            if (!r.values.empty())
            {
                tables.push_back(r.values[0]);
            }
        }
    }
    else
    {
        std::vector<row_data_t> rows2;
        std::string sql2 = "SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_SCHEMA = DATABASE()";
        if (mysql_fetch_rows(mysql_conn, sql2, rows2))
        {
            for (auto &r : rows2)
            {
                if (!r.values.empty())
                {
                    tables.push_back(r.values[0]);
                }
            }
        }
    }
    return tables;
}

inline std::vector<std::string> get_pg_tables(std::shared_ptr<orm::pg_conn_base> pg_conn)
{
    std::vector<std::string> tables;
    if (!pg_conn)
    {
        std::cerr << "  [ERROR] PostgreSQL connection is null" << std::endl;
        return tables;
    }
    std::vector<row_data_t> rows;
    if (pg_fetch_rows(pg_conn,
                      "SELECT tablename FROM pg_tables WHERE schemaname = 'public' ORDER BY tablename",
                      rows))
    {
        for (auto &r : rows)
        {
            if (!r.values.empty())
            {
                tables.push_back(r.values[0]);
            }
        }
    }
    return tables;
}

// ===================== 主入口 =====================

inline int dbtablecli(const std::string &dbtag = "", const std::string &filename = "", const std::string &target_type_str = "")
{
    if (dbtag.empty() || filename.empty())
    {
        std::cout << "Usage: paozhu_cli dbtable <dbtag> <filename.sql> [-target=mysql|postgresql]" << std::endl;
        std::cout << "  dbtag:    database tag (from conf/orm.conf)" << std::endl;
        std::cout << "  filename: output SQL file path" << std::endl;
        std::cout << "  -target:  target database type for DDL conversion (optional)" << std::endl;
        std::cout << "            mysql, postgresql, pg" << std::endl;
        std::cout << "Example: paozhu_cli dbtable cms ./schema.sql" << std::endl;
        std::cout << "         (export MySQL [cms] table structures to ./schema.sql)" << std::endl;
        std::cout << "         paozhu_cli dbtable cms ./pg_schema.sql -target=postgresql" << std::endl;
        std::cout << "         (export MySQL [cms] table structures as PostgreSQL DDL)" << std::endl;
        return 1;
    }

    std::cout << "\033[36m 🍄 Database Table Exporter\033[0m" << std::endl;
    std::cout << "  Source: \033[35m[" << dbtag << "]\033[0m" << std::endl;
    std::cout << "  Output: \033[35m" << filename << "\033[0m" << std::endl;

    std::vector<orm::orm_conn_t> myconfig = orm::get_orm_config_file("conf/orm.conf");
    if (myconfig.empty())
    {
        std::cerr << "  [ERROR] Cannot read conf/orm.conf" << std::endl;
        return 1;
    }

    orm::orm_conn_t src_config;
    bool found_src = false;

    for (auto &c : myconfig)
    {
        if (c.link_type == 0 && c.tag == dbtag)
        {
            src_config = c;
            found_src  = true;
        }
    }

    if (!found_src)
    {
        std::cerr << "  [ERROR] Tag [" << dbtag << "] not found in conf/orm.conf" << std::endl;
        return 1;
    }

    DB_TYPE src_type    = (to_lower(src_config.dbtype) == "postgresql") ? DB_TYPE::POSTGRESQL : DB_TYPE::MYSQL;
    DB_TYPE target_type = src_type;
    if (!target_type_str.empty())
    {
        target_type = parse_target_type(target_type_str);
    }

    std::cout << "  Database type: " << (src_type == DB_TYPE::POSTGRESQL ? "PostgreSQL" : "MySQL") << std::endl;
    if (target_type != src_type)
    {
        std::cout << "  Target DDL:   " << (target_type == DB_TYPE::POSTGRESQL ? "PostgreSQL" : "MySQL") << " (converted)" << std::endl;
    }
    std::cout << "  Host: " << src_config.host << ":" << src_config.port << std::endl;
    std::cout << "  Database: " << src_config.dbname << std::endl;

    asio::io_context io_context;

    // Connect to source
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
            std::cerr << "  [ERROR] MySQL connect failed: " << e.what() << std::endl;
            return 1;
        }
        std::cout << "  [OK] MySQL connected" << std::endl;
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
            std::cerr << "  [ERROR] PostgreSQL connect failed: " << src_pg->error_msg << std::endl;
            return 1;
        }
        std::cout << "  [OK] PostgreSQL connected" << std::endl;
    }

    // Get table list
    std::vector<std::string> table_list;
    if (src_type == DB_TYPE::MYSQL)
    {
        table_list = get_mysql_tables(src_mysql);
    }
    else
    {
        table_list = get_pg_tables(src_pg);
    }

    if (table_list.empty())
    {
        std::cout << "  [INFO] No tables found in database." << std::endl;
        return 0;
    }

    std::cout << "\n  Found " << table_list.size() << " tables." << std::endl;

    // Open output file
    std::ofstream outfile(filename);
    if (!outfile.is_open())
    {
        std::cerr << "  [ERROR] Cannot open output file: " << filename << std::endl;
        return 1;
    }

    // Write file header
    time_t now = time(nullptr);
    char time_buf[64];
#if defined(_WIN32)
    struct tm tm_buf;
    localtime_s(&tm_buf, &now);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
#else
    struct tm tm_buf;
    localtime_r(&now, &tm_buf);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
#endif

    outfile << "-- ============================================\n";
    outfile << "-- Database Table Structure Export\n";
    outfile << "-- Source: " << dbtag << " (" << (src_type == DB_TYPE::POSTGRESQL ? "PostgreSQL" : "MySQL") << ")\n";
    if (target_type != src_type)
    {
        outfile << "-- Target DDL: " << (target_type == DB_TYPE::POSTGRESQL ? "PostgreSQL" : "MySQL") << " (converted)\n";
    }
    outfile << "-- Database: " << src_config.dbname << "\n";
    outfile << "-- Host: " << src_config.host << "\n";
    outfile << "-- Export time: " << time_buf << "\n";
    outfile << "-- Tables: " << table_list.size() << "\n";
    outfile << "-- ============================================\n\n";

    unsigned int success_count = 0;
    unsigned int fail_count    = 0;

    for (size_t ti = 0; ti < table_list.size(); ti++)
    {
        const std::string &table_name = table_list[ti];

        if (!is_valid_sql_identifier(table_name))
        {
            std::cout << " \033[33m[SKIP]\033[0m Invalid table name: '" << table_name << "'" << std::endl;
            fail_count++;
            continue;
        }

        std::cout << "  [" << (ti + 1) << "/" << table_list.size() << "] " << table_name << std::flush;

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
            std::cout << " \033[31m[FAIL]\033[0m" << std::endl;
            fail_count++;
            continue;
        }

        if (target_type != src_type)
        {
            convert_table_for_target(table_info, src_type, target_type);
        }

        std::string ddl = gen_ddl(table_info, target_type);

        outfile << "-- -------------------------------------------\n";
        outfile << "-- Table: " << table_name << "\n";
        if (!table_info.table_comment.empty())
        {
            outfile << "-- Comment: " << table_info.table_comment << "\n";
        }
        outfile << "-- -------------------------------------------\n";
        outfile << ddl << "\n";

        std::cout << " \033[32m[OK]\033[0m" << std::endl;
        success_count++;
    }

    outfile.close();

    std::cout << "\n  ===============================" << std::endl;
    std::cout << "  Export completed!" << std::endl;
    std::cout << "  Success: \033[32m" << success_count << "\033[0m tables" << std::endl;
    if (fail_count > 0)
    {
        std::cout << "  Failed:  \033[31m" << fail_count << "\033[0m tables" << std::endl;
    }
    std::cout << "  Output:  " << filename << std::endl;

    return (fail_count > 0 && success_count == 0) ? 1 : 0;
}

}// namespace dbtable

#endif// _DBTABLE_HPP
