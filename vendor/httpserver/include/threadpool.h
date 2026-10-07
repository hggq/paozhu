#ifndef FRAME_THREADPOOLS_H
#define FRAME_THREADPOOLS_H

#include <cstdio>
#include <cstddef>
#include <stdio.h>

#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <ctime>
#include <iostream>
#include <string>
#include <asio.hpp>

#include <thread>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <list>
#include <string>

#include <atomic>
#include <queue>
#include <memory>
#include <map>
#include <thread>
#include <mutex>

// #ifndef _WIN32
// #include <sys/socket.h>
// #include <sys/wait.h>
// #endif

#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "httppeer.h"
#include "websockets.h"

namespace http
{

struct threadinfo_t
{
    unsigned int index = 0;
    std::thread::id id;
    std::thread thread;
    bool stop                = false;
    bool busy                = false;
    bool close               = false;
    unsigned int timelimit   = 0;// 0为不限制
    unsigned long long begin = 0;
    unsigned long long end   = 0;
    char ip[65]              = {0};
    char url[65]             = {0};
};

// conn_tasks 的一个任务：静态 tag + 业务闭包。
// 故意不与裸 std::function 同形 —— tag 只能跟着任务走，取货线程才把它写进
// 自己的诊断字段（mythread_info->url）；连接类任务没有 URL，否则诊断页对它们永远是空的。
struct conn_task_t
{
    const char *tag = nullptr;
    std::function<void()> fn;
};

class ThreadPool
{
  public:
    ThreadPool(size_t);

    void threadloop(std::shared_ptr<threadinfo_t>);
    bool addthread(size_t);
    bool addclient(std::shared_ptr<httppeer>);
    bool add_sync_task(std::function<void()> fn);
    // lane B：连接类任务（ws 数据消息）投完就走，调用方不 await。tag 必须是静态字符串常量。
    bool add_conn_task(const char *tag, std::function<void()> fn);

    // 整链同步跑的老通路，源文件里 #if 0 备查；链上每一步现在走 add_sync_task。
    // std::string http_clientrun(std::shared_ptr<httppeer>, std::shared_ptr<threadinfo_t> mythread_info);
    void timetasks_run(std::shared_ptr<httppeer>, std::shared_ptr<threadinfo_t> mythread_info);
    void run_conn_task(conn_task_t &&task, std::shared_ptr<threadinfo_t> mythread_info);
    bool fixthread();
    unsigned int getpoolthreadnum();
    std::string printthreads(bool);
    unsigned int getlivenum() { return livethreadcount.load(); };
    unsigned int gettasknum() { return clienttasks.size(); };
    unsigned int getmixthreads() { return mixthreads.load(); };
    void stop()
    {
        isstop = true;
        condition.notify_all();
    }

    // 测试钩子：下次 add_sync_task 被一次性拒绝。路由框架用它来触发 503 通路，
    // exchange 自动复位，不遗留状态影响后续请求。
    void force_reject_sync_once() { test_force_reject_sync.store(true); }
    // 自己那把，形状同上，与 HTTP 的 503 通路互不干扰。
    void force_reject_conn_once() { test_force_reject_conn.store(true); }
    // 投递被拒（池在关或测试钩子）的任务数；被丢掉的那一条在这里可见。
    unsigned int getconndropped() { return conn_task_dropped.load(); }
    // lane A 的同款计数：拒单只回一个 false，调用方分不清"钩子跑了但回绝"和"钩子根本没跑"，
    // 这条累计数就是用来区分这两者的。
    unsigned int getsyncdropped() { return sync_task_dropped.load(); };

    ~ThreadPool();

  public:
    asio::io_context *io_context = nullptr;
    std::string error_message;

  private:
    bool isstop;
    bool isclose_add = true;
    // 测试钩子：被 exchange(true) 消费后自动复位
    std::atomic<bool> test_force_reject_sync{false};
    std::atomic<bool> test_force_reject_conn{false};
    // 投递被拒的累计数（关池窗口 + 测试钩子）
    std::atomic<unsigned int> conn_task_dropped{0};
    std::atomic<unsigned int> sync_task_dropped{0};
    unsigned int cpu_threads = 2;
    std::queue<std::shared_ptr<httppeer>> clienttasks;
    std::queue<std::function<void()>> sync_tasks;
    std::queue<conn_task_t> conn_tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;

    std::atomic<unsigned int> mixthreads;
    std::atomic<unsigned int> livethreadcount;

    std::list<std::shared_ptr<threadinfo_t>> thread_arrays;
};

}// namespace http
#endif
