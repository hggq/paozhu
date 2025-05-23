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
    //auto &charset_obj = get_orm_mysql_charset();
    auto charset_obj = std::make_unique<mysql_charset_store>();
    charset_obj->mysql_charset_init();
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
                        typeone = strval;
                        //mysqlconf.tag = keyname;

                        mysqlconf.tag.clear();
                        for (unsigned int jj = 0; jj < keyname.size(); jj++)
                        {
                            if (keyname[jj] >= '0' && keyname[jj] <= '9')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }
                            else if (keyname[jj] == '_')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }
                            else if (keyname[jj] >= 'A' && keyname[jj] <= 'Z')
                            {
                                mysqlconf.tag.push_back(keyname[jj] + 32);
                            }
                            else if (keyname[jj] >= 'a' && keyname[jj] <= 'z')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }

                            if (jj > 20)
                            {
                                break;
                            }
                        }
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
                        mysqlconf.charset.clear();
                        mysqlconf.charset_val = 0;
                        //mysqlconf.tag = keyname;

                        mysqlconf.tag.clear();
                        for (unsigned int jj = 0; jj < keyname.size(); jj++)
                        {
                            if (keyname[jj] >= '0' && keyname[jj] <= '9')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }
                            else if (keyname[jj] == '_')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }
                            else if (keyname[jj] >= 'A' && keyname[jj] <= 'Z')
                            {
                                mysqlconf.tag.push_back(keyname[jj] + 32);
                            }
                            else if (keyname[jj] >= 'a' && keyname[jj] <= 'z')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }

                            if (jj > 20)
                            {
                                break;
                            }
                        }

                        mysqlconf.dbtype.clear();

                        mysqlconf.max_pool = 0;
                        mysqlconf.min_pool = 0;
                        mysqlconf.issock   = false;
                        mysqlconf.isssl    = false;
                        mysqlconf.isdebug  = false;
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
                        typeone = strval;
                        //mysqlconf.tag = keyname;

                        mysqlconf.tag.clear();
                        for (unsigned int jj = 0; jj < keyname.size(); jj++)
                        {
                            if (keyname[jj] >= '0' && keyname[jj] <= '9')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }
                            else if (keyname[jj] == '_')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }
                            else if (keyname[jj] >= 'A' && keyname[jj] <= 'Z')
                            {
                                mysqlconf.tag.push_back(keyname[jj] + 32);
                            }
                            else if (keyname[jj] >= 'a' && keyname[jj] <= 'z')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }

                            if (jj > 20)
                            {
                                break;
                            }
                        }

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
                        mysqlconf.charset_val = 0;
                        mysqlconf.max_pool = 0;
                        mysqlconf.min_pool = 0;
                        //mysqlconf.tag      = keyname;
                        
                        mysqlconf.tag.clear();
                        for (unsigned int jj = 0; jj < keyname.size(); jj++)
                        {
                            if (keyname[jj] >= '0' && keyname[jj] <= '9')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }
                            else if (keyname[jj] == '_')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }
                            else if (keyname[jj] >= 'A' && keyname[jj] <= 'Z')
                            {
                                mysqlconf.tag.push_back(keyname[jj] + 32);
                            }
                            else if (keyname[jj] >= 'a' && keyname[jj] <= 'z')
                            {
                                mysqlconf.tag.push_back(keyname[jj]);
                            }

                            if (jj > 20)
                            {
                                break;
                            }
                        }

                        mysqlconf.issock  = false;
                        mysqlconf.isssl   = false;
                        mysqlconf.isdebug = false;
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
                    if (str_casecmp(strval, "127.0.0.1"))
                    {
                        mysqlconf.islocal = true;
                    }
                    if (str_casecmp(strval, "localhost"))
                    {
                        mysqlconf.islocal = true;
                    }
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
                    mysqlconf.max_pool = std::stoi(strval) % 256;
                }
                if (str_casecmp(linestr, "minpool"))
                {
                    mysqlconf.min_pool = std::stoi(strval) % 256;
                }
                if (str_casecmp(linestr, "ssl"))
                {
                    if (strval == "1" || strval == "true" || strval == "True" || strval == "TRUE" || strval == "On" || strval == "ON")
                    {
                        mysqlconf.isssl = true;
                    }
                }
                if (str_casecmp(linestr, "debug"))
                {
                    if (strval == "1" || strval == "true" || strval == "True" || strval == "TRUE" || strval == "On" || strval == "ON")
                    {
                        mysqlconf.isdebug = true;
                    }
                }
                if (str_casecmp(linestr, "dbtype"))
                {
                    mysqlconf.dbtype = strval;
                }
                if (str_casecmp(linestr, "charset"))
                {
                    mysqlconf.charset     = strval;
                    std::transform(mysqlconf.charset.begin(), mysqlconf.charset.end(), mysqlconf.charset.begin(), ::tolower);
                    mysqlconf.charset_val = charset_obj->mysql_charset_find(mysqlconf.charset);
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
            if (str_casecmp(strval, "127.0.0.1"))
            {
                mysqlconf.islocal = true;
            }
            if (str_casecmp(strval, "localhost"))
            {
                mysqlconf.islocal = true;
            }
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
            mysqlconf.max_pool = std::stoi(strval) % 256;
        }
        if (str_casecmp(linestr, "minpool"))
        {
            mysqlconf.min_pool = std::stoi(strval) % 256;
        }
        if (str_casecmp(linestr, "ssl"))
        {
            if (strval == "1" || strval == "true" || strval == "True" || strval == "TRUE" || strval == "On" || strval == "ON")
            {
                mysqlconf.isssl = true;
            }
        }
        if (str_casecmp(linestr, "debug"))
        {
            if (strval == "1" || strval == "true" || strval == "True" || strval == "TRUE" || strval == "On" || strval == "ON")
            {
                mysqlconf.isdebug = true;
            }
        }
        if (str_casecmp(linestr, "dbtype"))
        {
            mysqlconf.dbtype = strval;
        }
        if (str_casecmp(linestr, "charset"))
        {
            mysqlconf.charset = strval;
            std::transform(mysqlconf.charset.begin(), mysqlconf.charset.end(), mysqlconf.charset.begin(), ::tolower);
            mysqlconf.charset_val = charset_obj->mysql_charset_find(mysqlconf.charset);
        }
        //mysqlconf.tag = keyname;
        mysqlconf.tag.clear();
        for (unsigned int jj = 0; jj < keyname.size(); jj++)
        {
            if (keyname[jj] >= '0' && keyname[jj] <= '9')
            {
                mysqlconf.tag.push_back(keyname[jj]);
            }
            else if (keyname[jj] == '_')
            {
                mysqlconf.tag.push_back(keyname[jj]);
            }
            else if (keyname[jj] >= 'A' && keyname[jj] <= 'Z')
            {
                mysqlconf.tag.push_back(keyname[jj] + 32);
            }
            else if (keyname[jj] >= 'a' && keyname[jj] <= 'z')
            {
                mysqlconf.tag.push_back(keyname[jj]);
            }

            if (jj > 20)
            {
                break;
            }
        }

        myconfig.push_back(mysqlconf);
    }
    charset_obj->mysql_charset_clear();
    return myconfig;
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
                    n = iter->second->init_edit_conn(item.min_pool);
                }
                catch (const std::string &error)
                {
                    error_log.append(error);
                }
                catch (std::exception &e)
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
                    n = conn->init_edit_conn(item.min_pool);
                }
                catch (const std::string &error)
                {
                    error_log.append(error);
                }
                catch (std::exception &e)
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
                    n = iter->second->init_select_conn(item.min_pool);
                }
                catch (const std::string &error)
                {
                    error_log.append(error);
                }
                catch (std::exception &e)
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
                    n = conn->init_select_conn(item.min_pool);
                }
                catch (const std::string &error)
                {
                    error_log.append(error);
                }
                catch (std::exception &e)
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

asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_edit_connect()
{
    std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
    bool isok                             = co_await conn->async_connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw " add_edit_connect failed for tag ";
}

std::shared_ptr<mysql_conn_base> orm_conn_pool::add_edit_connect()
{
    std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
    bool isok                             = conn->connect(conf_data[0]);
    if (isok)
    {
        if (conf_data[0].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        return conn;
    }
    throw " add_edit_connect failed for tag ";
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
    unsigned char i = 0;
    for (; i < n; ++i)
    {
        try
        {
            std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
            bool isok                             = conn->connect(conf_data[0]);
            if (isok)
            {
                if (conf_data[0].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(conn_edit_mutex);
                conn_edit_pool.emplace_back(conn);
                lock.unlock();
                continue;
            }
            error_msg = conn->error_msg;
            throw error_msg;
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
}
std::shared_ptr<mysql_conn_base> orm_conn_pool::add_select_connect()
{
    std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
    bool isok                             = conn->connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        conn->issynch = true;
        return conn;
    }
    throw " add_select_connect failed for tag ";
}
asio::awaitable<std::shared_ptr<mysql_conn_base>> orm_conn_pool::async_add_select_connect()
{
    std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
    bool isok                             = co_await conn->async_connect(conf_data[1]);
    if (isok)
    {
        if (conf_data[1].isdebug)
        {
            conn->isdebug = true;
        }
        co_return conn;
    }
    throw " add_select_connect failed for tag ";
}
unsigned int orm_conn_pool::init_select_conn(unsigned char n)
{
    unsigned char i = 0;
    for (; i < n; ++i)
    {
        try
        {
            std::shared_ptr<mysql_conn_base> conn = std::make_shared<mysql_conn_base>(*io_context);
            bool isok                             = conn->connect(conf_data[1]);
            if (isok)
            {
                if (conf_data[1].isdebug)
                {
                    conn->isdebug = true;
                }
                std::unique_lock<std::mutex> lock(conn_edit_mutex);
                conn_select_pool.emplace_back(conn);
                lock.unlock();
            }
            error_msg = conn->error_msg;
            throw error_msg;
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
        }
    }
    return i;
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
        co_return temp;
    }
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
    temp->issynch = true;

    if (temp->is_closed())
    {
        return temp;
    }
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
        co_return temp;
    }
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
    temp->issynch = true;
    if (temp->is_closed())
    {
        return temp;
    }
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

asio::awaitable<bool> orm_conn_pool::clear_select_conn_2hour()
{
    unsigned int nowtimeid = time((time_t *)NULL);
    if (nowtimeid > CONST_ORM_CLEAR_TIME)
    {
        nowtimeid = nowtimeid - CONST_ORM_CLEAR_TIME;
    }
    std::unique_lock<std::mutex> lock(conn_select_mutex);
    if (conn_select_pool.size() > 0)
    {
        auto temp = std::move(conn_select_pool.front());
        conn_select_pool.pop_front();

        if (temp->time_start < nowtimeid)
        {
            lock.unlock();
            co_await temp->async_close();
            co_return true;
        }
        else if (temp->query_num > CONST_ORM_CLEAR_NUMBER)
        {
            lock.unlock();
            co_await temp->async_close();
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
    if (nowtimeid > CONST_ORM_CLEAR_TIME)
    {
        nowtimeid = nowtimeid - CONST_ORM_CLEAR_TIME;
    }
    std::unique_lock<std::mutex> lock(conn_edit_mutex);
    if (conn_edit_pool.size() > 0)
    {
        auto temp = std::move(conn_edit_pool.front());
        conn_edit_pool.pop_front();

        if (temp->time_start < nowtimeid)
        {
            lock.unlock();
            co_await temp->async_close();
            co_return true;
        }
        else if (temp->query_num > CONST_ORM_CLEAR_NUMBER)
        {
            lock.unlock();
            co_await temp->async_close();
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
    }
    lock.unlock();
    return n;
}
}// namespace orm