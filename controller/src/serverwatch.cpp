// serverwatch.cpp — 服务自检路由，原先由 httpserver::httpwatch_register_builtin_routes() 在
// httpwatch() 线程里注册，现在走正常的 controller 注解注册口（注册键带 '/'，URL 面可直接命中）。
//
// frametasks_timeloop 这一条不只是 URL：router 在链上判到 chain_act::run_timeloop 时是按名字
// 找它的（router.cpp 的 call_sync_regfun / find_sitecontent）。这个文件被删掉的话间隔任务不再
// 登记，只留一条 "frametasks_timeloop not registered" 日志。

#include <chrono>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <asio.hpp>
#include "func.h"
#include "httppeer.h"
#include "router.h"
#include "server.h"
#include "server_localvar.h"
#include "terminal_color.h"
#include "version.h"

namespace http
{

//@urlpath(null,paozhu_status)
std::string paozhu_status(std::shared_ptr<httppeer> peer)
{
    httpserver &app  = get_server_app();
    httppeer &client = peer->get_peer();
    client << "<h3 align=\"center\">";
    client << "<span style=\"font-size:2em\">Paozhu</h3> <p align=\"center\">Version ";
    client << (PAOZHU_VERSION / 100000);
    client << "." << (PAOZHU_VERSION / 100 % 1000);
    client << "." << (PAOZHU_VERSION % 100);
    client << "</p>";

    int isshow = -1;
    if (client.get.isset("show_visitinfo"))
    {
        isshow                             = client.get["show_visitinfo"].to_int();
        server_loaclvar &static_server_var = get_server_global_var();
        if (isshow == 1 && static_server_var.debug_enable)
        {
            static_server_var.show_visitinfo = true;
            client << "<p>";
            client << "online:" << app.total_count.load() << " ";
            try
            {
                client << "]</p>";
                client << app.clientrunpool.printthreads(true);
            }
            catch (...)
            {
                client << "<p>exception</p>";
            }
        }
        else
        {
            static_server_var.show_visitinfo = false;
        }
    }
    return "";
}

//@urlpath(null,paozhu_routes)
std::string paozhu_routes(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // 这张表导出每一条路由的名字、前置钩子和注册位置（文件:行号）。任何来源都能读到它，
    // 等于把站点内部的路径清单双手递出去。所以只放行本机/内网来源（ip_is_local()，
    // 与证书下载页同一个闸门）；client_ip 取套接字对端地址，伪造请求头换不来本机身份。
    // 拒绝时正文与"本构建没编这一功能"那批桩逐字节一致：连"这条诊断路由存在"也不透露。
    if (!ip_is_local(client.client_ip))
    {
        client.status(404);
        client.val.set_object();
        client.val["error"] = "not found";
        client.out_json();
        return "";
    }
    server_loaclvar &static_server_var = get_server_global_var();
    if (!static_server_var.debug_enable)
    {
        client << "debug_enable off";
        return "";
    }
    std::string filter;
    if (client.get.isset("name"))
    {
        filter = client.get["name"].to_string();
    }
    peer->type("text/plain; charset=utf-8");
    client << get_router().routes_text(filter);
    return "";
}

//@urlpath(null,frametasks_timeloop)
std::string frametasks_timeloop(std::shared_ptr<httppeer> peer)
{
    if (peer->linktype != 7)
    {
        DEBUG_LOG("timeloop enqueue skipped, linktype:%u", (unsigned int)peer->linktype);
        return "";
    }
    httpserver &app = get_server_app();
    if (peer->timeloop_taskname.size() > 0 && peer->etag.size() > 0)
    {
        std::string temptaskhash = peer->timeloop_taskname;
        temptaskhash.append(peer->url);
        std::size_t temp_name_id = std::hash<std::string>{}(temptaskhash);
        std::ostringstream oss;
        oss << temp_name_id;
        temptaskhash = oss.str();
        if (temptaskhash == peer->etag)
        {
            bool isintask = true;
            {
                std::lock_guard<std::mutex> lk(app.clientlooptasks_mutex);
                for (auto iter = app.clientlooptasks.begin(); iter != app.clientlooptasks.end();)
                {
                    if (iter->first == temp_name_id)
                    {
                        isintask = false;
                        break;
                    }
                    ++iter;
                }
                if (isintask)
                    app.clientlooptasks.push_back({temp_name_id, peer});
            }
            if (isintask)
            {
                app.websocketcondition.notify_one();
                DEBUG_LOG("timeloop task queued:%s every:%u", peer->timeloop_taskname.c_str(), peer->timeloop_num.load());
            }
        }
    }
    return "";
}

}// namespace http

// ============================================================================
// 常驻出站客户端的停机自检
//
// redis 订阅 / websocket / 裸 TCP / mqtt 这四类长连接由框架在启动时各起一条长期协程：
// 连上 → 停在"读"上等对端发数据 → 收到就派钩子 → 断了退避重连。
// 业务想让它停下来，只写 isclose 是不够的：置旗那一刻它正挂在读操作上，没有谁把这次读
// 取消掉，它就永远不会回来查这面旗——连接、协程帧和 fd 一起留在那儿。
// stop() 干的是两件事：置旗 + 取消这条连接上的在途读。
//
// 这一页验的就是第二件事：先记下调用前有没有活跃连接，调 stop()，然后在有界等待里看常驻
// 协程有没有退出（它退出时会把句柄清空，所以"句柄空了"就等于"它真的醒了"）。
// 调用前没有活跃连接 ⇒ 这次结果不作数，可能只是段没配、对端没起来。
//
// 用法：/resident_stop?kind=sock&name=default      kind: redis | ws | sock | mqtt
// ============================================================================
namespace http
{

// 一家常驻注册表：按段名摘出匹配的客户端，逐个 stop()，再等它们的连接句柄清空。
template <typename T>
asio::awaitable<std::string> resident_stop_one(httpserver &app,
                                               std::list<std::weak_ptr<T>> &lst,
                                               std::mutex &mtx,
                                               const std::string &name)
{
    constexpr unsigned int kBoundMs = 5000;// 有界等待：再长就该怀疑是没醒，不是慢
    constexpr unsigned int kStepMs  = 100;

    // "当前有没有活跃连接"：ws/sock/mqtt 三家是 conn()，redis 是 active_sub()
    auto alive_now = [](const std::shared_ptr<T> &p) -> bool
    {
        if constexpr (requires { p->conn(); })
            return p->conn() != nullptr;
        else
            return p->active_sub() != nullptr;
    };

    std::vector<std::shared_ptr<T>> matched;
    {
        std::lock_guard<std::mutex> lk(mtx);
        for (auto &weak_item : lst)
        {
            auto p = weak_item.lock();
            if (p && p->section_name() == name)
                matched.push_back(p);
        }
    }
    if (matched.empty())
    {
        co_return "matched=0 INCONCLUSIVE (no resident client is registered under this section "
                  "name, or the tick already dropped it because isclose is set)\n";
    }

    std::vector<bool> had_conn;
    had_conn.reserve(matched.size());
    for (auto &p : matched)
        had_conn.push_back(alive_now(p));

    for (auto &p : matched)
        p->stop();

    std::vector<bool> still_conn = had_conn;
    unsigned int waited_ms       = 0;
    while (waited_ms < kBoundMs)
    {
        bool any_alive = false;
        for (std::size_t i = 0; i < matched.size(); ++i)
        {
            if (!had_conn[i])
                continue;// 调用前就没连上，不参与判定
            still_conn[i] = alive_now(matched[i]);
            if (still_conn[i])
                any_alive = true;
        }
        if (!any_alive)
            break;
        co_await asio::steady_timer(app.get_ctx(), std::chrono::milliseconds(kStepMs))
            .async_wait(asio::use_awaitable);
        waited_ms += kStepMs;
    }

    unsigned int need = 0, cleared = 0;
    for (std::size_t i = 0; i < matched.size(); ++i)
    {
        if (!had_conn[i])
            continue;
        ++need;
        if (!still_conn[i])
            ++cleared;
    }

    std::string out = "matched=" + std::to_string(matched.size()) +
                      " had_connection=" + std::to_string(need) +
                      " cleared=" + std::to_string(cleared) +
                      " waited=" + std::to_string(waited_ms) + "ms";
    if (need == 0)
        out += " INCONCLUSIVE (no active connection before the call, this client was simply not connected)\n";
    else
        out += (cleared == need ? " PASS (stop() cancelled the in-flight read, the resident coroutine exited)\n" : " FAIL (the flag was set but the coroutine parked on the read never woke up)\n");

    for (std::size_t i = 0; i < matched.size(); ++i)
    {
        out += "  [" + name + "] before=" + std::string(had_conn[i] ? "connected" : "idle") +
               " after=" + std::string(still_conn[i] ? "connected" : "cleared") +
               " isclose=" + std::string(matched[i]->isclose ? "1" : "0") + "\n";
    }
    co_return out;
}

//@urlpath(null,resident_stop)
asio::awaitable<std::string> resident_stop(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    peer->type("text/plain; charset=utf-8");

    std::string kind = client.get["kind"].to_string();
    std::string name = client.get["name"].to_string();
    if (kind.empty() || name.empty())
    {
        client << "usage: /resident_stop?kind=redis|ws|sock|mqtt&name=<section name>\n";
        co_return "";
    }
    httpserver &app = get_server_app();

#ifdef ENABLE_REDIS_CLIENT
    if (kind == "redis")
    {
        auto text = co_await resident_stop_one(app, app.redis_subpub_tasks, app.redis_subpub_task_mutex, name);
        client << "kind=redis " << text;
        co_return "";
    }
#endif
#ifdef ENABLE_WEBSOCKETS_CLIENT
    if (kind == "ws")
    {
        auto text = co_await resident_stop_one(app, app.ws_subpub_tasks, app.ws_subpub_task_mutex, name);
        client << "kind=ws " << text;
        co_return "";
    }
#endif
#ifdef ENABLE_SOCKETS_CLIENT
    if (kind == "sock")
    {
        auto text = co_await resident_stop_one(app, app.sockets_clients, app.sockets_clients_mutex, name);
        client << "kind=sock " << text;
        co_return "";
    }
#endif
#ifdef ENABLE_MQTT_CLIENT
    if (kind == "mqtt")
    {
        auto text = co_await resident_stop_one(app, app.mqtt_clients, app.mqtt_clients_mutex, name);
        client << "kind=mqtt " << text;
        co_return "";
    }
#endif
    client << "unknown kind: " << kind << " (this build does not include that client type, or the name is misspelled)\n";
    co_return "";
}

}// namespace http
