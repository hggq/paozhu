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
#include "sqlite_conn.h"

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

    // SQLite connections
    std::shared_ptr<sqlite_conn_base> get_sqlite_edit_conn();
    std::shared_ptr<sqlite_conn_base> add_sqlite_edit_connect();
    void back_sqlite_edit_conn(std::shared_ptr<sqlite_conn_base> conn);
    unsigned int init_sqlite_edit_conn(unsigned char n);
    asio::awaitable<std::shared_ptr<sqlite_conn_base>> async_add_sqlite_edit_connect();
    asio::awaitable<std::shared_ptr<sqlite_conn_base>> async_get_sqlite_edit_conn();

    std::shared_ptr<sqlite_conn_base> get_sqlite_select_conn();
    std::shared_ptr<sqlite_conn_base> add_sqlite_select_connect();
    void back_sqlite_select_conn(std::shared_ptr<sqlite_conn_base> conn);
    unsigned int init_sqlite_select_conn(unsigned char n);
    asio::awaitable<std::shared_ptr<sqlite_conn_base>> async_add_sqlite_select_connect();
    asio::awaitable<std::shared_ptr<sqlite_conn_base>> async_get_sqlite_select_conn();

    // Common
    unsigned int clear_edit_conn();
    unsigned int clear_select_conn();
    asio::awaitable<bool> clear_edit_conn_2hour();
    asio::awaitable<bool> clear_select_conn_2hour();

    // Helper: check if this pool is for PostgreSQL
    bool is_postgresql() const;
    // Helper: check if this pool is for SQLite
    bool is_sqlite() const;

    ~orm_conn_pool()
    {
        // 析构体先于任何成员析构运行：在这里关连接，关闭时机就不依赖下面
        // io_context_holder 与三个连接表的声明顺序。close() 要往 socket 发包，
        // 此刻 io_context 必须还活着——由 holder 担保。走的是
        // init_orm_conn_pool_release() 已有的同一条路径。
        clear_select_conn();
        clear_edit_conn();
    }

  public:
    // 连接关闭时 io_context 必须还活着：pool 持一份所有权，ioc 就活到最后一次
    // close 之后。成员按声明逆序析构，所以 holder 必须在三个连接表之前，
    // 才能比它们后死。
    std::shared_ptr<asio::io_context> io_context_holder;
    // MySQL pools
    std::list<std::shared_ptr<mysql_conn_base>> mysql_edit_pool;
    std::list<std::shared_ptr<mysql_conn_base>> mysql_select_pool;
    // PostgreSQL pools
    std::list<std::shared_ptr<pg_conn_base>> pg_edit_pool;
    std::list<std::shared_ptr<pg_conn_base>> pg_select_pool;
    // SQLite 单连接模式: 同库任务已在框架 worker 线程串行,
    // 每角色一条常驻连接即可 (多连接也只是在同一线程排队)
    std::shared_ptr<sqlite_conn_base> sqlite_edit_conn;
    std::shared_ptr<sqlite_conn_base> sqlite_select_conn;

    std::array<orm_conn_t, 2> conf_data;
    std::mutex conn_edit_mutex;
    std::mutex conn_select_mutex;
    std::string error_msg;
    asio::io_context *io_context = nullptr;
};
std::vector<orm_conn_t> get_orm_config_file(const std::string &filename);
std::map<std::string, std::shared_ptr<orm_conn_pool>> &get_orm_conn_pool_obj();

// pool 接管 io_context 的生命周期：io_context 活到最后一个 pool 析构完毕。
std::string init_orm_conn_pool(std::shared_ptr<asio::io_context> ioc,
                               const std::string &orm_config_file);
// pool 不接管生命周期，只记录地址：调用方必须让自己的 ioc 活得比全局 pool 久，
// 也就是在自己的 ioc 析构前调 init_orm_conn_pool_release() 把连接关完。
// 保留只为兼容外部代码；新代码一律传 shared_ptr，否则退出期仍可能踩析构顺序。
[[deprecated("pass shared_ptr<asio::io_context> so the pool ties its lifetime")]]
std::string init_orm_conn_pool(asio::io_context &ioc, const std::string &orm_config_file);
std::string init_orm_conn_pool_release();
}// namespace orm
#endif