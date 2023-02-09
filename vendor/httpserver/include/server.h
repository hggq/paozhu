#pragma once
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <asio.hpp>
#include <asio/ssl.hpp>
// #include <asio/co_spawn.hpp>
// #include <asio/detached.hpp>
// #include <asio/io_context.hpp>
// #include <asio/ip/tcp.hpp>
// #include <asio/signal_set.hpp>
// #include <asio/write.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <memory>
#include <set>

#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include <array>
#include <iostream>
#include <ctime>
#include <map>
#include <sys/time.h>
#include <map>
#include <thread>
#include <mutex>
#include <filesystem>
#include <future>
#include <functional>
#include <stdexcept>

#include <zlib.h>

#include "terminal_color.h"
#include "http_socket.h"
#include "client_session.h"
#include "http2_parse.h"
#include "threadpool.h"
#include "httppeer.h"
#include "http2_flow.h"
// #include "http2_define.h"
// #include "http_domain.h"
//namespace this_coro = asio::this_coro;
namespace fs = std::filesystem;

namespace http
{

  class httpserver
  {
  public:
    httpserver() {}
    asio::awaitable<void> clientpeerfun(struct httpsocket_t sock_temp, bool isssl, bool httpversion);
    asio::awaitable<void> sslhandshake(asio::ip::tcp::socket socket,asio::ssl::context& context_, unsigned long long temp_domain );
    
    void http2pool(int threadid);
    asio::awaitable<void> http2loop(std::shared_ptr<httppeer>);
    bool http2_send_body(std::shared_ptr<httppeer> peer, const unsigned char *buffer, unsigned int begin_end);
    bool http2_send_file(std::shared_ptr<httppeer>);
    bool http2_send_file_range(std::shared_ptr<httppeer> peer);

    void http2send_filedata(struct http2sendblock_t &http2_ff_send);
    
    void http1loop(unsigned int sig,std::shared_ptr<httppeer>,std::shared_ptr<client_session>);
    void websocket_loop(int myid);
    void listeners();
    void listener();
    void add_runsocketthread();
    int  checkhttp2(std::shared_ptr<client_session> peer_session);
    void http1_send_bad_request(unsigned int,std::shared_ptr<client_session>);
    void http1_send_bad_server(unsigned int,std::shared_ptr<httppeer>,std::shared_ptr<client_session>);
    bool http1_send_body(unsigned int streamid,std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session,const unsigned char *buffer, unsigned int begin_end);
    bool http1_send_file(unsigned int streamid,std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session,const std::string &filename);
    bool http1_send_file_range(unsigned int streamid,std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session,const std::string &filename);
    void run(const std::string &);
    void httpwatch();
    ~httpserver()
    {
      std::printf("~httpserver\n");

      io_context.stop();

      for (unsigned int i = 0; i < runthreads.size(); ++i)
      {
        if (runthreads[i].joinable())
        {
          runthreads[i].join();
        }
      }
    }

  public:
    // httpheader begin
    unsigned char runhands_num = 4;
    asio::io_context io_context{1};
    std::vector<std::thread> runthreads;
    std::vector<std::thread> websocketthreads;
    std::list<std::weak_ptr<httppeer>> websockettasks;
    std::queue<httpsocket_t> tasks;
    // std::queue<std::shared_ptr<client_session>> tasks;
    // std::list<std::shared_ptr<chat_room>> clientlist;
    // std::mutex headqueue_mutex;
    // std::condition_variable headqueue_condition;
    bool stop;
    std::atomic_uint total_count = 0;
    // httpheader end
    // log
    std::set<std::shared_ptr<client_session>> client_sessions;
    std::list<std::string> access_loglist;
    std::list<std::string> error_loglist;
    std::mutex log_mutex;

    ThreadPool clientrunpool{16};

    std::mutex http2_task_mutex;
    std::condition_variable http2condition;
    std::list<struct http2sendblock_t> http2send_tasks;
  
    const unsigned char magicstr[24] = {0x50, 0x52, 0x49, 0x20, 0x2A, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2F, 0x32, 0x2E, 0x30, 0x0D, 0x0A, 0x0D, 0x0A, 0x53, 0x4D, 0x0D, 0x0A, 0x0D, 0x0A};
  };
}
#endif
