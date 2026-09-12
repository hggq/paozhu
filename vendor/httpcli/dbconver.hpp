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

using dbtypes::build_source_column_expr;
using dbtypes::collect_pk_columns;
using dbtypes::convert_type_for_target;
using dbtypes::create_connection;
using dbtypes::db_table_info;
using dbtypes::db_type_display;
using dbtypes::dedup_global_index_names;
using dbtypes::exec_ddl;
using dbtypes::fetch_rows;
using dbtypes::gen_ddl;
using dbtypes::gen_drop_table;
using dbtypes::get_table_schema;
using dbtypes::get_tables;
using dbtypes::insert_row;
using dbtypes::is_safe_dbtag;
using dbtypes::is_valid_sql_identifier;
using dbtypes::parse_target_type;
using dbtypes::quote_identifier;
using dbtypes::reset_autoincrement;
using dbtypes::row_data_t;
using dbtypes::table_exists;
using dbtypes::validate_database_name;
using orm::DB_TYPE;

// SQLite 数据库文件路径 (host 字段, 回退 dbname)
inline std::string sqlite_conn_path(const orm::orm_conn_t &cfg)
{
    return cfg.host.empty() ? cfg.dbname : cfg.host;
}

// ===================== DDL 生成 (统一走 dbtypes 共用实现) =====================

// include_drop=false: DROP 与 CREATE 必须分开执行。MySQL 握手未启用
// CLIENT_MULTI_STATEMENTS, "DROP;CREATE" 拼一条 COM_QUERY 会被服务端 1064 拒绝。
inline std::string build_create_table(const db_table_info &info, DB_TYPE target_type)
{
    return gen_ddl(info, target_type, false);
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

    bool tables_ok  = false;
    auto table_list = get_tables(src.conn, src_type, &tables_ok);

    if (!tables_ok)
    {
        // 列表查询失败 != 源库没表: 当成成功空跑会让一次迁移静默什么都没做
        std::cerr << "  [ERROR] Failed to list source tables, nothing migrated." << std::endl;
        return 1;
    }

    if (table_list.empty())
    {
        std::cout << "  [INFO] No tables found in source database." << std::endl;
        return 0;
    }

    std::cout << "\n  Found " << table_list.size() << " tables in source." << std::endl;
    std::cout << "  ===============================" << std::endl;

    std::size_t total_success      = 0;
    std::size_t total_fail         = 0;
    std::size_t total_skip         = 0;
    std::size_t total_rows         = 0;
    std::size_t total_row_failures = 0;
    std::size_t batch_size         = 500;

    // ---- 阶段 1: 先把全部源表结构读进来 ----
    // 索引名去重必须是全局视角: PG/SQLite 的索引名在整个 schema 内唯一,
    // 逐表边读边建看不到后面的表, 重名索引会让整表 DDL 失败。
    std::vector<db_table_info> staged;
    staged.reserve(table_list.size());

    for (const auto &table_name : table_list)
    {
        if (!is_valid_sql_identifier(table_name))
        {
            std::cerr << "  \033[31m[FAIL]\033[0m Invalid table name: '" << table_name << "'" << std::endl;
            total_fail++;
            continue;
        }

        db_table_info table_info;
        if (!get_table_schema(src.conn, src_type, table_name, table_info))
        {
            std::cerr << "  \033[31m[FAIL]\033[0m " << table_name << " (read schema failed)" << std::endl;
            total_fail++;
            continue;
        }

        if (src_type != dst_type)
        {
            for (auto &f : table_info.fields)
            {
                f.field_type = convert_type_for_target(
                    f.field_type,
                    f.length,
                    f.is_unsigned,
                    src_type,
                    dst_type,
                    f.decimals);
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
        staged.push_back(std::move(table_info));
    }

    int renamed = dedup_global_index_names(staged, dst_type);
    if (renamed > 0)
    {
        std::cout << "  [INFO] Deduplicated " << renamed << " colliding index name(s)." << std::endl;
    }

    // ---- 阶段 2: 建表 + 搬数据 ----
    for (size_t ti = 0; ti < staged.size(); ti++)
    {
        db_table_info &table_info     = staged[ti];
        const std::string &table_name = table_info.table_name;

        std::cout << "\n  [" << (ti + 1) << "/" << staged.size() << "] " << table_name << std::flush;

        bool tbl_exists = table_exists(dst.conn, dst_type, table_name, dst_config.dbname);

        if (tbl_exists && !force_overwrite)
        {
            std::cout << " \033[33m[SKIP]\033[0m (table already exists in target, use 'force' to overwrite)" << std::endl;
            total_skip++;
            continue;
        }

        std::string ddl_err;
        if (tbl_exists)
        {
            // force: DROP 与 CREATE 分别执行 (MySQL 未启用 CLIENT_MULTI_STATEMENTS)
            // DROP 没成功就不能往下走, 否则会往没清掉的旧表里追加行
            if (!exec_ddl(dst.conn, dst_type, gen_drop_table(table_name, dst_type), ddl_err))
            {
                std::cout << " \033[31m[FAIL]\033[0m DROP TABLE failed: " << ddl_err << std::endl;
                total_fail++;
                continue;
            }
        }
        bool create_ok =
            exec_ddl(dst.conn, dst_type, build_create_table(table_info, dst_type), ddl_err);
        if (!create_ok)
        {
            std::cout << " \033[31m[FAIL]\033[0m DDL error: " << ddl_err << std::endl;
            total_fail++;
            continue;
        }

        std::cout << " \033[33m[CREATED]\033[0m " << std::flush;

        unsigned long long total_count = 0;
        bool count_ok                  = false;
        {
            std::string count_sql = "SELECT COUNT(*) FROM " + quote_identifier(src_type, table_name);
            std::vector<row_data_t> count_rows;
            if (fetch_rows(src.conn, src_type, count_sql, count_rows) &&
                !count_rows.empty() && !count_rows[0].values.empty())
            {
                try
                {
                    total_count = std::stoull(count_rows[0].values[0]);
                    count_ok    = true;
                }
                catch (const std::exception &)
                {
                }
            }
        }
        if (!count_ok)
        {
            // 拿不到源表行数就无法验证写出数, 不能算成功
            std::cout << " \033[31m[FAIL]\033[0m COUNT(*) failed or unparsable" << std::endl;
            total_fail++;
            continue;
        }

        std::vector<std::string> pk_cols = collect_pk_columns(table_info);
        bool can_page                    = !pk_cols.empty();
        if (!can_page && total_count > batch_size)
        {
            // 无 ORDER BY 的 LIMIT/OFFSET 不保证行序稳定, 会重读或漏读
            std::cout << " \033[31m[FAIL]\033[0m no primary key: refusing unstable LIMIT/OFFSET paging over "
                      << total_count << " rows" << std::endl;
            total_fail++;
            continue;
        }

        // 读取列清单与插入列表同源 (table_info.fields), 列数与顺序天然一致
        std::string select_cols;
        for (size_t i = 0; i < table_info.fields.size(); i++)
        {
            if (i > 0)
                select_cols += ", ";
            select_cols += build_source_column_expr(src_type, table_info.fields[i]);
        }

        unsigned long long rows_read    = 0;
        unsigned long long rows_written = 0;
        unsigned long long rows_failed  = 0;
        bool read_failed                = false;

        while (rows_read < total_count)
        {
            std::string select_sql = "SELECT " + select_cols + " FROM " + quote_identifier(src_type, table_name);
            if (can_page)
            {
                select_sql += " ORDER BY ";
                for (size_t i = 0; i < pk_cols.size(); i++)
                {
                    if (i > 0)
                        select_sql += ", ";
                    select_sql += quote_identifier(src_type, pk_cols[i]);
                }
                select_sql += " LIMIT " + std::to_string(batch_size) + " OFFSET " + std::to_string(rows_read);
            }

            std::vector<row_data_t> batch_rows;
            if (!fetch_rows(src.conn, src_type, select_sql, batch_rows))
            {
                read_failed = true;
                std::cerr << "\n  \033[31m[FAIL]\033[0m " << table_name << " batch read failed" << std::flush;
                break;
            }
            if (batch_rows.empty())
                break;// 源表实际行数少于 COUNT(*) (并发写入), 由下面的写出数比对兜住

            bool in_txn = false;
            if (dst_type == DB_TYPE::SQLITE)
            {
                // SQLite 目标: 参数绑定插入 (二进制安全), 每批次包裹事务提速
                auto dst_sqlite = dst.get<orm::sqlite_conn_base>();
                if (dst_sqlite->exec_sql("BEGIN") < 0)
                {
                    std::cerr << "\n  \033[33m[WARN]\033[0m BEGIN failed: " << dst_sqlite->error_msg << std::flush;
                }
                else
                {
                    in_txn = true;
                }
            }

            for (auto &row : batch_rows)
            {
                std::string ins_err;
                if (insert_row(dst.conn, dst_type, table_info.table_name, table_info.fields, row, ins_err))
                    rows_written++;
                else
                {
                    rows_failed++;
                    std::cerr << "\n  \033[31m[WARN]\033[0m INSERT failed: " << ins_err << std::flush;
                }
            }

            if (in_txn)
            {
                auto dst_sqlite = dst.get<orm::sqlite_conn_base>();
                if (dst_sqlite->exec_sql("COMMIT") < 0)
                {
                    // COMMIT 失败 = 整批回滚, 这批不能算写出
                    std::cerr << "\n  \033[31m[FAIL]\033[0m COMMIT failed: " << dst_sqlite->error_msg << std::flush;
                    rows_written = rows_written > batch_rows.size() ? rows_written - batch_rows.size() : 0;
                    rows_failed += batch_rows.size();
                }
            }

            rows_read += batch_rows.size();

            std::cout << "\r  [" << (ti + 1) << "/" << staged.size() << "] " << table_name
                      << " \033[33m[MIGRATING]\033[0m written " << rows_written << "/" << total_count
                      << " rows" << std::flush;
        }

        if (!table_info.auto_inc_field.empty())
        {
            if (!is_valid_sql_identifier(table_info.auto_inc_field))
            {
                std::cout << "\n  \033[33m[WARN]\033[0m Invalid auto increment field name: '"
                          << table_info.auto_inc_field << "', skip reset" << std::flush;
            }
            else
            {
                std::string ai_err;
                if (!reset_autoincrement(dst.conn, dst_type, table_info.table_name, table_info.auto_inc_field, ai_err))
                {
                    std::cout << "\n  \033[33m[WARN]\033[0m Failed to reset auto-increment: " << ai_err << std::flush;
                }
            }
        }

        total_rows += rows_written;
        total_row_failures += rows_failed;

        // 判据是"写出数 == 源表行数", 不是"有没有逐行报错"
        if (!read_failed && rows_written == total_count)
        {
            std::cout << " \033[32m[OK]\033[0m " << rows_written << " rows migrated" << std::endl;
            total_success++;
        }
        else
        {
            std::cout << "\n  \033[31m[FAIL]\033[0m " << table_name << ": written " << rows_written << "/"
                      << total_count << " rows, " << rows_failed << " insert failures" << std::endl;
            total_fail++;
        }
    }

    std::cout << "\n  ===============================" << std::endl;
    std::cout << "  Migration Summary:" << std::endl;
    std::cout << "    Total tables:     " << table_list.size() << std::endl;
    std::cout << "    Success:          \033[32m" << total_success << "\033[0m" << std::endl;
    std::cout << "    Failed:           \033[31m" << total_fail << "\033[0m" << std::endl;
    std::cout << "    Skipped:          " << total_skip << std::endl;
    std::cout << "    Total rows:       " << total_rows << std::endl;
    std::cout << "    Insert failures:  " << total_row_failures << std::endl;
    std::cout << "  ===============================" << std::endl;

    if (total_fail > 0)
    {
        return 1;
    }
    return 0;
}

}// namespace dbconver

#endif// _DBCONVER_HPP
