/*
 * test_orm_lazy_reconnect.cpp
 *
 * 回归用例：启动期数据库不可用时，ORM 不能注销整个 tag。
 * 全局池必须保留该 tag 的 edit/select 配置与槽位，
 * 这样数据库起来后下一条请求走 get_*_conn 才能懒建连接恢复。
 *
 * 同时验证健康标记 pool_down：
 *   - 启动预建失败 ⇒ true；
 *   - 运行期懒建失败（同步 get_*_conn 抛出）⇒ 仍然 true，指标不会停在"健康"；
 *   - 懒建成功 ⇒ 复位 false（该腿要连一条真能通的库，见 argv[1]）。
 */
#include <asio.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "orm_conn_pool.h"

static int g_pass = 0;
static int g_fail = 0;

static void check(bool cond, const std::string &name)
{
    if (cond)
    {
        ++g_pass;
        std::cout << "[PASS] " << name << "\n";
    }
    else
    {
        ++g_fail;
        std::cout << "[FAIL] " << name << "\n";
    }
}

int main(int argc, char **argv)
{
    namespace fs = std::filesystem;
    fs::create_directories("temp");
    const fs::path conf_path = "temp/test_orm_lazy_reconnect.conf";
    {
        std::ofstream out(conf_path);
        // 两个角色故意指向不同端口，用来确认 edit 失败后 select 配置也完成了注册。
        // 127.0.0.1 上的这两个端口应当无人监听，预建必然失败。
        out << R"([lazyreconnect]
type = main
host = 127.0.0.1
port = 1
dbname = lazyreconnect
user = lazyreconnect
password = lazyreconnect
minpool = 1
maxpool = 2
dbtype = mysql
charset = utf8mb4

type = second
host = 127.0.0.1
port = 2
dbname = lazyreconnect
user = lazyreconnect
password = lazyreconnect
minpool = 1
maxpool = 2
dbtype = mysql
charset = utf8mb4
)";
    }

    auto ioc = std::make_shared<asio::io_context>();
    const std::string log = orm::init_orm_conn_pool(ioc, conf_path.string());

    auto &pools = orm::get_orm_conn_pool_obj();
    auto iter   = pools.find("lazyreconnect");
    check(iter != pools.end(), "prebuild failure keeps tag in global pool");
    check(log.find("pool kept for lazy reconnect") != std::string::npos,
          "prebuild failure is logged as retryable warning");
    if (iter != pools.end())
    {
        auto &pool = *iter->second;
        check(pool.conf_data[0].port == "1", "edit config saved for lazy reconnect");
        check(pool.conf_data[1].port == "2", "select config saved after edit prebuild failure");
        check(!pool.slots.empty(), "slots kept so get_*_conn can build on demand");
        check(pool.pool_down.load(std::memory_order_relaxed),
              "pool_down set after startup prebuild failure");

        // 运行期腿：池是空的，get_*_conn 直接建连 ⇒ 失败抛出而不是交出死连接，
        // 且这一次失败必须把 pool_down 顶住，指标才不会停在"健康"。
        bool edit_threw = false;
        try
        {
            auto conn = pool.get_mysql_edit_conn();
            (void)conn;
        }
        catch (const std::exception &e)
        {
            edit_threw = std::string(e.what()).find("add_mysql_edit_connect failed") != std::string::npos;
        }
        check(edit_threw, "failed lazy edit connect throws with its own reason");
        check(pool.pool_down.load(std::memory_order_relaxed),
              "failed lazy edit connect keeps pool_down set");

        bool select_threw = false;
        try
        {
            auto conn = pool.get_mysql_select_conn();
            (void)conn;
        }
        catch (const std::exception &e)
        {
            select_threw = std::string(e.what()).find("add_mysql_select_connect failed") != std::string::npos;
        }
        check(select_threw, "failed lazy select connect throws with its own reason");
        check(pool.pool_down.load(std::memory_order_relaxed),
              "failed lazy select connect keeps pool_down set");
    }

    // 复位腿需要一条真能连上的库：argv[1] = orm.conf 路径，argv[2] = 其中的 MySQL tag（默认 default）。
    // 用法 ./test_orm_lazy_reconnect conf/orm.conf default
    if (argc > 1)
    {
        const std::string live_tag = argc > 2 ? argv[2] : "default";
        orm::init_orm_conn_pool(ioc, argv[1]);
        auto it = pools.find(live_tag);
        if (it == pools.end())
        {
            check(false, "live tag " + live_tag + " found in " + argv[1]);
        }
        else
        {
            auto &live = *it->second;
            live.clear_edit_conn();
            live.clear_select_conn();
            live.pool_down.store(true, std::memory_order_relaxed);
            try
            {
                auto conn = live.get_mysql_edit_conn();
                check(conn != nullptr, "lazy edit connect built against a reachable database");
                check(!live.pool_down.load(std::memory_order_relaxed),
                      "successful lazy connect clears pool_down");
            }
            catch (const std::exception &e)
            {
                check(false, std::string("lazy edit connect against a reachable database: ") + e.what());
            }
        }
    }
    else
    {
        std::cout << "[SKIP] pool_down reset leg needs argv[1]=orm.conf with a reachable MySQL tag\n";
    }

    fs::remove(conf_path);
    std::cout << "pass=" << g_pass << " fail=" << g_fail << "\n";
    return g_fail == 0 ? 0 : 1;
}
