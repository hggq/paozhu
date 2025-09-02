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
    try
    {
        while (!this->isstop)
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(lock,
                                 [&, this]
                                 { return this->isstop || mythread_info->stop || !this->clienttasks.empty(); });

            if (mythread_info->stop)
            {
                break;
            }

            if (this->isstop && this->clienttasks.empty())
                break;

            if (this->clienttasks.empty())
                continue;

            auto task = std::move(this->clienttasks.front());
            this->clienttasks.pop();
            lock.unlock();

            mythread_info->begin = time((time_t *)NULL);
            livethreadcount += 1;
            mythread_info->busy = true;

            if (task->linktype == 0)
            {
                this->http_clientrun(std::move(task), mythread_info);
            }
            else if (task->linktype == 3)
            {
                this->http_websocketsrun(std::move(task), mythread_info);
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
    }

    condition.notify_one();
    return false;
}

//
void ThreadPool::http_clientrun(std::shared_ptr<httppeer> peer, std::shared_ptr<threadinfo_t> mythread_info)
{
    try
    {
        //std::set<std::string> method_alone;
        DEBUG_LOG("pool in");
        std::string regmethold_path;
        std::string sitecontent;

        server_loaclvar &static_server_var = get_server_global_var();
        serverconfig &sysconfigpath        = getserversysconfig();

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
                unsigned int offsetnum = peer->client_ip.size();
                if (offsetnum < 61)
                {
                    memcpy(mythread_info->ip, peer->client_ip.data(), offsetnum);
                    mythread_info->ip[offsetnum] = 0x00;
                }
            }
        }
        DEBUG_LOG("begin method");

        bool is_use_alias_domain = false;

        if (sysconfigpath.sitehostinfos[peer->host_index].alias_domain.size() > 0)
        {
            is_use_alias_domain = true;
        }
        std::map<std::string, std::map<std::string, regmethold_t>>::iterator domain_method_map_iter          = _domain_regmethod_table.end();
        std::map<std::string, std::map<std::string, std::vector<std::string>>>::iterator domain_url_map_iter = _domain_regurlpath_table.end();
        if (is_use_alias_domain)
        {
            domain_method_map_iter = _domain_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].alias_domain);
            domain_url_map_iter    = _domain_regurlpath_table.find(sysconfigpath.sitehostinfos[peer->host_index].alias_domain);
        }
        else
        {
            domain_method_map_iter = _domain_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].mainhost);
            domain_url_map_iter    = _domain_regurlpath_table.find(sysconfigpath.sitehostinfos[peer->host_index].mainhost);
        }

        if (peer->pathinfos.size() > 0)
        {
            unsigned int pathinfos_size = peer->pathinfos.size();
            for (unsigned int i = pathinfos_size; i > 0; i--)
            {
                regmethold_path.clear();
                for (unsigned int j = 0; j < i; j++)
                {
                    if (j > 0)
                    {
                        regmethold_path.push_back('/');
                    }
                    regmethold_path.append(get_filename(peer->pathinfos[j]));
                }

                if (domain_method_map_iter != _domain_regmethod_table.end())
                {
                    if (domain_method_map_iter->second.contains(regmethold_path))//!= _http_regmethod_table.end()
                    {
                        if (pathinfos_size != i)
                        {
                            if (domain_url_map_iter != _domain_regurlpath_table.end() && domain_url_map_iter->second.contains(regmethold_path))//!= _http_regurlpath_table.end()
                            {
                                for (unsigned int m = i; m < pathinfos_size; m++)
                                {
                                    if (domain_url_map_iter->second[regmethold_path].size() > m)
                                    {
                                        if (domain_url_map_iter->second[regmethold_path][m].size() > 0)
                                        {
                                            peer->get[domain_url_map_iter->second[regmethold_path][m]] = peer->pathinfos[m];
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                }

                if (_http_regmethod_table.contains(regmethold_path))//!= _http_regmethod_table.end()
                {
                    if (pathinfos_size != i)
                    {
                        if (_http_regurlpath_table.contains(regmethold_path))//!= _http_regurlpath_table.end()
                        {
                            for (unsigned int m = i; m < pathinfos_size; m++)
                            {
                                if (_http_regurlpath_table[regmethold_path].size() > m)
                                {
                                    if (_http_regurlpath_table[regmethold_path][m].size() > 0)
                                    {
                                        peer->get[_http_regurlpath_table[regmethold_path][m]] = peer->pathinfos[m];
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
                else
                {
                    regmethold_path.clear();
                }
            }
        }

        if (regmethold_path.empty())
        {
            if (peer->pathinfos.size() > 0)
            {
                if (domain_method_map_iter != _domain_regmethod_table.end())
                {
                    if (domain_method_map_iter->second.contains("404"))//!= _http_regmethod_table.end()
                    {
                        regmethold_path = "404";
                    }
                }
            }
            if (regmethold_path.empty())
            {
                regmethold_path = "home";
            }
        }
        DEBUG_LOG("http_regmethod pre in");

        if (sysconfigpath.sitehostinfos[peer->host_index].is_method_pre)
        {
            if (sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists.size() > 0 && sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists.size() < 16)
            {
                for (unsigned char jk = 0; jk < sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists.size(); jk++)
                {
                    if (sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists[jk].size() > 0 && _http_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists[jk]) != _http_regmethod_table.end())
                    {
                        sitecontent = _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists[jk]].regfun(peer);
                        if (sitecontent.size() == 4 && str_casecmp(sitecontent, "exit"))
                        {
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
                            return;
                        }
                    }
                }
            }
        }

        sitecontent.clear();
        unsigned char method_type = 0;

        if (domain_method_map_iter != _domain_regmethod_table.end())
        {
            auto method_iter = domain_method_map_iter->second.find(regmethold_path);
            if (method_iter != domain_method_map_iter->second.end())
            {
                DEBUG_LOG("http domain regmethod main in");
                method_type = 10;
                for (int i = 0; i < 30; i++)
                {
                    if (method_iter->second.pre != nullptr)
                    {
                        sitecontent = method_iter->second.pre(peer);
                        if (sitecontent.size() == 2 && str_casecmp(sitecontent, "ok"))
                        {
                            sitecontent = method_iter->second.regfun(peer);
                            if (sitecontent.size() == 1 && sitecontent[0] == 'T')
                            {
                                auto method_loop_iter = domain_method_map_iter->second.find("frametasks_timeloop");
                                if (method_loop_iter != domain_method_map_iter->second.end())// != _http_regmethod_table.end()
                                {
                                    sitecontent = method_loop_iter->second.regfun(peer);
                                }
                                sitecontent.clear();
                            }
                        }
                        else
                        {
                            if (sitecontent.empty())
                            {
                                regmethold_path.clear();
                                break;
                            }
                            if (str_casecmp(regmethold_path, sitecontent))
                            {
                                sitecontent = method_iter->second.regfun(peer);
                            }
                            else
                            {
                                peer->push_flow(regmethold_path);// record not execute method
                                auto method_loop_iter = domain_method_map_iter->second.find(sitecontent);
                                if (method_loop_iter != domain_method_map_iter->second.end())//_http_regmethod_table.find(sitecontent) == _http_regmethod_table.end()
                                {
                                    sitecontent = method_loop_iter->second.regfun(peer);
                                }
                                else
                                {
                                    //sitecontent.clear();
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        sitecontent = method_iter->second.regfun(peer);
                    }

                    if (sitecontent.empty())
                    {
                        regmethold_path.clear();
                        break;
                    }
                    if (sitecontent.size() == 4 && str_casecmp(sitecontent, "exit"))
                    {
                        sitecontent.clear();
                        regmethold_path.clear();
                        break;
                    }

                    method_iter = domain_method_map_iter->second.find(sitecontent);
                    if (method_iter == domain_method_map_iter->second.end())// == _http_regmethod_table.end()
                    {
                        //sitecontent.clear();
                        regmethold_path.clear();
                        break;
                    }
                    regmethold_path = sitecontent;
                }
            }
            if (!sitecontent.empty())
            {
                regmethold_path = sitecontent;
                method_type     = 1;
            }
        }

        if (method_type > 9)
        {
        }
        else if (auto method_iter = _http_regmethod_table.find(regmethold_path); method_iter != _http_regmethod_table.end())
        {
            DEBUG_LOG("http_regmethod main in");
            for (int i = 0; i < 30; i++)
            {
                if (method_iter->second.pre != nullptr)
                {
                    sitecontent = method_iter->second.pre(peer);
                    if (sitecontent.size() == 2 && str_casecmp(sitecontent, "ok"))
                    {
                        sitecontent = method_iter->second.regfun(peer);
                        if (sitecontent.size() == 1 && sitecontent[0] == 'T')
                        {
                            auto method_loop_iter = _http_regmethod_table.find("frametasks_timeloop");
                            if (method_loop_iter != _http_regmethod_table.end())// != _http_regmethod_table.end()
                            {
                                sitecontent = method_loop_iter->second.regfun(peer);
                            }
                            sitecontent.clear();
                        }
                    }
                    else
                    {
                        if (sitecontent.empty())
                        {
                            break;
                        }
                        if (str_casecmp(regmethold_path, sitecontent))
                        {
                            sitecontent = method_iter->second.regfun(peer);
                        }
                        else
                        {
                            peer->push_flow(regmethold_path);// record not execute method
                            auto method_loop_iter = _http_regmethod_table.find(sitecontent);
                            if (method_loop_iter != _http_regmethod_table.end())//_http_regmethod_table.find(sitecontent) == _http_regmethod_table.end()
                            {
                                sitecontent = method_loop_iter->second.regfun(peer);
                            }
                            else
                            {
                                sitecontent.clear();
                            }
                        }
                    }
                }
                else
                {
                    sitecontent = method_iter->second.regfun(peer);
                }

                if (sitecontent.empty())
                {
                    break;
                }
                if (sitecontent.size() == 4 && str_casecmp(sitecontent, "exit"))
                {
                    sitecontent.clear();
                    break;
                }

                method_iter = _http_regmethod_table.find(sitecontent);
                if (method_iter == _http_regmethod_table.end())// == _http_regmethod_table.end()
                {
                    break;
                }
                regmethold_path = sitecontent;
            }
        }
        else if (method_type > 0)
        {
        }
        else
        {
            make_404_content(peer);
        }
        DEBUG_LOG("action method after");
        if (sysconfigpath.sitehostinfos[peer->host_index].is_method_after)
        {
            if (sysconfigpath.sitehostinfos[peer->host_index].action_after_lists.size() > 0 && sysconfigpath.sitehostinfos[peer->host_index].action_after_lists.size() < 16)
            {
                for (unsigned char jk = 0; jk < sysconfigpath.sitehostinfos[peer->host_index].action_after_lists.size(); jk++)
                {
                    if (sysconfigpath.sitehostinfos[peer->host_index].action_after_lists[jk].size() > 0 && _http_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].action_after_lists[jk]) != _http_regmethod_table.end())
                    {
                        _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].action_after_lists[jk]].regfun(peer);
                    }
                }
            }
        }
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
}
//
void ThreadPool::http_websocketsrun(std::shared_ptr<httppeer> peer, std::shared_ptr<threadinfo_t> mythread_info)
{
    try
    {
        DEBUG_LOG("websockets pool");
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
                unsigned int offsetnum = peer->client_ip.size();
                if (offsetnum < 61)
                {
                    memcpy(mythread_info->ip, peer->client_ip.data(), offsetnum);
                    mythread_info->ip[offsetnum] = 0x00;
                }
            }
        }

        if (peer->ws->isfile)
        {
            peer->websockets->onfiles(peer->ws->filename);
        }
        else
        {
            peer->websockets->onmessage(peer->ws->indata);
        }
        peer->ws->filename.clear();
        peer->ws->indata.clear();
    }
    catch (std::exception &e)
    {
    }
    catch (...)
    {
    }
}
void ThreadPool::timetasks_run(std::shared_ptr<httppeer> peer, std::shared_ptr<threadinfo_t> mythread_info)
{
    try
    {
        DEBUG_LOG("timetasks_run pool");
        std::string regmethold_path;
        regmethold_path                    = get_filename(peer->pathinfos[0]);
        regmethold_path                    = str2safepath((const char *)&regmethold_path[0], regmethold_path.size());
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

        if (_http_regmethod_table.find(regmethold_path) != _http_regmethod_table.end())
        {
            std::string sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
        }
    }
    catch (std::exception &e)
    {
    }
    catch (...)
    {
    }
}

}// namespace http
