#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <list>
#include <mutex>
#include <memory>
#include "mysql.h"
#include "mysqlpool.h"

std::map<std::size_t, mysqllinkpool> &get_mysqlpool()
{
    static std::map<std::size_t, mysqllinkpool> instance;
    return instance;
}
std::mutex lock_select_list;
std::mutex lock_edit_list;
mysqllinkpool::mysqllinkpool(struct mysql_connect_link_info s_info,struct mysql_connect_link_info e_info,unsigned int num, unsigned int editnum):select_link(std::move(s_info)),edit_link(std::move(e_info))
{
    for (unsigned int i = 0; i < num; i++)
    {
        MYSQL_CONN_PTR conn(new MYSQL, &mysql_close);

        mysql_init(conn.get());
        bool rebool = true;
        mysql_options(conn.get(), MYSQL_OPT_RECONNECT, &rebool);
        if (!mysql_real_connect(conn.get(),(select_link.host.size()>0?select_link.host.c_str():NULL),select_link.username.c_str(),select_link.password.c_str(),select_link.db.c_str(),select_link.port,(select_link.unix_socket.size()>0?select_link.unix_socket.c_str():NULL),0))
        {
             throw mysql_error(conn.get());
        }
        mysql_select_pool_list.emplace_back(std::move(conn));
        select_current_num++;
    }

    for (unsigned int i = 0; i < editnum; i++)
    {
        MYSQL_CONN_PTR conn(new MYSQL, &mysql_close);

        mysql_init(conn.get());
        bool rebool = true;
        mysql_options(conn.get(), MYSQL_OPT_RECONNECT, &rebool);
        if (!mysql_real_connect(conn.get(),(edit_link.host.size()>0?edit_link.host.c_str():NULL),edit_link.username.c_str(),edit_link.password.c_str(),edit_link.db.c_str(),edit_link.port,(edit_link.unix_socket.size()>0?edit_link.unix_socket.c_str():NULL),0))
        {
            throw mysql_error(conn.get());
        }
        mysql_edit_pool_list.emplace_back(std::move(conn));
        edit_current_num++;
    }
}
mysqllinkpool::mysqllinkpool(struct mysql_connect_link_info s_info,struct mysql_connect_link_info e_info):select_link(std::move(s_info)),edit_link(std::move(e_info))
{
    for (int i = 0; i < 2; i++)
    {
        MYSQL_CONN_PTR conn(new MYSQL, &mysql_close);

        mysql_init(conn.get());
        bool rebool = true;
        mysql_options(conn.get(), MYSQL_OPT_RECONNECT, &rebool);
        if (!mysql_real_connect(conn.get(), (select_link.host.size()>0?select_link.host.c_str():NULL),select_link.username.c_str(),select_link.password.c_str(),select_link.db.c_str(),select_link.port,(select_link.unix_socket.size()>0?select_link.unix_socket.c_str():NULL),0))
        {
            throw mysql_error(conn.get());
        }
        mysql_select_pool_list.emplace_back(std::move(conn));
    }

    for (int i = 0; i < 1; i++)
    {
        MYSQL_CONN_PTR conn(new MYSQL, &mysql_close);

        mysql_init(conn.get());
        bool rebool = true;
        mysql_options(conn.get(), MYSQL_OPT_RECONNECT, &rebool);
        if (!mysql_real_connect(conn.get(),(edit_link.host.size()>0?edit_link.host.c_str():NULL),edit_link.username.c_str(),edit_link.password.c_str(),edit_link.db.c_str(),edit_link.port,(edit_link.unix_socket.size()>0?edit_link.unix_socket.c_str():NULL),0))
        {
            throw mysql_error(conn.get());
        }
        mysql_edit_pool_list.emplace_back(std::move(conn));
    }
}


MYSQL_CONN_PTR mysqllinkpool::get_select_connect()
{
    std::unique_lock<std::mutex> lock(lock_select_list);
    if (mysql_select_pool_list.empty())
    {
        lock.unlock();
        return add_select_connect();
    }
    auto temp = std::move(mysql_select_pool_list.front());
    mysql_select_pool_list.pop_front();
    select_current_num--;
    lock.unlock();

    return temp;
}

MYSQL_CONN_PTR mysqllinkpool::get_edit_connect()
{   
    std::unique_lock<std::mutex> lock(lock_edit_list);
    if (mysql_edit_pool_list.empty())
    {
        lock.unlock();
        return add_edit_connect();
    };
    
    auto temp = std::move(mysql_edit_pool_list.front());
    mysql_edit_pool_list.pop_front();
    edit_current_num--;
    lock.unlock();
    
    return temp;
}

MYSQL_CONN_PTR mysqllinkpool::add_select_connect()
{
    MYSQL_CONN_PTR conn(new MYSQL, &mysql_close);

    mysql_init(conn.get());
    bool rebool = true;
    mysql_options(conn.get(), MYSQL_OPT_RECONNECT, &rebool);
    if (!mysql_real_connect(conn.get(), (select_link.host.size()>0?select_link.host.c_str():NULL),select_link.username.c_str(),select_link.password.c_str(),select_link.db.c_str(),select_link.port,(select_link.unix_socket.size()>0?select_link.unix_socket.c_str():NULL),0))
    {
        throw mysql_error(conn.get());
    }
    return conn;
}
MYSQL_CONN_PTR mysqllinkpool::add_edit_connect()
{
    MYSQL_CONN_PTR conn(new MYSQL, &mysql_close);

    mysql_init(conn.get());
    bool rebool = true;
    mysql_options(conn.get(), MYSQL_OPT_RECONNECT, &rebool);
    if (!mysql_real_connect(conn.get(),(edit_link.host.size()>0?edit_link.host.c_str():NULL),edit_link.username.c_str(),edit_link.password.c_str(),edit_link.db.c_str(),edit_link.port,(edit_link.unix_socket.size()>0?edit_link.unix_socket.c_str():NULL),0))
    {
        throw mysql_error(conn.get());
    }
    return conn;
}

void mysqllinkpool::back_select_connect(MYSQL_CONN_PTR temp)
{
    std::unique_lock<std::mutex> lock(lock_select_list);
    mysql_select_pool_list.emplace_back(std::move(temp));
    select_current_num++;
    lock.unlock();
}
void mysqllinkpool::back_edit_connect(MYSQL_CONN_PTR temp)
{
    std::unique_lock<std::mutex> lock(lock_edit_list);
    mysql_edit_pool_list.emplace_back(std::move(temp));
    edit_current_num++;
    lock.unlock();
}

unsigned int mysqllinkpool::clearpool()
{   
    {
        int i=0;
        std::unique_lock<std::mutex> lock(lock_edit_list);
        for ( ;!mysql_edit_pool_list.empty();  )
        {
                auto temp = std::move(mysql_edit_pool_list.front());
                mysql_edit_pool_list.pop_front();
                temp.reset();
                edit_current_num--;
                i++;
                if(i>1)
                {
                    break;
                }
        }
        lock.unlock();
    }


    {
        int i=0;
        std::unique_lock<std::mutex> lock(lock_select_list);
        for ( ;!mysql_select_pool_list.empty();  )
        {
                auto temp = std::move(mysql_select_pool_list.front());
                mysql_select_pool_list.pop_front();
                temp.reset();
                select_current_num--;
                i++;
                if(i>1)
                {
                    break;
                }
        }
        lock.unlock();
    }

    
    return 0;
}