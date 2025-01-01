#ifndef __HTTP_CLIENT_SESSION_H
#define __HTTP_CLIENT_SESSION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif// defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
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
#include <string_view>
#include <atomic>

#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>

#ifndef _MSC_VER
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#endif

#ifndef WIN32
#include <sys/wait.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include <array>
#include <iostream>
#include <ctime>
#include <map>
#include <map>
#include <thread>
#include <mutex>
#include <filesystem>
#include <future>
#include <functional>
#include <stdexcept>

#include <zlib.h>
#include "sendqueue.h"
#include "http_header.h"
#include "http_socket.h"
#include "http2_frame.h"
#include "cookie.h"
#include "clientdatacache.h"

namespace http
{

class client_session : public std::enable_shared_from_this<client_session>
{
  public:
    client_session();
    ~client_session();
    bool send_data(const std::string &msg);
    bool send_data(const unsigned char *, unsigned int);

    void http2_send_data(std::string_view msg);

    void http2_send_data_loop();

    bool isopensocket();
    std::shared_ptr<client_session> get_ptr();
    std::string getremoteip();
    unsigned int getremoteport();

    std::string getlocalip();
    unsigned int getlocalport();

    bool send_enddata(unsigned int);
    asio::awaitable<void> co_send_enddata(unsigned int);
    asio::awaitable<void> co_send_setting();

    bool send_setting();
    bool send_switch101();
    bool send_goway();
    bool send_ping();
    bool send_recv_setting();
    void send_window_update(unsigned int, unsigned int streamid = 0);
    void recv_window_update(unsigned int, unsigned int streamid = 0);
    void stop();
    asio::awaitable<void> co_send_goway();
    asio::awaitable<void> loopwriter(const unsigned char *buffer, unsigned int buffersize);
    asio::awaitable<void> send_writer(const std::string &msg);
    asio::awaitable<void> send_writer(const unsigned char *, unsigned int);
    asio::awaitable<void> co_send_writer(const std::string &msg);
    asio::awaitable<void> co_send_writer(const unsigned char *, unsigned int);
    asio::awaitable<bool> read_some(unsigned int& readnum,std::string &log_item);

    asio::awaitable<void> http2_send_writer(std::string_view msg);
    void http2_pool_send_data(std::string_view msg);

    asio::awaitable<void> http2_send_data_loop_co();
    void clsoesend(asio::io_context &);

  public:
    // client_data_cache_back cache_back_obj;
    unsigned char *_cache_data=nullptr;

    // unsigned char _cache_data[4096];
    unsigned int _cache_size  = 0;
    unsigned int _write_size  = 0;
    std::atomic_uint time_limit  = 0;
    std::atomic_bool sendtype = false;

    bool isssl     = false;
    bool isgoway   = false;
    bool isclose   = false;
    bool sendother = false;
    //asio::error_code ec;

    unsigned char httpv   = 0;
    unsigned int recvtype = 0;

    std::unique_ptr<asio::ip::tcp::socket> socket;
    std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket;

    std::list<sendqueue_t *> send_queue_list;

    std::string server_ip;
    std::string client_ip;
    unsigned int client_port=0;
    unsigned int server_port=0;


    std::atomic<unsigned long long> window_update_num=0;
    std::mutex pop_user_handleer_mutex;
    std::list<asio::detail::awaitable_handler<asio::any_io_executor, size_t>> user_code_handler_call;

    std::queue<std::string> setting_lists;
    std::string other_msg;
};
}// namespace http

#endif
