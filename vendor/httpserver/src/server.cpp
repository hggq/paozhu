#include <zlib.h>
#include <functional>
#include <cstring>
#include "version.h"
#include "terminal_color.h"
#include "sendqueue.h"
#include "http_socket.h"
#include "client_session.h"
#include "http2_parse.h"
#include "http_mime.h"
#include "http2_define.h"
#include "http_domain.h"
#include "server.h"
#include "http2_huffman.h"
#include "http_parse.h"
#include "serverconfig.h"
#include "httppeer.h"
#include "http2_flow.h"
#include "directory_fun.h"
#include "https_brotli.h"
#include "gzip.h"
#include "http2_send_queue.h"
#include "httphook.h"
#include "func.h"
#include "mysqlconfig.h"
#include "mysqlpool.h"
#include "autocontrolmethod.hpp"
#include "reghttpmethod.hpp"
#include "reghttpmethod_pre.hpp"
#include "regviewmethod.hpp"
#include "autorestfulpaths.hpp"
#include "client_context.h"
#include "fastcgi.h"
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

namespace fs = std::filesystem;
#if defined(ASIO_ENABLE_HANDLER_TRACKING)
#define use_awaitable asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

namespace http
{
httpserver &get_server_app()
{
    static httpserver instance;
    return instance;
}
void add_server_timetask(std::size_t keyname, std::shared_ptr<httppeer> peer)
{
    get_server_app().clientlooptasks.push_back({keyname, peer});
}

asio::awaitable<void> httpserver::http2_send_file_range(std::shared_ptr<httppeer> peer)
{
    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(16384);
    send_file_obj->fp.reset(std::fopen(peer->sendfilename.c_str(), "rb"));

    DEBUG_LOG("%s", peer->sendfilename.c_str());

    if (send_file_obj->fp.get())
    {
        send_file_obj->peer = peer;
        send_file_obj->type = 1;
        fseek(send_file_obj->fp.get(), 0, SEEK_END);
        send_file_obj->content_length = ftell(send_file_obj->fp.get());
        fseek(send_file_obj->fp.get(), 0, SEEK_SET);

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

        if (peer->state.rangebegin >= send_file_obj->content_length || peer->state.rangeend > send_file_obj->content_length)
        {
            co_await http2_send_status_content(peer, 400, "client range request error!");
            peer->issend = true;
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        unsigned long long filesize = send_file_obj->content_length;
        unsigned int statecode      = 200;
        if (peer->state.rangebegin > 0)
        {
            send_file_obj->current_num = peer->state.rangebegin;
            statecode                  = 206;
            fseek(send_file_obj->fp.get(), send_file_obj->current_num, SEEK_SET);
        }
        if (peer->state.rangeend > 0)
        {
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
        send_file_obj->header = peer->make_http2_header(0);
        co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
        send_file_obj->is_sendheader = true;

        //add send sequence

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
        send_file_obj->header = peer->make_http2_header(0);

        co_await http2_send_content_append(peer->stream_id, send_file_obj->header, send_file_obj->content, true);

        co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
        peer->issend = true;
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

    send_file_obj->content.clear();
    co_await http2_send_content_append(peer->stream_id, send_file_obj->header, send_file_obj->content, true);
    co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
    peer->issend = true;
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
    send_file_obj->file_name.push_back(send_file_obj->etag[0]);
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
        std::FILE *fp = std::fopen(send_file_obj->file_name.c_str(), "rb");
        send_file_obj->fp.reset(std::fopen(send_file_obj->file_name.c_str(), "rb"));
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            send_file_obj->content_length = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            send_file_obj->fp.reset(fp);
        }
        co_return;
    }
    else
    {
        //create content
        std::string tempcompress;
        long long filesize = 0;
        tempcompress.resize(send_file_obj->content_length);
        filesize = fread(&tempcompress[0], 1, send_file_obj->content_length, send_file_obj->fp.get());
        tempcompress.resize(filesize);

        if (peer->compress == 2)
        {
            send_file_obj->content.clear();
            brotli_encode(tempcompress, send_file_obj->content);
            send_file_obj->content_length = send_file_obj->content.size();
            send_file_obj->type           = 17;
        }
        else if (peer->compress == 1)
        {
            if (compress(tempcompress.data(), tempcompress.size(), send_file_obj->content, Z_DEFAULT_COMPRESSION) ==
                Z_OK)
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
    send_file_obj->cache_data.resize(16384);
    send_file_obj->fp.reset(std::fopen(peer->sendfilename.c_str(), "rb"));

    DEBUG_LOG("%s", peer->sendfilename.c_str());

    if (send_file_obj->fp.get())
    {
        send_file_obj->peer = peer;
        send_file_obj->type = 1;
        fseek(send_file_obj->fp.get(), 0, SEEK_END);
        send_file_obj->content_length = ftell(send_file_obj->fp.get());
        fseek(send_file_obj->fp.get(), 0, SEEK_SET);

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

        //etag cache
        send_file_obj->etag = make_header_etag(send_file_obj->content_length, peer->fileinfo.st_mtime + peer->url.size());
        if (peer->etag == send_file_obj->etag)
        {
            co_await http2_co_send_304(peer, send_file_obj);
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        //compress

        if (peer->state.gzip || peer->state.br)
        {
            if (send_file_obj->content_length < 16877216 && send_file_obj->file_ext.size() > 0 && mime_compress.contains(send_file_obj->file_ext))
            {
                if (send_file_obj->etag.size() > 0)
                {
                    co_await http2_co_send_compress(peer, send_file_obj);
                }
            }
        }

        co_await http2_send_sequence_header(peer, send_file_obj);
        //add send sequence
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
        send_file_obj->header = peer->make_http2_header(0);

        co_await http2_send_content_append(peer->stream_id, send_file_obj->header, send_file_obj->content, true);

        co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
        peer->issend = true;
        send_queue_obj.back_cache_ptr(send_file_obj);
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
    unsigned int new_offset   = _send_data.size();

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
    unsigned int new_offset   = _send_data.size();

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
    peer->status(status_code);
    peer->length(bodycontent.size());
    peer->type("text/html; charset=utf-8");
    std::string _send_header = peer->make_http2_header(0);

    co_await http2_send_content_append(peer->stream_id, _send_header, bodycontent, true);
    co_await peer->socket_session->http2_send_queue_add_co(_send_header);

    co_return;
}
asio::awaitable<bool> httpserver::http2_static_file_authority(std::shared_ptr<httppeer> peer)
{
    serverconfig &sysconfigpath = getserversysconfig();
    unsigned int p_s            = sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists.size();
    std::string htmlcontent;
    DEBUG_LOG("static_pre_lists:%zu", sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists.size());
    //all static files
    if (p_s == 0)
    {
        htmlcontent = _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].static_pre_method].regfun(peer);
        if (htmlcontent.size() == 0)
        {
            co_return true;
        }
        else
        {
            htmlcontent.append(peer->output);
            co_await http2_send_status_content(peer, 403, htmlcontent);
            co_return false;
        }
    }
    unsigned int j = peer->urlpath.size();
    DEBUG_LOG("sendfilename:%s", peer->urlpath.c_str());
    for (unsigned int i = 0; i < p_s; i++)
    {
        unsigned int k = sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i].size();
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
                DEBUG_LOG("static_pre_lists: %u %s", i, sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i].c_str());
                //match pre urlpath
                htmlcontent = _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].static_pre_method].regfun(peer);
                if (htmlcontent.size() == 0)
                {
                    co_return true;
                }
                else
                {
                    htmlcontent.append(peer->output);
                    co_await http2_send_status_content(peer, 403, htmlcontent);
                    co_return false;
                }
            }
        }
    }
    DEBUG_LOG("not authority");
    co_return true;
}

asio::awaitable<void> httpserver::http2_fastcgi(std::shared_ptr<httppeer> peer)
{
    DEBUG_LOG("http2_fastcgi in");

    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(16384);
    send_file_obj->content.clear();

    peer->parse_session();
    peer->status(200);
    peer->content_type.clear();
    peer->etag.clear();

    co_await co_user_fastcgi_task(peer);

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
    co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);

    DEBUG_LOG("fastcgi send content");
    if (peer->content_length > 16384)
    {
        if (peer->compress == 0)
        {
            send_file_obj->content = peer->output;
        }
        send_file_obj->peer          = peer;
        send_file_obj->is_sendheader = true;
        std::unique_lock<std::mutex> lock(send_data_mutex);
        sent_data_list.emplace_back(send_file_obj);
        lock.unlock();
        send_data_condition.notify_one();

        peer->output.clear();
        peer->output.shrink_to_fit();
        send_queue_obj.back_cache_ptr(send_file_obj);
        co_return;
    }

    if (peer->compress > 0)
    {
        send_file_obj->header.clear();
        co_await http2_send_content(peer->stream_id, send_file_obj->header, send_file_obj->content, true);
        co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
    }
    else
    {
        send_file_obj->header.clear();
        co_await http2_send_content(peer->stream_id, send_file_obj->header, peer->output, true);
        co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
    }
    send_file_obj->header.clear();
    send_file_obj->header.shrink_to_fit();
    peer->output.clear();
    peer->output.shrink_to_fit();
    peer->issend = true;
    send_queue_obj.back_cache_ptr(send_file_obj);
    co_return;
}
asio::awaitable<void> httpserver::http2loop(std::shared_ptr<httppeer> peer)
{
    try
    {
        serverconfig &sysconfigpath = getserversysconfig();
        if (peer->compress == 10)
        {
            co_await http2_fastcgi(peer);
            co_return;
        }
        DEBUG_LOG("http2 host_index");

        peer->sitepath         = sysconfigpath.getsitewwwpath(peer->host_index);
        unsigned char sendtype = 0;
        sendtype               = peer->has_urlfileext();
        if (sendtype < 4)
        {
            sendtype = peer->get_fileinfo();
        }
        else if (sendtype == 5)
        {
            sendtype = 1;
        }
        DEBUG_LOG("http2loop:%s %d", peer->sendfilename.c_str(), sendtype);
        if (sendtype == 1)
        {
            DEBUG_LOG("is_static_pre:%d", sysconfigpath.sitehostinfos[peer->host_index].is_static_pre);
            if (sysconfigpath.sitehostinfos[peer->host_index].is_static_pre)
            {
                if ((co_await http2_static_file_authority(peer)))
                {
                    peer->output.clear();
                }
                else
                {
                    peer->issend = true;
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
        else if (sendtype == 2 && sysconfigpath.sitehostinfos[peer->host_index].is_show_directory)
        {
            std::string htmlcontent = displaydirectory(peer->sendfilename,
                                                       peer->urlpath,
                                                       peer->get["sort"].as_string(),
                                                       sysconfigpath.configpath);
            peer->status(200);
            peer->type("text/html; charset=utf-8");

            http2_send_queue &send_queue_obj = get_http2_send_queue();
            auto send_file_obj               = send_queue_obj.get_cache_ptr();
            send_file_obj->cache_data.resize(16384);
            send_file_obj->content.clear();
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

            if (peer->get_status() < 100)
            {
                peer->status(200);
            }
            DEBUG_LOG("htttp2 pool out");
            if (!peer->isset_type())
            {
                peer->type("text/html; charset=utf-8");
            }
            send_file_obj->header = peer->make_http2_header(0);

            DEBUG_LOG("send http2_send_writer");
            //co_await peer->socket_session->http2_send_writer(_send_header);
            co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
            DEBUG_LOG("http2_send_content");

            if (peer->content_length > 16384)
            {
                if (peer->compress == 0)
                {
                    send_file_obj->content = peer->output;
                }
                send_file_obj->peer          = peer;
                send_file_obj->is_sendheader = true;
                std::unique_lock<std::mutex> lock(send_data_mutex);
                sent_data_list.emplace_back(send_file_obj);
                lock.unlock();
                send_data_condition.notify_one();

                peer->output.clear();
                peer->output.shrink_to_fit();
                co_return;
            }

            if (peer->compress > 0)
            {
                send_file_obj->header.clear();
                co_await http2_send_content(peer->stream_id, send_file_obj->header, send_file_obj->content, true);
                co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
                //co_await http2_send_content(peer, (const unsigned char *)&tempcompress[0], tempcompress.size());
            }
            else
            {
                send_file_obj->header.clear();
                co_await http2_send_content(peer->stream_id, send_file_obj->header, peer->output, true);
                co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
                //co_await http2_send_content(peer, (const unsigned char *)&peer->output[0], peer->output.size());
            }
            send_file_obj->header.clear();
            send_file_obj->header.shrink_to_fit();

            peer->output.shrink_to_fit();
            peer->issend = true;
            send_queue_obj.back_cache_ptr(send_file_obj);
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
            send_file_obj->cache_data.resize(16384);
            send_file_obj->content.clear();
            send_file_obj->header.clear();
            bool not_co_handle = true;

            DEBUG_LOG("---  htttp2 co handle --------");
            auto co_iter = _co_http_regmethod_table.find(peer->sendfilename);
            if (co_iter != _co_http_regmethod_table.end())
            {
                DEBUG_LOG("---  coll %s handle --------", peer->sendfilename.c_str());
                send_file_obj->header = co_await co_iter->second.regfun(peer);
                not_co_handle         = false;

                for (unsigned int co_loop_num = 0; co_loop_num < 30; ++co_loop_num)
                {
                    if (send_file_obj->header.size() > 0)
                    {
                        co_iter = _co_http_regmethod_table.find(send_file_obj->header);
                        if (co_iter != _co_http_regmethod_table.end())
                        {
                            send_file_obj->header = co_await co_iter->second.regfun(peer);
                        }
                        else
                        {
                            auto iter = _http_regmethod_table.find(send_file_obj->header);
                            if (iter != _http_regmethod_table.end())
                            {
                                not_co_handle = true;
                            }
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            send_file_obj->header.clear();
            if (not_co_handle)
            {
                DEBUG_LOG("---  htttp2 pool pre --------");
                sendtype = co_await co_user_task(peer);
                DEBUG_LOG("htttp2 pool out [%d]", sendtype);
            }

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

            if (peer->get_status() < 100)
            {
                peer->status(200);
            }

            if (!peer->isset_type())
            {
                peer->type("text/html; charset=utf-8");
            }
            send_file_obj->header = peer->make_http2_header(0);

            co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
            DEBUG_LOG("http2_send_content");

            if (peer->content_length > 16384)
            {
                if (peer->compress == 0)
                {
                    send_file_obj->content = peer->output;
                }
                send_file_obj->peer          = peer;
                send_file_obj->is_sendheader = true;
                std::unique_lock<std::mutex> lock(send_data_mutex);
                sent_data_list.emplace_back(send_file_obj);
                lock.unlock();
                send_data_condition.notify_one();

                peer->output.clear();
                peer->output.shrink_to_fit();
                co_return;
            }

            if (peer->compress > 0)
            {
                send_file_obj->header.clear();
                co_await http2_send_content(peer->stream_id, send_file_obj->header, send_file_obj->content, true);
                co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
                //co_await http2_send_content(peer, (const unsigned char *)&tempcompress[0], tempcompress.size());
            }
            else
            {
                send_file_obj->header.clear();
                co_await http2_send_content(peer->stream_id, send_file_obj->header, peer->output, true);
                co_await peer->socket_session->http2_send_queue_add_co(send_file_obj->header);
                //co_await http2_send_content(peer, (const unsigned char *)&peer->output[0], peer->output.size());
            }
            send_file_obj->header.clear();
            send_file_obj->header.shrink_to_fit();
            send_queue_obj.back_cache_ptr(send_file_obj);
        }
        peer->output.shrink_to_fit();
        peer->issend = true;

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
        co_await peer->socket_session->co_send_goway();
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
    co_await peer_session->co_send_writer(sq_obj->header);
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

    send_file_obj->content.clear();
    send_file_obj->content = peer->make_http1_header();
    send_file_obj->content.append("\r\n");
    co_await peer_session->co_send_writer(send_file_obj->content);

    co_return;
}
asio::awaitable<void> httpserver::http1_send_file(std::shared_ptr<httppeer> peer,
                                                  std::shared_ptr<client_session> peer_session)
{

    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(16384);
    send_file_obj->fp.reset(std::fopen(peer->sendfilename.c_str(), "rb"));

    DEBUG_LOG("news http1_send_file %s", peer->sendfilename.c_str());

    if (send_file_obj->fp.get())
    {
        send_file_obj->peer = peer;
        send_file_obj->type = 1;
        fseek(send_file_obj->fp.get(), 0, SEEK_END);
        send_file_obj->content_length = ftell(send_file_obj->fp.get());
        fseek(send_file_obj->fp.get(), 0, SEEK_SET);

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

        //etag cache
        send_file_obj->etag = make_header_etag(send_file_obj->content_length, peer->fileinfo.st_mtime + peer->url.size());
        if (peer->etag == send_file_obj->etag)
        {
            co_await http1_co_send_304(peer, peer_session, send_file_obj);
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        //compress

        if (peer->state.gzip || peer->state.br)
        {
            if (send_file_obj->content_length < 16877216 && send_file_obj->file_ext.size() > 0 && mime_compress.contains(send_file_obj->file_ext))
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
                while (readnum < send_file_obj->content_length)
                {
                    send_file_obj->content.resize(4096);
                    unsigned int nread = fread(&send_file_obj->content[0], 1, 4096, send_file_obj->fp.get());
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
                    co_await peer_session->co_send_writer(send_file_obj->content);
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
            co_await peer_session->co_send_writer(send_file_obj->content);
        }
    }
    else
    {
        co_await http1_send_bad_server(peer, peer_session);
    }
    send_queue_obj.back_cache_ptr(send_file_obj);
    co_return;
}

asio::awaitable<void> httpserver::http1_send_file_range(std::shared_ptr<httppeer> peer,
                                                        std::shared_ptr<client_session> peer_session)
{

    http2_send_queue &send_queue_obj = get_http2_send_queue();
    auto send_file_obj               = send_queue_obj.get_cache_ptr();
    send_file_obj->cache_data.resize(16384);
    send_file_obj->fp.reset(std::fopen(peer->sendfilename.c_str(), "rb"));

    DEBUG_LOG("news http1_send_file_range %s", peer->sendfilename.c_str());

    if (send_file_obj->fp.get())
    {
        send_file_obj->peer = peer;
        send_file_obj->type = 1;
        fseek(send_file_obj->fp.get(), 0, SEEK_END);
        send_file_obj->content_length = ftell(send_file_obj->fp.get());
        fseek(send_file_obj->fp.get(), 0, SEEK_SET);

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

        if (peer->state.rangebegin >= send_file_obj->content_length || peer->state.rangeend > send_file_obj->content_length)
        {
            co_await http1_send_bad_request(400, peer_session);
            send_queue_obj.back_cache_ptr(send_file_obj);
            co_return;
        }

        unsigned long long filesize = send_file_obj->content_length;
        unsigned int statecode      = 200;
        if (peer->state.rangebegin > 0)
        {
            send_file_obj->current_num = peer->state.rangebegin;
            statecode                  = 206;
            fseek(send_file_obj->fp.get(), send_file_obj->current_num, SEEK_SET);
        }
        if (peer->state.rangeend > 0)
        {
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

        co_await peer_session->co_send_writer(send_file_obj->header);

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
                while (readnum < send_file_obj->content_length)
                {
                    send_file_obj->content.resize(4096);
                    unsigned int nread = fread(&send_file_obj->content[0], 1, 4096, send_file_obj->fp.get());
                    if (nread == 0)
                    {
                        DEBUG_LOG("nread 0 ");
                        peer_session->isclose = true;
                        peer->isclose         = true;
                        peer->state.keepalive = false;
                        break;
                    }
                    send_file_obj->content.resize(nread);

                    co_await peer_session->co_send_writer(send_file_obj->content);
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
    co_await peer->socket_session->co_send_writer(htmlcontent);
    if (bodycontent.size() > 0)
    {
        co_await peer->socket_session->co_send_writer(bodycontent);
    }
    co_return;
}
asio::awaitable<bool> httpserver::http1_static_file_authority(std::shared_ptr<httppeer> peer)
{
    serverconfig &sysconfigpath = getserversysconfig();
    unsigned int p_s            = sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists.size();
    std::string htmlcontent;
    DEBUG_LOG("static_pre_lists:%zu", sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists.size());
    //all static files
    if (p_s == 0)
    {
        htmlcontent = _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].static_pre_method].regfun(peer);
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
    unsigned int j = peer->urlpath.size();
    DEBUG_LOG("sendfilename:%s", peer->urlpath.c_str());
    for (unsigned int i = 0; i < p_s; i++)
    {
        unsigned int k = sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i].size();
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
                DEBUG_LOG("static_pre_lists: %u %s", i, sysconfigpath.sitehostinfos[peer->host_index].static_pre_lists[i].c_str());
                //match pre urlpath
                htmlcontent = _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].static_pre_method].regfun(peer);
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
asio::awaitable<size_t> httpserver::co_user_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h)
{
    auto initiate = [self = this](asio::detail::awaitable_handler<asio::any_io_executor, size_t> &&handler,
                                  std::shared_ptr<httppeer> peer) mutable
    {
        peer->user_code_handler_call.push_back(std::move(handler));
        self->clientrunpool.addclient(peer);
    };
    return asio::async_initiate<asio::use_awaitable_t<>, void(size_t)>(initiate, h, peer);
}
asio::awaitable<size_t> httpserver::co_user_fastcgi_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h)
{
    auto initiate = [self = this](asio::detail::awaitable_handler<asio::any_io_executor, size_t> &&handler,
                                  std::shared_ptr<httppeer> peer) mutable
    {
        serverconfig &sysconfigpath = getserversysconfig();
        peer->user_code_handler_call.push_back(std::move(handler));
        std::shared_ptr<http::fastcgi> fcgi = std::make_shared<http::fastcgi>();
        fcgi->peer_ptr                      = peer;
        fcgi->host                          = sysconfigpath.sitehostinfos[peer->host_index].fastcgi_host;// "127.0.0.1";
        fcgi->port                          = sysconfigpath.sitehostinfos[peer->host_index].fastcgi_port;// 9000
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
                    co_await peer->socket_session->co_send_writer(peer->output);
                    co_await peer->socket_session->co_send_writer(tempcompress);
                    co_return;
                }
            }
        }
    }
    peer->length(peer->output.size());
    std::string htmlcontent = peer->make_http1_header();
    htmlcontent.append("\r\n");
    co_await peer->socket_session->co_send_writer(htmlcontent);
    co_await peer->socket_session->co_send_writer(peer->output);
    peer->output.clear();
    co_return;
}
asio::awaitable<void> httpserver::http1loop(std::shared_ptr<httppeer> peer,
                                            std::shared_ptr<client_session> peer_session)
{
    if (peer->compress == 10)
    {
        co_await http1_fastcgi(peer);
        co_return;
    }
    serverconfig &sysconfigpath = getserversysconfig();
    peer->sitepath              = sysconfigpath.getsitewwwpath(peer->host_index);
    unsigned char sendtype      = 0;
    sendtype                    = peer->has_urlfileext();
    if (sendtype < 4)
    {
        sendtype = peer->get_fileinfo();
    }
    else if (sendtype == 5)
    {
        sendtype = 1;
    }

    DEBUG_LOG("http1loop:%s %d", peer->sendfilename.c_str(), sendtype);
    if (sendtype == 1)
    {
        if (sysconfigpath.sitehostinfos[peer->host_index].is_static_pre)
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
    else if (sendtype == 2 && sysconfigpath.sitehostinfos[peer->host_index].is_show_directory)
    {
        peer->output = displaydirectory(peer->sendfilename,
                                        peer->urlpath,
                                        peer->get["sort"].as_string(),
                                        sysconfigpath.configpath);
        peer->status(200);
        peer->type("text/html; charset=utf-8");
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
                        co_await peer_session->co_send_writer(peer->output);
                        co_await peer_session->co_send_writer(tempcompress);
                        co_return;
                    }
                }
            }
        }
        peer->length(peer->output.size());
        std::string htmlcontent = peer->make_http1_header();
        htmlcontent.append("\r\n");
        co_await peer_session->co_send_writer(htmlcontent);
        co_await peer_session->co_send_writer(peer->output);

        co_return;
    }
    else
    {
        peer->linktype = 0;
        peer->parse_session();

        peer->status(200);
        peer->content_type.clear();
        peer->etag.clear();
        peer->output.clear();

        std::string respcontent;
        bool not_co_handle = true;
        DEBUG_LOG("---  http1 co handle --------");
        auto co_iter = _co_http_regmethod_table.find(peer->sendfilename);
        if (co_iter != _co_http_regmethod_table.end())
        {
            DEBUG_LOG("---  coll %s handle --------", peer->sendfilename.c_str());
            respcontent   = co_await co_iter->second.regfun(peer);
            not_co_handle = false;

            for (unsigned int co_loop_num = 0; co_loop_num < 30; ++co_loop_num)
            {
                if (respcontent.size() > 0)
                {
                    co_iter = _co_http_regmethod_table.find(respcontent);
                    if (co_iter != _co_http_regmethod_table.end())
                    {
                        respcontent = co_await co_iter->second.regfun(peer);
                    }
                    else
                    {
                        auto iter = _http_regmethod_table.find(respcontent);
                        if (iter != _http_regmethod_table.end())
                        {
                            not_co_handle = true;
                        }
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if (not_co_handle)
        {
            DEBUG_LOG("---  http1 pool pre --------");
            sendtype = co_await co_user_task(peer);
            DEBUG_LOG("---  http1 pool post re_num [%d]--------", sendtype);
        }

        if (peer->get_status() < 100)
        {
            peer->status(200);
        }
        if (!peer->isset_type())
        {
            peer->type("text/html; charset=utf-8");
        }
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
                        std::string htmlcontent = peer->make_http1_header();
                        htmlcontent.append("\r\n");
                        co_await peer_session->co_send_writer(htmlcontent);
                        co_await peer_session->co_send_writer(tempcompress);
                        DEBUG_LOG("---  http1 compress send --------");
                        co_return;
                    }
                }
            }
        }
        peer->length(peer->output.size());
        std::string htmlcontent = peer->make_http1_header();
        htmlcontent.append("\r\n");
        co_await peer_session->co_send_writer(htmlcontent);
        co_await peer_session->co_send_writer(peer->output);
        DEBUG_LOG("---  http1 output send --------");
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
    //peer_session->send_data(str);
    co_await peer_session->co_send_writer(str);
    co_return;
}
asio::awaitable<void> httpserver::http1_send_bad_request(unsigned int error_code, std::shared_ptr<client_session> peer_session)
{
    std::string stfilecom = "<h3>";
    stfilecom.append(std::to_string(error_code));
    stfilecom.append("400 Bad Request</h3>");
    stfilecom.append("<hr /><p>Error Code: " + std::to_string(error_code) + "</p>");
    std::string str = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=utf-8\r\nConnection: "
                      "close\r\nContent-Length: ";
    str.append(std::to_string(stfilecom.size()));
    str.append("\r\n\r\n");
    str.append(stfilecom);
    //peer_session->send_data(str);
    co_await peer_session->co_send_writer(str);
    co_return;
}

int httpserver::checkhttp2(std::shared_ptr<client_session> peer_session)
{

    if (peer_session->_cache_data[0] == 0x50 && peer_session->_cache_data[1] == 0x52 &&
        peer_session->_cache_data[2] == 0x49 && peer_session->_cache_data[3] == 0x20 &&
        peer_session->_cache_data[4] == 0x2A && peer_session->_cache_data[5] == 0x20)
    {
        peer_session->httpv = 2;
        return 4;
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

asio::awaitable<void> httpserver::http2task(std::shared_ptr<http2parse> http2peer)
{
    std::unique_lock lk(http2peer->http2loop_mutex);
    if (http2peer->http2_loop_in)
    {
        co_return;
    }
    http2peer->http2_loop_in = true;
    lk.unlock();
    while (http2peer->http2_loop_in)
    {
        if (isstop)
        {
            break;
        }
        std::unique_lock lk(http2peer->http2loop_mutex);
        if (http2peer->httppeer_lists.size() > 0)
        {
            auto peer = http2peer->httppeer_lists.front();
            http2peer->httppeer_lists.pop();
            lk.unlock();
            co_await http2loop(peer);
            if (http2peer->peer_session->isclose)
            {
                break;
            }
        }
        else
        {
            http2peer->http2_loop_in = false;
            break;
        }
    }
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

        std::string log_item;
        std::shared_ptr<http2parse> http2pre;
        std::unique_ptr<httpparse> http1pre;
        int error_state                = 0;
        unsigned char linktype         = 0;
        std::shared_ptr<httppeer> peer = std::make_shared<httppeer>();

        peer_session->time_limit.store(timeid());
        peer->client_ip = peer_session->getremoteip();

        log_item.append("client in ");
        log_item.append(peer->client_ip);
        log_item.append(" ");
        log_item.append(std::to_string(peer_session->time_limit.load()));
        log_item.append("\n");

        std::unique_lock<std::mutex> lock(log_mutex);
        access_loglist.emplace_back(log_item);
        lock.unlock();
        log_item.clear();

        try
        {
            if (peer_session->_cache_data == nullptr)
            {
                add_nullptrlog(peer->client_ip);
                co_return;
            }

            if (check_blockip(peer->client_ip))
            {
                co_return;
            }
#ifdef DEBUG
            {
                DEBUG_LOG("new client in");
                std::ostringstream oss;
                oss << std::this_thread::get_id();
                oss << " ";
                oss << peer->client_ip;
                std::string tempthread = oss.str();
                DEBUG_LOG("\033[31mclient thread:%s\033[0m", tempthread.c_str());
            }
#endif

            peer->isssl            = isssl ? true : false;
            peer->socket_session   = peer_session;
            unsigned int offsetnum = 0, readnum = 0;
            for (;;)
            {
                log_item.clear();
                bool is_error = co_await peer_session->read_some(readnum, log_item);
                if (is_error)
                {
                    DEBUG_LOG("read_some error");
                    log_item.push_back(0x20);
                    log_item.append(peer->client_ip);
                    log_item.push_back(0x20);
                    log_item.append(peer->host);
                    log_item.push_back(0x20);
                    log_item.append(peer->url);
                    log_item.push_back('\n');
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(log_item);
                    lock.unlock();
                    break;
                }

                if (linktype == 0)
                {
                    linktype = checkhttp2(peer_session);
                    if (linktype == 1)
                    {
                        http1pre = std::make_unique<httpparse>();
                        http1pre->setpeer(peer);
                        peer_session->httpv = 1;
                        peer->httpv         = 1;
                    }
                    else if (linktype == 4)
                    {
                        http2pre            = std::make_shared<http2parse>();
                        peer_session->httpv = 2;
                        peer->httpv         = 2;
                    }
                }

                // http1
                if (linktype == 1)
                {
                    http1pre->process(peer_session->_cache_data, readnum);
                    if (http1pre->error > 0)
                    {
#ifdef DEBUG
                        DEBUG_LOG("http1 client request error!");
#endif
                        co_await http1_send_bad_request(http1pre->error, peer_session);
                        break;
                    }
                    if (readnum == 0 || http1pre->getfinish())
                    {

                        //  LOG_OUT << "http1parse fininsh" << peer->url << LOG_END;
#ifdef DEBUG
                        DEBUG_LOG("http1parse begin");
                        DEBUG_LOG("urlpath:%s", peer->urlpath.c_str());
                        DEBUG_LOG("host:%s", peer->host.c_str());
                        DEBUG_LOG("User-Agent:%s", peer->header["User-Agent"].c_str());
                        DEBUG_LOG("http1parse end");
#endif
                        peer->isssl = isssl ? true : false;
                        if (peer->server_port == 0)
                        {
                            peer->socket_session = peer_session;
                            peer->server_ip      = peer_session->getlocalip();
                            peer->client_ip      = peer_session->getremoteip();
                            peer->client_port    = peer_session->getremoteport();
                            peer->server_port    = peer_session->getlocalport();
                        }

                        {
                            log_item.clear();
                            log_item.append(peer->client_ip);
                            log_item.push_back(0x20);
                            log_item.append(get_date("%Y-%m-%d %X"));
                            log_item.push_back(0x20);
                            log_item.append(std::to_string(peer->server_port));
                            log_item.push_back(0x20);
                            log_item.append("H1");
                            log_item.push_back(0x20);
                            log_item.append(peer->host);
                            log_item.push_back(0x20);
                            log_item.append(peer->url);
                            log_item.push_back('\n');
                            std::unique_lock<std::mutex> lock(log_mutex);
                            access_loglist.emplace_back(log_item);
                            lock.unlock();
                        }

                        if (peer->state.h2c)
                        {
                            linktype = 4;

                            peer_session->httpv = 2;
                            peer->httpv         = 2;
                            peer->isfinish      = true;
                            peer->issend        = false;

                            peer_session->send_switch101();
                            peer->stream_id = 1;
                            peer->isssl     = isssl ? true : false;
                            http2pre        = std::make_shared<http2parse>();
                            http2pre->setsession(peer_session);
                            http2pre->http_data.emplace(1, peer);
                            http2pre->stream_list.emplace(1);
                            continue;
                        }

                        if (hook_host_http1(peer))
                        {
                            break;
                        }

                        if (peer->state.websocket)
                        {
                            linktype       = 5;
                            peer->linktype = 3;
                            peer->ws       = std::make_unique<websocketparse>();
                            peer->ws->setWebsocketkey(peer->websocket.key);
                            std::string resp = peer->ws->respondHandshake();
                            peer_session->send_data(resp);
                            WEBSOCKET_REG &wsreg = get_websocket_reg();

                            if (peer->pathinfos.size() == 0)
                            {
                                break;
                            }
                            auto wsiter = wsreg.find(peer->pathinfos[0]);
                            if (wsiter == wsreg.end())
                            {
                                break;
                            }
                            auto myclientwsplugin = wsreg[peer->pathinfos[0]];
                            peer->websockets      = myclientwsplugin(peer);
                            peer->ws->isopen      = true;
                            peer->websockets->onopen();
                            peer->ws->contentlength = 0;
                            if (peer->websockets->timeloop_num > 0)
                            {
                                websockettasks.emplace_back(peer);
                                websocketcondition.notify_one();
                            }

                            continue;
                        }
                        //traffic save
                        if (istraffic && peer->method < 3)
                        {
                            std::unique_lock<std::mutex> lock(log_mutex);
                            if (isssl)
                            {
                                traffic_arrays.append("H1");
                            }
                            else
                            {
                                traffic_arrays.append("H0");
                            }
                            traffic_arrays.push_back((readnum & 0xFF));
                            traffic_arrays.push_back((readnum >> 8 & 0xFF));
                            traffic_arrays.push_back(0x00);
                            traffic_arrays.push_back(0x00);
                            traffic_arrays.append((char *)peer_session->_cache_data, readnum);
                            lock.unlock();
                        }

                        co_await http1loop(peer, peer_session);
                        DEBUG_LOG("http1loop end");
                        if (peer->state.keepalive == false)
                        {
                            break;
                        }
                        http1pre->clear();
                        peer_session->time_limit.store(timeid());
                    }
                }

                // websocket
                if (linktype == 5)
                {
                    if (peer->ws->contentlength == 0)
                    {
                        DEBUG_LOG("websockets big data");
                        peer->ws->getprocssdata(peer_session->_cache_data, readnum);
                    }
                    else
                    {
                        DEBUG_LOG("websockets small data");
                        peer->ws->parsedata(peer_session->_cache_data, readnum);
                    }

                    if (peer->ws->isfinish)
                    {
                        DEBUG_LOG("onmessage isfinish");
                        if (peer->ws->opcode < 0x08 && peer->ws->opcode > 0x00)
                        {
                            peer->ws->closefile();
                            clientrunpool.addclient(peer);
                        }
                        else if (peer->ws->opcode == 0x08)
                        {
                            peer->ws->isopen = false;
                            peer->websockets->onclose();
                            DEBUG_LOG("websockets");
                            break;
                        }
                        else if (peer->ws->opcode == 0x09)
                        {
                            peer->websockets->onpong();
                            std::string outhello;
                            outhello = peer->ws->makePong();
                            peer->send(outhello);
                        }
                    }
                }

                // h2c
                if (linktype == 4)
                {
                    int j = 0;
                    for (int i = 0; i < 24; i++, j++)
                    {
                        if (peer_session->_cache_data[j] != magicstr[i])
                        {
                            break;
                        }
                    }
                    if (j != 24)
                    {
                        break;
                    }
                    offsetnum = 24;
                    linktype  = 3;
                    //peer_session->send_setting();
                    co_await peer_session->co_send_setting();
                    http2pre->setsession(peer_session);
                }

                if (linktype == 3)
                {
                    http2pre->process(&peer_session->_cache_data[offsetnum], readnum - offsetnum);
                    offsetnum = 0;
                    if (peer_session->isgoway)
                    {
                        DEBUG_LOG("nread 0 ");
                        break;
                    }
                    if (http2pre->error > 0)
                    {
                        co_await http2_send_status_content(peer, 403, "client request error!");
#ifdef DEBUG
                        DEBUG_LOG("http2 client request error!");
#endif
                        co_await peer_session->co_send_goway();
                        break;
                    }

                    peer_session->time_limit.store(timeid());

                    if (http2pre->stream_list.size() > 0)
                    {
                        for (; http2pre->stream_list.size() > 0;)
                        {
                            unsigned int block_steamid = http2pre->stream_list.front();
                            if (http2pre->http_data[block_steamid]->socket_session == nullptr)
                            {
                                http2pre->http_data[block_steamid]->socket_session = peer_session->get_ptr();
                            }

                            if (hook_host_http2(http2pre->http_data[block_steamid]))
                            {
                                http2pre->stream_list.pop();
                                error_state = 1;
                                break;
                            }
                            //http2pre->http_data[block_steamid]->linktype    = 0;
                            if (http2pre->http_data[block_steamid]->server_port == 0)
                            {
                                http2pre->http_data[block_steamid]->server_ip   = peer_session->getlocalip();
                                http2pre->http_data[block_steamid]->client_ip   = peer_session->getremoteip();
                                http2pre->http_data[block_steamid]->client_port = peer_session->getremoteport();
                                http2pre->http_data[block_steamid]->server_port = peer_session->getlocalport();
                            }

                            {
                                log_item.clear();
                                log_item.append(http2pre->http_data[block_steamid]->client_ip);
                                log_item.push_back(0x20);
                                log_item.append(get_date("%Y-%m-%d %X"));
                                log_item.push_back(0x20);
                                log_item.append(std::to_string(http2pre->http_data[block_steamid]->server_port));
                                log_item.push_back(0x20);
                                log_item.append("H2");
                                log_item.push_back(0x20);
                                log_item.append(http2pre->http_data[block_steamid]->host);
                                log_item.push_back(0x20);
                                log_item.append(http2pre->http_data[block_steamid]->url);
                                log_item.push_back('\n');
                                std::unique_lock<std::mutex> lock(log_mutex);
                                access_loglist.emplace_back(log_item);
                                lock.unlock();
                            }
#ifdef DEBUG
                            DEBUG_LOG("http2parse begin");
                            DEBUG_LOG("urlpath:%s", http2pre->http_data[block_steamid]->urlpath.c_str());
                            DEBUG_LOG("host:%s", http2pre->http_data[block_steamid]->host.c_str());
                            DEBUG_LOG("User-Agent:%s",
                                      http2pre->http_data[block_steamid]->header["User-Agent"].c_str());
                            DEBUG_LOG("http2parse end");
#endif

                            // clientrunpool.addclient(http2pre->http_data[block_steamid]);
                            //co_spawn(this->io_context, http2loop(http2pre->http_data[block_steamid]), asio::detached);

                            //traffic save
                            if (istraffic && http2pre->http_data[block_steamid]->method < 3)
                            {
                                if (http2pre->ispost && http2pre->block_steam_httppeer->content_length > 4090)
                                {
                                }
                                else
                                {
                                    std::unique_lock<std::mutex> lock(log_mutex);
                                    traffic_arrays.append("H2");
                                    traffic_arrays.push_back((readnum & 0xFF));
                                    traffic_arrays.push_back((readnum >> 8 & 0xFF));
                                    traffic_arrays.push_back(0x00);
                                    traffic_arrays.push_back(0x00);
                                    traffic_arrays.append((char *)peer_session->_cache_data, readnum);
                                    lock.unlock();
                                }
                            }

                            std::unique_lock lk(http2pre->http2loop_mutex);
                            http2pre->httppeer_lists.push(http2pre->http_data[block_steamid]);
                            lk.unlock();
                            co_spawn(this->io_context, http2task(http2pre), asio::detached);
                            //co_spawn(this->io_context, http2loop(http2pre->http_data[block_steamid]), asio::detached);

                            http2pre->block_steam_httppeer = nullptr;
                            http2pre->stream_list.pop();
                            http2pre->steam_count += 1;
                        }
                    }
                    if (error_state > 0)
                    {
                        co_await peer_session->co_send_goway();
                        break;
                    }
                    DEBUG_LOG("client http2 > 2024 stream close %d", http2pre->steam_count);
                    if (http2pre->steam_count > 4096)
                    {
                        DEBUG_LOG("client http2 > 2024 stream close ");
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        co_await peer_session->co_send_goway();
                        break;
                    }
                }
            }
        }
        catch (std::exception &e)
        {
            log_item.clear();
            log_item.append(" client thread exception  ");
            log_item.append(e.what());
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(log_item);
            lock.unlock();
        }
        peer_session->stop();
        peer_session->isclose = true;
        peer->isclose         = true;

        if (linktype == 3)
        {
            DEBUG_LOG("client http2 break");
            http2pre->clsoesend(this->io_context);
        }
        else
        {
            DEBUG_LOG("client http1 break");
            peer->clsoesend(this->io_context);
        }
        DEBUG_LOG("\033[35mclient run exit:%s\033[0m", peer->client_ip.c_str());
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("client exit exception");
    }
    catch (...)
    {
    }

    co_return;
}
asio::awaitable<void> httpserver::http2_send_sequence_header(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t> sq_obj)
{
    if (peer.use_count() == 0)
    {
        co_return;
    }
    if (peer->socket_session.use_count() == 0)
    {
        peer->issend = true;
        co_return;
    }
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

    sq_obj->header = peer->make_http2_header(0);
    co_await peer->socket_session->http2_send_queue_add_co(sq_obj->header);
    sq_obj->is_sendheader = true;
    co_return;
}
asio::awaitable<void> httpserver::http2_send_sequence(std::shared_ptr<http2_send_data_t> sq_obj)
{

    std::shared_ptr<httppeer> peer = sq_obj->peer;
    if (sq_obj->peer.use_count() == 0)
    {
        co_return;
    }
    if (sq_obj->peer->socket_session.use_count() == 0)
    {
        peer->issend = true;
        co_return;
    }

    if (sq_obj->is_sendheader == false)
    {
        co_await http2_send_sequence_header(sq_obj->peer, sq_obj);
    }

    unsigned int data_send_id = peer->stream_id;

    if (sq_obj->content_length == 0)
    {
        co_await peer->socket_session->co_send_zero_data(peer->stream_id);
        peer->issend         = true;
        sq_obj->standby_next = true;
        co_return;
    }

    unsigned long long per_size = 0;
    unsigned int vsize_send     = 8181;

    sq_obj->cache_data.clear();

    if (peer->socket_session->old_window_update_num.load() == 0)
    {
        vsize_send = 4096;
        peer->socket_session->old_window_update_num.store(peer->socket_session->window_update_num.load());
        peer->socket_session->new_send_balance_num = peer->socket_session->window_update_num.load() / 2;
    }
    else
    {
        long long temp_num = peer->socket_session->window_update_num.load() - peer->socket_session->old_window_update_num.load();

        if (temp_num > 0)
        {
            peer->socket_session->new_send_balance_num += temp_num;
        }
        if (temp_num != 0)
        {
            peer->socket_session->old_window_update_num.store(peer->socket_session->window_update_num.load());
        }
    }

    if (peer->socket_session->new_send_balance_num.load() > 307200)
    {
        vsize_send = 16384;
    }
    else if (peer->socket_session->new_send_balance_num.load() > 102400)
    {
        vsize_send = 8192;
    }
    else
    {
        vsize_send = 4096;
    }

    //small file and content
    if (sq_obj->content_length < 307200)
    {
        vsize_send = 16384;
    }
    if (sq_obj->content_length > 20971520)
    {
        vsize_send = 8192;
    }

    sq_obj->cache_data.resize(vsize_send);

    vsize_send            = sq_obj->cache_data.size() - 9;
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
        //send file
        if (sq_obj->current_num < sq_obj->content_length)
        {
            per_size = fread(&sq_obj->cache_data[9], 1, vsize_send, sq_obj->fp.get());
            if (per_size == 0 && vsize_send > 0)
            {
                co_await peer->socket_session->co_http2_send_enddata(peer->stream_id);
                peer->issend         = true;
                sq_obj->standby_next = true;
                co_return;
            }
            sq_obj->cache_data.resize(9 + per_size);
        }
    }
    else
    {
        per_size = vsize_send;
        if ((sq_obj->current_num + per_size) > sq_obj->content.size())
        {
            per_size = sq_obj->content.size() - sq_obj->current_num;
        }
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

    co_await peer->socket_session->http2_send_queue_add_co(sq_obj->cache_data);
    if (sq_obj->current_num >= sq_obj->content_length)
    {
        peer->issend = true;
    }

    peer->socket_session->has_send_update_num += per_size + 9;
    peer->socket_session->new_send_balance_num -= (per_size + 9);

    if (sq_obj->content_length > 20971520)
    {
        sq_obj->sleep_time = 2000000;
    }
    else
    {
        sq_obj->sleep_time = 1200000;
    }

    if (sq_obj->current_num < 20480)
    {
        sq_obj->sleep_time = 500000;
    }
    //small file and content
    if (sq_obj->content_length < 307200)
    {
        sq_obj->sleep_time = 50000;
    }

    if (peer->socket_session->new_send_balance_num.load() < 40960)
    {
        sq_obj->sleep_time = 200000000;
    }
    else if (peer->socket_session->new_send_balance_num.load() < 81920)
    {
        sq_obj->sleep_time = 12000000;
    }
    else if (peer->socket_session->new_send_balance_num.load() < 102400)
    {
        sq_obj->sleep_time = 2000000;
    }
    sq_obj->standby_next = true;
    sq_obj->last_time    = std::chrono::steady_clock::now();
    co_return;
}

bool httpserver::http2_loop_send_sequence(std::shared_ptr<http2_send_data_t> sq_obj)
{

    std::shared_ptr<httppeer> peer = sq_obj->peer;
    if (sq_obj->peer.use_count() == 0)
    {
        return false;
    }
    if (sq_obj->peer->socket_session.use_count() == 0)
    {
        peer->issend = true;
        return false;
    }

    unsigned int data_send_id = peer->stream_id;

    if (sq_obj->content_length == 0)
    {
        peer->socket_session->send_zero_data(peer->stream_id);
        peer->issend         = true;
        sq_obj->standby_next = true;
        return false;
    }

    unsigned long long per_size = 0;
    unsigned int vsize_send     = 8181;

    sq_obj->cache_data.clear();

    if (peer->socket_session->old_window_update_num.load() == 0)
    {
        vsize_send = 4096;
        peer->socket_session->old_window_update_num.store(peer->socket_session->window_update_num.load());
        peer->socket_session->new_send_balance_num = peer->socket_session->window_update_num.load() / 2;
    }
    else
    {
        long long temp_num = peer->socket_session->window_update_num.load() - peer->socket_session->old_window_update_num.load();

        if (temp_num > 0)
        {
            peer->socket_session->new_send_balance_num += temp_num;
        }
        if (temp_num != 0)
        {
            peer->socket_session->old_window_update_num.store(peer->socket_session->window_update_num.load());
        }
    }

    if (peer->socket_session->new_send_balance_num.load() > 307200)
    {
        vsize_send = 16384;
    }
    else if (peer->socket_session->new_send_balance_num.load() > 102400)
    {
        vsize_send = 8192;
    }
    else
    {
        vsize_send = 4096;
    }

    //small file and content
    if (sq_obj->content_length < 307200)
    {
        vsize_send = 16384;
    }
    if (sq_obj->content_length > 20971520)
    {
        vsize_send = 8192;
    }

    sq_obj->cache_data.resize(vsize_send);

    vsize_send            = sq_obj->cache_data.size() - 9;
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
        //send file
        if (sq_obj->current_num < sq_obj->content_length)
        {
            per_size = fread(&sq_obj->cache_data[9], 1, vsize_send, sq_obj->fp.get());
            if (per_size == 0 && vsize_send > 0)
            {
                peer->socket_session->http2_send_enddata(peer->stream_id);
                peer->issend         = true;
                sq_obj->standby_next = true;
                return false;
            }
            sq_obj->cache_data.resize(9 + per_size);
        }
    }
    else
    {
        per_size = vsize_send;
        if ((sq_obj->current_num + per_size) > sq_obj->content.size())
        {
            per_size = sq_obj->content.size() - sq_obj->current_num;
        }
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

    data_send_id = peer->socket_session->http2_loop_send_queue_add(sq_obj->cache_data);
    if (sq_obj->current_num >= sq_obj->content_length)
    {
        peer->issend = true;
    }

    peer->socket_session->has_send_update_num += per_size + 9;
    peer->socket_session->new_send_balance_num -= (per_size + 9);

    if (data_send_id > 3)
    {
        sq_obj->sleep_time = sq_obj->sleep_time * 2 + 1000000;

        if (sq_obj->sleep_time > 10000000000)
        {
            if (data_send_id > 5)
            {
                peer->socket_session->http2_send_rst_stream(peer->stream_id, 1);
                peer->isclose = true;
                peer->issend  = true;
                return false;
            }
            sq_obj->sleep_time = 5000000000;
        }
    }
    else if (data_send_id > 0)
    {
        sq_obj->sleep_time = 2000000;
    }
    else
    {
        sq_obj->sleep_time = 1000000;
        if (sq_obj->content_length > 16793600)
        {
            sq_obj->sleep_time = 1500000;
        }
    }

    if (sq_obj->current_num < 20480)
    {
        sq_obj->sleep_time = 500000;
    }
    //small file and content
    if (sq_obj->content_length < 307200)
    {
        sq_obj->sleep_time = 50000;
    }

    if (peer->socket_session->new_send_balance_num.load() < 40960)
    {
        if (data_send_id < 3)
        {
            sq_obj->sleep_time = 200000000;
        }
    }
    else if (peer->socket_session->new_send_balance_num.load() < 81920)
    {
        if (data_send_id < 3)
        {
            sq_obj->sleep_time = 12000000;
        }
    }
    else if (peer->socket_session->new_send_balance_num.load() < 102400)
    {
        if (data_send_id < 3)
        {
            sq_obj->sleep_time = 2000000;
        }
    }
    sq_obj->standby_next = true;
    sq_obj->last_time    = std::chrono::steady_clock::now();
    return true;
}

void httpserver::http2_send_queue_loop([[maybe_unused]] unsigned char index_id)
{
    DEBUG_LOG("http2_send_queue_loop");

    std::list<std::shared_ptr<http2_send_data_t>> thread_sent_data_list;
    unsigned int send_loop_count = 0;
    unsigned int mini_sleep_num  = 0;
    while (true)
    {
        DEBUG_LOG("http2_send_queue_loop send_data_mutex");

        std::unique_lock lk(send_data_mutex);
        send_data_condition.wait(lk, [this]
                                 { return sent_data_list.size() > 0; });
        auto tpsend = sent_data_list.front();
        sent_data_list.pop_front();
        lk.unlock();
        if (isstop)
        {
            break;
        }

        thread_sent_data_list.emplace_back(tpsend);
        send_loop_count = 0;

        for (;; send_loop_count++)
        {
            if (send_loop_count % 13 == 0)
            {
                std::unique_lock lk(send_data_mutex);
                if (sent_data_list.size() > 0)
                {
                    auto tpsend = sent_data_list.front();
                    sent_data_list.pop_front();
                    lk.unlock();
                    thread_sent_data_list.emplace_back(tpsend);
                }
                else
                {
                    lk.unlock();
                }
            }
            mini_sleep_num                = 111222000;
            unsigned int send_count_num_i = 0;

            const std::chrono::time_point<std::chrono::steady_clock> sq_start = std::chrono::steady_clock::now();

            for (auto iter = thread_sent_data_list.begin(); iter != thread_sent_data_list.end();)
            {
                std::shared_ptr<http2_send_data_t> sp = *iter;
                if (sp->peer->isclose || sp->peer.use_count() == 0 || sp->peer->socket_session.use_count() == 0 || sp->peer->issend || sp->peer->socket_session->isclose)
                {
                    thread_sent_data_list.erase(iter++);
                    http2_send_queue &send_queue_obj = get_http2_send_queue();
                    send_queue_obj.back_cache_ptr(sp);
                    continue;
                }

                long long sq_obj_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(sq_start - sp->last_time).count();
                if (sq_obj_duration > sp->sleep_time || sp->current_num == 0)
                {
                    if (sp->standby_next && sp->peer->socket_session->window_update_num.load() > sp->peer->socket_session->has_send_update_num.load())
                    {
                        sp->standby_next = false;

                        if (http2_loop_send_sequence(sp))
                        {

                            send_count_num_i++;
                        }
                        else
                        {
                            thread_sent_data_list.erase(iter++);
                            http2_send_queue &send_queue_obj = get_http2_send_queue();
                            send_queue_obj.back_cache_ptr(sp);
                            continue;
                        }

                        DEBUG_LOG("http2_loop_send_sequence %llu", sp->sleep_time);
                    }
                }
                if (mini_sleep_num > sp->sleep_time)
                {
                    mini_sleep_num = sp->sleep_time;
                }
                iter++;
            }
            if (thread_sent_data_list.size() == 0)
            {
                break;
            }
            //to fast
            if (send_count_num_i == 0 && mini_sleep_num < 40000)
            {
                mini_sleep_num += 300000;
            }

            mini_sleep_num = std::ceil((double)mini_sleep_num / 1000);
            std::this_thread::sleep_for(std::chrono::microseconds(mini_sleep_num));
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
    //unsigned frame_count_per_second = 0;

    for (;;)
    {
        auto time_in_seconds = time_point_cast<seconds>(system_clock::now());
        //++frame_count_per_second;
        if (time_in_seconds > prev_time_in_seconds)
        {

            //frame_count_per_second = 0;
            prev_time_in_seconds = time_in_seconds;
            ++fps;

            if (!this->websockettasks.empty())
            {
                for (auto iter = websockettasks.begin(); iter != websockettasks.end();)
                {
                    std::shared_ptr<httppeer> peer = iter->lock();
                    try
                    {

                        if (peer)
                        {
                            if (peer->websockets->timeloop_num > 0 && fps % peer->websockets->timeloop_num == 0)
                            {
                                peer->websockets->pushloop();
                            }

                            if (peer->websockets->timeloop_num == 0)
                            {
                                peer.reset();
                                websockettasks.erase(iter++);
                            }
                            else
                            {
                                ++iter;
                            }
                        }
                        else
                        {
                            websockettasks.erase(iter++);
                        }
                    }
                    catch (...)
                    {
                        websockettasks.erase(iter++);
                    }
                }
            }

            if (!this->clientlooptasks.empty())
            {
                for (auto iter = clientlooptasks.begin(); iter != clientlooptasks.end();)
                {
                    try
                    {
                        if (iter->second->timeloop_num > 0 && (fps % iter->second->timeloop_num) == 0)
                        {
                            clientrunpool.addclient(iter->second);
                        }
                        if (iter->second->timecount_num == 0 || iter->second->timeloop_num == 0)
                        {
                            clientlooptasks.erase(iter++);
                        }
                        else
                        {
                            ++iter;
                        }
                    }
                    catch (...)
                    {
                        clientlooptasks.clear();
                    }
                }
            }

            if (fps % 13 == 0)
            {
                unsigned int session_num = 0;

                std::unique_lock lk(wait_clear_mutex);
                session_num = socket_session_wait_clear.size();
                lk.unlock();

                if (session_num > 0)
                {
                    log_item.clear();
                    log_item.append("-- clear sock num ");
                    log_item.append(std::to_string(session_num));
                    log_item.append(" --\n");
                    std::unique_lock<std::mutex> lock(log_mutex);
                    error_loglist.emplace_back(log_item);
                    lock.unlock();

                    for (unsigned int i = 0; i < 100; i++)
                    {
                        std::unique_lock lk(wait_clear_mutex);
                        if (socket_session_wait_clear.size() > 0)
                        {
                            auto p_sock_session = std::move(socket_session_wait_clear.front());
                            socket_session_wait_clear.pop_front();
                            lk.unlock();
                            p_sock_session->stop();
                        }
                        else
                        {
                            lk.unlock();
                            break;
                        }

                        DEBUG_LOG("socket_session_wait_clear stop");
                    }
                }
            }

            if (fps > 31536000)
            {
                fps = 1;
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
        asio::error_code ec_error;

        DEBUG_LOG("https accept ok!");
        peer_session->sslsocket->handshake(asio::ssl::stream_base::server, ec_error);
        if (ec_error)
        {
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(ec_error.message());
            lock.unlock();
            DEBUG_LOG(" handshake ec_error ! %s\n", ec_error.message().c_str());

            peer_session->sslsocket->shutdown(ec_error);
            if (peer_session->sslsocket->lowest_layer().is_open())
            {
                peer_session->sslsocket->lowest_layer().close();
            }
            co_return;
        }

#ifndef DEBUG

        const unsigned char *for_next_proto = nullptr;
        unsigned int next_proto_len         = 0;
        SSL_get0_alpn_selected(peer_session->sslsocket->native_handle(), &for_next_proto, &next_proto_len);
        if (next_proto_len > 0)
        {
            if (for_next_proto[0] == 'h' && for_next_proto[1] == '2')
            {
                DEBUG_LOG(" h2 ");
            }
        }

#endif

        co_spawn(this->io_context, clientpeerfun(peer_session, true), asio::detached);
        co_return;
    }
    catch (std::exception &e)
    {
    }
    co_return;
}
void httpserver::listeners()
{
    serverconfig &sysconfigpath = getserversysconfig();
    unsigned short portnum      = sysconfigpath.get_ssl_port();

    asio::error_code ec_error;
    asio::ip::tcp::acceptor acceptor(this->io_context);
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), portnum);
    acceptor.open(endpoint.protocol());

    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.set_option(asio::ip::tcp::no_delay(true));
#if (defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)) && !defined(__CYGWIN__)

    typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
    acceptor.set_option(reuse_port(true));

#endif

    acceptor.bind(endpoint, ec_error);
    acceptor.listen(asio::socket_base::max_connections, ec_error);
    if (ec_error)
    {

        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back(" acceptor listen https error ");
        lock.unlock();
        DEBUG_LOG("Acceptor listen https error ");
        exit(1);
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
        //use_certificate_file
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
    //SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_AUTO_RETRY);
    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_RELEASE_BUFFERS);

    if (SSL_CTX_set_cipher_list(context_.native_handle(), DEFAULT_CIPHER_LIST) == 0)
    {
        //std::cerr << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back(ERR_error_string(ERR_get_error(), nullptr));
        lock.unlock();
    }

    // std::vector<unsigned char> next_proto;
    // next_proto = get_default_alpn();

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

    for (;;)
    {
        try
        {
            asio::ip::tcp::socket socket(this->io_context);
            std::shared_ptr<client_session> peer_session = std::make_shared<client_session>();
            peer_session->sslsocket                      = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(socket), context_);
            peer_session->isssl                          = true;

            acceptor.accept(peer_session->sslsocket->lowest_layer(), ec_error);
            if (ec_error)
            {
                std::unique_lock<std::mutex> lock(log_mutex);
                error_loglist.emplace_back("https accept ec_error ");
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::nanoseconds(200));
                continue;
            }
            std::unique_lock<std::mutex> lock_sock(socket_session_lists_mutex);
            socket_session_lists.push_back(peer_session);
            lock_sock.unlock();

            total_http2_count++;
            co_spawn(this->io_context, sslhandshake(peer_session), asio::detached);
        }
        catch (std::exception &e)
        {
        }
        if (isstop)
        {
            break;
        }
    }
}
void httpserver::listener()
{
    serverconfig &sysconfigpath = getserversysconfig();
    asio::error_code ec;

    unsigned short portnum = sysconfigpath.get_port();

    asio::ip::tcp::acceptor acceptor(this->io_context);

    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), portnum);
    acceptor.open(endpoint.protocol());

    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

#if (defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)) && !defined(__CYGWIN__)

    typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
    acceptor.set_option(reuse_port(true));

#endif

    acceptor.bind(endpoint, ec);
    acceptor.listen(asio::socket_base::max_connections, ec);
    if (ec)
    {
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back("  acceptor listen http error  ");
        lock.unlock();
        DEBUG_LOG("Acceptor listen http error ");
        exit(1);
    }
    DEBUG_LOG("http accept");
    for (;;)
    {
        try
        {
            // asio::ip::tcp::socket socket(this->io_context);
            std::shared_ptr<client_session> peer_session = std::make_shared<client_session>();
            peer_session->socket                         = std::make_unique<asio::ip::tcp::socket>(this->io_context);
            peer_session->isssl                          = false;

            acceptor.accept(*peer_session->socket, ec);
            if (ec)
            {
                std::unique_lock<std::mutex> lock(log_mutex);
                error_loglist.emplace_back("http accept ec_error ");
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::nanoseconds(200));
                continue;
            }
            std::unique_lock<std::mutex> lock_sock(socket_session_lists_mutex);
            socket_session_lists.push_back(peer_session);
            lock_sock.unlock();

            total_http1_count++;
            co_spawn(this->io_context, clientpeerfun(peer_session, false), asio::detached);
        }
        catch (std::exception &e)
        {
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
void httpserver::httpwatch()
{
    serverconfig &sysconfigpath = getserversysconfig();
    try
    {
        mysqlconfig_init_link();
    }
    catch (const char *e)
    {
        std::string errorstr(e);
        errorstr.push_back('\n');
        error_loglist.emplace_back(errorstr);
    }
    std::unique_lock<std::mutex> loglock(log_mutex);
    error_loglist.push_back("------------begin-----------");
    error_loglist.push_back(get_date("%Y-%m-%d %X\n"));
    loglock.unlock();

    struct regmethold_t temp;
    temp.pre    = nullptr;
    temp.regfun = [self = this](std::shared_ptr<httppeer> peer) -> std::string
    {
        httppeer &client = peer->get_peer();
        client << "<h3 align=\"center\">";
        client << "<span style=\"font-size:2em\">🧨 Paozhu</h3> <p align=\"center\">Version ";
        client << (PAOZHU_VERSION / 100000);
        client << ".";
        client << (PAOZHU_VERSION / 100 % 1000);
        client << ".";
        client << (PAOZHU_VERSION % 100);
        client << "</p>";

        int isshow = -1;
        if (client.get.isset("show_visitinfo"))
        {
            isshow                             = client.get["show_visitinfo"].to_int();
            server_loaclvar &static_server_var = get_server_global_var();
            if (isshow == 1 && static_server_var.debug_enable)
            {
                static_server_var.show_visitinfo = true;
                client << "<p>";
                client << "online:" << self->total_count.load();
                client << " ";
                try
                {
                    std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &mysqldbpoolglobal = get_mysqlpool();
                    isshow                                                                         = 0;
                    for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
                    {
                        client << " [db:" << iter->second->select_link.db
                               << " select:" << std::to_string(iter->second->select_current_num);
                        client << " edit:" << std::to_string(iter->second->edit_current_num);
                        ++isshow;
                    }
                    client << "]</p>";
                    client << self->clientrunpool.printthreads(true);
                }
                catch (...)
                {
                    client << "<p>exception</p>";
                }
            }
            else
            {
                static_server_var.show_visitinfo = false;
            }
        }

        return "";
    };
    _http_regmethod_table.emplace("paozhu_status", std::move(temp));
    temp.pre    = nullptr;
    temp.regfun = [self = this](std::shared_ptr<httppeer> peer) -> std::string
    {
        if (peer->linktype != 7)
        {
            return "";
        }
        if (peer->pathinfos.size() > 0 && peer->etag.size() > 0)
        {
            std::string temptaskhash = peer->pathinfos[0];
            temptaskhash.append(peer->url);
            std::size_t temp_name_id = std::hash<std::string>{}(temptaskhash);
            std::ostringstream oss;
            oss << temp_name_id;
            temptaskhash = oss.str();

            if (temptaskhash == peer->etag)
            {
                bool isintask = true;
                for (auto iter = self->clientlooptasks.begin(); iter != self->clientlooptasks.end();)
                {
                    if (iter->first == temp_name_id)
                    {
                        isintask = false;
                        break;
                    }
                }
                if (isintask)
                {
                    self->clientlooptasks.push_back({temp_name_id, peer});
                    self->websocketcondition.notify_one();
                }
            }
        }
        return "";
    };
    _http_regmethod_table.emplace("frametasks_timeloop", std::move(temp));

    unsigned int updatetimetemp = 0;
    std::string currentpath;
    std::string error_path;
    std::string error_msg_loop;
    std::string traffic_switch_file;

    server_loaclvar &static_server_var = get_server_global_var();
    currentpath                        = static_server_var.log_path;

    if (currentpath.size() > 0 && currentpath.back() != '/')
    {
        currentpath.push_back('/');
    }
    error_path          = currentpath;
    traffic_switch_file = currentpath;
    currentpath.append("access.log");
    error_path.append("error.log");
    traffic_switch_file.append("traffic_switch_file");

    unsigned int mysqlpool_time  = 1;
    unsigned int remove_linknum  = 0;
    unsigned int old_total_count = 0;

    unsigned int old_ten_total_count = 0;

    std::size_t n_write     = 0;
    unsigned char cron_type = 0x00;
    unsigned char cron_day  = 0x00;
    unsigned char cron_hour = 0x00;

    unsigned int clean_cron_min      = 60;
    unsigned int clean_cron_time_ago = 0;

    unsigned int restart_process_num = 0;
    int restart_process_time_start   = 0;
    int restart_process_time_end     = 0;

    unsigned char plan_http1_exit = 0x00;
    unsigned char plan_http2_exit = 0x00;

    bool is_clear_sock = false;

    //reboot server
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
            for (unsigned int i = 1; i < sysconfigpath.map_value["default"]["reboot_cron"].size(); ++i)
            {
                if (sysconfigpath.map_value["default"]["reboot_cron"][i] >= '0' && sysconfigpath.map_value["default"]["reboot_cron"][i] <= '9')
                {
                    cron_day = cron_day * 10 + (sysconfigpath.map_value["default"]["reboot_cron"][i] - '0');
                }
                else
                {
                    if (sysconfigpath.map_value["default"]["reboot_cron"][i] == 'H' || sysconfigpath.map_value["default"]["reboot_cron"][i] == 'h')
                    {
                        for (unsigned int j = i + 1; j < sysconfigpath.map_value["default"]["reboot_cron"].size(); ++j)
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

    if (sysconfigpath.map_value["default"]["clean_cron"].size() > 3)
    {

        if (sysconfigpath.map_value["default"]["clean_cron"][0] == 'M' || sysconfigpath.map_value["default"]["clean_cron"][0] == 'm')
        {
            for (unsigned int i = 1; i < sysconfigpath.map_value["default"]["clean_cron"].size(); ++i)
            {
                if (sysconfigpath.map_value["default"]["clean_cron"][i] >= '0' && sysconfigpath.map_value["default"]["clean_cron"][i] <= '9')
                {
                    clean_cron_min = clean_cron_min * 10 + (sysconfigpath.map_value["default"]["clean_cron"][i] - '0');
                }
                else
                {
                    if (sysconfigpath.map_value["default"]["clean_cron"][i] == 'T' || sysconfigpath.map_value["default"]["clean_cron"][i] == 't')
                    {
                        for (unsigned int j = i + 1; j < sysconfigpath.map_value["default"]["clean_cron"].size(); ++j)
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

    if (sysconfigpath.map_value["default"]["links_restart_process"].size() > 3)
    {

        if (sysconfigpath.map_value["default"]["links_restart_process"][0] == 'N' || sysconfigpath.map_value["default"]["links_restart_process"][0] == 'n')
        {
            for (unsigned int i = 1; i < sysconfigpath.map_value["default"]["links_restart_process"].size(); ++i)
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
#ifndef _WIN32
    struct flock lockstr = {};
#endif
    DEBUG_LOG("httpwatch run");
    for (;;)
    {
        try
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
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
                if (updatetimetemp == 5)
                {
                    clientrunpool.fixthread();
                    updatetimetemp = 0;
                }
            }
            DEBUG_LOG("pool thread tasknum:%d %d", clientrunpool.gettasknum(), clientrunpool.getlivenum());

#ifdef DEBUG
            clientrunpool.printthreads(false);
#endif

            // save access.log
            if (!access_loglist.empty())
            {
#ifndef _MSC_VER
                int fd = open(currentpath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
                if (fd == -1)
                {
                    continue;
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
                    continue;
                }
#else
                auto native_handle = (HANDLE)_get_osfhandle(fd);
                auto file_size     = GetFileSize(native_handle, nullptr);
                if (!LockFile(native_handle, file_size, 0, file_size, 0))
                {
                    close(fd);
                    continue;
                }
#endif
                std::unique_lock<std::mutex> loglock(log_mutex);
                while (!access_loglist.empty())
                {
                    n_write = write(fd, access_loglist.front().data(), access_loglist.front().size());
                    access_loglist.pop_front();
                }
                loglock.unlock();

#ifndef _WIN32
                lockstr.l_type = F_UNLCK;
                if (fcntl(fd, F_SETLK, &lockstr) == -1)
                {
                    close(fd);
                    continue;
                }
#else
                if (!UnlockFile(native_handle, file_size, 0, file_size, 0))
                {
                    close(fd);
                    continue;
                }
#endif
                close(fd);
#endif
            }

            // save error.log
            if (!error_loglist.empty())
            {
#ifndef _MSC_VER
                int fd = open(error_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
                if (fd == -1)
                {
                    continue;
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
                    continue;
                }
#else
                auto native_handle = (HANDLE)_get_osfhandle(fd);
                auto file_size     = GetFileSize(native_handle, nullptr);
                if (!LockFile(native_handle, file_size, 0, file_size, 0))
                {
                    close(fd);
                    continue;
                }
#endif

                std::unique_lock<std::mutex> loglock(log_mutex);
                while (!error_loglist.empty())
                {
                    n_write = write(fd, error_loglist.front().data(), error_loglist.front().size());
                    error_loglist.pop_front();
                }
                loglock.unlock();

#ifndef _WIN32
                lockstr.l_type = F_UNLCK;
                if (fcntl(fd, F_SETLK, &lockstr) == -1)
                {
                    close(fd);
                    continue;
                }
#else
                if (!UnlockFile(native_handle, file_size, 0, file_size, 0))
                {
                    close(fd);
                    continue;
                }
#endif
                close(fd);
#endif
            }
            if (n_write > 0)
            {
                n_write = 0;
            }

            std::time_t t = std::time(nullptr);
            std::tm *now  = std::localtime(&t);

            if (now->tm_min == 2 && now->tm_sec < 9)
            {
                if (total_count.load() == old_total_count)
                {
                    std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &mysqldbpoolglobal = get_mysqlpool();
                    for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
                    {
                        iter->second->clearpool();
                        iter->second->addpool_select_connect();
                        iter->second->addpool_edit_connect();
                        DEBUG_LOG("mysql pool clearpoool ");
                    }
                }
            }

            if (now->tm_hour < 3 && mysqlpool_time > 82800)
            {
                std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &mysqldbpoolglobal = get_mysqlpool();
                for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
                {
                    iter->second->clearpool();
                }
                mysqlpool_time = 1;
            }

            mysqlpool_time += 1;
            DEBUG_LOG("clear mysql poll time:%d,client live:%d", mysqlpool_time, total_count.load());
            DEBUG_LOG("cron type:%c day:%d,hour:%d min:%d", cron_type, cron_day, cron_hour, now->tm_min);

            //cron reboot process
            if (cron_type > 0 && cron_day > 0 && cron_hour > 0 && mysqlpool_time > 40 && now->tm_min < 3)
            {
                if (cron_type == 'd')
                {
                    if ((now->tm_yday + 1) % cron_day == 0)
                    {
                        if (cron_hour > 0 && now->tm_hour == cron_hour)
                        {
                            isstop = true;
                        }
                    }
                }
                else if (cron_type == 'm')
                {
                    if (now->tm_mday == cron_day)
                    {
                        if (cron_hour > 0 && now->tm_hour == cron_hour)
                        {
                            isstop = true;
                        }
                    }
                }
                else if (cron_type == 'w')
                {
                    if (cron_day == 7 && 0 == now->tm_wday)
                    {
                        if (cron_hour > 0 && now->tm_hour == cron_hour)
                        {
                            isstop = true;
                        }
                    }
                    else if (now->tm_wday == cron_day)
                    {
                        if (cron_hour > 0 && now->tm_hour == cron_hour)
                        {
                            isstop = true;
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
                                isstop = true;
                            }
                        }
                    }
                }
                if (isstop)
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
                    std::unique_lock<std::mutex> loglock(log_mutex);
                    for (unsigned int i = 0; i < 10; i++)
                    {
                        error_loglist.push_back(logstr);
                    }
                    loglock.unlock();
                    cron_type = 0x00;

                    continue;
                }
            }

            //clear timeout sock 10 minute
            if (clean_cron_min > 0 && mysqlpool_time % clean_cron_min == 0)
            {
                is_clear_sock = true;
            }
            if (is_clear_sock)
            {
                unsigned int nowtimeid         = timeid();
                unsigned int erase_count_num   = 0;
                unsigned int ok_count_num      = 0;
                unsigned int session_count_num = 0;
                nowtimeid -= clean_cron_time_ago;

                std::unique_lock<std::mutex> lock_sock(socket_session_lists_mutex);
                session_count_num = socket_session_lists.size();
                lock_sock.unlock();

                remove_linknum = session_count_num / 20;
                if (remove_linknum < 200)
                {
                    remove_linknum = 200;
                }
                for (unsigned int ic = 0; ic < session_count_num; ic += remove_linknum)
                {
                    unsigned int jc = 0;
                    std::unique_lock<std::mutex> lock_sock(socket_session_lists_mutex);
                    for (auto iter = socket_session_lists.begin(); iter != socket_session_lists.end();)
                    {
                        std::shared_ptr<client_session> p_session = iter->lock();
                        if (p_session)
                        {
                            if (p_session->time_limit.load() > 100 && p_session->time_limit.load() < nowtimeid)
                            {
                                std::unique_lock lk(wait_clear_mutex);
                                socket_session_wait_clear.push_back(std::move(p_session));
                                lk.unlock();
                                DEBUG_LOG("socket_session_wait_clear push_back");
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
                            socket_session_lists.erase(iter++);
                            ok_count_num++;
                        }
                        jc++;
                        if (jc > remove_linknum)
                        {
                            break;
                        }
                    }
                    lock_sock.unlock();
                    if (jc == 0)
                    {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }

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

                error_msg_loop.append(" --\n");
                std::unique_lock<std::mutex> loglock(log_mutex);
                error_loglist.push_back(error_msg_loop);
                loglock.unlock();
                is_clear_sock = false;
            }
            //
            if (mysqlpool_time % 128 == 0)
            {
                error_msg_loop.clear();
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

                error_msg_loop.append(" --\n");
                std::unique_lock<std::mutex> loglock(log_mutex);
                error_loglist.push_back(error_msg_loop);
                loglock.unlock();
            }

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
            //may be asio post pool is die;
            if (total_http2_count.load() > 2)
            {
                error_msg_loop.clear();
                error_msg_loop.append("-- total_http2_count ");
                error_msg_loop.append(std::to_string(total_http1_count.load()));
                error_msg_loop.append(" --\n");

                if (plan_http2_exit == 0)
                {
                    old_ten_total_count = old_total_count;
                }

                plan_http2_exit++;
                if (plan_http2_exit > 1)
                {
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
            if (total_http1_count.load() > 2)
            {
                error_msg_loop.clear();
                error_msg_loop.append("-- total_http1_count ");
                error_msg_loop.append(std::to_string(total_http1_count.load()));
                error_msg_loop.append(" --\n");

                if (plan_http1_exit == 0)
                {
                    old_ten_total_count = old_total_count;
                }

                plan_http1_exit++;
                if (plan_http1_exit > 1)
                {
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

            //Check every 5 seconds for over 12000
            if (now->tm_hour < restart_process_time_end && now->tm_hour > restart_process_time_start)
            {
                if (restart_process_num > 0)
                {
                    if (old_total_count > restart_process_num && total_count.load() == old_total_count)
                    {
                        std::unique_lock<std::mutex> loglock(log_mutex);
                        error_loglist.push_back("--restart_process_time_start--\n");
                        loglock.unlock();
                        isstop = true;
                        continue;
                    }
                }
            }
            old_total_count = total_count.load();
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("frame thread:%s", e.what());
            isstop = true;
        }
        if (isstop)
        {
            DEBUG_LOG("std::abort");
            break;
        }
    }
    //std::abort();
    std::terminate();
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
        //not use
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
void httpserver::run(const std::string &sysconfpath)
{
    try
    {
        isstop = false;
        _initauto_control_httpmethodregto(_http_regmethod_table);
        _inithttpmethodregto(_http_regmethod_table);
        _inithttpmethodregto_pre(_http_regmethod_table);
        _initauto_control_httprestful_paths(_http_regurlpath_table);

        _initauto_domain_httpmethodregto(_domain_regmethod_table);
        _initauto_domain_httprestful_paths(_domain_regurlpath_table);

        _initauto_co_control_httpmethodregto(_co_http_regmethod_table);
        _initauto_co_domain_httpmethodregto(_co_domain_regmethod_table);

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
        server_loaclvar &static_server_var = get_server_global_var();

        debug_log::instance().setDebug(!static_server_var.deamon_enable);
        debug_log::instance().setLogfile(static_server_var.log_path);

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

        total_count = sysconfigpath.get_co_thread_num();
        if (total_count < std::thread::hardware_concurrency())
        {
            total_count = std::thread::hardware_concurrency();
            total_count += 2;
        }
        if (total_count < 8)
        {
            total_count = 8;
        }
        //asio::io_context::work worker(io_context);
        asio::executor_work_guard<asio::io_context::executor_type> worker(io_context.get_executor());

        for (std::size_t i = 0; i < total_count; ++i)
        {
            runthreads.emplace_back(
                [this]()
                {
                    std::ostringstream oss;
                    oss << std::this_thread::get_id();
                    std::string tempthread = oss.str();
                    DEBUG_LOG("frame thread:%s", tempthread.c_str());
                    this->io_context.run();
                });
        }
        total_count              = 0;
        clientrunpool.io_context = &io_context;

        std::thread httpwatch(std::bind(&httpserver::httpwatch, this));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::thread https(std::bind(&httpserver::listeners, this));
        std::thread http(std::bind(&httpserver::listener, this));

        {
            client_context &client_context = get_client_context_obj();
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

        if (https.joinable())
        {
            https.join();
        }
        if (http.joinable())
        {
            http.join();
        }

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
    }
    catch (std::exception &e)
    {
        LOG_ERROR << " httpserver Exception " << e.what() << LOG_END;
    }
    DEBUG_LOG("httpserver exit!");
}
void httpserver::stop()
{
    isstop                         = true;
    client_context &client_context = get_client_context_obj();
    client_context.stop();
    websocketcondition.notify_all();
    clientrunpool.stop();
    io_context.stop();
}
}// namespace http
