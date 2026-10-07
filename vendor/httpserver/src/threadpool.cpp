#include <cstdio>
#include <cstddef>

#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <array>
#include <set>
#include <memory>
#include <ctime>
#include <map>
#include <string>
#include <atomic>
#include <queue>
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <stack>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>

#ifdef ENABLE_BOOST
#include <boost/dll/import.hpp>
#include <boost/function.hpp>
#endif
// #include "threadlocalvariable.h"

#include "threadpool.h"
#include "client_session.h"
#include "httppeer.h"
#include "router.h"
#include "terminal_color.h"
#include "func.h"
#include "sendqueue.h"
#include "http2_parse.h"
#include "http_mime.h"
#include "http2_define.h"
#include "http2_huffman.h"
#include "serverconfig.h"
#include "directory_fun.h"
#include "https_brotli.h"
#include "gzip.h"

#ifdef ENABLE_BOOST
#include "loadmodule.h"
#endif

#include "debug_log.h"
#include "server_localvar.h"
#include "websockets.h"

namespace http
{

std::string ThreadPool::printthreads(bool is_onlineout)
{
    std::string temp_thread, temp_str;
    std::ostringstream oss;
    if (isclose_add)
    {
        return temp_str;
    }
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    for (auto iter = thread_arrays.begin(); iter != thread_arrays.end(); iter++)
    {
        oss.str("");
        oss << (*iter)->id << " isbusy:" << (*iter)->busy << " ip:" << ((*iter)->ip)
            << " url:" << (*iter)->url;
        temp_thread = oss.str();
#ifdef DEBUG
        INFO("[INFO  ] %s", temp_thread.c_str());
#endif
        if (is_onlineout)
        {
            temp_str.append(temp_thread);
            temp_str.append("<br/>");
        }
    }
    lock.unlock();
#ifdef DEBUG
    INFO("-------------");
#endif
    return temp_str;
}

unsigned int ThreadPool::getpoolthreadnum() { return thread_arrays.size(); }

void ThreadPool::threadloop(std::shared_ptr<threadinfo_t> mythread_info)
{
    // 采样计数：每线程一个栈上局部量，不共享也不原子 —— 它只决定"这一拍要不要
    // 跨线程精确无意义，加原子反而把每拍变成一次 contended RMW。
    unsigned int lane_b_sample = 0;
    try
    {
        while (!this->isstop)
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(lock,
                                 [&, this]
                                 { return this->isstop || mythread_info->stop || !this->clienttasks.empty() || !this->conn_tasks.empty() || !this->sync_tasks.empty(); });

            if (mythread_info->stop || this->isstop)
            {
                break;
            }

            // — 优先级 1：sync_tasks（协程路径派来的 sync handler）—
            // HTTP 永远插队赢：lane A 有货时每 10 拍（n % 10 == 1，n 从 0 起数所以第一次检查
            // 落在第 2 拍）才检查一次 lane B；命中且 B 真有货才让路。
            // 注意不能写成"先采样、再统一判断"的一条 if：那样 lane A 空、lane B 有货但这一拍
            // 没命中时，谓词仍为真却什么都没取，线程会立刻返回并空转（condition 白醒）。
            if (!this->sync_tasks.empty())
            {
                if ((lane_b_sample++ % 10) == 1 && !this->conn_tasks.empty())
                {
                    auto btask = std::move(this->conn_tasks.front());
                    this->conn_tasks.pop();
                    lock.unlock();

                    this->run_conn_task(std::move(btask), mythread_info);
                    continue;
                }

                auto fn = std::move(this->sync_tasks.front());
                this->sync_tasks.pop();
                lock.unlock();

                mythread_info->begin = time((time_t *)NULL);
                livethreadcount += 1;
                mythread_info->busy = true;

                // 业务闭包抛出来的异常在这里收口：静默 catch 会让"钩子挂了"查无实据。
                try
                {
                    fn();
                }
                catch (const std::exception &e)
                {
                    DEBUG_LOG("sync task exception:%s", e.what());
                }
                catch (...)
                {
                    DEBUG_LOG("sync task exception:unknown");
                }

                livethreadcount -= 1;
                mythread_info->busy = false;
                mythread_info->end  = time((time_t *)NULL);
                continue;
            }

            // — lane B：lane A 空着就无条件服务，采样只在 A 有货时生效；
            //    计数器复位，免得 A 空档期的拍数攒进下一轮 HTTP 突发 —
            if (!this->conn_tasks.empty())
            {
                lane_b_sample = 0;
                auto btask    = std::move(this->conn_tasks.front());
                this->conn_tasks.pop();
                lock.unlock();

                this->run_conn_task(std::move(btask), mythread_info);
                continue;
            }

            if (this->clienttasks.empty())
            {
                continue;
            }

            auto task = std::move(this->clienttasks.front());
            this->clienttasks.pop();
            lock.unlock();

            mythread_info->begin = time((time_t *)NULL);
            livethreadcount += 1;
            mythread_info->busy = true;

            // 间隔任务的单飞旗在这里放，不在 timetasks_run() 里放：投递方（每秒拍）只看
            // "这个任务有没有人接手"，接手了就一定会走到这里，跟下面走哪一支、走没走完无关。
            // 放在被调函数里就得每个出口都记得放，而 check_fastcgi_request()、fastcgi 收尾那些
            // 按请求写 linktype=0 的地方，正好能把挂了任务的 peer 送进"没人放"的那一支。
            struct inflight_release
            {
                std::shared_ptr<httppeer> task;
                ~inflight_release() { task->timeloop_inflight.store(false); }
            } flag_guard{task};

            if (task->linktype == 0)
            {
                // 整链同步跑的业务通路已经停用（见本文件 http_clientrun 处的说明），
                // 现在链上每一步走 sync_tasks。真收到 linktype==0 就是有人重新投了
                // 这条已停用的队列，只报不跑，免得任务被静默丢掉。
                DEBUG_LOG("clienttasks linktype 0 has no consumer any more, dropped url:%s",
                          task->url.c_str());
            }
            else if (task->linktype == 7)
            {
                this->timetasks_run(std::move(task), mythread_info);
            }

            livethreadcount -= 1;
            mythread_info->busy = false;
            mythread_info->end  = time((time_t *)NULL);
        }

        mythread_info->close = true;
    }
    catch (const std::exception &e)
    {
        mythread_info->close = true;
        error_message.append(e.what());
    }
}
bool ThreadPool::fixthread()
{
    unsigned int tempcount = 0;
    for (auto iter = thread_arrays.begin(); iter != thread_arrays.end();)
    {
        if ((*iter)->close == false)
        {
            tempcount++;
        }
        iter++;
    }

    if (tempcount <= ((mixthreads.load() + cpu_threads) / 2))
    {
        return false;
    }
    {
        for (auto iter = thread_arrays.begin(); iter != thread_arrays.end();)
        {
            if ((*iter)->busy == false)
            {
                (*iter)->stop = true;
                tempcount--;
            }
            if (tempcount <= mixthreads.load())
            {
                break;
            }
            iter++;
        }
    }
    condition.notify_all();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::unique_lock<std::mutex> lock(queue_mutex);
    for (auto iter = thread_arrays.begin(); iter != thread_arrays.end();)
    {
        if ((*iter)->close)
        {
            if ((*iter)->thread.joinable())
            {
                (*iter)->thread.join();
                thread_arrays.erase(iter++);
                continue;
            }
        }
        ++iter;
    }
    lock.unlock();
    return true;
}

bool ThreadPool::addthread(size_t threads)
{
    unsigned int index_num = thread_arrays.size();

    if (index_num > cpu_threads)
    {
        return false;
    }

    for (size_t i = 0; i < threads; ++i)
    {
        std::shared_ptr<threadinfo_t> tinfo = std::make_shared<threadinfo_t>();
        tinfo->close                        = false;
        tinfo->thread                       = std::thread(&ThreadPool::threadloop, this, tinfo);
        tinfo->id                           = tinfo->thread.get_id();
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        thread_arrays.emplace_back(tinfo);
        lock.unlock();
    }

    return true;
}

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads) : isstop(false)
{
    isclose_add = true;
    cpu_threads = std::thread::hardware_concurrency();
    cpu_threads = cpu_threads * 2 + 2;
    mixthreads.store(cpu_threads);
    cpu_threads = cpu_threads * 2;
    livethreadcount.store(0);

    for (size_t i = 0; i < threads; ++i)
    {
        std::shared_ptr<threadinfo_t> tinfo = std::make_shared<threadinfo_t>();
        tinfo->close                        = false;
        tinfo->thread                       = std::thread(&ThreadPool::threadloop, this, tinfo);
        tinfo->id                           = tinfo->thread.get_id();

        std::unique_lock<std::mutex> lock(this->queue_mutex);
        thread_arrays.emplace_back(tinfo);
        lock.unlock();
    }
    isclose_add = false;
}

// the destructor joins all threads
ThreadPool::~ThreadPool()
{
    isstop = true;
    condition.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (auto iter = thread_arrays.begin(); iter != thread_arrays.end();)
    {
        if ((*iter)->thread.joinable())
        {
            (*iter)->thread.join();
        }
        iter++;
    }
}
//
bool ThreadPool::addclient(std::shared_ptr<httppeer> peer)
{
    if (isclose_add)
    {
        return false;
    }
    if (!isstop)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        clienttasks.emplace(peer);
        condition.notify_one();
        return true;
    }
    return false;
}
bool ThreadPool::add_conn_task(const char *tag, std::function<void()> fn)
{
    // 同一把 isclose_add 闸、同一个 condition；不复用 lane A 的 test_force_reject_sync
    // （那是 HTTP 503 通路专用钩子），lane B 用自己的那把。
    if (isclose_add || isstop)
    {
        conn_task_dropped += 1;
        DEBUG_LOG("conn task dropped, pool closing tag:%s", tag == nullptr ? "" : tag);
        return false;
    }
    if (test_force_reject_conn.exchange(false))
    {
        conn_task_dropped += 1;
        DEBUG_LOG("conn task rejected by test hook tag:%s", tag == nullptr ? "" : tag);
        return false;
    }
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        conn_tasks.emplace(conn_task_t{tag, std::move(fn)});
    }
    condition.notify_one();
    return true;
}

bool ThreadPool::add_sync_task(std::function<void()> fn)
{
    if (isclose_add || isstop)
    {
        sync_task_dropped += 1;
        DEBUG_LOG("sync task dropped, pool closing");
        return false;
    }
    // 测试钩子：exchange 原子地读旧值 + 复位；旧值为 true 时本次拒绝
    if (test_force_reject_sync.exchange(false))
    {
        sync_task_dropped += 1;
        DEBUG_LOG("sync task rejected by test hook");
        return false;
    }
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        sync_tasks.emplace(std::move(fn));
    }
    condition.notify_one();
    return true;
}
//
// 整条链在业务线程同步跑完的老通路（无活喂入口，整块保留备查）。原流程：
//   co_await co_user_task(peer) 把 awaitable handler 挂到 peer->user_code_handler_call，
//   再把 peer 投进 clienttasks；业务线程 pop 到它 → router::resolve(peer) 同步跑完
//   站点 pre / pre / regfun / 跳转 / 站点 after → 取 handler 用 asio::dispatch(*io_context,
//   handler(1)) 唤醒协程。唤醒点写在 io_context 上，所以续体落在 io 池的某个线程，但不在
//   这条会话的 strand 上（同会话的续体与其它 handler 可能重叠）；池 isclose_add/isstop 时
//   addclient 不入队、handler 永不调用，协程永久挂起。
//   取代它的是 router::co_resolve + co_pool_run_step：一个任务只跑一个 sync 步，
//   唤醒 dispatch 回协程自己的执行域（会话 strand），业务线程跑完这一步立刻回队列取下一个任务。
//   同一段里那三个 catch 的 500 正文形状（"Internal Server Error <hr />" + e.what()）与
//   router.cpp 的 fail_500 一致。
#if 0
std::string ThreadPool::http_clientrun(std::shared_ptr<httppeer> peer, std::shared_ptr<threadinfo_t> mythread_info)
{
    std::string sitecontent;
    try
    {
        DEBUG_LOG("pool in");
        server_loaclvar &static_server_var = get_server_global_var();

        if (static_server_var.show_visitinfo == true)
        {
            unsigned int offsetnum = 0;
            if (peer->url.size() > 0)
            {
                offsetnum = peer->url.size();
                if (offsetnum > 63)
                {
                    offsetnum = 63;
                }
                memcpy(mythread_info->url, peer->url.data(), offsetnum);
            }
            mythread_info->url[offsetnum] = 0x00;
            {
                offsetnum = peer->client_ip.size();
                if (offsetnum < 61)
                {
                    memcpy(mythread_info->ip, peer->client_ip.data(), offsetnum);
                    mythread_info->ip[offsetnum] = 0x00;
                }
            }
        }
        DEBUG_LOG("begin method");

        sitecontent = get_router().resolve(peer);

        std::unique_lock<std::mutex> lock(peer->pop_user_handleer_mutex);
        if (peer->user_code_handler_call.size() > 0)
        {
            auto handle = std::move(peer->user_code_handler_call.front());
            peer->user_code_handler_call.pop_front();
            lock.unlock();
            asio::dispatch(*io_context,
                           [handler = std::move(handle)]() mutable -> void
                           {
                               handler(1);
                           });
        }
        else
        {
            lock.unlock();
        }
        DEBUG_LOG("leave pool");
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("catch exception");
        peer->status(500);
        peer->output = "Internal Server Error <hr />";
        peer->output.append(e.what());

        if (peer->user_code_handler_call.size() > 0)
        {
            auto handle = std::move(peer->user_code_handler_call.front());
            peer->user_code_handler_call.pop_front();
            asio::dispatch(*io_context,
                           [handler = std::move(handle)]() mutable -> void
                           {
                               /////////////
                               handler(1);
                               //////////
                           });
        }
    }
    catch (const char *e)
    {
        DEBUG_LOG("catch ... ");
        if (peer->user_code_handler_call.size() > 0)
        {
            peer->status(500);
            peer->output = "Internal Server Error <hr />";
            peer->output.append(e);
            auto handle = std::move(peer->user_code_handler_call.front());
            peer->user_code_handler_call.pop_front();
            asio::dispatch(*io_context,
                           [handler = std::move(handle)]() mutable -> void
                           {
                               /////////////
                               handler(1);
                               //////////
                           });
        }
    }
    catch (...)
    {
        DEBUG_LOG("catch ... ");
        if (peer->user_code_handler_call.size() > 0)
        {
            peer->status(500);
            peer->output = "Internal Server Error";
            auto handle  = std::move(peer->user_code_handler_call.front());
            peer->user_code_handler_call.pop_front();
            asio::dispatch(*io_context,
                           [handler = std::move(handle)]() mutable -> void
                           {
                               /////////////
                               handler(1);
                               //////////
                           });
        }
    }
    return sitecontent;
}
#endif
//
void ThreadPool::run_conn_task(conn_task_t &&task, std::shared_ptr<threadinfo_t> mythread_info)
{
    // 一条 lane B 任务的全部记账都收在这里：线程占位、诊断信息、异常日志。
    // 两个取货点（采样命中 / lane A 空）因此不用重复写尾巴。
    const char *tagtext = (task.tag != nullptr) ? task.tag : "";

    mythread_info->begin = time((time_t *)NULL);
    livethreadcount += 1;
    mythread_info->busy = true;

    try
    {
        server_loaclvar &static_server_var = get_server_global_var();
        if (static_server_var.show_visitinfo == true)
        {
            // 连接类任务没有 URL，线程诊断里的 URL 靠静态 tag 填，否则它永远是空的
            unsigned int offsetnum = strlen(tagtext);
            if (offsetnum > 63)
            {
                offsetnum = 63;
            }
            if (offsetnum > 0)
            {
                memcpy(mythread_info->url, tagtext, offsetnum);
            }
            mythread_info->url[offsetnum] = 0x00;
        }

        task.fn();
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("conn task %s exception:%s", tagtext, e.what());
    }
    catch (...)
    {
        DEBUG_LOG("conn task %s exception:unknown", tagtext);
    }

    livethreadcount -= 1;
    mythread_info->busy = false;
    mythread_info->end  = time((time_t *)NULL);
}
void ThreadPool::timetasks_run(std::shared_ptr<httppeer> peer, std::shared_ptr<threadinfo_t> mythread_info)
{
    // 单飞旗由 worker 在弹出这个任务之后统一放（见本文件取货循环里的 flag_guard），
    // 这里的所有出口（任务名为空的提前 return、两个 catch）都不用管它。
    try
    {
        DEBUG_LOG("timetasks_run pool");
        // 任务名取登记时写下的那一份（peer->timeloop_taskname），不再从 pathinfos 里捞：
        // pathinfos 是解析器每个请求重填的路由段表，keep-alive 上的下一个请求就把任务名换掉了。
        // 也不再过 get_filename()/str2safepath()——那两步只适用于名字来自 URL 的写法，
        // 而这个名字只由业务代码调 add_timeloop_task() 给出，带斜杠也照原样查表。
        if (peer->timeloop_taskname.empty())
        {
            return;
        }
        server_loaclvar &static_server_var = get_server_global_var();

        if (static_server_var.show_visitinfo == true)
        {
            unsigned int offsetnum = 0;
            if (peer->url.size() > 0)
            {
                offsetnum = peer->url.size();
                if (offsetnum > 63)
                {
                    offsetnum = 63;
                }
                memcpy(mythread_info->url, peer->url.data(), offsetnum);
            }
            mythread_info->url[offsetnum] = 0x00;
        }

        get_router().run_timeloop_task(peer, peer->timeloop_taskname);
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("timetasks_run exception:%s", e.what());
    }
    catch (...)
    {
        DEBUG_LOG("timetasks_run unknown exception");
    }
}

}// namespace http
