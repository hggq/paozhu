/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#ifndef _ORM_CONN_POOL_H
#define _ORM_CONN_POOL_H

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
#include "pg_conn.h"

namespace orm
{

bool str_casecmp(std::string_view str1, std::string_view str2);
class orm_conn_pool
{
  public:
    // MySQL connections
    std::shared_ptr<mysql_conn_base> get_mysql_edit_conn();
    std::shared_ptr<mysql_conn_base> add_mysql_edit_connect();
    void back_mysql_edit_conn(std::shared_ptr<mysql_conn_base> conn);
    unsigned int init_mysql_edit_conn(unsigned char n);
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_add_mysql_edit_connect();
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_get_mysql_edit_conn();

    std::shared_ptr<mysql_conn_base> get_mysql_select_conn();
    std::shared_ptr<mysql_conn_base> add_mysql_select_connect();
    void back_mysql_select_conn(std::shared_ptr<mysql_conn_base> conn);
    unsigned int init_mysql_select_conn(unsigned char n);
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_add_mysql_select_connect();
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_get_mysql_select_conn();

    // PostgreSQL connections
    std::shared_ptr<pg_conn_base> get_pg_edit_conn();
    std::shared_ptr<pg_conn_base> add_pg_edit_connect();
    void back_pg_edit_conn(std::shared_ptr<pg_conn_base> conn);
    unsigned int init_pg_edit_conn(unsigned char n);
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_add_pg_edit_connect();
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_get_pg_edit_conn();

    std::shared_ptr<pg_conn_base> get_pg_select_conn();
    std::shared_ptr<pg_conn_base> add_pg_select_connect();
    void back_pg_select_conn(std::shared_ptr<pg_conn_base> conn);
    unsigned int init_pg_select_conn(unsigned char n);
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_add_pg_select_connect();
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_get_pg_select_conn();

    // Common
    unsigned int clear_edit_conn();
    unsigned int clear_select_conn();
    asio::awaitable<bool> clear_edit_conn_2hour();
    asio::awaitable<bool> clear_select_conn_2hour();

    // Helper: check if this pool is for PostgreSQL
    bool is_postgresql() const;

    ~orm_conn_pool()
    {
    }

  public:
    // MySQL pools
    std::list<std::shared_ptr<mysql_conn_base>> mysql_edit_pool;
    std::list<std::shared_ptr<mysql_conn_base>> mysql_select_pool;
    // PostgreSQL pools
    std::list<std::shared_ptr<pg_conn_base>> pg_edit_pool;
    std::list<std::shared_ptr<pg_conn_base>> pg_select_pool;

    std::array<orm_conn_t, 2> conf_data;
    std::mutex conn_edit_mutex;
    std::mutex conn_select_mutex;
    std::string error_msg;
    asio::io_context *io_context = nullptr;
};
std::vector<orm_conn_t> get_orm_config_file(const std::string &filename);
std::map<std::string, std::shared_ptr<orm_conn_pool>> &get_orm_conn_pool_obj();

std::string init_orm_conn_pool(asio::io_context &ioc, const std::string &orm_config_file);
std::string init_orm_conn_pool_release();
}// namespace orm
#endif