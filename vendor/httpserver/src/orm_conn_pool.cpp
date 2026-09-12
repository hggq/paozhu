/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <map>
#include <list>
#include <array>
#include <vector>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#if defined(_WIN32) && !defined(_OPENSSL_Applink) && \
    defined(HAVE_OPENSSL_APPLINK_C)
#include <openssl/applink.c>
#endif
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "mysql_conn.h"
#include "orm_conn_pool.h"
#include "pg_conn.h"
#include "sqlite_conn.h"
#include "orm_common.h"
#include "cost_define.h"
#include "parse_ini.h"
#include <algorithm>
#include <filesystem>
#include <cctype>

namespace orm
{
bool orm_conn_pool::is_postgresql() const
{
    return conf_data[0].db_type == DB_TYPE::POSTGRESQL;
}
bool orm_conn_pool::is_sqlite() const
{
    return conf_data[0].db_type == DB_TYPE::SQLITE;
}
bool str_casecmp(std::string_view str1, std::string_view str2)
{
    if (str1.size() != str2.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < str1.size(); i++)
    {
        if (str1[i] != str2[i])
        {
            if (str1[i] < 91 && str1[i] > 64)
            {
                if ((str1[i] + 32) == str2[i])
                {
                    continue;
                }
            }
            else if (str2[i] < 91 && str2[i] > 64)
            {
                if (str1[i] == (str2[i] + 32))
                {
                    continue;
                }
            }
            return false;
        }
    }
    return true;
}
std::map<std::string, std::shared_ptr<orm_conn_pool>> &get_orm_conn_pool_obj()
{
    static std::map<std::string, std::shared_ptr<orm_conn_pool>> instance;
    return instance;
}
// ---- 4 个内部 helper ----
static std::string normalize_tag(const std::string &raw)
{
    std::string out;
    for (char ch : raw)
    {
        if (out.size() > 20)
            break;
        if (ch >= '0' && ch <= '9')
            out += ch;
        else if (ch == '_')
            out += ch;
        else if (ch >= 'A' && ch <= 'Z')
            out += (ch + 32);
        else if (ch >= 'a' && ch <= 'z')
            out += ch;
    }
    return out;
}

static bool issock_suffix(const std::string &host)
{
    if (host.size() < 5)
        return false;
    return host.compare(host.size() - 5, 5, ".sock") == 0;
}

static bool parse_bool(const std::string &v)
{
    return v == "1" || v == "true" || v == "True" || v == "TRUE" || v == "On" || v == "ON";
}

static DB_TYPE parse_dbtype(const std::string &v)
{
    if (v == "postgresql" || v == "pg" || v == "POSTGRESQL" || v == "PG")
        return DB_TYPE::POSTGRESQL;
    if (v == "sqlite" || v == "sqlite3" || v == "sq3" || v == "SQLITE" || v == "SQLITE3")
        return DB_TYPE::SQLITE;
    return DB_TYPE::MYSQL;
}

static unsigned char clamp_stoi(const std::string &s, unsigned char def)
{
    try
    {
        int v = std::stoi(s);
        if (v < 0)
            return 0;
        if (v > 255)
            return 255;
        return (unsigned char)v;
    }
    catch (...)
    {
        return def;
    }
}

// 判断字符串是否含口令不允许的字节。统一约束：orm.conf 中 mysql/pg/sqlite 的口令只允许
// 可打印 ASCII 的 0x21 ~ 0x7E（! 到 ~），空格(0x20)、全部控制码(0x00~0x1F、0x7F)以及
// 非 ASCII 字节(>=0x80)一律非法，解析时发现即打印错误并跳过该条配置，而非等到连接阶段才失败。
static bool contains_forbidden_password_char(const std::string &s)
{
    for (unsigned char ch : s)
    {
        if (ch < 0x21 || ch > 0x7E)
            return true;
    }
    return false;
}

std::vector<orm_conn_t> get_orm_config_file(const std::string &filename)
{
    std::vector<orm_conn_t> result;
    auto charset_obj = std::make_unique<mysql_charset_store>();
    charset_obj->mysql_charset_init();

    namespace fs = std::filesystem;
    if (!fs::exists(filename))
        return result;

    http::parse_ini ini;
    ini.parse_file(filename);

    for (auto &section : ini.config)
    {
        std::string tag = normalize_tag(section.name);
        if (tag.empty())
            continue;

        auto groups = section.value.splits();// 默认 skip_empty_name=true, 跳过注释行
        for (const auto &group : groups)
        {
            orm_conn_t c;
            c.tag         = tag;
            c.db_type     = DB_TYPE::MYSQL;// 默认
            c.link_type   = 0;
            c.max_pool    = 0;
            c.min_pool    = 0;
            c.issock      = false;
            c.islocal     = false;
            c.isssl       = false;
            c.sslverify   = false;
            c.isdebug     = false;
            c.charset_val = 0;

            for (const auto &kv : group)
            {
                const std::string &k = kv.name;
                const std::string &v = kv.value;

                if (k == "type")
                    c.link_type = (v == "main") ? 0 : 1;
                else if (k == "host")
                {
                    c.host    = v;
                    c.issock  = issock_suffix(v);
                    c.islocal = (v == "127.0.0.1" || v == "localhost");
                }
                else if (k == "port")
                    c.port = v;
                else if (k == "dbname")
                    c.dbname = v;
                else if (k == "user")
                    c.user = v;
                else if (k == "password")
                    c.password = v;
                else if (k == "pretable")
                    c.pretable = v;
                else if (k == "maxpool")
                    c.max_pool = clamp_stoi(v, 2);
                else if (k == "minpool")
                    c.min_pool = clamp_stoi(v, 2);
                else if (k == "ssl")
                    c.isssl = parse_bool(v);
                else if (k == "sslverify")
                    c.sslverify = parse_bool(v);
                else if (k == "sslhost")
                    c.sslhost = v;
                else if (k == "server_public_key")
                    c.server_public_key = v;
                else if (k == "debug")
                    c.isdebug = parse_bool(v);
                else if (k == "dbtype")
                {
                    c.dbtype  = v;
                    c.db_type = parse_dbtype(v);
                }
                else if (k == "charset")
                {
                    std::string low = v;
                    std::transform(low.begin(), low.end(), low.begin(), [](unsigned char x)
                                   { return std::tolower(x); });
                    c.charset     = low;
                    c.charset_val = charset_obj->mysql_charset_find(low);
                }
            }

            // 统一口令规则：mysql/pg/sqlite 的口令只允许可打印 ASCII 0x21~0x7E
            // （不含空格 0x20 与控制码），发现非法字节即报错并跳过该条配置，不进入连接池。
            if (!c.password.empty() && contains_forbidden_password_char(c.password))
            {
                std::string dtype_name = "mysql";
                if (c.db_type == DB_TYPE::POSTGRESQL)
                    dtype_name = "postgresql";
                else if (c.db_type == DB_TYPE::SQLITE)
                    dtype_name = "sqlite";

                std::cerr << "[orm.conf] ERROR: password allows printable ASCII 0x21-0x7E only "
                             "(spaces/control/non-ASCII bytes are forbidden). "
                          << "Skip config [tag=" << tag
                          << ", dbtype=" << dtype_name
                          << (c.host.empty() ? "" : ", host=" + c.host)
                          << (c.dbname.empty() ? "" : ", dbname=" + c.dbname)
                          << (c.user.empty() ? "" : ", user=" + c.user)
                          << "]. Reset the database password to characters in 0x21-0x7E."
                          << std::endl;
                continue;
            }
            result.push_back(std::move(c));
        }
    }

    charset_obj->mysql_charset_clear();
    return result;
}

std::string init_orm_conn_pool_release()
{
    std::map<std::string, std::shared_ptr<orm_conn_pool>> &int_pool = get_orm_conn_pool_obj();

    for (auto iter = int_pool.begin(); iter != int_pool.end(); iter++)
    {
        iter->second->clear_select_conn();
        iter->second->clear_edit_conn();
    }

    return "";
}
std::string init_orm_conn_pool(asio::io_context &ioc, const std::string &orm_config_file)
{

    std::map<std::string, std::shared_ptr<orm_conn_pool>> &int_pool = get_orm_conn_pool_obj();
    std::vector<orm_conn_t> myconfig                                = get_orm_config_file(orm_config_file);
    std::string error_log;

    error_log.append("-- begin init_orm_conn_pool -- \n");
    for (auto &item : myconfig)
    {

        if (item.link_type == 0)
        {
            auto iter = int_pool.find(item.tag);
            if (item.min_pool < 1)
            {
                item.min_pool = 1;
            }
            if (iter != int_pool.end())
            {
                iter->second->conf_data[0] = item;
                unsigned int n             = 0;
                try
                {
                    if (item.db_type == DB_TYPE::POSTGRESQL)
                    {
                        n = iter->second->init_pg_edit_conn(item.min_pool);
                    }
                    else if (item.db_type == DB_TYPE::SQLITE)
                    {
                        n = iter->second->init_sqlite_edit_conn(item.min_pool);
                    }
                    else
                    {
                        n = iter->second->init_mysql_edit_conn(item.min_pool);
                    }
                }

                catch (const std::exception &e)
                {
                    error_log.append(e.what());
                }
                if (n == 0)
                {
                    iter->second->error_msg.append(" int_pool.init_edit_conn failed for tag " + item.tag);
                }
                continue;
            }
            else
            {
                std::shared_ptr<orm_conn_pool> conn = std::make_shared<orm_conn_pool>();
                conn->io_context                    = &ioc;
                conn->conf_data[0]                  = item;
                unsigned int n                      = 0;
                try
                {
                    if (item.db_type == DB_TYPE::POSTGRESQL)
                    {
                        n = conn->init_pg_edit_conn(item.min_pool);
                    }
                    else if (item.db_type == DB_TYPE::SQLITE)
                    {
                        n = conn->init_sqlite_edit_conn(item.min_pool);
                    }
                    else
                    {
                        n = conn->init_mysql_edit_conn(item.min_pool);
                    }
                }

                catch (const std::exception &e)
                {
                    error_log.append(e.what());
                }
                if (n == 0)
                {
                    conn->error_msg.append(" int_pool.init_edit_conn failed for tag " + item.tag);
                }
                int_pool.emplace(item.tag, conn);
            }
        }
        else if (item.link_type == 1)
        {
            auto iter = int_pool.find(item.tag);
            if (item.min_pool < 1)
            {
                item.min_pool = 1;
            }
            if (iter != int_pool.end())
            {
                iter->second->conf_data[1] = item;
                unsigned int n             = 0;
                try
                {
                    if (item.db_type == DB_TYPE::POSTGRESQL)
                    {
                        n = iter->second->init_pg_select_conn(item.min_pool);
                    }
                    else if (item.db_type == DB_TYPE::SQLITE)
                    {
                        n = iter->second->init_sqlite_select_conn(item.min_pool);
                    }
                    else
                    {
                        n = iter->second->init_mysql_select_conn(item.min_pool);
                    }
                }

                catch (const std::exception &e)
                {
                    error_log.append(e.what());
                }
                if (n == 0)
                {
                    iter->second->error_msg.append(" int_pool.init_select_conn failed for tag " + item.tag);
                }
                continue;
            }
            else
            {
                std::shared_ptr<orm_conn_pool> conn = std::make_shared<orm_conn_pool>();
                conn->io_context                    = &ioc;
                conn->conf_data[1]                  = item;
                unsigned int n                      = 0;
                try
                {
                    if (item.db_type == DB_TYPE::POSTGRESQL)
                    {
                        n = conn->init_pg_select_conn(item.min_pool);
                    }
                    else if (item.db_type == DB_TYPE::SQLITE)
                    {
                        n = conn->init_sqlite_select_conn(item.min_pool);
                    }
                    else
                    {
                        n = conn->init_mysql_select_conn(item.min_pool);
                    }
                }

                catch (const std::exception &e)
                {
                    error_log.append(e.what());
                }
                if (n == 0)
                {
                    conn->error_msg.append(" int_pool.init_select_conn failed for tag " + item.tag);
                }
                int_pool.emplace(item.tag, conn);
            }
        }
    }
    error_log.append("\n-- end init_orm_conn_pool -- \n");
    return error_log;
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_mysql_edit_connect()
{
    auto link = orm_conn_link_t::create(*io_context, DB_TYPE::MYSQL);
    auto conn = std::make_shared<mysql_conn_base>(std::move(link));
    bool isok = co_await conn->async_connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw std::runtime_error(" add_mysql_edit_connect failed for tag " + conn->error_msg);
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::add_mysql_edit_connect()
{
    auto link = orm_conn_link_t::create(*io_context, DB_TYPE::MYSQL);
    auto conn = std::make_shared<mysql_conn_base>(std::move(link));
    bool isok = conn->connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        return conn;
    }
    throw std::runtime_error(" add_mysql_edit_connect failed for tag " + conn->error_msg);
}
void orm_conn_pool::back_mysql_edit_conn(std::shared_ptr<mysql_conn_base> conn)
{
    // 约定：事务经 orm_query.h edit_query 独立使用（外层异常包裹自管回滚），
    // 归还仅检查连接未断开，不做事务清理
    if (!conn)
    {
        return;
    }
    if (conn->isclose)
    {
        // 连接已断开：销毁
        return;
    }
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (conn->pooled_)
    {
        std::cerr << "[orm_pool] duplicate return ignored: mysql_edit" << std::endl;
    }
    else
    {
        conn->pooled_ = true;
        conn->issynch = false;
        mysql_edit_pool.emplace_back(std::move(conn));
    }
}
unsigned int orm_conn_pool::init_mysql_edit_conn(unsigned char n)
{
    unsigned char i = 0;
    for (; i < n; ++i)
    {
        try
        {
            auto link = orm_conn_link_t::create(*io_context, DB_TYPE::MYSQL);
            auto conn = std::make_shared<mysql_conn_base>(std::move(link));
            bool isok = conn->connect(conf_data[0]);
            if (isok)
            {
                if (conf_data[0].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(conn_edit_mutex);
                mysql_edit_pool.emplace_back(conn);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw std::runtime_error(error_msg);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}
std::shared_ptr<mysql_conn_base> orm_conn_pool::add_mysql_select_connect()
{
    auto link = orm_conn_link_t::create(*io_context, DB_TYPE::MYSQL);
    auto conn = std::make_shared<mysql_conn_base>(std::move(link));
    bool isok = conn->connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        return conn;
    }
    throw std::runtime_error(" add_mysql_select_connect failed for tag ");
}
asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_mysql_select_connect()
{
    auto link = orm_conn_link_t::create(*io_context, DB_TYPE::MYSQL);
    auto conn = std::make_shared<mysql_conn_base>(std::move(link));
    bool isok = co_await conn->async_connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw std::runtime_error(" add_mysql_select_connect failed for tag ");
}
unsigned int orm_conn_pool::init_mysql_select_conn(unsigned char n)
{
    unsigned char i = 0;
    for (; i < n; ++i)
    {
        try
        {
            auto link = orm_conn_link_t::create(*io_context, DB_TYPE::MYSQL);
            auto conn = std::make_shared<mysql_conn_base>(std::move(link));
            bool isok = conn->connect(conf_data[1]);
            if (isok)
            {
                if (conf_data[1].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(conn_select_mutex);
                mysql_select_pool.emplace_back(conn);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw std::runtime_error(error_msg);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}
void orm_conn_pool::back_mysql_select_conn(std::shared_ptr<mysql_conn_base> conn)
{
    if (!conn)
    {
        return;
    }
    if (conn->isclose)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (conn->pooled_)
    {
        std::cerr << "[orm_pool] duplicate return ignored: mysql_select" << std::endl;
    }
    else
    {
        conn->pooled_ = true;
        conn->issynch = false;
        mysql_select_pool.emplace_back(std::move(conn));
    }
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_get_mysql_edit_conn()
{
    // 不理会 max_pool：池中无空闲连接时直接从数据库新建
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (mysql_edit_pool.empty())
    {
        lock.unlock();
        auto conn = co_await async_add_mysql_edit_connect();
        co_return conn;
    }

    auto temp = std::move(mysql_edit_pool.front());
    mysql_edit_pool.pop_front();
    temp->pooled_ = false;
    lock.unlock();
    if (!temp->is_closed())
    {
        co_return temp;
    }
    auto conn = co_await async_add_mysql_edit_connect();
    co_return conn;
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::get_mysql_edit_conn()
{
    // 不理会 max_pool：池中无空闲连接时直接从数据库新建
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (mysql_edit_pool.empty())
    {
        lock.unlock();
        auto conn = add_mysql_edit_connect();
        return conn;
    }

    auto temp = std::move(mysql_edit_pool.front());
    mysql_edit_pool.pop_front();
    temp->pooled_ = false;
    lock.unlock();
    temp->issynch = true;
    if (!temp->is_closed())
    {
        return temp;
    }
    auto conn = add_mysql_edit_connect();
    return conn;
}
asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_get_mysql_select_conn()
{
    // 不理会 max_pool：池中无空闲连接时直接从数据库新建
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (mysql_select_pool.empty())
    {
        lock.unlock();
        auto conn = co_await async_add_mysql_select_connect();
        co_return conn;
    }
    auto temp = std::move(mysql_select_pool.front());
    mysql_select_pool.pop_front();
    temp->pooled_ = false;
    lock.unlock();

    if (!temp->is_closed())
    {
        co_return temp;
    }
    auto conn = co_await async_add_mysql_select_connect();
    co_return conn;
}
std::shared_ptr<mysql_conn_base> orm_conn_pool::get_mysql_select_conn()
{
    // 不理会 max_pool：池中无空闲连接时直接从数据库新建
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (mysql_select_pool.empty())
    {
        lock.unlock();
        auto conn = add_mysql_select_connect();
        return conn;
    };

    auto temp = std::move(mysql_select_pool.front());
    mysql_select_pool.pop_front();
    temp->pooled_ = false;
    lock.unlock();
    temp->issynch = true;
    if (!temp->is_closed())
    {
        return temp;
    }
    auto conn = add_mysql_select_connect();
    return conn;
}

asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_add_pg_edit_connect()
{
    auto link = orm_conn_link_t::create(*io_context, DB_TYPE::POSTGRESQL);
    auto conn = std::make_shared<pg_conn_base>(std::move(link));
    bool isok = co_await conn->async_connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw std::runtime_error(" add_pg_edit_connect failed for tag ");
}

std::shared_ptr<pg_conn_base> orm_conn_pool::add_pg_edit_connect()
{
    auto link = orm_conn_link_t::create(*io_context, DB_TYPE::POSTGRESQL);
    auto conn = std::make_shared<pg_conn_base>(std::move(link));
    bool isok = conn->connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        return conn;
    }
    throw std::runtime_error(" add_pg_edit_connect failed for tag ");
}
void orm_conn_pool::back_pg_edit_conn(std::shared_ptr<pg_conn_base> conn)
{
    if (!conn)
    {
        return;
    }
    if (conn->isclose)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (conn->pooled_)
    {
        std::cerr << "[orm_pool] duplicate return ignored: pg_edit" << std::endl;
    }
    else
    {
        conn->pooled_ = true;
        conn->issynch = false;
        pg_edit_pool.emplace_back(std::move(conn));
    }
}
unsigned int orm_conn_pool::init_pg_edit_conn(unsigned char n)
{
    unsigned char i = 0;
    for (; i < n; ++i)
    {
        try
        {
            auto link = orm_conn_link_t::create(*io_context, DB_TYPE::POSTGRESQL);
            auto conn = std::make_shared<pg_conn_base>(std::move(link));
            bool isok = conn->connect(conf_data[0]);
            if (isok)
            {
                if (conf_data[0].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(conn_edit_mutex);
                pg_edit_pool.emplace_back(conn);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw std::runtime_error(error_msg);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}
std::shared_ptr<pg_conn_base> orm_conn_pool::add_pg_select_connect()
{
    auto link = orm_conn_link_t::create(*io_context, DB_TYPE::POSTGRESQL);
    auto conn = std::make_shared<pg_conn_base>(std::move(link));
    bool isok = conn->connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        return conn;
    }
    throw std::runtime_error(" add_pg_select_connect failed for tag ");
}
asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_add_pg_select_connect()
{
    auto link = orm_conn_link_t::create(*io_context, DB_TYPE::POSTGRESQL);
    auto conn = std::make_shared<pg_conn_base>(std::move(link));
    bool isok = co_await conn->async_connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw std::runtime_error(" add_pg_select_connect failed for tag ");
}
unsigned int orm_conn_pool::init_pg_select_conn(unsigned char n)
{
    unsigned char i = 0;
    for (; i < n; ++i)
    {
        try
        {
            auto link = orm_conn_link_t::create(*io_context, DB_TYPE::POSTGRESQL);
            auto conn = std::make_shared<pg_conn_base>(std::move(link));
            bool isok = conn->connect(conf_data[1]);
            if (isok)
            {
                if (conf_data[1].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(conn_select_mutex);
                pg_select_pool.emplace_back(conn);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw std::runtime_error(error_msg);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}
void orm_conn_pool::back_pg_select_conn(std::shared_ptr<pg_conn_base> conn)
{
    if (!conn)
    {
        return;
    }
    if (conn->isclose)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (conn->pooled_)
    {
        std::cerr << "[orm_pool] duplicate return ignored: pg_select" << std::endl;
    }
    else
    {
        conn->pooled_ = true;
        conn->issynch = false;
        pg_select_pool.emplace_back(std::move(conn));
    }
}

asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_get_pg_edit_conn()
{
    // 不理会 max_pool：池中无空闲连接时直接从数据库新建
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (pg_edit_pool.empty())
    {
        lock.unlock();
        auto conn = co_await async_add_pg_edit_connect();
        co_return conn;
    }

    auto temp = std::move(pg_edit_pool.front());
    pg_edit_pool.pop_front();
    temp->pooled_ = false;
    lock.unlock();
    if (!temp->is_closed())
    {
        co_return temp;
    }
    auto conn = co_await async_add_pg_edit_connect();
    co_return conn;
}

std::shared_ptr<pg_conn_base> orm_conn_pool::get_pg_edit_conn()
{
    // 不理会 max_pool：池中无空闲连接时直接从数据库新建
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (pg_edit_pool.empty())
    {
        lock.unlock();
        auto conn = add_pg_edit_connect();
        return conn;
    }

    auto temp = std::move(pg_edit_pool.front());
    pg_edit_pool.pop_front();
    temp->pooled_ = false;
    lock.unlock();
    temp->issynch = true;
    if (!temp->is_closed())
    {
        return temp;
    }
    auto conn = add_pg_edit_connect();
    return conn;
}
asio::awaitable<std::shared_ptr<pg_conn_base>> orm_conn_pool::async_get_pg_select_conn()
{
    // 不理会 max_pool：池中无空闲连接时直接从数据库新建
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (pg_select_pool.empty())
    {
        lock.unlock();
        auto conn = co_await async_add_pg_select_connect();
        co_return conn;
    }
    auto temp = std::move(pg_select_pool.front());
    pg_select_pool.pop_front();
    temp->pooled_ = false;
    lock.unlock();

    if (!temp->is_closed())
    {
        co_return temp;
    }
    auto conn = co_await async_add_pg_select_connect();
    co_return conn;
}
std::shared_ptr<pg_conn_base> orm_conn_pool::get_pg_select_conn()
{
    // 不理会 max_pool：池中无空闲连接时直接从数据库新建
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (pg_select_pool.empty())
    {
        lock.unlock();
        auto conn = add_pg_select_connect();
        return conn;
    };

    auto temp = std::move(pg_select_pool.front());
    pg_select_pool.pop_front();
    temp->pooled_ = false;
    lock.unlock();
    temp->issynch = true;
    if (!temp->is_closed())
    {
        return temp;
    }
    auto conn = add_pg_select_connect();
    return conn;
}

// ======================== SQLite connections ========================
// SQLite 为本地文件库, 单连接模式: 同库任务已由框架级常驻 worker 线程串行,
// 每个角色 (edit/select) 保持一条常驻连接共享使用, 无需借出/归还式连接池。
// 公开接口签名与 MySQL/PG 保持一致, 调用方无感知。

asio::awaitable<std::shared_ptr<sqlite_conn_base>> orm_conn_pool::async_add_sqlite_edit_connect()
{
    auto conn = std::make_shared<sqlite_conn_base>();
    conn->start_worker_thread(*io_context, sqlite_db_path_of(conf_data[0]));
    bool isok = co_await conn->async_connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw std::runtime_error(" add_sqlite_edit_connect failed for tag ");
}

std::shared_ptr<sqlite_conn_base> orm_conn_pool::add_sqlite_edit_connect()
{
    auto conn = std::make_shared<sqlite_conn_base>();
    conn->start_worker_thread(*io_context, sqlite_db_path_of(conf_data[0]));
    bool isok = conn->connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        return conn;
    }
    throw std::runtime_error(" add_sqlite_edit_connect failed for tag ");
}
void orm_conn_pool::back_sqlite_edit_conn(std::shared_ptr<sqlite_conn_base> conn)
{
    // 单连接模式: 连接常驻共享, 无需归还, 仅保留用量统计 (与 MySQL/PG 接口一致)
    if (conn)
    {
        conn->time_start = time((time_t *)NULL);
        conn->query_num++;
    }
}
unsigned int orm_conn_pool::init_sqlite_edit_conn(unsigned char n)
{
    (void)n;
#ifndef ENABLE_SQLITE
    error_msg = "SQLite support not compiled. Rebuild with -DENABLE_SQLITE=ON";
    return 0;
#else
    // 单连接模式: 忽略 n, 只建一条常驻连接 (同库任务已在 worker 线程串行)
    {
        std::unique_lock<std::mutex> lock(conn_edit_mutex);
        if (sqlite_edit_conn && !sqlite_edit_conn->is_closed())
        {
            return 1;
        }
    }
    try
    {
        auto conn = add_sqlite_edit_connect();
        std::unique_lock<std::mutex> lock(conn_edit_mutex);
        sqlite_edit_conn = conn;
        return 1;
    }
    catch (const std::exception &e)
    {
        error_msg.append(e.what());
        return 0;
    }
#endif
}
std::shared_ptr<sqlite_conn_base> orm_conn_pool::add_sqlite_select_connect()
{
    auto conn = std::make_shared<sqlite_conn_base>();
    conn->start_worker_thread(*io_context, sqlite_db_path_of(conf_data[1]));
    bool isok = conn->connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        return conn;
    }
    throw std::runtime_error(" add_sqlite_select_connect failed for tag ");
}
asio::awaitable<std::shared_ptr<sqlite_conn_base>> orm_conn_pool::async_add_sqlite_select_connect()
{
    auto conn = std::make_shared<sqlite_conn_base>();
    conn->start_worker_thread(*io_context, sqlite_db_path_of(conf_data[1]));
    bool isok = co_await conn->async_connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw std::runtime_error(" add_sqlite_select_connect failed for tag ");
}
void orm_conn_pool::back_sqlite_select_conn(std::shared_ptr<sqlite_conn_base> conn)
{
    // 单连接模式: 连接常驻共享, 无需归还, 仅保留用量统计 (与 MySQL/PG 接口一致)
    if (conn)
    {
        conn->time_start = time((time_t *)NULL);
        conn->query_num++;
    }
}
unsigned int orm_conn_pool::init_sqlite_select_conn(unsigned char n)
{
    (void)n;
#ifndef ENABLE_SQLITE
    error_msg = "SQLite support not compiled. Rebuild with -DENABLE_SQLITE=ON";
    return 0;
#else
    // 单连接模式: 忽略 n, 只建一条常驻连接 (同库任务已在 worker 线程串行)
    {
        std::unique_lock<std::mutex> lock(conn_select_mutex);
        if (sqlite_select_conn && !sqlite_select_conn->is_closed())
        {
            return 1;
        }
    }
    try
    {
        auto conn = add_sqlite_select_connect();
        std::unique_lock<std::mutex> lock(conn_select_mutex);
        sqlite_select_conn = conn;
        return 1;
    }
    catch (const std::exception &e)
    {
        error_msg.append(e.what());
        return 0;
    }
#endif
}

asio::awaitable<std::shared_ptr<sqlite_conn_base>> orm_conn_pool::async_get_sqlite_edit_conn()
{
    // 单连接模式: 直接共享常驻连接; 断开时重建 (并发重建时复用先到的)
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (sqlite_edit_conn && !sqlite_edit_conn->is_closed())
    {
        co_return sqlite_edit_conn;
    }
    lock.unlock();
    auto conn = co_await async_add_sqlite_edit_connect();
    lock.lock();
    if (!sqlite_edit_conn || sqlite_edit_conn->is_closed())
    {
        sqlite_edit_conn = conn;
    }
    else
    {
        conn = sqlite_edit_conn;
    }
    co_return conn;
}

std::shared_ptr<sqlite_conn_base> orm_conn_pool::get_sqlite_edit_conn()
{
    // 单连接模式: 直接共享常驻连接; 断开时重建 (并发重建时复用先到的)
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (sqlite_edit_conn && !sqlite_edit_conn->is_closed())
    {
        return sqlite_edit_conn;
    }
    lock.unlock();
    auto conn = add_sqlite_edit_connect();
    lock.lock();
    if (!sqlite_edit_conn || sqlite_edit_conn->is_closed())
    {
        sqlite_edit_conn = conn;
    }
    else
    {
        conn = sqlite_edit_conn;
    }
    return conn;
}
asio::awaitable<std::shared_ptr<sqlite_conn_base>> orm_conn_pool::async_get_sqlite_select_conn()
{
    // 单连接模式: 直接共享常驻连接; 断开时重建 (并发重建时复用先到的)
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (sqlite_select_conn && !sqlite_select_conn->is_closed())
    {
        co_return sqlite_select_conn;
    }
    lock.unlock();
    auto conn = co_await async_add_sqlite_select_connect();
    lock.lock();
    if (!sqlite_select_conn || sqlite_select_conn->is_closed())
    {
        sqlite_select_conn = conn;
    }
    else
    {
        conn = sqlite_select_conn;
    }
    co_return conn;
}
std::shared_ptr<sqlite_conn_base> orm_conn_pool::get_sqlite_select_conn()
{
    // 单连接模式: 直接共享常驻连接; 断开时重建 (并发重建时复用先到的)
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (sqlite_select_conn && !sqlite_select_conn->is_closed())
    {
        return sqlite_select_conn;
    }
    lock.unlock();
    auto conn = add_sqlite_select_connect();
    lock.lock();
    if (!sqlite_select_conn || sqlite_select_conn->is_closed())
    {
        sqlite_select_conn = conn;
    }
    else
    {
        conn = sqlite_select_conn;
    }
    return conn;
}

asio::awaitable<bool> orm_conn_pool::clear_select_conn_2hour()
{
    unsigned int nowtimeid = time((time_t *)NULL);
    if (nowtimeid > CONST_ORM_CLEAR_TIME)
    {
        nowtimeid = nowtimeid - CONST_ORM_CLEAR_TIME;
    }
    bool cleared = false;
    {
        std::unique_lock<std::mutex> lock(conn_select_mutex);
        if (mysql_select_pool.size() > 0)
        {
            auto temp = std::move(mysql_select_pool.front());
            mysql_select_pool.pop_front();
            temp->pooled_ = false;

            if (temp->time_start < nowtimeid)
            {
                lock.unlock();
                co_await temp->async_close();
                cleared = true;
            }
            else if (temp->query_num > CONST_ORM_CLEAR_NUMBER)
            {
                lock.unlock();
                co_await temp->async_close();
                cleared = true;
            }
            else
            {
                temp->pooled_ = true;
                mysql_select_pool.emplace_back(temp);
            }
        }
    }
    {
        std::unique_lock<std::mutex> lock(conn_select_mutex);
        if (pg_select_pool.size() > 0)
        {
            auto temp = std::move(pg_select_pool.front());
            pg_select_pool.pop_front();
            temp->pooled_ = false;

            if (temp->time_start < nowtimeid)
            {
                lock.unlock();
                co_await temp->async_close();
                cleared = true;
            }
            else if (temp->query_num > CONST_ORM_CLEAR_NUMBER)
            {
                lock.unlock();
                co_await temp->async_close();
                cleared = true;
            }
            else
            {
                temp->pooled_ = true;
                pg_select_pool.emplace_back(temp);
            }
        }
        lock.unlock();
    }
    // SQLite 单连接常驻共享, 不参与定时回收 (断开时由 get 自动重建)

    co_return cleared;
}

asio::awaitable<bool> orm_conn_pool::clear_edit_conn_2hour()
{
    unsigned int nowtimeid = time((time_t *)NULL);
    if (nowtimeid > CONST_ORM_CLEAR_TIME)
    {
        nowtimeid = nowtimeid - CONST_ORM_CLEAR_TIME;
    }
    bool cleared = false;
    {
        std::unique_lock<std::mutex> lock(conn_edit_mutex);
        if (mysql_edit_pool.size() > 0)
        {
            auto temp = std::move(mysql_edit_pool.front());
            mysql_edit_pool.pop_front();
            temp->pooled_ = false;

            if (temp->time_start < nowtimeid)
            {
                lock.unlock();
                co_await temp->async_close();
                cleared = true;
            }
            else if (temp->query_num > CONST_ORM_CLEAR_NUMBER)
            {
                lock.unlock();
                co_await temp->async_close();
                cleared = true;
            }
            else
            {
                temp->pooled_ = true;
                mysql_edit_pool.emplace_back(temp);
            }
        }
    }
    {
        std::unique_lock<std::mutex> lock(conn_edit_mutex);
        if (pg_edit_pool.size() > 0)
        {
            auto temp = std::move(pg_edit_pool.front());
            pg_edit_pool.pop_front();
            temp->pooled_ = false;

            if (temp->time_start < nowtimeid)
            {
                lock.unlock();
                co_await temp->async_close();
                cleared = true;
            }
            else if (temp->query_num > CONST_ORM_CLEAR_NUMBER)
            {
                lock.unlock();
                co_await temp->async_close();
                cleared = true;
            }
            else
            {
                temp->pooled_ = true;
                pg_edit_pool.emplace_back(temp);
            }
        }
        lock.unlock();
    }
    // SQLite 单连接常驻共享, 不参与定时回收 (断开时由 get 自动重建)

    co_return cleared;
}

unsigned int orm_conn_pool::clear_select_conn()
{
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    unsigned int n = 0;
    for (; !mysql_select_pool.empty();)
    {
        auto temp = std::move(mysql_select_pool.front());
        mysql_select_pool.pop_front();
        temp->close();
        n++;
    }
    for (; !pg_select_pool.empty();)
    {
        auto temp = std::move(pg_select_pool.front());
        pg_select_pool.pop_front();
        temp->close();
        n++;
    }
    if (sqlite_select_conn)
    {
        sqlite_select_conn->close();
        sqlite_select_conn.reset();
        n++;
    }
    lock.unlock();
    return n;
}
unsigned int orm_conn_pool::clear_edit_conn()
{
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    unsigned int n = 0;
    for (; !mysql_edit_pool.empty();)
    {
        auto temp = std::move(mysql_edit_pool.front());
        mysql_edit_pool.pop_front();
        temp->close();
        n++;
    }
    for (; !pg_edit_pool.empty();)
    {
        auto temp = std::move(pg_edit_pool.front());
        pg_edit_pool.pop_front();
        temp->close();
        n++;
    }
    if (sqlite_edit_conn)
    {
        sqlite_edit_conn->close();
        sqlite_edit_conn.reset();
        n++;
    }
    lock.unlock();
    return n;
}
}// namespace orm