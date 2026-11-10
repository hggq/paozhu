/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#ifndef _ORM_CONN_POOL_H
#define _ORM_CONN_POOL_H

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <list>
#include <array>
#include <unordered_map>
#include <vector>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "mysql_conn.h"
#include "pg_conn.h"
#include "sqlite_conn.h"

namespace orm
{

bool str_casecmp(std::string_view str1, std::string_view str2);

// 一条 tag 的连接队列按 io_context 拆成 N 个槽，槽按 io_context 下标对齐。
// 只有本槽的连接队列、锁与计数；tag 级配置（conf_data / min_pool / max_pool）留在 pool，
// 槽只读不复制。
struct orm_slot
{
    // ioc 必须自持，且声明在队列之前：成员逆序析构 => holder 最后释放，
    // 保证连接 close() 时 socket 所属 io_context 仍然活着。
    std::shared_ptr<asio::io_context> io_context_holder;
    asio::io_context *io_context = nullptr;

    std::list<std::shared_ptr<mysql_conn_base>> mysql_edit_pool;
    std::list<std::shared_ptr<mysql_conn_base>> mysql_select_pool;
    std::list<std::shared_ptr<pg_conn_base>> pg_edit_pool;
    std::list<std::shared_ptr<pg_conn_base>> pg_select_pool;
    // SQLite 单连接模式：只使用槽 0
    std::shared_ptr<sqlite_conn_base> sqlite_edit_conn;
    std::shared_ptr<sqlite_conn_base> sqlite_select_conn;

    // 每槽各一套锁，避免不同 ioc 的线程争同一把
    std::mutex edit_mutex;
    std::mutex select_mutex;

    // 计数与在途建连都按角色分开，下标同 conf_data：0=edit(main)，1=select(second)。
    // orm.conf 每个角色各写一条 maxpool（默认 5 / 20），共用一份计数会让 select 池
    // 正常涨满后把 edit 的名额一起吃光，写路径从此只在退避循环里打转。
    std::atomic<unsigned int> connecting[2]  = {0, 0};// 已发起、尚未入池的建连
    // 空闲 + 借出 + 在途；门闩判断用它。计数由连接自己的 conn_link 持 shared_ptr 归还：
    // 建连成功时 bind 报数，连接对象析构时还名额，槽销毁后名额也还得回去。
    std::shared_ptr<std::atomic<unsigned int>> total_count[2] = {
        std::make_shared<std::atomic<unsigned int>>(0),
        std::make_shared<std::atomic<unsigned int>>(0)};
};

class orm_conn_pool
{
  public:
    // MySQL connections
    std::shared_ptr<mysql_conn_base> get_mysql_edit_conn();
    std::shared_ptr<mysql_conn_base> add_mysql_edit_connect();
    void back_mysql_edit_conn(std::shared_ptr<mysql_conn_base> conn);
    unsigned int init_mysql_edit_conn(unsigned char n, unsigned int slot_index = 0);
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_add_mysql_edit_connect();
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_get_mysql_edit_conn();

    std::shared_ptr<mysql_conn_base> get_mysql_select_conn();
    std::shared_ptr<mysql_conn_base> add_mysql_select_connect();
    void back_mysql_select_conn(std::shared_ptr<mysql_conn_base> conn);
    unsigned int init_mysql_select_conn(unsigned char n, unsigned int slot_index = 0);
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_add_mysql_select_connect();
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_get_mysql_select_conn();

    // PostgreSQL connections
    std::shared_ptr<pg_conn_base> get_pg_edit_conn();
    std::shared_ptr<pg_conn_base> add_pg_edit_connect();
    void back_pg_edit_conn(std::shared_ptr<pg_conn_base> conn);
    unsigned int init_pg_edit_conn(unsigned char n, unsigned int slot_index = 0);
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_add_pg_edit_connect();
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_get_pg_edit_conn();

    std::shared_ptr<pg_conn_base> get_pg_select_conn();
    std::shared_ptr<pg_conn_base> add_pg_select_connect();
    void back_pg_select_conn(std::shared_ptr<pg_conn_base> conn);
    unsigned int init_pg_select_conn(unsigned char n, unsigned int slot_index = 0);
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_add_pg_select_connect();
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_get_pg_select_conn();

    // SQLite connections
    std::shared_ptr<sqlite_conn_base> get_sqlite_edit_conn();
    std::shared_ptr<sqlite_conn_base> add_sqlite_edit_connect();
    void back_sqlite_edit_conn(std::shared_ptr<sqlite_conn_base> conn);
    unsigned int init_sqlite_edit_conn(unsigned char n, unsigned int slot_index = 0);
    asio::awaitable<std::shared_ptr<sqlite_conn_base>> async_add_sqlite_edit_connect();
    asio::awaitable<std::shared_ptr<sqlite_conn_base>> async_get_sqlite_edit_conn();

    std::shared_ptr<sqlite_conn_base> get_sqlite_select_conn();
    std::shared_ptr<sqlite_conn_base> add_sqlite_select_connect();
    void back_sqlite_select_conn(std::shared_ptr<sqlite_conn_base> conn);
    unsigned int init_sqlite_select_conn(unsigned char n, unsigned int slot_index = 0);
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
        // slots 与连接表的声明顺序。close() 要往 socket 发包，此刻 io_context
        // 必须还活着——由槽内的 holder 担保。走的是
        // init_orm_conn_pool_release() 已有的同一条路径。
        clear_select_conn();
        clear_edit_conn();
    }

  public:
    // 按 io_context 下标对齐的槽数组；init 时定长创建，之后不再 resize。
    std::vector<std::unique_ptr<orm_slot>> slots;

    // 兼容视图：老代码与 tests 直接读 pool->mysql_edit_pool 等，语义是"槽 0 的队列"。
    // 分槽后队列搬进了 orm_slot，这里保留同名入口，避免改那些独立二进制。
    template <typename ListT>
    class slot0_list_view
    {
      public:
        slot0_list_view() = default;
        slot0_list_view(orm_conn_pool *pool, ListT orm_slot::*member)
            : pool_(pool), member_(member)
        {
        }
        bool empty() { return list().empty(); }
        std::size_t size() { return list().size(); }
        typename ListT::iterator begin() { return list().begin(); }
        typename ListT::iterator end() { return list().end(); }

      private:
        ListT &list() { return pool_->slot_ref(0).*member_; }

        orm_conn_pool *pool_     = nullptr;
        ListT orm_slot::*member_ = nullptr;
    };

    slot0_list_view<std::list<std::shared_ptr<mysql_conn_base>>> mysql_edit_pool{this, &orm_slot::mysql_edit_pool};
    slot0_list_view<std::list<std::shared_ptr<mysql_conn_base>>> mysql_select_pool{this, &orm_slot::mysql_select_pool};
    slot0_list_view<std::list<std::shared_ptr<pg_conn_base>>> pg_edit_pool{this, &orm_slot::pg_edit_pool};
    slot0_list_view<std::list<std::shared_ptr<pg_conn_base>>> pg_select_pool{this, &orm_slot::pg_select_pool};
    // execution_context -> 槽下标 的只读反查表。pool 与 server 共持一份 shared_ptr，
    // 保证 pool 析构前表仍然有效；查不到/越界一律回退槽 0。
    std::shared_ptr<const std::unordered_map<const asio::execution_context *, unsigned int>> ioc_index;

    // tag 级配置：host/port/dbname/user/password/ssl/charset/pretable/db_type，
    // 以及每槽的上限/预建策略 max_pool / min_pool。槽只读，不持有副本。
    std::array<orm_conn_t, 2> conf_data;
    std::string error_msg;
    // 兼容成员：槽 0 的 ioc 裸指针。httpcli 代码生成器与旧调用点直接写它，
    // 新代码一律用 slots[s]->io_context。
    asio::io_context *io_context = nullptr;

  private:
    // ---- 槽定位与门闩 ----
    orm_slot &slot_ref(unsigned int slot_index);
    unsigned int slot_of(const asio::execution_context &ctx) const;
    unsigned int slot_of(const asio::io_context *ioc) const;
    // 每槽上限：max_pool 未配置（0）按 1；link_type 0=edit，1=select。
    unsigned int max_pool_of(unsigned int link_type) const;

    // ---- 槽内建连（锁外调用，成功后把该角色的名额绑到这条连接上）----
    std::shared_ptr<mysql_conn_base> add_mysql_edit_connect(orm_slot &slot);
    std::shared_ptr<mysql_conn_base> add_mysql_select_connect(orm_slot &slot);
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_add_mysql_edit_connect(orm_slot &slot);
    asio::awaitable<std::shared_ptr<mysql_conn_base>> async_add_mysql_select_connect(orm_slot &slot);

    std::shared_ptr<pg_conn_base> add_pg_edit_connect(orm_slot &slot);
    std::shared_ptr<pg_conn_base> add_pg_select_connect(orm_slot &slot);
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_add_pg_edit_connect(orm_slot &slot);
    asio::awaitable<std::shared_ptr<pg_conn_base>> async_add_pg_select_connect(orm_slot &slot);
};
std::vector<orm_conn_t> get_orm_config_file(const std::string &filename);
std::map<std::string, std::shared_ptr<orm_conn_pool>> &get_orm_conn_pool_obj();

// 多 ioc：pool 按 iocs 下标建槽，ioc_index 是 execution_context -> 下标的只读反查表。
// pool 持一份 ioc_index，保证表比 pool 活得久。
std::string init_orm_conn_pool(
    std::vector<std::shared_ptr<asio::io_context>> iocs,
    std::shared_ptr<const std::unordered_map<const asio::execution_context *, unsigned int>> ioc_index,
    const std::string &orm_config_file);

// 单 ioc：等价于 1 个槽、空反查表（所有查询回退槽 0）。独立二进制与 tests 继续用它。
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
