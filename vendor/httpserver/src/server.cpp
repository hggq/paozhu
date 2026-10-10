#include <datetime.h>
#include <string>
#include <zlib.h>
#include <functional>
#include <cstring>
#include "version.h"
#include "terminal_color.h"
#include "cost_define.h"
#include "atomic_count.h"
#include "sendqueue.h"
#include "http_socket.h"
#include "client_session.h"
#include "mqtt_frame.h"
#include "mqtt_session.h"
#include "mqtt_broker.h"
#include "mqtt_api.h"
#include "mqtt_method_reg.hpp"
#include "http2_parse.h"
#include "http_mime.h"
#include "http2_define.h"
#include "http_domain.h"
#include "server.h"
#include "pool_step.h"
#include "http2_huffman.h"
#include "http_parse.h"
#include "serverconfig.h"
#include "httppeer.h"
#include "router.h"
#include "http2_flow.h"
#include "directory_fun.h"
#include "https_brotli.h"
#include "gzip.h"
#include "http2_send_queue.h"
#include "httphook.h"
#include "func.h"

#include "orm_conn_pool.h"
#include "orm_connect_mar.h"

#include "autocontrolmethod.hpp"
#include "reghttpmethod.hpp"
#include "reghttpmethod_pre.hpp"
#include "regviewmethod.hpp"
#include "autorestfulpaths.hpp"
#include "client_context.h"
#ifdef ENABLE_REDIS
#include "pzredis_config.h"
#include "redis_pool.h"
#endif
#ifdef ENABLE_REDIS_CLIENT
#include "redis_subpub_reg.h"
#include "redis_regmethod.hpp"
#endif
#ifdef ENABLE_WEBSOCKETS_CLIENT
#include "websockets_config.h"
#include "ws_subpub_reg.h"
#include "ws_client_regmethod.hpp"
#endif
#ifdef ENABLE_SOCKETS_CLIENT
#include "sockets_config.h"
#include "sock_subpub_reg.h"
#include "sock_client_regmethod.hpp"
#endif
#ifdef ENABLE_MQTT_CLIENT
#include "mqtt_config.h"
#include "mqtt_subpub_reg.h"
#include "mqtt_client_regmethod.hpp"
#endif
#include "fastcgi.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#endif

#ifdef ENABLE_BOOST
#include "loadviewso.h"
#include "loadmodule.h"
#include "http_so_common_api.h"
#endif

#include <openssl/ssl.h>
#include "server_localvar.h"
#include "debug_log.h"
#include "websockets_callback.h"
#include "websockets_method_reg.hpp"
#include "ws_handshake.h"
#include "ws_parser.h"

#include "rpc_parse.h"
#include "sockets_method_reg.hpp"
#include "get_rss_memory.h"

#include "acme_config.h"
#include "acme_client.h"
#include "acme_crypto.hpp"
#include "acme_run.h"

namespace fs = std::filesystem;

namespace http
{

httpserver &get_server_app()
{
    static httpserver instance;
    return instance;
}

// 间隔任务的实际登记口是路由 frametasks_timeloop（注册点在 controller/src/serverwatch.cpp，
// router 按名字找它）：它自己往 clientlooptasks 里 push，
// 帧循环按 timeloop_num 拍数把它转交 clientrunpool.addclient()（linktype==7 → timetasks_run）。
// 这个自由函数没有调用点，注释掉以免被当成入口误用。
// void add_server_timetask(std::size_t keyname, std::shared_ptr<httppeer> peer)
// {
//     get_server_app().clientlooptasks.push_back({keyname, peer});
// }

asio::awaitable<void> httpserver::http2_send_file_range(std::shared_ptr<httppeer> peer)
{
    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(15360);
    send_file_obj->fp.reset(std::fopen(peer->sendfilename.c_str(), "rb"));

    DEBUG_LOG("%s", peer->sendfilename.c_str());

    long long file_length = -1;
    if (send_file_obj->fp.get())
    {
        fseek(send_file_obj->fp.get(), 0, SEEK_END);
        file_length = ftell(send_file_obj->fp.get());
        fseek(send_file_obj->fp.get(), 0, SEEK_SET);
    }

    // ftell returns -1 when size read fails; treat as access error to avoid content_length overflow.
    if (send_file_obj->fp.get() && file_length >= 0)
    {
        send_file_obj->peer           = peer;
        send_file_obj->type           = 1;
        send_file_obj->content_length = static_cast<unsigned long long>(file_length);

        send_file_obj->file_ext = get_fileext(peer->sendfilename);

        send_file_obj->content_type.clear();

        if (send_file_obj->file_ext.size() > 0)
        {
            auto mime_iter = mime_map.find(send_file_obj->file_ext);

            if (mime_iter != mime_map.end())
            {
                send_file_obj->content_type = mime_iter->second;
            }
        }

        if (send_file_obj->content_type.size() == 0)
        {
            if (send_file_obj->content_length > 204800)
            {
                send_file_obj->content_type = "application/octet-stream";
            }
            else
            {
                send_file_obj->content_type = "text/plain";
            }
        }

        unsigned long long filesize = send_file_obj->content_length;
        // 后缀范围 bytes=-N 先归一化成绝对范围，再统一做合法性判断。
        // 不能用 "rangebegin > 0 / rangeend > 0" 反推是否给过端点：bytes=0-0 与 bytes=-N 都会读错。
        if (peer->state.rangebytes && peer->state.range_suffix)
        {
            unsigned long long suffix_len = peer->state.rangeend;
            if (suffix_len > filesize)
            {
                suffix_len = filesize;
            }
            peer->state.rangebegin    = filesize - suffix_len;
            peer->state.rangeend      = (filesize > 0) ? (filesize - 1) : 0;
            peer->state.range_has_end = true;
        }

        if (peer->state.rangebegin >= filesize ||
            (peer->state.range_has_end && peer->state.rangeend >= filesize) ||
            (peer->state.range_has_end && peer->state.rangeend < peer->state.rangebegin))
        {
            // http2_send_status_content() 内部已经入队并 notify，此处不得再把同一个
            // send_file_obj 入队：它的 header 从未构建、is_sendheader 为 false，
            // push("") 仍会成功占槽 → 对端先收到一个空帧，再收到没有 HEADERS 前缀的
            // DATA 帧，直接帧错位。该对象并未入队，这里直接归还对象池。
            co_await http2_send_status_content(peer, 400, "client range request error!");
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        unsigned int statecode = 200;
        if (peer->state.rangebytes)
        {
            send_file_obj->current_num = peer->state.rangebegin;
            statecode                  = 206;
            fseek(send_file_obj->fp.get(), send_file_obj->current_num, SEEK_SET);
        }
        if (peer->state.range_has_end)
        {
            // 末端越界按 RFC 7233 收敛到最后一个字节：门槛必须含等号，
            // 只拦 rangeend > filesize 时 rangeend == filesize 声明长度会多出 1 字节
            if (peer->state.rangeend >= filesize)
            {
                peer->state.rangeend = filesize - 1;
            }
            send_file_obj->content_length = peer->state.rangeend + 1;
            statecode                     = 206;
        }

        peer->status(statecode);
        peer->length(send_file_obj->content_length - send_file_obj->current_num);
        peer->type(send_file_obj->content_type);

        send_file_obj->etag = make_header_etag(filesize, peer->fileinfo.st_mtime + peer->url.size());

        if (statecode == 206)
        {
            peer->set_header("content-range",
                             "bytes " + std::to_string(send_file_obj->current_num) + "-" + std::to_string(send_file_obj->content_length - 1) + "/" +
                                 std::to_string(filesize));
        }
        else
        {
            peer->set_header("accept-ranges", "bytes");
        }
        DEBUG_LOG("start send file range");
        peer->set_header("date", get_gmttime());
        peer->set_header("last-modified", get_gmttime(peer->fileinfo.st_mtime));

        peer->set_header("etag", send_file_obj->etag);
        send_file_obj->header        = peer->make_http2_header(0);
        send_file_obj->is_sendheader = false;

        // add send sequence

        std::unique_lock<std::mutex> lock(send_data_mutex);
        sent_data_list.emplace_back(send_file_obj);
        lock.unlock();
        send_data_condition.notify_one();
    }
    else
    {
        send_file_obj->type    = 2;
        send_file_obj->content = "<h3>500 Internal Server Error</h3>";
        send_file_obj->content.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
        peer->status(500);
        peer->length(send_file_obj->content.size());
        peer->type("text/html; charset=utf-8");
        send_file_obj->header        = peer->make_http2_header(0);
        send_file_obj->is_sendheader = false;

        std::unique_lock<std::mutex> lock(send_data_mutex);
        sent_data_list.emplace_back(send_file_obj);
        lock.unlock();
        send_data_condition.notify_one();
    }
    co_return;
}

asio::awaitable<void> httpserver::http2_co_send_304(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t> send_file_obj)
{
    peer->status(304);
    peer->length(0);
    peer->set_header("date", get_gmttime());
    peer->set_header("last-modified", get_gmttime(peer->fileinfo.st_mtime));
    peer->set_header("etag", send_file_obj->etag);
    peer->type(send_file_obj->content_type);
    send_file_obj->header = peer->make_http2_header(0);
    set_http2_headers_flag(send_file_obj->header, HTTP2_HEADER_END_STREAM | HTTP2_HEADER_END_HEADERS);

    // body size 0 enddata
    send_file_obj->content.clear();
    send_file_obj->is_sendheader = false;// usr body not send header

    co_return;
}
asio::awaitable<void> httpserver::http2_co_send_compress(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t> send_file_obj)
{

    server_loaclvar &static_server_var = get_server_global_var();
    if (static_server_var.static_file_compress_cache == false)
    {
        co_return;
    }

    if (static_server_var.temp_path.empty())
    {
        co_return;
    }

    send_file_obj->file_name = static_server_var.temp_path;
    if (send_file_obj->file_name.size() > 0 && send_file_obj->file_name.back() != '/')
    {
        send_file_obj->file_name.push_back('/');
    }
    send_file_obj->file_name.append("statichtml");
    fs::path paths = send_file_obj->file_name;
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                        fs::perm_options::add);
    }

    send_file_obj->file_name.push_back('/');
    send_file_obj->file_name.push_back(send_file_obj->etag.size() > 0 ? send_file_obj->etag[0] : '0');
    paths = send_file_obj->file_name;
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                        fs::perm_options::add);
    }
    send_file_obj->file_name.push_back('/');
    send_file_obj->file_name.append(send_file_obj->etag);
    if (peer->isssl && peer->state.br)
    {
        peer->compress = 2;
        send_file_obj->file_name.append(".br");
    }
    else if (peer->state.gzip)
    {
        peer->compress = 1;
        send_file_obj->file_name.append(".gzip");
    }
    paths = send_file_obj->file_name;

    if (fs::exists(paths))
    {
        // If a compressed cache file already exists, return directly.
        std::unique_ptr<std::FILE, int (*)(FILE *)> fp(std::fopen(send_file_obj->file_name.c_str(), "rb"), std::fclose);
        if (fp)
        {
            fseek(fp.get(), 0, SEEK_END);
            long long file_length = ftell(fp.get());
            fseek(fp.get(), 0, SEEK_SET);
            if (file_length < 0)
            {
                // ftell failed — drop the compressed cache to avoid content_length overflow.
                peer->compress = 0;
            }
            else
            {
                send_file_obj->content_length = static_cast<unsigned long long>(file_length);
                send_file_obj->fp             = std::move(fp);
            }
        }
        else
        {
            // Handle fopen failure.
            peer->compress = 0;
        }

        // std::FILE *fp = std::fopen(send_file_obj->file_name.c_str(), "rb");
        // send_file_obj->fp.reset(std::fopen(send_file_obj->file_name.c_str(), "rb"));
        // if (fp)
        // {
        //     fseek(fp, 0, SEEK_END);
        //     send_file_obj->content_length = ftell(fp);
        //     fseek(fp, 0, SEEK_SET);

        //     send_file_obj->fp.reset(fp);
        // }
        co_return;
    }
    else
    {
        // Streaming compression: read and compress chunks so the whole file never loads into memory.
        if (send_file_obj->content_length > 16877216)
        {
            peer->compress = 0;
            co_return;
        }

        long long file_size = static_cast<long long>(send_file_obj->content_length);
        send_file_obj->content.clear();

        if (peer->compress == 2)
        {
            brotli_encode_file(send_file_obj->fp.get(), file_size, send_file_obj->content);
            send_file_obj->content_length = send_file_obj->content.size();
            send_file_obj->type           = 17;
        }
        else if (peer->compress == 1)
        {
            if (compress_file(send_file_obj->fp.get(), file_size, send_file_obj->content, Z_DEFAULT_COMPRESSION) == Z_OK)
            {
                send_file_obj->content_length = send_file_obj->content.size();
                send_file_obj->type           = 16;
            }
            else
            {
                peer->compress = 0;
                co_return;
            }
        }
        // Persist the compressed result.
        std::unique_ptr<std::FILE, int (*)(FILE *)> fp(std::fopen(send_file_obj->file_name.c_str(), "wb"), std::fclose);
        if (fp.get())
        {
            fwrite(&send_file_obj->content[0], 1, send_file_obj->content.size(), fp.get());
        }
    }
    co_return;
}
asio::awaitable<void> httpserver::http2_co_send_file(std::shared_ptr<httppeer> peer)
{
    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(15360);
    send_file_obj->fp.reset(std::fopen(peer->sendfilename.c_str(), "rb"));

    DEBUG_LOG("%s", peer->sendfilename.c_str());

    long long file_length = -1;
    if (send_file_obj->fp.get())
    {
        fseek(send_file_obj->fp.get(), 0, SEEK_END);
        file_length = ftell(send_file_obj->fp.get());
        fseek(send_file_obj->fp.get(), 0, SEEK_SET);
    }

    // ftell returns -1 when size read fails; treat as access error to avoid content_length overflow.
    if (send_file_obj->fp.get() && file_length >= 0)
    {
        send_file_obj->peer           = peer;
        send_file_obj->type           = 1;
        send_file_obj->content_length = static_cast<unsigned long long>(file_length);

        send_file_obj->file_ext = get_fileext(peer->sendfilename);

        send_file_obj->content_type.clear();

        if (send_file_obj->file_ext.size() > 0)
        {
            auto mime_iter = mime_map.find(send_file_obj->file_ext);

            if (mime_iter != mime_map.end())
            {
                send_file_obj->content_type = mime_iter->second;
            }
        }

        if (send_file_obj->content_type.size() == 0)
        {
            if (send_file_obj->content_length > 204800)
            {
                send_file_obj->content_type = "application/octet-stream";
            }
            else
            {
                send_file_obj->content_type = "text/plain";
            }
        }

        // etag cache
        send_file_obj->etag = make_header_etag(send_file_obj->content_length, peer->fileinfo.st_mtime + peer->url.size());
        if (peer->etag == send_file_obj->etag)
        {
            co_await http2_co_send_304(peer, send_file_obj);
            std::unique_lock<std::mutex> lock(send_data_mutex);
            sent_data_list.emplace_back(send_file_obj);
            lock.unlock();
            send_data_condition.notify_one();
            co_return;
        }

        // compress

        if (peer->state.gzip || peer->state.br)
        {
            if (send_file_obj->content_length > 0 && send_file_obj->content_length < 16877216 && send_file_obj->file_ext.size() > 0 && mime_compress.contains(send_file_obj->file_ext))
            {
                if (send_file_obj->etag.size() > 0)
                {
                    co_await http2_co_send_compress(peer, send_file_obj);
                }
            }
        }

        co_await http2_send_sequence_header(peer, send_file_obj);
        // add send sequence
        std::unique_lock<std::mutex> lock(send_data_mutex);
        sent_data_list.emplace_back(send_file_obj);
        lock.unlock();
        send_data_condition.notify_one();
    }
    else
    {
        send_file_obj->type    = 11;
        send_file_obj->content = "<h3>500 Internal Server Error</h3>";
        send_file_obj->content.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
        // Same as above: in-memory bodies must also set content_length, otherwise the 500 payload gets dropped by the zero-length cleanup branch.
        send_file_obj->content_length = send_file_obj->content.size();
        peer->status(500);
        peer->length(send_file_obj->content.size());
        peer->type("text/html; charset=utf-8");
        send_file_obj->header        = peer->make_http2_header(0);
        send_file_obj->is_sendheader = false;
        std::unique_lock<std::mutex> lock(send_data_mutex);
        sent_data_list.emplace_back(send_file_obj);
        lock.unlock();
        send_data_condition.notify_one();
    }
    co_return;
}

asio::awaitable<void>
httpserver::http2_send_content(unsigned int stream_id, std::string &_send_data, const unsigned char *buffer, unsigned int begin_end, bool is_end)
{
    unsigned int data_send_id = stream_id;
    _send_data.resize(9);
    _send_data[3] = 0x00;
    _send_data[4] = 0x00;
    _send_data[8] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[7] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[6] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[5] = data_send_id & 0xFF;
    data_send_id  = 0;
    if (begin_end == 0)
    {
        _send_data[4] = 0x01;
        _send_data[2] = 0;
        _send_data[1] = 0;
        _send_data[0] = 0;
        co_return;
    }
    if (is_end)
    {
        _send_data[4] = 0x01;
    }
    _send_data[3] = 0x00;
    data_send_id  = begin_end;
    _send_data[2] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[1] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[0] = data_send_id & 0xFF;

    _send_data.append((const char *)buffer, begin_end);

    co_return;
}
asio::awaitable<void>
httpserver::http2_send_content(unsigned int stream_id, std::string &_send_data, const std::string &_source_data, bool is_end)
{
    unsigned int data_send_id = stream_id;
    _send_data.resize(9);
    _send_data[3] = 0x00;
    _send_data[4] = 0x00;
    _send_data[8] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[7] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[6] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[5] = data_send_id & 0xFF;
    data_send_id  = 0;
    if (_source_data.size() == 0)
    {
        _send_data[4] = 0x01;
        _send_data[2] = 0;
        _send_data[1] = 0;
        _send_data[0] = 0;
        co_return;
    }
    if (is_end)
    {
        _send_data[4] = 0x01;
    }
    _send_data[3] = 0x00;
    data_send_id  = _source_data.size();
    _send_data[2] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[1] = data_send_id & 0xFF;
    data_send_id  = data_send_id >> 8;
    _send_data[0] = data_send_id & 0xFF;

    _send_data.append(_source_data);

    co_return;
}

asio::awaitable<void>
httpserver::http2_send_content_append(unsigned int stream_id, std::string &_send_data, const unsigned char *buffer, unsigned int begin_end, bool is_end)
{
    unsigned int data_send_id = stream_id;
    unsigned int new_offset   = static_cast<unsigned int>(_send_data.size());

    _send_data.resize(new_offset + 9);
    _send_data[new_offset + 3] = 0x00;
    _send_data[new_offset + 4] = 0x00;
    _send_data[new_offset + 8] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 7] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 6] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 5] = data_send_id & 0xFF;
    data_send_id               = 0;
    if (begin_end == 0)
    {
        _send_data[new_offset + 4] = 0x01;
        _send_data[new_offset + 2] = 0;
        _send_data[new_offset + 1] = 0;
        _send_data[new_offset + 0] = 0;
        co_return;
    }
    if (is_end)
    {
        _send_data[new_offset + 4] = 0x01;
    }
    _send_data[new_offset + 3] = 0x00;
    data_send_id               = begin_end;
    _send_data[new_offset + 2] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 1] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 0] = data_send_id & 0xFF;

    _send_data.append((const char *)buffer, begin_end);

    co_return;
}

asio::awaitable<void>
httpserver::http2_send_content_append(unsigned int stream_id, std::string &_send_data, const std::string &_source_data, bool is_end)
{
    unsigned int data_send_id = stream_id;
    unsigned int new_offset   = static_cast<unsigned int>(_send_data.size());

    _send_data.resize(new_offset + 9);
    _send_data[new_offset + 3] = 0x00;
    _send_data[new_offset + 4] = 0x00;
    _send_data[new_offset + 8] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 7] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 6] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 5] = data_send_id & 0xFF;
    data_send_id               = 0;
    if (_source_data.size() == 0)
    {
        _send_data[new_offset + 4] = 0x01;
        _send_data[new_offset + 2] = 0;
        _send_data[new_offset + 1] = 0;
        _send_data[new_offset + 0] = 0;
        co_return;
    }
    if (is_end)
    {
        _send_data[new_offset + 4] = 0x01;
    }
    _send_data[new_offset + 3] = 0x00;
    data_send_id               = _source_data.size();
    _send_data[new_offset + 2] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 1] = data_send_id & 0xFF;
    data_send_id               = data_send_id >> 8;
    _send_data[new_offset + 0] = data_send_id & 0xFF;

    _send_data.append(_source_data);

    co_return;
}

asio::awaitable<void> httpserver::http2_send_status_content(std::shared_ptr<httppeer> peer, unsigned int status_code, const std::string &bodycontent)
{
    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(15360);
    send_file_obj->content.clear();

    peer->compress = 0;
    peer->status(status_code);
    peer->length(bodycontent.size());
    peer->type("text/html; charset=utf-8");
    send_file_obj->header  = peer->make_http2_header(0);
    send_file_obj->content = bodycontent;

    // content_length decides whether the sender takes the zero-length cleanup branch
    // (content_length == 0 → only an empty DATA frame). Setting peer->length() alone
    // (which only writes the HEADERS content-length header) is insufficient — the job's
    // own content_length stays 0 after pool reset → body never sent, client gets
    // content-length: N but 0 bytes.
    send_file_obj->content_length = bodycontent.size();
    // type must not be 1 (that's the file branch, which calls fread); use 11 to stay
    // consistent with http2loop's in-memory body branch.
    send_file_obj->type = 11;

    send_file_obj->peer          = peer;
    send_file_obj->is_sendheader = false;
    std::unique_lock<std::mutex> lock(send_data_mutex);
    sent_data_list.emplace_back(send_file_obj);
    lock.unlock();
    send_data_condition.notify_one();

    co_return;
}

// 集中处理 HTTP/2 解析期错误。返回 true 表示连接应被拆除（已 GOAWAY）。
asio::awaitable<bool> httpserver::http2_handle_parse_error(http2parse &h2, std::shared_ptr<client_session> sess)
{
    if (h2.error == 0)
        co_return false;

    if (h2.error_is_conn)
    {
        // 连接级错误（RFC 9113 §5.4.1）：GOAWAY 断连，不带正文。
        // 这里不再"先回一帧 403 让客户端有可读响应"：403 那帧进的是全局发送线程的工作表
        // （sent_data_list），GOAWAY 进的是本连接的发送环，两者没有先后保证，环满时 403
        // 还会整帧丢掉。既然那一帧既可能晚于 GOAWAY 也可能根本发不出去，就不要留一句
        // 和代码不符的承诺——客户端判断为什么断连，看 GOAWAY 的错误码。
        // RFC 9113 §6.8：last_stream_id 是对端判断「哪些流可以安全重发」的唯一依据，
        // 报 0 等于谎称一条流都没处理过，已应答的 POST 会被重发；错误码同理要带出去。
        co_await sess->async_send_goway(h2.max_client_stream_id, h2.h2_error_code);
        co_return true;
    }

    // 流级错误（RFC 9113 §5.4.2）：只 RST_STREAM 重置该流，保留连接。
    sess->http2_send_rst_stream(h2.error_stream_id, h2.h2_error_code);
    h2.cleanup_stream(h2.error_stream_id);
    h2.clear_error();
    // 落到主循环末尾统一唤醒发送协程，把 RST 真正发走（read_some 可能长期阻塞，
    // 不能依赖下一轮；直接 waituphttp2() 与发送协程停泊存在竞态，统一在末尾兜底最稳）。
    h2.need_wakeup_send_threads = true;
    co_return false;
}

asio::awaitable<void> httpserver::send_cors_domain(std::shared_ptr<httppeer> peer)
{
    // OPTIONS preflight: emit capability headers, decide Allow-Origin from whitelist,
    // reply 200 with an empty body — no business handler invoked.
    // Allow-Origin for regular requests is decided by cors_origin_process() during parsing.
    peer->cors_method();
    peer->status(200);
    peer->length(0);

    // Strip headers that cors_origin_process() may have set during parsing; preflight
    // must emit its own set according to the spec:
    //   ACAO:     may be omitted if Origin whitelist or Request-Method checks fail
    //   Vary:     only emitted when whitelist matches (echoes the specific origin);
    //             must not linger when preflight fails
    //   Expose-Headers: response-only header, preflight must not carry it
    // h1 headers live in peer->send_header (set_header() preserves case for h1, so we
    // clear both cases); h2 ACAO and Vary hit HPACK static table entries and are stored
    // by index in http2_send_header rather than send_header — erase by slot, otherwise
    // an ACAO set during parsing still leaks out when preflight fails.
    peer->send_header.erase("Access-Control-Allow-Origin");
    peer->send_header.erase("access-control-allow-origin");
    peer->send_header.erase("Vary");
    peer->send_header.erase("vary");
    peer->send_header.erase("Access-Control-Expose-Headers");
    peer->send_header.erase("access-control-expose-headers");
    // 这个头名不在 HPACK 静态表里，h2 走 send_header 的字面量分支，不用按槽号擦
    peer->send_header.erase("Access-Control-Allow-Credentials");
    peer->send_header.erase("access-control-allow-credentials");
    peer->http2_send_header.erase(HTTP2_CODE_access_control_allow_origin);// 20
    peer->http2_send_header.erase(HTTP2_CODE_vary);                       // 59

    // Allow-Origin — same policy as regular requests, via cors_allow_origin():
    //   explicit cors_domain = "*" → "*"
    //   whitelist match             → echo the request Origin as-is
    //   no match / site has no whitelist → omit ACAO (preflight fails; default deny).
    std::string req_origin = peer->get_header("origin");
    std::string allow_origin;
    bool site_credentials = false;
    // Method list and credentials flag both come from this site's config; hold the
    // pointer so we don't re-lookup below.
    const site_host_info_t *site_cors = nullptr;
    {
        serverconfig &sysconfigpath = getserversysconfig();
        if (peer->host_index < sysconfigpath.sitehostinfos.size())
        {
            site_cors        = &sysconfigpath.sitehostinfos[peer->host_index];
            allow_origin     = site_cors->cors_allow_origin(req_origin);
            site_credentials = site_cors->cors_credentials;
        }
    }
    // Validate Access-Control-Request-Method; preflight fails if it's not allowed.
    // （仍回 200 但不带 ACAO，浏览器自然拒绝）
    // The list shares the same source as the Allow-Methods response header (site config
    // cors_allow_methods); str_casecmp is case-insensitive.
    if (site_cors != nullptr && !allow_origin.empty())
    {
        std::string req_method = peer->get_header("access-control-request-method");
        if (!req_method.empty())
        {
            bool method_allowed = false;
            for (auto &allow_method_item : site_cors->cors_allow_methods)
            {
                if (str_casecmp(req_method, allow_method_item))
                {
                    method_allowed = true;
                    break;
                }
            }
            if (!method_allowed)
            {
                allow_origin.clear();
            }
        }
    }
    if (!allow_origin.empty())
    {
        peer->set_header("Access-Control-Allow-Origin", allow_origin);
        // Credentialed cross-origin requires Allow-Credentials on both preflight and the
        // actual response; browsers won't send credentials without the preflight header.
        // Emit it under the same conditions as the response: site enables cors_credentials
        // and ACAO echoes a specific Origin (not "*").
        if (site_credentials && allow_origin != "*")
        {
            peer->set_header("Access-Control-Allow-Credentials", "true");
        }
    }

    // Allow-Headers: browsers send Access-Control-Request-Headers whenever they include
    // non-safelisted request headers (Content-Type: application/json, x-requested-with,
    // etc.). We must echo the request to Allow-Headers or the preflight is blocked.
    // If the request has none (purely safelisted headers), reply "*" as fallback.
    // Request header names are lowercased during parsing (http_parse.cpp); get_header()
    // also looks up lowercased keys — consistent between h1 and h2.
    std::string req_headers = peer->get_header("access-control-request-headers");
    if (!req_headers.empty())
    {
        peer->set_header("Access-Control-Allow-Headers", req_headers);
    }
    else
    {
        peer->set_header("Access-Control-Allow-Headers", "*");
    }

    // Vary: the preflight response is shaped by three request headers; omitting the
    // declaration lets intermediate caches / CDNs mis-serve it —
    //   Origin                          → which ACAO value we echo
    //   Access-Control-Request-Method   → whether ACAO is emitted at all (preflight
    //                                       fails if method is not allowed)
    //   Access-Control-Request-Headers  → which Allow-Headers we echo
    // Only emit when whitelist matches (allow_origin non-empty): on preflight failure
    // we shouldn't advertise these capabilities anyway, consistent with "no ACAO on
    // failure". When ACAO is the literal "*" (cors_domain explicitly set), skip the
    // Origin Vary entry.
    if (!allow_origin.empty())
    {
        if (allow_origin == "*")
        {
            peer->add_vary("Access-Control-Request-Method, Access-Control-Request-Headers");
        }
        else
        {
            peer->add_vary("Origin, Access-Control-Request-Method, Access-Control-Request-Headers");
        }
    }

    if (peer->httpv == 2)
    {
        http2_send_queue &send_queue_obj = get_http2_send_queue();
        auto send_file_obj               = send_queue_obj.get_cache_ptr();
        send_file_obj->cache_data.resize(15360);
        send_file_obj->content.clear();

        peer->compress                = 0;
        send_file_obj->header         = peer->make_http2_header(0);
        send_file_obj->content_length = 0;
        send_file_obj->type           = 11;
        send_file_obj->peer           = peer;
        send_file_obj->is_sendheader  = false;

        std::unique_lock<std::mutex> lock(send_data_mutex);
        sent_data_list.emplace_back(send_file_obj);
        lock.unlock();
        send_data_condition.notify_one();
    }
    else
    {
        std::string htmlcontent = peer->make_http1_header();
        htmlcontent.append("\r\n");
        co_await peer->socket_session->async_send_writer(htmlcontent);
    }
    co_return;
}
void httpserver::http2_compress_output(std::shared_ptr<httppeer> &peer, std::shared_ptr<http2_send_data_t> &send_file_obj)
{
    peer->compress = 0;
    if (peer->state.gzip || peer->state.br)
    {
        if (str_casecmp(peer->content_type, "text/html; charset=utf-8") ||
            str_casecmp(peer->content_type, "application/json") ||
            str_casecmp(peer->content_type, "text/html") ||
            str_casecmp(peer->content_type, "application/json; charset=utf-8"))
        {
            if (peer->output.size() > 100)
            {
                if (peer->state.br)
                {
                    brotli_encode(peer->output, send_file_obj->content);
                    peer->compress = 2;
                }
                else if (peer->state.gzip)
                {
                    if (compress(peer->output.data(),
                                 peer->output.size(),
                                 send_file_obj->content,
                                 Z_DEFAULT_COMPRESSION) == Z_OK)
                    {
                        peer->compress = 1;
                    }
                }
            }
        }
    }

    if (peer->compress > 0)
    {
        send_file_obj->content_length = send_file_obj->content.size();
        peer->length(send_file_obj->content.size());
    }
    else
    {
        send_file_obj->content_length = peer->output.size();
        peer->length(peer->output.size());
    }
}
void httpserver::clear_peer_data(std::shared_ptr<httppeer> &peer)
{
    peer->output.clear();
    peer->output.shrink_to_fit();
    peer->get.clear();
    peer->post.clear();
    peer->files.clear();
    peer->json.clear();
    peer->rawcontent.clear();
    peer->rawcontent.shrink_to_fit();
    peer->val.clear();
    peer->cookie.clear();
    peer->session.clear();
}
asio::awaitable<bool> httpserver::http2_static_file_authority(std::shared_ptr<httppeer> peer)
{
    serverconfig &sysconfigpath = getserversysconfig();
    unsigned int p_s            = static_cast<unsigned int>(sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists.size());
    std::string htmlcontent;
    DEBUG_LOG("static_pre_lists:%zu", sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists.size());
    // all static files
    if (p_s == 0)
    {
        htmlcontent = co_await get_router().co_call_regfun(peer, sysconfigpath.sitehostinfos[peer->host_index].static_pre_method);
        if (htmlcontent.size() == 0)
        {
            co_return true;
        }
        else
        {
            http2_send_queue &send_queue_obj = get_http2_send_queue();
            auto send_file_obj               = send_queue_obj.get_cache_ptr();
            send_file_obj->cache_data.resize(15360);
            send_file_obj->content.clear();

            peer->status(403);
            if (!peer->isset_type())
            {
                peer->type("text/html; charset=utf-8");
            }

            http2_compress_output(peer, send_file_obj);

            send_file_obj->header = peer->make_http2_header(0);

            if (peer->compress == 0)
            {
                send_file_obj->content = std::move(peer->output);
            }
            send_file_obj->peer          = peer;
            send_file_obj->is_sendheader = false;
            std::unique_lock<std::mutex> lock(send_data_mutex);
            sent_data_list.emplace_back(send_file_obj);
            lock.unlock();
            send_data_condition.notify_one();

            clear_peer_data(peer);

            co_return false;
        }
    }
    unsigned int j = static_cast<unsigned int>(peer->urlpath.size());
    DEBUG_LOG("sendfilename:%s", peer->urlpath.c_str());
    for (size_t i = 0; i < p_s; i++)
    {
        unsigned int k = static_cast<unsigned int>(sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i].size());
        if (k > j)
        {
            co_return true;
        }
        if (k > 0)
        {
            unsigned int n = 0;
            for (; n < k; n++)
            {
                if (sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i][n] != peer->urlpath[n + 1])
                {
                    break;
                }
            }
            DEBUG_LOG("check list %u %u", n, k);
            if (n == k)
            {
                DEBUG_LOG("static_pre_lists: %zu %s", i, sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i].c_str());
                // match pre urlpath
                htmlcontent = co_await get_router().co_call_regfun(peer, sysconfigpath.sitehostinfos[peer->host_index].static_pre_method);
                if (htmlcontent.size() == 0)
                {
                    co_return true;
                }
                else
                {
                    http2_send_queue &send_queue_obj = get_http2_send_queue();
                    auto send_file_obj               = send_queue_obj.get_cache_ptr();
                    send_file_obj->cache_data.resize(15360);
                    send_file_obj->content.clear();

                    peer->status(403);
                    if (!peer->isset_type())
                    {
                        peer->type("text/html; charset=utf-8");
                    }

                    http2_compress_output(peer, send_file_obj);

                    send_file_obj->header = peer->make_http2_header(0);

                    if (peer->compress == 0)
                    {
                        send_file_obj->content = std::move(peer->output);
                    }
                    send_file_obj->peer          = peer;
                    send_file_obj->is_sendheader = false;
                    std::unique_lock<std::mutex> lock(send_data_mutex);
                    sent_data_list.emplace_back(send_file_obj);
                    lock.unlock();
                    send_data_condition.notify_one();

                    clear_peer_data(peer);

                    co_return false;
                }
            }
        }
    }
    DEBUG_LOG("not authority");
    co_return true;
}

// — PHP 文件查找：从原 isuse_fastcgi 里拆出来，只做 php_root_document + rewrite_php_lists 检查，
//    不查路由表（路由预查已在 loop 里完成）、不碰 wwwpath 静态文件判断（wwwpath 里有文件/目录直接走磁盘）。
//    返回 true → 已设 compress=10/linktype，调用方走 fastcgi；false → 没找到，发 404 —
bool httpserver::check_php_dispatch(std::shared_ptr<httppeer> peer)
{
    serverconfig &sysconfigpath = getserversysconfig();
    auto &hostinfo              = sysconfigpath.sitehostinfos[peer->host_index];

    peer->compress = 0;
    peer->linktype = 0;
    peer->output.clear();
    peer->sendfilename.clear();

    if (hostinfo.isuse_php != 1)
    {
        return false;
    }

    DEBUG_LOG("check php file");
    if (peer->pathinfos.empty())
    {
        // no url path: if default index not exist try index.php in php root document
        if (stat_is_regfile(hostinfo.wwwpath + hostinfo.document_index))
        {
            return false;
        }
        if (!hostinfo.php_root_document.empty() &&
            stat_is_regfile(hostinfo.php_root_document + "index.php"))
        {
            peer->compress = 10;
            peer->linktype = 12;
        }
        return false;
    }

    for (unsigned int i = 0; i < peer->pathinfos.size(); i++)
    {
        if (!peer->sendfilename.empty())
        {
            peer->sendfilename.append("/");
        }
        peer->sendfilename.append(peer->pathinfos[i]);
        auto const &seg = peer->pathinfos[i];
        if (seg.size() > 4 && seg[seg.size() - 1] == 'p' && seg[seg.size() - 2] == 'h' &&
            seg[seg.size() - 3] == 'p' && seg[seg.size() - 4] == '.')
        {
            // xxx.php exist in php root document
            if (stat_is_regfile(hostinfo.php_root_document + peer->sendfilename))
            {
                peer->compress = 10;
                peer->linktype = (i == 0) ? 10 : (50 + i);
                DEBUG_LOG("is php file %s", peer->sendfilename.c_str());
                return true;
            }
        }
    }

    DEBUG_LOG("check urlpath reg %s", peer->sendfilename.c_str());
    // whole url path exist in wwwpath: static file or directory → 不走 php
    struct stat sessfileinfo;
    std::string tempac = hostinfo.wwwpath + peer->sendfilename;
    memset(&sessfileinfo, 0, sizeof(sessfileinfo));
    if (stat(tempac.c_str(), &sessfileinfo) == 0)
    {
        if (sessfileinfo.st_mode & S_IFREG)
        {
            return false;// wwwpath 里有同名静态文件
        }
        else if (sessfileinfo.st_mode & S_IFDIR)
        {
            // directory, try index.php in php root document
            if (stat_is_regfile(hostinfo.php_root_document + peer->sendfilename + "/index.php"))
            {
                peer->compress = 10;
                peer->linktype = 15;
                peer->sendfilename.append("/index.php");
                return true;
            }
            return false;
        }
    }

    DEBUG_LOG("rewrite_php_lists: %zu", hostinfo.rewrite_php_lists.size());
    // url path not exist in wwwpath, check php rewrite rules
    if (!hostinfo.rewrite_php_lists.empty())
    {
        unsigned int i = 0;
        if (hostinfo.rewrite_php_lists[0].first.empty())
        {
            // empty prefix, default rewrite entry
            if (stat_is_regfile(hostinfo.php_root_document + hostinfo.rewrite_php_lists[0].second))
            {
                peer->compress = 10;
                peer->linktype = 19;
                return true;
            }
            i = 1;
        }
        for (; i < hostinfo.rewrite_php_lists.size(); i++)
        {
            auto const &rewrite_pre = hostinfo.rewrite_php_lists[i].first;
            if (rewrite_pre.size() <= peer->sendfilename.size() &&
                peer->sendfilename.compare(0, rewrite_pre.size(), rewrite_pre) == 0)
            {
                peer->compress = 10;
                peer->linktype = 20 + i;
                return true;
            }
        }
    }
    return false;
}

asio::awaitable<void> httpserver::http2_fastcgi(std::shared_ptr<httppeer> peer)
{
    DEBUG_LOG("http2_fastcgi in");

    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(15360);
    send_file_obj->content.clear();

    peer->parse_session();
    peer->status(200);
    peer->content_type.clear();
    peer->etag.clear();

    co_await co_user_fastcgi_task(peer);

    http2_compress_output(peer, send_file_obj);

    if (peer->get_status() < 100)
    {
        peer->status(200);
    }

    DEBUG_LOG("htttp2 php out");

    if (!peer->isset_type())
    {
        peer->type("text/html; charset=utf-8");
    }
    send_file_obj->header = peer->make_http2_header(0);

    DEBUG_LOG("fastcgi send content");

    if (peer->compress == 0)
    {
        send_file_obj->content = std::move(peer->output);
    }
    send_file_obj->peer          = peer;
    send_file_obj->is_sendheader = false;
    std::unique_lock<std::mutex> lock(send_data_mutex);
    sent_data_list.emplace_back(send_file_obj);
    lock.unlock();
    send_data_condition.notify_one();

    clear_peer_data(peer);

    co_return;
}
asio::awaitable<void> httpserver::http2loop(std::shared_ptr<httppeer> peer)
{
    try
    {
        serverconfig &sysconfigpath = getserversysconfig();

        // sitepath 必须在预查之前定好：带点 URL 的「先落盘」要用它拼出盘上那个文件
        peer->sitepath = sysconfigpath.getsitewwwpath(peer->host_index);

        // 路由预查：带点 URL 先落盘，然后两段查表；>=0 是命中的 handler 下标
        int routeidx = peer->prefetch_routing();

        // compress==10 是解析阶段的轻量 php 后缀标记；真正要不要走 fastcgi：
        // 先排除注解路由已命中（routeidx>=0 优先），再查 hostinfo.isuse_php 和 php_root_document
#ifdef ENABLE_FASTCGI
        if (peer->compress == 10 && routeidx < 0)
        {
            if (peer->host_index < sysconfigpath.sitehostinfos.size() && sysconfigpath.sitehostinfos[peer->host_index].isuse_php != 1)
            {
                // 本站没开 php 支持，请求 .php → 直接 404，不走动态链
                peer->compress = 0;
            }
            else if (!check_php_dispatch(peer))
            {
                // 开了 php 但找不到对应 .php 文件 → 清零 compress，走正常磁盘/动态链流程
                peer->compress = 0;
            }
            else
            {
                // check_php_dispatch 已设好 compress/linktype/sendfilename，直接走 fastcgi
                co_await http2_fastcgi(peer);
                co_return;
            }
        }
#endif
        // 注解路由命中 → 直接进动态链
        unsigned char sendtype = 0;
        if (routeidx >= 0)
        {
            // 跳过 get_fileinfo，直接进 router 动态链
        }
        else if (!peer->sitepath.empty())
        {
            sendtype = peer->get_fileinfo();
        }
        DEBUG_LOG("http2loop:%s regfun:%d sendtype:%d", peer->sendfilename.c_str(), routeidx, sendtype);
        if (sendtype == 1)
        {
            DEBUG_LOG("is_static_pre:%d", sysconfigpath.sitehostinfos[peer->host_index].is_static_pre);
            if (peer->host_index < sysconfigpath.sitehostinfos.size() && sysconfigpath.sitehostinfos[peer->host_index].is_static_pre)
            {
                if ((co_await http2_static_file_authority(peer)))
                {
                    peer->output.clear();
                }
                else
                {
                    co_return;
                }
            }

            if (peer->state.rangebytes)
            {
                co_await http2_send_file_range(peer);
                co_return;
            }
            else
            {
                co_await http2_co_send_file(peer);
                co_return;
            }
        }
        else if (sendtype == 2 && peer->host_index < sysconfigpath.sitehostinfos.size() && sysconfigpath.sitehostinfos[peer->host_index].is_show_directory)
        {
            peer->output = displaydirectory(peer->sendfilename,
                                            peer->urlpath,
                                            peer->get["sort"].as_string(),
                                            sysconfigpath.configpath);
            peer->status(200);
            peer->type("text/html; charset=utf-8");

            http2_send_queue &send_queue_obj = get_http2_send_queue();
            auto send_file_obj               = send_queue_obj.get_cache_ptr();
            send_file_obj->cache_data.resize(15360);
            send_file_obj->content.clear();
            http2_compress_output(peer, send_file_obj);

            if (peer->get_status() < 100)
            {
                peer->status(200);
            }
            DEBUG_LOG("htttp2 displaydirectory out");
            if (!peer->isset_type())
            {
                peer->type("text/html; charset=utf-8");
            }
            send_file_obj->header = peer->make_http2_header(0);

            if (peer->compress == 0)
            {
                send_file_obj->content = std::move(peer->output);
            }
            send_file_obj->peer          = peer;
            send_file_obj->is_sendheader = false;
            send_file_obj->type          = 11;
            std::unique_lock<std::mutex> lock(send_data_mutex);
            sent_data_list.emplace_back(send_file_obj);
            lock.unlock();
            send_data_condition.notify_one();

            clear_peer_data(peer);
            co_return;
        }
        else
        {
            DEBUG_LOG("htttp2 pool in %u", peer->stream_id);
            peer->parse_session();
            peer->status(200);
            peer->content_type.clear();
            peer->type("text/html; charset=utf-8");
            peer->linktype = 0;
            peer->etag.clear();

            http2_send_queue &send_queue_obj = get_http2_send_queue();
            auto send_file_obj               = send_queue_obj.get_cache_ptr();
            send_file_obj->cache_data.resize(15360);
            send_file_obj->content.clear();
            send_file_obj->header.clear();

            DEBUG_LOG("---  htttp2 co handle --------");

            co_await get_router().co_resolve(peer, routeidx);

            if (peer->ischunked)
            {
                co_return;
            }
            send_file_obj->header.clear();

            http2_compress_output(peer, send_file_obj);

            if (peer->get_status() < 100)
            {
                peer->status(200);
            }

            if (!peer->isset_type())
            {
                peer->type("text/html; charset=utf-8");
            }
            send_file_obj->header = peer->make_http2_header(0);
            DEBUG_LOG("http2_send_content");
            if (peer->compress == 0)
            {
                send_file_obj->content = std::move(peer->output);
            }
            send_file_obj->peer          = peer;
            send_file_obj->is_sendheader = false;
            send_file_obj->type          = 11;
            std::unique_lock<std::mutex> lock(send_data_mutex);
            sent_data_list.emplace_back(send_file_obj);
            lock.unlock();
            send_data_condition.notify_one();

            clear_peer_data(peer);
            co_return;
        }
        co_return;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("http2loop exception send_goway");
        peer->isclose = true;
    }
    if (peer->isclose)
    {
        DEBUG_LOG("http2loop exception send_goway");
        co_await peer->socket_session->async_send_goway();
    }
    co_return;
}

asio::awaitable<void> httpserver::http1_send_file_header(std::shared_ptr<httppeer> peer,
                                                         std::shared_ptr<client_session> peer_session,
                                                         std::shared_ptr<http2_send_data_t> sq_obj)
{
    peer->status(200);
    peer->length(sq_obj->content_length);
    peer->type(sq_obj->content_type);
    if (peer->compress == 0)
    {
        peer->set_header("accept-ranges", "bytes");
    }
    DEBUG_LOG("start send file");
    peer->set_header("date", get_gmttime());
    peer->set_header("last-modified", get_gmttime(peer->fileinfo.st_mtime));

    peer->set_header("etag", sq_obj->etag);

    sq_obj->header = peer->make_http1_header();
    sq_obj->header.append("\r\n");
    co_await peer_session->async_send_writer(sq_obj->header);
    co_return;
}
asio::awaitable<void> httpserver::http1_co_send_304(std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session, std::shared_ptr<http2_send_data_t> send_file_obj)
{

    DEBUG_LOG("http1_send_file:status 304");
    peer->status(304);
    peer->length(0);
    peer->set_header("date", get_gmttime());
    peer->set_header("last-modified", get_gmttime(peer->fileinfo.st_mtime));
    peer->set_header("etag", send_file_obj->etag);
    peer->type(send_file_obj->content_type);
    send_file_obj->only_send_header = true;
    send_file_obj->content.clear();
    send_file_obj->content = peer->make_http1_header();
    send_file_obj->content.append("\r\n");
    co_await peer_session->async_send_writer(send_file_obj->content);

    co_return;
}
asio::awaitable<void> httpserver::http1_send_file(std::shared_ptr<httppeer> peer,
                                                  std::shared_ptr<client_session> peer_session)
{

    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(15360);
    send_file_obj->fp.reset(std::fopen(peer->sendfilename.c_str(), "rb"));

    DEBUG_LOG("news http1_send_file %s", peer->sendfilename.c_str());

    long long file_length = -1;
    if (send_file_obj->fp.get())
    {
        fseek(send_file_obj->fp.get(), 0, SEEK_END);
        file_length = ftell(send_file_obj->fp.get());
        fseek(send_file_obj->fp.get(), 0, SEEK_SET);
    }

    // ftell returns -1 when size read fails; treat as access error to avoid content_length overflow.
    if (send_file_obj->fp.get() && file_length >= 0)
    {
        send_file_obj->peer           = peer;
        send_file_obj->type           = 1;
        send_file_obj->content_length = static_cast<unsigned long long>(file_length);

        send_file_obj->file_ext = get_fileext(peer->sendfilename);

        send_file_obj->content_type.clear();

        if (send_file_obj->file_ext.size() > 0)
        {
            auto mime_iter = mime_map.find(send_file_obj->file_ext);

            if (mime_iter != mime_map.end())
            {
                send_file_obj->content_type = mime_iter->second;
            }
        }

        if (send_file_obj->content_type.size() == 0)
        {
            if (send_file_obj->content_length > 204800)
            {
                send_file_obj->content_type = "application/octet-stream";
            }
            else
            {
                send_file_obj->content_type = "text/plain";
            }
        }

        // etag cache
        send_file_obj->etag = make_header_etag(send_file_obj->content_length, peer->fileinfo.st_mtime + peer->url.size());
        if (peer->etag == send_file_obj->etag)
        {
            co_await http1_co_send_304(peer, peer_session, send_file_obj);
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        // compress

        if (peer->state.gzip || peer->state.br)
        {
            if (send_file_obj->content_length > 0 && send_file_obj->content_length < 16877216 && send_file_obj->file_ext.size() > 0 && mime_compress.contains(send_file_obj->file_ext))
            {
                if (send_file_obj->etag.size() > 0)
                {
                    co_await http2_co_send_compress(peer, send_file_obj);
                }
            }
        }

        co_await http1_send_file_header(peer, peer_session, send_file_obj);
        if (send_file_obj->content_length == 0)
        {
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        if (send_file_obj->type == 1)
        {
            try
            {
                unsigned long long readnum = 0;
                unsigned int f_inc         = 1;
                // content_length is the absolute value "range end + 1"; readnum is bytes already
                // read this call (starts from 0). Must subtract current_num (always 0 for
                // non-range requests). Do not use the raw absolute value as the upper bound:
                // Range: bytes=500-599 would read 600 bytes → advertise 100 but actually send
                // 500, violating the request range and leaking file content.
                unsigned long long need_send_num = send_file_obj->content_length - send_file_obj->current_num;
                while (readnum < need_send_num)
                {
                    // Per-read size must also cap at the remaining bytes — otherwise the
                    // final fread reads a full 4096 and overshoots.
                    unsigned int want_num = 4096;
                    if ((need_send_num - readnum) < (unsigned long long)want_num)
                    {
                        want_num = (unsigned int)(need_send_num - readnum);
                    }
                    send_file_obj->content.resize(want_num);
                    if (!send_file_obj->fp.get())
                    {
                        peer_session->isclose = true;
                        peer->isclose         = true;
                        peer->state.keepalive = false;
                        break;
                    }
                    unsigned int nread = fread(&send_file_obj->content[0], 1, want_num, send_file_obj->fp.get());
                    if (nread == 0)
                    {
                        DEBUG_LOG("nread 0 ");
                        peer_session->isclose = true;
                        peer->isclose         = true;
                        peer->state.keepalive = false;
                        break;
                    }
                    send_file_obj->content.resize(nread);
                    // peer_session->send_data(htmlcontent);
                    co_await peer_session->async_send_writer(send_file_obj->content);
                    readnum += nread;
                    if (peer_session->isclose)
                    {
                        break;
                    }
                    if (f_inc % 1024 == 0)
                    {
                        peer_session->time_limit.store(timeid());
                    }
                    f_inc++;
                }
                send_queue_obj.back_cache_ptr(send_file_obj);
                co_return;
            }
            catch (std::exception &e)
            {
                DEBUG_LOG("http1_send_file exception");
            }

            co_await http1_send_bad_server(peer, peer_session);
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }
        else
        {
            co_await peer_session->async_send_writer(send_file_obj->content);
        }
    }
    else
    {
        co_await http1_send_bad_server(peer, peer_session);
    }
    send_file_obj->content.clear();
    send_queue_obj.back_cache_ptr(send_file_obj);
    co_return;
}

asio::awaitable<void> httpserver::http1_send_file_range(std::shared_ptr<httppeer> peer,
                                                        std::shared_ptr<client_session> peer_session)
{

    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(15360);
    send_file_obj->fp.reset(std::fopen(peer->sendfilename.c_str(), "rb"));

    DEBUG_LOG("news http1_send_file_range %s", peer->sendfilename.c_str());

    long long file_length = -1;
    if (send_file_obj->fp.get())
    {
        fseek(send_file_obj->fp.get(), 0, SEEK_END);
        file_length = ftell(send_file_obj->fp.get());
        fseek(send_file_obj->fp.get(), 0, SEEK_SET);
    }

    // ftell returns -1 when size read fails; treat as access error to avoid content_length overflow.
    if (send_file_obj->fp.get() && file_length >= 0)
    {
        send_file_obj->peer           = peer;
        send_file_obj->type           = 1;
        send_file_obj->content_length = static_cast<unsigned long long>(file_length);

        send_file_obj->file_ext = get_fileext(peer->sendfilename);

        send_file_obj->content_type.clear();

        if (send_file_obj->file_ext.size() > 0)
        {
            auto mime_iter = mime_map.find(send_file_obj->file_ext);

            if (mime_iter != mime_map.end())
            {
                send_file_obj->content_type = mime_iter->second;
            }
        }

        if (send_file_obj->content_type.size() == 0)
        {
            if (send_file_obj->content_length > 204800)
            {
                send_file_obj->content_type = "application/octet-stream";
            }
            else
            {
                send_file_obj->content_type = "text/plain";
            }
        }

        unsigned long long filesize = send_file_obj->content_length;
        // 后缀范围 bytes=-N 先归一化成绝对范围，再统一做合法性判断。
        // 不能用 "rangebegin > 0 / rangeend > 0" 反推是否给过端点：bytes=0-0 与 bytes=-N 都会读错。
        if (peer->state.rangebytes && peer->state.range_suffix)
        {
            unsigned long long suffix_len = peer->state.rangeend;
            if (suffix_len > filesize)
            {
                suffix_len = filesize;
            }
            peer->state.rangebegin    = filesize - suffix_len;
            peer->state.rangeend      = (filesize > 0) ? (filesize - 1) : 0;
            peer->state.range_has_end = true;
        }

        if (peer->state.rangebegin >= filesize ||
            (peer->state.range_has_end && peer->state.rangeend >= filesize) ||
            (peer->state.range_has_end && peer->state.rangeend < peer->state.rangebegin))
        {
            // http1_send_bad_request() 的响应头是 Connection: close，
            // 这里同步关掉 keep-alive，避免外层循环继续按长连接等待下一个请求。
            peer->state.keepalive = false;
            co_await http1_send_bad_request(400, peer_session);
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        unsigned int statecode = 200;
        if (peer->state.rangebytes)
        {
            send_file_obj->current_num = peer->state.rangebegin;
            statecode                  = 206;
            fseek(send_file_obj->fp.get(), send_file_obj->current_num, SEEK_SET);
        }
        if (peer->state.range_has_end)
        {
            // 末端越界按 RFC 7233 收敛到最后一个字节：门槛必须含等号，
            // 只拦 rangeend > filesize 时 rangeend == filesize 声明长度会多出 1 字节
            if (peer->state.rangeend >= filesize)
            {
                peer->state.rangeend = filesize - 1;
            }
            send_file_obj->content_length = peer->state.rangeend + 1;
            statecode                     = 206;
        }

        peer->status(statecode);
        peer->length(send_file_obj->content_length - send_file_obj->current_num);
        peer->type(send_file_obj->content_type);

        send_file_obj->etag = make_header_etag(filesize, peer->fileinfo.st_mtime + peer->url.size());

        if (statecode == 206)
        {
            peer->set_header("content-range",
                             "bytes " + std::to_string(send_file_obj->current_num) + "-" + std::to_string(send_file_obj->content_length - 1) + "/" +
                                 std::to_string(filesize));
        }
        else
        {
            peer->set_header("accept-ranges", "bytes");
        }
        DEBUG_LOG("start http1 send file range");
        peer->set_header("date", get_gmttime());
        peer->set_header("last-modified", get_gmttime(peer->fileinfo.st_mtime));
        peer->set_header("etag", send_file_obj->etag);
        send_file_obj->header = peer->make_http1_header();
        send_file_obj->header.append("\r\n");

        co_await peer_session->async_send_writer(send_file_obj->header);

        if ((send_file_obj->content_length - send_file_obj->current_num) == 0)
        {
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        if (send_file_obj->type == 1)
        {
            try
            {
                unsigned long long readnum = 0;
                unsigned int f_inc         = 1;
                // content_length is the absolute value "range end + 1"; readnum is bytes already
                // read this call (starts from 0). Must subtract current_num (always 0 for
                // non-range requests). Do not use the raw absolute value as the upper bound:
                // Range: bytes=500-599 would read 600 bytes → advertise 100 but actually send
                // 500, violating the request range and leaking file content.
                unsigned long long need_send_num = send_file_obj->content_length - send_file_obj->current_num;
                while (readnum < need_send_num)
                {
                    // Per-read size must also cap at the remaining bytes — otherwise the
                    // final fread reads a full 4096 and overshoots.
                    unsigned int want_num = 4096;
                    if ((need_send_num - readnum) < (unsigned long long)want_num)
                    {
                        want_num = (unsigned int)(need_send_num - readnum);
                    }
                    send_file_obj->content.resize(want_num);
                    if (!send_file_obj->fp.get())
                    {
                        peer_session->isclose = true;
                        peer->isclose         = true;
                        peer->state.keepalive = false;
                        break;
                    }
                    unsigned int nread = fread(&send_file_obj->content[0], 1, want_num, send_file_obj->fp.get());
                    if (nread == 0)
                    {
                        DEBUG_LOG("nread 0 ");
                        peer_session->isclose = true;
                        peer->isclose         = true;
                        peer->state.keepalive = false;
                        break;
                    }
                    send_file_obj->content.resize(nread);

                    co_await peer_session->async_send_writer(send_file_obj->content);
                    readnum += nread;
                    if (peer_session->isclose)
                    {
                        break;
                    }
                    if (f_inc % 1024 == 0)
                    {
                        peer_session->time_limit.store(timeid());
                    }
                    f_inc++;
                }
                send_queue_obj.back_cache_ptr(send_file_obj);
                co_return;
            }
            catch (std::exception &e)
            {
                DEBUG_LOG("http1_send_file exception");
            }

            co_await http1_send_bad_server(peer, peer_session);
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }
    }
    else
    {
        co_await http1_send_bad_server(peer, peer_session);
    }
    send_queue_obj.back_cache_ptr(send_file_obj);
    co_return;
}

asio::awaitable<void> httpserver::http1_send_status_content(std::shared_ptr<httppeer> peer, unsigned int status_code, const std::string &bodycontent)
{
    std::string htmlcontent;
    peer->status(status_code);
    peer->type("text/html; charset=utf-8");
    peer->length(bodycontent.size());
    htmlcontent = peer->make_http1_header();
    htmlcontent.append("\r\n");
    co_await peer->socket_session->async_send_writer(htmlcontent);
    if (bodycontent.size() > 0)
    {
        co_await peer->socket_session->async_send_writer(bodycontent);
    }
    co_return;
}
asio::awaitable<bool> httpserver::http1_static_file_authority(std::shared_ptr<httppeer> peer)
{
    serverconfig &sysconfigpath = getserversysconfig();
    unsigned int p_s            = static_cast<unsigned int>(sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists.size());
    std::string htmlcontent;
    DEBUG_LOG("static_pre_lists:%zu", sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists.size());
    // all static files
    if (p_s == 0)
    {
        htmlcontent = co_await get_router().co_call_regfun(peer, sysconfigpath.sitehostinfos[peer->host_index].static_pre_method);
        if (htmlcontent.size() == 0)
        {
            co_return true;
        }
        else
        {
            htmlcontent.append(peer->output);
            co_await http1_send_status_content(peer, 403, htmlcontent);
            co_return false;
        }
    }
    unsigned int j = static_cast<unsigned int>(peer->urlpath.size());
    DEBUG_LOG("sendfilename:%s", peer->urlpath.c_str());
    for (size_t i = 0; i < p_s; i++)
    {
        unsigned int k = static_cast<unsigned int>(sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i].size());
        if (k > j)
        {
            co_return true;
        }
        if (k > 0)
        {
            unsigned int n = 0;
            for (; n < k; n++)
            {
                if (sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i][n] != peer->urlpath[n + 1])
                {
                    break;
                }
            }
            DEBUG_LOG("check list %u %u", n, k);
            if (n == k)
            {
                DEBUG_LOG("static_pre_lists: %zu %s", i, sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i].c_str());
                // match pre urlpath
                htmlcontent = co_await get_router().co_call_regfun(peer, sysconfigpath.sitehostinfos[peer->host_index].static_pre_method);
                if (htmlcontent.size() == 0)
                {
                    co_return true;
                }
                else
                {
                    htmlcontent.append(peer->output);
                    co_await http1_send_status_content(peer, 403, htmlcontent);
                    co_return false;
                }
            }
        }
    }
    DEBUG_LOG("not authority");
    co_return true;
}

void httpserver::add_error_lists(const std::string &log_item)
{
    std::unique_lock<std::mutex> lock(log_mutex);
    error_loglist.emplace_back(log_item);
    lock.unlock();
}
// 旧的同步业务派单通路（无调用点，整块保留备查）：
//   协程里 co_await co_user_task(peer) → 把 awaitable 的 handler 存进 peer->user_code_handler_call，
//   再把整个 peer 投进 clientrunpool 的 clienttasks；协程就地挂起（帧在堆上，不占 io 线程），
//   业务线程 pop 到它以后用 router::resolve 把整条链同步跑完，跑完取 handler 用
//   asio::dispatch(*io_context, handler(1)) 唤醒。
// 两个洞：池 isclose_add/isstop 时 addclient 不入队也不调 handler，协程永久挂起（peer 与 h2 stream
// 不回收）；一整条链只占一格任务，步与步之间回不到 io 池。现在由 router::co_resolve 逐步派单
// （协程线程池唤醒），业务线程跑完那一步就回队列取下一格。
// 注意：co_user_fastcgi_task 仍然活着（http1_fastcgi / http2_fastcgi 用它挂 handler 到
// user_code_handler_call，由 fastcgi::send_exit 唤醒），别一起当死码处理。
// asio::awaitable<size_t> httpserver::co_user_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h)
// {
//     auto initiate = [self = this](asio::detail::awaitable_handler<asio::any_io_executor, size_t> &&handler,
//                                   std::shared_ptr<httppeer> in_peer) mutable
//     {
//         in_peer->user_code_handler_call.push_back(std::move(handler));
//         self->clientrunpool.addclient(in_peer);
//     };
//     return asio::async_initiate<asio::use_awaitable_t<>, void(size_t)>(initiate, h, peer);
// }
asio::awaitable<size_t> httpserver::co_client_session_task(std::shared_ptr<client_session> peer_session, asio::use_awaitable_t<> h)
{
    auto initiate = [](asio::detail::awaitable_handler<asio::any_io_executor, size_t> &&handler,
                       std::shared_ptr<client_session> in_session) mutable
    {
        {
            std::unique_lock lk(in_session->waituphttp2_mutex);
            in_session->user_code_handler_call.push_back(std::move(handler));
            in_session->http2_need_wakeup = true;
        }
        // 注册后复查：封死"消费者查空→挂起"与"生产者入队→查 waiter"之间的丢唤醒窗口。
        // 生产者先入队则这里看到非空、立刻自唤；生产者后入队则它那边能看到 need_wakeup。
        if (in_session->http2_ring_queue)
        {
            bool has_item = false;
            {
                std::unique_lock qk(in_session->http2_ring_queue->http2_queue_send_mutex);
                has_item = in_session->http2_ring_queue->head_.load(std::memory_order_acquire) !=
                           in_session->http2_ring_queue->tail_.load(std::memory_order_acquire);
            }
            if (has_item)
            {
                in_session->waituphttp2();
            }
        }
    };
    return asio::async_initiate<asio::use_awaitable_t<>, void(size_t)>(initiate, h, peer_session);
}

asio::awaitable<size_t> httpserver::co_user_fastcgi_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h)
{
    auto initiate = [self = this](asio::detail::awaitable_handler<asio::any_io_executor, size_t> &&handler,
                                  std::shared_ptr<httppeer> in_peer) mutable
    {
        serverconfig &sysconfigpath = getserversysconfig();
        in_peer->user_code_handler_call.push_back(std::move(handler));
        std::shared_ptr<http::fastcgi> fcgi = std::make_shared<http::fastcgi>();
        fcgi->peer_ptr                      = in_peer;
        fcgi->host                          = sysconfigpath.sitehostinfos[in_peer->host_index].fastcgi_host;// "127.0.0.1";
        fcgi->port                          = sysconfigpath.sitehostinfos[in_peer->host_index].fastcgi_port;// 9000
        fcgi->add_error_msg                 = std::bind(&httpserver::add_error_lists, self, std::placeholders::_1);
        fcgi->server_ioc                    = &self->io_context;
        client_context &fcgi_content        = get_client_context_obj();

        fcgi_content.add_fastcgi_task(fcgi);
    };
    return asio::async_initiate<asio::use_awaitable_t<>, void(size_t)>(initiate, h, peer);
}
asio::awaitable<void> httpserver::http1_fastcgi(std::shared_ptr<httppeer> peer)
{
    DEBUG_LOG("http1_fastcgi php in");
    peer->parse_session();
    peer->status(200);
    peer->content_type.clear();
    peer->etag.clear();

    co_await co_user_fastcgi_task(peer);

    if (peer->get_status() < 100)
    {
        peer->status(200);
    }
    if (!peer->isset_type())
    {
        peer->type("text/html; charset=utf-8");
    }
    DEBUG_LOG("http1_fastcgi php out");
    peer->compress = 0;
    if (peer->state.gzip)
    {
        if (str_casecmp(peer->content_type, "text/html; charset=utf-8") ||
            str_casecmp(peer->content_type, "application/json") ||
            str_casecmp(peer->content_type, "text/html") ||
            str_casecmp(peer->content_type, "application/json; charset=utf-8"))
        {
            if (peer->output.size() > 100)
            {
                std::string tempcompress;
                if (compress(peer->output.data(), peer->output.size(), tempcompress, Z_DEFAULT_COMPRESSION) == Z_OK)
                {
                    // peer->output   = tempcompress;
                    peer->compress = 1;
                    peer->length(tempcompress.size());
                    peer->output = peer->make_http1_header();
                    peer->output.append("\r\n");
                    co_await peer->socket_session->async_send_writer(peer->output);
                    co_await peer->socket_session->async_send_writer(tempcompress);
                    co_return;
                }
            }
        }
    }
    peer->length(peer->output.size());
    std::string htmlcontent = peer->make_http1_header();
    htmlcontent.append("\r\n");
    co_await peer->socket_session->async_send_writer(htmlcontent);
    co_await peer->socket_session->async_send_writer(peer->output);
    clear_peer_data(peer);
    co_return;
}
asio::awaitable<void> httpserver::http1loop(std::shared_ptr<httppeer> peer,
                                            std::shared_ptr<client_session> peer_session)
{
    if (peer->socket_session == nullptr)
    {
        peer->socket_session = peer_session->get_ptr();
    }

    serverconfig &sysconfigpath = getserversysconfig();

    // sitepath 必须在预查之前定好：带点 URL 的「先落盘」要用它拼出盘上那个文件
    peer->sitepath = sysconfigpath.getsitewwwpath(peer->host_index);

    // 路由预查：带点 URL 先落盘，然后两段查表；>=0 是命中的 handler 下标
    int routeidx = peer->prefetch_routing();

    // compress==10 是解析阶段的轻量 php 后缀标记；真正要不要走 fastcgi：
    // 先排除注解路由已命中（routeidx>=0 优先），再查 hostinfo.isuse_php 和 php_root_document
#ifdef ENABLE_FASTCGI
    if (peer->compress == 10 && routeidx < 0)
    {
        if (peer->host_index < sysconfigpath.sitehostinfos.size() && sysconfigpath.sitehostinfos[peer->host_index].isuse_php != 1)
        {
            // 本站没开 php 支持，请求 .php → 直接 404
            peer->compress = 0;
        }
        else if (!check_php_dispatch(peer))
        {
            // 开了 php 但找不到对应 .php 文件 → 清零 compress，走正常磁盘/动态链流程
            peer->compress = 0;
        }
        else
        {
            // check_php_dispatch 已设好 compress/linktype/sendfilename，直接走 fastcgi
            co_await http1_fastcgi(peer);
            co_return;
        }
    }
#endif

    // 注解路由命中 → 直接进动态链
    unsigned char sendtype = 0;
    if (routeidx >= 0)
    {
        // 跳过 get_fileinfo，直接进 router 动态链
    }
    else if (!peer->sitepath.empty())
    {
        sendtype = peer->get_fileinfo();
    }

    DEBUG_LOG("http1loop:%s regfun:%d sendtype:%d", peer->sendfilename.c_str(), routeidx, sendtype);
    if (sendtype == 1)
    {
        if (peer->host_index < sysconfigpath.sitehostinfos.size() && sysconfigpath.sitehostinfos[peer->host_index].is_static_pre)
        {
            if ((co_await http1_static_file_authority(peer)))
            {
                peer->output.clear();
            }
            else
            {
                co_return;
            }
        }

        if (peer->state.rangebytes)
        {
            co_await http1_send_file_range(peer, peer_session);
        }
        else
        {
            co_await http1_send_file(peer, peer_session);
        }
        co_return;
    }
    else if (sendtype == 2 && peer->host_index < sysconfigpath.sitehostinfos.size() && sysconfigpath.sitehostinfos[peer->host_index].is_show_directory)
    {
        peer->output = displaydirectory(peer->sendfilename,
                                        peer->urlpath,
                                        peer->get["sort"].as_string(),
                                        sysconfigpath.configpath);
        peer->status(200);
        peer->type("text/html; charset=utf-8");
        peer->compress = 0;
        // 与 http1_send_file / http2_compress_output 保持一致：gzip 与 br 都要支持
        if (peer->state.gzip || peer->state.br)
        {
            if (str_casecmp(peer->content_type, "text/html; charset=utf-8") ||
                str_casecmp(peer->content_type, "application/json") ||
                str_casecmp(peer->content_type, "text/html") ||
                str_casecmp(peer->content_type, "application/json; charset=utf-8"))
            {
                if (peer->output.size() > 100)
                {
                    std::string tempcompress;
                    bool compressed_ok = false;
                    if (peer->state.br)
                    {
                        brotli_encode(peer->output, tempcompress);
                        peer->compress = 2;
                        compressed_ok  = true;
                    }
                    else if (compress(peer->output.data(), peer->output.size(), tempcompress, Z_DEFAULT_COMPRESSION) == Z_OK)
                    {
                        // peer->output   = tempcompress;
                        peer->compress = 1;
                        compressed_ok  = true;
                    }

                    if (compressed_ok)
                    {
                        peer->length(tempcompress.size());
                        peer->output = peer->make_http1_header();
                        peer->output.append("\r\n");
                        co_await peer_session->async_send_writer(peer->output);
                        co_await peer_session->async_send_writer(tempcompress);
                        clear_peer_data(peer);
                        co_return;
                    }
                    // 压缩失败：回退为不压缩，避免响应头声明了 Content-Encoding 而内容未压缩
                    peer->compress = 0;
                }
            }
        }
        peer->length(peer->output.size());
        std::string htmlcontent = peer->make_http1_header();
        htmlcontent.append("\r\n");
        co_await peer_session->async_send_writer(htmlcontent);
        co_await peer_session->async_send_writer(peer->output);
        clear_peer_data(peer);
        co_return;
    }
    else
    {
        DEBUG_LOG("---  http1 dynamic --------");
        peer->linktype = 0;
        peer->parse_session();

        peer->status(200);
        peer->content_type.clear();
        peer->etag.clear();
        peer->output.clear();

        DEBUG_LOG("---  http1 co handle %s--------", peer->sendfilename.c_str());

        // — v6: 链的执行统一在 router::co_resolve —
        // sync pre/regfun → ThreadPool，coro → 就地 co_await
        co_await get_router().co_resolve(peer, routeidx);

        if (peer->ischunked)
        {
            co_return;
        }

        if (peer->get_status() < 100)
        {
            peer->status(200);
        }
        // 默认 Content-Type 在链跑完之后、由 isset_type() 把关补：co_resolve 期间 content_type
        // 保持"未设置"，业务 handler 内部可用 isset_type() 判断自己有没有设过类型；没设才落到 text/html。
        if (!peer->isset_type())
        {
            peer->type("text/html; charset=utf-8");
        }
        peer->compress = 0;
        // 与 http1_send_file / http2_compress_output 保持一致：gzip 与 br 都要支持
        if (peer->state.gzip || peer->state.br)
        {
            if (str_casecmp(peer->content_type, "text/html; charset=utf-8") ||
                str_casecmp(peer->content_type, "application/json") ||
                str_casecmp(peer->content_type, "text/html") ||
                str_casecmp(peer->content_type, "application/json; charset=utf-8"))
            {
                if (peer->output.size() > 100)
                {
                    std::string tempcompress;
                    bool compressed_ok = false;
                    if (peer->state.br)
                    {
                        brotli_encode(peer->output, tempcompress);
                        peer->compress = 2;
                        compressed_ok  = true;
                    }
                    else if (compress(peer->output.data(), peer->output.size(), tempcompress, Z_DEFAULT_COMPRESSION) == Z_OK)
                    {
                        // peer->output   = tempcompress;
                        peer->compress = 1;
                        compressed_ok  = true;
                    }

                    if (compressed_ok)
                    {
                        peer->length(tempcompress.size());
                        std::string htmlcontent = peer->make_http1_header();
                        htmlcontent.append("\r\n");
                        co_await peer_session->async_send_writer(htmlcontent);
                        co_await peer_session->async_send_writer(tempcompress);
                        DEBUG_LOG("---  http1 compress send --------");
                        clear_peer_data(peer);
                        co_return;
                    }
                    // 压缩失败：回退为不压缩，避免响应头声明了 Content-Encoding 而内容未压缩
                    peer->compress = 0;
                }
            }
        }
        peer->length(peer->output.size());
        std::string htmlcontent = peer->make_http1_header();
        htmlcontent.append("\r\n");
        co_await peer_session->async_send_writer(htmlcontent);
        co_await peer_session->async_send_writer(peer->output);
        DEBUG_LOG("---  http1 output send --------");
        clear_peer_data(peer);
    }
    co_return;
}
asio::awaitable<void> httpserver::http1_send_bad_server(std::shared_ptr<httppeer> peer,
                                                        std::shared_ptr<client_session> peer_session)
{
    std::string str       = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html; charset=utf-8\r\nConnection: "
                            "close\r\nContent-Length: ";
    std::string stfilecom = "<h3>500 Internal Server Error</h3>";
    stfilecom.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
    str.append(std::to_string(stfilecom.size()));
    str.append("\r\n\r\n");
    str.append(stfilecom);
    // peer_session->send_data(str);
    co_await peer_session->async_send_writer(str);
    co_return;
}
asio::awaitable<void> httpserver::http1_send_bad_request(unsigned int error_code, std::shared_ptr<client_session> peer_session)
{
    // error_code 是解析器内部的诊断码（形如 40095），不是 HTTP 状态码，
    // 状态行固定 400 Bad Request，内部码只在正文里提示。
    std::string stfilecom = "<h3>400 Bad Request</h3>";
    stfilecom.append("<hr /><p>Error Code: " + std::to_string(error_code) + "</p>");
    std::string str = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=utf-8\r\nConnection: "
                      "close\r\nContent-Length: ";
    str.append(std::to_string(stfilecom.size()));
    str.append("\r\n\r\n");
    str.append(stfilecom);
    // peer_session->send_data(str);
    co_await peer_session->async_send_writer(str);
    co_return;
}

asio::awaitable<void> httpserver::http1_send_method_not_allowed(std::shared_ptr<client_session> peer_session)
{
    // H3 补齐：PUT/DELETE/TRACE/CONNECT 属于有意不支持的方法，统一回 405 + Connection: close，
    // 这样声明了 body 的请求不会留下「未消费的字节」被下一轮当成新请求解析。
    std::string stfilecom = "<h3>405 Method Not Allowed</h3><hr /><p>Allow: GET, POST, QUERY, HEAD, OPTIONS</p>";
    std::string str       = "HTTP/1.1 405 Method Not Allowed\r\nAllow: GET, POST, QUERY, HEAD, OPTIONS\r\nContent-Type: "
                            "text/html; charset=utf-8\r\nConnection: close\r\nContent-Length: ";
    str.append(std::to_string(stfilecom.size()));
    str.append("\r\n\r\n");
    str.append(stfilecom);
    co_await peer_session->async_send_writer(str);
    co_return;
}

int httpserver::checkhttp2(std::shared_ptr<client_session> peer_session)
{
    if (peer_session->_cache_data[0] == 0x50 && peer_session->_cache_data[1] == 0x52 &&
        peer_session->_cache_data[2] == 0x49 && peer_session->_cache_data[3] == 0x20 &&
        peer_session->_cache_data[4] == 0x2A && peer_session->_cache_data[5] == 0x20)
    {
        peer_session->httpv = 2;
        return 2;
    }
    else if (peer_session->_cache_data[0] == 'r' && peer_session->_cache_data[1] == 'p' &&
             peer_session->_cache_data[2] == 'c' && peer_session->_cache_data[3] == 0x20)
    {
        //rpc
        peer_session->httpv = 7;
        return 7;
    }
    else if (peer_session->_cache_data[0] == 't' && peer_session->_cache_data[1] == 'c' &&
             peer_session->_cache_data[2] == 'p' && peer_session->_cache_data[3] == 0x20)
    {
        //tcp
        peer_session->httpv = 8;
        return 8;
    }
    else if (peer_session->_cache_data[0] == 0x10)
    {
        //mqtt CONNECT fixed header: 0x10 = type 1 << 4, no flags
        peer_session->httpv = 9;
        return 9;
    }
    else
    {
        peer_session->httpv = 1;
        return 1;
    }
}
void httpserver::add_nullptrlog(const std::string &logstrb)
{
    std::string log_item;
    log_item.append("cache data malloc empty for empty peer_session->_cache_data ");
    log_item.push_back(0x20);
    log_item.append(logstrb);
    log_item.push_back('\n');
    std::unique_lock<std::mutex> lock(log_mutex);
    error_loglist.emplace_back(log_item);
    lock.unlock();
}

asio::awaitable<void> httpserver::ring_client_server(std::shared_ptr<client_session> peer_session)
{
    try
    {
        while (isstop == false)
        {
            std::unique_lock lk(peer_session->http2_ring_queue->http2_queue_send_mutex);
            unsigned char head = peer_session->http2_ring_queue->head_.load(std::memory_order_acquire);
            if (head == peer_session->http2_ring_queue->tail_.load(std::memory_order_acquire))
            {
                lk.unlock();
                DEBUG_LOG(" ring wait for wake up ");
                if (peer_session->half_close)
                {
                    DEBUG_LOG(" full close ");
                    peer_session->isclose = true;
                    break;
                }
                // 环空且已关闭才退出，否则会丢掉最后一帧
                if (peer_session->isclose)
                {
                    DEBUG_LOG("ring_client_server peer_session isclose ");
                    break;
                }

                co_await co_client_session_task(peer_session);
            }
            else
            {
                lk.unlock();
            }

            if (isstop)
            {
                break;
            }
            for (;;)
            {
                DEBUG_LOG("ring_client_server for loop ");
                std::unique_lock for_lk(peer_session->http2_ring_queue->http2_queue_send_mutex);
                unsigned char for_head = peer_session->http2_ring_queue->head_.load(std::memory_order_acquire);
                if (for_head == peer_session->http2_ring_queue->tail_.load(std::memory_order_acquire))
                {
                    for_lk.unlock();
                    break;
                }
                for_lk.unlock();

                unsigned long long sent_bytes =
                    peer_session->http2_ring_queue->data[for_head].size();
                if (peer_session->isssl)
                {
                    co_await asio::async_write(*peer_session->sslsocket, asio::buffer(peer_session->http2_ring_queue->data[for_head]), asio::use_awaitable);
                }
                else
                {
                    co_await asio::async_write(*peer_session->socket, asio::buffer(peer_session->http2_ring_queue->data[for_head]), asio::use_awaitable);
                }
                // 积压字节饱和减：只有 post_write 入口累加，http2 直 push 恒 0 自然跳过
                if (peer_session->http2_ring_queue->bytes.load(std::memory_order_relaxed) >= sent_bytes)
                {
                    peer_session->http2_ring_queue->bytes.fetch_sub(sent_bytes, std::memory_order_relaxed);
                }
                peer_session->http2_ring_queue->head_.store((for_head + 1) & (peer_session->http2_ring_queue->capacity_ - 1), std::memory_order_release);
                // 事件边：环腾出一个槽位就把本连接挂起的对象重算一遍闸门。 parked 为 0
                // 时这只是一次原子读，不抢锁；有挂起时逐帧回灌，让发送侧能持续把环填到
                // 让路阈值，而不是等环彻底排空。
                if (get_http2_send_queue().parked.load() > 0)
                {
                    requeue_parked(*peer_session);
                }
            }
        }
        co_return;
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("ring_client_server std::exception %s", e.what());
        peer_session->iserror = true;
        peer_session->isclose = true;
    }
    DEBUG_LOG("ring_client_server exit ");
    co_return;
}

asio::awaitable<void> httpserver::clientpeerstop(std::shared_ptr<client_session> peer_session)
{
    std::string temp_log_stop;
    temp_log_stop = "-- begin async_stop -- " + peer_session->client_ip + " ";
    //temp_log_stop = co_await peer_session->async_stop();
    temp_log_stop.append(co_await peer_session->async_stop());
    std::unique_lock<std::mutex> lockb(log_mutex);
    error_loglist.emplace_back(temp_log_stop);
    lockb.unlock();
    co_return;
}
asio::awaitable<void> httpserver::orm_connect_clear(std::shared_ptr<orm::orm_conn_pool> peer_connect)
{
    co_await peer_connect->clear_edit_conn_2hour();
    co_await peer_connect->clear_select_conn_2hour();
    co_return;
}

unsigned int httpserver::make_h2c_header(std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session, std::string &log_item)
{
    const static unsigned char pre_fix_char[] = {0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x01, 0x82, 0x86, 0x44};
    std::memcpy(peer_session->_cache_data, pre_fix_char, sizeof(pre_fix_char));

    // _cache_data 是 malloc(CACHE_DATA_LENGTH=4096) 的连接池缓冲：逐字节写入而不做边界判断，
    // 会被 url / host / User-Agent（各自可达 16KB）冲出去，帧长度也不能截断成 8 位。
    // 这里三个字段都是 HPACK 7 位长度前缀的字符串(≤127)，先算总需求长度，
    // 单字段超长或超出缓冲一律返回 0（调用方按握手失败关连接）。
    const std::string user_agent = peer->get_header("User-Agent");

    if (peer->url.size() > 127 || peer->host.size() > 127 || user_agent.size() > 127)
    {
        return 0;
    }

    unsigned int need = 13 + static_cast<unsigned int>(peer->url.size()) + 1 +
                        static_cast<unsigned int>(peer->host.size()) + 1;
    if (user_agent.size() > 0)
    {
        need += 2 + static_cast<unsigned int>(user_agent.size());
    }
    if (need + 1 > CACHE_DATA_LENGTH)
    {
        return 0;
    }

    unsigned int i                = 13;
    peer_session->_cache_data[12] = static_cast<unsigned char>(peer->url.size());
    if (peer->url.size() > 0)
    {
        std::memcpy(peer_session->_cache_data + i, peer->url.data(), peer->url.size());
        i += static_cast<unsigned int>(peer->url.size());
    }
    peer_session->_cache_data[i] = 0x41;
    i++;
    peer_session->_cache_data[i] = static_cast<unsigned char>(peer->host.size());
    i++;
    if (peer->host.size() > 0)
    {
        std::memcpy(peer_session->_cache_data + i, peer->host.data(), peer->host.size());
        i += static_cast<unsigned int>(peer->host.size());
    }

    if (user_agent.size() > 0)
    {
        peer_session->_cache_data[i] = 0x7A;
        i++;
        peer_session->_cache_data[i] = static_cast<unsigned char>(user_agent.size());
        i++;
        std::memcpy(peer_session->_cache_data + i, user_agent.data(), user_agent.size());
        i += static_cast<unsigned int>(user_agent.size());
    }

    // 帧长度字段是 3 字节大端；只写最低字节的话 payload > 255 时长度就错了
    unsigned int payload         = i - 9;
    peer_session->_cache_data[0] = static_cast<unsigned char>((payload >> 16) & 0xFF);
    peer_session->_cache_data[1] = static_cast<unsigned char>((payload >> 8) & 0xFF);
    peer_session->_cache_data[2] = static_cast<unsigned char>(payload & 0xFF);

    log_item = user_agent;
    return i;
}

asio::awaitable<unsigned int> httpserver::client_http1_loop(bool isssl, unsigned int readnum, std::shared_ptr<client_session> peer_session)
{
    try
    {
        std::shared_ptr<httppeer> peer = std::make_shared<httppeer>();
        peer_session->time_limit.store(timeid());
        // 提前绑定 socket_session：CORS 预检分支（send_cors_domain）在 http1loop 之前
        // 就会用 peer->socket_session 写回响应，而该成员原先只在 http1loop 内部赋值，
        // 连接上第一条请求就是 OPTIONS 时会空指针解引用崩溃。
        // http1loop 内的赋值条件是「== nullptr 才赋值」，提前绑定不会被覆盖；
        // httppeer::clear() 也不重置该成员，所以绑定一次即可。
        peer->socket_session                = peer_session->get_ptr();
        peer->client_ip                     = peer_session->getremoteip();
        std::unique_ptr<httpparse> http1pre = std::make_unique<httpparse>();

        std::string log_item;
        unsigned int single_link_count = 1;
        http1pre->setpeer(peer);
        peer_session->httpv = 1;
        peer->httpv         = 1;

        for (;;)
        {
            log_item.clear();
            if (readnum == 0)
            {
                bool is_error = co_await peer_session->read_some(readnum, log_item);
                if (is_error)
                {
#ifndef BENCHMARK
                    log_item.push_back(0x20);
                    log_item.append(peer_session->client_ip);
                    log_item.push_back(0x20);
                    log_item.append(std::to_string(peer_session->client_port));

                    log_item.push_back('\n');
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(log_item);
                    lock.unlock();
#endif
                    peer_session->stop();
                    co_return 3;
                }
            }
            http1pre->process(peer_session->_cache_data, readnum);
            if (http1pre->method_not_allowed)
            {
                // H3 补齐：有意不支持的方法（PUT/DELETE/TRACE/CONNECT）回 405 并关闭连接，
                // 不再复用连接，避免未消费的 body 被当成下一个请求解析。
                co_await http1_send_method_not_allowed(peer_session);
                co_return 3;
            }
            if (http1pre->error > 0)
            {
                co_await http1_send_bad_request(http1pre->error, peer_session);
                co_return 3;
            }
            if (readnum == 0)
            {
                // 0 字节且无错误等价于对端关闭，不能再拿未解析的 peer 派发请求
                co_return 3;
            }
            if (http1pre->getfinish())
            {
                peer->isssl = isssl ? true : false;
                if (peer->server_port == 0)
                {
                    peer->server_ip   = peer_session->getlocalip();
                    peer->client_ip   = peer_session->getremoteip();
                    peer->client_port = peer_session->getremoteport();
                    peer->server_port = peer_session->getlocalport();
                }

                if (peer->state.h2c)
                {
                    peer_session->httpv = 2;
                    peer->httpv         = 2;
                    peer->isfinish      = true;
                    peer->issend        = false;
                    bool isok           = co_await peer_session->co_send_switch101();
                    peer->stream_id     = 1;
                    peer->isssl         = isssl ? true : false;

                    if (isok)
                    {
                        readnum = make_h2c_header(peer, peer_session, log_item);
                        co_return readnum;
                    }
                    else
                    {
                        co_return 3;
                    }
                }

                if (hook_host_http1(peer))
                {
                    co_return 3;
                }

                if (peer->state.websocket)
                {
                    if (peer->pathinfos.size() == 0)
                    {
                        log_item = ws::make_error(404);
                        co_await peer_session->async_send_writer(log_item);
                        co_return 3;
                    }

                    // 握手校验：state.websocket 由 Upgrade: websocket 置位，
                    // websocket 对象在 Sec-WebSocket-Key/Version 头解析时创建。
                    if (http1pre->websocket == nullptr ||
                        !ws::validate(*http1pre->websocket, peer->state.websocket, peer->state.upgradeconnection, peer->method))
                    {
                        log_item = ws::make_error(400);
                        co_await peer_session->async_send_writer(log_item);
                        co_return 3;
                    }

                    auto ws = std::shared_ptr<websocket_t>(std::move(http1pre->websocket));

                    ws->gzip    = peer->state.gzip;
                    ws->zstd    = peer->state.zstd;
                    ws->deflate = peer->state.deflate;
                    ws->br      = peer->state.br;

                    co_await client_websocket_loop(peer, ws, peer_session);
                    co_return 3;
                }

#ifndef BENCHMARK
                {
                    log_item.clear();
                    log_item.append(peer->client_ip);
                    log_item.push_back(0x20);
                    log_item.append(get_date("%Y-%m-%d %X"));
                    log_item.push_back(0x20);
                    log_item.append(std::to_string(single_link_count));
                    log_item.push_back(0x20);
                    log_item.append(std::to_string(peer->server_port));
                    log_item.push_back(0x20);
                    log_item.append("H1");
                    log_item.push_back(0x20);
                    log_item.append(peer->host);
                    log_item.push_back(0x20);
                    log_item.append(std::to_string(peer->method));
                    log_item.push_back(0x20);
                    log_item.append(peer->url);
                    log_item.push_back('\n');
                    std::unique_lock<std::mutex> lock(log_mutex);
                    access_loglist.emplace_back(log_item);
                    lock.unlock();
                }
#endif

                // 普通请求的 CORS 响应头已在解析阶段设好（解析到 Origin 头时按白名单判定）；
                // OPTIONS 预检不进业务：由 send_cors_domain() 重新按白名单输出预检头
                if (peer->iscors)
                {
                    co_await send_cors_domain(peer);
                }
                else
                {
                    co_await http1loop(peer, peer_session);
                }
                DEBUG_LOG("http1loop end");
                if (peer->state.keepalive == false)
                {
                    co_return 3;
                }
                http1pre->clear();
                peer->clear();
                peer_session->time_limit.store(timeid());
                single_link_count++;
            }
            readnum = 0;
        }
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("client exit exception");
        peer_session->stop();
    }
    catch (...)
    {
        peer_session->stop();
    }
    co_return 3;
}

asio::awaitable<unsigned int> httpserver::client_http2_loop(unsigned int offsetnum, unsigned int readnum, std::shared_ptr<client_session> peer_session)
{
    try
    {
        DEBUG_LOG("client_http2_loop");
        std::shared_ptr<http2parse> http2pre = std::make_shared<http2parse>();
        http2pre->setsession(peer_session);
        std::shared_ptr<httppeer> peer = std::make_shared<httppeer>();
        peer_session->time_limit.store(timeid());
        peer->client_ip = peer_session->getremoteip();
        // 这个 peer 只用于连接级错误时的提示响应。必须绑定 socket_session：
        // 发送侧第一步就查 socket_session.use_count() == 0，未绑定会直接把作业回收，
        // 403 正文永远发不出去（只剩 GOAWAY）。
        peer->socket_session = peer_session;

        peer_session->httpv = 2;
        peer->httpv         = 2;

        unsigned int error_state       = 0;
        unsigned int single_link_count = 0;
        // 本条连接上已上报过的「丢弃 WINDOW_UPDATE」计数，用于只在数字变化时写日志行。
        unsigned int logged_winupdate_dropped = 0;
        std::string log_item;
        for (;;)
        {
            log_item.clear();
            if (readnum == 0)
            {
                bool is_error = co_await peer_session->read_some(readnum, log_item);
                if (is_error)
                {
#ifndef BENCHMARK
                    log_item.push_back(0x20);
                    log_item.append(peer_session->client_ip);
                    log_item.push_back(0x20);
                    log_item.append(std::to_string(peer_session->client_port));

                    log_item.push_back('\n');
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(log_item);
                    lock.unlock();
#endif
                    break;
                }
                offsetnum = 0;
            }

            // 只前移真正消费掉的长度：流级错误时 process() 停在坏帧末尾，
            // 其后已经读进来的其它流帧要留给下一轮，不能跟着这一帧一起作废
            offsetnum += http2pre->process(&peer_session->_cache_data[offsetnum], readnum - offsetnum);
            // 落在陌生/已结束流上的 WINDOW_UPDATE 只丢不报，没有这条输出就完全不可观测
            // （要点是"丢掉的帧不能变成永久条目"）。攒批上报：坏帧可以按字节
            // 洪水，日志不能跟着一起洪水。
            if (http2pre->winupdate_dropped > logged_winupdate_dropped)
            {
                logged_winupdate_dropped     = http2pre->winupdate_dropped;
                unsigned long long kept_size = 0;
                {
                    std::lock_guard<std::mutex> winlk(peer_session->stream_send_window_mutex);
                    kept_size = peer_session->stream_send_window.size();
                }
                log_item.clear();
                log_item.append("http2 window_update dropped ");
                log_item.append(std::to_string(http2pre->winupdate_dropped));
                log_item.append(" stream_send_window ");
                log_item.append(std::to_string(kept_size));
                log_item.append(" max_sid ");
                log_item.append(std::to_string(http2pre->max_client_stream_id));
                log_item.push_back('\n');
                {
                    std::lock_guard<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(log_item);
                }
            }
            if (peer_session->isgoway)
            {
                DEBUG_LOG("http2pre goway %d;", http2pre->error);
                break;
            }
            if (http2pre->error > 0)
            {
                DEBUG_LOG("http2 error:%d;", http2pre->error);
                // 统一错误分流：连接级→GOAWAY 断连（返回 true），流级→RST 重置该流（返回 false，连接复用）。
                if (co_await http2_handle_parse_error(*http2pre, peer_session))
                {
                    break;
                }
            }
            peer_session->time_limit.store(timeid());
            while (!http2pre->stream_list.empty())
            {
                unsigned int block_steamid = http2pre->stream_list.front();
                http2pre->stream_list.pop();

                auto node = http2pre->http_data.extract(block_steamid);
                if (node.empty())
                    continue;

                auto stream_ptr = std::move(node.mapped());

                if (stream_ptr->socket_session == nullptr)
                    stream_ptr->socket_session = peer_session;

                if (hook_host_http2(stream_ptr))
                {
                    error_state = 1;
                    break;
                }

                if (stream_ptr->server_port == 0)
                {
                    stream_ptr->server_ip   = peer_session->getlocalip();
                    stream_ptr->client_ip   = peer_session->getremoteip();
                    stream_ptr->client_port = peer_session->getremoteport();
                    stream_ptr->server_port = peer_session->getlocalport();
                }

                // 同 h1：OPTIONS 预检不进业务，由 send_cors_domain() 输出预检头；
                // 只捕 sp 即可，响应头挂在 peer 自己的容器里，协程持有 sp 就不会悬垂
                if (stream_ptr->iscors)
                {
                    asio::co_spawn(peer_session->strand_, [this, sp = stream_ptr]() -> asio::awaitable<void>
                                   { co_await send_cors_domain(sp); },
                                   asio::detached);
                }
                else if (stream_ptr->reject_status > 0)
                {
                    // 站点上传限额：头块解完就判掉了，正文一帧没收，直接回状态页，不进业务控制器。
                    // 与下面 405 那条同一个形状；这条流后续到达的 DATA 由解析器丢弃并还回连接窗口。
                    std::string limit_body = "<h3>" + std::to_string(stream_ptr->reject_status) + " upload size limit</h3>";
                    asio::co_spawn(peer_session->strand_,
                                   [this, sp = stream_ptr, body = std::move(limit_body)]() -> asio::awaitable<void>
                                   { co_await http2_send_status_content(sp, sp->reject_status, body); },
                                   asio::detached);
                }
                else if (stream_ptr->method == HEAD_METHOD::PUT || stream_ptr->method == HEAD_METHOD::DELETE ||
                         stream_ptr->method == HEAD_METHOD::TRACE || stream_ptr->method == HEAD_METHOD::CONNECT)
                {
                    // 与 h1 的 method_not_allowed 同一口径：这四个方法框架有意不支持，
                    // 直接回 405，不进业务控制器（h1 走 http1_send_method_not_allowed）
                    stream_ptr->set_header("Allow", "GET, POST, QUERY, HEAD, OPTIONS");
                    asio::co_spawn(peer_session->strand_, [this, sp = stream_ptr]() -> asio::awaitable<void>
                                   { co_await http2_send_status_content(sp, 405, "<h3>405 Method Not Allowed</h3>"); },
                                   asio::detached);
                }
                else
                {
                    asio::co_spawn(peer_session->strand_, [this, sp = stream_ptr]() -> asio::awaitable<void>
                                   { co_await http2loop(sp); },
                                   asio::detached);
                }

                single_link_count++;
                http2pre->steam_count += 1;

#ifndef BENCHMARK
                log_item.clear();
                log_item.append(stream_ptr->client_ip);
                log_item.push_back(0x20);
                log_item.append(get_date("%Y-%m-%d %X"));
                log_item.push_back(0x20);
                log_item.append(std::to_string(single_link_count));
                log_item.push_back(0x20);
                log_item.append(std::to_string(stream_ptr->server_port));
                log_item.push_back(0x20);
                log_item.append("H2");
                log_item.push_back(0x20);
                log_item.append(stream_ptr->host);
                log_item.push_back(0x20);
                log_item.append(std::to_string(stream_ptr->method));
                log_item.push_back(0x20);
                log_item.append(stream_ptr->url);
                log_item.push_back('\n');
                {
                    std::unique_lock<std::mutex> lock(log_mutex);
                    access_loglist.emplace_back(log_item);
                }
#endif
                http2pre->http_data_weak.insert_or_assign(block_steamid, stream_ptr);
            }

            if (error_state > 0)
            {
                co_await peer_session->async_send_goway();
                break;
            }

#ifndef BENCHMARK
            if (http2pre->steam_count > 512)
            {
                DEBUG_LOG("client http2 > 2024 stream close ");
                co_await peer_session->async_send_goway();
                break;
            }
#endif
            if (http2pre->need_wakeup_send_threads)
            {
                DEBUG_LOG("http2 setting need_wakeup_send_threads");
                // 事件边：对端抬了窗口（或改了 SETTINGS），本连接挂起的发送对象要重算闸门。
                requeue_parked(*peer_session);
                if (peer_session->http2_need_wakeup)
                {
                    peer_session->waituphttp2();
                }

                http2pre->need_wakeup_send_threads = false;
            }
            // 只有整块读完才允许下一轮 read_some 覆盖 _cache_data（它从下标 0 写，
            // 未解析的尾部一旦被覆盖就永久丢帧）；留着尾巴就接着从 offsetnum 解。
            if (offsetnum >= readnum)
            {
                readnum   = 0;
                offsetnum = 0;
            }
        }
        // 发送环三个计数在本连接内收尾时报一次：overflow 是"环满、这一帧留到下一轮重发"
        // （push 直接被拒），backpressure 是"环积压到阈值、本轮主动让路"（还没 push 就退），
        // dropped 是"这一帧真的没了"。三者都不报出来，"零丢弃"就只是一句无法核实的说法。
        if (peer_session->http2_ring_overflow_count.load() > 0 ||
            peer_session->http2_ring_queue_drop_count.load() > 0 ||
            peer_session->http2_ring_backpressure_count.load() > 0)
        {
            log_item.clear();
            log_item.append("http2 ring stats overflow ");
            log_item.append(std::to_string(peer_session->http2_ring_overflow_count.load()));
            log_item.append(" dropped ");
            log_item.append(std::to_string(peer_session->http2_ring_queue_drop_count.load()));
            log_item.append(" backpressure ");
            log_item.append(std::to_string(peer_session->http2_ring_backpressure_count.load()));
            // 全局挂起表与在途对象数：连接收尾时这两个都该归零，不归零就是有对象
            // 停在 parked_list 里拽着 peer 和文件句柄（泄漏），这条是运行期唯一的观测手段。
            log_item.append(" parked ");
            log_item.append(std::to_string(get_http2_send_queue().parked.load()));
            log_item.append(" outstanding ");
            log_item.append(std::to_string(get_http2_send_queue().outstanding.load()));
            log_item.append(" ip ");
            log_item.append(peer_session->client_ip);
            log_item.push_back('\n');
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(log_item);
            lock.unlock();
        }
        peer_session->cancel();
        peer_session->waituphttp2();
        http2pre->clsoesend();
        co_await peer_session->async_stop();
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("client exit exception");
        peer_session->stop();
    }
    catch (...)
    {
        peer_session->stop();
    }
    co_return 0;
}

// 首包阶段的错误日志：记录客户端 ip/port 与已读到的原始字节。
// 原先在 clientpeerfun 里重复了两遍，收敛到这里。
void httpserver::log_client_first_error(std::shared_ptr<client_session> peer_session, unsigned int readnum)
{
#ifndef BENCHMARK
    std::string log_item;
    log_item.push_back(0x20);
    log_item.append(peer_session->client_ip);
    log_item.push_back(0x20);
    log_item.append(std::to_string(peer_session->client_port));
    log_item.push_back(0x20);

    if (readnum < 128)
    {
        for (size_t i = 0; i < readnum; i++)
        {
            log_item.push_back(peer_session->_cache_data[i]);
        }
    }

    log_item.push_back('\n');
    std::unique_lock<std::mutex> lock(log_mutex);
    error_loglist.emplace_back(log_item);
    lock.unlock();
#else
    (void)peer_session;
    (void)readnum;
#endif
}

// HTTP/2 统一入口。
// 收敛前两条路径各写一遍「校验前言 → 建发送环 → 回 SETTINGS → 进解析循环」，
// 且对首包字节数的处理互不一致（直连分支无上限、h2c 分支额外限制 readnum > 256），
// 而 read_first/read_some 都是单次 async_read_some、不保证读满：
//   - readnum < 24 直接断连 → preface 被 TCP 拆包就误杀合法客户端
//   - h2c 的 readnum > 256  → 客户端把 preface+SETTINGS+HEADERS 一次发出就断连，
//                             且 256 小于 make_h2c_header 的合法返回上界 398
// 这里统一改为「补读前言 + 按缓冲区实际余量限流」，不再按首包大小判生死。
asio::awaitable<unsigned int> httpserver::client_h2_enter(std::shared_ptr<client_session> peer_session,
                                                          unsigned int &readnum,
                                                          std::string_view h2c_first_request)
{
    // 缓冲区尾部要留给 h2c 首帧，读入上限相应收缩（放不下 24 字节前言则直接失败）
    if (h2c_first_request.size() >= CACHE_DATA_LENGTH)
    {
        co_return 3;
    }
    unsigned int limit = CACHE_DATA_LENGTH - static_cast<unsigned int>(h2c_first_request.size());
    if (limit < 24)
    {
        co_return 3;
    }

    // 连接前言固定 24 字节（RFC 9113 §3.4），TCP 拆包时补读而不是直接断连
    if (co_await peer_session->read_at_least(24, readnum, limit))
    {
        log_client_first_error(peer_session, readnum);
        co_return 3;
    }

    for (size_t i = 0; i < 24; i++)
    {
        if (peer_session->_cache_data[i] != magicstr[i])
        {
            co_return 3;
        }
    }

    // h2c：用于升级的 HTTP/1 请求即 stream 1，把转换出的首帧追加在前言数据之后，
    // 让解析器按「客户端前言 → 服务端构造的首帧」顺序消费
    if (h2c_first_request.empty() == false)
    {
        if (static_cast<unsigned int>(h2c_first_request.size()) > CACHE_DATA_LENGTH - readnum)
        {
            co_return 3;
        }
        std::memcpy(peer_session->_cache_data + readnum, h2c_first_request.data(), h2c_first_request.size());
        readnum += static_cast<unsigned int>(h2c_first_request.size());
    }

    auto &cc                       = get_http2_ring_queue_obj();
    peer_session->http2_ring_queue = cc.get_cache_ptr();
    if (peer_session->http2_ring_queue == nullptr)
    {
        co_return 3;
    }
    asio::co_spawn(peer_session->strand_, ring_client_server(peer_session), asio::detached);

    co_await peer_session->co_send_setting();

    co_return co_await client_http2_loop(24, readnum, peer_session);
}

// 关闭前等发送环排空，再 stop()。
// 否则 Close 帧刚入环就关 socket，最后一帧会丢失。最多等约 2 秒，出错立即放弃。
static asio::awaitable<void> ws_drain_ring_then_stop(std::shared_ptr<client_session> peer_session)
{
    if (peer_session->http2_ring_queue)
    {
        asio::steady_timer timer(peer_session->strand_);
        for (size_t i = 0; i < 100 && !peer_session->iserror; ++i)
        {
            if (peer_session->http2_ring_queue->head_.load(std::memory_order_acquire) ==
                peer_session->http2_ring_queue->tail_.load(std::memory_order_acquire))
            {
                break;
            }
            timer.expires_after(std::chrono::milliseconds(20));
            co_await timer.async_wait(asio::use_awaitable);
        }
    }
    peer_session->stop();
    co_return;
}

// 退出前回收业务从未取走的落盘消息：只清 content_list 中残留的 isfile 项，
// 业务已 pop 走的（含读后的删除责任）不碰。与线程池 pop 靠 content_list_mutex 互斥。
static void ws_cleanup_spilled_files(const std::shared_ptr<websockets_api> &websockets)
{
    if (!websockets)
        return;
    std::unique_lock<std::mutex> lock(websockets->content_list_mutex);
    for (auto iter = websockets->content_list.begin(); iter != websockets->content_list.end();)
    {
        if (iter->isfile)
        {
            std::remove(iter->value.c_str());
            iter = websockets->content_list.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

// 入站派发水位：等本连接退到 LOW 水位，最多 CONST_WEBSOCKET_QUEUE_STALL_MAX_MS（每 20ms 一轮）。
// 返回 true = 已退到水位下，调用方可以重新入队/派发。
// 暂停 async_read 就是背压本身：每轮只喂 4096B 给解析器，停在这里时未读字节留在内核缓冲，
// 压力退回对端的 TCP 窗口，不丢消息也不断连。代价边界：本连接的控制帧（close/ping）与这条
// 读协程同一条循环，等待期间一并延迟，上界就是那个 2 秒，远小于既有 8s 心跳周期。
// 出错或业务置 isclose 立刻放弃等待。
static asio::awaitable<bool> ws_wait_ingress_low_water(const std::shared_ptr<websockets_api> &websockets,
                                                       const std::shared_ptr<client_session> &peer_session,
                                                       bool is_coroutine_mode)
{
    asio::steady_timer timer(peer_session->strand_);
    unsigned long long waited_ms = 0;
    while (waited_ms < CONST_WEBSOCKET_QUEUE_STALL_MAX_MS &&
           !peer_session->iserror && !websockets->isclose)
    {
        bool still_over = false;
        if (is_coroutine_mode)
        {
            still_over = websockets->inflight_dispatch.load(std::memory_order_acquire) >=
                             CONST_WEBSOCKET_QUEUE_LOW_ITEMS ||
                         websockets->inflight_bytes.load(std::memory_order_acquire) >=
                             CONST_WEBSOCKET_QUEUE_LOW_BYTES;
        }
        else
        {
            still_over = websockets->queue_items() >= CONST_WEBSOCKET_QUEUE_LOW_ITEMS ||
                         websockets->queue_bytes() >= CONST_WEBSOCKET_QUEUE_LOW_BYTES;
        }
        if (!still_over)
        {
            co_return true;
        }
        timer.expires_after(std::chrono::milliseconds(20));
        co_await timer.async_wait(asio::use_awaitable);
        waited_ms += 20;
    }
    co_return false;
}

// 协程模式没有在途上限，尺子是在途 async_onmessage 的条数与载荷字节。
// reserve 在派发前（拒了就不 spawn）；release 挂在协程帧的局部对象上，
// 业务提前 return、异常展开、协程被取消三条出口都会走到。
struct ws_inflight_release
{
    std::shared_ptr<websockets_api> ws;
    unsigned long long bytes;
    ~ws_inflight_release()
    {
        ws->inflight_dispatch.fetch_sub(1, std::memory_order_acq_rel);
        ws->inflight_bytes.fetch_sub(bytes, std::memory_order_acq_rel);
    }
};

// 条数是主尺子，判定和占位必须在同一次 CAS 里完成：先 load 再 fetch_add 是 check-then-act，
// 两个占位者可以各自看着界内、然后一起加上去。字节尺子仍然只能"看一眼再加"——两个原子量没法
// 比在同一次 CAS 里，残差上界就是一条在途消息（落盘消息的 value 是临时文件路径，量的是路径长度）。
static bool ws_inflight_reserve(websockets_api &websockets, unsigned long long bytes)
{
    unsigned long long items = websockets.inflight_dispatch.load(std::memory_order_acquire);
    for (;;)
    {
        if (items >= CONST_WEBSOCKET_QUEUE_HIGH_ITEMS ||
            websockets.inflight_bytes.load(std::memory_order_acquire) >= CONST_WEBSOCKET_QUEUE_HIGH_BYTES)
        {
            return false;
        }
        if (websockets.inflight_dispatch.compare_exchange_weak(items, items + 1, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            break;
        }
    }
    websockets.inflight_bytes.fetch_add(bytes, std::memory_order_acq_rel);
    return true;
}

static asio::awaitable<void> ws_dispatch_onmessage_co(std::shared_ptr<websockets_api> websockets,
                                                      websockets_data_list_t msg)
{
    // bytes 必须在 msg 被 move 进业务之前取（落盘消息的 value 是临时文件路径，这里量的是路径长度，
    // 与 queue_bytes() 同口径——大消息的体量由条数尺子兜住）
    ws_inflight_release release_guard{websockets, msg.value.size()};
    co_await websockets->async_onmessage(std::move(msg));
    co_return;
}

asio::awaitable<unsigned int> httpserver::client_websocket_loop(std::shared_ptr<httppeer> peer, std::shared_ptr<websocket_t> ws, std::shared_ptr<client_session> peer_session)
{
    std::shared_ptr<websockets_api> websockets;
    try
    {
        std::string log_item;
        WEBSOCKET_REG &wsreg = get_websocket_reg();

        // 下面两格借的是 http2 的连接级发送窗口累计量：websocket 是从 HTTP/1.1 升级进来的
        // 一条独立通路，这条连接上不会有 http2 的窗口记账在跑，所以借得干净。
        // 交进工厂之后（再往下几十行）会清回 0，别在中间拿它们当窗口的实时值。
        peer_session->window_update_num   = 0;
        peer_session->has_send_update_num = 0;

        if (peer->pathinfos.size() > 1)
        {
            for (size_t i = 0; i < peer->pathinfos[1].size(); i++)
            {
                if (peer->pathinfos[1][i] >= '0' && peer->pathinfos[1][i] <= '9')
                {
                    peer_session->window_update_num = peer_session->window_update_num * 10 + (peer->pathinfos[1][i] - '0');
                }
                if (peer_session->window_update_num.load() > 0xFFFFFFFE)
                {
                    peer_session->stop();
                    co_return 0;
                }
            }
        }
        if (peer->pathinfos.size() > 2)
        {
            for (size_t i = 0; i < peer->pathinfos[2].size(); i++)
            {
                if (peer->pathinfos[2][i] >= '0' && peer->pathinfos[2][i] <= '9')
                {
                    peer_session->has_send_update_num = peer_session->has_send_update_num * 10 + (peer->pathinfos[2][i] - '0');
                }
                if (peer_session->has_send_update_num.load() > 0xFFFFFFFE)
                {
                    peer_session->stop();
                    co_return 0;
                }
            }
        }

        if (peer->pathinfos.empty())
        {
            peer_session->stop();
            co_return 0;
        }

        auto wsiter = wsreg.find(peer->pathinfos[0]);
        if (wsiter == wsreg.end())
        {
            // 路由未注册：101 尚未发出，仍是 HTTP 阶段，回 404 再断
            log_item = ws::make_error(404);
            co_await peer_session->async_send_writer(log_item);
            peer_session->stop();
            co_return 0;
        }

        websockets               = wsiter->second(peer_session->window_update_num, peer_session->has_send_update_num);
        websockets->session_sock = peer_session;
        websockets->url          = peer->url;

        // 分配发送环，ring_client_server 作为唯一写者，避免并发写同一 socket。
        bool need_start_ring = false;
        if (!peer_session->http2_ring_queue)
        {
            auto &ring_pool                = get_http2_ring_queue_obj();
            peer_session->http2_ring_queue = ring_pool.get_cache_ptr();
            need_start_ring                = true;
        }

        // 101 响应作为环中首帧入环，确保先握手再发数据。
        // 对端 offer 含 permessage-deflate 时协商（双 no-context，每消息独立流）；
        // 未协商时 RSV≠0 的帧仍由 ws_parser 按 1002 拒绝。
        bool ws_deflate_ext = ws->permessagedeflate && !ws->perframedeflate && !ws->deflateframe;
        log_item            = ws::make_101(ws->key, ws_deflate_ext);
        if (!peer_session->post_write(log_item))
        {
            // 环满或连接已关闭，101 发不出去，放弃握手
            DEBUG_LOG("websocket 101 post_write failed ");
            peer_session->stop();
            co_return 0;
        }

        // 101 入环后再启动消费者，保证 101 是首帧
        if (need_start_ring)
        {
            asio::co_spawn(peer_session->strand_, ring_client_server(peer_session), asio::detached);
        }

        // 有定时器的连接加入定时队列，放在 101 入环之后保证 FIFO
        if (websockets->durtime > 0)
        {
            std::lock_guard<std::mutex> lk(websocket_task_mutex);
            websockettasks.emplace_back(websockets);
            websocketcondition.notify_one();
        }

        if (websockets->is_coroutine())
        {
            co_await websockets->async_onopen();
        }
        else
        {
            websockets->onopen();
        }

        // URL 参数已经交进工厂，借来的两格清回去（借用说明见本函数开头）。
        peer_session->window_update_num   = 0;
        peer_session->has_send_update_num = 0;
        websockets->host                  = peer->host;
        if (peer->get.is_object())
        {
            for (auto &[key, value] : peer->get.as_object())
            {
                websockets_api_data_t temp;
                temp.name  = key;
                temp.value = value.to_string();
                websockets->header.push_back(temp);
            }
        }

        for (auto iter = peer->cookie.begin(); iter != peer->cookie.end(); iter++)
        {
            websockets_api_data_t temp;
            temp.name  = iter->first;
            temp.value = iter->second;
            websockets->header.push_back(temp);
        }

        unsigned int readnum               = 0;
        unsigned int seq_id                = 0;
        unsigned long long total_recv_data = 0;
        // 越水位且有界等待后仍不退的丢弃数（本连接累计，收尾时一行日志）
        unsigned long long ws_ingress_dropped = 0;
        ws::ws_parser ws_in_parser;
        // 入站限额来自编译期常量（common/cost_define.h）
        static_assert(CONST_WEBSOCKET_SPILL_THRESHOLD < CONST_WEBSOCKET_MAX_FRAME_SIZE &&
                          CONST_WEBSOCKET_MAX_FRAME_SIZE <= CONST_WEBSOCKET_MAX_MESSAGE_SIZE,
                      "ws limits invariant: spill < max_frame <= max_message");
        static_assert(ws::kDefaultMaxFramePayload == CONST_WEBSOCKET_MAX_FRAME_SIZE &&
                          ws::kDefaultMaxMessagePayload == CONST_WEBSOCKET_MAX_MESSAGE_SIZE &&
                          ws::kDefaultSpillThreshold == CONST_WEBSOCKET_SPILL_THRESHOLD,
                      "cost_define.h and ws_wire.h default limits drifted");
        ws_in_parser.set_limits(CONST_WEBSOCKET_MAX_FRAME_SIZE,
                                CONST_WEBSOCKET_MAX_MESSAGE_SIZE,
                                CONST_WEBSOCKET_SPILL_THRESHOLD);
        peer_session->ws_deflate = ws_deflate_ext;
        ws_in_parser.set_deflate_ext(ws_deflate_ext);

        for (;;)
        {
            log_item.clear();
            readnum       = 0;
            bool is_error = co_await peer_session->read_some(readnum, log_item);
            if (is_error)
            {
#ifndef BENCHMARK
                log_item.push_back(0x20);
                log_item.append(peer_session->client_ip);
                log_item.push_back(0x20);
                log_item.append(std::to_string(peer_session->client_port));

                log_item.push_back('\n');
                std::unique_lock<std::mutex> lock(log_mutex);
                error_loglist.emplace_back(log_item);
                lock.unlock();
#endif
                //save error log
                peer_session->stop();
                if (websockets->is_coroutine())
                {
                    co_await websockets->async_onclose();
                }
                else
                {
                    websockets->onclose();
                }
                ws_cleanup_spilled_files(websockets);
                co_return 3;
            }

            // 入站解析
            if (!ws_in_parser.feed(
                    reinterpret_cast<unsigned char *>(peer_session->_cache_data),
                    readnum))
            {
                // 协议错误：回对应 Close 码（1002/1007/1009/1011）后等环排空再关
                ws::post_send_close(*peer_session, ws_in_parser.error_code());
                if (websockets->is_coroutine())
                {
                    co_await websockets->async_onclose();
                }
                else
                {
                    websockets->onclose();
                }
                ws_cleanup_spilled_files(websockets);
                co_await ws_drain_ring_then_stop(peer_session);
                co_return 3;
            }

            total_recv_data += readnum;
            if (total_recv_data > CONST_WEBSOCKET_POST_DATA_SIZE)
            {
                // data too large (>4G)
                DEBUG_LOG("websockets big data");
#ifndef BENCHMARK
                log_item.clear();
                log_item.append(peer_session->client_ip);
                log_item.push_back(0x20);
                log_item.append(std::to_string(peer_session->client_port));
                log_item.append(" websocket data big 4G ");
                log_item.push_back('\n');
                std::unique_lock<std::mutex> lock(log_mutex);
                error_loglist.emplace_back(log_item);
                lock.unlock();
#endif
                // 数据量超限：回 Close(1009) 后等环排空再关
                ws::post_send_close(*peer_session, 1009);
                if (websockets->is_coroutine())
                {
                    co_await websockets->async_onclose();
                }
                else
                {
                    websockets->onclose();
                }
                ws_cleanup_spilled_files(websockets);
                co_await ws_drain_ring_then_stop(peer_session);
                co_return 3;
            }

            // 处理就绪的控制帧（close/ping/pong）
            while (ws_in_parser.has_control())
            {
                auto ctrl = ws_in_parser.pop_control();
                if (ctrl.op == ws::opcode::close)
                {
                    // 回 Close 握手：0 字节→1000；1 字节→1002；≥2 字节取状态码
                    unsigned short echo_code = 1000;
                    if (ctrl.payload.size() == 1)
                    {
                        echo_code = 1002;
                    }
                    else if (ctrl.payload.size() >= 2)
                    {
                        echo_code = static_cast<unsigned short>(
                            (static_cast<unsigned char>(ctrl.payload[0]) << 8) |
                            static_cast<unsigned char>(ctrl.payload[1]));
                        if (echo_code < 1000 || echo_code > 4999 ||
                            (echo_code >= 1004 && echo_code <= 1006) || echo_code == 1015)
                        {
                            echo_code = 1000;
                        }
                        // RFC 6455 §7.1.7：close 原因（状态码之后的字节）必须是合法 UTF-8
                        if (ctrl.payload.size() > 2 &&
                            !ws::utf8_valid(std::string_view(ctrl.payload).substr(2)))
                        {
                            echo_code = 1002;
                        }
                    }
                    ws::post_send_close(*peer_session, echo_code);
                    websockets->isclose = true;
                    if (websockets->is_coroutine())
                    {
                        co_await websockets->async_onclose();
                    }
                    else
                    {
                        websockets->onclose();
                    }
                    DEBUG_LOG("websockets close");
                    break;
                }
                else if (ctrl.op == ws::opcode::ping)
                {
                    websockets->onpong();
                    // Pong 走环，与业务 send 统一出口
                    ws::post_send_pong(*peer_session, ctrl.payload);
                }
                // opcode::pong 忽略
            }

            // 处理就绪的数据消息
            while (ws_in_parser.has_message())
            {
                websockets_data_list_t ws_temp_data = ws_in_parser.pop_message();
                ws_temp_data.seqid                  = seq_id++;
                bool is_co_mode                     = websockets->is_coroutine();
                bool delivered                      = false;

                if (is_co_mode)
                {
                    delivered = ws_inflight_reserve(*websockets, ws_temp_data.value.size());
                }
                else
                {
                    // push 在水位上返回 false 时不移动 msg，同一条可以拿回来重试
                    delivered = websockets->push(std::move(ws_temp_data));
                }

                if (!delivered)
                {
                    // 越水位：暂停 async_read 做背压，等它退到 LOW 水位再重试
                    if (co_await ws_wait_ingress_low_water(websockets, peer_session, is_co_mode))
                    {
                        delivered = is_co_mode ? ws_inflight_reserve(*websockets, ws_temp_data.value.size()) : websockets->push(std::move(ws_temp_data));
                    }
                    // 单次等待超时也补一枪：wait 的 LOW 闸门和 reserve 的 HIGH 闸门之间
                    // 有一段灰区，wait 超时那一刻 inflight 可能刚巧从 HIGH 退到中间，
                    // 不重试直接丢会不必要地多出一条 dropped 计数。
                    if (!delivered)
                    {
                        delivered = is_co_mode ? ws_inflight_reserve(*websockets, ws_temp_data.value.size()) : websockets->push(std::move(ws_temp_data));
                    }
                }

                if (delivered)
                {
                    if (is_co_mode)
                    {
                        // 异步处理，支持双工收发；在途计数由 ws_dispatch_onmessage_co 成对释放
                        asio::co_spawn(peer_session->strand_,
                                       ws_dispatch_onmessage_co(websockets, std::move(ws_temp_data)),
                                       asio::detached);
                    }
                    else
                    {
                        // 双工：投完立刻回去继续收帧，不等业务钩子（seqid 已经带在消息里，
                        // 要排序是业务的事）。钩子抛异常由 run_conn_task 收口并记日志。
                        // 池没接单时这条任务被丢弃并计数（getconndropped），读循环不断、连接不关。
                        post_conn_step("ws.onmessage", [ws = websockets]()
                                       { ws->onmessage(); });
                    }
                }
                else
                {
                    // 有界等待（CONST_WEBSOCKET_QUEUE_STALL_MAX_MS）后水位仍堵着：丢最新一条并计数，
                    // 默认不断连。落盘消息必须顺手 unlink，丢一条漏一个临时文件（R3 同族）。
                    if (ws_temp_data.isfile)
                    {
                        std::remove(ws_temp_data.value.c_str());
                    }
                    ws_ingress_dropped += 1;
                    if (ws_ingress_dropped == 1 || ws_ingress_dropped % 256 == 0)
                    {
#ifndef BENCHMARK
                        log_item.clear();
                        log_item.append(peer_session->client_ip);
                        log_item.push_back(0x20);
                        log_item.append(std::to_string(peer_session->client_port));
                        log_item.append(" websocket ingress queue over watermark, dropped ");
                        log_item.append(std::to_string(ws_ingress_dropped));
                        log_item.append(" messages");
                        log_item.push_back('\n');
                        std::unique_lock<std::mutex> lock(log_mutex);
                        error_loglist.emplace_back(log_item);
                        lock.unlock();
#endif
                    }
                }
            }

            peer_session->time_limit.store(timeid() + 86400);
            if (websockets->isclose)
            {
                if (ws_ingress_dropped > 0)
                {
#ifndef BENCHMARK
                    log_item.clear();
                    log_item.append(peer_session->client_ip);
                    log_item.push_back(0x20);
                    log_item.append(std::to_string(peer_session->client_port));
                    log_item.append(" websocket ingress dropped total ");
                    log_item.append(std::to_string(ws_ingress_dropped));
                    log_item.append(" on close");
                    log_item.push_back('\n');
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(log_item);
                    lock.unlock();
#endif
                }
                ws_cleanup_spilled_files(websockets);
                // 等 Close 帧出环后再关，避免握手丢失
                co_await ws_drain_ring_then_stop(peer_session);
                co_return 0;
            }
        }
        peer_session->stop();
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("client exit exception");
        ws_cleanup_spilled_files(websockets);
        peer_session->stop();
    }
    catch (...)
    {
        ws_cleanup_spilled_files(websockets);
        peer_session->stop();
    }
    co_return 0;
}

asio::awaitable<unsigned int> httpserver::client_rpc_loop(unsigned int readnum, std::shared_ptr<client_session> peer_session)
{
    std::shared_ptr<httppeer> peer = std::make_shared<httppeer>();
    std::shared_ptr<rpc_parse> rpc = std::make_shared<rpc_parse>();

    peer->socket_session = peer_session->shared_from_this();
    rpc->peer            = peer;
    std::string log_item;
    try
    {
        for (;;)
        {
            log_item.clear();
            if (readnum == 0)
            {
                bool is_error = co_await peer_session->read_some(readnum, log_item);
                if (is_error)
                {
#ifndef BENCHMARK
                    log_item.push_back(0x20);
                    log_item.append(peer_session->client_ip);
                    log_item.push_back(0x20);
                    log_item.append(std::to_string(peer_session->client_port));

                    log_item.push_back('\n');
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(log_item);
                    lock.unlock();
#endif
                    peer_session->stop();
                    co_return 3;
                }
            }

            if (rpc->isbegin)
            {
                rpc->process_append(peer_session->_cache_data, readnum);
            }
            else
            {
                rpc->process(peer_session->_cache_data, readnum);
            }

            if (rpc->iserror)
            {
                rpc->async_send_error();
                co_await peer_session->async_send_writer(rpc->send_content);
                co_return 3;
            }

            if (rpc->isfinish)
            {
                DEBUG_LOG("---  rpc co handle --------");
                // 对齐 http1 dynamic：解析会话、设置默认响应状态
                peer->linktype = 0;
                peer->parse_session();
                peer->status(200);
                peer->content_type.clear();
                peer->etag.clear();
                peer->output.clear();

                // — v6: 链的执行统一在 router::co_resolve —
                co_await get_router().co_resolve(peer);

                if (!rpc->is_send)
                {
                    if (peer->get_status() < 100)
                    {
                        peer->status(200);
                    }
                    if (!peer->isset_type())
                    {
                        peer->type("text/html; charset=utf-8");
                    }

                    rpc->build_header();
                    if (rpc->iserror)
                    {
                        rpc->async_send_error();
                        co_await peer_session->async_send_writer(rpc->send_content);
                        clear_peer_data(peer);
                        rpc->reset();
                        co_return 3;
                    }

                    co_await peer_session->async_send_writer(rpc->send_content);
                    if (peer->output.size() > 0)
                    {
                        co_await peer_session->async_send_writer(peer->output);
                    }
                }

                // 请求间清理 peer 数据，准备下一个请求
                clear_peer_data(peer);
                rpc->reset();
            }

            readnum = 0;
        }
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("client exit exception");
        peer_session->stop();
    }
    catch (...)
    {
        peer_session->stop();
    }
    co_return 0;
}

// 同步钩子没给出结论（抛出 / 池没接单）时的日志形状：本文件里几条投池通路共用。
static std::string hook_fail_log(const char *what, const std::exception_ptr &eptr, bool rejected, const std::shared_ptr<client_session> &peer_session)
{
    std::string log_item(what);
    log_item.push_back(0x20);
    log_item += eptr ? exception_text(eptr) : (rejected ? "thread pool rejected" : "no result");
    if (peer_session)
    {
        log_item.push_back(0x20);
        log_item.append(peer_session->client_ip);
        log_item.push_back(0x20);
        log_item.append(std::to_string(peer_session->client_port));
    }
    log_item.push_back('\n');
    return log_item;
}

// 三套常驻出站客户端（redis_subpub / ws_subpub / sock_subpub）钩子失败的日志说明：这些连接
// 没有 client_session，只有投递用的 tag。抛出与"池没接单"都必须留一条生产可见的日志，
// 否则消息在无声中消失，线上没有任何线索可查。
static void resident_hook_failed(const char *what, const std::exception_ptr &eptr, bool rejected)
{
    get_server_app().add_error_lists(hook_fail_log(what, eptr, rejected, nullptr));
}

// 协程钩子的收口，和 co_sync_hook_void 对称：抛出留在协程里接住并记录失败原因。
// 放出去会一路撞到 co_spawn(..., asio::detached) 的默认 handler —— 那是在 io_context
// 线程上重抛，整个进程跟着没；而这三条循环是常驻的，一次业务抛出不能带走服务。
[[maybe_unused]] static asio::awaitable<void>
co_resident_hook_void(const char *what, asio::awaitable<void> hook)
{
    try
    {
        co_await std::move(hook);
    }
    catch (...)
    {
        resident_hook_failed(what, std::current_exception(), false);
    }
    co_return;
}

// 同步业务钩子交给业务线程池（clientrunpool）跑的两种收口：钩子在池线程执行，续体回到本连接
// 的协程，所以协议报文的先后次序照旧；抛出与"池没接单"都算业务没给出结论——bool 钩子按否决
// 处理（fail-closed），void 钩子只记日志，两种都不踢连接：钩子异常不该比业务返回 false
// 造成更大的破坏。
asio::awaitable<bool> httpserver::co_sync_hook_bool(const char *what, std::function<bool()> fn, const std::shared_ptr<client_session> &peer_session)
{
    auto out = co_await co_pool_run_bool(std::move(fn), asio::use_awaitable);
    if (!out.eptr && !out.rejected)
        co_return out.value;

    add_error_lists(hook_fail_log(what, out.eptr, out.rejected, peer_session));
    co_return false;
}

// 协程版 bool 钩子的同款收口：抛出照样按否决处理，和上面那句对称。
// 不接住的话异常会一路撞到本连接的会话协程（client_mqtt_loop 外层的 catch），整条连接被拆掉
// —— 比业务老实回 false 的破坏大得多，而同步版走的是线程池收口，两支本来该同果。
[[maybe_unused]] static asio::awaitable<bool>
co_async_hook_bool(const char *what, asio::awaitable<bool> hook, const std::shared_ptr<client_session> &peer_session)
{
    bool value = false;
    try
    {
        value = co_await std::move(hook);
    }
    catch (...)
    {
        get_server_app().add_error_lists(hook_fail_log(what, std::current_exception(), false, peer_session));
        value = false;
    }
    co_return value;
}

asio::awaitable<void> httpserver::co_sync_hook_void(const char *what, std::function<void()> fn, const std::shared_ptr<client_session> &peer_session)
{
    auto out = co_await co_pool_run_void(std::move(fn), asio::use_awaitable);
    if (!out.eptr && !out.rejected)
        co_return;

    add_error_lists(hook_fail_log(what, out.eptr, out.rejected, peer_session));
}

// socket 的 on_close 有三条出口（握手后业务立刻置 isclose、读错、报文循环里看到 isclose），
// 收敛成这一个提交点。await 到钩子跑完才返回：投完就走的写法要让任务捕
// shared_ptr<socket_api>，而它持有 session_sock，会话对象会被任务续命到协程退出之后。
asio::awaitable<void> httpserver::co_socket_on_close(const std::shared_ptr<socket_api> &sock_temp,
                                                     const std::shared_ptr<client_session> &peer_session)
{
    if (sock_temp->isco)
    {
        co_await sock_temp->async_on_close();
        co_return;
    }
    co_await co_sync_hook_void("socket on_close",
                               std::function<void()>([h = sock_temp]
                                                     { h->on_close(); }),
                               peer_session);
}

static void socket_pump_inbound(const std::shared_ptr<socket_api> &sock_temp);

// 业务线程池上的一次派发出站：取队头一片，交给同步钩子，然后把单飞门开回来。
// 开门之后如果队列还有存货，就自己再投一条接着跑 —— 这样一次投递能把积压一路排干，
// 不必等读环下一次进队才想起来唤人。
static void socket_run_inbound_one(const std::shared_ptr<socket_api> &sock_temp)
{
    socket_data_list_t msg;
    if (!sock_temp->isclose && sock_temp->pop_front(msg) && !sock_temp->isclose)
    {
        // 这一片已经从队头取走了，钩子抛异常就把它吞在手里：接住它、记一条日志，
        // 让下面那把单飞门照常开回来。不接的话异常由池在外面 catch，门永远不开，
        // 这条连接的入站从此不再派发（读环还会照常攒到水位，最后歇满阈值关连接）。
        try
        {
            sock_temp->on_message(std::move(msg));
        }
        catch (...)
        {
            get_server_app().add_error_lists(
                hook_fail_log("socket on_message", std::current_exception(), false, sock_temp->session_sock));
        }
    }
    sock_temp->inbound_running.clear(std::memory_order_release);
    if (!sock_temp->isclose && sock_temp->queue_items() > 0)
    {
        socket_pump_inbound(sock_temp);
    }
}

// 队列里有一片、并且这条连接当前没有任务在跑，就投一条；否则直接返回（已经有人在跑了）。
// 那把门是"每连接单飞"：一次 read 拿到的是字节流的一片，没有帧边界，
// 两片同时落在业务手里就会把这条流咬乱，所以同一时刻只允许一个任务处理这一连接的入站。
static void socket_pump_inbound(const std::shared_ptr<socket_api> &sock_temp)
{
    if (sock_temp->isclose)
    {
        return;
    }
    if (sock_temp->inbound_running.test_and_set(std::memory_order_acq_rel))
    {
        return;
    }
    if (!post_conn_step("socket.onmessage", [sock = sock_temp]
                        { socket_run_inbound_one(sock); }))
    {
        // 池没接单，这条任务不会被执行，也不会有人去开门；门不开回来，这条连接的入站从此不再派发。
        sock_temp->inbound_running.clear(std::memory_order_release);
    }
}

// 读环侧唯一的入站投递：入队 → 满了就停读歇拍等队列退下去 → 抢到门就投一条任务 → 立刻返回。
// 返回 false 表示这条连接不能再收了：要么歇满 CONST_SOCKET_QUEUE_STALL_GIVEUP_MS 还是进不了队
// （一片没丢，改成关这条连接退出，已经写过一条日志），要么会话读错 / 业务已经置 isclose。
asio::awaitable<bool> httpserver::co_socket_inbound_push(const std::shared_ptr<socket_api> &sock_temp,
                                                         const std::shared_ptr<client_session> &peer_session,
                                                         socket_data_list_t &&item)
{
    // 到达时刻就在入队这一刻：钩子在业务线程池上可能排在后面很久才跑，
    // 业务拿这个时间值能算出自己这一片等了多久。
    item.arrived_ms = static_cast<unsigned long long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());

    // 直接绑到这条连接的 strand，不依赖协程当前 executor——
    // 如果以后有人换个 executor 调进来，timer 也不会跑错线程。
    asio::steady_timer timer(peer_session->strand_);
    std::string log_item;

    unsigned int tick_ms = CONST_SOCKET_QUEUE_STALL_MS;
    // 停多久拿时钟量，不按拍累加：每拍只是"最多睡这么久"的上界，真睡了多久还含调度开销，
    // 而且决策点那一拍根本还没睡——按累加算，日志里的时长会比实际多报一整拍。
    const auto stall_begin = std::chrono::steady_clock::now();

    // push 在水位上返回 false 时不移动 item，所以每一轮重试拿的都是同一片，一片不丢。
    while (!sock_temp->push(std::move(item)))
    {
        unsigned long long stalled = static_cast<unsigned long long>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - stall_begin)
                .count());
        if (stalled >= CONST_SOCKET_QUEUE_STALL_GIVEUP_MS ||
            peer_session->iserror || sock_temp->isclose)
        {
            if (stalled >= CONST_SOCKET_QUEUE_STALL_GIVEUP_MS && !sock_temp->isclose)
            {
                sock_temp->isclose    = true;
                peer_session->isclose = true;

                log_item.clear();
                log_item.append(peer_session->client_ip);
                log_item.push_back(0x20);
                log_item.append(std::to_string(peer_session->client_port));
                log_item.append(" socket inbound queue still full after stalling ");
                log_item.append(std::to_string(stalled));
                log_item.append(" ms, closing this connection");
                log_item.push_back('\n');
                add_error_lists(log_item);
            }
            co_return false;
        }

        timer.expires_after(std::chrono::milliseconds(tick_ms));
        co_await timer.async_wait(asio::use_awaitable);

        // 下一拍的间隔：200/400/800/1000 各一轮，之后固定 2 秒循环检查队列有没有退下去。
        if (tick_ms < CONST_SOCKET_QUEUE_STALL_TOP_MS)
        {
            tick_ms *= 2;
            if (tick_ms > CONST_SOCKET_QUEUE_STALL_TOP_MS)
            {
                tick_ms = CONST_SOCKET_QUEUE_STALL_TOP_MS;
            }
        }
        else if (tick_ms == CONST_SOCKET_QUEUE_STALL_TOP_MS)
        {
            tick_ms = CONST_SOCKET_QUEUE_STALL_LOOP_MS;
        }
    }

    socket_pump_inbound(sock_temp);
    co_return true;
}

// 原生 socket 发送环的启动次数（全局累计）。一条连接只在还没有环时启动一次消费者协程，
// 所以它应当和"tcp ring close"的行数一致：多出来就是同一条连接上有了两个写者。
static std::atomic<unsigned long long> tcp_ring_spawn_num = 0;

// 连接关闭时把这条连接的出站账落一行：refuse= 发送环拒过的片数（就是
// http2_ring_overflow_count，环满和积压过闸都算它，原生 socket 这条支路唯一会动的一个），
// spawn= 此刻的全局启动累计。没建过环的连接（握手之前就错掉的）不落这一行。
static void tcp_ring_close_log(const std::shared_ptr<client_session> &peer_session)
{
    if (!peer_session->http2_ring_queue)
    {
        return;
    }
    std::string ring_log;
    ring_log.append("tcp ring close ");
    ring_log.append(peer_session->client_ip);
    ring_log.push_back(0x20);
    ring_log.append(std::to_string(peer_session->client_port));
    ring_log.append(" refuse=");
    ring_log.append(std::to_string(peer_session->http2_ring_overflow_count.load(std::memory_order_relaxed)));
    ring_log.append(" spawn=");
    ring_log.append(std::to_string(tcp_ring_spawn_num.load(std::memory_order_relaxed)));
    ring_log.push_back('\n');
    get_server_app().add_error_lists(ring_log);
}

asio::awaitable<unsigned int> httpserver::client_tcp_loop(unsigned int readnum, std::shared_ptr<client_session> peer_session)
{
    std::string log_item;
    unsigned long long socket_seq_id = 0;

    if (readnum < 4)
    {
        peer_session->isclose = true;
        co_return 0;
    }

    if (!(peer_session->_cache_data[0] == 't' && peer_session->_cache_data[1] == 'c' && peer_session->_cache_data[2] == 'p' && peer_session->_cache_data[3] == 0x20))
    {
        peer_session->isclose = true;
        co_return 0;
    }

    HTTP_SOCKET_REG &hsock = get_http_socket_reg();

    // 累积请求行：tcp name[/myid[/groupid]][?query]\n
    // 路径与握手都可能跨多个 TCP 包，统一在 req_buf 里累积。
    std::string req_buf;
    req_buf.append((char *)peer_session->_cache_data, readnum);
    if (req_buf.size() > CONST_TCP_HANDSHAKE_MAX)
    {
        peer_session->isclose = true;
        co_return 0;
    }

    // 阶段一：扫描路径终止符（\n ? 或空格），可能需要读更多包
    std::size_t path_end = std::string::npos;
    while (path_end == std::string::npos)
    {
        for (std::size_t i = 4; i < req_buf.size(); i++)
        {
            char c = req_buf[i];
            if (c == 0x0A || c == '?' || c == 0x20)
            {
                path_end = i;
                break;
            }
        }
        if (path_end == std::string::npos)
        {
            unsigned int rn = 0;
            std::string more;
            peer_session->time_limit.store(timeid());
            bool is_error = co_await peer_session->read_socket(rn, more);
            if (is_error)
            {
                peer_session->isclose = true;
                co_return 0;
            }
            req_buf.append(more);
            if (req_buf.size() > CONST_TCP_HANDSHAKE_MAX)
            {
                peer_session->isclose = true;
                co_return 0;
            }
        }
    }

    // 解析路径段 [4, path_end)：name 或 name/myid 或 name/myid/groupid
    std::string name;
    unsigned int myid = 0, groupid = 0;
    {
        unsigned char seg = 0;// 0=name, 1=myid, 2=groupid
        std::string seg_buf;
        for (std::size_t i = 4; i < path_end; i++)
        {
            char c = req_buf[i];
            if (c == '/')
            {
                // 名字本身可以写成 "/name"（和 HTTP 路径同一个习惯）：段首的空名字不算
                // 分隔符，不然 "/name" 会被切成"空名字 + myid=name"，谁都对不上。
                if (seg == 0 && seg_buf.empty())
                {
                    continue;
                }
                if (seg == 0)
                {
                    name = seg_buf;
                }
                else if (seg == 1)
                {
                    myid = 0;
                    for (char d : seg_buf)
                        if (d >= '0' && d <= '9')
                            myid = myid * 10 + (d - '0');
                }
                seg_buf.clear();
                seg++;
                continue;
            }
            seg_buf.push_back(c);
        }
        if (seg == 0)
            name = seg_buf;
        else if (seg == 1)
        {
            myid = 0;
            for (char d : seg_buf)
                if (d >= '0' && d <= '9')
                    myid = myid * 10 + (d - '0');
        }
        else if (seg == 2)
        {
            groupid = 0;
            for (char d : seg_buf)
                if (d >= '0' && d <= '9')
                    groupid = groupid * 10 + (d - '0');
        }
    }

    if (name.size() < 2 || name.size() > 36)
    {
        peer_session->isclose = true;
        co_return 0;
    }

    // 注册表里的键在启动时统一补过前导 '/'（见 _inithttpsocketmethodregto 之后那段），
    // 握手侧不管写成 name 还是 /name，切出来的都是裸名字，这里补上 '/' 就是唯一的查找键。
    std::string sock_name;
    sock_name.push_back('/');
    sock_name += name;

    auto iter_s = hsock.find(sock_name);
    if (iter_s != hsock.end())
    {
        peer_session->httpv = 8;

        // 发送环 + 唯一写者协程：从这一刻起这条连接的出站数据都从环里串行出去。
        // 已经有环就不重复启动 —— 同一条连接上派两个写者，两帧的字节会交错。
        if (!peer_session->http2_ring_queue)
        {
            auto &ring_pool                = get_http2_ring_queue_obj();
            peer_session->http2_ring_queue = ring_pool.get_cache_ptr();
            tcp_ring_spawn_num.fetch_add(1, std::memory_order_relaxed);
            asio::co_spawn(peer_session->strand_, ring_client_server(peer_session), asio::detached);
        }

        std::shared_ptr<socket_api> sock_temp = iter_s->second(myid, groupid, peer_session);

        sock_temp->url = name;
        {
            std::lock_guard<std::mutex> lk(socket_task_mutex);
            sockettasks.push_back(sock_temp);
        }

        if (sock_temp->isco)
        {
            co_await sock_temp->async_on_open();
        }
        else
        {
            // 同步 on_open 下业务线程池。await 到跑完：业务在这一步置 isclose 来否决本连接，
            // 而循环要到握手之后才读它，投完就走等于把"可否决"变成竞态。
            co_await co_sync_hook_void("socket on_open",
                                       std::function<void()>([h = sock_temp]
                                                             { h->on_open(); }),
                                       peer_session);
        }

        // 阶段二：握手 [?query]\n\n 或空格分隔，剩余是 body
        // 从 path_end 继续，req_buf 可能已含部分 body
        bool handshake_done     = false;
        std::size_t body_offset = 0;
        while (!handshake_done)
        {
            if (path_end < req_buf.size())
            {
                char sep = req_buf[path_end];
                if (sep == '?')
                {
                    // 有 query：解析到第一个 '\n'，再等第二个 '\n' 组成 \n\n
                    std::size_t nl = req_buf.find('\n', path_end);
                    if (nl != std::string::npos)
                    {
                        // 解析 query 字符串（? 之后、\n 之前）
                        std::string key, val;
                        bool in_val = false;
                        for (std::size_t i = path_end + 1; i < nl; i++)
                        {
                            char c = req_buf[i];
                            if (c == '=')
                            {
                                in_val = true;
                            }
                            else if (c == '&')
                            {
                                if (!key.empty())
                                {
                                    sock_temp->header.push_back({key, val});
                                }
                                key.clear();
                                val.clear();
                                in_val = false;
                            }
                            else if (in_val)
                            {
                                val.push_back(c);
                            }
                            else
                            {
                                key.push_back(c);
                            }
                        }
                        if (!key.empty())
                        {
                            sock_temp->header.push_back({key, val});
                        }

                        // 检查 \n\n
                        if (nl + 1 < req_buf.size() && req_buf[nl + 1] == '\n')
                        {
                            body_offset    = nl + 2;
                            handshake_done = true;
                        }
                    }
                }
                else if (sep == '\n')
                {
                    // 无 query：\n\n 分隔
                    if (path_end + 1 < req_buf.size() && req_buf[path_end + 1] == '\n')
                    {
                        body_offset    = path_end + 2;
                        handshake_done = true;
                    }
                }
                else if (sep == ' ')
                {
                    // 无 query：空格分隔
                    body_offset    = path_end + 1;
                    handshake_done = true;
                }
                else
                {
                    // 非法握手：on_open 可能已经投过东西，但握手没成，不必等环排空，
                    // stop() 会把挂在环上的消费者唤醒让它退出。
                    peer_session->isclose = true;
                    tcp_ring_close_log(peer_session);
                    peer_session->stop();
                    co_return 0;
                }
            }

            if (!handshake_done)
            {
                unsigned int rn = 0;
                std::string more;
                peer_session->time_limit.store(timeid());
                bool is_error = co_await peer_session->read_socket(rn, more);
                if (is_error)
                {
                    peer_session->isclose = true;
                    tcp_ring_close_log(peer_session);
                    peer_session->stop();
                    co_return 0;
                }
                req_buf.append(more);
                if (req_buf.size() > CONST_TCP_HANDSHAKE_MAX)
                {
                    peer_session->isclose = true;
                    tcp_ring_close_log(peer_session);
                    peer_session->stop();
                    co_return 0;
                }
            }
        }

        // 握手完成，把剩余 body 直接交给业务
        if (body_offset < req_buf.size())
        {
            if (sock_temp->issyncmsg)
            {
                socket_data_list_t rest;
                rest.seqid = socket_seq_id++;
                rest.value = req_buf.substr(body_offset);
                if (!co_await co_socket_inbound_push(sock_temp, peer_session, std::move(rest)))
                {
                    peer_session->isclose = true;
                    co_await co_socket_on_close(sock_temp, peer_session);
                    tcp_ring_close_log(peer_session);
                    co_await ws_drain_ring_then_stop(peer_session);
                    co_return 0;
                }
            }
            else
            {
                co_await sock_temp->async_on_message(req_buf.substr(body_offset));
            }
        }

        if (sock_temp->isclose)
        {
            peer_session->isclose = true;
            co_await co_socket_on_close(sock_temp, peer_session);
            tcp_ring_close_log(peer_session);
            co_await ws_drain_ring_then_stop(peer_session);
            co_return 0;
        }

        for (;;)
        {
            peer_session->time_limit.store(timeid());
            log_item        = {};
            unsigned int rn = 0;
            bool is_error   = co_await peer_session->read_socket(rn, log_item);

            if (is_error)
            {
                log_item.push_back(0x20);
                log_item.append(peer_session->client_ip);
                log_item.push_back(0x20);
                log_item.append(std::to_string(peer_session->client_port));

                log_item.push_back('\n');
                std::unique_lock<std::mutex> lock(log_mutex);
                error_loglist.emplace_back(log_item);
                lock.unlock();

                sock_temp->isclose = true;
                co_await co_socket_on_close(sock_temp, peer_session);

                tcp_ring_close_log(peer_session);
                co_await ws_drain_ring_then_stop(peer_session);
                co_return 3;
            }
            //Error handling here
            if (sock_temp->isclose)
            {
                co_await co_socket_on_close(sock_temp, peer_session);
                break;
            }

            if (sock_temp->issyncmsg)
            {
                // 投完就走：这一片进接收队列后读环立刻回去收下一片，钩子在业务线程池上跑。
                // 返回 false 意味着队列歇满 5 分钟还满（或会话已读错），这一片没丢，
                // 改走关闭路径 —— 循环里下一个 isclose 检查在"再读一片"之后，等不到。
                socket_data_list_t piece;
                piece.seqid = socket_seq_id++;
                piece.value = std::move(log_item);
                if (!co_await co_socket_inbound_push(sock_temp, peer_session, std::move(piece)))
                {
                    peer_session->isclose = true;
                    co_await co_socket_on_close(sock_temp, peer_session);
                    break;
                }
            }
            else
            {
                co_await sock_temp->async_on_message(log_item);
            }
        }

        // 两条 break 都落到这里：业务置了 isclose，或者入站歇满阈值不再收。
        // 先关掉入环（isclose 之后 post_write 一律拒），再等环排空、最后 stop()，
        // 否则在途数据会跟着 socket 一起被丢掉，消费者协程也醒不过来。
        peer_session->isclose = true;
        tcp_ring_close_log(peer_session);
        co_await ws_drain_ring_then_stop(peer_session);
    }
    else
    {
        peer_session->isclose = true;
        co_return 0;
    }

    co_return 0;
}

// ====== MQTT 5 Will 延迟发布（will_delay_interval，§3.2.2.26.3 / §4.3.3）======
// 客户端在 CONNECT 属性里声明的延迟秒数此前只是存进结构体（mqtt_frame.cpp:565），
// 服务端收到断连就立刻发遗嘱。下面这两个计数是这条支路的在途量与丢弃量。
static std::atomic<unsigned long long> mqtt_will_delay_pending = 0;
static std::atomic<unsigned long long> mqtt_will_delay_dropped = 0;

// 认领一个在途名额：判定和占位必须在同一次 CAS 里完成，先 load 再 fetch_add 是
// check-then-act，两个同时断开的连接可以各自看着界内、然后一起加上去。
// 返回 false = 名额已满，调用方丢弃这条遗嘱并计数（丢最新 + 计数，不断连，与 ws 入站水位同口径）。
static bool mqtt_will_delay_acquire()
{
    unsigned long long cur = mqtt_will_delay_pending.load(std::memory_order_acquire);
    for (;;)
    {
        if (cur >= CONST_MQTT_WILL_DELAY_MAX_PENDING)
        {
            return false;
        }
        if (mqtt_will_delay_pending.compare_exchange_weak(cur, cur + 1, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            return true;
        }
    }
}

// 等满 delay_sec 再发这条遗嘱。它只带 client_id 和遗嘱的副本，不带会话：
// 挂 shared_ptr 等于把整条会话（读缓冲 + inflight 表）按住最长一小时，那正是这里要封顶的东西。
static asio::awaitable<void> mqtt_publish_delayed_will(std::string client_id,
                                                       mqtt_will_message will,
                                                       unsigned long long delay_sec)
{
    // 三条退出路径（定时器出错、被接管跳过、正常发布）都要还名额，所以用析构守卫
    struct slot_guard
    {
        ~slot_guard()
        {
            mqtt_will_delay_pending.fetch_sub(1, std::memory_order_acq_rel);
        }
    } slot;

    asio::steady_timer timer(co_await asio::this_coro::executor);
    timer.expires_after(std::chrono::seconds(delay_sec));
    try
    {
        co_await timer.async_wait();
    }
    catch (...)
    {
        // co_spawn(..., asio::detached) 的默认 handler 会把异常抛回 io_context 所在线程，
        // 那等于把整个服务打停，所以这里必须接住
        co_return;
    }

    // 取消判据：到期时这个 client_id 已被别的会话占着 ⇒ 客户端在延迟窗口内重连过，遗嘱作废
    // （MQTT 5 §4.3.3）。调度之后本会话就 cleanup() 把自己从索引摘掉了，所以"没被接管"的正常
    // 到期在这里读到的是空指针 —— 不能反过来写成"find_client 等于本会话才发"，那样每条都发不出去。
    // 已知边界：窗口内"重连、又断开"会把索引腾空，于是这条旧遗嘱仍会发出去。要修它得给
    // client_id 加一个"已被接管过"的世代号，本框架没有可恢复会话状态，先如实记下。
    if (mqtt_broker::instance().find_client(client_id))
    {
        co_return;
    }

    mqtt_publish_info pub;
    pub.topic.assign(will.topic);
    pub.payload = std::make_shared<std::string>(will.payload);
    pub.qos     = will.qos;
    pub.retain  = will.retain;
    // received_at 取"真正发出去的那一刻"：沿用断开时的读数会把整段延迟算进消息在服务端的停留时间，
    // 转发时 Message Expiry 的剩余寿命就被多扣了一截（§3.3.2.3.3）
    pub.received_at = mqtt_detail::now_monotonic_sec();
    // exclude = nullptr：这条连接早没了，不需要"不回投给自己"的过滤
    mqtt_api::publish_to_broker(pub, nullptr);
    co_return;
}

asio::awaitable<unsigned int> httpserver::client_mqtt_loop(unsigned int readnum, std::shared_ptr<client_session> peer_session)
{
    peer_session->httpv = 9;

    std::shared_ptr<http::mqtt_session> sess;
    std::shared_ptr<http::mqtt_api> handler;
    bool clean_disconnect = false;// 收到 Clean DISCONNECT 帧 → 不发布 Will
    bool established      = false;// CONNECT 握手成功并建立会话后才允许发布 Will

    // 内层协程 + 外层 try/catch：所有退出路径统一 cleanup + on_disconnect
    auto session_body = [&]() -> asio::awaitable<void>
    {
        using namespace http;

        // readnum = 嗅探阶段预读量，作为会话初始读缓冲。
        sess = std::make_shared<mqtt_session>(peer_session, peer_session->_cache_data, readnum);
        sess->init();
        sess->touch_activity();

        // 发送环：从对象池分配 + 启动专用消费者协程（与 http2 同源）。
        // CONNACK 起所有出站报文都走 mqtt_session::write → post_write → 环。
        if (!peer_session->http2_ring_queue)
        {
            auto &ring_pool                = get_http2_ring_queue_obj();
            peer_session->http2_ring_queue = ring_pool.get_cache_ptr();
            asio::co_spawn(peer_session->strand_, ring_client_server(peer_session), asio::detached);
        }

        // 会话注册清单：全局分发协程（mqtt_send_loop）据此回灌挂起帧；
        // 不主动注销，weak expired 由巡检剪除
        {
            std::lock_guard<std::mutex> list_lk(mqtt_sessions_mutex);
            mqtt_sessions.emplace_back(sess);
        }

        // 断开辅助：发 DISCONNECT + 标记关闭
        auto drop_with_reason = [&](mqtt_reason rc)
        {
            sess->write(make_disconnect(rc));
            peer_session->isclose = true;
        };

        // --- CONNECT 握手 ---
        auto first = co_await sess->read_packet();
        if (!first)
        {
            co_return;
        }
        if (first->type != mqtt_packet_type::CONNECT)
        {
            // 首帧必须是 CONNECT
            drop_with_reason(mqtt_reason::protocol_error);
            co_return;
        }

        mqtt_client_info info;
        mqtt_reason reject = mqtt_reason::success;
        if (!parse_connect(first->body_data(), first->body_size(), info, reject))
        {
            mqtt_connack_props props;
            props.reason_string = "CONNECT rejected";
            sess->write(make_connack(false, reject, props, false));
            peer_session->isclose = true;
            co_return;
        }

        if (!parse_client_id(info.client_id, info))
        {
            mqtt_connack_props props;
            props.reason_string = "client id format invalid";
            sess->write(make_connack(false, mqtt_reason::client_identifier_not_valid, props, true));
            peer_session->isclose = true;
            co_return;
        }

        MQTT_REG &mqtt_reg = get_mqtt_reg();
        auto iter          = mqtt_reg.find(info.reg_key);
        if (iter == mqtt_reg.end())
        {
            mqtt_connack_props props;
            props.reason_string = "reg_key not registered";
            sess->write(make_connack(false, mqtt_reason::not_authorized, props, true));
            peer_session->isclose = true;
            co_return;
        }

        auto new_handler = iter->second(info, sess);
        if (!new_handler)
        {
            mqtt_connack_props props;
            sess->write(make_connack(false, mqtt_reason::server_unavailable, props, true));
            peer_session->isclose = true;
            co_return;
        }

        // 业务认证：协程版走 async_on_auth，同步版交给业务线程池，两条都不占协程线程
        bool authed = new_handler->is_coroutine() ? co_await new_handler->async_on_auth() : co_await co_sync_hook_bool("mqtt on_auth", std::function<bool()>([h = new_handler]
                                                                                                                                                             { return h->on_auth(); }),
                                                                                                                       peer_session);
        if (!authed)
        {
            mqtt_connack_props props;
            props.reason_string = "auth rejected";
            sess->write(make_connack(false, mqtt_reason::not_authorized, props, true));
            peer_session->isclose = true;
            co_return;
        }

        // 同 ClientID 踢旧连接
        auto old_session = mqtt_broker::instance().register_client(info.client_id, sess);
        if (old_session && old_session != sess)
        {
            // 先标记再关：旧 loop 的统一清理段据此跳过 Will 发布。客户端本人还活着（只是换了条
            // TCP），发旧遗嘱会向订阅者谎报掉线；写在 DISCONNECT 之前，让"标记没赶上"的窗口
            // 只剩旧 loop 已经进到 Will 那一行的极端情况。
            old_session->mark_taken_over();
            old_session->write(make_disconnect(mqtt_reason::session_taken_over));
            old_session->cleanup();
            old_session->transport()->isclose = true;
        }

        sess->set_client_info(info);

        // --- CONNACK ---
        // CONNACK 三个入站方向属性宣告的是"服务端自己的策略"，与客户端 CONNECT 宣告值无关：
        //   receive_maximum      = 服务端能并发接收的 QoS>0 数
        //   maximum_packet_size  = 服务端入站单帧上限
        //   topic_alias_maximum  = 服务端接受的入站别名上限
        // （客户端宣告值只用于约束服务端出站方向，见 mqtt_session::deliver）
        mqtt_connack_props cprops;
        cprops.receive_maximum     = MQTT_DEFAULT_RECEIVE_MAXIMUM;
        cprops.maximum_qos         = MQTT_DEFAULT_MAXIMUM_QOS;
        cprops.maximum_packet_size = static_cast<uint32_t>(MQTT_MAX_PACKET_SIZE);
        cprops.topic_alias_maximum = MQTT_DEFAULT_TOPIC_ALIAS_MAX;
        // 入站拆帧上限 = 服务端策略，不再跟随客户端宣告值
        sess->set_max_inbound_packet(MQTT_MAX_PACKET_SIZE);
        sess->set_server_topic_alias_max(MQTT_DEFAULT_TOPIC_ALIAS_MAX);
        sess->write(make_connack(false, mqtt_reason::success, cprops, true));

        handler = new_handler;
        // CONNACK 已经发出，会话算建立：从这一刻起非正常断连才发布 Will，
        // 建会话之前失败（工厂、auth、踢旧）那条路径上对端还没登记过遗嘱，不该发。
        established = true;
        if (handler->is_coroutine())
        {
            co_await handler->async_on_connect();
        }
        else
        {
            co_await co_sync_hook_void(
                "mqtt on_connect",
                std::function<void()>([h = handler]
                                      { h->on_connect(); }),
                peer_session);
        }
        // 定时 tick 登记（websocket_loop 的 mqtttasks 段扫描）；
        // 须在 on_connect 之后：业务在 on_connect 里置 loop_num，先登记会以 0 被摘除
        {
            std::lock_guard<std::mutex> lk(mqtt_task_mutex);
            mqtttasks.emplace_back(handler);
        }

        // --- 报文循环 ---
        for (;;)
        {
            if (peer_session->isclose || sess->is_closed())
            {
                co_return;
            }

            auto pkt = co_await sess->read_packet();
            if (!pkt)
            {
                // 读异常
                co_return;
            }
            sess->touch_activity();// 刷新活跃时间

            switch (pkt->type)
            {
            case mqtt_packet_type::PINGREQ:
                sess->write(make_pingresp());
                break;

            case mqtt_packet_type::DISCONNECT:
                clean_disconnect = true;// 干净断连：按 MQTT5 §3.1.2.5 不发布 Will
                co_return;

            case mqtt_packet_type::SUBSCRIBE:
                co_await handle_mqtt_subscribe(sess, handler, *pkt, drop_with_reason);
                break;

            case mqtt_packet_type::UNSUBSCRIBE:
            {
                uint16_t packet_id = 0;
                std::vector<mqtt_unsubscribe_entry> entries;
                if (!parse_unsubscribe(pkt->body_data(), pkt->body_size(), packet_id, entries))
                {
                    drop_with_reason(mqtt_reason::malformed_packet);
                    co_return;
                }
                std::vector<mqtt_reason> codes;
                codes.reserve(entries.size());
                for (auto &e : entries)
                {
                    // 语义非法的 filter（空串 / 通配符位置错误 / $share 形态错）：
                    // 逐条回 0x8F，不落库、也不触发业务退订回调（MQTT 5 §3.11.3）
                    if (!e.valid)
                    {
                        codes.push_back(mqtt_reason::topic_filter_invalid);
                        continue;
                    }
                    mqtt_broker::instance().unsubscribe(e.topic, sess);
                    // 退订通知：协程版业务走 async_on_unsubscribe，同步版交给业务线程池
                    if (handler->is_coroutine())
                        co_await handler->async_on_unsubscribe(e.topic);
                    else
                        co_await co_sync_hook_void(
                            "mqtt on_unsubscribe",
                            std::function<void()>([h = handler, t = e.topic]
                                                  { h->on_unsubscribe(t); }),
                            peer_session);
                    codes.push_back(mqtt_reason::success);
                }
                sess->write(make_unsuback(packet_id, codes));
            }
            break;

            case mqtt_packet_type::PUBLISH:
                co_await handle_mqtt_publish(sess, handler, *pkt, drop_with_reason);
                break;

            case mqtt_packet_type::PUBACK:
            case mqtt_packet_type::PUBREC:
            case mqtt_packet_type::PUBCOMP:
            {
                uint16_t pid   = 0;
                mqtt_reason rc = mqtt_reason::success;
                bool has_pid   = false;
                if (parse_ack(pkt->type, pkt->body_data(), pkt->body_size(), pid, rc, has_pid) && has_pid)
                {
                    sess->ack_outgoing(pkt->type, pid, rc);
                }
                else
                {
                    drop_with_reason(mqtt_reason::malformed_packet);
                    co_return;
                }
            }
            break;

            case mqtt_packet_type::PUBREL:
            {
                uint16_t pid   = 0;
                mqtt_reason rc = mqtt_reason::success;
                bool has_pid   = false;
                if (!parse_ack(pkt->type, pkt->body_data(), pkt->body_size(), pid, rc, has_pid) || !has_pid)
                {
                    drop_with_reason(mqtt_reason::malformed_packet);
                    co_return;
                }
                auto pending = sess->take_inbound_qos2(pid);
                bool applied = true;
                if (pending)
                {
                    // QoS 2 最终投递 + retained 落库（返回 false = 落库被配额拒绝）
                    applied = mqtt_api::publish_to_broker(*pending, sess);
                }
                // PUBCOMP 放在投递之后：retained 落库被配额拒绝时回 0x97
                sess->write(make_pubcomp(pid, applied ? mqtt_reason::success : mqtt_reason::quota_exceeded));
            }
            break;

            case mqtt_packet_type::AUTH:
                // 跳过增强认证
                break;

            default:
                // 未支持的 packet type → 协议错误
                drop_with_reason(mqtt_reason::protocol_error);
                co_return;
            }

            // 读循环检查点：本轮若有报文入环且消费者挂起，顺带唤醒清积压（与 http2 循环同款）
            // 续灌挂起队列：收到帧 = 对端刚消费/确认过报文，它的发送环多半腾了槽。
            // 环腾槽本身没有通知分发协程的通道，这一拍就是 QoS1/2 的回灌事件源。
            sess->flush_pending();
            if (peer_session->http2_need_wakeup)
            {
                peer_session->waituphttp2();
            }
        }
    };

    try
    {
        co_await session_body();
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("client_mqtt_loop exception: %s", e.what());
    }
    catch (...)
    {
        DEBUG_LOG("client_mqtt_loop unknown exception");
    }

    // --- 统一清理 ---
    if (sess)
    {
        // 异常/网络断连（非 Clean DISCONNECT、且会话已建立）时发布 Will（MQTT 5 §3.1.2.5）。
        // 被同 ClientID 的新 CONNECT 接管（is_taken_over）也跳过：客户端还活着，只是换了条 TCP。
        const auto &wi = sess->client_info().will;
        if (wi.has && !clean_disconnect && established && !sess->is_taken_over())
        {
            // will_delay_interval>0 就把发布推迟到客户端声明的秒数（超上限按上限，不丢弃），
            // 窗口内该 client_id 被新会话占着则取消；0 走原来的立即发布，行为与改造前一致。
            unsigned long long delay = wi.will_delay_interval;
            if (delay > CONST_MQTT_WILL_DELAY_MAX_SEC)
            {
                delay = CONST_MQTT_WILL_DELAY_MAX_SEC;
            }
            if (delay == 0)
            {
                mqtt_publish_info pub;
                pub.topic.assign(wi.topic);
                pub.payload     = std::make_shared<std::string>(wi.payload);
                pub.qos         = wi.qos;
                pub.retain      = wi.retain;
                pub.received_at = mqtt_detail::now_monotonic_sec();
                mqtt_api::publish_to_broker(pub, sess);
            }
            else if (mqtt_will_delay_acquire())
            {
                // 不挂在这条连接的 strand 上：peer_session 的会话协程已经要退出了，
                // 定时器只需要 io_context 活着，遗嘱是全局广播，与这条 TCP 无关。
                asio::co_spawn(this->io_context,
                               mqtt_publish_delayed_will(sess->client_id(), wi, delay),
                               asio::detached);
            }
            else
            {
                // 在途定时器已满：丢这一条遗嘱并计数，不断连（与 ws 入站水位同口径）
                unsigned long long dropped =
                    mqtt_will_delay_dropped.fetch_add(1, std::memory_order_acq_rel) + 1;
                if (dropped == 1 || dropped % 256 == 0)
                {
#ifndef BENCHMARK
                    std::string log_item;
                    log_item.append(peer_session->client_ip);
                    log_item.push_back(0x20);
                    log_item.append(std::to_string(peer_session->client_port));
                    log_item.append(" mqtt will delay queue full, dropped ");
                    log_item.append(std::to_string(dropped));
                    log_item.append(" wills");
                    log_item.push_back('\n');
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(log_item);
                    lock.unlock();
#endif
                }
            }
        }
        sess->cleanup();
    }
    if (handler)
    {
        try
        {
            // 断开回调必达：协程版业务走 async_on_disconnect，同步版交给业务线程池
            if (handler->is_coroutine())
                co_await handler->async_on_disconnect();
            else
                co_await co_sync_hook_void(
                    "mqtt on_disconnect",
                    std::function<void()>([h = handler]
                                          { h->on_disconnect(); }),
                    peer_session);
        }
        catch (...)
        {
        }
    }
    peer_session->isclose = true;
    // 唤醒可能挂起的发送消费者协程，让它看到 isclose 后退出
    peer_session->waituphttp2();
    co_return 0;
}

// MQTT 全局分发协程：扫 mqtt_sessions 注册清单，把有挂起帧的会话按 FIFO 回灌
// 发送环（绝不在调用线程 sleep；本协程跑在 io_context 上）。
// 运行协议（httpserver 的 mqtt_sender_need_spawn）：
//   * true ⟺ 协程不在跑；websocket_loop 每秒拍 exchange(false) 认领并 spawn，防双跑。
//   * 每轮之间固定歇 200ms：这条挂起队列不追求"立刻发完"，让出 CPU 由各连接的
//     消费者协程自己把发送环写空，回来再灌下一轮（≤16 槽/轮）；
//     与 http2 环消费者同族——那边"攒一批再抽干"，间隔由环的空/满决定；挂起帧的
//     QoS0 读者不 ack、没有任何对端事件可依，所以把同一间隔写成显式 200ms；
//   * 有货却一格都灌不动（各连接环都满）→ 退出置标志，交给下一拍 respawn；
//   * 全空 → 先置标志、再复查清单（Dekker），仍无活才退出——封死与 enqueue_pending
//     之间的丢唤醒窗口。
asio::awaitable<void> httpserver::mqtt_send_loop()
{
    auto execut = co_await asio::this_coro::executor;
    asio::steady_timer timer(execut);
    while (true)
    {
        // 锁序红线：mqtt_sessions_mutex 内绝不取会话 out_mu_（deliver 方向相反）——
        // 持锁只拷快照 + 剪 expired，判定与 flush 全部离锁做
        std::vector<std::weak_ptr<http::mqtt_session>> snapshot;
        {
            std::lock_guard<std::mutex> list_lk(mqtt_sessions_mutex);
            std::erase_if(mqtt_sessions,
                          [](const std::weak_ptr<http::mqtt_session> &w)
                          { return w.expired(); });
            snapshot = mqtt_sessions;
        }
        bool any_pending = false;
        size_t moved     = 0;
        for (auto &w : snapshot)
        {
            auto sess = w.lock();
            if (!sess || !sess->has_pending())
                continue;
            any_pending = true;
            moved += sess->flush_pending();
        }
        if (!any_pending)
        {
            mqtt_sender_need_spawn.store(true);// 置标志后复查（Dekker 防丢）
            std::vector<std::weak_ptr<http::mqtt_session>> recheck;
            {
                std::lock_guard<std::mutex> list_lk(mqtt_sessions_mutex);
                recheck = mqtt_sessions;
            }
            bool still_empty = true;
            for (auto &w : recheck)
            {
                auto sess = w.lock();
                if (sess && sess->has_pending())
                {
                    still_empty = false;
                    break;
                }
            }
            if (still_empty)
                co_return;
            // 复查期间又有货：收回标志续跑。用 CAS 而非裸 store——若 ① 置 true 后
            // websocket_loop 那拍已经 exchange 走并 spawn 了新协程，这里 CAS 失败，
            // 本协程直接退出，避免两个 mqtt_send_loop 同时跑
            bool expect = true;
            if (!mqtt_sender_need_spawn.compare_exchange_strong(expect, false))
                co_return;
            continue;
        }
        if (moved == 0)
        {
            // 有货却一格都灌不动（各连接发送环都满）：不在协程内等槽，
            // 退出置标志交给下一拍 respawn
            mqtt_sender_need_spawn.store(true);
            co_return;
        }
        // 灌进过货也要歇一拍：这条队列不追求"立刻发完"，200ms 把 CPU 整段让出去
        // （各连接的消费者协程在此期间自己把环写空），回来再扫下一轮
        timer.expires_after(std::chrono::milliseconds(200));
        co_await timer.async_wait(asio::use_awaitable);
    }
}

// SUBSCRIBE: 订阅 + SUBACK + Retained 投递
asio::awaitable<void> httpserver::handle_mqtt_subscribe(std::shared_ptr<http::mqtt_session> sess,
                                                        std::shared_ptr<http::mqtt_api> handler,
                                                        const http::mqtt_session::packet &pkt,
                                                        std::function<void(http::mqtt_reason)> drop)
{
    using namespace http;

    uint16_t packet_id = 0;
    std::vector<mqtt_subscribe_entry> entries;
    if (!parse_subscribe(pkt.body_data(), pkt.body_size(), packet_id, entries))
    {
        drop(mqtt_reason::malformed_packet);
        co_return;
    }

    auto info = sess->client_info();
    std::vector<mqtt_reason> codes;
    codes.reserve(entries.size());

    for (auto &e : entries)
    {
        // 订阅选项非法（QoS=3 / Retain Handling=3 / 保留位非 0）：该 filter 回 0x8F，
        // 不得降级授予（原实现 min(qos,2) 会把 QoS=3 静默变成 QoS=2）。
        if (!e.valid)
        {
            codes.push_back(mqtt_reason::topic_filter_invalid);
            continue;
        }

        uint8_t granted_qos = std::min<uint8_t>(e.options.qos, MQTT_DEFAULT_MAXIMUM_QOS);

        bool allowed = handler->is_coroutine() ? co_await co_async_hook_bool("mqtt async_on_can_subscribe", handler->async_on_can_subscribe(e.topic, granted_qos), sess->transport()) : co_await co_sync_hook_bool("mqtt on_can_subscribe", std::function<bool()>([h = handler, t = e.topic, q = granted_qos]
                                                                                                                                                                                                                                                                  { return h->on_can_subscribe(t, q); }),
                                                                                                                                                                                                                   sess->transport());
        if (!allowed)
        {
            codes.push_back(mqtt_reason::not_authorized);
            continue;
        }

        auto sub_ret = mqtt_broker::instance().subscribe(
            info.reg_key,
            info.group,
            info.device,
            e.topic,
            granted_qos,
            e.options.no_local,
            e.options.retain_as_published,
            e.options.retain_handling,
            sess);

        if (sub_ret == mqtt_subscribe_result::rejected_filter)
        {
            codes.push_back(mqtt_reason::topic_filter_invalid);
            continue;
        }
        if (sub_ret == mqtt_subscribe_result::rejected_quota)
        {
            // 会话级 / 全局订阅数配额：回 0x97，不落库、不投 retained
            codes.push_back(mqtt_reason::quota_exceeded);
            continue;
        }

        if (handler->is_coroutine())
        {
            co_await handler->async_on_subscribe(e.topic, granted_qos);
        }
        else
        {
            co_await co_sync_hook_void(
                "mqtt on_subscribe",
                std::function<void()>([h = handler, t = e.topic, q = granted_qos]
                                      { h->on_subscribe(t, q); }),
                sess->transport());
        }

        switch (granted_qos)
        {
        case 0: codes.push_back(mqtt_reason::success); break;
        case 1: codes.push_back(mqtt_reason::granted_qos1); break;
        default: codes.push_back(mqtt_reason::granted_qos2); break;
        }

        // Retain Handling（MQTT 5 §3.8.3.1）：0=总是投递；1=仅当该订阅此前不存在；2=从不投递。
        // 原实现只判了 !=2，等于把 1 当成 0，客户端每次重订阅都会被 retained 洪水冲一遍。
        bool send_retained = (e.options.retain_handling == 0) ||
                             (e.options.retain_handling == 1 &&
                              sub_ret == mqtt_subscribe_result::added);
        if (send_retained)
        {
            const uint64_t now_sec = mqtt_detail::now_monotonic_sec();
            auto retained_list     = mqtt_broker::instance().match_retained(e.topic, now_sec);
            for (auto &rm : retained_list)
            {
                // 属性要还原成「出站 PUBLISH 属性」（剩余寿命递减 + 丢弃方向相关字段），
                // 否则入库时存的 content_type / user_properties 等全部丢失
                //（to_publish_props 此前一直是死代码，见 P1-4）。
                // 返回 false = 该条滞留期间已过 Message Expiry → 不再投递。
                mqtt_publish_props rprops;
                if (!rm.to_publish_props(rprops, now_sec))
                    continue;
                sess->deliver(rm.topic, rm.payload, std::min<uint8_t>(rm.qos, granted_qos), true, rprops);
            }
        }
    }

    sess->write(make_suback(packet_id, codes));
    co_return;
}

// PUBLISH: 业务回调 + QoS 流程 + broker 转发
asio::awaitable<void> httpserver::handle_mqtt_publish(std::shared_ptr<http::mqtt_session> sess,
                                                      std::shared_ptr<http::mqtt_api> handler,
                                                      const http::mqtt_session::packet &pkt,
                                                      std::function<void(http::mqtt_reason)> drop)
{
    using namespace http;

    mqtt_publish_info pub;
    if (!parse_publish(pkt.fixed, pkt.body_data(), pkt.body_size(), pub))
    {
        drop(mqtt_reason::malformed_packet);
        co_return;
    }

    // Topic Alias 解析
    if (!sess->resolve_inbound_alias(pub))
    {
        drop(mqtt_reason::topic_alias_invalid);
        co_return;
    }
    if (!is_valid_topic_name(pub.topic))
    {
        drop(mqtt_reason::topic_name_invalid);
        co_return;
    }

    bool allowed = handler->is_coroutine() ? co_await co_async_hook_bool("mqtt async_on_can_publish", handler->async_on_can_publish(pub.topic, *pub.payload, pub.qos), sess->transport()) : co_await co_sync_hook_bool("mqtt on_can_publish", std::function<bool()>([h = handler, t = pub.topic, p = std::string(*pub.payload), q = pub.qos]
                                                                                                                                                                                                                                                                    { return h->on_can_publish(t, p, q); }),
                                                                                                                                                                                                                       sess->transport());
    if (!allowed)
    {
        // QoS 握手照常回完，只是不转发也不写 retained：不回 ACK 只会让客户端一直重发
        if (pub.qos == 1)
            sess->write(make_puback(pub.packet_id, mqtt_reason::not_authorized));
        else if (pub.qos == 2)
            sess->write(make_pubrec(pub.packet_id, mqtt_reason::not_authorized));
        co_return;
    }

    if (handler->is_coroutine())
    {
        co_await handler->async_on_message(pub.topic, *pub.payload, pub.qos);
    }
    else
    {
        // await：转发与 ACK 排在这一步之后，投完就走会让 echo 落后于原消息
        co_await co_sync_hook_void(
            "mqtt on_message",
            std::function<void()>([h = handler,
                                   t = pub.topic,
                                   p = std::string(*pub.payload),
                                   q = pub.qos]
                                  { h->on_message(t, p, q); }),
            sess->transport());
    }

    if (pub.qos == 0)
    {
        // QoS 0 无 ACK：retained 落库结果无处可报，由 publish_impl 内部处理
        mqtt_api::publish_to_broker(pub, sess);
    }
    else if (pub.qos == 1)
    {
        // PUBACK 放在转发之后：retained 落库被配额拒绝时回 0x97（此前是静默丢弃却回 success）
        bool applied = mqtt_api::publish_to_broker(pub, sess);
        sess->write(make_puback(pub.packet_id, applied ? mqtt_reason::success : mqtt_reason::quota_exceeded));
    }
    else// QoS 2 等 PUBREL 后转发（含 retained 落库）
    {
        if (!sess->store_inbound_qos2(pub.packet_id, pub))
        {
            sess->write(make_pubrec(pub.packet_id, mqtt_reason::quota_exceeded));
            co_return;
        }
        sess->write(make_pubrec(pub.packet_id, mqtt_reason::success));
    }
    // retained 落库统一由 publish_impl 完成：QoS0/1 在上面这次转发里，
    // QoS2 在 PUBREL 的最终投递里。原先这里再写一次会造成「QoS2 在 PUBLISH 阶段
    // 就提前落库 + 双重写入」。
    co_return;
}

asio::awaitable<void> httpserver::clientpeerfun(std::shared_ptr<client_session> peer_session, bool isssl)
{
    try
    {
        if (isssl == false)
        {
            total_http1_count--;
        }

        total_count++;
        if (check_blockip(peer_session))
        {
            co_return;
        }

        if (peer_session->_cache_data == nullptr)
        {
            co_return;
        }
#ifndef BENCHMARK
        atomic_count live_count_guard(live_link_count);
#endif
        unsigned int readnum = 0, client_type_num = 0;

        bool is_error = co_await peer_session->read_first(readnum);
        if (is_error)
        {
            log_client_first_error(peer_session, readnum);
            co_return;
        }

        // 协议判定至少需要 6 字节（checkhttp2 读 _cache_data[0..5]）。
        // 原为 readnum > 6，恰好 6 字节时会被误判成 HTTP/1 错误请求并回 400。
        if (readnum >= 6)
        {
            client_type_num = checkhttp2(peer_session);

            if (client_type_num == 1)
            {
                client_type_num = co_await client_http1_loop(isssl, readnum, peer_session);
                if (client_type_num > 9)
                {
                    // h2c 升级：http1_loop 已回 101，此处 client_type_num 是
                    // make_h2c_header 写出的 stream 1 首帧长度（合法 15~398，
                    // 返回 0 表示构造失败，已被 >9 的门槛排除）。
                    // 门槛不能写成 >256：URL/Host/UA 较长的合法升级会被误杀。
                    std::string request_content;
                    request_content.resize(client_type_num);

                    for (size_t i = 0; i < client_type_num; i++)
                    {
                        request_content[i] = peer_session->_cache_data[i];
                    }

                    readnum = 0;
                    co_await client_h2_enter(peer_session, readnum, request_content);
                    co_return;
                }
                else if (client_type_num == 3)
                {
                    co_return;
                }
            }
            else if (client_type_num == 2)
            {
                // 明文 H2C 直连（PRI * HTTP/2.0 前言）或 TLS ALPN 协商出的 h2。
                // H2C cleartext (PRI * HTTP/2.0 preface) or h2 via TLS ALPN.
                // 与 h2c 共用同一入口，前言校验与补读都在里面完成
                DEBUG_LOG("peer_session co_send_setting");
                co_await client_h2_enter(peer_session, readnum, std::string_view{});
                co_return;
            }
            else if (client_type_num == 7)
            {
                co_await client_rpc_loop(readnum, peer_session);
                co_return;
            }
            else if (client_type_num == 8)
            {
                co_await client_tcp_loop(readnum, peer_session);
                co_return;
            }
            else if (client_type_num == 9)
            {
                co_await client_mqtt_loop(readnum, peer_session);
                co_return;
            }
        }
        else if (readnum > 0)
        {
            // 首包不足 6 字节时不能静默断连（TCP 拆包会把合法请求切成小段），
            // 明确回 400 后关闭连接。
            co_await http1_send_bad_request(400, peer_session);
        }
        co_return;
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("client exit exception");
        peer_session->stop();
    }
    catch (...)
    {
        peer_session->stop();
    }
    co_return;
}

asio::awaitable<void> httpserver::http2_send_sequence_header(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t> sq_obj)
{
    peer->status(200);
    peer->length(sq_obj->content_length);
    peer->type(sq_obj->content_type);
    if (peer->compress == 0)
    {
        peer->set_header("accept-ranges", "bytes");
    }

    peer->set_header("date", get_gmttime());
    peer->set_header("last-modified", get_gmttime(peer->fileinfo.st_mtime));

    peer->set_header("etag", sq_obj->etag);

    sq_obj->header        = peer->make_http2_header(0);
    sq_obj->is_sendheader = false;

    co_return;
}

unsigned long long httpserver::http2_reserve_send_window(client_session *session_obj, unsigned int stream_id, unsigned long long want)
{
    if (want == 0)
    {
        return 0;
    }
    // 连接级：window_update_num = 累计授予，has_send_update_num = 累计已发。
    // Connection-level: window_update_num = cumulative granted; has_send_update_num = cumulative sent.
    // 剩余额 = 两者之差；预留必须对 has_send_update_num 做 CAS。两个发送线程共用
    // Headroom = granted - sent; reserves CAS has_send_update_num. Two send threads share
    // 同一条连接时，先读后加会双双放行再合计超发。
    // the same connection — read-then-add races would both pass, then collectively over-send.
    unsigned long long granted   = session_obj->window_update_num.load();
    unsigned long long sent      = session_obj->has_send_update_num.load();
    unsigned long long conn_take = want;
    for (;;)
    {
        unsigned long long avail = (granted > sent) ? (granted - sent) : 0;
        if (avail == 0)
        {
            return 0;
        }
        if (conn_take > avail)
        {
            conn_take = avail;
        }
        if (session_obj->has_send_update_num.compare_exchange_weak(sent, sent + conn_take))
        {
            break;
        }
        // 失败可能是另一线程发走了额度，或退还了额度；granted 也可能刚被
        // Failure: other thread consumed or returned quota; granted may also just have been
        // WINDOW_UPDATE 抬升。两个值都要重取。
        // raised by WINDOW_UPDATE. Re-read both values.
        granted = session_obj->window_update_num.load();
    }

    unsigned long long got = conn_take;
    {
        std::lock_guard<std::mutex> lk(session_obj->stream_send_window_mutex);
        auto it = session_obj->stream_send_window.find(stream_id);
        if (it == session_obj->stream_send_window.end())
        {
            it = session_obj->stream_send_window
                     .emplace(stream_id, session_obj->remote_initial_window_size.load())
                     .first;
        }
        unsigned int avail = it->second;
        if (avail == 0)
        {
            got = 0;
        }
        else if (avail < conn_take)
        {
            got        = avail;
            it->second = 0;
        }
        else
        {
            it->second -= static_cast<unsigned int>(conn_take);
        }
    }
    if (got < conn_take)
    {
        // 流级见底：连接级那笔（整笔或差额）必须退回，否则窗口单向流失。
        // Stream-level depleted: must return the connection-level chunk (full or partial),
        // otherwise window drains one-way.
        session_obj->has_send_update_num.fetch_sub(conn_take - got, std::memory_order_acq_rel);
    }
    return got;
}

void httpserver::http2_refund_send_window(client_session *session_obj, unsigned int stream_id, unsigned long long back)
{
    if (back == 0)
    {
        return;
    }
    session_obj->has_send_update_num.fetch_sub(back, std::memory_order_acq_rel);

    {
        std::lock_guard<std::mutex> lk(session_obj->stream_send_window_mutex);
        auto it = session_obj->stream_send_window.find(stream_id);
        if (it == session_obj->stream_send_window.end())
        {
            // 条目已被并发 RST 或流结束回收。那条流不会再发 DATA，新建条目等于把回收掉
            // Entry already reaped by concurrent RST or stream end. That stream sends no more
            // 的账重新撑回来——对端可任意挑流 id，无界增长就是这么来的，直接放弃。
            // DATA; creating a fresh entry reinflates reaped debt — peer can pick any stream id,
            // unbounded growth ensues. Just drop this reservation.
            return;
        }
        // 全 unsigned 域：cur + back 用 unsigned long long 中间态绕开 32-bit wrap，
        // 加完 clamp 回 RFC MAX_WINDOW。
        // Fully unsigned: cur + back in unsigned long long to dodge 32-bit wrap,
        // then clamp to RFC MAX_WINDOW.
        unsigned long long restored =
            static_cast<unsigned long long>(it->second) + back;
        if (restored > static_cast<unsigned long long>(CONST_HTTP2_MAX_WINDOW))
        {
            restored = CONST_HTTP2_MAX_WINDOW;
        }
        it->second = static_cast<unsigned int>(restored);
    }
    // 事件边：退还额度时，连接级那笔可能正好松开另一条挂起流（剩余额 =
    // Event edge: returned quota may unblock another parked stream (headroom =
    // 累计授予 - 累计已发，已发变小 = 余额变大）。抢锁前先原子读，没人挂起就什么都不做。
    // cumulative granted - cumulative sent; sent goes down → headroom goes up).
    // Atomic-read before locking; nothing to do if nobody parked.
    if (get_http2_send_queue().parked.load() > 0)
    {
        requeue_parked(*session_obj);
    }
}

// 发送对象被闸门挡住时唯一去处：挂起，不留在发送线程链表里逐 tick 复查。
// When gated out, a send object must park — not stay in the send-thread list for tick-by-tick recheck.
// 返回 false 表示本连接已冲刷（关闭路径先立闸门），调用方必须自己回池。
// Returns false if this connection was already flushed (close-path gates first); caller must recycle.
bool httpserver::http2_park_send(std::shared_ptr<http2_send_data_t> &sp, unsigned char reason)
{
    if (!sp || !sp->peer || !sp->peer->socket_session)
    {
        return false;
    }
    client_session *session_obj = sp->peer->socket_session.get();
    if (session_obj->send_park_closed.load())
    {
        return false;
    }
    // 挂起对象无人逐轮刷新 time_limit，idle 清理会把它当空闲回收；
    // Parked objects get no per-round time_limit refresh; idle sweep would reap them as idle.
    // 大文件下载挂几分钟是正常状态，这里补一次，之后由兜底扫描续期。
    // Slow clients with large downloads parked for minutes are normal — refresh here once,
    // then the belt sweep keeps renewing.
    session_obj->time_limit.store(timeid());
    get_http2_send_queue().park(sp, reason);
    return true;
}

// 回灌闸门必须与实际发送处逐条对齐，否则挂起/回灌互相空转，
// Belt-recompute gates must align one-to-one with actual send gates, otherwise park/refill spin —
// 所以下面发送循环的前置判断也走这一个函数。why：挡住时写回原因（1=窗口 2=环）。
// that is why the send loop also gates through this same function. Reason code on block: 1=window, 2=ring.
static bool http2_send_gate_open(const std::shared_ptr<http2_send_data_t> &sp, unsigned char *why = nullptr)
{
    if (!sp->peer || !sp->peer->socket_session)
    {
        return true;
    }
    httppeer *peer              = sp->peer.get();
    client_session *session_obj = peer->socket_session.get();
    if (peer->isclose || peer->issend || session_obj->isclose)
    {
        // 已收尾的流直接放回队列，让发送线程走回收分支（回池 + 清窗口记账）
        // Reaped stream → straight back to queue, let the send thread do its own recycle path (pool + window bookkeeping).
        return true;
    }
    if (session_obj->window_update_num.load() <= session_obj->has_send_update_num.load())
    {
        if (why)
            *why = 1;
        return false;
    }
    {
        std::lock_guard<std::mutex> lk(session_obj->stream_send_window_mutex);
        auto it = session_obj->stream_send_window.find(peer->stream_id);
        if (it == session_obj->stream_send_window.end())
        {
            // 与 http2_reserve_send_window 懒初始化同口径：无条目就按对端
            // Same policy as http2_reserve_send_window lazy init: no entry → use peer's
            // SETTINGS_INITIAL_WINDOW_SIZE 起算。只读不写，陌生流不能凭回灌进表。
            // SETTINGS_INITIAL_WINDOW_SIZE. Read-only here; unknown streams must not be inserted by belt.
            if (session_obj->remote_initial_window_size.load() == 0)
            {
                if (why)
                    *why = 1;
                return false;
            }
        }
        else if (it->second == 0)
        {
            if (why)
                *why = 1;
            return false;
        }
    }
    if (session_obj->http2_ring_queue &&
        session_obj->http2_ring_queue->has_size() > CONST_HTTP2_RING_BACKPRESSURE_SLOTS)
    {
        // 让路计数挂在闸门这一支：主循环里闸门先于发送序列判同一条阈值，
        // Yield count lives on the gate path: main loop gates before sequence checks the same threshold,
        // 序列内部那道检查是第二道闸（并发 push 让环在两道闸之间又涨时才走到）。
        // sequence check is the 2nd gate (only reached when concurrent push refills the ring between gates).
        // 只在序列里计数的话，环积压记不到，看起来像从没让过路。
        // Counting only in sequence misses ring backlog entirely — runtime looks like it never yields.
        session_obj->http2_ring_backpressure_count++;
        if (why)
            *why = 2;
        return false;
    }
    return true;
}

// 挂起对象不被逐轮复查，所以"多久没推进就该撤流"这条死线只能由回灌侧量。
// Parked objects get no per-round recheck, so the stall-deadline is measured from the belt side.
// 撤流口径与序列内部一致：只 RST 这一条流，不带走连接。
// Same RST policy as sequence: RST_STREAM only, connection stays.
static bool http2_send_progress_expired(const std::shared_ptr<http2_send_data_t> &sp)
{
    if (!sp || !sp->peer || !sp->peer->socket_session)
    {
        return false;
    }
    long long stalled_sec = std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::steady_clock::now() - sp->last_progress_time)
                                .count();
    return stalled_sec > CONST_HTTP2_SEND_NO_PROGRESS_TIMEOUT;
}

// 摘出的挂起对象按闸门分流：够格的移进 sent_data_list 并唤醒发送线程，
// Drained parked batch splits by gate: eligible → sent_data_list + notify send thread;
// 不够格的留在 in 里，由调用方挂回表。挂起对象没人刷 time_limit，
// not eligible → leave in "in", caller re-parks them. Parked objects get no time_limit refresh,
// 这里顺手续期，别让 idle 清理把慢下载回收掉。返回入队条数。
// so we refresh here to prevent idle sweep from reaping slow downloads. Returns count enqueued.
unsigned int httpserver::dispatch_parked(std::list<std::shared_ptr<http2_send_data_t>> &in)
{
    std::list<std::shared_ptr<http2_send_data_t>> ready;
    std::list<std::shared_ptr<http2_send_data_t>> dead;
    for (auto iter = in.begin(); iter != in.end();)
    {
        if (http2_send_gate_open(*iter))
        {
            ready.splice(ready.end(), in, iter++);
        }
        else if (http2_send_progress_expired(*iter))
        {
            dead.splice(dead.end(), in, iter++);
        }
        else
        {
            if (*iter && (*iter)->peer && (*iter)->peer->socket_session)
            {
                (*iter)->peer->socket_session->time_limit.store(timeid());
            }
            ++iter;
        }
    }
    // 到点的流在这里收口：撤流 + 清该流发送窗口记账 + 回池。少了这一步，
    // Expired streams close out here: RST + clear stream window bookkeeping + pool return.
    // 永久停读的客户端会永远躺在挂起表里（没人再跑它那一轮，死线没人判）。
    // Without this, a permanently-stop-reading client lives forever in the parked table —
    // nobody runs its round anymore → deadline never checked.
    for (auto &sp : dead)
    {
        if (sp && sp->peer && sp->peer->socket_session)
        {
            LOG_ERROR << " http2 send no progress while parked, rst stream, stream_id:"
                      << sp->peer->stream_id << " current:" << sp->current_num << "/"
                      << sp->content_length << " url:" << sp->peer->url << LOG_END;
            sp->peer->socket_session->http2_send_rst_stream(sp->peer->stream_id,
                                                            CONST_HTTP2_STREAM_ERROR_CANCEL);
            if (sp->peer->socket_session->http2_need_wakeup)
            {
                sp->peer->socket_session->waituphttp2();
            }
            sp->peer->issend = true;
            {
                std::lock_guard<std::mutex> lk(sp->peer->socket_session->stream_send_window_mutex);
                sp->peer->socket_session->stream_send_window.erase(sp->peer->stream_id);
            }
        }
        get_http2_send_queue().back_cache_ptr(sp);
    }
    if (ready.empty())
    {
        return 0;
    }
    unsigned int requeued = static_cast<unsigned int>(ready.size());
    {
        std::unique_lock<std::mutex> lock(send_data_mutex);
        for (auto &sp : ready)
        {
            sp->own_state = 1;
        }
        sent_data_list.splice(sent_data_list.end(), ready);
    }
    // 唤醒全部：requeue_parked / requeue_stuck_parked 可能一次回灌数十条 ready 对象，
    // 若两条发送线程此刻都在 wait_for 里（全部 parked 时常见），notify_one 只醒 1 条，
    // 另一条要等 CONST_HTTP2_BELT_SWEEP_SECONDS(6s) 超时自然醒，形成毛刺。
    // Wake all: requeue_parked / requeue_stuck_parked may inject tens of ready objects at once;
    // if both send threads are in wait_for (common when everything is parked), notify_one wakes only
    // one — the other waits up to CONST_HTTP2_BELT_SWEEP_SECONDS(6s) timeout, a visible stall.
    send_data_condition.notify_all();
    return requeued;
}

void httpserver::requeue_parked(client_session &session_obj)
{
    http2_send_queue &send_queue_obj = get_http2_send_queue();
    if (send_queue_obj.parked.load() == 0)
    {
        return;
    }
    std::list<std::shared_ptr<http2_send_data_t>> drained;
    if (!send_queue_obj.detach_parked(&session_obj, drained))
    {
        return;
    }
    // 摘出后本批归本函数独占（own_state 仍为 3，别人摸不到），锁外重算闸门，
    // After drain, this batch is ours exclusively (own_state stays 3, nobody else touches).
    // Recompute gates outside the lock —
    // 避免持 parked_mutex 再去抢 stream_send_window_mutex / send_data_mutex。
    // don't hold parked_mutex while acquiring stream_send_window_mutex / send_data_mutex.
    dispatch_parked(drained);
    send_queue_obj.reattach_parked(drained);
}

void httpserver::requeue_stuck_parked()
{
    http2_send_queue &send_queue_obj = get_http2_send_queue();

    // 一拍只有一个赢家：两条发送线程都会走到这里，未到期那一次
    // One winner per sweep beat: both send threads reach here (with or without work);
    // the not-due one is just an atomic read + failed CAS.
    // 就是一次原子读加一次失败的 CAS。下面三个静态量只允许赢家抵达，靠 acq_rel CAS
    // Only the winner touches the three statics; acq_rel CAS establishes happens-before
    // 与上一拍自己建立 happens-before；挪到 CAS 之前或之外访问就是数据竞争。
    // with the previous beat's self. Accessing them before or outside the CAS is a data race.
    // 这条 CAS 管"谁代表这一拍"，下面那把忙标志管"上一拍结束了没有"。
    // This CAS picks "who represents this beat"; the busy flag below tracks "previous beat finished".
    const unsigned long long this_second = (unsigned long long)timeid();
    unsigned long long prev_second       = send_queue_obj.belt_sweep_second.load(std::memory_order_acquire);
    do
    {
        if (this_second < prev_second + CONST_HTTP2_BELT_SWEEP_SECONDS)
        {
            return;
        }
    } while (!send_queue_obj.belt_sweep_second.compare_exchange_weak(prev_second, this_second, std::memory_order_acq_rel, std::memory_order_acquire));
    // 拍与拍之间还要一把"只试不等"的互斥：上一拍还在跑（摘全表 + 逐条重算闸门，表长
    // Try-lock mutex between beats: previous beat still running (full-table drain + per-entry
    // gate recompute — table long or ASan-slowed can stretch past one beat),
    // 或 ASan 拖慢能跑过一拍），下面那三个静态量就有第二个写者。抢不到就跳过这一拍，
    // three statics would have a second writer. Skip this beat on contention —
    // 兜底本来就允许某一拍被漏掉——事件边才是正常路径。
    // belt sweep is allowed to miss beats; event edges are the normal path.
    unsigned char expect_idle = 0;
    if (!send_queue_obj.belt_sweep_busy.compare_exchange_strong(expect_idle, 1, std::memory_order_acq_rel, std::memory_order_acquire))
    {
        return;
    }

    unsigned int now_parked      = send_queue_obj.parked.load();
    unsigned int now_outstanding = send_queue_obj.outstanding.load();

    // 排水边沿：上一拍还有对象"取出未还"或"停在挂起表"，这一拍两头同时归零就记一行。
    // Drain edge: if previous beat had objects "out-not-returned" or "still parked", and this beat
    // both counters hit zero — log a line.
    // 排查泄漏只能读这一行——两头都为 0 时本函数本来什么都不写，沉默不能被读成排干。
    // Leak diagnosis reads this line only — when both are 0 we'd write nothing anyway;
    // silence must not be mistaken for "drained".
    static unsigned int last_parked      = 0;
    static unsigned int last_outstanding = 0;
    static unsigned int stuck_sweeps     = 0;
    if ((last_parked + last_outstanding > 0) && (now_parked + now_outstanding == 0))
    {
        std::string log_item;
        log_item.append("http2 send queue drained parked 0 outstanding 0");
        log_item.append(" was parked ");
        log_item.append(std::to_string(last_parked));
        log_item.append(" outstanding ");
        log_item.append(std::to_string(last_outstanding));
        log_item.append(" parks ");
        log_item.append(std::to_string(send_queue_obj.park_total.load(std::memory_order_relaxed)));
        log_item.append(" extra_feed ");
        log_item.append(std::to_string(send_queue_obj.extra_feed_total.load(std::memory_order_relaxed)));
        log_item.push_back('\n');
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back(log_item);
        lock.unlock();
    }
    last_parked      = now_parked;
    last_outstanding = now_outstanding;

    if (now_parked == 0)
    {
        // 表空时这一拍开销到此：两条 CAS（未到期的那条直接返回）+ 两次原子读
        // Empty table — beat's cost ends here: two CAS (not-due one returns directly) + two atomic reads.
        stuck_sweeps = 0;
    }
    else
    {
        // 表本身就是名册：不必遍历 socket_session_lists，摘全表逐条重算即可。
        // The parked table is the roster — no need to walk socket_session_lists; drain all and recompute each entry.
        // 这一趟是为"事件边一条都没等到"收口——正常路径下闸门一抬开就有边来回灌。
        // This sweep is the catch-all for "no event edges fired" — normally gates lift and edges refill.
        std::list<std::shared_ptr<http2_send_data_t>> drained;
        send_queue_obj.detach_parked(nullptr, drained);
        dispatch_parked(drained);
        send_queue_obj.reattach_parked(drained);

        // 扫完仍留在表里 = 闸门还是关着。连着十拍都这样就报一行，让"有人永远回不了灌"
        // Still in table after sweep = gates still closed. Ten consecutive beats like this →
        // 变成一个读得到的数；没有这一行，"没泄漏" 只能靠沉默来证明。一拍是
        // log a readable count. Without it, "no leak" can only be proved by silence.
        // CONST_HTTP2_BELT_SWEEP_SECONDS 秒，所以单位报"拍"而不是秒。
        // One beat = CONST_HTTP2_BELT_SWEEP_SECONDS — so unit is "beats", not seconds.
        unsigned int still_parked = send_queue_obj.parked.load();
        if (still_parked == 0)
        {
            stuck_sweeps = 0;
        }
        else if (++stuck_sweeps % 10 == 0)
        {
            std::string log_item;
            log_item.append("http2 send queue parked ");
            log_item.append(std::to_string(still_parked));
            log_item.append(" outstanding ");
            log_item.append(std::to_string(send_queue_obj.outstanding.load()));
            log_item.append(" stuck ");
            log_item.append(std::to_string(stuck_sweeps));
            log_item.append(" sweeps\n");
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(log_item);
            lock.unlock();
        }
    }
    // 只有一个出口：忙标志的释放跟着这条尾走，中途新增的 return 不会把它漏在 1 上。
    // Single exit point: busy-flag release follows this tail; any mid-function return
    // must not leave it stuck at 1.
    send_queue_obj.belt_sweep_busy.store(0, std::memory_order_release);
}

bool httpserver::http2_loop_send_sequence(std::shared_ptr<http2_send_data_t> sq_obj)
{
    if (!sq_obj->peer)
    {
        return false;
    }
    std::shared_ptr<httppeer> peer = sq_obj->peer;

    if (sq_obj->peer->socket_session.use_count() == 0)
    {
        peer->issend = true;
        return false;
    }
    if (sq_obj->peer->isclose)
    {
        return false;
    }
    // 大文件下载时客户端（如 Safari）可能因磁盘写入停顿导致 TCP 窗口短暂关闭，
    // Large file downloads: client (e.g. Safari) may stall on disk writes → TCP window closes briefly,
    // 发送环会积压。阈值给慢客户端留缓冲，同时留槽位给控制帧收尾。
    // ring backlogs. Thresholds buffer slow clients while reserving slots for control frame tail.
    if (peer->socket_session->http2_ring_queue->has_size() > CONST_HTTP2_RING_BACKPRESSURE_SLOTS)
    {
        peer->socket_session->http2_ring_backpressure_count++;

        // 撤流条件是"多久没推进"，不是"让路让了多久"：把背压时间一路累加、
        // Stream RST condition is "no progress for X", not "yielded for X" — accumulating
        // 加过门槛就 RST_STREAM，等于用"客户端还在收、只是收得慢"这个正常状态
        // backpressure time past a threshold to RST_STREAM treats a healthy-slow client
        // 去判死一条流。只要偏移还在动，多久都不撤；
        // as dead. As long as offset advances, never RST —
        // 连续 CONST_HTTP2_SEND_NO_PROGRESS_TIMEOUT 秒一个字节都没发出去才撤，
        // only RST when CONST_HTTP2_SEND_NO_PROGRESS_TIMEOUT seconds pass with zero bytes out.
        // 而且只撤这条流，连接留给别的流继续用。
        // RST_STREAM only; connection stays for other streams.
        long long stalled_sec = std::chrono::duration_cast<std::chrono::seconds>(
                                    std::chrono::steady_clock::now() - sq_obj->last_progress_time)
                                    .count();
        if (stalled_sec > CONST_HTTP2_SEND_NO_PROGRESS_TIMEOUT)
        {
            LOG_ERROR << " http2 send no progress, rst stream, stream_id:" << peer->stream_id
                      << " stalled_sec:" << stalled_sec
                      << " current:" << sq_obj->current_num << "/" << sq_obj->content_length
                      << " url:" << peer->url << LOG_END;
            peer->socket_session->http2_send_rst_stream(peer->stream_id, CONST_HTTP2_STREAM_ERROR_CANCEL);
            if (peer->socket_session->http2_need_wakeup)
            {
                peer->socket_session->waituphttp2();
            }
            peer->issend = true;
            return false;
        }

        // 环满让路：挂起，等 ring_client_server 腾槽位的事件边回灌。
        // Ring full → park, wait for ring_client_server slot-freed event edge to refill.
        sq_obj->standby_next = true;

        if (peer->socket_session->http2_need_wakeup)
        {
            peer->socket_session->waituphttp2();
        }
        sq_obj->block_reason = 2;
        return true;
    }
    if (sq_obj->is_sendheader == false)
    {
        // 环满时 push() 返回 false（16 槽环永远留一个空槽，实际可用 15 槽）。
        // push() returns false when ring full (16-slot ring keeps one empty, 15 usable).
        // 此时绝不能置 is_sendheader：头部帧没入队却标记为"已发"，客户端会先
        // MUST NOT set is_sendheader here: header frame didn't enqueue but marked "sent" —
        // 收到 DATA 却永远等不到 HEADERS，只能断开连接。
        // client receives DATA first, waits forever for HEADERS, disconnects.
        if (!peer->socket_session->http2_ring_queue->push(sq_obj->header))
        {
            peer->socket_session->http2_ring_overflow_count++;
            DEBUG_LOG("http2 ring full, headers not queued, retry later");
            sq_obj->standby_next = true;
            if (peer->socket_session->http2_need_wakeup)
            {
                peer->socket_session->waituphttp2();
            }
            sq_obj->block_reason = 2;
            return true;
        }
        sq_obj->is_sendheader = true;
        sq_obj->standby_next  = true;
        if (peer->socket_session->http2_need_wakeup)
        {
            peer->socket_session->waituphttp2();
        }
        if (sq_obj->only_send_header)
        {
            // has end stream
            peer->issend = true;
            return true;
        }
    }

    if (sq_obj->content_length == 0)
    {
        if (!peer->socket_session->send_zero_data(peer->stream_id))
        {
            // 环满，空 DATA + END_STREAM 没能入队。同上面几处：置了 issend 就等于
            // Ring full, empty DATA + END_STREAM didn't enqueue. Same pattern as above:
            // 让外层回收这条流，而客户端还在等它的收尾帧。
            // setting issend lets outer code recycle the stream while client still waits for its tail frame.
            sq_obj->standby_next = true;
            if (peer->socket_session->http2_need_wakeup)
            {
                peer->socket_session->waituphttp2();
            }
            sq_obj->block_reason = 2;
            return true;
        }
        peer->issend         = true;
        sq_obj->standby_next = true;
        if (peer->socket_session->http2_need_wakeup)
        {
            peer->socket_session->waituphttp2();
        }
        return false;
    }
    unsigned int data_send_id   = peer->stream_id;
    unsigned long long per_size = 0;

    // 先判"还有没有内容要发"，再算额度、开缓冲、读盘：预留之后每个不走
    // First check "anything left to send?", then reserve quota, alloc buffer, fread — every exit
    // fread/push 的出口都得退还额度；把无内容出口挡在预留之前既少两处退还，
    // that skips fread/push must return quota. Blocking empty-content exits before reserve saves
    // 也避免 content_length - current_num 下溢。
    // two return paths and avoids content_length - current_num underflow.
    if (sq_obj->type == 1)
    {
        if (sq_obj->current_num >= sq_obj->content_length)
        {
            // 数据已发完（END_STREAM 上一轮已置位）。不推任何帧，也不推进
            // All data sent (END_STREAM already set last round). No frame pushed, no progress —
            // 进度（推空帧会让 current_num 永远不增长而死循环）。
            // pushing empty DATA would freeze current_num → infinite loop.
            sq_obj->standby_next = true;
            return false;
        }
        if (!sq_obj->fp.get())
        {
            if (!peer->socket_session->http2_send_enddata(peer->stream_id))
            {
                // 环满，END_STREAM 没能入队。绝不能置 issend：置了之后外层回收分支
                // Ring full, END_STREAM didn't enqueue. MUST NOT set issend — outer recycle path
                // 会在环排空后把对象回收，这条流永远等不到收尾帧。
                // reclaims the object after ring drains; stream waits forever for tail frame.
                sq_obj->standby_next = true;
                if (peer->socket_session->http2_need_wakeup)
                {
                    peer->socket_session->waituphttp2();
                }
                sq_obj->block_reason = 2;
                return true;
            }
            peer->issend         = true;
            sq_obj->standby_next = true;
            if (peer->socket_session->http2_need_wakeup)
            {
                peer->socket_session->waituphttp2();
            }
            return false;
        }
    }
    else if (sq_obj->current_num >= sq_obj->content.size())
    {
        // 同上：数据已发完，收尾即可，避免空 DATA 帧死循环。
        // Same as above: all data out, just finish — avoid empty DATA infinite loop.
        sq_obj->standby_next = true;
        return false;
    }

    // 本帧计划量：载荷上限取 min(对端 SETTINGS_MAX_FRAME_SIZE, 本端帧缓冲 - 9 字节帧头)
    // Planned payload per frame: min(peer SETTINGS_MAX_FRAME_SIZE, local frame buffer - 9-byte header),
    // 再按剩余内容夹一次。分片大小只由额度和对端能力决定，
    // then clamped to remaining content. Chunk size is driven solely by quota and peer capability —
    // 不再有按 content_length / 累计额度排出来的分档。
    // no more tiers derived from content_length or cumulative quota.
    unsigned long long frame_cap = std::min<unsigned long long>(
        peer->socket_session->remote_max_frame_size.load(),
        CONST_HTTP2_SEND_FRAME_BUF - 9);
    unsigned long long remain_src = 0;
    if (sq_obj->type == 1)
    {
        remain_src = sq_obj->content_length - sq_obj->current_num;
    }
    else
    {
        remain_src = sq_obj->content.size() - sq_obj->current_num;
    }
    unsigned long long planned = std::min(remain_src, frame_cap);

    unsigned long long reserved = http2_reserve_send_window(peer->socket_session.get(), peer->stream_id, planned);
    if (reserved == 0)
    {
        // 窗口见底：本轮让路，等内容侧 WINDOW_UPDATE。偏移未动，下一轮幂等重发。
        // Window empty: yield this round, wait for content-side WINDOW_UPDATE. Offset unchanged,
        // next round retries idempotently.
        sq_obj->standby_next = true;
        if (peer->socket_session->http2_need_wakeup)
        {
            peer->socket_session->waituphttp2();
        }
        sq_obj->block_reason = 1;
        return true;
    }

    // 缓冲按实际预留量开：余额不足计划量时缩帧发，而不是整轮弃发。
    // Buffer sized by actual reserved quota: shrink frame when balance < planned, don't drop whole round.
    // 硬约束"读盘量 ≤ 已预留额度"在这里由缓冲大小直接保证。
    // Hard invariant "bytes_read ≤ reserved quota" enforced directly by buffer size here.
    sq_obj->cache_data.clear();
    sq_obj->cache_data.resize(9 + reserved);
    sq_obj->cache_data[3] = 0x00;
    sq_obj->cache_data[4] = 0x00;
    data_send_id          = peer->stream_id;
    sq_obj->cache_data[8] = data_send_id & 0xFF;
    data_send_id          = data_send_id >> 8;
    sq_obj->cache_data[7] = data_send_id & 0xFF;
    data_send_id          = data_send_id >> 8;
    sq_obj->cache_data[6] = data_send_id & 0xFF;
    data_send_id          = data_send_id >> 8;
    sq_obj->cache_data[5] = data_send_id & 0x7F;

    if (sq_obj->type == 1)
    {
        // 文件偏移一律由 current_num 单方面决定：让路/回滚分支不可能留下偏移漂移。
        // File offset is solely driven by current_num — yield/rollback paths can't cause drift.
        // Range 请求的 current_num 初值就是绝对偏移 rangebegin，非 Range 为 0。
        // Range request: initial current_num = absolute offset rangebegin; non-Range = 0.
        if (0 != fseek(sq_obj->fp.get(), (long)sq_obj->current_num, SEEK_SET))
        {
            LOG_ERROR << " http2 send file seek error, stream_id:" << peer->stream_id
                      << " offset:" << sq_obj->current_num
                      << " file:" << sq_obj->file_name << LOG_END;
            http2_refund_send_window(peer->socket_session.get(), peer->stream_id, reserved);
            if (!peer->socket_session->http2_send_enddata(peer->stream_id))
            {
                sq_obj->standby_next = true;
                if (peer->socket_session->http2_need_wakeup)
                {
                    peer->socket_session->waituphttp2();
                }
                sq_obj->block_reason = 2;
                return true;
            }
            peer->issend         = true;
            sq_obj->standby_next = true;
            if (peer->socket_session->http2_need_wakeup)
            {
                peer->socket_session->waituphttp2();
            }
            return false;
        }
        per_size = fread(&sq_obj->cache_data[9], 1, reserved, sq_obj->fp.get());
        if (per_size == 0)
        {
            // 区分"已经读到文件结尾"与"读盘失败 / 文件传输途中被截断"。
            // Distinguish "reached EOF" from "disk read failure / file truncated mid-transport".
            // 后者意味着实发字节 < 声明 Content-Length，客户端只能判错，
            // The latter means bytes_sent < declared Content-Length — client treats as error,
            // 必须留下日志而不是静默当成正常结束。
            // so we must log, not silently treat as normal end.
            if (sq_obj->current_num < sq_obj->content_length)
            {
                LOG_ERROR << " http2 send file read error, stream_id:" << peer->stream_id
                          << " current:" << sq_obj->current_num
                          << " content_length:" << sq_obj->content_length
                          << " file:" << sq_obj->file_name << LOG_END;
            }
            http2_refund_send_window(peer->socket_session.get(), peer->stream_id, reserved);
            if (!peer->socket_session->http2_send_enddata(peer->stream_id))
            {
                sq_obj->standby_next = true;
                if (peer->socket_session->http2_need_wakeup)
                {
                    peer->socket_session->waituphttp2();
                }
                sq_obj->block_reason = 2;
                return true;
            }
            peer->issend         = true;
            sq_obj->standby_next = true;
            if (peer->socket_session->http2_need_wakeup)
            {
                peer->socket_session->waituphttp2();
            }
            return false;
        }
        sq_obj->cache_data.resize(9 + per_size);
        if (per_size < reserved)
        {
            // 磁盘短读：没读满的额度要还回去，否则窗口单向流失。
            // Short disk read: return unconsumed quota, otherwise window drains one-way.
            http2_refund_send_window(peer->socket_session.get(), peer->stream_id, reserved - per_size);
        }
    }
    else
    {
        per_size = reserved;
        sq_obj->cache_data.resize(9);
        sq_obj->cache_data.append(&sq_obj->content[sq_obj->current_num], per_size);
    }

    sq_obj->current_num += per_size;
    if (sq_obj->current_num >= sq_obj->content_length)
    {
        sq_obj->cache_data[4] = 0x01;
    }

    data_send_id          = per_size;
    sq_obj->cache_data[2] = data_send_id & 0xFF;
    data_send_id          = data_send_id >> 8;
    sq_obj->cache_data[1] = data_send_id & 0xFF;
    data_send_id          = data_send_id >> 8;
    sq_obj->cache_data[0] = data_send_id & 0xFF;

    // 环满时 push() 返回 false。旧实现丢弃返回值并继续推进 current_num / issend /
    // push() returns false when ring full. Old code discarded the return and advanced current_num, issend,
    // 流控记账，等于这帧 DATA 凭空消失：响应缺洞、长度与 Content-Length 不符，
    // flow-control bookkeeping anyway — frame vanishes: response has holes, length mismatches,
    // 丢的若是最后一帧，END_STREAM 也一并丢失，客户端会一直等下去。
    // and if it's the last frame, END_STREAM is lost too — client waits forever.
    // 必须整体回退本轮进度，留到下一轮重发。
    // Must roll back this round entirely, retry next round.
    if (!peer->socket_session->http2_ring_queue->push(sq_obj->cache_data))
    {
        peer->socket_session->http2_ring_overflow_count++;
        DEBUG_LOG("http2 ring full, data frame not queued, retry later");
        // 本帧没真正入环，不能计入已消费发送窗口。
        // Frame didn't actually enter ring — do not count toward consumed send window.
        sq_obj->current_num   = sq_obj->current_num - per_size;
        sq_obj->cache_data[4] = 0x00;
        http2_refund_send_window(peer->socket_session.get(), peer->stream_id, per_size);
        // 下一轮会重建 cache_data，长度字段无需手工回退。
        // Next round rebuilds cache_data — length fields need no manual rollback.
        sq_obj->standby_next = true;
        if (peer->socket_session->http2_need_wakeup)
        {
            peer->socket_session->waituphttp2();
        }
        sq_obj->block_reason = 2;
        return true;
    }

    if (sq_obj->current_num >= sq_obj->content_length)
    {
        peer->issend = true;
    }
    if (peer->socket_session->http2_need_wakeup)
    {
        peer->socket_session->waituphttp2();
    }
    // 帧已入环，本轮进度"真的发出去了"：无进展死线的起点跟着前移。
    // Frame enqueued → progress is real: no-progress deadline advances from here.
    // 让路/回滚分支不刷新它，所以慢客户端持续收 = 持续刷新，不会误撤流。
    // Yield/rollback paths don't refresh it — slow client keeps receiving = keeps refreshing,
    // no false RST.
    sq_obj->last_progress_time = std::chrono::steady_clock::now();
    sq_obj->standby_next       = true;
    return true;
}

void httpserver::http2_send_queue_loop([[maybe_unused]] unsigned char index_id)
{
    DEBUG_LOG("http2_send_queue_loop");

    std::list<std::shared_ptr<http2_send_data_t>> thread_sent_data_list;
    unsigned int send_loop_count = 0;
    std::chrono::time_point<std::chrono::steady_clock> last_loop_time;
    std::string this_thread_tag = "++ http2_send_queue_loop " + std::to_string(index_id);
    this_thread_tag.append(" ++\n");
    while (true)
    {
        DEBUG_LOG("http2_send_queue_loop begin");
        try
        {
            last_loop_time = std::chrono::steady_clock::now();
            std::unique_lock lk(send_data_mutex);
            // 谓词必须包含 isstop：否则 stop()/~httpserver 的 notify_all() 会被判假丢弃，
            // Predicate must include isstop — otherwise notify_all() from stop()/~httpserver
            // 线程重新求值后回睡，run() 与析构里的 join() 永久阻塞，进程无法优雅退出。
            // gets re-evaluated as false, thread goes back to sleep, run() and destructor join()
            // block forever — no graceful exit.
            // 等待必须带期限：所有发送对象都挂在挂起表上时，谓词一直假，无超时 wait
            // Wait must have a deadline: when all sends are parked, predicate stays false;
            // 就把兜底扫描饿死——而挂在表上的那些对象的 time_limit 续期与无进展撤流，
            // timeout-less wait starves the belt sweep — whose job is exactly time_limit refresh
            // 恰恰只有这一趟扫描会去做。
            // and no-progress RST for parked objects.
            bool have_work = send_data_condition.wait_for(lk,
                                                          std::chrono::seconds(CONST_HTTP2_BELT_SWEEP_SECONDS),
                                                          [this]
                                                          { return this->isstop || this->sent_data_list.size() > 0; });
            if (isstop)
            {
                break;
            }
            // 兜底要挪到 send_data_mutex 之外跑：它够格时得把对象移进 sent_data_list，
            // Belt must run outside send_data_mutex: when eligible it moves objects into sent_data_list
            // 抢的就是这把锁。没到期时开销 = 一次原子读 + 一次失败 CAS。
            // — that's the lock it needs. When not due: one atomic read + failed CAS.
            lk.unlock();
            requeue_stuck_parked();
            if (!have_work)
            {
                // 纯超时醒（确实没活儿）：跑完兜底就回睡。
                // Pure timeout wake (nothing to do): run belt then back to sleep.
                continue;
            }
            lk.lock();
            // have_work 到这里已不可信：解锁去跑兜底那段里，另一发送线程可能把活摘走，
            // have_work is stale by now: while unlocked running belt, the other send thread may have
            // 兜底自己也可能刚回灌出新活儿 ⇒ 必须重判，不能照着超时前看到的值取 front()。
            // drained work, or belt may have just refilled some — re-check, don't trust pre-timeout value.
            if (sent_data_list.size() == 0)
            {
                lk.unlock();
                continue;
            }
            auto tpsend = sent_data_list.front();
            sent_data_list.pop_front();
            // 所有权转移必须和摘链表在同一临界区内完成：出了锁这对象归本线程独占。
            // Ownership transfer must happen inside the same critical section as unlinking — once
            // lock released this object is ours exclusively.
            tpsend->own_state = 2;
            lk.unlock();

            thread_sent_data_list.emplace_back(tpsend);
            send_loop_count = 0;

            for (;;)
            {
                send_loop_count++;
                if (send_loop_count % 4 == 0)
                {
                    std::unique_lock lock_in_loop_two(send_data_mutex);
                    if (sent_data_list.size() > 0)
                    {
                        auto temp_get_send_obj = sent_data_list.front();
                        sent_data_list.pop_front();
                        temp_get_send_obj->own_state = 2;
                        lock_in_loop_two.unlock();
                        thread_sent_data_list.emplace_back(temp_get_send_obj);
                    }
                    else
                    {
                        lock_in_loop_two.unlock();
                    }
                }
                // 长时间有活儿可干时，这一条线程不会回到上面那个带期限的等待，兜底就得在这里
                // 也递一次到（未到期的开销仍然只有一次原子读 + 一次失败的 CAS）。此刻
                // send_data_mutex 是空的：上面那次 %4 摘活的锁已经出了作用域。
                requeue_stuck_parked();
                int loop_per_num                                                  = thread_sent_data_list.size();
                const std::chrono::time_point<std::chrono::steady_clock> sq_start = std::chrono::steady_clock::now();

                for (auto iter = thread_sent_data_list.begin(); iter != thread_sent_data_list.end();)
                {
                    loop_per_num--;
                    if (loop_per_num < 0)
                    {
                        break;
                    }
                    // 需要检查 sp 对象
                    // Need to inspect sp object.
                    std::shared_ptr<http2_send_data_t> sp = *iter;
                    if (!sp)
                    {
                        // 测试是否存活
                        // Liveness check.
                        thread_sent_data_list.erase(iter++);
                        continue;
                    }
                    if (!sp->peer || !sp->peer->socket_session)
                    {
                        // 连接失效
                        // Connection invalidated.
                        thread_sent_data_list.erase(iter++);
                        http2_send_queue &send_queue_obj = get_http2_send_queue();
                        send_queue_obj.back_cache_ptr(sp);
                        continue;
                    }

                    if (sp->peer->isclose || sp->peer->issend || sp->peer->socket_session->isclose)
                    {
                        DEBUG_LOG("-- get_http2_send_queue -- %d", sp->peer->socket_session->http2_ring_queue ? (int)sp->peer->socket_session->http2_ring_queue->has_size() : -1);

                        // 这里不等「整条连接的发送环排空」就把对象回池，靠三条不变量：
                        // 1. issend=true 只在最后一帧成功 push 进环之后才置位，push 失败的分支一律让路下一轮。
                        // 2. 环里存的是帧的字节拷贝，ring_client_server 独立消费它们，跟对象在不在链表上无关。
                        // 3. 要防的是「END_STREAM 没入环就回池」，第 1 条已经管住；换成等全连接 has_size()
                        //    反而误事：别的流持续喂环时它永远大于 0，已发完的对象一直挂着，
                        //    fp 要等到连接被 httpwatch 杀掉才关。
                        // We recycle the object here without waiting for the whole connection ring to drain,
                        // on three invariants: 1. issend=true is set only after the last frame pushed into the
                        // ring successfully — every push-failure branch yields to the next round. 2. The ring
                        // holds byte copies of the frames, consumed by ring_client_server regardless of whether
                        // this object is still on a list. 3. What must be prevented is "END_STREAM recycled
                        // before it entered the ring", which 1 already covers; waiting on the connection's
                        // has_size() instead backfires — other streams keep it above 0, finished objects
                        // linger, and fp only closes once httpwatch kills the connection.

                        // 流响应已发完（issend）或连接/流已关闭，释放该流的
                        // stream response fully sent (issend) or connection/stream closed — free this stream's
                        // 发送窗口记账，避免 map 随请求数无限增长。
                        // send-window bookkeeping; prevents map from growing unbounded with request count.
                        {
                            std::lock_guard<std::mutex> lk_stream(sp->peer->socket_session->stream_send_window_mutex);
                            sp->peer->socket_session->stream_send_window.erase(sp->peer->stream_id);
                        }
                        thread_sent_data_list.erase(iter++);
                        http2_send_queue &send_queue_obj = get_http2_send_queue();
                        send_queue_obj.back_cache_ptr(sp);
                        continue;
                    }

                    // 大文件下载可能持续数分钟，time_limit 仅在收到请求时设过值，会被
                    // httpwatch_clear_timeout_sessions 误判为 idle 而清理。只要本流在
                    // 发送队列中（无论正在发数据还是等窗口/背压），就刷新 time_limit。
                    sp->peer->socket_session->time_limit.store(timeid());

                    // 闸门只有一套条件：连接级剩余额、该流剩余额、发送环槽位。
                    // 挡住就挂起等事件边回灌，不再留在本线程链表里逐 tick 复查；
                    // 链表空了就回到 send_data_condition 上阻塞，全阻塞时不占 CPU。
                    unsigned char gate_block = 0;
                    if (!http2_send_gate_open(sp, &gate_block))
                    {
                        if (!http2_park_send(sp, gate_block))
                        {
                            // 本连接已冲刷过挂起表（正在关闭），不再接收新的挂起：直接回池
                            thread_sent_data_list.erase(iter++);
                            get_http2_send_queue().back_cache_ptr(sp);
                        }
                        else
                        {
                            iter = thread_sent_data_list.erase(iter);
                        }
                        continue;
                    }

                    if (sp->standby_next)
                    {
                        bool seq_keep           = true;
                        unsigned char seq_block = 0;
                        // 首帧之外，本轮还允许再喂 CONST_HTTP2_SEND_FEED_MAX 帧。每次补喂之前
                        // 重取发送环积压，到 CONST_HTTP2_RING_FEED_SLOTS 就停：环「喂得动」就继续，
                        // 不是等它空 —— 第一帧是同步压进环的，压完再判空几乎永不成立。
                        // 每帧共用下面同一套后置处置（让路 ⇒ 挂起；返回 false ⇒ 回池），不新写分支。
                        for (unsigned char feed_once = 0; feed_once <= CONST_HTTP2_SEND_FEED_MAX; feed_once++)
                        {
                            if (feed_once != 0)
                            {
                                get_http2_send_queue().extra_feed_total.fetch_add(1,
                                                                                  std::memory_order_relaxed);
                            }
                            sp->standby_next = false;
                            sp->block_reason = 0;

                            DEBUG_LOG("-- http2_loop_send_sequence -- begin");
                            seq_keep  = http2_loop_send_sequence(sp);
                            seq_block = sp->block_reason;
                            if (!seq_keep || seq_block != 0)
                            {
                                break;
                            }
                            // 出了一帧、还有后续内容，才谈得上补喂
                            if (sp->peer->issend || !sp->standby_next || feed_once == CONST_HTTP2_SEND_FEED_MAX)
                            {
                                break;
                            }
                            const unsigned char ring_backlog =
                                sp->peer->socket_session->http2_ring_queue ? sp->peer->socket_session->http2_ring_queue->has_size() : 0;
                            if (ring_backlog >= CONST_HTTP2_RING_FEED_SLOTS)
                            {
                                break;
                            }
                        }
                        if (!seq_keep)
                        {
                            thread_sent_data_list.erase(iter++);
                            http2_send_queue &send_queue_obj = get_http2_send_queue();
                            send_queue_obj.back_cache_ptr(sp);
                            continue;
                        }
                        if (seq_block != 0)
                        {
                            // 序列内部让路（环满 / 额度见底），原因写明了挡住的那一道门
                            sp->block_reason = 0;
                            if (!http2_park_send(sp, seq_block))
                            {
                                thread_sent_data_list.erase(iter++);
                                get_http2_send_queue().back_cache_ptr(sp);
                            }
                            else
                            {
                                iter = thread_sent_data_list.erase(iter);
                            }
                            continue;
                        }
                    }

                    if (sp->peer->socket_session->http2_need_wakeup)
                    {
                        sp->peer->socket_session->waituphttp2();
                    }
                    iter++;
                }

                if (thread_sent_data_list.size() == 0)
                {
                    break;
                }
                // to fast
                const auto sq_end    = std::chrono::steady_clock::now();
                auto sq_for_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(sq_end - sq_start).count();

                // 链表里还有在推进的流时，这一轮几微秒就干完了活，于是这根 tick 每轮都触发：
                // 它给发送线程的轮次定了 cadence，单流上界 = 帧大小 × 每轮帧数 / 本值，
                // 不是"收尾窄窗口"的兜底。被闸门挡住的流已经挂进 parked_list、链表空得快，
                // 那种场合才真的靠事件边唤醒。
                if (sq_for_duration < CONST_HTTP2_SlEEP_MIN_TIME)
                {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(CONST_HTTP2_SlEEP_MIN_TIME));
                }

                if (isstop)
                {
                    break;
                }

                long long sq_obj_duration = std::chrono::duration_cast<std::chrono::seconds>(sq_start - last_loop_time).count();

                if (sq_obj_duration > 600)
                {
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(this_thread_tag);
                    lock.unlock();
                    last_loop_time = sq_start;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(e.what());
            lock.unlock();
        }
    }
}

void httpserver::websocket_loop(int fps)
{

    using namespace std::chrono;
    using dsec                = duration<double>;
    auto invFpsLimit          = duration_cast<system_clock::duration>(dsec{1. / 4.1});
    auto m_BeginFrame         = system_clock::now();
    auto m_EndFrame           = m_BeginFrame + invFpsLimit;
    auto prev_time_in_seconds = time_point_cast<seconds>(m_BeginFrame);
    fps                       = 0;
    std::string log_item;
    // unsigned frame_count_per_second = 0;
    orm::orm_connect_mar_t &watch_conn = orm::get_orm_connect_mar();

    for (;;)
    {
        auto time_in_seconds = time_point_cast<seconds>(system_clock::now());
        //++frame_count_per_second;
        if (time_in_seconds > prev_time_in_seconds)
        {

            // frame_count_per_second = 0;
            prev_time_in_seconds = time_in_seconds;
            ++fps;

            // 锁内摘取快照、锁外执行。持锁调业务钩子会自死锁
            // （业务在 run_loop 里还会 emplace_back 到 websockettasks）。
            std::vector<std::pair<std::shared_ptr<websockets_api>, bool>> tick_items;
            {
                std::lock_guard<std::mutex> lk(websocket_task_mutex);
                for (auto iter = websockettasks.begin(); iter != websockettasks.end();)
                {
                    std::shared_ptr<websockets_api> peer = iter->lock();
                    if (!peer || peer->isclose ||
                        (peer->session_sock && peer->session_sock->isclose))
                    {
                        iter = websockettasks.erase(iter);
                        continue;
                    }
                    bool need_run = (peer->loop_num > 0) && (peer->durtime > 0) && (fps % peer->durtime == 0);
                    tick_items.emplace_back(peer, need_run);
                    if (peer->loop_num == 0)
                    {
                        iter = websockettasks.erase(iter);
                    }
                    else
                    {
                        ++iter;
                    }
                }
            }

            // 锁外执行：run_loop / co_spawn / 环唤醒
            for (auto &item : tick_items)
            {
                auto &peer = item.first;
                try
                {
                    if (peer->session_sock && peer->session_sock->http2_need_wakeup)
                    {
                        peer->session_sock->waituphttp2();
                    }
                    if (item.second && peer->session_sock)
                    {
                        if (peer->is_loop_coroutine())
                        {
                            asio::co_spawn(peer->session_sock->strand_, peer->async_run_loop(), asio::detached);
                        }
                        else
                        {
                            peer->run_loop();
                        }
                    }
                }
                catch (...)
                {
                    // 业务钩子异常不影响其他连接
                }
            }

            // MQTT 全局分发协程 respawn：exchange(false) 认领成功者负责 co_spawn。
            // 协程自己排空/灌不动退出前置回 true，最坏 1 拍（1s）后接手。
            if (mqtt_sender_need_spawn.exchange(false))
            {
                asio::co_spawn(this->io_context, mqtt_send_loop(), asio::detached);
            }

            // 锁内摘快照、剔死项，锁外唤醒/执行业务钩子——持锁调 run_loop /
            // 业务回调可能重入 mqtt_task_mutex 自死锁（与 websockettasks 同款处理）。
            struct mqtt_tick_item
            {
                std::shared_ptr<mqtt_api> peer;
                std::shared_ptr<client_session> trans;
                bool need_tick = false;
            };
            std::vector<mqtt_tick_item> mqtt_ticks;
            {
                std::lock_guard<std::mutex> lk(mqtt_task_mutex);
                for (auto iter = mqtttasks.begin(); iter != mqtttasks.end();)
                {
                    std::shared_ptr<mqtt_api> peer = iter->lock();
                    if (!peer)
                    {
                        iter = mqtttasks.erase(iter);
                        continue;
                    }
                    auto trans = peer->transport();
                    if (!trans || trans->isclose)
                    {
                        iter = mqtttasks.erase(iter);
                        continue;
                    }
                    // loop_num==0：本轮做最后一次兜底唤醒后剔除（沿用原实现的顺序：waitup 先于 erase）
                    bool dying     = (peer->loop_num == 0);
                    bool need_tick = (!dying) && (peer->durtime > 0) && (fps % peer->durtime == 0);
                    mqtt_ticks.push_back(mqtt_tick_item{peer, trans, need_tick});
                    if (dying)
                    {
                        iter = mqtttasks.erase(iter);
                    }
                    else
                    {
                        ++iter;
                    }
                }
            }

            for (auto &item : mqtt_ticks)
            {
                try
                {
                    // 发送环兜底唤醒（丢唤醒的最后一道防线）
                    if (item.trans->http2_need_wakeup)
                    {
                        item.trans->waituphttp2();
                    }
                    if (item.need_tick)
                    {
                        if (item.peer->isloopco)
                        {
                            asio::co_spawn(item.trans->strand_, item.peer->async_run_loop(), asio::detached);
                        }
                        else
                        {
                            item.peer->run_loop();
                        }
                    }
                }
                catch (...)
                {
                    // 业务钩子异常不影响其他连接
                }
            }

#ifdef ENABLE_REDIS_CLIENT

            // ===== Redis pubsub tick 扫描 =====
            struct redis_tick_item
            {
                std::shared_ptr<pz::redis::redis_subpub_client> peer;
                bool need_tick = false;
            };
            std::vector<redis_tick_item> redis_ticks;
            std::vector<std::shared_ptr<pz::redis::redis_subpub_client>> redis_retire;
            {
                std::lock_guard<std::mutex> lk(redis_subpub_task_mutex);
                for (auto iter = redis_subpub_tasks.begin(); iter != redis_subpub_tasks.end();)
                {
                    auto peer = iter->lock();
                    if (!peer || peer->isclose || peer->loop_num == 0)
                    {
                        // 业务可以直接写 isclose（那面旗就是这么设计的），写了就得收摊：
                        // 只摘登记条目的话 pump 继续读、继续给一个已宣告收摊的客户端派消息。
                        // loop_num==0 是 tick 预算用完，订阅按语义留着，那一支不收摊。
                        if (peer && peer->isclose)
                            redis_retire.push_back(peer);
                        iter = redis_subpub_tasks.erase(iter);
                        continue;
                    }
                    bool need_tick = (peer->durtime > 0) && (fps % peer->durtime == 0);
                    redis_ticks.push_back(redis_tick_item{peer, need_tick});
                    ++iter;
                }
            }
            // 锁外收摊：stop() 要拿订阅句柄的锁并投递停止动作，别握着登记表的锁跑别人的代码
            for (auto &peer : redis_retire)
            {
                peer->stop();
            }
            for (auto &item : redis_ticks)
            {
                if (!item.need_tick)
                {
                    continue;
                }
                auto peer = item.peer;
                try
                {
                    // redis 这一路只有 is_coroutine_ 一个旗。
                    if (peer->is_coroutine_)
                    {
                        asio::co_spawn(this->io_context, [peer]() -> asio::awaitable<void>
                                       {
                                try
                                {
                                    co_await peer->async_run_loop();
                                }
                                catch (...)
                                {
                                    resident_hook_failed("redis_subpub.async_run_loop",
                                                         std::current_exception(), false);
                                }
                                co_return; },
                                       asio::detached);
                    }
                    else if (!post_conn_step("redis_subpub.run_loop", [peer]
                                             {
                                 try
                                 {
                                     peer->run_loop();
                                 }
                                 catch (...)
                                 {
                                     resident_hook_failed("redis_subpub.run_loop",
                                                          std::current_exception(), false);
                                 } }))
                    {
                        // 池没接单：这一拍的心跳整条丢掉，丢弃已经在计数里记了，这里只补一条日志说明原因
                        resident_hook_failed("redis_subpub.run_loop", nullptr, true);
                    }
                }
                catch (...)
                {
                    resident_hook_failed("redis_subpub.run_loop spawn", std::current_exception(), false);
                }
            }
#endif

#ifdef ENABLE_WEBSOCKETS_CLIENT
            // ===== WebSocket 长连接 tick 扫描（完全镜像 Redis pubsub 段）=====
            struct ws_tick_item
            {
                std::shared_ptr<http::ws_subpub_client> peer;
                bool need_tick = false;
            };
            std::vector<ws_tick_item> ws_ticks;
            std::vector<std::shared_ptr<http::ws_subpub_client>> ws_retire;
            {
                std::lock_guard<std::mutex> lk(ws_subpub_task_mutex);
                for (auto iter = ws_subpub_tasks.begin(); iter != ws_subpub_tasks.end();)
                {
                    auto peer = iter->lock();
                    if (!peer || peer->isclose || peer->loop_num == 0)
                    {
                        // 业务置了 isclose 就得真的收摊：只把条目从登记表摘掉，常驻 pump 还
                        // parked 在途读上，fd 和协程帧一起留下。loop_num==0 只是这一路 tick
                        // 的预算用完，连接按语义还要留着，所以那一支不收摊。
                        if (peer && peer->isclose)
                            ws_retire.push_back(peer);
                        iter = ws_subpub_tasks.erase(iter);
                        continue;
                    }
                    bool need_tick = (peer->durtime > 0) && (fps % peer->durtime == 0);
                    ws_ticks.push_back(ws_tick_item{peer, need_tick});
                    ++iter;
                }
            }
            // 锁外收摊：stop() 要拿连接自己的锁并动这条 socket，别握着登记表的锁跑别人的代码
            for (auto &peer : ws_retire)
            {
                peer->stop();
            }
            for (auto &item : ws_ticks)
            {
                if (!item.need_tick)
                {
                    continue;
                }
                auto peer = item.peer;
                try
                {
                    if (peer->is_loop_co_)
                    {
                        asio::co_spawn(this->io_context, [peer]() -> asio::awaitable<void>
                                       {
                                try
                                {
                                    co_await peer->async_run_loop();
                                }
                                catch (...)
                                {
                                    resident_hook_failed("ws_subpub.async_run_loop",
                                                         std::current_exception(), false);
                                }
                                co_return; },
                                       asio::detached);
                    }
                    else if (!post_conn_step("ws_subpub.run_loop", [peer]
                                             {
                                 try
                                 {
                                     peer->run_loop();
                                 }
                                 catch (...)
                                 {
                                     resident_hook_failed("ws_subpub.run_loop",
                                                          std::current_exception(), false);
                                 } }))
                    {
                        resident_hook_failed("ws_subpub.run_loop", nullptr, true);
                    }
                }
                catch (...)
                {
                    resident_hook_failed("ws_subpub.run_loop spawn", std::current_exception(), false);
                }
            }
#endif

#ifdef ENABLE_SOCKETS_CLIENT
            // ===== Socket 长连接 tick 扫描 =====
            struct sock_tick_item
            {
                std::shared_ptr<http::sock_subpub_client> peer;
                bool need_tick = false;
            };
            std::vector<sock_tick_item> sock_ticks;
            std::vector<std::shared_ptr<http::sock_subpub_client>> sock_retire;
            {
                std::lock_guard<std::mutex> lk(this->sockets_clients_mutex);
                for (auto iter = this->sockets_clients.begin(); iter != this->sockets_clients.end();)
                {
                    auto peer = iter->lock();
                    if (!peer || peer->isclose || peer->loop_num == 0)
                    {
                        // 同 ws 侧：isclose 只摘登记条目收不了摊，pump 还 parked 在途读上；
                        // loop_num==0 只是 tick 预算用完，连接按语义留着，那一支不收摊。
                        if (peer && peer->isclose)
                            sock_retire.push_back(peer);
                        iter = this->sockets_clients.erase(iter);
                        continue;
                    }
                    bool need_tick = (peer->durtime > 0) && (fps % peer->durtime == 0);
                    sock_ticks.push_back(sock_tick_item{peer, need_tick});
                    ++iter;
                }
            }
            // 锁外收摊：stop() 要拿连接自己的锁并动这条 socket，别握着登记表的锁跑别人的代码
            for (auto &peer : sock_retire)
            {
                peer->stop();
            }
            for (auto &item : sock_ticks)
            {
                if (!item.need_tick)
                {
                    continue;
                }
                auto peer = item.peer;
                try
                {
                    if (peer->is_loop_co_)
                    {
                        asio::co_spawn(this->io_context, [peer]() -> asio::awaitable<void>
                                       {
                                try
                                {
                                    co_await peer->async_run_loop();
                                }
                                catch (...)
                                {
                                    resident_hook_failed("sock_subpub.async_run_loop",
                                                         std::current_exception(), false);
                                }
                                co_return; },
                                       asio::detached);
                    }
                    else if (!post_conn_step("sock_subpub.run_loop", [peer]
                                             {
                                 try
                                 {
                                     peer->run_loop();
                                 }
                                 catch (...)
                                 {
                                     resident_hook_failed("sock_subpub.run_loop",
                                                          std::current_exception(), false);
                                 } }))
                    {
                        resident_hook_failed("sock_subpub.run_loop", nullptr, true);
                    }
                }
                catch (...)
                {
                    resident_hook_failed("sock_subpub.run_loop spawn", std::current_exception(), false);
                }
            }
#endif

#ifdef ENABLE_MQTT_CLIENT
            // ===== MQTT 长连接 tick 扫描（出站 broker 客户端） =====
            struct mqtt_client_tick_item
            {
                std::shared_ptr<http::mqtt_subpub_client> peer;
                bool need_tick = false;
            };
            std::vector<mqtt_client_tick_item> mqtt_client_ticks;
            std::vector<std::shared_ptr<http::mqtt_subpub_client>> mqtt_client_retire;
            {
                std::lock_guard<std::mutex> lk(this->mqtt_clients_mutex);
                for (auto iter = this->mqtt_clients.begin(); iter != this->mqtt_clients.end();)
                {
                    auto peer = iter->lock();
                    if (!peer || peer->isclose || peer->loop_num == 0)
                    {
                        // 同 ws/sock：isclose 只摘登记条目收不了摊；loop_num==0 是 tick 预算
                        // 用完，连接按语义留着，那一支不收摊。
                        if (peer && peer->isclose)
                            mqtt_client_retire.push_back(peer);
                        iter = this->mqtt_clients.erase(iter);
                        continue;
                    }
                    bool need_tick = (peer->durtime > 0) && (fps % peer->durtime == 0);
                    mqtt_client_ticks.push_back(mqtt_client_tick_item{peer, need_tick});
                    ++iter;
                }
            }
            // 锁外收摊：stop() 要拿连接自己的锁，别握着登记表的锁跑别人的代码
            for (auto &peer : mqtt_client_retire)
            {
                peer->stop();
            }
            for (auto &item : mqtt_client_ticks)
            {
                if (!item.need_tick)
                {
                    continue;
                }
                auto peer = item.peer;
                try
                {
                    if (peer->is_loop_co_)
                    {
                        asio::co_spawn(this->io_context, [peer]() -> asio::awaitable<void>
                                       {
                                try
                                {
                                    co_await peer->async_run_loop();
                                }
                                catch (...)
                                {
                                    resident_hook_failed("mqtt_subpub.async_run_loop",
                                                         std::current_exception(), false);
                                }
                                co_return; },
                                       asio::detached);
                    }
                    else if (!post_conn_step("mqtt_subpub.run_loop", [peer]
                                             {
                                 try
                                 {
                                     peer->run_loop();
                                 }
                                 catch (...)
                                 {
                                     resident_hook_failed("mqtt_subpub.run_loop",
                                                          std::current_exception(), false);
                                 } }))
                    {
                        resident_hook_failed("mqtt_subpub.run_loop", nullptr, true);
                    }
                }
                catch (...)
                {
                    resident_hook_failed("mqtt_subpub.run_loop spawn", std::current_exception(), false);
                }
            }
#endif

            // 锁内摘快照（连同该擦除的节点一起擦掉），锁外再投递给业务线程池。
            // addclient() 自己握着 clientrunpool 的 queue_mutex，攥着 clientlooptasks_mutex
            // 去拿它，等于给这两把锁定了先后顺序；而 frametasks_timeloop 那条登记口拿的是
            // 后一把，哪天有路径反过来先拿 queue_mutex，就是互相等死。
            // 和 sockettasks / websockettasks 同一把锁同一形状：锁里只碰表，不跑别人的代码。
            std::vector<std::shared_ptr<httppeer>> clientloop_drops;
            {
                std::lock_guard<std::mutex> clientlooptasks_lk(this->clientlooptasks_mutex);
                this->clientloop_ticks += 1;
                for (auto iter = clientlooptasks.begin(); iter != clientlooptasks.end();)
                {
                    auto &task = iter->second;
                    if (task->timeloop_num > 0 && (fps % task->timeloop_num) == 0)
                    {
                        // 单飞门：这条 peer 的上一拍还在池线程里跑就不投了，本次整条跳过（不积压），
                        // 下一拍再抢。比自己的间隔还慢的间隔任务因此始终只有一个执行者。
                        bool not_running = false;
                        if (task->timeloop_inflight.compare_exchange_strong(not_running, true))
                        {
                            clientloop_drops.push_back(task);
                        }
                        else
                        {
                            this->clientloop_skipped += 1;
                        }
                    }

                    if (task->timecount_num == 0 || task->timeloop_num == 0)
                    {
                        iter = clientlooptasks.erase(iter);
                    }
                    else
                    {
                        ++iter;
                    }
                }
            }
            for (auto &task : clientloop_drops)
            {
                try
                {
                    // 旗是这里抢的；池线程只要把这一条任务从 clienttasks 里弹出来就会放它
                    // （threadpool.cpp 取货循环的 flag_guard），没进队列就没人弹，所以入队失败要就地收回这一条。
                    if (!clientrunpool.addclient(task))
                    {
                        task->timeloop_inflight.store(false);
                        resident_hook_failed("clientlooptasks.addclient", nullptr, true);
                    }
                }
                catch (...)
                {
                    // 一条投递失败不影响同拍的其他间隔任务；这一拍这一条整条丢掉，必须留下日志
                    task->timeloop_inflight.store(false);
                    resident_hook_failed("clientlooptasks.addclient", std::current_exception(), false);
                }
            }

            // 锁内摘快照、剔死项，锁外执行业务钩子——run_loop 里业务可能再登记
            // sockettasks，持锁调用会重入 socket_task_mutex 自死锁（同 websockettasks）。
            struct socket_tick_item
            {
                std::shared_ptr<socket_api> peer;
                std::shared_ptr<client_session> sess;
                bool need_tick = false;
            };
            std::vector<socket_tick_item> socket_ticks;
            {
                std::lock_guard<std::mutex> lk(socket_task_mutex);
                for (auto iter = sockettasks.begin(); iter != sockettasks.end();)
                {
                    std::shared_ptr<socket_api> peer = iter->lock();
                    if (!peer || peer->isclose || !peer->session_sock ||
                        peer->session_sock->isclose || peer->timeloop_num == 0)
                    {
                        iter = sockettasks.erase(iter);
                        continue;
                    }
                    bool need_tick = (fps % peer->timeloop_num == 0);
                    socket_ticks.push_back(socket_tick_item{peer, peer->session_sock, need_tick});
                    ++iter;
                }
            }

            for (auto &item : socket_ticks)
            {
                try
                {
                    if (item.need_tick)
                    {
                        if (item.peer->isloopco)
                        {
                            asio::co_spawn(item.sess->strand_, item.peer->async_run_loop(), asio::detached);
                        }
                        else
                        {
                            item.peer->run_loop();
                        }
                    }
                }
                catch (...)
                {
                    // 业务钩子异常不影响其他连接
                }
            }
            // if (fps % 13 == 0)
            // {
            //     unsigned int session_num = 0;

            //     std::unique_lock lk(wait_clear_mutex);
            //     session_num = socket_session_wait_clear.size();
            //     lk.unlock();

            //     if (session_num > 0)
            //     {
            //         log_item.clear();
            //         log_item.append("-- clear sock num ");
            //         log_item.append(std::to_string(session_num));
            //         log_item.append(" --\n");
            //         std::unique_lock<std::mutex> lock(log_mutex);
            //         error_loglist.emplace_back(log_item);
            //         lock.unlock();

            //         for (size_t i = 0; i < 100; i++)
            //         {
            //             std::unique_lock lk(wait_clear_mutex);
            //             if (socket_session_wait_clear.size() > 0)
            //             {
            //                 auto p_sock_session = std::move(socket_session_wait_clear.front());
            //                 socket_session_wait_clear.pop_front();
            //                 lk.unlock();
            //                 //p_sock_session->stop();
            //                 asio::co_spawn(this->io_context, clientpeerstop(p_sock_session), asio::detached);
            //             }
            //             else
            //             {
            //                 lk.unlock();
            //                 break;
            //             }

            //             DEBUG_LOG("socket_session_wait_clear stop");
            //         }
            //     }
            // }

            if (fps > 31536000)
            {
                fps = 1;
            }

            if (fps % 1024 == 0)
            {
                log_item.clear();
                log_item.append("-- loop websocket is live ");
                log_item.append(std::to_string(fps));
                log_item.append(" --\n");
                std::unique_lock<std::mutex> lock(log_mutex);
                error_loglist.emplace_back(log_item);
                lock.unlock();
            }

            if (fps % 320 == 0)
            {
                watch_conn.clear_connect();
            }

            // MQTT broker 失效订阅/client 记录定期清理（每 300 秒）。
            // 原先在 publish() 每消息 O(N) 扫描，改到此处避免阻塞发布热路径。
            if (fps % 300 == 0)
            {
                mqtt_broker::instance().prune_expired();
            }

            if ((fps % 256) == 0)
            {
                http2_send_queue &send_queue_obj = get_http2_send_queue();
                if (send_queue_obj.queue_list.size() > rate_limit_accept_wait_num.load())
                {
                    send_queue_obj.fix_queue_list(rate_limit_accept_wait_num.load());
                }
                // 定期清理 ring queue 缓存池，避免无限增长
                http2_ring_queue_obj &ring_queue_obj = get_http2_ring_queue_obj();
                ring_queue_obj.fix_queue_list();
            }
        }
        std::this_thread::sleep_until(m_EndFrame);
        m_BeginFrame = m_EndFrame;
        m_EndFrame   = m_BeginFrame + invFpsLimit;
        if (isstop)
        {
            break;
        }
    }
}
asio::awaitable<void>
httpserver::sslhandshake(std::shared_ptr<client_session> peer_session)
{
    try
    {
        total_http2_count--;
        if (check_pressl_blockip(peer_session))
        {
            co_return;
        }
        unsigned int next_proto_len    = 0;
        constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
        asio::error_code ec_error;

        std::tie(ec_error) = co_await peer_session->sslsocket->async_handshake(asio::ssl::stream_base::server, tuple_awaitable);
        if (ec_error)
        {
#ifndef BENCHMARK
            next_proto_len = peer_session->client_ip.size();
            peer_session->client_ip.append(" ");
            peer_session->client_ip.append(ec_error.message());
            peer_session->client_ip.append(" sslhandshake ec_error\n");
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(peer_session->client_ip);
            lock.unlock();
            peer_session->client_ip.resize(next_proto_len);
            next_proto_len = 0;
            DEBUG_LOG(" handshake ec_error ! %s\n", ec_error.message().c_str());
#endif
            co_return;
        }

        DEBUG_LOG("https accept ok!");
        const unsigned char *for_next_proto = nullptr;
        next_proto_len                      = 0;
        SSL_get0_alpn_selected(peer_session->sslsocket->native_handle(), &for_next_proto, &next_proto_len);
        if (next_proto_len > 1)
        {
            if (for_next_proto[0] == 'h' && for_next_proto[1] == '2')
            {
                DEBUG_LOG(" h2 ");
                peer_session->httpv = 2;
            }
        }

        asio::co_spawn(peer_session->strand_, [peer_session, this]() mutable
                       { return clientpeerfun(peer_session, true); },
                       asio::detached);
        co_return;
    }
    catch (std::exception &e)
    {
#ifndef BENCHMARK
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back(std::string(e.what()) + " " + peer_session->getremoteip() + "\n");
        lock.unlock();
        DEBUG_LOG(" handshake ec_error ! %s\n", e.what());
#endif
        co_return;
    }
    catch (...)
    {
        co_return;
    }
    co_return;
}

#ifdef ENABLE_REDIS_CLIENT
asio::awaitable<void> httpserver::async_redis_subpub_loop(
    std::shared_ptr<pz::redis::redis_subpub_client> client)
{
    auto &io_ctx = this->io_context;
    // 指数退避重连：3s → 6s → 9s → 12s（封顶），连接成功后重置 3s
    int retry_sec = 3;
    auto backoff  = [&retry_sec]() -> std::chrono::seconds
    {
        auto v = std::chrono::seconds(retry_sec);
        if (retry_sec < 12)
            retry_sec += 3;// 下次 +3s，封顶 12s
        return v;
    };
    while (!client->isclose)
    {
        auto sub = std::make_shared<pz::redis::redis_subscriber>(io_ctx);
        client->set_active_sub(sub);
        client->sub_state_.store(pz::redis::subscriber_state::connecting);

        auto sec = pz::redis::get_redis_pool().section(client->section_name());
        if (!sec)
        {
            auto wait = backoff();
            fprintf(stderr, "[redis_subpub] %s: no section → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }
        auto cfg       = sec->take_config();
        client->io_ctx = &io_ctx;// 让业务回调里能 co_spawn
        if (!co_await sub->async_start(cfg))
        {
            // 断连回调丢业务池（同步版），异步版在本协程 co_await
            if (client->is_coroutine_)
                co_await co_resident_hook_void("redis_subpub.async_on_disconnect", client->async_on_disconnect());
            else
                co_await co_sync_hook_void("redis_subpub.on_disconnect(start fail)", [client]
                                           { client->on_disconnect(); },
                                           nullptr);
            auto wait = backoff();
            fprintf(stderr, "[redis_subpub] %s: async_start fail → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }

        // pump 消息回调：pump 在 io_context worker 上触发
        //   同步版 → post_conn_step 丢业务线程池（投了就走）
        //   异步版 → co_spawn 到 io_context，业务协程里 co_await async_on_message
        // subscribe/psubscribe 在 sub 的 strand 上执行。
        for (auto &ch : client->channels())
        {
            if (client->isclose)
                break;
            co_await asio::co_spawn(sub->strand(), [&]() -> asio::awaitable<bool>
                                    { co_return co_await sub->async_subscribe(
                                          {ch},
                                          [client, &io_ctx](const std::string &c, const std::string &p)
                                          {
                                              // 收摊后 pump 可能还在读这一轮：宣告收摊就不该再叫业务钩子
                                              if (client->isclose)
                                                  return;
                                              if (client->is_coroutine_)
                                              {
                                                  asio::co_spawn(io_ctx, [client, c, p]() -> asio::awaitable<void>
                                                                 {
                                                                        try
                                                                        {
                                                                            co_await client->async_on_message(c, p);
                                                                        }
                                                                        catch (...)
                                                                        {
                                                                            resident_hook_failed("redis_subpub.async_on_message",
                                                                                                    std::current_exception(), false);
                                                                        }
                                                                        co_return; },
                                                                 asio::detached);
                                              }
                                              else if (!post_conn_step("redis_pubsub.on_message",
                                                                       [client, c, p]
                                                                       {
                                                                           try
                                                                           {
                                                                               client->on_message(c, p);
                                                                           }
                                                                           catch (...)
                                                                           {
                                                                               resident_hook_failed("redis_pubsub.on_message",
                                                                                                    std::current_exception(),
                                                                                                    false);
                                                                           }
                                                                       }))
                                              {
                                                  resident_hook_failed("redis_pubsub.on_message", nullptr, true);
                                              }
                                          }); },
                                    asio::use_awaitable);
        }
        for (auto &pat : client->patterns())
        {
            if (client->isclose)
                break;
            co_await asio::co_spawn(sub->strand(), [&]() -> asio::awaitable<bool>
                                    { co_return co_await sub->async_psubscribe(
                                          pat,
                                          [client, &io_ctx](const std::string &pt,
                                                            const std::string &c,
                                                            const std::string &p)
                                          {
                                              // 同 channel 回调：收摊后 pump 可能还在读这一轮
                                              if (client->isclose)
                                                  return;
                                              if (client->is_coroutine_)
                                              {
                                                  asio::co_spawn(
                                                      io_ctx,
                                                      [client, pt, c, p]() -> asio::awaitable<void>
                                                      {
                                        try
                                        {
                                            co_await client->async_on_pmessage(pt, c, p);
                                        }
                                        catch (...)
                                        {
                                            resident_hook_failed("redis_subpub.async_on_pmessage",
                                                                 std::current_exception(), false);
                                        }
                                        co_return; },
                                                      asio::detached);
                                              }
                                              else if (!post_conn_step(
                                                           "redis_pubsub.on_pmessage",
                                                           [client, pt, c, p]
                                                           {
                                                               try
                                                               {
                                                                   client->on_pmessage(pt, c, p);
                                                               }
                                                               catch (...)
                                                               {
                                                                   resident_hook_failed("redis_pubsub.on_pmessage",
                                                                                        std::current_exception(),
                                                                                        false);
                                                               }
                                                           }))
                                              {
                                                  resident_hook_failed("redis_pubsub.on_pmessage", nullptr, true);
                                              }
                                          }); },
                                    asio::use_awaitable);
        }

        if (client->isclose)
        {
            // 收摊也要把订阅句柄停掉并交回：只 break 的话 pump 还在读，
            // 一个已宣告收摊的客户端会继续被派发消息，订阅对象和 fd 都不释放。
            sub->stop();
            client->set_active_sub(nullptr);
            break;
        }
        client->sub_state_.store(pz::redis::subscriber_state::subscribed);
        retry_sec = 3;// ✅ 连上了 → 重置退避
        fprintf(stderr, "[redis_subpub] %s: subscribed OK, retry reset to 3s\n", client->section_name().c_str());
        if (client->is_coroutine_)
            co_await co_resident_hook_void("redis_subpub.async_on_subscribe_ok", client->async_on_subscribe_ok());
        else
            co_await co_sync_hook_void("redis_subpub.on_subscribe_ok", [client]
                                       { client->on_subscribe_ok(); },
                                       nullptr);

        // 等 pump 退出（200ms 轮询 state 或业务置 isclose）
        while (sub->state() != pz::redis::subscriber_state::stopped && !client->isclose)
        {
            asio::steady_timer t(io_ctx, std::chrono::milliseconds(200));
            co_await t.async_wait(asio::use_awaitable);
        }
        client->sub_state_.store(pz::redis::subscriber_state::stopped);
        if (!client->isclose)
        {
            if (client->is_coroutine_)
                co_await co_resident_hook_void("redis_subpub.async_on_disconnect", client->async_on_disconnect());
            else
                co_await co_sync_hook_void("redis_subpub.on_disconnect", [client]
                                           { client->on_disconnect(); },
                                           nullptr);
        }
        // 钩子跑完再交回订阅句柄：断连回调里业务还能读这一轮的订阅者
        sub->stop();
        client->set_active_sub(nullptr);
        if (client->isclose)
            break;
        auto wait = backoff();
        fprintf(stderr, "[redis_subpub] %s: pump exit → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
        asio::steady_timer t(io_ctx, wait);
        co_await t.async_wait(asio::use_awaitable);
    }
    client->sub_state_.store(pz::redis::subscriber_state::stopped);
    co_return;
}
#endif

#ifdef ENABLE_WEBSOCKETS_CLIENT
asio::awaitable<void> httpserver::async_ws_subpub_loop(
    std::shared_ptr<http::ws_subpub_client> client)
{
    auto &io_ctx  = this->io_context;
    int retry_sec = 3;
    auto backoff  = [&retry_sec]() -> std::chrono::seconds
    {
        auto v = std::chrono::seconds(retry_sec);
        if (retry_sec < 12)
            retry_sec += 3;
        return v;
    };

    while (!client->isclose)
    {
        // 1. 取配置
        auto sec = http::get_websockets_config().get(client->section_name());
        if (!sec)
        {
            auto wait = backoff();
            fprintf(stderr, "[ws_subpub] %s: no section → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }
        auto &cfg = *sec;

        // 2. 创建 websocket_client
        auto ws = std::make_shared<http::websocket_client>();
        ws->set_host(cfg.host);
        ws->set_port(cfg.port);
        ws->set_url(cfg.url);
        ws->strand_ = asio::make_strand(io_ctx);
        ws->timeout(cfg.timeout_sec);
        // 常驻连接交给看护线程的 dur 托管分支：dur_time_loop_fun 非空且 durtime > 0 时，
        // 看护线程那一拍只发心跳、不做超时检查（client_context.cpp:136-149）。
        // 不这么写的话，pump 正 parked 在读上、链路空闲到期限就被在看护线程上 close_connect()。
        // 注：看护线程 1 拍 = 250ms，这里的 durtime 是它的拍数；业务侧 durtime 是 1s 一拍，两者不同单位。
        ws->durtime           = client->durtime > 0 ? client->durtime : 8;
        ws->dur_time_loop_fun = [](std::shared_ptr<http::websocket_client> s)
        { s->reset_timeout(); };
        // 自定义 header（格式: "Key1: Val1\r\nKey2: Val2" 或 "Key1: Val1;Key2: Val2"）
        // 配置层 header
        if (!cfg.header.empty())
            ws->add_headers(cfg.header);
        // 业务侧 virtual add_headers() 返回的字符串
        auto biz_hdrs = client->add_headers();
        if (!biz_hdrs.empty())
            ws->add_headers(biz_hdrs);
        // 业务侧构造时 add_header()/add_headers() 灌的 vector（最高优先级）
        for (auto &[n, v] : client->user_headers_)
            ws->set_header(n, v);
        client->io_ctx = &io_ctx;

        // 3. async_connect + handshake
        if (!co_await ws->async_connect())
        {
            auto wait = backoff();
            fprintf(stderr, "[ws_subpub] %s: async_connect fail err='%s' → wait %lld\n", client->section_name().c_str(), ws->error_msg.c_str(), (long long)wait.count());
            if (client->is_coroutine_)
                co_await co_resident_hook_void("ws_subpub.async_on_close", client->async_on_close());
            else
                co_await co_sync_hook_void("ws_subpub.on_close(connect fail)", [client]
                                           { client->on_close(); },
                                           nullptr);
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }

        // 连上了才把连接交给业务：async_connect 里还在 reset()/重建 sock，
        // 提前赋值等于让业务线程和这条协程同时读写同一个 shared_ptr 和半个连接对象。
        client->set_conn(ws);

        retry_sec = 3;
        fprintf(stderr, "[ws_subpub] %s: connected %s:%u%s\n", client->section_name().c_str(), cfg.host.c_str(), cfg.port, cfg.url.c_str());

        // 4. on_open
        if (client->is_coroutine_)
            co_await co_resident_hook_void("ws_subpub.async_on_open", client->async_on_open());
        else
            co_await co_sync_hook_void("ws_subpub.on_open", [client]
                                       { client->on_open(); },
                                       nullptr);

        // 5. pump async_text_read / async_data_read 循环
        //    async_text_read() 完成时返回消息首帧 opcode，错误/EOF 返回 0；
        //    存活判定看 iserror（ready_state 是历史字段，全树没有人写过非 0）
        while (!client->isclose && !ws->iserror)
        {
            std::string buf;
            auto n = co_await ws->async_text_read();
            if (n == 0)
                break;// 对端关了

            bool is_binary = (ws->recv_data.opcode == 0x02);// RFC 6455 opcode 2 = binary
            auto payload   = std::move(ws->recv_data.content);

            if (client->is_coroutine_)
            {
                asio::co_spawn(io_ctx, [client, payload = std::move(payload), is_binary]() -> asio::awaitable<void>
                               {
                        try
                        {
                            co_await client->async_on_message(payload, is_binary);
                        }
                        catch (...)
                        {
                            resident_hook_failed("ws_subpub.async_on_message",
                                                 std::current_exception(), false);
                        }
                        co_return; },
                               asio::detached);
            }
            else if (!post_conn_step("ws_subpub.on_message",
                                     [client, payload = std::move(payload), is_binary]
                                     {
                                         try
                                         {
                                             client->on_message(payload, is_binary);
                                         }
                                         catch (...)
                                         {
                                             resident_hook_failed("ws_subpub.on_message",
                                                                  std::current_exception(),
                                                                  false);
                                         }
                                     }))
            {
                // 池没接单：这一帧整条丢掉（threadpool 里已经计数），业务侧完全看不出少了一条
                resident_hook_failed("ws_subpub.on_message", nullptr, true);
            }
        }

        // 6. 断连 → 钩子（还能读这一轮连接）→ 清 → backoff → 重连
        if (!client->isclose)
        {
            if (client->is_coroutine_)
                co_await co_resident_hook_void("ws_subpub.async_on_close", client->async_on_close());
            else
                co_await co_sync_hook_void("ws_subpub.on_close", [client]
                                           { client->on_close(); },
                                           nullptr);
        }
        // 先关连接再交回句柄，对齐 sock / mqtt 侧的收摊顺序：只清槽位的话这一轮的
        // websocket_client 要等出了循环、析构函数里才关，中间还隔着一轮退避。
        ws->close_connect();
        client->set_conn(nullptr);
        if (client->isclose)
            break;
        auto wait = backoff();
        fprintf(stderr, "[ws_subpub] %s: pump exit → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
        asio::steady_timer t(io_ctx, wait);
        co_await t.async_wait(asio::use_awaitable);
    }
    co_return;
}
#endif

#ifdef ENABLE_SOCKETS_CLIENT
asio::awaitable<void> httpserver::async_sock_subpub_loop(
    std::shared_ptr<http::sock_subpub_client> client)
{
    auto &io_ctx  = this->io_context;
    int retry_sec = 3;
    auto backoff  = [&retry_sec]() -> std::chrono::seconds
    {
        auto v = std::chrono::seconds(retry_sec);
        if (retry_sec < 12)
            retry_sec += 3;
        return v;
    };

    while (!client->isclose)
    {
        // 1. 取配置
        auto sec = http::get_sockets_config().get(client->section_name());
        if (!sec)
        {
            auto wait = backoff();
            fprintf(stderr, "[sock_subpub] %s: no section → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }
        auto &cfg = *sec;

        // 2. 创建 socket_client（裸 TCP）
        auto sock = std::make_shared<http::socket_client>();
        sock->set_host(cfg.host);
        sock->set_port(cfg.port);
        sock->set_url(cfg.url);
        sock->timeout(cfg.timeout_sec);
        // 同 ws 侧：看护线程 1 拍 = 250ms，业务侧 durtime 是 1s 一拍，单位不同但都只做"拍频分频"。
        // 裸 TCP 的 socket 列表没有 timeout==0 豁免，不托管心跳就一定被 idle-kill。
        sock->durtime           = client->durtime > 0 ? client->durtime : 8;
        sock->dur_time_loop_fun = [](std::shared_ptr<http::socket_client> s)
        { s->reset_timeout(); };
        client->io_ctx = &io_ctx;

        // 3. async_tcp_connect（裸 TCP，发 "tcp url\n\n"，不走 HTTP header）
        //    注: socket_client::set_header() 仅在 async_connect()（HTTP 模式）里用
        //        async_tcp_connect() 是纯 TCP，不发 HTTP header，所以跳过 header 注入
        if (!co_await sock->async_tcp_connect())
        {
            auto wait = backoff();
            fprintf(stderr, "[sock_subpub] %s: async_tcp_connect fail err='%s' → wait %lld\n", client->section_name().c_str(), sock->error_msg.c_str(), (long long)wait.count());
            if (client->is_coroutine_)
                co_await co_resident_hook_void("sock_subpub.async_on_close", client->async_on_close());
            else
                co_await co_sync_hook_void("sock_subpub.on_close(connect fail)", [client]
                                           { client->on_close(); },
                                           nullptr);
            sock->close_connect();
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }

        // 连上了才交给业务，理由同 ws 侧（async_tcp_connect 还在动 sock 成员）
        client->set_conn(sock);

        retry_sec = 3;
        fprintf(stderr, "[sock_subpub] %s: connected %s:%u%s (bare TCP)\n", client->section_name().c_str(), cfg.host.c_str(), cfg.port, cfg.url.c_str());

        // 4. on_open
        if (client->is_coroutine_)
            co_await co_resident_hook_void("sock_subpub.async_on_open", client->async_on_open());
        else
            co_await co_sync_hook_void("sock_subpub.on_open", [client]
                                       { client->on_open(); },
                                       nullptr);

        // 5. pump async_read 循环（裸 TCP 原始字节，无 frame）
        std::string read_buf(4096, '\0');
        while (!client->isclose && !sock->iserror)
        {
            auto n = co_await sock->async_read(read_buf);
            if (n == 0)
                break;// 对端关了
            auto payload = std::string_view(read_buf.data(), n);

            if (client->is_coroutine_)
            {
                asio::co_spawn(io_ctx, [client, payload = std::string(payload)]() -> asio::awaitable<void>
                               {
                        try
                        {
                            co_await client->async_on_message(payload);
                        }
                        catch (...)
                        {
                            resident_hook_failed("sock_subpub.async_on_message",
                                                 std::current_exception(), false);
                        }
                        co_return; },
                               asio::detached);
            }
            else if (!post_conn_step("sock_subpub.on_message",
                                     [client, payload = std::string(payload)]
                                     {
                                         try
                                         {
                                             client->on_message(payload);
                                         }
                                         catch (...)
                                         {
                                             resident_hook_failed("sock_subpub.on_message",
                                                                  std::current_exception(),
                                                                  false);
                                         }
                                     }))
            {
                resident_hook_failed("sock_subpub.on_message", nullptr, true);
            }
        }

        // 6. 断连 → 钩子（还能读这一轮连接）→ 关 → 清 → backoff → 重连
        if (!client->isclose)
        {
            if (client->is_coroutine_)
                co_await co_resident_hook_void("sock_subpub.async_on_close", client->async_on_close());
            else
                co_await co_sync_hook_void("sock_subpub.on_close", [client]
                                           { client->on_close(); },
                                           nullptr);
        }
        sock->close_connect();
        client->set_conn(nullptr);
        if (client->isclose)
            break;
        auto wait = backoff();
        fprintf(stderr, "[sock_subpub] %s: pump exit → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
        asio::steady_timer t(io_ctx, wait);
        co_await t.async_wait(asio::use_awaitable);
    }
    co_return;
}
#endif

#ifdef ENABLE_MQTT_CLIENT
asio::awaitable<void> httpserver::async_mqtt_subpub_loop(
    std::shared_ptr<http::mqtt_subpub_client> client)
{
    auto &io_ctx  = this->io_context;
    int retry_sec = 3;
    auto backoff  = [&retry_sec]() -> std::chrono::seconds
    {
        auto v = std::chrono::seconds(retry_sec);
        if (retry_sec < 12)
            retry_sec += 3;
        return v;
    };

    while (!client->isclose)
    {
        // 1. 取配置
        auto sec = http::get_mqtt_config().get(client->section_name());
        if (!sec)
        {
            auto wait = backoff();
            fprintf(stderr, "[mqtt_subpub] %s: no section → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }
        auto &cfg = *sec;

        // 2. 创建 mqtt_client（出站 broker 连接）
        auto mqc         = std::make_shared<http::mqtt_client>(io_ctx, io_ctx);
        mqc->server_ioc_ = &io_ctx;
        mqc->timeout(cfg.timeout_sec);
        mqc->set_host(cfg.host);
        mqc->set_port(cfg.port);

        // 构造 CONNECT 参数
        http::mqtt_client_config_t mqtt_cfg;
        mqtt_cfg.client_id = cfg.clientid.empty() ? "mqtt_subpub_client" : cfg.clientid;
        mqtt_cfg.username  = cfg.username;
        mqtt_cfg.password  = cfg.password;
        mqtt_cfg.keepalive = cfg.keepalive;
        mqc->set_config(mqtt_cfg);

        client->io_ctx = &io_ctx;

        // 3. 注册回调：mqtt_client 收 PUBLISH 时派发业务钩子
        auto self               = client;
        mqc->async_run_loop_fun = [self, &io_ctx](std::shared_ptr<http::mqtt_client> mqc,
                                                  const http::mqtt_recv_packet_t &pkt)
            -> asio::awaitable<void>
        {
            if (self->isclose)
                co_return;
            if (self->is_coroutine_)
            {
                asio::co_spawn(io_ctx, [self, topic = pkt.topic, payload = pkt.payload, qos = pkt.qos]() -> asio::awaitable<void>
                               {
                        try
                        {
                            co_await self->async_on_message(topic, payload, qos);
                        }
                        catch (...)
                        {
                            resident_hook_failed("mqtt_subpub.async_on_message",
                                                 std::current_exception(), false);
                        }
                        co_return; },
                               asio::detached);
            }
            else if (!post_conn_step("mqtt_subpub.on_message",
                                     [self, topic = pkt.topic, payload = pkt.payload, qos = pkt.qos]
                                     {
                                         try
                                         {
                                             self->on_message(topic, payload, qos);
                                         }
                                         catch (...)
                                         {
                                             resident_hook_failed("mqtt_subpub.on_message",
                                                                  std::current_exception(),
                                                                  false);
                                         }
                                     }))
            {
                resident_hook_failed("mqtt_subpub.on_message", nullptr, true);
            }
            co_return;
        };

        // 4. async_tcp_connect + async_mqtt_connect（发 CONNECT → 等 CONNACK → 启 run_loop）
        if (!co_await mqc->async_tcp_connect())
        {
            auto wait = backoff();
            fprintf(stderr, "[mqtt_subpub] %s: async_tcp_connect fail err='%s' → wait %lld\n", client->section_name().c_str(), mqc->error_msg.c_str(), (long long)wait.count());
            if (client->is_coroutine_)
                co_await co_resident_hook_void("mqtt_subpub.async_on_close", client->async_on_close());
            else
                co_await co_sync_hook_void("mqtt_subpub.on_close(connect fail)", [client]
                                           { client->on_close(); },
                                           nullptr);
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }
        if (!co_await mqc->async_mqtt_connect())
        {
            auto wait = backoff();
            fprintf(stderr, "[mqtt_subpub] %s: async_mqtt_connect fail err='%s' → wait %lld\n", client->section_name().c_str(), mqc->error_msg.c_str(), (long long)wait.count());
            if (client->is_coroutine_)
                co_await co_resident_hook_void("mqtt_subpub.async_on_close", client->async_on_close());
            else
                co_await co_sync_hook_void("mqtt_subpub.on_close(connect fail)", [client]
                                           { client->on_close(); },
                                           nullptr);
            mqc->close_connect();
            asio::steady_timer t(io_ctx, wait);
            co_await t.async_wait(asio::use_awaitable);
            continue;
        }

        // 连上了才交给业务，理由同 ws/sock 侧
        client->set_conn(mqc);

        retry_sec = 3;
        fprintf(stderr, "[mqtt_subpub] %s: connected %s:%u (clientid=%s)\n", client->section_name().c_str(), cfg.host.c_str(), cfg.port, mqtt_cfg.client_id.c_str());

        // 5. on_open（业务可在此 subscribe）
        if (client->is_coroutine_)
            co_await co_resident_hook_void("mqtt_subpub.async_on_open", client->async_on_open());
        else
            co_await co_sync_hook_void("mqtt_subpub.on_open", [client]
                                       { client->on_open(); },
                                       nullptr);

        // 6. 等待连接断开：mqtt_client 的 run_loop 在 strand_ 上持续读包，
        //    我们在这里只需要周期性检查 iserror / connected_。
        //    run_loop_alive_ 是 atomic，断链时会置 false。
        while (!client->isclose && !mqc->iserror && mqc->run_loop_alive_)
        {
            asio::steady_timer t(io_ctx, std::chrono::milliseconds(500));
            co_await t.async_wait(asio::use_awaitable);
        }

        // 7. 断连 → 钩子 → 关 → 清 → backoff → 重连
        if (!client->isclose)
        {
            if (client->is_coroutine_)
                co_await co_resident_hook_void("mqtt_subpub.async_on_close", client->async_on_close());
            else
                co_await co_sync_hook_void("mqtt_subpub.on_close", [client]
                                           { client->on_close(); },
                                           nullptr);
        }
        mqc->close_connect();
        client->set_conn(nullptr);
        if (client->isclose)
            break;
        auto wait = backoff();
        fprintf(stderr, "[mqtt_subpub] %s: disconnected → wait %lld\n", client->section_name().c_str(), (long long)wait.count());
        asio::steady_timer t(io_ctx, wait);
        co_await t.async_wait(asio::use_awaitable);
    }
    co_return;
}
#endif

// ========== 限速入口辅助函数 ==========
// 按当前负载百分比算放行率：has_save_link_count ∈ [lo=300, hi=600] → pct ∈ [0,100] → rate ∈ [num1=20, num2=5]
// 线性从Level1到Level2递减
void httpserver::compute_rate_from_load(unsigned int has_save_link_count)
{
    unsigned int lo  = rate_limit_new_wait_num.load();
    unsigned int hi  = rate_limit_accept_wait_num.load();
    unsigned int span = (hi > lo) ? (hi - lo) : 1;
    unsigned int pct = (has_save_link_count > hi) ? 100
                    : (has_save_link_count > lo) ? (uint64_t)(has_save_link_count - lo) * 100 / span
                    : 0;
    unsigned int num1 = rate_limit_second_num1.load();
    unsigned int num2 = rate_limit_second_num2.load();
    unsigned int r = num1 - (num1 - num2) * pct / 100;
    rate_limit_second_time_num.store(r);
}

// 被限速连接入队：跟踪表 + 限速队列 + 叫醒 ratelimiter
void httpserver::enqueue_rate_limited(std::shared_ptr<client_session> peer)
{
    std::unique_lock<std::mutex> lk(socket_session_lists_mutex);
    socket_session_lists.push_back(std::weak_ptr<client_session>(peer));
    socket_session_wait_rate.push_back(peer);
    lk.unlock();
    rate_queue_count.fetch_add(1, std::memory_order_relaxed);
    rate_limited_count.fetch_add(1, std::memory_order_relaxed);
    rate_limit_condition.notify_one();
}

// ========== 限速线程 ==========
// 双层循环：外循环 5s wait_for（省电），内循环 200ms slot 高频放行
void httpserver::ratelimiter()
{
    unsigned int per_sec_passed  = 0;
    unsigned int per_slot_passed = 0;
    unsigned int pass_per_slot   = 0;
    unsigned int total_passed_5s = 0;
    unsigned int slot_frac_acc   = 0;   // 整数累积，单位 1/1000

    auto last_sec_tp  = std::chrono::steady_clock::now();
    auto last_log_tp  = last_sec_tp;
    auto last_slot_tp = last_sec_tp;

    std::unique_lock<std::mutex> lk(rate_limit_mutex);

    unsigned int limited_base = rate_limited_count.load(std::memory_order_relaxed);
    unsigned int expired_base = rate_expired_count.load(std::memory_order_relaxed);

    // 队列排空之后不再有这一拍，所以统计块放内循环里：风暴持续期间照样 5 秒一条。
    // force=true 用于排空那一刻收尾，把这一批放行数记掉。
    auto flush_rate_log = [&](bool force)
    {
        auto now = std::chrono::steady_clock::now();
        if (!force && now - last_log_tp < std::chrono::seconds(5))
        {
            return;
        }
        last_log_tp = now;

        unsigned int qsz;
        {
            std::unique_lock<std::mutex> lk2(socket_session_lists_mutex);
            qsz = (unsigned int)socket_session_wait_rate.size();
        }
        unsigned int limited_now   = rate_limited_count.load(std::memory_order_relaxed);
        unsigned int limited_delta = limited_now - limited_base;
        limited_base = limited_now;
        unsigned int expired_now   = rate_expired_count.load(std::memory_order_relaxed);
        unsigned int expired_delta = expired_now - expired_base;
        expired_base = expired_now;

        if (qsz > 0 || total_passed_5s > 0 || limited_delta > 0)
        {
            std::string logtemp = "ratelimiter: queue=";
            logtemp.append(std::to_string(qsz));
            logtemp.append(" passed_5s=");
            logtemp.append(std::to_string(total_passed_5s));
            logtemp.append(" limited_5s=");
            logtemp.append(std::to_string(limited_delta));
            logtemp.append(" expired_5s=");
            logtemp.append(std::to_string(expired_delta));
            logtemp.append(" rate=");
            logtemp.append(std::to_string(rate_limit_second_time_num.load()));
            logtemp.append("\n");
            std::unique_lock<std::mutex> lk3(log_mutex);
            error_loglist.emplace_back(logtemp);
            lk3.unlock();
        }
        total_passed_5s = 0;
    };

    while (!isstop)
    {
        rate_limit_condition.wait_for(lk, std::chrono::seconds(5),
            [this] { return isstop || rate_queue_count.load(std::memory_order_relaxed) > 0; });
        if (isstop) break;

        // ===== 内循环：200ms slot 放行，队空回外循环 =====
        while (!isstop)
        {
            auto inow = std::chrono::steady_clock::now();

            if (inow - last_slot_tp >= std::chrono::milliseconds(200))
            {
                auto elapsed_ms = (unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                                      inow - last_slot_tp).count();
                last_slot_tp = inow;
                per_slot_passed = 0;

                if (inow - last_sec_tp >= std::chrono::seconds(1))
                {
                    last_sec_tp = inow;
                    per_sec_passed = 0;
                }

                // 整数累积：按实际流逝时间加 rate*elapsed_ms，够 1000 放 1 个
                unsigned int cur_rate = rate_limit_second_time_num.load();
                slot_frac_acc += cur_rate * elapsed_ms;
                pass_per_slot  = slot_frac_acc / 1000;
                slot_frac_acc %= 1000;
            }

            bool did_pass = false;
            while (per_slot_passed < pass_per_slot &&
                   per_sec_passed  < rate_limit_second_time_num.load())
            {
                std::shared_ptr<client_session> peer = nullptr;
                {
                    std::unique_lock<std::mutex> lk2(socket_session_lists_mutex);
                    if (socket_session_wait_rate.empty()) break;
                    peer = std::move(socket_session_wait_rate.front());
                    socket_session_wait_rate.pop_front();
                }
                if (!peer) break;
                rate_queue_count.fetch_sub(1, std::memory_order_relaxed);

                // 排队期间没人刷新 time_limit，超过 76 秒上下 httpwatch 会把它关掉；
                // 这里只按原子量租约记一笔账，放行行为不变（照旧 co_spawn）。
                if (peer->time_limit.load() < timeid())
                {
                    rate_expired_count.fetch_add(1, std::memory_order_relaxed);
                }

                if (peer->isssl)
                    asio::co_spawn(peer->strand_,
                        [peer, this]() mutable { return sslhandshake(peer); }, asio::detached);
                else
                    asio::co_spawn(peer->strand_,
                        [peer, this]() mutable { return clientpeerfun(peer, false); }, asio::detached);

                per_slot_passed++;
                per_sec_passed++;
                total_passed_5s++;
                did_pass = true;
            }

            bool queue_empty = false;
            {
                std::unique_lock<std::mutex> lk2(socket_session_lists_mutex);
                queue_empty = socket_session_wait_rate.empty();
            }
            if (queue_empty)
            {
                flush_rate_log(true);
                break;
            }
            flush_rate_log(false);

            if (!did_pass)
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
}
void httpserver::listeners()
{
#ifdef __APPLE__
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif
    serverconfig &sysconfigpath = getserversysconfig();
    unsigned short portnum      = sysconfigpath.get_ssl_port();

    asio::error_code ec_error;
    // 监听对象放进共享所有权：stop() 要在别的线程上拿到同一个 acceptor 才谈得上 cancel()/close()。
    // 只缓存 fd 不行——close 之后那个号会被别的线程立刻复用，二次 close 打到新 socket；
    // 而 Windows 下 closesocket 也唤醒不了挂在 accept() 上的线程（asio 的 close 就是 closesocket）。
    // 本函数内仍按普通对象用。
    auto acceptor_ptr                 = std::make_shared<asio::ip::tcp::acceptor>(this->io_context);
    asio::ip::tcp::acceptor &acceptor = *acceptor_ptr;
    asio::ip::tcp::endpoint endpoint;
    if (server_ip6_listen)
    {
        endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v6(), portnum);
        acceptor.open(endpoint.protocol());
        acceptor.set_option(asio::ip::v6_only(false));
    }
    else
    {
        endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), portnum);
        acceptor.open(endpoint.protocol());
    }

    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.set_option(asio::ip::tcp::no_delay(true));
#if (defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)) && !defined(__CYGWIN__)

    typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
    acceptor.set_option(reuse_port(true));

#endif

    acceptor.bind(endpoint, ec_error);
    acceptor.listen(asio::socket_base::max_listen_connections, ec_error);
    if (ec_error)
    {
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back(" acceptor listen https error ");
        lock.unlock();
        DEBUG_LOG("Acceptor listen https error ");
        exit(1);
    }

    // 注册 acceptor 对象，供 stop() 唤醒挂起的 accept() 并关掉监听
    {
        std::lock_guard<std::mutex> lock(acceptors_mutex);
        acceptors.push_back(acceptor_ptr);
    }

    asio::ssl::context context_(asio::ssl::context::sslv23);
    context_.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 |
                         asio::ssl::context::single_dh_use);
    context_.set_password_callback(std::bind(get_password));

    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_AUTO_RETRY);
    try
    {
        context_.use_certificate_chain_file(sysconfigpath.ssl_chain_file());
        context_.use_private_key_file(sysconfigpath.ssl_key_file(), asio::ssl::context::pem);
        // use_certificate_file
        context_.use_tmp_dh_file(sysconfigpath.ssl_dh_file());
        context_.use_certificate_file(sysconfigpath.ssl_chain_crt_file(), asio::ssl::context::pem);
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Chain file error:%s", e.what());
    }
    SSL_CTX_set_tlsext_servername_callback(context_.native_handle(), serverNameCallback);

    auto ssl_opts = (SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS) | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                    SSL_OP_NO_COMPRESSION | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_SINGLE_ECDH_USE |
                    SSL_OP_NO_TICKET | SSL_OP_CIPHER_SERVER_PREFERENCE;
    SSL_CTX_set_options(context_.native_handle(), ssl_opts);
    // SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_AUTO_RETRY);
    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_RELEASE_BUFFERS);

    if (SSL_CTX_set_cipher_list(context_.native_handle(), DEFAULT_CIPHER_LIST) == 0)
    {
        // std::cerr << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back(ERR_error_string(ERR_get_error(), nullptr));
        lock.unlock();
    }

    unsigned int protos_len;
    const char *protos;

    protos     = HTTP2_H2_ALPN HTTP1_NPN;
    protos_len = sizeof(HTTP2_H2_ALPN HTTP1_NPN) - 1;

    if (SSL_CTX_set_alpn_protos(context_.native_handle(), (const unsigned char *)protos, protos_len) < 0)
    {
    }

    SSL_CTX_set_next_protos_advertised_cb(context_.native_handle(), next_proto_cb, (void *)HTTP2_H2H1_STR);
    unsigned long long temp_domain = 0;
    SSL_CTX_set_alpn_select_cb(context_.native_handle(), alpn_cb, (void *)temp_domain);

    // 在默认 SSL_CTX 上注册最小化 OCSP Stapling 回调
    // OpenSSL 3.x 的 tls_construct_certificate_status 要求 tlsext_status_cb 存在
    // 才会发送 OCSP response。实际 OCSP 响应由 SNI 回调设置。
    sysconfigpath.enable_ocsp_stapling(context_.native_handle());

    unsigned int error_count            = 0;
    unsigned int clear_error_count_time = 0;

    unsigned int time_num_count[60];
    unsigned int time_record[60];
    unsigned int time_head           = 0;
    unsigned int time_tail           = 0;
    unsigned int has_save_link_count = 0;

    std::memset(time_num_count, 0, sizeof(time_num_count));
    std::memset(time_record, 0, sizeof(time_record));
    std::string logtemp;

    for (;;)
    {
        try
        {
            for (;;)
            {
                std::shared_ptr<client_session> peer_session = std::make_shared<client_session>(this->io_context);
                asio::ip::tcp::socket socket(peer_session->strand_);
                peer_session->sslsocket = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(socket), context_);
                peer_session->isssl     = true;
                peer_session->time_limit.store(timeid() + 16);

                acceptor.accept(peer_session->sslsocket->lowest_layer(), ec_error);
                if (ec_error)
                {
                    // stop() 已经 cancel + shutdown + close 过监听，accept 返回错误 → 退出
                    if (isstop)
                    {
                        break;
                    }
                    logtemp = "https accept ec_error ";
                    logtemp.append(ec_error.message());
                    logtemp.append(" ");
                    logtemp.append(std::to_string(error_count));
                    logtemp.append("\n");
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(logtemp);
                    lock.unlock();

                    if ((error_count % 6) > 4)
                    {
                        hard_kill_old_link = true;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(2));

                    error_count++;
                    if (error_count > 128)
                    {
                        isstop = true;
                    }

                    peer_session->stop();

                    if ((clear_error_count_time + CONST_ERROR_COUNT_TIME) < timeid())
                    {
                        clear_error_count_time = timeid();
                        error_count            = 0;
                    }
                    continue;
                }
                //The IP should be available now
                peer_session->getremoteip();
                if (peer_session->isclose)
                {
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(peer_session->client_ip);
                    error_loglist.emplace_back(" https accept client close\n");
                    lock.unlock();
                    continue;
                }
                peer_session->getremoteport();
                if (peer_session->isclose)
                {
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(peer_session->client_ip);
                    error_loglist.emplace_back(" https accept client close\n");
                    lock.unlock();
                    continue;
                }

                //begin rate limiting
                unsigned int sp_time     = timeid();
                peer_session->time_begin = sp_time;
                total_http2_count++;

#ifndef BENCHMARK

                if (rate_limit_status)
                {
                    unsigned int cur_offset = time_head;

                    for (; cur_offset != time_tail;)
                    {
                        if (sp_time - time_record[cur_offset] > 60)
                        {
                            cur_offset = (cur_offset + 1) % 60;
                            continue;
                        }
                        break;
                    }
                    time_head = cur_offset;

                    //update time_tail num
                    if (sp_time > time_record[time_tail])
                    {
                        time_tail                 = (time_tail + 1) % 60;
                        time_record[time_tail]    = sp_time;
                        time_num_count[time_tail] = total_count.load();
                    }
                    else
                    {
                        time_num_count[time_tail] = total_count.load();
                    }
                    has_save_link_count = time_num_count[time_tail] - time_num_count[time_head];
                    http2_minute_count.store(has_save_link_count);
                    if (has_save_link_count > rate_limit_new_wait_num.load() && live_link_count.load() > rate_limit_new_wait_num.load())
                    {
                        compute_rate_from_load(has_save_link_count);
                        enqueue_rate_limited(peer_session);
                        continue;
                    }
                }

                std::unique_lock<std::mutex> lock_sock(socket_session_lists_mutex);
                socket_session_lists.push_back(peer_session);
                lock_sock.unlock();
#endif

                
                asio::co_spawn(peer_session->strand_, [peer_session, this]() mutable
                               { return sslhandshake(peer_session); },
                               asio::detached);
                if (isstop)
                {
                    break;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(e.what());
            error_loglist.emplace_back(" https accept exception\n");
            lock.unlock();
        }
        catch (...)
        {
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back("https accept catch\n");
            lock.unlock();
        }
        if (isstop)
        {
            break;
        }
    }
}
void httpserver::listener()
{
#ifdef __APPLE__
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif
    serverconfig &sysconfigpath = getserversysconfig();
    asio::error_code ec_error;

    unsigned short portnum = sysconfigpath.get_port();

    // 同 listeners()：acceptor 由 shared_ptr 持有，stop() 在别的线程上拿同一个对象唤醒并关闭
    auto acceptor_ptr                 = std::make_shared<asio::ip::tcp::acceptor>(this->io_context);
    asio::ip::tcp::acceptor &acceptor = *acceptor_ptr;
    asio::ip::tcp::endpoint endpoint;
    if (server_ip6_listen)
    {
        endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v6(), portnum);
        acceptor.open(endpoint.protocol());
        acceptor.set_option(asio::ip::v6_only(false));
    }
    else
    {
        endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), portnum);
        acceptor.open(endpoint.protocol());
    }

    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

#if (defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)) && !defined(__CYGWIN__)

    typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
    acceptor.set_option(reuse_port(true));

#endif

    acceptor.bind(endpoint, ec_error);
    acceptor.listen(asio::socket_base::max_listen_connections, ec_error);
    if (ec_error)
    {
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back("  acceptor listen http error  ");
        lock.unlock();
        DEBUG_LOG("Acceptor listen http error ");
        exit(1);
    }

    // 注册 acceptor 对象，供 stop() 唤醒挂起的 accept() 并关掉监听
    {
        std::lock_guard<std::mutex> lock(acceptors_mutex);
        acceptors.push_back(acceptor_ptr);
    }

    DEBUG_LOG("http accept");
    unsigned int error_count            = 0;
    unsigned int clear_error_count_time = 0;

    unsigned int time_num_count[60];
    unsigned int time_record[60];
    unsigned int time_head           = 0;
    unsigned int time_tail           = 0;
    unsigned int has_save_link_count = 0;

    std::memset(time_num_count, 0, sizeof(time_num_count));
    std::memset(time_record, 0, sizeof(time_record));
    std::string logtemp;

    for (;;)
    {
        try
        {
            for (;;)
            {
                std::shared_ptr<client_session> peer_session = std::make_shared<client_session>(this->io_context);
                peer_session->socket                         = std::make_unique<asio::ip::tcp::socket>(peer_session->strand_);
                peer_session->isssl                          = false;
                peer_session->time_limit.store(timeid() + 16);

                acceptor.accept(*peer_session->socket, ec_error);
                if (ec_error)
                {
                    // stop() 已经 cancel + shutdown + close 过监听，accept 返回错误 → 退出
                    if (isstop)
                    {
                        break;
                    }
                    logtemp = "http accept ec_error ";
                    logtemp.append(ec_error.message());
                    logtemp.append(" ");
                    logtemp.append(std::to_string(error_count));
                    logtemp.append("\n");
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(logtemp);
                    lock.unlock();

                    if ((error_count % 6) > 4)
                    {
                        hard_kill_old_link = true;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(2));

                    error_count++;
                    if (error_count > 128)
                    {
                        isstop = true;
                    }

                    peer_session->stop();

                    if ((clear_error_count_time + CONST_ERROR_COUNT_TIME) < timeid())
                    {
                        clear_error_count_time = timeid();
                        error_count            = 0;
                    }
                    continue;
                }

                //The IP should be available now
                peer_session->getremoteip();
                if (peer_session->isclose)
                {
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(peer_session->client_ip);
                    error_loglist.emplace_back(" http accept client close\n");
                    lock.unlock();
                    continue;
                }
                peer_session->getremoteport();
                if (peer_session->isclose)
                {
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(peer_session->client_ip);
                    error_loglist.emplace_back(" http accept client close\n");
                    lock.unlock();
                    continue;
                }

                //begin rate limiting
                unsigned int sp_time     = timeid();
                peer_session->time_begin = sp_time;
                total_http1_count++;

#ifndef BENCHMARK

                if (rate_limit_status)
                {
                    unsigned int cur_offset = time_head;

                    for (; cur_offset != time_tail;)
                    {
                        if (sp_time - time_record[cur_offset] > 60)
                        {
                            cur_offset = (cur_offset + 1) % 60;
                            continue;
                        }
                        break;
                    }
                    time_head = cur_offset;

                    //update time_tail num
                    if (sp_time > time_record[time_tail])
                    {
                        time_tail                 = (time_tail + 1) % 60;
                        time_record[time_tail]    = sp_time;
                        time_num_count[time_tail] = total_count.load();
                    }
                    else
                    {
                        time_num_count[time_tail] = total_count.load();
                    }
                    has_save_link_count = time_num_count[time_tail] - time_num_count[time_head];

                    if (has_save_link_count > rate_limit_new_wait_num.load() && live_link_count.load() > rate_limit_new_wait_num.load())
                    {
                        compute_rate_from_load(has_save_link_count);
                        enqueue_rate_limited(peer_session);
                        continue;
                    }
                }

                std::unique_lock<std::mutex> lock_sock(socket_session_lists_mutex);
                socket_session_lists.push_back(peer_session);
                lock_sock.unlock();
#endif

                asio::co_spawn(peer_session->strand_, [peer_session, this]() mutable
                               { return clientpeerfun(peer_session, false); },
                               asio::detached);
                if (isstop)
                {
                    break;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(e.what());
            error_loglist.emplace_back(" http accept exception\n");
            lock.unlock();
        }
        catch (...)
        {
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back("http accept catch\n");
            lock.unlock();
        }
        if (isstop)
        {
            break;
        }
    }
}

void httpserver::add_runsocketthread()
{
    runthreads.emplace_back(
        [self = this]()
        {
            std::unique_lock<std::mutex> lock(self->log_mutex);
            self->error_loglist.emplace_back(" add socket thread ");
            lock.unlock();

            self->io_context.run();
        });
}

// ==================== httpwatch sub-methods ====================

void httpserver::httpwatch_init_paths(std::string &currentpath, std::string &error_path, std::string &traffic_switch_file, std::string &restart_file, std::string &restart_ssl_file, std::string &orm_log_file)
{
    serverconfig &sysconfigpath = getserversysconfig();

    // ORM init
    try
    {
        currentpath = sysconfigpath.configpath;
        if (currentpath.size() > 0 && currentpath.back() != '/')
        {
            currentpath.push_back('/');
        }
        currentpath.append("orm.conf");
        orm_log_file = orm::init_orm_conn_pool(io_context, currentpath);
        error_loglist.emplace_back(orm_log_file);
        orm_log_file.clear();
    }
    catch (const char *e)
    {
        std::string errorstr(e);
        errorstr.push_back('\n');
        error_loglist.emplace_back(errorstr);
    }

    server_loaclvar &static_server_var = get_server_global_var();
    currentpath                        = static_server_var.log_path;

    if (currentpath.size() > 0 && currentpath.back() != '/')
    {
        currentpath.push_back('/');
    }
    error_path          = currentpath;
    traffic_switch_file = currentpath;
    restart_file        = currentpath;
    restart_ssl_file    = currentpath;
    orm_log_file        = currentpath;

    currentpath.append("access.log");
    error_path.append("error.log");
    traffic_switch_file.append("traffic_switch_file");
    restart_file.append("restart_server");
    restart_ssl_file.append("restart_ssl_config");
    orm_log_file.append("orm_debug.log");

    rate_limit_new_wait_num    = sysconfigpath.rate_limit_new_wait_num;
    rate_limit_accept_wait_num = sysconfigpath.rate_limit_accept_wait_num;
    rate_limit_second_num1     = sysconfigpath.rate_limit_second_num1;
    rate_limit_second_num2     = sysconfigpath.rate_limit_second_num2;

    // 负载插值走 unsigned 的 (num1 - num2)，num2 必须严格低于 num1，否则回绕成巨大的放行率
    if (rate_limit_second_num2 >= rate_limit_second_num1)
    {
        rate_limit_second_num2 = rate_limit_second_num1 - 1;
    }

    if (rate_limit_second_num2 < 1)
    {
        rate_limit_second_num2 = 1;
    }
}

void httpserver::httpwatch_parse_reboot_cron(unsigned char &cron_type, unsigned char &cron_day, unsigned char &cron_hour)
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.map_value["default"]["reboot_cron"].size() > 1)
    {
        if (sysconfigpath.map_value["default"]["reboot_cron"][0] == 'M' || sysconfigpath.map_value["default"]["reboot_cron"][0] == 'm')
        {
            cron_type = 'm';
        }
        else if (sysconfigpath.map_value["default"]["reboot_cron"][0] == 'D' || sysconfigpath.map_value["default"]["reboot_cron"][0] == 'd')
        {
            cron_type = 'd';
        }
        else if (sysconfigpath.map_value["default"]["reboot_cron"][0] == 'W' || sysconfigpath.map_value["default"]["reboot_cron"][0] == 'w')
        {
            cron_type = 'w';
        }
        else if (sysconfigpath.map_value["default"]["reboot_cron"][0] == 'S' || sysconfigpath.map_value["default"]["reboot_cron"][0] == 's')
        {
            cron_type = 's';
        }
        if (cron_type != 0x00)
        {
            for (size_t i = 1; i < sysconfigpath.map_value["default"]["reboot_cron"].size(); ++i)
            {
                if (sysconfigpath.map_value["default"]["reboot_cron"][i] >= '0' && sysconfigpath.map_value["default"]["reboot_cron"][i] <= '9')
                {
                    cron_day = cron_day * 10 + (sysconfigpath.map_value["default"]["reboot_cron"][i] - '0');
                }
                else
                {
                    if (sysconfigpath.map_value["default"]["reboot_cron"][i] == 'H' || sysconfigpath.map_value["default"]["reboot_cron"][i] == 'h')
                    {
                        for (size_t j = i + 1; j < sysconfigpath.map_value["default"]["reboot_cron"].size(); ++j)
                        {
                            if (sysconfigpath.map_value["default"]["reboot_cron"][j] >= '0' && sysconfigpath.map_value["default"]["reboot_cron"][j] <= '9')
                            {
                                cron_hour = cron_hour * 10 + (sysconfigpath.map_value["default"]["reboot_cron"][j] - '0');
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            if (cron_day == 0)
            {
                cron_day = 1;
            }

            if (cron_type == 'w')
            {
                cron_day = cron_day % 8;
            }
            else if (cron_type == 's')
            {
                cron_day = cron_day % 32;
            }
            else if (cron_type == 'm')
            {
                cron_day = cron_day % 32;
            }
        }
    }
}

void httpserver::httpwatch_parse_clean_cron(unsigned int &clean_cron_min, unsigned int &clean_cron_time_ago)
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.map_value["default"]["clean_cron"].size() > 3)
    {
        clean_cron_min = 0;
        if (sysconfigpath.map_value["default"]["clean_cron"][0] == 'M' || sysconfigpath.map_value["default"]["clean_cron"][0] == 'm')
        {
            for (size_t i = 1; i < sysconfigpath.map_value["default"]["clean_cron"].size(); ++i)
            {
                if (sysconfigpath.map_value["default"]["clean_cron"][i] >= '0' && sysconfigpath.map_value["default"]["clean_cron"][i] <= '9')
                {
                    clean_cron_min = clean_cron_min * 10 + (sysconfigpath.map_value["default"]["clean_cron"][i] - '0');
                }
                else
                {
                    if (sysconfigpath.map_value["default"]["clean_cron"][i] == 'T' || sysconfigpath.map_value["default"]["clean_cron"][i] == 't')
                    {
                        for (size_t j = i + 1; j < sysconfigpath.map_value["default"]["clean_cron"].size(); ++j)
                        {
                            if (sysconfigpath.map_value["default"]["clean_cron"][j] >= '0' && sysconfigpath.map_value["default"]["clean_cron"][j] <= '9')
                            {
                                clean_cron_time_ago = clean_cron_time_ago * 10 + (sysconfigpath.map_value["default"]["clean_cron"][j] - '0');
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            clean_cron_min = clean_cron_min * 60;
            if (clean_cron_min < 300)
            {
                clean_cron_min = 300;
            }

            clean_cron_min = clean_cron_min / 5;

            if (clean_cron_time_ago < 320)
            {
                clean_cron_time_ago = 320;
            }
            if (clean_cron_time_ago > 604800)
            {
                clean_cron_time_ago = 604800;
            }
        }
    }
}

void httpserver::httpwatch_parse_temp_clean(unsigned int &temp_live_time)
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.map_value["default"]["temp_clean_time"].size() > 0)
    {
        unsigned long long temp_value = 0;
        if (http::str2uint64_strict(sysconfigpath.map_value["default"]["temp_clean_time"], temp_value, 10))
        {
            if (temp_value < 60)
            {
                temp_value = 60;
            }
            if (temp_value > 604800)
            {
                temp_value = 604800;
            }
            temp_live_time = static_cast<unsigned int>(temp_value);
        }
    }
}

void httpserver::httpwatch_parse_links_restart(unsigned int &restart_process_num,
                                               int &restart_process_time_start,
                                               int &restart_process_time_end)
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.map_value["default"]["links_restart_process"].size() > 3)
    {
        if (sysconfigpath.map_value["default"]["links_restart_process"][0] == 'N' || sysconfigpath.map_value["default"]["links_restart_process"][0] == 'n')
        {
            for (size_t i = 1; i < sysconfigpath.map_value["default"]["links_restart_process"].size(); ++i)
            {
                if (sysconfigpath.map_value["default"]["links_restart_process"][i] >= '0' && sysconfigpath.map_value["default"]["links_restart_process"][i] <= '9')
                {
                    restart_process_num = restart_process_num * 10 + (sysconfigpath.map_value["default"]["links_restart_process"][i] - '0');
                }
                else
                {
                    if (sysconfigpath.map_value["default"]["links_restart_process"][i] == 'T' || sysconfigpath.map_value["default"]["links_restart_process"][i] == 't')
                    {
                        i++;
                        if (i < sysconfigpath.map_value["default"]["links_restart_process"].size())
                        {
                            if (sysconfigpath.map_value["default"]["links_restart_process"][i] == 's' || sysconfigpath.map_value["default"]["links_restart_process"][i] == 'S')
                            {
                                for (i = i + 1; i < sysconfigpath.map_value["default"]["links_restart_process"].size(); i++)
                                {
                                    if (sysconfigpath.map_value["default"]["links_restart_process"][i] >= '0' && sysconfigpath.map_value["default"]["links_restart_process"][i] <= '9')
                                    {
                                        restart_process_time_start = restart_process_time_start * 10 + (sysconfigpath.map_value["default"]["links_restart_process"][i] - '0');
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                        if (i < sysconfigpath.map_value["default"]["links_restart_process"].size())
                        {
                            if (sysconfigpath.map_value["default"]["links_restart_process"][i] == 'T' || sysconfigpath.map_value["default"]["links_restart_process"][i] == 't')
                            {
                                i++;
                                if (i < sysconfigpath.map_value["default"]["links_restart_process"].size())
                                {
                                    if (sysconfigpath.map_value["default"]["links_restart_process"][i] == 'E' || sysconfigpath.map_value["default"]["links_restart_process"][i] == 'e')
                                    {
                                        for (i = i + 1; i < sysconfigpath.map_value["default"]["links_restart_process"].size(); i++)
                                        {
                                            if (sysconfigpath.map_value["default"]["links_restart_process"][i] >= '0' && sysconfigpath.map_value["default"]["links_restart_process"][i] <= '9')
                                            {
                                                restart_process_time_end = restart_process_time_end * 10 + (sysconfigpath.map_value["default"]["links_restart_process"][i] - '0');
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
            restart_process_time_start = restart_process_time_start % 24;
            restart_process_time_end   = restart_process_time_end % 24;

            if (restart_process_num < 10000)
            {
                if (restart_process_num > 0)
                {
                    restart_process_num = 10000;
                }
            }
        }
    }
}

void httpserver::httpwatch_adjust_thread_pool(unsigned int &updatetimetemp)
{
    if (clientrunpool.gettasknum() > 1)
    {
        DEBUG_LOG("add thread:%d", clientrunpool.gettasknum());
        clientrunpool.addthread(3);
        updatetimetemp = 0;
    }
    else if (clientrunpool.getlivenum() < 32)
    {
        DEBUG_LOG("fix thread:%d", clientrunpool.getlivenum());
        updatetimetemp += 1;
        if (updatetimetemp % 19 == 0)
        {
            clientrunpool.fixthread();
            updatetimetemp = 0;
        }
    }
}

void httpserver::httpwatch_memory_monitor(unsigned int mysqlpool_time)
{
    if ((mysqlpool_time % 2) == 0)
    {
        unsigned long long self_rss_num    = get_rss_kb();
        unsigned long long total_memory_kb = get_total_memory_kb();

        DEBUG_LOG("memory info:total:%llu process:%llu, live:%d", total_memory_kb, self_rss_num, live_link_count.load());
        if (self_rss_num > (total_memory_kb / 2))
        {
            std::string temp_l = "-- memory info -- time:";
            temp_l.append(get_date("%Y-%m-%d %X"));
            temp_l.append(" total:");
            temp_l.append(std::to_string(total_memory_kb));
            temp_l.append(" process:");
            temp_l.append(std::to_string(self_rss_num));
            temp_l.append(" live:");
            temp_l.append(std::to_string(live_link_count.load()));
            temp_l.append("\n");

            std::unique_lock<std::mutex> logmilock(log_mutex);
            error_loglist.push_back(temp_l);
            logmilock.unlock();
        }

        if (total_memory_kb > 100)
        {
            if (self_rss_num > ((total_memory_kb / 100) * 75))
            {
                unsigned int nowtimeid = timeid() - 180;
                std::unique_lock<std::mutex> lock_sock_c(socket_session_lists_mutex);

                for (auto iter = socket_session_lists.begin(); iter != socket_session_lists.end();)
                {
                    std::shared_ptr<client_session> p_session = iter->lock();
                    if (p_session)
                    {
                        if (p_session->time_limit.load() < nowtimeid)
                        {
                            DEBUG_LOG("clear nowtimeid pre session");
                            asio::co_spawn(p_session->strand_, clientpeerstop(p_session), asio::detached);
                            if (p_session->half_close)
                            {
                                socket_session_lists.erase(iter++);
                            }
                            else
                            {
                                ++iter;
                            }
                        }
                        else
                        {
                            ++iter;
                        }
                    }
                    else
                    {
                        ++iter;
                    }
                }
                lock_sock_c.unlock();
            }
        }
    }
}

void httpserver::httpwatch_flush_access_log(const std::string &access_path)
{
    if (!access_loglist.empty())
    {
#ifndef _MSC_VER
#ifndef _WIN32
        struct flock lockstr = {};
#endif
        int fd = open(access_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd == -1)
        {
            return;
        }

#ifndef _WIN32
        lockstr.l_type   = F_WRLCK;
        lockstr.l_whence = SEEK_END;
        lockstr.l_start  = 0;
        lockstr.l_len    = 0;
        lockstr.l_pid    = 0;

        if (fcntl(fd, F_SETLK, &lockstr) == -1)
        {
            close(fd);
            return;
        }
#else
        auto native_handle = (HANDLE)_get_osfhandle(fd);
        auto file_size     = GetFileSize(native_handle, nullptr);
        if (!LockFile(native_handle, file_size, 0, file_size, 0))
        {
            close(fd);
            return;
        }
#endif
        std::unique_lock<std::mutex> logacclock(log_mutex);
        std::size_t n_write = 0;
        while (!access_loglist.empty())
        {
            n_write = write(fd, access_loglist.front().data(), access_loglist.front().size());
            access_loglist.pop_front();
            if (n_write < 1)
            {
                access_loglist.clear();
                break;
            }
        }
        logacclock.unlock();

#ifndef _WIN32
        lockstr.l_type = F_UNLCK;
        if (fcntl(fd, F_SETLK, &lockstr) == -1)
        {
            close(fd);
            return;
        }
#else
        if (!UnlockFile(native_handle, file_size, 0, file_size, 0))
        {
            close(fd);
            return;
        }
#endif
        close(fd);
#endif
    }
}

void httpserver::httpwatch_flush_error_log(const std::string &error_path)
{
    if (!error_loglist.empty())
    {
#ifndef _MSC_VER
#ifndef _WIN32
        struct flock lockstr = {};
#endif
        int fd = open(error_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd == -1)
        {
            return;
        }

#ifndef _WIN32
        lockstr.l_type   = F_WRLCK;
        lockstr.l_whence = SEEK_END;
        lockstr.l_start  = 0;
        lockstr.l_len    = 0;
        lockstr.l_pid    = 0;

        if (fcntl(fd, F_SETLK, &lockstr) == -1)
        {
            close(fd);
            return;
        }
#else
        auto native_handle = (HANDLE)_get_osfhandle(fd);
        auto file_size     = GetFileSize(native_handle, nullptr);
        if (!LockFile(native_handle, file_size, 0, file_size, 0))
        {
            close(fd);
            return;
        }
#endif

        std::unique_lock<std::mutex> logerrlock(log_mutex);
        while (!error_loglist.empty())
        {
            std::size_t n_write = write(fd, error_loglist.front().data(), error_loglist.front().size());
            error_loglist.pop_front();
            (void)n_write;
        }
        logerrlock.unlock();

#ifndef _WIN32
        lockstr.l_type = F_UNLCK;
        if (fcntl(fd, F_SETLK, &lockstr) == -1)
        {
            close(fd);
            return;
        }
#else
        if (!UnlockFile(native_handle, file_size, 0, file_size, 0))
        {
            close(fd);
            return;
        }
#endif
        close(fd);
#endif
    }
}

void httpserver::httpwatch_mysql_pool_maintenance(unsigned int &mysqlpool_time, const std::tm *now, unsigned int old_total_count)
{
    // Clear idle connections at 2am when traffic is low
    if (now->tm_min == 2 && now->tm_sec < 9)
    {
        if (total_count.load() == old_total_count)
        {
            std::map<std::string, std::shared_ptr<orm::orm_conn_pool>> &mysqldbpoolglobal = orm::get_orm_conn_pool_obj();
            for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
            {
                iter->second->clear_select_conn();
                DEBUG_LOG("mysql pool clearpoool ");
            }
        }
    }

    // Force clear every 2 days at 3am
    if (now->tm_hour < 3 && mysqlpool_time > 172800)
    {
        std::map<std::string, std::shared_ptr<orm::orm_conn_pool>> &mysqldbpoolglobal = orm::get_orm_conn_pool_obj();
        for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
        {
            iter->second->clear_select_conn();
            DEBUG_LOG("mysql pool clearpoool ");
        }
        mysqlpool_time = 1;
    }
}

void httpserver::httpwatch_check_cron_reboot(unsigned char cron_type, unsigned char cron_day, unsigned char cron_hour, const std::tm *now)
{
    if (cron_type > 0 && cron_day > 0 && cron_hour > 0 && now->tm_min < 3)
    {
        bool should_stop = false;
        if (cron_type == 'd')
        {
            if ((now->tm_yday + 1) % cron_day == 0)
            {
                if (cron_hour > 0 && now->tm_hour == cron_hour)
                {
                    should_stop = true;
                }
            }
        }
        else if (cron_type == 'm')
        {
            if (now->tm_mday == cron_day)
            {
                if (cron_hour > 0 && now->tm_hour == cron_hour)
                {
                    should_stop = true;
                }
            }
        }
        else if (cron_type == 'w')
        {
            if (cron_day == 7 && 0 == now->tm_wday)
            {
                if (cron_hour > 0 && now->tm_hour == cron_hour)
                {
                    should_stop = true;
                }
            }
            else if (now->tm_wday == cron_day)
            {
                if (cron_hour > 0 && now->tm_hour == cron_hour)
                {
                    should_stop = true;
                }
            }
        }
        else if (cron_type == 's')
        {
            if (now->tm_mon == 0 || now->tm_mon == 3 || now->tm_mon == 6 || now->tm_mon == 9)
            {
                if (now->tm_mday == cron_day)
                {
                    if (cron_hour > 0 && now->tm_hour == cron_hour)
                    {
                        should_stop = true;
                    }
                }
            }
        }
        if (should_stop)
        {
            std::string logstr = "--- server restart ";
            logstr.append(std::to_string(now->tm_mon + 1));
            logstr.push_back('-');
            logstr.append(std::to_string(now->tm_mday));
            logstr.push_back(0x20);
            logstr.append(std::to_string(now->tm_hour));
            logstr.push_back(0x20);
            logstr.push_back(cron_type);
            logstr.append(std::to_string(cron_day));
            logstr.push_back('h');
            logstr.append(std::to_string(cron_hour));
            logstr.append(" ---\n");
            DEBUG_LOG("exit now:%s", logstr.c_str());
            std::unique_lock<std::mutex> logvlock(log_mutex);
            for (size_t i = 0; i < 10; i++)
            {
                error_loglist.push_back(logstr);
            }
            logvlock.unlock();
            isstop = true;
        }
    }
}

void httpserver::httpwatch_clear_timeout_sessions(unsigned int clean_cron_min, unsigned int clean_cron_time_ago)
{
    unsigned int nowtimeid         = timeid();
    unsigned int erase_count_num   = 0;
    unsigned int ok_count_num      = 0;
    unsigned int session_count_num = 0;
    unsigned int hard_kill_8hours  = nowtimeid - CONST_HARD_KILL_TIME;
    nowtimeid                      = nowtimeid - clean_cron_time_ago - 60;

    std::unique_lock<std::mutex> lock_sock(socket_session_lists_mutex);
    session_count_num = socket_session_lists.size();
    lock_sock.unlock();

    unsigned int remove_linknum = session_count_num / 10;
    if (remove_linknum < 300)
    {
        remove_linknum = 300;
    }

    unsigned int offset_remove = session_count_num / remove_linknum;

    if (offset_remove < 5)
    {
        offset_remove = 4;
    }

    offset_remove = rand_range(0, offset_remove);

    unsigned int jc = 0;
    unsigned int bc = 0;
    if (offset_remove == 0)
    {
        offset_remove = remove_linknum;
        jc            = 0;
    }
    else
    {
        offset_remove = offset_remove * remove_linknum;
        if (offset_remove > remove_linknum)
        {
            jc = offset_remove - remove_linknum;
        }
        else
        {
            jc = 0;
        }
    }
    // double remove num;
    offset_remove = offset_remove + remove_linknum;

    std::unique_lock<std::mutex> lock_sock_b(socket_session_lists_mutex);
    for (auto iter = socket_session_lists.begin(); iter != socket_session_lists.end();)
    {
        bc++;
        if (bc > offset_remove)
        {
            break;
        }
        if (bc < jc)
        {
            ++iter;
            continue;
        }

        std::shared_ptr<client_session> p_session = iter->lock();
        if (p_session)
        {
            if (p_session->time_limit.load() < nowtimeid)
            {
                asio::co_spawn(p_session->strand_, clientpeerstop(p_session), asio::detached);
                DEBUG_LOG("socket_session_wait_clear asio::co_spawn");
                if (p_session->half_close)
                {
                    socket_session_lists.erase(iter++);
                }
                else
                {
                    ++iter;
                }
                erase_count_num++;
                continue;
            }

            if (p_session->time_begin < hard_kill_8hours)
            {
                asio::co_spawn(p_session->strand_, clientpeerstop(p_session), asio::detached);
                DEBUG_LOG("socket_session_wait_clear asio::co_spawn");
                if (p_session->half_close)
                {
                    socket_session_lists.erase(iter++);
                }
                else
                {
                    ++iter;
                }
                erase_count_num++;
            }
            else
            {
                ++iter;
            }
        }
        else
        {
            socket_session_lists.erase(iter++);
            ok_count_num++;
        }
    }
    lock_sock_b.unlock();

    if (hard_kill_old_link)
    {
        nowtimeid = timeid() - 180;
        std::unique_lock<std::mutex> lock_sock_c(socket_session_lists_mutex);

        for (auto iter = socket_session_lists.begin(); iter != socket_session_lists.end();)
        {
            std::shared_ptr<client_session> p_session = iter->lock();
            if (p_session)
            {
                if (p_session->time_limit.load() < nowtimeid)
                {
                    DEBUG_LOG("clear nowtimeid pre session");
                    asio::co_spawn(p_session->strand_, clientpeerstop(p_session), asio::detached);
                    if (p_session->half_close)
                    {
                        socket_session_lists.erase(iter++);
                        erase_count_num++;
                    }
                    else
                    {
                        ++iter;
                    }
                }
                else
                {
                    ++iter;
                }
            }
            else
            {
                ++iter;
            }
        }
        lock_sock_c.unlock();
    }

    hard_kill_old_link = false;

    std::string error_msg_loop;
    error_msg_loop.clear();
    error_msg_loop = "-- clear sock L:";
    error_msg_loop.append(std::to_string(total_count.load()));
    error_msg_loop.append(" t:");
    error_msg_loop.append(std::to_string(session_count_num));
    error_msg_loop.append(" O:");
    error_msg_loop.append(std::to_string(ok_count_num));
    error_msg_loop.append(" E:");
    error_msg_loop.append(std::to_string(erase_count_num));

    error_msg_loop.append(" P:");
    error_msg_loop.append(std::to_string(clean_cron_min * 5));

    error_msg_loop.append(" G:");
    error_msg_loop.append(std::to_string(clean_cron_time_ago));

    http2_send_queue &send_queue_obj = get_http2_send_queue();
    error_msg_loop.append(" C:");
    error_msg_loop.append(std::to_string(send_queue_obj.queue_list.size()));
    error_msg_loop.append(" M:");
    error_msg_loop.append(std::to_string(http2_minute_count.load()));
    error_msg_loop.append(" L:");
    error_msg_loop.append(std::to_string(live_link_count.load()));

    unsigned long long self_rss_num    = get_rss_kb();
    unsigned long long total_memory_kb = get_total_memory_kb();
    error_msg_loop.append(" TOTAL:");
    error_msg_loop.append(std::to_string(total_memory_kb));
    error_msg_loop.append(" SELF:");
    error_msg_loop.append(std::to_string(self_rss_num));

    error_msg_loop.append(" --\n");
    std::unique_lock<std::mutex> logvvlock(log_mutex);
    error_loglist.push_back(error_msg_loop);
    logvvlock.unlock();

    //clear mysql connect
    std::map<std::string, std::shared_ptr<orm::orm_conn_pool>> &mysqldbpoolglobal = orm::get_orm_conn_pool_obj();
    for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
    {
        asio::co_spawn(this->io_context, orm_connect_clear(iter->second), asio::detached);
        DEBUG_LOG("mysql connect clear 1024q or 2hour ");
    }
}

// 请求落盘临时文件（rawcontent/tempraw 与 multipart 上传件）过去由业务自行清理，
// Temp files (rawcontent/tempraw and multipart uploads) used to be cleaned up by business code;
// 未被消费的文件（未知 Content-Type、进程异常退出、业务漏删）会永久留在 temp_path。
// unconsumed ones (unknown Content-Type, crash, business forgot to delete) linger forever in temp_path.
// 这里由 httpwatch 统一回收，回收条件 = 命名命中框架前缀白名单(见 http::is_http_temp_filename) 且 mtime 超存活期。
// httpwatch now handles it universally: framework-prefix whitelist (http::is_http_temp_filename) + mtime past TTL.
// 只删普通文件、非递归，statichtml 等子目录（压缩缓存）与业务文件不会被误伤。
// Only regular files, non-recursive — statichtml (compression cache) and business files are safe.
void httpserver::httpwatch_clear_temp_files(unsigned int temp_live_time)
{
    server_loaclvar &static_server_var = get_server_global_var();
    const std::string &temp_path       = static_server_var.temp_path;
    if (temp_path.empty())
    {
        return;
    }
    if (temp_live_time < 60)
    {
        temp_live_time = CONST_HTTP_TEMP_FILE_LIVE_TIME;
    }

    std::error_code errcode;
    fs::path dirpath(temp_path);
    if (!fs::is_directory(dirpath, errcode))
    {
        return;
    }

    // 用 file_time_type 自己的时钟做阈值，避免与 time_t 换算带来的平台/时区差异
    fs::file_time_type now_time   = fs::file_time_type::clock::now();
    fs::file_time_type live_limit = now_time - std::chrono::seconds(temp_live_time);

    unsigned int scan_num           = 0;
    unsigned int remove_num         = 0;
    unsigned long long remove_bytes = 0;

    // 非递归：static_html 等子目录（压缩缓存）不参与，避免误删缓存
    fs::directory_iterator iter(dirpath, fs::directory_options::skip_permission_denied, errcode);
    if (errcode)
    {
        return;
    }

    for (const fs::directory_entry &entry : iter)
    {
        scan_num++;
        errcode.clear();
        // 只处理普通文件：目录（如 statichtml）、符号链接、FIFO 一律跳过
        if (!entry.is_regular_file(errcode) || errcode)
        {
            continue;
        }
        if (!http::is_http_temp_filename(entry.path().filename().string()))
        {
            continue;
        }
        fs::file_time_type mtime = fs::last_write_time(entry.path(), errcode);
        if (errcode)
        {
            continue;
        }
        if (mtime > live_limit)
        {
            // 还在存活期内：业务可能正在读取/rename，本轮不删
            continue;
        }
        unsigned long long file_size = 0;
        errcode.clear();
        file_size = entry.file_size(errcode);
        if (errcode)
        {
            file_size = 0;
        }
        errcode.clear();
        if (fs::remove(entry.path(), errcode))
        {
            remove_num++;
            remove_bytes += file_size;
        }
    }

    if (remove_num > 0)
    {
        std::string error_msg_loop = "-- clear temp files:";
        error_msg_loop.append(std::to_string(remove_num));
        error_msg_loop.append(" scan:");
        error_msg_loop.append(std::to_string(scan_num));
        error_msg_loop.append(" bytes:");
        error_msg_loop.append(std::to_string(remove_bytes));
        error_msg_loop.append(" live:");
        error_msg_loop.append(std::to_string(temp_live_time));
        error_msg_loop.append(" --\n");
        std::unique_lock<std::mutex> logvvlock(log_mutex);
        error_loglist.push_back(error_msg_loop);
    }
}

void httpserver::httpwatch_check_deadlock(unsigned char &plan_http1_exit, unsigned char &plan_http2_exit, unsigned int &old_ten_total_count, unsigned int old_total_count)
{
    std::string error_msg_loop;
    // HTTP/2 deadlock detection
    if (total_http2_count.load() > 4)
    {
        error_msg_loop.clear();
        error_msg_loop.append("-- total_http2_count ");
        error_msg_loop.append(std::to_string(total_http2_count.load()));
        error_msg_loop.append(" - ");
        error_msg_loop.append(std::to_string(old_ten_total_count));
        error_msg_loop.append(" - ");
        error_msg_loop.append(std::to_string(total_count.load()));
        error_msg_loop.append(" - ");
        error_msg_loop.append(std::to_string(plan_http2_exit));
        error_msg_loop.append(" --\n");

        if (plan_http2_exit == 0)
        {
            old_ten_total_count = old_total_count;
        }

        plan_http2_exit++;
        if (plan_http2_exit > 2)
        {
            std::unique_lock<std::mutex> logh2lock(log_mutex);
            error_loglist.push_back(error_msg_loop);
            logh2lock.unlock();
            if (old_ten_total_count == total_count.load())
            {
                isstop = true;
            }
            else
            {
                plan_http2_exit = 0;
            }
        }
    }
    else
    {
        if (plan_http2_exit > 0)
        {
            plan_http2_exit = 0;
        }
    }

    // HTTP/1 deadlock detection
    if (total_http1_count.load() > 4)
    {
        error_msg_loop.clear();
        error_msg_loop.append("-- total_http2_count ");
        error_msg_loop.append(std::to_string(total_http1_count.load()));
        error_msg_loop.append(" - ");
        error_msg_loop.append(std::to_string(old_ten_total_count));
        error_msg_loop.append(" - ");
        error_msg_loop.append(std::to_string(total_count.load()));
        error_msg_loop.append(" - ");
        error_msg_loop.append(std::to_string(plan_http2_exit));
        error_msg_loop.append(" --\n");

        if (plan_http1_exit == 0)
        {
            old_ten_total_count = old_total_count;
        }

        plan_http1_exit++;
        if (plan_http1_exit > 2)
        {
            std::unique_lock<std::mutex> logh1lock(log_mutex);
            error_loglist.push_back(error_msg_loop);
            logh1lock.unlock();

            if (old_ten_total_count == total_count.load())
            {
                isstop = true;
            }
            else
            {
                plan_http1_exit = 0;
            }
        }
    }
    else
    {
        if (plan_http1_exit > 0)
        {
            plan_http1_exit = 0;
        }
    }
}

void httpserver::httpwatch_check_restart_threshold(unsigned int restart_process_num,
                                                   int restart_process_time_start,
                                                   int restart_process_time_end,
                                                   const std::tm *now,
                                                   unsigned int old_total_count)
{
    // Check every 5 seconds for over threshold
    if (now->tm_hour < restart_process_time_end && now->tm_hour > restart_process_time_start)
    {
        if (restart_process_num > 0)
        {
            if (old_total_count > restart_process_num && total_count.load() == old_total_count)
            {
                std::unique_lock<std::mutex> logrblock(log_mutex);
                error_loglist.push_back("--restart_process_time_start--\n");
                logrblock.unlock();
                isstop = true;
            }
        }
    }
}

void httpserver::httpwatch()
{
#ifdef __APPLE__
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, 0);
#endif
    serverconfig &sysconfigpath = getserversysconfig();

    // 1. Startup log
    {
        std::unique_lock<std::mutex> loglock(log_mutex);
        error_loglist.push_back("------------begin-----------");
        error_loglist.push_back(get_date("%Y-%m-%d %X\n"));
    }

    // 2. Initialize paths and configuration
    std::string currentpath, error_path, traffic_switch_file;
    std::string restart_file, restart_ssl_file, orm_log_file;
    httpwatch_init_paths(currentpath, error_path, traffic_switch_file, restart_file, restart_ssl_file, orm_log_file);

    // 3. Parse cron configurations
    unsigned char cron_type = 0, cron_day = 0, cron_hour = 0;
    unsigned int clean_cron_min = 60, clean_cron_time_ago = 0;
    unsigned int restart_process_num = 0;
    int restart_process_time_start = 0, restart_process_time_end = 0;
    unsigned int temp_live_time = CONST_HTTP_TEMP_FILE_LIVE_TIME;

    httpwatch_parse_reboot_cron(cron_type, cron_day, cron_hour);
    httpwatch_parse_clean_cron(clean_cron_min, clean_cron_time_ago);
    httpwatch_parse_temp_clean(temp_live_time);
    httpwatch_parse_links_restart(restart_process_num,
                                  restart_process_time_start,
                                  restart_process_time_end);

    // 4. Main loop variables
    unsigned int updatetimetemp      = 0;
    unsigned int mysqlpool_time      = 1;
    unsigned int old_total_count     = 0;
    unsigned int old_ten_total_count = 0;
    unsigned char plan_http1_exit    = 0x00;
    unsigned char plan_http2_exit    = 0x00;
    bool is_clear_sock               = false;
    bool is_run_acme                 = false;
    // 下次清扫 temp_path 落盘临时文件的时刻（秒）。按墙钟计，不受主循环 5 秒/拍漂移影响。
    unsigned int temp_clean_next_time = timeid() + temp_live_time;
    int acme_every_day_time           = sysconfigpath.acme_every_day_time;
    int acme_every_day_time_reset     = 1;
    int ocsp_interval_time            = sysconfigpath.ocsp_interval_time;

    if (ocsp_interval_time < 3600)
    {
        ocsp_interval_time = 3600;
    }
    if (acme_every_day_time < 2)
    {
        acme_every_day_time = 2;
    }
    if (acme_every_day_time > 23)
    {
        acme_every_day_time = 23;
    }
    acme_every_day_time_reset = acme_every_day_time - 1;

    DEBUG_LOG("httpwatch run");

    // 跑分模式 benchmark mode
#ifdef BENCHMARK
    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (isstop)
        {
            return;
        }
        DEBUG_LOG("httpwatch run");
    }
#endif

    // 5. Main event loop
    for (;;)
    {
        try
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));

            // Thread pool adjustment
            httpwatch_adjust_thread_pool(updatetimetemp);

            // Memory monitoring
            httpwatch_memory_monitor(mysqlpool_time);

            // Collect thread pool errors
            if (clientrunpool.error_message.size() > 0)
            {
                std::unique_lock<std::mutex> logthoollock(log_mutex);
                error_loglist.push_back(clientrunpool.error_message);
                logthoollock.unlock();
                clientrunpool.error_message.clear();
            }

            DEBUG_LOG("pool thread tasknum:%d %d", clientrunpool.gettasknum(), clientrunpool.getlivenum());

#ifdef DEBUG
            clientrunpool.printthreads(false);
#endif

            // Flush logs
            httpwatch_flush_access_log(currentpath);
            httpwatch_flush_error_log(error_path);

            std::time_t t = std::time(nullptr);
            std::tm *now  = std::localtime(&t);

            // MySQL pool maintenance
            httpwatch_mysql_pool_maintenance(mysqlpool_time, now, old_total_count);

            // ACME task trigger
            if (now->tm_hour == acme_every_day_time && is_run_acme)
            {
                is_run_acme = false;
                std::thread t_acme(&httpserver::acme_task, this);
                t_acme.detach();
            }
            if (now->tm_hour == acme_every_day_time_reset)
            {
                is_run_acme = true;
            }

            // OCSP Stapling refresh
            if (mysqlpool_time % ocsp_interval_time == 9)
            {
                std::thread t_ocsp([]()
                                   { refresh_all_ocsp_staples(); });
                t_ocsp.detach();
            }

            mysqlpool_time += 1;
            DEBUG_LOG("clear mysql poll time:%d,client live:%d", mysqlpool_time, total_count.load());
            DEBUG_LOG("cron type:%c day:%d,hour:%d min:%d", cron_type, cron_day, cron_hour, now->tm_min);

            // Cron reboot check (only after 40 cycles and within first 3 minutes)
            if (mysqlpool_time > 40)
            {
                httpwatch_check_cron_reboot(cron_type, cron_day, cron_hour, now);
            }

            // Clear timeout sessions
            if (mysqlpool_time % (clean_cron_min + 1) == 0)
            {
                is_clear_sock = true;
            }
            if (is_clear_sock)
            {
                httpwatch_clear_timeout_sessions(clean_cron_min, clean_cron_time_ago);
                is_clear_sock = false;
            }

            // 每 temp_live_time（默认 24 小时）清扫一次 temp_path，
            // 只删框架自己产生、且修改时间超过 temp_live_time 的落盘临时文件（pzraw_/pzup_）
            if (timeid() >= temp_clean_next_time)
            {
                temp_clean_next_time = timeid() + temp_live_time;
                httpwatch_clear_temp_files(temp_live_time);
            }

            // Watch status log
            if (mysqlpool_time % 128 == 0)
            {
                std::string error_msg_loop;
                error_msg_loop = "-- watch ";
                error_msg_loop.append(std::to_string(total_count.load()));
                error_msg_loop.push_back(0x20);
                error_msg_loop.append(std::to_string(total_http2_count.load()));
                error_msg_loop.push_back(0x20);
                error_msg_loop.append(std::to_string(total_http1_count.load()));
                error_msg_loop.push_back(0x20);
                error_msg_loop.append(get_date("%Y-%m-%d %X", t));

                error_msg_loop.push_back(0x20);
                error_msg_loop.append(std::to_string(restart_process_num));
                error_msg_loop.push_back(0x20);
                error_msg_loop.append(std::to_string(restart_process_time_start));
                error_msg_loop.push_back(0x20);
                error_msg_loop.append(std::to_string(restart_process_time_end));
                error_msg_loop.push_back(0x20);
                error_msg_loop.append(std::to_string(mysqlpool_time));
                error_msg_loop.push_back(0x20);
                error_msg_loop.append(std::to_string(clean_cron_min));

                error_msg_loop.append(" --\n");
                std::unique_lock<std::mutex> logwlock(log_mutex);
                error_loglist.push_back(error_msg_loop);
                logwlock.unlock();
            }

            // ORM query log save
            if (mysqlpool_time % (CONST_ORM_QUERY_LOG_TIME + 1) == 0)
            {
                orm::orm_connect_mar_t &watch_conn = orm::get_orm_connect_mar();
                watch_conn.save_log(orm_log_file);
            }

            // Traffic statistics and restart file detection
            if (mysqlpool_time % 4 == 0)
            {
                if (istraffic)
                {
                    save_traffic_arrays();
                }

                if (fs::exists(traffic_switch_file))
                {
                    istraffic = true;
                }
                else
                {
                    istraffic = false;
                }
            }
            if (mysqlpool_time % 13 == 0)
            {
                if (fs::exists(restart_file))
                {
                    isstop = true;
                    if (remove(restart_file.c_str()) == 0)
                    {
                        DEBUG_LOG(" -- remove restart_file -- ");
                    }
                }
            }
            if (mysqlpool_time % 13 == 0)
            {
                if (fs::exists(restart_ssl_file))
                {
                    DEBUG_LOG(" -- clearctx-- ");
                    sysconfigpath.clearctx();
                    if (remove(restart_ssl_file.c_str()) == 0)
                    {
                        DEBUG_LOG(" -- remove restart_ssl_file -- ");
                    }
                }
            }

            // Deadlock detection
            httpwatch_check_deadlock(plan_http1_exit, plan_http2_exit, old_ten_total_count, old_total_count);

            // Restart threshold check
            httpwatch_check_restart_threshold(restart_process_num,
                                              restart_process_time_start,
                                              restart_process_time_end,
                                              now,
                                              old_total_count);

            old_total_count = total_count.load();
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("frame thread:%s", e.what());
        }
        if (isstop)
        {
            DEBUG_LOG("std::abort");
            break;
        }
    }
    DEBUG_LOG("httpwatch exit");
    // 看护线程自己退出说明框架内部坏了，宁可 abort 让 supervisor 重新拉起；
    // stop() 之后的收摊不算坏，那时 isstop 已置位，返回让 run() join 掉本线程
    if (!isstop)
    {
        std::terminate();
    }
}
void httpserver::acme_task()
{
    serverconfig &sysconfigpath        = getserversysconfig();
    unsigned int site_num              = static_cast<unsigned int>(sysconfigpath.sitehostinfos.size());
    server_loaclvar &static_server_var = get_server_global_var();
    std::this_thread::sleep_for(std::chrono::seconds(12));
    //--begin save acme log --
    std::string domain_log = static_server_var.log_path;
    if (domain_log.size() > 0 && domain_log.back() != '/')
    {
        domain_log.push_back('/');
    }
    domain_log.append("acme_log");

    bool is_success = std::filesystem::create_directories(domain_log);
    if (is_success)
    {
        fs::permissions(domain_log,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }
    domain_log.append("/acme");

    std::ofstream out_acme_log(domain_log, std::ios::app);
    if (out_acme_log.is_open())
    {
        out_acme_log << " begin acme task :" << get_date("%Y-%m-%d %X", timeid()) << "\n";
        out_acme_log.close();
    }
    //--end save acme log --

    site_num = site_num / 10;
    if (site_num < 5)
    {
        site_num = sysconfigpath.acme_every_num;
        if (site_num < 1)
        {
            site_num = 1;
        }
    }
    if (site_num > 30)
    {
        site_num = 30;
    }

    for (size_t i = 0; i < site_num; i++)
    {
        acme_update();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    std::string fullchain_target;
    fullchain_target = static_server_var.log_path;

    if (fullchain_target.size() > 0 && fullchain_target.back() != '/')
    {
        fullchain_target.push_back('/');
    }
    fullchain_target.append("restart_ssl_config");

    std::ofstream outFile(fullchain_target, std::ios::trunc);
    if (outFile.is_open())
    {
        outFile << site_num;
        outFile.close();
    }
}
void httpserver::acme_update()
{
    serverconfig &sysconfigpath        = getserversysconfig();
    server_loaclvar &static_server_var = get_server_global_var();

    std::string acme_path;
    std::string conf_path;
    std::string webroot;
    std::vector<std::string> domains;
    std::string primary_domain;
    std::string domain;
    std::string error_message;
    conf_path = static_server_var.config_path;

    if (!std::filesystem::exists(conf_path))
    {
        //--begin save acme log --
        std::string domain_log = static_server_var.log_path;
        if (domain_log.size() > 0 && domain_log.back() != '/')
        {
            domain_log.push_back('/');
        }
        domain_log.append("acme_log");

        bool is_success = std::filesystem::create_directories(domain_log);
        if (is_success)
        {
            fs::permissions(domain_log,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }
        domain_log.append("/acme");

        std::ofstream out_acme_log(domain_log, std::ios::app);
        if (out_acme_log.is_open())
        {
            out_acme_log << "--[acme] The conf_path directory does not exist:" << conf_path << "--\n";
            out_acme_log.close();
        }
        //--end save acme log --
        return;
    }

    if (conf_path.size() > 0 && conf_path.back() != '/')
    {
        conf_path.push_back('/');
    }
    std::string acme_conf_file = conf_path + "acme.conf";
    acme_run acme;
    try
    {
        // 设置配置路径
        acme.set_acme_config_path(conf_path);
        {
            std::string domain_log = static_server_var.log_path;
            if (domain_log.size() > 0 && domain_log.back() != '/')
            {
                domain_log.push_back('/');
            }
            domain_log.append("acme_log");
            domain_log.append("/acme");
            std::ofstream out_acme_log(domain_log, std::ios::app);
            if (out_acme_log.is_open())
            {
                out_acme_log << "set_acme_config_path:" << acme_conf_file << "--\n";
                out_acme_log.close();
            }
        }

        if (!acme.load_acme_config(acme_conf_file))
        {
            //--begin save acme log --
            std::string domain_log = static_server_var.log_path;
            if (domain_log.size() > 0 && domain_log.back() != '/')
            {
                domain_log.push_back('/');
            }
            domain_log.append("acme_log");

            bool is_success = std::filesystem::create_directories(domain_log);
            if (is_success)
            {
                fs::permissions(domain_log,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
            domain_log.append("/acme");

            std::ofstream out_acme_log(domain_log, std::ios::app);
            if (out_acme_log.is_open())
            {
                out_acme_log << "--[acme] Unable to load configuration file:" << acme_conf_file << "--\n";
                out_acme_log.close();
            }
            //--end save acme log --
            return;
        }

        // 检查邮箱
        if (acme.email.empty())
        {
            //--begin save acme log --
            std::string domain_log = static_server_var.log_path;
            if (domain_log.size() > 0 && domain_log.back() != '/')
            {
                domain_log.push_back('/');
            }
            domain_log.append("acme_log");

            bool is_success = std::filesystem::create_directories(domain_log);
            if (is_success)
            {
                fs::permissions(domain_log,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
            domain_log.append("/acme");

            std::ofstream out_acme_log(domain_log, std::ios::app);
            if (out_acme_log.is_open())
            {
                out_acme_log << "--[acme] email is empty:" << acme_conf_file << "--\n";
                out_acme_log.close();
            }
            //--end save acme log --
            return;
        }

        for (size_t domain_num = 0; domain_num < sysconfigpath.sitehostinfos.size(); domain_num++)
        {
            if (!sysconfigpath.sitehostinfos[domain_num].is_acme)
            {
                //is not auto get ssl
                continue;
            }
            domains.clear();
            domain  = sysconfigpath.sitehostinfos[domain_num].mainhost;
            webroot = sysconfigpath.sitehostinfos[domain_num].wwwpath;
            if (domain.size() > 5 && domain[0] == 'w' && domain[1] == 'w' && domain[2] == 'w' && domain[3] == '.')
            {
                primary_domain = domain.substr(4);
                domains.push_back(primary_domain);
                domains.push_back(domain);
            }
            else
            {
                primary_domain = domain;
                domains.push_back(primary_domain);
                unsigned char pos_n    = domain.size() - 1;
                unsigned int dou_count = 0;

                //测试有几个点
                for (size_t k = 0; k < domain.size(); k++)
                {
                    if (domain[k] == '.')
                    {
                        dou_count++;
                    }
                }

                if (dou_count == 1)
                {
                    //可以加上www.一起申请
                    std::string wwwdomain = "www." + domain;
                    domains.push_back(wwwdomain);
                    domain = wwwdomain;
                }
                else if (dou_count == 2)
                {
                    //查看倒数第二个是不是　com net org gov edu
                    std::string subdomain;
                    pos_n = domain.size() - 1;
                    for (size_t k = pos_n; k > 0; k--)
                    {
                        if (domain[k] == '.')
                        {
                            unsigned int nn = k;
                            k--;
                            subdomain.clear();

                            for (; k > 0; k--)
                            {
                                if (domain[k] == '.')
                                {
                                    //回退一个
                                    k++;
                                    break;
                                }
                            }
                            if (k < nn)
                            {
                                for (; k < nn; k++)
                                {
                                    subdomain.push_back(domain[k]);
                                }
                            }
                            if (subdomain == "com" || subdomain == "net" || subdomain == "org" || subdomain == "gov" || subdomain == "edu")
                            {
                                std::string wwwdomain = "www." + domain;
                                domains.push_back(wwwdomain);
                                domain = wwwdomain;
                            }
                            break;
                        }
                    }
                }
            }
            //介绍域名检查
            //输出cert证书目录
            acme.output_dir = acme.acme_path + "/" + primary_domain;
            if (!std::filesystem::exists(acme.acme_path))
            {
                error_message = error_message + "--[acme] The acme_path directory does not exist:" + acme.acme_path + "--\n";
                //--begin save acme log --
                std::string domain_log = static_server_var.log_path;
                if (domain_log.size() > 0 && domain_log.back() != '/')
                {
                    domain_log.push_back('/');
                }
                domain_log.append("acme_log");

                bool is_success = std::filesystem::create_directories(domain_log);
                if (is_success)
                {
                    fs::permissions(domain_log,
                                    fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                                    fs::perm_options::replace);
                }
                domain_log.append("/acme");

                std::ofstream out_acme_log(domain_log, std::ios::app);
                if (out_acme_log.is_open())
                {
                    out_acme_log << error_message;
                    out_acme_log.close();
                }
                //--end save acme log --
                return;
            }
            // 创建输出目录
            bool is_success = std::filesystem::create_directories(acme.output_dir);
            if (is_success)
            {
                fs::permissions(acme.output_dir,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                                fs::perm_options::replace);
            }

            acme.set_webroot(webroot);
            acme.domains = domains;

            cert_validity_t cv = acme.check_domain_cert_pem(primary_domain);

            std::ostringstream oss;
            oss << "--[acme] webroot: " << webroot << "\n";
            oss << "--[acme] domain: " << primary_domain << "\n";
            if (!cv.is_expired)
            {
                oss << "--[acme] days remain: " << cv.days_remaining << " save path:" << acme.output_dir << "\n";
                if (cv.days_remaining > acme.days_remain)
                {
                    oss << "\nThe certificate is still within its validity period (days remain " << cv.days_remaining << " day)\n";
                    error_message.append(oss.str());
                    continue;
                }
                oss << "The certificate is about to expire, starting the reapplication process ..\n";
            }
            else
            {
                oss << "--[acme] The certificate does not exist or has expired. Initiating application ..\n";
            }

            bool ok = acme.run();

            if (ok)
            {
                oss << "--[acme] Certificate application successful!\n";
            }
            else
            {
                oss << "--[acme] failed!\n";
                oss << acme.message_debug;
                error_message = oss.str();

                //--begin save acme log --
                std::string domain_log = static_server_var.log_path;
                if (domain_log.size() > 0 && domain_log.back() != '/')
                {
                    domain_log.push_back('/');
                }
                domain_log.append("acme_log");

                is_success = std::filesystem::create_directories(domain_log);
                if (is_success)
                {
                    fs::permissions(domain_log,
                                    fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                                    fs::perm_options::replace);
                }
                domain_log.append("/");
                domain_log.append(primary_domain);

                std::ofstream out_acme_log(domain_log, std::ios::app);
                if (out_acme_log.is_open())
                {
                    out_acme_log << acme.message_debug;
                    out_acme_log.close();
                    error_message.append("--[acme] save acme log ok\n");
                }
                else
                {
                    error_message.append("--[acme] save acme log faild\n");
                }
                //--end save acme log --
                break;
            }

            error_message = oss.str();
            oss.str("");

            //--begin save acme log --
            std::string domain_log = static_server_var.log_path;
            if (domain_log.size() > 0 && domain_log.back() != '/')
            {
                domain_log.push_back('/');
            }
            domain_log.append("acme_log");

            is_success = std::filesystem::create_directories(domain_log);
            if (is_success)
            {
                fs::permissions(domain_log,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                                fs::perm_options::replace);
            }
            domain_log.append("/");
            domain_log.append(primary_domain);

            std::ofstream out_acme_log(domain_log, std::ios::app);
            if (out_acme_log.is_open())
            {
                out_acme_log << acme.message_debug;
                out_acme_log.close();
                error_message.append("--[acme] save acme log ok\n");
            }
            else
            {
                error_message.append("--[acme] save acme log faild\n");
            }
            //--end save acme log --

            try
            {
                // 如果目标已存在则覆盖（注意：copy_file 默认不会覆盖，需指定选项）

                std::string fullchain = acme.output_dir;
                if (fullchain.size() > 0 && fullchain.back() != '/')
                {
                    fullchain.push_back('/');
                }
                fullchain.append("fullchain.pem");
                std::string fullchain_target = conf_path;
                if (fullchain_target.size() > 0 && fullchain_target.back() != '/')
                {
                    fullchain_target.push_back('/');
                }
                fullchain_target.append(primary_domain);
                fullchain_target.append(".pem");

                std::filesystem::copy_file(fullchain, fullchain_target, fs::copy_options::overwrite_existing);
                //项目目录也复制一份
                fullchain_target = dir_name(static_server_var.log_path);
                if (fullchain_target.size() > 0 && fullchain_target.back() != '/')
                {
                    fullchain_target.push_back('/');
                }
                fullchain_target.append("conf");
                if (std::filesystem::exists(fullchain_target))
                {
                    fullchain_target.append("/");
                    fullchain_target.append(primary_domain);
                    fullchain_target.append(".pem");
                    std::filesystem::copy_file(fullchain, fullchain_target, fs::copy_options::overwrite_existing);
                }

                //privkey.pem
                fullchain = acme.output_dir;
                if (fullchain.size() > 0 && fullchain.back() != '/')
                {
                    fullchain.push_back('/');
                }
                fullchain.append("privkey.pem");

                fullchain_target = conf_path;
                if (fullchain_target.size() > 0 && fullchain_target.back() != '/')
                {
                    fullchain_target.push_back('/');
                }
                fullchain_target.append(primary_domain);
                fullchain_target.append(".key");

                std::filesystem::copy_file(fullchain, fullchain_target, fs::copy_options::overwrite_existing);
                //项目目录也复制一份
                fullchain_target = dir_name(static_server_var.log_path);
                if (fullchain_target.size() > 0 && fullchain_target.back() != '/')
                {
                    fullchain_target.push_back('/');
                }
                fullchain_target.append("conf");
                if (std::filesystem::exists(fullchain_target))
                {
                    fullchain_target.append("/");
                    fullchain_target.append(primary_domain);
                    fullchain_target.append(".key");
                    std::filesystem::copy_file(fullchain, fullchain_target, fs::copy_options::overwrite_existing);
                }

                //cert.pem
                fullchain = acme.output_dir;
                if (fullchain.size() > 0 && fullchain.back() != '/')
                {
                    fullchain.push_back('/');
                }
                fullchain.append("cert.pem");

                fullchain_target = conf_path;
                if (fullchain_target.size() > 0 && fullchain_target.back() != '/')
                {
                    fullchain_target.push_back('/');
                }
                fullchain_target.append(primary_domain);
                fullchain_target.append(".crt");

                std::filesystem::copy_file(fullchain, fullchain_target, fs::copy_options::overwrite_existing);
                //项目目录也复制一份
                fullchain_target = dir_name(static_server_var.log_path);
                if (fullchain_target.size() > 0 && fullchain_target.back() != '/')
                {
                    fullchain_target.push_back('/');
                }
                fullchain_target.append("conf");
                if (std::filesystem::exists(fullchain_target))
                {
                    fullchain_target.append("/");
                    fullchain_target.append(primary_domain);
                    fullchain_target.append(".crt");
                    std::filesystem::copy_file(fullchain, fullchain_target, fs::copy_options::overwrite_existing);
                }

                break;
            }
            catch (const fs::filesystem_error &e)
            {
                error_message.append("--[acme] filesystem_error ");
                error_message.append(e.what());
                error_message.append("\n");
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));

            break;
        }
    }
    catch (const std::exception &e)
    {
        error_message.append("--[acme] exception ");
        error_message.append(e.what());
        error_message.append("\n");
    }

    //--begin save acme log --
    std::string domain_log = static_server_var.log_path;
    if (domain_log.size() > 0 && domain_log.back() != '/')
    {
        domain_log.push_back('/');
    }
    domain_log.append("acme_log");

    bool is_success = std::filesystem::create_directories(domain_log);
    if (is_success)
    {
        fs::permissions(domain_log,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                        fs::perm_options::replace);
    }
    domain_log.append("/acme");

    std::ofstream out_acme_log(domain_log, std::ios::app);
    if (out_acme_log.is_open())
    {
        out_acme_log << error_message;
        out_acme_log.close();
    }
    //--end save acme log --
}

void httpserver::save_traffic_arrays()
{
    server_loaclvar &static_server_var = get_server_global_var();
    std::string currentpath            = static_server_var.log_path;
#ifndef _WIN32
    struct flock lockstr = {};
#endif

    if (currentpath.size() > 0 && currentpath.back() != '/')
    {
        currentpath.push_back('/');
    }
    currentpath.append("traffic.log");

    if (!traffic_arrays.empty())
    {
#ifndef _MSC_VER
        int fd = open(currentpath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd == -1)
        {
            return;
        }

#ifndef _WIN32
        lockstr.l_type   = F_WRLCK;
        lockstr.l_whence = SEEK_END;
        lockstr.l_start  = 0;
        lockstr.l_len    = 0;

        lockstr.l_pid = 0;

        if (fcntl(fd, F_SETLK, &lockstr) == -1)
        {
            close(fd);
            return;
        }
#else
        auto native_handle = (HANDLE)_get_osfhandle(fd);
        auto file_size     = GetFileSize(native_handle, nullptr);
        if (!LockFile(native_handle, file_size, 0, file_size, 0))
        {
            close(fd);
            return;
        }
#endif
        std::unique_lock<std::mutex> loglock(log_mutex);
        std::size_t n_write = write(fd, traffic_arrays.data(), traffic_arrays.size());
        traffic_arrays.clear();
        loglock.unlock();
        // not use
        if (true || n_write > 0)
        {
            n_write = 0;
        }

#ifndef _WIN32
        lockstr.l_type = F_UNLCK;
        if (fcntl(fd, F_SETLK, &lockstr) == -1)
        {
            close(fd);
            return;
        }
#else
        if (!UnlockFile(native_handle, file_size, 0, file_size, 0))
        {
            close(fd);
            return;
        }
#endif
        close(fd);
#endif
    }
}

void httpserver::set_thread_priority(std::thread &thread, int priority)
{
    auto native_handle = thread.native_handle();

#ifdef _WIN32
    SetThreadPriority(native_handle, priority);
#elif defined(__APPLE__)
    // macOS: pthread_set_qos_class_self_np only affects the calling thread,
    // so QoS is set inside each thread entry function instead.
    (void)native_handle;
    (void)priority;
#else
    // Linux/POSIX: try SCHED_RR with given priority, fallback to SCHED_OTHER
    struct sched_param sch;
    sch.sched_priority = priority;
    if (pthread_setschedparam(native_handle, SCHED_RR, &sch) != 0)
    {
        // Fallback: no permission or unsupported, reset to SCHED_OTHER priority 0
        sch.sched_priority = 0;
        pthread_setschedparam(native_handle, SCHED_OTHER, &sch);
    }
#endif
}

// 常驻出站客户端的按段软开关：conf 里该段 enable = 0 ⇒ 进程起来时不 spawn 这条重连循环。
// 键缺失或值为空都算 1——现有 conf 全都没写过它，默认必须维持今天"每条都启动"的行为。
// 真值拼写与各模块配置层里的 parse_bool 一致（1/true/True/TRUE/On/ON），其余写法按 0 处理，
// 所以跳过那行日志把原文一起打出来：写成 enable = yes 时能一眼看出是被这个开关关掉的。
[[maybe_unused]] static bool resident_client_enabled(const char *log_tag, const std::string &reg_name, const std::string &section, const std::string &enable_raw)
{
    if (enable_raw.empty() || enable_raw == "1" || enable_raw == "true" || enable_raw == "True" ||
        enable_raw == "TRUE" || enable_raw == "On" || enable_raw == "ON")
        return true;
    fprintf(stderr, "[%s] skip '%s' sec=%s enable=%s\n", log_tag, reg_name.c_str(), section.c_str(), enable_raw.c_str());
    return false;
}

void httpserver::run(const std::string &sysconfpath)
{
    try
    {
        isstop = false;

        serverconfig &sysconfigpath = getserversysconfig();
        sysconfigpath.init_path();
        if (sysconfigpath.configfile.empty())
        {
            sysconfigpath.configfile.clear();
            sysconfigpath.configfile.append(sysconfpath);
            sysconfigpath.init_path();
        }

        if (sysconfigpath.configfile.empty())
        {
            LOG_ERROR << " specify server.conf fullpath,example: ./xxx_run /etc/paozhu/server.conf " << LOG_END;
            std::this_thread::sleep_for(std::chrono::seconds(3));
            return;
        }

        // — v6 router 初始化 —
        router_init_sites();
        _initauto_all_httputils();
        _inithttpmethodregto();
        _inithttpmethodregto_pre();
        // conf 解析期注册表还是空的，钩子名的存在性只能在这里补验
        get_router().validate_site_hooks();

#ifdef ENABLE_REDIS
        // 用当前 server.conf 全路径推导同目录下的 redis.conf。
        // 这里必须传自己的 io_context，不能顺手 get_client_context_obj()：那个单例由"第一个
        // 调用的人"定型（参数默认 nullptr），在下面 9024 行带着真 ioc 构造之前先调一次，
        // 就等于把 ioc 永久钉成 nullptr——之后 http::client 的构造（make_strand(*ioc)）就是空指针。
        if (!pz::redis::load_redis_config(io_context, sysconfigpath.configfile))
        {
            std::cerr << "[redis] redis.conf load failed or missing, redis disabled" << std::endl;
        }
#endif

#ifdef ENABLE_WEBSOCKETS_CLIENT
        http::load_websockets_config(sysconfigpath.configfile);
#endif

#ifdef ENABLE_SOCKETS_CLIENT
        http::load_sockets_config(sysconfigpath.configfile);
#endif

#ifdef ENABLE_MQTT_CLIENT
        http::load_mqtt_config(sysconfigpath.configfile);
#endif

        server_ip6_listen                  = sysconfigpath.ip6_enable;
        server_loaclvar &static_server_var = get_server_global_var();

        debug_log::instance().setDebug(!static_server_var.deamon_enable);
        debug_log::instance().setLogfile(static_server_var.log_path);

        rate_limit_status = static_server_var.rate_limit_status;

#ifdef DEBUG
        static_server_var.show_visitinfo = true;
#endif

        sendqueue &send_cache = get_sendqueue();
        send_cache.inti_sendqueue(512);
        auto &link_cache = get_client_data_cache();
        link_cache.inti_sendqueue(1024);

        VIEW_REG &viewreg = get_viewmetholdreg();
        _initview_method_regto(viewreg);

        WEBSOCKET_REG &wsreg = get_websocket_reg();
        _initwebsocketmethodregto(wsreg);

        HTTP_SOCKET_REG &hsock = get_http_socket_reg();

        _inithttpsocketmethodregto(hsock);

        // 注册键统一成带前导 '/' 的形状（和 HTTP 路径、握手串一个习惯）：
        // sockets_method_reg.hpp 里写 "mytestsocket" 还是 "/mytestsocket" 都一样，
        // 裸 TCP 握手侧只需要拼一次 '/' 就能查到（client_tcp_loop）。
        for (auto hsock_iter = hsock.begin(); hsock_iter != hsock.end();)
        {
            if (!hsock_iter->first.empty() && hsock_iter->first.front() == '/')
            {
                ++hsock_iter;
                continue;
            }
            std::string slashed_key = "/" + hsock_iter->first;
            if (hsock.count(slashed_key) > 0)
            {
                ++hsock_iter;
                continue;
            }
            hsock.emplace(slashed_key, std::move(hsock_iter->second));
            hsock_iter = hsock.erase(hsock_iter);
        }

        total_count = sysconfigpath.get_co_thread_num();
        if (total_count < std::thread::hardware_concurrency())
        {
            total_count = std::thread::hardware_concurrency();
            total_count += 4;
        }
        if (total_count < 8)
        {
            total_count = 8;
        }

        asio::executor_work_guard<asio::io_context::executor_type> worker(io_context.get_executor());

        for (std::size_t i = 0; i < total_count; ++i)
        {
            runthreads.emplace_back(
                [this]()
                {
#ifdef DEBUG
                    std::ostringstream oss;
                    oss << std::this_thread::get_id();
                    std::string tempthread = oss.str();
                    DEBUG_LOG("frame thread:%s", tempthread.c_str());
#endif
                    do
                    {
                        try
                        {
                            this->io_context.run();
                        }
                        catch (...)
                        {
                        }
                    } while (!isstop);
                });
        }
#ifdef ENABLE_REDIS
        // 这批线程就是 redis 协程路跑在上面的人手，池的线程数统计照它来（跑在线程上，不由池自己管）。
        pz::redis::get_redis_pool().set_worker_count(static_cast<unsigned int>(runthreads.size()));
#endif
        total_count              = 0;
        clientrunpool.io_context = &io_context;
        std::thread httpwatch(std::bind(&httpserver::httpwatch, this));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::thread https(std::bind(&httpserver::listeners, this));
        std::thread http(std::bind(&httpserver::listener, this));

#ifdef BENCHMARK
        std::thread https2(std::bind(&httpserver::listeners, this));
        std::thread http2(std::bind(&httpserver::listener, this));
#endif
// set thread priority — 设置线程优先级
// set thread priority.
#ifdef _WIN32
        set_thread_priority(https, THREAD_PRIORITY_HIGHEST);
        set_thread_priority(http, THREAD_PRIORITY_HIGHEST);
#else
        set_thread_priority(https, 90);
        set_thread_priority(http, 90);
#ifndef BENCHMARK
        set_thread_priority(httpwatch, 50);
#endif
#endif

#ifdef BENCHMARK
#ifdef _WIN32
        set_thread_priority(https2, THREAD_PRIORITY_HIGHEST);
        set_thread_priority(http2, THREAD_PRIORITY_HIGHEST);
#else
        set_thread_priority(https2, 90);
        set_thread_priority(http2, 90);
#endif
#endif

        {
            client_context &client_context = get_client_context_obj(&io_context);
            client_context.run();
        }

        for (unsigned char i = 0; i < 2; ++i)
        {
            http2_send_data_threads.emplace_back(std::bind(&httpserver::http2_send_queue_loop, this, i));
        }
        for (int i = 0; i < 1; ++i)
        {
            websocketthreads.emplace_back(std::bind(&httpserver::websocket_loop, this, i));
        }

        std::thread ratelimiter(std::bind(&httpserver::ratelimiter, this));

#ifdef ENABLE_REDIS_CLIENT
        http::_initredissubpubregto(pz::redis::get_redis_subpub_reg());
        fprintf(stderr, "[redis_subpub] init: registered %zu clients\n", pz::redis::get_redis_subpub_reg().size());
        for (auto &[_name, factory] : pz::redis::get_redis_subpub_reg())
        {
            auto client = factory();
            if (!client)
                continue;
            if (!resident_client_enabled("redis_subpub", _name, client->section_name(), pz::redis::get_redis_config().raw_field(client->section_name(), "enable")))
                continue;
            asio::co_spawn(this->io_context,
                           this->async_redis_subpub_loop(client),
                           asio::detached);

            {
                std::lock_guard<std::mutex> lk(this->redis_subpub_task_mutex);
                this->redis_subpub_tasks.push_back(client);
            }
        }
#endif
#ifdef ENABLE_WEBSOCKETS_CLIENT
        http::_initwssubpubregto(http::get_ws_subpub_reg());
        fprintf(stderr, "[ws_subpub] init: registered %zu clients\n", http::get_ws_subpub_reg().size());
        for (auto &[_name, factory] : http::get_ws_subpub_reg())
        {
            auto client = factory();
            if (!client)
                continue;
            if (!resident_client_enabled("ws_subpub", _name, client->section_name(), http::get_websockets_config().raw_field(client->section_name(), "enable")))
                continue;
            asio::co_spawn(this->io_context,
                           this->async_ws_subpub_loop(client),
                           asio::detached);
            {
                std::lock_guard<std::mutex> lk(this->ws_subpub_task_mutex);
                this->ws_subpub_tasks.push_back(client);
            }
        }
#endif
#ifdef ENABLE_SOCKETS_CLIENT
        http::_initsockssubpubregto(http::get_sock_subpub_reg());
        fprintf(stderr, "[sock_subpub] init: registered %zu clients\n", http::get_sock_subpub_reg().size());
        for (auto &[_name, factory] : http::get_sock_subpub_reg())
        {
            auto client = factory();
            if (!client)
                continue;
            if (!resident_client_enabled("sock_subpub", _name, client->section_name(), http::get_sockets_config().raw_field(client->section_name(), "enable")))
                continue;
            fprintf(stderr, "[sock_subpub] spawning loop for '%s' sec=%s\n", _name.c_str(), client->section_name().c_str());
            asio::co_spawn(this->io_context,
                           this->async_sock_subpub_loop(client),
                           asio::detached);
            {
                std::lock_guard<std::mutex> lk(this->sockets_clients_mutex);
                this->sockets_clients.push_back(client);
            }
        }
#endif
#ifdef ENABLE_MQTT_CLIENT
        http::_initmqttsubpubregto(http::get_mqtt_subpub_reg());
        fprintf(stderr, "[mqtt_subpub] init: registered %zu clients\n", http::get_mqtt_subpub_reg().size());
        for (auto &[_name, factory] : http::get_mqtt_subpub_reg())
        {
            auto client = factory();
            if (!client)
                continue;
            if (!resident_client_enabled("mqtt_subpub", _name, client->section_name(), http::get_mqtt_config().raw_field(client->section_name(), "enable")))
                continue;
            fprintf(stderr, "[mqtt_subpub] spawning loop for '%s' sec=%s\n", _name.c_str(), client->section_name().c_str());
            asio::co_spawn(this->io_context,
                           this->async_mqtt_subpub_loop(client),
                           asio::detached);
            {
                std::lock_guard<std::mutex> lk(this->mqtt_clients_mutex);
                this->mqtt_clients.push_back(client);
            }
        }
#endif
        if (https.joinable())
        {
            https.join();
        }
        if (http.joinable())
        {
            http.join();
        }
#ifdef BENCHMARK
        if (https2.joinable())
        {
            https2.join();
        }
        if (http2.joinable())
        {
            http2.join();
        }
#endif

        for (int i = 0; i < 2; ++i)
        {
            if (http2_send_data_threads[i].joinable())
            {
                http2_send_data_threads[i].join();
            }
        }
        for (int i = 0; i < 1; ++i)
        {
            if (websocketthreads[i].joinable())
            {
                websocketthreads[i].join();
            }
        }
        // httpwatch 是这里的局部 thread：析构 joinable 的 std::thread 会直接 std::terminate，
        // 所以 stop() 之后必须 join 它（它按 isstop 退出，见 httpwatch() 主循环）。
        // 放在 join runthreads 之前：httpwatch 会增删 runthreads，先收口再收线程池。
        if (httpwatch.joinable())
        {
            httpwatch.join();
        }
        for (size_t i = 0; i < runthreads.size(); ++i)
        {
            if (runthreads[i].joinable())
            {
                runthreads[i].join();
            }
        }
        if (ratelimiter.joinable())
        {
            ratelimiter.join();
        }
    }
    catch (std::exception &e)
    {
        LOG_ERROR << " httpserver Exception " << e.what() << LOG_END;
    }
    DEBUG_LOG("httpserver exit!");
}
asio::io_context &httpserver::get_ctx()
{
    return io_context;
}
void httpserver::stop()
{
    isstop = true;
    rate_limit_condition.notify_all();   // 叫醒 ratelimiter 检查 isstop
    // 唤醒所有阻塞在 accept() 的 listener 线程，并把监听 fd 交回 acceptor 自己关。三步各管一段：
    //   cancel()   Windows(Vista+) 上是 CancelIoEx，asio 唯一的跨线程唤醒入口——
    //              那边的 closesocket 叫不醒挂在 accept() 里的线程；POSIX 下只作废 asio
    //              自己排队的异步操作，对阻塞 accept 无副作用
    //   shutdown() POSIX 下真正把阻塞中的 accept() 叫醒的是这一步，asio 的 close() 不做它
    //   close()    fd 由 owner 关那一次就够，stop() 不再自己 ::close——否则那个号被别的线程
    //              立刻复用之后，二次 close 打到的是新开的 socket
    // 先把登记的对象换出来，锁里不做系统调用
    std::vector<std::shared_ptr<asio::ip::tcp::acceptor>> stopping_acceptors;
    {
        std::lock_guard<std::mutex> lock(acceptors_mutex);
        stopping_acceptors.swap(acceptors);
    }
    for (auto &one_acceptor : stopping_acceptors)
    {
        asio::error_code temp_ec;
        if (one_acceptor->is_open())
        {
            one_acceptor->cancel(temp_ec);
#ifdef _WIN32
            ::shutdown(one_acceptor->native_handle(), SD_BOTH);
#else
            ::shutdown(one_acceptor->native_handle(), SHUT_RDWR);
#endif
        }
        one_acceptor->close(temp_ec);
    }
    client_context &client_context = get_client_context_obj();
    client_context.stop();
    websocketcondition.notify_all();
    send_data_condition.notify_all();
    clientrunpool.stop();
#ifdef ENABLE_REDIS
    // 现在共用框架 io_context，进程退出时 io_context.stop() 自然收掉所有协程，无需单独 stop redis pool
#endif
    io_context.stop();
    DEBUG_LOG("httpserver stop!");
}

unsigned int http::httpserver::socket_broadcast(unsigned int groupid, std::string_view payload)
{
    // 把同组 peer 先摘到本地 vector，锁里只做 shared_ptr 搬运，不调 send()
    std::vector<std::shared_ptr<socket_api>> targets;
    {
        std::lock_guard<std::mutex> lk(socket_task_mutex);
        for (auto &w : sockettasks)
        {
            auto p = w.lock();
            if (p && !p->isclose && p->session_sock && !p->session_sock->isclose &&
                p->groupid == groupid)
            {
                targets.push_back(p);
            }
        }
    }
    // 在锁外调 send()（send 会 post_write，而 socket_task_mutex 不允许重入）。
    // 返回成功 send 的 peer 数。
    unsigned int sent = 0;
    for (auto &p : targets)
    {
        if (p->send(payload))
        {
            ++sent;
        }
    }
    return sent;
}
}// namespace http
