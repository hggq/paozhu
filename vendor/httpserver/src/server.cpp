#include <zlib.h>
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
#include "httphook.h"
#include "func.h"
#include "mysqlconfig.h"
#include "mysqlpool.h"
#include "autocontrolmethod.hpp"
#include "reghttpmethod.hpp"
#include "reghttpmethod_pre.hpp"
#include "regviewmethod.hpp"

#ifdef ENABLE_BOOST
#include "loadviewso.h"
#include "loadmodule.h"
#include "http_so_common_api.h"
#endif

#include "mysqlproxyfun.h"
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
bool httpserver::http2_send_file_range(std::shared_ptr<httppeer> peer)
{
    std::string _send_header;
    std::string _send_data;
    FILE_AUTO fp(std::fopen(peer->sendfilename.c_str(), "rb"), &std::fclose);
    if (fp.get())
    {

        fseek(fp.get(), 0, SEEK_END);
        unsigned long long file_size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);
        std::string htmlcontent;

        std::string fileexttype;
        std::string mime_value     = "text/html; charset=utf-8";
        unsigned short statecode   = 200;
        unsigned long long readnum = 0, mustnum = 0;
        peer->compress = 0;

        if (peer->state.rangebegin >= file_size || peer->state.rangeend < 0 || peer->state.rangeend > file_size)
        {
            peer->status(400);
            peer->length(0);
            peer->type(mime_value);
            _send_header = peer->make_http2_header(HTTP2_HEADER_END_STREAM);
            peer->socket_session->send_data(_send_header);
            peer->socket_session->send_enddata(peer->stream_id);
            return false;
        }

        unsigned int filebasesize   = peer->sendfilename.size();
        unsigned int filenameoffset = 0;

        if (filebasesize > 0)
        {
            for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
            {
                if (peer->sendfilename[filenameoffset] == '.')
                {
                    break;
                }
            }
            filenameoffset += 1;
            for (; filenameoffset < filebasesize; filenameoffset++)
            {
                fileexttype.push_back(peer->sendfilename[filenameoffset]);
            }
        }

        if (fileexttype.size() > 0)
        {
            mime_value = mime_map[fileexttype];
            if (mime_value.empty())
            {
                if (file_size > 20480)
                {
                    mime_value = "application/octet-stream";
                }
                else
                {
                    mime_value = "text/html; charset=utf-8";
                }
            }
        }
        else
        {
            if (file_size > 20480)
            {
                mime_value = "application/octet-stream";
            }
            else
            {
                mime_value = "text/plain";
            }
        }

        mustnum = file_size;
        if (peer->state.rangebegin > 0)
        {
            readnum   = peer->state.rangebegin;
            statecode = 206;
        }
        if (peer->state.rangeend > 0)
        {
            mustnum   = peer->state.rangeend + 1;
            statecode = 206;
        }

        peer->status(statecode);
        peer->length(mustnum - readnum);
        peer->type(mime_value);

        if (statecode == 206)
        {
            peer->set_header("content-range",
                             "bytes " + std::to_string(readnum) + "-" + std::to_string(mustnum - 1) + "/" +
                                 std::to_string(file_size));
        }
        else
        {
            peer->set_header("accept-ranges", "bytes");
        }
        DEBUG_LOG("start send file range");
        peer->set_header("date", get_gmttime());
        peer->set_header("last-modified", get_gmttime((unsigned long long)peer->fileinfo.st_mtime));

        std::string etag;
        etag = make_header_etag(file_size, peer->fileinfo.st_mtime + peer->url.size());

        peer->set_header("etag", etag);

        _send_header = peer->make_http2_header();

        peer->socket_session->send_data(_send_header);

        unsigned int data_send_id = peer->stream_id;
        data_send_id              = 0;

        fseek(fp.get(), readnum, SEEK_SET);

        if ((mustnum - readnum) > 16877215)
        {
            struct http2sendblock_t temp_send_obj;
            temp_send_obj.filename   = peer->sendfilename;
            temp_send_obj.isfinish   = false;
            temp_send_obj.peer       = peer;
            temp_send_obj.offset     = readnum;
            temp_send_obj.total_size = mustnum;
            temp_send_obj.last_time  = std::chrono::steady_clock::now();
            temp_send_obj.last_size  = 0;
            temp_send_obj.pre_size   = 0;
            temp_send_obj.pre_count  = 0;
            temp_send_obj.stream_id  = peer->stream_id;
            temp_send_obj.fp         = std::move(fp);

            http2send_tasks.emplace_back(std::move(temp_send_obj));
            http2condition.notify_one();
            return true;
        }

        // unsigned int jj = 0;

        sendqueue &send_cache_list     = get_sendqueue();
        struct sendqueue_t *send_cache = send_cache_list.get_cache_ptr();
        sendqueue_back unsetcahceback;
        unsetcahceback.setptr(send_cache);

        if (send_cache == nullptr)
        {
            peer->socket_session->send_enddata(peer->stream_id);
            // peer->socket_session->send_goway();
            return false;
        }

        unsigned long long totalsend_num = 0;
        unsigned int vsize_send          = 8181;
        if (readnum >= mustnum)
        {
            send_cache->data[3] = 0x00;
            send_cache->data[4] = 0x01;
            data_send_id        = peer->stream_id;
            send_cache->data[8] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[7] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[6] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[5] = data_send_id & 0x7F;

            send_cache->data[2] = 0;
            send_cache->data[1] = 0;
            send_cache->data[0] = 0;

            if (peer->isclose)
            {
                return true;
            }
            if (peer->socket_session->isclose)
            {
                return true;
            }

            if (peer->socket_session->send_data(send_cache->data, 9))
            {
            }
            return true;
        }
        for (unsigned long long m = readnum; m < mustnum;)
        {
            int per_size = 0;

            send_cache->data[3] = 0x00;
            send_cache->data[4] = 0x00;
            data_send_id        = peer->stream_id;
            send_cache->data[8] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[7] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[6] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[5] = data_send_id & 0x7F;

            if (m < mustnum)
            {
                memset(&send_cache->data[9], 0x00, vsize_send);
                data_send_id = vsize_send;
                if ((mustnum - m) < vsize_send)
                {
                    data_send_id = mustnum - m;
                }

                per_size = fread(&send_cache->data[9], 1, data_send_id, fp.get());
                m += per_size;
            }
            if (m >= mustnum)
            {
                send_cache->data[4] = 0x01;
            }

            data_send_id        = per_size;
            send_cache->data[2] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[1] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[0] = data_send_id & 0xFF;

            if (peer->isclose)
            {
                DEBUG_LOG("---  peer->isclose --------");

                break;
            }
            if (peer->socket_session->isclose)
            {
                DEBUG_LOG("---  peer->socket_session->isclose exit --------");
                return true;
            }

            if (peer->socket_session->send_data(send_cache->data, per_size + 9))
            {
            }
            else
            {
                LOG_ERROR << " send_data error " << LOG_END;
                return false;
            }

            peer->socket_session->window_update_num -= per_size;
            totalsend_num += per_size;
            if (peer->socket_session->window_update_num < 8192)
            {
                DEBUG_LOG("--- wait window_update_num --------");
                std::promise<int> p;
                std::future<int> f{p.get_future()};

                peer->window_update_results.emplace_back(std::move(f));
                peer->window_update_promise = std::move(p);
                peer->window_update_bool    = true;
                peer->socket_session->promise_list.emplace_back(peer->stream_id);
                try
                {
                    peer->window_update_results.front().get();
                    peer->window_update_results.pop_front();
                    DEBUG_LOG("--- from window_update_num --------");
                }
                catch (const std::exception &e)
                {
                    // peer->window_update_results.clear();
                }
            }

            if (file_size > 10485760)
            {
                if (peer->socket_session->window_update_num < 1048576)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            else if (file_size > 1048576)
            {
                if (m < 20096 && peer->socket_session->window_update_num < 1048576)
                {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(512));
                }
            }
        }
        DEBUG_LOG("send file ok!");
        return false;
    }
    else
    {
        std::string stfilecom = "<h3>500 Internal Server Error</h3>";
        stfilecom.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
        peer->status(500);
        peer->length(stfilecom.size());
        peer->type("text/html; charset=utf-8");
        _send_header = peer->make_http2_header();
        peer->socket_session->send_data(_send_header);

        http2_send_body(peer, (const unsigned char *)&stfilecom[0], stfilecom.size());
        return false;
    }
    return true;
}
bool httpserver::http2_send_file(std::shared_ptr<httppeer> peer)
{
    std::string _send_header;
    std::string _send_data;
    FILE_AUTO fp(std::fopen(peer->sendfilename.c_str(), "rb"), &std::fclose);
    DEBUG_LOG("%s", peer->sendfilename.c_str());
    if (fp.get())
    {
        fseek(fp.get(), 0, SEEK_END);
        unsigned long long file_size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);
        std::string htmlcontent;
        std::string etag;
        std::string fileexttype;
        std::string mime_value = "text/html; charset=utf-8";
        peer->compress         = 0;

        unsigned int filebasesize   = peer->sendfilename.size();
        unsigned int filenameoffset = 0;

        if (filebasesize > 0)
        {
            for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
            {
                if (peer->sendfilename[filenameoffset] == '.')
                {
                    break;
                }
            }
            filenameoffset += 1;
            for (; filenameoffset < filebasesize; filenameoffset++)
            {
                fileexttype.push_back(peer->sendfilename[filenameoffset]);
            }
        }
        if (fileexttype.size() > 0)
        {
            mime_value = mime_map[fileexttype];
            if (mime_value.empty())
            {
                if (file_size > 20480)
                {
                    mime_value = "application/octet-stream";
                }
                else
                {
                    mime_value = "text/html; charset=utf-8";
                }
            }
        }
        else
        {
            if (file_size > 20480)
            {
                mime_value = "application/octet-stream";
            }
            else
            {
                mime_value = "text/plain";
            }
        }

        etag = make_header_etag(file_size, peer->fileinfo.st_mtime + peer->url.size());
        if (peer->etag == etag)
        {
            peer->status(304);
            peer->length(0);
            peer->set_header("date", get_gmttime());
            peer->set_header("last-modified", get_gmttime((unsigned long long)peer->fileinfo.st_mtime));
            peer->set_header("etag", etag);
            peer->type(mime_value);
            _send_header = peer->make_http2_header();
            set_http2_headers_flag(_send_header, HTTP2_HEADER_END_STREAM | HTTP2_HEADER_END_HEADERS);
            peer->socket_session->send_data(_send_header);
            peer->socket_session->send_enddata(peer->stream_id);
            return true;
        }

        if (file_size < 16877216 && fileexttype.size() > 0 && mime_compress.contains(fileexttype))
        {

            if (peer->state.gzip || peer->state.br)
            {
                // check cache compress content
                bool is_not_cache_content = true;
                std::string path_temp;
                server_loaclvar &static_server_var = get_server_global_var();
                if (etag.size() > 0)
                {
                    if (static_server_var.static_file_compress_cache)
                    {

                        path_temp = static_server_var.temp_path;
                        if (path_temp.size() > 0 && path_temp.back() != '/')
                        {
                            path_temp.push_back('/');
                        }
                        path_temp.append("statichtml");
                        fs::path paths = path_temp;
                        if (!fs::exists(paths))
                        {
                            fs::create_directories(paths);
                            fs::permissions(paths,
                                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                                            fs::perm_options::add);
                        }
                        path_temp.push_back('/');
                        path_temp.push_back(etag[0]);
                        paths = path_temp;
                        if (!fs::exists(paths))
                        {
                            fs::create_directories(paths);
                            fs::permissions(paths,
                                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                                            fs::perm_options::add);
                        }
                        path_temp.push_back('/');
                        path_temp.append(etag);
                        if (peer->isssl && peer->state.br)
                        {
                            peer->compress = 2;
                            path_temp.append(".br");
                        }
                        else if (peer->state.gzip)
                        {
                            peer->compress = 1;
                            path_temp.append(".gzip");
                        }
                        FILE_AUTO fpcompress(std::fopen(path_temp.c_str(), "rb"), &std::fclose);
                        if (fpcompress.get())
                        {
                            fseek(fpcompress.get(), 0, SEEK_END);
                            unsigned long long file_size = ftell(fpcompress.get());
                            fseek(fpcompress.get(), 0, SEEK_SET);
                            htmlcontent.resize(file_size);
                            file_size = fread(&htmlcontent[0], 1, file_size, fpcompress.get());
                            htmlcontent.resize(file_size);
                            is_not_cache_content = false;
                            // if (peer->state.br)
                            // {
                            //     peer->compress = 2;
                            // }
                            // else if (peer->state.gzip)
                            // {
                            //     peer->compress = 1;
                            // }
                        }
                    }
                }
                if (is_not_cache_content)
                {
                    htmlcontent.resize(file_size);
                    file_size = fread(&htmlcontent[0], 1, file_size, fp.get());
                    htmlcontent.resize(file_size);
                    std::string tempcompress;

                    if (peer->compress == 2)
                    {
                        brotli_encode(htmlcontent, tempcompress);
                        // peer->compress = 2;
                        htmlcontent = tempcompress;
                    }
                    else if (peer->compress == 1)
                    {

                        if (compress(htmlcontent.data(), htmlcontent.size(), tempcompress, Z_DEFAULT_COMPRESSION) ==
                            Z_OK)
                        {
                            htmlcontent = tempcompress;
                            // peer->compress = 1;
                        }
                    }
                }

                file_size = htmlcontent.size();
                if (is_not_cache_content && etag.size() > 0)
                {
                    if (static_server_var.static_file_compress_cache)
                    {
                        if (path_temp.size() > 0)
                        {
                            FILE_AUTO fpcompress(std::fopen(path_temp.c_str(), "wb"), &std::fclose);
                            if (fpcompress.get())
                            {
                                fwrite(&htmlcontent[0], 1, htmlcontent.size(), fpcompress.get());
                            }
                        }
                    }
                }
            }
        }

        peer->status(200);
        peer->length(file_size);
        peer->type(mime_value);
        if (peer->compress == 0)
        {
            peer->set_header("accept-ranges", "bytes");
        }
        DEBUG_LOG("start send file");
        peer->set_header("date", get_gmttime());
        peer->set_header("last-modified", get_gmttime((unsigned long long)peer->fileinfo.st_mtime));

        peer->set_header("etag", etag);

        _send_header = peer->make_http2_header();
        peer->socket_session->send_data(_send_header);

        unsigned int data_send_id = peer->stream_id;
        data_send_id              = 0;
        // int jj = 0;
        if (file_size > 16877215)
        {
            struct http2sendblock_t temp_send_obj;
            temp_send_obj.filename   = peer->sendfilename;
            temp_send_obj.isfinish   = false;
            temp_send_obj.peer       = peer;
            temp_send_obj.offset     = 0;
            temp_send_obj.total_size = file_size;
            temp_send_obj.last_time  = std::chrono::steady_clock::now();
            temp_send_obj.last_size  = 0;
            temp_send_obj.pre_size   = 0;
            temp_send_obj.pre_count  = 0;
            temp_send_obj.stream_id  = peer->stream_id;
            temp_send_obj.fp         = std::move(fp);

            http2send_tasks.emplace_back(std::move(temp_send_obj));
            http2condition.notify_one();
            return true;
        }

        sendqueue &send_cache_list     = get_sendqueue();
        struct sendqueue_t *send_cache = send_cache_list.get_cache_ptr();

        sendqueue_back unsetcahceback;
        unsetcahceback.setptr(send_cache);

        if (send_cache == nullptr)
        {
            peer->socket_session->send_enddata(peer->stream_id);
            // peer->socket_session->send_goway();
            return false;
        }

        unsigned long long totalsend_num = 0;
        unsigned int vsize_send          = 8181;
        if (file_size == 0)
        {
            send_cache->data[3] = 0x00;
            send_cache->data[4] = 0x01;
            data_send_id        = peer->stream_id;
            send_cache->data[8] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[7] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[6] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[5] = data_send_id & 0x7F;

            send_cache->data[2] = 0;
            send_cache->data[1] = 0;
            send_cache->data[0] = 0;

            if (peer->isclose)
            {
                return true;
            }
            if (peer->socket_session->isclose)
            {
                return true;
            }
            if (peer->socket_session->send_data(send_cache->data, 9))
            {
            }
            return true;
        }
        for (unsigned long long m = 0; m < file_size;)
        {
            unsigned int per_size = 0;

            send_cache->data[3] = 0x00;
            send_cache->data[4] = 0x00;
            data_send_id        = peer->stream_id;
            send_cache->data[8] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[7] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[6] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[5] = data_send_id & 0x7F;

            if (peer->compress > 0)
            {
                for (; m < file_size;)
                {

                    send_cache->data[9 + per_size] = htmlcontent[m];
                    m += 1;
                    per_size++;
                    if (per_size == vsize_send)
                    {
                        break;
                    }
                }
                send_cache->data[vsize_send + 9] = 0x00;
                if (m >= file_size)
                {
                    send_cache->data[4] = 0x01;
                }
            }
            else
            {
                if (m < file_size)
                {
                    memset(&send_cache->data[9], 0x00, vsize_send);
                    per_size = fread(&send_cache->data[9], 1, vsize_send, fp.get());
                    m += per_size;
                }
                if (m >= file_size)
                {
                    send_cache->data[4] = 0x01;
                }
            }

            data_send_id        = per_size;
            send_cache->data[2] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[1] = data_send_id & 0xFF;
            data_send_id        = data_send_id >> 8;
            send_cache->data[0] = data_send_id & 0xFF;

            if (peer->isclose)
            {
                DEBUG_LOG(" peer->isclose  ");
                LOG_ERROR << "peer->isclose " << LOG_END;
                break;
            }

            if (peer->socket_session->isclose)
            {
                DEBUG_LOG(" peer->socket_session->isclose exit ");
                LOG_ERROR << "peer->socket_session->isclose exit " << LOG_END;
                return true;
            }

            if (peer->socket_session->send_data(send_cache->data, per_size + 9))
            {
            }
            else
            {
                LOG_ERROR << " range error ";
                return false;
            }

            peer->socket_session->window_update_num -= per_size;
            totalsend_num += per_size;
            if (peer->socket_session->window_update_num < 8192)
            {
                std::promise<int> p;
                std::future<int> f{p.get_future()};

                peer->window_update_results.emplace_back(std::move(f));
                peer->window_update_promise = std::move(p);
                peer->window_update_bool    = true;
                peer->socket_session->promise_list.emplace_back(peer->stream_id);
                try
                {
                    peer->window_update_results.front().get();
                    peer->window_update_results.pop_front();
                }
                catch (const std::exception &e)
                {
                    break;
                }
            }

            if (file_size > 10485760)
            {
                if (peer->socket_session->window_update_num < 1048576)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            else if (file_size > 1048576)
            {
                if (m < 10096 && peer->socket_session->window_update_num < 1048576)
                {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(512));
                }
            }
        }
        DEBUG_LOG("send files ok");
        return false;
    }
    else
    {
        std::string stfilecom = "<h3>500 Internal Server Error</h3>";
        stfilecom.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
        peer->status(500);
        peer->length(stfilecom.size());
        peer->type("text/html; charset=utf-8");
        _send_header = peer->make_http2_header();
        peer->socket_session->send_data(_send_header);

        http2_send_body(peer, (const unsigned char *)&stfilecom[0], stfilecom.size());
        return false;
    }

    return true;
}

bool httpserver::http2_send_body(std::shared_ptr<httppeer> peer, const unsigned char *buffer, unsigned int begin_end)
{
    std::string _send_data;
    unsigned int data_send_id = peer->stream_id;
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
        _send_data.resize(9);
        _send_data[2] = 0;
        _send_data[1] = 0;
        _send_data[0] = 0;

        if (peer->socket_session->isclose)
        {
            return true;
        }
        if (peer->socket_session->send_data(_send_data))
        {
        }
        return true;
    }
    for (unsigned long long m = 0; m < begin_end; m += PER_DATA_BLOCK_SIZE)
    {
        unsigned int offset = PER_DATA_BLOCK_SIZE;
        if (offset > begin_end)
        {
            offset        = begin_end;
            _send_data[4] = 0x01;
        }
        else
        {
            if ((m + offset) > begin_end)
            {
                offset        = begin_end - m;
                _send_data[4] = 0x01;
            }
        }

        // 还剩多少窗口
        if (peer->socket_session->window_update_num - offset <= 0)
        {
            offset                                  = peer->socket_session->window_update_num;
            peer->socket_session->window_update_num = 0;
            m                                       = m - PER_DATA_BLOCK_SIZE + offset;
        }
        else
        {
            peer->socket_session->window_update_num -= offset;
        }
        _send_data.resize(9);
        _send_data.append((char *)&buffer[m], offset);

        data_send_id  = offset;
        _send_data[2] = data_send_id & 0xFF;
        data_send_id  = data_send_id >> 8;
        _send_data[1] = data_send_id & 0xFF;
        data_send_id  = data_send_id >> 8;
        _send_data[0] = data_send_id & 0xFF;

        if (peer->socket_session->isclose)
        {
            return true;
        }
        if (peer->socket_session->send_data(_send_data))
        {
        }
        else
        {
            return false;
        }
        // 结束流
        if (peer->isclose)
        {
            return true;
        }

        if (peer->socket_session->window_update_num < 8192)
        {

            std::promise<int> p;
            std::future<int> f{p.get_future()};

            peer->window_update_results.emplace_back(std::move(f));
            peer->window_update_promise = std::move(p);
            peer->window_update_bool    = true;
            peer->socket_session->promise_list.emplace_back(peer->stream_id);
            try
            {
                peer->window_update_results.front().get();
                peer->window_update_results.clear();
            }
            catch (const std::exception &e)
            {
            }
        }

        // peer_session->window_update_num -= ii;
        if (begin_end > 10485760)
        {
            if (m < 35000)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            if (peer->socket_session->window_update_num < 125535)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
    return true;
}

asio::awaitable<void> httpserver::http2loop(std::shared_ptr<httppeer> peer)
{
    try
    {
        serverconfig &sysconfigpath = getserversysconfig();
        std::string _send_header;
        std::string _send_data;

        DEBUG_LOG("%s", peer->urlpath.c_str());
        DEBUG_LOG("%s", peer->host.c_str());
        DEBUG_LOG("%s", peer->header["user-agent"].c_str());

        peer->sitepath         = sysconfigpath.getsitepath(peer->host);
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
            if (peer->state.rangebytes)
            {
                http2_send_file_range(peer);
            }
            else
            {
                http2_send_file(peer);
            }
        }
        else if (sendtype == 2 && peer->isshow_directory())
        {
            std::string htmlcontent = displaydirectory(peer->sendfilename,
                                                       peer->urlpath,
                                                       peer->get["sort"].as_string(),
                                                       sysconfigpath.configpath);
            peer->status(200);
            peer->length(htmlcontent.size());
            peer->type("text/html; charset=utf-8");
            _send_header = peer->make_http2_header();

            peer->socket_session->send_data(_send_header);
            http2_send_body(peer, (const unsigned char *)&htmlcontent[0], htmlcontent.size());
        }
        else
        {
            DEBUG_LOG("htttp2 pool in");

            peer->parse_session();
            peer->status(200);

            peer->type("text/html; charset=utf-8");
            // http_clientrun(peer);
            //////////////
            peer->linktype = 0;

            peer->status(200);
            peer->content_type.clear();
            peer->etag.clear();

            size_t length = co_await co_user_task(peer);    


            std::string tempcompress;
            peer->compress = 0;
            if (peer->state.gzip || peer->state.br)
            {
                if (strcasecmp(peer->content_type.c_str(), "text/html; charset=utf-8") == 0 ||
                    strcasecmp(peer->content_type.c_str(), "application/json") == 0 ||
                    strcasecmp(peer->content_type.c_str(), "text/html") == 0 ||
                    strcasecmp(peer->content_type.c_str(), "application/json; charset=utf-8") == 0)
                {
                    if (peer->output.size() > 100)
                    {

                        if (peer->state.br)
                        {
                            brotli_encode(peer->output, tempcompress);
                            peer->compress = 2;
                        }
                        else if (peer->state.gzip)
                        {
                            if (compress(peer->output.data(),
                                            peer->output.size(),
                                            tempcompress,
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
                peer->length(tempcompress.size());
            }
            else
            {
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
            _send_header = peer->make_http2_header();

            peer->socket_session->send_data(_send_header);

            if (peer->compress > 0)
            {
                http2_send_body(peer, (const unsigned char *)&tempcompress[0], tempcompress.size());
            }
            else
            {
                http2_send_body(peer, (const unsigned char *)&peer->output[0], peer->output.size());
            }
        }
        peer->output.shrink_to_fit();
        peer->val.clear();
        peer->post.clear();
        peer->session.clear();
        peer->issend = true;
    }
    catch (std::exception &e)
    {
        peer->socket_session->send_goway();
        peer->socket_session->isclose = true;
    }
    co_return;
}
void httpserver::http2send_filedata(struct http2sendblock_t &http2_ff_send)
{
    http2_ff_send.last_time = std::chrono::steady_clock::now();
    if (http2_ff_send.peer->isclose)
    {
        http2_ff_send.isfinish = true;
        return;
    }
    if (http2_ff_send.peer->socket_session->isclose)
    {
        http2_ff_send.isfinish = true;
        return;
    }
    if (http2_ff_send.peer->socket_session->window_update_num == 0)
    {
        return;
    }
    sendqueue &send_cache_list     = get_sendqueue();
    struct sendqueue_t *send_cache = send_cache_list.get_cache_ptr();
    if (send_cache == nullptr)
    {
        http2_ff_send.peer->socket_session->send_enddata(http2_ff_send.peer->stream_id);
        // http2_ff_send.peer->socket_session->send_goway();
        http2_ff_send.isfinish = true;
        return;
    }
    sendqueue_back unsetcahceback;
    unsetcahceback.setptr(send_cache);

    unsigned int data_send_id;

    send_cache->data[3] = 0x00;
    send_cache->data[4] = 0x00;
    data_send_id        = http2_ff_send.stream_id;
    send_cache->data[8] = data_send_id & 0xFF;
    data_send_id        = data_send_id >> 8;
    send_cache->data[7] = data_send_id & 0xFF;
    data_send_id        = data_send_id >> 8;
    send_cache->data[6] = data_send_id & 0xFF;
    data_send_id        = data_send_id >> 8;
    send_cache->data[5] = data_send_id & 0x7F;

    data_send_id = fread(&send_cache->data[9], 1, 8183, http2_ff_send.fp.get());

    unsigned int temp_data_size = data_send_id;
    send_cache->data[2]         = temp_data_size & 0xFF;
    temp_data_size              = temp_data_size >> 8;
    send_cache->data[1]         = temp_data_size & 0xFF;
    temp_data_size              = temp_data_size >> 8;
    send_cache->data[0]         = temp_data_size & 0xFF;

    http2_ff_send.offset += data_send_id;
    if (http2_ff_send.offset >= http2_ff_send.total_size)
    {
        http2_ff_send.isfinish = true;
        send_cache->data[4]    = 0x01;
    }
    if (http2_ff_send.peer->socket_session->window_update_num >= data_send_id)
    {
        http2_ff_send.peer->socket_session->window_update_num -= data_send_id;
    }
    else
    {
        http2_ff_send.peer->socket_session->window_update_num = 0;
    }
    http2_ff_send.pre_size = http2_ff_send.peer->socket_session->window_update_num;
    data_send_id += 9;

    http2_ff_send.peer->socket_session->send_data((const unsigned char *)&send_cache->data[0], data_send_id);
}
void httpserver::http2pool(int threadid)
{
    long long tmp = threadid;
    while (true)
    {
        if (this->http2send_tasks.empty())
        {
            std::unique_lock<std::mutex> lock(this->http2_task_mutex);
            this->http2condition.wait(lock, [this] { return this->stop || !this->http2send_tasks.empty(); });
        }

        const std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

        try
        {
            for (auto iter = http2send_tasks.begin(); iter != http2send_tasks.end();)
            {
                if (iter->isfinish == true)
                {
                    DEBUG_LOG("big file end");
                    iter = http2send_tasks.erase(iter);
                }
                else
                {
                    tmp = std::chrono::duration_cast<std::chrono::microseconds>(start - (iter->last_time)).count();
                    if (iter->pre_size > 10485760)
                    {
                        tmp = tmp * 3;
                    }
                    if (tmp > 500)
                    {
                        http2send_filedata(*iter);
                    }
                    ++iter;
                }
            }
        }
        catch (...)
        {
            http2send_tasks.clear();
        }
        const auto end = std::chrono::steady_clock::now();
        tmp            = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        if (tmp < 20)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        if (stop)
        {
            break;
        }
    }
}

bool httpserver::http1_send_file(unsigned int streamid,
                                 std::shared_ptr<httppeer> peer,
                                 std::shared_ptr<client_session> peer_session,
                                 const std::string &filename)
{
    DEBUG_LOG("http1_send_file:%s %u", filename.c_str(), streamid);
    FILE_AUTO fp(std::fopen(peer->sendfilename.c_str(), "rb"), &std::fclose);
    if (fp.get())
    {
        fseek(fp.get(), 0, SEEK_END);
        long long file_size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);
        std::string htmlcontent;
        std::string etag;

        etag = make_header_etag(file_size, peer->fileinfo.st_mtime + peer->url.size());
        std::string fileexttype;
        std::string mime_value = "text/html; charset=utf-8";

        unsigned int filebasesize   = peer->sendfilename.size();
        unsigned int filenameoffset = 0;
        peer->compress              = 0;

        if (filebasesize > 0)
        {
            for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
            {
                if (peer->sendfilename[filenameoffset] == '.')
                {
                    break;
                }
            }
            filenameoffset += 1;
            for (; filenameoffset < filebasesize; filenameoffset++)
            {
                fileexttype.push_back(peer->sendfilename[filenameoffset]);
            }
        }
        DEBUG_LOG("http1_send_file:%s [%s|%s]", peer->sendfilename.c_str(), peer->etag.c_str(), etag.c_str());
        if (peer->etag == etag)
        {
            DEBUG_LOG("http1_send_file:status 304");
            peer->status(304);
            peer->length(0);
            peer->set_header("date", get_gmttime());
            peer->set_header("last-modified", get_gmttime((unsigned long long)peer->fileinfo.st_mtime));
            peer->set_header("etag", etag);
            peer->type(mime_value);
            etag = peer->make_http1_header();
            etag.append("\r\n");
            peer_session->send_data(etag);
            return true;
        }

        peer->compress = 0;
        if (file_size < 16877216 && fileexttype.size() > 0 && mime_compress.contains(fileexttype))
        {

            if (peer->state.gzip || peer->state.br)
            {
                // check cache compress content
                bool is_not_cache_content = true;
                std::string path_temp;
                server_loaclvar &static_server_var = get_server_global_var();
                if (etag.size() > 0)
                {
                    if (static_server_var.static_file_compress_cache)
                    {

                        path_temp = static_server_var.temp_path;
                        if (path_temp.size() > 0 && path_temp.back() != '/')
                        {
                            path_temp.push_back('/');
                        }
                        path_temp.append("statichtml");
                        fs::path paths = path_temp;
                        if (!fs::exists(paths))
                        {
                            fs::create_directories(paths);
                            fs::permissions(paths,
                                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                                            fs::perm_options::add);
                        }
                        path_temp.push_back('/');
                        path_temp.push_back(etag[0]);
                        paths = path_temp;
                        if (!fs::exists(paths))
                        {
                            fs::create_directories(paths);
                            fs::permissions(paths,
                                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                                            fs::perm_options::add);
                        }
                        path_temp.push_back('/');
                        path_temp.append(etag);
                        if (peer->isssl && peer->state.br)
                        {
                            peer->compress = 2;
                            path_temp.append(".br");
                        }
                        else if (peer->state.gzip)
                        {
                            peer->compress = 1;
                            path_temp.append(".gzip");
                        }
                        FILE_AUTO fpcompress(std::fopen(path_temp.c_str(), "rb"), &std::fclose);
                        if (fpcompress.get())
                        {
                            DEBUG_LOG("http1_send_file:file compress %s %d", path_temp.c_str(), peer->compress);
                            fseek(fpcompress.get(), 0, SEEK_END);
                            unsigned long long file_size = ftell(fpcompress.get());
                            fseek(fpcompress.get(), 0, SEEK_SET);
                            htmlcontent.resize(file_size);
                            file_size = fread(&htmlcontent[0], 1, file_size, fpcompress.get());
                            htmlcontent.resize(file_size);
                            is_not_cache_content = false;
                            // if (peer->state.br)
                            // {
                            //     peer->compress = 2;
                            // }
                            // else if (peer->state.gzip)
                            // {
                            //     peer->compress = 1;
                            // }
                        }
                    }
                }
                if (is_not_cache_content)
                {
                    htmlcontent.resize(file_size);
                    file_size = fread(&htmlcontent[0], 1, file_size, fp.get());
                    htmlcontent.resize(file_size);

                    std::string tempcompress;
                    if (peer->compress == 1)
                    {

                        if (compress(htmlcontent.data(), htmlcontent.size(), tempcompress, Z_DEFAULT_COMPRESSION) ==
                            Z_OK)
                        {
                            htmlcontent = tempcompress;
                            // peer->compress = 1;
                        }
                    }
                    else if (peer->compress == 2)
                    {
                        brotli_encode(htmlcontent, tempcompress);
                        // peer->compress = 2;
                        htmlcontent = tempcompress;
                    }
                }

                file_size = htmlcontent.size();
                if (is_not_cache_content && etag.size() > 0)
                {
                    if (static_server_var.static_file_compress_cache)
                    {
                        if (path_temp.size() > 0)
                        {
                            FILE_AUTO fpcompress(std::fopen(path_temp.c_str(), "wb"), &std::fclose);
                            if (fpcompress.get())
                            {
                                fwrite(&htmlcontent[0], 1, htmlcontent.size(), fpcompress.get());
                            }
                        }
                    }
                }
            }
        }

        if (fileexttype.size() > 0)
        {
            mime_value = mime_map[fileexttype];
            if (mime_value.empty())
            {
                if (file_size > 20480)
                {
                    mime_value = "application/octet-stream";
                }
                else
                {
                    mime_value = "text/plain";
                }
            }
        }
        else
        {
            if (file_size > 20480)
            {
                mime_value = "application/octet-stream";
            }
            else
            {
                mime_value = "text/plain";
            }
        }
        DEBUG_LOG("http1_send_file:file exttype %s %lld", mime_value.c_str(), file_size);
        peer->status(200);
        peer->length(file_size);
        peer->type(mime_value);
        if (peer->compress == 0)
        {
            peer->set_header("accept-ranges", "bytes");
        }

        peer->set_header("date", get_gmttime());
        peer->set_header("last-modified", get_gmttime((unsigned long long)peer->fileinfo.st_mtime));

        peer->set_header("etag", etag);

        etag = peer->make_http1_header();
        etag.append("\r\n");

        if (peer->compress > 0 && htmlcontent.size() == 0)
        {
            peer_session->send_data(etag);
            return true;
        }
        if (peer->compress > 0 && htmlcontent.size() > 0)
        {
            etag.append(&htmlcontent[0], htmlcontent.size());
            peer_session->send_data(etag);
        }
        else
        {
            peer_session->send_data(etag);
            fseek(fp.get(), 0, SEEK_SET);
            try
            {
                long long readnum = 0;
                while (readnum < file_size)
                {
                    htmlcontent.resize(4096);
                    unsigned int nread = fread(&htmlcontent[0], 1, 4096, fp.get());
                    htmlcontent.resize(nread);
                    peer_session->send_data(htmlcontent);
                    readnum += nread;
                }
            }
            catch (std::exception &e)
            {
            }
        }
    }
    else
    {
        http1_send_bad_server(500, peer, peer_session);
    }
    return true;
}
bool httpserver::http1_send_file_range(unsigned int streamid,
                                       std::shared_ptr<httppeer> peer,
                                       std::shared_ptr<client_session> peer_session,
                                       const std::string &filename)
{
    DEBUG_LOG("http1_send_file_range %s %u", filename.c_str(), streamid);

    FILE_AUTO fp(std::fopen(peer->sendfilename.c_str(), "rb"), &std::fclose);
    if (fp.get())
    {
        fseek(fp.get(), 0, SEEK_END);
        long long file_size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);
        std::string htmlcontent;
        std::string etag;

        etag              = make_header_etag(file_size, peer->fileinfo.st_mtime + peer->url.size());
        long long readnum = 0, mustnum = 0;

        std::string fileexttype;
        std::string mime_value = "text/html; charset=utf-8";

        unsigned int filebasesize   = peer->sendfilename.size();
        unsigned int filenameoffset = 0;

        if (filebasesize > 0)
        {
            for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
            {
                if (peer->sendfilename[filenameoffset] == '.')
                {
                    break;
                }
            }
            filenameoffset += 1;
            for (; filenameoffset < filebasesize; filenameoffset++)
            {
                fileexttype.push_back(peer->sendfilename[filenameoffset]);
            }
        }

        unsigned int statecode = 200;
        peer->compress         = 0;

        if (fileexttype.size() > 0)
        {
            mime_value = mime_map[fileexttype];
            if (mime_value.empty())
            {
                if (file_size > 20480)
                {
                    mime_value = "application/octet-stream";
                }
                else
                {
                    mime_value = "text/plain";
                }
            }
        }
        else
        {
            if (file_size > 20480)
            {
                mime_value = "application/octet-stream";
            }
            else
            {
                mime_value = "text/plain";
            }
        }
        mustnum = file_size;
        if (peer->state.rangebegin > 0)
        {
            readnum   = peer->state.rangebegin;
            statecode = 206;
        }
        if (peer->state.rangeend > 0)
        {
            mustnum   = peer->state.rangeend + 1;
            statecode = 206;
        }

        peer->status(statecode);
        peer->length(mustnum - readnum);
        peer->type(mime_value);

        if (statecode == 206)
        {
            peer->set_header("content-range",
                             "bytes " + std::to_string(readnum) + "-" + std::to_string(mustnum - 1) + "/" +
                                 std::to_string(file_size));
        }
        else
        {
            peer->set_header("accept-ranges", "bytes");
        }

        peer->set_header("date", get_gmttime());
        peer->set_header("last-modified", get_gmttime((unsigned long long)peer->fileinfo.st_mtime));
        peer->set_header("etag", etag);
        htmlcontent = peer->make_http1_header();
        htmlcontent.append("\r\n");

        peer_session->send_data(htmlcontent);
        fseek(fp.get(), readnum, SEEK_SET);
        try
        {

            while (readnum < mustnum)
            {
                htmlcontent.resize(4096);
                unsigned int nread = fread(&htmlcontent[0], 1, 4096, fp.get());
                htmlcontent.resize(nread);
                peer_session->send_data(htmlcontent);

                readnum += nread;
            }
        }
        catch (std::exception &e)
        {
        }
    }
    else
    {
        http1_send_bad_server(500, peer, peer_session);
    }
    return true;
}
// bool httpserver::http1_send_body(unsigned int streamid,
//                                  std::shared_ptr<httppeer> peer,
//                                  std::shared_ptr<client_session> peer_session,
//                                  const unsigned char *buffer,
//                                  unsigned int begin_end)
// {

//     return true;
// }
  asio::awaitable<size_t> httpserver::co_user_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h)
  {
    auto initiate = [&,self = this](asio::detail::awaitable_handler<asio::any_io_executor, size_t> &&handler) mutable
    {
      peer->user_code_handler_call.push_back(std::move(handler));
      self->clientrunpool.addclient(peer);
    };
    return asio::async_initiate<asio::use_awaitable_t<>, void(size_t)>(initiate, h);
  }
 asio::awaitable<void> httpserver::http1loop(unsigned int stream_id,
                           std::shared_ptr<httppeer> peer,
                           std::shared_ptr<client_session> peer_session)
{
    serverconfig &sysconfigpath = getserversysconfig();
    peer->sitepath              = sysconfigpath.getsitepath(peer->host);
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
        if (peer->state.rangebytes)
        {
            http1_send_file_range(stream_id, peer, peer_session, peer->sendfilename);
        }
        else
        {
            http1_send_file(stream_id, peer, peer_session, peer->sendfilename);
        }
    }
    else if (sendtype == 2 && peer->isshow_directory())
    {
        std::string htmlcontent = displaydirectory(peer->sendfilename,
                                                   peer->urlpath,
                                                   peer->get["sort"].as_string(),
                                                   sysconfigpath.configpath);
        std::string str         = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: "
                                  "Keep-Alive\r\nContent-Length: ";
        str.append(std::to_string(htmlcontent.size()));
        str.append("\r\n\r\n");
        str.append(htmlcontent);
        peer_session->send_data(str);
    }
    else
    {
        DEBUG_LOG("---  http1 pool pre --------");
        peer->linktype = 0;

        peer->parse_session();
 
        peer->status(200);
        peer->content_type.clear();
        peer->etag.clear();

        size_t length = co_await co_user_task(peer);    

        if (peer->get_status() < 100)
        {
            peer->status(200);
        }
        if (!peer->isset_type())
        {
            peer->type("text/html; charset=utf-8");
        }
        if (peer->state.gzip)
        {
            if (strcasecmp(peer->content_type.c_str(), "text/html; charset=utf-8") == 0 ||
                strcasecmp(peer->content_type.c_str(), "application/json") == 0 ||
                strcasecmp(peer->content_type.c_str(), "text/html") == 0 ||
                strcasecmp(peer->content_type.c_str(), "application/json; charset=utf-8") == 0)
            {
                if (peer->output.size() > 100)
                {
                    std::string tempcompress;
                    if (compress(peer->output.data(), peer->output.size(), tempcompress, Z_DEFAULT_COMPRESSION)
                    == Z_OK)
                    {
                        peer->output   = tempcompress;
                        peer->compress = 1;
                    }
                }
            }
        }
        peer->length(peer->output.size());
        std::string htmlcontent = peer->make_http1_header();
        htmlcontent.append("\r\n");
        htmlcontent.append(&peer->output[0], peer->output.size());
        peer_session->send_data(htmlcontent);
    }
}
void httpserver::http1_send_bad_server(unsigned int error_code,
                                       std::shared_ptr<httppeer> peer,
                                       std::shared_ptr<client_session> peer_session)
{
    error_code      = 0;
    std::string str = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html; charset=utf-8\r\nConnection: "
                      "close\r\nContent-Length: ";
    std::string stfilecom = "<h3>500 Internal Server Error</h3>";
    stfilecom.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
    str.append(std::to_string(stfilecom.size()));
    str.append("\r\n\r\n");
    str.append(stfilecom);
    peer_session->send_data(str);
}
void httpserver::http1_send_bad_request(unsigned int error_code, std::shared_ptr<client_session> peer_session)
{
    std::string stfilecom = "<h3>400 Bad Request</h3>";
    stfilecom.append("<hr /><p>Error Code: " + std::to_string(error_code) + "</p>");
    std::string str = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=utf-8\r\nConnection: "
                      "close\r\nContent-Length: ";
    str.append(std::to_string(stfilecom.size()));
    str.append("\r\n\r\n");
    str.append(stfilecom);
    peer_session->send_data(str);
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
asio::awaitable<void> httpserver::clientpeerfun(struct httpsocket_t sock_temp, bool isssl, bool httpversion)
{
    try
    {
        std::shared_ptr<client_session> peer_session =
            isssl ? std::make_shared<client_session>(std::move(sock_temp._sslsocket)) :
                    std::make_shared<client_session>(std::move(sock_temp._socket));
        std::string log_item;
        try
        {
            // serverconfig &sysconfigpath = getserversysconfig();
            std::shared_ptr<httppeer> peer = std::make_shared<httppeer>();

            peer->client_ip = peer_session->getremoteip();
            if (peer_session->_cache_data == nullptr)
            {
                add_nullptrlog(peer->client_ip);
                co_return;
            }

            if (check_blockip(peer->client_ip))
            {
                total_count--;
                co_return;
            }
            peer->httpv = httpversion;
#ifdef DEBUG
            {
                DEBUG_LOG("new client in");
                std::ostringstream oss;
                oss << std::this_thread::get_id();
                std::string tempthread = oss.str();
                DEBUG_LOG("\033[31mclient thread:%s\033[0m", tempthread.c_str());
            }
#endif
            int readnum, error_state = 0;
            unsigned linktype = 0;
            // bool isbegin = false;

            std::unique_ptr<http2parse> http2pre;
            std::unique_ptr<httpparse> http1pre;

            peer->isssl            = isssl ? true : false;
            peer->socket_session   = peer_session;
            unsigned int offsetnum = 0;
            for (;;)
            {
                memset(peer_session->_cache_data, 0x00, 4096);
                try
                {
                    if (isssl)
                    {
                        readnum = co_await peer_session->_sslsocket.front().async_read_some(
                            asio::buffer(peer_session->_cache_data, 4096),
                            asio::use_awaitable);
                    }
                    else
                    {
                        readnum = co_await peer_session->_socket.front().async_read_some(
                            asio::buffer(peer_session->_cache_data, 4096),
                            asio::use_awaitable);
                    }
                }
                catch (const std::exception &e)
                {
                    log_item.clear();
                    log_item.append("co_await exception: ");
                    log_item.append(e.what());
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
                    }
                    else if (linktype == 4)
                    {
                        http2pre = std::make_unique<http2parse>();
                    }
                }

                // http1
                if (linktype == 1)
                {
                    if (!http1pre)
                    {
                        break;
                    }
                    http1pre->process(peer_session->_cache_data, readnum);
                    if (readnum == 0 || http1pre->getfinish())
                    {

                        //  LOG_OUT << "http1parse fininsh" << peer->url << LOG_END;
#ifdef DEBUG
                        DEBUG_LOG("http1parse begin");
                        DEBUG_LOG("urlpath:%s", peer->urlpath.c_str());
                        DEBUG_LOG("host:%s", peer->host.c_str());
                        DEBUG_LOG("user-agent:%s", peer->header["user-agent"].c_str());
                        DEBUG_LOG("http1parse end");
#endif
                        peer->isssl          = isssl ? true : false;
                        peer->socket_session = peer_session;
                        peer->server_ip      = peer_session->getlocalip();
                        peer->client_ip      = peer_session->getremoteip();
                        peer->client_port    = peer_session->getremoteport();
                        peer->server_port    = peer_session->getlocalport();

                        {
                            log_item.clear();
                            log_item.append(peer->server_ip);
                            log_item.push_back(0x20);
                            log_item.append(dateid("%Y-%m-%d %X"));
                            log_item.push_back(0x20);
                            log_item.append(std::to_string(peer->server_port));
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

                            peer->isssl = isssl ? true : false;
                            http2pre    = std::make_unique<http2parse>();
                            http2pre->setsession(peer_session);
                            http2pre->http_data[1] = peer;
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
                            WEBSOCKET_REG &wsreg = get_websocket_config();

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
                        co_await http1loop(1, peer, peer_session);

                        // LOG_OUT << "http1loop end" << LOG_END;
                        DEBUG_LOG("http1loop end");
                        if (peer->state.keeplive == false)
                        {
                            DEBUG_LOG("--- keeplive false --------");
                            break;
                        }
                        http1pre->clear();
                    }
                    if (http1pre->error > 0)
                    {
                        http1_send_bad_request(http1pre->error, peer_session);
                        break;
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
                        error_state = 1;
                    }
                    offsetnum = 24;
                    if (error_state > 0)
                    {
                        break;
                    }
                    linktype = 3;
                    peer_session->send_setting();
                    if (!http2pre)
                    {
                        http2pre = std::make_unique<http2parse>();
                    }
                    http2pre->setsession(peer_session);
                }

                if (linktype == 3)
                {
                    if (!http2pre)
                    {
                        break;
                    }
                    http2pre->process(&peer_session->_cache_data[offsetnum], readnum - offsetnum);
                    offsetnum = 0;
                    if (http2pre->stream_list.size() > 0)
                    {
                        for (; http2pre->stream_list.size() > 0;)
                        {
                            unsigned int block_steamid                         = http2pre->stream_list.front();
                            http2pre->http_data[block_steamid]->socket_session = peer_session;
                            if (hook_host_http2(http2pre->http_data[block_steamid]))
                            {
                                http2pre->stream_list.pop();
                                error_state = 1;
                                break;
                            }
                            http2pre->http_data[block_steamid]->linktype    = 0;
                            http2pre->http_data[block_steamid]->server_ip   = peer_session->getlocalip();
                            http2pre->http_data[block_steamid]->client_ip   = peer_session->getremoteip();
                            http2pre->http_data[block_steamid]->client_port = peer_session->getremoteport();
                            http2pre->http_data[block_steamid]->server_port = peer_session->getlocalport();

                            {
                                log_item.clear();
                                log_item.append(http2pre->http_data[block_steamid]->client_ip);
                                log_item.push_back(0x20);
                                log_item.append(dateid("%Y-%m-%d %X"));
                                log_item.push_back(0x20);
                                log_item.append(std::to_string(http2pre->http_data[block_steamid]->server_port));
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
                            DEBUG_LOG("user-agent:%s",
                                      http2pre->http_data[block_steamid]->header["user-agent"].c_str());
                            DEBUG_LOG("http2parse end");
#endif

                            // clientrunpool.addclient(http2pre->http_data[block_steamid]);
                            co_spawn(this->io_context, http2loop(http2pre->http_data[block_steamid]), asio::detached);
                            http2pre->stream_list.pop();
                        }
                    }
                    if (error_state > 0)
                    {
                        peer_session->send_goway();
                        peer_session->isclose = true;
                        break;
                    }
                    if (peer_session->isgoway)
                    {
                        peer_session->isclose = true;
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
        peer_session->isclose = true;
        total_count--;
        DEBUG_LOG("client run exit:%d", total_count.load());
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("client exit exception");
    }

    co_return;
}

void httpserver::websocket_loop(int myid)
{

    using namespace std::chrono;
    using dsec                      = duration<double>;
    auto invFpsLimit                = duration_cast<system_clock::duration>(dsec{1. / 0.5});
    auto m_BeginFrame               = system_clock::now();
    auto m_EndFrame                 = m_BeginFrame + invFpsLimit;
    unsigned frame_count_per_second = 0;
    auto prev_time_in_seconds       = time_point_cast<seconds>(m_BeginFrame);
    unsigned int fps                = 0;
    myid                            = 0;
    for (;;)
    {
        if (this->websockettasks.empty() && this->clientlooptasks.empty())
        {
            std::unique_lock<std::mutex> lock(this->websocket_task_mutex);
            this->websocketcondition.wait(
                lock,
                [this] { return this->stop || !this->websockettasks.empty() || !this->clientlooptasks.empty(); });
        }

        auto time_in_seconds = time_point_cast<seconds>(system_clock::now());
        ++frame_count_per_second;
        if (time_in_seconds > prev_time_in_seconds)
        {

            frame_count_per_second = 0;
            prev_time_in_seconds   = time_in_seconds;

            ++fps;

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

            if (fps > 31536000)
            {
                fps = 1;
            }
        }
        std::this_thread::sleep_until(m_EndFrame);
        m_BeginFrame = m_EndFrame;
        m_EndFrame   = m_BeginFrame + invFpsLimit;
        if (stop)
        {
            break;
        }
    }
}
asio::awaitable<void>
httpserver::sslhandshake(asio::ip::tcp::socket socket, asio::ssl::context &context_, unsigned long long temp_domain)
{
    try
    {

        serverconfig &sysconfigpath         = getserversysconfig();
        const unsigned char *for_next_proto = nullptr;
        unsigned int next_proto_len         = 0;
        bool httpversion                    = false;
        asio::error_code ec_error;
        DEBUG_LOG(" accept ok!");
        // if all http2
        if (sysconfigpath.isallnothttp2)
        {
            SSL_CTX_set_alpn_select_cb(context_.native_handle(), alpn_cb, (void *)temp_domain);
        }
        asio::ssl::stream<asio::ip::tcp::socket> sslsocket(std::move(socket), context_);
        sslsocket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
        sslsocket.handshake(asio::ssl::stream_base::server, ec_error);
        if (ec_error)
        {
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(" handshake ec_error ");
            lock.unlock();
            DEBUG_LOG(" handshake ec_error !");
            co_return;
        }
        // client select proto 看看客户端是否指定 协议，如果没有指定为null
        SSL_get0_next_proto_negotiated(sslsocket.native_handle(), &for_next_proto, &next_proto_len);
        if (for_next_proto == nullptr)
        {
            // openssl >2.0,5 必须大于2.0.5 alpn选择的协议
            SSL_get0_alpn_selected(sslsocket.native_handle(), &for_next_proto, &next_proto_len);
        }
        if (next_proto_len > 0)
        {
            if (for_next_proto[0] == 'h' && for_next_proto[1] == '2')
            {
                DEBUG_LOG(" h2 ");
                httpversion = true;
            }
        }
        DEBUG_LOG(" https ok!");
        total_count++;

        struct httpsocket_t sock_temp(std::move(sslsocket));
        co_spawn(this->io_context, clientpeerfun(std::move(sock_temp), true, httpversion), asio::detached);
    }
    catch (std::exception &e)
    {
    }
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
        exit(1);
    }

    asio::ssl::context context_(asio::ssl::context::sslv23);
    context_.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 |
                         asio::ssl::context::single_dh_use);
    context_.set_password_callback(std::bind(get_password));
    context_.use_certificate_chain_file(sysconfigpath.ssl_chain_file());

    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_AUTO_RETRY);
    context_.use_private_key_file(sysconfigpath.ssl_key_file(), asio::ssl::context::pem);
    context_.use_tmp_dh_file(sysconfigpath.ssl_dh_file());
    SSL_CTX_set_tlsext_servername_callback(context_.native_handle(), serverNameCallback);

    auto ssl_opts = (SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS) | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                    SSL_OP_NO_COMPRESSION | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_SINGLE_ECDH_USE |
                    SSL_OP_NO_TICKET | SSL_OP_CIPHER_SERVER_PREFERENCE;
    SSL_CTX_set_options(context_.native_handle(), ssl_opts);
    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_AUTO_RETRY);
    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_RELEASE_BUFFERS);

    constexpr char DEFAULT_CIPHER_LIST[] = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-"
                                           "AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-"
                                           "POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-"
                                           "AES256-GCM-SHA384";

    if (SSL_CTX_set_cipher_list(context_.native_handle(), DEFAULT_CIPHER_LIST) == 0)
    {
        std::cerr << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    }

    std::vector<unsigned char> next_proto;
    next_proto = get_default_alpn();

    unsigned int protos_len;
    const char *protos;

    protos     = HTTP2_H2_ALPN HTTP1_NPN;
    protos_len = sizeof(HTTP2_H2_ALPN HTTP1_NPN) - 1;

    if (SSL_CTX_set_alpn_protos(context_.native_handle(), (const unsigned char *)protos, protos_len) < 0)
    {
    }

    SSL_CTX_set_next_protos_advertised_cb(context_.native_handle(), next_proto_cb, &next_proto);
    unsigned long long temp_domain          = 0;
    sysconfigpath.domain_http2[temp_domain] = false;
    server_loaclvar &localvar               = get_server_global_var();
    if (sysconfigpath.isallnothttp2)
    {
        temp_domain = std::hash<const char *>{}((const char *)&sysconfigpath.map_value["default"]["mainhost"]);
        // SSL_CTX_set_alpn_select_cb(context_.native_handle(), alpn_cb, &alpn_ctx);
        if (localvar.http2_enable)
        {
            sysconfigpath.domain_http2[temp_domain] = true;
        }
        else
        {
            sysconfigpath.domain_http2[temp_domain] = false;
            if (sysconfigpath.map_value.find(sysconfigpath.map_value["default"]["mainhost"]) !=
                sysconfigpath.map_value.end())
            {
                if (sysconfigpath.map_value[sysconfigpath.map_value["default"]["mainhost"]].find("http2_enable") !=
                    sysconfigpath.map_value[sysconfigpath.map_value["default"]["mainhost"]].end())
                {

                    if (sysconfigpath.map_value[sysconfigpath.map_value["default"]["mainhost"]]["http2_enable"][0] ==
                        '1')
                    {
                        sysconfigpath.domain_http2[temp_domain] = true;
                    }
                }
            }
        }
    }
    SSL_CTX_set_alpn_select_cb(context_.native_handle(), alpn_cb, (void *)temp_domain);

    // const unsigned char *for_next_proto = nullptr;
    // unsigned int next_proto_len = 0;
    // bool httpversion = false;
    for (;;)
    {
        try
        {
            // httpversion = false;
            asio::ip::tcp::socket socket(this->io_context);
            DEBUG_LOG("https accept");

            acceptor.accept(socket, ec_error);
            if (ec_error)
            {

                std::unique_lock<std::mutex> lock(log_mutex);
                error_loglist.emplace_back(" accept ec_error ");
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            DEBUG_LOG(" accept ok!");

            co_spawn(this->io_context,
                     sslhandshake(std::move(socket), std::ref(context_), temp_domain),
                     asio::detached);
        }
        catch (std::exception &e)
        {
        }
        if (stop)
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
        exit(1);
    }

    for (;;)
    {
        try
        {
            asio::ip::tcp::socket socket(this->io_context);
            acceptor.accept(socket, ec);
            total_count++;
            struct httpsocket_t sock_temp(std::move(socket));
            co_spawn(this->io_context, clientpeerfun(std::move(sock_temp), false, false), asio::detached);
        }
        catch (std::exception &e)
        {
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(" http accept error  ");
            lock.unlock();
        }
        if (stop)
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
        error_loglist.emplace_back(errorstr);
    }

    struct regmethold_t temp;
    temp.pre    = nullptr;
    temp.regfun = [self = this](std::shared_ptr<httppeer> peer) -> std::string
    {
        httppeer &client = peer->getpeer();
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
                static_server_var.show_visit_info = true;
                client << "<p>";
                client << "online:" << self->total_count.load();
                client << " ";
                try
                {
                    std::map<size_t, mysqllinkpool> &mysqldbpoolglobal = get_mysqlpool();
                    isshow                                             = 0;
                    for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
                    {
                        client << " [db:" << iter->second.select_link.db
                               << " select:" << std::to_string(iter->second.select_current_num);
                        client << " edit:" << std::to_string(iter->second.edit_current_num);
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
                static_server_var.show_visit_info = false;
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

#ifdef ENABLE_BOOST
    clientapi *pn       = clientapi::instance();
    pn->api_loadview    = loadviewso;
    pn->api_loadcontrol = loadcontrol;

    pn->api_mysqlselect     = get_mysqlselectexecute;
    pn->api_mysqledit       = get_mysqlselectexecute;
    pn->api_mysqlcommit     = get_mysqlselectexecute;
    pn->map_value           = sysconfigpath.map_value;
    pn->server_global_var   = get_server_global_var;
    pn->api_mysql_back_conn = back_mysql_connect;
#endif

    int catch_num               = 0;
    unsigned int updatetimetemp = sysconfigpath.siteusehtmlchachetime;
    std::string currentpath;
    std::string error_path;
    server_loaclvar &static_server_var = get_server_global_var();
    currentpath                        = static_server_var.log_path;

    if (currentpath.size() > 0 && currentpath.back() != '/')
    {
        currentpath.push_back('/');
    }
    error_path = currentpath;
    currentpath.append("access.log");
    error_path.append("error.log");
    struct flock lockstr        = {};
    unsigned int mysqlpool_time = 1;
    std::size_t n_write         = 0;
    updatetimetemp              = 0;
    for (;;)
    {
        if (catch_num > 10)
        {
            std::this_thread::sleep_for(std::chrono::seconds(20));
            catch_num = 0;
        }
        try
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (clientrunpool.gettasknum() > 3)
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
                int fd = open(currentpath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
                if (fd == -1)
                {
                    continue;
                }

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
                std::unique_lock<std::mutex> loglock(log_mutex);
                while (!access_loglist.empty())
                {
                    n_write = write(fd, access_loglist.front().data(), access_loglist.front().size());
                    access_loglist.pop_front();
                }
                loglock.unlock();

                lockstr.l_type = F_UNLCK;
                if (fcntl(fd, F_SETLK, &lockstr) == -1)
                {
                    close(fd);
                    continue;
                }
                close(fd);
            }

            // save error.log
            if (!error_loglist.empty())
            {
                int fd = open(error_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
                if (fd == -1)
                {
                    continue;
                }

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
                std::unique_lock<std::mutex> loglock(log_mutex);
                while (!error_loglist.empty())
                {
                    n_write = write(fd, error_loglist.front().data(), error_loglist.front().size());
                    error_loglist.pop_front();
                }
                loglock.unlock();

                lockstr.l_type = F_UNLCK;
                if (fcntl(fd, F_SETLK, &lockstr) == -1)
                {
                    close(fd);
                    continue;
                }
                close(fd);
            }
            if (n_write > 0)
            {
                n_write = 0;
            }

            if (mysqlpool_time % 100 == 0)
            {
                if (total_count.load() == 0)
                {
                    std::map<size_t, mysqllinkpool> &mysqldbpoolglobal = get_mysqlpool();
                    for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
                    {
                        iter->second.clearpool();

                        DEBUG_LOG("mysql pool clearpoool ");
                    }
                    mysqlpool_time = 1;
                }
            }

            std::time_t t = std::time(nullptr);
            std::tm *now  = std::localtime(&t);

            if (now->tm_hour < 3 && mysqlpool_time > 82800)
            {
                std::map<size_t, mysqllinkpool> &mysqldbpoolglobal = get_mysqlpool();
                for (auto iter = mysqldbpoolglobal.begin(); iter != mysqldbpoolglobal.end(); iter++)
                {
                    iter->second.clearpool();
                }
                mysqlpool_time = 1;
            }

            mysqlpool_time += 1;
            DEBUG_LOG("clear mysql poll time:%d,client live:%d", mysqlpool_time, total_count.load());
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("frame thread:%s", e.what());
            catch_num += 1;
        }
        if (stop)
        {
            break;
        }
    }
}
void httpserver::run(const std::string &sysconfpath)
{
    try
    {
        stop                        = false;
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
        static_server_var.show_visit_info = true;
#endif

        _initauto_control_httpmethodregto(_http_regmethod_table);
        _inithttpmethodregto(_http_regmethod_table);
        _inithttpmethodregto_pre(_http_regmethod_table);
        sendqueue &send_cache = get_sendqueue();
        send_cache.inti_sendqueue(512);
        auto &link_cache = get_client_data_cache();
        link_cache.inti_sendqueue(1024);

        VIEW_REG &viewreg = get_viewmetholdreg();
        _initview_method_regto(viewreg);

        WEBSOCKET_REG &wsreg = get_websocket_config();
        _initwebsocketmethodregto(wsreg);

        total_count = sysconfigpath.get_co_thread_num();
        asio::io_context::work worker(io_context);
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
        total_count = 0;

        std::thread httpwatch(std::bind(&httpserver::httpwatch, this));
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::thread https(std::bind(&httpserver::listeners, this));
        std::thread http(std::bind(&httpserver::listener, this));
        std::thread http2pool(std::bind(&httpserver::http2pool, this, 0));
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
        if (http2pool.joinable())
        {
            http2pool.join();
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
}
}// namespace http
