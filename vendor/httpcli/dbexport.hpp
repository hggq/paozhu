/*
 *  @author 黄自权 huangziquan
 *  @date 2026-08-15
 *  @dest 导出数据库表结构+数据到SQL文件 (支持跨数据库类型)
 *  Usage: paozhu_cli dbexport <dbtag> <filename.sql> [-target=mysql|postgresql|sqlite]
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

namespace fs = std::filesystem;
namespace dbexport
{

using dbtypes::build_insert_sql_mysql;
using dbtypes::build_insert_sql_pg;
using dbtypes::build_insert_sql_sqlite;
using dbtypes::build_source_column_expr;
using dbtypes::check_row_width;
using dbtypes::convert_table_for_target;
using dbtypes::create_connection;
using dbtypes::db_table_info;
using dbtypes::db_type_display;
using dbtypes::dedup_global_index_names;
using dbtypes::fetch_rows;
using dbtypes::flatten_line_comment;
using dbtypes::gen_ddl;
using dbtypes::get_table_schema;
using dbtypes::get_tables;
using dbtypes::is_safe_dbtag;
using dbtypes::is_safe_filename;
using dbtypes::is_valid_sql_identifier;
using dbtypes::is_valid_target_type_str;
using dbtypes::now_stamp;// 产物头部写真实生成时刻，不用 __DATE__ __TIME__
using dbtypes::parse_target_type;
using dbtypes::quote_identifier;
using orm::DB_TYPE;

// ===================== 主导出逻辑 =====================

inline int dbexportcli(const std::string &dbtag           = "",
                       const std::string &filename        = "",
                       const std::string &target_type_str = "",
                       bool include_drop                  = true)
{
    if (dbtag.empty() || filename.empty())
    {
        std::cout << "Usage: paozhu_cli dbexport <dbtag> <filename.sql> [-target=mysql|postgresql|sqlite] [-nodrop]" << std::endl;
        std::cout << "  dbtag:    database tag (from conf/orm.conf)" << std::endl;
        std::cout << "  filename: output SQL file path" << std::endl;
        std::cout << "  -target:  target database type for DDL conversion (optional)" << std::endl;
        std::cout << "            mysql, postgresql, pg, sqlite, sqlite3" << std::endl;
        std::cout << "  -nodrop:  The product does not output DROP TABLE IF EXISTS (when rolling back to a database with a table of the same name, the table will not be deleted)" << std::endl;
        std::cout << "Example: paozhu_cli dbexport cms ./dump.sql" << std::endl;
        std::cout << "         (export MySQL [cms] schema+data to ./dump.sql)" << std::endl;
        std::cout << "         paozhu_cli dbexport pg ./mysql_dump.sql -target=mysql" << std::endl;
        std::cout << "         (export PG schema+data as MySQL DDL+INSERT)" << std::endl;
        std::cout << "         paozhu_cli dbexport cms ./sqlite_dump.sql -target=sqlite" << std::endl;
        std::cout << "         (export MySQL schema+data as SQLite DDL+INSERT)" << std::endl;
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
        std::cerr << "          Path traversal (..) and dangerous characters are not allowed" << std::endl;
        return 1;
    }
    if (!target_type_str.empty() && !is_valid_target_type_str(target_type_str))
    {
        std::cerr << "  [ERROR] Invalid target type: '" << target_type_str << "'" << std::endl;
        std::cerr << "          Valid types: mysql, postgresql, pg, sqlite, sqlite3" << std::endl;
        return 1;
    }
    // ---- 安全验证完成 ----

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
    std::cout << "  \033[32m[OK]\033[0m " << db_type_display(src_type) << " connected" << std::endl;

    bool tables_ok = false;
    auto tables    = get_tables(src.conn, src_type, &tables_ok);

    if (!tables_ok)
    {
        // "没表"和"列表查询失败"必须分开: 否则失败会伪装成一个合法的空库导出
        std::cerr << "  [ERROR] Failed to list tables, nothing exported." << std::endl;
        return 1;
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
    outfile << "-- Type: " << db_type_display(src_type) << "\n";
    if (target_type != src_type)
    {
        outfile << "-- Target: " << db_type_display(target_type) << " (converted)\n";
    }
    outfile << "-- Date: " << now_stamp() << "\n";
    outfile << "-- Drop: " << (include_drop ? "yes" : "no (-nodrop)") << "\n";
    outfile << "-- ============================================\n\n";

    if (tables.empty())
    {
        // 文件已截断并写上头部: 零表也要覆盖掉上一轮的陈旧产物
        outfile << "-- No tables in database.\n";
        outfile.close();
        std::cout << "  [INFO] No tables found in database." << std::endl;
        return 0;
    }

    unsigned int success_count = 0;
    unsigned int fail_count    = 0;
    std::vector<db_table_info> collected_tables;// collect first, dedup indexes, then write

    // --- Phase 1: collect schemas ---
    for (const auto &table_name : tables)
    {
        std::cout << "  Collecting: " << table_name << " ... " << std::flush;

        // 白名单比三库实际允许的字面量更窄，但保留: 产物中的表名不加引号,
        // 放行奇形表名只会生成回放必炸的 SQL, 所以这里跳过并计入 fail_count。
        // 判定放在进度行之后, 否则 [SKIP] 无法对应到第几张表。
        if (!is_valid_sql_identifier(table_name))
        {
            std::cout << " \033[33m[SKIP]\033[0m invalid table name (not in artifact)" << std::endl;
            fail_count++;
            continue;
        }

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

        collected_tables.push_back(std::move(table_info));
        std::cout << " \033[32m[OK]\033[0m" << std::endl;
    }

    // P1-2 fix: PostgreSQL/SQLite require schema-global unique index names.
    // MySQL only requires per-table uniqueness, so this is a no-op there.
    int renamed = dedup_global_index_names(collected_tables, target_type);
    if (renamed > 0)
    {
        std::cout << "\n  [P1-2] Deduplicated " << renamed << " colliding index name(s) by prepending table name." << std::endl;
    }

    // --- Phase 2: write DDL + data ---
    for (const auto &table_info : collected_tables)
    {
        const std::string &table_name = table_info.table_name;
        std::cout << "  Exporting: " << table_name << " ... ";

        std::string ddl = gen_ddl(table_info, target_type, include_drop);

        outfile << "-- -------------------------------------------\n";
        outfile << "-- Table: " << table_name << "\n";
        if (!table_info.table_comment.empty())
        {
            outfile << "-- Comment: " << flatten_line_comment(table_info.table_comment) << "\n";
        }
        outfile << "-- -------------------------------------------\n";
        outfile << ddl << "\n";

        // Export data in batches
        const unsigned int batch_size = 500;
        unsigned long long offset     = 0;
        unsigned long long exported   = 0;
        bool data_failed              = false;

        // 显式列清单: 与 INSERT 列表同源 (table_info.fields), 生成列 / 隐藏列被过滤后
        // SELECT * 会比 schema 多列, 按位置映射就串列了
        std::string select_cols;
        for (size_t i = 0; i < table_info.fields.size(); i++)
        {
            if (i > 0)
                select_cols += ", ";
            select_cols += build_source_column_expr(src_type, table_info.fields[i]);
        }

        while (true)
        {
            std::string data_sql = "SELECT " + select_cols + " FROM " + quote_identifier(src_type, table_name) +
                                   " LIMIT " + std::to_string(batch_size) + " OFFSET " + std::to_string(offset);

            std::vector<dbtypes::row_data_t> batch_rows;
            bool fetch_ok = fetch_rows(src.conn, src_type, data_sql, batch_rows);

            if (!fetch_ok)
            {
                // 查询出错与"读到末尾"是两件事: 混在一起会把半截数据报成导出成功
                std::cout << " \033[31m[FAIL]\033[0m data query failed after " << exported << " rows" << std::endl;
                outfile << "-- !! INTERRUPTED: data export aborted for table " << table_name << " after " << exported
                        << " rows; the INSERTs above are a PARTIAL dump !!\n";
                fail_count++;
                data_failed = true;
                break;
            }
            if (batch_rows.empty())
                break;

            for (const auto &row : batch_rows)
            {
                // 直接调 builder 会绕过 insert_row 里的行宽校验, 列数不符时 builder 静默截断
                std::string width_err;
                if (!check_row_width(table_info.fields, row, table_name, width_err))
                {
                    std::cout << " \033[31m[FAIL]\033[0m " << width_err << std::endl;
                    outfile << "-- !! INTERRUPTED: " << width_err
                            << ", the INSERTs above are a PARTIAL dump !!\n";
                    fail_count++;
                    data_failed = true;
                    break;
                }

                if (target_type == DB_TYPE::POSTGRESQL)
                {
                    outfile << build_insert_sql_pg(table_name, table_info.fields, row, 0, table_info.fields.size());
                }
                else if (target_type == DB_TYPE::SQLITE)
                {
                    // SQLite 目标: 文本 INSERT (BLOB 列输出 X'十六进制' 字面量)
                    outfile << build_insert_sql_sqlite(table_name, table_info.fields, row, 0, table_info.fields.size());
                }
                else
                {
                    outfile << build_insert_sql_mysql(table_name, table_info.fields, row, 0, table_info.fields.size());
                }
                outfile << ";\n";
                exported++;
            }

            if (data_failed)
                break;

            offset += batch_rows.size();
            if (batch_rows.size() < batch_size)
                break;
        }

        outfile << "\n";

        if (data_failed)
        {
            continue;// 上面已打 [FAIL] 并计入 fail_count，这里不能再报成功
        }

        std::cout << " \033[32m[OK]\033[0m (" << exported << " rows)" << std::endl;
        success_count++;
    }

    // 写失败(磁盘满/只读目录等)不能被静默吞掉: 产物残缺却报成功
    outfile.flush();
    if (!outfile.good())
    {
        std::cerr << "  [ERROR] Write failed on " << filename << std::endl;
        outfile.close();
        return 1;
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

    return fail_count > 0 ? 1 : 0;
}

}// namespace dbexport

#endif