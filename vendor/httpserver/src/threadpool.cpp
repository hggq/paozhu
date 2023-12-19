#include <cstdio>
#include <cstddef>
#include <stdio.h>

// #include <unistd.h>
// #include <errno.h>
// #include <stdlib.h>
// #include <signal.h>
// #include <setjmp.h>
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

    for (unsigned int i = 0; i < thread_arrays.size(); i++)
    {
        oss.str("");
        oss << thread_arrays[i].id << " isbusy:" << thread_arrays[i].busy << " ip:" << (thread_arrays[i].ip)
            << " url:" << thread_arrays[i].url;
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
#ifdef DEBUG
    INFO("-------------");
#endif
    return temp_str;
}

unsigned int ThreadPool::getpoolthreadnum() { return thread_arrays.size(); }

void ThreadPool::threadloop(int index)
{
    while (!this->stop)
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        this->condition.wait(lock,
                             [this, index]
                             { return this->stop || this->thread_arrays[index].stop || !this->clienttasks.empty(); });

        if (this->stop && this->clienttasks.empty())
            break;

        if (this->thread_arrays[index].stop)
        {
            break;
        }

        if (this->clienttasks.empty())
            continue;

        auto task = std::move(this->clienttasks.front());
        this->clienttasks.pop();
        lock.unlock();

        this->thread_arrays[index].begin = time((time_t *)NULL);
        livethreadcount += 1;
        this->thread_arrays[index].busy = true;

        if (task->linktype == 0)
        {
            this->http_clientrun(task, index);
        }
        else if (task->linktype == 3)
        {
            this->http_websocketsrun(task, index);
        }
        else if (task->linktype == 7)
        {
            this->timetasks_run(task, index);
        }
        livethreadcount -= 1;
        this->thread_arrays[index].busy = false;
        this->thread_arrays[index].end  = time((time_t *)NULL);
    }

    this->thread_arrays[index].close = true;
}
bool ThreadPool::fixthread()
{
    // unsigned int tempcount = thread_arrays.size();
    // if (tempcount < 32)
    // {
    //     return false;
    // }
    // if (tempcount < (mixthreads.load() + 10))
    // {
    //     return false;
    // }
    unsigned int tempcount = 0;

    for (unsigned int i = 0; i < thread_arrays.size(); i++)
    {
        if (thread_arrays[i].close == false)
        {
            tempcount++;
        }
    }
    if (tempcount < mixthreads.load())
    {
        return false;
    }
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        for (unsigned int i = 0; i < thread_arrays.size(); i++)
        {
            if (thread_arrays[i].busy == false)
            {
                thread_arrays[i].stop = true;
                tempcount--;
            }
            if (tempcount <= mixthreads.load())
            {
                break;
            }
        }
        lock.unlock();
    }

    condition.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::unique_lock<std::mutex> lock(queue_mutex);
    for (unsigned int i = 0; i < thread_arrays.size(); i++)
    {
        if (thread_arrays[i].close)
        {
            if (thread_arrays[i].thread.joinable())
            {
                thread_arrays[i].thread.join();
                thread_arrays[i].close = true;
                pooltotalnum -= 1;
            }
        }
    }
    lock.unlock();
    return true;
}

bool ThreadPool::addthread(size_t threads)
{

    if (thread_arrays.size() > 1023)
    {
        return false;
    }

    for (size_t i = 0; i < threads; ++i)
    {
        unsigned int index_num = 0;
        for (; index_num < thread_arrays.size(); index_num++)
        {
            if (thread_arrays[index_num].close == true)
            {
                break;
            }
        }
        if (index_num == thread_arrays.size())
        {
            threadinfo_t tinfo;
            thread_arrays.push_back(std::move(tinfo));
        }
        thread_arrays[index_num].thread = std::thread(&ThreadPool::threadloop, this, index_num);
        thread_arrays[index_num].id     = thread_arrays[index_num].thread.get_id();
        thread_arrays[index_num].close  = false;
        pooltotalnum++;
    }
    return true;
}

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    isclose_add = true;
    pooltotalnum.store(0);
    livethreadcount.store(0);
    mixthreads.store(16);
    for (size_t i = 0; i < threads; ++i)
    {
        struct threadinfo_t tinfo;
        unsigned int index_num = 0;
        for (; index_num < thread_arrays.size(); index_num++)
        {
            if (thread_arrays[index_num].close == true)
            {
                break;
            }
        }
        if (index_num == thread_arrays.size())
        {
            thread_arrays.push_back(std::move(tinfo));
        }
        thread_arrays[index_num].thread = std::thread(&ThreadPool::threadloop, this, index_num);
        thread_arrays[index_num].id     = thread_arrays[index_num].thread.get_id();
        thread_arrays[index_num].close  = false;
        pooltotalnum++;
    }
    isclose_add = false;
}

// the destructor joins all threads
ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();

    for (unsigned int i = 0; i < thread_arrays.size(); i++)
    {
        if (thread_arrays[i].thread.joinable())
        {
            thread_arrays[i].thread.join();
        }
    }
}
//
bool ThreadPool::addclient(std::shared_ptr<httppeer> peer)
{
    if (isclose_add)
    {
        return false;
    }
    if (!stop)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        clienttasks.emplace(peer);
    }

    condition.notify_one();
    return false;
}

//
void ThreadPool::http_clientrun(std::shared_ptr<httppeer> peer, unsigned int id_index)
{
    try
    {
        //std::set<std::string> method_alone;
        DEBUG_LOG("pool in");
        std::string regmethold_path;
        bool isfindpath = false;

        server_loaclvar &static_server_var = get_server_global_var();
        serverconfig &sysconfigpath        = getserversysconfig();

        if (static_server_var.show_visit_info == true)
        {

            unsigned int offsetnum = 0;
            for (; offsetnum < peer->host.size(); offsetnum++)
            {
                thread_arrays[id_index].url[offsetnum] = peer->host[offsetnum];
                if (offsetnum > 60)
                {
                    break;
                }
            }
            for (unsigned int j = 0; j < peer->url.size(); j++)
            {
                thread_arrays[id_index].url[offsetnum] = peer->url[j];
                offsetnum++;
                if (offsetnum > 63)
                {
                    break;
                }
            }
            thread_arrays[id_index].url[offsetnum] = 0x00;

            {
                unsigned int offsetnum = peer->client_ip.size();
                if (offsetnum < 61)
                {
                    memcpy(thread_arrays[id_index].ip, peer->client_ip.data(), offsetnum);
                    thread_arrays[id_index].ip[offsetnum] = 0x00;
                }
            }
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
                if (_http_regmethod_table.find(regmethold_path) != _http_regmethod_table.end())
                {
                    if (pathinfos_size != i)
                    {
                        if (_http_regurlpath_table.find(regmethold_path) != _http_regurlpath_table.end())
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
                    isfindpath = true;
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
            regmethold_path = "home";
            if (_http_regmethod_table.contains(regmethold_path))// != _http_regmethod_table.end()
            {
                isfindpath = true;
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
                        std::string re_str = _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists[jk]].regfun(peer);
                        if (re_str.size() > 0)
                        {
                            if (re_str.size() > 3 && re_str[0] == 'e' && re_str[1] == 'x' && re_str[2] == 'i' && re_str[3] == 't')
                            {
                                //auto ex = asio::get_associated_executor(peer->user_code_handler_call.front());
                                // asio::dispatch(ex,
                                //                [handler = std::move(peer->user_code_handler_call.front())]() mutable -> void
                                //                {
                                //                    /////////////
                                //                    handler(1);
                                //                    //////////
                                //                });

                                asio::dispatch(*io_context,
                                               [handler = std::move(peer->user_code_handler_call.front())]() mutable -> void
                                               {
                                                   handler(1);
                                               });
                                peer->user_code_handler_call.pop_front();
                                return;
                            }
                        }
                    }
                }
            }
        }

        if (isfindpath)
        {
            DEBUG_LOG("http_regmethod main in");
            std::string sitecontent;
            for (int i = 0; i < 30; i++)
            {
                if (i > 0 && !_http_regmethod_table.contains(regmethold_path))// == _http_regmethod_table.end()
                {
                    break;
                }
                if (_http_regmethod_table[regmethold_path].pre != nullptr)
                {
                    // sitecontent = regmethold_path;
                    // sitecontent.append("pre");
                    // if (method_alone.contains(sitecontent))
                    // {
                    //     break;
                    // }
                    sitecontent = _http_regmethod_table[regmethold_path].pre(peer);

                    if (sitecontent.size() == 2 && str_casecmp(sitecontent, "ok"))
                    {
                        //method_alone.emplace(regmethold_path);
                        sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
                        if (sitecontent.size() == 1 && sitecontent[0] == 'T')
                        {
                            sitecontent = "frametasks_timeloop";
                            if (_http_regmethod_table.contains(sitecontent))// != _http_regmethod_table.end()
                            {
                                sitecontent = _http_regmethod_table[sitecontent].regfun(peer);
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
                        if (regmethold_path.size() != sitecontent.size() || regmethold_path != sitecontent)
                        {
                            peer->push_path_method(regmethold_path);// record not execute method
                            // if (method_alone.contains(sitecontent))
                            // {
                            //     break;
                            // }
                            if (i > 0 && !_http_regmethod_table.contains(sitecontent))//_http_regmethod_table.find(sitecontent) == _http_regmethod_table.end()
                            {
                                //method_alone.emplace(sitecontent);
                                sitecontent = _http_regmethod_table[sitecontent].regfun(peer);

                                // again check same in regmethold_path
                                //
                                //    a -> b -> a
                                //
                                if (regmethold_path == sitecontent)
                                {
                                    //method_alone.emplace(regmethold_path);
                                    sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
                                }
                            }
                        }
                        else
                        {
                            // or self
                            sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
                        }
                    }
                    // regmethold_path.append("pre");
                    // method_alone.emplace(regmethold_path);
                }
                else
                {
                    //method_alone.emplace(regmethold_path);
                    sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
                }

                if (sitecontent.empty())
                {
                    break;
                }

                regmethold_path = sitecontent;
                // if (method_alone.contains(regmethold_path))
                // {
                //     break;
                // }
            }
        }
        else
        {
#ifdef ENABLE_BOOST

            std::string moduleso, sopath;
            if (peer->pathinfos.size() > 0)
            {
                regmethold_path = get_filename(peer->pathinfos[0]);
                regmethold_path = str2safepath((const char *)&regmethold_path[0], regmethold_path.size());

                peer->pathinfos[0] = regmethold_path;
            }
            else
            {
                regmethold_path = "";
            }

            if (sysconfigpath.map_value.find(peer->host) != sysconfigpath.map_value.end())
            {
                if (sysconfigpath.map_value[peer->host].find("controlsopath") !=
                    sysconfigpath.map_value[peer->host].end())
                {
                    moduleso = sysconfigpath.map_value[peer->host]["controlsopath"];
                }
            }
            if (moduleso.empty())
            {
                moduleso = sysconfigpath.map_value["default"]["controlsopath"];
            }
            if (moduleso.size() > 0 && moduleso.back() != '/')
            {
                moduleso.push_back('/');
            }

            sopath = moduleso;
            if (peer->pathinfos.size() > 0)
            {
                moduleso.append(peer->pathinfos[0]);
            }
            moduleso.append(".so");
            struct stat modso;
            DEBUG_LOG("so:%s", moduleso.c_str());
            if (stat(moduleso.c_str(), &modso) == 0)
            {
                if (modso.st_mode & S_IFREG)
                {

                    peer->isso = true;
                    if (peer->pathinfos.size() > 1)
                    {

                        regmethold_path =
                            str2safemethold((const char *)&peer->pathinfos[1][0], peer->pathinfos[1].size());
                        peer->pathinfos[1] = regmethold_path;
                        if (regmethold_path[0] == 'i' && str_casecmp(regmethold_path, "index"))
                        {
                            regmethold_path = "home";
                        }
                    }
                    else
                    {
                        regmethold_path = "home";
                    }
                    auto sitemodloadis      = loadcontrol(moduleso, regmethold_path);
                    std::string sitecontent = sitemodloadis(peer);
                    peer->isso              = false;

                    if (sitecontent.size() > 2 &&
                        _http_regmethod_table.find(sitecontent) != _http_regmethod_table.end())
                    {

                        if (_http_regmethod_table[sitecontent].pre != nullptr)
                        {
                            sitecontent = _http_regmethod_table[sitecontent].pre(peer);
                            if (str_casecmp(sitecontent, "ok"))
                            {
                                sitecontent = _http_regmethod_table[sitecontent].regfun(peer);
                            }
                        }
                        else
                        {
                            sitecontent = _http_regmethod_table[sitecontent].regfun(peer);
                        }
                    }
                    else if (sitecontent.size() > 2)
                    {
                        std::string filename;
                        unsigned int i = 0;
                        regmethold_path.clear();
                        for (; i < sitecontent.size(); i++)
                        {
                            if (sitecontent[i] == '/')
                            {
                                i++;
                                break;
                            }
                            filename.push_back(sitecontent[i]);
                        }
                        if (filename.size() > 0)
                        {
                            for (; i < sitecontent.size(); i++)
                            {
                                if (sitecontent[i] == '/')
                                {
                                    i++;
                                    break;
                                }
                                regmethold_path.push_back(sitecontent[i]);
                            }
                            if (regmethold_path.size() > 0)
                            {
                                moduleso = sopath;
                                moduleso.append(filename);
                                moduleso.append(".so");
                                memset(&modso, 0, sizeof(modso));
                                if (stat(moduleso.c_str(), &modso) == 0)
                                {
                                    if (modso.st_mode & S_IFREG)
                                    {
                                        peer->isso    = true;
                                        sitemodloadis = loadcontrol(moduleso, regmethold_path);
                                        sitecontent   = sitemodloadis(peer);
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    make_404_content(peer);
                }
            }
            else
            {
                make_404_content(peer);
            }
#endif
#ifndef ENABLE_BOOST
            make_404_content(peer);
#endif
        }

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
        asio::dispatch(*io_context,
                       [handler = std::move(peer->user_code_handler_call.front())]() mutable -> void
                       {
                           /////////////
                           handler(1);
                           //////////
                       });
        peer->user_code_handler_call.pop_front();
        DEBUG_LOG("leave pool");
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("catch exception");
        if (peer->user_code_handler_call.size() > 0)
        {
            // auto ex = asio::get_associated_executor(peer->user_code_handler_call.front());
            // asio::dispatch(ex,
            //                [handler = std::move(peer->user_code_handler_call.front())]() mutable -> void
            //                { handler(1); });
            // peer->user_code_handler_call.pop_front();
            asio::dispatch(*io_context,
                           [handler = std::move(peer->user_code_handler_call.front())]() mutable -> void
                           {
                               /////////////
                               handler(1);
                               //////////
                           });
            peer->user_code_handler_call.pop_front();
        }
    }
    catch (...)
    {
        DEBUG_LOG("catch ... ");
        if (peer->user_code_handler_call.size() > 0)
        {
            asio::dispatch(*io_context,
                           [handler = std::move(peer->user_code_handler_call.front())]() mutable -> void
                           {
                               /////////////
                               handler(1);
                               //////////
                           });
            peer->user_code_handler_call.pop_front();
        }
    }
}
//
void ThreadPool::http_websocketsrun(std::shared_ptr<httppeer> peer, unsigned int id_index)
{
    try
    {
        DEBUG_LOG("websockets pool");
        unsigned int offsetnum             = 0;
        server_loaclvar &static_server_var = get_server_global_var();

        if (static_server_var.show_visit_info == true)
        {
            for (; offsetnum < peer->host.size(); offsetnum++)
            {
                thread_arrays[id_index].url[offsetnum] = peer->host[offsetnum];
                if (offsetnum > 60)
                {
                    break;
                }
            }
            for (unsigned int j = 0; j < peer->url.size(); j++)
            {
                thread_arrays[id_index].url[offsetnum] = peer->url[j];
                offsetnum++;
                if (offsetnum > 63)
                {
                    break;
                }
            }
            thread_arrays[id_index].url[offsetnum] = 0x00;

            {
                unsigned int offsetnum = peer->client_ip.size();
                if (offsetnum < 61)
                {
                    memcpy(thread_arrays[id_index].ip, peer->client_ip.data(), offsetnum);
                    thread_arrays[id_index].ip[offsetnum] = 0x00;
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
void ThreadPool::timetasks_run(std::shared_ptr<httppeer> peer, unsigned int id_index)
{
    try
    {
        DEBUG_LOG("timetasks_run pool");
        std::string regmethold_path;
        regmethold_path                    = get_filename(peer->pathinfos[0]);
        regmethold_path                    = str2safepath((const char *)&regmethold_path[0], regmethold_path.size());
        server_loaclvar &static_server_var = get_server_global_var();

        if (static_server_var.show_visit_info == true)
        {
            unsigned int offsetnum = 0;
            for (unsigned int j = 0; j < regmethold_path.size(); j++)
            {
                thread_arrays[id_index].url[offsetnum] = regmethold_path[j];
                offsetnum++;
                if (offsetnum > 63)
                {
                    break;
                }
            }
            for (unsigned int j = 0; j < peer->url.size(); j++)
            {
                thread_arrays[id_index].url[offsetnum] = peer->url[j];
                offsetnum++;
                if (offsetnum > 63)
                {
                    break;
                }
            }
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