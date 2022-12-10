#ifndef __HTTP2_SOCKET_H
#define __HTTP2_SOCKET_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp>
#include <asio/write.hpp>
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

using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
using asio::ip::tcp;
namespace this_coro = asio::this_coro;

namespace fs = std::filesystem;

#if defined(ASIO_ENABLE_HANDLER_TRACKING)
#define use_awaitable \
  asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::redirect_error;
using asio::use_awaitable;
using asio::ip::tcp;

struct httpsocket_t
{
  httpsocket_t(asio::ip::tcp::socket sock){
        _socket.emplace_back(std::move(sock));
  }
  httpsocket_t(asio::ssl::stream<asio::ip::tcp::socket> sslsock){
        _sslsocket.emplace_back(std::move(sslsock));
  }
  std::list<asio::ip::tcp::socket> _socket;
  std::list<asio::ssl::stream<asio::ip::tcp::socket>> _sslsocket;
};


#endif