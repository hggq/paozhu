/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-03-08
 */
#ifndef _MYSQL_CONNECT_MAR_H
#define _MYSQL_CONNECT_MAR_H

#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <list>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "mysql_conn.h"

namespace orm
{


class orm_connect_mar_t
{
  public:
    void watch_connect(std::weak_ptr<mysql_conn_base> conn);
    void clear_connect();
    void push_log(const std::string &str);
    void push_log(const std::string &str1,const std::string &str2);
    void push_log(const std::string &str1,const std::string &str2,const std::string &str3);
    void save_log(const std::string &str);
    
  public:
    std::list<std::weak_ptr<mysql_conn_base>> conn_list;
    std::string log_content;
    std::mutex connect_mutex;
    std::mutex log_mutex;
};
 
orm_connect_mar_t &get_orm_connect_mar(); 
 
}// namespace orm
#endif