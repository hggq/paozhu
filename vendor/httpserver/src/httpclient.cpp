/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file httpclient.cpp
 *  @date 2022-05-15
 *
 *  http client supper http(s) file
 *
 *
 */
#include <algorithm>
#include <sstream>
#include <ctime>
#include <array>
#include <string>
#include <string_view>
#include <random>
#include <sys/stat.h>
#include <cstdlib>
#include "http_socket.h"
#include "request.h"
#include "httpclient.h"
#include "urlcode.h"
#include "md5.h"
#include "zlib.h"
#include "http_mime.h"
#include "datetime.h"
#include "gzip.h"
#include "client_context.h"
#include "terminal_color.h"
#include "func.h"
#include "atomic_guard.h"

namespace http
{
client::client() : strand_(asio::make_strand(*(get_client_context_obj().ioc))), rawfile(nullptr, std::fclose) {};
client::client(std::string_view url) : _url(url), strand_(asio::make_strand(*(get_client_context_obj().ioc))), rawfile(nullptr, std::fclose)
{
    requesttype = 0;
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
};
client::~client()
{
    if (isssl)
    {
        if (sslsock)
        {
            // sslsock->shutdown(ec);
            // if (sslsock->lowest_layer().is_open())
            // {
            //     sslsock->lowest_layer().close();
            // }
            if (sslsock->lowest_layer().is_open())
            {
                sslsock->lowest_layer().cancel(ec);
                sslsock->lowest_layer().close(ec);
            }
            sslsock.reset();
        }
    }
    else
    {
        if (sock && sock->is_open())
        {
            sock->close(ec);
            sock.reset();
        }
    }
}

void client::close_connect()
{
    std::lock_guard<std::mutex> lk(lock_close);
    if (isssl)
    {
        if (sslsock)
        {
            // sslsock->shutdown(ec);
            // if (sslsock->lowest_layer().is_open())
            // {
            //     sslsock->lowest_layer().close();
            // }
            if (sslsock->lowest_layer().is_open())
            {
                sslsock->lowest_layer().cancel(ec);
                sslsock->lowest_layer().close(ec);
            }
            sslsock.reset();
        }
    }
    else
    {
        if (sock && sock->is_open())
        {
            sock->close(ec);
            sock.reset();
        }
    }
}

client &client::requst_clear()
{
    senddata.clear();
    header.clear();
    postdata.clear();
    iserror = 0;
    return *this;
}
client &client::clear()
{
    senddata.clear();
    header.clear();
    postdata.clear();
    query_.clear();
    path.clear();
    host.clear();
    port.clear();
    iserror       = 0;
    headerfinish  = 0;
    timeout_count = 0;
    machnum       = 0;
    response_header.clear();
    page.code     = 0;
    page.length   = 0;
    page.istxt    = false;
    page.isjson   = false;
    page.chunked  = false;
    page.keeplive = false;
    page.encode   = 0x00;

    page.content.clear();
    page.codemessage.clear();
    page.cookie.clear();

    page.file.name.clear();
    page.file.filename.clear();
    page.file.tempfile.clear();
    page.file.type.clear();
    page.file.size  = 0;
    page.file.error = 0;
    page.padd       = 0;
    page.json.clear();
    close_connect();
    return *this;
}
client &client::get(std::string_view url, unsigned int time_out_num)
{
    requesttype = 0;
    exptime     = time_out_num;
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    return *this;
}

void client::query(std::string_view url)
{
    requesttype = 2;
    // senddata.clear();
    // header.clear();
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
}

client &client::get(std::string_view url)
{
    requesttype = 0;
    // senddata.clear();
    // header.clear();
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::get(std::string_view url, http::obj_val param)
{
    requesttype = 0;
    // senddata.clear();
    // header.clear();
    parameter = std::move(param);
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::post(std::string_view url)
{
    requesttype = 1;
    // senddata.clear();
    // header.clear();
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::post(std::string_view url, http::obj_val param)
{
    requesttype = 1;
    // senddata.clear();
    parameter = std::move(param);
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}

client &client::get_json(std::string_view url)
{
    requesttype = 0;
    parsetojson = 1;
    // header.clear();
    // senddata.clear();
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::get_json(std::string_view url, http::obj_val param)
{
    requesttype = 0;
    parsetojson = 1;
    // header.clear();
    // senddata.clear();
    parameter = std::move(param);
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}

client &client::post_json(std::string_view url)
{
    // header.clear();
    // senddata.clear();
    requesttype            = 1;
    parsetojson            = 1;
    header["Content-Type"] = "application/json";
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::post_json(std::string_view url, http::obj_val param)
{
    // header.clear();
    // senddata.clear();
    requesttype            = 1;
    header["Content-Type"] = "application/json";

    parsetojson = 1;
    parameter   = std::move(param);
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}

client &client::send()
{

    if (requesttype > 0)
    {
        buildcontent();
        buildheader();
    }
    else
    {
        buildheader();
    }

    if (scheme == "https")
    {
        send_ssl_data();
    }
    else
    {
        send_data();
    }

    return *this;
}

asio::awaitable<void> client::async_send()
{

    if (requesttype > 0)
    {
        buildcontent();
        buildheader();
    }
    else
    {
        buildheader();
    }
    if (scheme == "https")
    {
        co_await async_send_ssl_data();
    }
    else
    {
        co_await async_send_data();
    }

    co_return;
}

asio::awaitable<bool> client::async_init_http_sock()
{
    error_msg.clear();
    asio::ip::tcp::resolver resolver(strand_);

    sock                           = std::make_shared<asio::ip::tcp::socket>(strand_);
    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    auto endpoints                 = co_await resolver.async_resolve(host, port, asio::use_awaitable);

    for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
    {
        std::tie(ec) = co_await sock->async_connect(*iter, tuple_awaitable);
        if (ec)
        {
            continue;
        }
        break;
    }

    if (ec)
    {
        //error_msg = "Unable to connect";
        error_msg = ec.message();
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}
asio::awaitable<bool> client::async_init_https_sock()
{
    error_msg.clear();
    ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    sslsock     = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(strand_, *ssl_context);

    ssl_context->set_default_verify_paths();

    asio::ip::tcp::resolver resolver(strand_);

    SSL_set_tlsext_host_name(sslsock->native_handle(), host.c_str());

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    auto endpoints                 = co_await resolver.async_resolve(host, port, asio::use_awaitable);

    for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
    {
        std::tie(ec) = co_await sslsock->lowest_layer().async_connect(*iter, tuple_awaitable);
        if (ec)
        {
            continue;
        }
        break;
    }

    sslsock->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

    ssl_context->set_verify_mode(asio::ssl::verify_peer);
    ssl_context->set_verify_callback(asio::ssl::host_name_verification(host));

    //sslsock->handshake(asio::ssl::stream_base::client, ec);
    std::tie(ec) = co_await sslsock->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
    if (ec)
    {
        error_msg = host + " handshake error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}

bool client::init_http_sock()
{
    error_msg.clear();
    //client_context &temp_io_context = get_client_context_obj();
    sock = std::make_shared<asio::ip::tcp::socket>(strand_);
    asio::ip::tcp::resolver resolver(strand_);
    auto endpoints = resolver.resolve(host, port);
    //socket->connect(endpoints, ec);
    asio::connect(*sock, endpoints, ec);
    //asio::ip::tcp::resolver::query checkquery(host, port);
    // asio::ip::tcp::resolver::iterator iter = resolver.resolve(checkquery);
    // asio::ip::tcp::resolver::iterator end;
    // asio::ip::tcp::endpoint endpoint;
    // if (iter == end)
    // {
    //     error_msg = "resolver " + host + port;
    //     DEBUG_LOG("%s", error_msg.c_str());
    //     return false;
    // }
    // while (iter != end)
    // {
    //     endpoint = *iter++;
    //     // at here  maybe under code
    //     sock->connect(endpoint, ec);
    //     if (ec)
    //     {
    //         continue;
    //     }
    // }
    // asio::error_code ec;

    if (ec)
    {
        //error_msg = "Unable to connect";
        error_msg = ec.message();
        DEBUG_LOG("%s", error_msg.c_str());
        return false;
    }
    return true;
}

asio::awaitable<void> client::async_send_data()
{
    try
    {
        headerfinish  = 0;
        timeout_count = 0;
        machnum       = 0;
        response_header.clear();
        page.code     = 0;
        page.length   = 0;
        page.istxt    = false;
        page.isjson   = false;
        page.chunked  = false;
        page.keeplive = false;
        page.encode   = 0x00;

        page.content.clear();
        page.codemessage.clear();
        page.cookie.clear();

        page.file.name.clear();
        page.file.filename.clear();
        page.file.tempfile.clear();
        page.file.type.clear();
        page.file.size  = 0;
        page.file.error = 0;
        page.padd       = 0;
        page.json.clear();

        if (!sock)
        {
            bool isinit = co_await async_init_http_sock();
            if (!isinit)
            {
                co_return;
            }
        }

        if (exptime > 90)
        {
            exptime = 90;
        }
        if (exptime > 0)
        {
            set_timeout(exptime);
            client_context &temp_io_context = get_client_context_obj();
            try
            {
                temp_io_context.timeout_lists.push_back(shared_from_this());
                temp_io_context.timeout_condition.notify_one();
            }
            catch (const std::exception &e)
            {
                DEBUG_LOG("Exception: %s", e.what());
                error_msg = e.what();
            }
        }
        co_await asio::async_write(*sock, asio::buffer(request), asio::use_awaitable);

        timeout_total = timeid() + timeout_total;
        char read_data[2051];
        unsigned int n;
        if (requesttype > 0)
        {
            for (auto &p : senddata)
            {
                switch (p.error)
                {
                case 0:
                    co_await asio::async_write(*sock, asio::buffer(p.tempfile), asio::use_awaitable);
                    break;
                case 3:
                    co_await asio::async_write(*sock, asio::buffer(p.tempfile), asio::use_awaitable);
                    break;
                case 1:
                {
                    unsigned int readnum = 0;
                    co_await asio::async_write(*sock, asio::buffer(p.tempfile), asio::use_awaitable);
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(p.filename.c_str(), "rb"), std::fclose);
                    if (!fp)
                    {
                        read_data[0] = 0x0D;
                        read_data[1] = 0x0A;
                        co_await asio::async_write(*sock, asio::buffer(read_data, 2), asio::use_awaitable);
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    read_data[2048] = 0x00;
                    while (readnum < n)
                    {
                        auto nread = fread(read_data, 1, 2048, fp.get());
                        readnum += nread;
                        if (readnum >= n)
                        {
                            read_data[nread] = 0x0D;
                            nread += 1;
                            read_data[nread] = 0x0A;
                            nread += 1;
                        }
                        co_await asio::async_write(*sock, asio::buffer(read_data, nread), asio::use_awaitable);
                        if (upload_process != nullptr)
                        {
                            upload_process(readnum, n);
                        }
                        if (exptime > 0)
                        {
                            if (iswait_exit)
                            {
                                iswait_exit = false;
                                reset_connect_timeout();
                                if (timeout_total > 0)
                                {
                                    if (timeout_count > timeout_total)
                                    {
                                        page.code = 0;
                                        iserror   = 2;
                                        error_msg = "time out";
                                        close_connect();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (iserror > 0)
                    {
                        co_return;
                    }
                    // fclose(fp);
                }
                break;
                case 2:
                {
                    unsigned int readnum = 0;
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(p.filename.c_str(), "rb"), std::fclose);
                    if (!fp)
                    {
                        read_data[0] = 0x0D;
                        read_data[1] = 0x0A;
                        co_await asio::async_write(*sock, asio::buffer(read_data, 2), asio::use_awaitable);
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    read_data[2048] = 0x00;
                    while (readnum < n)
                    {
                        auto nread = fread(read_data, 1, 2048, fp.get());
                        readnum += nread;
                        co_await asio::async_write(*sock, asio::buffer(read_data, nread), asio::use_awaitable);
                        if (upload_process != nullptr)
                        {
                            upload_process(readnum, n);
                        }

                        if (exptime > 0)
                        {
                            if (iswait_exit)
                            {
                                iswait_exit = false;
                                reset_connect_timeout();
                                if (timeout_total > 0)
                                {
                                    if (timeout_count > timeout_total)
                                    {
                                        page.code = 0;
                                        iserror   = 2;
                                        error_msg = "time out";
                                        close_connect();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (iserror > 0)
                    {
                        co_return;
                    }
                    // fclose(fp);
                }
                break;
                }
            }
        }

        // unsigned long long totalsize;
        // rawfile = NULL;

        while (true)
        {
            memset(read_data, 0x00, 2048);
            //BUG Not considering the file state
            if (page.file.size > 0 && (page.file.size - page.content.size() < 2048))
            {
                n = co_await sock->async_read_some(asio::buffer(read_data, page.file.size - page.content.size()), asio::use_awaitable);
            }
            else
            {
                n = co_await sock->async_read_some(asio::buffer(read_data, 2048), asio::use_awaitable);
            }
            read_data[2048] = 0x00;
            if (n == 0)
            {
                break;
            }
            readoffset = 0;
            if (process(read_data, n))
            {
                break;
            }
            if (page.file.size > 0 && (page.file.size == page.content.size()))
            {
                break;
            }
            if (exptime > 0)
            {
                if (iswait_exit)
                {
                    iswait_exit = false;
                    reset_connect_timeout();
                    if (timeout_total > 0)
                    {
                        if (timeout_count > timeout_total)
                        {
                            page.code = 0;
                            iserror   = 2;
                            error_msg = "time out";
                            close_connect();
                            break;
                        }
                    }
                }
            }
        }
        finishprocess();
        if (onload != nullptr)
        {
            onload(page.content, shared_from_this());
        }
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        page.code = 0;
        iserror   = 3;
    }

    co_return;
}

client &client::send_data()
{
    try
    {
        headerfinish  = 0;
        timeout_count = 0;
        machnum       = 0;
        response_header.clear();
        page.code     = 0;
        page.length   = 0;
        page.istxt    = false;
        page.isjson   = false;
        page.chunked  = false;
        page.keeplive = false;
        page.encode   = 0x00;

        page.content.clear();
        page.codemessage.clear();
        page.cookie.clear();

        page.file.name.clear();
        page.file.filename.clear();
        page.file.tempfile.clear();
        page.file.type.clear();
        page.file.size  = 0;
        page.file.error = 0;
        page.padd       = 0;
        page.json.clear();

        //asio::io_context clientio_context(1);
        //client_context &temp_io_context = get_client_context_obj();
        // asio::signal_set signals(temp_io_context, SIGINT, SIGTERM);
        // signals.async_wait([&](auto, auto)
        //                    { clientio_context.stop(); });

        // std::shared_ptr<httpclient_t> clientpeer = std::make_shared<httpclient_t>();

        // sock = std::make_shared<asio::ip::tcp::socket>(temp_io_context.ioc);

        // //asio::ip::tcp::socket socket(temp_io_context.ioc);
        // asio::ip::tcp::resolver resolver(temp_io_context.ioc);

        // asio::ip::tcp::resolver::query checkquery(host, port);
        // asio::ip::tcp::resolver::iterator iter = resolver.resolve(checkquery);
        // asio::ip::tcp::resolver::iterator end;
        // asio::ip::tcp::endpoint endpoint;
        // while (iter != end)
        // {
        //     endpoint = *iter++;
        //     // at here  maybe under code
        // }
        // // asio::error_code ec;
        // sock->connect(endpoint, ec);
        // if (ec)
        // {
        //     std::cerr << "Unable to connect:  \r\n";
        //     return *this;
        // }
        if (!sock)
        {
            if (!init_http_sock())
            {
                return *this;
            }
        }

        if (exptime > 90)
        {
            exptime = 90;
        }
        if (exptime > 0)
        {
            set_timeout(exptime);
            client_context &temp_io_context = get_client_context_obj();
            try
            {
                temp_io_context.timeout_lists.push_back(shared_from_this());
                temp_io_context.timeout_condition.notify_one();
            }
            catch (const std::exception &e)
            {
                DEBUG_LOG("Exception: %s", e.what());
                error_msg = e.what();
            }
        }
        sock->write_some(asio::buffer(request));

        timeout_total = timeid() + timeout_total;
        char read_data[2051];
        unsigned int n;
        if (requesttype > 0)
        {
            for (auto &p : senddata)
            {
                switch (p.error)
                {
                case 0: sock->write_some(asio::buffer(p.tempfile)); break;
                case 3: sock->write_some(asio::buffer(p.tempfile)); break;
                case 1:
                {
                    unsigned int readnum = 0;
                    sock->write_some(asio::buffer(p.tempfile));
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(p.filename.c_str(), "rb"), std::fclose);
                    if (!fp)
                    {
                        read_data[0] = 0x0D;
                        read_data[1] = 0x0A;
                        asio::write(*sock, asio::buffer(read_data, 2));
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    read_data[2048] = 0x00;
                    while (readnum < n)
                    {
                        auto nread = fread(read_data, 1, 2048, fp.get());
                        readnum += nread;
                        if (readnum >= n)
                        {
                            read_data[nread] = 0x0D;
                            nread += 1;
                            read_data[nread] = 0x0A;
                            nread += 1;
                        }
                        asio::write(*sock, asio::buffer(read_data, nread));
                        if (upload_process != nullptr)
                        {
                            upload_process(readnum, n);
                        }
                        if (exptime > 0)
                        {
                            if (iswait_exit)
                            {
                                iswait_exit = false;
                                reset_connect_timeout();
                                if (timeout_total > 0)
                                {
                                    if (timeout_count > timeout_total)
                                    {
                                        page.code = 0;
                                        iserror   = 2;
                                        error_msg = "time out";
                                        close_connect();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (iserror > 0)
                    {
                        return *this;
                    }
                    // fclose(fp);
                }
                break;
                case 2:
                {
                    unsigned int readnum = 0;
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(p.filename.c_str(), "rb"), std::fclose);
                    if (!fp)
                    {
                        read_data[0] = 0x0D;
                        read_data[1] = 0x0A;
                        asio::write(*sock, asio::buffer(read_data, 2));
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    read_data[2048] = 0x00;
                    while (readnum < n)
                    {
                        auto nread = fread(read_data, 1, 2048, fp.get());
                        readnum += nread;
                        asio::write(*sock, asio::buffer(read_data, nread));
                        if (upload_process != nullptr)
                        {
                            upload_process(readnum, n);
                        }
                        if (exptime > 0)
                        {
                            if (iswait_exit)
                            {
                                iswait_exit = false;
                                reset_connect_timeout();
                                if (timeout_total > 0)
                                {
                                    if (timeout_count > timeout_total)
                                    {
                                        page.code = 0;
                                        iserror   = 2;
                                        error_msg = "time out";
                                        close_connect();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (iserror > 0)
                    {
                        return *this;
                    }
                    // fclose(fp);
                }
                break;
                }
            }
        }

        // unsigned long long totalsize;
        // rawfile = NULL;

        while (true)
        {
            memset(read_data, 0x00, 2048);
            //BUG Not considering the file state
            if (page.file.size > 0 && (page.file.size - page.content.size() < 2048))
            {
                n = sock->read_some(asio::buffer(read_data, page.file.size - page.content.size()), ec);
            }
            else
            {
                n = sock->read_some(asio::buffer(read_data, 2048), ec);
            }
            read_data[2048] = 0x00;
            if (n == 0)
            {
                break;
            }
            readoffset = 0;
            if (process(read_data, n))
            {
                break;
            }
            if (page.file.size > 0 && (page.file.size == page.content.size()))
            {
                break;
            }
            if (exptime > 0)
            {
                if (iswait_exit)
                {
                    iswait_exit = false;
                    reset_connect_timeout();
                    if (timeout_total > 0)
                    {
                        if (timeout_count > timeout_total)
                        {
                            page.code = 0;
                            iserror   = 2;
                            error_msg = "time out";
                            close_connect();
                            break;
                        }
                    }
                }
            }
        }
        finishprocess();
        DEBUG_LOG(" http finishprocess: %zu", page.content.size());
    }
    catch (std::exception &e)
    {
        // std::printf("Exception: %s\n", e.what());
        //std::cerr << "Exception:  " << e.what() << "\r\n";
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        page.code = 0;
        iserror   = 3;
    }

    return *this;
}

bool client::init_https_sock()
{
    error_msg.clear();
    //client_context &temp_io_context = get_client_context_obj();
    //asio::ssl::context ssl_context(asio::ssl::context::sslv23);
    ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    //std::shared_ptr<httpclient_t> clientpeer = std::make_shared<httpclient_t>();
    sslsock = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(strand_, *ssl_context);

    //asio::ssl::stream<asio::ip::tcp::socket> socket(temp_io_context.ioc, ssl_context);
    ssl_context->set_default_verify_paths();
    asio::ip::tcp::resolver resolver(strand_);
    auto endpoints = resolver.resolve(host.c_str(), port);

    SSL_set_tlsext_host_name(sslsock->native_handle(), host.c_str());

    asio::connect(sslsock->lowest_layer(), endpoints, ec);
    if (ec)
    {
        error_msg = host + " connect error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        return false;
    }

    sslsock->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

    ssl_context->set_verify_mode(asio::ssl::verify_peer);
    ssl_context->set_verify_callback(asio::ssl::host_name_verification(host));

    sslsock->handshake(asio::ssl::stream_base::client, ec);

    if (ec)
    {
        error_msg = host + " handshake error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        return false;
    }
    return true;
}

asio::awaitable<void> client::async_send_ssl_data()
{
    // ssl请求
    try
    {
        headerfinish  = 0;
        timeout_count = 0;
        machnum       = 0;
        response_header.clear();
        page.code     = 0;
        page.length   = 0;
        page.istxt    = false;
        page.isjson   = false;
        page.chunked  = false;
        page.keeplive = false;
        page.encode   = 0x00;

        page.content.clear();
        page.codemessage.clear();
        page.cookie.clear();

        page.file.name.clear();
        page.file.filename.clear();
        page.file.tempfile.clear();
        page.file.type.clear();
        page.file.size  = 0;
        page.file.error = 0;
        page.padd       = 0;
        page.json.clear();

        if (!sslsock)
        {
            error_msg.clear();
            //auto executor = co_await asio::this_coro::executor;
            ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
            sslsock     = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(strand_, *ssl_context);
            ssl_context->set_default_verify_paths();

            asio::ip::tcp::resolver resolver(strand_);
            // asio::ip::tcp::resolver::iterator iter = co_await resolver.async_resolve(host, port, asio::use_awaitable);
            // asio::ip::tcp::resolver::iterator end;
            // asio::ip::tcp::endpoint endpoint;

            SSL_set_tlsext_host_name(sslsock->native_handle(), host.c_str());

            constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
            auto endpoints                 = co_await resolver.async_resolve(host, port, asio::use_awaitable);
            for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
            {
                std::tie(ec) = co_await sslsock->lowest_layer().async_connect(*iter, tuple_awaitable);
                if (ec)
                {
                    continue;
                }
                break;
            }
            if (ec)
            {
                error_msg = host + " async_connect error! ";
                DEBUG_LOG("%s", error_msg.c_str());
                co_return;
            }
            // while (iter != end)
            // {
            //     endpoint     = *iter++;
            //     std::tie(ec) = co_await sslsock->lowest_layer().async_connect(endpoint, tuple_awaitable);
            //     if (ec)
            //     {
            //         continue;
            //     }
            //     else
            //     {
            //         break;
            //     }
            // }

            sslsock->lowest_layer().set_option(asio::ip::tcp::no_delay(true));
            ssl_context->set_verify_mode(asio::ssl::verify_peer);
            ssl_context->set_verify_callback(asio::ssl::host_name_verification(host));

            std::tie(ec) = co_await sslsock->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
            if (ec)
            {
                error_msg = host + " handshake error! ";
                DEBUG_LOG("%s", error_msg.c_str());
                co_return;
            }
        }
        if (exptime > 90)
        {
            exptime = 90;
        }
        if (exptime > 0)
        {
            set_timeout(exptime);
            client_context &temp_io_context = get_client_context_obj();
            try
            {
                temp_io_context.timeout_lists.push_back(shared_from_this());
                temp_io_context.timeout_condition.notify_one();
            }
            catch (const std::exception &e)
            {
                error_msg = e.what();
            }
        }

        int n;

        n = co_await asio::async_write(*sslsock, asio::buffer(request), asio::use_awaitable);

        timeout_total = timeid() + timeout_total;
        char read_data[2051];
        if (requesttype > 0)
        {
            for (auto &p : senddata)
            {

                switch (p.error)
                {
                case 0:
                    co_await asio::async_write(*sslsock, asio::buffer(p.tempfile), asio::use_awaitable);
                    break;
                case 3:
                    co_await asio::async_write(*sslsock, asio::buffer(p.tempfile), asio::use_awaitable);
                    break;
                case 1:
                {
                    int readnum = 0;
                    co_await asio::async_write(*sslsock, asio::buffer(p.tempfile), asio::use_awaitable);
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(p.filename.c_str(), "rb"), std::fclose);
                    if (!fp)
                    {
                        read_data[0] = 0x0D;
                        read_data[1] = 0x0A;
                        co_await asio::async_write(*sslsock, asio::buffer(read_data, 2), asio::use_awaitable);
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    read_data[2048] = 0x00;
                    while (readnum < n)
                    {
                        auto nread = fread(read_data, 1, 2048, fp.get());
                        readnum += nread;
                        if (readnum >= n)
                        {
                            read_data[nread] = 0x0D;
                            nread += 1;
                            read_data[nread] = 0x0A;
                            nread += 1;
                        }
                        co_await asio::async_write(*sslsock, asio::buffer(read_data, nread), asio::use_awaitable);
                        if (upload_process != nullptr)
                        {
                            upload_process(readnum, n);
                        }
                        if (exptime > 0)
                        {
                            if (iswait_exit)
                            {
                                iswait_exit = false;
                                reset_connect_timeout();
                                if (timeout_total > 0)
                                {
                                    if (timeout_count > timeout_total)
                                    {
                                        page.code = 0;
                                        iserror   = 2;
                                        error_msg = "time out";
                                        close_connect();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (iserror > 0)
                    {
                        co_return;
                    }
                    // fclose(fp);
                }
                break;
                case 2:
                {
                    int readnum = 0;
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(p.filename.c_str(), "rb"), std::fclose);
                    if (!fp)
                    {
                        read_data[0] = 0x0D;
                        read_data[1] = 0x0A;
                        co_await asio::async_write(*sslsock, asio::buffer(read_data, 2), asio::use_awaitable);
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    read_data[2048] = 0x00;
                    while (readnum < n)
                    {
                        auto nread = fread(read_data, 1, 2048, fp.get());
                        readnum += nread;
                        co_await asio::async_write(*sslsock, asio::buffer(read_data, nread), asio::use_awaitable);
                        if (upload_process != nullptr)
                        {
                            upload_process(readnum, n);
                        }
                        if (exptime > 0)
                        {
                            if (iswait_exit)
                            {
                                iswait_exit = false;
                                reset_connect_timeout();
                                if (timeout_total > 0)
                                {
                                    if (timeout_count > timeout_total)
                                    {
                                        page.code = 0;
                                        iserror   = 2;
                                        error_msg = "time out";
                                        close_connect();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (iserror > 0)
                    {
                        co_return;
                    }
                    // fclose(fp);
                }
                break;
                }
            }
        }

        DEBUG_LOG("while async_read_some: %s", host.c_str());

        while (true)
        {
            memset(read_data, 0x00, 2048);
            //BUG Not considering the file state
            if (page.file.size > 0 && (page.file.size - page.content.size() < 2048))
            {
                n = co_await sslsock->async_read_some(asio::buffer(read_data, page.file.size - page.content.size()), asio::use_awaitable);
            }
            else
            {
                n = co_await sslsock->async_read_some(asio::buffer(read_data, 2048), asio::use_awaitable);
            }
            read_data[2048] = 0x00;
            if (n == 0)
            {
                break;
            }
            readoffset = 0;
            if (process(read_data, n))
            {
                break;
            }
            if (page.file.size > 0 && (page.file.size == page.content.size()))
            {
                break;
            }

            if (exptime > 0)
            {
                if (iswait_exit)
                {
                    iswait_exit = false;
                    reset_connect_timeout();
                    if (timeout_total > 0)
                    {
                        if (timeout_count > timeout_total)
                        {
                            page.code = 0;
                            iserror   = 2;
                            error_msg = "time out";
                            close_connect();
                            break;
                        }
                    }
                }
            }
        }
        finishprocess();
        if (onload != nullptr)
        {
            onload(page.content, shared_from_this());
        }
        co_return;
    }
    catch (std::exception &e)
    {
        //std::printf("Exception: %s\n", e.what());
        DEBUG_LOG("Exception co ssl: %s", e.what());
        error_msg = std::string(e.what());
        page.code = 0;
        iserror   = 3;
    }
    co_return;
}
client &client::send_ssl_data()
{
    // ssl请求
    try
    {
        headerfinish  = 0;
        timeout_count = 0;
        machnum       = 0;
        response_header.clear();
        page.code     = 0;
        page.length   = 0;
        page.istxt    = false;
        page.isjson   = false;
        page.chunked  = false;
        page.keeplive = false;
        page.encode   = 0x00;

        page.content.clear();
        page.codemessage.clear();
        page.cookie.clear();

        page.file.name.clear();
        page.file.filename.clear();
        page.file.tempfile.clear();
        page.file.type.clear();
        page.file.size  = 0;
        page.file.error = 0;
        page.padd       = 0;
        page.json.clear();

        //asio::io_context io_context;
        // client_context &temp_io_context = get_client_context_obj();
        // asio::ssl::context ssl_context(asio::ssl::context::sslv23);

        // //std::shared_ptr<httpclient_t> clientpeer = std::make_shared<httpclient_t>();
        // sslsock = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(temp_io_context.ioc, ssl_context);

        // //asio::ssl::stream<asio::ip::tcp::socket> socket(temp_io_context.ioc, ssl_context);
        // ssl_context.set_default_verify_paths();
        // asio::ip::tcp::resolver resolver(temp_io_context.ioc);
        // auto endpoints = resolver.resolve(host.c_str(), port);

        // SSL_set_tlsext_host_name(sslsock->native_handle(), host.c_str());

        // asio::connect(sslsock->lowest_layer(), endpoints);
        // sslsock->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

        // ssl_context.set_verify_mode(asio::ssl::verify_peer);
        // ssl_context.set_verify_callback(asio::ssl::host_name_verification(host));

        // sslsock->handshake(asio::ssl::stream_base::client, ec);

        // if (ec)
        // {
        //     return *this;
        // }
        if (!sslsock)
        {
            if (!init_https_sock())
            {
                return *this;
            }
        }
        if (exptime > 90)
        {
            exptime = 90;
        }
        if (exptime > 0)
        {
            set_timeout(exptime);
            client_context &temp_io_context = get_client_context_obj();
            try
            {
                temp_io_context.timeout_lists.push_back(shared_from_this());
                temp_io_context.timeout_condition.notify_one();
            }
            catch (const std::exception &e)
            {
                error_msg = e.what();
            }
        }

        int n;
        n = sslsock->write_some(asio::buffer(request));

        timeout_total = timeid() + timeout_total;
        char read_data[2051];
        if (requesttype > 0)
        {
            for (auto &p : senddata)
            {

                switch (p.error)
                {
                case 0: sslsock->write_some(asio::buffer(p.tempfile)); break;
                case 3: sslsock->write_some(asio::buffer(p.tempfile)); break;
                case 1:
                {
                    int readnum = 0;
                    sslsock->write_some(asio::buffer(p.tempfile));
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(p.filename.c_str(), "rb"), std::fclose);
                    if (!fp)
                    {
                        read_data[0] = 0x0D;
                        read_data[1] = 0x0A;
                        asio::write(*sslsock, asio::buffer(read_data, 2));
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    read_data[2048] = 0x00;
                    while (readnum < n)
                    {
                        auto nread = fread(read_data, 1, 2048, fp.get());
                        readnum += nread;
                        if (readnum >= n)
                        {
                            read_data[nread] = 0x0D;
                            nread += 1;
                            read_data[nread] = 0x0A;
                            nread += 1;
                        }
                        asio::write(*sslsock, asio::buffer(read_data, nread));
                        if (upload_process != nullptr)
                        {
                            upload_process(readnum, n);
                        }
                        if (exptime > 0)
                        {
                            if (iswait_exit)
                            {
                                iswait_exit = false;
                                reset_connect_timeout();
                                if (timeout_total > 0)
                                {
                                    if (timeout_count > timeout_total)
                                    {
                                        page.code = 0;
                                        iserror   = 2;
                                        error_msg = "time out";
                                        close_connect();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (iserror > 0)
                    {
                        return *this;
                    }
                    // fclose(fp);
                }
                break;
                case 2:
                {
                    int readnum = 0;
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(p.filename.c_str(), "rb"), std::fclose);
                    if (!fp)
                    {
                        read_data[0] = 0x0D;
                        read_data[1] = 0x0A;
                        asio::write(*sslsock, asio::buffer(read_data, 2));
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    read_data[2048] = 0x00;
                    while (readnum < n)
                    {
                        auto nread = fread(read_data, 1, 2048, fp.get());
                        readnum += nread;
                        asio::write(*sslsock, asio::buffer(read_data, nread));
                        if (upload_process != nullptr)
                        {
                            upload_process(readnum, n);
                        }

                        if (exptime > 0)
                        {
                            if (iswait_exit)
                            {
                                iswait_exit = false;
                                reset_connect_timeout();
                                if (timeout_total > 0)
                                {
                                    if (timeout_count > timeout_total)
                                    {
                                        page.code = 0;
                                        iserror   = 2;
                                        error_msg = "time out";
                                        close_connect();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (iserror > 0)
                    {
                        return *this;
                    }
                    // fclose(fp);
                }
                break;
                }
            }
        }
        DEBUG_LOG("while ssl_read_some: %s", host.c_str());

        while (true)
        {
            memset(read_data, 0x00, 2048);
            //BUG Not considering the file state
            if (page.file.size > 0 && (page.file.size - page.content.size() < 2048))
            {
                n = sslsock->read_some(asio::buffer(read_data, page.file.size - page.content.size()), ec);
            }
            else
            {
                n = sslsock->read_some(asio::buffer(read_data, 2048), ec);
            }
            read_data[2048] = 0x00;
            if (n == 0)
            {
                break;
            }
            readoffset = 0;
            if (process(read_data, n))
            {
                break;
            }
            if (page.file.size > 0 && (page.file.size == page.content.size()))
            {
                break;
            }
            if (exptime > 0)
            {
                if (iswait_exit)
                {
                    iswait_exit = false;
                    reset_connect_timeout();
                    if (timeout_total > 0)
                    {
                        if (timeout_count > timeout_total)
                        {
                            page.code = 0;
                            iserror   = 2;
                            error_msg = "time out";
                            close_connect();
                            break;
                        }
                    }
                }
            }
        }
        finishprocess();
        DEBUG_LOG("ssl_finishprocess %zu", page.content.size());
        return *this;
    }
    catch (std::exception &e)
    {
        //std::printf("Exception: %s\n", e.what());
        DEBUG_LOG("Exception ssl: %s", e.what());
        error_msg = e.what();
        page.code = 0;
        iserror   = 3;
    }

    return *this;
}
void client::processcode()
{
    std::string key, value;
    unsigned int code = 0;
    unsigned int j    = 0;
    for (; j < contentline.size(); j++)
    {
        if (contentline[j] == 0x20)
        {
            break;
        }
        value.push_back(contentline[j]);
    }
    if (value[0] != 'H' && value[0] != 'h')
    {
        page.code = 0;
        iserror   = 1;

        return;
    }

    j++;
    for (; j < contentline.size(); j++)
    {
        if (contentline[j] != 0x20)
        {
            break;
        }
    }
    value.clear();
    for (; j < contentline.size(); j++)
    {
        if (contentline[j] == 0x20)
        {
            break;
        }
        if (contentline[j] >= '0' && contentline[j] <= '9')
        {
            code = code * 10 + (contentline[j] - '0');
        }
        if (code > 1000)
        {
            iserror = 1;
            return;
        }
    }
    page.code = code;
    if (code == 0)
    {
        iserror = 1;
        return;
    }

    for (; j < contentline.size(); j++)
    {
        if (contentline[j] != 0x20)
        {
            break;
        }
    }
    for (; j < contentline.size(); j++)
    {
        page.codemessage.push_back(contentline[j]);
    }
}

bool client::parse_header_fields(const std::string &line_temp)
{
    if (page.code == 0)
    {
        processcode();
        if (page.code == 0)
        {
            iserror = 1;
            return false;
        }
    }
    else
    {
        std::string key, value;
        unsigned int k = 0;
        for (; k < line_temp.size(); k++)
        {
            if (line_temp[k] != 0x20)
            {
                break;
            }
        }

        for (; k < line_temp.size(); k++)
        {
            if (line_temp[k] == ':')
            {
                break;
            }
            key.push_back(line_temp[k]);
        }

        if (key.size() > 0)
        {
            if (key.back() == 0x20)
            {
                key.pop_back();
            }
        }

        for (; k < line_temp.size(); k++)
        {
            if (line_temp[k] != 0x20 && line_temp[k] != ':')
            {
                break;
            }
        }

        for (; k < line_temp.size(); k++)
        {
            value.push_back(line_temp[k]);
        }

        if (value.size() > 0)
        {
            if (value.back() == 0x20)
            {
                value.pop_back();
            }
        }

        if (key.size() > 0)
        {
            if (key.size() == 10 && str_casecmp(key, "Set-Cookie"))
            {
                respcookieprocess(value);
            }
            else
            {
                page.header[key] = value;
                responseheader(key, value);
            }
        }
    }
    return true;
}

void client::readheaderline(const char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;

    if (contentline.size() > 0)
    {
        if (i < buffersize)
        {
            if (buffer[i] != '\n')
            {
                iserror    = 1;
                readoffset = buffersize;
                return;
            }
            readoffset++;
            i++;
        }

        if (contentline.size() == 0)
        {
            //这样不用+1，因为上面已经加了
            headerfinish = 1;
            return;
        }
        page.rawheader.append(contentline);
        page.rawheader.append("\r\n");
        if (!parse_header_fields(contentline))
        {
            iserror    = 1;
            readoffset = buffersize;
            return;
        }
        contentline.clear();
    }

    for (; i < buffersize; i++)
    {
        if (buffer[i] == '\r')
        {
            unsigned int j = i;
            j++;
            if (j < buffersize)
            {
                if (buffer[j] == '\n')
                {
                    i = j;
                    if (contentline.size() == 0)
                    {
                        //如果是头部结束需要下一位，不让头部信息到body
                        i++;
                        readoffset   = i;
                        headerfinish = 1;
                        return;
                    }
                    page.rawheader.append(contentline);
                    page.rawheader.append("\r\n");
                    if (!parse_header_fields(contentline))
                    {
                        iserror    = 1;
                        readoffset = buffersize;
                        return;
                    }
                    contentline.clear();
                    //下一行
                    continue;
                }
                else
                {
                    //必须是\r\n
                    iserror    = 1;
                    readoffset = buffersize;
                    return;
                }
            }
            else
            {
                //跨包
                readoffset = buffersize;
                return;
            }
        }
        contentline.push_back(buffer[i]);
    }
    readoffset = i;
}

void client::respcookieprocess(std::string_view str)
{
    // Set-Cookie: name=aabb; expires=Mon, 28-Mar-2022 12:10:55 GMT; Max-Age=3600; path=/; domain=localhost; secure;
    // HttpOnly
    std::string name, value;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        if (str[i] == '=')
        {
            name = value;

            value.clear();
            continue;
        }
        if (str[i] == ';')
        {

            break;
        }
        value.push_back(str[i]);
    }
    if (name.size() > 0)
    {
        name  = url_decode(name.data(), name.size());
        value = url_decode(value.data(), value.size());
        page.cookie.set(name, value);
    }
}
void client::respattachmentprocess(std::string_view str)
{
    // Content-Disposition: attachment; filename="filename.jpg"
    std::string name, value;
    unsigned int i = 0;
    for (; i < str.length(); i++)
    {
        if (str[i] == 0x20)
        {
            continue;
        }
        break;
    }
    for (; i < str.length(); i++)
    {
        if (str[i] == '=')
        {
            if (value.size() > 0 && value.back() == 0x20)
            {
                value.pop_back();
            }

            if (str_casecmp(value, "filename"))
            {
                name = value;
                i++;
                for (; i < str.length(); i++)
                {
                    if (str[i] == 0x20)
                    {
                        continue;
                    }
                    break;
                }
                value.clear();
                break;
            }
            value.clear();
            continue;
        }

        if (str[i] == ';')
        {
            if (str_casecmp(value, "attachment"))
            {
                page.istxt = false;
            }
            value.clear();
            for (; i < str.length(); i++)
            {
                if (str[i + 1] == 0x20)
                {
                    continue;
                }
                break;
            }
            continue;
        }
        value.push_back(str[i]);
    }

    if (str_casecmp(name, "filename"))
    {
        page.istxt = false;
        page.file.filename.clear();

        for (; i < str.length(); i++)
        {
            if (str[i] == 0x22 || str[i] == 0x27)
            {
                continue;
            }
            break;
        }

        for (; i < str.length(); i++)
        {
            //" ' / ; char is end of filename
            if (str[i] == 0x22 || str[i] == 0x27 || str[i] == 0x2F || str[i] == 0x3B)
            {
                break;
            }
            page.file.filename.push_back(str[i]);
        }
    }

    if (value.size() > 0 && str_casecmp(value, "attachment"))
    {
        page.istxt = false;
    }
}
void client::respcontenttypeprocess(std::string_view str)
{
    // text/html; charset=UTF-8 //application/xhtml+xml, application/json, application/xml
    std::string name, value;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        if (str[i] == '=')
        {
            name = value;

            value.clear();
            continue;
        }

        if (str[i] == ';')
        {
            if (name.empty())
            {
                page.file.type = value;
            }
            value.clear();
            continue;
        }
        value.push_back(str[i]);
    }
    if (name.empty())
    {
        if (page.file.type.empty())
        {
            page.file.type = value;
        }
    }
    switch (page.file.type.size())
    {
    case 22:
        if (page.file.type == "application/javascript")
        {
            page.istxt = true;
        }
        break;
    case 21:
        if (page.file.type == "application/xhtml+xml")
        {
            page.istxt = true;
        }
        break;
    case 17:
        if (page.file.type == "text/event-stream")
        {
            page.istxt = true;
            page.issse = true;
        }
        break;
    case 16:
        if (page.file.type == "application/json")
        {
            page.istxt  = true;
            page.isjson = true;
        }
        break;
    case 15:
        if (page.file.type == "application/xml")
        {
            page.istxt = true;
            page.isxml = true;
        }
        break;
    case 10:
        if (page.file.type == "text/plain")
        {
            page.istxt = true;
        }
        break;
    case 9:
        if (page.file.type == "text/html")
        {
            page.istxt = true;
        }
        break;
    case 8:
        if (page.file.type == "text/css")
        {
            page.istxt = true;
        }
        else if (page.file.type == "text/xml")
        {
            page.istxt = true;
            page.isxml = true;
        }
        break;
    case 7:
        if (page.file.type == "text/js")
        {
            page.istxt = true;
        }
        break;
    }
    if (page.istxt == false)
    {
        if (page.file.filename.empty())
        {
            std::string::size_type lastpos = path.find_last_of(0x2F);
            if (lastpos != std::string::npos)
            {
                page.file.filename = path.substr(lastpos + 1);
                if (page.file.filename.size() > 120)
                {
                    page.file.filename.resize(120);
                }
            }
        }
    }
}
void client::responseheader(std::string_view key, std::string_view value)
{
    // content-encoding
    switch (key.size())
    {
    case 19:

        if (str_casecmp(key, "Content-Disposition"))
        {
            respattachmentprocess(value);
        }
        break;
    case 17:
        if (str_casecmp(key, "Transfer-Encoding"))
        {
            std::string temp_str;
            for (unsigned int k = 0; k < value.size(); k++)
            {
                if (value[k] == ',')
                {
                    if (str_casecmp(temp_str, "chunked"))
                    {
                        page.chunked = true;
                    }
                    else if (str_casecmp(temp_str, "gzip"))
                    {
                        page.encode = 'g';
                    }
                    else if (str_casecmp(temp_str, "zstd"))
                    {
                        page.encode = 'z';
                    }
                    else if (str_casecmp(temp_str, "deflate"))
                    {
                        page.encode = 'd';
                    }
                    temp_str.clear();
                    continue;
                }
                else if (value[k] == ' ')
                {
                    continue;
                }
                temp_str.push_back(value[k]);
            }
            if (temp_str.size() > 0)
            {
                if (str_casecmp(temp_str, "chunked"))
                {
                    page.chunked = true;
                }
                else if (str_casecmp(temp_str, "gzip"))
                {
                    page.encode = 'g';
                }
                else if (str_casecmp(temp_str, "zstd"))
                {
                    page.encode = 'z';
                }
                else if (str_casecmp(temp_str, "deflate"))
                {
                    page.encode = 'd';
                }
            }
        }
        break;
    case 16:

        if (str_casecmp(key, "Content-Encoding"))
        {
            switch (value.size())
            {
            case 2:
                if (value[0] == 'b')
                {
                    // page.br=true;
                    page.encode = 'b';
                }

                break;
            case 4:
                if (value[0] == 'g')
                {
                    //  page.gzip=true;
                    page.encode = 'g';
                }
                else if (value[0] == 'z')
                {
                    //  page.zstd=true;
                    page.encode = 'z';
                }
                break;
            case 7:
                if (value[0] == 'd')
                {
                    //     page.deflate=true;
                    page.encode = 'd';
                }

                break;
            default:;
            }
        }
        break;
    case 14:
        if (str_casecmp(key, "Content-Length"))
        {
            try
            {
                page.file.size = std::stoi(value.data());
            }
            catch (...)
            {
                page.file.size = 0;
            }
        }
        break;
    case 12:
        if (str_casecmp(key, "Content-Type"))
        {
            respcontenttypeprocess(value);
        }
        break;
    case 10:
        if (str_casecmp(key, "Connection"))
        {
            // Keep-Alive
            if (value[0] == 'K' || value[0] == 'k')
            {
                //    if(value[1]=='e'&&value[2]=='e'){
                page.keeplive = true;
                //     }
            }
            else
            {
                page.keeplive = false;
            }
        }
        break;
    }
}
unsigned int client::get_length() { return page.file.size; }
unsigned int client::get_status() { return page.code; }
std::string client::get_status_msg() { return page.codemessage; }
std::string client::get_header() { return response_header; }
std::string client::get_tempfile() { return page.file.tempfile; }
std::map<std::string, std::string> client::get_headers() { return page.header; }
std::string client::get_type() { return page.file.type; }
client &client::add_post(std::string_view key, std::string_view value)
{
    postdata.emplace_back(key, value);
    return *this;
}
client &client::set_body(std::string_view value)
{
    page.content = value;
    return *this;
}
client &client::set_body(const std::string &value)
{
    page.content = value;
    return *this;
}
client &client::set_body(std::string &&value)
{
    page.content = std::move(value);
    return *this;
}
std::string client::get_body()
{
    if (page.istxt)
    {
        return page.content;
    }
    else
    {
        // FILE *ffp = fopen(page.file.tempfile.c_str(), "rb");
        std::unique_ptr<std::FILE, int (*)(FILE *)> ffp(fopen(page.file.tempfile.c_str(), "rb"), std::fclose);
        if (!ffp)
        {
            return "";
        }
        fseek(ffp.get(), 0, SEEK_END);
        unsigned int nsize = ftell(ffp.get());
        fseek(ffp.get(), 0, SEEK_SET);

        page.content.resize(nsize);

        unsigned int nread = fread(&page.content[0], 1, nsize, ffp.get());
        page.content.resize(nread);
        // fclose(ffp);
    }
    return page.content;
}
const std::string &client::ref_body()
{
    if (page.istxt)
    {
        return page.content;
    }
    else
    {
        // FILE *ffp = fopen(page.file.tempfile.c_str(), "rb");
        std::unique_ptr<std::FILE, int (*)(FILE *)> ffp(fopen(page.file.tempfile.c_str(), "rb"), std::fclose);
        if (!ffp)
        {
            return page.content;
        }
        fseek(ffp.get(), 0, SEEK_END);
        unsigned int nsize = ftell(ffp.get());
        fseek(ffp.get(), 0, SEEK_SET);

        page.content.resize(nsize);

        unsigned int nread = fread(&page.content[0], 1, nsize, ffp.get());
        page.content.resize(nread);
        // fclose(ffp);
    }
    return page.content;
}

http::cookie client::get_cookie() { return page.cookie; }
std::string &client::get_cookie(const std::string &cookie_key) { return page.cookie[cookie_key]; }
http::obj_val client::json() { return page.json; }
void client::respreadtocontent(const char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;
    // unsigned int offset = buffersize - i;
    if (!page.chunked)
    {
        for (; i < buffersize; i++)
        {
            page.content.push_back(buffer[i]);
        }
        return;
    }

    while (i < buffersize)
    {
        if (page.length == 0)
        {
            //是尾部或一个块开始
            unsigned int n = 0;
            for (; i < buffersize; i++)
            {
                if (buffer[i] == 0x0D || buffer[i] == 0x0A)
                {
                    break;
                }
                if (buffer[i] >= '0' && buffer[i] <= '9')
                {
                    n = n << 4;
                    n = n + (unsigned char)(buffer[i] - '0');
                }
                else if (buffer[i] >= 'A' && buffer[i] <= 'F')
                {
                    n = n << 4;
                    n = n + (unsigned char)(buffer[i] - 'A') + 10;
                }
                else if (buffer[i] >= 'a' && buffer[i] <= 'f')
                {
                    n = n << 4;
                    n = n + (unsigned char)(buffer[i] - 'a') + 10;
                }
            }
            page.length = n;
            if (page.length == 0)
            {
                //尾部了
                page.file.size = page.content.size();
                for (; i < buffersize; i++)
                {
                    if (buffer[i] == 0x0D || buffer[i] == 0x0A)
                    {
                        continue;
                    }
                    break;
                }
                readoffset = i;
                return;
            }

            //后面必须是\r\n
            if (i < buffersize && buffer[i] == 0x0D)
            {
                i++;
                if (i < buffersize && buffer[i] == 0x0A)
                {
                    i++;
                }
                else
                {
                    if (i < buffersize)
                    {
                        iserror = 1;
                    }
                    page.padd = 1;
                    break;
                }
            }
            else
            {
                if (i < buffersize)
                {
                    iserror = 1;
                    break;
                }
                page.padd = 2;
                break;
            }
        }

        if (page.padd == 2)
        {
            //跨包
            if (i < buffersize && buffer[i] == 0x0D)
            {
                i++;
                if (i < buffersize && buffer[i] == 0x0A)
                {
                    i++;
                }
                else
                {
                    iserror = 1;
                    return;
                }
            }
            else
            {
                iserror = 1;
                return;
            }
            page.padd = 0;
        }
        else if (page.padd == 1)
        {
            //跨包
            if (i < buffersize && buffer[i] == 0x0A)
            {
                i++;
            }
            else
            {
                iserror = 1;
                return;
            }
            page.padd = 0;
        }

        if (page.length > 0)
        {
            for (; i < buffersize; i++)
            {
                page.content.push_back(buffer[i]);
                page.length--;
                if (page.length == 0)
                {
                    //下一个循环，消化下一个块
                    break;
                }
            }
        }
        else
        {
            //防止死循环
            i++;
        }
    }
}

void client::respreadtofile(const char *buffer, unsigned int buffersize)
{

    unsigned int i      = readoffset;
    unsigned int offset = 0;

    if (i >= buffersize)
    {
        i = buffersize;
    }
    offset = buffersize - i;
    if (offset > buffersize)
    {
        offset = 0;
    }
    if (!rawfile)
    {
        unsigned long long tetime = time((time_t *)NULL);
        srand(tetime);
        unsigned long long randnum = rand() % 100000 + 100000;

        page.file.tempfile = "c" + std::to_string(tetime) + std::to_string(randnum);
        page.file.tempfile.append("_t");

        struct stat s;
        std::string tempfilename;
        if (temppath.empty())
        {
            temppath = "./temp";
        }
        if (stat(temppath.c_str(), &s) == 0)
        {

            if (s.st_mode & S_IFDIR)
            {
                if (temppath.back() == '/')
                {
                    tempfilename = temppath;
                    tempfilename.append(page.file.tempfile);
                    page.file.tempfile = tempfilename;
                }
                else
                {
                    tempfilename = temppath;
                    tempfilename.push_back('/');
                    tempfilename.append(page.file.tempfile);
                    page.file.tempfile = tempfilename;
                }
            }
        }

        // rawfile = fopen(page.file.tempfile.c_str(), "wb");
        rawfile.reset(fopen(page.file.tempfile.c_str(), "wb"));
    }
    if (i < buffersize && rawfile)
    {
        fwrite(&buffer[i], offset, 1, rawfile.get());
    }
}
void client::finishprocess()
{
    if (rawfile)
    {
        rawfile.reset(nullptr);
        // fclose(rawfile);
        // rawfile = NULL;
    }
    if (page.encode == 'g')
    {
        std::string content;
        uncompress(page.content, content);
        page.content = std::move(content);
    }
    if (page.isjson || parsetojson == 1)
    {
        page.json.from_json(page.content);
        if (parsetojson == 1)
            page.isjson = true;
    }
}
bool client::process(const char *buffer, unsigned int buffersize)
{
    if (readoffset >= buffersize)
    {
        return true;
    }
    if (iserror > 0)
    {
        return true;
    }
    if (headerfinish == 0)
    {
        for (; readoffset < buffersize;)
        {
            readheaderline(buffer, buffersize);
            if (iserror > 0)
            {
                break;
            }
            if (headerfinish == 1)
            {
                page.length = 0;
                if (onheader != nullptr)
                {
                    if (onheader(buffer, readoffset, page.code))
                    {
                        return true;
                    }
                }
                break;
            }
        }
    }
    if (headerfinish == 1 && iserror == 0 && readoffset < buffersize)
    {
        // type length chunked
        if (page.istxt)
        {
            respreadtocontent(buffer, buffersize);
        }
        else
        {
            page.length = page.length + (buffersize - readoffset);
            respreadtofile(buffer, buffersize);
            if (download_process != nullptr)
            {
                download_process(page.length, page.file.size);
            }
        }
    }
    return false;
}
client &client::data_type(std::string str)
{

    if (str.empty())
    {
        parsetojson = 1;
    }
    else
    {
        if (str == "json")
        {
            parsetojson = 1;
        }
    }
    return *this;
}
client &client::post_type(std::string str)
{

    if (str.empty())
    {
        header["Content-Type"] = "application/x-www-form-urlencoded";
    }
    else
    {
        header["Content-Type"] = str;
    }
    return *this;
}
client &client::save(std::string path_file)
{

    // fix prepath safe path
    if (path_file.empty())
    {
        path_file.append("./");
        if (page.file.filename.empty())
        {
            unsigned long long tetime = time((time_t *)NULL);
            page.file.filename        = std::to_string(tetime);
        }
        else
        {
            path_file.append(page.file.filename);
        }
    }

    struct stat s_f;
    if (stat(path_file.c_str(), &s_f) == 0)
    {

        if (s_f.st_mode & S_IFDIR)
        {
            if (path_file.back() == '/')
            {
                path_file.append(page.file.filename);
            }
            else
            {
                path_file.push_back('/');
                path_file.append(page.file.filename);
            }
        }
    }
    if (page.file.tempfile.size() > 0 && page.file.tempfile.size() < 138)
    {
        rename(page.file.tempfile.c_str(), path_file.c_str());
    }

    return *this;
}

client &client::set_header(std::string key, std::string value)
{

    header[key] = value;
    return *this;
}
client &client::add_header(std::string key, std::string value)
{

    header[key] = value;
    return *this;
}
client &client::add_header(std::string vvalue)
{
    std::string key;
    std::string value;

    for (unsigned int j = 0; j < vvalue.size(); j++)
    {
        if (vvalue[j] == 0x0A)
        {
            continue;
        }
        if (vvalue[j] == 0x0D)
        {
            header[key] = value;
            key.clear();
            value.clear();
        }
        else if (vvalue[j] == ':')
        {
            key = value;
            value.clear();
        }
        else
        {
            value.push_back(vvalue[j]);
        }
    }

    if (key.size() > 0 && value.size() > 0)
    {
        header[key] = value;
    }

    return *this;
}
bool client::parse()
{
    if (_url.length() == 0)
    {
        return false;
    }
    unsigned int i = 0;
    for (; i < _url.length(); i++)
    {
        if (_url[i] != 0x20)
        {
            break;
        }
    }
    scheme.clear();
    host.clear();
    port.clear();
    path.clear();
    query_.clear();

    if (_url[i] == 'h' && _url[i + 1] == 't' && _url[i + 2] == 't' && _url[i + 3] == 'p' && _url[i + 4] == ':')
    {
        scheme = "http";
        isssl  = false;
        i += 5;
    }
    if (_url[i] == 'h' && _url[i + 1] == 't' && _url[i + 2] == 't' && _url[i + 3] == 'p' && _url[i + 4] == 's' &&
        _url[i + 5] == ':')
    {
        scheme = "https";
        isssl  = true;
        i += 6;
    }
    if (scheme.empty())
    {
        scheme = "http";
        isssl  = false;
    }
    for (; i < _url.length(); i++)
    {
        if (_url[i] == 0x2F)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    for (; i < _url.length(); i++)
    {
        if (_url[i] == 0x2F || _url[i] == 0x3A || _url[i] == 0x3F)
        {

            break;
        }
        host.push_back(_url[i]);
    }

    if (_url[i] == 0x3A)
    {
        i++;
        for (; i < _url.length(); i++)
        {
            if (_url[i] == 0x2F)
            {
                break;
            }
            port.push_back(_url[i]);
        }
    }

    if (_url[i] == 0x3F)
    {
        path = "/";
        i++;
        for (; i < _url.length(); i++)
        {
            query_.push_back(_url[i]);
        }
    }
    else if (_url[i] != 0x2F)
    {
        path = "/";
    }
    else
    {
        for (; i < _url.length(); i++)
        {
            if (_url[i] == 0x3F)
            {
                break;
            }
            path.push_back(_url[i]);
        }

        if (_url[i] == 0x3F)
        {
            i++;
            for (; i < _url.length(); i++)
            {
                query_.push_back(_url[i]);
            }
        }
    }
    if (port.empty())
    {
        if (scheme == "https")
        {
            port = "443";
        }
        else
        {
            port = "80";
        }
    }

    transform(host.begin(), host.end(), host.begin(), ::tolower);
    return true;
}

client &client::build_query(const std::map<std::string, std::string> &param)
{
    bool isfirst = true;
    std::string tempport;
    for (auto [first, second] : param)
    {
        if (isfirst)
        {
            isfirst = false;
            if (query_.size() > 0)
            {
                query_.push_back('&');
            }
        }
        else
        {
            query_.push_back('&');
        }
        {
            tempport = url_encode(first.data(), first.size());
            query_.append(tempport);
            tempport = url_encode(second.data(), second.size());
            query_.push_back('=');
            query_.append(tempport);
        }
    }
    return *this;
}

client &client::build_query(http::obj_val param)
{
    bool isfirst = true;
    std::string tempport;
    for (auto &[first, second] : param.as_object())
    {
        if (isfirst)
        {
            isfirst = false;
            if (query_.size() > 0)
            {
                query_.push_back('&');
            }
        }
        else
        {
            query_.push_back('&');
        }
        if (!second.is_array())
        {
            tempport = first;
            tempport = url_encode(tempport.data(), tempport.size());
            query_.append(tempport);
            tempport = second.to_string();
            tempport = url_encode(tempport.data(), tempport.size());
            query_.push_back('=');

            query_.append(tempport);
        }
    }
    return *this;
}

client &client::build_query(const std::string &a)
{
    query_.append(url_encode(a.data(), a.size()));
    return *this;
}

std::string client::get_query()
{
    return query_;
}

void client::buildheader()
{
    sendcontent.str("");
    request.clear();
    std::string tempport;
    if (requesttype == 0)
    {
        request.append("GET ");
        request.append(path);
        if (!query_.empty())
        {
            request.push_back('?');
            request.append(query_);
        }
        request.append(" HTTP/1.1\r\n");
    }
    else
    {
        if (requesttype > 1)
        {
            request.append("QUERY ");
        }
        else
        {
            request.append("POST ");
        }
        request.append(path);
        if (query_.empty())
        {
            request.append(" HTTP/1.1\r\n");
        }
        else
        {
            request.append("?");
            request.append(query_);
            request.append(" HTTP/1.1\r\n");
        }
    }

    request.append("Host: ");
    request.append(host);

    tempport = port;
    if (!port.empty())
    {
        if (scheme == "https")
        {

            if (port == "443")
            {
                tempport.clear();
            }
        }
        else
        {
            if (port == "80")
            {
                tempport.clear();
            }
        }
    }

    if (tempport.empty())
    {
        request.append("\r\n");
    }
    else
    {
        request.push_back(':');
        request.append(tempport);
        request.append("\r\n");
    }

    if (header.isset("User-Agent") || header.isset("user-agent"))
    {

        request.append("User-Agent: ");
        request.append(header["User-Agent"].as_string());
        request.append("\r\n");

        header.unset("User-Agent");
    }
    else
    {
        request.append("User-Agent: httpclient\r\n");
    }

    if (!header.isset("Accept") && !header.isset("accept"))
    {
        header["Accept"] = "text/html, application/xhtml+xml, application/json, application/xml;q=0.9, */*;q=0.8";
    }

    for (auto &[first, second] : header.as_object())
    {
        if (!second.is_array())
        {
            request.append(first);
            request.append(": ");
            request.append(second.to_string());
            request.append("\r\n");
        }
    }
    if (!header.isset("Connection"))
    {
        request.append("Connection: close\r\n");
    }
    if (cookie.size() > 0)
    {
        request.append("Cookie: ");
        unsigned ci = 0;
        for (auto &[first, second] : cookie)
        {
            if (ci > 0)
            {
                request.append(";");
            }
            request.append(url_encode(first.data(), first.size()));
            request.append("=");
            request.append(url_encode(second.data(), second.size()));
            ci++;
        }
        request.append("\r\n");
    }

    request.append("\r\n");
    if (onrequest != nullptr)
    {
        onrequest(request);
    }
}
client &client::add_cookie(const std::string &k, const std::string &v)
{
    cookie[k] = v;
    return *this;
}
client &client::add_file(std::string filename)
{
    std::string key = "upfile[]";
    assign_file(std::move(key), std::move(filename));
    return *this;
}
client &client::add_file(std::string key, std::string filename)
{
    assign_file(std::move(key), std::move(filename));
    return *this;
}
void client::assign_file(std::string key, std::string filename)
{
    upload_file temp;
    temp.name     = key;
    temp.filename = filename;

    std::string::size_type found = filename.find_last_of('.');
    std::string extfile;
    if (found != std::string::npos)
    {
        extfile = filename.substr(found + 1);
        transform(extfile.begin(), extfile.end(), extfile.begin(), ::tolower);

        temp.type = mime_map[extfile];
        if (temp.type.empty())
        {
            temp.type = "application/octet-stream";
        }
    }
    else
    {
        temp.type = "application/octet-stream";
    }

    // FILE *f = fopen(filename.c_str(), "rb");
    std::unique_ptr<std::FILE, int (*)(FILE *)> f(fopen(filename.c_str(), "rb"), std::fclose);
    if (f)
    {
        fseek(f.get(), 0, SEEK_END);
        temp.size = ftell(f.get());
        fseek(f.get(), 0, SEEK_SET);

        // fclose(f);
    }
    else
    {
        temp.size = 0;
    }

    files.emplace_back(std::move(temp));
    header["Content-Type"] = "multipart/form-data";
}
void client::buildcontent()
{

    if (!header.isset("Content-Type"))
    {
        if (files.size() == 0)
        {
            contenttype            = "application/x-www-form-urlencoded";
            header["Content-Type"] = "application/x-www-form-urlencoded";
        }
        else
        {
            contenttype = "multipart/form-data";
        }
    }
    else
    {
        contenttype = header["Content-Type"].as_string();
    }

    if (contenttype == "multipart/form-data")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000000, 1999999);
        unsigned int rdnum         = dis(gen);
        unsigned long long randnum = rdnum * 1000000;
        rdnum                      = dis(gen);
        randnum                    = randnum + rdnum;
        rdnum                      = time((time_t *)NULL);

        boundary = "----------paozhuhttpclient" + std::to_string(randnum) + std::to_string(rdnum);

        header["Content-Type"] = "multipart/form-data; boundary=" + boundary;
    }

    unsigned int beginpos      = 0;
    unsigned int contentlength = 0;
    upload_file ptemp;
    // FILE *fp;
    if (str_casecmp(contenttype, "application/x-www-form-urlencoded"))
    {
        beginpos      = 0;
        contentlength = 0;
        ptemp.error   = 3;
        ptemp.tempfile.clear();
        ptemp.filename.clear();
        ptemp.name.clear();
        ptemp.size = 0;
        ptemp.type.clear();

        if (postdata.size() > 0)
        {
            for (auto &[first, second] : postdata)
            {
                if (beginpos > 0)
                {
                    ptemp.tempfile.push_back('&');
                }
                ptemp.tempfile.append(url_encode(first.data(), first.size()));
                ptemp.tempfile.push_back('=');
                ptemp.tempfile.append(url_encode(second.data(), second.size()));
                beginpos++;
            }
        }
        else
        {
            if (page.content.size() > 0)
            {
                ptemp.tempfile.append(page.content);
            }
        }
        contentlength = ptemp.tempfile.size();
        senddata.push_back(ptemp);
    }
    else if (str_casecmp(contenttype, "multipart/form-data"))
    {
        beginpos      = 0;
        contentlength = 0;
        ptemp.error   = 0;
        ptemp.tempfile.clear();
        ptemp.filename.clear();
        ptemp.name.clear();
        ptemp.size = 0;
        ptemp.type.clear();
        for (auto &[first, second] : postdata)
        {
            ptemp.error = 0;
            ptemp.tempfile.clear();
            ptemp.filename.clear();
            ptemp.name.clear();
            ptemp.size = 0;
            ptemp.type.clear();
            ptemp.tempfile.append("--");
            ptemp.tempfile.append(boundary);
            ptemp.tempfile.append("\r\n");

            ptemp.tempfile.append("Content-Disposition: form-data; name=\"");
            ptemp.tempfile.append(first);
            ptemp.tempfile.append("\"\r\n\r\n");

            ptemp.tempfile.append(second);
            ptemp.tempfile.append("\r\n");
            contentlength += ptemp.tempfile.size();
            senddata.push_back(ptemp);
        }

        if (files.size() > 0)
        {
            for (auto finfo : files)
            {
                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();
                if (finfo.size > 16384)
                {
                    ptemp.error    = 1;
                    ptemp.filename = finfo.filename;
                    ptemp.name     = finfo.name;
                    ptemp.size     = finfo.size;
                    ptemp.type     = finfo.type;

                    ptemp.tempfile.append("--");
                    ptemp.tempfile.append(boundary);
                    ptemp.tempfile.append("\r\n");

                    ptemp.tempfile.append("Content-Disposition: form-data; name=\"");
                    ptemp.tempfile.append(finfo.name);
                    ptemp.tempfile.append("\"; filename=\"");
                    ptemp.tempfile.append(finfo.filename);
                    ptemp.tempfile.append("\"\r\nContent-Type: ");
                    ptemp.tempfile.append(finfo.type);
                    ptemp.tempfile.append("\r\n\r\n");
                    contentlength += ptemp.tempfile.size() + finfo.size + 2;
                }
                else
                {
                    ptemp.error    = 0;
                    ptemp.filename = finfo.filename;
                    ptemp.name     = finfo.name;
                    ptemp.size     = finfo.size;
                    ptemp.type     = finfo.type;

                    ptemp.tempfile.append("--");
                    ptemp.tempfile.append(boundary);
                    ptemp.tempfile.append("\r\n");

                    ptemp.tempfile.append("Content-Disposition: form-data; name=\"");
                    ptemp.tempfile.append(finfo.name);
                    ptemp.tempfile.append("\"; filename=\"");
                    ptemp.tempfile.append(finfo.filename);
                    ptemp.tempfile.append("\"\r\nContent-Type: ");
                    ptemp.tempfile.append(finfo.type);
                    ptemp.tempfile.append("\r\n\r\n");
                    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(finfo.filename.c_str(), "rb"), std::fclose);
                    if (fp)
                    {
                        unsigned int n = ptemp.tempfile.size();
                        ptemp.tempfile.resize(n + ptemp.size);
                        n = fread(&ptemp.tempfile[n], 1, ptemp.size, fp.get());

                        // fclose(fp);
                    }
                    ptemp.tempfile.append("\r\n");
                    contentlength += ptemp.tempfile.size();
                }
                senddata.push_back(ptemp);
            }
        }

        ptemp.error = 0;
        ptemp.tempfile.clear();
        ptemp.filename.clear();
        ptemp.name.clear();
        ptemp.size = 0;
        ptemp.type.clear();

        ptemp.tempfile.append("--");
        ptemp.tempfile.append(boundary);
        ptemp.tempfile.append("--");
        ptemp.size = ptemp.tempfile.size();
        contentlength += ptemp.size;
        senddata.push_back(ptemp);
    }
    else if (str_casecmp(contenttype, "application/octet-stream"))
    {
        if (files.size() > 0)
        {
            ptemp.error = 0;
            ptemp.tempfile.clear();
            ptemp.filename.clear();
            ptemp.name.clear();
            ptemp.size = 0;
            ptemp.type.clear();

            ptemp.size = files.front().size;
            if (ptemp.size < 32768)
            {
                // fp = fopen(files.front().filename.c_str(), "rb");
                std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(files.front().filename.c_str(), "rb"), std::fclose);
                if (fp)
                {
                    ptemp.tempfile.resize(ptemp.size);
                    unsigned int n = fread(&ptemp.tempfile[0], 1, ptemp.size, fp.get());
                    ptemp.tempfile.resize(n);
                    // fclose(fp);
                }

                contentlength = ptemp.tempfile.size();
            }
            else
            {
                ptemp.error    = 2;
                ptemp.filename = files.front().filename;
                contentlength  = ptemp.size;
            }
            senddata.push_back(ptemp);
        }
        else
        {
            if (page.content.size() > 0)
            {
                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();

                ptemp.size     = page.content.size();
                ptemp.tempfile = page.content;
                contentlength  = page.content.size();
                senddata.push_back(ptemp);
            }
        }
    }
    else if (str_casecmp(contenttype, "application/json"))
    {
        ptemp.error = 0;
        ptemp.tempfile.clear();
        ptemp.filename.clear();
        ptemp.name.clear();
        ptemp.size = 0;
        ptemp.type.clear();

        ptemp.size     = page.content.size();
        ptemp.tempfile = page.content;
        contentlength  = page.content.size();
        senddata.push_back(ptemp);
    }
    else if (str_casecmp(contenttype, "application/xml"))
    {
        ptemp.error = 0;
        ptemp.tempfile.clear();
        ptemp.filename.clear();
        ptemp.name.clear();
        ptemp.size = 0;
        ptemp.type.clear();

        ptemp.size     = page.content.size();
        ptemp.tempfile = page.content;
        contentlength  = page.content.size();
        senddata.push_back(ptemp);
    }
    else if (str_casecmp(contenttype, "text/xml"))
    {
        ptemp.error = 0;
        ptemp.tempfile.clear();
        ptemp.filename.clear();
        ptemp.name.clear();
        ptemp.size = 0;
        ptemp.type.clear();

        ptemp.size     = page.content.size();
        ptemp.tempfile = page.content;
        contentlength  = page.content.size();
        senddata.push_back(ptemp);
    }
    else
    {
        if (files.size() > 0)
        {
            ptemp.error = 0;
            ptemp.tempfile.clear();
            ptemp.filename.clear();
            ptemp.name.clear();
            ptemp.size = 0;
            ptemp.type.clear();

            ptemp.size = files.front().size;
            if (ptemp.size < 32768)
            {
                // fp = fopen(files.front().filename.c_str(), "rb");
                std::unique_ptr<std::FILE, int (*)(FILE *)> fp(fopen(files.front().filename.c_str(), "rb"), std::fclose);
                if (fp)
                {
                    ptemp.tempfile.resize(ptemp.size);
                    unsigned int n = fread(&ptemp.tempfile[0], 1, ptemp.size, fp.get());
                    ptemp.tempfile.resize(n);
                    // fclose(fp);
                }
                contentlength = ptemp.tempfile.size();
            }
            else
            {
                ptemp.error    = 2;
                ptemp.filename = files.front().filename;
                contentlength  = ptemp.size;
            }
            senddata.push_back(ptemp);
        }
        else
        {
            ptemp.error = 0;
            ptemp.tempfile.clear();
            ptemp.filename.clear();
            ptemp.name.clear();
            ptemp.size = 0;
            ptemp.type.clear();

            ptemp.size     = page.content.size();
            ptemp.tempfile = page.content;
            contentlength  = page.content.size();
            senddata.push_back(ptemp);
        }
    }
    header["Content-Length"] = contentlength;
}

bool client::connect(std::string_view url, unsigned int time_out_num)
{
    requesttype = 0;
    if (url.length() < 3)
    {
        iserror   = true;
        error_msg = "url to short";
        return false;
    }
    exptime = time_out_num;
    _url    = url;
    parse();

    buildheader();

    if (exptime > 90)
    {
        exptime = 90;
    }

    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.timeout_lists.push_back(shared_from_this());
            temp_io_context.timeout_condition.notify_one();
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            return false;
        }
    }

    if (scheme == "https")
    {
        return init_https_sock();
    }
    else
    {
        return init_http_sock();
    }

    return false;
}
bool client::connect()
{
    requesttype = 0;
    if (_url.length() < 3)
    {
        iserror   = true;
        error_msg = "url to short";
        return false;
    }

    buildheader();

    if (exptime > 90)
    {
        exptime = 90;
    }

    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.timeout_lists.push_back(shared_from_this());
            temp_io_context.timeout_condition.notify_one();
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            return false;
        }
    }

    if (scheme == "https")
    {
        return init_https_sock();
    }
    else
    {
        return init_http_sock();
    }

    return false;
}
asio::awaitable<bool> client::async_connect(std::string_view url, unsigned int time_out_num)
{
    requesttype = 0;
    if (url.length() < 3)
    {
        iserror   = true;
        error_msg = "url to short";
        co_return false;
    }
    exptime = time_out_num;
    _url    = url;
    parse();

    buildheader();

    if (exptime > 90)
    {
        exptime = 90;
    }

    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.timeout_lists.push_back(shared_from_this());
            temp_io_context.timeout_condition.notify_one();
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            co_return false;
        }
    }

    if (scheme == "https")
    {
        co_return co_await async_init_https_sock();
    }
    else
    {
        co_return co_await async_init_http_sock();
    }
    co_return false;
}
asio::awaitable<bool> client::async_connect()
{
    requesttype = 0;
    if (_url.length() < 3)
    {
        iserror   = true;
        error_msg = "url to short";
        co_return false;
    }

    buildheader();

    if (exptime > 90)
    {
        exptime = 90;
    }

    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.timeout_lists.push_back(shared_from_this());
            temp_io_context.timeout_condition.notify_one();
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            co_return false;
        }
    }

    if (scheme == "https")
    {
        co_return co_await async_init_https_sock();
    }
    else
    {
        co_return co_await async_init_http_sock();
    }

    co_return false;
}

asio::awaitable<unsigned int> client::async_read(unsigned char *buffer_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        co_return 0;
    }
    atomic_guard guard{socket_read_lock};
    if (iserror)
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await sslsock->async_read_some(asio::buffer(buffer_data, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await sock->async_read_some(asio::buffer(buffer_data, buffersize), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    socket_read_lock.clear();
    co_return 0;
}

asio::awaitable<unsigned int> client::async_read(std::string &buffer_data)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        co_return 0;
    }
    atomic_guard guard{socket_read_lock};
    if (iserror)
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await sslsock->async_read_some(asio::buffer(buffer_data), asio::use_awaitable);
        }
        else
        {
            n = co_await sock->async_read_some(asio::buffer(buffer_data), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    co_return 0;
}

asio::awaitable<unsigned int> client::async_write(unsigned char *send_data_p, unsigned int buffersize)
{

    if (iserror)
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await asio::async_write(*sslsock, asio::buffer(send_data_p, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*sock, asio::buffer(send_data_p, buffersize), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    co_return 0;
}

asio::awaitable<unsigned int> client::async_write(std::string_view value)
{
    if (iserror)
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }

    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await asio::async_write(*sslsock, asio::buffer(value), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*sock, asio::buffer(value), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    co_return 0;
}

unsigned int client::write(unsigned char *send_data_p, unsigned int buffersize)
{
    if (iserror)
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = asio::write(*sslsock, asio::buffer(send_data_p, buffersize));
        }
        else
        {
            n = asio::write(*sock, asio::buffer(send_data_p, buffersize));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    return 0;
}

unsigned int client::write(std::string_view value)
{
    if (iserror)
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }

    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = asio::write(*sslsock, asio::buffer(value));
        }
        else
        {
            n = asio::write(*sock, asio::buffer(value));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    return 0;
}

unsigned int client::read(unsigned char *buffer_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        return 0;
    }
    atomic_guard guard{socket_read_lock};
    if (iserror)
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = sslsock->read_some(asio::buffer(buffer_data, buffersize));
        }
        else
        {
            n = sock->read_some(asio::buffer(buffer_data, buffersize));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    return 0;
}

unsigned int client::read(std::string &buffer_data)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        return 0;
    }
    atomic_guard guard{socket_read_lock};
    if (iserror)
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = sslsock->read_some(asio::buffer(buffer_data));
        }
        else
        {
            n = sock->read_some(asio::buffer(buffer_data));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    return 0;
}

}// namespace http
