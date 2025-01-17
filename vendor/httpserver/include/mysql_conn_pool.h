/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#ifndef _MYSQL_CONN_POOL_H
#define _MYSQL_CONN_POOL_H

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <list>
#include <array>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "mysql_conn.h"

namespace orm
{
struct orm_conn_t
{
    std::string type;
    std::string host;
    std::string port;
    std::string dbname;
    std::string user;
    std::string password;
    std::string pretable;
    std::string dbtype;

    std::string tag;
    std::string charset;

    bool isssl              = false;
    bool issock             = false;
    unsigned char link_type = 0;// 0 edit 1 select 2 backup
    unsigned char max_pool  = 0;
    unsigned char min_pool  = 0;
};
bool str_casecmp(std::string_view str1, std::string_view str2);
class orm_conn_pool
{
  public:
    std::shared_ptr<mysql_conn_base> get_edit_conn();
    std::shared_ptr<mysql_conn_base> add_edit_connect();
    void back_edit_conn(std::shared_ptr<mysql_conn_base> conn);
    unsigned int clear_edit_conn();
    unsigned int init_edit_conn(unsigned char n);

    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_add_edit_connect();
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_get_edit_conn();

    std::shared_ptr<mysql_conn_base> get_select_conn();
    std::shared_ptr<mysql_conn_base> add_select_connect();
    void back_select_conn(std::shared_ptr<mysql_conn_base> conn);
    unsigned int clear_select_conn();
    unsigned int init_select_conn(unsigned char n);

    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_add_select_connect();
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_get_select_conn();

  public:
    std::list<std::shared_ptr<mysql_conn_base>> conn_edit_pool;
    std::list<std::shared_ptr<mysql_conn_base>> conn_select_pool;
    std::array<orm_conn_t, 2> conf_data;
    std::mutex conn_edit_mutex;
    std::mutex conn_select_mutex;
    std::string error_msg;
    asio::io_context *io_context = nullptr;
};
std::map<std::string, std::shared_ptr<orm_conn_pool>> &get_orm_conn_pool_obj();
void init_orm_conn_pool(asio::io_context &ioc, const std::string &orm_config_file);
}// namespace orm
#endif