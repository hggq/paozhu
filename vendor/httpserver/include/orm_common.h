#ifndef _ORM_COMMON_H
#define _ORM_COMMON_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "session_buf.h"
#include "request.h"
#include "unicode.h"

// ---- 类型定义 ----
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

// #x 必须直接写在每个 FE 宏里
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

// ======================== 2. 轻量级反射核心 ========================
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
        //一层json深度，只有 { } 对象

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
                    //json key name end;
                    offset_++;
                    break;
                }
                json_key_name.push_back(json_content[offset_]);
            }
            //去除空格
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
                //not json
                return json_content.size();
            }
            offset_++;
            //去除空格
            for (; offset_ < json_content.size(); offset_++)
            {
                if (json_content[offset_] == ' ' || json_content[offset_] == '\t' || json_content[offset_] == 0x0D || json_content[offset_] == 0x0A)
                {
                    continue;
                }
                break;
            }
            //json value,not []
            if (offset_ < json_content.size() && json_content[offset_] == '[')
            {
                //not json
                return json_content.size();
            }
            if (offset_ < json_content.size() && json_content[offset_] == '"')
            {
                //string value
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
                //跳过 , 消除空格
                offset_++;
                for (; offset_ < json_content.size(); offset_++)
                {
                    if (json_content[offset_] == ' ' || json_content[offset_] == '\t' || json_content[offset_] == 0x0D || json_content[offset_] == 0x0A)
                    {
                        continue;
                    }
                    break;
                }
                //恢复 " ，等待下一轮循环
                offset_--;
            }
            //处理json_key_name json_value_name
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
        false;// 是否校验服务器证书链（配合系统 CA 或 SSL_CERT_FILE）
    std::string
        sslhost;                  // 证书主机名/SNI（域名校验），为空则只校验证书链不校验域名
    std::string server_public_key;// MySQL 非 SSL 认证时钉住的服务器 RSA 公钥（PEM 路径或内联 PEM），为空则不校验
    bool issock               = false;
    bool isdebug              = false;
    bool islocal              = false;
    unsigned char link_type   = 0;// 0 edit 1 select 2 backup
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

// 结果集行数据 (PG / SQLite 共用)
struct pg_row_data_t
{
    std::vector<std::string> values;// 每列的值（NULL 用空字符串+is_null标记）
    std::vector<bool> is_null;
};

// SQLite 结果集 (values 与 is_null 一一对应, BLOB 以原始字节存于 string)
struct sqlite_query_result
{
    std::vector<std::string> column_names;
    std::vector<std::vector<std::string>> rows;
    std::vector<std::vector<bool>> is_null;
    int affected_rows = 0;
};

// SQLite 标量查询结果 (如 SELECT COUNT(*))
struct sqlite_scalar_result
{
    std::string value;
    bool is_null = true;
};

// SQLite 绑定参数 (用于参数化执行, 二进制安全)
struct sqlite_bind_param
{
    int type = 0;// SQLITE_TEXT = 3, SQLITE_INTEGER = 1, etc.
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
    unsigned char big_type  = 1;//string number float, 默认字符串类, 避免未分类类型码读未初始化值
    bool is_pk              = false;
    bool is_auto_inc        = false;
    bool is_unsigned        = false;
    bool is_datetime        = false;
    bool is_nullable        = true; // 字段是否允许 NULL（默认 true 保守）
    bool is_indexed         = false;// 字段是否在任何索引中 (PK/UNI/MUL)
};

struct orm_where_sql_t
{
    bool begin_sub        = false;// ture 为左边 ( sql开始
    bool end_sub          = false;// ture 为右边 ) sql结束
    wq op_type            = wq::qb;
    unsigned char pre_op  = 0;   //前面是 and or 链接, 如果为0 表示没有, 1 是AND 2是 OR
    unsigned char col_idx = 255; // 255=无效, 0-254=col_names索引
    bool need_quote       = true;// 根据列类型决定; col_idx==255 时保守取 true
    std::string filed_name;
    http::obj_val filed_value;// BETWEEN 先不做
};

//左连接where
struct orm_where_join_t
{
    wq op_type              = wq::qb;
    unsigned char pre_op    = 0;   //前面是 and or 链接, 如果为0 表示没有, 1 是AND 2是 OR
    unsigned char left_idx  = 255; // 255=无效, 0-254=左表 col_names索引
    unsigned char right_idx = 255; // 255=无效, 0-254=右表 col_names索引
    bool need_quote         = true;// 根据列类型决定; col_idx==255 时保守取 true
    http::obj_val filed_value;     //  on 后面 比如 id = 1; name = 'admin' 简单的，不考虑复杂的
    std::string left_filed_name;
    std::string right_filed_name;
};

struct orm_left_join_t
{
    bool isleft = true;
    bool parsed = false;   // parse_leftjion 已执行过，避免二次加工 selectsql
    std::string selectsql; //连接表选择字段
    std::string join_table;//连接表名
    std::string ordersql;
    std::string limitsql;
    std::string parbysql;
    using find_join_col_indexd_ptr_t                    = unsigned char (*)(std::string_view) noexcept;
    find_join_col_indexd_ptr_t find_join_col_indexd_ptr = nullptr;
    std::vector<orm_where_join_t> sub_sqls;// 连接表 子查询 WHERE (joinWhere 填充)
    std::vector<orm_where_join_t> on_sqls; // 连接表 ON 条件 (joinOn 填充)
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
    unsigned char sock_type  = 0;// 0: none, 1: tcp, 2: ssl, 3: local socket
    // 工厂方法：根据配置创建 conn_link
    static std::unique_ptr<orm_conn_link_t> create(asio::io_context &ioc,
                                                   DB_TYPE type);

    // 关闭连接
    void close();

    // 检查是否已关闭
    bool is_closed() const;
};

std::string to_escape(std::string_view val, bool no_backslash_escapes = false);

// ---------------------------------------------------------------------------
// 全 ORM 共用累积缓冲容量池（设计与 tls13 项目 session_buf 一致，两项目合并后共用）：
// 协议解析需要跨读次累积缓冲（read_some 每次最多 CACHE_DATA_LENGTH 字节），
// 每次查询新建/销毁会产生重复堆分配；改为池借出/归还，热路径稳定后零堆分配。
// MySQL / PG / SQLite 连接层统一使用本池，不再各自建池。
// ---------------------------------------------------------------------------
inline session_buf g_orm_buf_pool;

// RAII：构造时从容量池借出累积缓冲（不足 4096 则补齐首次容量），
// 析构时归还——覆盖错误/中止等所有提前返回路径，避免漏归还。
// 用法：pooled_accum_buf holder; auto &accum_buf = holder.buf;
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

// 方言标识符引用函数（inline，每个 opsql.h 只 include orm_common.h 一次，无重定义）
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
    return escape_pg_col(col);// SQLite 同 PG，双引号
}

}// namespace orm

#endif
