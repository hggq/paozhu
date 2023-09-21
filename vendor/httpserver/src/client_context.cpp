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
#include <asio/spawn.hpp>
#include <asio/ssl.hpp>

#include "client_context.h"
#include "datetime.h"
#include "httpclient.h"
#include "terminal_color.h"

namespace http
{
client_context &get_client_context_obj()
{
    static client_context instance;
    return instance;
}
client_context::~client_context()
{
    ioc.stop();
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
}
void client_context::add_http_task(std::shared_ptr<client> tempc)
{
    clienttasks.emplace(tempc);
    condition.notify_one();
}
void client_context::time_out_loop()
{
    using namespace std::chrono;
    using dsec                      = duration<double>;
    auto invFpsLimit                = duration_cast<system_clock::duration>(dsec{1. / 0.25});
    auto m_BeginFrame               = system_clock::now();
    auto m_EndFrame                 = m_BeginFrame + invFpsLimit;
    unsigned frame_count_per_second = 0;
    auto prev_time_in_seconds       = time_point_cast<seconds>(m_BeginFrame);

    for (;;)
    {
        if (this->timeout_lists.empty())
        {
            std::unique_lock<std::mutex> lock(this->timeout_mutex);
            this->timeout_condition.wait(
                lock,
                [this]
                { return this->stop || !this->timeout_lists.empty(); });
        }

        auto time_in_seconds = time_point_cast<seconds>(system_clock::now());
        ++frame_count_per_second;
        if (time_in_seconds > prev_time_in_seconds)
        {
            DEBUG_LOG("------time loop------");
            frame_count_per_second = 0;
            prev_time_in_seconds   = time_in_seconds;

            unsigned int nowtimeid = timeid();
            for (auto iter = timeout_lists.begin(); iter != timeout_lists.end();)
            {
                std::shared_ptr<client> peer = iter->lock();
                try
                {
                    if (peer)
                    {
                        DEBUG_LOG("time out:%d", (nowtimeid - peer->timeout_end));
                        if (peer->timeout_end < nowtimeid)
                        {
                            if (peer->isssl)
                            {
                                if (peer->sslsock->lowest_layer().is_open())
                                {
                                    peer->sslsock->shutdown();
                                }
                            }
                            else
                            {
                                if (peer->sock->is_open())
                                {
                                    peer->sock->close();
                                }
                            }
                            //peer.reset();
                            timeout_lists.erase(iter++);
                        }
                        else
                        {
                            ++iter;
                        }
                    }
                    else
                    {
                        DEBUG_LOG(" peer empty");
                        timeout_lists.erase(iter++);
                    }
                }
                catch (...)
                {
                    DEBUG_LOG("catch");
                    timeout_lists.erase(iter++);
                }
            }
        }

        std::this_thread::sleep_until(m_EndFrame);
        m_BeginFrame = m_EndFrame;
        m_EndFrame   = m_BeginFrame + invFpsLimit;
        if (stop)
        {
            break;
        }
    }
}
void client_context::run()
{
    worker = std::unique_ptr<asio::io_context::work>(new asio::io_context::work(ioc));
    // 创建thread
    for (unsigned int i = 0; i < thread_size; i++)
    {
        threads.emplace_back([this]()
                             {
                                 std::ostringstream oss;
                                 oss << std::this_thread::get_id();
                                 std::string tempthread = oss.str();
                                 DEBUG_LOG("frame thread:%s", tempthread.c_str());
                                 this->ioc.run(); });
    }
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
                                 { return this->stop || !this->clienttasks.empty(); });
            if (this->stop && this->clienttasks.empty())
                break;
            if (this->clienttasks.empty())
                continue;

            auto task = std::move(this->clienttasks.front());
            this->clienttasks.pop();
            lock.unlock();
            if (task->linktype == 0)
            {
                co_spawn(this->ioc, http_client_task(std::move(task)), asio::detached);
            }
            else if (task->linktype == 1)
            {
                co_spawn(this->ioc, websocket_client_task(std::move(task)), asio::detached);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}
asio::awaitable<void> client_context::http_client_task(std::shared_ptr<client> clientpeer)
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string tempthread = oss.str();
    DEBUG_LOG("http_client_task:%s", tempthread.c_str());
    if (clientpeer->host.size() > 0)
    {
        co_await clientpeer->co_send();
    }
    co_return;
}

asio::awaitable<void> client_context::websocket_client_task(std::shared_ptr<client> clientpeer)
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string tempthread = oss.str();
    DEBUG_LOG("websocket_client_task:%s", tempthread.c_str());
    if (clientpeer->host.size() > 0)
    {
        co_await clientpeer->co_send();
    }
    co_return;
}

}// namespace http