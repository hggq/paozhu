/**
 *  @copyright copyright 2026, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file http rpc client.cpp
 *  @date 2026-05-10
 *
 *  http rpc client file
 *
 *
 */
#include <memory>
#include <string>
#include "server_localvar.h"
#include "http_rpcclient.h"
#include "client_context.h"
#include "common/cost_define.h"// CONST_RPC_KV_MAX_ITEMS / CONST_RPC_KV_MAX_BYTES
#include "func.h"

namespace http
{
rpc_client::rpc_client() : uprawfile(nullptr, std::fclose), strand_(asio::make_strand(*(get_client_context_obj().ioc))) { iserror = false; }
rpc_client::~rpc_client()
{
    if (data != nullptr)
    {
        std::free(data);
    }
}
void rpc_client::set_header(std::string_view name, std::string_view value)
{
    rpc_parameter_t a;
    a.name  = name;
    a.value = value;
    parameter.emplace_back(a);
}

void rpc_client::set_host(std::string_view name)
{
    host = name;
}

void rpc_client::close_connect()
{
    if (isssl)
    {
        if (sslsock)
        {
            if (sslsock->lowest_layer().is_open())
            {
                sslsock->lowest_layer().cancel(ec);
                sslsock->lowest_layer().close(ec);
            }
        }
    }
    else
    {
        if (sock && sock->is_open())
        {
            sock->close(ec);
        }
    }
}

void rpc_client::set_url(std::string_view name)
{
    if (name.size() > 7)
    {
        if (name[0] == 'h' && name[1] == 't' && name[2] == 't' && name[3] == 'p' && name[4] == ':' && name[5] == '/' && name[6] == '/')
        {
            std::string temp_str;
            unsigned int i = 7;
            for (; i < name.size(); i++)
            {
                if (name[i] == '/')
                {
                    if (temp_str.size() > 0)
                    {
                        host = temp_str;
                    }
                    temp_str.clear();
                    break;
                }
                else if (name[i] == ':')
                {
                    if (temp_str.size() > 0)
                    {
                        host = temp_str;
                    }
                    temp_str.clear();
                    port = 0;
                    for (; i < name.size(); i++)
                    {
                        if (name[i] == '/')
                        {
                            break;
                        }
                        if (name[i] >= '0' && name[i] <= '9')
                        {
                            port = port * 10 + (name[i] - '0');
                        }
                    }
                    break;
                }
                temp_str.push_back(name[i]);
            }
            for (; i < name.size(); i++)
            {
                url.push_back(name[i]);
            }
        }
        else if (name[0] == 'h' && name[1] == 't' && name[2] == 't' && name[3] == 'p' && name[4] == 's' && name[5] == ':' && name[6] == '/' && name[7] == '/')
        {
            std::string temp_str;
            unsigned int i = 8;
            isssl          = true;
            for (; i < name.size(); i++)
            {
                if (name[i] == '/')
                {
                    if (temp_str.size() > 0)
                    {
                        host = temp_str;
                    }

                    break;
                }
                else if (name[i] == ':')
                {
                    if (temp_str.size() > 0)
                    {
                        host = temp_str;
                    }

                    port = 0;
                    for (; i < name.size(); i++)
                    {
                        if (name[i] == '/')
                        {
                            break;
                        }
                        if (name[i] >= '0' && name[i] <= '9')
                        {
                            port = port * 10 + (name[i] - '0');
                        }
                    }
                    break;
                }
                temp_str.push_back(name[i]);
            }
            for (; i < name.size(); i++)
            {
                url.push_back(name[i]);
            }
        }
        else
        {
            url = name;
        }
    }
    else
    {
        url = name;
    }
}
asio::awaitable<void> rpc_client::async_send()
{
    std::string send_content;
    send_content.append("rpc");
    send_content.push_back(0x20);
    send_content.append(url);
    send_content.push_back(0x0A);
    send_content.append("HOST ");

    // 长度字段只有两字节，超长直接失败，不截断
    if (host.size() > 0xFFFF)
    {
        iserror = true;
        co_return;
    }
    unsigned short value_size_h = static_cast<unsigned short>(host.size());
    unsigned char aa            = value_size_h & 0xFF;

    value_size_h     = value_size_h & 0xFF00;
    unsigned char bb = value_size_h >> 8;
    send_content.push_back(bb);
    send_content.push_back(aa);

    send_content.append(host);
    send_content.push_back(0x0A);

    send_content.append("size ");

    if (content.size() <= 0xFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = static_cast<unsigned short>(content.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x04);

        unsigned int value_size = static_cast<unsigned int>(content.size());
        unsigned char a;
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x05);

        unsigned long long value_size = content.size();
        unsigned char a;
        a = (value_size & 0xFF00000000) >> 32;
        send_content.push_back(a);
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else
    {
        iserror = true;
        co_return;
    }
    send_content.push_back(0x0A);

    send_content.append("type ");

    if (content_type.size() == 0)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if (content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");
            iserror = true;
            co_return;
        }

        unsigned short value_size = static_cast<unsigned short>(content_type.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(content_type);
    }
    send_content.push_back(0x0A);

    for (unsigned int i = 0; i < parameter.size(); i++)
    {
        // 长度字段只有两字节，超长直接失败，不截断
        if (parameter[i].value.size() > 0xFFFF)
        {
            iserror = true;
            co_return;
        }
        send_content.append(parameter[i].name);
        send_content.push_back(0x20);
        unsigned short value_size = static_cast<unsigned short>(parameter[i].value.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(parameter[i].value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
    send_content.append(content);

    if (isssl)
    {
        if (port == 0)
        {
            port = 443;
        }
        co_await async_ssl_send_data(send_content);
    }
    else
    {
        if (port == 0)
        {
            port = 80;
        }
        co_await async_send_data(send_content);
    }
    co_return;
}

void rpc_client::send()
{
    std::string send_content;
    send_content.append("rpc");
    send_content.push_back(0x20);
    send_content.append(url);
    send_content.push_back(0x0A);
    send_content.append("HOST ");

    // 长度字段只有两字节，超长直接失败，不截断
    if (host.size() > 0xFFFF)
    {
        iserror = true;
        return;
    }
    unsigned short value_size_h = static_cast<unsigned short>(host.size());
    unsigned char aa            = value_size_h & 0xFF;

    value_size_h     = value_size_h & 0xFF00;
    unsigned char bb = value_size_h >> 8;
    send_content.push_back(bb);
    send_content.push_back(aa);

    send_content.append(host);
    send_content.push_back(0x0A);

    send_content.append("size ");

    if (content.size() <= 0xFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = static_cast<unsigned short>(content.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x04);

        unsigned int value_size = static_cast<unsigned int>(content.size());
        unsigned char a;
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x05);

        unsigned long long value_size = content.size();
        unsigned char a;
        a = (value_size & 0xFF00000000) >> 32;
        send_content.push_back(a);
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else
    {
        iserror = true;
        return;
    }
    send_content.push_back(0x0A);

    send_content.append("type ");

    if (content_type.size() == 0)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if (content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");
            iserror = true;
            return;
        }

        unsigned short value_size = static_cast<unsigned short>(content_type.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(content_type);
    }
    send_content.push_back(0x0A);

    for (unsigned int i = 0; i < parameter.size(); i++)
    {
        // 长度字段只有两字节，超长直接失败，不截断
        if (parameter[i].value.size() > 0xFFFF)
        {
            iserror = true;
            return;
        }
        send_content.append(parameter[i].name);
        send_content.push_back(0x20);
        unsigned short value_size = static_cast<unsigned short>(parameter[i].value.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(parameter[i].value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
    send_content.append(content);

    if (isssl)
    {
        if (port == 0)
        {
            port = 443;
        }
        ssl_send_data(send_content);
    }
    else
    {
        if (port == 0)
        {
            port = 80;
        }
        send_data(send_content);
    }
    return;
}

asio::awaitable<bool> rpc_client::async_init_https_sock()
{
    ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    sslsock     = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(strand_, *ssl_context);
    ssl_context->set_default_verify_paths();

    asio::ip::tcp::resolver resolver(strand_);
    SSL_set_tlsext_host_name(sslsock->native_handle(), host.c_str());

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    auto endpoints                 = co_await resolver.async_resolve(host, std::to_string(port), asio::use_awaitable);
    for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
    {
        std::tie(ec) = co_await sslsock->lowest_layer().async_connect(*iter, tuple_awaitable);
        if (ec)
        {
            ++iter;
            continue;
        }
        break;
    }
    if (ec)
    {
        error_msg = host + " ssl async_connect error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }

    sslsock->lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    ssl_context->set_verify_mode(asio::ssl::verify_peer);
    ssl_context->set_verify_callback(asio::ssl::host_name_verification(host));

    std::tie(ec) = co_await sslsock->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
    if (ec)
    {
        error_msg = host + " ssl handshake error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}

asio::awaitable<bool> rpc_client::async_init_http_sock()
{
    error_msg.clear();
    //auto executor = co_await asio::this_coro::executor;
    asio::ip::tcp::resolver resolver(strand_);

    sock = std::make_shared<asio::ip::tcp::socket>(strand_);

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    auto endpoints                 = co_await resolver.async_resolve(host, std::to_string(port), asio::use_awaitable);

    for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
    {
        std::tie(ec) = co_await sock->async_connect(*iter, tuple_awaitable);
        if (ec)
        {
            ++iter;
            continue;
        }
        break;
    }

    if (ec)
    {
        error_msg = ec.message();
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}

asio::awaitable<void> rpc_client::async_ssl_send_data(std::string_view send_content)
{
    try
    {
        if (!sslsock)
        {
            bool isinit = co_await async_init_https_sock();
            if (!isinit)
            {
                co_return;
            }

            if (exptime > 30)
            {
                exptime = 30;
            }
            if (exptime > 0)
            {
                set_timeout(exptime);
                client_context &temp_io_context = get_client_context_obj();
                try
                {
                    temp_io_context.add_timeout_list(temp_io_context.rpc_timeout_lists, shared_from_this());
                }
                catch (const std::exception &e)
                {
                    DEBUG_LOG("Exception: %s", e.what());
                    error_msg = e.what();
                    iserror   = true;
                }
            }
            if (iserror)
            {
                co_return;
            }

            unsigned int n = 0;
            n              = co_await asio::async_write(*sslsock, asio::buffer(send_content), asio::use_awaitable);
            unsigned char read_data[2052];

            bool begin_data = true;
            isbody          = false;
            while (true)
            {
                memset(read_data, 0x00, sizeof(read_data));
                if (exptime > 0)
                {
                    reset_timeout();
                }
                if (isbody && page.read_size < 2048)
                {
                    n = co_await sslsock->async_read_some(asio::buffer(read_data, page.read_size), asio::use_awaitable);
                }
                else
                {
                    n = co_await sslsock->async_read_some(asio::buffer(read_data, 2048), asio::use_awaitable);
                }
                offsetnum = 0;
                if (n == 0)
                {
                    break;
                }
                if (begin_data)
                {
                    begin_data = false;
                    process(read_data, n);
                }
                else
                {
                    process_append(read_data, n);
                }

                if (isfinish)
                {
                    break;
                }
                if (iserror)
                {
                    break;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    co_return;
}

asio::awaitable<void> rpc_client::async_send_data(std::string_view send_content)
{
    try
    {
        if (!sock)
        {
            bool isinit = co_await async_init_http_sock();
            if (!isinit)
            {
                co_return;
            }

            if (exptime > 30)
            {
                exptime = 30;
            }
            if (exptime > 0)
            {
                set_timeout(exptime);
                client_context &temp_io_context = get_client_context_obj();
                try
                {
                    temp_io_context.add_timeout_list(temp_io_context.rpc_timeout_lists, shared_from_this());
                }
                catch (const std::exception &e)
                {
                    DEBUG_LOG("Exception: %s", e.what());
                    error_msg = e.what();
                    iserror   = true;
                }
            }
            if (iserror)
            {
                co_return;
            }

            co_await asio::async_write(*sock, asio::buffer(send_content), asio::use_awaitable);
            unsigned char read_data[2052];
            unsigned int n  = 0;
            bool begin_data = true;
            isbody          = false;
            while (true)
            {
                memset(read_data, 0x00, sizeof(read_data));
                if (exptime > 0)
                {
                    reset_timeout();
                }
                if (isbody && page.read_size < 2048)
                {
                    n = co_await sock->async_read_some(asio::buffer(read_data, page.read_size), asio::use_awaitable);
                }
                else
                {
                    n = co_await sock->async_read_some(asio::buffer(read_data, 2048), asio::use_awaitable);
                }
                offsetnum = 0;
                if (n == 0)
                {
                    break;
                }
                if (begin_data)
                {
                    begin_data = false;
                    process(read_data, n);
                }
                else
                {
                    process_append(read_data, n);
                }

                if (isfinish)
                {
                    break;
                }
                if (iserror)
                {
                    break;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    co_return;
}

void rpc_client::process_headkv()
{
    // [加固] 对齐服务端 rpc_parse.cpp：每完成一个 KV 段即累计条数 / 字节，超限按协议错误断连。
    kv_num++;
    kv_bytes += static_cast<unsigned long long>(read_key.size()) + read_value.size();
    if (kv_num > CONST_RPC_KV_MAX_ITEMS || kv_bytes > CONST_RPC_KV_MAX_BYTES)
    {
        iserror = true;
        return;
    }
    if (read_key.size() == 6 && read_key == "cookie")
    {
        read_key.clear();
        unsigned int i = 0;
        for (; i < read_value.size(); i++)
        {
            if (read_value[i] == '=')
            {
                break;
            }
            read_key.push_back(read_value[i]);
        }

        if (read_key.empty())
        {
            //ignore
            return;
        }

        if (read_value[i] == '=')
        {
            i++;
            rpc_parameter_t a;
            if (i < read_value.size())
            {
                a.name  = read_key;
                a.value = read_value.substr(i);
            }
            else
            {
                //empty
                a.name  = read_key;
                a.value = "";
            }
            page.cookie.emplace_back(a);
        }
    }
    else if (read_key.size() == 4 && read_key == "type")
    {
        page.content_type = read_value;
    }
    else if (read_key.size() == 5 && read_key == "chunk")
    {
        if (read_value == "none")
        {
            page.ischunked = false;
        }
        else
        {
            if (read_value.size() > 0)
            {
                page.ischunked = true;
            }
            else
            {
                page.ischunked = false;
            }
        }
    }
    else if (read_key.size() == 6 && read_key == "encode")
    {
        page.encode_type = read_value;
    }
    else if (read_key.size() == 4 && read_key == "size")
    {
        if (read_value.size() > 8)
        {
            //长度字段最多 8 字节
            iserror = true;
            return;
        }
        unsigned long long v = 0;
        for (unsigned int i = 0; i < read_value.size(); i++)
        {
            v = (v << 8) + static_cast<unsigned char>(read_value[i]);
        }
        if (v > 68719476735ULL)
        {
            //超过 64G
            iserror = true;
            return;
        }
        page.page_size = v;
        content_size   = static_cast<long long>(v);
        page.read_size = v;
    }
    else
    {
        rpc_parameter_t a;
        a.name  = read_key;
        a.value = read_value;
        page.header.emplace_back(a);
    }
}
void rpc_client::process_value(const unsigned char *buffer, unsigned int readnum)
{
    if (val_size == 0)
    {
        //空值：不消费净荷字节，直接去读值结束符
        cur_process_type = 11;
        return;
    }
    for (; offsetnum < readnum; offsetnum++)
    {
        read_value.push_back(buffer[offsetnum]);
        // [加固] read_value 单条 KV 值的后盾上限：2 字节长度前缀已把单值框在 ≤64KB，
        // 此处再封顶 CONST_RPC_KV_MAX_BYTES，作为前缀逻辑被绕过时的兜底。
        if (read_value.size() > CONST_RPC_KV_MAX_BYTES)
        {
            iserror = true;
            return;
        }
        val_size--;
        if (val_size == 0)
        {
            cur_process_type = 11;
            offsetnum++;
            return;
        }
    }
}

void rpc_client::process_append(const unsigned char *buffer, unsigned int readnum)
{
    if (iserror)
    {
        return;
    }

    if (isbody)
    {
        if (page.ischunked)
        {
            process_body_chunk(buffer, readnum);
        }
        else
        {
            process_body(buffer, readnum);
        }
    }
    else
    {
        process_parameter(buffer, readnum);
    }
}

void rpc_client::process(const unsigned char *buffer, unsigned int readnum)
{
    offsetnum = 0;
    if (readnum < 7)
    {
        error_msg = "too short response";
        iserror   = true;
        return;
    }

    if (buffer[0] == 'r' && buffer[1] == 'p' && buffer[2] == 'c' && buffer[3] == 0x20)
    {
        if (buffer[4] >= '0' && buffer[4] <= '9' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] >= '0' && buffer[6] <= '9')
        {
            page.status_code = buffer[4] - '0';
            page.status_code = page.status_code * 10;
            page.status_code = page.status_code + (buffer[5] - '0');
            page.status_code = page.status_code * 10;
            page.status_code = page.status_code + (buffer[6] - '0');
        }
        else
        {
            error_msg = "header info error";
            iserror   = true;
            return;
        }
        offsetnum = 7;
        if (offsetnum < readnum && buffer[offsetnum] == 0x20)
        {
            for (; offsetnum < readnum; offsetnum++)
            {
                if (buffer[offsetnum] == 0x0A)
                {
                    break;
                }
                page.status_msg.push_back(buffer[offsetnum]);
            }

            if (offsetnum >= readnum || buffer[offsetnum] != 0x0A)
            {
                error_msg = "header info error";
                iserror   = true;
                return;
            }
            offsetnum++;
            //process parameter
            process_parameter(buffer, readnum);
            if (isbody)
            {
                if (page.ischunked)
                {
                    process_body_chunk(buffer, readnum);
                }
                else
                {
                    process_body(buffer, readnum);
                }
            }
        }
        else
        {
            error_msg = "header info error";
            iserror   = true;
            return;
        }
    }
    else
    {
        error_msg = "header info error";
        iserror   = true;
        return;
    }
}

void rpc_client::process_parameter(const unsigned char *buffer, unsigned int readnum)
{
    if (cur_process_type == 0)
    {
        for (unsigned int i = 0; offsetnum < readnum; offsetnum++)
        {
            if (buffer[offsetnum] == 0x20)
            {
                break;
            }
            read_key.push_back(buffer[offsetnum]);
            i++;
            if (i > 128)
            {
                iserror = true;
                return;
            }
        }

        if (read_key.size() > 128)
        {
            iserror = true;
            return;
        }

        if (offsetnum < readnum && buffer[offsetnum] == 0x20)
        {
            offsetnum++;
        }
        else
        {
            // not finished
            cur_process_type = 0;
            return;
        }

        if (offsetnum < readnum)
        {
            val_size = buffer[offsetnum];
            offsetnum++;
            if (offsetnum < readnum)
            {
                val_size         = (val_size << 8) + buffer[offsetnum];
                cur_process_type = 10;
                offsetnum++;
                process_value(buffer, readnum);
            }
            else
            {
                // not finished
                cur_process_type = 1;
                return;
            }
        }
        else
        {
            // not finished
            cur_process_type = 2;
            return;
        }
    }
    else if (cur_process_type == 1)
    {
        if (offsetnum < readnum)
        {
            val_size         = (val_size << 8) + buffer[offsetnum];
            cur_process_type = 10;
            offsetnum++;
            process_value(buffer, readnum);
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }
    else if (cur_process_type == 2)
    {
        if (offsetnum < readnum)
        {
            val_size = buffer[offsetnum];
            offsetnum++;
            if (offsetnum < readnum)
            {
                val_size         = (val_size << 8) + buffer[offsetnum];
                cur_process_type = 10;
                offsetnum++;
                process_value(buffer, readnum);
            }
            else
            {
                // not finished
                iserror = true;
                return;
            }
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }
    else if (cur_process_type == 10)
    {
        cur_process_type = 10;
        process_value(buffer, readnum);
    }
    else if (cur_process_type == 12)
    {
        //值还没有读到空格 The value has not yet reached the space
        if (offsetnum < readnum && buffer[offsetnum] == 0x0A)
        {
            offsetnum++;
        }
        else
        {
            iserror = true;
            return;
        }
        cur_process_type = 0;
        //重新开始kv read kv
    }

    if (cur_process_type == 11)
    {
        if (offsetnum < readnum)
        {
            if (buffer[offsetnum] == 0x0A)
            {
                offsetnum++;
            }
            process_headkv();
            cur_process_type = 0;
            read_key.clear();
            read_value.clear();
        }
        else if (offsetnum == readnum)
        {
            process_headkv();
            cur_process_type = 12;
            read_key.clear();
            read_value.clear();
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }

    if (offsetnum < readnum)
    {
        //test the end
        if (buffer[offsetnum] == 0x0A)
        {
            offsetnum++;
            isbody = true;
            return;
        }
    }
    else
    {
        //the end
        return;
    }

    // next
    process_parameter(buffer, readnum);
}
void rpc_client::process_body_chunk(const unsigned char *buffer, unsigned int readnum)
{
    if (isfinish)
    {
        return;
    }
    if (iserror)
    {
        return;
    }
    if (offsetnum < readnum)
    {
        if (page.read_size == 0)
        {
            if (chunk_head_high)
            {
                page.read_size = ((unsigned int)chunk_head_byte << 8) | buffer[offsetnum];
                offsetnum++;
                chunk_head_high = false;
            }
            else
            {
                chunk_head_byte = buffer[offsetnum];
                offsetnum++;
                if (offsetnum >= readnum)
                {
                    //长度头跨段：等下一段的低字节
                    chunk_head_high = true;
                    return;
                }
                page.read_size = ((unsigned int)chunk_head_byte << 8) | buffer[offsetnum];
                offsetnum++;
            }
            if (page.read_size == 0)
            {
                if (page.isfile)
                {
                    // 内容已在临时文件：关闭句柄收尾；content_length 已在 append_body 累计
                    uprawfile.reset(nullptr);
                }
                else
                {
                    page.content_length = page.content.size();
                }
                isfinish = true;
                return;
            }
        }

        if (offsetnum < readnum)
        {
            // 落盘门面：未超 CONST_RPC_BODY_MEM_LIMIT 留内存；超了（或已切到文件）写临时文件。
            // 首次切文件时把已累积在 page.content 的部分先刷入文件，保证 body 完整不断裂。
            auto append_body = [&](const unsigned char *p, size_t n)
            {
                if (page.isfile || (page.content.size() + n) > CONST_RPC_BODY_MEM_LIMIT)
                {
                    if (uprawfile == nullptr)
                    {
                        server_loaclvar &localvar = get_server_global_var();
                        std::string tmppath       = localvar.temp_path;
                        tmppath.append(make_http_temp_raw_name());
                        uprawfile.reset(fopen(tmppath.c_str(), "wb"));
                        page.tempfile = tmppath;
                        if (uprawfile == nullptr)
                        {
                            iserror = true;
                            return;
                        }
                        page.isfile = true;
                        if (!page.content.empty())
                        {
                            fwrite(page.content.data(), page.content.size(), 1, uprawfile.get());
                            page.content.clear();
                        }
                    }
                    fwrite(p, n, 1, uprawfile.get());
                }
                else
                {
                    page.content.append((const char *)p, n);
                }
                page.content_length += n;
            };

            unsigned long long temp_size = readnum - offsetnum;
            if (temp_size > page.read_size)
            {
                unsigned long long consumed = page.read_size;
                append_body(&buffer[offsetnum], (size_t)consumed);
                offsetnum += (unsigned int)consumed;
                page.read_size = 0;
                if (chunk_process != nullptr)
                {
                    chunk_process(shared_from_this());
                }
                process_body_chunk(buffer, readnum);
                return;
            }
            else
            {
                append_body(&buffer[offsetnum], (size_t)temp_size);
                if (chunk_process != nullptr)
                {
                    chunk_process(shared_from_this());
                }
            }
        }

        page.read_size = page.read_size - (readnum - offsetnum);
        if (page.read_size > 68719476735)
        {
            // too conent
            iserror = true;
            return;
        }
    }
    else if (offsetnum == readnum)
    {
        return;
    }
    else
    {
        //read error
        iserror = true;
        return;
    }
}
void rpc_client::process_body(const unsigned char *buffer, unsigned int readnum)
{
    if (isfinish)
    {
        return;
    }
    if (iserror)
    {
        return;
    }

    if (page.page_size == 0)
    {
        isfinish = true;
        return;
    }
    if (page.page_size > CONST_RPC_BODY_MEM_LIMIT)
    {
        if (offsetnum >= readnum)
        {
            //本段没有净荷字节：不建文件、不开句柄
            return;
        }
        if (uprawfile == nullptr)
        {
            server_loaclvar &localvar = get_server_global_var();
            read_key                  = std::to_string(timeid()) + std::to_string(std::hash<std::string>{}(url)) + std::to_string(rand_range(1000, 9999));

            // 落盘名统一由 make_http_temp_raw_name() 生成：文件名带 pzraw_ 前缀，
            // 该前缀是 httpwatch 周期清理时识别「框架自己的临时文件」的唯一依据，
            // 不要在此自行拼接文件名，否则文件会长期留在 temp_path 里。
            read_value = localvar.temp_path;
            read_value.append(make_http_temp_raw_name());

            // uprawfile = fopen(upfile.tempfile.c_str(), "wb");
            uprawfile.reset(fopen(read_value.c_str(), "wb"));

            page.tempfile = read_value;

            page.isfile = true;
        }

        if (uprawfile == nullptr)
        {
            //
            iserror = true;
            return;
        }
        unsigned long long avail = readnum - offsetnum;
        unsigned long long take  = (avail < page.read_size) ? avail : page.read_size;
        fwrite(&buffer[offsetnum], take, 1, uprawfile.get());
        offsetnum += (unsigned int)take;
        page.read_size = page.read_size - take;
        if (page.read_size > 68719476735)
        {
            // too conent
            iserror = true;
            uprawfile.reset(nullptr);
            return;
        }
        if (page.read_size == 0)
        {
            uprawfile.reset(nullptr);
            isfinish = true;
            return;
        }
    }
    else
    {
        if (offsetnum >= readnum)
        {
            return;
        }
        unsigned long long avail = readnum - offsetnum;
        unsigned long long take  = (avail < page.read_size) ? avail : page.read_size;
        page.content.append((char *)&buffer[offsetnum], take);
        offsetnum += (unsigned int)take;
        if (content_size >= 0 && page.content.size() == static_cast<size_t>(content_size))
        {
            isfinish            = true;
            page.content_length = content_size;
            return;
        }

        page.read_size = page.read_size - take;
        if (page.read_size > 68719476735)
        {
            // too conent
            iserror = true;
            return;
        }

        if (page.read_size == 0)
        {
            isfinish            = true;
            page.content_length = content_size;
            return;
        }
    }
}

void rpc_client::run_loop()
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        return;
    }

    auto self = shared_from_this();
    if (data == nullptr)
    {
        data = static_cast<unsigned char *>(std::malloc(512 * sizeof(unsigned char)));
        if (data == nullptr)
        {
            error_msg = "read buffer alloc failed";
            iserror   = true;
            socket_read_lock.clear();
            return;
        }
    }

    for (;;)
    {
        if (iserror)
        {
            socket_read_lock.clear();
            return;
        }

        if (isclose)
        {
            socket_read_lock.clear();
            return;
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
                if (sslsock->lowest_layer().is_open())
                {
                    n = sslsock->read_some(asio::buffer(data, 512));
                }
                else
                {
                    socket_read_lock.clear();
                    return;
                }
            }
            else
            {
                if (sock->is_open())
                {
                    n = sock->read_some(asio::buffer(data, 512));
                }
                else
                {
                    socket_read_lock.clear();
                    return;
                }
            }

            if (run_loop_fun != nullptr)
            {
                run_loop_fun(self, n);
            }
            else if (async_run_loop_fun != nullptr)
            {
                asio::co_spawn(strand_, [self, n]() mutable
                               { return self->async_run_loop_fun(self, n); },
                               asio::detached);
            }
            else
            {
                socket_read_lock.clear();
                return;
            }
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
            socket_read_lock.clear();
            return;
        }
    }
    socket_read_lock.clear();
    return;
}

asio::awaitable<void> rpc_client::async_run_loop()
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        co_return;
    }
    auto self = shared_from_this();
    if (data == nullptr)
    {
        data = static_cast<unsigned char *>(std::malloc(512 * sizeof(unsigned char)));
        if (data == nullptr)
        {
            error_msg = "read buffer alloc failed";
            iserror   = true;
            socket_read_lock.clear();
            co_return;
        }
    }

    for (;;)
    {
        if (iserror)
        {
            socket_read_lock.clear();
            co_return;
        }

        if (isclose)
        {
            socket_read_lock.clear();
            co_return;
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
                n = co_await sslsock->async_read_some(asio::buffer(data, 512), asio::use_awaitable);
            }
            else
            {
                n = co_await sock->async_read_some(asio::buffer(data, 512), asio::use_awaitable);
            }

            if (run_loop_fun != nullptr)
            {
                run_loop_fun(self, n);
            }
            else if (async_run_loop_fun != nullptr)
            {
                co_await async_run_loop_fun(self, n);
            }
            else
            {
                socket_read_lock.clear();
                co_return;
            }
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
            socket_read_lock.clear();
            co_return;
        }
    }
    socket_read_lock.clear();
    co_return;
}

asio::awaitable<unsigned int> rpc_client::async_read(unsigned char *buffer_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        co_return 0;
    }

    if (iserror)
    {
        socket_read_lock.clear();
        co_return 0;
    }

    if (isclose)
    {
        socket_read_lock.clear();
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
        socket_read_lock.clear();
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

asio::awaitable<unsigned int> rpc_client::async_read(std::string &buffer_data)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        co_return 0;
    }

    if (iserror)
    {
        socket_read_lock.clear();
        co_return 0;
    }

    if (isclose)
    {
        socket_read_lock.clear();
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
        socket_read_lock.clear();
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

asio::awaitable<unsigned int> rpc_client::async_write(unsigned char *data_out, unsigned int buffersize)
{
    if (iserror)
    {
        co_return 0;
    }

    if (isclose)
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
            n = co_await asio::async_write(*sslsock, asio::buffer(data_out, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*sock, asio::buffer(data_out, buffersize), asio::use_awaitable);
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

asio::awaitable<unsigned int> rpc_client::async_write(std::string_view value)
{
    if (iserror)
    {
        co_return 0;
    }

    if (isclose)
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

//synchronous
unsigned int rpc_client::write(unsigned char *data_out, unsigned int buffersize)
{
    if (iserror)
    {
        return 0;
    }

    if (isclose)
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
            n = asio::write(*sslsock, asio::buffer(data_out, buffersize));
        }
        else
        {
            n = asio::write(*sock, asio::buffer(data_out, buffersize));
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

unsigned int rpc_client::write(std::string_view value)
{
    if (iserror)
    {
        return 0;
    }

    if (isclose)
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

unsigned int rpc_client::read(unsigned char *buffer_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        return 0;
    }

    if (iserror)
    {
        socket_read_lock.clear();
        return 0;
    }

    if (isclose)
    {
        socket_read_lock.clear();
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
        socket_read_lock.clear();
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    socket_read_lock.clear();
    return 0;
}

unsigned int rpc_client::read(std::string &buffer_data)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        return 0;
    }

    if (iserror)
    {
        socket_read_lock.clear();
        return 0;
    }

    if (isclose)
    {
        socket_read_lock.clear();
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
        socket_read_lock.clear();
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    socket_read_lock.clear();
    return 0;
}

bool rpc_client::connect(std::string_view rpcurl, unsigned int time_out_num)
{
    set_url(rpcurl);
    std::string send_content;
    send_content.append("rpc");
    send_content.push_back(0x20);
    send_content.append(url);
    send_content.push_back(0x0A);
    send_content.append("HOST ");

    // 长度字段只有两字节，超长直接失败，不截断
    if (host.size() > 0xFFFF)
    {
        iserror = true;
        return false;
    }
    unsigned short value_size_h = static_cast<unsigned short>(host.size());
    unsigned char aa            = value_size_h & 0xFF;

    value_size_h     = value_size_h & 0xFF00;
    unsigned char bb = value_size_h >> 8;
    send_content.push_back(bb);
    send_content.push_back(aa);

    send_content.append(host);
    send_content.push_back(0x0A);

    send_content.append("size ");

    if (content.size() <= 0xFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = static_cast<unsigned short>(content.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x04);

        unsigned int value_size = static_cast<unsigned int>(content.size());
        unsigned char a;
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x05);

        unsigned long long value_size = content.size();
        unsigned char a;
        a = (value_size & 0xFF00000000) >> 32;
        send_content.push_back(a);
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else
    {
        iserror = true;
        return false;
    }
    send_content.push_back(0x0A);

    send_content.append("type ");

    if (content_type.size() == 0)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if (content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");
            iserror = true;
            return false;
        }

        unsigned short value_size = static_cast<unsigned short>(content_type.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(content_type);
    }
    send_content.push_back(0x0A);

    for (unsigned int i = 0; i < parameter.size(); i++)
    {
        // 长度字段只有两字节，超长直接失败，不截断
        if (parameter[i].value.size() > 0xFFFF)
        {
            iserror = true;
            return false;
        }
        send_content.append(parameter[i].name);
        send_content.push_back(0x20);
        unsigned short value_size = static_cast<unsigned short>(parameter[i].value.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(parameter[i].value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
    send_content.append(content);

    exptime = time_out_num;
    if (exptime > 30)
    {
        exptime = 30;
    }

    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.add_timeout_list(temp_io_context.rpc_timeout_lists, shared_from_this());
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
        }
    }
    if (iserror)
    {
        return false;
    }

    if (isssl)
    {
        return init_https_sock();
    }
    else
    {
        return init_http_sock();
    }
    return false;
}
bool rpc_client::connect()
{
    std::string send_content;
    send_content.append("rpc");
    send_content.push_back(0x20);
    send_content.append(url);
    send_content.push_back(0x0A);
    send_content.append("HOST ");

    // 长度字段只有两字节，超长直接失败，不截断
    if (host.size() > 0xFFFF)
    {
        iserror = true;
        return false;
    }
    unsigned short value_size_h = static_cast<unsigned short>(host.size());
    unsigned char aa            = value_size_h & 0xFF;

    value_size_h     = value_size_h & 0xFF00;
    unsigned char bb = value_size_h >> 8;
    send_content.push_back(bb);
    send_content.push_back(aa);

    send_content.append(host);
    send_content.push_back(0x0A);

    send_content.append("size ");

    if (content.size() <= 0xFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = static_cast<unsigned short>(content.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x04);

        unsigned int value_size = static_cast<unsigned int>(content.size());
        unsigned char a;
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x05);

        unsigned long long value_size = content.size();
        unsigned char a;
        a = (value_size & 0xFF00000000) >> 32;
        send_content.push_back(a);
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else
    {
        iserror = true;
        return false;
    }
    send_content.push_back(0x0A);

    send_content.append("type ");

    if (content_type.size() == 0)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if (content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");
            iserror = true;
            return false;
        }

        unsigned short value_size = static_cast<unsigned short>(content_type.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(content_type);
    }
    send_content.push_back(0x0A);

    for (unsigned int i = 0; i < parameter.size(); i++)
    {
        // 长度字段只有两字节，超长直接失败，不截断
        if (parameter[i].value.size() > 0xFFFF)
        {
            iserror = true;
            return false;
        }
        send_content.append(parameter[i].name);
        send_content.push_back(0x20);
        unsigned short value_size = static_cast<unsigned short>(parameter[i].value.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(parameter[i].value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
    send_content.append(content);

    if (exptime > 30)
    {
        exptime = 30;
    }

    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.add_timeout_list(temp_io_context.rpc_timeout_lists, shared_from_this());
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
        }
    }
    if (iserror)
    {
        return false;
    }

    if (isssl)
    {
        return init_https_sock();
    }
    else
    {
        return init_http_sock();
    }
    return false;
}
asio::awaitable<bool> rpc_client::async_connect(std::string_view rpcurl, unsigned int time_out_num)
{
    set_url(rpcurl);
    std::string send_content;
    send_content.append("rpc");
    send_content.push_back(0x20);
    send_content.append(url);
    send_content.push_back(0x0A);
    send_content.append("HOST ");

    // 长度字段只有两字节，超长直接失败，不截断
    if (host.size() > 0xFFFF)
    {
        iserror = true;
        co_return false;
    }
    unsigned short value_size_h = static_cast<unsigned short>(host.size());
    unsigned char aa            = value_size_h & 0xFF;

    value_size_h     = value_size_h & 0xFF00;
    unsigned char bb = value_size_h >> 8;
    send_content.push_back(bb);
    send_content.push_back(aa);

    send_content.append(host);
    send_content.push_back(0x0A);

    send_content.append("size ");

    if (content.size() <= 0xFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = static_cast<unsigned short>(content.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x04);

        unsigned int value_size = static_cast<unsigned int>(content.size());
        unsigned char a;
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x05);

        unsigned long long value_size = content.size();
        unsigned char a;
        a = (value_size & 0xFF00000000) >> 32;
        send_content.push_back(a);
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else
    {
        iserror = true;
        co_return false;
    }
    send_content.push_back(0x0A);

    send_content.append("type ");

    if (content_type.size() == 0)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if (content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");
            iserror = true;
            co_return false;
        }

        unsigned short value_size = static_cast<unsigned short>(content_type.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(content_type);
    }
    send_content.push_back(0x0A);

    for (unsigned int i = 0; i < parameter.size(); i++)
    {
        // 长度字段只有两字节，超长直接失败，不截断
        if (parameter[i].value.size() > 0xFFFF)
        {
            iserror = true;
            co_return false;
        }
        send_content.append(parameter[i].name);
        send_content.push_back(0x20);
        unsigned short value_size = static_cast<unsigned short>(parameter[i].value.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(parameter[i].value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
    send_content.append(content);

    exptime = time_out_num;
    if (exptime > 30)
    {
        exptime = 30;
    }

    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.add_timeout_list(temp_io_context.rpc_timeout_lists, shared_from_this());
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
        }
    }
    if (iserror)
    {
        co_return false;
    }

    if (isssl)
    {
        co_return co_await async_init_https_sock();
    }
    else
    {
        co_return co_await async_init_http_sock();
    }
    co_return false;
}
asio::awaitable<bool> rpc_client::async_connect()
{
    std::string send_content;
    send_content.append("rpc");
    send_content.push_back(0x20);
    send_content.append(url);
    send_content.push_back(0x0A);
    send_content.append("HOST ");

    // 长度字段只有两字节，超长直接失败，不截断
    if (host.size() > 0xFFFF)
    {
        iserror = true;
        co_return false;
    }
    unsigned short value_size_h = static_cast<unsigned short>(host.size());
    unsigned char aa            = value_size_h & 0xFF;

    value_size_h     = value_size_h & 0xFF00;
    unsigned char bb = value_size_h >> 8;
    send_content.push_back(bb);
    send_content.push_back(aa);

    send_content.append(host);
    send_content.push_back(0x0A);

    send_content.append("size ");

    if (content.size() <= 0xFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = static_cast<unsigned short>(content.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x04);

        unsigned int value_size = static_cast<unsigned int>(content.size());
        unsigned char a;
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else if (content.size() <= 0xFFFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x05);

        unsigned long long value_size = content.size();
        unsigned char a;
        a = (value_size & 0xFF00000000) >> 32;
        send_content.push_back(a);
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else
    {
        iserror = true;
        co_return false;
    }
    send_content.push_back(0x0A);

    send_content.append("type ");

    if (content_type.size() == 0)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if (content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");
            iserror = true;
            co_return false;
        }

        unsigned short value_size = static_cast<unsigned short>(content_type.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(content_type);
    }
    send_content.push_back(0x0A);

    for (unsigned int i = 0; i < parameter.size(); i++)
    {
        // 长度字段只有两字节，超长直接失败，不截断
        if (parameter[i].value.size() > 0xFFFF)
        {
            iserror = true;
            co_return false;
        }
        send_content.append(parameter[i].name);
        send_content.push_back(0x20);
        unsigned short value_size = static_cast<unsigned short>(parameter[i].value.size());
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(parameter[i].value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
    send_content.append(content);

    if (exptime > 30)
    {
        exptime = 30;
    }

    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.add_timeout_list(temp_io_context.rpc_timeout_lists, shared_from_this());
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
        }
    }
    if (iserror)
    {
        co_return false;
    }

    if (isssl)
    {
        co_return co_await async_init_https_sock();
    }
    else
    {
        co_return co_await async_init_http_sock();
    }
    co_return false;
}

bool rpc_client::init_https_sock()
{
    error_msg.clear();

    ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    sslsock     = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(strand_, *ssl_context);

    ssl_context->set_default_verify_paths();
    asio::ip::tcp::resolver resolver(strand_);
    auto endpoints = resolver.resolve(host.c_str(), std::to_string(port));

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

bool rpc_client::init_http_sock()
{
    error_msg.clear();
    sock = std::make_shared<asio::ip::tcp::socket>(strand_);
    asio::ip::tcp::resolver resolver(strand_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    asio::connect(*sock, endpoints, ec);

    if (ec)
    {
        error_msg = ec.message();
        DEBUG_LOG("%s", error_msg.c_str());
        return false;
    }
    return true;
}

void rpc_client::send_data(std::string_view send_content)
{
    try
    {
        if (!sock)
        {
            bool isinit = init_http_sock();
            if (!isinit)
            {
                return;
            }

            if (exptime > 30)
            {
                exptime = 30;
            }
            if (exptime > 0)
            {
                set_timeout(exptime);
                client_context &temp_io_context = get_client_context_obj();
                try
                {
                    temp_io_context.add_timeout_list(temp_io_context.rpc_timeout_lists, shared_from_this());
                }
                catch (const std::exception &e)
                {
                    DEBUG_LOG("Exception: %s", e.what());
                    error_msg = e.what();
                    iserror   = true;
                }
            }
            if (iserror)
            {
                return;
            }

            unsigned int n = 0;
            n              = asio::write(*sock, asio::buffer(send_content));
            unsigned char read_data[2052];
            if (n == 0)
            {
                return;
            }
            n               = 0;
            bool begin_data = true;
            isbody          = false;
            while (true)
            {
                memset(read_data, 0x00, sizeof(read_data));
                if (exptime > 0)
                {
                    reset_timeout();
                }
                if (isbody && page.read_size < 2048)
                {
                    n = sock->read_some(asio::buffer(read_data, page.read_size));
                }
                else
                {
                    n = sock->read_some(asio::buffer(read_data, 2048));
                }
                offsetnum = 0;
                if (n == 0)
                {
                    break;
                }
                if (begin_data)
                {
                    begin_data = false;
                    process(read_data, n);
                }
                else
                {
                    process_append(read_data, n);
                }

                if (isfinish)
                {
                    break;
                }
                if (iserror)
                {
                    break;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
}
void rpc_client::ssl_send_data(std::string_view send_content)
{
    try
    {
        if (!sslsock)
        {
            bool isinit = init_https_sock();
            if (!isinit)
            {
                return;
            }

            if (exptime > 30)
            {
                exptime = 30;
            }
            if (exptime > 0)
            {
                set_timeout(exptime);
                client_context &temp_io_context = get_client_context_obj();
                try
                {
                    temp_io_context.add_timeout_list(temp_io_context.rpc_timeout_lists, shared_from_this());
                }
                catch (const std::exception &e)
                {
                    DEBUG_LOG("Exception: %s", e.what());
                    error_msg = e.what();
                    iserror   = true;
                }
            }
            if (iserror)
            {
                return;
            }

            unsigned int n = 0;
            n              = asio::write(*sslsock, asio::buffer(send_content));
            unsigned char read_data[2052];

            bool begin_data = true;
            isbody          = false;
            while (true)
            {
                memset(read_data, 0x00, sizeof(read_data));
                if (exptime > 0)
                {
                    reset_timeout();
                }
                if (isbody && page.read_size < 2048)
                {
                    n = sslsock->read_some(asio::buffer(read_data, page.read_size));
                }
                else
                {
                    n = sslsock->read_some(asio::buffer(read_data, 2048));
                }
                offsetnum = 0;
                if (n == 0)
                {
                    break;
                }
                if (begin_data)
                {
                    begin_data = false;
                    process(read_data, n);
                }
                else
                {
                    process_append(read_data, n);
                }

                if (isfinish)
                {
                    break;
                }
                if (iserror)
                {
                    break;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
}

}// namespace http