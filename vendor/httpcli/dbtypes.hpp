/*
 *  @author 黄自权 huangziquan
 *  @date 2026-08-14
 *  @dest 数据库类型定义和工具函数 - MySQL & PostgreSQL & SQLite 共用
 */

#ifndef _DBTYPES_HPP
#define _DBTYPES_HPP

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
#include <cstdlib>
#include <cerrno>
#include <variant>
#include "mysql_conn.h"
#include "pg_conn.h"
#include "sqlite_conn.h"
#include "orm_conn_pool.h"
#include "orm_common.h"

namespace dbtypes
{

using orm::DB_TYPE;

// ===================== 工具函数 =====================

inline std::string to_lower(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) -> char
                   { return ::tolower(c); });
    return result;
}

inline std::string trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

inline std::string remove_quotes(const std::string &str)
{
    if (str.size() >= 2)
    {
        char front = str.front(), back = str.back();
        if ((front == '`' && back == '`') ||
            (front == '"' && back == '"') ||
            (front == '\'' && back == '\''))
        {
            return str.substr(1, str.size() - 2);
        }
    }
    return str;
}

inline bool starts_with_icase(const std::string &str, const std::string &prefix)
{
    if (str.size() < prefix.size())
        return false;
    std::string str_lower = str.substr(0, prefix.size());
    std::transform(str_lower.begin(), str_lower.end(), str_lower.begin(), [](unsigned char c) -> char
                   { return ::tolower(c); });
    std::string prefix_lower = prefix;
    std::transform(prefix_lower.begin(), prefix_lower.end(), prefix_lower.begin(), [](unsigned char c) -> char
                   { return ::tolower(c); });
    return str_lower == prefix_lower;
}

inline std::string escape_mysql_string(const std::string &val)
{
    std::string result;
    result.reserve(val.size() + 2);
    for (char c : val)
    {
        switch (c)
        {
        case '\\': result += "\\\\"; break;
        case '\'': result += "\\'"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\0': result += "\\0"; break;
        default: result += c; break;
        }
    }
    return result;
}

inline std::string escape_pg_string(const std::string &val)
{
    std::string result;
    result.reserve(val.size() + 2);
    for (char c : val)
    {
        switch (c)
        {
        case '\\': result += "\\\\"; break;
        case '\'': result += "''"; break;
        case '\0': result += "\\000"; break;
        default: result += c; break;
        }
    }
    return result;
}

inline bool is_valid_sql_identifier(const std::string &name)
{
    if (name.empty())
        return false;
    if (name.size() > 64)
        return false;
    for (char c : name)
    {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
        {
            return false;
        }
    }
    return true;
}

inline bool validate_table_name(const std::string &table_name)
{
    if (!is_valid_sql_identifier(table_name))
    {
        std::cerr << "  [ERROR] Invalid table name: '" << table_name << "'" << std::endl;
        return false;
    }
    return true;
}

inline bool validate_database_name(const std::string &db_name)
{
    if (!is_valid_sql_identifier(db_name))
    {
        std::cerr << "  [ERROR] Invalid database name: '" << db_name << "'" << std::endl;
        return false;
    }
    return true;
}

// 验证 CLI 输入的 filename 安全性
inline bool is_safe_filename(const std::string &filename)
{
    if (filename.empty())
        return false;
    if (filename.size() > 1024)
        return false;
    // 禁止路径遍历
    if (filename.find("..") != std::string::npos)
        return false;
    // 禁止绝对路径
    if (filename.front() == '/' || filename.front() == '\\')
        return false;
    // 禁止 SQL / shell 危险字符
    for (char c : filename)
    {
        if (c == '\'' || c == '"' || c == ';' || c == '|' ||
            c == '`' || c == '$' || c == '&' || c == '!' ||
            c == '(' || c == ')' || c == '{' || c == '}')
        {
            return false;
        }
    }
    return true;
}

// 验证 CLI 输入的 dbtag 格式
inline bool is_safe_dbtag(const std::string &dbtag)
{
    if (!is_valid_sql_identifier(dbtag))
    {
        std::cerr << "  [ERROR] Invalid dbtag format: '" << dbtag << "'" << std::endl;
        std::cerr << "          Only alphanumeric characters and underscore are allowed" << std::endl;
        return false;
    }
    return true;
}

// 验证 CLI 输入的 target type 字符串
inline bool is_valid_target_type_str(const std::string &target_str)
{
    std::string lower = to_lower(target_str);
    if (lower == "mysql" || lower == "postgresql" || lower == "pg" || lower == "postgres" ||
        lower == "sqlite" || lower == "sqlite3" || lower == "sq3")
    {
        return true;
    }
    return false;
}

inline bool is_valid_mysql_engine(const std::string &engine)
{
    static const std::vector<std::string> whitelist = {
        "InnoDB",
        "MyISAM",
        "MEMORY",
        "CSV",
        "Merge",
        "Archive",
        "Blackhole",
        "Federated",
        "Example",
        "PARTITIONED",
        "TokuDB",
        "RocksDB",
        "MyRocks"};
    for (const auto &e : whitelist)
    {
        if (to_lower(engine) == to_lower(e))
            return true;
    }
    return false;
}

inline bool is_valid_mysql_charset(const std::string &charset)
{
    static const std::vector<std::string> whitelist = {
        "utf8mb4",
        "utf8mb3",
        "utf8",
        "utf16",
        "utf16le",
        "utf32",
        "ascii",
        "binary",
        "latin1",
        "latin2",
        "latin5",
        "latin7",
        "cp1252",
        "cp1256",
        "cp1259",
        "cp866",
        "cp923",
        "euckr",
        "eucjpms",
        "gbk",
        "gb18030",
        "big5",
        "sjis",
        "ujis",
        "euckjp",
        "latin1_swedish_ci",
        "utf8mb4_general_ci",
        "utf8mb4_unicode_ci",
        "utf8mb4_bin",
        "utf8mb4_0900_ai_ci"};
    for (const auto &c : whitelist)
    {
        if (to_lower(charset) == to_lower(c))
            return true;
    }
    return false;
}

inline std::string escape_pg_identifier(const std::string &identifier)
{
    std::string result;
    result.reserve(identifier.size() + 2);
    result.push_back('"');
    for (char c : identifier)
    {
        if (c == '"')
            result += "\"\"";
        else
            result += c;
    }
    result.push_back('"');
    return result;
}

// SQLite 字符串字面量: 唯一合法转义是单引号翻倍 (不识别反斜杠转义,
// 若用 MySQL 式 \\ / \n 转义会把数据写坏)
inline std::string escape_sqlite_string(const std::string &val)
{
    std::string result;
    result.reserve(val.size());
    for (char c : val)
    {
        if (c == '\'')
            result += "''";
        else
            result += c;
    }
    return result;
}

// SQLite 标识符: 双引号包裹, 规则与 PG 一致
inline std::string escape_sqlite_identifier(const std::string &identifier)
{
    return escape_pg_identifier(identifier);
}

// BLOB 二进制 → SQLite X'十六进制' 字面量 (供文本形式 INSERT 导出使用)
inline std::string bytes_to_hex_sqlite(const std::string &data)
{
    static const char *hex_digits = "0123456789ABCDEF";
    std::string out;
    out.reserve(3 + data.size() * 2);
    out += "X'";
    for (unsigned char c : data)
    {
        out.push_back(hex_digits[c >> 4]);
        out.push_back(hex_digits[c & 0x0F]);
    }
    out.push_back('\'');
    return out;
}

// ===================== 表结构定义 =====================

struct db_field_info
{
    std::string field_name;
    std::string field_type;
    unsigned int length    = 0;
    unsigned char decimals = 0;
    bool is_unsigned       = false;
    bool is_nullable       = true;
    std::string default_value;
    std::string comment;
    bool is_auto_inc         = false;
    bool is_pk               = false;
    unsigned char mysql_type = 0;
};

struct db_index_info
{
    std::string index_name;
    std::vector<std::string> columns;
    bool is_unique  = false;
    bool is_primary = false;
};

struct db_table_info
{
    std::string table_name;
    std::string engine;
    std::string charset;
    std::string collation;
    std::string table_comment;
    std::string pk_name;
    std::string auto_inc_field;
    std::vector<db_field_info> fields;
    std::vector<db_index_info> indexes;
    DB_TYPE source_db_type = DB_TYPE::MYSQL;
};

struct row_data_t
{
    std::vector<std::string> values;
    std::vector<bool> is_null;
};

// ===================== 类型映射 =====================
// 本层: SQL 类型字符串映射 (std::string → std::string), 用于 DDL 生成和跨库迁移
// modelfun.hpp 层: MySQL 协议类型码映射 (unsigned char → unsigned char), 用于 ORM 代码生成
// 两套映射层次不同, 不合并: dbtypes.hpp 操作 SQL 类型名 (如 "varchar(255)" → "text"),
// modelfun.hpp 操作 MySQL 线协议类型码 (如 MYSQL_TYPE_VARCHAR → MYSQL_TYPE_VAR_STRING)

inline std::string mysql_type_to_pg(const std::string &mysql_type, unsigned int length, bool is_unsigned)
{
    std::string type_lower = to_lower(mysql_type);

    if (type_lower == "tinyint")
    {
        if (length == 1)
            return "boolean";
        return "smallint";
    }
    if (type_lower == "smallint")
        return "smallint";
    if (type_lower == "mediumint")
        return "integer";
    if (type_lower == "int" || type_lower == "integer")
        return "integer";
    if (type_lower == "bigint")
    {
        if (is_unsigned)
        {
            std::cout << "  [WARN] UNSIGNED BIGINT mapped to PG BIGINT (may overflow, consider NUMERIC(20))" << std::endl;
        }
        return "bigint";
    }
    if (type_lower == "float")
        return "real";
    if (type_lower == "double")
        return "double precision";
    if (type_lower == "decimal" || type_lower == "numeric")
    {
        if (length > 0)
        {
            return "numeric(" + std::to_string(length) + ")";
        }
        return "numeric";
    }
    if (type_lower == "char")
    {
        return "char(" + std::to_string(length > 0 ? length : 1) + ")";
    }
    if (type_lower == "varchar")
    {
        return "varchar(" + std::to_string(length > 0 ? length : 255) + ")";
    }
    if (type_lower == "tinytext" || type_lower == "text" ||
        type_lower == "mediumtext" || type_lower == "longtext")
    {
        return "text";
    }
    if (type_lower == "blob" || type_lower == "mediumblob" ||
        type_lower == "longblob" || type_lower == "tinyblob")
    {
        return "bytea";
    }
    if (type_lower == "date")
        return "date";
    if (type_lower == "time")
        return "time";
    if (type_lower == "datetime")
        return "timestamp";
    if (type_lower == "timestamp")
        return "timestamp";
    if (type_lower == "year")
        return "smallint";
    if (type_lower == "json")
        return "text";
    if (type_lower == "enum" || type_lower == "set")
    {
        std::cout << "  [WARN] ENUM/SET type mapped to TEXT" << std::endl;
        return "text";
    }
    return type_lower;
}

inline std::string pg_type_to_mysql(const std::string &pg_type, unsigned int length)
{
    std::string type_lower = to_lower(pg_type);

    if (type_lower == "smallint" || type_lower == "int2" || type_lower == "smallserial")
        return "smallint";
    if (type_lower == "integer" || type_lower == "int4" || type_lower == "serial")
        return "int";
    if (type_lower == "bigint" || type_lower == "int8" || type_lower == "bigserial")
        return "bigint";
    if (type_lower == "real" || type_lower == "float4")
        return "float";
    if (type_lower == "double precision" || type_lower == "float8")
        return "double";
    if (type_lower == "numeric" || type_lower == "decimal")
    {
        if (length > 0)
        {
            return "decimal(" + std::to_string(length) + ")";
        }
        return "decimal";
    }
    if (type_lower == "character varying" || type_lower == "varchar")
    {
        return "varchar(" + std::to_string(length > 0 ? length : 255) + ")";
    }
    if (type_lower == "character" || type_lower == "char")
    {
        return "char(" + std::to_string(length > 0 ? length : 1) + ")";
    }
    if (type_lower == "text")
        return "longtext";
    if (type_lower == "bytea")
        return "longblob";
    if (type_lower == "date")
        return "date";
    if (type_lower == "time" || type_lower == "time without time zone")
        return "time";
    if (type_lower == "time with time zone" || type_lower == "timetz")
        return "time";
    if (type_lower == "timestamp" || type_lower == "timestamp without time zone")
        return "datetime";
    if (type_lower == "timestamp with time zone" || type_lower == "timestamptz")
        return "datetime";
    if (type_lower == "boolean" || type_lower == "bool")
        return "tinyint(1)";
    if (type_lower == "uuid")
        return "char(36)";
    if (type_lower == "json" || type_lower == "jsonb")
        return "longtext";

    size_t bracket_pos = type_lower.find('(');
    if (bracket_pos != std::string::npos)
    {
        return pg_type;
    }

    return pg_type;
}

// ===================== SQLite 类型映射 (保留原始声明策略) =====================
// SQLite 接受任意类型名 (按亲和性存储), 因此默认原样保留源库声明,
// 仅对 SQLite 语义明显不合适的个别写法做归一, 以支持高保真反向转换

// 源库类型 → SQLite 声明类型
inline std::string sqlite_decl_type_for(const std::string &src_type, unsigned int length, unsigned char decimals, DB_TYPE src_db_type)
{
    std::string tl = to_lower(trim(src_type));
    if (tl.empty())
        return "TEXT";
    if (tl == "enum" || tl == "set" || tl == "jsonb" || tl == "uuid" || tl == "money")
        return "TEXT";
    if (tl == "serial" || tl == "smallserial" || tl == "bigserial")
        return "INTEGER";
    if (tl == "bytea")
        return "BLOB";
    (void)src_db_type;

    // 原始声明已带参数 (如 VARCHAR(255)/DECIMAL(10,2)) 则原样保留
    if (src_type.find('(') != std::string::npos)
        return src_type;

    // 基类型名 + length/decimals 分离形式 (MySQL schema 解析风格):
    // 字符串类/数值类附加长度参数, 其余直接保留类型名 (大写归一)
    std::string base = src_type;
    for (auto &c : base)
        c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));

    if (tl == "varchar" || tl == "nvarchar" || tl == "char" || tl == "character varying" || tl == "character")
    {
        return base + "(" + std::to_string(length > 0 ? length : 255) + ")";
    }
    if (tl == "decimal" || tl == "numeric")
    {
        if (length > 0)
        {
            if (decimals > 0)
                return base + "(" + std::to_string(length) + "," + std::to_string(static_cast<unsigned int>(decimals)) + ")";
            return base + "(" + std::to_string(length) + ")";
        }
    }
    return base;
}

// SQLite 声明类型 → MySQL 类型 (亲和性规则 + 原始声明识别)
inline std::string sqlite_type_to_mysql(const std::string &sqlite_type)
{
    std::string tl = to_lower(trim(sqlite_type));
    if (tl.empty())
        return "text";

    if (tl == "integer" || tl == "int8")
        return "bigint";
    if (tl == "int" || tl == "int4" || tl == "mediumint")
        return "int";
    if (tl == "smallint" || tl == "int2")
        return "smallint";
    if (tl == "tinyint")
        return "tinyint";
    if (tl == "boolean" || tl == "bool")
        return "tinyint(1)";
    if (tl == "real" || tl == "float" || tl == "float4")
        return "float";
    if (tl == "double" || tl == "double precision" || tl == "float8")
        return "double";
    if (tl == "text" || tl == "clob")
        return "text";
    if (tl == "blob")
        return "longblob";
    if (tl == "date")
        return "date";
    if (tl == "time")
        return "time";
    if (tl == "datetime" || tl == "timestamp")
        return "datetime";

    // 带参数声明 (varchar(255)/decimal(10,2) 等) 原样透传
    if (starts_with_icase(tl, "varchar") || starts_with_icase(tl, "nvarchar") ||
        starts_with_icase(tl, "char") || starts_with_icase(tl, "decimal") ||
        starts_with_icase(tl, "numeric"))
        return tl;

    // 亲和性规则 (与 SQLite 官方亲和性判定顺序一致)
    if (tl.find("int") != std::string::npos)
        return "bigint";
    if (tl.find("char") != std::string::npos || tl.find("clob") != std::string::npos ||
        tl.find("text") != std::string::npos)
        return "text";
    if (tl.find("blob") != std::string::npos)
        return "longblob";
    if (tl.find("real") != std::string::npos || tl.find("floa") != std::string::npos ||
        tl.find("doub") != std::string::npos)
        return "double";
    return "text";// NUMERIC 亲和性
}

// SQLite 声明类型 → PostgreSQL 类型
inline std::string sqlite_type_to_pg(const std::string &sqlite_type)
{
    std::string tl = to_lower(trim(sqlite_type));
    if (tl.empty())
        return "text";

    if (tl == "integer" || tl == "int" || tl == "int4" || tl == "mediumint")
        return "integer";
    if (tl == "bigint" || tl == "int8")
        return "bigint";
    if (tl == "smallint" || tl == "int2" || tl == "tinyint")
        return "smallint";
    if (tl == "boolean" || tl == "bool")
        return "boolean";
    if (tl == "real" || tl == "float" || tl == "float4")
        return "real";
    if (tl == "double" || tl == "double precision" || tl == "float8")
        return "double precision";
    if (tl == "text" || tl == "clob")
        return "text";
    if (tl == "blob")
        return "bytea";
    if (tl == "date")
        return "date";
    if (tl == "time")
        return "time";
    if (tl == "datetime" || tl == "timestamp")
        return "timestamp";

    if (starts_with_icase(tl, "varchar") || starts_with_icase(tl, "nvarchar") ||
        starts_with_icase(tl, "char") || starts_with_icase(tl, "decimal") ||
        starts_with_icase(tl, "numeric"))
        return tl;

    if (tl.find("int") != std::string::npos)
        return "integer";
    if (tl.find("char") != std::string::npos || tl.find("clob") != std::string::npos ||
        tl.find("text") != std::string::npos)
        return "text";
    if (tl.find("blob") != std::string::npos)
        return "bytea";
    if (tl.find("real") != std::string::npos || tl.find("floa") != std::string::npos ||
        tl.find("doub") != std::string::npos)
        return "double precision";
    return "text";// NUMERIC 亲和性
}

inline std::string convert_type_for_target(const std::string &source_type,
                                           unsigned int length,
                                           bool is_unsigned,
                                           DB_TYPE src_type,
                                           DB_TYPE target_type)
{
    if (src_type == DB_TYPE::SQLITE)
    {
        if (target_type == DB_TYPE::POSTGRESQL)
            return sqlite_type_to_pg(source_type);
        if (target_type == DB_TYPE::MYSQL)
            return sqlite_type_to_mysql(source_type);
        return source_type;
    }
    if (target_type == DB_TYPE::SQLITE)
    {
        return sqlite_decl_type_for(source_type, length, 0, src_type);
    }
    if (target_type == DB_TYPE::POSTGRESQL)
    {
        return mysql_type_to_pg(source_type, length, is_unsigned);
    }
    return pg_type_to_mysql(source_type, length);
}

inline DB_TYPE parse_target_type(const std::string &target_str)
{
    std::string lower = to_lower(target_str);
    if (lower == "postgresql" || lower == "pg" || lower == "postgres")
    {
        return DB_TYPE::POSTGRESQL;
    }
    if (lower == "sqlite" || lower == "sqlite3" || lower == "sq3")
    {
        return DB_TYPE::SQLITE;
    }
    return DB_TYPE::MYSQL;
}

// ===================== MySQL 类型码转 PostgreSQL 类型字符串 =====================
// 用于将 modelfun.hpp 中 pg_get_column_info 获取的 col_type (MySQL 协议类型码) 转换为 PG 类型
inline std::string mysql_col_type_to_pg_type(unsigned char col_type, unsigned int length, [[maybe_unused]] bool is_unsigned)
{
    // MySQL 协议类型码定义参考:
    // 0x01=TINYINT, 0x02=SMALLINT, 0x03=INT, 0x08=BIGINT, 0x09=MEDIUMINT
    // 0x04=FLOAT, 0x05=DOUBLE, 0xF6=DECIMAL/NUMERIC
    // 0xFE=CHAR/VARCHAR/TEXT等, 0xFD=VARCHAR, 0xFC=TEXT/BLOB
    // 0x0A=DATE, 0x0B=TIME, 0x0C=DATETIME/TIMESTAMP, 0x07=TIMESTAMP(old)
    // 0xF5=JSON

    switch (col_type)
    {
    case 0x01:// TINYINT
        if (length == 1)
            return "boolean";
        return "smallint";
    case 0x02:// SMALLINT
        return "smallint";
    case 0x03:// INT
        return "integer";
    case 0x08:// BIGINT
        return "bigint";
    case 0x09:// MEDIUMINT
        return "integer";
    case 0x04:// FLOAT
        return "real";
    case 0x05:// DOUBLE
        return "double precision";
    case 0xF6:// DECIMAL/NUMERIC
        if (length > 0)
            return "numeric(" + std::to_string(length) + ")";
        return "numeric";
    case 0xFE:// CHAR/VARCHAR/ENUM/SET etc.
        if (length > 0 && length < 256)
            return "varchar(" + std::to_string(length) + ")";
        return "text";
    case 0xFD:// VARCHAR
        if (length > 0)
            return "varchar(" + std::to_string(length) + ")";
        return "varchar(255)";
    case 0xFC:// TEXT/BLOB
        return "text";
    case 0x0A:// DATE
        return "date";
    case 0x0B:// TIME
        return "time";
    case 0x0C:// DATETIME/TIMESTAMP
        return "timestamp";
    case 0x07:// TIMESTAMP (old)
        return "timestamp";
    case 0xF5:// JSON
        return "jsonb";
    default:
        return "text";
    }
}

inline void convert_table_for_target(db_table_info &table, DB_TYPE src_type, DB_TYPE target_type)
{
    if (src_type == target_type)
        return;

    for (auto &f : table.fields)
    {
        if (src_type == DB_TYPE::MYSQL && target_type == DB_TYPE::POSTGRESQL)
        {
            f.field_type = mysql_type_to_pg(f.field_type, f.length, f.is_unsigned);
            if (f.is_auto_inc && f.is_pk)
            {
                std::string type_lower = to_lower(f.field_type);
                if (type_lower == "integer" || type_lower == "int")
                {
                    f.field_type = "SERIAL";
                }
                else if (type_lower == "bigint")
                {
                    f.field_type = "BIGSERIAL";
                }
                f.is_auto_inc = false;
            }
        }
        else if (src_type == DB_TYPE::POSTGRESQL && target_type == DB_TYPE::MYSQL)
        {
            f.field_type = pg_type_to_mysql(f.field_type, f.length);
        }
        else if (target_type == DB_TYPE::SQLITE)
        {
            // 保留原始声明 (长度/精度拼回声明), 仅归一 SQLite 不接受的写法;
            // 声明已自带参数时 f.length/f.decimals 清零避免 DDL 阶段重复追加
            f.field_type = sqlite_decl_type_for(f.field_type, f.length, f.decimals, src_type);
            f.length     = 0;
            f.decimals   = 0;
        }
        else if (src_type == DB_TYPE::SQLITE && target_type == DB_TYPE::MYSQL)
        {
            // SQLite 源 field_type 可能带参数声明 (如 varchar(255)), 转换结果自带参数, 清零避免重复追加
            f.field_type = sqlite_type_to_mysql(f.field_type);
            f.length     = 0;
            f.decimals   = 0;
        }
        else if (src_type == DB_TYPE::SQLITE && target_type == DB_TYPE::POSTGRESQL)
        {
            f.field_type = sqlite_type_to_pg(f.field_type);
            f.length     = 0;
            f.decimals   = 0;
        }
        f.is_unsigned = false;
    }

    table.source_db_type = target_type;
}

// ===================== MySQL SHOW CREATE TABLE 解析 =====================

inline bool parse_mysql_show_create(const std::string &show_create_sql, db_table_info &table)
{
    size_t create_pos = show_create_sql.find("CREATE TABLE");
    if (create_pos == std::string::npos)
        return false;

    size_t open_brace = show_create_sql.find('(', create_pos);
    if (open_brace == std::string::npos)
        return false;

    // 从 open_brace 开始扫描匹配的闭合括号，避免表 COMMENT 中的 ')' 干扰 rfind
    size_t close_brace = std::string::npos;
    {
        int depth       = 0;
        bool in_string  = false;
        char quote_char = '\0';
        for (size_t i = open_brace; i < show_create_sql.size(); i++)
        {
            char c = show_create_sql[i];
            if (in_string)
            {
                if (c == '\\' && i + 1 < show_create_sql.size())
                {
                    i++;
                    continue;
                }
                if (c == quote_char)
                    in_string = false;
            }
            else if (c == '\'' || c == '"' || c == '`')
            {
                in_string  = true;
                quote_char = c;
            }
            else if (c == '(')
            {
                depth++;
            }
            else if (c == ')')
            {
                if (depth > 0)
                    depth--;
                if (depth == 0)
                {
                    close_brace = i;
                    break;
                }
            }
        }
    }
    if (close_brace == std::string::npos)
        return false;

    std::string table_name_part = show_create_sql.substr(create_pos + 12, open_brace - create_pos - 12);
    table.table_name            = remove_quotes(trim(table_name_part));

    std::string fields_part = show_create_sql.substr(open_brace + 1, close_brace - open_brace - 1);

    size_t pos = 0;
    while (pos < fields_part.size())
    {
        while (pos < fields_part.size() &&
               (fields_part[pos] == ' ' || fields_part[pos] == '\t' ||
                fields_part[pos] == '\n' || fields_part[pos] == '\r'))
        {
            pos++;
        }
        if (pos >= fields_part.size())
            break;

        size_t field_start = pos;
        size_t paren_count = 0;
        bool in_string     = false;
        char quote_char    = '\0';

        while (pos < fields_part.size())
        {
            char c = fields_part[pos];
            if (in_string && c == '\\' && pos + 1 < fields_part.size())
            {
                pos += 2;
                continue;
            }
            if (c == '\'' || c == '"')
            {
                if (!in_string || quote_char == c)
                {
                    in_string  = !in_string;
                    quote_char = in_string ? c : '\0';
                }
            }
            else if (!in_string)
            {
                if (c == '(')
                    paren_count++;
                else if (c == ')')
                {
                    if (paren_count > 0)
                        paren_count--;
                }
                else if (c == ',' && paren_count == 0)
                    break;
            }
            pos++;
        }

        std::string field_def = trim(fields_part.substr(field_start, pos - field_start));
        if (field_def.empty())
        {
            pos++;
            continue;
        }

        if (starts_with_icase(field_def, "PRIMARY KEY"))
        {
            size_t pk_open  = field_def.find('(');
            size_t pk_close = field_def.find(')');
            if (pk_open != std::string::npos && pk_close != std::string::npos)
            {
                std::string pk_cols = trim(field_def.substr(pk_open + 1, pk_close - pk_open - 1));
                table.pk_name       = remove_quotes(pk_cols);
                for (auto &f : table.fields)
                {
                    if (f.field_name == table.pk_name)
                        f.is_pk = true;
                }
            }
            pos++;
            continue;
        }

        if (starts_with_icase(field_def, "UNIQUE KEY") ||
            starts_with_icase(field_def, "UNIQUE INDEX"))
        {
            db_index_info idx;
            idx.is_unique  = true;
            idx.is_primary = false;

            size_t name_end = field_def.find_first_of(" \t(");
            if (name_end != std::string::npos)
            {
                std::string idx_part = field_def.substr(0, name_end);
                size_t space_pos     = idx_part.find_first_of(" \t");
                if (space_pos != std::string::npos)
                {
                    idx.index_name = remove_quotes(trim(idx_part.substr(space_pos)));
                }
            }

            size_t paren_open  = field_def.find('(');
            size_t paren_close = field_def.find(')');
            if (paren_open != std::string::npos && paren_close != std::string::npos)
            {
                std::string cols_str = field_def.substr(paren_open + 1, paren_close - paren_open - 1);
                std::istringstream cols_iss(cols_str);
                std::string col_token;
                while (std::getline(cols_iss, col_token, ','))
                {
                    idx.columns.push_back(remove_quotes(trim(col_token)));
                }
            }
            if (!idx.columns.empty())
                table.indexes.push_back(idx);
            pos++;
            continue;
        }

        if (starts_with_icase(field_def, "KEY ") ||
            starts_with_icase(field_def, "INDEX "))
        {
            db_index_info idx;
            idx.is_unique  = false;
            idx.is_primary = false;

            size_t name_end = field_def.find_first_of(" \t(");
            if (name_end != std::string::npos)
            {
                std::string idx_part = field_def.substr(0, name_end);
                size_t space_pos     = idx_part.find_first_of(" \t");
                if (space_pos != std::string::npos)
                {
                    idx.index_name = remove_quotes(trim(idx_part.substr(space_pos)));
                }
            }

            size_t paren_open  = field_def.find('(');
            size_t paren_close = field_def.find(')');
            if (paren_open != std::string::npos && paren_close != std::string::npos)
            {
                std::string cols_str = field_def.substr(paren_open + 1, paren_close - paren_open - 1);
                std::istringstream cols_iss(cols_str);
                std::string col_token;
                while (std::getline(cols_iss, col_token, ','))
                {
                    idx.columns.push_back(remove_quotes(trim(col_token)));
                }
            }
            if (!idx.columns.empty())
                table.indexes.push_back(idx);
            pos++;
            continue;
        }

        if (starts_with_icase(field_def, "CONSTRAINT") ||
            starts_with_icase(field_def, "FOREIGN KEY") ||
            starts_with_icase(field_def, "FULLTEXT") ||
            starts_with_icase(field_def, "SPATIAL") ||
            starts_with_icase(field_def, "CHECK"))
        {
            pos++;
            continue;
        }

        db_field_info field;

        size_t name_end = field_def.find_first_of(" \t");
        if (name_end == std::string::npos)
        {
            pos++;
            continue;
        }
        field.field_name = remove_quotes(trim(field_def.substr(0, name_end)));

        size_t type_start = name_end;
        while (type_start < field_def.size() &&
               (field_def[type_start] == ' ' || field_def[type_start] == '\t'))
        {
            type_start++;
        }

        paren_count     = 0;
        in_string       = false;
        quote_char      = '\0';
        size_t type_end = type_start;

        while (type_end < field_def.size())
        {
            char c = field_def[type_end];
            if (c == '\'' || c == '"')
            {
                if (!in_string || quote_char == c)
                {
                    in_string  = !in_string;
                    quote_char = in_string ? c : '\0';
                }
            }
            else if (!in_string)
            {
                if (c == '(')
                    paren_count++;
                else if (c == ')')
                {
                    if (paren_count > 0)
                        paren_count--;
                    if (paren_count == 0)
                    {
                        type_end++;
                        break;
                    }
                }
                else if ((c == ' ' || c == '\t') && paren_count == 0)
                    break;
            }
            type_end++;
        }

        std::string type_str = trim(field_def.substr(type_start, type_end - type_start));

        size_t paren_start = type_str.find('(');
        size_t paren_end   = type_str.find(')');
        if (paren_start != std::string::npos && paren_end != std::string::npos)
        {
            field.field_type   = type_str.substr(0, paren_start);
            std::string params = type_str.substr(paren_start + 1, paren_end - paren_start - 1);
            size_t comma_pos   = params.find(',');
            if (comma_pos != std::string::npos)
            {
                try
                {
                    unsigned long len = std::stoul(params.substr(0, comma_pos));
                    unsigned long dec = std::stoul(params.substr(comma_pos + 1));
                    if (len <= 0xFFFFFFFF)
                    {
                        field.length = static_cast<unsigned int>(len);
                    }
                    if (dec <= 0xFF)
                    {
                        field.decimals = static_cast<unsigned char>(dec);
                    }
                }
                catch (...)
                {
                }
            }
            else
            {
                try
                {
                    unsigned long len = std::stoul(params);
                    if (len <= 0xFFFFFFFF)
                    {
                        field.length = static_cast<unsigned int>(len);
                    }
                }
                catch (...)
                {
                }
            }
        }
        else
        {
            field.field_type = type_str;
        }

        std::string type_lower = to_lower(field.field_type);

        size_t attr_start = type_end;
        while (attr_start < field_def.size() &&
               (field_def[attr_start] == ' ' || field_def[attr_start] == '\t'))
        {
            attr_start++;
        }

        std::string attr_str = field_def.substr(attr_start);
        std::istringstream iss(attr_str);
        std::string token;

        while (iss >> token)
        {
            std::string lower_token = to_lower(token);

            if (lower_token == "unsigned")
            {
                field.is_unsigned = true;
            }
            else if (lower_token == "not")
            {
                iss >> token;
                if (to_lower(token) == "null")
                    field.is_nullable = false;
            }
            else if (lower_token == "null")
            {
                field.is_nullable = true;
            }
            else if (lower_token == "default")
            {
                iss >> std::ws;
                char def_quote = static_cast<char>(iss.peek());
                if (def_quote == '\'' || def_quote == '"')
                {
                    iss.get();
                    std::string dv;
                    char dc;
                    while (iss.get(dc))
                    {
                        if (dc == '\\' && def_quote == '\'')
                        {
                            char dn;
                            if (iss.get(dn))
                            {
                                if (dn == '\\')
                                {
                                    dv += '\\';// \\\\ → literal backslash
                                }
                                else if (dn == '\'')
                                {
                                    dv += '\'';// \\' → literal single quote
                                }
                                else
                                {
                                    dv += dn;// other escape: keep as-is
                                }
                            }
                            continue;
                        }
                        if (dc == def_quote)
                            break;
                        dv += dc;
                    }
                    field.default_value = dv;
                }
                else
                {
                    iss >> token;
                    field.default_value = token;
                }
            }
            else if (lower_token == "auto_increment")
            {
                field.is_auto_inc    = true;
                table.auto_inc_field = field.field_name;
            }
            else if (lower_token == "primary")
            {
                field.is_pk = true;
                iss >> token;
            }
            else if (lower_token == "comment")
            {
                std::string comment;
                std::getline(iss, comment);
                field.comment = trim(comment);
                size_t fq     = field.comment.find_first_of("'\"");
                size_t lq     = field.comment.find_last_of("'\"");
                if (fq != std::string::npos && lq != std::string::npos && fq < lq)
                {
                    field.comment = field.comment.substr(fq + 1, lq - fq - 1);
                }
            }
            else if (lower_token == "character")
            {
                iss >> token;
                if (to_lower(token) == "set")
                    iss >> token;
            }
            else if (lower_token == "collate")
            {
                iss >> token;
            }
        }

        if (type_lower == "tinyint")
        {
            field.mysql_type = 0x01;
        }
        else if (type_lower == "smallint")
        {
            field.mysql_type = 0x02;
        }
        else if (type_lower == "mediumint")
        {
            field.mysql_type = 0x03;
        }
        else if (type_lower == "int" || type_lower == "integer")
        {
            field.mysql_type = 0x03;
        }
        else if (type_lower == "bigint")
        {
            field.mysql_type = 0x08;
        }
        else if (type_lower == "float")
        {
            field.mysql_type = 0x04;
        }
        else if (type_lower == "double")
        {
            field.mysql_type = 0x05;
        }
        else if (type_lower == "decimal" || type_lower == "numeric")
        {
            field.mysql_type = 0xF6;
        }
        else if (type_lower == "date")
        {
            field.mysql_type = 0x0A;
        }
        else if (type_lower == "time")
        {
            field.mysql_type = 0x0B;
        }
        else if (type_lower == "datetime")
        {
            field.mysql_type = 0x0C;
        }
        else if (type_lower == "timestamp")
        {
            field.mysql_type = 0x07;
        }
        else if (type_lower == "char")
        {
            field.mysql_type = 0xFE;
        }
        else if (type_lower == "varchar")
        {
            field.mysql_type = 0xFD;
        }
        else if (type_lower == "text" || type_lower == "tinytext" ||
                 type_lower == "mediumtext" || type_lower == "longtext")
        {
            field.mysql_type = 0xFC;
        }
        else if (type_lower == "blob" || type_lower == "tinyblob" ||
                 type_lower == "mediumblob" || type_lower == "longblob")
        {
            field.mysql_type = 0xFC;
        }
        else if (type_lower == "json")
        {
            field.mysql_type = 0xF5;
        }

        table.fields.push_back(field);
        pos++;
    }

    std::string tail_part = show_create_sql.substr(close_brace + 1);
    std::istringstream tail_iss(tail_part);
    std::string tail_token;
    while (tail_iss >> tail_token)
    {
        std::string lower_token = to_lower(tail_token);
        if (lower_token == "engine")
        {
            tail_iss >> table.engine;
        }
        else if (lower_token == "charset")
        {
            tail_iss >> tail_token;
            if (to_lower(tail_token) == "=")
                tail_iss >> table.charset;
            else
                table.charset = tail_token;
        }
        else if (lower_token == "collate")
        {
            tail_iss >> table.collation;
        }
        else if (lower_token == "comment")
        {
            std::string comment;
            std::getline(tail_iss, comment);
            table.table_comment = trim(comment);
            size_t fq           = table.table_comment.find_first_of("'\"");
            size_t lq           = table.table_comment.find_last_of("'\"");
            if (fq != std::string::npos && lq != std::string::npos && fq < lq)
            {
                table.table_comment = table.table_comment.substr(fq + 1, lq - fq - 1);
            }
        }
    }

    return true;
}

// ===================== DDL 生成 =====================

inline std::string gen_mysql_create_table(const db_table_info &info)
{
    if (info.fields.empty())
        return "";

    std::ostringstream oss;
    oss << "DROP TABLE IF EXISTS `" << info.table_name << "`;\n\n";
    oss << "CREATE TABLE `" << info.table_name << "` (\n";

    for (size_t i = 0; i < info.fields.size(); i++)
    {
        const auto &f = info.fields[i];
        oss << "  `" << f.field_name << "` ";

        std::string type_str = f.field_type;
        std::string tl       = to_lower(type_str);

        if (f.is_auto_inc && f.is_pk)
        {
            if (tl == "integer" || tl == "int" || tl == "serial")
            {
                type_str = "INT";
            }
            else if (tl == "bigint" || tl == "bigserial")
            {
                type_str = "BIGINT";
            }
            else if (tl == "smallint" || tl == "smallserial")
            {
                type_str = "SMALLINT";
            }
        }

        if (tl == "int")
            type_str = "INT";
        else if (tl == "integer")
            type_str = "INT";
        else if (tl == "serial")
            type_str = "INT";
        else if (tl == "bigint")
            type_str = "BIGINT";
        else if (tl == "bigserial")
            type_str = "BIGINT";
        else if (tl == "smallint")
            type_str = "SMALLINT";
        else if (tl == "smallserial")
            type_str = "SMALLINT";
        else if (tl == "tinyint")
            type_str = "TINYINT";
        else if (tl == "mediumint")
            type_str = "MEDIUMINT";
        else if (tl == "float")
            type_str = "FLOAT";
        else if (tl == "double")
            type_str = "DOUBLE";
        else if (tl == "decimal")
            type_str = "DECIMAL";
        else if (tl == "numeric")
            type_str = "NUMERIC";
        else if (tl == "char")
            type_str = "CHAR";
        else if (tl == "varchar")
            type_str = "VARCHAR";
        else if (tl == "text")
            type_str = "TEXT";
        else if (tl == "tinytext")
            type_str = "TINYTEXT";
        else if (tl == "mediumtext")
            type_str = "MEDIUMTEXT";
        else if (tl == "longtext")
            type_str = "LONGTEXT";
        else if (tl == "blob")
            type_str = "BLOB";
        else if (tl == "tinyblob")
            type_str = "TINYBLOB";
        else if (tl == "mediumblob")
            type_str = "MEDIUMBLOB";
        else if (tl == "longblob")
            type_str = "LONGBLOB";
        else if (tl == "date")
            type_str = "DATE";
        else if (tl == "time")
            type_str = "TIME";
        else if (tl == "datetime")
            type_str = "DATETIME";
        else if (tl == "timestamp")
            type_str = "TIMESTAMP";
        else if (tl == "json")
            type_str = "JSON";
        else if (tl == "boolean")
            type_str = "TINYINT(1)";

        oss << type_str;

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
        bool is_nextval = false;
        if (!f.default_value.empty())
        {
            std::string dl = to_lower(f.default_value);
            if (dl.find("nextval") != std::string::npos)
            {
                is_nextval = true;
            }
            else
            {
                oss << " DEFAULT ";
                if (f.default_value == "CURRENT_TIMESTAMP" || f.default_value == "now()")
                {
                    oss << f.default_value;
                }
                else if (f.default_value.size() >= 2 &&
                         (f.default_value.front() == '\'' || f.default_value.front() == '"'))
                {
                    oss << f.default_value;
                }
                else
                {
                    oss << "'" << escape_mysql_string(f.default_value) << "'";
                }
            }
        }
        if (f.is_auto_inc || is_nextval)
            oss << " AUTO_INCREMENT";
        if (!f.comment.empty())
            oss << " COMMENT '" << escape_mysql_string(f.comment) << "'";

        if (i < info.fields.size() - 1 || !info.pk_name.empty() || !info.indexes.empty())
            oss << ",";
        oss << "\n";
    }

    if (!info.pk_name.empty())
    {
        oss << "  PRIMARY KEY (`" << info.pk_name << "`)";
        if (!info.indexes.empty())
            oss << ",";
        oss << "\n";
    }

    for (size_t i = 0; i < info.indexes.size(); i++)
    {
        const auto &idx = info.indexes[i];
        if (idx.is_primary)
            continue;

        oss << "  ";
        if (idx.is_unique)
        {
            oss << "UNIQUE ";
        }
        oss << "KEY `" << idx.index_name << "` (";
        for (size_t j = 0; j < idx.columns.size(); j++)
        {
            if (j > 0)
                oss << ", ";
            oss << "`" << idx.columns[j] << "`";
        }
        oss << ")";
        if (i < info.indexes.size() - 1)
            oss << ",";
        oss << "\n";
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

    if (!info.collation.empty())
        oss << " COLLATE=" << info.collation;

    if (!info.table_comment.empty())
        oss << " COMMENT='" << escape_mysql_string(info.table_comment) << "'";

    oss << ";\n";
    return oss.str();
}

inline std::string gen_pg_create_table(const db_table_info &info)
{
    if (info.fields.empty())
        return "";

    std::ostringstream oss;
    oss << "DROP TABLE IF EXISTS " << escape_pg_identifier(info.table_name) << ";\n\n";
    oss << "CREATE TABLE " << escape_pg_identifier(info.table_name) << " (\n";

    for (size_t i = 0; i < info.fields.size(); i++)
    {
        const auto &f = info.fields[i];
        oss << "  " << escape_pg_identifier(f.field_name) << " ";

        std::string tl = to_lower(f.field_type);

        bool is_serial_type = (tl == "serial" || tl == "bigserial" || tl == "smallserial");
        bool needs_serial   = f.is_auto_inc || is_serial_type;

        if (needs_serial && f.is_pk)
        {
            if (tl == "bigserial" || tl == "bigint")
            {
                oss << "BIGSERIAL";
            }
            else if (tl == "smallserial" || tl == "smallint")
            {
                oss << "SMALLSERIAL";
            }
            else
            {
                oss << "SERIAL";
            }
            oss << " PRIMARY KEY";
        }
        else if (needs_serial && !f.is_pk)
        {
            if (tl == "bigserial" || tl == "bigint")
            {
                oss << "BIGSERIAL";
            }
            else if (tl == "smallserial" || tl == "smallint")
            {
                oss << "SMALLSERIAL";
            }
            else
            {
                oss << "SERIAL";
            }
        }
        else
        {
            std::string target_type = f.field_type;

            if (tl == "serial" || tl == "smallserial" || tl == "bigserial")
            {
                target_type = "integer";
                tl          = "integer";
            }

            if (tl == "int2")
                target_type = "smallint";
            else if (tl == "int4")
                target_type = "integer";
            else if (tl == "int8")
                target_type = "bigint";
            else if (tl == "float4")
                target_type = "real";
            else if (tl == "float8")
                target_type = "double precision";
            else if (tl == "character varying")
                target_type = "varchar";
            else if (tl == "character")
                target_type = "char";
            else if (tl == "bool")
                target_type = "boolean";
            else if (tl == "timestamp without time zone")
                target_type = "timestamp";
            else if (tl == "timestamp with time zone")
                target_type = "timestamptz";
            else if (tl == "time without time zone")
                target_type = "time";
            else if (tl == "time with time zone")
                target_type = "timetz";

            oss << target_type;

            if (!f.is_nullable)
                oss << " NOT NULL";

            if (!f.default_value.empty())
            {
                std::string dl = to_lower(f.default_value);
                if (dl.find("nextval") == std::string::npos)
                {
                    oss << " DEFAULT ";
                    if (f.default_value == "CURRENT_TIMESTAMP" || f.default_value == "now()")
                    {
                        oss << f.default_value;
                    }
                    else if (f.default_value.size() >= 2 &&
                             (f.default_value.front() == '\'' || f.default_value.front() == '"'))
                    {
                        oss << f.default_value;
                    }
                    else
                    {
                        oss << "'" << escape_pg_string(f.default_value) << "'";
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

    for (const auto &idx : info.indexes)
    {
        if (idx.is_primary)
            continue;

        oss << "CREATE ";
        if (idx.is_unique)
            oss << "UNIQUE ";
        oss << "INDEX " << escape_pg_identifier(idx.index_name) << " ON "
            << escape_pg_identifier(info.table_name) << " (";
        for (size_t j = 0; j < idx.columns.size(); j++)
        {
            if (j > 0)
                oss << ", ";
            oss << escape_pg_identifier(idx.columns[j]);
        }
        oss << ");\n";
    }

    if (!info.table_comment.empty())
    {
        oss << "COMMENT ON TABLE " << escape_pg_identifier(info.table_name) << " IS '"
            << escape_pg_string(info.table_comment) << "';\n";
    }

    for (const auto &f : info.fields)
    {
        if (!f.comment.empty())
        {
            oss << "COMMENT ON COLUMN " << escape_pg_identifier(info.table_name) << "."
                << escape_pg_identifier(f.field_name) << " IS '"
                << escape_pg_string(f.comment) << "';\n";
        }
    }

    if (!info.auto_inc_field.empty())
    {
        std::string seq_name = info.table_name + "_" + info.auto_inc_field + "_seq";
        oss << "-- Note: sequence " << seq_name << " created by SERIAL/BIGSERIAL automatically\n";
    }

    return oss.str();
}

// SQLite 不支持列/表 COMMENT 与 ENGINE/CHARSET, 注释以 -- SQL 注释保留
inline std::string gen_sqlite_create_table(const db_table_info &info)
{
    if (info.fields.empty())
        return "";

    std::ostringstream oss;

    // 收集主键列 (优先用 is_pk 标记, 回退拆分 pk_name)
    std::vector<std::string> pk_cols;
    for (const auto &f : info.fields)
    {
        if (f.is_pk)
            pk_cols.push_back(f.field_name);
    }
    if (pk_cols.empty() && !info.pk_name.empty())
    {
        std::istringstream pk_iss(info.pk_name);
        std::string pk_token;
        while (std::getline(pk_iss, pk_token, ','))
        {
            pk_token = trim(pk_token);
            if (!pk_token.empty())
                pk_cols.push_back(pk_token);
        }
    }
    bool composite_pk = pk_cols.size() > 1;

    if (!info.table_comment.empty())
    {
        oss << "-- Table comment: " << info.table_comment << "\n";
    }
    oss << "DROP TABLE IF EXISTS " << escape_sqlite_identifier(info.table_name) << ";\n\n";
    oss << "CREATE TABLE " << escape_sqlite_identifier(info.table_name) << " (\n";

    for (size_t i = 0; i < info.fields.size(); i++)
    {
        const auto &f = info.fields[i];
        oss << "  " << escape_sqlite_identifier(f.field_name) << " ";

        std::string tl = to_lower(f.field_type);

        if (f.is_auto_inc && f.is_pk)
        {
            // SQLite AUTOINCREMENT 必须是 INTEGER PRIMARY KEY (不支持复合主键)
            oss << "INTEGER PRIMARY KEY AUTOINCREMENT";
        }
        else
        {
            oss << f.field_type;

            // 追加长度/精度 (声明已带参数时跳过, 如 SQLite 源回读的 varchar(255))
            if (tl.find('(') == std::string::npos)
            {
                if (f.length > 0 && (tl == "varchar" || tl == "nvarchar" || tl == "char" ||
                                     tl == "character varying" || tl == "character"))
                {
                    oss << "(" << f.length << ")";
                }
                else if (f.length > 0 && (tl == "decimal" || tl == "numeric"))
                {
                    if (f.decimals > 0)
                        oss << "(" << f.length << "," << (unsigned int)f.decimals << ")";
                    else
                        oss << "(" << f.length << ")";
                }
            }

            if (!f.is_nullable)
                oss << " NOT NULL";

            if (!f.default_value.empty())
            {
                std::string dl = to_lower(f.default_value);
                if (dl.find("nextval") == std::string::npos)
                {
                    oss << " DEFAULT ";
                    if (dl == "current_timestamp" || dl == "now()" || dl == "now")
                    {
                        // now()/CURRENT_TIMESTAMP 映射为 SQLite 支持的 CURRENT_TIMESTAMP
                        oss << "CURRENT_TIMESTAMP";
                    }
                    else if (dl == "null")
                    {
                        oss << "NULL";
                    }
                    else if (f.default_value.size() >= 2 && f.default_value.front() == '\'')
                    {
                        // 去除 PG 风格 ::type 后缀后原样输出 (已带引号)
                        size_t cast_pos = f.default_value.find("::");
                        if (cast_pos != std::string::npos)
                            oss << f.default_value.substr(0, cast_pos);
                        else
                            oss << f.default_value;
                    }
                    else
                    {
                        oss << "'" << escape_sqlite_string(f.default_value) << "'";
                    }
                }
            }

            // 非自增单列主键行内声明 (自增主键已在上方分支处理)
            if (f.is_pk && pk_cols.size() == 1)
                oss << " PRIMARY KEY";
        }

        if (i < info.fields.size() - 1 || composite_pk)
            oss << ",";
        if (!f.comment.empty())
            oss << " -- " << f.comment;
        oss << "\n";
    }

    if (composite_pk)
    {
        oss << "  PRIMARY KEY (";
        for (size_t j = 0; j < pk_cols.size(); j++)
        {
            if (j > 0)
                oss << ", ";
            oss << escape_sqlite_identifier(pk_cols[j]);
        }
        oss << ")";
    }

    oss << "\n);\n";

    for (const auto &idx : info.indexes)
    {
        if (idx.is_primary)
            continue;

        oss << "CREATE ";
        if (idx.is_unique)
            oss << "UNIQUE ";
        oss << "INDEX " << escape_sqlite_identifier(idx.index_name) << " ON "
            << escape_sqlite_identifier(info.table_name) << " (";
        for (size_t j = 0; j < idx.columns.size(); j++)
        {
            if (j > 0)
                oss << ", ";
            oss << escape_sqlite_identifier(idx.columns[j]);
        }
        oss << ");\n";
    }

    if (!info.auto_inc_field.empty())
    {
        oss << "-- Note: sqlite_sequence for table " << info.table_name
            << " updated after data migration\n";
    }

    return oss.str();
}

inline std::string gen_ddl(const db_table_info &info, DB_TYPE db_type)
{
    if (db_type == DB_TYPE::POSTGRESQL)
    {
        return gen_pg_create_table(info);
    }
    if (db_type == DB_TYPE::SQLITE)
    {
        return gen_sqlite_create_table(info);
    }
    return gen_mysql_create_table(info);
}

// ===================== orm::table_columns_info_t 转 db_table_info =====================
// 用于将 modelfun.hpp 中 pg_get_column_info 获取的列信息转换为 db_table_info，以便生成 DDL
inline db_table_info convert_columns_to_table_info(
    const std::string &table_name,
    const std::vector<orm::table_columns_info_t> &columns,
    const std::string &table_comment = "")
{
    db_table_info info;
    info.table_name     = table_name;
    info.table_comment  = table_comment;
    info.source_db_type = DB_TYPE::POSTGRESQL;

    for (const auto &col : columns)
    {
        db_field_info field;
        field.field_name    = col.col_name;
        field.comment       = col.comment;
        field.default_value = col.default_value;
        field.is_auto_inc   = col.is_auto_inc;
        field.is_pk         = col.is_pk;
        field.is_unsigned   = col.is_unsigned;
        field.length        = col.col_length;
        field.decimals      = col.decimals;

        // 将 MySQL 协议类型码转换为 PostgreSQL 类型字符串
        field.field_type = mysql_col_type_to_pg_type(col.col_type, col.col_length, col.is_unsigned);

        info.fields.push_back(field);

        if (col.is_pk)
        {
            info.pk_name = col.col_name;
        }
        if (col.is_auto_inc)
        {
            info.auto_inc_field = col.col_name;
        }
    }

    return info;
}

// ===================== PG 表结构读取 =====================

inline bool pg_get_table_schema(std::shared_ptr<orm::pg_conn_base> pg_conn,
                                const std::string &table_name,
                                db_table_info &table)
{
    if (!pg_conn)
    {
        std::cerr << "  [ERROR] PostgreSQL connection is null" << std::endl;
        return false;
    }

    if (!is_valid_sql_identifier(table_name))
    {
        std::cerr << "  [ERROR] Invalid table name: '" << table_name << "'" << std::endl;
        return false;
    }

    table.table_name     = table_name;
    table.source_db_type = DB_TYPE::POSTGRESQL;

    std::string sql =
        "SELECT a.attname, t.typname, a.attlen, a.atttypmod, a.attnotnull, "
        "pg_get_expr(d.adbin, d.adrelid) AS default_value, "
        "col_description(a.attrelid, a.attnum) AS comment "
        "FROM pg_attribute a "
        "JOIN pg_type t ON a.atttypid = t.oid "
        "LEFT JOIN pg_attrdef d ON a.attrelid = d.adrelid AND a.attnum = d.adnum "
        "WHERE a.attrelid = (SELECT oid FROM pg_class WHERE relname = '" +
        table_name + "' AND relkind = 'r') "
                     "AND a.attnum > 0 AND NOT a.attisdropped ORDER BY a.attnum";

    std::vector<orm::field_info_t> fields_out;
    std::vector<orm::pg_row_data_t> rows_out;
    unsigned int affected = 0;
    unsigned int err      = pg_conn->execute_and_fetch(sql, fields_out, rows_out, affected);
    if (err > 0)
    {
        std::cerr << "  [ERROR] PG read table schema failed: " << pg_conn->error_msg << std::endl;
        return false;
    }

    for (auto &row : rows_out)
    {
        db_field_info field;
        for (size_t i = 0; i < fields_out.size() && i < row.values.size(); i++)
        {
            const std::string &col_name = fields_out[i].name;
            const std::string &col_val  = row.values[i];

            if (col_name == "attname")
            {
                field.field_name = col_val;
                std::transform(field.field_name.begin(), field.field_name.end(), field.field_name.begin(), [](unsigned char c) -> char
                               { return ::tolower(c); });
            }
            else if (col_name == "typname")
            {
                field.field_type = col_val;
                std::string tl   = to_lower(col_val);
                if (tl == "serial" || tl == "smallserial" || tl == "bigserial")
                {
                    field.is_auto_inc    = true;
                    field.is_pk          = true;
                    table.auto_inc_field = field.field_name;
                    field.field_type     = (tl == "bigserial") ? "bigint" : "integer";
                }
            }
            else if (col_name == "attlen")
            {
                if (!col_val.empty())
                {
                    try
                    {
                        unsigned long len = std::stoul(col_val);
                        if (len <= 0xFFFFFFFF)
                        {
                            field.length = static_cast<unsigned int>(len);
                        }
                    }
                    catch (...)
                    {
                    }
                }
            }
            else if (col_name == "atttypmod")
            {
                if (!col_val.empty())
                {
                    try
                    {
                        int32_t mod = std::stoi(col_val);
                        if (mod > 4 && mod <= 0x7FFFFFF0)
                        {
                            field.length = static_cast<unsigned int>(mod - 4);
                        }
                    }
                    catch (...)
                    {
                    }
                }
            }
            else if (col_name == "attnotnull")
            {
                if (col_val == "t" || col_val == "true")
                {
                    field.is_nullable = false;
                }
            }
            else if (col_name == "default_value")
            {
                if (!col_val.empty())
                {
                    field.default_value = col_val;
                    std::string lv      = to_lower(col_val);
                    if (lv.find("nextval") != std::string::npos)
                    {
                        field.is_auto_inc    = true;
                        table.auto_inc_field = field.field_name;
                    }
                }
            }
            else if (col_name == "comment")
            {
                if (!col_val.empty())
                {
                    field.comment = col_val;
                }
            }
        }
        table.fields.push_back(field);
    }

    // Read primary key
    sql = "SELECT a.attname FROM pg_index i JOIN pg_attribute a ON a.attrelid = i.indrelid AND a.attnum = ANY(i.indkey) "
          "WHERE i.indrelid = '" +
          table_name + "'::regclass AND i.indisprimary";

    err = pg_conn->execute_and_fetch(sql, fields_out, rows_out, affected);
    if (err > 0)
    {
        std::cerr << "  [WARN] PG read primary key failed: " << pg_conn->error_msg << std::endl;
    }
    else
    {
        if (!rows_out.empty() && !rows_out[0].values.empty())
        {
            std::string pk = rows_out[0].values[0];
            std::transform(pk.begin(), pk.end(), pk.begin(), [](unsigned char c) -> char
                           { return ::tolower(c); });
            table.pk_name = pk;
            for (auto &f : table.fields)
            {
                if (f.field_name == pk)
                {
                    f.is_pk = true;
                    if (to_lower(f.field_type) == "integer" ||
                        to_lower(f.field_type) == "bigint" ||
                        to_lower(f.field_type) == "smallint")
                    {
                        f.is_auto_inc        = true;
                        table.auto_inc_field = f.field_name;
                    }
                }
            }
        }
    }

    // Read unique indexes (excluding primary keys)
    sql = "SELECT i.relname AS index_name, a.attname "
          "FROM pg_index idx "
          "JOIN pg_class i ON idx.indexrelid = i.oid "
          "JOIN pg_attribute a ON a.attrelid = idx.indrelid AND a.attnum = ANY(idx.indkey) "
          "WHERE idx.indrelid = '" +
          table_name + "'::regclass AND idx.indisunique AND NOT idx.indisprimary "
                       "ORDER BY i.relname, a.attnum";

    err = pg_conn->execute_and_fetch(sql, fields_out, rows_out, affected);
    if (err == 0 && !rows_out.empty())
    {
        std::map<std::string, db_index_info> unique_map;
        for (auto &row : rows_out)
        {
            if (row.values.size() < 2)
                continue;
            std::string idx_name = row.values[0];
            std::string col_name = row.values[1];
            std::transform(col_name.begin(), col_name.end(), col_name.begin(), [](unsigned char c) -> char
                           { return ::tolower(c); });
            if (unique_map.find(idx_name) == unique_map.end())
            {
                db_index_info idx;
                idx.index_name       = idx_name;
                idx.is_unique        = true;
                idx.is_primary       = false;
                unique_map[idx_name] = idx;
            }
            unique_map[idx_name].columns.push_back(col_name);
        }
        for (auto &[name, idx] : unique_map)
        {
            table.indexes.push_back(idx);
        }
    }

    // Read normal indexes
    sql = "SELECT i.relname AS index_name, a.attname "
          "FROM pg_index idx "
          "JOIN pg_class i ON idx.indexrelid = i.oid "
          "JOIN pg_attribute a ON a.attrelid = idx.indrelid AND a.attnum = ANY(idx.indkey) "
          "WHERE idx.indrelid = '" +
          table_name + "'::regclass AND NOT idx.indisunique AND NOT idx.indisprimary "
                       "ORDER BY i.relname, a.attnum";

    err = pg_conn->execute_and_fetch(sql, fields_out, rows_out, affected);
    if (err == 0 && !rows_out.empty())
    {
        std::map<std::string, db_index_info> normal_map;
        for (auto &row : rows_out)
        {
            if (row.values.size() < 2)
                continue;
            std::string idx_name = row.values[0];
            std::string col_name = row.values[1];
            std::transform(col_name.begin(), col_name.end(), col_name.begin(), [](unsigned char c) -> char
                           { return ::tolower(c); });
            if (normal_map.find(idx_name) == normal_map.end())
            {
                db_index_info idx;
                idx.index_name       = idx_name;
                idx.is_unique        = false;
                idx.is_primary       = false;
                normal_map[idx_name] = idx;
            }
            normal_map[idx_name].columns.push_back(col_name);
        }
        for (auto &[name, idx] : normal_map)
        {
            table.indexes.push_back(idx);
        }
    }

    // Read table comment
    sql = "SELECT obj_description('" + table_name + "'::regclass)";
    err = pg_conn->execute_and_fetch(sql, fields_out, rows_out, affected);
    if (err == 0 && !rows_out.empty() && !rows_out[0].values.empty())
    {
        table.table_comment = rows_out[0].values[0];
    }

    return true;
}

// ===================== MySQL 表结构读取 =====================

inline bool mysql_get_table_schema(std::shared_ptr<orm::mysql_conn_base> mysql_conn,
                                   const std::string &table_name,
                                   db_table_info &table)
{
    if (!mysql_conn)
    {
        std::cerr << "  [ERROR] MySQL connection is null" << std::endl;
        return false;
    }

    if (!is_valid_sql_identifier(table_name))
    {
        std::cerr << "  [ERROR] Invalid table name: '" << table_name << "'" << std::endl;
        return false;
    }

    // table_name already validated by is_valid_sql_identifier above
    std::string sql = "SHOW CREATE TABLE `" + table_name + "`";

    if (mysql_conn->write_sql(sql) == 0)
    {
        std::cerr << "  [ERROR] MySQL SHOW CREATE TABLE failed: " << mysql_conn->error_msg << std::endl;
        return false;
    }

    orm::pack_info_t temp_pack_data;
    temp_pack_data.seq_id     = 1;
    bool is_first_row         = false;
    bool got_create_table     = false;
    unsigned int action_setup = 0;
    unsigned int column_num   = 0;

    bool is_done = false;
    while (!is_done)
    {
        unsigned int n = mysql_conn->read_loop();
        if (n == 0)
        {
            std::cerr << "  [ERROR] MySQL read failed: " << mysql_conn->error_msg << std::endl;
            return false;
        }

        unsigned int offset = 0;
        for (; offset < n;)
        {
            mysql_conn->read_field_pack(mysql_conn->_cache_data, n, offset, temp_pack_data);
            if (temp_pack_data.error > 0)
            {
                is_done = true;
                break;
            }

            if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
            {
                if (mysql_conn->pack_eof_check(temp_pack_data))
                {
                    is_done = true;
                    break;
                }

                if (action_setup == 0)
                {
                    if (temp_pack_data.length >= 1 && temp_pack_data.data.size() > 0 &&
                        (unsigned char)temp_pack_data.data[0] < 251 &&
                        (unsigned char)temp_pack_data.data[0] > 0)
                    {
                        action_setup = 1;
                        column_num   = (unsigned char)temp_pack_data.data[0];
                    }
                }
                else if (action_setup == 1)
                {
                    orm::field_info_t temp_filed_col;
                    mysql_conn->read_col_info(temp_pack_data.data, temp_filed_col);
                    column_num--;
                    if (column_num == 0)
                    {
                        action_setup = 2;
                    }
                }
                else if (action_setup == 2)
                {
                    column_num           = 0;
                    unsigned int tempnum = 0;

                    if (!is_first_row)
                    {
                        is_first_row = true;
                        if (temp_pack_data.data.size() == 0)
                        {
                            is_done = true;
                            break;
                        }
                        unsigned long long name_length = 0;
                        name_length                    = mysql_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                        tempnum += name_length;
                        column_num--;
                    }

                    if (temp_pack_data.data.size() == 0 || tempnum >= temp_pack_data.data.size())
                    {
                        is_done = true;
                        break;
                    }

                    unsigned long long name_length = 0;
                    name_length                    = mysql_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);

                    if (name_length > 0 && name_length < 0xFFFFFFFF &&
                        tempnum + name_length <= temp_pack_data.data.size())
                    {
                        std::string create_sql;
                        create_sql.resize(name_length);
                        for (unsigned long long i = 0; i < name_length; i++)
                        {
                            create_sql[i] = temp_pack_data.data[tempnum + i];
                        }
                        got_create_table     = parse_mysql_show_create(create_sql, table);
                        table.source_db_type = DB_TYPE::MYSQL;
                    }
                    is_done = true;
                    break;
                }
            }
            else
            {
                if (offset >= n)
                    break;
            }
        }
    }

    return got_create_table;
}

// ===================== SQLite 表结构读取 =====================

inline std::vector<std::string> get_sqlite_tables(std::shared_ptr<orm::sqlite_conn_base> sqlite_conn)
{
    std::vector<std::string> tables;
    if (!sqlite_conn)
    {
        std::cerr << "  [ERROR] SQLite connection is null" << std::endl;
        return tables;
    }
    // 已过滤 sqlite_% 内部表 (见 sqlite_conn_base::get_table_list)
    return sqlite_conn->get_table_list();
}

inline bool sqlite_get_table_schema(std::shared_ptr<orm::sqlite_conn_base> sqlite_conn,
                                    const std::string &table_name,
                                    db_table_info &table)
{
    if (!sqlite_conn)
    {
        std::cerr << "  [ERROR] SQLite connection is null" << std::endl;
        return false;
    }

    if (!is_valid_sql_identifier(table_name))
    {
        std::cerr << "  [ERROR] Invalid table name: '" << table_name << "'" << std::endl;
        return false;
    }

    table.table_name     = table_name;
    table.source_db_type = DB_TYPE::SQLITE;
    table.fields.clear();
    table.indexes.clear();
    table.pk_name.clear();
    table.auto_inc_field.clear();

    // PRAGMA table_info (键: cid/name/type/notnull/dflt_value/pk)
    auto cols = sqlite_conn->get_table_info(table_name);
    if (cols.empty())
    {
        std::cerr << "  [ERROR] SQLite PRAGMA table_info empty for: " << table_name << std::endl;
        return false;
    }

    // AUTOINCREMENT 判定: 查 sqlite_sequence 而非按类型猜测 (仅在 sqlite_sequence 有该表记录时成立)
    bool has_autoincrement = false;
    {
        orm::sqlite_query_result qr;
        if (sqlite_conn->query_fetch(
                "SELECT name FROM sqlite_master WHERE type='table' AND name='sqlite_sequence'",
                qr) &&
            !qr.rows.empty())
        {
            orm::sqlite_query_result qr2;
            if (sqlite_conn->query_fetch(
                    "SELECT seq FROM sqlite_sequence WHERE name = '" + escape_sqlite_string(table_name) + "'",
                    qr2) &&
                !qr2.rows.empty())
            {
                has_autoincrement = true;
            }
        }
    }

    size_t pk_count = 0;
    std::string pk_single;

    for (auto &c : cols)
    {
        db_field_info field;
        field.field_name = c["name"];

        // 保留完整声明 (如 VARCHAR(255)), 同时解析长度/精度供 DDL 与反向转换使用
        std::string decl = trim(c["type"]);
        field.field_type = decl;

        size_t po = decl.find('(');
        size_t pc = decl.rfind(')');
        if (po != std::string::npos && pc != std::string::npos && pc > po)
        {
            std::string params = decl.substr(po + 1, pc - po - 1);
            size_t comma_pos   = params.find(',');
            try
            {
                if (comma_pos != std::string::npos)
                {
                    unsigned long len = std::stoul(trim(params.substr(0, comma_pos)));
                    unsigned long dec = std::stoul(trim(params.substr(comma_pos + 1)));
                    if (len <= 0xFFFFFFFF)
                        field.length = static_cast<unsigned int>(len);
                    if (dec <= 0xFF)
                        field.decimals = static_cast<unsigned char>(dec);
                }
                else
                {
                    unsigned long len = std::stoul(trim(params));
                    if (len <= 0xFFFFFFFF)
                        field.length = static_cast<unsigned int>(len);
                }
            }
            catch (...)
            {
            }
        }

        if (c["notnull"] == "1")
            field.is_nullable = false;

        // dflt_value: 空为无默认值; 字符串默认值带引号 'xxx', DEFAULT NULL 返回字面量 NULL (丢弃)
        const std::string &dflt = c["dflt_value"];
        if (!dflt.empty() && to_lower(dflt) != "null")
        {
            field.default_value = dflt;
        }

        if (!c["pk"].empty() && c["pk"] != "0")
        {
            field.is_pk = true;
            pk_count++;
            pk_single = field.field_name;
        }

        table.fields.push_back(field);
    }

    // AUTOINCREMENT 只存在于"INTEGER PRIMARY KEY AUTOINCREMENT"形式: 单列 INTEGER 主键
    if (has_autoincrement && pk_count == 1)
    {
        for (auto &f : table.fields)
        {
            if (f.is_pk && to_lower(f.field_type).find("int") != std::string::npos)
            {
                f.is_auto_inc        = true;
                table.auto_inc_field = f.field_name;
            }
        }
    }

    if (pk_count == 1)
    {
        table.pk_name = pk_single;
    }

    // 索引: PRAGMA index_list (列: seq/name/unique/origin/partial)
    orm::sqlite_query_result idx_qr;
    if (sqlite_conn->query_fetch("PRAGMA index_list(\"" + table_name + "\")", idx_qr))
    {
        for (auto &r : idx_qr.rows)
        {
            if (r.size() < 3)
                continue;
            std::string idx_name = r[1];
            std::string origin   = (r.size() > 3) ? r[3] : "";
            if (origin == "pk")
                continue;// INTEGER PRIMARY KEY 隐式索引, 跳过
            if (starts_with_icase(idx_name, "sqlite_"))
                continue;
            if (!is_valid_sql_identifier(idx_name))
                continue;// 校验索引名, 非法则跳过 (避免拼接风险)

            db_index_info idx;
            idx.index_name = idx_name;
            idx.is_unique  = (r[2] == "1");
            idx.is_primary = false;

            orm::sqlite_query_result col_qr;
            if (sqlite_conn->query_fetch("PRAGMA index_info(\"" + idx_name + "\")", col_qr))
            {
                for (auto &cr : col_qr.rows)
                {
                    // 列: seqno/cid/name; name 为空表示表达式索引, 整个索引丢弃
                    if (cr.size() < 3 || cr[2].empty())
                    {
                        idx.columns.clear();
                        break;
                    }
                    idx.columns.push_back(cr[2]);
                }
            }
            if (!idx.columns.empty())
                table.indexes.push_back(idx);
        }
    }

    return true;
}

}// namespace dbtypes

// ===================== 数据读取共用函数 =====================

namespace dbtypes
{

using orm::mysql_conn_base;
using orm::pg_conn_base;
using orm::sqlite_conn_base;

inline bool mysql_fetch_rows(std::shared_ptr<mysql_conn_base> mysql_conn,
                             const std::string &sql,
                             std::vector<row_data_t> &rows)
{
    rows.clear();

    if (!mysql_conn)
    {
        std::cerr << "  [ERROR] MySQL connection is null" << std::endl;
        return false;
    }

    if (mysql_conn->write_sql(sql) == 0)
    {
        return false;
    }

    unsigned int action_setup = 0;
    unsigned int column_num   = 0;
    std::vector<orm::field_info_t> field_array;
    orm::pack_info_t temp_pack_data;
    temp_pack_data.seq_id = 1;

    bool is_done = false;
    while (!is_done)
    {
        unsigned int n = mysql_conn->read_loop();
        if (n == 0)
            return false;

        unsigned int offset = 0;
        for (; offset < n;)
        {
            mysql_conn->read_field_pack(mysql_conn->_cache_data, n, offset, temp_pack_data);
            if (temp_pack_data.error > 0)
                return false;

            if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
            {
                if (mysql_conn->pack_eof_check(temp_pack_data))
                {
                    is_done = true;
                    break;
                }

                if (action_setup == 0)
                {
                    if (temp_pack_data.length >= 1 && temp_pack_data.data.size() > 0 &&
                        (unsigned char)temp_pack_data.data[0] < 251 &&
                        (unsigned char)temp_pack_data.data[0] > 0)
                    {
                        action_setup = 1;
                        column_num   = (unsigned char)temp_pack_data.data[0];
                    }
                }
                else if (action_setup == 1)
                {
                    orm::field_info_t temp_filed_col;
                    mysql_conn->read_col_info(temp_pack_data.data, temp_filed_col);
                    field_array.emplace_back(std::move(temp_filed_col));
                    column_num--;
                    if (column_num == 0)
                        action_setup = 2;
                }
                else if (action_setup == 2)
                {
                    row_data_t row;
                    unsigned int tempnum = 0;

                    for (unsigned int ij = 0; ij < field_array.size(); ij++)
                    {
                        if (tempnum < temp_pack_data.data.size() &&
                            (unsigned char)temp_pack_data.data[tempnum] == 0xFB)
                        {
                            row.values.push_back("");
                            row.is_null.push_back(true);
                            tempnum++;
                            continue;
                        }

                        // 防御：tempnum 越过包末尾时停止解析（pack_real_num 首字节无条件读取）
                        if (tempnum >= temp_pack_data.data.size())
                            break;

                        unsigned long long name_length = 0;
                        name_length                    = mysql_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);

                        row.is_null.push_back(false);
                        if (name_length > 0 && name_length < 0xFFFFFFFF &&
                            tempnum + name_length <= temp_pack_data.data.size())
                        {
                            std::string val(name_length, '\0');
                            for (unsigned long long k = 0; k < name_length; k++)
                            {
                                val[k] = temp_pack_data.data[tempnum + k];
                            }
                            row.values.push_back(val);
                        }
                        else
                        {
                            row.values.push_back("");
                        }
                        tempnum += name_length;
                    }
                    rows.push_back(std::move(row));
                }
            }
            else
            {
                if (offset >= n)
                    break;
            }
        }
    }

    return true;
}

inline bool pg_fetch_rows(std::shared_ptr<pg_conn_base> pg_conn,
                          const std::string &sql,
                          std::vector<row_data_t> &rows)
{
    rows.clear();

    if (!pg_conn)
    {
        std::cerr << "  [ERROR] PostgreSQL connection is null" << std::endl;
        return false;
    }

    std::vector<orm::field_info_t> fields_out;
    std::vector<orm::pg_row_data_t> rows_out;
    unsigned int affected = 0;

    unsigned int err = pg_conn->execute_and_fetch(sql, fields_out, rows_out, affected);
    if (err > 0)
        return false;

    for (auto &r : rows_out)
    {
        row_data_t row;
        for (size_t i = 0; i < r.values.size(); i++)
        {
            row.values.push_back(r.values[i]);
            row.is_null.push_back(r.is_null[i]);
        }
        rows.push_back(std::move(row));
    }

    return true;
}

// SQLite 结果集 → row_data_t (BLOB 以原始字节存于 values)
inline bool sqlite_fetch_rows(std::shared_ptr<sqlite_conn_base> sqlite_conn,
                              const std::string &sql,
                              std::vector<row_data_t> &rows)
{
    rows.clear();

    if (!sqlite_conn)
    {
        std::cerr << "  [ERROR] SQLite connection is null" << std::endl;
        return false;
    }

    orm::sqlite_query_result qr;
    if (!sqlite_conn->query_fetch(sql, qr))
        return false;

    for (size_t i = 0; i < qr.rows.size(); i++)
    {
        row_data_t row;
        row.values = qr.rows[i];
        for (size_t j = 0; j < qr.rows[i].size(); j++)
        {
            row.is_null.push_back(i < qr.is_null.size() && j < qr.is_null[i].size() ? qr.is_null[i][j] : false);
        }
        rows.push_back(std::move(row));
    }

    return true;
}

// 判断列是否为二进制类型 (BLOB 亲和或 bytea/binary) → 强制 bind_blob / X'hex' 输出
inline bool sqlite_is_blob_field(const db_field_info &f)
{
    std::string tl = to_lower(f.field_type);
    if (tl.find("blob") != std::string::npos)
        return true;
    if (tl == "bytea" || tl == "binary" || tl == "varbinary")
        return true;
    return false;
}

inline bool sqlite_str_is_int(const std::string &v)
{
    if (v.empty())
        return false;
    size_t i = 0;
    if (v[0] == '+' || v[0] == '-')
        i = 1;
    if (i >= v.size())
        return false;
    for (; i < v.size(); i++)
    {
        if (!std::isdigit(static_cast<unsigned char>(v[i])))
            return false;
    }
    return true;
}

inline bool sqlite_str_is_double(const std::string &v)
{
    if (v.empty())
        return false;
    char *end        = nullptr;
    const char *cstr = v.c_str();
    errno            = 0;
    std::strtod(cstr, &end);
    return errno == 0 && end == cstr + v.size();
}

// 参数绑定单行 INSERT (二进制安全: BLOB 用 bind_blob, 不走文本 SQL 拼接)
inline bool sqlite_insert_row(std::shared_ptr<sqlite_conn_base> sqlite_conn,
                              const std::string &table_name,
                              const std::vector<db_field_info> &fields,
                              const row_data_t &row)
{
    if (!sqlite_conn)
    {
        std::cerr << "  [ERROR] SQLite connection is null" << std::endl;
        return false;
    }

    size_t max_col = std::min({fields.size(), row.values.size(), row.is_null.size()});
    if (max_col == 0)
        return false;

    std::ostringstream oss;
    oss << "INSERT INTO " << escape_sqlite_identifier(table_name) << " (";
    for (size_t i = 0; i < max_col; i++)
    {
        if (i > 0)
            oss << ", ";
        oss << escape_sqlite_identifier(fields[i].field_name);
    }
    oss << ") VALUES (";
    for (size_t i = 0; i < max_col; i++)
    {
        if (i > 0)
            oss << ", ";
        oss << "?";
    }
    oss << ")";

    std::vector<orm::sqlite_bind_param> params(max_col);
    for (size_t i = 0; i < max_col; i++)
    {
        orm::sqlite_bind_param &p = params[i];
        if (row.is_null[i])
        {
            p.type = SQLITE_NULL;
            continue;
        }

        const std::string &v = row.values[i];

        // 二进制列强制 bind_blob
        if (sqlite_is_blob_field(fields[i]))
        {
            p.type = SQLITE_BLOB;
            p.data = v;
            continue;
        }

        std::string tl = to_lower(fields[i].field_type);
        // DECIMAL/日期类声明按 TEXT 绑定, 保留精度与格式 (NUMERIC 亲和不会改写文本)
        if (starts_with_icase(tl, "decimal") || starts_with_icase(tl, "numeric") ||
            tl == "datetime" || tl == "date" || tl == "time" || tl == "timestamp")
        {
            p.type = SQLITE_TEXT;
            p.data = v;
        }
        else if (tl.find("int") != std::string::npos)
        {
            // INTEGER 亲和: 先验证是数字再绑定, 否则按文本 (避免非数字直出出错)
            p.type = sqlite_str_is_int(v) ? SQLITE_INTEGER : SQLITE_TEXT;
            p.data = v;
        }
        else if (tl.find("real") != std::string::npos || tl.find("floa") != std::string::npos ||
                 tl.find("doub") != std::string::npos)
        {
            p.type = sqlite_str_is_double(v) ? SQLITE_FLOAT : SQLITE_TEXT;
            p.data = v;
        }
        else
        {
            p.type = SQLITE_TEXT;
            p.data = v;
        }
    }

    return sqlite_conn->exec_bound(oss.str(), params);
}

inline std::string build_insert_sql_mysql(const std::string &table_name,
                                          const std::vector<db_field_info> &fields,
                                          const row_data_t &row,
                                          size_t start_col,
                                          size_t end_col)
{
    std::ostringstream oss;
    oss << "INSERT INTO `" << table_name << "` (";

    size_t max_col = std::min({end_col, fields.size(), row.values.size(), row.is_null.size()});

    for (size_t i = start_col; i < max_col; i++)
    {
        if (i > start_col)
            oss << ", ";
        oss << "`" << fields[i].field_name << "`";
    }
    oss << ") VALUES (";

    for (size_t i = start_col; i < max_col; i++)
    {
        if (i > start_col)
            oss << ", ";
        if (row.is_null[i])
        {
            oss << "NULL";
        }
        else
        {
            const auto &v = row.values[i];
            if (v.empty())
            {
                oss << "''";
            }
            else
            {
                oss << "'" << escape_mysql_string(v) << "'";
            }
        }
    }
    oss << ")";
    return oss.str();
}

inline std::string build_insert_sql_pg(const std::string &table_name,
                                       const std::vector<db_field_info> &fields,
                                       const row_data_t &row,
                                       size_t start_col,
                                       size_t end_col)
{
    std::ostringstream oss;
    oss << "INSERT INTO " << escape_pg_identifier(table_name) << " (";

    size_t max_col = std::min({end_col, fields.size(), row.values.size(), row.is_null.size()});

    for (size_t i = start_col; i < max_col; i++)
    {
        if (i > start_col)
            oss << ", ";
        oss << escape_pg_identifier(fields[i].field_name);
    }
    oss << ") VALUES (";

    for (size_t i = start_col; i < max_col; i++)
    {
        if (i > start_col)
            oss << ", ";
        if (row.is_null[i])
        {
            oss << "NULL";
        }
        else
        {
            const auto &v = row.values[i];
            if (v.empty())
            {
                oss << "''";
            }
            else
            {
                oss << "'" << escape_pg_string(v) << "'";
            }
        }
    }
    oss << ")";
    return oss.str();
}

// SQLite 目标的文本形式 INSERT (供 dbexport .sql 输出; 直接写库请用 sqlite_insert_row 绑定形式)
// BLOB 列输出 X'十六进制' 字面量; 转义仅用 '' (不识别反斜杠转义)
inline std::string build_insert_sql_sqlite(const std::string &table_name,
                                           const std::vector<db_field_info> &fields,
                                           const row_data_t &row,
                                           size_t start_col,
                                           size_t end_col)
{
    std::ostringstream oss;
    oss << "INSERT INTO " << escape_sqlite_identifier(table_name) << " (";

    size_t max_col = std::min({end_col, fields.size(), row.values.size(), row.is_null.size()});

    for (size_t i = start_col; i < max_col; i++)
    {
        if (i > start_col)
            oss << ", ";
        oss << escape_sqlite_identifier(fields[i].field_name);
    }
    oss << ") VALUES (";

    for (size_t i = start_col; i < max_col; i++)
    {
        if (i > start_col)
            oss << ", ";
        if (row.is_null[i])
        {
            oss << "NULL";
        }
        else
        {
            const auto &v = row.values[i];
            if (sqlite_is_blob_field(fields[i]))
            {
                oss << bytes_to_hex_sqlite(v);
            }
            else if (v.empty())
            {
                oss << "''";
            }
            else
            {
                oss << "'" << escape_sqlite_string(v) << "'";
            }
        }
    }
    oss << ")";
    return oss.str();
}

// ===================== C++20 类型安全连接封装 =====================

// 类型安全连接变体: 任意时刻只持有一种数据库连接
using db_conn_variant = std::variant<
    std::shared_ptr<orm::mysql_conn_base>,
    std::shared_ptr<orm::pg_conn_base>,
    std::shared_ptr<orm::sqlite_conn_base>>;

// 连接创建结果 (含错误信息)
struct db_conn_result
{
    db_conn_variant conn;
    std::string error_msg;

    bool ok() const { return error_msg.empty(); }

    // 类型安全的连接访问 (编译期检查)
    template <typename T>
    std::shared_ptr<T> get() const
    {
        return std::get<std::shared_ptr<T>>(conn);
    }

    // 运行时按类型获取 DB_TYPE
    DB_TYPE type() const
    {
        return std::visit([](const auto &p) -> DB_TYPE
                          {
            using T = std::decay_t<decltype(p)>;
            if constexpr (std::is_same_v<T, std::shared_ptr<orm::mysql_conn_base>>)
                return DB_TYPE::MYSQL;
            else if constexpr (std::is_same_v<T, std::shared_ptr<orm::pg_conn_base>>)
                return DB_TYPE::POSTGRESQL;
            else
                return DB_TYPE::SQLITE; },
                          conn);
    }
};

// 数据库类型显示名 (三库统一)
inline std::string db_type_display(DB_TYPE t)
{
    if (t == DB_TYPE::POSTGRESQL)
        return "PostgreSQL";
    if (t == DB_TYPE::SQLITE)
        return "SQLite";
    return "MySQL";
}

// 按数据库类型引用标识符 (MySQL 反引号, PG/SQLite 双引号)
inline std::string quote_identifier(DB_TYPE type, const std::string &identifier)
{
    if (type == DB_TYPE::MYSQL)
        return "`" + identifier + "`";
    return escape_pg_identifier(identifier);// PG/SQLite 共用双引号逻辑
}

// 统一连接创建 (替代三处重复的连接代码)
inline db_conn_result create_connection(
    const orm::orm_conn_t &config,
    DB_TYPE type,
    asio::io_context &ioc)
{
    db_conn_result result;

    if (type == DB_TYPE::MYSQL)
    {
        auto pool          = std::make_shared<orm::orm_conn_pool>();
        pool->io_context   = &ioc;
        pool->conf_data[0] = config;
        try
        {
            result.conn = pool->add_mysql_edit_connect();
        }
        catch (const std::exception &e)
        {
            result.error_msg = std::string("MySQL connect failed: ") + e.what();
            return result;
        }
    }
    else if (type == DB_TYPE::POSTGRESQL)
    {
        auto pg = std::make_shared<orm::pg_conn_base>(
            orm::orm_conn_link_t::create(ioc, orm::DB_TYPE::POSTGRESQL));
        orm::orm_conn_t pg_cfg = config;
        if (pg_cfg.port.empty())
            pg_cfg.port = "5432";
        if (!pg->connect(pg_cfg))
        {
            result.error_msg = "PostgreSQL connect failed: " + pg->error_msg;
            return result;
        }
        result.conn = std::move(pg);
    }
    else// SQLITE
    {
        auto sqlite = std::make_shared<orm::sqlite_conn_base>();
        if (!sqlite->connect(config))
        {
            result.error_msg = "SQLite connect failed: " + sqlite->error_msg;
            return result;
        }
        result.conn = std::move(sqlite);
    }

    return result;
}

// ===================== 统一操作分发函数 =====================

// 统一获取表列表
inline std::vector<std::string> get_tables(const db_conn_variant &conn, DB_TYPE type)
{
    switch (type)
    {
    case DB_TYPE::MYSQL:
    {
        auto mysql = std::get<std::shared_ptr<orm::mysql_conn_base>>(conn);
        std::vector<row_data_t> rows;
        std::vector<std::string> tables;
        if (mysql_fetch_rows(mysql, "SHOW TABLES", rows))
        {
            for (auto &r : rows)
                if (!r.values.empty())
                    tables.push_back(r.values[0]);
        }
        return tables;
    }
    case DB_TYPE::POSTGRESQL:
    {
        auto pg = std::get<std::shared_ptr<orm::pg_conn_base>>(conn);
        std::vector<row_data_t> rows;
        std::vector<std::string> tables;
        if (pg_fetch_rows(pg,
                          "SELECT tablename FROM pg_tables WHERE schemaname='public' ORDER BY tablename",
                          rows))
        {
            for (auto &r : rows)
                if (!r.values.empty())
                    tables.push_back(r.values[0]);
        }
        return tables;
    }
    case DB_TYPE::SQLITE:
        return get_sqlite_tables(std::get<std::shared_ptr<orm::sqlite_conn_base>>(conn));
    }
    return {};
}

// 统一获取表结构
inline bool get_table_schema(const db_conn_variant &conn, DB_TYPE type, const std::string &table_name, db_table_info &info)
{
    switch (type)
    {
    case DB_TYPE::MYSQL:
        return mysql_get_table_schema(std::get<std::shared_ptr<orm::mysql_conn_base>>(conn), table_name, info);
    case DB_TYPE::POSTGRESQL:
        return pg_get_table_schema(std::get<std::shared_ptr<orm::pg_conn_base>>(conn), table_name, info);
    case DB_TYPE::SQLITE:
        return sqlite_get_table_schema(std::get<std::shared_ptr<orm::sqlite_conn_base>>(conn), table_name, info);
    }
    return false;
}

// 统一执行查询获取行
inline bool fetch_rows(const db_conn_variant &conn, DB_TYPE type, const std::string &sql, std::vector<row_data_t> &rows)
{
    switch (type)
    {
    case DB_TYPE::MYSQL:
        return mysql_fetch_rows(std::get<std::shared_ptr<orm::mysql_conn_base>>(conn), sql, rows);
    case DB_TYPE::POSTGRESQL:
        return pg_fetch_rows(std::get<std::shared_ptr<orm::pg_conn_base>>(conn), sql, rows);
    case DB_TYPE::SQLITE:
        return sqlite_fetch_rows(std::get<std::shared_ptr<orm::sqlite_conn_base>>(conn), sql, rows);
    }
    return false;
}

// 统一检查表是否存在
inline bool table_exists(const db_conn_variant &conn, DB_TYPE type, const std::string &table_name, const std::string &dbname = "")
{
    std::string sql;
    switch (type)
    {
    case DB_TYPE::MYSQL:
        sql = "SELECT 1 FROM information_schema.TABLES WHERE TABLE_SCHEMA='" +
              dbname + "' AND TABLE_NAME='" + table_name + "'";
        break;
    case DB_TYPE::POSTGRESQL:
        sql = "SELECT 1 FROM information_schema.tables WHERE table_schema='public' AND table_name='" +
              table_name + "'";
        break;
    case DB_TYPE::SQLITE:
        sql = "SELECT 1 FROM sqlite_master WHERE type='table' AND name='" + table_name + "'";
        break;
    }
    std::vector<row_data_t> rows;
    if (!fetch_rows(conn, type, sql, rows))
        return false;
    return !rows.empty();
}

// 统一执行 DDL (处理三种完全不同的执行协议)
inline bool exec_ddl(const db_conn_variant &conn, DB_TYPE type, const std::string &ddl, std::string &error_msg)
{
    switch (type)
    {
    case DB_TYPE::MYSQL:
    {
        auto mysql = std::get<std::shared_ptr<orm::mysql_conn_base>>(conn);
        if (mysql->write_sql(ddl) == 0)
        {
            error_msg = mysql->error_msg;
            return false;
        }
        unsigned int rn = mysql->read_loop();
        if (rn > 0)
        {
            orm::pack_info_t resp;
            resp.seq_id      = 1;
            unsigned int off = 0;
            mysql->read_field_pack(mysql->_cache_data, rn, off, resp);
            if (resp.error == 0 && resp.data.size() > 0 &&
                (unsigned char)resp.data[0] == 0xFF)
            {
                error_msg = resp.data.substr(1);
                return false;
            }
        }
        return true;
    }
    case DB_TYPE::POSTGRESQL:
    {
        auto pg = std::get<std::shared_ptr<orm::pg_conn_base>>(conn);
        std::vector<orm::field_info_t> dummy_f;
        std::vector<orm::pg_row_data_t> dummy_r;
        unsigned int affected = 0;
        unsigned int err      = pg->execute_and_fetch(ddl, dummy_f, dummy_r, affected);
        if (err > 0)
        {
            error_msg = pg->error_msg;
            return false;
        }
        return true;
    }
    case DB_TYPE::SQLITE:
    {
        auto sqlite = std::get<std::shared_ptr<orm::sqlite_conn_base>>(conn);
        if (sqlite->exec_sql(ddl) < 0)
        {
            error_msg = sqlite->error_msg;
            return false;
        }
        return true;
    }
    }
    return false;
}

// 统一插入单行
inline bool insert_row(const db_conn_variant &conn, DB_TYPE type, const std::string &table_name, const std::vector<db_field_info> &fields, const row_data_t &row, std::string &error_msg)
{
    switch (type)
    {
    case DB_TYPE::MYSQL:
    {
        auto mysql = std::get<std::shared_ptr<orm::mysql_conn_base>>(conn);
        auto sql   = build_insert_sql_mysql(table_name, fields, row, 0, row.values.size());
        if (mysql->write_sql(sql) == 0)
        {
            error_msg = mysql->error_msg;
            return false;
        }
        unsigned int rn = mysql->read_loop();
        if (rn == 0)
        {
            error_msg = mysql->error_msg;
            return false;
        }
        return true;
    }
    case DB_TYPE::POSTGRESQL:
    {
        auto pg  = std::get<std::shared_ptr<orm::pg_conn_base>>(conn);
        auto sql = build_insert_sql_pg(table_name, fields, row, 0, row.values.size());
        std::vector<orm::field_info_t> df;
        std::vector<orm::pg_row_data_t> dr;
        unsigned int affected = 0;
        unsigned int err      = pg->execute_and_fetch(sql, df, dr, affected);
        if (err > 0)
        {
            error_msg = pg->error_msg;
            return false;
        }
        return true;
    }
    case DB_TYPE::SQLITE:
    {
        auto sqlite = std::get<std::shared_ptr<orm::sqlite_conn_base>>(conn);
        if (!sqlite_insert_row(sqlite, table_name, fields, row))
        {
            error_msg = sqlite->error_msg;
            return false;
        }
        return true;
    }
    }
    return false;
}

// 统一 AUTOINCREMENT 重置
inline bool reset_autoincrement(const db_conn_variant &conn, DB_TYPE type, const std::string &table_name, const std::string &auto_inc_field, std::string &error_msg)
{
    switch (type)
    {
    case DB_TYPE::MYSQL:
    {
        auto mysql          = std::get<std::shared_ptr<orm::mysql_conn_base>>(conn);
        std::string max_sql = "SELECT MAX(`" + auto_inc_field + "`) FROM `" + table_name + "`";
        std::vector<row_data_t> max_rows;
        if (mysql_fetch_rows(mysql, max_sql, max_rows) &&
            !max_rows.empty() && !max_rows[0].values.empty() &&
            !max_rows[0].values[0].empty() && !max_rows[0].is_null[0])
        {
            try
            {
                unsigned long long max_val = std::stoull(max_rows[0].values[0]);
                std::string reset_sql      = "ALTER TABLE `" + table_name +
                                        "` AUTO_INCREMENT = " + std::to_string(max_val + 1);
                if (mysql->write_sql(reset_sql) == 0)
                {
                    error_msg = mysql->error_msg;
                    return false;
                }
            }
            catch (...)
            {
            }
        }
        return true;
    }
    case DB_TYPE::POSTGRESQL:
    {
        auto pg               = std::get<std::shared_ptr<orm::pg_conn_base>>(conn);
        std::string seq_name  = table_name + "_" + auto_inc_field + "_seq";
        std::string alter_sql = "SELECT setval('" + seq_name +
                                "', (SELECT MAX(" + auto_inc_field + ") FROM " + table_name + "))";
        std::vector<orm::field_info_t> df;
        std::vector<orm::pg_row_data_t> dr;
        unsigned int affected = 0;
        unsigned int err      = pg->execute_and_fetch(alter_sql, df, dr, affected);
        if (err > 0)
        {
            error_msg = pg->error_msg;
            return false;
        }
        return true;
    }
    case DB_TYPE::SQLITE:
    {
        auto sqlite = std::get<std::shared_ptr<orm::sqlite_conn_base>>(conn);
        std::string reset_sql =
            "UPDATE sqlite_sequence SET seq = (SELECT MAX(" +
            escape_sqlite_identifier(auto_inc_field) + ") FROM " +
            escape_sqlite_identifier(table_name) +
            ") WHERE name = '" + escape_sqlite_string(table_name) + "'";
        if (sqlite->exec_sql(reset_sql) < 0)
        {
            error_msg = sqlite->error_msg;
            return false;
        }
        return true;
    }
    }
    return false;
}

}// namespace dbtypes

#endif
