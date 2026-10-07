/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file http2_parse.cpp
 *  @date 2022-10-12
 *
 *  http2 protocol parse file
 *
 *
 */
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <filesystem>

#include <string_view>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <cstring>

#include "http2_frame.h"
#include "http2_parse.h"
#include "client_session.h"
#include "http2_huffman.h"

#include "terminal_color.h"
#include "debug_log.h"
#include "server_localvar.h"
#include "urlcode.h"
#include "request.h"
#include "func.h"
#include "cost_define.h"
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

namespace http
{
void http2parse::setsession(std::shared_ptr<client_session> peer_sock) { peer_session = peer_sock; }

// multipart 的 boundary 归一化：去引号、限长。
// 与 HTTP/1 (http_parse.cpp 的 boundary_normalize) 保持完全一致的口径。
// 但 HTTP/2 的 getcontenttype 当时漏了，这里补上，避免两边再次分叉。
// 放在匿名 namespace 里，既不影响其他翻译单元，也能在 unity 构建下与 HTTP/1 的同名函数共存。
namespace
{
bool http2_boundary_normalize(std::string &out, const std::string &raw)
{
    std::string temp = raw;
    if (temp.size() >= 2 && temp.front() == '\"' && temp.back() == '\"')
    {
        temp = temp.substr(1, temp.size() - 2);
    }
    if (temp.size() == 0 || temp.size() > 72)
    {
        return false;
    }
    out = temp;
    return true;
}
}// namespace

void http2parse::clsoesend()
{
    for (auto iter = http_data.begin(); iter != http_data.end();)
    {
        if (iter->second->issend)
        {
            try
            {
                iter->second->isclose = true;
                iter->second->clsoesend();
            }
            catch (const std::exception &e)
            {
                DEBUG_LOG("http2parse user_code_handler_call error");
            }
        }
        iter++;
    }
}

void http2parse::readheaders(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    auto iter = http2_header_recvs.find(temp_pack_data.stream_id);
    if ((temp_pack_data.flags & 0xD2) != 0)
    {
        //非头部内容
        error = 40015;
        return;
    }

    // ---- 流合法性校验（RFC 9113 §5.1.1 / §6.10）----
    // 必须校验流 id：不校验时客户端可以用偶数 id、0、以及已用过的 id 反复开新流，
    // 只要只发 HEADERS 不结束 body，就不会计入 server.cpp 的 steam_count，
    // http_data / http_post_data / http2_header_recvs 会被无上限撑大。
    if (iter == http2_header_recvs.end())
    {
        if (temp_pack_data.frame_type != 0x01)
        {
            // CONTINUATION 必须紧跟在 HEADERS 之后，不能凭空出现
            error = 40008;
            return;
        }
        if (temp_pack_data.stream_id == 0 || (temp_pack_data.stream_id & 0x1) == 0)
        {
            // 客户端发起的流必须是奇数；流 0 只能用于连接级帧
            error = 40006;
            return;
        }
        if (temp_pack_data.stream_id <= max_client_stream_id)
        {
            // 新流的 id 必须严格递增，重复或倒退的 id 属于协议错误
            error = 40006;
            return;
        }
        if (http_data.size() >= CONST_HTTP2_MAX_STREAMS || http2_header_recvs.size() >= CONST_HTTP2_MAX_STREAMS)
        {
            // 「body 未结束的流」和「头部块未结束的流」都必须计数
            error = 40010;
            return;
        }
        max_client_stream_id = temp_pack_data.stream_id;
    }

    if (iter == http2_header_recvs.end())
    {
        HTTP2_HEADER_FRAME_T temp;
        temp.length     = temp_pack_data.length;
        temp.frame_type = temp_pack_data.frame_type;

        temp.flags.END_STREAM  = temp_pack_data.flags & HTTP2_HEADER_END_STREAM;
        temp.flags.END_HEADERS = temp_pack_data.flags & HTTP2_HEADER_END_HEADERS;
        temp.flags.PADDED      = temp_pack_data.flags & HTTP2_HEADER_PADDED;
        temp.flags.PRIORITY    = temp_pack_data.flags & HTTP2_HEADER_PRIORITY;

        temp.stream_id = temp_pack_data.stream_id;
        temp.content   = temp_pack_data.payload;

        http2_header_recvs.insert_or_assign(temp_pack_data.stream_id, std::move(temp));
        iter = http2_header_recvs.find(temp_pack_data.stream_id);
    }
    else
    {
        if (iter->second.flags.END_HEADERS > 0)
        {
            error = 40008;
            return;
        }
        iter->second.content.append(temp_pack_data.payload);
    }

    if (iter->second.content.size() > CONST_HTTP_HEADER_BODY_SIZE)
    {
        //two check header payload size
        error = 40014;
        return;
    }

    // HEADERS 帧才带 END_STREAM / PADDED / PRIORITY，CONTINUATION 只带 END_HEADERS。
    // 这些位只能在 HEADERS 帧上取；用「当前帧」的 flags 无条件覆盖全部标志时，
    // HEADERS(PADDED|END_STREAM) + CONTINUATION 会把 PADDED/END_STREAM 抹掉，
    // padding 字节被当成头部块正文解析，整块 HPACK 错位。
    if (temp_pack_data.frame_type == 0x01)
    {
        iter->second.flags.END_STREAM = (temp_pack_data.flags & HTTP2_HEADER_END_STREAM) ? 1 : 0;
        iter->second.flags.PRIORITY   = (temp_pack_data.flags & HTTP2_HEADER_PRIORITY) ? 1 : 0;
        iter->second.flags.PADDED     = (temp_pack_data.flags & HTTP2_HEADER_PADDED) ? 1 : 0;
    }
    iter->second.flags.END_HEADERS = (temp_pack_data.flags & HTTP2_HEADER_END_HEADERS) ? 1 : 0;

    if (iter->second.flags.END_HEADERS == 0)
    {
        //未结束
        return;
    }

    unsigned header_offset = 0;
    if (iter->second.flags.PADDED)
    {
        if (iter->second.content.size() == 0)
        {
            error = 40013;
            return;
        }
        iter->second.padded_length = iter->second.content[header_offset];
        header_offset++;
    }

    if (iter->second.flags.PRIORITY)
    {
        header_offset += 5;
    }

    unsigned int new_size_num = header_offset + iter->second.padded_length;

    if (new_size_num > 0)
    {
        if (new_size_num >= iter->second.content.size())
        {
            error = 40019;
            return;
        }
        new_size_num         = iter->second.content.size() - new_size_num;
        std::string temp_str = iter->second.content.substr(header_offset, new_size_num);
        iter->second.content = temp_str;
    }

    auto steam_httppeer = std::make_shared<httppeer>();
    if (!http_data.emplace(iter->second.stream_id, steam_httppeer).second)
    {
        // 同一流 id 已经建过请求对象：emplace 会静默失败，新 peer 变成没人应答的孤儿，
        // 而 http_data 里留着旧的。这里直接判协议错误（正常情况下前面的递增校验已拦住）。
        error = 40006;
        return;
    }
    steam_httppeer->content_length = 0;
    steam_httppeer->stream_id      = iter->second.stream_id;
    steam_httppeer->httpv          = 2;
    steam_httppeer->socket_session = peer_session;

    headers_parse(iter->second, steam_httppeer);

    if (ispost)
    {
        error = steam_httppeer->check_upload_limit();
    }
    // isuse_fastcgi() 已删除：路由预查挪到 loop 里，php 处理挪到 server 新方法
    http2_header_recvs.erase(iter);
}
void http2parse::headers_parse(const HTTP2_HEADER_FRAME_T &header_block_obj, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int header_stream_id = steam_httppeer->stream_id;
    for (unsigned int h_begin = 0; h_begin < (unsigned int)header_block_obj.content.size(); h_begin++)
    {
        unsigned char c = header_block_obj.content[h_begin];

        if (c & 0x80)
        {
            headertype1(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else if ((c & 0xC0) == 0x40)
        {
            headertype2(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else if ((c & 0xF0) == 0x10)
        {
            headertype3(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else if ((c & 0xF0) == 0)
        {
            headertype4(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else
        {
            error = 40205;
            return;
        }

        if (error > 0)
        {
            return;
        }
    }

    // CORS：OPTIONS 预检的 ACAO 由 send_cors_domain() 按白名单判定输出，
    // 普通请求的 ACAO 在解析到 origin 头那一刻就判定了（头名长度 6 的 'o' 分支），
    // 这里请求头虽已解完，但不再需要任何 CORS 处理

    if (steam_httppeer->method == 2 || steam_httppeer->method == 4)
    {
        DEBUG_LOG("http2 post client: %s %ud", steam_httppeer->url.c_str(), header_stream_id);
        // 这里不再重置/授予接收窗口：
        //  * 新流的流级额度已由本端广告的 SETTINGS_INITIAL_WINDOW_SIZE 给出，
        //    不需要（也不应该）再补一次；
        //  * 每来一个 POST/PUT 就把整条连接的接收记账重置成窗口目标值、并发出
        //    「连接级 + 流级」两个 WINDOW_UPDATE 的话：既抹掉其他流已消费的额度（并发 POST
        //    会静默挂死），又把对端连接窗口每请求顶高 16MB（约 129 次后超 2^31-1，
        //    对端必须按 FLOW_CONTROL_ERROR 断连）。
        need_wakeup_send_threads = true;

        auto iter = http_post_data.find(header_stream_id);
        if (iter == http_post_data.end())
        {
            HTTP2_POST_DATA_T temp_p;
            temp_p.peer      = steam_httppeer;
            temp_p.stream_id = header_stream_id;
            http_post_data.insert_or_assign(steam_httppeer->stream_id, std::move(temp_p));
        }
    }
    else
    {
        DEBUG_LOG("http2 get client: %s", steam_httppeer->url.c_str());
        steam_httppeer->isfinish = true;
        stream_list.emplace(header_stream_id);
    }
}
void http2parse::cookie_process([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("cookie_process:%s:%s", header_name.c_str(), header_value.c_str());
    unsigned int i = 0, linesize = static_cast<unsigned int>(header_value.size());
    std::string buffer_key;
    std::string buffer_value;

    if (steam_httppeer->header["cookie"].empty())
    {
    }
    else
    {
        steam_httppeer->header["cookie"].append("; ");
    }
    steam_httppeer->header["cookie"].append(header_value);
    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x3D)
        {
            buffer_key = http::url_decode(buffer_value.data(), buffer_value.length());
            // Safe filtering allowed: A-Z a-z 0-9 _-
            std::string safe_key;
            safe_key.reserve(buffer_key.size());
            for (unsigned char c : buffer_key)
            {
                if ((c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9') ||
                    c == '_' || c == '-')
                {
                    safe_key.push_back(static_cast<char>(c));
                }
            }
            buffer_key.swap(safe_key);
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3B)
        {
            buffer_value = http::url_decode(buffer_value.data(), buffer_value.length());
            if (buffer_key.size() > 48)
            {
                error = 40157;
                return;
            }
            if (buffer_key.empty())
            {
                error = 40157;
                return;
            }
            steam_httppeer->cookie[buffer_key] = buffer_value;
            buffer_key.clear();
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        buffer_value.push_back(header_value[i]);
    }
    if (buffer_value.size() > 0)
    {
        buffer_value = http::url_decode(buffer_value.data(),
                                        buffer_value.length());
        if (buffer_key.size() > 48)
        {
            error = 40158;
            return;
        }
        if (buffer_key.empty())
        {
            error = 40158;
            return;
        }
        steam_httppeer->cookie[buffer_key] = buffer_value;
    }
    else
    {
        if (buffer_key.size() > 48)
        {
            error = 40159;
            return;
        }
        if (buffer_key.size() > 0)
        {
            steam_httppeer->cookie[buffer_key] = "";
        }
        buffer_key.clear();
    }
}
void http2parse::path_process([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("path_process:%s:%s", header_name.c_str(), header_value.c_str());

    std::string buffer_key;
    std::string buffer_value;
    unsigned char headerstep = 0;
    steam_httppeer->pathinfos.clear();
    steam_httppeer->url.clear();
    unsigned int linesize = static_cast<unsigned int>(header_value.size());
    unsigned int ioffset  = 0;

    // 按原始 '/' 逐段切分，每段交给 http::url_segments_normalize()（func.cpp 统一实现）
    for (; ioffset < linesize; ioffset++)
    {
        if (header_value[ioffset] == 0x3F)// '?'
        {
            headerstep = 6;
            break;
        }
        if (header_value[ioffset] == 0x2F)// '/'
        {
            if (buffer_key.size() > 255)
            {
                error = 40007;
                return;
            }
            if (buffer_key.size() > 0)
            {
                if (!http::url_segments_normalize(steam_httppeer->pathinfos, buffer_key))
                {
                    error = 40095;
                    return;
                }
                buffer_key.clear();
            }
        }
        else
        {
            buffer_key.push_back(header_value[ioffset]);
        }
    }
    if (buffer_key.size() > 0)
    {
        if (buffer_key.size() > 255)
        {
            error = 40090;
            return;
        }
        if (!http::url_segments_normalize(steam_httppeer->pathinfos, buffer_key))
        {
            error = 40095;
            return;
        }
    }

    steam_httppeer->header["urlpath"] = header_value.substr(0, ioffset);

    if (steam_httppeer->pathinfos.size() > 0)
    {
        steam_httppeer->urlpath.clear();
        steam_httppeer->urlpath.reserve(header_value.size() > 64 && header_value.size() < 10000 ? header_value.size() : 0);

        for (size_t nn = 0; nn < steam_httppeer->pathinfos.size(); nn++)
        {
            steam_httppeer->urlpath.push_back('/');
            steam_httppeer->urlpath.append(steam_httppeer->pathinfos[nn]);
        }
        if (steam_httppeer->urlpath.size() == 0)
        {
            steam_httppeer->urlpath = "/";
        }
    }
    else
    {
        steam_httppeer->urlpath.clear();
        steam_httppeer->urlpath = "/";
    }

    // 最后一段含 '.' 标记为文件请求；.php 结尾标 compress=10 交给 loop 处理 —
    // last segment has '.' means file-like URL; .php suffix marks fastcgi candidate
    if (!steam_httppeer->pathinfos.empty())
    {
        auto const &last       = steam_httppeer->pathinfos.back();
        steam_httppeer->isfile = (last.find('.') != std::string::npos);
#ifdef ENABLE_FASTCGI
        // 超轻量：只比后缀 4 字符，不查路由表不查磁盘 —
        // ultra-light: only check suffix, no route lookup no disk stat

        if (last.size() > 4 && last[last.size() - 1] == 'p' && last[last.size() - 2] == 'h' &&
            last[last.size() - 3] == 'p' && last[last.size() - 4] == '.')
        {
            steam_httppeer->compress = 10;
        }
#endif

    }

    steam_httppeer->url = steam_httppeer->urlpath;

    buffer_key.clear();
    if (headerstep == 6)
    {
        for (; ioffset < linesize; ioffset++)
        {
            if (header_value[ioffset] == 0x3F)
            {
                continue;
            }
            break;
        }
        headerstep = 0;
        steam_httppeer->querystring.clear();
        steam_httppeer->header["querystring"].clear();
        steam_httppeer->querystring.append(&header_value[ioffset], (linesize - ioffset));// http::url_decode(&header_value[ioffset], (linesize - ioffset));
        steam_httppeer->header["querystring"].append(steam_httppeer->querystring);
        steam_httppeer->url.push_back(0x3F);
        steam_httppeer->url.append(steam_httppeer->querystring);
        buffer_key.clear();
        buffer_value.clear();
        unsigned int jj = 0;
        for (; ioffset < linesize; ioffset++)
        {
            if (header_value[ioffset] == 0x3D)
            {
                for (; ioffset < linesize; ioffset++)
                {
                    if (header_value[ioffset] == 0x3D)
                    {
                        continue;
                    }
                    else
                    {
                        ioffset -= 1;
                        break;
                    }
                }
                buffer_key = http::url_decode(buffer_value.data(),
                                              buffer_value.length());
                buffer_value.clear();
                headerstep = 1;
                jj         = 0;
                continue;
            }
            else if (header_value[ioffset] == 0x26)
            {
                for (; ioffset < linesize; ioffset++)
                {
                    if (header_value[ioffset] == 0x26)
                    {
                        continue;
                    }
                    else
                    {
                        ioffset -= 1;
                        break;
                    }
                }

                buffer_value =
                    http::url_decode(buffer_value.data(),
                                     buffer_value.length());
                if (buffer_key.size() > 48)
                {
                    error = 40173;
                    return;
                }
                procssparamter(buffer_key, buffer_value, steam_httppeer);
                buffer_key.clear();
                buffer_value.clear();
                headerstep = 2;
                jj         = 0;
                continue;
            }
            buffer_value.push_back(header_value[ioffset]);

            if (headerstep == 0 || headerstep == 2)
            {
                //key name too long
                if (jj > 72)
                {
                    error = 40174;
                    return;
                }
            }
            jj++;
        }

        if (headerstep == 1)
        {
            // full kv
            buffer_value = http::url_decode(buffer_value.data(),
                                            buffer_value.length());
            if (buffer_key.size() > 48)
            {
                error = 40175;
                return;
            }
            procssparamter(buffer_key, buffer_value, steam_httppeer);
        }
        else if (headerstep == 2)
        {
            // half k
            buffer_key = http::url_decode(buffer_value.data(),
                                          buffer_value.length());
            buffer_value.clear();
            if (buffer_key.size() > 48)
            {
                error = 40176;
                return;
            }
            procssparamter(buffer_key, buffer_value, steam_httppeer);
        }
        else if (buffer_value.size() > 0)
        {
            // only one k
            buffer_key = http::url_decode(buffer_value.data(),
                                          buffer_value.length());
            buffer_value.clear();
            if (buffer_key.size() > 48)
            {
                error = 40177;
                return;
            }
            procssparamter(buffer_key, buffer_value, steam_httppeer);
        }
    }
}
void http2parse::procssparamter(std::string_view buffer_key, std::string_view buffer_value, std::shared_ptr<httppeer> steam_httppeer)
{
    bool isgroup = true;
    if (buffer_key.length() > 72)
    {
        error = 40011;
        return;
    }

    for (size_t j = 0; j < buffer_key.length(); j++)
    {
        if (buffer_key[j] == '[')
        {
            isgroup = false;
        }
    }

    if (isgroup)
    {
        steam_httppeer->get[buffer_key] = buffer_value;
        return;
    }

    isgroup = true;
    std::string objname;
    for (size_t j = 0; j < buffer_key.length(); j++)
    {
        if (buffer_key[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < buffer_key.length(); n++)
            {
                if (buffer_key[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (buffer_key[n] == '[')
                {

                    break;
                }
                if (buffer_key[n] != 0x22)
                {
                    key1name.push_back(buffer_key[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < buffer_key.length())
                {
                    if (buffer_key[m] == '[')
                    {
                        m += 1;
                        for (; m < buffer_key.length(); m++)
                        {
                            if (buffer_key[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (buffer_key[m] == '[')
                            {

                                break;
                            }
                            if (buffer_key[m] != 0x22)
                            {
                                key2name.push_back(buffer_key[m]);
                            }
                        }

                        if (ishaskey2 && m == buffer_key.length())
                        {
                        }
                        else
                        {
                            ishaskey2 = false;
                        }
                    }
                }

                if (ishaskey2)
                {
                    // 双数组
                    if (key1name.empty())
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40095;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->get[objname].size());
                            key1name         = std::to_string(iii);
                            steam_httppeer->get[objname][key1name].set_object();

                            iii      = steam_httppeer->get[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40096;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40097;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->get[objname].size());
                            key1name         = std::to_string(iii);
                            steam_httppeer->get[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40098;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40099;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            steam_httppeer->get[objname][key1name].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->get[objname][key1name].size());
                            key2name         = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40100;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40101;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40102;
                                return;
                            }
                            steam_httppeer->get[objname].set_object();
                            steam_httppeer->get[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == buffer_key.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        if (objname.size() > 48)
                        {
                            error = 40103;
                            return;
                        }

                        steam_httppeer->get[objname].set_object();
                        unsigned int iii = static_cast<unsigned int>(steam_httppeer->get[objname].size());
                        key1name         = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp = buffer_value;

                        steam_httppeer->get[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40104;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40105;
                            return;
                        }

                        steam_httppeer->get[objname].set_object();

                        http::obj_val objtemp;
                        objtemp = buffer_value;
                        steam_httppeer->get[objname].push(key1name, std::move(objtemp));
                    }
                    j       = n;
                    isgroup = false;
                }
                else
                {

                    // 没有数组
                }
            }
            if (isgroup)
            {
                objname.push_back(buffer_key[j]);
            }
        }
        else
        {
            objname.push_back(buffer_key[j]);
        }
    }
    if (isgroup)
    {
        steam_httppeer->get[buffer_key] = buffer_value;
    }
}
void http2parse::range_process([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("range_process:%s:%s", header_name.c_str(), header_value.c_str());
    // 语法不合法时按 RFC 9110 §14.2 忽略该头：本端没有「单条请求头错误」通道，
    // 走 error 会把整条连接按协议错误断开（403 + GOAWAY），比忽略严重得多。
    parse_range_header(header_value, steam_httppeer->state);
}

bool http2parse::header_host_process(const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    bool ishasport = false;
    steam_httppeer->host.clear();
    // 注意：这里必须是 unsigned int。用 unsigned char 做下标时，header_value 长度超过 255
    // 后会从 255 回绕到 0，循环永远不结束，host 字符串被无上限 push_back（远程死循环 + 内存耗尽）。
    unsigned int i = 0;
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] == 0x3A)
        {
            ishasport = true;
            i++;
            break;
        }
        else if (header_value[i] >= '0' && header_value[i] <= '9')
        {
            steam_httppeer->host.push_back(header_value[i]);
        }
        else if (header_value[i] >= 'a' && header_value[i] <= 'z')
        {
            steam_httppeer->host.push_back(header_value[i]);
        }
        else if (header_value[i] >= 'A' && header_value[i] <= 'Z')
        {
            steam_httppeer->host.push_back(header_value[i] + 32);
        }
        else if (header_value[i] == '.' || header_value[i] == '-')
        {
            steam_httppeer->host.push_back(header_value[i]);
        }
        else if (header_value[i] == '[')
        {
            if (steam_httppeer->host.size() > 0)
            {
                error = 40160;
                return false;
            }

            i++;
            for (; i < header_value.size(); i++)
            {
                if (header_value[i] >= '0' && header_value[i] <= '9')
                {
                    steam_httppeer->host.push_back(header_value[i]);
                }
                else if (header_value[i] >= 'a' && header_value[i] <= 'f')
                {
                    steam_httppeer->host.push_back(header_value[i]);
                }
                else if (header_value[i] >= 'A' && header_value[i] <= 'F')
                {
                    steam_httppeer->host.push_back(header_value[i] + 32);
                }
                else if (header_value[i] == ':')
                {
                    steam_httppeer->host.push_back(header_value[i]);
                }
                else if (header_value[i] == ']')
                {
                    i++;
                    if (i < header_value.size() && header_value[i] == 0x3A)
                    {
                        ishasport = true;
                        i++;
                        break;
                    }
                    break;
                }
                else
                {
                    error = 40161;
                    return false;
                }
            }
        }
        else
        {
            error = 40162;
            return false;
        }
    }

    unsigned int port_temp = 0;
    if (ishasport)
    {
        for (; i < header_value.size(); i++)
        {
            if (header_value[i] < 0x3A && header_value[i] > 0x2F)
            {
                port_temp = port_temp * 10 + (header_value[i] - 0x30);
                if (port_temp > 65535)
                {
                    break;
                }
            }
        }
    }
    // state.port = port;
    steam_httppeer->state.port = port_temp;
    return true;
}

void http2parse::header_process(std::string header_name, std::string header_value, int table_num, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("header:%s:%s|%d", header_name.c_str(), header_value.c_str(), table_num);
    if (table_num > 0)
    {

        switch (table_num)
        {
        case 1:
            if (header_value.size() > CONST_HTTP2_HOST_MAX_SIZE)
            {
                error = 40163;
                return;
            }
            if (header_host_process(header_value, steam_httppeer))
            {
                steam_httppeer->header["host"]       = header_value;
                steam_httppeer->header[":authority"] = std::move(header_value);
                steam_httppeer->find_host_index();
            }
            else
            {
                if (error > 0)
                {
                    return;
                }
            }
            //block_steam_httppeer->host                 = header_value;
            break;
        case 2:
            if (str_casecmp(header_value, "OPTIONS"))
            {
                steam_httppeer->method = 3;
            }
            else if (str_casecmp(header_value, "GET"))
            {
                steam_httppeer->method = 1;
            }
            else if (str_casecmp(header_value, "POST"))
            {
                steam_httppeer->method = 2;
            }
            else if (str_casecmp(header_value, "head"))
            {
                steam_httppeer->method = 5;
            }
            else if (str_casecmp(header_value, "put"))
            {
                steam_httppeer->method = 6;
            }
            else if (str_casecmp(header_value, "delete"))
            {
                steam_httppeer->method = 7;
            }
            else if (str_casecmp(header_value, "QUERY"))
            {
                steam_httppeer->method = 4;
            }
            else if (str_casecmp(header_value, "trace"))
            {
                steam_httppeer->method = 8;
            }
            else if (str_casecmp(header_value, "connect"))
            {
                steam_httppeer->method = 9;
            }
            steam_httppeer->iscors           = (steam_httppeer->method == 3);
            steam_httppeer->header["method"] = std::move(header_value);
            break;
        case 3:
            if (str_casecmp(header_value, "OPTIONS"))
            {
                steam_httppeer->method = 3;
            }
            else if (str_casecmp(header_value, "GET"))
            {
                steam_httppeer->method = 1;
            }
            else if (str_casecmp(header_value, "POST"))
            {
                steam_httppeer->method = 2;
            }
            else if (str_casecmp(header_value, "head"))
            {
                steam_httppeer->method = 5;
            }
            else if (str_casecmp(header_value, "put"))
            {
                steam_httppeer->method = 6;
            }
            else if (str_casecmp(header_value, "delete"))
            {
                steam_httppeer->method = 7;
            }
            else if (str_casecmp(header_value, "QUERY"))
            {
                steam_httppeer->method = 4;
            }
            else if (str_casecmp(header_value, "trace"))
            {
                steam_httppeer->method = 8;
            }
            else if (str_casecmp(header_value, "connect"))
            {
                steam_httppeer->method = 9;
            }
            steam_httppeer->iscors           = (steam_httppeer->method == 3);
            steam_httppeer->header["method"] = std::move(header_value);
            break;
        case 4:
        case 5:
            path_process(header_name, header_value, steam_httppeer);
            break;
        case 16:
            getacceptencoding(header_name, header_value, steam_httppeer);
            break;
        case 17:
            getacceptlanguage(header_name, header_value, steam_httppeer);
            break;
        case 19:
            getaccept(header_name, header_value, steam_httppeer);
            break;
        case 20:
            steam_httppeer->header["access-control-allow-origin"] = std::move(header_value);
            break;
        case 23:
            steam_httppeer->header["authorization"] = std::move(header_value);
            break;
        case 28:
        {
            unsigned long long temp_cl = 0;
            if (!str2uint64_strict(header_value, temp_cl) || temp_cl > CONST_HTTP_BODY_POST_SIZE)
            {
                error = 40187;
                return;
            }
            steam_httppeer->content_length           = temp_cl;
            steam_httppeer->header["content-length"] = std::move(header_value);
            break;
        }
        case 31:
            getcontenttype(header_name, header_value, steam_httppeer);
            break;
        case 32:
            cookie_process(header_name, header_value, steam_httppeer);
            break;
        case 38:
            // 静态表 38 = "host"。host 值和 :authority 一样限 72 字节：
            // 超长值会进入 header_host_process（见该函数的死循环注释）。
            if (header_value.size() > CONST_HTTP2_HOST_MAX_SIZE)
            {
                error = 40163;
                return;
            }
            if (header_host_process(header_value, steam_httppeer))
            {
                steam_httppeer->header["host"] = std::move(header_value);
                steam_httppeer->find_host_index();
            }
            else
            {
                if (error > 0)
                {
                    return;
                }
            }
            break;
        case 41:
            getifnonematch(header_name, header_value, steam_httppeer);
            break;
        case 48:
            steam_httppeer->header["proxy-authenticate"] = std::move(header_value);
            break;
        case 49:
            steam_httppeer->header["proxy-authorization"] = std::move(header_value);
            break;
        case 50:
            range_process(header_name, header_value, steam_httppeer);
            break;
        case 51:
            steam_httppeer->header["referer"] = std::move(header_value);
            break;
        case 58:
            steam_httppeer->header["user-agent"] = std::move(header_value);
            break;
        case 61:
            steam_httppeer->header["www-authenticate"] = std::move(header_value);
            break;
        }
    }
    else
    {
        if (header_name.size() > 48)
        {
            error = 40164;
            return;
        }
        std::string lower_name = str_tolower(header_name);
        switch (header_name.size())
        {
        case 5:
            if (str_casecmp(header_name, ":path"))
            {
                path_process(header_name, header_value, steam_httppeer);
            }
            else if (str_casecmp(header_name, "range"))
            {
                range_process(header_name, header_value, steam_httppeer);
            }
            steam_httppeer->header[lower_name] = std::move(header_value);
            break;
        case 6:

            switch (header_name[0])
            {
            case 'c':
            case 'C':
                if (str_casecmp(header_name, "Cookie"))
                {
                    cookie_process(header_name, header_value, steam_httppeer);
                }
                else
                {
                    steam_httppeer->header[lower_name] = std::move(header_value);
                }
                break;
            case 'a':
            case 'A':
                if (str_casecmp(header_name, "Accept"))
                {
                    getaccept(header_name, header_value, steam_httppeer);
                }
                else
                {
                    if (header_name[0] != ':')
                    {
                        steam_httppeer->header[lower_name] = std::move(header_value);
                    }
                }
                break;
            case 'o':
            case 'O':
                if (str_casecmp(header_name, "origin"))
                {
                    // 同 h1：跨域请求才走到这里，Origin 值就在手边直接判定。
                    // :authority 按规范必在普通头之前；万一违序或整单没带，host 为空，
                    // cors_origin_process() 在 h2 下不挂起，直接按路由回落的站点当场判定
                    steam_httppeer->cors_origin_process(header_value);
                }
                steam_httppeer->header[lower_name] = std::move(header_value);
                break;
            default:
                if (header_name[0] != ':')
                {
                    steam_httppeer->header[lower_name] = std::move(header_value);
                }
            }

            break;
        case 7:
            if (str_casecmp(header_name, ":method"))
            {
                if (str_casecmp(header_value, "OPTIONS"))
                {
                    steam_httppeer->method = 3;
                }
                else if (str_casecmp(header_value, "GET"))
                {
                    steam_httppeer->method = 1;
                }
                else if (str_casecmp(header_value, "POST"))
                {
                    steam_httppeer->method = 2;
                }
                else if (str_casecmp(header_value, "head"))
                {
                    steam_httppeer->method = 5;
                }
                else if (str_casecmp(header_value, "put"))
                {
                    steam_httppeer->method = 6;
                }
                else if (str_casecmp(header_value, "delete"))
                {
                    steam_httppeer->method = 7;
                }
                else if (str_casecmp(header_value, "QUERY"))
                {
                    steam_httppeer->method = 4;
                }
                else if (str_casecmp(header_value, "trace"))
                {
                    steam_httppeer->method = 8;
                }
                else if (str_casecmp(header_value, "connect"))
                {
                    steam_httppeer->method = 9;
                }
                steam_httppeer->iscors            = (steam_httppeer->method == 3);
                steam_httppeer->header["method"]  = header_value;
                steam_httppeer->header[":method"] = std::move(header_value);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;
        case 10:
            if (str_casecmp(header_name, ":authority"))
            {
                if (header_value.size() > CONST_HTTP2_HOST_MAX_SIZE)
                {
                    error = 40165;
                    return;
                }

                if (header_host_process(header_value, steam_httppeer))
                {
                    steam_httppeer->header["host"]       = header_value;
                    steam_httppeer->header[":authority"] = std::move(header_value);
                    steam_httppeer->find_host_index();
                }
                else
                {
                    if (error > 0)
                    {
                        return;
                    }
                }
            }
            else if (str_casecmp(header_name, "User-Agent"))
            {
                steam_httppeer->header["user-agent"] = std::move(header_value);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;
        case 12:
            if (str_casecmp(header_name, "Content-Type"))
            {
                getcontenttype(header_name, header_value, steam_httppeer);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;
        case 13:
            if (str_casecmp(header_name, "if-none-match"))
            {
                getifnonematch(header_name, header_value, steam_httppeer);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;

        case 14:
            if (str_casecmp(header_name, "Content-Length"))
            {
                unsigned long long temp_cl = 0;
                if (!str2uint64_strict(header_value, temp_cl) || temp_cl > CONST_HTTP_BODY_POST_SIZE)
                {
                    error = 40187;
                    return;
                }
                steam_httppeer->content_length           = temp_cl;
                steam_httppeer->header["content-length"] = std::move(header_value);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;
        case 15:
            if (header_name[7] == 'e' || header_name[7] == 'E')
            {
                if (str_casecmp(header_name, "accept-encoding"))
                {
                    getacceptencoding(header_name, header_value, steam_httppeer);
                    break;
                }
                else
                {
                    steam_httppeer->header[lower_name] = std::move(header_value);
                }
            }
            else if (header_name[7] == 'L' || header_name[7] == 'l')
            {
                if (str_casecmp(header_name, "Accept-Language"))
                {
                    getacceptlanguage(header_name, header_value, steam_httppeer);
                    break;
                }
                else
                {
                    steam_httppeer->header[lower_name] = std::move(header_value);
                }
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }

            if (str_casecmp(header_name, "Accept-Encoding"))
            {
                getacceptencoding(header_name, header_value, steam_httppeer);
            }
            if (str_casecmp(header_name, "Accept-Language"))
            {
                getacceptlanguage(header_name, header_value, steam_httppeer);
            }

            break;
        default:
            steam_httppeer->header[lower_name] = std::move(header_value);
        }
    }
}

void http2parse::getacceptlanguage([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    steam_httppeer->header["accept-language"] = header_value;
    unsigned int i                            = 0;
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] == 0x2C)
        {
            break;
        }
        steam_httppeer->state.language[i] = header_value[i];
        if (i > 6)
        {
            break;
        }
    }
    for (; i < 8; i++)
    {
        steam_httppeer->state.language[i] = 0x00;
    }
}
void http2parse::getacceptencoding([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i = 0, linesize = static_cast<unsigned int>(header_value.size());
    steam_httppeer->header["accept-encoding"] = header_value;
    std::string buffer_value;
    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C)
        {

            switch (buffer_value.size())
            {
            case 2:
                if (buffer_value[0] == 'b')
                {
                    steam_httppeer->state.br = true;
                }
                break;
            case 4:
                if (buffer_value[0] == 'g')
                {
                    steam_httppeer->state.gzip = true;
                }
                else if (buffer_value[0] == 'z')
                {
                    steam_httppeer->state.zstd = true;
                }
                break;
            case 7:
                if (buffer_value[0] == 'd')
                {
                    steam_httppeer->state.deflate = true;
                }
                break;
            default:;
            }
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        buffer_value.push_back(header_value[i]);
    }
    if (buffer_value.size() > 0)
    {

        switch (buffer_value.size())
        {
        case 2:
            if (buffer_value[0] == 'b')
            {
                steam_httppeer->state.br = true;
            }
            break;
        case 4:
            if (buffer_value[0] == 'g')
            {
                steam_httppeer->state.gzip = true;
            }
            else if (buffer_value[0] == 'z')
            {
                steam_httppeer->state.zstd = true;
            }
            break;
        case 7:
            if (buffer_value[0] == 'd')
            {
                steam_httppeer->state.deflate = true;
            }
            break;
        default:;
        }
    }
}
void http2parse::getifnonematch([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i                          = 0;
    steam_httppeer->header["if-none-match"] = header_value;
    steam_httppeer->etag.clear();
    if (header_value.size() < 2)
    {
        return;
    }
    if (header_value[i] == 'W' || header_value[i] == 'w')
    {
        if (header_value[i + 1] == 0x2F)
        {
            i += 2;
        }
    }
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] != 0x22)
        {
            steam_httppeer->etag.push_back(header_value[i]);
        }
    }
}

void http2parse::callposttype(const std::string &buffer_value, std::shared_ptr<httppeer> steam_httppeer, HTTP2_POST_DATA_T &pd)
{
    switch (buffer_value.size())
    {
    case 33:
        if (str_casecmp(buffer_value, "application/x-www-form-urlencoded"))
        {
            steam_httppeer->content_type = "application/x-www-form-urlencoded";
            pd.posttype                  = 1;
            steam_httppeer->posttype     = 1;
            return;
        }
        break;
    case 24:
        if (str_casecmp(buffer_value, "application/octet-stream"))
        {
            steam_httppeer->content_type = "application/octet-stream";
            pd.posttype                  = 5;
            steam_httppeer->posttype     = 5;
            return;
        }
        break;
    case 19:
        if (str_casecmp(buffer_value, "multipart/form-data"))
        {
            steam_httppeer->content_type = "multipart/form-data";
            //block_data_info_ptr->posttype      = 2;
            steam_httppeer->posttype = 2;
            return;
        }
        break;
    case 16:
        if (str_casecmp(buffer_value, "application/json"))
        {
            steam_httppeer->content_type = "application/json";
            pd.posttype                  = 3;
            steam_httppeer->posttype     = 3;
            return;
        }
        break;
    case 15:
        if (str_casecmp(buffer_value, "application/xml"))
        {
            steam_httppeer->content_type = "application/xml";
            pd.posttype                  = 4;
            steam_httppeer->posttype     = 4;
            return;
        }
        break;
    case 8:
        if (str_casecmp(buffer_value, "text/xml"))
        {
            steam_httppeer->content_type = "text/xml";
            pd.posttype                  = 4;
            steam_httppeer->posttype     = 4;
            return;
        }
        break;
    case 6:
        if (str_casecmp(buffer_value, "binary"))
        {
            steam_httppeer->content_type = "binary";
            pd.posttype                  = 6;
            steam_httppeer->posttype     = 5;
            return;
        }
        break;
    default:
        steam_httppeer->content_type = "raw";
        pd.posttype                  = 7;
        steam_httppeer->posttype     = 5;
        return;
    }
    steam_httppeer->content_type = "raw";
    pd.posttype                  = 7;
    steam_httppeer->posttype     = 5;
    return;
}

void http2parse::getcontenttype([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i = 0, linesize = static_cast<unsigned int>(header_value.size());
    steam_httppeer->header["content-type"] = header_value;
    std::string buffer_value;
    unsigned char statetemp = 0;
    auto iter               = http_post_data.find(steam_httppeer->stream_id);
    if (iter == http_post_data.end())
    {
        HTTP2_POST_DATA_T temp_p;
        temp_p.peer      = steam_httppeer;
        temp_p.stream_id = steam_httppeer->stream_id;
        http_post_data.insert_or_assign(steam_httppeer->stream_id, std::move(temp_p));
        iter = http_post_data.find(steam_httppeer->stream_id);
        if (iter == http_post_data.end())
        {
            return;
        }
    }

    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x3B)
        {

            if (statetemp == 0)
            {
                callposttype(buffer_value, steam_httppeer, iter->second);
            }
            else if (statetemp == 1)
            {
                iter->second.chartset = buffer_value;
                statetemp             = 0;
            }
            else if (statetemp == 2)
            {
                if (!http2_boundary_normalize(iter->second.boundary, buffer_value))
                {
                    error = 40099;
                    return;
                }
                statetemp = 0;
            }
            /////////////////////
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3D)
        {
            if (str_casecmp(buffer_value, "charset"))
            {
                buffer_value.clear();
                statetemp = 1;
                continue;
            }
            else if (str_casecmp(buffer_value, "boundary"))
            {
                buffer_value.clear();
                statetemp = 2;
                continue;
            }
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        buffer_value.push_back(header_value[i]);
    }
    if (buffer_value.size() > 0)
    {
        if (statetemp == 1)
        {
            iter->second.chartset = buffer_value;
        }
        else if (statetemp == 2)
        {
            if (!http2_boundary_normalize(iter->second.boundary, buffer_value))
            {
                error = 40099;
                return;
            }
        }
        else
        {
            callposttype(buffer_value, steam_httppeer, iter->second);
        }
    }
}
void http2parse::getaccept([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i = 0, linesize = static_cast<unsigned int>(header_value.size());
    steam_httppeer->header["accept"] = header_value;
    std::string buffer_value;

    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C || header_value[i] == 0x3B)
        {
            switch (buffer_value.length())
            {
            case 16:
                //application/json
                if (str_casecmp(buffer_value, "application/json"))
                {
                    steam_httppeer->state.accept_json = true;
                }
                break;
            case 15:
                //application/xml
                if (str_casecmp(buffer_value, "application/xml"))
                {
                    steam_httppeer->state.accept_xml = true;
                }
                break;
            case 9:
                //text/json
                if (str_casecmp(buffer_value, "text/json"))
                {
                    steam_httppeer->state.accept_xml = true;
                }
                break;
            case 8:
                //text/xml
                if (str_casecmp(buffer_value, "text/xml"))
                {
                    steam_httppeer->state.accept_xml = true;
                }
                break;
            case 10:
                if (buffer_value[6] == 'a' &&
                    buffer_value[7] == 'v' &&
                    buffer_value[8] == 'i' && buffer_value[9] == 'f')
                {
                    steam_httppeer->state.avif = true;
                }
                else if (buffer_value[6] == 'w' &&
                         buffer_value[7] == 'e' &&
                         buffer_value[8] == 'b' &&
                         buffer_value[9] == 'p')
                {
                    steam_httppeer->state.webp = true;
                }
                break;
            default:;
            }
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        buffer_value.push_back(header_value[i]);
    }
    if (buffer_value.size() > 0)
    {
        switch (buffer_value.length())
        {
        case 10:
            if (buffer_value[6] == 'a' && buffer_value[7] == 'v' &&
                buffer_value[8] == 'i' && buffer_value[9] == 'f')
            {
                steam_httppeer->state.avif = true;
            }
            else if (buffer_value[6] == 'w' &&
                     buffer_value[7] == 'e' &&
                     buffer_value[8] == 'b' && buffer_value[9] == 'p')
            {
                steam_httppeer->state.webp = true;
            }
            break;
        case 16:
            //application/json
            if (str_casecmp(buffer_value, "application/json"))
            {
                steam_httppeer->state.accept_json = true;
            }
            break;
        case 15:
            //application/xml
            if (str_casecmp(buffer_value, "application/xml"))
            {
                steam_httppeer->state.accept_xml = true;
            }
            break;
        case 9:
            //text/json
            if (str_casecmp(buffer_value, "text/json"))
            {
                steam_httppeer->state.accept_xml = true;
            }
            break;
        case 8:
            //text/xml
            if (str_casecmp(buffer_value, "text/xml"))
            {
                steam_httppeer->state.accept_xml = true;
            }
            break;
        default:;
        }
    }
}
void http2parse::headertype1(unsigned char c,
                             [[maybe_unused]] std::string_view header_data,
                             [[maybe_unused]] unsigned int &begin,
                             std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned char a = c & 0x7F;
    // std::string name_key;
    // std::string value;

    if (a < 62)
    {
        // name_key = http2_header_static_table[a].key;
        // value    = http2_header_static_table[a].value;
        if (a == 2 || a == 3)
        {
            steam_httppeer->header[":method"] = http2_header_static_table[a].value;
            if (a == 3)
            {
                steam_httppeer->method = 2;
                //ispost                       = true;
            }
            else
            {
                steam_httppeer->method = 1;
                //ispost                       = false;
            }
        }
        else if (a == 4 || a == 5)
        {
            steam_httppeer->url             = http2_header_static_table[a].value;
            steam_httppeer->urlpath         = http2_header_static_table[a].value;
            steam_httppeer->header["path"]  = http2_header_static_table[a].value;
            steam_httppeer->header[":path"] = http2_header_static_table[a].value;
        }
        else if (a == 6 || a == 7)
        {

            steam_httppeer->isssl             = (a == 7) ? true : false;
            steam_httppeer->header[":scheme"] = http2_header_static_table[a].value;
        }
        else
        {
            header_process(http2_header_static_table[a].key, http2_header_static_table[a].value, a, steam_httppeer);
        }
    }
    else
    {
        a -= 62;
        unsigned int j = 0;
        for (const auto &hinfo : dynamic_lists)
        {
            if (j == a)
            {
                header_process(hinfo.first, hinfo.second, 0, steam_httppeer);
                // name有可能缓存起来了 header name maybe cache to dynamic table
                break;
            }
            j++;
        }
    }
}
void http2parse::headertype2(unsigned char c, std::string_view header_data, unsigned int &begin, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned char a;
    unsigned int item_length;
    unsigned char field_state = 0;
    bool ishuffman_value      = false;
    std::string name_key;
    std::string value;

    a = c & 0x3f;
    if (a == 0)
    {
        begin += 1;
        if (begin >= header_data.size())
        {
            error = 40128;
            return;
        }
        item_length = header_data[begin] & 0x7F;

        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        bool iscontinue = false;
        if (item_length == 0x7F)
        {
            iscontinue = true;
        }
        begin += 1;

        if (iscontinue)
        {
            if (begin >= header_data.size())
            {
                error = 40129;
                return;
            }
            iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
            begin += 1;
            if (begin >= header_data.size())
            {
                error = 40130;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 40178;
            return;
        }

        if (ishuffman_value)
        {
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, name_key, 1);
        }
        else
        {
            name_key.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;
        if (begin >= header_data.size())
        {
            error = 40131;
            return;
        }
        // huffman
        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {
            if (begin >= header_data.size())
            {
                error = 40132;
                return;
            }
            iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
            begin += 1;
            if (begin >= header_data.size())
            {
                error = 40133;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 40179;
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;

        header_process(name_key, value, 0, steam_httppeer);
        dynamic_lists.push_front({name_key, std::move(value)});

        begin -= 1;
        if (dynamic_lists.size() > 255)
        {
            dynamic_lists.pop_back();
        }
    }
    else
    {
        a = header_data[begin] & 0x3F;
        if (a < 62)
        {
            name_key = http2_header_static_table[a].key;
            c        = a;
        }
        else
        {
            // 此处需要第二个字节 need next char
            c = 0;
            if (a == 0x3F)
            {
                begin += 1;
                if (begin >= header_data.size())
                {
                    error = 40134;
                    return;
                }

                a = a + (header_data[begin] & 0x7F);
                if (header_data[begin] & 0x80)
                {
                    error = 40181;
                    return;
                }
            }
            if (a < 62)
            {
                name_key = http2_header_static_table[a].key;
                c        = a;
            }
            else
            {
                a -= 62;
                unsigned int j = 0;
                for (const auto &hinfo : dynamic_lists)
                {
                    if (j == a)
                    {
                        name_key = hinfo.first;
                        break;
                    }
                    j++;
                }
            }
        }

        begin += 1;
        if (begin >= header_data.size())
        {
            error = 40135;
            return;
        }
        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }
        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {
            if (begin >= header_data.size())
            {
                error = 40136;
                return;
            }
            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
            begin += 1;
            if (begin >= header_data.size())
            {
                error = 40137;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 40182;
                    return;
                }
                begin += 1;
            }
        }
        if ((begin + item_length) > header_data.size())
        {
            error = 40180;
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;

        header_process(name_key, value, c, steam_httppeer);
        dynamic_lists.push_front({name_key, std::move(value)});

        begin -= 1;
        if (dynamic_lists.size() > 255)
        {
            dynamic_lists.pop_back();
        }
    }
}
void http2parse::headertype3(unsigned char c, std::string_view header_data, unsigned int &begin, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned char a;
    unsigned int item_length;
    unsigned char field_state = 0;
    bool ishuffman_value      = false;
    std::string name_key;
    std::string value;

    a = c & 0x0F;
    if (a == 0)
    {
        begin += 1;
        if (begin >= header_data.size())
        {
            error = 40138;
            return;
        }
        item_length = header_data[begin] & 0x7F;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        bool iscontinue = false;
        if (item_length == 0x7F)
        {
            iscontinue = true;
        }
        begin += 1;

        if (iscontinue)
        {
            if (begin >= header_data.size())
            {
                error = 40139;
                return;
            }
            if (header_data[begin] & 0x80)
            {
                error = 40186;
                return;
            }
            else
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 40183;
            return;
        }
        if (ishuffman_value)
        {
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, name_key, 1);
        }
        else
        {
            name_key.append((char *)&header_data[begin], item_length);
        }

        begin           = begin + item_length;
        ishuffman_value = false;
        if (begin >= header_data.size())
        {
            error = 40140;
            return;
        }
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {
            if (begin >= header_data.size())
            {
                error = 40141;
                return;
            }
            iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
            begin += 1;
            if (begin >= header_data.size())
            {
                error = 40142;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 40188;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 40184;
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }
        begin = begin + item_length;

        header_process(name_key, value, 0, steam_httppeer);
        begin -= 1;
    }
    else
    {
        a = header_data[begin] & 0x0F;
        if (a != 0x0F)
        {
            name_key = http2_header_static_table[a].key;
            c        = a;
        }
        else
        {

            begin += 1;
            if (begin >= header_data.size())
            {
                error = 40143;
                return;
            }
            a += header_data[begin];
            if (header_data[begin] & 0x80)
            {
                error = 40187;
                return;
            }
            if (a < 62)
            {
                name_key = http2_header_static_table[a].key;
                c        = a;
            }
            else
            {
                c = 0;
                a -= 62;
                unsigned int j = 0;
                for (const auto &hinfo : dynamic_lists)
                {
                    if (j == a)
                    {
                        name_key = hinfo.first;
                        break;
                    }
                    j++;
                }
            }
        }

        begin += 1;
        if (begin >= header_data.size())
        {
            error = 40144;
            return;
        }
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }
        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {
            if (begin >= header_data.size())
            {
                error = 40145;
                return;
            }
            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
            begin += 1;
            if (begin >= header_data.size())
            {
                error = 40146;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 40189;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 40185;
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;
        header_process(name_key, value, c, steam_httppeer);
        begin -= 1;
    }
}
void http2parse::headertype4(unsigned char c, std::string_view header_data, unsigned int &begin, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned char a;
    unsigned int item_length;
    unsigned char field_state = 0;
    bool ishuffman_value      = false;
    std::string name_key;
    std::string value;
    a = c & 0x0F;
    if (a == 0)
    {

        begin += 1;
        if (begin >= header_data.size())
        {
            error = 40147;
            return;
        }

        item_length = header_data[begin] & 0x7F;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        bool iscontinue = false;
        if (header_data[begin] == 0x7F)
        {
            iscontinue = true;
        }
        begin += 1;

        if (iscontinue)
        {
            if (begin >= header_data.size())
            {
                error = 40148;
                return;
            }

            if (header_data[begin] & 0x80)
            {
                error = 40192;
                return;
            }
            else
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 40190;
            return;
        }

        if (ishuffman_value)
        {
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, name_key, 1);
        }
        else
        {
            name_key.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;
        if (begin >= header_data.size())
        {
            error = 40149;
            return;
        }
        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {

            if (begin >= header_data.size())
            {
                error = 40150;
                return;
            }
            iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
            begin += 1;
            if (begin >= header_data.size())
            {
                error = 40151;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 40194;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 40191;
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;
        header_process(name_key, value, 0, steam_httppeer);
        begin -= 1;
    }
    else
    {
        a = header_data[begin] & 0x0F;
        if (a == 0x0F)
        {
            begin += 1;
            if (begin >= header_data.size())
            {
                error = 40152;
                return;
            }
            a = a + (header_data[begin] & 0x7F);
            if (header_data[begin] & 0x80)
            {
                error = 40193;
                return;
            }
        }

        if (a < 62)
        {
            name_key = http2_header_static_table[a].key;
            c        = a;
        }
        else
        {
            c = 0;
            a -= 62;
            unsigned int j = 0;
            for (const auto &hinfo : dynamic_lists)
            {
                if (j == a)
                {
                    name_key = hinfo.first;
                    break;
                }
                j++;
            }
        }

        begin += 1;
        ishuffman_value = false;
        if (begin >= header_data.size())
        {
            error = 40153;
            return;
        }

        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;

        if (item_length == 0x7F)
        {
            if (begin >= header_data.size())
            {
                error = 40154;
                return;
            }
            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (unsigned int)(header_data[begin] & 0x7F);
            begin += 1;

            if (iscontinue)
            {
                if (begin >= header_data.size())
                {
                    error = 40155;
                    return;
                }
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 40195;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 40156;
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;

        header_process(name_key, value, c, steam_httppeer);
        begin -= 1;
    }
}

void http2parse::readsetting(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    // unsigned int j = readoffset;
    if (temp_pack_data.flags == 0x01)
    {
        DEBUG_LOG("readsetting fackback %zu ", temp_pack_data.payload.size());
        return;
    }

    unsigned short ident_type;
    unsigned int ident_value;

    // 发送窗口只在「本连接还没初始化」时取 RFC 默认值 65535。
    // 若每收到一个 SETTINGS 帧都无条件重置成 65535：客户端中途补发一个 SETTINGS
    // （哪怕不含 INITIAL_WINDOW_SIZE）就会把之前 WINDOW_UPDATE 发放的发送额度全部抹掉，
    // 正在发送的响应会莫名其妙卡住。
    if (!conn_send_window_inited)
    {
        conn_send_window_inited         = true;
        peer_session->window_update_num = CONST_HTTP2_DEFAULT_WINDOW;
    }

    for (size_t n = 0; n < temp_pack_data.payload.size(); n += 6)
    {
        if ((n + 5) >= temp_pack_data.payload.size())
        {
            break;
        }
        ident_type = temp_pack_data.payload[n];
        ident_type = ident_type << 8 | (unsigned char)temp_pack_data.payload[n + 1];

        ident_value = temp_pack_data.payload[n + 2];
        ident_value = ident_value << 8 | (unsigned char)temp_pack_data.payload[n + 3];
        ident_value = ident_value << 8 | (unsigned char)temp_pack_data.payload[n + 4];
        ident_value = ident_value << 8 | (unsigned char)temp_pack_data.payload[n + 5];

        switch (ident_type)
        {
        case HTTP2_SETTINGS_HEADER_TABLE_SIZE:
            setting_data.header_table_size = ident_value;
            break;

        case HTTP2_SETTINGS_ENABLE_PUSH:
            if (ident_value > 1)
            {
                // RFC 9113 §6.5.2: 只允许 0/1，其他值 MUST 视为 PROTOCOL_ERROR
                error = 40037;
                return;
            }
            setting_data.enable_push = ident_value;
            break;

        case HTTP2_SETTINGS_MAX_CONCURRENT_STREAM:
            setting_data.max_concurrent_streams = ident_value;
            break;

        case HTTP2_SETTINGS_INITIAL_WINDOW_SIZE:

            if (ident_value > CONST_HTTP2_MAX_WINDOW)
            {
                // RFC 9113 §6.5.2: 超过 2^31-1 必须按 FLOW_CONTROL_ERROR 处理。
                // 不校验的话客户端报 0xFFFFFFFF 就能把发送窗口直接顶到 4G。
                error = 40018;
                return;
            }
            setting_data.initial_window_size = ident_value;
            // INITIAL_WINDOW_SIZE 是「每一条流」的发送窗口初值，
            // 绝不能赋给连接级窗口 window_update_num。RFC 9113 §6.9.2 还要求：
            // 该值变化时，所有已存在流的发送窗口按 (new - old) 等额调整。
            {
                std::lock_guard<std::mutex> lk(peer_session->stream_send_window_mutex);
                long long old_iws = static_cast<long long>(peer_session->remote_initial_window_size.load());
                long long new_iws = static_cast<long long>(ident_value);
                long long delta   = new_iws - old_iws;
                for (auto &kv : peer_session->stream_send_window)
                {
                    long long cur = static_cast<long long>(kv.second);
                    cur += delta;
                    if (cur < 0)
                    {
                        cur = 0;
                    }
                    // 只归零不封顶会留下 > 2^31-1 的窗口：delta 最大 ~2GB，一条本来就有
                    // ~2GB 额度的流相加能到 4.29e9，而本端没广告 §6.9.2 扩展流控。更要紧的是
                    // readwindowupdate 那道 "cur > MAX - inc" 在这种值上恒真，此后这条流收到
                    // 的每一个合法 WINDOW_UPDATE 都会被本端 RST 掉。与退还处同法封顶。
                    // Flooring at 0 without capping the top leaves windows above 2^31-1: delta can be
                    // ~2GB, so a stream already holding ~2GB sums to ~4.29e9 — still fits a u32, so it
                    // doesn't crash, but this end never advertised the extended flow control of §6.9.2.
                    // Worse, readwindowupdate's "cur > MAX - inc" gate is then always true, and every
                    // legitimate WINDOW_UPDATE on this stream gets RST by us. Cap it the same way the
                    // refund path does.
                    if (cur > static_cast<long long>(CONST_HTTP2_MAX_WINDOW))
                    {
                        cur = CONST_HTTP2_MAX_WINDOW;
                    }
                    kv.second = static_cast<unsigned int>(cur);
                }
            }
            peer_session->remote_initial_window_size.store(ident_value);
            break;

        case HTTP2_SETTINGS_MAX_FRAME_SIZE:
            if (ident_value < 16384 || ident_value > 16777215)
            {
                // RFC 9113 §6.5.2: 合法范围 [2^14, 2^24-1]
                error = 40034;
                return;
            }
            setting_data.max_frame_size = ident_value;
            // 发送分片要按对端上限封顶，所以这个值必须镜像到连接上（发送侧读得到）。
            peer_session->remote_max_frame_size.store(ident_value);
            break;

        case HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE:
            setting_data.max_heaer_list_size = ident_value;
            break;

        default:
            break;
        }
    }

    peer_session->send_recv_setting();
    // 客户端的 INITIAL_WINDOW_SIZE 描述的是「它自己愿意接收多少」，约束的是本端的发送额度，
    // 与本端的接收窗口无关。把它直接写进本端接收窗口计数器，客户端报个超大值就能让
    // 本端长期不回 WINDOW_UPDATE。这里固定用本端自己声明的窗口值。
    //
    // 连接级窗口另有 RFC 9113 §6.9.2 固定的初值 65535，SETTINGS 改不了它，只能用
    // stream id = 0 的 WINDOW_UPDATE 抬升。抬升必须「只做一次」：每收到一个 SETTINGS
    // 就重发一次会让对端连接窗口线性增长并顶穿 2^31-1（对端 FLOW_CONTROL_ERROR）。
    if (!conn_recv_window_raised)
    {
        conn_recv_window_raised = true;
        peer_session->send_window_update_conn(CONST_HTTP2_WINDOW_UPDATE_STEP);
        conn_recv_window_num = CONST_HTTP2_LOCAL_INITIAL_WINDOW;
    }
    need_wakeup_send_threads = true;
}
void http2parse::readpriority(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    if (temp_pack_data.payload.size() > 16384)
    {
        error = 40017;
    }
}

void http2parse::readgoaway(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    DEBUG_LOG("readgoaway %zu", temp_pack_data.payload.size());

    if (temp_pack_data.stream_id != 0)
    {
        error = 40044;
        return;
    }
    if (temp_pack_data.flags != 0)
    {
        error = 40203;
        return;
    }

    peer_session->isgoway = true;
}

void http2parse::readwinupdate(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    DEBUG_LOG("readwinupdate %zu", temp_pack_data.payload.size());

    // RFC 9113 §6.9.1：WINDOW_UPDATE 的载荷恰好 4 字节，其他长度是 FRAME_SIZE_ERROR。
    // 只判 < 4 时，多出来的字节会被静默忽略（分帧按帧头声明的 length 精确消费，
    // 不会造成后续帧错位，但该报错的不报）。
    if (temp_pack_data.payload.size() != 4)
    {
        error = 40012;
        return;
    }

    unsigned char temp_n  = 0;
    temp_n                = temp_pack_data.payload[0] & 0x7F;
    uint32_t ident_stream = temp_n;
    ident_stream          = ident_stream << 8;

    temp_n       = temp_pack_data.payload[1];
    ident_stream = ident_stream + temp_n;
    ident_stream = ident_stream << 8;

    temp_n       = temp_pack_data.payload[2];
    ident_stream = ident_stream + temp_n;
    ident_stream = ident_stream << 8;

    temp_n       = temp_pack_data.payload[3];
    ident_stream = ident_stream + temp_n;

    if (ident_stream == 0)
    {
        // RFC 9113 §6.9.1: WINDOW_UPDATE 的增量必须大于 0，为 0 属于 PROTOCOL_ERROR
        error = 40036;
        return;
    }

    // ident_stream 实际上是 window_size_increment（4 字节载荷），帧头里的
    // 流 id 在 temp_pack_data.stream_id。按 stream_id 路由到连接级或流级发送窗口，
    // 两套记账各自独立做上溢校验。
    DEBUG_LOG("window_update stream_id=%u inc=%u", temp_pack_data.stream_id, ident_stream);

    if (temp_pack_data.stream_id == 0)
    {
        // 连接级发送窗口：只受 stream id = 0 的 WINDOW_UPDATE 影响。
        //
        // window_update_num 是「对端累计授予额」，不是剩余额：发送侧扣的是
        // has_send_update_num（累计已发），剩余额 = 两者之差（见
        // http2_loop_send_sequence 的连接级窗口判定）。2^31-1 上限约束的是
        // 「未确认的剩余额度」，所以溢出校验必须作用在差值上；作用在累计值上会
        // 在长连接上必然误杀：累计授予额只增不减，发满 2^31-1 字节之后，
        // 每一个合法的 WINDOW_UPDATE 都会被拒成连接级错误。
        const unsigned long long granted = peer_session->window_update_num.load();
        if (!http2_wu_conn_avail_ok(granted, peer_session->has_send_update_num.load(), ident_stream))
        {
            // RFC 9113 §6.9.1: 发送窗口超过 2^31-1 必须按 FLOW_CONTROL_ERROR 处理。
            // 连接级的额度错在连接上，只能整条断开（error 通道回 GOAWAY）。
            error = 40036;
            return;
        }
        peer_session->window_update_num.store(granted + ident_stream);
        // 连接级发送窗口抬升：可能有流因为「累计授予 - 累计已发 = 0」挂在那里
        need_wakeup_send_threads = true;
    }
    else
    {
        // 流级发送窗口：只受对应流的 WINDOW_UPDATE 影响。
        const unsigned int wu_sid = temp_pack_data.stream_id;

        // 三道门，任一不过就丢弃这一帧：
        //  ① 偶数流 id 留给服务端发起的流，本实现不开 push，收到即非法；
        //  ② 大于本连接已见过的最大客户端流 id，说明这条流还没 OPEN；
        //  ③ 本端根本没见过这条流，或它已经结束/被撤销。
        // 只丢弃、不报错，也**绝不 emplace**：emplace 会把一个陌生流 id 写进
        // stream_send_window，而该表只按 RST 和流结束清理，等于给远程对端留了一个
        // 无界增长的入口。检查流是否存活，只看本端真正认下的流：
        // http_data（readheaders 收 HEADERS 时 emplace，分发时 extract 走）与
        // http_data_weak（分发末尾登记，响应结束后随 shared_ptr 释放而失效）并集，
        // 恰好覆盖「HEADERS 已收下」到「响应发完」整段。单用后者会在
        // 「同一批字节里 HEADERS 之后紧跟 WINDOW_UPDATE」处漏掉——那时 weak 还没写。
        // 也不能只靠 ②：对端发一个 HEADERS 把 max_client_stream_id 抬到 99 万，
        // 就能凭 ② 放行 50 万个陌生流 id。
        // 也不能对上面没通过检查、被丢弃的流回 RST_STREAM：本端没有流状态机，分不清 idle 与 closed，
        // 对 closed 流发 RST 是我们自己新造出来的协议违规（RFC 9113 §5.1 只允许
        // 对 half-closed 之外的流发 RST，且对 closed 流的一切帧都该忽略）。
        if (!http2_wu_stream_id_ok(wu_sid, max_client_stream_id))
        {
            winupdate_dropped++;
            DEBUG_LOG("http2 window_update on invalid stream %u", wu_sid);
            return;
        }
        {
            auto weak_iter = http_data_weak.find(wu_sid);
            bool stream_known =
                http_data.contains(wu_sid) ||
                (weak_iter != http_data_weak.end() && weak_iter->second.lock() != nullptr);
            if (!stream_known)
            {
                winupdate_dropped++;
                DEBUG_LOG("http2 window_update on unknown or closed stream %u", wu_sid);
                return;
            }
        }

        std::lock_guard<std::mutex> lk(peer_session->stream_send_window_mutex);
        auto it = peer_session->stream_send_window.find(wu_sid);
        if (it == peer_session->stream_send_window.end())
        {
            // 上面三道门已确认流是活的：服务端建流时并不预填本表（只在真正发 DATA
            // 前懒初始化），所以这里合法的空位表示「流在，只是还没发过 body」。
            // 这是全函数唯一允许的懒初始化入口。
            it = peer_session->stream_send_window
                     .emplace(wu_sid, peer_session->remote_initial_window_size.load())
                     .first;
        }
        // stream_send_window 存的是「剩余额度」（发送时直接扣减），所以溢出校验
        // 作用在本值上就是对的，与连接级那套「累计授予额」不同。
        unsigned long long cur = it->second;
        if (cur > (unsigned long long)(CONST_HTTP2_MAX_WINDOW - ident_stream))
        {
            // 流级溢出只撤这一条流：额度错在流上，按 RFC 9113 §5.1 用流错误处理，
            // 不必带走整条连接上其它在途的流。
            peer_session->http2_send_rst_stream(wu_sid, CONST_HTTP2_STREAM_ERROR_FLOW_CONTROL);
            need_wakeup_send_threads = true;
            return;
        }
        it->second = static_cast<unsigned int>(cur + ident_stream);
        // 流级窗口抬升：这条流可能就是挂在 parked_list 里等它的那一个
        need_wakeup_send_threads = true;
    }
    // 注意：本函数只影响「发送」窗口。本端接收侧另有 conn_recv_window_num /
    // stream_recv_window 两套记账，客户端的 WINDOW_UPDATE 绝不能改动它们。
}
//
void http2parse::readping(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    if ((temp_pack_data.flags & 0x01) > 0)
    {
        DEBUG_LOG("readping ack %d", temp_pack_data.length);
        return;
    }

    unsigned char _recvack[] =
        {0x00, 0x00, 0x08, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned int i = 0;
    for (; i < temp_pack_data.payload.size(); i++)
    {
        if (i < 8)
        {
            _recvack[i + 9] = temp_pack_data.payload[i];
        }
    }
    peer_session->http2_ring_queue->push(_recvack, 17);
    need_wakeup_send_threads = true;
    DEBUG_LOG("need ack ping");
}
void http2parse::readrst_stream(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    DEBUG_LOG("readrst_stream %u ", temp_pack_data.stream_id);
    // connection means the peer is opening then immediately cancelling streams
    // to exhaust CPU. Once the count exceeds 250, flag an error so the caller
    // sends GOAWAY and closes the connection.
    if (rst_stream_count < 255)
    {
        rst_stream_count++;
    }
    if (rst_stream_count > 250)
    {
        error = 40210;
    }
    auto iter = http_data_weak.find(temp_pack_data.stream_id);
    if (iter != http_data_weak.end())
    {
        std::shared_ptr<httppeer> temp_peer = iter->second.lock();
        if (temp_peer)
        {
            temp_peer->isclose = true;
        }
        http_data_weak.erase(iter);
    }
    // 流已被对端撤销，不再需要为它记账
    stream_recv_window.erase(temp_pack_data.stream_id);
    // 同步清理发送侧该流的窗口记账，避免 map 随被撤销的流累积
    {
        std::lock_guard<std::mutex> lk(peer_session->stream_send_window_mutex);
        peer_session->stream_send_window.erase(temp_pack_data.stream_id);
    }
}

void http2parse::readrawfileformdata(HTTP2_POST_DATA_T &temp_post_data, unsigned char islast_pack)
{
    if (!temp_post_data.fp)
    {
        server_loaclvar &localvar = get_server_global_var();

        temp_post_data.filename   = "rawcontent";
        temp_post_data.field_name = "rawcontent";

        // 落盘名统一由 make_http_temp_raw_name() 生成：文件名带 pzraw_ 前缀，
        // 该前缀是 httpwatch 周期清理时识别「框架自己的临时文件」的唯一依据，
        // 不要在此自行拼接文件名，否则文件会长期留在 temp_path 里。
        temp_post_data.temp_filename = localvar.temp_path;// + "temp/";
        temp_post_data.temp_filename.append(http::make_http_temp_raw_name());

        std::unique_ptr<std::FILE, int (*)(FILE *)> fpa(std::fopen(temp_post_data.temp_filename.c_str(), "wb"), std::fclose);
        if (fpa)
        {
            temp_post_data.fp = std::move(fpa);
        }
        else
        {
            error = 40167;
            return;
        }
        temp_post_data.peer->files["rawcontent"].set_object();
        temp_post_data.peer->files["rawcontent"]["filename"] = temp_post_data.filename;
        temp_post_data.peer->files["rawcontent"]["name"]     = temp_post_data.field_name;
        temp_post_data.peer->files["rawcontent"]["tempfile"] = temp_post_data.temp_filename;
        temp_post_data.peer->files["rawcontent"]["type"]     = temp_post_data.mimetype;
        temp_post_data.peer->files["rawcontent"]["size"]     = temp_post_data.cur_length;
        temp_post_data.peer->files["rawcontent"]["error"]    = 0;
    }

    if (temp_post_data.fp == nullptr)
    {
        return;
    }
    if (temp_post_data.fp)
    {
        temp_post_data.cur_length += temp_post_data.content.size();
        size_t n = fwrite(temp_post_data.content.data(), 1, temp_post_data.content.size(), temp_post_data.fp.get());
        if (n != temp_post_data.content.size())
        {
            temp_post_data.fp.reset();
            error = 40168;
        }
    }

    if (islast_pack > 0)
    {
        temp_post_data.fp.reset();
        temp_post_data.peer->files["rawcontent"]["size"] = temp_post_data.cur_length;
    }
}

void http2parse::post_form_to_postfield(std::string_view form_post_name, std::string_view form_post_value, std::shared_ptr<httppeer> steam_httppeer)
{
    bool isgroup = true;
    if (form_post_name.length() > 72)
    {
        error = 40207;
        return;
    }

    for (size_t j = 0; j < form_post_name.length(); j++)
    {
        if (form_post_name[j] == '[')
        {
            isgroup = false;
        }
    }

    if (isgroup)
    {
        steam_httppeer->post[form_post_name] = form_post_value;
        return;
    }

    isgroup = true;
    std::string objname;
    for (size_t j = 0; j < form_post_name.length(); j++)
    {
        if (form_post_name[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < form_post_name.length(); n++)
            {
                if (form_post_name[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (form_post_name[n] == '[')
                {

                    break;
                }
                if (form_post_name[n] != 0x22)
                {
                    key1name.push_back(form_post_name[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < form_post_name.length())
                {
                    if (form_post_name[m] == '[')
                    {
                        m += 1;
                        for (; m < form_post_name.length(); m++)
                        {
                            if (form_post_name[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (form_post_name[m] == '[')
                            {

                                break;
                            }
                            if (form_post_name[m] != 0x22)
                            {
                                key2name.push_back(form_post_name[m]);
                            }
                        }

                        if (ishaskey2 && m == form_post_name.length())
                        {
                        }
                        else
                        {
                            ishaskey2 = false;
                        }
                    }
                }

                if (ishaskey2)
                {
                    // 双数组
                    if (key1name.empty())
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40106;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->post[objname].size());
                            key1name         = std::to_string(iii);
                            steam_httppeer->post[objname][key1name].set_object();

                            iii      = steam_httppeer->post[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40107;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40108;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->post[objname].size());
                            key1name         = std::to_string(iii);
                            steam_httppeer->post[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40109;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40110;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            steam_httppeer->post[objname][key1name].set_object();

                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->post[objname][key1name].size());
                            key2name         = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {

                            if (objname.size() > 48)
                            {
                                error = 40111;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40112;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40113;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            steam_httppeer->post[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == form_post_name.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        if (objname.size() > 48)
                        {
                            error = 40114;
                            return;
                        }

                        steam_httppeer->post[objname].set_object();
                        unsigned int iii = static_cast<unsigned int>(steam_httppeer->post[objname].size());
                        key1name         = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp = form_post_value;

                        steam_httppeer->post[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40115;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40116;
                            return;
                        }
                        steam_httppeer->post[objname].set_object();

                        http::obj_val objtemp;
                        objtemp = form_post_value;

                        steam_httppeer->post[objname].push(key1name, std::move(objtemp));
                    }
                    j       = n;
                    isgroup = false;
                }
                else
                {

                    // 没有数组
                }
            }
            if (isgroup)
            {
                objname.push_back(form_post_name[j]);
            }
        }
        else
        {
            objname.push_back(form_post_name[j]);
        }
    }
    if (isgroup)
    {
        steam_httppeer->post[form_post_name] = form_post_value;
    }
}

void http2parse::multipart_post_file_field(HTTP2_POST_DATA_T &temp_post_data)
{
    std::string objname;
    bool isgroup = true;

    for (size_t j = 0; j < temp_post_data.field_name.length(); j++)
    {
        if (temp_post_data.field_name[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < temp_post_data.field_name.length(); n++)
            {
                if (temp_post_data.field_name[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (temp_post_data.field_name[n] == '[')
                {

                    break;
                }
                if (temp_post_data.field_name[n] != 0x22)
                {
                    key1name.push_back(temp_post_data.field_name[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < temp_post_data.field_name.length())
                {
                    if (temp_post_data.field_name[m] == '[')
                    {
                        m += 1;
                        for (; m < temp_post_data.field_name.length(); m++)
                        {
                            if (temp_post_data.field_name[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (temp_post_data.field_name[m] == '[')
                            {

                                break;
                            }
                            if (temp_post_data.field_name[m] != 0x22)
                            {
                                key2name.push_back(temp_post_data.field_name[m]);
                            }
                        }

                        if (ishaskey2 && m == temp_post_data.field_name.length())
                        {
                        }
                        else
                        {
                            ishaskey2 = false;
                        }
                    }
                }

                if (ishaskey2)
                {
                    // 双数组
                    if (key1name.empty())
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40117;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(temp_post_data.peer->files[objname].size());
                            key1name         = std::to_string(iii);
                            temp_post_data.peer->files[objname][key1name].set_object();

                            iii      = temp_post_data.peer->files[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data.filename;
                            objtemp["name"]     = temp_post_data.field_name;
                            objtemp["tempfile"] = temp_post_data.temp_filename;
                            objtemp["type"]     = temp_post_data.mimetype;
                            objtemp["size"]     = temp_post_data.cur_length;
                            objtemp["error"]    = 0;

                            temp_post_data.peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40118;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40119;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(temp_post_data.peer->files[objname].size());
                            key1name         = std::to_string(iii);
                            temp_post_data.peer->files[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data.filename;
                            objtemp["name"]     = temp_post_data.field_name;
                            objtemp["tempfile"] = temp_post_data.temp_filename;
                            objtemp["type"]     = temp_post_data.mimetype;
                            objtemp["size"]     = temp_post_data.cur_length;
                            objtemp["error"]    = 0;

                            temp_post_data.peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40120;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40121;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            temp_post_data.peer->files[objname][key1name].set_object();

                            unsigned iii = temp_post_data.peer->files[objname][key1name].size();
                            key2name     = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data.filename;
                            objtemp["name"]     = temp_post_data.field_name;
                            objtemp["tempfile"] = temp_post_data.temp_filename;
                            objtemp["type"]     = temp_post_data.mimetype;
                            objtemp["size"]     = temp_post_data.cur_length;
                            objtemp["error"]    = 0;

                            temp_post_data.peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {

                            if (objname.size() > 48)
                            {
                                error = 40122;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40123;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40124;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            temp_post_data.peer->files[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data.filename;
                            objtemp["name"]     = temp_post_data.field_name;
                            objtemp["tempfile"] = temp_post_data.temp_filename;
                            objtemp["type"]     = temp_post_data.mimetype;
                            objtemp["size"]     = temp_post_data.cur_length;
                            objtemp["error"]    = 0;

                            temp_post_data.peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == temp_post_data.field_name.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        if (objname.size() > 48)
                        {
                            error = 40125;
                            return;
                        }

                        temp_post_data.peer->files[objname].set_object();
                        unsigned iii = temp_post_data.peer->files[objname].size();
                        key1name     = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp.set_object();
                        objtemp["filename"] = temp_post_data.filename;
                        objtemp["name"]     = temp_post_data.field_name;
                        objtemp["tempfile"] = temp_post_data.temp_filename;
                        objtemp["type"]     = temp_post_data.mimetype;
                        objtemp["size"]     = temp_post_data.cur_length;
                        objtemp["error"]    = 0;

                        temp_post_data.peer->files[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40126;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40127;
                            return;
                        }
                        temp_post_data.peer->files[objname].set_object();

                        http::obj_val objtemp;
                        objtemp.set_object();
                        objtemp["filename"] = temp_post_data.filename;
                        objtemp["name"]     = temp_post_data.field_name;
                        objtemp["tempfile"] = temp_post_data.temp_filename;
                        objtemp["type"]     = temp_post_data.mimetype;
                        objtemp["size"]     = temp_post_data.cur_length;
                        objtemp["error"]    = 0;

                        temp_post_data.peer->files[objname].push(key1name, std::move(objtemp));
                    }
                    j       = n;
                    isgroup = false;
                }
                else
                {

                    // not array
                }
            }
            if (isgroup)
            {
                objname.push_back(temp_post_data.field_name[j]);
            }
        }
        else
        {
            objname.push_back(temp_post_data.field_name[j]);
        }
    }
    if (isgroup)
    {
        temp_post_data.peer->files[temp_post_data.field_name].set_object();
        temp_post_data.peer->files[temp_post_data.field_name]["filename"] = temp_post_data.filename;
        temp_post_data.peer->files[temp_post_data.field_name]["name"]     = temp_post_data.field_name;
        temp_post_data.peer->files[temp_post_data.field_name]["tempfile"] = temp_post_data.temp_filename;
        temp_post_data.peer->files[temp_post_data.field_name]["type"]     = temp_post_data.mimetype;
        temp_post_data.peer->files[temp_post_data.field_name]["size"]     = temp_post_data.cur_length;
        temp_post_data.peer->files[temp_post_data.field_name]["error"]    = 0;
    }
}

void http2parse::post_www_form_urlencoded(HTTP2_POST_DATA_T &temp_post_data)
{
    std::string temp_value;
    std::string buffer_key;
    std::string field_value;
    unsigned int qsize    = static_cast<unsigned int>(temp_post_data.peer->rawcontent.size());
    unsigned char partype = 0;
    unsigned int j        = 0;
    unsigned int jj       = 0;
    for (j = 0; j < qsize; j++)
    {
        if (temp_post_data.peer->rawcontent[j] == 0x3D)
        {
            buffer_key = http::url_decode(temp_value.data(), temp_value.length());
            temp_value.clear();
            partype = 1;
            jj      = 0;
            continue;
        }
        else if (temp_post_data.peer->rawcontent[j] == 0x26)
        {
            field_value = http::url_decode(temp_value.data(), temp_value.length());
            if (buffer_key.size() > 72)
            {
                error = 40196;
                return;
            }
            post_form_to_postfield(buffer_key, field_value, temp_post_data.peer);
            temp_value.clear();
            field_value.clear();
            partype = 2;
            jj      = 0;
            continue;
        }
        temp_value.push_back(temp_post_data.peer->rawcontent[j]);

        if (partype == 0 || partype == 2)
        {
            //key name too long
            if (jj > 72)
            {
                error = 40197;
                return;
            }
        }
        jj++;
    }
    if (partype == 1)
    {
        field_value = http::url_decode(temp_value.data(), temp_value.length());
        if (buffer_key.size() > 72)
        {
            error = 40198;
            return;
        }
        post_form_to_postfield(buffer_key, field_value, temp_post_data.peer);
    }
    else if (partype == 2)
    {
        buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        field_value.clear();
        if (buffer_key.size() > 72)
        {
            error = 40199;
            return;
        }
        post_form_to_postfield(buffer_key, field_value, temp_post_data.peer);
    }
    else if (temp_value.size() > 0)
    {
        buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        field_value.clear();
        if (buffer_key.size() > 72)
        {
            error = 40200;
            return;
        }
        post_form_to_postfield(buffer_key, field_value, temp_post_data.peer);
    }
}

void http2parse::post_multipart_itemcontent_append(HTTP2_POST_DATA_T &temp_post_data)
{
    //only process upload file
    if (temp_post_data.temp_filename.size() > 0)
    {
        if (temp_post_data.fp)
        {
            if (temp_post_data.field_item.size() == 0)
            {
                return;
            }
            temp_post_data.cur_length += temp_post_data.field_item.size();
            size_t n = fwrite(&temp_post_data.field_item[0], 1, temp_post_data.field_item.size(), temp_post_data.fp.get());
            if (n != temp_post_data.field_item.size())
            {
                error = 40038;
                temp_post_data.fp.reset();
                std::remove(temp_post_data.temp_filename.c_str());// 再删除孤儿文件
                temp_post_data.field_item.clear();
                return;
            }
            temp_post_data.field_item.clear();
            return;
        }
    }
}
void http2parse::post_multipart_itemcontent(HTTP2_POST_DATA_T &temp_post_data, bool isfilefull)
{
    if (temp_post_data.field_item.size() == 0)
    {
        return;
    }

    unsigned int pos_m = 0;
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == '\r' || temp_post_data.field_item[pos_m] == '\n' || temp_post_data.field_item[pos_m] == '-')
        {
            continue;
        }
        break;
    }
    //Content-Disposition
    std::string keyname_field;
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == ':')
        {
            break;
        }
        keyname_field.push_back(temp_post_data.field_item[pos_m]);
        if (keyname_field.size() > 64)
        {
            error = 40045;
            return;
        }
    }

    if (pos_m >= temp_post_data.field_item.size())
    {
        return;
    }

    if (!str_casecmp(keyname_field, "Content-Disposition"))
    {
        error = 40026;
        return;
    }

    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == ';')
        {
            pos_m++;
            break;
        }
    }

    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] != ' ')
        {
            break;
        }
    }
    keyname_field.clear();
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == '=')
        {
            pos_m++;
            break;
        }
        keyname_field.push_back(temp_post_data.field_item[pos_m]);
        if (keyname_field.size() > 72)
        {
            error = 40027;
            return;
        }
    }
    if (!str_casecmp(keyname_field, "name"))
    {
        error = 40204;
        return;
    }
    //fieldname
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == ' ' || temp_post_data.field_item[pos_m] == '"')
        {
            continue;
        }
        else if (temp_post_data.field_item[pos_m] == '=')
        {
            continue;
        }
        break;
    }

    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == '"')
        {
            pos_m++;
            break;
        }
        temp_post_data.field_name.push_back(temp_post_data.field_item[pos_m]);
        if (temp_post_data.field_name.size() > 512)
        {
            error = 40028;
            return;
        }
    }

    //filename
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] != ' ')
        {
            break;
        }
    }
    if (pos_m >= temp_post_data.field_item.size())
    {
        error = 40029;
        return;
    }

    if (temp_post_data.field_item[pos_m] == ';')
    {
        temp_post_data.isfile = true;
        pos_m++;
        keyname_field.clear();
        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] != ' ')
            {
                break;
            }
        }

        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '=')
            {
                pos_m++;
                break;
            }
            keyname_field.push_back(temp_post_data.field_item[pos_m]);
            if (keyname_field.size() > 64)
            {
                error = 40030;
                return;
            }
        }

        if (!str_casecmp(keyname_field, "filename"))
        {
            error = 40031;
            return;
        }

        if (pos_m >= temp_post_data.field_item.size())
        {
            error = 40032;
            return;
        }

        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] != ' ')
            {
                break;
            }
        }

        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '"')
            {
                pos_m++;
                break;
            }
        }

        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '"')
            {
                pos_m++;
                break;
            }
            temp_post_data.filename.push_back(temp_post_data.field_item[pos_m]);
            if (temp_post_data.filename.size() > 512)
            {
                error = 40034;
                return;
            }
        }
        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '\r')
            {
                pos_m++;
                if (pos_m < temp_post_data.field_item.size())
                {
                    if (temp_post_data.field_item[pos_m] == '\n')
                    {
                        pos_m++;
                    }
                }
                break;
            }
        }
        keyname_field.clear();
        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '\r')
            {
                pos_m++;
                if (pos_m < temp_post_data.field_item.size())
                {
                    if (temp_post_data.field_item[pos_m] == '\n')
                    {
                        pos_m++;
                    }
                }
                break;
            }
            keyname_field.push_back(temp_post_data.field_item[pos_m]);
            if (keyname_field.size() > 128)
            {
                error = 40035;
                return;
            }
        }
        if (!keyname_field.empty())
        {
            unsigned int i = 0;
            for (; i < keyname_field.size(); i++)
            {
                if (keyname_field[i] == ':')
                {
                    i++;
                    for (; i < keyname_field.size(); i++)
                    {
                        if (keyname_field[i] != ' ')
                        {
                            break;
                        }
                    }
                    break;
                }
            }
            for (; i < keyname_field.size(); i++)
            {
                temp_post_data.mimetype.push_back(keyname_field[i]);
            }
        }
    }
    else
    {
        if (temp_post_data.field_item[pos_m] == '\r')
        {
            pos_m++;
            if (pos_m < temp_post_data.field_item.size())
            {
                if (temp_post_data.field_item[pos_m] == '\n')
                {
                    pos_m++;
                }
            }
        }
    }
    keyname_field.clear();
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == '\r')
        {
            pos_m++;
            if (pos_m < temp_post_data.field_item.size())
            {
                if (temp_post_data.field_item[pos_m] == '\n')
                {
                    pos_m++;
                }
            }
            break;
        }
    }

    std::string_view keyvalue_field(&temp_post_data.field_item[pos_m], temp_post_data.field_item.size() - pos_m);
    if (temp_post_data.isfile)
    {
        server_loaclvar &localvar    = get_server_global_var();
        temp_post_data.temp_filename = localvar.temp_path;
        if (temp_post_data.temp_filename.size() > 0 && temp_post_data.temp_filename.back() != '/')
        {
            temp_post_data.temp_filename.push_back('/');
        }

        temp_post_data.temp_filename = temp_post_data.temp_filename + http::make_http_temp_upload_name(temp_post_data.peer->content_length);
        std::unique_ptr<std::FILE, int (*)(FILE *)> fpa(std::fopen(temp_post_data.temp_filename.c_str(), "wb"), std::fclose);
        if (fpa)
        {
            temp_post_data.fp         = std::move(fpa);
            temp_post_data.cur_length = keyvalue_field.size();
            size_t n                  = fwrite(&keyvalue_field[0], 1, keyvalue_field.size(), temp_post_data.fp.get());
            if (n != keyvalue_field.size())
            {
                error = 40036;
                temp_post_data.fp.reset();
                std::remove(temp_post_data.temp_filename.c_str());// 再删除孤儿文件
                return;
            }
        }
        if (isfilefull)
        {
            multipart_post_file_field(temp_post_data);
            temp_post_data.isfile = false;
            temp_post_data.temp_filename.clear();
            temp_post_data.field_name.clear();
            temp_post_data.filename.clear();
            temp_post_data.fp.reset();
        }
    }
    else
    {
        post_form_to_postfield(temp_post_data.field_name, keyvalue_field, temp_post_data.peer);
    }
}
void http2parse::reset_uploadfile(HTTP2_POST_DATA_T &temp_post_data)
{
    if (temp_post_data.temp_filename.size() > 0)
    {
        if (temp_post_data.fp)
        {
            post_multipart_itemcontent_append(temp_post_data);
            multipart_post_file_field(temp_post_data);
        }
    }
    temp_post_data.isfile = false;
    temp_post_data.temp_filename.clear();
    temp_post_data.field_name.clear();
    temp_post_data.filename.clear();
    temp_post_data.fp.reset();
}
void http2parse::post_multipart_formdata(HTTP2_POST_DATA_T &temp_post_data, [[maybe_unused]] unsigned char islast_pack)
{
    unsigned int pos_m = temp_post_data.field_offset;

    //处理上次剩余的
    if (temp_post_data.pre_content.size() > 0)
    {
        if (temp_post_data.pre_content.size() == 1)
        {
            bool isnotmatch = true;
            if (temp_post_data.content[pos_m] == '-')
            {
                //说明已经命中两个，接着直接匹配boundary
                pos_m++;
                if ((pos_m + temp_post_data.boundary.size()) > temp_post_data.content.size())
                {
                    //两次数量小于boundary.size()
                    temp_post_data.field_offset = temp_post_data.content.size();
                    return;
                }
                unsigned int i = 0;
                for (; i < temp_post_data.boundary.size(); i++)
                {
                    if (temp_post_data.content[pos_m] != temp_post_data.boundary[i])
                    {
                        pos_m = temp_post_data.field_offset;
                        break;
                    }
                    pos_m++;
                }
                if (i == temp_post_data.boundary.size())
                {
                    isnotmatch = false;
                }
            }
            //说明是假匹配
            temp_post_data.pre_content.clear();
            if (isnotmatch)
            {
                temp_post_data.field_item.push_back('-');
                //重新开始当作没有发生过
                pos_m = temp_post_data.field_offset;
            }
            else
            {
                temp_post_data.field_offset = pos_m;
                if (temp_post_data.field_item.size() > 1)
                {
                    temp_post_data.field_item.resize(temp_post_data.field_item.size() - 2);
                }
                reset_uploadfile(temp_post_data);
                post_multipart_itemcontent(temp_post_data, true);
                temp_post_data.field_item.clear();
                if ((pos_m + 1) < temp_post_data.content.size())
                {
                    if (temp_post_data.content[pos_m] == '-')
                    {
                        if (temp_post_data.content[pos_m + 1] == '-')
                        {
                            temp_post_data.peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data.content[pos_m] == '\r')
                    {
                        if (temp_post_data.content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                }
                return;
            }
        }
        else if (temp_post_data.pre_content.size() == 2)
        {
            bool isnotmatch = true;
            if ((pos_m + temp_post_data.boundary.size()) > temp_post_data.content.size())
            {
                //两次数量小于boundary.size()
                temp_post_data.field_offset = temp_post_data.content.size();
                return;
            }
            unsigned int i = 0;
            for (; i < temp_post_data.boundary.size(); i++)
            {
                if (temp_post_data.content[pos_m] != temp_post_data.boundary[i])
                {
                    pos_m = temp_post_data.field_offset;
                    break;
                }
                pos_m++;
            }
            if (i == temp_post_data.boundary.size())
            {
                isnotmatch = false;
            }

            //说明是假匹配
            temp_post_data.pre_content.clear();
            if (isnotmatch)
            {
                //需要恢复2个
                temp_post_data.field_item.push_back('-');
                temp_post_data.field_item.push_back('-');
                //重新开始当作没有发生过
                pos_m = temp_post_data.field_offset;
            }
            else
            {
                temp_post_data.field_offset = pos_m;
                if (temp_post_data.field_item.size() > 1)
                {
                    temp_post_data.field_item.resize(temp_post_data.field_item.size() - 2);
                }
                reset_uploadfile(temp_post_data);
                post_multipart_itemcontent(temp_post_data, true);
                temp_post_data.field_item.clear();
                if ((pos_m + 1) < temp_post_data.content.size())
                {
                    if (temp_post_data.content[pos_m] == '-')
                    {
                        if (temp_post_data.content[pos_m + 1] == '-')
                        {
                            temp_post_data.peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data.content[pos_m] == '\r')
                    {
                        if (temp_post_data.content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                }
                return;
            }
        }
        else
        {
            //3个字符，进入boundary 匹配
            bool isnotmatch = true;

            if ((pos_m + temp_post_data.boundary.size()) > temp_post_data.content.size())
            {
                temp_post_data.field_offset = temp_post_data.content.size();
                return;
            }
            unsigned int i = static_cast<unsigned int>(temp_post_data.pre_content.size() - 2);
            for (; i < temp_post_data.boundary.size(); i++)
            {
                if (temp_post_data.content[pos_m] != temp_post_data.boundary[i])
                {
                    pos_m = temp_post_data.field_offset;
                    break;
                }
                pos_m++;
            }
            if (i == temp_post_data.boundary.size())
            {
                isnotmatch = false;
            }

            //说明是假匹配
            if (isnotmatch)
            {
                //恢复所有temp_post_data.pre_content
                temp_post_data.field_item.append(temp_post_data.pre_content);
                //重新开始当作没有发生过
                pos_m = temp_post_data.field_offset;
                temp_post_data.pre_content.clear();
            }
            else
            {
                temp_post_data.pre_content.clear();
                temp_post_data.field_offset = pos_m;

                if (temp_post_data.field_item.size() > 1)
                {
                    temp_post_data.field_item.resize(temp_post_data.field_item.size() - 2);
                }
                reset_uploadfile(temp_post_data);
                post_multipart_itemcontent(temp_post_data, true);
                temp_post_data.field_item.clear();
                if ((pos_m + 1) < temp_post_data.content.size())
                {
                    if (temp_post_data.content[pos_m] == '-')
                    {
                        if (temp_post_data.content[pos_m + 1] == '-')
                        {
                            temp_post_data.peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data.content[pos_m] == '\r')
                    {
                        if (temp_post_data.content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                }
                return;
            }
        }
    }

    temp_post_data.pre_content.clear();
    for (; pos_m < temp_post_data.content.size(); pos_m++)
    {
        if (temp_post_data.content[pos_m] == '-')
        {
            unsigned int j = pos_m;
            j++;
            if (j < temp_post_data.content.size())
            {
                if (temp_post_data.content[j] == '-')
                {
                    j++;
                    if (j < temp_post_data.content.size())
                    {
                        //主要是检查是否匹配或最后
                        unsigned int i = 0;
                        for (; i < temp_post_data.boundary.size();)
                        {
                            if (temp_post_data.content[j] != temp_post_data.boundary[i])
                            {
                                break;
                            }
                            j++;
                            i++;
                            if (j >= temp_post_data.content.size())
                            {
                                break;
                            }
                        }
                        if (i == temp_post_data.boundary.size())
                        {
                            temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);

                            if (temp_post_data.field_item.size() > 1)
                            {
                                temp_post_data.field_item.resize(temp_post_data.field_item.size() - 2);
                            }
                            reset_uploadfile(temp_post_data);
                            post_multipart_itemcontent(temp_post_data, true);
                            temp_post_data.field_offset = j;
                            temp_post_data.field_item.clear();

                            if (j + 1 < temp_post_data.content.size())
                            {
                                if (temp_post_data.content[j] == '-' && temp_post_data.content[j + 1] == '-')
                                {
                                    temp_post_data.field_offset   = j + 1;
                                    temp_post_data.peer->isfinish = true;
                                }
                                else if (temp_post_data.content[j] == '\r' && temp_post_data.content[j + 1] == '\n')
                                {
                                    temp_post_data.field_offset = j + 1;
                                }
                            }

                            return;
                        }
                        if (j >= temp_post_data.content.size())
                        {
                            //例外已经到头了
                            //让进入下一轮
                            temp_post_data.pre_content.append(&temp_post_data.content[pos_m], temp_post_data.content.size() - pos_m);
                            temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);
                            temp_post_data.field_offset = temp_post_data.content.size();
                            return;
                        }
                    }
                    else
                    {
                        temp_post_data.pre_content.push_back('-');
                        temp_post_data.pre_content.push_back('-');
                        temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);
                        temp_post_data.field_offset = temp_post_data.content.size();
                        return;
                    }
                }
            }
            else
            {
                temp_post_data.pre_content.push_back('-');
                temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);
                temp_post_data.field_offset = temp_post_data.content.size();
                return;
            }
        }
    }
    temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);
    temp_post_data.field_offset = pos_m;
}

void http2parse::post_data_process(HTTP2_POST_DATA_T &temp_post_data, unsigned char islast_pack)
{
#ifdef ENABLE_FASTCGI
    if (temp_post_data.peer->compress == 10)
    {
        //ready output to php
        if (temp_post_data.peer->content_length < CONST_PHP_BODY_POST_SIZE)
        {
            if (temp_post_data.peer->output.size() < temp_post_data.peer->content_length)
            {
                temp_post_data.peer->output.append(temp_post_data.content);
            }
        }
        else
        {
            error = 40172;
            return;
        }
    }
    else
#endif
    {
        if (temp_post_data.posttype == 0)
        {
            if (temp_post_data.content.size() > 0)
            {
                char first = temp_post_data.content[0];
                if (first == '-')
                {
                    temp_post_data.posttype = 2;// multipart
                }
                else if (first == '{' || first == '[')
                {
                    temp_post_data.posttype = 3;// json
                }
                else if (first == '<')
                {
                    temp_post_data.posttype = 4;// xml
                }
                else
                {
                    temp_post_data.posttype = 5;// fallback: raw
                }
            }
        }

        switch (temp_post_data.posttype)
        {
        case 1:
            // x-www-form-urlencoded
            temp_post_data.peer->rawcontent.append(temp_post_data.content);
            if (temp_post_data.peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
            {
                error = 40166;
                return;
            }
            if (islast_pack)
            {
                post_www_form_urlencoded(temp_post_data);
            }
            if (error > 0)
            {
                return;
            }
            break;
        case 2:
            // multipart/form-data
            temp_post_data.field_offset = 0;
            for (; temp_post_data.field_offset < temp_post_data.content.size();)
            {
                post_multipart_formdata(temp_post_data, islast_pack);
            }

            if (islast_pack)
            {
                reset_uploadfile(temp_post_data);
            }
            else
            {
                if (temp_post_data.isfile && temp_post_data.field_item.size() > 2097152)
                {
                    //及时消化掉大文件中间内容，尽快保存到文件
                    post_multipart_itemcontent_append(temp_post_data);
                    temp_post_data.field_item.clear();
                }
                else if (temp_post_data.field_item.size() > 2097152)
                {
                    if (!temp_post_data.isfile)
                    {

                        std::string_view check_filename = std::string_view(temp_post_data.field_item.data(), 80);
                        size_t pos                      = check_filename.find("filename");
                        if (pos != std::string::npos)
                        {
                            reset_uploadfile(temp_post_data);
                            post_multipart_itemcontent(temp_post_data, false);
                            temp_post_data.field_item.clear();
                        }
                    }
                }
            }

            if (error > 0)
            {
                return;
            }
            break;
        case 3:
            // json
            temp_post_data.peer->rawcontent.append(temp_post_data.content);
            if (temp_post_data.peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
            {
                error = 40170;
                return;
            }
            if (islast_pack)
            {
                temp_post_data.peer->json.from_json(temp_post_data.peer->rawcontent);
            }
            break;
        case 4:
            //xml
            temp_post_data.peer->rawcontent.append(temp_post_data.content);
            if (temp_post_data.peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
            {
                error = 40171;
                return;
            }
            break;
        case 5:
            // octet-stream
            readrawfileformdata(temp_post_data, islast_pack);
            break;
        }
    }
}
void http2parse::readpostdata(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    if ((temp_pack_data.flags & 0xF6) != 0)
    {
        //非数据部分
        error = 40042;
        return;
    }

    unsigned char last_pack    = temp_pack_data.flags & HTTP2_DATA_END_STREAM;
    unsigned int header_offset = 0, padded_length = 0;
    if ((temp_pack_data.flags & HTTP2_DATA_PADDED) > 0)
    {
        if (temp_pack_data.payload.size() == 0)
        {
            error = 40041;
            return;
        }
        padded_length = temp_pack_data.payload[header_offset];
        header_offset++;
    }

    unsigned int new_size_num = header_offset + padded_length;
    auto iter                 = http_data.find(temp_pack_data.stream_id);
    if (iter == http_data.end())
    {
        //如果没有头部信息，那么是错误的协议
        error = 40025;
        return;
    }

    auto post_iter = http_post_data.find(temp_pack_data.stream_id);
    if (post_iter == http_post_data.end())
    {
        HTTP2_POST_DATA_T temp_p;
        temp_p.peer = iter->second;

        temp_p.stream_id = temp_pack_data.stream_id;
        http_post_data.insert_or_assign(temp_pack_data.stream_id, std::move(temp_p));
        post_iter = http_post_data.find(temp_pack_data.stream_id);
        if (post_iter == http_post_data.end())
        {
            error = 40024;
            return;
        }
    }

    // DATA 帧的 padding（1 字节 pad length + N 字节填充）要占 flow-control 额度，
    // 但不属于正文。若用整个 payload.size() 累加 exp_length，
    // 客户端一旦使用 PADDED，exp_length 必然大于 content-length，收尾时误报 40202。
    unsigned int content_bytes = static_cast<unsigned int>(temp_pack_data.payload.size());
    if (new_size_num > 0)
    {
        if (new_size_num >= temp_pack_data.payload.size())
        {
            error = 40206;
            return;
        }
        new_size_num = temp_pack_data.payload.size() - new_size_num;
        if (header_offset > temp_pack_data.payload.size() || new_size_num > (temp_pack_data.payload.size() - header_offset))
        {
            error = 40023;
            return;
        }
        content_bytes             = new_size_num;
        post_iter->second.content = std::string_view(temp_pack_data.payload.data() + header_offset, new_size_num);
    }
    else
    {
        post_iter->second.content = std::string_view(temp_pack_data.payload.data(), temp_pack_data.payload.size());
    }

    post_iter->second.exp_length += content_bytes;
    if (post_iter->second.exp_length > CONST_HTTP_BODY_POST_SIZE)
    {
        error = 40009;
        return;
    }

    // RFC 9113 §6.9.1：DATA 帧的整个 payload 都计入流控（含 Pad Length 与 Padding），
    // temp_pack_data.length 正是这个口径。连接级与流级必须各自独立扣减、各自独立补量，
    // 不能用「全局计数越线」去决定该补给哪一条流。
    unsigned int fc_bytes = temp_pack_data.length;

    if (conn_recv_window_num < fc_bytes)
    {
        conn_recv_window_num = 0;
    }
    else
    {
        conn_recv_window_num -= fc_bytes;
    }

    auto s_win_iter = stream_recv_window.find(temp_pack_data.stream_id);
    if (s_win_iter == stream_recv_window.end())
    {
        s_win_iter = stream_recv_window.emplace(temp_pack_data.stream_id, CONST_HTTP2_LOCAL_INITIAL_WINDOW).first;
    }
    if (s_win_iter->second < fc_bytes)
    {
        s_win_iter->second = 0;
    }
    else
    {
        s_win_iter->second -= fc_bytes;
    }

    if (conn_recv_window_num < CONST_HTTP2_WINDOW_UPDATE_THRESHOLD)
    {
        // 只补连接级：增量正好等于本端已消费掉的量
        peer_session->send_window_update_conn(CONST_HTTP2_LOCAL_INITIAL_WINDOW - conn_recv_window_num);
        need_wakeup_send_threads = true;
        conn_recv_window_num     = CONST_HTTP2_LOCAL_INITIAL_WINDOW;
    }

    if (s_win_iter->second < CONST_HTTP2_WINDOW_UPDATE_THRESHOLD)
    {
        // 只补当前这条流，不要波及别的流
        peer_session->send_window_update_stream(temp_pack_data.stream_id,
                                                CONST_HTTP2_LOCAL_INITIAL_WINDOW - s_win_iter->second);
        need_wakeup_send_threads = true;
        s_win_iter->second       = CONST_HTTP2_LOCAL_INITIAL_WINDOW;
    }

    bool is_last_pack = last_pack;

    post_data_process(post_iter->second, is_last_pack);

    if (is_last_pack)
    {
        if (post_iter->second.exp_length != iter->second->content_length)
        {
            error = 40202;
            return;
        }

        http_post_data.erase(post_iter);
        // 流已结束，释放该流的窗口记账，避免 map 随请求数无限增长
        stream_recv_window.erase(temp_pack_data.stream_id);
        iter->second->isfinish = true;
        stream_list.emplace(temp_pack_data.stream_id);
    }
}

void http2parse::process_pack()
{
    switch (pack_data.frame_type)
    {
    case 0x00:
        // DATA
        readpostdata(pack_data);
        break;
    case 0x01:
        // HEADERS
        readheaders(pack_data);
        break;
    case 0x02:
        // PRIORITY
        readpriority(pack_data);
        break;
    case 0x03:
        // RST_STREAM
        readrst_stream(pack_data);
        break;
    case 0x04:
        // SETTINGS
        readsetting(pack_data);
        break;
    case 0x05:
        error = 40016;
        return;
        break;
    case 0x06:
        // PING
        readping(pack_data);
        break;
    case 0x07:
        // GOAWAY
        readgoaway(pack_data);
        break;
    case 0x08:
        // WINDOW_UPDATE
        readwinupdate(pack_data);

        break;
    case 0x09:
        // CONTINUATION
        readheaders(pack_data);
        break;
    default:
        error = 40169;
    }
}

void http2parse::read_pack_data(const unsigned char *buffer, unsigned int buffersize)
{
    if (pack_data.length == 0)
    {
        // 帧头不足 9 字节：把「还缺的字节」攒进 subpad。
        // 不能把本包剩余字节无条件塞进 subpad 而不看它已有多少：
        // 客户端按 1 字节/包发送时 subpad 会无上限增长；一旦 subpad > 9，
        // 下面只解析前 9 字节、多出来的字节被静默丢弃，后续帧头/载荷整体错位。
        if ((readoffset + 9) > buffersize && pack_data.subpad.size() < 9)
        {
            unsigned int need = static_cast<unsigned int>(9 - pack_data.subpad.size());
            for (; readoffset < buffersize && need > 0; readoffset++, need--)
            {
                pack_data.subpad.push_back(buffer[readoffset]);
            }
            if (pack_data.subpad.size() < 9)
            {
                // 帧头还没凑齐，等下一次读
                return;
            }
        }

        unsigned int j = 0;

        if (pack_data.subpad.size() > 0)
        {
            j = pack_data.subpad.size();
            for (; j < 9; j++)
            {
                pack_data.subpad.push_back(buffer[readoffset]);
                readoffset++;
                // j 已经走到 8 说明第 9 个字节刚补齐，帧头是完整的，不能报错
                // （只要 readoffset 触底就报 40005，会在 TCP 恰好切在帧头最后一个字节时误杀连接）
                if (j < 8 && readoffset >= buffersize)
                {
                    error = 40005;
                    return;
                }
            }
            j                = 0;
            pack_data.length = (unsigned char)pack_data.subpad[j++];
            pack_data.length = (pack_data.length << 8) | (unsigned char)pack_data.subpad[j++];
            pack_data.length = (pack_data.length << 8) | (unsigned char)pack_data.subpad[j++];

            pack_data.frame_type = pack_data.subpad[j++];
            pack_data.flags      = pack_data.subpad[j++];

            if ((pack_data.subpad[j] & 0x7F) > 0)
            {
                error = 40002;
                return;
            }

            pack_data.stream_id = pack_data.subpad[j++] & 0x7F;
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)pack_data.subpad[j++];
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)pack_data.subpad[j++];
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)pack_data.subpad[j++];
            j                   = readoffset;
        }
        else
        {
            j = readoffset;
            if ((j + 5) > buffersize)
            {
                error = 40003;
                return;
            }

            pack_data.length = buffer[j++];
            pack_data.length = (pack_data.length << 8) | (unsigned char)buffer[j++];
            pack_data.length = (pack_data.length << 8) | (unsigned char)buffer[j++];

            pack_data.frame_type = buffer[j++];
            pack_data.flags      = buffer[j++];

            if (j < buffersize && (buffer[j] & 0x7F) > 0)
            {
                error = 40208;
                return;
            }

            if ((j + 4) > buffersize)
            {
                error = 40004;
                return;
            }

            pack_data.stream_id = buffer[j++] & 0x7F;
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)buffer[j++];
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)buffer[j++];
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)buffer[j++];
        }

        // 非 DATA 帧按帧头声明的 length 提前拒绝：本函数末尾那个 16K 检查是在
        // DATA 帧不在这里卡（它由 CONST_HTTP_BODY_POST_SIZE 与 flow-control 窗口限制）。
        if (pack_data.frame_type != 0x00 && pack_data.length > CONST_HTTP_HEADER_BODY_SIZE)
        {
            error      = 40001;
            readoffset = buffersize;
            return;
        }

        for (; j < buffersize; j++)
        {
            if (pack_data.payload.size() < pack_data.length)
            {
                pack_data.payload.push_back(buffer[j]);
            }
            else
            {
                break;
            }
        }
        readoffset = j;
    }
    else
    {
        unsigned int j = readoffset;
        for (; j < buffersize; j++)
        {
            if (pack_data.payload.size() < pack_data.length)
            {
                pack_data.payload.push_back(buffer[j]);
            }
            else
            {
                break;
            }
        }
        readoffset = j;
    }

    if (pack_data.frame_type != 0x00 && pack_data.payload.size() > CONST_HTTP_HEADER_BODY_SIZE)
    {
        error      = 40001;
        readoffset = buffersize;
        return;
    }

    if (pack_data.payload.size() >= pack_data.length)
    {
        //每次收完整的包
        process_pack();
        pack_data.length     = 0;
        pack_data.frame_type = 0;
        pack_data.flags      = 0;
        pack_data.stream_id  = 0;
        pack_data.payload.clear();
        pack_data.subpad.clear();
    }
}

void http2parse::process(const unsigned char *buffer, unsigned int buffersize)
{
    if (error > 0)
    {
        return;
    }
    readoffset = 0;
    for (; readoffset < buffersize;)
    {
        read_pack_data(buffer, buffersize);
        if (error > 0)
        {
            break;
        }
    }
}
}// namespace http
