/**
 *  @copyright copyright 2023, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file client_context.cpp
 *  @date 2023-09-21
 *
 *
 */
#include <iostream>
#include <thread>
#include <asio.hpp>
#include <chrono>
#include <list>
#include <functional>
#include <atomic>
#include <queue>
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
//#include <asio/spawn.hpp>
#include <asio/ssl.hpp>

#include "client_context.h"
#include "datetime.h"
#include "httpclient.h"
#include "fastcgi.h"
#include "terminal_color.h"
#include "http_rpcclient.h"
#include "http_socket_client.h"
#include "http_websocket_client.h"

namespace http
{
client_context &get_client_context_obj(asio::io_context *io_context)
{
    static client_context instance(io_context);
    return instance;
}
client_context::~client_context()
{
    ioc->stop();
    // 这里是静态对象退出期，可能没走过 httpserver::stop()：先把收摊标志置上，
    // 否则下面 join 的是一个还在等活的线程，进程会挂死在 exit() 里
    isstop = true;
    condition.notify_all();
    timeout_condition.notify_all();
    for (unsigned int i = 0; i < httptask_th.size(); i++)
    {
        if (httptask_th[i].joinable())
            httptask_th[i].join();
    }
    for (unsigned int i = 0; i < threads.size(); i++)
    {
        if (threads[i].joinable())
            threads[i].join();
    }
    // time_out_loop_th 是构造函数里起的成员线程，这里必须 join：
    // 析构一个还 joinable 的 std::thread 会直接 std::terminate（退出期表现为 SIGABRT）
    if (time_out_loop_th.joinable())
    {
        time_out_loop_th.join();
    }
}
void client_context::add_http_task(std::shared_ptr<client> temp_task)
{
    clienttasks.emplace(temp_task);
    condition.notify_one();
}
void client_context::add_fastcgi_task(std::shared_ptr<fastcgi> temp_task)
{
    cgitasks.emplace(temp_task);
    condition.notify_one();
}
void client_context::add_websocket_task(std::shared_ptr<websocket_client> temp_task)
{
    websocket_clienttasks.emplace(temp_task);
    condition.notify_one();
}
void client_context::add_socket_task(std::shared_ptr<socket_client> temp_task)
{
    socket_clienttasks.emplace(temp_task);
    condition.notify_one();
}
void client_context::add_mqtt_task(std::shared_ptr<mqtt_client> temp_task)
{
    mqtt_clienttasks.emplace(temp_task);
    condition.notify_one();
}
namespace
{
    enum class timeout_action
    {
        close_erase,
        dur_tick_async,
        dur_tick_sync,
    };

    template <typename T>
    struct timeout_action_item
    {
        std::shared_ptr<T> peer;
        timeout_action action;
    };

    // 锁内摘快照（含擦除链表节点），锁外执行 close_connect/心跳回调；
    // HasDur=false 的类别（mqtt）没有 dur 心跳分支；
    // GuardZeroTimeout=true 的类别（websocket）timeout==0 表示常驻不过期
    template <typename T, bool HasDur, bool GuardZeroTimeout>
    void scan_timeout_list(std::list<std::weak_ptr<T>> &lst, std::mutex &mtx,
                           unsigned int nowtimeid, unsigned int fps)
    {
        std::vector<timeout_action_item<T>> actions;
        {
            std::lock_guard<std::mutex> lk(mtx);
            for (auto iter = lst.begin(); iter != lst.end();)
            {
                std::shared_ptr<T> peer = iter->lock();
                if (!peer)
                {
                    lst.erase(iter++);
                    continue;
                }
                if (peer->iserror || peer->iswait_exit)
                {
                    actions.push_back({peer, timeout_action::close_erase});
                    lst.erase(iter++);
                    continue;
                }
                if constexpr (HasDur)
                {
                    if ((peer->dur_time_loop_fun != nullptr || peer->async_dur_time_loop_fun != nullptr) && peer->durtime > 0)
                    {
                        // dur 托管的连接本拍跳过超时检查（与原实现一致），到拍才发心跳
                        if ((fps % peer->durtime) == 0)
                        {
                            actions.push_back({peer, peer->async_dur_time_loop_fun != nullptr
                                                      ? timeout_action::dur_tick_async
                                                      : timeout_action::dur_tick_sync});
                        }
                        ++iter;
                        continue;
                    }
                }
                unsigned int timeout_val = peer->get_timeout();
                bool expired = (timeout_val < nowtimeid) && (!GuardZeroTimeout || timeout_val > 0);
                if (expired)
                {
                    peer->iswait_exit = true;
                }
                ++iter;
            }
        }
        for (auto &item : actions)
        {
            try
            {
                if (item.action == timeout_action::close_erase)
                {
                    item.peer->close_connect();
                }
                else if constexpr (HasDur)
                {
                    if (item.action == timeout_action::dur_tick_async)
                    {
                        asio::co_spawn(item.peer->strand_, [p = item.peer]() mutable
                                       { return p->async_dur_time_loop_fun(p->shared_from_this()); }, asio::detached);
                    }
                    else
                    {
                        item.peer->dur_time_loop_fun(item.peer->shared_from_this());
                    }
                }
            }
            catch (...)
            {
                std::lock_guard<std::mutex> lk(mtx);
                for (auto iter = lst.begin(); iter != lst.end();)
                {
                    auto p = iter->lock();
                    if (!p || p == item.peer)
                        lst.erase(iter++);
                    else
                        ++iter;
                }
            }
        }
    }
}// namespace
void client_context::time_out_loop()
{
    using namespace std::chrono;
    using dsec                = duration<double>;
    auto invFpsLimit          = duration_cast<system_clock::duration>(dsec{1. / 0.25});
    auto m_BeginFrame         = system_clock::now();
    auto m_EndFrame           = m_BeginFrame + invFpsLimit;
    auto prev_time_in_seconds = time_point_cast<seconds>(m_BeginFrame);
    // unsigned frame_count_per_second = 0;
    unsigned int fps=0;
    for (;;)
    {
        {
            std::unique_lock<std::mutex> lock(this->timeout_mutex);
            if (this->timeout_lists.empty() && this->rpc_timeout_lists.empty() && this->socket_timeout_lists.empty() && this->websocket_timeout_lists.empty() && this->mqtt_timeout_lists.empty())
            {
                fps = 0;
                this->timeout_condition.wait(
                    lock,
                    [this]
                    { return this->isstop || !this->timeout_lists.empty() || !this->rpc_timeout_lists.empty() || !this->socket_timeout_lists.empty() || !this->websocket_timeout_lists.empty() || !this->mqtt_timeout_lists.empty(); });
            }
        }
        // 被 isstop 叫醒就收摊：下面那一拍含 sleep_until，最长要等 4 秒，
        // 而 ~client_context join 的就是本线程，退出期没必要再跑一拍
        if (this->isstop)
        {
            break;
        }

        auto time_in_seconds = time_point_cast<seconds>(system_clock::now());
        //++frame_count_per_second;
        if (time_in_seconds > prev_time_in_seconds)
        {
            DEBUG_LOG("------time loop------");
            //frame_count_per_second = 0;
            prev_time_in_seconds = time_in_seconds;

            unsigned int nowtimeid = timeid();
            scan_timeout_list<client, true, false>(timeout_lists, timeout_mutex, nowtimeid, fps);
            //rpc
            scan_timeout_list<rpc_client, true, false>(rpc_timeout_lists, timeout_mutex, nowtimeid, fps);
            //socket
            scan_timeout_list<socket_client, true, false>(socket_timeout_lists, timeout_mutex, nowtimeid, fps);
            //websocket
            scan_timeout_list<websocket_client, true, true>(websocket_timeout_lists, timeout_mutex, nowtimeid, fps);
            //mqtt
            scan_timeout_list<mqtt_client, false, false>(mqtt_timeout_lists, timeout_mutex, nowtimeid, fps);

        }

        std::this_thread::sleep_until(m_EndFrame);
        m_BeginFrame = m_EndFrame;
        m_EndFrame   = m_BeginFrame + invFpsLimit;
        fps++;
        if (isstop)
        {
            break;
        }
    }
}
void client_context::run()
{
    // worker = std::unique_ptr<asio::io_context::work>(new asio::io_context::work(*ioc));
    // // 创建thread
    // for (unsigned int i = 0; i < thread_size; i++)
    // {
    //     threads.emplace_back([this]()
    //                          {
    //                              std::ostringstream oss;
    //                              oss << std::this_thread::get_id();
    //                              std::string tempthread = oss.str();
    //                              DEBUG_LOG("frame thread:%s", tempthread.c_str());
    //                              this->ioc->run(); });
    // }
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    httptask_th.emplace_back(std::bind(&client_context::taskloop, this));
}

void client_context::taskloop()
{
    for (;;)
    {
        try
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(lock,
                                 [this]
                                 { return this->isstop || !this->clienttasks.empty() || !this->cgitasks.empty()|| !this->websocket_clienttasks.empty()|| !this->socket_clienttasks.empty()|| !this->mqtt_clienttasks.empty(); });

            if (this->clienttasks.size() > 0)
            {
                auto task = std::move(this->clienttasks.front());
                this->clienttasks.pop();
                lock.unlock();
                http_client_task(std::move(task));
            }
            else if (this->cgitasks.size() > 0)
            {
                auto task = std::move(this->cgitasks.front());
                this->cgitasks.pop();
                lock.unlock();
                asio::co_spawn(*this->ioc, fastcgi_client_task(std::move(task)), asio::detached);
            }
            else if (this->websocket_clienttasks.size() > 0)
            {
                auto task = std::move(this->websocket_clienttasks.front());
                this->websocket_clienttasks.pop();
                lock.unlock();
                websocket_client_task(std::move(task));
            }
            else if (this->socket_clienttasks.size() > 0)
            {
                auto task = std::move(this->socket_clienttasks.front());
                this->socket_clienttasks.pop();
                lock.unlock();
                socket_client_task(std::move(task));
            }            
            else if (this->mqtt_clienttasks.size() > 0)
            {
                auto task = std::move(this->mqtt_clienttasks.front());
                this->mqtt_clienttasks.pop();
                lock.unlock();
                mqtt_client_task(std::move(task));
            } 
            else
            {
                lock.unlock();
                if (this->isstop)
                {
                    break;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}
asio::awaitable<void> client_context::fastcgi_client_task(std::shared_ptr<fastcgi> clientpeer)
{

#ifdef DEBUG
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string tempthread = oss.str();
    DEBUG_LOG("fastcgi_client_task:%s", tempthread.c_str());
#endif

    if (clientpeer->host.size() > 0)
    {
        co_await clientpeer->async_send();
    }
    co_return;
}

void client_context::http_client_task(std::shared_ptr<client> clientpeer)
{
#ifdef DEBUG
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string tempthread = oss.str();
    DEBUG_LOG("http_client_task:%s", tempthread.c_str());
#endif
    if (clientpeer->run_task_fun !=nullptr)
    {
        clientpeer->run_task_fun(clientpeer->shared_from_this());
    }
    return;
}

void client_context::websocket_client_task(std::shared_ptr<websocket_client> wspeer)
{
#ifdef DEBUG
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string tempthread = oss.str();
    DEBUG_LOG("websocket_client_task:%s", tempthread.c_str());
#endif
    if (wspeer->run_task_fun !=nullptr)
    {
        wspeer->run_task_fun(wspeer->shared_from_this());
    }
    return;
}

void client_context::socket_client_task(std::shared_ptr<socket_client> wspeer)
{
#ifdef DEBUG
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string tempthread = oss.str();
    DEBUG_LOG("socket_client:%s", tempthread.c_str());
#endif
    if (wspeer->run_task_fun !=nullptr)
    {
        wspeer->run_task_fun(wspeer->shared_from_this());
    }
    return;
}

void client_context::mqtt_client_task(std::shared_ptr<mqtt_client> cli)
{
#ifdef DEBUG
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    DEBUG_LOG("mqtt_client_task:%s", oss.str().c_str());
#endif
    if (cli->async_run_task_fun != nullptr)
    {
        auto self = cli;
        co_spawn(*this->ioc, [self]{ return self->async_run_task_fun(self); }, asio::detached);
    }
    else if (cli->run_task_fun != nullptr)
    {
        cli->run_task_fun(cli->shared_from_this());
    }
}

asio::io_context &client_context::get_ctx()
{
    return *ioc;
}

void client_context::stop()
{
    isstop = true;
    condition.notify_all();
    timeout_condition.notify_all();
    // ioc->stop();
}

}// namespace http
