/*
 * test_orm_lazy_reconnect.cpp
 *
 * 回归用例：启动期数据库不可用时，ORM 不能注销整个 tag。
 * 全局池必须保留该 tag 的 edit/select 配置与槽位，
 * 这样数据库起来后下一条请求走 get_*_conn 才能懒建连接恢复。
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

int main()
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
        const auto &pool = *iter->second;
        check(pool.conf_data[0].port == "1", "edit config saved for lazy reconnect");
        check(pool.conf_data[1].port == "2", "select config saved after edit prebuild failure");
        check(!pool.slots.empty(), "slots kept so get_*_conn can build on demand");
    }

    fs::remove(conf_path);
    std::cout << "pass=" << g_pass << " fail=" << g_fail << "\n";
    return g_fail == 0 ? 0 : 1;
}
