#pragma once
#ifndef HTTP2_DEFINE_H
#define HTTP2_DEFINE_H

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

namespace http
{
    

#define HTTP2_H2_ALPN "\x02h2"
#define HTTP2_H2_16_ALPN "\x05h2-16"
#define HTTP2_H2_14_ALPN "\x05h2-14"
#define HTTP1_NPN "\x08http/1.1"
#define HTTP2_H1_1_ALPN "\x08http/1.1"
#define HTTP2_H1_1 "http/1.1"

    extern std::string PZ_HTTP2_H2_ALPN;
    extern std::string PZ_HTTP2_H2;

    extern std::string PZ_HTTP2_H2_16_ALPN;
    extern std::string PZ_HTTP2_H2_16;

    extern std::string PZ_HTTP2_H2_14_ALPN;
    extern std::string PZ_HTTP2_H2_14;

    extern std::string PZ_HTTP2_H1_1_ALPN;
    extern std::string PZ_HTTP2_H1_1;

    extern void close_file(FILE *fp);

    using FILE_HANDLER = std::unique_ptr<FILE, std::function<void(FILE *)>>;
    // FILE_HANDLER fp(fopen("demo.txt", "r"), close_file);
    using FILE_AUTO = std::unique_ptr<std::FILE, decltype(&std::fclose)>;
    // FILE_AUTO fp(std::fopen("demo.txt", "r"),&std::fclose);
}
#endif