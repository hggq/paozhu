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
#include <unistd.h>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <strings.h>
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
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

namespace http
{
void http2parse::setsession(std::shared_ptr<client_session> peer) { peer_session = peer->get_ptr(); }

void http2parse::processblockheader(const unsigned char *buffer, unsigned int buffersize)
{
    blocklength    = 0;
    unsigned int j = readoffset;
    blocklength    = (unsigned char)buffer[j];

    j++;
    blocklength = blocklength << 8 | (unsigned char)buffer[j];

    j++;
    blocklength = blocklength << 8 | (unsigned char)buffer[j];

    j++;

    frame_type = (unsigned char)buffer[j];
    j++;
    flag_type = (unsigned char)buffer[j];
    j++;

    block_steamid = 0;

    block_steamid = (unsigned char)buffer[j] & 0x7F;
    j++;
    block_steamid = block_steamid << 8 | (unsigned char)buffer[j];
    j++;
    block_steamid = block_steamid << 8 | (unsigned char)buffer[j];
    j++;
    block_steamid = block_steamid << 8 | (unsigned char)buffer[j];
    j++;

    if (frame_type == 0x00)
    {
        data_info[block_steamid].length    = blocklength;
        data_info[block_steamid].curnum    = 0;
        data_info[block_steamid].endstream = ((flag_type & 0x01) == 0x01) ? true : false;
        data_info[block_steamid].padded    = ((flag_type & 0x08) == 0x08) ? true : false;
        // padded_length;
        if (data_info[block_steamid].padded)
        {
            if (j < buffersize)
            {
                data_info[block_steamid].pad_length = (unsigned char)buffer[j];
            }
        }
        //data_steam_id = block_steamid;
    }
    else if (frame_type == 0x01)
    {
        data_info[block_steamid].length    = blocklength;
        data_info[block_steamid].curnum    = 0;
        data_info[block_steamid].endstream = ((flag_type & 0x01) == 0x01) ? true : false;
        data_info[block_steamid].padded    = ((flag_type & 0x08) == 0x08) ? true : false;

        data_info[block_steamid].endheader = ((flag_type & 0x04) == 0x04) ? true : false;
        data_info[block_steamid].priority  = ((flag_type & 0x20) == 0x20) ? true : false;

        if (data_info[block_steamid].padded)
        {
            if (j < buffersize)
            {
                data_info[block_steamid].pad_length = (unsigned char)buffer[j];
            }
            j += 1;
        }
        if (data_info[block_steamid].priority)
        {
            if ((j + 5) < buffersize)
            {
                unsigned int bsteamid = 0;

                bsteamid = (unsigned char)buffer[j] & 0x7F;
                j++;
                bsteamid = bsteamid << 8 | (unsigned char)buffer[j];
                j++;
                bsteamid = bsteamid << 8 | (unsigned char)buffer[j];
                j++;
                bsteamid = bsteamid << 8 | (unsigned char)buffer[j];
                j++;
                data_info[block_steamid].stream_id = bsteamid;

                data_info[block_steamid].weight = (unsigned char)buffer[j];
                j++;
            }
        }

        //data_steam_id = block_steamid;
        stream_data[block_steamid].clear();
    }

    readoffset    = readoffset + 9;
    processheader = 1;
}
void http2parse::readheaders(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int buffer_short_length;
    unsigned int block_short_length;

    buffer_short_length = buffersize - readoffset;
    block_short_length  = data_info[block_steamid].length - data_info[block_steamid].curnum;

    if (buffer_short_length <= block_short_length)
    {
        block_short_length = buffer_short_length;
    }

    stream_data[block_steamid].append((const char *)&buffer[readoffset], block_short_length);
    data_info[block_steamid].curnum += block_short_length;

    readoffset = readoffset + block_short_length;
    // 数据块处理完成
    if (data_info[block_steamid].curnum == data_info[block_steamid].length)
    {
        if (data_info[block_steamid].endheader)
        {
            bool ishasold = true;
            for (auto iter = http_data.begin(); iter != http_data.end();)
            {
                if (iter->second->issend)
                {
                    std::shared_ptr<httppeer> peer = iter->second;
                    unsigned int oldstream_id      = iter->first;
                    iter                           = http_data.erase(iter);
                    peer->state.gzip               = false;
                    peer->state.deflate            = false;
                    peer->state.br                 = false;
                    peer->state.avif               = false;
                    peer->state.webp               = false;
                    peer->state.keepalive          = false;
                    peer->state.websocket          = false;
                    peer->state.upgradeconnection  = false;
                    peer->state.rangebytes         = false;
                    peer->state.language[0]        = {0};
                    peer->state.version            = 0;
                    peer->state.port               = 0;
                    peer->state.ifmodifiedsince    = 0;
                    peer->state.rangebegin         = 0;
                    peer->state.rangeend           = 0;
                    peer->keepalive                = false;
                    peer->issend                   = false;
                    peer->send_header.clear();
                    peer->send_cookie_lists.clear();
                    peer->http2_send_header.clear();
                    peer->header.clear();
                    peer->pathinfos.clear();
                    peer->querystring.clear();
                    peer->urlpath.clear();
                    peer->host.clear();
                    peer->etag.clear();
                    peer->output.clear();
                    peer->val.clear();
                    peer->post.clear();
                    peer->get.clear();
                    peer->files.clear();
                    peer->json.clear();
                    peer->cookie.clear();
                    peer->rawcontent.clear();
                    peer->httpv    = 2;
                    peer->isso     = false;
                    peer->compress = 0;
                    // peer->websocket.deflate           = false;
                    // peer->websocket.permessagedeflate = false;
                    // peer->websocket.perframedeflate   = false;
                    // peer->websocket.deflateframe      = false;
                    // peer->websocket.isopen            = false;
                    // peer->websocket.version           = 0x00;
                    // peer->websocket.key.clear();
                    // peer->websocket.ext.clear();

                    http_data.emplace(block_steamid, peer);
                    ishasold = false;

                    for (auto it = data_info.begin(); it != data_info.end();)
                    {
                        if (oldstream_id == it->first)
                        {
                            it = data_info.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                    break;
                }
                ++iter;
            }
            if (ishasold)
            {
                http_data.emplace(block_steamid, std::make_shared<httppeer>());
            }
            headers_parse();
            http_data[block_steamid]->isuse_fastcgi();
        }
        processheader = 0;
    }
}
void http2parse::headers_parse()
{
    std::string_view setting_data(stream_data[block_steamid]);
    if (setting_data.size() == 0)
    {
        error = 101;
        return;
    }
    unsigned int begin = 0, header_end = setting_data.size();
    // peer_session->stream[block_steamid].content_length = 0;
    http_data[block_steamid]->content_length     = 0;
    http_data[block_steamid]->stream_id          = block_steamid;
    http_data[block_steamid]->httpv              = 2;
    http_data[block_steamid]->socket_session     = peer_session;
    http_data[block_steamid]->window_update_bool = false;

    ispost = false;
    HTTP2_HEADER_FLAG header_flags;
    header_flags.value = flag_type;

    if (header_flags.data.PADDED)
    {
        header_end -= setting_data[begin];
        begin += 1;
    }
    if (header_flags.data.PRIORITY)
    {
        begin += 5;
    }

    for (; begin < header_end; begin++)
    {
        unsigned char c = setting_data[begin];
        if (c & 0x80)
        {
            headertype1(c, setting_data, begin, header_end);
        }
        else if ((c & 0xC0) == 0x40)
        {
            headertype2(c, setting_data, begin, header_end);
        }
        else if ((c & 0xF0) == 0x10)
        {
            headertype3(c, setting_data, begin, header_end);
        }
        else if ((c & 0xF0) == 0)
        {
            headertype4(c, setting_data, begin, header_end);
        }

        else
        {
            error = 102;
            return;
        }
    }

    processheader = 0;

    if (ispost)
    {
        DEBUG_LOG("http2 post client: %s %ud", http_data[block_steamid]->url.c_str(), block_steamid);
        window_update_recv_num = RECV_WINDOW_UPDATE_NUM;
        peer_session->send_window_update(window_update_recv_num, block_steamid);
        data_info[block_steamid].stream_id      = block_steamid;
        data_info[block_steamid].curnum         = 0;
        data_info[block_steamid].length         = 0;
        data_info[block_steamid].content_length = http_data[block_steamid]->content_length;

        data_info[block_steamid].isbegin       = false;
        data_info[block_steamid].isend         = false;
        data_info[block_steamid].postfieldtype = 0;
        data_info[block_steamid].match_offset  = 0;

        http_data[block_steamid]->isfinish = false;
    }
    else
    {
        DEBUG_LOG("http2 get client: %s", http_data[block_steamid]->url.c_str());
        http_data[block_steamid]->isfinish = true;
        stream_list.emplace(block_steamid);
    }
    stream_data[block_steamid].clear();
    data_info[block_steamid].buffer_value.clear();
}
void http2parse::cookie_process([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    DEBUG_LOG("cookie_process:%s:%s", header_name.c_str(), header_value.c_str());
    //http_data[block_steamid]->header[header_name] = header_value;
    unsigned int i = 0, linesize = header_value.size();
    data_info[block_steamid].buffer_key.clear();
    data_info[block_steamid].buffer_value.clear();

    if (http_data[block_steamid]->header["Cookie"].empty())
    {
    }
    else
    {
        http_data[block_steamid]->header["Cookie"].append("; ");
    }
    http_data[block_steamid]->header["Cookie"].append(header_value);
    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x3D)
        {
            data_info[block_steamid].buffer_key = http::url_decode(data_info[block_steamid].buffer_value.data(),
                                                                   data_info[block_steamid].buffer_value.length());
            data_info[block_steamid].buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3B)
        {
            data_info[block_steamid].buffer_value = http::url_decode(data_info[block_steamid].buffer_value.data(),
                                                                     data_info[block_steamid].buffer_value.length());
            http_data[block_steamid]->cookie[data_info[block_steamid].buffer_key] =
                data_info[block_steamid].buffer_value;
            data_info[block_steamid].buffer_key.clear();
            data_info[block_steamid].buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        data_info[block_steamid].buffer_value.push_back(header_value[i]);
    }
    if (data_info[block_steamid].buffer_value.size() > 0)
    {
        data_info[block_steamid].buffer_value                                 = http::url_decode(data_info[block_steamid].buffer_value.data(),
                                                                 data_info[block_steamid].buffer_value.length());
        http_data[block_steamid]->cookie[data_info[block_steamid].buffer_key] = data_info[block_steamid].buffer_value;
    }
    else
    {
        if (data_info[block_steamid].buffer_key.size() > 0)
        {
            http_data[block_steamid]->cookie[data_info[block_steamid].buffer_key] = "";
        }
        data_info[block_steamid].buffer_key.clear();
    }
}
void http2parse::path_process([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    DEBUG_LOG("path_process:%s:%s", header_name.c_str(), header_value.c_str());
    data_info[block_steamid].buffer_key.clear();
    data_info[block_steamid].buffer_value.clear();
    unsigned char headerstep = 0;
    http_data[block_steamid]->pathinfos.clear();
    unsigned int ioffset = 0, linesize = header_value.size();
    http_data[block_steamid]->url = http::url_decode(header_value.data(), header_value.length());
    for (; ioffset < linesize; ioffset++)
    {
        if (header_value[ioffset] == 0x3F)
        {
            headerstep = 6;
            break;
        }
        if (header_value[ioffset] == 0x2F)
        {
            if (data_info[block_steamid].buffer_key.size() > 0)
            {
                if (data_info[block_steamid].buffer_key.size() == 2 && data_info[block_steamid].buffer_key[0] == '.' &&
                    data_info[block_steamid].buffer_key[1] == '.')
                {
                    if (http_data[block_steamid]->pathinfos.size() > 0)
                    {
                        http_data[block_steamid]->pathinfos.pop_back();
                    }
                }
                else if (data_info[block_steamid].buffer_key.size() == 1 &&
                         data_info[block_steamid].buffer_key[0] == '.')
                {
                }
                else
                {
                    http_data[block_steamid]->pathinfos.emplace_back(
                        http::url_decode(data_info[block_steamid].buffer_key.data(),
                                         data_info[block_steamid].buffer_key.length()));
                }
                data_info[block_steamid].buffer_key.clear();
            }
        }
        else
        {
            data_info[block_steamid].buffer_key.push_back(header_value[ioffset]);
        }
        data_info[block_steamid].buffer_value.push_back(header_value[ioffset]);
    }

    if (data_info[block_steamid].buffer_key.size() > 0)
    {
        if (data_info[block_steamid].buffer_key.size() == 2 && data_info[block_steamid].buffer_key[0] == '.' &&
            data_info[block_steamid].buffer_key[1] == '.')
        {
            if (http_data[block_steamid]->pathinfos.size() > 0)
            {
                http_data[block_steamid]->pathinfos.pop_back();
            }
        }
        else if (data_info[block_steamid].buffer_key.size() == 1 && data_info[block_steamid].buffer_key[0] == '.')
        {
        }
        else
        {
            http_data[block_steamid]->pathinfos.emplace_back(
                http::url_decode(data_info[block_steamid].buffer_key.data(),
                                 data_info[block_steamid].buffer_key.length()));
        }
    }

    if (headerstep == 6)
    {
        http_data[block_steamid]->header["urlpath"] = data_info[block_steamid].buffer_value;
        http_data[block_steamid]->urlpath           = http::url_decode(data_info[block_steamid].buffer_value.data(),
                                                             data_info[block_steamid].buffer_value.length());
    }
    else
    {
        http_data[block_steamid]->header["urlpath"] = data_info[block_steamid].buffer_value;
        http_data[block_steamid]->urlpath           = http_data[block_steamid]->url;
    }

    // if (http_data[block_steamid]->pathinfos.size() > 0)
    // {
    //     http_data[block_steamid]->urlpath.clear();
    //     for (unsigned int i = 0; i < http_data[block_steamid]->pathinfos.size(); i++)
    //     {
    //         http_data[block_steamid]->urlpath.push_back('/');
    //         http_data[block_steamid]->urlpath.append(http_data[block_steamid]->pathinfos[i]);
    //     }
    // }
    // else
    // {
    //     http_data[block_steamid]->header["urlpath"] = "/";
    //     http_data[block_steamid]->urlpath           = "/";
    // }

    data_info[block_steamid].buffer_key.clear();
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
        data_info[block_steamid].buffer_key.append(&header_value[ioffset], (linesize - ioffset));
        //http_data[block_steamid]->header["querystring"] = data_info[block_steamid].buffer_key;
        http_data[block_steamid]->querystring =
            http::url_decode(data_info[block_steamid].buffer_key.data(), data_info[block_steamid].buffer_key.length());

        data_info[block_steamid].buffer_key.clear();
        data_info[block_steamid].buffer_value.clear();
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
                data_info[block_steamid].buffer_key = http::url_decode(data_info[block_steamid].buffer_value.data(),
                                                                       data_info[block_steamid].buffer_value.length());
                data_info[block_steamid].buffer_value.clear();
                headerstep = 1;
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

                data_info[block_steamid].buffer_value =
                    http::url_decode(data_info[block_steamid].buffer_value.data(),
                                     data_info[block_steamid].buffer_value.length());
                procssparamter();
                data_info[block_steamid].buffer_key.clear();
                data_info[block_steamid].buffer_value.clear();
                headerstep = 2;
                continue;
            }
            data_info[block_steamid].buffer_value.push_back(header_value[ioffset]);
        }

        if (headerstep == 1)
        {
            // full kv
            data_info[block_steamid].buffer_value = http::url_decode(data_info[block_steamid].buffer_value.data(),
                                                                     data_info[block_steamid].buffer_value.length());
            procssparamter();
        }
        else if (headerstep == 2)
        {
            // half k
            data_info[block_steamid].buffer_key = http::url_decode(data_info[block_steamid].buffer_value.data(),
                                                                   data_info[block_steamid].buffer_value.length());
            data_info[block_steamid].buffer_value.clear();
            procssparamter();
        }
        else if (data_info[block_steamid].buffer_value.size() > 0)
        {
            // only one k
            data_info[block_steamid].buffer_key = http::url_decode(data_info[block_steamid].buffer_value.data(),
                                                                   data_info[block_steamid].buffer_value.length());
            data_info[block_steamid].buffer_value.clear();
            procssparamter();
        }
    }
}
void http2parse::procssparamter()
{
    std::string objname;
    bool isgroup = true;
    for (unsigned int j = 0; j < data_info[block_steamid].buffer_key.length(); j++)
    {
        if (data_info[block_steamid].buffer_key[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < data_info[block_steamid].buffer_key.length(); n++)
            {
                if (data_info[block_steamid].buffer_key[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (data_info[block_steamid].buffer_key[n] == '[')
                {

                    break;
                }
                if (data_info[block_steamid].buffer_key[n] != 0x22)
                {
                    key1name.push_back(data_info[block_steamid].buffer_key[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < data_info[block_steamid].buffer_key.length())
                {
                    if (data_info[block_steamid].buffer_key[m] == '[')
                    {
                        m += 1;
                        for (; m < data_info[block_steamid].buffer_key.length(); m++)
                        {
                            if (data_info[block_steamid].buffer_key[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (data_info[block_steamid].buffer_key[m] == '[')
                            {

                                break;
                            }
                            if (data_info[block_steamid].buffer_key[m] != 0x22)
                            {
                                key2name.push_back(data_info[block_steamid].buffer_key[m]);
                            }
                        }

                        if (ishaskey2 && m == data_info[block_steamid].buffer_key.length())
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
                            http::OBJ_ARRAY objtemp;
                            objtemp.push(data_info[block_steamid].buffer_value);
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            http_data[block_steamid]->get[objname].push(std::move(objtemp1));
                        }
                        else
                        {
                            http::OBJ_VALUE objtemp;
                            objtemp[key2name] = data_info[block_steamid].buffer_value;
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            http_data[block_steamid]->get[objname].push(std::move(objtemp1));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            http_data[block_steamid]->get[objname][key1name].set_array();
                            http_data[block_steamid]->get[objname][key1name] = data_info[block_steamid].buffer_value;
                        }
                        else
                        {

                            http_data[block_steamid]->get[objname][key1name][key2name] =
                                data_info[block_steamid].buffer_value;
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == data_info[block_steamid].buffer_key.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        http_data[block_steamid]->get[objname].set_array();
                        http_data[block_steamid]->get[objname].push(data_info[block_steamid].buffer_value);
                    }
                    else
                    {
                        http_data[block_steamid]->get[objname].push(key1name, data_info[block_steamid].buffer_value);
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
                objname.push_back(data_info[block_steamid].buffer_key[j]);
            }
        }
        else
        {
            objname.push_back(data_info[block_steamid].buffer_key[j]);
        }
    }
    if (isgroup)
    {
        http_data[block_steamid]->get[data_info[block_steamid].buffer_key] = data_info[block_steamid].buffer_value;
    }
}
void http2parse::range_process([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    DEBUG_LOG("range_process:%s:%s", header_name.c_str(), header_value.c_str());
    unsigned int j = 0, linesize = header_value.size();
    data_info[block_steamid].buffer_value.clear();
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
        data_info[block_steamid].buffer_value.push_back(header_value[j]);
    }

    if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "bytes") == 0)
    {
        http_data[block_steamid]->state.rangebytes = true;
    }
    http_data[block_steamid]->state.rangebegin = 0;
    http_data[block_steamid]->state.rangeend   = 0;
    data_info[block_steamid].buffer_value.clear();
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
            for (unsigned int qi = 0; qi < data_info[block_steamid].buffer_value.size(); qi++)
            {
                if (data_info[block_steamid].buffer_value[qi] < 0x3A &&
                    data_info[block_steamid].buffer_value[qi] > 0x2F)
                {
                    tm = tm * 10 + (data_info[block_steamid].buffer_value[qi] - 0x30);
                }
            }
            http_data[block_steamid]->state.rangebegin = tm;
            data_info[block_steamid].buffer_value.clear();
            continue;
        }
        data_info[block_steamid].buffer_value.push_back(header_value[j]);
    }

    if (data_info[block_steamid].buffer_value.size() > 0)
    {
        long long tm = 0;
        for (unsigned int qi = 0; qi < data_info[block_steamid].buffer_value.size(); qi++)
        {
            if (data_info[block_steamid].buffer_value[qi] < 0x3A && data_info[block_steamid].buffer_value[qi] > 0x2F)
            {
                tm = tm * 10 + (data_info[block_steamid].buffer_value[qi] - 0x30);
            }
        }
        http_data[block_steamid]->state.rangeend = tm;
    }
}

void http2parse::header_process(const std::string &header_name, const std::string &header_value, int table_num)
{
    DEBUG_LOG("header:%s:%s|%d", header_name.c_str(), header_value.c_str(), table_num);
    if (table_num > 0)
    {

        switch (table_num)
        {
        case 1:
            http_data[block_steamid]->host                 = header_value;
            http_data[block_steamid]->header["Host"]       = header_value;
            http_data[block_steamid]->header[":authority"] = header_value;
            break;
        case 2:
            if (strcasecmp(header_value.c_str(), "OPTIONS") == 0)
            {
                http_data[block_steamid]->method = 3;
            }
            else if (strcasecmp(header_value.c_str(), "GET") == 0)
            {
                http_data[block_steamid]->method = 1;
            }
            else if (strcasecmp(header_value.c_str(), "POST") == 0)
            {
                http_data[block_steamid]->method = 2;
            }
            else if (strcasecmp(header_value.c_str(), "head") == 0)
            {
                http_data[block_steamid]->method = 4;
            }
            else if (strcasecmp(header_value.c_str(), "put") == 0)
            {
                http_data[block_steamid]->method = 5;
            }
            else if (strcasecmp(header_value.c_str(), "delete") == 0)
            {
                http_data[block_steamid]->method = 6;
            }
            http_data[block_steamid]->header["method"] = header_value;
            break;
        case 3:
            if (strcasecmp(header_value.c_str(), "OPTIONS") == 0)
            {
                http_data[block_steamid]->method = 3;
            }
            else if (strcasecmp(header_value.c_str(), "GET") == 0)
            {
                http_data[block_steamid]->method = 1;
            }
            else if (strcasecmp(header_value.c_str(), "POST") == 0)
            {
                http_data[block_steamid]->method = 2;
            }
            else if (strcasecmp(header_value.c_str(), "head") == 0)
            {
                http_data[block_steamid]->method = 4;
            }
            else if (strcasecmp(header_value.c_str(), "put") == 0)
            {
                http_data[block_steamid]->method = 5;
            }
            else if (strcasecmp(header_value.c_str(), "delete") == 0)
            {
                http_data[block_steamid]->method = 6;
            }
            http_data[block_steamid]->header["method"] = header_value;
            break;
        case 4:
        case 5: path_process(header_name, header_value); break;
        case 16: getacceptencoding(header_name, header_value); break;
        case 17: getacceptlanguage(header_name, header_value); break;
        case 19: getaccept(header_name, header_value); break;
        case 20:
            http_data[block_steamid]->header["Access-Control-Allow-Origin"] = header_value;
            break;
        case 23:
            http_data[block_steamid]->header["Authorization"] = header_value;
            break;
        case 28:
            http_data[block_steamid]->content_length           = str2int(&header_value[0], header_value.size());
            http_data[block_steamid]->header["Content-Length"] = header_value;
            break;
        case 31: getcontenttype(header_name, header_value); break;
        case 32: cookie_process(header_name, header_value); break;
        case 38:
            http_data[block_steamid]->header["Host"] = header_value;
            break;
        case 41: getifnonematch(header_name, header_value); break;
        case 48:
            http_data[block_steamid]->header["Proxy-Authenticate"] = header_value;
            break;
        case 49:
            http_data[block_steamid]->header["Proxy-Authorization"] = header_value;
            break;
        case 50: range_process(header_name, header_value); break;
        case 51:
            http_data[block_steamid]->header["Referer"] = header_value;
            break;
        case 58:
            http_data[block_steamid]->header["User-Agent"] = header_value;
            break;
        case 61:
            http_data[block_steamid]->header["WWW-Authenticate"] = header_value;
            break;
        }
    }
    else
    {
        switch (header_name.size())
        {
        case 5:
            if (strcasecmp(header_name.c_str(), ":path") == 0)
            {
                path_process(header_name, header_value);
            }
            else if (strcasecmp(header_name.c_str(), "range") == 0)
            {
                range_process(header_name, header_value);
            }
            http_data[block_steamid]->header[header_name] = header_value;
            break;
        case 6:

            switch (header_name[0])
            {
            case 'c':
            case 'C':
                if (strcasecmp(header_name.c_str(), "Cookie") == 0)
                {
                    cookie_process(header_name, header_value);
                }
                else
                {
                    http_data[block_steamid]->header[header_name] = header_value;
                }
                break;
            case 'a':
            case 'A':
                if (strcasecmp(header_name.c_str(), "Accept") == 0)
                {
                    getaccept(header_name, header_value);
                }
                else
                {
                    if (header_name[0] != ':')
                    {
                        http_data[block_steamid]->header[header_name] = header_value;
                    }
                }
                break;
            default:
                if (header_name[0] != ':')
                {
                    http_data[block_steamid]->header[header_name] = header_value;
                }
            }

            break;
        case 7:
            if (strcasecmp(header_name.c_str(), ":method") == 0)
            {
                if (strcasecmp(header_value.c_str(), "OPTIONS") == 0)
                {
                    http_data[block_steamid]->method = 3;
                }
                else if (strcasecmp(header_value.c_str(), "GET") == 0)
                {
                    http_data[block_steamid]->method = 1;
                }
                else if (strcasecmp(header_value.c_str(), "POST") == 0)
                {
                    http_data[block_steamid]->method = 2;
                }
                else if (strcasecmp(header_value.c_str(), "head") == 0)
                {
                    http_data[block_steamid]->method = 4;
                }
                else if (strcasecmp(header_value.c_str(), "put") == 0)
                {
                    http_data[block_steamid]->method = 5;
                }
                else if (strcasecmp(header_value.c_str(), "delete") == 0)
                {
                    http_data[block_steamid]->method = 6;
                }
                http_data[block_steamid]->header["method"]  = header_value;
                http_data[block_steamid]->header[":method"] = header_value;
            }
            else
            {
                http_data[block_steamid]->header[header_name] = header_value;
            }
            break;
        case 10:
            if (strcasecmp(header_name.c_str(), ":authority") == 0)
            {
                http_data[block_steamid]->host                 = header_value;
                http_data[block_steamid]->header["Host"]       = header_value;
                http_data[block_steamid]->header[":authority"] = header_value;
            }
            else if (strcasecmp(header_name.c_str(), "User-Agent") == 0)
            {
                http_data[block_steamid]->header["User-Agent"] = header_value;
            }
            else
            {
                http_data[block_steamid]->header[header_name] = header_value;
            }
            break;
        case 12:
            if (strcasecmp(header_name.c_str(), "Content-Type") == 0)
            {
                getcontenttype(header_name, header_value);
            }
            else
            {
                http_data[block_steamid]->header[header_name] = header_value;
            }
            break;
        case 13:
            if (strcasecmp(header_name.c_str(), "if-none-match") == 0)
            {
                getifnonematch(header_name, header_value);
            }
            else
            {
                http_data[block_steamid]->header[header_name] = header_value;
            }
            break;

        case 14:
            if (strcasecmp(header_name.c_str(), "Content-Length") == 0)
            {
                http_data[block_steamid]->content_length           = str2int(&header_value[0], header_value.size());
                http_data[block_steamid]->header["Content-Length"] = header_value;
            }
            else
            {
                http_data[block_steamid]->header[header_name] = header_value;
            }
            break;
        case 15:
            if (header_name[7] == 'e')
            {
                if (strcasecmp(header_name.c_str(), "accept-encoding") == 0)
                {
                    getacceptencoding(header_name, header_value);
                    break;
                }
                else
                {
                    http_data[block_steamid]->header[header_name] = header_value;
                }
            }
            else if (header_name[7] == 'L')
            {
                if (strcasecmp(header_name.c_str(), "Accept-Language") == 0)
                {
                    getacceptlanguage(header_name, header_value);
                    break;
                }
                else
                {
                    http_data[block_steamid]->header[header_name] = header_value;
                }
            }
            else
            {
                http_data[block_steamid]->header[header_name] = header_value;
            }

            if (strcasecmp(header_name.c_str(), "Accept-Encoding") == 0)
            {
                getacceptencoding(header_name, header_value);
            }
            if (strcasecmp(header_name.c_str(), "Accept-Language") == 0)
            {
                getacceptlanguage(header_name, header_value);
            }

            break;
        default:
            http_data[block_steamid]->header[header_name] = header_value;
        }
    }
}

void http2parse::getacceptlanguage(const std::string &header_name, const std::string &header_value)
{
    http_data[block_steamid]->header["Accept-Language"] = header_value;
    unsigned int i                                      = 0;
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] == 0x2C)
        {
            break;
        }
        http_data[block_steamid]->state.language[i] = header_value[i];
        if (i > 6)
        {
            if (sizeof(header_name))
            {
            }
            break;
        }
    }
    for (; i < 8; i++)
    {
        http_data[block_steamid]->state.language[i] = 0x00;
    }
}
void http2parse::getacceptencoding(const std::string &header_name, const std::string &header_value)
{
    unsigned int i = 0, linesize = header_value.size();
    http_data[block_steamid]->header["Accept-Encoding"] = header_value;
    data_info[block_steamid].buffer_value.clear();
    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C)
        {

            switch (data_info[block_steamid].buffer_value.size())
            {
            case 2:
                if (data_info[block_steamid].buffer_value[0] == 'b')
                {
                    http_data[block_steamid]->state.br = true;
                }
                break;
            case 4:
                if (data_info[block_steamid].buffer_value[0] == 'g')
                {
                    http_data[block_steamid]->state.gzip = true;
                }
                break;
            case 7:
                if (data_info[block_steamid].buffer_value[0] == 'd')
                {
                    http_data[block_steamid]->state.deflate = true;
                    if (sizeof(header_name))
                    {
                    }
                }
                break;
            default:;
            }
            data_info[block_steamid].buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        data_info[block_steamid].buffer_value.push_back(header_value[i]);
    }
    if (data_info[block_steamid].buffer_value.size() > 0)
    {

        switch (data_info[block_steamid].buffer_value.size())
        {
        case 2:
            if (data_info[block_steamid].buffer_value[0] == 'b')
            {
                http_data[block_steamid]->state.br = true;
            }
            break;
        case 4:
            if (data_info[block_steamid].buffer_value[0] == 'g')
            {
                http_data[block_steamid]->state.gzip = true;
            }
            break;
        case 7:
            if (data_info[block_steamid].buffer_value[0] == 'd')
            {
                http_data[block_steamid]->state.deflate = true;
            }
            break;
        default:;
        }
    }
    data_info[block_steamid].buffer_value.clear();
}
void http2parse::getifnonematch([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    unsigned int i                                    = 0;
    http_data[block_steamid]->header["If-None-Match"] = header_value;
    http_data[block_steamid]->etag.clear();
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
            http_data[block_steamid]->etag.push_back(header_value[i]);
        }
    }
}

void http2parse::callposttype()
{
    switch (data_info[block_steamid].buffer_value.size())
    {
    case 33:
        if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "application/x-www-form-urlencoded") == 0)
        {
            http_data[block_steamid]->content_type = "application/x-www-form-urlencoded";
            data_info[block_steamid].posttype      = 1;
            http_data[block_steamid]->posttype     = 1;
            return;
        }
        break;
    case 24:
        if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "application/octet-stream") == 0)
        {
            http_data[block_steamid]->content_type = "application/octet-stream";
            data_info[block_steamid].posttype      = 5;
            http_data[block_steamid]->posttype     = 5;
            return;
        }
        break;
    case 19:
        if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "multipart/form-data") == 0)
        {
            http_data[block_steamid]->content_type = "multipart/form-data";
            data_info[block_steamid].posttype      = 2;
            http_data[block_steamid]->posttype     = 2;
            return;
        }
        break;
    case 16:
        if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "application/json") == 0)
        {
            http_data[block_steamid]->content_type = "application/json";
            data_info[block_steamid].posttype      = 3;
            http_data[block_steamid]->posttype     = 3;
            return;
        }
        break;
    case 15:
        if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "application/xml") == 0)
        {
            http_data[block_steamid]->content_type = "application/xml";
            data_info[block_steamid].posttype      = 4;
            http_data[block_steamid]->posttype     = 4;
            return;
        }
        break;
    case 6:
        if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "binary") == 0)
        {
            http_data[block_steamid]->content_type = "binary";
            data_info[block_steamid].posttype      = 6;
            http_data[block_steamid]->posttype     = 5;
            return;
        }
        break;
    default:
        http_data[block_steamid]->content_type = "raw";
        data_info[block_steamid].posttype      = 7;
        http_data[block_steamid]->posttype     = 5;
        return;
    }
    http_data[block_steamid]->content_type = "raw";
    data_info[block_steamid].posttype      = 7;
    http_data[block_steamid]->posttype     = 5;
    return;
}

void http2parse::getcontenttype([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    unsigned int i = 0, linesize = header_value.size();
    http_data[block_steamid]->header["Content-Type"] = header_value;
    data_info[block_steamid].buffer_value.clear();
    unsigned char statetemp           = 0;
    data_info[block_steamid].posttype = 0;
    for (; i < linesize; i++)
    {

        if (header_value[i] == 0x3B)
        {

            if (statetemp == 0)
            {
                callposttype();
            }
            else if (statetemp == 1)
            {
                http_data[block_steamid]->chartset = data_info[block_steamid].buffer_value;
                statetemp                          = 0;
            }
            else if (statetemp == 2)
            {
                data_info[block_steamid].boundary = data_info[block_steamid].buffer_value;
                statetemp                         = 0;
            }
            /////////////////////
            data_info[block_steamid].buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3D)
        {
            if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "charset") == 0)
            {
                data_info[block_steamid].buffer_value.clear();
                statetemp = 1;
                continue;
            }
            else if (strcasecmp(data_info[block_steamid].buffer_value.c_str(), "boundary") == 0)
            {
                data_info[block_steamid].buffer_value.clear();
                statetemp = 2;
                continue;
            }
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        data_info[block_steamid].buffer_value.push_back(header_value[i]);
    }
    if (data_info[block_steamid].buffer_value.size() > 0)
    {
        if (statetemp == 1)
        {
            http_data[block_steamid]->chartset = data_info[block_steamid].buffer_value;
        }
        else if (statetemp == 2)
        {
            data_info[block_steamid].boundary = data_info[block_steamid].buffer_value;
        }
        else
        {
            callposttype();
        }
    }
}
void http2parse::getaccept([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    unsigned int i = 0, linesize = header_value.size();
    http_data[block_steamid]->header["Accept"] = header_value;
    data_info[block_steamid].buffer_value.clear();

    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C || header_value[i] == 0x3B)
        {
            switch (data_info[block_steamid].buffer_value.length())
            {
            case 10:
                if (data_info[block_steamid].buffer_value[6] == 'a' &&
                    data_info[block_steamid].buffer_value[7] == 'v' &&
                    data_info[block_steamid].buffer_value[8] == 'i' && data_info[block_steamid].buffer_value[9] == 'f')
                {
                    http_data[block_steamid]->state.avif = true;
                }
                else if (data_info[block_steamid].buffer_value[6] == 'w' &&
                         data_info[block_steamid].buffer_value[7] == 'e' &&
                         data_info[block_steamid].buffer_value[8] == 'b' &&
                         data_info[block_steamid].buffer_value[9] == 'p')
                {
                    http_data[block_steamid]->state.webp = true;
                }
                break;
            default:;
            }
            data_info[block_steamid].buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        data_info[block_steamid].buffer_value.push_back(header_value[i]);
    }
    if (data_info[block_steamid].buffer_value.size() > 0)
    {
        switch (data_info[block_steamid].buffer_value.length())
        {
        case 10:
            if (data_info[block_steamid].buffer_value[6] == 'a' && data_info[block_steamid].buffer_value[7] == 'v' &&
                data_info[block_steamid].buffer_value[8] == 'i' && data_info[block_steamid].buffer_value[9] == 'f')
            {
                http_data[block_steamid]->state.avif = true;
            }
            else if (data_info[block_steamid].buffer_value[6] == 'w' &&
                     data_info[block_steamid].buffer_value[7] == 'e' &&
                     data_info[block_steamid].buffer_value[8] == 'b' && data_info[block_steamid].buffer_value[9] == 'p')
            {
                http_data[block_steamid]->state.webp = true;
            }
            break;
        default:;
        }
    }
}
void http2parse::headertype1(unsigned char c,
                             [[maybe_unused]] std::string_view header_data,
                             [[maybe_unused]] unsigned int &begin,
                             [[maybe_unused]] unsigned int end)
{
    unsigned char a = c & 0x7F;
    std::string name_key;
    std::string value;

    if (a < 62)
    {
        name_key = http2_header_static_table[a].key;
        value    = http2_header_static_table[a].value;
        if (a == 2 || a == 3)
        {
            http_data[block_steamid]->header[":method"] = http2_header_static_table[a].value;
            if (a == 3)
            {
                http_data[block_steamid]->method = 2;
                ispost                           = true;
            }
            else
            {
                http_data[block_steamid]->method = 1;
                ispost                           = false;
            }
        }
        else if (a == 4 || a == 5)
        {
            http_data[block_steamid]->url             = http2_header_static_table[a].value;
            http_data[block_steamid]->urlpath         = http2_header_static_table[a].value;
            http_data[block_steamid]->header["path"]  = http2_header_static_table[a].value;
            http_data[block_steamid]->header[":path"] = http2_header_static_table[a].value;
        }
        else if (a == 6 || a == 7)
        {

            http_data[block_steamid]->isssl             = (a == 7) ? true : false;
            http_data[block_steamid]->header[":scheme"] = http2_header_static_table[a].value;
        }
        else
        {
            header_process(http2_header_static_table[a].key, http2_header_static_table[a].value, a);
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
                header_process(hinfo.first, hinfo.second, 0);
                // name有可能缓存起来了 header name maybe cache to dynamic table
                break;
            }
            j++;
        }
    }
}
void http2parse::headertype2(unsigned char c, std::string_view header_data, unsigned int &begin, unsigned int end)
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
            iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (header_data[begin] & 0x7F);
            begin += 1;

            if (iscontinue)
            {
                item_length = item_length + (header_data[begin] & 0x7F) * 128;
                begin += 1;
            }
        }

        if ((begin + item_length) > end)
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

            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (header_data[begin] & 0x7F);
            begin += 1;

            if (iscontinue)
            {
                item_length = item_length + (header_data[begin] & 0x7F) * 128;
                begin += 1;
            }
        }
        if ((begin + item_length) > end)
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

        header_process(name_key, value, 0);
        dynamic_lists.push_front({name_key, std::move(value)});

        begin -= 1;
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
        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }
        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {

            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (header_data[begin] & 0x7F);
            begin += 1;

            if (iscontinue)
            {
                item_length = item_length + (header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 126;
                    return;
                }
                begin += 1;
            }
        }
        if ((begin + item_length) > end)
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
        header_process(name_key, value, c);
        dynamic_lists.push_front({name_key, std::move(value)});

        begin -= 1;
    }
}
void http2parse::headertype3(unsigned char c, std::string_view header_data, unsigned int &begin, unsigned int end)
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
            if (header_data[begin] & 0x80)
            {
                error = 135;
                return;
            }
            else
            {
                item_length = item_length + (header_data[begin] & 0x7F);
                begin += 1;
            }
        }

        if ((begin + item_length) > end)
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

        begin                = begin + item_length;
        bool ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {
            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (header_data[begin] & 0x7F);
            begin += 1;

            if (iscontinue)
            {
                item_length = item_length + (header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 136;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > end)
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

        header_process(name_key, value, 0);
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
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }
        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {
            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (header_data[begin] & 0x7F);
            begin += 1;

            if (iscontinue)
            {
                item_length = item_length + (header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 136;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > end)
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
        header_process(name_key, value, c);
        begin -= 1;
    }
}
void http2parse::headertype4(unsigned char c, std::string_view header_data, unsigned int &begin, unsigned int end)
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
            if (header_data[begin] & 0x80)
            {
                error = 145;
                return;
            }
            else
            {
                item_length = item_length + (header_data[begin] & 0x7F);
                begin += 1;
            }
        }

        if ((begin + item_length) > end)
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

        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (item_length == 0x7F)
        {

            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (header_data[begin] & 0x7F);
            begin += 1;

            if (iscontinue)
            {
                item_length = item_length + (header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 146;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > end)
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

        header_process(name_key, value, 0);
        begin -= 1;
    }
    else
    {
        a = header_data[begin] & 0x0F;
        if (a == 0x0F)
        {
            begin += 1;
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
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;

        if (item_length == 0x7F)
        {

            bool iscontinue = false;
            if (header_data[begin] & 0x80)
            {
                iscontinue = true;
            }

            item_length = item_length + (header_data[begin] & 0x7F);
            begin += 1;

            if (iscontinue)
            {
                item_length = item_length + (header_data[begin] & 0x7F) * 128;
                if (header_data[begin] & 0x80)
                {
                    error = 146;
                    return;
                }
                begin += 1;
            }
        }

        if ((begin + item_length) > end)
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

        header_process(name_key, value, c);
        begin -= 1;
    }
}

void http2parse::readsetting(const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    // unsigned int j = readoffset;
    if (flag_type == 0x01)
    {
        readoffset += blocklength;
        processheader = 0;
        DEBUG_LOG("readsetting 0x01 %ul", buffersize);
        return;
    }

    unsigned int pin;
    pin = readoffset + blocklength;

    unsigned short ident_type;
    unsigned int ident_value;
    peer_session->window_update_num = 65535;

    for (unsigned int n = readoffset; n < pin; n += 6)
    {

        ident_type = buffer[n];
        ident_type = ident_type << 8 | buffer[n + 1];

        ident_value = buffer[n + 2];
        ident_value = ident_value << 8 | buffer[n + 3];
        ident_value = ident_value << 8 | buffer[n + 4];
        ident_value = ident_value << 8 | buffer[n + 5];

        switch (ident_type)
        {
        case HTTP2_SETTINGS_HEADER_TABLE_SIZE: setting_data.header_table_size = ident_value; break;

        case HTTP2_SETTINGS_ENABLE_PUSH: setting_data.enable_push = ident_value; break;

        case HTTP2_SETTINGS_MAX_CONCURRENT_STREAM: setting_data.max_concurrent_streams = ident_value; break;

        case HTTP2_SETTINGS_INITIAL_WINDOW_SIZE:

            setting_data.initial_window_size = ident_value;
            peer_session->window_update_num  = ident_value;
            break;

        case HTTP2_SETTINGS_MAX_FRAME_SIZE: setting_data.max_frame_size = ident_value; break;

        case HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE: setting_data.max_heaer_list_size = ident_value; break;

        default: break;
        }
    }

    processheader = 0;
    parse_setup   = 2;

    peer_session->send_recv_setting();
    window_update_recv_num = setting_data.initial_window_size;
    readoffset += blocklength;
}
void http2parse::readpriority(const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    unsigned int pin;
    pin = readoffset + blocklength;
    DEBUG_LOG("readpriority %ul", buffersize);
    unsigned int ident_stream;

    for (unsigned int n = readoffset; n < pin; n += 5)
    {
        struct http2_priority_t temp;

        ident_stream          = buffer[n];
        ident_stream          = ident_stream << 8 | buffer[n + 1];
        ident_stream          = ident_stream << 8 | buffer[n + 2];
        ident_stream          = ident_stream << 8 | buffer[n + 3];
        temp.weight           = buffer[n + 4];
        temp.depend_stream_id = ident_stream;

        priority_data[block_steamid] = temp;
    }

    readoffset += 5;
    processheader = 0;
}
void http2parse::readdata([[maybe_unused]] const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readdata %ul %c", buffersize, (buffer[readoffset] ? '0' : '1'));
    readoffset += blocklength;
}
void http2parse::readgoaway(const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readgoaway %ul", buffersize);
    unsigned int j = readoffset;
    unsigned int ident_stream;
    ident_stream = buffer[j];
    ident_stream = ident_stream << 8 | buffer[j + 1];
    ident_stream = ident_stream << 8 | buffer[j + 2];
    ident_stream = ident_stream << 8 | buffer[j + 3];

    goaway_data.last_stream_id = ident_stream;
    j += 4;
    ident_stream = 0;
    ident_stream = buffer[j];
    ident_stream = ident_stream << 8 | buffer[j + 1];
    ident_stream = ident_stream << 8 | buffer[j + 2];
    ident_stream = ident_stream << 8 | buffer[j + 3];

    goaway_data.error_code = ident_stream;

    j += 4;
    ident_stream = j + blocklength;
    for (; j < ident_stream; j++)
    {
        goaway_data.data.push_back(buffer[j]);
    }

    readoffset += blocklength;
    processheader = 0;

    peer_session->isgoway = true;
}

void http2parse::readsubdata([[maybe_unused]] const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readsubdata %ul %c", buffersize, (buffer[readoffset] ? '0' : '1'));
    readoffset += blocklength;
    processheader = 0;
}
void http2parse::readcontinuation([[maybe_unused]] const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readcontinuation %ul %c", buffersize, (buffer[readoffset] ? '0' : '1'));
    readoffset += blocklength;
    processheader = 0;
}
void http2parse::readwinupdate(const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readwinupdate %ul", buffersize);
    unsigned int ident_stream, j;
    j            = readoffset;
    ident_stream = buffer[j];
    ident_stream = ident_stream << 8 | buffer[j + 1];
    ident_stream = ident_stream << 8 | buffer[j + 2];
    ident_stream = ident_stream << 8 | buffer[j + 3];
    readoffset += 4;
    processheader = 0;
    DEBUG_LOG("window_update %d", ident_stream);
    peer_session->window_update_num = ident_stream + peer_session->window_update_num;
    window_update_recv_num          = ident_stream;

    if (peer_session->atomic_bool)
    {
        try
        {
            peer_session->atomic_bool = false;
            peer_session->window_update_promise.set_value(1);
        }
        catch (const std::exception &e)
        {
            LOG_OUT << "winupdate promise" << LOG_END;
        }
    }
}
//
void http2parse::readping(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int j = readoffset;
    processheader  = 0;
    unsigned char _recvack[] =
        {0x00, 0x00, 0x08, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int i = 0;
    for (; j < buffersize; j++)
    {
        if (i < 8)
        {
            _recvack[i + 9] = buffer[j];
        }
        else
        {
            break;
        }
    }
    processheader = 0;
    readoffset += blocklength;
    //peer_session->send_data(_recvack, 17);
    peer_session->http2_send_data(_recvack, 17);
}
void http2parse::readrst_stream([[maybe_unused]] const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readrst_stream %ul %c", buffersize, (buffer[readoffset] ? '0' : '1'));
    readoffset += blocklength;
    processheader                     = 0;
    http_data[block_steamid]->isclose = true;
}
void http2parse::procssxformurlencoded()
{
    std::string objname;
    bool isgroup = true;

    for (unsigned int j = 0; j < data_info[block_steamid].buffer_key.length(); j++)
    {
        if (data_info[block_steamid].buffer_key[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < data_info[block_steamid].buffer_key.length(); n++)
            {
                if (data_info[block_steamid].buffer_key[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (data_info[block_steamid].buffer_key[n] == '[')
                {

                    break;
                }
                if (data_info[block_steamid].buffer_key[n] != 0x22)
                {
                    key1name.push_back(data_info[block_steamid].buffer_key[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < data_info[block_steamid].buffer_key.length())
                {
                    if (data_info[block_steamid].buffer_key[m] == '[')
                    {
                        m += 1;
                        for (; m < data_info[block_steamid].buffer_key.length(); m++)
                        {
                            if (data_info[block_steamid].buffer_key[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (data_info[block_steamid].buffer_key[m] == '[')
                            {

                                break;
                            }
                            if (data_info[block_steamid].buffer_key[m] != 0x22)
                            {
                                key2name.push_back(data_info[block_steamid].buffer_key[m]);
                            }
                        }

                        if (ishaskey2 && m == data_info[block_steamid].buffer_key.length())
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
                            http::OBJ_ARRAY objtemp;
                            objtemp.push(data_info[block_steamid].field_value);
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            http_data[block_steamid]->post[objname].push(std::move(objtemp1));
                        }
                        else
                        {
                            http::OBJ_VALUE objtemp;
                            objtemp[key2name] = data_info[block_steamid].field_value;
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            http_data[block_steamid]->post[objname].push(std::move(objtemp1));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            http_data[block_steamid]->post[objname][key1name].set_array();
                            http_data[block_steamid]->post[objname][key1name] = data_info[block_steamid].field_value;
                        }
                        else
                        {

                            http_data[block_steamid]->post[objname][key1name][key2name] =
                                data_info[block_steamid].field_value;
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == data_info[block_steamid].buffer_key.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        http_data[block_steamid]->post[objname].set_array();
                        http_data[block_steamid]->post[objname].push(data_info[block_steamid].field_value);
                    }
                    else
                    {
                        http_data[block_steamid]->post[objname].push(key1name, data_info[block_steamid].field_value);
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
                objname.push_back(data_info[block_steamid].buffer_key[j]);
            }
        }
        else
        {
            objname.push_back(data_info[block_steamid].buffer_key[j]);
        }
    }
    if (isgroup)
    {
        http_data[block_steamid]->post[data_info[block_steamid].buffer_key] = data_info[block_steamid].field_value;
    }
}
void http2parse::readformurlencoded()
{
    std::string temp_value;
    data_info[block_steamid].buffer_key.clear();
    data_info[block_steamid].field_value.clear();
    unsigned int qsize    = data_info[block_steamid].buffer_value.size();
    unsigned char partype = 0;
    unsigned int j        = 0;
    for (j = 0; j < qsize; j++)
    {
        if (data_info[block_steamid].buffer_value[j] == 0x3D)
        {
            data_info[block_steamid].buffer_key = http::url_decode(temp_value.data(), temp_value.length());
            temp_value.clear();
            partype = 1;
            continue;
        }
        else if (data_info[block_steamid].buffer_value[j] == 0x26)
        {
            data_info[block_steamid].field_value = http::url_decode(temp_value.data(), temp_value.length());
            procssxformurlencoded();
            temp_value.clear();
            data_info[block_steamid].field_value.clear();
            partype = 2;
            continue;
        }
        temp_value.push_back(data_info[block_steamid].buffer_value[j]);
    }
    if (partype == 1)
    {
        data_info[block_steamid].field_value = http::url_decode(temp_value.data(), temp_value.length());
        procssxformurlencoded();
    }
    else if (partype == 2)
    {
        data_info[block_steamid].buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        data_info[block_steamid].field_value.clear();
        procssxformurlencoded();
    }
    else if (temp_value.size() > 0)
    {
        data_info[block_steamid].buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        data_info[block_steamid].field_value.clear();
        procssxformurlencoded();
    }
}

void http2parse::procssformfile()
{
    std::string objname;
    bool isgroup = true;
    for (unsigned int j = 0; j < data_info[block_steamid].upfile.name.length(); j++)
    {
        if (data_info[block_steamid].upfile.name[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < data_info[block_steamid].upfile.name.length(); n++)
            {
                if (data_info[block_steamid].upfile.name[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (data_info[block_steamid].upfile.name[n] == '[')
                {

                    break;
                }
                if (data_info[block_steamid].upfile.name[n] != 0x22)
                {
                    key1name.push_back(data_info[block_steamid].upfile.name[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < data_info[block_steamid].upfile.name.length())
                {
                    if (data_info[block_steamid].upfile.name[m] == '[')
                    {
                        m += 1;
                        for (; m < data_info[block_steamid].upfile.name.length(); m++)
                        {
                            if (data_info[block_steamid].upfile.name[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (data_info[block_steamid].upfile.name[m] == '[')
                            {

                                break;
                            }
                            if (data_info[block_steamid].upfile.name[m] != 0x22)
                            {
                                key2name.push_back(data_info[block_steamid].upfile.name[m]);
                            }
                        }

                        if (ishaskey2 && m == data_info[block_steamid].upfile.name.length())
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

                            http::OBJ_VALUE objtemp;
                            objtemp.set_array();
                            objtemp["filename"] = data_info[block_steamid].upfile.filename;
                            objtemp["name"]     = data_info[block_steamid].upfile.name;
                            objtemp["tempfile"] = data_info[block_steamid].upfile.tempfile;
                            objtemp["type"]     = data_info[block_steamid].upfile.type;
                            objtemp["size"]     = data_info[block_steamid].upfile.size;
                            objtemp["error"]    = data_info[block_steamid].upfile.error;

                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            http_data[block_steamid]->files[objname].push(std::move(objtemp1));
                        }
                        else
                        {

                            http::OBJ_VALUE objtemp;
                            objtemp[key2name].set_array();
                            objtemp[key2name]["filename"] = data_info[block_steamid].upfile.filename;
                            objtemp[key2name]["name"]     = data_info[block_steamid].upfile.name;
                            objtemp[key2name]["tempfile"] = data_info[block_steamid].upfile.tempfile;
                            objtemp[key2name]["type"]     = data_info[block_steamid].upfile.type;
                            objtemp[key2name]["size"]     = data_info[block_steamid].upfile.size;
                            objtemp[key2name]["error"]    = data_info[block_steamid].upfile.error;
                            http_data[block_steamid]->files[objname].set_array();
                            http_data[block_steamid]->files[objname].push(std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {

                            http_data[block_steamid]->files[objname][key1name].set_array();
                            http::OBJ_VALUE objtemp;
                            objtemp.set_array();
                            objtemp["filename"] = data_info[block_steamid].upfile.filename;
                            objtemp["name"]     = data_info[block_steamid].upfile.name;
                            objtemp["tempfile"] = data_info[block_steamid].upfile.tempfile;
                            objtemp["type"]     = data_info[block_steamid].upfile.type;
                            objtemp["size"]     = data_info[block_steamid].upfile.size;
                            objtemp["error"]    = data_info[block_steamid].upfile.error;

                            http_data[block_steamid]->files[objname][key1name] = objtemp;
                        }
                        else
                        {

                            http_data[block_steamid]->files[objname][key1name][key2name].set_array();
                            http_data[block_steamid]->files[objname][key1name][key2name]["filename"] =
                                data_info[block_steamid].upfile.filename;
                            http_data[block_steamid]->files[objname][key1name][key2name]["name"] =
                                data_info[block_steamid].upfile.name;
                            http_data[block_steamid]->files[objname][key1name][key2name]["tempfile"] =
                                data_info[block_steamid].upfile.tempfile;
                            http_data[block_steamid]->files[objname][key1name][key2name]["type"] =
                                data_info[block_steamid].upfile.type;
                            http_data[block_steamid]->files[objname][key1name][key2name]["size"] =
                                data_info[block_steamid].upfile.size;
                            http_data[block_steamid]->files[objname][key1name][key2name]["error"] =
                                data_info[block_steamid].upfile.error;
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == data_info[block_steamid].upfile.name.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        http_data[block_steamid]->files[objname].set_array();
                        http::OBJ_VALUE objtemp;
                        objtemp.set_array();
                        objtemp["filename"]                      = data_info[block_steamid].upfile.filename;
                        objtemp["name"]                          = data_info[block_steamid].upfile.name;
                        objtemp["tempfile"]                      = data_info[block_steamid].upfile.tempfile;
                        objtemp["type"]                          = data_info[block_steamid].upfile.type;
                        objtemp["size"]                          = data_info[block_steamid].upfile.size;
                        objtemp["error"]                         = data_info[block_steamid].upfile.error;
                        http_data[block_steamid]->files[objname] = objtemp;
                    }
                    else
                    {
                        // files[objname].push(key1name,"");
                        http_data[block_steamid]->files[objname][key1name].set_array();
                        http_data[block_steamid]->files[objname][key1name]["filename"] =
                            data_info[block_steamid].upfile.filename;
                        http_data[block_steamid]->files[objname][key1name]["name"] =
                            data_info[block_steamid].upfile.name;
                        http_data[block_steamid]->files[objname][key1name]["tempfile"] =
                            data_info[block_steamid].upfile.tempfile;
                        http_data[block_steamid]->files[objname][key1name]["type"] =
                            data_info[block_steamid].upfile.type;
                        http_data[block_steamid]->files[objname][key1name]["size"] =
                            data_info[block_steamid].upfile.size;
                        http_data[block_steamid]->files[objname][key1name]["error"] =
                            data_info[block_steamid].upfile.error;
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
                objname.push_back(data_info[block_steamid].upfile.name[j]);
            }
        }
        else
        {
            objname.push_back(data_info[block_steamid].upfile.name[j]);
        }
    }
    if (isgroup)
    {
        // files[upfile.name]=data_info[block_steamid].buffer_value;
        http_data[block_steamid]->files[data_info[block_steamid].upfile.name].set_array();
        http_data[block_steamid]->files[data_info[block_steamid].upfile.name]["filename"] =
            data_info[block_steamid].upfile.filename;
        http_data[block_steamid]->files[data_info[block_steamid].upfile.name]["name"] =
            data_info[block_steamid].upfile.name;
        http_data[block_steamid]->files[data_info[block_steamid].upfile.name]["tempfile"] =
            data_info[block_steamid].upfile.tempfile;
        http_data[block_steamid]->files[data_info[block_steamid].upfile.name]["type"] =
            data_info[block_steamid].upfile.type;
        http_data[block_steamid]->files[data_info[block_steamid].upfile.name]["size"] =
            data_info[block_steamid].upfile.size;
        http_data[block_steamid]->files[data_info[block_steamid].upfile.name]["error"] =
            data_info[block_steamid].upfile.error;
    }
    data_info[block_steamid].upfile.filename.clear();
    data_info[block_steamid].upfile.name.clear();
    data_info[block_steamid].upfile.tempfile.clear();
    data_info[block_steamid].upfile.type.clear();
    data_info[block_steamid].upfile.size  = 0;
    data_info[block_steamid].upfile.error = 0;
}

void http2parse::readformfieldfilecontent(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize)
{
    unsigned int pmi = 0, ib = 0, isbound = 0;
    if (data_info[block_steamid].match_offset > 0 || data_info[block_steamid].buffer_key.size() > 0)
    {
        pmi = begin, ib = data_info[block_steamid].match_offset;
        if (data_info[block_steamid].buffer_key.size() < 5)
        {
            pmi = pmi + (4 - data_info[block_steamid].buffer_key.size());
            ib  = 0;
        }
        data_info[block_steamid].match_offset = 0;
        for (; ib < data_info[block_steamid].boundary.size(); ib++)
        {
            if (buffer[pmi] != data_info[block_steamid].boundary[ib])
            {
                break;
            }
            pmi++;
        }
        if (ib == data_info[block_steamid].boundary.size())
        {

            if (data_info[block_steamid].uprawfile)
            {
                // fclose(data_info[block_steamid].uprawfile);
                data_info[block_steamid].uprawfile.reset(nullptr);
            }
            procssformfile();
            data_info[block_steamid].buffer_key.clear();
            data_info[block_steamid].buffer_value.clear();
            if (buffer[pmi] == '-' && buffer[pmi + 1] == '-')
            {
                pmi += 2;
                http_data[block_steamid]->isfinish = true;

                if (buffer[pmi] == 0x0D)
                {
                    pmi++;
                }
                if (buffer[pmi] == 0x0A)
                {
                    pmi++;
                }
                begin                                  = pmi;
                data_info[block_steamid].postfieldtype = 2;
                stream_list.emplace(block_steamid);
                return;
            }
            if (buffer[pmi] == 0x0D)
            {
                pmi++;
            }
            if (buffer[pmi] == 0x0A)
            {
                pmi++;
            }
            begin                                  = pmi;
            data_info[block_steamid].postfieldtype = 2;

            return;
        }
        if (data_info[block_steamid].uprawfile)
        {
            data_info[block_steamid].upfile.size += data_info[block_steamid].buffer_key.size();
            fwrite(&data_info[block_steamid].buffer_key[0],
                   1,
                   data_info[block_steamid].buffer_key.size(),
                   data_info[block_steamid].uprawfile.get());
        }
        data_info[block_steamid].buffer_key.clear();
    }
    unsigned int baseoffset = begin;
    pmi = 0, ib = 0;
    data_info[block_steamid].match_offset = 0;

    for (; begin < buffersize; begin++)
    {
        if (buffer[begin] == 0x0D)
        {

            data_info[block_steamid].buffer_key.clear();
            data_info[block_steamid].buffer_key.push_back(0x0D);
            unsigned int j = begin + 1;
            isbound        = 0;
            if (j < buffersize && buffer[j] == 0x0A)
            {

                j++;

                data_info[block_steamid].buffer_key.push_back(0x0A);
                if (j < buffersize && buffer[j] == 0x2D)
                {
                    j++;

                    data_info[block_steamid].buffer_key.push_back(0x2D);
                    if (j < buffersize && buffer[j] == 0x2D)
                    {
                        j++;

                        data_info[block_steamid].buffer_key.push_back(0x2D);
                        for (unsigned int nnn = 0; nnn < data_info[block_steamid].boundary.size(); nnn++)
                        {
                            if (buffer[j] != data_info[block_steamid].boundary[nnn])
                            {
                                isbound = 2;
                                break;
                            }
                            j += 1;
                            data_info[block_steamid].buffer_key.push_back(buffer[j]);
                            if (j >= buffersize)
                            {
                                data_info[block_steamid].match_offset = nnn + 1;
                                isbound                               = 1;
                                break;
                            }
                        }
                        if (isbound == 0)
                        {

                            data_info[block_steamid].match_offset  = 0;
                            data_info[block_steamid].postfieldtype = 2;

                            if (data_info[block_steamid].uprawfile)
                            {
                                data_info[block_steamid].upfile.size += (begin - baseoffset);
                                fwrite(&buffer[baseoffset],
                                       1,
                                       (begin - baseoffset),
                                       data_info[block_steamid].uprawfile.get());
                                // fclose(data_info[block_steamid].uprawfile);
                                data_info[block_steamid].uprawfile.reset(nullptr);
                                procssformfile();
                            }
                            data_info[block_steamid].buffer_key.clear();
                            data_info[block_steamid].buffer_value.clear();

                            // i++;
                            if (buffer[begin] == 0x2D && buffer[begin + 1] == 0x2D)
                            {
                                http_data[block_steamid]->isfinish = true;
                                begin += 2;
                                stream_list.emplace(block_steamid);
                                // return;
                            }
                            if (buffer[begin] == 0x0D && begin < buffersize)
                            {
                                begin++;
                            }
                            if (buffer[begin] == 0x0A)
                            {
                                begin++;
                            }

                            return;
                        }
                    }
                }
            }
            if (j >= buffersize)
            {
                // i += 1;
                break;
            }
            data_info[block_steamid].buffer_key.clear();
            isbound = 0;
        }
    }
    if (data_info[block_steamid].uprawfile)
    {
        data_info[block_steamid].upfile.size += (begin - baseoffset);
        fwrite(&buffer[baseoffset], 1, (begin - baseoffset), data_info[block_steamid].uprawfile.get());
    }
    begin = buffersize;
}

void http2parse::readformfieldcontent(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize)
{
    unsigned int pmi = 0, ib = 0;
    if (data_info[block_steamid].match_offset > 0 || data_info[block_steamid].buffer_key.size() > 0)
    {

        pmi = begin, ib = data_info[block_steamid].match_offset;

        if (data_info[block_steamid].buffer_key.size() < 5)
        {
            pmi = pmi + (4 - data_info[block_steamid].buffer_key.size());
            ib  = 0;
        }

        data_info[block_steamid].match_offset = 0;
        for (; ib < data_info[block_steamid].boundary.size(); ib++)
        {
            if (buffer[pmi] != data_info[block_steamid].boundary[ib])
            {
                break;
            }
            pmi++;
            if (pmi >= buffersize)
            {
                break;
            }
        }
        if (ib == data_info[block_steamid].boundary.size())
        {
            if (buffer[pmi] == '-' && buffer[pmi + 1] == '-')
            {
                pmi += 2;

                if (data_info[block_steamid].uprawfile)
                {
                    fclose(data_info[block_steamid].uprawfile.get());
                }
                data_info[block_steamid].buffer_key = data_info[block_steamid].fieldname;
                procssxformurlencoded();
                http_data[block_steamid]->isfinish = true;
                data_info[block_steamid].buffer_key.clear();
                data_info[block_steamid].field_value.clear();
                if (buffer[pmi] == 0x0D)
                {
                    pmi++;
                }
                if (buffer[pmi] == 0x0A)
                {
                    pmi++;
                }
                begin                                  = pmi;
                data_info[block_steamid].postfieldtype = 2;
                stream_list.emplace(block_steamid);
                return;
            }
            if (buffer[pmi] == 0x0D)
            {
                pmi++;
            }
            if (buffer[pmi] == 0x0A)
            {
                pmi++;
            }
            begin                                  = pmi;
            data_info[block_steamid].postfieldtype = 2;
            return;
        }

        data_info[block_steamid].field_value.append(data_info[block_steamid].buffer_key);
        data_info[block_steamid].buffer_key.clear();
    }
    unsigned baseoffset = begin;
    // bool islastmatch = false;
    pmi = 0, ib = 0;
    for (; begin < buffersize; begin++)
    {
        if (buffer[begin] == 0x0D)
        {
            // islastmatch = false;
            pmi = begin + 1;
            ib  = 0;

            data_info[block_steamid].buffer_key.clear();
            data_info[block_steamid].buffer_key.push_back(0x0D);

            char splitchar[] = {0x0A, 0x2D, 0x2D};
            if (pmi < buffersize)
            {
                for (int ik = 0; ik < 3; ik++)
                {
                    if (splitchar[ik] != buffer[pmi])
                    {
                        ib = 1;
                        break;
                    }

                    data_info[block_steamid].buffer_key.push_back(splitchar[ik]);
                    pmi += 1;
                    if (pmi >= buffersize)
                    {
                        ib = 1;
                        break;
                    }
                }
            }

            if (ib == 0)
            {
                data_info[block_steamid].match_offset = 0;
                for (; ib < data_info[block_steamid].boundary.size(); ib++)
                {
                    if (buffer[pmi] != data_info[block_steamid].boundary[ib])
                    {
                        break;
                    }
                    data_info[block_steamid].buffer_key.push_back(buffer[pmi]);
                    pmi++;
                    if (pmi >= buffersize)
                    {
                        // islastmatch = true;
                        data_info[block_steamid].match_offset = ib + 1;
                        break;
                    }
                }
                if (ib == data_info[block_steamid].boundary.size())
                {
                    data_info[block_steamid].match_offset = 0;
                    data_info[block_steamid].buffer_key.clear();
                    data_info[block_steamid].field_value.append((char *)&buffer[baseoffset], (begin - baseoffset));
                    data_info[block_steamid].buffer_key = data_info[block_steamid].fieldname;
                    procssxformurlencoded();
                    data_info[block_steamid].buffer_key.clear();
                    data_info[block_steamid].field_value.clear();

                    if (buffer[pmi] == '-' && buffer[pmi + 1] == '-')
                    {
                        pmi += 2;
                        http_data[block_steamid]->isfinish = true;

                        if (buffer[pmi] == 0x0D)
                        {
                            pmi++;
                        }
                        if (buffer[pmi] == 0x0A)
                        {
                            pmi++;
                        }
                        begin                                  = pmi;
                        data_info[block_steamid].postfieldtype = 2;
                        stream_list.emplace(block_steamid);
                        return;
                    }
                    if (buffer[pmi] == 0x0D)
                    {
                        pmi++;
                    }
                    if (buffer[pmi] == 0x0A)
                    {
                        pmi++;
                    }
                    begin                                  = pmi;
                    data_info[block_steamid].postfieldtype = 2;
                    return;
                }
            }
            // doubtful mactch
            if (pmi >= buffersize)
            {
                // begin +=1;
                pmi = 0;
                break;
            }
            data_info[block_steamid].buffer_key.clear();
        }
    }
    data_info[block_steamid].field_value.append((char *)&buffer[baseoffset], (begin - baseoffset));
    begin = buffersize;
}
void http2parse::fieldname_process(const std::string &tfheader_name, std::string &fieldname, std::string &fieldfilename)
{
    std::string header_temp;
    for (unsigned int jj = 0; jj < tfheader_name.size(); jj++)
    {
        if (tfheader_name[jj] == ';')
        {
            header_temp.clear();
            continue;
        }
        if (tfheader_name[jj] == 0x20)
        {
            header_temp.clear();
            continue;
        }
        if (tfheader_name[jj] == 0x3D)
        {
            if (header_temp.size() == 4 && strcasecmp(header_temp.c_str(), "name") == 0)
            {
                unsigned int mm = jj;
                mm += 1;
                if (tfheader_name[mm] == '"')
                {
                    mm += 1;
                }
                fieldname.clear();
                for (; mm < tfheader_name.size(); mm++)
                {
                    if (tfheader_name[mm] == '"')
                    {
                        break;
                    }
                    fieldname.push_back(tfheader_name[mm]);
                }
                if (fieldname.size() > 0)
                {
                    if (data_info[block_steamid].postfieldtype < 3)
                    {
                        data_info[block_steamid].postfieldtype = 4;
                    }
                }

                if (tfheader_name[mm] == '"')
                {
                    mm += 1;
                }
                jj = mm;
            }
            else if (header_temp.size() == 8 && strcasecmp(header_temp.c_str(), "filename") == 0)
            {

                unsigned int mm = jj;
                mm += 1;
                if (tfheader_name[mm] == '"')
                {
                    mm += 1;
                }
                fieldfilename.clear();
                for (; mm < tfheader_name.size(); mm++)
                {
                    if (tfheader_name[mm] == '"')
                    {
                        break;
                    }
                    fieldfilename.push_back(tfheader_name[mm]);
                }

                if (data_info[block_steamid].postfieldtype < 5)
                {
                    data_info[block_steamid].postfieldtype = 6;
                }

                if (tfheader_name[mm] == '"')
                {
                    mm += 1;
                }
                jj = mm;
            }
            header_temp.clear();
            continue;
        }
        header_temp.push_back(tfheader_name[jj]);
    }
}
void http2parse::fieldtype_process(const std::string &tfheader_name)
{

    unsigned int jj = 0;
    for (; jj < tfheader_name.size(); jj++)
    {
        if (tfheader_name[jj] == ':')
        {
            jj++;
            break;
        }
    }

    for (; jj < tfheader_name.size(); jj++)
    {
        if (tfheader_name[jj] != 0x20)
        {
            break;
        }
    }
    data_info[block_steamid].upfile.type.clear();
    for (; jj < tfheader_name.size(); jj++)
    {
        if (tfheader_name[jj] == 0x20 || tfheader_name[jj] == 0x0D || tfheader_name[jj] == ';')
        {
            break;
        }
        data_info[block_steamid].upfile.type.push_back(tfheader_name[jj]);
    }
}
void http2parse::readformfilename(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize)
{
    unsigned int i       = begin;
    unsigned int isbound = 0;

    // defence code
    if (headendhitnum > 0)
    {
        if (headendhitnum == 2)
        {
            if (buffer[i] == 0x0D && i < buffersize && headendhitnum < 4)
            {
                i++;
                headendhitnum++;
                if (buffer[i] == 0x0A && i < buffersize && headendhitnum < 4)
                {
                    i++;
                    headendhitnum++;
                }
            }
        }
        else
        {
            if (buffer[i] == 0x0A && i < buffersize && headendhitnum < 4)
            {
                i++;
                headendhitnum++;
                if (buffer[i] == 0x0D && i < buffersize && headendhitnum < 4)
                {
                    i++;
                    headendhitnum++;
                    if (buffer[i] == 0x0A && i < buffersize && headendhitnum < 4)
                    {
                        i++;
                        headendhitnum++;
                    }
                }
            }
        }
    }

    if (headendhitnum > 3)
    {
        isbound = 1;
    }
    else
    {
        i = begin;
    }
    headendhitnum = 0;

    if (isbound != 1)
    {
        // find header info

        for (; i < buffersize; i++)
        {
            if (buffer[i] == 0x0D)
            {
                headendhitnum = 1;
                if (buffer[i + 1] == 0x0A && i < buffersize)
                {

                    headendhitnum++;
                    if (buffer[i + 2] == 0x0D && i < buffersize)
                    {

                        headendhitnum++;
                        if (buffer[i + 3] == 0x0A && i < buffersize)
                        {

                            headendhitnum++;
                        }
                    }
                }

                if (headendhitnum == 4)
                {
                    isbound       = 1;
                    i             = i + 4;
                    headendhitnum = 0;
                    break;
                }
                else
                {
                    headendhitnum = 0;
                }
            }

            data_info[block_steamid].buffer_key.push_back(buffer[i]);
        }
    }
    begin = i;
    if (isbound == 1)
    {
        std::string fieldname;
        std::string filesname;
        std::string fieldheader_temp;
        unsigned int jj         = 0;
        unsigned char fieldtype = 0;
        for (; jj < data_info[block_steamid].buffer_key.size(); jj++)
        {
            if (data_info[block_steamid].buffer_key[jj] == 0x0D || data_info[block_steamid].buffer_key[jj] == 0x0A)
            {
                if (fieldheader_temp.size() > 3)
                {

                    if (fieldtype == 1)
                    {
                        fieldname_process(fieldheader_temp, fieldname, filesname);
                    }
                    else if (fieldtype == 2)
                    {
                        fieldtype_process(fieldheader_temp);
                    }
                }
                fieldtype = 0;
                fieldheader_temp.clear();
                continue;
            }
            if (data_info[block_steamid].buffer_key[jj] == ':')
            {
                if (fieldheader_temp.size() == 19 && strcasecmp(fieldheader_temp.c_str(), "Content-Disposition") == 0)
                {
                    fieldtype = 1;
                }
                else if (fieldheader_temp.size() == 12 && strcasecmp(fieldheader_temp.c_str(), "Content-Type") == 0)
                {
                    fieldtype = 2;
                }
            }
            fieldheader_temp.push_back(data_info[block_steamid].buffer_key[jj]);
        }
        if (fieldtype > 0)
        {
            if (fieldtype == 1)
            {
                fieldname_process(fieldheader_temp, fieldname, filesname);
            }
            else if (fieldtype == 2)
            {
                fieldtype_process(fieldheader_temp);
            }
            fieldtype = 0;
            fieldheader_temp.clear();
        }
        fieldheader_temp.clear();
        // check upload file
        if (data_info[block_steamid].postfieldtype == 6)
        {
            server_loaclvar &localvar                = get_server_global_var();
            data_info[block_steamid].upfile.filename = filesname;
            data_info[block_steamid].upfile.name     = fieldname;

            fieldheader_temp = data_info[block_steamid].upfile.filename + std::to_string(http::timeid()) + fieldname;

            data_info[block_steamid].upfile.tempfile = localvar.temp_path;// + "temp/";
            data_info[block_steamid].upfile.tempfile.append(std::to_string(std::hash<std::string>{}(fieldheader_temp)));

            DEBUG_LOG("filename:%s", data_info[block_steamid].upfile.tempfile.c_str());
            // data_info[block_steamid].uprawfile = fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb");
            data_info[block_steamid].uprawfile.reset(fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb"));
            if (!data_info[block_steamid].uprawfile)
            {
                data_info[block_steamid].upfile.tempfile.append("_t");
                // data_info[block_steamid].uprawfile = fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb");
                data_info[block_steamid].uprawfile.reset(fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb"));
                if (!data_info[block_steamid].uprawfile)
                {
                    error = 303;
                }
            }
            data_info[block_steamid].upfile.size = 0;
        }
        data_info[block_steamid].buffer_key.clear();
        data_info[block_steamid].buffer_value.clear();
        data_info[block_steamid].fieldname = fieldname;
    }
}

void http2parse::readboundaryline(const unsigned char *buffer, unsigned int &begin, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readboundaryline:%ul\n", buffersize);
    unsigned int ni = 0, baseoffset = begin;
    if (buffer[begin] == '-' && buffer[begin + 1] == '-')
    {
        begin += 2;
    }
    for (; ni < data_info[block_steamid].boundary.size(); ni++)
    {
        if (buffer[begin] != data_info[block_steamid].boundary[ni])
        {
            break;
        }
        begin++;
    }
    if (ni == data_info[block_steamid].boundary.size())
    {
        if (buffer[begin] == 0x0D)
        {
            begin++;
        }
        if (buffer[begin] == 0x0A)
        {
            begin++;
        }
        if ((begin - baseoffset) == (ni + 4))
        {
            data_info[block_steamid].postfieldtype = 2;
        }
    }
    else
    {
        error = 201;
    }
}
void http2parse::readmultipartformdata(const unsigned char *buffer, unsigned int buffersize)
{

    for (unsigned int i = 0; i < buffersize;)
    {
        if (error > 0)
        {
            return;
        }
        switch (data_info[block_steamid].postfieldtype)
        {
        case 0:
            // begin post content
            readboundaryline(buffer, i, buffersize);
            data_info[block_steamid].buffer_key.clear();
            data_info[block_steamid].buffer_value.clear();
            data_info[block_steamid].field_value.clear();
            break;
        case 2: readformfilename(buffer, i, buffersize); break;
        case 4: readformfieldcontent(buffer, i, buffersize); break;
        case 6: readformfieldfilecontent(buffer, i, buffersize); break;
        default: i++;
        }
    }
}

void http2parse::readrawfileformdata(const unsigned char *buffer, unsigned int buffersize)
{
    if (!data_info[block_steamid].uprawfile)
    {
        server_loaclvar &localvar                = get_server_global_var();
        data_info[block_steamid].upfile.filename = "rawcontent";
        data_info[block_steamid].upfile.name     = "rawcontent";

        std::string fieldheader_temp =
            data_info[block_steamid].upfile.filename + std::to_string(http::timeid()) + "rawcontent";

        data_info[block_steamid].upfile.tempfile = localvar.temp_path;// + "temp/";
        data_info[block_steamid].upfile.tempfile.append(std::to_string(std::hash<std::string>{}(fieldheader_temp)));
        // data_info[block_steamid].uprawfile = fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb");
        data_info[block_steamid].uprawfile.reset(fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb"));
        if (!data_info[block_steamid].uprawfile)
        {
            data_info[block_steamid].upfile.tempfile.append("_t");
            // data_info[block_steamid].uprawfile = fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb");
            data_info[block_steamid].uprawfile.reset(fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb"));
            if (!data_info[block_steamid].uprawfile)
            {
                error = 303;
            }
        }
        data_info[block_steamid].upfile.type = http_data[block_steamid]->content_type;
        data_info[block_steamid].upfile.size = 0;
    }
    if (data_info[block_steamid].uprawfile)
    {
        data_info[block_steamid].upfile.size += buffersize;
        fwrite(buffer, 1, buffersize, data_info[block_steamid].uprawfile.get());
    }

    if (http_data[block_steamid]->content_length < (data_info[block_steamid].upfile.size + 2))
    {
        if (data_info[block_steamid].uprawfile)
        {
            fclose(data_info[block_steamid].uprawfile.get());
        }
        procssformfile();
    }
}

void http2parse::data_process(unsigned int data_stream_id)
{
    unsigned int j = 0, w_size = stream_data[block_steamid].size();

    if (data_info[data_stream_id].padded)
    {
        j += 1;
        w_size -= 1;
        w_size -= data_info[data_stream_id].pad_length;
    }

    if (http_data[block_steamid]->compress == 10)
    {
        if (http_data[block_steamid]->content_length < 16777216)
        {
            if (http_data[block_steamid]->output.size() < http_data[block_steamid]->content_length)
            {
                http_data[block_steamid]->output.append(&stream_data[block_steamid][j], w_size);
            }
        }
    }
    else
    {
        switch (data_info[data_stream_id].posttype)
        {
        case 1:
            // x-www-form-urlencoded
            data_info[block_steamid].buffer_value.append(&stream_data[block_steamid][j], w_size);
            if (data_info[block_steamid].buffer_value.size() == (unsigned int)http_data[block_steamid]->content_length)
            {
                readformurlencoded();
                data_info[block_steamid].buffer_value.clear();
            }
            break;
        case 2:
            // multipart/form-data
            readmultipartformdata((const unsigned char *)&stream_data[block_steamid][j], w_size);
            break;
        case 3:
            // json
            http_data[block_steamid]->rawcontent.append(&stream_data[block_steamid][j], w_size);
            if (http_data[block_steamid]->rawcontent.size() == (unsigned int)http_data[block_steamid]->content_length)
            {
                http_data[block_steamid]->json.from_json(http_data[block_steamid]->rawcontent);
            }
            break;
        case 4:
            // xml
            // readrawfileformdata((const unsigned char *)&stream_data[block_steamid][j], w_size);
            http_data[block_steamid]->rawcontent.append(&stream_data[block_steamid][j], w_size);
            break;
        case 5:
            // octet-stream
            readrawfileformdata((const unsigned char *)&stream_data[block_steamid][j], w_size);
            break;
        }
    }

    stream_data[block_steamid].clear();

    if (data_info[data_stream_id].endstream)
    {
        isfinsish     = 2;
        processheader = 0;

        if (data_info[block_steamid].uprawfile)
        {
            fclose(data_info[block_steamid].uprawfile.get());
        }

        if (!http_data[block_steamid]->isfinish)
        {

            http_data[block_steamid]->isfinish     = true;
            data_info[block_steamid].postfieldtype = 0;
            stream_list.emplace(block_steamid);
        }
    }
}

void http2parse::readdatablock(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int j = readoffset;

    unsigned int buffer_short_length;
    unsigned int block_short_length;
    unsigned int short_loop_max;

    buffer_short_length = buffersize - readoffset;
    block_short_length  = data_info[block_steamid].length - data_info[block_steamid].curnum;

    if (buffer_short_length <= block_short_length)
    {
        short_loop_max = buffer_short_length;
    }
    else
    {
        short_loop_max = block_short_length;
    }

    stream_data[block_steamid].append((const char *)&buffer[j], short_loop_max);
    data_info[block_steamid].curnum += short_loop_max;

    window_update_recv_num -= short_loop_max;

    if (window_update_recv_num * 2 < RECV_WINDOW_UPDATE_NUM)
    {
        peer_session->recv_window_update(RECV_WINDOW_UPDATE_NUM - window_update_recv_num, block_steamid);
        window_update_recv_num = RECV_WINDOW_UPDATE_NUM;
    }
    readoffset = readoffset + short_loop_max;
    // 数据块处理完成
    if (data_info[block_steamid].curnum == data_info[block_steamid].length)
    {
        DEBUG_LOG("data_process block:%llu", data_info[block_steamid].length);
        data_process(block_steamid);
        processheader = 0;
    }
}
void http2parse::readblock(const unsigned char *buffer, unsigned int buffersize)
{

    if (processheader == 0)
    {
        processblockheader(buffer, buffersize);
    }
    switch (frame_type)
    {
    case 0x00:
        // DATA
        readdatablock(buffer, buffersize);

        break;
    case 0x01:
        // HEADERS
        readheaders(buffer, buffersize);

        break;
    case 0x02:
        // PRIORITY
        readpriority(buffer, buffersize);

        break;
    case 0x03:
        // RST_STREAM
        readrst_stream(buffer, buffersize);

        break;
    case 0x04:
        // SETTINGS
        readsetting(buffer, buffersize);

        break;

    case 0x05:
        // PUSH_PROMISE
        readsubdata(buffer, buffersize);

        break;
    case 0x06:
        // PING
        readping(buffer, buffersize);

        break;
    case 0x07:
        // GOAWAY
        readgoaway(buffer, buffersize);

        break;
    case 0x08:
        // WINDOW_UPDATE
        readwinupdate(buffer, buffersize);

        break;
    case 0x09:
        // CONTINUATION
        readcontinuation(buffer, buffersize);

        break;
    default: readsubdata(buffer, buffersize);
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
        readblock(buffer, buffersize);
        if (error > 0)
        {
            break;
        }
    }
}
}// namespace http
