/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <iostream>
#include <memory>
#include <string>
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
#include "mysql_conn_pool.h"
#include "cost_define.h"

namespace orm
{
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
std::vector<orm_conn_t> get_orm_config_file(const std::string &filename)
{
    std::vector<orm_conn_t> myconfig;

    // 打开文件
    FILE *f = fopen(filename.c_str(), "rt");
    if (f == nullptr)
    {
        return myconfig;
    }

    // 移动文件指针到文件末尾，获取文件大小
    fseek(f, 0, SEEK_END);
    auto const size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // 读取文件内容到字符串
    std::string s, linestr, keyname, strval;
    s.resize(size);

    auto nread = fread(&s[0], 1, size, f);
    s.resize(nread);
    fclose(f);

    // 初始化配置结构体
    struct orm_conn_t mysqlconf;
    bool readkey = false;
    bool isvalue = false;

    // 初始化键名为默认值
    keyname = "default";
    std::string typeone;

    for (unsigned int i = 0; i < s.size(); i++)
    {
        // 忽略注释行
        if (s[i] == ';' || s[i] == '#')
        {
            i++;
            if (linestr.size() > 0)
            {
                //  mysqldbconfig[keyname][linestr]=strval;
                if (linestr == "type")
                {
                    if (typeone.empty())
                    {
                        typeone       = strval;
                        mysqlconf.tag = keyname;
                    }
                    else
                    {
                        // 保存当前配置到列表中
                        myconfig.push_back(mysqlconf);
                        typeone = strval;
                        //mysqlconf.type = strval;
                        if (strval == "main")
                        {
                            mysqlconf.link_type = 0;
                        }
                        else
                        {
                            mysqlconf.link_type = 1;
                        }
                        mysqlconf.host.clear();
                        mysqlconf.port.clear();
                        mysqlconf.dbname.clear();
                        mysqlconf.user.clear();
                        mysqlconf.password.clear();
                        mysqlconf.pretable.clear();

                        mysqlconf.tag = keyname;
                        mysqlconf.dbtype.clear();

                        mysqlconf.max_pool = 0;
                        mysqlconf.min_pool = 0;
                        mysqlconf.issock   = false;
                        mysqlconf.isssl    = false;
                        //mysqlconf.link_type = 0;
                    }
                }
            }
            linestr.clear();
            strval.clear();
            isvalue = false;
            // 跳过注释行的剩余部分
            for (; i < s.size(); i++)
            {
                if (s[i] == 0x0A)
                {
                    break;
                }
            }
        }

        // 处理换行符
        if (i < s.size() && s[i] == 0x0A)
        {
            readkey = false;
            // myconfig[linestr]=strval;
            if (linestr.size() > 0)
            {
                // mysqldbconfig[keyname][linestr]=strval;
                if (str_casecmp(linestr, "type"))
                {
                    if (typeone.empty())
                    {
                        typeone       = strval;
                        mysqlconf.tag = keyname;

                        if (strval == "main")
                        {
                            mysqlconf.link_type = 0;
                        }
                        else
                        {
                            mysqlconf.link_type = 1;
                        }
                    }
                    else
                    {
                        // 保存当前配置到列表中
                        myconfig.push_back(mysqlconf);
                        typeone = strval;

                        if (strval == "main")
                        {
                            mysqlconf.link_type = 0;
                        }
                        else
                        {
                            mysqlconf.link_type = 1;
                        }

                        //mysqlconf.type = strval;
                        mysqlconf.host.clear();
                        mysqlconf.port.clear();
                        mysqlconf.dbname.clear();
                        mysqlconf.user.clear();
                        mysqlconf.password.clear();
                        mysqlconf.pretable.clear();
                        mysqlconf.charset.clear();

                        mysqlconf.max_pool = 0;
                        mysqlconf.min_pool = 0;
                        mysqlconf.tag      = keyname;
                        mysqlconf.issock   = false;
                        mysqlconf.isssl    = false;
                        //mysqlconf.link_type = 0;
                    }
                }
                // 处理各个配置项
                if (str_casecmp(linestr, "host"))
                {
                    if (strval.size() > 5)
                    {
                        // 处理 socket 文件
                        if (strval[strval.size() - 1] == 'k' && strval[strval.size() - 2] == 'c' && strval[strval.size() - 3] == 'o' && strval[strval.size() - 4] == 's' && strval[strval.size() - 5] == '.')
                        {
                            mysqlconf.issock = true;
                        }
                    }
                    mysqlconf.host = strval;
                }
                if (str_casecmp(linestr, "port"))
                {
                    mysqlconf.port = strval;
                }
                if (str_casecmp(linestr, "dbname"))
                {
                    mysqlconf.dbname = strval;
                }
                if (str_casecmp(linestr, "user"))
                {
                    mysqlconf.user = strval;
                }
                if (str_casecmp(linestr, "password"))
                {
                    mysqlconf.password = strval;
                }
                if (str_casecmp(linestr, "pretable"))
                {
                    mysqlconf.pretable = strval;
                }
                if (str_casecmp(linestr, "maxpool"))
                {
                    mysqlconf.max_pool = std::stoi(strval);
                }
                if (str_casecmp(linestr, "minpool"))
                {
                    mysqlconf.min_pool = std::stoi(strval);
                }
                if (str_casecmp(linestr, "ssl"))
                {
                    if (strval == "1" || strval == "true" || strval == "On" || strval == "ON")
                    {
                        mysqlconf.isssl = true;
                    }
                }
                if (str_casecmp(linestr, "dbtype"))
                {
                    mysqlconf.dbtype = strval;
                }
            }

            linestr.clear();
            strval.clear();
            isvalue = false;
            continue;
        }

        // 处理方括号，用于分隔不同的配置块
        if (s[i] == '[')
        {
            keyname.clear();
            readkey = true;
            continue;
        }
        if (s[i] == ']')
        {
            readkey = false;
            continue;
        }

        // 忽略空白字符
        if (s[i] == 0x20)
        {
            continue;
        }
        if (s[i] == '\t')
        {
            continue;
        }
        if (s[i] == '"')
        {
            continue;
        }

        // 处理等号，用于分隔键和值
        if (s[i] == '=')
        {
            isvalue = true;
            continue;
        }

        // 读取键或值
        if (readkey)
        {
            keyname.push_back(s[i]);
        }
        else
        {
            if (isvalue)
            {
                strval.push_back(s[i]);
            }
            else
            {
                linestr.push_back(s[i]);
            }
        }
    }

    // 处理最后一个配置块
    if (mysqlconf.host.size() > 0)
    {
        if (str_casecmp(linestr, "host"))
        {
            if (strval.size() > 5)
            {
                // 处理 socket 文件
                if (strval[strval.size() - 1] == 'k' && strval[strval.size() - 2] == 'c' && strval[strval.size() - 3] == 'o' && strval[strval.size() - 4] == 's' && strval[strval.size() - 5] == '.')
                {
                    mysqlconf.issock = true;
                }
            }
            mysqlconf.host = strval;
        }
        if (str_casecmp(linestr, "port"))
        {
            mysqlconf.port = strval;
        }
        if (str_casecmp(linestr, "dbname"))
        {
            mysqlconf.dbname = strval;
        }
        if (str_casecmp(linestr, "user"))
        {
            mysqlconf.user = strval;
        }
        if (str_casecmp(linestr, "password"))
        {
            mysqlconf.password = strval;
        }
        if (str_casecmp(linestr, "pretable"))
        {
            mysqlconf.pretable = strval;
        }
        if (str_casecmp(linestr, "maxpool"))
        {
            mysqlconf.max_pool = std::stoi(strval);
        }
        if (str_casecmp(linestr, "minpool"))
        {
            mysqlconf.min_pool = std::stoi(strval);
        }
        if (str_casecmp(linestr, "ssl"))
        {
            if (strval == "1" || strval == "true" || strval == "On" || strval == "ON")
            {
                mysqlconf.isssl = true;
            }
        }
        if (str_casecmp(linestr, "dbtype"))
        {

            mysqlconf.dbtype = strval;
        }
        mysqlconf.tag = keyname;
        myconfig.push_back(mysqlconf);
    }

    return myconfig;
}

void init_orm_conn_pool(asio::io_context &ioc, const std::string &orm_config_file)
{
    std::map<std::string, std::shared_ptr<orm_conn_pool>> &int_pool = get_orm_conn_pool_obj();
    std::vector<orm_conn_t> myconfig                                = get_orm_config_file(orm_config_file);

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
                unsigned int n             = iter->second->init_edit_conn(item.min_pool);
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
                unsigned int n                      = conn->init_edit_conn(item.min_pool);
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
                unsigned int n             = iter->second->init_select_conn(item.min_pool);
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
                unsigned int n                      = conn->init_select_conn(item.min_pool);
                if (n == 0)
                {
                    conn->error_msg.append(" int_pool.init_select_conn failed for tag " + item.tag);
                }
                int_pool.emplace(item.tag, conn);
            }
        }
    }
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_edit_connect()
{
    std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
    bool isok                             = co_await conn->async_connect(conf_data[0].host, conf_data[0].port, conf_data[0].user, conf_data[0].password, conf_data[0].dbname, false);
    if (isok)
    {
        co_return conn;
    }
    error_msg.append(" add_edit_connect failed for tag " + conf_data[0].tag);
    conn->error_msg.append(" add_edit_connect failed for tag " + conf_data[0].tag);
    throw conn->error_msg;
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::add_edit_connect()
{
    std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
    bool isok                             = conn->connect(conf_data[0].host, conf_data[0].port, conf_data[0].user, conf_data[0].password, conf_data[0].dbname, false);
    if (isok)
    {
        return conn;
    }
    error_msg.append(" add_edit_connect failed for tag " + conf_data[0].tag);
    conn->error_msg.append(" add_edit_connect failed for tag " + conf_data[0].tag);
    throw conn->error_msg;
}
void orm_conn_pool::back_edit_conn(std::shared_ptr<mysql_conn_base> conn)
{
    conn->issynch = false;
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    conn_edit_pool.emplace_back(conn);
    lock.unlock();
}
unsigned int orm_conn_pool::init_edit_conn(unsigned char n)
{
    for (unsigned char i = 0; i < n; ++i)
    {
        try
        {
            std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
            bool isok                             = conn->connect(conf_data[0].host, conf_data[0].port, conf_data[0].user, conf_data[0].password, conf_data[0].dbname, false);
            if (isok)
            {
                std::unique_lock<std::mutex> lock(conn_edit_mutex);
                conn_edit_pool.emplace_back(conn);
                lock.unlock();
                continue;
            }
            error_msg.append(" add_select_connect failed for tag " + conf_data[0].tag);
            conn->error_msg.append(" add_select_connect failed for tag " + conf_data[0].tag);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }

        return i;
    }
    return n;
}
std::shared_ptr<mysql_conn_base> orm_conn_pool::add_select_connect()
{
    std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
    bool isok                             = conn->connect(conf_data[1].host, conf_data[1].port, conf_data[1].user, conf_data[1].password, conf_data[1].dbname, false);
    if (isok)
    {
        return conn;
    }
    error_msg.append(" add_select_connect failed for tag " + conf_data[1].tag);
    conn->error_msg.append(" add_select_connect failed for tag " + conf_data[1].tag);
    throw conn->error_msg;
}
asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_select_connect()
{
    std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
    bool isok                             = co_await conn->async_connect(conf_data[1].host, conf_data[1].port, conf_data[1].user, conf_data[1].password, conf_data[1].dbname, false);
    if (isok)
    {
        co_return conn;
    }
    error_msg.append(" add_select_connect failed for tag " + conf_data[1].tag);
    conn->error_msg.append(" add_select_connect failed for tag " + conf_data[1].tag);
    throw conn->error_msg;
}
unsigned int orm_conn_pool::init_select_conn(unsigned char n)
{
    for (unsigned char i = 0; i < n; ++i)
    {
        try
        {
            std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
            bool isok                             = conn->connect(conf_data[1].host, conf_data[1].port, conf_data[1].user, conf_data[1].password, conf_data[1].dbname, false);
            if (isok)
            {
                std::unique_lock<std::mutex> lock(conn_edit_mutex);
                conn_select_pool.emplace_back(conn);
                lock.unlock();
            }
            error_msg.append(" add_select_connect failed for tag " + conf_data[1].tag);
            conn->error_msg.append(" add_select_connect failed for tag " + conf_data[1].tag);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
        return i;
    }
    return n;
}
void orm_conn_pool::back_select_conn(std::shared_ptr<mysql_conn_base> conn)
{
    conn->issynch = false;
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    conn_select_pool.emplace_back(conn);
    lock.unlock();
}

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_get_edit_conn()
{
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (conn_edit_pool.empty())
    {
        lock.unlock();
        try
        {
            co_return co_await async_add_edit_connect();
        }
        catch (const char *e)
        {
            throw e;
        }
        catch (const std::string &e)
        {
            throw e;
        }
    };

    auto temp = std::move(conn_edit_pool.front());
    conn_edit_pool.pop_front();
    lock.unlock();
    if (temp->is_closed())
    {
        try
        {
            co_return co_await async_add_edit_connect();
        }
        catch (const char *e)
        {
            throw e;
        }
        catch (const std::string &e)
        {
            throw e;
        }
    }
    co_return temp;
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::get_edit_conn()
{
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (conn_edit_pool.empty())
    {
        lock.unlock();
        try
        {
            return add_edit_connect();
        }
        catch (const char *e)
        {
            throw e;
        }
        catch (const std::string &e)
        {
            throw e;
        }
    };

    auto temp = std::move(conn_edit_pool.front());
    conn_edit_pool.pop_front();
    lock.unlock();
    if (temp->is_closed())
    {
        try
        {
            return add_edit_connect();
        }
        catch (const char *e)
        {
            throw e;
        }
        catch (const std::string &e)
        {
            throw e;
        }
    }
    return temp;
}
asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_get_select_conn()
{
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (conn_select_pool.empty())
    {
        lock.unlock();
        try
        {
            co_return co_await async_add_select_connect();
        }
        catch (const char *e)
        {
            throw e;
        }
        catch (const std::string &e)
        {
            throw e;
        }
    };

    auto temp = std::move(conn_select_pool.front());
    conn_select_pool.pop_front();
    lock.unlock();

    if (temp->is_closed())
    {
        try
        {
            co_return co_await async_add_select_connect();
        }
        catch (const char *e)
        {
            throw e;
        }
        catch (const std::string &e)
        {
            throw e;
        }
    }

    co_return temp;
}
std::shared_ptr<mysql_conn_base> orm_conn_pool::get_select_conn()
{
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (conn_select_pool.empty())
    {
        lock.unlock();
        try
        {
            return add_select_connect();
        }
        catch (const char *e)
        {
            throw e;
        }
        catch (const std::string &e)
        {
            throw e;
        }
    };

    auto temp = std::move(conn_select_pool.front());
    conn_select_pool.pop_front();
    lock.unlock();
    if (temp->is_closed())
    {
        try
        {
            return add_select_connect();
        }
        catch (const char *e)
        {
            throw e;
        }
        catch (const std::string &e)
        {
            throw e;
        }
    }
    return temp;
}

asio::awaitable<bool> orm_conn_pool::clear_select_conn_2hour()
{
    unsigned int nowtimeid = time((time_t *)NULL);
    if(nowtimeid > CONST_ORM_CLEAR_TIME)
    {
        nowtimeid = nowtimeid-CONST_ORM_CLEAR_TIME;
    }
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if(conn_select_pool.size()>0)
    {
        auto temp = std::move(conn_select_pool.front());
        conn_select_pool.pop_front();

        if(temp->time_start < nowtimeid)
        {
            lock.unlock();
            co_await  temp->async_close();
            co_return true;
        }
        else if(temp->query_num > CONST_ORM_CLEAR_NUMBER)
        {
            lock.unlock();
            co_await  temp->async_close();
            co_return true;
        }
        else 
        {
            conn_select_pool.emplace_back(temp);
        }
    }
    lock.unlock();
 
    co_return false;
}

asio::awaitable<bool> orm_conn_pool::clear_edit_conn_2hour()
{
    unsigned int nowtimeid = time((time_t *)NULL);
    if(nowtimeid > CONST_ORM_CLEAR_TIME)
    {
        nowtimeid = nowtimeid - CONST_ORM_CLEAR_TIME;
    }
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if(conn_edit_pool.size()>0)
    {
        auto temp = std::move(conn_edit_pool.front());
        conn_edit_pool.pop_front();

        if(temp->time_start < nowtimeid)
        {
            lock.unlock();
            co_await  temp->async_close();
            co_return true;
        }
        else if(temp->query_num > CONST_ORM_CLEAR_NUMBER)
        {
            lock.unlock();
            co_await  temp->async_close();
            co_return true;
        }
        else 
        {
            conn_edit_pool.emplace_back(temp);
        }
    }
    lock.unlock();
 
    co_return false;
}

unsigned int orm_conn_pool::clear_select_conn()
{
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    unsigned int n = 0;
    for (; !conn_select_pool.empty();)
    {
        auto temp = std::move(conn_select_pool.front());
        conn_select_pool.pop_front();
        temp->close();
        n++;
        if (n > 5)
        {
            break;
        }
    }
    lock.unlock();
    return n;
}
unsigned int orm_conn_pool::clear_edit_conn()
{
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    unsigned int n = 0;
    for (; !conn_edit_pool.empty();)
    {
        auto temp = std::move(conn_edit_pool.front());
        conn_edit_pool.pop_front();
        temp->close();
        n++;
        if (n > 5)
        {
            break;
        }
    }
    lock.unlock();
    return n;
}
}// namespace orm