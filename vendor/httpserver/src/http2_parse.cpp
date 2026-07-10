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

    iter->second.flags.END_STREAM = (temp_pack_data.flags & HTTP2_HEADER_END_STREAM) ? 1 : 0;
    ;
    iter->second.flags.END_HEADERS = (temp_pack_data.flags & HTTP2_HEADER_END_HEADERS) ? 1 : 0;
    iter->second.flags.PRIORITY    = (temp_pack_data.flags & HTTP2_HEADER_PRIORITY) ? 1 : 0;
    iter->second.flags.PADDED      = (temp_pack_data.flags & HTTP2_HEADER_PADDED) ? 1 : 0;

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
    http_data.emplace(iter->second.stream_id, steam_httppeer);
    steam_httppeer->content_length = 0;
    steam_httppeer->stream_id      = iter->second.stream_id;
    steam_httppeer->httpv          = 2;
    steam_httppeer->socket_session = peer_session;

    headers_parse(iter->second, steam_httppeer);

    if (ispost)
    {
        error = steam_httppeer->check_upload_limit();
    }
    steam_httppeer->isuse_fastcgi();
}
void http2parse::headers_parse(const HTTP2_HEADER_FRAME_T &header_block_obj, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int header_stream_id = steam_httppeer->stream_id;
    for (unsigned int h_begin = 0; h_begin < header_block_obj.content.size(); h_begin++)
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
            error = 40014;
            return;
        }

        if (error > 0)
        {
            return;
        }
    }

    if (steam_httppeer->method == 2 || steam_httppeer->method == 4)
    {
        DEBUG_LOG("http2 post client: %s %ud", steam_httppeer->url.c_str(), header_stream_id);
        window_update_recv_num = RECV_WINDOW_UPDATE_NUM;
        peer_session->send_window_update(window_update_recv_num, header_stream_id);
        need_wakeup_send_data = true;

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
    unsigned int i = 0, linesize = header_value.size();
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
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3B)
        {
            buffer_value = http::url_decode(buffer_value.data(), buffer_value.length());
            if (buffer_key.size() > 48)
            {
                error = 4140;
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
            error = 4140;
            return;
        }
        steam_httppeer->cookie[buffer_key] = buffer_value;
    }
    else
    {
        if (buffer_key.size() > 48)
        {
            error = 4140;
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
    unsigned int ioffset = 0, linesize = header_value.size();
    steam_httppeer->url.clear();
    unsigned int p_begin = ioffset;
    for (; ioffset < linesize; ioffset++)
    {
        if (header_value[ioffset] == 0x3F)
        {
            headerstep = 6;
            break;
        }
        if (header_value[ioffset] == 0x2F)
        {
            if (buffer_key.size() > 255)
            {
                error = 40007;
                return;
            }

            if (buffer_key.size() > 0)
            {
                if (buffer_key.size() == 2 && buffer_key[0] == '.' &&
                    buffer_key[1] == '.')
                {
                    if (steam_httppeer->pathinfos.size() > 0)
                    {
                        steam_httppeer->pathinfos.pop_back();
                    }
                }
                else if (buffer_key.size() == 1 && buffer_key[0] == '.')
                {
                }
                else
                {
                    steam_httppeer->pathinfos.emplace_back(
                        http::url_decode(buffer_key.data(), buffer_key.length()));
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
            error = 40007;
            return;
        }
        if (buffer_key.size() == 2 && buffer_key[0] == '.' &&
            buffer_key[1] == '.')
        {
            if (steam_httppeer->pathinfos.size() > 0)
            {
                steam_httppeer->pathinfos.pop_back();
            }
        }
        else if (buffer_key.size() == 1 && buffer_key[0] == '.')
        {
        }
        else
        {
            steam_httppeer->pathinfos.emplace_back(
                http::url_decode(buffer_key.data(),
                                 buffer_key.length()));
        }
    }

    unsigned int p_pos_offset         = ioffset - p_begin;
    steam_httppeer->header["urlpath"] = header_value.substr(p_begin, p_pos_offset);
    if (steam_httppeer->pathinfos.size() > 0)
    {
        steam_httppeer->urlpath.clear();
        if (p_pos_offset > 64 && p_pos_offset < 10000)
        {
            steam_httppeer->urlpath.reserve(p_pos_offset);
        }

        for (unsigned int nn = 0; nn < steam_httppeer->pathinfos.size(); nn++)
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
                    error = 155;
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
                    error = 159;
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
                error = 156;
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
                error = 157;
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
                error = 157;
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

    for (unsigned int j = 0; j < buffer_key.length(); j++)
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
    for (unsigned int j = 0; j < buffer_key.length(); j++)
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
                                error = 400000;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            unsigned int iii = steam_httppeer->get[objname].size();
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
                                error = 400000;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 400000;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            unsigned int iii = steam_httppeer->get[objname].size();
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
                                error = 400000;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 400000;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            steam_httppeer->get[objname][key1name].set_object();
                            unsigned int iii = steam_httppeer->get[objname][key1name].size();
                            key2name         = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 400000;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 400000;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 400000;
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
                            error = 400000;
                            return;
                        }

                        steam_httppeer->get[objname].set_object();
                        unsigned int iii = steam_httppeer->get[objname].size();
                        key1name         = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp = buffer_value;

                        steam_httppeer->get[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 400000;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 400000;
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
    unsigned int j = 0, linesize = header_value.size();
    std::string buffer_value;
    for (; j < linesize; j++)
    {
        if (header_value[j] == 0x20)
        {
            continue;
        }
        if (header_value[j] == 0x3D)
        {
            j++;
            break;
        }
        buffer_value.push_back(header_value[j]);
    }

    if (str_casecmp(buffer_value, "bytes"))
    {
        steam_httppeer->state.rangebytes = true;
    }
    steam_httppeer->state.rangebegin = 0;
    steam_httppeer->state.rangeend   = 0;
    buffer_value.clear();
    // bool ismuilt = false;
    for (; j < linesize; j++)
    {
        if (header_value[j] == 0x2C)
        {
            j++;
            // ismuilt = true;
            break;
        }
        if (header_value[j] == 0x2D)
        {

            long long tm = 0;
            for (unsigned int qi = 0; qi < buffer_value.size(); qi++)
            {
                if (buffer_value[qi] < 0x3A &&
                    buffer_value[qi] > 0x2F)
                {
                    tm = tm * 10 + (buffer_value[qi] - 0x30);
                }
            }
            steam_httppeer->state.rangebegin = tm;
            buffer_value.clear();
            continue;
        }
        buffer_value.push_back(header_value[j]);
    }

    if (buffer_value.size() > 0)
    {
        long long tm = 0;
        for (unsigned int qi = 0; qi < buffer_value.size(); qi++)
        {
            if (buffer_value[qi] < 0x3A && buffer_value[qi] > 0x2F)
            {
                tm = tm * 10 + (buffer_value[qi] - 0x30);
            }
        }
        steam_httppeer->state.rangeend = tm;
    }
}

bool http2parse::header_host_process(const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    bool ishasport = false;
    steam_httppeer->host.clear();
    unsigned char i = 0;
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
                error = 4140;
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
                    error = 4140;
                    return false;
                }
            }
        }
        else
        {
            error = 4140;
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
            }
        }
    }
    // state.port = port;
    steam_httppeer->state.port = port_temp;
    return true;
}

void http2parse::header_process(const std::string &header_name, const std::string &header_value, int table_num, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("header:%s:%s|%d", header_name.c_str(), header_value.c_str(), table_num);
    if (table_num > 0)
    {

        switch (table_num)
        {
        case 1:
            if (header_value.size() > 72)
            {
                error = 4140;
                return;
            }
            if (header_host_process(header_value, steam_httppeer))
            {
                steam_httppeer->header["host"]       = header_value;
                steam_httppeer->header[":authority"] = header_value;
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
            steam_httppeer->header["method"] = header_value;
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
            steam_httppeer->header["method"] = header_value;
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
            steam_httppeer->header["Access-Control-Allow-Origin"] = header_value;
            break;
        case 23:
            steam_httppeer->header["Authorization"] = header_value;
            break;
        case 28:
            steam_httppeer->content_length           = str2int(&header_value[0], header_value.size());
            steam_httppeer->header["Content-Length"] = header_value;
            break;
        case 31:
            getcontenttype(header_name, header_value, steam_httppeer);
            break;
        case 32:
            cookie_process(header_name, header_value, steam_httppeer);
            break;
        case 38:
            if (header_host_process(header_value, steam_httppeer))
            {
                steam_httppeer->header["Host"] = header_value;
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
            steam_httppeer->header["Proxy-Authenticate"] = header_value;
            break;
        case 49:
            steam_httppeer->header["Proxy-Authorization"] = header_value;
            break;
        case 50:
            range_process(header_name, header_value, steam_httppeer);
            break;
        case 51:
            steam_httppeer->header["Referer"] = header_value;
            break;
        case 58:
            steam_httppeer->header["User-Agent"] = header_value;
            break;
        case 61:
            steam_httppeer->header["WWW-Authenticate"] = header_value;
            break;
        }
    }
    else
    {
        if (header_name.size() > 48)
        {
            error = 4140;
            return;
        }
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
            steam_httppeer->header[header_name] = header_value;
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
                    steam_httppeer->header[header_name] = header_value;
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
                        steam_httppeer->header[header_name] = header_value;
                    }
                }
                break;
            default:
                if (header_name[0] != ':')
                {
                    steam_httppeer->header[header_name] = header_value;
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
                steam_httppeer->header["method"]  = header_value;
                steam_httppeer->header[":method"] = header_value;
            }
            else
            {
                steam_httppeer->header[header_name] = header_value;
            }
            break;
        case 10:
            if (str_casecmp(header_name, ":authority"))
            {
                if (header_value.size() > 72)
                {
                    error = 4140;
                    return;
                }

                if (header_host_process(header_value, steam_httppeer))
                {
                    steam_httppeer->header["Host"]       = header_value;
                    steam_httppeer->header[":authority"] = header_value;
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
                steam_httppeer->header["User-Agent"] = header_value;
            }
            else
            {
                steam_httppeer->header[header_name] = header_value;
            }
            break;
        case 12:
            if (str_casecmp(header_name, "Content-Type"))
            {
                getcontenttype(header_name, header_value, steam_httppeer);
            }
            else
            {
                steam_httppeer->header[header_name] = header_value;
            }
            break;
        case 13:
            if (str_casecmp(header_name, "if-none-match"))
            {
                getifnonematch(header_name, header_value, steam_httppeer);
            }
            else
            {
                steam_httppeer->header[header_name] = header_value;
            }
            break;

        case 14:
            if (str_casecmp(header_name, "Content-Length"))
            {
                steam_httppeer->content_length           = str2int(&header_value[0], header_value.size());
                steam_httppeer->header["Content-Length"] = header_value;
            }
            else
            {
                steam_httppeer->header[header_name] = header_value;
            }
            break;
        case 15:
            if (header_name[7] == 'e')
            {
                if (str_casecmp(header_name, "accept-encoding"))
                {
                    getacceptencoding(header_name, header_value, steam_httppeer);
                    break;
                }
                else
                {
                    steam_httppeer->header[header_name] = header_value;
                }
            }
            else if (header_name[7] == 'L')
            {
                if (str_casecmp(header_name, "Accept-Language"))
                {
                    getacceptlanguage(header_name, header_value, steam_httppeer);
                    break;
                }
                else
                {
                    steam_httppeer->header[header_name] = header_value;
                }
            }
            else
            {
                steam_httppeer->header[header_name] = header_value;
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
            std::string key;
            key.resize(header_name.size());
            std::transform(header_name.begin(), header_name.end(), key.begin(), [](unsigned char c)
                           { return std::tolower(c); });
            steam_httppeer->header[key] = header_value;
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
    unsigned int i = 0, linesize = header_value.size();
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
    unsigned int i = 0, linesize = header_value.size();
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
                iter->second.boundary = buffer_value;
                statetemp             = 0;
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
            iter->second.boundary = buffer_value;
        }
        else
        {
            callposttype(buffer_value, steam_httppeer, iter->second);
        }
    }
}
void http2parse::getaccept([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i = 0, linesize = header_value.size();
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
        for (auto hinfo : dynamic_lists)
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
            error = 143;
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
                error = 143;
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
                error = 143;
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
            error = 121;
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
            error = 143;
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
                error = 143;
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
                error = 143;
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
            error = 122;
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
                    error = 143;
                    return;
                }

                a = a + (header_data[begin] & 0x7F);
                if (header_data[begin] & 0x80)
                {
                    error = 125;
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
                for (auto hinfo : dynamic_lists)
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
            error = 143;
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
                error = 143;
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
                error = 143;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 126;
                    return;
                }
                begin += 1;
            }
        }
        if ((begin + item_length) > header_data.size())
        {
            error = 123;
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
            error = 143;
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
                error = 143;
                return;
            }
            if (header_data[begin] & 0x80)
            {
                error = 135;
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
            error = 131;
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
            error = 143;
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
                error = 143;
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
                error = 143;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 136;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 132;
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
                error = 143;
                return;
            }
            a += header_data[begin];
            if (header_data[begin] & 0x80)
            {
                error = 135;
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
                for (auto hinfo : dynamic_lists)
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
            error = 143;
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
                error = 143;
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
                error = 143;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 136;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 133;
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
            error = 143;
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
                error = 143;
                return;
            }

            if (header_data[begin] & 0x80)
            {
                error = 145;
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
            error = 141;
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
            error = 143;
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
                error = 143;
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
                error = 143;
                return;
            }
            if (iscontinue)
            {
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 146;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 142;
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
                error = 143;
                return;
            }
            a = a + (header_data[begin] & 0x7F);
            if (header_data[begin] & 0x80)
            {
                error = 145;
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
            for (auto hinfo : dynamic_lists)
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
            error = 143;
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
                error = 143;
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
                    error = 143;
                    return;
                }
                item_length = item_length + (unsigned int)(header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 146;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > header_data.size())
        {
            error = 143;
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
    peer_session->window_update_num = 65535;

    for (unsigned int n = 0; n < temp_pack_data.payload.size(); n += 6)
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
            setting_data.enable_push = ident_value;
            break;

        case HTTP2_SETTINGS_MAX_CONCURRENT_STREAM:
            setting_data.max_concurrent_streams = ident_value;
            break;

        case HTTP2_SETTINGS_INITIAL_WINDOW_SIZE:

            setting_data.initial_window_size = ident_value;
            peer_session->window_update_num  = ident_value;
            break;

        case HTTP2_SETTINGS_MAX_FRAME_SIZE:
            setting_data.max_frame_size = ident_value;
            break;

        case HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE:
            setting_data.max_heaer_list_size = ident_value;
            break;

        default:
            break;
        }
    }

    peer_session->send_recv_setting();
    window_update_recv_num = setting_data.initial_window_size;
    need_wakeup_send_data  = true;
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
        error = 40044;
        return;
    }

    peer_session->isgoway = true;
}

void http2parse::readwinupdate(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    DEBUG_LOG("readwinupdate %zu", temp_pack_data.payload.size());

    if (temp_pack_data.payload.size() < 4)
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

    DEBUG_LOG("window_update %u", ident_stream);
    peer_session->window_update_num = ident_stream;
    window_update_recv_num          = ident_stream;
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
    need_wakeup_send_data = true;
    DEBUG_LOG("need ack ping");
}
void http2parse::readrst_stream(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    DEBUG_LOG("readrst_stream %u ", temp_pack_data.stream_id);
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
}

void http2parse::readrawfileformdata(HTTP2_POST_DATA_T &temp_post_data, unsigned char islast_pack)
{
    if (!temp_post_data.fp)
    {
        server_loaclvar &localvar = get_server_global_var();

        temp_post_data.filename   = "rawcontent";
        temp_post_data.field_name = "rawcontent";

        std::string fieldheader_temp = temp_post_data.filename + std::to_string(http::timeid()) + rand_string(6, 0);

        temp_post_data.temp_filename = localvar.temp_path;// + "temp/";
        temp_post_data.temp_filename.append(std::to_string(std::hash<std::string>{}(fieldheader_temp)));

        std::unique_ptr<std::FILE, int (*)(FILE *)> fpa(std::fopen(temp_post_data.temp_filename.c_str(), "wb"), std::fclose);
        if (fpa)
        {
            temp_post_data.fp = std::move(fpa);
        }
        else
        {
            error = 400026;
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
            error = 400026;
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
        error = 40011;
        return;
    }

    for (unsigned int j = 0; j < form_post_name.length(); j++)
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
    for (unsigned int j = 0; j < form_post_name.length(); j++)
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
                                error = 400000;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            unsigned int iii = steam_httppeer->post[objname].size();
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
                                error = 400000;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 400000;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            unsigned int iii = steam_httppeer->post[objname].size();
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
                                error = 400000;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 400000;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            steam_httppeer->post[objname][key1name].set_object();

                            unsigned int iii = steam_httppeer->post[objname][key1name].size();
                            key2name         = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {

                            if (objname.size() > 48)
                            {
                                error = 400000;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 400000;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 400000;
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
                            error = 400000;
                            return;
                        }

                        steam_httppeer->post[objname].set_object();
                        unsigned int iii = steam_httppeer->post[objname].size();
                        key1name         = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp = form_post_value;

                        steam_httppeer->post[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 400000;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 400000;
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

    for (unsigned int j = 0; j < temp_post_data.field_name.length(); j++)
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
                                error = 400000;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            unsigned int iii = temp_post_data.peer->files[objname].size();
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
                                error = 400000;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 400000;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            unsigned int iii = temp_post_data.peer->files[objname].size();
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
                                error = 400000;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 400000;
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
                                error = 400000;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 400000;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 400000;
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
                            error = 400000;
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
                            error = 400000;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 400000;
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
    unsigned int qsize    = temp_post_data.peer->rawcontent.size();
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
                error = 40001;
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
                error = 40001;
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
            error = 40001;
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
            error = 40001;
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
            error = 40001;
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
        error = 40026;
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

        temp_post_data.temp_filename = temp_post_data.temp_filename + std::to_string(http::timeid()) + rand_string(6, 0) + "_" + std::to_string(temp_post_data.peer->content_length);
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
            unsigned int i = temp_post_data.pre_content.size() - 2;
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
            error = 403;
            return;
        }
    }
    else
    {
        if (temp_post_data.posttype == 0)
        {
            //fix weixin browser
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
                error = 4140;
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
                error = 41403;
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
                error = 41404;
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

    if (new_size_num > 0)
    {
        if (new_size_num >= temp_pack_data.payload.size())
        {
            error = 40013;
            return;
        }
        new_size_num = temp_pack_data.payload.size() - new_size_num;
        if (header_offset > temp_pack_data.payload.size() || new_size_num > (temp_pack_data.payload.size() - header_offset))
        {
            error = 40023;
            return;
        }
        post_iter->second.content = std::string_view(temp_pack_data.payload.data() + header_offset, new_size_num);
    }
    else
    {
        post_iter->second.content = std::string_view(temp_pack_data.payload.data(), temp_pack_data.payload.size());
    }

    post_iter->second.exp_length += temp_pack_data.payload.size();
    if (post_iter->second.exp_length > CONST_HTTP_BODY_POST_SIZE)
    {
        error = 40009;
        return;
    }

    if (window_update_recv_num < temp_pack_data.length)
    {
        window_update_recv_num = 0;
    }
    else
    {
        window_update_recv_num -= temp_pack_data.length;
    }

    if (window_update_recv_num < (RECV_WINDOW_UPDATE_NUM / 2))
    {
        peer_session->recv_window_update(RECV_WINDOW_UPDATE_NUM - window_update_recv_num, temp_pack_data.stream_id);
        need_wakeup_send_data  = true;
        window_update_recv_num = RECV_WINDOW_UPDATE_NUM;
    }

    post_data_process(post_iter->second, last_pack);

    if (last_pack)
    {
        if (post_iter->second.exp_length != iter->second->content_length)
        {
            error = 40007;
            return;
        }

        http_post_data.erase(post_iter);
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
        error = 400018;
    }
}

void http2parse::read_pack_data(const unsigned char *buffer, unsigned int buffersize)
{
    if (pack_data.length == 0)
    {
        //如果是跨包又不足
        if ((readoffset + 9) > buffersize)
        {
            for (; readoffset < buffersize; readoffset++)
            {
                pack_data.subpad.push_back(buffer[readoffset]);
            }
            return;
        }

        unsigned int j = 0;

        if (pack_data.subpad.size() > 0)
        {
            j = pack_data.subpad.size();
            for (; j < 9; j++)
            {
                pack_data.subpad.push_back(buffer[readoffset]);
                readoffset++;
                if (readoffset >= buffersize)
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
                error = 40002;
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
