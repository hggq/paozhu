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
#include "str_fun.h"

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

  void ThreadPool::printthreads()
  {
    std::unique_lock<std::mutex> lck(livemtx);
    for (auto iter = threadlist.begin(); iter != threadlist.end(); iter++)
    {
      std::ostringstream oss;
      oss << iter->first << " isbusy:" << iter->second.busy << " ip:" << (iter->second.ip) << " url:" << iter->second.url;
      std::string tempthread = oss.str();
      INFO("[INFO  ] %s", tempthread.c_str());
    }
    INFO("-------------");
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
      else if (task->linktype == 1)
      {
        this->http2loop(task);
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
      tinfo.thread = std::thread(
          std::bind(&ThreadPool::threadloop, this, pooltotalnum.load()));
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
    if (!stop)
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      clienttasks.emplace(peer);
    }

    condition.notify_one();
    return false;
  }

  bool ThreadPool::http2_send_file_range(std::shared_ptr<httppeer> peer)
  {
    std::string _send_header;
    std::string _send_data;
    FILE_AUTO fp(std::fopen(peer->sendfilename.c_str(), "rb"), &std::fclose);
    if (fp.get())
    {

      fseek(fp.get(), 0, SEEK_END);
      long long file_size = ftell(fp.get());
      fseek(fp.get(), 0, SEEK_SET);
      std::string htmlcontent;

      std::string fileexttype;
      std::string mime_value = "text/html; charset=utf-8";
      unsigned short statecode = 200;
      unsigned long long readnum = 0, mustnum = 0;
      peer->compress = 0;

      if (peer->state.rangebegin >= file_size || peer->state.rangeend < 0 || peer->state.rangeend > file_size)
      {
        peer->status(400);
        peer->length(0);
        peer->type(mime_value);
        _send_header = peer->make_http2_header(HTTP2_HEADER_END_STREAM);
        peer->socket_session->send_data(_send_header);
        return false;
      }

      unsigned int filebasesize = peer->sendfilename.size();
      unsigned int filenameoffset = 0;

      if (filebasesize > 0)
      {
        for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
        {
          if (peer->sendfilename[filenameoffset] == '.')
          {
            break;
          }
        }
        filenameoffset += 1;
        for (; filenameoffset < filebasesize; filenameoffset++)
        {
          fileexttype.push_back(peer->sendfilename[filenameoffset]);
        }
      }

      if (fileexttype.size() > 0)
      {
        mime_value = mime_map[fileexttype];
        if (mime_value.empty())
        {
          mime_value = "text/html; charset=utf-8";
        }
      }
      else
      {
        if (file_size > 20480)
        {
          mime_value = "application/octet-stream";
        }
        else
        {
          mime_value = "text/plain";
        }
      }

      mustnum = file_size;
      if (peer->state.rangebegin > 0)
      {
        readnum = peer->state.rangebegin;
        statecode = 206;
      }
      if (peer->state.rangeend > 0)
      {
        mustnum = peer->state.rangeend + 1;
        statecode = 206;
      }

      peer->status(statecode);
      peer->length(mustnum - readnum);
      peer->type(mime_value);

      if (statecode == 206)
      {
        peer->setHeader("content-range", "bytes " + std::to_string(readnum) + "-" + std::to_string(mustnum - 1) + "/" + std::to_string(file_size));
      }
      else
      {
        peer->setHeader("accept-ranges", "bytes");
      }

      peer->setHeader("date", getGmtTime());
      peer->setHeader("last-modified", getGmtTime((unsigned long long)peer->fileinfo.st_mtime));

      std::string etag;
      etag = make_header_etag(file_size, peer->fileinfo.st_mtime + peer->url.size());

      peer->setHeader("etag", etag);

      _send_header = peer->make_http2_header();

      peer->socket_session->send_data(_send_header);

      unsigned int data_send_id = peer->stream_id;
      _send_data.resize(9);
      _send_data[3] = 0x00;
      _send_data[4] = 0x00;
      _send_data[8] = data_send_id & 0xFF;
      data_send_id = data_send_id >> 8;
      _send_data[7] = data_send_id & 0xFF;
      data_send_id = data_send_id >> 8;
      _send_data[6] = data_send_id & 0xFF;
      data_send_id = data_send_id >> 8;
      _send_data[5] = data_send_id & 0x7F;
      data_send_id = 0;

      fseek(fp.get(), readnum, SEEK_SET);
      unsigned int jj = 0;

      sendqueue &send_cache_list = get_sendqueue();
      struct sendqueue_t *send_cache = send_cache_list.get_cache_ptr();
      sendqueue_back unsetcahceback;
      unsetcahceback.setptr(send_cache);

      bool mustwait_window_update = true;
      unsigned long long totalsend_num = 0;
      unsigned long long old_window_update_num = peer->socket_session->window_update_num;
      unsigned long long per_window_update_num = peer->socket_session->window_update_num;
      unsigned int vsize_send = 4096;
      for (unsigned long long m = readnum; m < mustnum;)
      {
        int per_size = 0;

        send_cache->data[3] = 0x00;
        send_cache->data[4] = 0x00;
        data_send_id = peer->stream_id;
        send_cache->data[8] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[7] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[6] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[5] = data_send_id & 0x7F;

        if (m < mustnum)
        {
          memset(&send_cache->data[9], 0x00, 4096);
          data_send_id = vsize_send;
          if ((mustnum - m) < vsize_send)
          {
            data_send_id = mustnum - m;
          }

          per_size = fread(&send_cache->data[9], 1, data_send_id, fp.get());
          m += per_size;
        }
        if (m >= mustnum)
        {
          send_cache->data[4] = 0x01;
        }

        data_send_id = per_size;
        send_cache->data[2] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[1] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[0] = data_send_id & 0xFF;

        if (peer->isclose)
        {
          DEBUG_LOG("---  peer->isclose --------");

          break;
        }
        if (peer->socket_session->isclose)
        {
          DEBUG_LOG("---  peer->socket_session->isclose exit --------");
          return true;
        }

        if (file_size > 10485760)
        {
          send_cache->data_size = per_size + 9;
          send_cache->timeid = peer->stream_id;
          peer->socket_session->send_queue_list.emplace_back(send_cache);

          send_file_promise temppromise;
          temppromise.send_results = temppromise.send_promise.get_future();

          peer->socket_session->peer_promise_list[peer->stream_id] = std::move(temppromise);

          try
          {
            //DEBUG_LOG("---  go to send --------");
            peer->socket_session->sendtype = true;
            peer->socket_session->flush_data();

            int result = peer->socket_session->peer_promise_list[peer->stream_id].send_results.get();
            // 优化时候删除
            //DEBUG_LOG("---  from send back --------");
          }
          catch (const std::exception &e)
          {
            // peer->window_update_results.clear();

            return false;
          }
        }
        else
        {
          if (peer->socket_session->send_data(send_cache->data, per_size + 9))
          {
          }
          else
          {
            LOG_ERROR<<" send_data error "<<LOG_END;
            return false;
          }
        }

        peer->socket_session->window_update_num -= per_size;
        totalsend_num += per_size;
        if (peer->socket_session->window_update_num < 8192)
        {
          DEBUG_LOG("--- wait window_update_num --------");
          std::promise<int> p;
          std::future<int> f{p.get_future()};

          peer->window_update_results.emplace_back(std::move(f));
          peer->window_update_promise = std::move(p);
          peer->window_update_bool = true;
          peer->socket_session->promise_list.emplace_back(peer->stream_id);
          try
          {
            int result = peer->window_update_results.front().get();
            peer->window_update_results.pop_front();
            DEBUG_LOG("--- from window_update_num --------");
            mustwait_window_update = false;
          }
          catch (const std::exception &e)
          {
            // peer->window_update_results.clear();
          }
        }

        if (file_size > 10485760)
        {

          if (mustwait_window_update && totalsend_num < 20971520)
          {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            old_window_update_num = peer->socket_session->window_update_num;
          }
          else
          {
            if (per_window_update_num < peer->socket_session->window_update_num)
            {
              old_window_update_num = peer->socket_session->window_update_num;
            }
            if (old_window_update_num > (peer->socket_session->window_update_num + 2048576))
            {
              vsize_send = 4096;
              std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else if (old_window_update_num > (peer->socket_session->window_update_num + 1048576))
            {
              vsize_send = 4096;
              std::this_thread::sleep_for(std::chrono::milliseconds(3));
            }
            else if (old_window_update_num > (peer->socket_session->window_update_num + 524288))
            {
              vsize_send = 4096;
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else if (old_window_update_num > (peer->socket_session->window_update_num + 262144))
            {
              vsize_send = 6144;
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else
            {
              vsize_send = 8181;
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
          }
        }
        else if (file_size > 1048576)
        {
          if (per_window_update_num < peer->socket_session->window_update_num)
          {
            old_window_update_num = peer->socket_session->window_update_num;
          }
          if (old_window_update_num > (peer->socket_session->window_update_num + 524288))
          {
            vsize_send = 4096;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }
          else if (old_window_update_num > (peer->socket_session->window_update_num + 262144))
          {
            vsize_send = 6144;
            std::this_thread::sleep_for(std::chrono::nanoseconds(512));
          }
          else
          {
            if (vsize_send == 4096)
            {
              std::this_thread::sleep_for(std::chrono::nanoseconds(512));
            }
            vsize_send = 8181;
          }
        }
        per_window_update_num = peer->socket_session->window_update_num;
      }

      return false;
    }
    else
    {
      std::string stfilecom = "<h3>500 Internal Server Error</h3>";
      stfilecom.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
      peer->status(500);
      peer->length(stfilecom.size());
      peer->type("text/html; charset=utf-8");
      _send_header = peer->make_http2_header();
      peer->socket_session->send_data(_send_header);

      http2_send_body(peer, (const unsigned char *)&stfilecom[0], stfilecom.size());
      return false;
    }
    return true;
  }
  bool ThreadPool::http2_send_file(std::shared_ptr<httppeer> peer)
  {
    std::string _send_header;
    std::string _send_data;
    FILE_AUTO fp(std::fopen(peer->sendfilename.c_str(), "rb"), &std::fclose);
    DEBUG_LOG("%s", peer->sendfilename.c_str());
    if (fp.get())
    {
      fseek(fp.get(), 0, SEEK_END);
      long long file_size = ftell(fp.get());
      fseek(fp.get(), 0, SEEK_SET);
      std::string htmlcontent;

      std::string fileexttype;
      std::string mime_value = "text/html; charset=utf-8";
      peer->compress = 0;

      unsigned int filebasesize = peer->sendfilename.size();
      unsigned int filenameoffset = 0;

      if (filebasesize > 0)
      {
        for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
        {
          if (peer->sendfilename[filenameoffset] == '.')
          {
            break;
          }
        }
        filenameoffset += 1;
        for (; filenameoffset < filebasesize; filenameoffset++)
        {
          fileexttype.push_back(peer->sendfilename[filenameoffset]);
        }
      }

      if (file_size < 16877216 && fileexttype.size() > 0 && mime_compress.contains(fileexttype))
      {

        if (peer->state.gzip || peer->state.br)
        {
          htmlcontent.resize(file_size);
          file_size = fread(&htmlcontent[0], 1, file_size, fp.get());
          htmlcontent.resize(file_size);
          std::string tempcompress;

          if (peer->state.br)
          {
            brotli_encode(htmlcontent, tempcompress);
            peer->compress = 2;
            htmlcontent = tempcompress;
          }
          else if (peer->state.gzip)
          {

            if (compress(htmlcontent.data(), htmlcontent.size(), tempcompress, Z_DEFAULT_COMPRESSION) == Z_OK)
            {
              htmlcontent = tempcompress;
              peer->compress = 1;
            }
          }
          file_size = htmlcontent.size();
        }
      }

      if (fileexttype.size() > 0)
      {
        mime_value = mime_map[fileexttype];
        if (mime_value.empty())
        {
          mime_value = "text/html; charset=utf-8";
        }
      }
      else
      {
        if (file_size > 20480)
        {
          mime_value = "application/octet-stream";
        }
        else
        {
          mime_value = "text/plain";
        }
      }

      peer->status(200);
      peer->length(file_size);
      peer->type(mime_value);
      if (peer->compress == 0)
      {
        peer->setHeader("accept-ranges", "bytes");
      }

      peer->setHeader("date", getGmtTime());
      peer->setHeader("last-modified", getGmtTime((unsigned long long)peer->fileinfo.st_mtime));

      peer->setHeader("etag", make_header_etag(file_size, peer->fileinfo.st_mtime + peer->url.size()));

      _send_header = peer->make_http2_header();
      peer->socket_session->send_data(_send_header);

      unsigned int data_send_id = peer->stream_id;
      _send_data.resize(9);
      _send_data[3] = 0x00;
      _send_data[4] = 0x00;
      _send_data[8] = data_send_id & 0xFF;
      data_send_id = data_send_id >> 8;
      _send_data[7] = data_send_id & 0xFF;
      data_send_id = data_send_id >> 8;
      _send_data[6] = data_send_id & 0xFF;
      data_send_id = data_send_id >> 8;
      _send_data[5] = data_send_id & 0x7F;
      data_send_id = 0;
      int jj = 0;

      sendqueue &send_cache_list = get_sendqueue();
      struct sendqueue_t *send_cache = send_cache_list.get_cache_ptr();

      sendqueue_back unsetcahceback;
      unsetcahceback.setptr(send_cache);

      bool mustwait_window_update = true;
      unsigned long long totalsend_num = 0;
      unsigned long long old_window_update_num = peer->socket_session->window_update_num;
      unsigned long long per_window_update_num = peer->socket_session->window_update_num;

      unsigned int vsize_send = 4096;
      for (unsigned long long m = 0; m < file_size;)
      {
        int per_size = 0;

        send_cache->data[3] = 0x00;
        send_cache->data[4] = 0x00;
        data_send_id = peer->stream_id;
        send_cache->data[8] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[7] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[6] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[5] = data_send_id & 0x7F;

        if (peer->compress > 0)
        {
          for (; m < file_size;)
          {

            send_cache->data[9 + per_size] = htmlcontent[m];
            m += 1;
            per_size++;
            if (per_size == vsize_send)
            {
              break;
            }
          }
          send_cache->data[vsize_send + 9] = 0x00;
          if (m >= file_size)
          {
            send_cache->data[4] = 0x01;
          }
        }
        else
        {
          if (m < file_size)
          {
            memset(&send_cache->data[9], 0x00, vsize_send);
            per_size = fread(&send_cache->data[9], 1, vsize_send, fp.get());
            m += per_size;
          }
          if (m >= file_size)
          {
            send_cache->data[4] = 0x01;
          }
        }

        data_send_id = per_size;
        send_cache->data[2] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[1] = data_send_id & 0xFF;
        data_send_id = data_send_id >> 8;
        send_cache->data[0] = data_send_id & 0xFF;

        if (peer->isclose)
        {
          DEBUG_LOG(" peer->isclose  ");
          LOG_ERROR << "peer->isclose " << LOG_END;
          break;
        }

        if (peer->socket_session->isclose)
        {
          DEBUG_LOG(" peer->socket_session->isclose exit ");
          LOG_ERROR << "peer->socket_session->isclose exit " << LOG_END;
          return true;
        }
        if (file_size > 10485760)
        {
          send_cache->data_size = per_size + 9;
          send_cache->timeid = peer->stream_id;
          peer->socket_session->send_queue_list.emplace_back(send_cache);

          send_file_promise temppromise;
          temppromise.send_results = temppromise.send_promise.get_future();
          peer->socket_session->peer_promise_list[peer->stream_id] = std::move(temppromise);
          try
          {
            //DEBUG_LOG("---  to send--------");
            peer->socket_session->sendtype = true;
            peer->socket_session->flush_data();

            int result = peer->socket_session->peer_promise_list[peer->stream_id].send_results.get();
            // 优化时候删除
            //DEBUG_LOG("---  back send--------");
          }
          catch (const std::exception &e)
          {
            break;
          }
        }
        else
        {
          if (peer->socket_session->send_data(send_cache->data, per_size + 9))
          {
          }
          else
          {
            LOG_ERROR << " range error ";
            return false;
          }
        }

        peer->socket_session->window_update_num -= per_size;
        totalsend_num += per_size;
        if (peer->socket_session->window_update_num < 8192)
        {
          std::promise<int> p;
          std::future<int> f{p.get_future()};

          peer->window_update_results.emplace_back(std::move(f));
          peer->window_update_promise = std::move(p);
          peer->window_update_bool = true;
          peer->socket_session->promise_list.emplace_back(peer->stream_id);
          try
          {
            int result = peer->window_update_results.front().get();
            peer->window_update_results.pop_front();
            mustwait_window_update = false;
          }
          catch (const std::exception &e)
          {
            break;
          }
        }

        if (file_size > 10485760)
        {
          if (mustwait_window_update && totalsend_num < 20971520)
          {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
          }
          else
          {
            if (per_window_update_num < peer->socket_session->window_update_num)
            {
              old_window_update_num = peer->socket_session->window_update_num;
            }
            if (old_window_update_num > (peer->socket_session->window_update_num + 2048576))
            {
              vsize_send = 4096;
              std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else if (old_window_update_num > (peer->socket_session->window_update_num + 1048576))
            {
              vsize_send = 4096;
              std::this_thread::sleep_for(std::chrono::milliseconds(3));
            }
            else if (old_window_update_num > (peer->socket_session->window_update_num + 524288))
            {
              vsize_send = 4096;
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else if (old_window_update_num > (peer->socket_session->window_update_num + 262144))
            {
              vsize_send = 6144;
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else
            {
              vsize_send = 8181;
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
          }
        }
        else if (file_size > 1048576)
        {
          if (per_window_update_num < peer->socket_session->window_update_num)
          {
            old_window_update_num = peer->socket_session->window_update_num;
          }
          if (old_window_update_num > (peer->socket_session->window_update_num + 524288))
          {
            vsize_send = 4096;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }
          else if (old_window_update_num > (peer->socket_session->window_update_num + 262144))
          {
            vsize_send = 6144;
            std::this_thread::sleep_for(std::chrono::nanoseconds(512));
          }
          else
          {
            if (vsize_send == 4096)
            {
              std::this_thread::sleep_for(std::chrono::nanoseconds(512));
            }
            vsize_send = 8181;
          }
        }
        per_window_update_num = peer->socket_session->window_update_num;
      }

      return false;
    }
    else
    {
      std::string stfilecom = "<h3>500 Internal Server Error</h3>";
      stfilecom.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
      peer->status(500);
      peer->length(stfilecom.size());
      peer->type("text/html; charset=utf-8");
      _send_header = peer->make_http2_header();
      peer->socket_session->send_data(_send_header);

      http2_send_body(peer, (const unsigned char *)&stfilecom[0], stfilecom.size());
      return false;
    }

    return true;
  }
  bool ThreadPool::http2_send_body(std::shared_ptr<httppeer> peer, const unsigned char *buffer, unsigned int begin_end)
  {
    std::string _send_data;
    unsigned int data_send_id = peer->stream_id;
    _send_data.resize(9);
    _send_data[3] = 0x00;
    _send_data[4] = 0x00;
    _send_data[8] = data_send_id & 0xFF;
    data_send_id = data_send_id >> 8;
    _send_data[7] = data_send_id & 0xFF;
    data_send_id = data_send_id >> 8;
    _send_data[6] = data_send_id & 0xFF;
    data_send_id = data_send_id >> 8;
    _send_data[5] = data_send_id & 0xFF;
    data_send_id = 0;

    for (unsigned long long m = 0; m < begin_end; m += PER_DATA_BLOCK_SIZE)
    {
      unsigned int offset = PER_DATA_BLOCK_SIZE;
      if (offset > begin_end)
      {
        offset = begin_end;
        _send_data[4] = 0x01;
      }
      else
      {
        if ((m + offset) > begin_end)
        {
          offset = begin_end - m;
          _send_data[4] = 0x01;
        }
      }

      // 还剩多少窗口
      if (peer->socket_session->window_update_num - offset <= 0)
      {
        offset = peer->socket_session->window_update_num;
        peer->socket_session->window_update_num = 0;
        m = m - PER_DATA_BLOCK_SIZE + offset;
      }
      else
      {
        peer->socket_session->window_update_num -= offset;
      }
      _send_data.resize(9);
      _send_data.append((char *)&buffer[m], offset);

      data_send_id = offset;
      _send_data[2] = data_send_id & 0xFF;
      data_send_id = data_send_id >> 8;
      _send_data[1] = data_send_id & 0xFF;
      data_send_id = data_send_id >> 8;
      _send_data[0] = data_send_id & 0xFF;

      if (peer->socket_session->isclose)
      {
        return true;
      }
      if (peer->socket_session->send_data(_send_data))
      {
      }
      else
      {
        return false;
      }
      // 结束流
      if (peer->isclose)
      {
        return true;
      }

      if (peer->socket_session->window_update_num < 8192)
      {

        std::promise<int> p;
        std::future<int> f{p.get_future()};

        peer->window_update_results.emplace_back(std::move(f));
        peer->window_update_promise = std::move(p);
        peer->window_update_bool = true;
        peer->socket_session->promise_list.emplace_back(peer->stream_id);
        try
        {
          int result = peer->window_update_results.front().get();
          peer->window_update_results.clear();
        }
        catch (const std::exception &e)
        {
        }
      }

      // peer_session->window_update_num -= ii;
      if (begin_end > 10485760)
      {
        if (m < 35000)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (peer->socket_session->window_update_num < 125535)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      }
    }
    return true;
  }

  void ThreadPool::http2loop(std::shared_ptr<httppeer> peer)
  {
    try
    {

      serverconfig &sysconfigpath = getserversysconfig();

      // peer->linktype = 0;
      // peer->server_ip = peer->socket_session->getlocalip();
      // peer->client_ip = peer->socket_session->getremoteip();
      // peer->client_port = peer->socket_session->getremoteport();
      // peer->server_port = peer->socket_session->getlocalport();

      std::thread::id thread_id = std::this_thread::get_id();
      unsigned int offsetnum = 0;
      for (; offsetnum < peer->host.size(); offsetnum++)
      {
        threadlist[thread_id].url[offsetnum] = peer->host[offsetnum];
        if (offsetnum > 60)
        {
          break;
        }
      }
      for (int j = 0; j < peer->url.size(); j++)
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

      std::string _send_header;
      std::string _send_data;

      DEBUG_LOG("%s", peer->urlpath.c_str());
      DEBUG_LOG("%s", peer->host.c_str());
      DEBUG_LOG("%s", peer->header["user-agent"].c_str());

      peer->sitepath = sysconfigpath.getsitepath(peer->host);

      unsigned char sendtype = peer->getfileinfo();
      if (sendtype == 1)
      {
        if (peer->state.rangebytes)
        {
          http2_send_file_range(peer);
        }
        else
        {
          http2_send_file(peer);
        }
      }
      else if (sendtype == 2 && peer->isshow_directory())
      {
        std::string htmlcontent = displaydirectory(peer->sendfilename, peer->urlpath, peer->get["sort"].as_string(), sysconfigpath.configpath);
        peer->status(200);
        peer->length(htmlcontent.size());
        peer->type("text/html; charset=utf-8");
        _send_header = peer->make_http2_header();

        peer->socket_session->send_data(_send_header);
        http2_send_body(peer, (const unsigned char *)&htmlcontent[0], htmlcontent.size());
      }
      else
      {
        DEBUG_LOG("htttp2 pool in");

        if (peer->cookie.check(COOKIE_SESSION_NAME))
        {
          peer->parse_session();
        }
        peer->status(200);

        peer->type("text/html; charset=utf-8");
        http_clientrun(peer);

        peer->length(peer->output.size());

        if (peer->getStatus() < 100)
        {
          peer->status(200);
        }
        DEBUG_LOG("htttp2 pool out");
        if (!peer->issettype())
        {
          peer->type("text/html; charset=utf-8");
        }
        _send_header = peer->make_http2_header();

        peer->socket_session->send_data(_send_header);
        http2_send_body(peer, (const unsigned char *)&peer->output[0], peer->output.size());
      }
      peer->issend = true;
    }
    catch (std::exception &e)
    {
    }
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

      if (peer->linktype == 0)
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
        for (int j = 0; j < peer->url.size(); j++)
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
          regmethold_path = peer->pathinfos[0];
        }
        else
        {
          regmethold_path = peer->pathinfos[0] + "/" + peer->pathinfos[1];
        }
      }
      else
      {
        regmethold_path = peer->pathinfos[0];
      }

      if (_http_regmethod_table.find(regmethold_path) != _http_regmethod_table.end())
      {
        std::string sitecontent;
        if (_http_regmethod_table[regmethold_path].pre != nullptr)
        {
          sitecontent = _http_regmethod_table[regmethold_path].pre(peer);
          if (strcasecmp(sitecontent.c_str(), "ok") == 0)
          {
            sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
          }
        }
        else
        {
          sitecontent = _http_regmethod_table[regmethold_path].regfun(peer);
        }

        if (sitecontent.size() > 2 && _http_regmethod_table.find(sitecontent) != _http_regmethod_table.end())
        {
          method_alone.emplace(sitecontent);
          // Flow max executed 5 times
          for (int i = 0; i < 5; i++)
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
            if (sitecontent.empty() || sitecontent.size() < 3)
            {
              break;
            }
            if (_http_regmethod_table.find(sitecontent) == _http_regmethod_table.end())
            {
              break;
            }
            if (method_alone.find(sitecontent) != method_alone.end())
            {
              method_alone.emplace(sitecontent);
            }
            else
            {
              break;
            }
          }
        }
      }
      else
      {
        serverconfig &sysconfigpath = getserversysconfig();

        std::string moduleso, sopath;
        regmethold_path = str2safepath((const char *)&peer->pathinfos[0][0], peer->pathinfos[0].size());

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
              int i = 0;
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
  }
  //
  void ThreadPool::http_websocketsrun(std::shared_ptr<httppeer> peer)
  {
    try
    {
      DEBUG_LOG("websockets pool");
       std::thread::id thread_id = std::this_thread::get_id();
      unsigned int offsetnum = 0;
      for (; offsetnum < peer->host.size(); offsetnum++)
      {
        threadlist[thread_id].url[offsetnum] = peer->host[offsetnum];
        if (offsetnum > 60)
        {
          break;
        }
      }
      for (int j = 0; j < peer->url.size(); j++)
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
  }

}