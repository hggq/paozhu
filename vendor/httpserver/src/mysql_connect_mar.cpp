/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-03-08
 */
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <ctime>
#include <cmath>
#include "terminal_color.h"
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
#include "mysql_connect_mar.h"
#include "cost_define.h"

namespace orm
{

orm_connect_mar_t &get_orm_connect_mar()
{
    static orm_connect_mar_t instance;
    return instance;
}
void orm_connect_mar_t::push_log(const std::string &str1, const std::string &str2, const std::string &str3)
{
    std::unique_lock lk(log_mutex);
    if (log_content.size() > 2097152)
    {
        lk.unlock();
        return;
    }
    log_content.push_back(0x0A);
    log_content.append(str1);
    log_content.push_back(0x20);
    log_content.append(str2);
    log_content.push_back(0x20);
    log_content.append(str3);
    lk.unlock();
}
void orm_connect_mar_t::push_log(const std::string &str1, const std::string &str2)
{
    std::unique_lock lk(log_mutex);
    if (log_content.size() > 2097152)
    {
        lk.unlock();
        return;
    }
    log_content.push_back(0x0A);
    log_content.append(str1);
    log_content.push_back(0x20);
    log_content.append(str2);
    lk.unlock();
}
void orm_connect_mar_t::push_log(const std::string &str)
{
    std::unique_lock lk(log_mutex);
    if (log_content.size() > 2097152)
    {
        lk.unlock();
        return;
    }
    log_content.push_back(0x0A);
    log_content.append(str);
    lk.unlock();
}

void orm_connect_mar_t::save_log(const std::string &filename)
{
    std::string temp_file_content;
    std::unique_lock lk(log_mutex);
    temp_file_content = log_content;
    log_content.clear();
    lk.unlock();

    if (temp_file_content.size() == 0)
    {
        return;
    }

#ifndef _WIN32
    struct flock lockstr = {};
#endif

#ifndef _MSC_VER
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1)
    {
        return;
    }

#ifndef _WIN32
    lockstr.l_type   = F_WRLCK;
    lockstr.l_whence = SEEK_END;
    lockstr.l_start  = 0;
    lockstr.l_len    = 0;

    lockstr.l_pid = 0;

    if (fcntl(fd, F_SETLK, &lockstr) == -1)
    {
        close(fd);
        return;
    }
#else
    auto native_handle = (HANDLE)_get_osfhandle(fd);
    auto file_size     = GetFileSize(native_handle, nullptr);
    if (!LockFile(native_handle, file_size, 0, file_size, 0))
    {
        close(fd);
        return;
    }
#endif
    std::size_t n_write = write(fd, temp_file_content.data(), temp_file_content.size());
    // not use
    if (true || n_write > 0)
    {
        n_write = 0;
    }

#ifndef _WIN32
    lockstr.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lockstr) == -1)
    {
        close(fd);
        return;
    }
#else
    if (!UnlockFile(native_handle, file_size, 0, file_size, 0))
    {
        close(fd);
        return;
    }
#endif
    close(fd);
#endif
}
void orm_connect_mar_t::watch_connect(std::weak_ptr<mysql_conn_base> conn)
{
    std::unique_lock lk(connect_mutex);
    conn_list.emplace_back(conn);
    lk.unlock();
}
void orm_connect_mar_t::clear_connect()
{
    const std::chrono::time_point<std::chrono::steady_clock> end_time = std::chrono::steady_clock::now();
    unsigned int nowtimeid                                            = 0;
    unsigned int total_connect                                        = 0;

    std::unique_lock lk(connect_mutex);
    total_connect = conn_list.size();
    lk.unlock();

    //at a time
    total_connect = std::ceil(total_connect / 10);

    if (total_connect == 0)
    {
        total_connect = 1;
    }

    for (unsigned int i = 0; i < total_connect; i++)
    {
        unsigned int j = 0;
        std::unique_lock lk(connect_mutex);
        for (auto iter = conn_list.begin(); iter != conn_list.end();)
        {
            std::shared_ptr<mysql_conn_base> p_session = iter->lock();
            if (p_session)
            {
                if (p_session->issynch)
                {
                    nowtimeid = std::chrono::duration_cast<std::chrono::seconds>(end_time - p_session->time_begin).count();

                    if (nowtimeid > CONST_ORM_QUERY_CONNECT_TIMEOUT)
                    {
                        p_session->hard_close();
                        conn_list.erase(iter++);
                    }
                    else
                    {
                        ++iter;
                    }
                }
                else
                {
                    conn_list.erase(iter++);
                }
            }
            else
            {
                conn_list.erase(iter++);
            }

            j++;
            if (j > 10)
            {
                break;
            }
        }
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::nanoseconds(200));
    }
    DEBUG_LOG("clear_connect:%d", total_connect);
}

}// namespace orm