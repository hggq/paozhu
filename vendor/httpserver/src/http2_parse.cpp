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

    auto dataiter = data_info.find(block_steamid);
    if (dataiter == data_info.end())
    {
        block_data_info_ptr = std::make_shared<http2_data_t>();
        data_info.emplace(block_steamid, block_data_info_ptr);
    }
    else
    {
        if (block_data_info_ptr->stream_id != block_steamid)
        {
            block_data_info_ptr = dataiter->second;
        }
    }

    if (frame_type == 0x00)
    {
        block_data_info_ptr->length    = blocklength;
        block_data_info_ptr->curnum    = 0;
        block_data_info_ptr->endstream = ((flag_type & 0x01) == 0x01) ? true : false;
        block_data_info_ptr->padded    = ((flag_type & 0x08) == 0x08) ? true : false;
        // padded_length;
        if (block_data_info_ptr->padded)
        {
            if (j < buffersize)
            {
                block_data_info_ptr->pad_length = (unsigned char)buffer[j];
            }
        }
        //data_steam_id = block_steamid;
        auto peeriter = http_data.find(block_steamid);
        if (peeriter == http_data.end())
        {
            error = 403;
            return;
        }
        block_steam_httppeer = peeriter->second;
    }
    else if (frame_type == 0x01)
    {
        block_data_info_ptr->length    = blocklength;
        block_data_info_ptr->curnum    = 0;
        block_data_info_ptr->endstream = ((flag_type & 0x01) == 0x01) ? true : false;
        block_data_info_ptr->padded    = ((flag_type & 0x08) == 0x08) ? true : false;

        block_data_info_ptr->endheader = ((flag_type & 0x04) == 0x04) ? true : false;
        block_data_info_ptr->priority  = ((flag_type & 0x20) == 0x20) ? true : false;

        if (block_data_info_ptr->padded)
        {
            if (j < buffersize)
            {
                block_data_info_ptr->pad_length = (unsigned char)buffer[j];
            }
            j += 1;
        }
        if (block_data_info_ptr->priority)
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
                block_data_info_ptr->stream_id = bsteamid;

                block_data_info_ptr->weight = (unsigned char)buffer[j];
                j++;
            }
        }

        //data_steam_id = block_steamid;
        //stream_data[block_steamid].clear();
        auto streamdataiter = stream_data.find(block_steamid);
        if (streamdataiter == stream_data.end())
        {
            stream_data_ptr = std::make_shared<std::string>();
            stream_data.emplace(block_steamid, stream_data_ptr);
        }
        else
        {
            stream_data_ptr = streamdataiter->second;
        }
        stream_data_ptr->clear();
    }

    readoffset    = readoffset + 9;
    processheader = 1;
}
void http2parse::clsoesend(asio::io_context &ioc)
{
    for (auto iter = http_data.begin(); iter != http_data.end();)
    {
        if (iter->second->issend)
        {
            try
            {
                iter->second->isclose = true;
                iter->second->clsoesend(ioc);
            }
            catch (const std::exception &e)
            {
                DEBUG_LOG("http2parse user_code_handler_call error");
            }
        }
        iter++;
    }
}
void http2parse::readheaders(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int buffer_short_length;
    unsigned int block_short_length;

    buffer_short_length = buffersize - readoffset;
    block_short_length  = block_data_info_ptr->length - block_data_info_ptr->curnum;

    if (buffer_short_length <= block_short_length)
    {
        block_short_length = buffer_short_length;
    }

    //stream_data[block_steamid].append((const char *)&buffer[readoffset], block_short_length);
    stream_data_ptr->append((const char *)&buffer[readoffset], block_short_length);
    block_data_info_ptr->curnum += block_short_length;

    readoffset = readoffset + block_short_length;
    if (block_data_info_ptr->curnum > 8192)
    {
        error = 403;
    }
    // 数据块处理完成
    if (block_data_info_ptr->curnum == block_data_info_ptr->length)
    {
        if (block_data_info_ptr->endheader)
        {
            bool ishasold = true;
            for (auto iter = http_data.begin(); iter != http_data.end();)
            {
                if (iter->second->issend)
                {
                    block_steam_httppeer                          = iter->second;
                    unsigned int oldstream_id                     = iter->first;
                    iter                                          = http_data.erase(iter);
                    block_steam_httppeer->state.gzip              = false;
                    block_steam_httppeer->state.deflate           = false;
                    block_steam_httppeer->state.br                = false;
                    block_steam_httppeer->state.avif              = false;
                    block_steam_httppeer->state.webp              = false;
                    block_steam_httppeer->state.keepalive         = false;
                    block_steam_httppeer->state.websocket         = false;
                    block_steam_httppeer->state.upgradeconnection = false;
                    block_steam_httppeer->state.rangebytes        = false;
                    block_steam_httppeer->state.language[0]       = {0};
                    block_steam_httppeer->state.version           = 0;
                    block_steam_httppeer->state.port              = 0;
                    block_steam_httppeer->state.ifmodifiedsince   = 0;
                    block_steam_httppeer->state.rangebegin        = 0;
                    block_steam_httppeer->state.rangeend          = 0;
                    block_steam_httppeer->upload_length           = 0;
                    block_steam_httppeer->content_length          = 0;
                    block_steam_httppeer->keepalive               = false;
                    block_steam_httppeer->issend                  = false;
                    block_steam_httppeer->isclose                 = false;
                    block_steam_httppeer->send_header.clear();
                    block_steam_httppeer->send_cookie_lists.clear();
                    block_steam_httppeer->http2_send_header.clear();
                    block_steam_httppeer->header.clear();
                    block_steam_httppeer->pathinfos.clear();
                    block_steam_httppeer->querystring.clear();
                    block_steam_httppeer->urlpath.clear();
                    block_steam_httppeer->host.clear();
                    block_steam_httppeer->etag.clear();
                    block_steam_httppeer->output.clear();
                    block_steam_httppeer->val.clear();
                    block_steam_httppeer->post.clear();
                    block_steam_httppeer->session.clear();
                    block_steam_httppeer->get.clear();
                    block_steam_httppeer->files.clear();
                    block_steam_httppeer->json.clear();
                    block_steam_httppeer->cookie.clear();
                    block_steam_httppeer->rawcontent.clear();
                    block_steam_httppeer->httpv    = 2;
                    block_steam_httppeer->isso     = false;
                    block_steam_httppeer->compress = 0;
                    // peer->websocket.deflate           = false;
                    // peer->websocket.permessagedeflate = false;
                    // peer->websocket.perframedeflate   = false;
                    // peer->websocket.deflateframe      = false;
                    // peer->websocket.isopen            = false;
                    // peer->websocket.version           = 0x00;
                    // peer->websocket.key.clear();
                    // peer->websocket.ext.clear();

                    http_data.emplace(block_steamid, block_steam_httppeer);
                    ishasold = false;

                    auto dataiter = data_info.find(oldstream_id);
                    if (dataiter == data_info.end())
                    {
                        block_data_info_ptr = std::make_shared<http2_data_t>();
                        data_info.emplace(block_steamid, block_data_info_ptr);
                    }
                    else
                    {
                        block_data_info_ptr                 = dataiter->second;
                        dataiter                            = data_info.erase(dataiter);
                        block_data_info_ptr->stream_id      = block_steamid;
                        block_data_info_ptr->match_offset   = 0;
                        block_data_info_ptr->isbegin        = false;
                        block_data_info_ptr->isend          = false;
                        block_data_info_ptr->endstream      = false;
                        block_data_info_ptr->endheader      = false;
                        block_data_info_ptr->padded         = false;
                        block_data_info_ptr->priority       = false;
                        block_data_info_ptr->posttype       = 0;
                        block_data_info_ptr->postfieldtype  = 0;
                        block_data_info_ptr->changetype     = 0;
                        block_data_info_ptr->pad_length     = 0;
                        block_data_info_ptr->weight         = 0;
                        block_data_info_ptr->length         = 0;// bloack data length;
                        block_data_info_ptr->curnum         = 0;// now block length
                        block_data_info_ptr->content_length = 0;// post length

                        //std::FILE *uprawfile = NULL;
                        block_data_info_ptr->uprawfile = nullptr;

                        block_data_info_ptr->boundary.clear();
                        block_data_info_ptr->fieldname.clear();
                        block_data_info_ptr->buffer_key.clear();
                        block_data_info_ptr->buffer_value.clear();
                        block_data_info_ptr->field_value.clear();
                        block_data_info_ptr->priority_lists.clear();
                        block_data_info_ptr->upfile.name.clear();
                        block_data_info_ptr->upfile.filename.clear();
                        block_data_info_ptr->upfile.tempfile.clear();
                        block_data_info_ptr->upfile.type.clear();
                        block_data_info_ptr->upfile.size  = 0;
                        block_data_info_ptr->upfile.error = 0;
                        data_info.emplace(block_steamid, block_data_info_ptr);
                    }
                    break;
                }
                ++iter;
            }
            if (ishasold)
            {
                block_steam_httppeer = std::make_shared<httppeer>();
                http_data.emplace(block_steamid, block_steam_httppeer);
            }
            headers_parse();
            if (ispost)
            {
                error = block_steam_httppeer->check_upload_limit();
            }
            block_steam_httppeer->isuse_fastcgi();
        }
        processheader = 0;
    }
}
void http2parse::headers_parse()
{
    //std::string_view setting_data(stream_data[block_steamid]);
    std::string_view setting_data(stream_data_ptr->data(), stream_data_ptr->size());
    if (setting_data.size() == 0)
    {
        error = 101;
        return;
    }
    unsigned int begin = 0, header_end = setting_data.size();
    // peer_session->stream[block_steamid].content_length = 0;
    block_steam_httppeer->content_length = 0;
    block_steam_httppeer->stream_id      = block_steamid;
    block_steam_httppeer->httpv          = 2;
    block_steam_httppeer->socket_session = peer_session;
    //http_data[block_steamid]->window_update_bool = false;

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
        DEBUG_LOG("http2 post client: %s %ud", block_steam_httppeer->url.c_str(), block_steamid);
        window_update_recv_num = RECV_WINDOW_UPDATE_NUM;
        peer_session->send_window_update(window_update_recv_num, block_steamid);
        block_data_info_ptr->stream_id      = block_steamid;
        block_data_info_ptr->curnum         = 0;
        block_data_info_ptr->length         = 0;
        block_data_info_ptr->content_length = block_steam_httppeer->content_length;

        block_data_info_ptr->isbegin       = false;
        block_data_info_ptr->isend         = false;
        block_data_info_ptr->postfieldtype = 0;
        block_data_info_ptr->match_offset  = 0;
        block_steam_httppeer->isfinish     = false;
    }
    else
    {
        DEBUG_LOG("http2 get client: %s", block_steam_httppeer->url.c_str());
        block_steam_httppeer->isfinish = true;
        stream_list.emplace(block_steamid);
    }
    //stream_data[block_steamid].clear();
    stream_data_ptr->clear();
    block_data_info_ptr->buffer_value.clear();
}
void http2parse::cookie_process([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    DEBUG_LOG("cookie_process:%s:%s", header_name.c_str(), header_value.c_str());
    //http_data[block_steamid]->header[header_name] = header_value;
    unsigned int i = 0, linesize = header_value.size();
    block_data_info_ptr->buffer_key.clear();
    block_data_info_ptr->buffer_value.clear();

    if (block_steam_httppeer->header["Cookie"].empty())
    {
    }
    else
    {
        block_steam_httppeer->header["Cookie"].append("; ");
    }
    block_steam_httppeer->header["Cookie"].append(header_value);
    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x3D)
        {
            block_data_info_ptr->buffer_key = http::url_decode(block_data_info_ptr->buffer_value.data(),
                                                               block_data_info_ptr->buffer_value.length());
            block_data_info_ptr->buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3B)
        {
            block_data_info_ptr->buffer_value = http::url_decode(block_data_info_ptr->buffer_value.data(),
                                                                 block_data_info_ptr->buffer_value.length());
            block_steam_httppeer->cookie[block_data_info_ptr->buffer_key] =
                block_data_info_ptr->buffer_value;
            block_data_info_ptr->buffer_key.clear();
            block_data_info_ptr->buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        block_data_info_ptr->buffer_value.push_back(header_value[i]);
    }
    if (block_data_info_ptr->buffer_value.size() > 0)
    {
        block_data_info_ptr->buffer_value                             = http::url_decode(block_data_info_ptr->buffer_value.data(),
                                                             block_data_info_ptr->buffer_value.length());
        block_steam_httppeer->cookie[block_data_info_ptr->buffer_key] = block_data_info_ptr->buffer_value;
    }
    else
    {
        if (block_data_info_ptr->buffer_key.size() > 0)
        {
            block_steam_httppeer->cookie[block_data_info_ptr->buffer_key] = "";
        }
        block_data_info_ptr->buffer_key.clear();
    }
}
void http2parse::path_process([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    DEBUG_LOG("path_process:%s:%s", header_name.c_str(), header_value.c_str());
    block_data_info_ptr->buffer_key.clear();
    block_data_info_ptr->buffer_value.clear();
    unsigned char headerstep = 0;
    block_steam_httppeer->pathinfos.clear();
    unsigned int ioffset = 0, linesize = header_value.size();
    block_steam_httppeer->url = http::url_decode(header_value.data(), header_value.length());
    for (; ioffset < linesize; ioffset++)
    {
        if (header_value[ioffset] == 0x3F)
        {
            headerstep = 6;
            break;
        }
        if (header_value[ioffset] == 0x2F)
        {
            if (block_data_info_ptr->buffer_key.size() > 0)
            {
                if (block_data_info_ptr->buffer_key.size() == 2 && block_data_info_ptr->buffer_key[0] == '.' &&
                    block_data_info_ptr->buffer_key[1] == '.')
                {
                    if (block_steam_httppeer->pathinfos.size() > 0)
                    {
                        block_steam_httppeer->pathinfos.pop_back();
                    }
                }
                else if (block_data_info_ptr->buffer_key.size() == 1 &&
                         block_data_info_ptr->buffer_key[0] == '.')
                {
                }
                else
                {
                    block_steam_httppeer->pathinfos.emplace_back(
                        http::url_decode(block_data_info_ptr->buffer_key.data(),
                                         block_data_info_ptr->buffer_key.length()));
                }
                block_data_info_ptr->buffer_key.clear();
            }
        }
        else
        {
            block_data_info_ptr->buffer_key.push_back(header_value[ioffset]);
        }
        block_data_info_ptr->buffer_value.push_back(header_value[ioffset]);
    }

    if (block_data_info_ptr->buffer_key.size() > 0)
    {
        if (block_data_info_ptr->buffer_key.size() == 2 && block_data_info_ptr->buffer_key[0] == '.' &&
            block_data_info_ptr->buffer_key[1] == '.')
        {
            if (block_steam_httppeer->pathinfos.size() > 0)
            {
                block_steam_httppeer->pathinfos.pop_back();
            }
        }
        else if (block_data_info_ptr->buffer_key.size() == 1 && block_data_info_ptr->buffer_key[0] == '.')
        {
        }
        else
        {
            block_steam_httppeer->pathinfos.emplace_back(
                http::url_decode(block_data_info_ptr->buffer_key.data(),
                                 block_data_info_ptr->buffer_key.length()));
        }
    }

    if (headerstep == 6)
    {
        block_steam_httppeer->header["urlpath"] = block_data_info_ptr->buffer_value;
        block_steam_httppeer->urlpath           = http::url_decode(block_data_info_ptr->buffer_value.data(),
                                                         block_data_info_ptr->buffer_value.length());
    }
    else
    {
        block_steam_httppeer->header["urlpath"] = block_data_info_ptr->buffer_value;
        block_steam_httppeer->urlpath           = block_steam_httppeer->url;
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

    block_data_info_ptr->buffer_key.clear();
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
        block_data_info_ptr->buffer_key.append(&header_value[ioffset], (linesize - ioffset));
        //http_data[block_steamid]->header["querystring"] = data_info[block_steamid].buffer_key;
        block_steam_httppeer->querystring =
            http::url_decode(block_data_info_ptr->buffer_key.data(), block_data_info_ptr->buffer_key.length());

        block_data_info_ptr->buffer_key.clear();
        block_data_info_ptr->buffer_value.clear();
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
                block_data_info_ptr->buffer_key = http::url_decode(block_data_info_ptr->buffer_value.data(),
                                                                   block_data_info_ptr->buffer_value.length());
                block_data_info_ptr->buffer_value.clear();
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

                block_data_info_ptr->buffer_value =
                    http::url_decode(block_data_info_ptr->buffer_value.data(),
                                     block_data_info_ptr->buffer_value.length());
                procssparamter();
                block_data_info_ptr->buffer_key.clear();
                block_data_info_ptr->buffer_value.clear();
                headerstep = 2;
                continue;
            }
            block_data_info_ptr->buffer_value.push_back(header_value[ioffset]);
        }

        if (headerstep == 1)
        {
            // full kv
            block_data_info_ptr->buffer_value = http::url_decode(block_data_info_ptr->buffer_value.data(),
                                                                 block_data_info_ptr->buffer_value.length());
            procssparamter();
        }
        else if (headerstep == 2)
        {
            // half k
            block_data_info_ptr->buffer_key = http::url_decode(block_data_info_ptr->buffer_value.data(),
                                                               block_data_info_ptr->buffer_value.length());
            block_data_info_ptr->buffer_value.clear();
            procssparamter();
        }
        else if (block_data_info_ptr->buffer_value.size() > 0)
        {
            // only one k
            block_data_info_ptr->buffer_key = http::url_decode(block_data_info_ptr->buffer_value.data(),
                                                               block_data_info_ptr->buffer_value.length());
            block_data_info_ptr->buffer_value.clear();
            procssparamter();
        }
    }
}
void http2parse::procssparamter()
{
    std::string objname;
    bool isgroup = true;
    for (unsigned int j = 0; j < block_data_info_ptr->buffer_key.length(); j++)
    {
        if (block_data_info_ptr->buffer_key[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < block_data_info_ptr->buffer_key.length(); n++)
            {
                if (block_data_info_ptr->buffer_key[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (block_data_info_ptr->buffer_key[n] == '[')
                {

                    break;
                }
                if (block_data_info_ptr->buffer_key[n] != 0x22)
                {
                    key1name.push_back(block_data_info_ptr->buffer_key[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < block_data_info_ptr->buffer_key.length())
                {
                    if (block_data_info_ptr->buffer_key[m] == '[')
                    {
                        m += 1;
                        for (; m < block_data_info_ptr->buffer_key.length(); m++)
                        {
                            if (block_data_info_ptr->buffer_key[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (block_data_info_ptr->buffer_key[m] == '[')
                            {

                                break;
                            }
                            if (block_data_info_ptr->buffer_key[m] != 0x22)
                            {
                                key2name.push_back(block_data_info_ptr->buffer_key[m]);
                            }
                        }

                        if (ishaskey2 && m == block_data_info_ptr->buffer_key.length())
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
                            objtemp.push(block_data_info_ptr->buffer_value);
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            block_steam_httppeer->get[objname].push(std::move(objtemp1));
                        }
                        else
                        {
                            http::OBJ_VALUE objtemp;
                            objtemp[key2name] = block_data_info_ptr->buffer_value;
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            block_steam_httppeer->get[objname].push(std::move(objtemp1));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            block_steam_httppeer->get[objname][key1name].set_array();
                            block_steam_httppeer->get[objname][key1name] = block_data_info_ptr->buffer_value;
                        }
                        else
                        {

                            block_steam_httppeer->get[objname][key1name][key2name] =
                                block_data_info_ptr->buffer_value;
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == block_data_info_ptr->buffer_key.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        block_steam_httppeer->get[objname].set_array();
                        block_steam_httppeer->get[objname].push(block_data_info_ptr->buffer_value);
                    }
                    else
                    {
                        block_steam_httppeer->get[objname].push(key1name, block_data_info_ptr->buffer_value);
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
                objname.push_back(block_data_info_ptr->buffer_key[j]);
            }
        }
        else
        {
            objname.push_back(block_data_info_ptr->buffer_key[j]);
        }
    }
    if (isgroup)
    {
        block_steam_httppeer->get[block_data_info_ptr->buffer_key] = block_data_info_ptr->buffer_value;
    }
}
void http2parse::range_process([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    DEBUG_LOG("range_process:%s:%s", header_name.c_str(), header_value.c_str());
    unsigned int j = 0, linesize = header_value.size();
    block_data_info_ptr->buffer_value.clear();
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
        block_data_info_ptr->buffer_value.push_back(header_value[j]);
    }

    if (str_casecmp(block_data_info_ptr->buffer_value, "bytes"))
    {
        block_steam_httppeer->state.rangebytes = true;
    }
    block_steam_httppeer->state.rangebegin = 0;
    block_steam_httppeer->state.rangeend   = 0;
    block_data_info_ptr->buffer_value.clear();
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
            for (unsigned int qi = 0; qi < block_data_info_ptr->buffer_value.size(); qi++)
            {
                if (block_data_info_ptr->buffer_value[qi] < 0x3A &&
                    block_data_info_ptr->buffer_value[qi] > 0x2F)
                {
                    tm = tm * 10 + (block_data_info_ptr->buffer_value[qi] - 0x30);
                }
            }
            block_steam_httppeer->state.rangebegin = tm;
            block_data_info_ptr->buffer_value.clear();
            continue;
        }
        block_data_info_ptr->buffer_value.push_back(header_value[j]);
    }

    if (block_data_info_ptr->buffer_value.size() > 0)
    {
        long long tm = 0;
        for (unsigned int qi = 0; qi < block_data_info_ptr->buffer_value.size(); qi++)
        {
            if (block_data_info_ptr->buffer_value[qi] < 0x3A && block_data_info_ptr->buffer_value[qi] > 0x2F)
            {
                tm = tm * 10 + (block_data_info_ptr->buffer_value[qi] - 0x30);
            }
        }
        block_steam_httppeer->state.rangeend = tm;
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
            block_steam_httppeer->host                 = header_value;
            block_steam_httppeer->header["Host"]       = header_value;
            block_steam_httppeer->header[":authority"] = header_value;
            block_steam_httppeer->find_host_index();
            break;
        case 2:
            if (str_casecmp(header_value, "OPTIONS"))
            {
                block_steam_httppeer->method = 3;
            }
            else if (str_casecmp(header_value, "GET"))
            {
                block_steam_httppeer->method = 1;
            }
            else if (str_casecmp(header_value, "POST"))
            {
                block_steam_httppeer->method = 2;
            }
            else if (str_casecmp(header_value, "head"))
            {
                block_steam_httppeer->method = 4;
            }
            else if (str_casecmp(header_value, "put"))
            {
                block_steam_httppeer->method = 5;
            }
            else if (str_casecmp(header_value, "delete"))
            {
                block_steam_httppeer->method = 6;
            }
            block_steam_httppeer->header["method"] = header_value;
            break;
        case 3:
            if (str_casecmp(header_value, "OPTIONS"))
            {
                block_steam_httppeer->method = 3;
            }
            else if (str_casecmp(header_value, "GET"))
            {
                block_steam_httppeer->method = 1;
            }
            else if (str_casecmp(header_value, "POST"))
            {
                block_steam_httppeer->method = 2;
            }
            else if (str_casecmp(header_value, "head"))
            {
                block_steam_httppeer->method = 4;
            }
            else if (str_casecmp(header_value, "put"))
            {
                block_steam_httppeer->method = 5;
            }
            else if (str_casecmp(header_value, "delete"))
            {
                block_steam_httppeer->method = 6;
            }
            block_steam_httppeer->header["method"] = header_value;
            break;
        case 4:
        case 5: path_process(header_name, header_value); break;
        case 16: getacceptencoding(header_name, header_value); break;
        case 17: getacceptlanguage(header_name, header_value); break;
        case 19: getaccept(header_name, header_value); break;
        case 20:
            block_steam_httppeer->header["Access-Control-Allow-Origin"] = header_value;
            break;
        case 23:
            block_steam_httppeer->header["Authorization"] = header_value;
            break;
        case 28:
            block_steam_httppeer->content_length           = str2int(&header_value[0], header_value.size());
            block_steam_httppeer->header["Content-Length"] = header_value;
            break;
        case 31: getcontenttype(header_name, header_value); break;
        case 32: cookie_process(header_name, header_value); break;
        case 38:
            block_steam_httppeer->header["Host"] = header_value;
            break;
        case 41: getifnonematch(header_name, header_value); break;
        case 48:
            block_steam_httppeer->header["Proxy-Authenticate"] = header_value;
            break;
        case 49:
            block_steam_httppeer->header["Proxy-Authorization"] = header_value;
            break;
        case 50: range_process(header_name, header_value); break;
        case 51:
            block_steam_httppeer->header["Referer"] = header_value;
            break;
        case 58:
            block_steam_httppeer->header["User-Agent"] = header_value;
            break;
        case 61:
            block_steam_httppeer->header["WWW-Authenticate"] = header_value;
            break;
        }
    }
    else
    {
        switch (header_name.size())
        {
        case 5:
            if (str_casecmp(header_name, ":path"))
            {
                path_process(header_name, header_value);
            }
            else if (str_casecmp(header_name, "range"))
            {
                range_process(header_name, header_value);
            }
            block_steam_httppeer->header[header_name] = header_value;
            break;
        case 6:

            switch (header_name[0])
            {
            case 'c':
            case 'C':
                if (str_casecmp(header_name, "Cookie"))
                {
                    cookie_process(header_name, header_value);
                }
                else
                {
                    block_steam_httppeer->header[header_name] = header_value;
                }
                break;
            case 'a':
            case 'A':
                if (str_casecmp(header_name, "Accept"))
                {
                    getaccept(header_name, header_value);
                }
                else
                {
                    if (header_name[0] != ':')
                    {
                        block_steam_httppeer->header[header_name] = header_value;
                    }
                }
                break;
            default:
                if (header_name[0] != ':')
                {
                    block_steam_httppeer->header[header_name] = header_value;
                }
            }

            break;
        case 7:
            if (str_casecmp(header_name, ":method"))
            {
                if (str_casecmp(header_value, "OPTIONS"))
                {
                    block_steam_httppeer->method = 3;
                }
                else if (str_casecmp(header_value, "GET"))
                {
                    block_steam_httppeer->method = 1;
                }
                else if (str_casecmp(header_value, "POST"))
                {
                    block_steam_httppeer->method = 2;
                }
                else if (str_casecmp(header_value, "head"))
                {
                    block_steam_httppeer->method = 4;
                }
                else if (str_casecmp(header_value, "put"))
                {
                    block_steam_httppeer->method = 5;
                }
                else if (str_casecmp(header_value, "delete"))
                {
                    block_steam_httppeer->method = 6;
                }
                block_steam_httppeer->header["method"]  = header_value;
                block_steam_httppeer->header[":method"] = header_value;
            }
            else
            {
                block_steam_httppeer->header[header_name] = header_value;
            }
            break;
        case 10:
            if (str_casecmp(header_name, ":authority"))
            {
                block_steam_httppeer->host                 = header_value;
                block_steam_httppeer->header["Host"]       = header_value;
                block_steam_httppeer->header[":authority"] = header_value;
                block_steam_httppeer->find_host_index();
            }
            else if (str_casecmp(header_name, "User-Agent"))
            {
                block_steam_httppeer->header["User-Agent"] = header_value;
            }
            else
            {
                block_steam_httppeer->header[header_name] = header_value;
            }
            break;
        case 12:
            if (str_casecmp(header_name, "Content-Type"))
            {
                getcontenttype(header_name, header_value);
            }
            else
            {
                block_steam_httppeer->header[header_name] = header_value;
            }
            break;
        case 13:
            if (str_casecmp(header_name, "if-none-match"))
            {
                getifnonematch(header_name, header_value);
            }
            else
            {
                block_steam_httppeer->header[header_name] = header_value;
            }
            break;

        case 14:
            if (str_casecmp(header_name, "Content-Length"))
            {
                block_steam_httppeer->content_length           = str2int(&header_value[0], header_value.size());
                block_steam_httppeer->header["Content-Length"] = header_value;
            }
            else
            {
                block_steam_httppeer->header[header_name] = header_value;
            }
            break;
        case 15:
            if (header_name[7] == 'e')
            {
                if (str_casecmp(header_name, "accept-encoding"))
                {
                    getacceptencoding(header_name, header_value);
                    break;
                }
                else
                {
                    block_steam_httppeer->header[header_name] = header_value;
                }
            }
            else if (header_name[7] == 'L')
            {
                if (str_casecmp(header_name, "Accept-Language"))
                {
                    getacceptlanguage(header_name, header_value);
                    break;
                }
                else
                {
                    block_steam_httppeer->header[header_name] = header_value;
                }
            }
            else
            {
                block_steam_httppeer->header[header_name] = header_value;
            }

            if (str_casecmp(header_name, "Accept-Encoding"))
            {
                getacceptencoding(header_name, header_value);
            }
            if (str_casecmp(header_name, "Accept-Language"))
            {
                getacceptlanguage(header_name, header_value);
            }

            break;
        default:
            block_steam_httppeer->header[header_name] = header_value;
        }
    }
}

void http2parse::getacceptlanguage(const std::string &header_name, const std::string &header_value)
{
    block_steam_httppeer->header["Accept-Language"] = header_value;
    unsigned int i                                  = 0;
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] == 0x2C)
        {
            break;
        }
        block_steam_httppeer->state.language[i] = header_value[i];
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
        block_steam_httppeer->state.language[i] = 0x00;
    }
}
void http2parse::getacceptencoding(const std::string &header_name, const std::string &header_value)
{
    unsigned int i = 0, linesize = header_value.size();
    block_steam_httppeer->header["Accept-Encoding"] = header_value;
    block_data_info_ptr->buffer_value.clear();
    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C)
        {

            switch (block_data_info_ptr->buffer_value.size())
            {
            case 2:
                if (block_data_info_ptr->buffer_value[0] == 'b')
                {
                    block_steam_httppeer->state.br = true;
                }
                break;
            case 4:
                if (block_data_info_ptr->buffer_value[0] == 'g')
                {
                    block_steam_httppeer->state.gzip = true;
                }
                break;
            case 7:
                if (block_data_info_ptr->buffer_value[0] == 'd')
                {
                    block_steam_httppeer->state.deflate = true;
                    if (sizeof(header_name))
                    {
                    }
                }
                break;
            default:;
            }
            block_data_info_ptr->buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        block_data_info_ptr->buffer_value.push_back(header_value[i]);
    }
    if (block_data_info_ptr->buffer_value.size() > 0)
    {

        switch (block_data_info_ptr->buffer_value.size())
        {
        case 2:
            if (block_data_info_ptr->buffer_value[0] == 'b')
            {
                block_steam_httppeer->state.br = true;
            }
            break;
        case 4:
            if (block_data_info_ptr->buffer_value[0] == 'g')
            {
                block_steam_httppeer->state.gzip = true;
            }
            break;
        case 7:
            if (block_data_info_ptr->buffer_value[0] == 'd')
            {
                block_steam_httppeer->state.deflate = true;
            }
            break;
        default:;
        }
    }
    block_data_info_ptr->buffer_value.clear();
}
void http2parse::getifnonematch([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    unsigned int i                                = 0;
    block_steam_httppeer->header["If-None-Match"] = header_value;
    block_steam_httppeer->etag.clear();
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
            block_steam_httppeer->etag.push_back(header_value[i]);
        }
    }
}

void http2parse::callposttype()
{
    switch (block_data_info_ptr->buffer_value.size())
    {
    case 33:
        if (str_casecmp(block_data_info_ptr->buffer_value, "application/x-www-form-urlencoded"))
        {
            block_steam_httppeer->content_type = "application/x-www-form-urlencoded";
            block_data_info_ptr->posttype      = 1;
            block_steam_httppeer->posttype     = 1;
            return;
        }
        break;
    case 24:
        if (str_casecmp(block_data_info_ptr->buffer_value, "application/octet-stream"))
        {
            block_steam_httppeer->content_type = "application/octet-stream";
            block_data_info_ptr->posttype      = 5;
            block_steam_httppeer->posttype     = 5;
            return;
        }
        break;
    case 19:
        if (str_casecmp(block_data_info_ptr->buffer_value, "multipart/form-data"))
        {
            block_steam_httppeer->content_type = "multipart/form-data";
            block_data_info_ptr->posttype      = 2;
            block_steam_httppeer->posttype     = 2;
            return;
        }
        break;
    case 16:
        if (str_casecmp(block_data_info_ptr->buffer_value, "application/json"))
        {
            block_steam_httppeer->content_type = "application/json";
            block_data_info_ptr->posttype      = 3;
            block_steam_httppeer->posttype     = 3;
            return;
        }
        break;
    case 15:
        if (str_casecmp(block_data_info_ptr->buffer_value, "application/xml"))
        {
            block_steam_httppeer->content_type = "application/xml";
            block_data_info_ptr->posttype      = 4;
            block_steam_httppeer->posttype     = 4;
            return;
        }
        break;
    case 6:
        if (str_casecmp(block_data_info_ptr->buffer_value, "binary"))
        {
            block_steam_httppeer->content_type = "binary";
            block_data_info_ptr->posttype      = 6;
            block_steam_httppeer->posttype     = 5;
            return;
        }
        break;
    default:
        block_steam_httppeer->content_type = "raw";
        block_data_info_ptr->posttype      = 7;
        block_steam_httppeer->posttype     = 5;
        return;
    }
    block_steam_httppeer->content_type = "raw";
    block_data_info_ptr->posttype      = 7;
    block_steam_httppeer->posttype     = 5;
    return;
}

void http2parse::getcontenttype([[maybe_unused]] const std::string &header_name, const std::string &header_value)
{
    unsigned int i = 0, linesize = header_value.size();
    block_steam_httppeer->header["Content-Type"] = header_value;
    block_data_info_ptr->buffer_value.clear();
    unsigned char statetemp       = 0;
    block_data_info_ptr->posttype = 0;
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
                block_steam_httppeer->chartset = block_data_info_ptr->buffer_value;
                statetemp                      = 0;
            }
            else if (statetemp == 2)
            {
                block_data_info_ptr->boundary = block_data_info_ptr->buffer_value;
                statetemp                     = 0;
            }
            /////////////////////
            block_data_info_ptr->buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3D)
        {
            if (str_casecmp(block_data_info_ptr->buffer_value, "charset"))
            {
                block_data_info_ptr->buffer_value.clear();
                statetemp = 1;
                continue;
            }
            else if (str_casecmp(block_data_info_ptr->buffer_value, "boundary"))
            {
                block_data_info_ptr->buffer_value.clear();
                statetemp = 2;
                continue;
            }
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        block_data_info_ptr->buffer_value.push_back(header_value[i]);
    }
    if (block_data_info_ptr->buffer_value.size() > 0)
    {
        if (statetemp == 1)
        {
            block_steam_httppeer->chartset = block_data_info_ptr->buffer_value;
        }
        else if (statetemp == 2)
        {
            block_data_info_ptr->boundary = block_data_info_ptr->buffer_value;
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
    block_steam_httppeer->header["Accept"] = header_value;
    block_data_info_ptr->buffer_value.clear();

    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C || header_value[i] == 0x3B)
        {
            switch (block_data_info_ptr->buffer_value.length())
            {
            case 10:
                if (block_data_info_ptr->buffer_value[6] == 'a' &&
                    block_data_info_ptr->buffer_value[7] == 'v' &&
                    block_data_info_ptr->buffer_value[8] == 'i' && block_data_info_ptr->buffer_value[9] == 'f')
                {
                    block_steam_httppeer->state.avif = true;
                }
                else if (block_data_info_ptr->buffer_value[6] == 'w' &&
                         block_data_info_ptr->buffer_value[7] == 'e' &&
                         block_data_info_ptr->buffer_value[8] == 'b' &&
                         block_data_info_ptr->buffer_value[9] == 'p')
                {
                    block_steam_httppeer->state.webp = true;
                }
                break;
            default:;
            }
            block_data_info_ptr->buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        block_data_info_ptr->buffer_value.push_back(header_value[i]);
    }
    if (block_data_info_ptr->buffer_value.size() > 0)
    {
        switch (block_data_info_ptr->buffer_value.length())
        {
        case 10:
            if (block_data_info_ptr->buffer_value[6] == 'a' && block_data_info_ptr->buffer_value[7] == 'v' &&
                block_data_info_ptr->buffer_value[8] == 'i' && block_data_info_ptr->buffer_value[9] == 'f')
            {
                block_steam_httppeer->state.avif = true;
            }
            else if (block_data_info_ptr->buffer_value[6] == 'w' &&
                     block_data_info_ptr->buffer_value[7] == 'e' &&
                     block_data_info_ptr->buffer_value[8] == 'b' && block_data_info_ptr->buffer_value[9] == 'p')
            {
                block_steam_httppeer->state.webp = true;
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
            block_steam_httppeer->header[":method"] = http2_header_static_table[a].value;
            if (a == 3)
            {
                block_steam_httppeer->method = 2;
                ispost                       = true;
            }
            else
            {
                block_steam_httppeer->method = 1;
                ispost                       = false;
            }
        }
        else if (a == 4 || a == 5)
        {
            block_steam_httppeer->url             = http2_header_static_table[a].value;
            block_steam_httppeer->urlpath         = http2_header_static_table[a].value;
            block_steam_httppeer->header["path"]  = http2_header_static_table[a].value;
            block_steam_httppeer->header[":path"] = http2_header_static_table[a].value;
        }
        else if (a == 6 || a == 7)
        {

            block_steam_httppeer->isssl             = (a == 7) ? true : false;
            block_steam_httppeer->header[":scheme"] = http2_header_static_table[a].value;
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
    // struct http2_priority_t temp;
    for (unsigned int n = readoffset; n < pin; n += 5)
    {
        ident_stream = buffer[n + 4];//buffer[n];
        ident_stream = ident_stream << 8 | buffer[n + 1];
        ident_stream = ident_stream << 8 | buffer[n + 2];
        ident_stream = ident_stream << 8 | buffer[n + 3];
        // temp.weight           = buffer[n + 4];
        // temp.depend_stream_id = ident_stream;
        //current not use priority data
        //priority_data[block_steamid] = temp;
        block_data_info_ptr->priority_lists.push_back(ident_stream);
    }

    readoffset += 5;
    processheader = 0;
}
void http2parse::readdata([[maybe_unused]] const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readdata %ul %c", buffersize, (buffer[readoffset] ? '0' : '1'));
    readoffset += blocklength;
}
void http2parse::readgoaway([[maybe_unused]] const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readgoaway %ul", buffersize);
    // unsigned int j = readoffset;
    // unsigned int ident_stream;
    // ident_stream = buffer[j];
    // ident_stream = ident_stream << 8 | buffer[j + 1];
    // ident_stream = ident_stream << 8 | buffer[j + 2];
    // ident_stream = ident_stream << 8 | buffer[j + 3];

    // goaway_data.last_stream_id = ident_stream;
    // j += 4;
    // ident_stream = 0;
    // ident_stream = buffer[j];
    // ident_stream = ident_stream << 8 | buffer[j + 1];
    // ident_stream = ident_stream << 8 | buffer[j + 2];
    // ident_stream = ident_stream << 8 | buffer[j + 3];

    // goaway_data.error_code = ident_stream;

    // j += 4;
    // ident_stream = j + blocklength;
    // for (; j < ident_stream; j++)
    // {
    //     goaway_data.data.push_back(buffer[j]);
    // }

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

    if (peer_session->user_code_handler_call.size() > 0)
    {
        try
        {
            std::unique_lock<std::mutex> lock(peer_session->pop_user_handleer_mutex);
            if (peer_session->user_code_handler_call.size() > 0)
            {
                auto ex = asio::get_associated_executor(peer_session->user_code_handler_call.front());
                asio::dispatch(ex,
                               [handler = std::move(peer_session->user_code_handler_call.front())]() mutable -> void
                               {
                                   /////////////
                                   handler(1);
                                   //////////
                               });
                peer_session->user_code_handler_call.pop_front();
            }
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
    peer_session->http2_send_data(std::string_view((char *)_recvack, 17));
}
void http2parse::readrst_stream([[maybe_unused]] const unsigned char *buffer, [[maybe_unused]] unsigned int buffersize)
{
    DEBUG_LOG("readrst_stream %ul %c", buffersize, (buffer[readoffset] ? '0' : '1'));
    readoffset += blocklength;
    processheader = 0;
    if (http_data.contains(block_steamid))
    {
        http_data[block_steamid]->isclose = true;
    }
}
void http2parse::procssxformurlencoded()
{
    std::string objname;
    bool isgroup = true;

    for (unsigned int j = 0; j < block_data_info_ptr->buffer_key.length(); j++)
    {
        if (block_data_info_ptr->buffer_key[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < block_data_info_ptr->buffer_key.length(); n++)
            {
                if (block_data_info_ptr->buffer_key[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (block_data_info_ptr->buffer_key[n] == '[')
                {

                    break;
                }
                if (block_data_info_ptr->buffer_key[n] != 0x22)
                {
                    key1name.push_back(block_data_info_ptr->buffer_key[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < block_data_info_ptr->buffer_key.length())
                {
                    if (block_data_info_ptr->buffer_key[m] == '[')
                    {
                        m += 1;
                        for (; m < block_data_info_ptr->buffer_key.length(); m++)
                        {
                            if (block_data_info_ptr->buffer_key[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (block_data_info_ptr->buffer_key[m] == '[')
                            {

                                break;
                            }
                            if (block_data_info_ptr->buffer_key[m] != 0x22)
                            {
                                key2name.push_back(block_data_info_ptr->buffer_key[m]);
                            }
                        }

                        if (ishaskey2 && m == block_data_info_ptr->buffer_key.length())
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
                            objtemp.push(block_data_info_ptr->field_value);
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            block_steam_httppeer->post[objname].push(std::move(objtemp1));
                        }
                        else
                        {
                            http::OBJ_VALUE objtemp;
                            objtemp[key2name] = block_data_info_ptr->field_value;
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            block_steam_httppeer->post[objname].push(std::move(objtemp1));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            block_steam_httppeer->post[objname][key1name].set_array();
                            block_steam_httppeer->post[objname][key1name] = block_data_info_ptr->field_value;
                        }
                        else
                        {

                            block_steam_httppeer->post[objname][key1name][key2name] =
                                block_data_info_ptr->field_value;
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == block_data_info_ptr->buffer_key.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        block_steam_httppeer->post[objname].set_array();
                        block_steam_httppeer->post[objname].push(block_data_info_ptr->field_value);
                    }
                    else
                    {
                        block_steam_httppeer->post[objname].push(key1name, block_data_info_ptr->field_value);
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
                objname.push_back(block_data_info_ptr->buffer_key[j]);
            }
        }
        else
        {
            objname.push_back(block_data_info_ptr->buffer_key[j]);
        }
    }
    if (isgroup)
    {
        block_steam_httppeer->post[block_data_info_ptr->buffer_key] = block_data_info_ptr->field_value;
    }
}
void http2parse::readformurlencoded()
{
    std::string temp_value;
    block_data_info_ptr->buffer_key.clear();
    block_data_info_ptr->field_value.clear();
    unsigned int qsize    = block_data_info_ptr->buffer_value.size();
    unsigned char partype = 0;
    unsigned int j        = 0;
    for (j = 0; j < qsize; j++)
    {
        if (block_data_info_ptr->buffer_value[j] == 0x3D)
        {
            block_data_info_ptr->buffer_key = http::url_decode(temp_value.data(), temp_value.length());
            temp_value.clear();
            partype = 1;
            continue;
        }
        else if (block_data_info_ptr->buffer_value[j] == 0x26)
        {
            block_data_info_ptr->field_value = http::url_decode(temp_value.data(), temp_value.length());
            procssxformurlencoded();
            temp_value.clear();
            block_data_info_ptr->field_value.clear();
            partype = 2;
            continue;
        }
        temp_value.push_back(block_data_info_ptr->buffer_value[j]);
    }
    if (partype == 1)
    {
        block_data_info_ptr->field_value = http::url_decode(temp_value.data(), temp_value.length());
        procssxformurlencoded();
    }
    else if (partype == 2)
    {
        block_data_info_ptr->buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        block_data_info_ptr->field_value.clear();
        procssxformurlencoded();
    }
    else if (temp_value.size() > 0)
    {
        block_data_info_ptr->buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        block_data_info_ptr->field_value.clear();
        procssxformurlencoded();
    }
}

void http2parse::procssformfile()
{
    std::string objname;
    bool isgroup = true;
    for (unsigned int j = 0; j < block_data_info_ptr->upfile.name.length(); j++)
    {
        if (block_data_info_ptr->upfile.name[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < block_data_info_ptr->upfile.name.length(); n++)
            {
                if (block_data_info_ptr->upfile.name[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (block_data_info_ptr->upfile.name[n] == '[')
                {

                    break;
                }
                if (block_data_info_ptr->upfile.name[n] != 0x22)
                {
                    key1name.push_back(block_data_info_ptr->upfile.name[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < block_data_info_ptr->upfile.name.length())
                {
                    if (block_data_info_ptr->upfile.name[m] == '[')
                    {
                        m += 1;
                        for (; m < block_data_info_ptr->upfile.name.length(); m++)
                        {
                            if (block_data_info_ptr->upfile.name[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (block_data_info_ptr->upfile.name[m] == '[')
                            {

                                break;
                            }
                            if (block_data_info_ptr->upfile.name[m] != 0x22)
                            {
                                key2name.push_back(block_data_info_ptr->upfile.name[m]);
                            }
                        }

                        if (ishaskey2 && m == block_data_info_ptr->upfile.name.length())
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
                            objtemp["filename"] = block_data_info_ptr->upfile.filename;
                            objtemp["name"]     = block_data_info_ptr->upfile.name;
                            objtemp["tempfile"] = block_data_info_ptr->upfile.tempfile;
                            objtemp["type"]     = block_data_info_ptr->upfile.type;
                            objtemp["size"]     = block_data_info_ptr->upfile.size;
                            objtemp["error"]    = block_data_info_ptr->upfile.error;

                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            block_steam_httppeer->files[objname].push(std::move(objtemp1));
                        }
                        else
                        {

                            http::OBJ_VALUE objtemp;
                            objtemp[key2name].set_array();
                            objtemp[key2name]["filename"] = block_data_info_ptr->upfile.filename;
                            objtemp[key2name]["name"]     = block_data_info_ptr->upfile.name;
                            objtemp[key2name]["tempfile"] = block_data_info_ptr->upfile.tempfile;
                            objtemp[key2name]["type"]     = block_data_info_ptr->upfile.type;
                            objtemp[key2name]["size"]     = block_data_info_ptr->upfile.size;
                            objtemp[key2name]["error"]    = block_data_info_ptr->upfile.error;
                            block_steam_httppeer->files[objname].set_array();
                            block_steam_httppeer->files[objname].push(std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {

                            block_steam_httppeer->files[objname][key1name].set_array();
                            http::OBJ_VALUE objtemp;
                            objtemp.set_array();
                            objtemp["filename"] = block_data_info_ptr->upfile.filename;
                            objtemp["name"]     = block_data_info_ptr->upfile.name;
                            objtemp["tempfile"] = block_data_info_ptr->upfile.tempfile;
                            objtemp["type"]     = block_data_info_ptr->upfile.type;
                            objtemp["size"]     = block_data_info_ptr->upfile.size;
                            objtemp["error"]    = block_data_info_ptr->upfile.error;

                            block_steam_httppeer->files[objname][key1name] = objtemp;
                        }
                        else
                        {

                            block_steam_httppeer->files[objname][key1name][key2name].set_array();
                            block_steam_httppeer->files[objname][key1name][key2name]["filename"] =
                                block_data_info_ptr->upfile.filename;
                            block_steam_httppeer->files[objname][key1name][key2name]["name"] =
                                block_data_info_ptr->upfile.name;
                            block_steam_httppeer->files[objname][key1name][key2name]["tempfile"] =
                                block_data_info_ptr->upfile.tempfile;
                            block_steam_httppeer->files[objname][key1name][key2name]["type"] =
                                block_data_info_ptr->upfile.type;
                            block_steam_httppeer->files[objname][key1name][key2name]["size"] =
                                block_data_info_ptr->upfile.size;
                            block_steam_httppeer->files[objname][key1name][key2name]["error"] =
                                block_data_info_ptr->upfile.error;
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == block_data_info_ptr->upfile.name.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        block_steam_httppeer->files[objname].set_array();
                        http::OBJ_VALUE objtemp;
                        objtemp.set_array();
                        objtemp["filename"]                  = block_data_info_ptr->upfile.filename;
                        objtemp["name"]                      = block_data_info_ptr->upfile.name;
                        objtemp["tempfile"]                  = block_data_info_ptr->upfile.tempfile;
                        objtemp["type"]                      = block_data_info_ptr->upfile.type;
                        objtemp["size"]                      = block_data_info_ptr->upfile.size;
                        objtemp["error"]                     = block_data_info_ptr->upfile.error;
                        block_steam_httppeer->files[objname] = objtemp;
                    }
                    else
                    {
                        // files[objname].push(key1name,"");
                        block_steam_httppeer->files[objname][key1name].set_array();
                        block_steam_httppeer->files[objname][key1name]["filename"] =
                            block_data_info_ptr->upfile.filename;
                        block_steam_httppeer->files[objname][key1name]["name"] =
                            block_data_info_ptr->upfile.name;
                        block_steam_httppeer->files[objname][key1name]["tempfile"] =
                            block_data_info_ptr->upfile.tempfile;
                        block_steam_httppeer->files[objname][key1name]["type"] =
                            block_data_info_ptr->upfile.type;
                        block_steam_httppeer->files[objname][key1name]["size"] =
                            block_data_info_ptr->upfile.size;
                        block_steam_httppeer->files[objname][key1name]["error"] =
                            block_data_info_ptr->upfile.error;
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
                objname.push_back(block_data_info_ptr->upfile.name[j]);
            }
        }
        else
        {
            objname.push_back(block_data_info_ptr->upfile.name[j]);
        }
    }
    if (isgroup)
    {
        // files[upfile.name]=block_data_info_ptr->buffer_value;
        block_steam_httppeer->files[block_data_info_ptr->upfile.name].set_array();
        block_steam_httppeer->files[block_data_info_ptr->upfile.name]["filename"] =
            block_data_info_ptr->upfile.filename;
        block_steam_httppeer->files[block_data_info_ptr->upfile.name]["name"] =
            block_data_info_ptr->upfile.name;
        block_steam_httppeer->files[block_data_info_ptr->upfile.name]["tempfile"] =
            block_data_info_ptr->upfile.tempfile;
        block_steam_httppeer->files[block_data_info_ptr->upfile.name]["type"] =
            block_data_info_ptr->upfile.type;
        block_steam_httppeer->files[block_data_info_ptr->upfile.name]["size"] =
            block_data_info_ptr->upfile.size;
        block_steam_httppeer->files[block_data_info_ptr->upfile.name]["error"] =
            block_data_info_ptr->upfile.error;
    }
    block_data_info_ptr->upfile.filename.clear();
    block_data_info_ptr->upfile.name.clear();
    block_data_info_ptr->upfile.tempfile.clear();
    block_data_info_ptr->upfile.type.clear();
    block_data_info_ptr->upfile.size  = 0;
    block_data_info_ptr->upfile.error = 0;
}

void http2parse::readformfieldfilecontent(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize)
{
    unsigned int pmi = 0, ib = 0, isbound = 0;
    if (block_data_info_ptr->match_offset > 0 || block_data_info_ptr->buffer_key.size() > 0)
    {
        pmi = begin, ib = block_data_info_ptr->match_offset;
        if (block_data_info_ptr->buffer_key.size() < 5)
        {
            pmi = pmi + (4 - block_data_info_ptr->buffer_key.size());
            ib  = 0;
        }
        block_data_info_ptr->match_offset = 0;
        for (; ib < block_data_info_ptr->boundary.size(); ib++)
        {
            if (buffer[pmi] != block_data_info_ptr->boundary[ib])
            {
                break;
            }
            pmi++;
        }
        if (ib == block_data_info_ptr->boundary.size())
        {

            if (block_data_info_ptr->uprawfile)
            {
                // fclose(data_info[block_steamid].uprawfile);
                block_data_info_ptr->uprawfile.reset(nullptr);
            }
            procssformfile();
            block_data_info_ptr->buffer_key.clear();
            block_data_info_ptr->buffer_value.clear();
            if (buffer[pmi] == '-' && buffer[pmi + 1] == '-')
            {
                pmi += 2;
                block_steam_httppeer->isfinish = true;

                if (buffer[pmi] == 0x0D)
                {
                    pmi++;
                }
                if (buffer[pmi] == 0x0A)
                {
                    pmi++;
                }
                begin                              = pmi;
                block_data_info_ptr->postfieldtype = 2;
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
            begin                              = pmi;
            block_data_info_ptr->postfieldtype = 2;

            return;
        }
        if (block_data_info_ptr->uprawfile)
        {
            block_data_info_ptr->upfile.size += block_data_info_ptr->buffer_key.size();
            fwrite(&block_data_info_ptr->buffer_key[0],
                   1,
                   block_data_info_ptr->buffer_key.size(),
                   block_data_info_ptr->uprawfile.get());
        }
        block_data_info_ptr->buffer_key.clear();
    }
    unsigned int baseoffset = begin;
    pmi = 0, ib = 0;
    block_data_info_ptr->match_offset = 0;

    for (; begin < buffersize; begin++)
    {
        if (buffer[begin] == 0x0D)
        {

            block_data_info_ptr->buffer_key.clear();
            block_data_info_ptr->buffer_key.push_back(0x0D);
            unsigned int j = begin + 1;
            isbound        = 0;
            if (j < buffersize && buffer[j] == 0x0A)
            {

                j++;

                block_data_info_ptr->buffer_key.push_back(0x0A);
                if (j < buffersize && buffer[j] == 0x2D)
                {
                    j++;

                    block_data_info_ptr->buffer_key.push_back(0x2D);
                    if (j < buffersize && buffer[j] == 0x2D)
                    {
                        j++;

                        block_data_info_ptr->buffer_key.push_back(0x2D);
                        for (unsigned int nnn = 0; nnn < block_data_info_ptr->boundary.size(); nnn++)
                        {
                            if (buffer[j] != block_data_info_ptr->boundary[nnn])
                            {
                                isbound = 2;
                                break;
                            }
                            j += 1;
                            block_data_info_ptr->buffer_key.push_back(buffer[j]);
                            if (j >= buffersize)
                            {
                                block_data_info_ptr->match_offset = nnn + 1;
                                isbound                           = 1;
                                break;
                            }
                        }
                        if (isbound == 0)
                        {

                            block_data_info_ptr->match_offset  = 0;
                            block_data_info_ptr->postfieldtype = 2;

                            if (block_data_info_ptr->uprawfile)
                            {
                                block_data_info_ptr->upfile.size += (begin - baseoffset);
                                fwrite(&buffer[baseoffset],
                                       1,
                                       (begin - baseoffset),
                                       block_data_info_ptr->uprawfile.get());
                                // fclose(data_info[block_steamid].uprawfile);
                                block_data_info_ptr->uprawfile.reset(nullptr);
                                procssformfile();
                            }
                            block_data_info_ptr->buffer_key.clear();
                            block_data_info_ptr->buffer_value.clear();

                            // i++;
                            if (buffer[begin] == 0x2D && buffer[begin + 1] == 0x2D)
                            {
                                block_steam_httppeer->isfinish = true;
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
            block_data_info_ptr->buffer_key.clear();
            isbound = 0;
        }
    }
    if (block_data_info_ptr->uprawfile)
    {
        block_data_info_ptr->upfile.size += (begin - baseoffset);
        fwrite(&buffer[baseoffset], 1, (begin - baseoffset), block_data_info_ptr->uprawfile.get());
    }
    begin = buffersize;
}

void http2parse::readformfieldcontent(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize)
{
    unsigned int pmi = 0, ib = 0;
    if (block_data_info_ptr->match_offset > 0 || block_data_info_ptr->buffer_key.size() > 0)
    {

        pmi = begin, ib = block_data_info_ptr->match_offset;

        if (block_data_info_ptr->buffer_key.size() < 5)
        {
            pmi = pmi + (4 - block_data_info_ptr->buffer_key.size());
            ib  = 0;
        }

        block_data_info_ptr->match_offset = 0;
        for (; ib < block_data_info_ptr->boundary.size(); ib++)
        {
            if (buffer[pmi] != block_data_info_ptr->boundary[ib])
            {
                break;
            }
            pmi++;
            if (pmi >= buffersize)
            {
                break;
            }
        }
        if (ib == block_data_info_ptr->boundary.size())
        {
            if (buffer[pmi] == '-' && buffer[pmi + 1] == '-')
            {
                pmi += 2;

                if (block_data_info_ptr->uprawfile)
                {
                    fclose(block_data_info_ptr->uprawfile.get());
                }
                block_data_info_ptr->buffer_key = block_data_info_ptr->fieldname;
                procssxformurlencoded();
                block_steam_httppeer->isfinish = true;
                block_data_info_ptr->buffer_key.clear();
                block_data_info_ptr->field_value.clear();
                if (buffer[pmi] == 0x0D)
                {
                    pmi++;
                }
                if (buffer[pmi] == 0x0A)
                {
                    pmi++;
                }
                begin                              = pmi;
                block_data_info_ptr->postfieldtype = 2;
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
            begin                              = pmi;
            block_data_info_ptr->postfieldtype = 2;
            return;
        }

        block_data_info_ptr->field_value.append(block_data_info_ptr->buffer_key);
        block_data_info_ptr->buffer_key.clear();
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

            block_data_info_ptr->buffer_key.clear();
            block_data_info_ptr->buffer_key.push_back(0x0D);

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

                    block_data_info_ptr->buffer_key.push_back(splitchar[ik]);
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
                block_data_info_ptr->match_offset = 0;
                for (; ib < block_data_info_ptr->boundary.size(); ib++)
                {
                    if (buffer[pmi] != block_data_info_ptr->boundary[ib])
                    {
                        break;
                    }
                    block_data_info_ptr->buffer_key.push_back(buffer[pmi]);
                    pmi++;
                    if (pmi >= buffersize)
                    {
                        // islastmatch = true;
                        block_data_info_ptr->match_offset = ib + 1;
                        break;
                    }
                }
                if (ib == block_data_info_ptr->boundary.size())
                {
                    block_data_info_ptr->match_offset = 0;
                    block_data_info_ptr->buffer_key.clear();
                    block_data_info_ptr->field_value.append((char *)&buffer[baseoffset], (begin - baseoffset));
                    block_data_info_ptr->buffer_key = block_data_info_ptr->fieldname;
                    procssxformurlencoded();
                    block_data_info_ptr->buffer_key.clear();
                    block_data_info_ptr->field_value.clear();

                    if (buffer[pmi] == '-' && buffer[pmi + 1] == '-')
                    {
                        pmi += 2;
                        block_steam_httppeer->isfinish = true;

                        if (buffer[pmi] == 0x0D)
                        {
                            pmi++;
                        }
                        if (buffer[pmi] == 0x0A)
                        {
                            pmi++;
                        }
                        begin                              = pmi;
                        block_data_info_ptr->postfieldtype = 2;
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
                    begin                              = pmi;
                    block_data_info_ptr->postfieldtype = 2;
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
            block_data_info_ptr->buffer_key.clear();
        }
    }
    block_data_info_ptr->field_value.append((char *)&buffer[baseoffset], (begin - baseoffset));
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
            if (header_temp.size() == 4 && str_casecmp(header_temp, "name"))
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
                    if (block_data_info_ptr->postfieldtype < 3)
                    {
                        block_data_info_ptr->postfieldtype = 4;
                    }
                }

                if (tfheader_name[mm] == '"')
                {
                    mm += 1;
                }
                jj = mm;
            }
            else if (header_temp.size() == 8 && str_casecmp(header_temp, "filename"))
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

                if (block_data_info_ptr->postfieldtype < 5)
                {
                    block_data_info_ptr->postfieldtype = 6;
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
    block_data_info_ptr->upfile.type.clear();
    for (; jj < tfheader_name.size(); jj++)
    {
        if (tfheader_name[jj] == 0x20 || tfheader_name[jj] == 0x0D || tfheader_name[jj] == ';')
        {
            break;
        }
        block_data_info_ptr->upfile.type.push_back(tfheader_name[jj]);
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

            block_data_info_ptr->buffer_key.push_back(buffer[i]);
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
        for (; jj < block_data_info_ptr->buffer_key.size(); jj++)
        {
            if (block_data_info_ptr->buffer_key[jj] == 0x0D || block_data_info_ptr->buffer_key[jj] == 0x0A)
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
            if (block_data_info_ptr->buffer_key[jj] == ':')
            {
                if (fieldheader_temp.size() == 19 && str_casecmp(fieldheader_temp, "Content-Disposition"))
                {
                    fieldtype = 1;
                }
                else if (fieldheader_temp.size() == 12 && str_casecmp(fieldheader_temp, "Content-Type"))
                {
                    fieldtype = 2;
                }
            }
            fieldheader_temp.push_back(block_data_info_ptr->buffer_key[jj]);
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
        if (block_data_info_ptr->postfieldtype == 6)
        {
            server_loaclvar &localvar            = get_server_global_var();
            block_data_info_ptr->upfile.filename = filesname;
            block_data_info_ptr->upfile.name     = fieldname;

            fieldheader_temp = block_data_info_ptr->upfile.filename + std::to_string(http::timeid()) + fieldname;

            block_data_info_ptr->upfile.tempfile = localvar.temp_path;// + "temp/";
            block_data_info_ptr->upfile.tempfile.append(std::to_string(std::hash<std::string>{}(fieldheader_temp)));

            DEBUG_LOG("filename:%s", block_data_info_ptr->upfile.tempfile.c_str());
            // block_data_info_ptr->uprawfile = fopen(block_data_info_ptr->upfile.tempfile.c_str(), "wb");
            block_data_info_ptr->uprawfile.reset(fopen(block_data_info_ptr->upfile.tempfile.c_str(), "wb"));
            if (!block_data_info_ptr->uprawfile)
            {
                block_data_info_ptr->upfile.tempfile.append("_t");
                // data_info[block_steamid].uprawfile = fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb");
                block_data_info_ptr->uprawfile.reset(fopen(block_data_info_ptr->upfile.tempfile.c_str(), "wb"));
                if (!block_data_info_ptr->uprawfile)
                {
                    error = 303;
                }
            }
            block_data_info_ptr->upfile.size = 0;
        }
        block_data_info_ptr->buffer_key.clear();
        block_data_info_ptr->buffer_value.clear();
        block_data_info_ptr->fieldname = fieldname;
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
    for (; ni < block_data_info_ptr->boundary.size(); ni++)
    {
        if (buffer[begin] != block_data_info_ptr->boundary[ni])
        {
            break;
        }
        begin++;
    }
    if (ni == block_data_info_ptr->boundary.size())
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
            block_data_info_ptr->postfieldtype = 2;
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
        switch (block_data_info_ptr->postfieldtype)
        {
        case 0:
            // begin post content
            readboundaryline(buffer, i, buffersize);
            block_data_info_ptr->buffer_key.clear();
            block_data_info_ptr->buffer_value.clear();
            block_data_info_ptr->field_value.clear();
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
    if (!block_data_info_ptr->uprawfile)
    {
        server_loaclvar &localvar            = get_server_global_var();
        block_data_info_ptr->upfile.filename = "rawcontent";
        block_data_info_ptr->upfile.name     = "rawcontent";

        std::string fieldheader_temp =
            block_data_info_ptr->upfile.filename + std::to_string(http::timeid()) + "rawcontent";

        block_data_info_ptr->upfile.tempfile = localvar.temp_path;// + "temp/";
        block_data_info_ptr->upfile.tempfile.append(std::to_string(std::hash<std::string>{}(fieldheader_temp)));
        // block_data_info_ptr->uprawfile = fopen(block_data_info_ptr->upfile.tempfile.c_str(), "wb");
        block_data_info_ptr->uprawfile.reset(fopen(block_data_info_ptr->upfile.tempfile.c_str(), "wb"));
        if (!block_data_info_ptr->uprawfile)
        {
            block_data_info_ptr->upfile.tempfile.append("_t");
            // data_info[block_steamid].uprawfile = fopen(data_info[block_steamid].upfile.tempfile.c_str(), "wb");
            block_data_info_ptr->uprawfile.reset(fopen(block_data_info_ptr->upfile.tempfile.c_str(), "wb"));
            if (!block_data_info_ptr->uprawfile)
            {
                error = 303;
            }
        }
        block_data_info_ptr->upfile.type = block_steam_httppeer->content_type;
        block_data_info_ptr->upfile.size = 0;
    }
    if (block_data_info_ptr->uprawfile)
    {
        block_data_info_ptr->upfile.size += buffersize;
        fwrite(buffer, 1, buffersize, block_data_info_ptr->uprawfile.get());
    }

    if (block_steam_httppeer->content_length < (block_data_info_ptr->upfile.size + 2))
    {
        if (block_data_info_ptr->uprawfile)
        {
            fclose(block_data_info_ptr->uprawfile.get());
        }
        procssformfile();
    }
}

void http2parse::data_process()
{
    //unsigned int j = 0, w_size = stream_data[block_steamid].size();
    unsigned int j = 0, w_size = stream_data_ptr->size();
    if (block_data_info_ptr->padded)
    {
        j += 1;
        if (w_size > 0)
        {
            w_size -= 1;
            w_size -= block_data_info_ptr->pad_length;
        }
    }

    if (block_steam_httppeer->compress == 10)
    {
        if (block_steam_httppeer->content_length < 16777216)
        {
            if (block_steam_httppeer->output.size() < block_steam_httppeer->content_length)
            {
                block_steam_httppeer->output.append(stream_data_ptr->data() + j, w_size);
            }
        }
        else
        {
            error = 403;
        }
    }
    else
    {
        block_steam_httppeer->upload_length = block_steam_httppeer->upload_length + w_size;
        if (block_steam_httppeer->upload_length > block_steam_httppeer->content_length)
        {
            error = 403;//now ruturn?
        }
        switch (block_data_info_ptr->posttype)
        {
        case 1:
            // x-www-form-urlencoded
            block_data_info_ptr->buffer_value.append(stream_data_ptr->data() + j, w_size);
            if (block_data_info_ptr->buffer_value.size() == (unsigned int)block_steam_httppeer->content_length)
            {
                readformurlencoded();
                block_data_info_ptr->buffer_value.clear();
            }
            break;
        case 2:
            // multipart/form-data
            readmultipartformdata((const unsigned char *)stream_data_ptr->data() + j, w_size);
            break;
        case 3:
            // json
            block_steam_httppeer->rawcontent.append(stream_data_ptr->data() + j, w_size);
            if (block_steam_httppeer->rawcontent.size() == (unsigned int)block_steam_httppeer->content_length)
            {
                block_steam_httppeer->json.from_json(block_steam_httppeer->rawcontent);
            }
            break;
        case 4:
            // xml
            // readrawfileformdata((const unsigned char *)&stream_data[block_steamid][j], w_size);
            block_steam_httppeer->rawcontent.append(stream_data_ptr->data() + j, w_size);
            break;
        case 5:
            // octet-stream
            readrawfileformdata((const unsigned char *)stream_data_ptr->data() + j, w_size);
            break;
        }
    }

    //stream_data[block_steamid].clear();
    stream_data_ptr->clear();

    if (block_data_info_ptr->endstream)
    {
        isfinsish     = 2;
        processheader = 0;

        if (block_data_info_ptr->uprawfile)
        {
            fclose(block_data_info_ptr->uprawfile.get());
        }

        if (!block_steam_httppeer->isfinish)
        {
            block_steam_httppeer->isfinish     = true;
            block_data_info_ptr->postfieldtype = 0;
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
    block_short_length  = block_data_info_ptr->length - block_data_info_ptr->curnum;

    if (buffer_short_length <= block_short_length)
    {
        short_loop_max = buffer_short_length;
    }
    else
    {
        short_loop_max = block_short_length;
    }

    //stream_data[block_steamid].append((const char *)&buffer[j], short_loop_max);
    stream_data_ptr->append((const char *)&buffer[j], short_loop_max);
    block_data_info_ptr->curnum += short_loop_max;

    window_update_recv_num -= short_loop_max;

    if (window_update_recv_num * 2 < RECV_WINDOW_UPDATE_NUM)
    {
        peer_session->recv_window_update(RECV_WINDOW_UPDATE_NUM - window_update_recv_num, block_steamid);
        window_update_recv_num = RECV_WINDOW_UPDATE_NUM;
    }
    readoffset = readoffset + short_loop_max;
    // 数据块处理完成
    if (block_data_info_ptr->curnum == block_data_info_ptr->length)
    {
        DEBUG_LOG("data_process block:%llu", block_data_info_ptr->length);
        data_process();
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
