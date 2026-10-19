// 常驻出站客户端的停机自检。
//
// redis 订阅 / websocket / 裸 TCP / mqtt 这四类长连接由框架在启动时各起一条长期协程：
// 连上 → 停在"读"上等对端发数据 → 收到就派钩子 → 断了退避重连。业务只写 isclose 停不下来：
// 置旗那一刻它正挂在读操作上，没人取消这次读，它就永远不会回来查这面旗。stop() 做两件事：
// 置旗 + 取消这条连接上的在途读。
//
// 本路由验的是第二件事：记下调用前有没有活跃连接，调 stop()，然后在有界等待里看常驻协程有没有
// 退出（退出时它会清空连接句柄，所以"句柄空了"就等于"它真的醒了"）。调用前没有活跃连接 ⇒
// 这次结果不作数，可能只是段没配、对端没起来。
//
// 用法：/resident_stop?kind=sock&name=default      kind: redis | ws | sock | mqtt

#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <asio.hpp>
#include "httppeer.h"
#include "resident_stop.h"
#include "server.h"

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

void reg_resident_stop_route()
{
    reg_raw("", "/resident_stop", nullptr, nullptr, nullptr, resident_stop);
}

}// namespace http
