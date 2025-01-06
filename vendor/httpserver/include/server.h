#pragma once
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>

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
// #include <sys/types.h>
// #include <sys/stat.h>

// #ifndef _MSC_VER
// #include <sys/fcntl.h>
// #include <unistd.h>
// #endif

// #ifndef _WIN32
// #include <sys/wait.h>
// #endif

// #ifdef WIN32
// #define stat _stat
// #endif

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

#include "zlib.h"
#include "terminal_color.h"
#include "http_socket.h"
#include "client_session.h"
#include "http2_parse.h"
#include "threadpool.h"
#include "httppeer.h"
#include "http2_flow.h"
#include "http2_send_queue.h"
// #include "http_domain.h"
// namespace this_coro = asio::this_coro;

namespace http
{

namespace fs = std::filesystem;
class httpserver
{
  public:
    httpserver() {}
    asio::awaitable<void> clientpeerfun(std::shared_ptr<client_session>, bool isssl);
    asio::awaitable<void>
        sslhandshake(std::shared_ptr<client_session>);

    // void http2pool(int threadid);
    asio::awaitable<void> http2_fastcgi(std::shared_ptr<httppeer>);
    asio::awaitable<void> http2loop(std::shared_ptr<httppeer>);
    asio::awaitable<void> http2task(std::shared_ptr<http2parse>);

    asio::awaitable<void> http2_send_content(unsigned int stream_id, std::string &_send_data, const unsigned char *buffer, unsigned int begin_end, bool is_end = false);
    asio::awaitable<void> http2_send_content(unsigned int stream_id, std::string &_send_data, const std::string &_source_data, bool is_end = false);
    asio::awaitable<void> http2_send_content_append(unsigned int stream_id, std::string &_send_data, const unsigned char *buffer, unsigned int begin_end, bool is_end = false);
    asio::awaitable<void> http2_send_content_append(unsigned int stream_id, std::string &_send_data, const std::string &_source_data, bool is_end = false);

    asio::awaitable<void> http2_co_send_file(std::shared_ptr<httppeer> peer);
    asio::awaitable<void> http2_co_send_304(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t>);
    asio::awaitable<void> http2_co_send_compress(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t> send_file_obj);

    bool http2_send_file(std::shared_ptr<httppeer>);
    asio::awaitable<void> http2_send_file_range(std::shared_ptr<httppeer> peer);

    void add_error_lists(const std::string &);
    asio::awaitable<void> http1_fastcgi(std::shared_ptr<httppeer>);
    asio::awaitable<void> http1loop(std::shared_ptr<httppeer>, std::shared_ptr<client_session>);

    asio::awaitable<void> http2_send_sequence_header(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t>);
    asio::awaitable<void> http2_send_sequence(std::shared_ptr<http2_send_data_t>);
    bool http2_loop_send_sequence(std::shared_ptr<http2_send_data_t>);
    void http2_send_queue_loop(unsigned char index_id);
    void websocket_loop(int myid);

    void listeners();
    void listener();

    asio::awaitable<void> http1_send_status_content(std::shared_ptr<httppeer> peer, unsigned int status_code, const std::string &bodycontent);
    asio::awaitable<bool> http1_static_file_authority(std::shared_ptr<httppeer> peer);

    asio::awaitable<void> http2_send_status_content(std::shared_ptr<httppeer> peer, unsigned int status_code, const std::string &bodycontent);
    asio::awaitable<bool> http2_static_file_authority(std::shared_ptr<httppeer> peer);

    asio::awaitable<size_t> co_user_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h = {});
    asio::awaitable<size_t> co_user_fastcgi_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h = {});

    void add_runsocketthread();
    int checkhttp2(std::shared_ptr<client_session> peer_session);
    asio::awaitable<void> http1_send_bad_request(unsigned int, std::shared_ptr<client_session>);
    asio::awaitable<void> http1_send_bad_server(std::shared_ptr<httppeer>, std::shared_ptr<client_session>);

    asio::awaitable<void> http1_send_file_header(std::shared_ptr<httppeer> peer,
                                                 std::shared_ptr<client_session> peer_session,
                                                 std::shared_ptr<http2_send_data_t> sq_ob);
    asio::awaitable<void> http1_co_send_304(std::shared_ptr<httppeer> peer,
                                            std::shared_ptr<client_session> peer_session,
                                            std::shared_ptr<http2_send_data_t> sq_ob);

    asio::awaitable<void> http1_send_file(std::shared_ptr<httppeer> peer,
                                          std::shared_ptr<client_session> peer_session);

    asio::awaitable<void> http1_send_file_range(std::shared_ptr<httppeer> peer,
                                                std::shared_ptr<client_session> peer_session);

    void run(const std::string &);

    void add_nullptrlog(const std::string &logstrb);
    void httpwatch();

    void save_traffic_arrays();
    void stop();
    ~httpserver()
    {
        std::printf("~httpserver\n");
        isstop = true;
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
    std::vector<std::thread> http2_send_data_threads;
    std::list<std::weak_ptr<httppeer>> websockettasks;
    std::list<std::pair<std::size_t, std::shared_ptr<httppeer>>> clientlooptasks;

    std::string traffic_arrays;
    std::queue<httpsocket_t> tasks;

    bool isstop                  = false;
    bool istraffic               = false;
    std::atomic_uint total_count = 0;

    std::atomic_uint total_http2_count = 0;
    std::atomic_uint total_http1_count = 0;

    std::mutex socket_session_lists_mutex;
    std::list<std::weak_ptr<client_session>> socket_session_lists;
    std::list<std::shared_ptr<client_session>> socket_session_wait_clear;

    std::condition_variable send_data_condition;

    std::list<std::shared_ptr<http2_send_data_t>> sent_data_list;

    std::mutex send_data_mutex;
    std::mutex wait_clear_mutex;
    std::list<std::string> access_loglist;
    std::list<std::string> error_loglist;
    std::mutex log_mutex;

    ThreadPool clientrunpool{std::thread::hardware_concurrency() * 2 + 2};

    std::mutex http2_task_mutex;
    std::list<struct http2sendblock_t> http2send_tasks;

    std::mutex websocket_task_mutex;
    std::condition_variable websocketcondition;

    const unsigned char magicstr[24] = {0x50, 0x52, 0x49, 0x20, 0x2A, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2F, 0x32, 0x2E, 0x30, 0x0D, 0x0A, 0x0D, 0x0A, 0x53, 0x4D, 0x0D, 0x0A, 0x0D, 0x0A};
};
httpserver &get_server_app();
void add_server_timetask(std::shared_ptr<httppeer>);
}// namespace http
#endif
