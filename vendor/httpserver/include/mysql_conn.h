#ifndef _MYSQL_CONN_BASE_H
#define _MYSQL_CONN_BASE_H
/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "unicode.h"

#define ORM_EXPAND(x) x
 

#define ORM_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#define ORM_NARGS(...) ORM_EXPAND(ORM_ARG_N(__VA_ARGS__,8,7,6,5,4,3,2,1))

#define ORM_CAT_(a, b) a##b
#define ORM_CAT(a, b)  ORM_CAT_(a, b)

// #x 必须直接写在每个 FE 宏里
#define ORM_FE_1(x)       #x
#define ORM_FE_2(x, ...)  #x, ORM_EXPAND(ORM_FE_1(__VA_ARGS__))
#define ORM_FE_3(x, ...)  #x, ORM_EXPAND(ORM_FE_2(__VA_ARGS__))
#define ORM_FE_4(x, ...)  #x, ORM_EXPAND(ORM_FE_3(__VA_ARGS__))
#define ORM_FE_5(x, ...)  #x, ORM_EXPAND(ORM_FE_4(__VA_ARGS__))
#define ORM_FE_6(x, ...)  #x, ORM_EXPAND(ORM_FE_5(__VA_ARGS__))
#define ORM_FE_7(x, ...)  #x, ORM_EXPAND(ORM_FE_6(__VA_ARGS__))
#define ORM_FE_8(x, ...)  #x, ORM_EXPAND(ORM_FE_7(__VA_ARGS__))

#define ORM_NAMES(...)                                                         \
    static constexpr std::size_t field_count =                                 \
        ::orm::count_fields({ORM_EXPAND(ORM_CAT(ORM_FE_, ORM_NARGS(__VA_ARGS__))(__VA_ARGS__))}); \
    static constexpr auto field_names =                                        \
        ::orm::make_field_names<field_count>(                                  \
            {ORM_EXPAND(ORM_CAT(ORM_FE_, ORM_NARGS(__VA_ARGS__))(__VA_ARGS__))})


namespace orm
{
template<typename T>
concept ResultHasSetVal = requires(T& obj, const std::string& s, const unsigned char* p, std::size_t n, unsigned char t) {
    obj.set_val(s, p, n, t);
};

template<typename F, typename T>
concept RecordLineCallback = requires(F&& f, T& obj,const std::string& name, 
                                  const unsigned char* data, std::size_t len, 
                                  unsigned char field_type, unsigned char flag) {
    { std::invoke(std::forward<F>(f), obj, name, data, len, field_type, flag) } 
        -> std::same_as<void>;
};
template<typename T>
concept HasOrgTablename = requires {
    T::org_tablename;
};

//ORM STRUCT REFLECT BEGIN

consteval std::size_t count_fields(std::initializer_list<const char*> names) {
    return names.size();
}

template <std::size_t N>
consteval std::array<const char*, N> make_field_names(std::initializer_list<const char*> names) {
    std::array<const char*, N> result{};
    std::size_t i = 0;
    for (auto name : names) result[i++] = name;
    return result;
}

// ======================== 2. 轻量级反射核心 ========================
namespace lite_reflect {

template <typename T, std::size_t N> struct StructToTuple;

#define DEFINE_TUPLE(N, ...)                                                   \
    template <typename T> struct StructToTuple<T, N> {                         \
        static auto apply(T &t) -> decltype(auto) {                            \
            auto &[__VA_ARGS__] = t;                                           \
            return std::forward_as_tuple(__VA_ARGS__);                         \
        }                                                                      \
    }

DEFINE_TUPLE(1, f0);
DEFINE_TUPLE(2, f0, f1);
DEFINE_TUPLE(3, f0, f1, f2);
DEFINE_TUPLE(4, f0, f1, f2, f3);
DEFINE_TUPLE(5, f0, f1, f2, f3, f4);
DEFINE_TUPLE(6, f0, f1, f2, f3, f4, f5);
DEFINE_TUPLE(7, f0, f1, f2, f3, f4, f5, f6);
DEFINE_TUPLE(8, f0, f1, f2, f3, f4, f5, f6, f7);
#undef DEFINE_TUPLE

template <typename T>
concept Reflectable = requires {
    { T::field_count } -> std::convertible_to<std::size_t>;
    { T::field_names };
};

template <Reflectable T, typename Func>
constexpr void for_each_field(T &obj, Func &&func) {
    constexpr std::size_t N = T::field_count;
    static_assert(N <= 8, "请为 StructToTuple 添加更多字段数特化");
    auto refs = StructToTuple<T, N>::apply(obj);
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (func(std::get<I>(refs), std::integral_constant<std::size_t, I>{}), ...);
    }(std::make_index_sequence<N>{});
}

template <typename T>
concept NumericButNotBool =
    (std::integral<T> || std::floating_point<T>) && !std::is_same_v<T, bool>;

} // namespace lite_reflect

template <typename Derived> 
struct Base {
        unsigned int from_json(std::string_view json_content) {
        
        //一层json深度，只有 { } 对象
        
        unsigned int offset_ = 0;
    
        for(; offset_<json_content.size(); offset_++)
        {
            if(json_content[offset_]==' '|| json_content[offset_]=='\t' || json_content[offset_]==0x0D || json_content[offset_]==0x0A)
            {
                continue;
            }
            break;
        }
        if((offset_+1) >= json_content.size())
        {
            return json_content.size();
        }
        if(json_content[offset_] != '{')
        {
            return json_content.size();
        }
        offset_++;

        std::string json_key_name;
        std::string json_value_name;
        for(; offset_<json_content.size(); offset_++)
        {
            if(json_content[offset_] != '"')
            {
                
                return offset_;
            }

            offset_++;
            for(; offset_<json_content.size(); offset_++)
            {
                if(json_content[offset_] == '"')
                {
                    //json key name end;
                    offset_++;
                    break;
                } 
                json_key_name.push_back(json_content[offset_]);
            }
            //去除空格
            for(; offset_<json_content.size(); offset_++)
            {
                if(json_content[offset_]==' '|| json_content[offset_]=='\t' || json_content[offset_]==0x0D || json_content[offset_]==0x0A)
                {
                    continue;
                }
                break;
            }

            if(offset_< json_content.size() && json_content[offset_] != ':')
            {
                //not json
                return json_content.size();
            }
            offset_++;
            //去除空格
            for(; offset_<json_content.size(); offset_++)
            {
                if(json_content[offset_]==' '|| json_content[offset_]=='\t' || json_content[offset_]==0x0D || json_content[offset_]==0x0A)
                {
                    continue;
                }
                break;
            }
            //json value,not []
            if(offset_< json_content.size() && json_content[offset_] == '[')
            {
                //not json
                return json_content.size();
            }
            if(offset_< json_content.size() && json_content[offset_] == '"')
            {
                //string value
                offset_++;
                for(; offset_<json_content.size(); offset_++)
                {
                    if(json_content[offset_] == '"')
                    {
                        if(json_content[offset_-1] == '\\')
                        {
                            json_value_name.push_back(json_content[offset_]);
                            continue;
                        }
                        else
                        {
                            for(; offset_<json_content.size(); offset_++)
                            {
                                if(json_content[offset_] == ',')
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
                for(; offset_<json_content.size(); offset_++)
                {
                    if(json_content[offset_] == ',')
                    {
                        break;
                    }
                    else if(json_content[offset_] == ' ')
                    {
                        for(; offset_<json_content.size(); offset_++)
                        {
                            if(json_content[offset_] == ',')
                            {
                                break;
                            }
                        }
                        break;
                    }
                    json_value_name.push_back(json_content[offset_]);
                }
            }

            if(offset_< json_content.size() && json_content[offset_] == ',')
            {
                //跳过 , 消除空格
                offset_++;
                for(; offset_<json_content.size(); offset_++)
                {
                    if(json_content[offset_]==' '|| json_content[offset_]=='\t' || json_content[offset_]==0x0D || json_content[offset_]==0x0A)
                    {
                        continue;
                    }
                    break;
                }
                //恢复 " ，等待下一轮循环
                offset_--;
            }
            //处理json_key_name json_value_name
            set_val(json_key_name, (const unsigned char *)json_value_name.data(),json_value_name.size(),0);
            
            json_key_name.clear();
            json_value_name.clear();
        }

        return offset_;
    }
    std::string to_json() const {
        auto &self = static_cast<const Derived &>(*this);
        std::string result;
        result.reserve(128); 
        result += '{';
        
        bool first = true;
        lite_reflect::for_each_field(self, [&](const auto &field, auto idx) {
            if (!first) result += ',';
            first = false;
            
            result += '"';
            result += Derived::field_names[idx.value];
            result += "\":";
            
            result += http::to_json_value(field);
        });
        
        result += '}';
        return result;
    }

    void set_val(const std::string &name,const unsigned char *buf, std::size_t length,unsigned char field_type) {
        auto &self = static_cast<Derived &>(*this);
        lite_reflect::for_each_field(self, [&](auto &field, auto idx) {
            if (http::str_colname_casecmp(name,Derived::field_names[idx.value])) {
                http::try_set_val(field,buf,length,field_type);
                return;
            }
        });
    }
};
 
enum class wq 
{ 
    qb =0,
    bt =1, 
    be,
    eq,
    lt,
    le,
    in,
    like,
    notin,
    between,
    isnull,
    notnull    
};
//ORM STRUCT REFLECT END
enum enum_field_types
{
    MYSQL_TYPE_DECIMAL,
    MYSQL_TYPE_TINY,
    MYSQL_TYPE_SHORT,
    MYSQL_TYPE_LONG,
    MYSQL_TYPE_FLOAT,
    MYSQL_TYPE_DOUBLE,
    MYSQL_TYPE_NULL,
    MYSQL_TYPE_TIMESTAMP,
    MYSQL_TYPE_LONGLONG,
    MYSQL_TYPE_INT24,
    MYSQL_TYPE_DATE,
    MYSQL_TYPE_TIME,
    MYSQL_TYPE_DATETIME,
    MYSQL_TYPE_YEAR,
    MYSQL_TYPE_NEWDATE, /**< Internal to MySQL. Not used in protocol */
    MYSQL_TYPE_VARCHAR,
    MYSQL_TYPE_BIT,
    MYSQL_TYPE_TIMESTAMP2,
    MYSQL_TYPE_DATETIME2,   /**< Internal to MySQL. Not used in protocol */
    MYSQL_TYPE_TIME2,       /**< Internal to MySQL. Not used in protocol */
    MYSQL_TYPE_TYPED_ARRAY, /**< Used for replication only */
    MYSQL_TYPE_INVALID     = 243,
    MYSQL_TYPE_BOOL        = 244, /**< Currently just a placeholder */
    MYSQL_TYPE_JSON        = 245,
    MYSQL_TYPE_NEWDECIMAL  = 246,
    MYSQL_TYPE_ENUM        = 247,
    MYSQL_TYPE_SET         = 248,
    MYSQL_TYPE_TINY_BLOB   = 249,
    MYSQL_TYPE_MEDIUM_BLOB = 250,
    MYSQL_TYPE_LONG_BLOB   = 251,
    MYSQL_TYPE_BLOB        = 252,
    MYSQL_TYPE_VAR_STRING  = 253,
    MYSQL_TYPE_STRING      = 254,
    MYSQL_TYPE_GEOMETRY    = 255
};

#define CLIENT_LONG_PASSWORD 1                           // 1
#define CLIENT_FOUND_ROWS 2                              // 2
#define CLIENT_LONG_FLAG 4                               // 3     Get all column flags
#define CLIENT_CONNECT_WITH_DB 8                         // 4
#define CLIENT_NO_SCHEMA 16                              // 5
#define CLIENT_COMPRESS 32                               // 6
#define CLIENT_ODBC 64                                   // 7
#define CLIENT_LOCAL_FILES 128                           // 8
#define CLIENT_IGNORE_SPACE 256                          // 9
#define CLIENT_PROTOCOL_41 512                           // 10    Uses the 4.1 protocol.
#define CLIENT_INTERACTIVE 1024                          // 11    Client is interactive.
#define CLIENT_SSL 2048                                  // 12    Switch to SSL after sending the capability-flags.
#define CLIENT_IGNORE_SIGPIPE 4096                       // 13
#define CLIENT_TRANSACTIONS 8192                         // 14
#define CLIENT_RESERVED 16384                            // 15
#define CLIENT_RESERVED2 32768                           // 16
#define CLIENT_MULTI_STATEMENTS (1UL << 16)              // 17 Enable/disable multi-stmt support
#define CLIENT_MULTI_RESULTS (1UL << 17)                 // 18 Enable/disable multi-results
#define CLIENT_PS_MULTI_RESULTS (1UL << 18)              // 19 Multi-results and OUT parameters in PS-protocol.
#define CLIENT_PLUGIN_AUTH (1UL << 19)                   // 20 Client supports plugin authentication
#define CLIENT_CONNECT_ATTRS (1UL << 20)                 // 21 Client supports connection attributes
#define CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA (1UL << 21)// 22    Enable authentication response packet to be larger than 255 bytes.
#define CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS (1UL << 22)  // 23    Don't close the connection for a user account with expired password.
#define CLIENT_SESSION_TRACK (1UL << 23)                 // 24
#define CLIENT_DEPRECATE_EOF (1UL << 24)                 // 25     Client no longer needs @ref page_protocol_basic_eof_packet and will use
#define CLIENT_OPTIONAL_RESULTSET_METADATA (1UL << 25)
#define CLIENT_ZSTD_COMPRESSION_ALGORITHM (1UL << 26)// 27    Compression protocol extended to support zstd compression method

#define CLIENT_PZORM_FLAGS                                               \
    (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG |       \
     CLIENT_CONNECT_WITH_DB | CLIENT_PROTOCOL_41 | CLIENT_TRANSACTIONS | \
     CLIENT_MULTI_RESULTS | CLIENT_PLUGIN_AUTH | CLIENT_DEPRECATE_EOF | CLIENT_OPTIONAL_RESULTSET_METADATA)

#define CLIENT_PZORM_SSL_FLAGS                                                  \
    (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG | CLIENT_SSL | \
     CLIENT_CONNECT_WITH_DB | CLIENT_PROTOCOL_41 | CLIENT_TRANSACTIONS |        \
     CLIENT_MULTI_RESULTS | CLIENT_PLUGIN_AUTH | CLIENT_DEPRECATE_EOF | CLIENT_OPTIONAL_RESULTSET_METADATA)

struct mysql_server_hello_data_t
{
    unsigned char protocol_version = 0;
    std::string server_version;
    unsigned int connection_id = 0;
    std::string auth_plugin_salt_data;
    unsigned short capability_flags_low  = 0;
    unsigned char character_set          = 0;
    unsigned short status_flags          = 0;
    unsigned short capability_flags_high = 0;
    unsigned char auth_plugin_data_len   = 0;
    // std::string auth_plugin_data_part_2;
    std::string auth_plugin_name;
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
    unsigned char field_type = 0;
    unsigned char decimals = 0;
    unsigned short charset = 0;
    unsigned short flags = 0;
    unsigned int column_length = 0;
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

    bool isssl                = false;
    bool issock               = false;
    bool isdebug              = false;
    bool islocal              = false;
    unsigned char link_type   = 0;// 0 edit 1 select 2 backup
    unsigned char max_pool    = 0;
    unsigned char min_pool    = 0;
    unsigned char charset_val = 0;
};

struct orm_left_join_t
{
    std::string selectsql;
    std::string join_table;
    std::string wheresql;
    std::string ordersql;
    std::string limitsql;
    std::string parbysql;
    std::string subsql;
};

struct orm_where_sql_t
{
    bool begin_sub;
    bool end_sub;
    wq op_type;
    std::string filed_name;
    std::string filed_value;
};

std::string to_escape(std::string_view val, bool no_backslash_escapes = false);

class mysql_conn_base
{
  public:
    mysql_conn_base(asio::io_context &ioc);
    ~mysql_conn_base();
    void read_server_hello(unsigned int offset, unsigned int length);
    bool server_public_key_encrypt(const std::string &password, unsigned char *data, unsigned int length);
    bool connect(const orm_conn_t &conn_config);
    asio::awaitable<bool> async_connect(const orm_conn_t &conn_config);

    void mysqlnd_xor_string(char *dst, const size_t dst_len, const char *xor_str, const size_t xor_str_len);
    unsigned int read_pack(unsigned char *data, unsigned int offset);
    void read_field_pack(unsigned char *data, unsigned int total_num, unsigned int &offset, pack_info_t &pack_info);
    bool pack_eof_check(const pack_info_t &temp_pack_data);
    void read_col_info(const std::string &pack_info, field_info_t &temp_field_data);
    unsigned long long pack_real_num(const unsigned char *data, unsigned int &offset);

    bool ping();
    bool close();
    bool hard_close();
    asio::awaitable<bool> async_close();
    asio::awaitable<unsigned int> async_read_loop();
    unsigned int read_loop();

    unsigned int write();
    asio::awaitable<unsigned int> async_write();

    unsigned int write_sql(const std::string &sql);
    asio::awaitable<unsigned int> async_write_sql(const std::string &sql);

    void reset_data();
    bool is_closed();

    void begin_time();
    void finish_time();
    long long count_time();

  public:
    unsigned char *_cache_data = nullptr;

    bool server_enable_ssl   = false;
    bool isclose             = false;
    bool isdebug             = false;
    std::atomic_bool issynch = false;
    unsigned char sock_type  = 0;
    unsigned char seq_next_id;
    unsigned short error_code = 0;
    unsigned int client_flags = 0;
    unsigned int time_start   = 0;
    unsigned int query_num    = 0;
    std::chrono::time_point<std::chrono::steady_clock> time_begin;
    std::chrono::time_point<std::chrono::steady_clock> time_finish;
    std::string send_data;
    std::string error_msg;
    //std::string server_public_key;
    std::unique_ptr<asio::ip::tcp::socket> socket;
    std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket;
    std::unique_ptr<asio::local::stream_protocol::socket> localsocket;
    std::shared_ptr<asio::ssl::context> ssl_context;
    asio::error_code ec;
    asio::io_context *io_ctx = nullptr;
    mysql_server_hello_data_t server_hello;
};
class mysql_charset_store
{
  public:
    void mysql_charset_init();
    void mysql_charset_clear();
    unsigned int mysql_charset_find(const std::string &str);
    ~mysql_charset_store();

  public:
    std::map<std::string, unsigned int> mysql_charset;
};

mysql_charset_store &get_orm_mysql_charset();

}// namespace orm
#endif