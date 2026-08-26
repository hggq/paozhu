/*
 *  @author 黄自权 huangziquan
 *  @date 2026-08-14
 *  @dest 导出数据库表结构到SQL文件
 *  Usage: paozhu_cli dbtable <dbtag> <filename.sql> [-target=mysql|postgresql|sqlite]
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
using dbtypes::create_connection;
using dbtypes::db_table_info;
using dbtypes::db_type_display;
using dbtypes::gen_ddl;
using dbtypes::get_tables;
using dbtypes::get_table_schema;
using dbtypes::is_valid_sql_identifier;
using dbtypes::is_safe_dbtag;
using dbtypes::is_safe_filename;
using dbtypes::is_valid_target_type_str;
using dbtypes::parse_target_type;
using orm::DB_TYPE;

// ===================== 主入口 =====================

inline int dbtablecli(const std::string &dbtag = "", const std::string &filename = "", const std::string &target_type_str = "")
{
    if (dbtag.empty() || filename.empty())
    {
        std::cout << "Usage: paozhu_cli dbtable <dbtag> <filename.sql> [-target=mysql|postgresql|sqlite]" << std::endl;
        std::cout << "  dbtag:    database tag (from conf/orm.conf)" << std::endl;
        std::cout << "  filename: output SQL file path" << std::endl;
        std::cout << "  -target:  target database type for DDL conversion (optional)" << std::endl;
        std::cout << "            mysql, postgresql, pg, sqlite, sqlite3" << std::endl;
        std::cout << "Example: paozhu_cli dbtable cms ./schema.sql" << std::endl;
        std::cout << "         (export MySQL [cms] table structures to ./schema.sql)" << std::endl;
        std::cout << "         paozhu_cli dbtable cms ./pg_schema.sql -target=postgresql" << std::endl;
        std::cout << "         (export MySQL [cms] table structures as PostgreSQL DDL)" << std::endl;
        std::cout << "         paozhu_cli dbtable cms ./sqlite_schema.sql -target=sqlite" << std::endl;
        std::cout << "         (export MySQL [cms] table structures as SQLite DDL)" << std::endl;
        return 1;
    }

    // ---- 输入安全验证 ----
    if (!is_safe_dbtag(dbtag))
    {
        return 1;
    }
    if (!is_safe_filename(filename))
    {
        std::cerr << "  [ERROR] Invalid filename: '" << filename << "'" << std::endl;
        std::cerr << "          Path traversal, absolute paths, and dangerous characters are not allowed" << std::endl;
        return 1;
    }
    if (!target_type_str.empty() && !is_valid_target_type_str(target_type_str))
    {
        std::cerr << "  [ERROR] Invalid target type: '" << target_type_str << "'" << std::endl;
        std::cerr << "          Valid types: mysql, postgresql, pg, sqlite, sqlite3" << std::endl;
        return 1;
    }
    // ---- 安全验证完成 ----

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

    DB_TYPE src_type    = parse_target_type(src_config.dbtype);
    DB_TYPE target_type = src_type;
    if (!target_type_str.empty())
    {
        target_type = parse_target_type(target_type_str);
    }

    std::cout << "  Database type: " << db_type_display(src_type) << std::endl;
    if (target_type != src_type)
    {
        std::cout << "  Target DDL:   " << db_type_display(target_type) << " (converted)" << std::endl;
    }
    if (src_type == DB_TYPE::SQLITE)
    {
        std::cout << "  Database file: " << (src_config.host.empty() ? src_config.dbname : src_config.host) << std::endl;
    }
    else
    {
        std::cout << "  Host: " << src_config.host << ":" << src_config.port << std::endl;
        std::cout << "  Database: " << src_config.dbname << std::endl;
    }

    asio::io_context io_context;

    auto src = create_connection(src_config, src_type, io_context);
    if (!src.ok())
    {
        std::cerr << "  [ERROR] " << src.error_msg << std::endl;
        return 1;
    }
    std::cout << "  [OK] " << db_type_display(src_type) << " connected" << std::endl;

    auto table_list = get_tables(src.conn, src_type);

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
    outfile << "-- Source: " << dbtag << " (" << db_type_display(src_type) << ")\n";
    if (target_type != src_type)
    {
        outfile << "-- Target DDL: " << db_type_display(target_type) << " (converted)\n";
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
        bool schema_ok = get_table_schema(src.conn, src_type, table_name, table_info);

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
