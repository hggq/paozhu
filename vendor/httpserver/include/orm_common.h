#ifndef _ORM_COMMON_H
#define _ORM_COMMON_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#include "session_buf.h"
#include "request.h"
#include "unicode.h"

// ---- type definitions ----
struct _row_data_cache_ptr_t
{
    unsigned char *ptr = nullptr;
    size_t len         = 0;
};

#define ORM_EXPAND(x) x

#define ORM_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define ORM_NARGS(...) ORM_EXPAND(ORM_ARG_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

#define ORM_CAT_(a, b) a##b
#define ORM_CAT(a, b) ORM_CAT_(a, b)

// #x must appear directly in each FE macro
#define ORM_FE_1(x) #x
#define ORM_FE_2(x, ...) #x, ORM_EXPAND(ORM_FE_1(__VA_ARGS__))
#define ORM_FE_3(x, ...) #x, ORM_EXPAND(ORM_FE_2(__VA_ARGS__))
#define ORM_FE_4(x, ...) #x, ORM_EXPAND(ORM_FE_3(__VA_ARGS__))
#define ORM_FE_5(x, ...) #x, ORM_EXPAND(ORM_FE_4(__VA_ARGS__))
#define ORM_FE_6(x, ...) #x, ORM_EXPAND(ORM_FE_5(__VA_ARGS__))
#define ORM_FE_7(x, ...) #x, ORM_EXPAND(ORM_FE_6(__VA_ARGS__))
#define ORM_FE_8(x, ...) #x, ORM_EXPAND(ORM_FE_7(__VA_ARGS__))
#define ORM_FE_9(x, ...) #x, ORM_EXPAND(ORM_FE_8(__VA_ARGS__))
#define ORM_FE_10(x, ...) #x, ORM_EXPAND(ORM_FE_9(__VA_ARGS__))

#define ORM_NAMES(...)                                                                            \
    static constexpr std::size_t field_count =                                                    \
        ::orm::count_fields({ORM_EXPAND(ORM_CAT(ORM_FE_, ORM_NARGS(__VA_ARGS__))(__VA_ARGS__))}); \
    static constexpr auto field_names =                                                           \
        ::orm::make_field_names<field_count>(                                                     \
            {ORM_EXPAND(ORM_CAT(ORM_FE_, ORM_NARGS(__VA_ARGS__))(__VA_ARGS__))})

namespace orm
{

// begin reflect
template <typename T>
concept ResultHasSetVal = requires(T &obj, const std::string &s, const unsigned char *p, std::size_t n, unsigned char t) {
    obj.set_val(s, p, n, t);
};

template <typename F, typename T>
concept RecordLineCallback = requires(F &&f, T &obj, const std::string &name, const unsigned char *data, std::size_t len, unsigned char field_type, unsigned char flag) {
    { std::invoke(std::forward<F>(f), obj, name, data, len, field_type, flag) }
    -> std::same_as<void>;
};
template <typename T>
concept HasOrgTablename = requires {
    T::org_tablename;
};

//ORM STRUCT REFLECT BEGIN

consteval std::size_t count_fields(std::initializer_list<const char *> names)
{
    return names.size();
}

template <std::size_t N>
consteval std::array<const char *, N> make_field_names(std::initializer_list<const char *> names)
{
    std::array<const char *, N> result{};
    std::size_t i = 0;
    for (auto name : names)
        result[i++] = name;
    return result;
}

// ======================== lightweight reflection core ========================
namespace lite_reflect
{

template <typename T, std::size_t N>
struct StructToTuple;

#define DEFINE_TUPLE(N, ...)                           \
    template <typename T>                              \
    struct StructToTuple<T, N>                         \
    {                                                  \
        static auto apply(T &t) -> decltype(auto)      \
        {                                              \
            auto &[__VA_ARGS__] = t;                   \
            return std::forward_as_tuple(__VA_ARGS__); \
        }                                              \
    }

DEFINE_TUPLE(1, f0);
DEFINE_TUPLE(2, f0, f1);
DEFINE_TUPLE(3, f0, f1, f2);
DEFINE_TUPLE(4, f0, f1, f2, f3);
DEFINE_TUPLE(5, f0, f1, f2, f3, f4);
DEFINE_TUPLE(6, f0, f1, f2, f3, f4, f5);
DEFINE_TUPLE(7, f0, f1, f2, f3, f4, f5, f6);
DEFINE_TUPLE(8, f0, f1, f2, f3, f4, f5, f6, f7);
DEFINE_TUPLE(9, f0, f1, f2, f3, f4, f5, f6, f7, f8);
DEFINE_TUPLE(10, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9);
#undef DEFINE_TUPLE

template <typename T>
concept Reflectable = requires {
    { T::field_count } -> std::convertible_to<std::size_t>;
    { T::field_names };
};

template <Reflectable T, typename Func>
constexpr void for_each_field(T &obj, Func &&func)
{
    constexpr std::size_t N = T::field_count;
    static_assert(N <= 10, "Please add more field count specializations for StructToTuple");
    auto refs = StructToTuple<T, N>::apply(obj);
    [&]<std::size_t... I>(std::index_sequence<I...>)
    {
        (func(std::get<I>(refs), std::integral_constant<std::size_t, I>{}), ...);
    }(std::make_index_sequence<N>{});
}

template <typename T>
concept NumericButNotBool =
    (std::integral<T> || std::floating_point<T>) && !std::is_same_v<T, bool>;

}// namespace lite_reflect

template <typename Derived>
struct Base
{
    unsigned int from_json(std::string_view json_content)
    {
        // single-level JSON object only: { }

        unsigned int offset_ = 0;

        for (; offset_ < json_content.size(); offset_++)
        {
            if (json_content[offset_] == ' ' || json_content[offset_] == '\t' || json_content[offset_] == 0x0D || json_content[offset_] == 0x0A)
            {
                continue;
            }
            break;
        }
        if ((offset_ + 1) >= json_content.size())
        {
            return json_content.size();
        }
        if (json_content[offset_] != '{')
        {
            return json_content.size();
        }
        offset_++;

        std::string json_key_name;
        std::string json_value_name;
        for (; offset_ < json_content.size(); offset_++)
        {
            if (json_content[offset_] != '"')
            {

                return offset_;
            }

            offset_++;
            for (; offset_ < json_content.size(); offset_++)
            {
                if (json_content[offset_] == '"')
                {
                    // end of JSON key
                    offset_++;
                    break;
                }
                json_key_name.push_back(json_content[offset_]);
            }
            // skip whitespace
            for (; offset_ < json_content.size(); offset_++)
            {
                if (json_content[offset_] == ' ' || json_content[offset_] == '\t' || json_content[offset_] == 0x0D || json_content[offset_] == 0x0A)
                {
                    continue;
                }
                break;
            }

            if (offset_ < json_content.size() && json_content[offset_] != ':')
            {
                // not JSON
                return json_content.size();
            }
            offset_++;
            // skip whitespace
            for (; offset_ < json_content.size(); offset_++)
            {
                if (json_content[offset_] == ' ' || json_content[offset_] == '\t' || json_content[offset_] == 0x0D || json_content[offset_] == 0x0A)
                {
                    continue;
                }
                break;
            }
            // JSON value, nested arrays unsupported
            if (offset_ < json_content.size() && json_content[offset_] == '[')
            {
                // not JSON
                return json_content.size();
            }
            if (offset_ < json_content.size() && json_content[offset_] == '"')
            {
                // string value
                offset_++;
                for (; offset_ < json_content.size(); offset_++)
                {
                    if (json_content[offset_] == '"')
                    {
                        if (json_content[offset_ - 1] == '\\')
                        {
                            json_value_name.push_back(json_content[offset_]);
                            continue;
                        }
                        else
                        {
                            for (; offset_ < json_content.size(); offset_++)
                            {
                                if (json_content[offset_] == ',')
                                {
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    json_value_name.push_back(json_content[offset_]);
                }
                json_value_name = http::json_str_to_utf8(json_value_name);
            }
            else
            {
                for (; offset_ < json_content.size(); offset_++)
                {
                    if (json_content[offset_] == ',')
                    {
                        break;
                    }
                    else if (json_content[offset_] == ' ')
                    {
                        for (; offset_ < json_content.size(); offset_++)
                        {
                            if (json_content[offset_] == ',')
                            {
                                break;
                            }
                        }
                        break;
                    }
                    json_value_name.push_back(json_content[offset_]);
                }
            }

            if (offset_ < json_content.size() && json_content[offset_] == ',')
            {
                // skip ',' and following whitespace
                offset_++;
                for (; offset_ < json_content.size(); offset_++)
                {
                    if (json_content[offset_] == ' ' || json_content[offset_] == '\t' || json_content[offset_] == 0x0D || json_content[offset_] == 0x0A)
                    {
                        continue;
                    }
                    break;
                }
                // step back so the next round sees the opening '"'
                offset_--;
            }
            // dispatch the parsed key/value
            set_val(json_key_name, (const unsigned char *)json_value_name.data(), json_value_name.size(), 0);

            json_key_name.clear();
            json_value_name.clear();
        }

        return offset_;
    }
    std::string to_json() const
    {
        auto &self = static_cast<const Derived &>(*this);
        std::string result;
        result.reserve(128);
        result += '{';

        bool first = true;
        lite_reflect::for_each_field(self, [&](const auto &field, auto idx)
                                     {
            if (!first) result += ',';
            first = false;
            
            result += '"';
            result += Derived::field_names[idx.value];
            result += "\":";
            
            result += http::to_json_value(field); });

        result += '}';
        return result;
    }

    void set_val(const std::string &name, const unsigned char *buf, std::size_t length, unsigned char field_type)
    {
        auto &self = static_cast<Derived &>(*this);
        lite_reflect::for_each_field(self, [&](auto &field, auto idx)
                                     {
            if (http::str_colname_casecmp(name,Derived::field_names[idx.value])) {
                http::try_set_val(field,buf,length,field_type);
                return;
            } });
    }
};

// end reflect

using col_value_variant = std::variant<
    std::monostate,
    int64_t,
    uint64_t,
    double,
    std::string_view>;
constexpr std::size_t VI_MONO = 0, VI_INT = 1, VI_UINT = 2, VI_DBL = 3, VI_SV = 4;
template <typename H>
concept BinaryRowHandler = requires(H h, int cc, char **nm, std::function<col_value_variant(int)> gd) {
    { h(cc, nm, gd) } -> std::convertible_to<bool>;
};

enum class DB_TYPE
{
    MYSQL,
    POSTGRESQL,
    SQLITE
};

enum class wq
{
    qb = 0,
    bt = 1,
    be,
    eq,
    nq,
    lt,
    le,
    in,
    like,
    llike,
    rlike,
    nlike,
    notin,
    isnull,
    notnull
};

/*
 * Column-name parameter type (orm::table_col): validates md.where("userid", 123) at compile time.
 *
 *   string literal  -> ① consteval ctor, name checked at compile time (typo/unknown => error)
 *   runtime string  -> ② plain ctor, passed through; the runtime col_idx==255 branch keeps
 *                      the pre-change behavior
 *
 * No extra overload needed: a literal is const char[N], and binding an array reference is an
 * exact match, which beats the string_view user-defined conversion, so ① always wins for
 * literals while variables/pointers fall through to ②. Adding an overload instead would make
 * both candidates user-defined conversions => ambiguous.
 *
 * Why the Names template param (idx != 255 is not enough): findcolpos only dispatches on
 * "first letter (lowercased) + length" and never compares for equality, so typos slip through:
 *     findcolpos("title") == findcolpos("titel") == 6      // same letter + same length
 *     findcolpos("userid") == findcolpos("usrid") == 3     // 'u' branch ignores length
 * col_names lives in <table>_info, not in the base class, so the caller passes it as a pointer
 * NTTP: &B_BASE::col_names, which the generator rewrites to &<table>_info::col_names.
 *
 * Usage: change whereXxx's std::string_view wq parameter to
 *        orm::table_col<B_BASE, &B_BASE::col_names> wq; the body stays untouched.
 */
template <class Base, auto *Names>
consteval std::string_view checked_col(std::string_view coln)
{
    const auto idx = Base::findcolpos(coln);
    if (idx == 255)
    {
        throw "orm: field is not table column";// ① no column with that letter/length
    }
    if ((*Names)[idx] != coln)
    {
        throw "orm: column name misspelled";// ② same length / shorter typo
    }
    return coln;
}

template <class Base, auto *Names>
struct table_col
{
    std::string_view sv;

    // ① string literal: checked at compile time.
    //    Error form: "call to consteval function ... is not a constant expression" at the call
    //    site, plus a note pointing at the failing throw above (unknown column vs. typo).
    //    static_assert is not usable: whether placed in the callee or in the consteval ctor,
    //    the compiler reports "function parameter 'coln' with unknown value cannot be used in
    //    a constant expression", breaking valid calls too.
    template <std::size_t N>
    consteval table_col(const char (&coln)[N]) : sv(checked_col<Base, Names>(std::string_view(coln, N - 1)))
    {
    }

    // ①' mutable char array, e.g. char buf[N]="userid"; md.where(buf, 1):
    //     not a literal, so it must pass as a runtime string; otherwise it binds to ① and
    //     fails with "not a constant expression" (legal before this change => regression).
    //     char(&)[N] is an exact match for a mutable array, beating ①'s const char(&)[N].
    //     Length must be truncated at '\0' via std::string_view(coln), not N-1 (the buffer
    //     may be longer than the string).
    //     Note: a non-constexpr const char buf[]="userid" still goes to ① and fails; that is
    //     the inherent trade-off of enforcing compile-time checks on literals.
    template <std::size_t N>
    constexpr table_col(char (&coln)[N]) noexcept : sv(coln)
    {
    }

    // ② runtime string (std::string / std::string_view / const char*): pass through
    template <class S, class = std::enable_if_t<std::is_convertible_v<const S &, std::string_view> && !std::is_array_v<std::remove_cvref_t<S>>>>
    constexpr table_col(const S &coln) noexcept : sv(coln)
    {
    }

    constexpr operator std::string_view() const noexcept { return sv; }
    constexpr operator std::string() const { return std::string(sv); }
};

// For std::stringstream/std::ostream: operator<< is a template and template argument deduction
// ignores user-defined conversions, so an explicit overload is required.
template <class Base, auto *Names>
inline std::ostream &operator<<(std::ostream &os, const table_col<Base, Names> &c)
{
    return os << c.sv;
}

struct orm_conn_t
{
    // std::string type;
    std::string host;
    std::string port;
    std::string dbname;
    std::string user;
    std::string password;
    std::string pretable;
    std::string dbtype;

    std::string tag;
    std::string charset;

    bool isssl = false;
    bool sslverify =
        false;// verify the server cert chain (system CA or SSL_CERT_FILE)
    std::string
        sslhost;                  // cert hostname/SNI (name check); empty => chain check only
    std::string server_public_key;// pinned server RSA key for MySQL non-SSL auth (PEM path or inline PEM); empty => no check
    bool issock               = false;
    bool isdebug              = false;
    bool islocal              = false;
    unsigned char link_type   = 0;// 0=edit 1=select 2=backup
    unsigned char max_pool    = 0;
    unsigned char min_pool    = 0;
    unsigned char charset_val = 0;
    DB_TYPE db_type           = DB_TYPE::MYSQL;

    bool operator==(const orm_conn_t &o) const
    {
        return std::tie(host, port, dbname, user, password, pretable, dbtype, tag, charset, sslhost, server_public_key, isssl, sslverify, issock, isdebug, islocal, link_type, max_pool, min_pool, charset_val, db_type) ==
               std::tie(o.host, o.port, o.dbname, o.user, o.password, o.pretable, o.dbtype, o.tag, o.charset, o.sslhost, o.server_public_key, o.isssl, o.sslverify, o.issock, o.isdebug, o.islocal, o.link_type, o.max_pool, o.min_pool, o.charset_val, o.db_type);
    }
    bool operator!=(const orm_conn_t &o) const { return !(*this == o); }
};

struct pack_info_t
{
    unsigned int length         = 0;
    unsigned int current_length = 0;
    unsigned char seq_id        = 0;
    unsigned char error         = 0;
    unsigned char padd_length   = 0;
    unsigned char padd_str[5]   = {0x00};
    std::string data;
};

struct field_info_t
{
    std::string org_name;
    std::string name;
    std::string org_table;
    std::string table;
    std::string dbname;
    unsigned char field_type   = 0;
    unsigned char decimals     = 0;
    unsigned short charset     = 0;
    unsigned short flags       = 0;
    unsigned int column_length = 0;
};

// result row data (shared by PG / SQLite)
struct pg_row_data_t
{
    std::vector<std::string> values;// per-column values (NULL = empty string + is_null flag)
    std::vector<bool> is_null;
};

// SQLite result set (values/is_null run parallel; BLOB kept as raw bytes)
struct sqlite_query_result
{
    std::vector<std::string> column_names;
    std::vector<std::vector<std::string>> rows;
    std::vector<std::vector<bool>> is_null;
    int affected_rows = 0;
};

// SQLite scalar result (e.g. SELECT COUNT(*))
struct sqlite_scalar_result
{
    std::string value;
    bool is_null = true;
};

// SQLite bind param (binary safe)
struct sqlite_bind_param
{
    int type = 0;// SQLITE_TEXT=3, SQLITE_INTEGER=1, ...
    std::string data;
};

struct table_columns_info_t
{
    std::string col_name;
    std::string comment;
    std::string default_value;
    std::string foreign_key;
    std::string foreign_table;
    unsigned char col_type  = 0;
    unsigned int col_length = 0;
    unsigned char decimals  = 0;
    unsigned char big_type  = 1;// string/number/float; default to string to avoid reading uninitialized codes
    bool is_pk              = false;
    bool is_auto_inc        = false;
    bool is_unsigned        = false;
    bool is_datetime        = false;
    bool is_nullable        = true; // NULL allowed (default true, conservative)
    bool is_indexed         = false;// part of any index (PK/UNI/MUL)
};

struct orm_where_sql_t
{
    bool begin_sub        = false;// true: opening '(' before this condition
    bool end_sub          = false;// true: closing ')' after this condition
    wq op_type            = wq::qb;
    unsigned char pre_op  = 0;   // 0=none, 1=AND, 2=OR
    unsigned char col_idx = 255; // 255=invalid, 0-254=index into col_names
    bool need_quote       = true;// per column type; conservative true when col_idx==255
    std::string filed_name;
    http::obj_val filed_value;// BETWEEN not supported yet
};

// left join where
struct orm_where_join_t
{
    wq op_type              = wq::qb;
    unsigned char pre_op    = 0;   // 0=none, 1=AND, 2=OR
    unsigned char left_idx  = 255; // 255=invalid, 0-254=index into left col_names
    unsigned char right_idx = 255; // 255=invalid, 0-254=index into right col_names
    bool need_quote         = true;// per column type; conservative true when col_idx==255
    http::obj_val filed_value;     // simple ON value: id = 1, name = 'admin'; no complex forms
    std::string left_filed_name;
    std::string right_filed_name;
};

struct orm_left_join_t
{
    bool isleft = true;
    bool parsed = false;   // parse_leftjion already ran; avoid re-processing selectsql
    std::string selectsql; // joined table select fields
    std::string join_table;// joined table name
    std::string ordersql;
    std::string limitsql;
    std::string parbysql;
    using find_join_col_indexd_ptr_t                    = unsigned char (*)(std::string_view) noexcept;
    find_join_col_indexd_ptr_t find_join_col_indexd_ptr = nullptr;
    std::vector<orm_where_join_t> sub_sqls;// joined table sub-query WHERE (filled by joinWhere)
    std::vector<orm_where_join_t> on_sqls; // joined table ON conditions (filled by joinOn)
};

struct orm_conn_link_t
{
    std::unique_ptr<asio::ip::tcp::socket> socket;
    std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket;
    std::unique_ptr<asio::local::stream_protocol::socket> localsocket;
    std::shared_ptr<asio::ssl::context> ssl_context;
    asio::error_code ec;
    asio::io_context *io_ctx = nullptr;
    unsigned int begin_time  = 0;
    DB_TYPE db_type          = DB_TYPE::MYSQL;
    unsigned char sock_type  = 0;// 0=none, 1=tcp, 2=ssl, 3=local socket
    // factory: create a conn_link from the config
    static std::unique_ptr<orm_conn_link_t> create(asio::io_context &ioc,
                                                   DB_TYPE type);

    // close the connection
    void close();

    // whether the connection is closed
    bool is_closed() const;
};

std::string to_escape(std::string_view val, bool no_backslash_escapes = false);

// ---------------------------------------------------------------------------
// Accumulate-buffer capacity pool shared by the whole ORM (same design as session_buf in the
// tls13 project; shared after the two projects merged). Protocol parsing accumulates across
// reads (read_some returns at most CACHE_DATA_LENGTH bytes), and creating/destroying a buffer
// per query means repeated heap allocations; buffers are borrowed and returned instead, so the
// hot path is allocation-free once warm. MySQL / PG / SQLite all use this one pool.
// ---------------------------------------------------------------------------
inline session_buf g_orm_buf_pool;

// RAII: borrow an accumulate buffer on construction (topped up to 4096 if smaller) and return
// it on destruction — covers every early return path (errors, aborts), so nothing is leaked.
// Usage: pooled_accum_buf holder; auto &accum_buf = holder.buf;
struct pooled_accum_buf
{
    std::vector<uint8_t> buf;
    pooled_accum_buf()
    {
        buf = g_orm_buf_pool.acquire();
        if (buf.capacity() < 4096)
            buf.reserve(4096);
    }
    ~pooled_accum_buf() { g_orm_buf_pool.release(std::move(buf)); }
};

// Dialect identifier quoting (inline: each opsql.h includes orm_common.h once, no redefinition)
inline std::string escape_mysql_col(const std::string &col)
{
    std::string r;
    r.reserve(col.size() + 2);
    r.push_back('`');
    for (char c : col)
    {
        if (c == '`')
            r += "``";
        else
            r += c;
    }
    r.push_back('`');
    return r;
}
inline std::string escape_pg_col(const std::string &col)
{
    std::string r;
    r.reserve(col.size() + 2);
    r.push_back('"');
    for (char c : col)
    {
        if (c == '"')
            r += "\"\"";
        else
            r += c;
    }
    r.push_back('"');
    return r;
}
inline std::string escape_sqlite_col(const std::string &col)
{
    return escape_pg_col(col);// SQLite quotes like PG: double quotes
}

// shared helpers for the exec_* prepared entries

namespace detail
{

inline char ascii_lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

inline bool is_ident_char(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

// `$tag$` start detection: `$` + [A-Za-z_0-9]* + `$`; an empty tag (`$$`) counts as well.
// An all-digit tag is not a start delimiter: `$1`/`$2` are parameter references, and PG uses
// exactly this rule to tell the two apart.
inline std::size_t dollar_tag_len(std::string_view sql, std::size_t i)
{
    std::size_t j  = i + 1;
    bool any       = false;
    bool all_digit = true;
    while (j < sql.size() && is_ident_char(sql[j]))
    {
        any = true;
        if (sql[j] < '0' || sql[j] > '9')
            all_digit = false;
        ++j;
    }
    if (any && all_digit)
        return 0;
    return (j < sql.size() && sql[j] == '$') ? (j - i + 1) : 0;
}

// One scanner serves both count_qmark() and pg_qmark_to_dollar(); two copies would drift.
// When dst is non-null the `?`->`$n` result is written there; the return value is the real
// placeholder count. A `?` inside a literal, comment, quoted identifier or dollar-quoted string
// is passed through and not counted. Anything undecidable is left unconverted (D-2 default) and
// then fails explicitly in exec_bind_guard's count check — better an error than a placeholder
// silently left inside a string.
// WARNING: pg_dialect is a dialect switch, not decoration: "`?` followed by `? | & #` consumes
// 2 chars" holds only for PG (scan.l:367 treats `?` as a plain op_char, so `??`/`?|` are longer
// operator tokens). MySQL sql_lex.cc:1545 and SQLite tokenize.c:504 both count `??` as two
// tokens, so sharing that rule would wrongly reject valid MySQL/SQLite `SELECT ?,?`.
inline unsigned int qmark_scan(std::string_view sql, std::string *dst, bool pg_dialect)
{
    if (dst)
        dst->clear();
    const std::size_t n = sql.size();
    unsigned int ph     = 0;
    std::size_t i       = 0;
    while (i < n)
    {
        const char c = sql[i];

        if (c == '\'' || c == '"')
        {
            // only E'...' honors backslash escapes; in standard strings `\` is literal
            // (standard_conforming_strings=on)
            const bool bs_esc = (c == '\'') && i > 0 && ascii_lower(sql[i - 1]) == 'e' &&
                                !(i >= 2 && is_ident_char(sql[i - 2]));
            std::size_t j = i + 1;
            for (;;)
            {
                if (j >= n)
                {
                    j = n;
                    break;
                }
                if (bs_esc && sql[j] == '\\' && j + 1 < n)
                {
                    j += 2;
                    continue;
                }
                if (sql[j] == c)
                {
                    if (j + 1 < n && sql[j + 1] == c)// '' and "" are escaped quotes, not the end
                    {
                        j += 2;
                        continue;
                    }
                    ++j;
                    break;
                }
                ++j;
            }
            if (dst)
                dst->append(sql.data() + i, j - i);
            i = j;
            continue;
        }

        if (c == '-' && i + 1 < n && sql[i + 1] == '-')
        {
            std::size_t j = i + 2;
            while (j < n && sql[j] != '\n')
                ++j;
            if (dst)
                dst->append(sql.data() + i, j - i);
            i = j;
            continue;
        }

        if (c == '/' && i + 1 < n && sql[i + 1] == '*')
        {
            // PG block comments nest: exit only at depth 0; an unterminated one runs to the end
            std::size_t j      = i + 2;
            unsigned int depth = 1;
            while (j < n && depth)
            {
                if (j + 1 < n && sql[j] == '/' && sql[j + 1] == '*')
                {
                    ++depth;
                    j += 2;
                    continue;
                }
                if (j + 1 < n && sql[j] == '*' && sql[j + 1] == '/')
                {
                    --depth;
                    j += 2;
                    continue;
                }
                ++j;
            }
            if (dst)
                dst->append(sql.data() + i, j - i);
            i = j;
            continue;
        }

        if (c == '$')
        {
            const std::size_t tag = dollar_tag_len(sql, i);
            if (tag)
            {
                const std::size_t end = sql.find(sql.data() + i, i + tag, tag);
                const std::size_t j   = (end == std::string_view::npos) ? n : end + tag;
                if (dst)
                    dst->append(sql.data() + i, j - i);
                i = j;
                continue;
            }
        }

        if (c == '?')
        {
            // jsonb existence operators ?? ?| ?& ?#: in the PG lexer this `?` is part of an
            // operator, so all four two-char forms must consume 2 chars — otherwise the second
            // `?` of `??` would be read as a placeholder.
            // MySQL / SQLite (pg_dialect=false) do not follow this: `??` is two tokens there.
            if (pg_dialect && i + 1 < n)
            {
                const char nx = sql[i + 1];
                if (nx == '?' || nx == '|' || nx == '&' || nx == '#')
                {
                    if (dst)
                    {
                        dst->push_back('?');
                        dst->push_back(nx);
                    }
                    i += 2;
                    continue;
                }
            }
            ++ph;
            if (dst)
            {
                dst->push_back('$');
                *dst += std::to_string(ph);
            }
            ++i;
            continue;
        }

        if (dst)
            dst->push_back(c);
        ++i;
    }
    return ph;
}

}// namespace detail

// Counts placeholders only, producing no output string. MySQL / SQLite use it for the count
// check, hence pg_dialect = false.
inline unsigned int count_qmark(std::string_view sql)
{
    return detail::qmark_scan(sql, nullptr, false);
}

// rawsql writes `?` for all three backends; the PG extended protocol needs `$n`, so only the PG
// branch calls this. ph_count_out carries the placeholder count to avoid a second scan.
inline std::string pg_qmark_to_dollar(std::string_view sql, unsigned int &ph_count_out)
{
    std::string out;
    out.reserve(sql.size() + 8);
    ph_count_out = detail::qmark_scan(sql, &out, true);
    return out;
}

// Skip leading whitespace, `--` line comments and `/* */` block comments; an unterminated block
// comment means "no statement follows". Block comments are treated as non-nested, matching
// sql_has_no_statement() in src/sqlite_conn.cpp line by line: once the two skip rules drift, the
// same SQL looks like different statements to this guard and to the engine.
// WARNING: MySQL's `#` line comment is not recognized, so a statement starting with `# ...` is
// rejected as non-SELECT. Erring towards rejection avoids a second comment rule.
inline std::size_t sql_skip_leading_noise(std::string_view sql)
{
    const std::size_t n = sql.size();
    std::size_t i       = 0;
    for (;;)
    {
        while (i < n && (sql[i] == ' ' || sql[i] == '\t' || sql[i] == '\r' ||
                         sql[i] == '\n' || sql[i] == '\v' || sql[i] == '\f'))
            ++i;
        if (n - i >= 2 && sql[i] == '-' && sql[i + 1] == '-')
        {
            i += 2;
            while (i < n && sql[i] != '\n')
                ++i;
            continue;
        }
        if (n - i >= 2 && sql[i] == '/' && sql[i + 1] == '*')
        {
            const std::size_t end = sql.find("*/", i + 2);
            if (end == std::string_view::npos)
                return n;
            i = end + 2;
            continue;
        }
        return i;
    }
}

// Whether pos holds exactly the whole word (case-insensitive; a following letter/digit/'_'
// means no match).
inline bool sql_word_at(std::string_view sql, std::size_t pos, std::string_view word)
{
    if (pos + word.size() > sql.size())
        return false;
    for (std::size_t k = 0; k < word.size(); ++k)
        if (detail::ascii_lower(sql[pos + k]) != word[k])
            return false;
    return pos + word.size() == sql.size() || !detail::is_ident_char(sql[pos + word.size()]);
}

// Shape guard for exec_query / async_exec_query. Strict by default: with/show/describe/
// explain/pragma/table/values are all rejected; relaxing it means comparing a few more words
// here and nothing else changes.
inline bool sql_is_select(std::string_view sql)
{
    return sql_word_at(sql, sql_skip_leading_noise(sql), "select");
}

// Entry guard for exec_*. ph comes from the caller (PG: pg_qmark_to_dollar's out param, others:
// count_qmark) so no extra scan is needed for the check.
// WARNING: only sets error_msg, not the sticky iserror — iserror is reserved for pool-level or
// structural misuse; one bad argument must not poison the whole db_conn.
inline bool exec_bind_guard(unsigned int ph, const std::vector<http::obj_val> &params, std::string &error_msg)
{
    if (ph != params.size())
    {
        error_msg = "placeholder/param count mismatch: sql=" + std::to_string(ph) +
                    " params=" + std::to_string(params.size());
        return false;
    }
    for (const auto &p : params)
    {
        const http::obj_type t = p.get_type();
        if (t == http::obj_type::OBJECT || t == http::obj_type::ARRAY)
        {
            error_msg = "exec_*: OBJECT/ARRAY obj_val cannot be bound as a parameter";
            return false;
        }
    }
    return true;
}

// One row loop shared by the three backends' prepared text handlers and the legacy
// fetch_directly, so the 42 thin wrappers need no copy.
// land(rec, col_name, ptr, len, col_index) is the only injection point: ① passes an adapter
// calling `rec.set_val(name, ptr, len, 0)`, ② passes one wrapping the user callback.
// Both shapes mirror the legacy text entry: the vec version default-constructs a row,
// emplace_back and returns true; the single version lands into result_record and returns false
// (one row only). NULL columns (ptr == nullptr) are skipped: no landing call, field untouched,
// same as the legacy entry.
template <class Conn, class Sink, class Land>
unsigned int exec_fetch_rows(Conn &conn,
                             const std::string &sql,
                             const std::vector<http::obj_val> &params,
                             std::vector<Sink> &out,
                             unsigned int &effect_num,
                             Land land)
{
    return conn.fetch_prepared(
        sql,
        params,
        [&out, &effect_num, land, col_cache = std::vector<std::string>{}](
            int col_count,
            char **col_names,
            auto get_data) mutable -> bool
        {
            Sink data_temp;
            if (col_cache.empty() && col_count > 0)
            {
                col_cache.reserve(col_count);
                for (int k = 0; k < col_count; k++)
                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
            }
            for (int ij = 0; ij < col_count; ij++)
            {
                auto [ptr, len] = get_data(ij);
                if (ptr == nullptr)
                    continue;
                if (!col_cache[ij].empty())
                    land(data_temp, col_cache[ij], ptr, len, ij);
            }
            out.emplace_back(std::move(data_temp));
            effect_num++;
            return true;
        });
}

template <class Conn, class Sink, class Land>
unsigned int exec_fetch_single(Conn &conn,
                               const std::string &sql,
                               const std::vector<http::obj_val> &params,
                               Sink &out,
                               unsigned int &effect_num,
                               Land land)
{
    return conn.fetch_prepared(
        sql,
        params,
        [&out, &effect_num, land, col_cache = std::vector<std::string>{}](
            int col_count,
            char **col_names,
            auto get_data) mutable -> bool
        {
            if (col_cache.empty() && col_count > 0)
            {
                col_cache.reserve(col_count);
                for (int k = 0; k < col_count; k++)
                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
            }
            for (int ij = 0; ij < col_count; ij++)
            {
                auto [ptr, len] = get_data(ij);
                if (ptr == nullptr)
                    continue;
                if (!col_cache[ij].empty())
                    land(out, col_cache[ij], ptr, len, ij);
            }
            effect_num++;
            return false;
        });
}

// Async twin: same row loop with fetch_prepared replaced by co_await async_fetch_prepared.
// Landing stays synchronous (the handler is not a coroutine), so land's contract matches the
// sync version exactly.
template <class Conn, class Sink, class Land>
asio::awaitable<unsigned int> async_exec_fetch_rows(Conn &conn,
                                                    const std::string &sql,
                                                    const std::vector<http::obj_val> &params,
                                                    std::vector<Sink> &out,
                                                    unsigned int &effect_num,
                                                    Land land)
{
    co_return co_await conn.async_fetch_prepared(
        sql,
        params,
        [&out, &effect_num, land, col_cache = std::vector<std::string>{}](
            int col_count,
            char **col_names,
            auto get_data) mutable -> bool
        {
            Sink data_temp;
            if (col_cache.empty() && col_count > 0)
            {
                col_cache.reserve(col_count);
                for (int k = 0; k < col_count; k++)
                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
            }
            for (int ij = 0; ij < col_count; ij++)
            {
                auto [ptr, len] = get_data(ij);
                if (ptr == nullptr)
                    continue;
                if (!col_cache[ij].empty())
                    land(data_temp, col_cache[ij], ptr, len, ij);
            }
            out.emplace_back(std::move(data_temp));
            effect_num++;
            return true;
        });
}

template <class Conn, class Sink, class Land>
asio::awaitable<unsigned int> async_exec_fetch_single(Conn &conn,
                                                      const std::string &sql,
                                                      const std::vector<http::obj_val> &params,
                                                      Sink &out,
                                                      unsigned int &effect_num,
                                                      Land land)
{
    co_return co_await conn.async_fetch_prepared(
        sql,
        params,
        [&out, &effect_num, land, col_cache = std::vector<std::string>{}](
            int col_count,
            char **col_names,
            auto get_data) mutable -> bool
        {
            if (col_cache.empty() && col_count > 0)
            {
                col_cache.reserve(col_count);
                for (int k = 0; k < col_count; k++)
                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
            }
            for (int ij = 0; ij < col_count; ij++)
            {
                auto [ptr, len] = get_data(ij);
                if (ptr == nullptr)
                    continue;
                if (!col_cache[ij].empty())
                    land(out, col_cache[ij], ptr, len, ij);
            }
            effect_num++;
            return false;
        });
}

}// namespace orm

#endif
