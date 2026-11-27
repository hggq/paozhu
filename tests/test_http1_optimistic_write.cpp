#include <array>
#include <chrono>
#include <string>
#include <system_error>

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/redirect_error.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/use_future.hpp>
#include <asio/write.hpp>

#include "client_session.h"
#include "test_framework.hpp"

namespace {

struct socket_pair {
    asio::io_context io;
    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::socket server;
    asio::ip::tcp::socket peer;

    socket_pair()
        : acceptor(io, {asio::ip::address_v4::loopback(), 0}), server(io), peer(io)
    {
        peer.connect(acceptor.local_endpoint());
        acceptor.accept(server);
    }

    ~socket_pair()
    {
        asio::error_code ec;
        server.close(ec);
        peer.close(ec);
    }
};

template <typename T>
T run(asio::io_context &io, asio::awaitable<T> awaitable)
{
    auto result = asio::co_spawn(io, std::move(awaitable), asio::use_future);
    io.run();
    return result.get();
}

} // namespace

TEST_CASE(http1_optimistic_write_limit_and_inline_completion)
{
    socket_pair sockets;
    std::string oversized(http::HTTP1_OPTIMISTIC_WRITE_LIMIT + 1, 'x');

    REQUIRE_EQ(run<std::size_t>(sockets.io, http::http1_optimistic_write(sockets.server, oversized)), 0u);
    REQUIRE(!sockets.server.non_blocking());

    sockets.io.restart();
    std::string payload(http::HTTP1_OPTIMISTIC_WRITE_LIMIT, 'h');
    auto written = run<std::size_t>(sockets.io, http::http1_optimistic_write(sockets.server, payload));
    REQUIRE_EQ(written, payload.size());
    REQUIRE(sockets.server.non_blocking());

    std::string received(payload.size(), '\0');
    asio::error_code ec;
    auto read = asio::read(sockets.peer, asio::buffer(received), ec);
    REQUIRE(!ec);
    REQUIRE_EQ(read, payload.size());
    REQUIRE_EQ(received, payload);
}

TEST_CASE(http1_non_blocking_socket_waits_and_supports_keepalive)
{
    socket_pair sockets;
    std::string first = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok";
    REQUIRE_EQ(run<std::size_t>(sockets.io, http::http1_optimistic_write(sockets.server, first)), first.size());

    std::string first_received(first.size(), '\0');
    asio::error_code ec;
    REQUIRE_EQ(asio::read(sockets.peer, asio::buffer(first_received), ec), first.size());
    REQUIRE(!ec);
    REQUIRE_EQ(first_received, first);

    sockets.io.restart();
    std::array<char, 128> request{};
    std::size_t request_size = 0;
    bool read_done = false;
    asio::error_code read_ec;
    asio::co_spawn(sockets.io,
                   [&]() -> asio::awaitable<void>
                   {
                       request_size = co_await sockets.server.async_read_some(
                           asio::buffer(request), asio::redirect_error(asio::use_awaitable, read_ec));
                       read_done = true;
                   },
                   asio::detached);

    sockets.io.poll();
    REQUIRE(!read_done);
    REQUIRE(!read_ec);

    std::string next_request = "GET /hello HTTP/1.1\r\nHost: localhost\r\n\r\n";
    REQUIRE_EQ(asio::write(sockets.peer, asio::buffer(next_request), ec), next_request.size());
    REQUIRE(!ec);

    sockets.io.restart();
    sockets.io.run();
    REQUIRE(read_done);
    REQUIRE(!read_ec);
    REQUIRE_EQ(request_size, next_request.size());
    REQUIRE_EQ(std::string(request.data(), request_size), next_request);

    sockets.io.restart();
    std::string second = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok";
    REQUIRE_EQ(run<std::size_t>(sockets.io, http::http1_optimistic_write(sockets.server, second)), second.size());
    REQUIRE(sockets.server.non_blocking());

    std::string second_received(second.size(), '\0');
    REQUIRE_EQ(asio::read(sockets.peer, asio::buffer(second_received), ec), second.size());
    REQUIRE(!ec);
    REQUIRE_EQ(second_received, second);
}

TEST_CASE(http1_optimistic_write_drains_backpressure_without_duplicates)
{
    socket_pair sockets;
    sockets.server.set_option(asio::socket_base::send_buffer_size(1024));
    sockets.peer.set_option(asio::socket_base::receive_buffer_size(1024));
    sockets.server.non_blocking(true);

    std::string prefix;
    const std::string padding(4096, 'p');
    asio::error_code ec;
    while (prefix.size() < 1024 * 1024)
    {
        auto written = sockets.server.write_some(asio::buffer(padding), ec);
        if (written > 0)
        {
            prefix.append(padding.data(), written);
        }
        if (ec)
        {
            break;
        }
    }
    REQUIRE(ec == asio::error::would_block || ec == asio::error::try_again);
    REQUIRE(prefix.size() >= 128);

    std::string drained(128, '\0');
    REQUIRE_EQ(asio::read(sockets.peer, asio::buffer(drained), ec), drained.size());
    REQUIRE(!ec);

    std::string payload(4096, 'w');
    bool write_done = false;
    bool write_threw = false;
    std::size_t optimistic_result = 0;
    asio::co_spawn(sockets.io,
                   [&]() -> asio::awaitable<void>
                   {
                       try
                       {
                           optimistic_result = co_await http::http1_optimistic_write(sockets.server, payload);
                       }
                       catch (...)
                       {
                           write_threw = true;
                       }
                       write_done = true;
                   },
                   asio::detached);

    sockets.io.poll();
    REQUIRE(!write_threw);
    if (write_done && optimistic_result == 0)
    {
        asio::co_spawn(sockets.io,
                       [&]() -> asio::awaitable<void>
                       {
                           try
                           {
                               co_await asio::async_write(sockets.server, asio::buffer(payload), asio::use_awaitable);
                           }
                           catch (...)
                           {
                               write_threw = true;
                           }
                           write_done = true;
                       },
                       asio::detached);
    }

    std::string expected = prefix.substr(drained.size()) + payload;
    std::string received(expected.size(), '\0');
    std::size_t read_size = 0;
    bool read_done = false;
    asio::error_code read_ec;
    asio::steady_timer deadline(sockets.io);
    deadline.expires_after(std::chrono::seconds(5));
    deadline.async_wait([&](asio::error_code timer_ec)
    {
        if (!timer_ec)
        {
            sockets.server.close(timer_ec);
            sockets.peer.close(timer_ec);
        }
    });

    asio::co_spawn(sockets.io,
                   [&]() -> asio::awaitable<void>
                   {
                       read_size = co_await asio::async_read(
                           sockets.peer, asio::buffer(received), asio::redirect_error(asio::use_awaitable, read_ec));
                       read_done = true;
                       deadline.cancel();
                   },
                   asio::detached);

    sockets.io.run();
    REQUIRE(write_done);
    REQUIRE(!write_threw);
    REQUIRE(read_done);
    REQUIRE(!read_ec);
    REQUIRE_EQ(read_size, expected.size());
    REQUIRE_EQ(received, expected);
    REQUIRE(sockets.server.non_blocking());
}

TEST_CASE(http1_optimistic_write_closed_socket_falls_back)
{
    socket_pair sockets;
    asio::error_code ec;
    sockets.server.close(ec);
    REQUIRE(!ec);

    REQUIRE_EQ(run<std::size_t>(sockets.io, http::http1_optimistic_write(sockets.server, "closed")), 0u);
}

TEST_MAIN()
