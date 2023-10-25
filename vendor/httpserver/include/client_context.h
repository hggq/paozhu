#ifndef FRAME_CLENT_CONTEXT_H
#define FRAME_CLENT_CONTEXT_H

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
#include "httpclient.h"
#include "fastcgi.h"
namespace http
{

class client_context
{
  public:
    client_context() : time_out_loop_th(std::bind(&client_context::time_out_loop, this)) {}
    void run();
    void time_out_loop();
    void taskloop();
    asio::awaitable<void> fastcgi_client_task(std::shared_ptr<fastcgi>);
    asio::awaitable<void> http_client_task(std::shared_ptr<client>);
    asio::awaitable<void> websocket_client_task(std::shared_ptr<client>);
    void add_http_task(std::shared_ptr<client>);
    void add_fastcgi_task(std::shared_ptr<fastcgi>);
    ~client_context();

  public:
    unsigned int thread_size      = 3;
    unsigned int thread_task_size = 1;
    asio::io_context ioc;
    std::unique_ptr<asio::io_context::work> worker;
    std::vector<std::thread> threads;
    std::vector<std::thread> httptask_th;
    std::thread time_out_loop_th;

    bool stop = false;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::queue<std::shared_ptr<client>> clienttasks;
    std::queue<std::shared_ptr<fastcgi>> cgitasks;

    std::mutex timeout_mutex;
    std::condition_variable timeout_condition;
    std::list<std::weak_ptr<client>> timeout_lists;
};
client_context &get_client_context_obj();

}// namespace http
#endif
