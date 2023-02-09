#include <cstdio>
#include <cstddef>
#include <stdio.h>

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <array>
#include <set>
#include <memory>

#include <ctime>
#include <map>
#include <string>
#include <sys/time.h>
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

#include <boost/dll/import.hpp>
#include <boost/function.hpp>
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
#include "loadmodule.h"

#include "debug_log.h"
#include "server_localvar.h"
namespace http
{

  std::string ThreadPool::printthreads(bool is_onlineout)
  {
    std::string temp_thread, temp_str;
    std::ostringstream oss;
    std::unique_lock<std::mutex> lck(livemtx);
    for (auto iter = threadlist.begin(); iter != threadlist.end(); iter++)
    {
      oss.str("");
      oss << iter->first << " isbusy:" << iter->second.busy << " ip:" << (iter->second.ip) << " url:" << iter->second.url;
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

  unsigned int ThreadPool::getpoolthreadnum() { return threadlist.size(); }

  bool ThreadPool::live_end(std::thread::id id)
  {

    auto iter = threadlist.find(id);
    if (iter != threadlist.end())
    {
      std::unique_lock<std::mutex> lck(livemtx);
      unsigned long long temp = time((time_t *)NULL);
      threadlist[id].end = temp;
      return true;
    }
    else
    {
      return false;
    }
  }
  bool ThreadPool::live_add(std::thread::id id)
  {
    unsigned long long temp = time((time_t *)NULL);
    std::unique_lock<std::mutex> lck(livemtx);
    threadlist[id].begin = temp;
    return true;
  }

  void ThreadPool::threadloop(int index)
  {
    std::thread::id thread_id = std::this_thread::get_id();
    while (!this->stop)
    {

      std::unique_lock<std::mutex> lock(this->queue_mutex);
      this->condition.wait(lock, [this, thread_id]
                           { return this->stop || this->threadlist[thread_id].stop || !this->clienttasks.empty(); });
      if (this->stop && this->clienttasks.empty())
        break; //

      if (this->threadlist[thread_id].stop)
      {
        break;
      }

      if (this->clienttasks.empty())
        continue;

      auto task = std::move(this->clienttasks.front());
      this->clienttasks.pop();
      lock.unlock();

      live_add(thread_id);
      livethreadcount += 1;
      this->threadlist[thread_id].busy = true;

      if (task->linktype == 0)
      {
        this->http_clientrun(task);
      }
      else if (task->linktype == 3)
      {
        this->http_websocketsrun(task);
      }

      livethreadcount -= 1;
      this->threadlist[thread_id].busy = false;
      live_end(thread_id);
    }

    this->threadlist[thread_id].close = true;
  }
  bool ThreadPool::fixthread()
  {

    unsigned int tempcount = threadlist.size();
    if (tempcount < 128)
    {
      return false;
    }
    if (tempcount < (mixthreads.load() + 10))
    {
      return false;
    }

    {

      std::unique_lock<std::mutex> lock(queue_mutex);
      for (auto &iter : threadlist)
      {
        if (iter.second.busy == false)
        {
          iter.second.stop = true;
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

    std::unique_lock<std::mutex> lock(queue_mutex);
    for (auto iter = threadlist.begin(); iter != threadlist.end();)
    {
      if (iter->second.close)
      {
        if (iter->second.thread.joinable())
        {
          iter->second.thread.join();
          threadlist.erase(iter++);
          pooltotalnum -= 1;
        }
        else
        {
          iter++;
        }
      }
      else
      {
        iter++;
      }
    }
    lock.unlock();

    return true;
  }

  bool ThreadPool::addthread(size_t threads)
  {

    if (threadlist.size() > 2048)
    {
      return false;
    }

    for (size_t i = 0; i < threads; ++i)
    {
      struct threadinfo_t tinfo;
      // tinfo.thread = std::thread(
      //     std::bind(&ThreadPool::threadloop, this, pooltotalnum.load()));
      tinfo.thread = std::thread(&ThreadPool::threadloop, this, pooltotalnum.load());
      std::thread::id temp = tinfo.thread.get_id();
      tinfo.id = temp;
      threadlist[tinfo.id] = std::move(tinfo);
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
    mixthreads.store(32);
    for (size_t i = 0; i < threads; ++i)
    {

      struct threadinfo_t tinfo;
      tinfo.thread = std::thread(
          std::bind(&ThreadPool::threadloop, this, pooltotalnum.load()));
      tinfo.id = tinfo.thread.get_id();
      threadlist[tinfo.id] = std::move(tinfo);
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

    for (auto &worker : threadlist)
    {
      if (worker.second.thread.joinable())
      {
        worker.second.thread.join();
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
  void ThreadPool::http_clientrun(std::shared_ptr<httppeer> peer)
  {
    try
    {
      std::set<std::string> method_alone;
      std::thread::id thread_id = std::this_thread::get_id();
      DEBUG_LOG("pool in");
      std::string regmethold_path;

      server_loaclvar &static_server_var = get_server_global_var();

      if (static_server_var.show_visit_info == true)
      {

        unsigned int offsetnum = 0;
        for (; offsetnum < peer->host.size(); offsetnum++)
        {
          threadlist[thread_id].url[offsetnum] = peer->host[offsetnum];
          if (offsetnum > 60)
          {
            break;
          }
        }
        for (unsigned int j = 0; j < peer->url.size(); j++)
        {
          threadlist[thread_id].url[offsetnum] = peer->url[j];
          offsetnum++;
          if (offsetnum > 63)
          {
            break;
          }
        }
        threadlist[thread_id].url[offsetnum] = 0x00;

        {
          unsigned int offsetnum = peer->client_ip.size();
          if (offsetnum < 61)
          {
            memcpy(threadlist[thread_id].ip, peer->client_ip.data(), offsetnum);
            threadlist[thread_id].ip[offsetnum] = 0x00;
          }
        }
      }

      if (peer->pathinfos.size() > 1)
      {

        if (_http_regmethod_table.find(peer->pathinfos[0]) != _http_regmethod_table.end())
        {
          regmethold_path = get_filename(peer->pathinfos[0]);
        }
        else
        {
          regmethold_path = peer->pathinfos[0] + "/" + get_filename(peer->pathinfos[1]);
        }
      }
      else
      {
        regmethold_path = get_filename(peer->pathinfos[0]);
      }

      if (_http_regmethod_table.find(regmethold_path) != _http_regmethod_table.end())
      {
        std::string sitecontent;
        for (int i = 0; i < 6; i++)
        {
          if (i > 0 && _http_regmethod_table.find(regmethold_path) == _http_regmethod_table.end())
          {
            break;
          }
          if (_http_regmethod_table[regmethold_path].pre != nullptr)
          {
            sitecontent = _http_regmethod_table[regmethold_path].pre(peer);
            if (sitecontent.size() == 2 && strcasecmp(sitecontent.c_str(), "ok") == 0)
            {
              method_alone.emplace(regmethold_path);
              sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
            }
            else
            {
              peer->push_path_method(regmethold_path);
            }
          }
          else
          {
            method_alone.emplace(regmethold_path);
            sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
          }

          if (sitecontent.empty())
          {
            break;
          }

          regmethold_path = sitecontent;
          if (method_alone.contains(regmethold_path))
          {
            break;
          }
        }
      }
      else
      {
        serverconfig &sysconfigpath = getserversysconfig();

        std::string moduleso, sopath;
        regmethold_path = get_filename(peer->pathinfos[0]);
        regmethold_path = str2safepath((const char *)&regmethold_path[0], regmethold_path.size());

        peer->pathinfos[0] = regmethold_path;

        if (sysconfigpath.map_value.find(peer->host) != sysconfigpath.map_value.end())
        {
          if (sysconfigpath.map_value[peer->host].find("controlsopath") != sysconfigpath.map_value[peer->host].end())
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
        moduleso.append(peer->pathinfos[0]);
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

              regmethold_path = str2safemethold((const char *)&peer->pathinfos[1][0], peer->pathinfos[1].size());
              peer->pathinfos[1] = regmethold_path;
              if (regmethold_path[0] == 'i' && strcasecmp(regmethold_path.c_str(), "index") == 0)
              {
                regmethold_path = "home";
              }
            }
            else
            {
              regmethold_path = "home";
            }
            auto sitemodloadis = loadcontrol(moduleso, regmethold_path);
            std::string sitecontent = sitemodloadis(peer);
            peer->isso = false;

            if (sitecontent.size() > 2 && _http_regmethod_table.find(sitecontent) != _http_regmethod_table.end())
            {

              if (_http_regmethod_table[sitecontent].pre != nullptr)
              {
                sitecontent = _http_regmethod_table[sitecontent].pre(peer);
                if (strcasecmp(sitecontent.c_str(), "ok") == 0)
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
                      peer->isso = true;
                      sitemodloadis = loadcontrol(moduleso, regmethold_path);
                      sitecontent = sitemodloadis(peer);
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
      }
      if (peer->loopresults.size() > 0)
      {
        try
        {
          peer->looprunpromise.set_value(1);
        }
        catch (std::exception &e)
        {
          return;
        }
      }
    }
    catch (std::exception &e)
    {
      if (peer->loopresults.size() > 0)
      {
        if (peer->loopresults.front().valid())
        {
          peer->looprunpromise.set_exception(std::current_exception());
        }
      }
    }
    catch (...)
    {
      if (peer->loopresults.size() > 0)
      {
        if (peer->loopresults.front().valid())
        {
          peer->looprunpromise.set_exception(std::current_exception());
        }
      }
    }
  }
  //
  void ThreadPool::http_websocketsrun(std::shared_ptr<httppeer> peer)
  {
    try
    {
      DEBUG_LOG("websockets pool");
      std::thread::id thread_id = std::this_thread::get_id();
      unsigned int offsetnum = 0;
      server_loaclvar &static_server_var = get_server_global_var();

      if (static_server_var.show_visit_info == true)
      {
        for (; offsetnum < peer->host.size(); offsetnum++)
        {
          threadlist[thread_id].url[offsetnum] = peer->host[offsetnum];
          if (offsetnum > 60)
          {
            break;
          }
        }
        for (unsigned int j = 0; j < peer->url.size(); j++)
        {
          threadlist[thread_id].url[offsetnum] = peer->url[j];
          offsetnum++;
          if (offsetnum > 63)
          {
            break;
          }
        }
        threadlist[thread_id].url[offsetnum] = 0x00;

        {
          unsigned int offsetnum = peer->client_ip.size();
          if (offsetnum < 61)
          {
            memcpy(threadlist[thread_id].ip, peer->client_ip.data(), offsetnum);
            threadlist[thread_id].ip[offsetnum] = 0x00;
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

}