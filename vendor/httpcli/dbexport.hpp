/*
 *  @author 黄自权 huangziquan
 *  @date 2026-08-15
 *  @dest 导出数据库表结构+数据到SQL文件 (支持跨数据库类型)
 *  Usage: paozhu_cli dbexport <dbtag> <filename.sql> [-target=mysql|postgresql]
 */

#ifndef _DBEXPORT_HPP
#define _DBEXPORT_HPP

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
#include "dbtable.hpp"

namespace fs = std::filesystem;
namespace dbexport
{

using dbtypes::build_insert_sql_mysql;
using dbtypes::build_insert_sql_pg;
using dbtypes::convert_table_for_target;
using dbtypes::escape_pg_identifier;
using dbtypes::gen_ddl;
using dbtypes::is_valid_sql_identifier;
using dbtypes::mysql_fetch_rows;
using dbtypes::parse_target_type;
using dbtypes::pg_fetch_rows;
using dbtypes::to_lower;
using orm::DB_TYPE;

// ===================== 主导出逻辑 =====================

inline int dbexportcli(const std::string &dbtag = "", const std::string &filename = "", const std::string &target_type_str = "")
{
    if (dbtag.empty() || filename.empty())
    {
        std::cout << "Usage: paozhu_cli dbexport <dbtag> <filename.sql> [-target=mysql|postgresql]" << std::endl;
        std::cout << "  dbtag:    database tag (from conf/orm.conf)" << std::endl;
        std::cout << "  filename: output SQL file path" << std::endl;
        std::cout << "  -target:  target database type for DDL conversion (optional)" << std::endl;
        std::cout << "            mysql, postgresql, pg" << std::endl;
        std::cout << "Example: paozhu_cli dbexport cms ./dump.sql" << std::endl;
        std::cout << "         (export MySQL [cms] schema+data to ./dump.sql)" << std::endl;
        std::cout << "         paozhu_cli dbexport pg ./mysql_dump.sql -target=mysql" << std::endl;
        std::cout << "         (export PG schema+data as MySQL DDL+INSERT)" << std::endl;
        return 1;
    }

    std::cout << "\033[36m 🍄 Database Exporter (Schema + Data)\033[0m" << std::endl;
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
        std::cout << "  \033[32m[OK]\033[0m MySQL connected" << std::endl;
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
        std::cout << "  \033[32m[OK]\033[0m PostgreSQL connected" << std::endl;
    }

    std::vector<std::string> tables;
    if (src_type == DB_TYPE::MYSQL)
    {
        if (!src_mysql)
        {
            std::cerr << "  [ERROR] MySQL connection failed (null)" << std::endl;
            return 1;
        }
        tables = dbtable::get_mysql_tables(src_mysql);
    }
    else
    {
        if (!src_pg)
        {
            std::cerr << "  [ERROR] PostgreSQL connection failed (null)" << std::endl;
            return 1;
        }
        tables = dbtable::get_pg_tables(src_pg);
    }

    if (tables.empty())
    {
        std::cout << "  [INFO] No tables found in database." << std::endl;
        return 0;
    }

    std::cout << "  Tables found: " << tables.size() << std::endl;

    std::ofstream outfile(filename);
    if (!outfile.is_open())
    {
        std::cerr << "  [ERROR] Cannot open output file: " << filename << std::endl;
        return 1;
    }

    outfile << "-- ============================================\n";
    outfile << "-- Database Export\n";
    outfile << "-- Source: " << dbtag << "\n";
    outfile << "-- Database: " << src_config.dbname << "\n";
    outfile << "-- Type: " << (src_type == DB_TYPE::POSTGRESQL ? "PostgreSQL" : "MySQL") << "\n";
    if (target_type != src_type)
    {
        outfile << "-- Target: " << (target_type == DB_TYPE::POSTGRESQL ? "PostgreSQL" : "MySQL") << " (converted)\n";
    }
    outfile << "-- Date: " << __DATE__ << " " << __TIME__ << "\n";
    outfile << "-- ============================================\n\n";

    unsigned int success_count = 0;
    unsigned int fail_count    = 0;

    for (const auto &table_name : tables)
    {
        if (!is_valid_sql_identifier(table_name))
        {
            std::cerr << " \033[33m[SKIP]\033[0m Invalid table name: '" << table_name << "'" << std::endl;
            fail_count++;
            continue;
        }

        std::cout << "  Exporting: " << table_name << " ... ";

        dbtypes::db_table_info table_info;
        bool schema_ok = false;

        if (src_type == DB_TYPE::MYSQL)
        {
            schema_ok = dbtable::mysql_get_table_schema(src_mysql, table_name, table_info);
        }
        else
        {
            schema_ok = dbtable::pg_get_table_schema(src_pg, table_name, table_info);
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

        // Export data in batches
        const unsigned int batch_size = 500;
        unsigned long long offset     = 0;
        unsigned long long exported   = 0;

        while (true)
        {
            std::string data_sql;
            if (src_type == DB_TYPE::MYSQL)
            {
                data_sql = "SELECT * FROM `" + table_name + "` LIMIT " + std::to_string(batch_size) + " OFFSET " + std::to_string(offset);
            }
            else
            {
                data_sql = "SELECT * FROM " + escape_pg_identifier(table_name) + " LIMIT " + std::to_string(batch_size) + " OFFSET " + std::to_string(offset);
            }

            std::vector<dbtypes::row_data_t> batch_rows;
            bool fetch_ok = false;
            if (src_type == DB_TYPE::MYSQL)
            {
                fetch_ok = mysql_fetch_rows(src_mysql, data_sql, batch_rows);
            }
            else
            {
                fetch_ok = pg_fetch_rows(src_pg, data_sql, batch_rows);
            }

            if (!fetch_ok || batch_rows.empty())
                break;

            for (const auto &row : batch_rows)
            {
                if (target_type == DB_TYPE::POSTGRESQL)
                {
                    outfile << build_insert_sql_pg(table_name, table_info.fields, row, 0, table_info.fields.size());
                }
                else
                {
                    outfile << build_insert_sql_mysql(table_name, table_info.fields, row, 0, table_info.fields.size());
                }
                outfile << ";\n";
                exported++;
            }

            offset += batch_rows.size();
            if (batch_rows.size() < batch_size)
                break;
        }

        outfile << "\n";

        std::cout << " \033[32m[OK]\033[0m (" << exported << " rows)" << std::endl;
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

}// namespace dbexport

#endif