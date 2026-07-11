/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file http_parse.cpp
 *  @date 2022-04-17
 *
 *  http1 protocol parse file
 *
 *
 */

#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <filesystem>
#include <string_view>
#include <utility>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

#include <cstring>

#include "datetime.h"
// #include "md5.hpp"
// #include "mime.hpp"
#include "unicode.h"
// #include "cookie.hpp"
#include "urlcode.h"
#include "request.h"

#include "http_parse.h"
#include "client_session.h"
#include "httppeer.h"
#include "server_localvar.h"
#include "func.h"
#include "cost_define.h"
namespace http
{

httpparse::httpparse() : uprawfile(nullptr, std::fclose) { error = 0; }
void httpparse::setpeer(std::shared_ptr<httppeer> hpeer) { peer = hpeer->get_ptr(); }
unsigned long long httpparse::header_valuetoint(std::string_view header_value)
{
    unsigned long long temp = 0;

    if (header_value.size() > 14)
    {
        error = 40008;
        return 0;
    }
    for (unsigned int qi = 0; qi < header_value.size(); qi++)
    {
        if (header_value[qi] < 0x3A && header_value[qi] > 0x2F)
        {
            temp = temp * 10 + (header_value[qi] - 0x30);
        }
    }
    return temp;
}

void httpparse::procssparamter(std::string_view header_temp, std::string_view header_input)
{
    bool isgroup = true;
    if (header_temp.length() > 72)
    {
        error = 40009;
        return;
    }

    for (unsigned int j = 0; j < header_temp.length(); j++)
    {
        if (header_temp[j] == '[')
        {
            isgroup = false;
        }
    }

    if (isgroup)
    {
        peer->get[header_temp] = header_input;
        return;
    }

    isgroup = true;
    std::string objname;
    for (unsigned int j = 0; j < header_temp.length(); j++)
    {
        if (header_temp[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < header_temp.length(); n++)
            {
                if (header_temp[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (header_temp[n] == '[')
                {

                    break;
                }
                if (header_temp[n] != 0x22)
                {
                    key1name.push_back(header_temp[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < header_temp.length())
                {
                    if (header_temp[m] == '[')
                    {
                        m += 1;
                        for (; m < header_temp.length(); m++)
                        {
                            if (header_temp[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (header_temp[m] == '[')
                            {

                                break;
                            }
                            if (header_temp[m] != 0x22)
                            {
                                key2name.push_back(header_temp[m]);
                            }
                        }

                        if (ishaskey2 && m == header_temp.length())
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
                                error = 40010;
                                return;
                            }

                            peer->get[objname].set_object();
                            unsigned int iii = peer->get[objname].size();
                            key1name         = std::to_string(iii);
                            peer->get[objname][key1name].set_object();

                            iii      = peer->get[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = header_input;

                            peer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40012;
                                return;
                            }

                            if (key2name.size() > 48)
                            {
                                error = 40013;
                                return;
                            }

                            peer->get[objname].set_object();
                            unsigned int iii = peer->get[objname].size();
                            key1name         = std::to_string(iii);
                            peer->get[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = header_input;

                            peer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40014;
                                return;
                            }

                            if (key1name.size() > 48)
                            {
                                error = 40015;
                                return;
                            }

                            peer->get[objname].set_object();
                            peer->get[objname][key1name].set_object();

                            unsigned iii = peer->get[objname][key1name].size();
                            key2name     = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = header_input;

                            peer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40016;
                                return;
                            }

                            if (key1name.size() > 48)
                            {
                                error = 40017;
                                return;
                            }

                            if (key2name.size() > 48)
                            {
                                error = 40018;
                                return;
                            }

                            peer->get[objname].set_object();
                            peer->get[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = header_input;
                            peer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == header_temp.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        if (objname.size() > 48)
                        {
                            error = 40019;
                            return;
                        }

                        peer->get[objname].set_object();
                        unsigned int iii = peer->get[objname].size();
                        key1name         = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp = header_input;
                        peer->get[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40020;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40021;
                            return;
                        }
                        peer->get[objname].set_object();
                        http::obj_val objtemp;
                        objtemp = header_input;
                        peer->get[objname].push(key1name, std::move(objtemp));
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
                objname.push_back(header_temp[j]);
            }
        }
        else
        {
            objname.push_back(header_temp[j]);
        }
    }
    if (isgroup)
    {
        peer->get[header_temp] = header_input;
    }
}

void httpparse::methodprocess(std::string_view contentline)
{
    unsigned char headerstep = 0;
    unsigned int ioffset     = 0, linesize;
    unsigned int j           = 0;
    linesize                 = contentline.size();
    std::string header_key;
    std::string header_value;
    std::string header_temp;
    for (; ioffset < linesize; ioffset++)
    {
        if (contentline[ioffset] == 0x20)
        {
            break;
        }
        header_key.push_back(contentline[ioffset]);
        j++;
        if (j > 9)
        {
            error = 40057;
            return;
        }
    }
    if (header_key.size() > 8)
    {
        error = 40058;
        return;
    }
    method = HEAD_METHOD::UNKNOW;
    switch (header_key[0])
    {
    case 'c':
    case 'C':
        if (str_casecmp(header_key, "CONNECT"))
        {
            method = HEAD_METHOD::CONNECT;
        }
        else
        {
            error = 40059;
        }
        break;
    case 'h':
    case 'H':
        if (str_casecmp(header_key, "HEAD"))
        {
            method = HEAD_METHOD::HEAD;
        }
        else
        {
            error = 40060;
        }
        break;
    case 'g':
    case 'G':
        if (str_casecmp(header_key, "GET"))
        {
            method = HEAD_METHOD::GET;
        }
        else
        {
            error = 40061;
        }
        break;
    case 'p':
    case 'P':

        if (str_casecmp(header_key, "POST"))
        {
            method = HEAD_METHOD::POST;
        }
        else if (str_casecmp(header_key, "PUT"))
        {
            method = HEAD_METHOD::PUT;
        }
        else
        {
            error = 40062;
        }

        break;
    case 'o':
    case 'O':
        if (str_casecmp(header_key, "OPTIONS"))
        {
            method = HEAD_METHOD::OPTIONS;
        }
        else
        {
            error = 40063;
        }
        break;
    case 'q':
    case 'Q':
        if (str_casecmp(header_key, "QUERY"))
        {
            method = HEAD_METHOD::QUERY;
        }
        else
        {
            error = 40064;
        }
        break;
    case 't':
    case 'T':
        if (str_casecmp(header_key, "TRACE"))
        {
            method = HEAD_METHOD::TRACE;
        }
        else
        {
            error = 40065;
        }
        break;
    default:
        method = HEAD_METHOD::UNKNOW;
        error  = 40066;
    }
    if (error > 0)
    {
        return;
    }
    peer->method = static_cast<unsigned char>(method);
    headerstep   = 1;
    for (; ioffset < linesize; ioffset++)
    {
        if (contentline[ioffset] == 0x20)
        {
            continue;
        }
        break;
    }
    header_value.clear();
    header_key.clear();
    header_temp.clear();
    // pathinfo.clear();
    //  url.clear();
    peer->pathinfos.clear();
    unsigned int p_begin = ioffset;
    for (; ioffset < linesize; ioffset++)
    {
        if (contentline[ioffset] == 0x20)
        {
            if ((ioffset + 10) >= linesize)
            {
                headerstep = 2;
                break;
            }
        }
        if (contentline[ioffset] == 0x3F)
        {
            headerstep = 6;
            break;
        }
        if (contentline[ioffset] == 0x2F)
        {
            if (header_temp.size() > 255)
            {
                error = 40007;
                return;
            }
            if (header_temp.size() > 0)
            {
                if (header_temp.size() == 2 && header_temp[0] == '.' && header_temp[1] == '.')
                {
                    if (peer->pathinfos.size() > 0)
                    {
                        peer->pathinfos.pop_back();
                    }
                }
                else if (header_temp.size() == 1 && header_temp[0] == '.')
                {
                }
                else
                {
                    peer->pathinfos.emplace_back(http::url_decode(header_temp.data(), header_temp.length()));
                }
                header_temp.clear();
            }
        }
        else
        {
            header_temp.push_back(contentline[ioffset]);
        }
    }
    if (header_temp.size() > 0)
    {
        if (header_temp.size() > 255)
        {
            error = 40090;
            return;
        }
        if (header_temp.size() == 2 && header_temp[0] == '.' && header_temp[1] == '.')
        {
            if (peer->pathinfos.size() > 0)
            {
                peer->pathinfos.pop_back();
            }
        }
        else if (header_temp.size() == 1 && header_temp[0] == '.')
        {
        }
        else
        {
            peer->pathinfos.emplace_back(http::url_decode(header_temp.data(), header_temp.length()));
        }
    }
    unsigned int p_pos_offset = ioffset - p_begin;
    peer->header["urlpath"]   = contentline.substr(p_begin, p_pos_offset);
    if (peer->pathinfos.size() > 0)
    {
        peer->urlpath.clear();
        if (p_pos_offset > 64 && p_pos_offset < 10000)
        {
            peer->urlpath.reserve(p_pos_offset);
        }

        for (unsigned int nn = 0; nn < peer->pathinfos.size(); nn++)
        {
            peer->urlpath.push_back('/');
            peer->urlpath.append(peer->pathinfos[nn]);
        }
        if (peer->urlpath.size() == 0)
        {
            peer->urlpath = "/";
        }
    }
    else
    {
        peer->urlpath.clear();
        peer->urlpath = "/";
    }

    header_key.clear();
    if (headerstep == 6)
    {
        for (; ioffset < linesize; ioffset++)
        {
            if (contentline[ioffset] == 0x3F)
            {
                continue;
            }
            break;
        }
        p_pos_offset = ioffset;
        for (; ioffset < linesize; ioffset++)
        {
            if (contentline[ioffset] == 0x20)
            {
                if ((ioffset + 10) >= linesize)
                {
                    headerstep = 7;
                    break;
                }
            }
        }
        header_key = contentline.substr(p_pos_offset, ioffset - p_pos_offset);
        peer->url  = peer->urlpath;
        peer->url.push_back(0x3F);
        peer->url.append(header_key);
        peer->querystring = header_key;
    }
    else
    {
        peer->url = peer->urlpath;
        peer->querystring.clear();
    }

    if (headerstep == 7)
    {
        // parameter
        header_value.clear();
        header_temp.clear();
        header_temp.clear();
        std::string url_keyname;
        unsigned int qsize    = header_key.size();
        unsigned char partype = 0;
        for (j = 0; j < qsize; j++)
        {
            if (header_key[j] == 0x3D)
            {
                for (; j < qsize; j++)
                {
                    if (header_key[j] == 0x3D)
                    {
                        continue;
                    }
                    else
                    {
                        j -= 1;
                        break;
                    }
                }
                url_keyname = http::url_decode(header_value.data(), header_value.length());
                header_value.clear();
                partype = 1;
                continue;
            }
            else if (header_key[j] == 0x26)
            {
                for (; j < qsize; j++)
                {
                    if (header_key[j] == 0x26)
                    {
                        continue;
                    }
                    else
                    {
                        j -= 1;
                        break;
                    }
                }
                header_temp = http::url_decode(header_value.data(), header_value.length());

                if (url_keyname.size() > 72)
                {
                    error = 40067;
                    return;
                }
                procssparamter(url_keyname, header_temp);
                header_value.clear();
                url_keyname.clear();
                partype = 2;
                continue;
            }
            header_value.push_back(header_key[j]);
        }
        if (partype == 1)
        {
            header_temp = http::url_decode(header_value.data(), header_value.length());
            if (url_keyname.size() > 72)
            {
                error = 40068;
                return;
            }
            procssparamter(url_keyname, header_temp);
        }
        else if (partype == 2)
        {
            url_keyname = http::url_decode(header_value.data(), header_value.length());

            if (url_keyname.size() > 72)
            {
                error = 40069;
                return;
            }
            procssparamter(url_keyname, "");
        }
        else if (header_value.size() > 0)
        {
            url_keyname = http::url_decode(header_value.data(), header_value.length());
            if (url_keyname.size() > 72)
            {
                error = 40070;
                return;
            }
            procssparamter(url_keyname, "");
        }
    }

    headerstep = 8;
}

void httpparse::callposttype(std::string_view buffer_value)
{
    switch (buffer_value.size())
    {
    case 33:
        if (str_casecmp(buffer_value, "application/x-www-form-urlencoded"))
        {
            peer->accept_type = "application/x-www-form-urlencoded";
            posttype          = 1;
            peer->posttype    = 1;
            return;
        }
        break;
    case 24:
        if (str_casecmp(buffer_value, "application/octet-stream"))
        {
            peer->accept_type = "application/octet-stream";
            posttype          = 5;
            peer->posttype    = 5;
            return;
        }
        break;
    case 19:
        if (str_casecmp(buffer_value, "multipart/form-data"))
        {
            peer->accept_type = "multipart/form-data";
            peer->posttype    = 2;
            posttype          = 2;
            return;
        }
        break;
    case 20:
        if (str_casecmp(buffer_value, "application/xslt+xml"))
        {
            peer->accept_type = "application/xslt+xml";
            peer->posttype    = 4;
            posttype          = 4;
            return;
        }
        break;
    case 16:
        if (str_casecmp(buffer_value, "application/json"))
        {
            peer->accept_type = "application/json";
            peer->posttype    = 3;
            posttype          = 3;
            return;
        }
        break;
    case 15:
        if (str_casecmp(buffer_value, "application/xml"))
        {
            peer->accept_type = "application/xml";
            peer->posttype    = 4;
            posttype          = 4;
            return;
        }
        break;
    case 8:
        if (str_casecmp(buffer_value, "text/xml"))
        {
            peer->accept_type = "text/xml";
            peer->posttype    = 4;
            posttype          = 4;
            return;
        }
        break;
    case 6:
        if (str_casecmp(buffer_value, "binary"))
        {
            peer->accept_type = "binary";
            peer->posttype    = 5;
            posttype          = 5;
            return;
        }
        break;
    default:
        peer->accept_type = buffer_value;
        peer->posttype    = 5;
        posttype          = 5;
        return;
    }

    return;
}
void httpparse::getcontenttype(std::string_view header_value)
{
    unsigned int i = 0, linesize;
    linesize       = header_value.size();
    std::string buffer_value;
    unsigned char statetemp = 0;

    if (temp_post_data == nullptr)
    {
        temp_post_data = std::make_unique<HTTP_POST_DATA_T>();
    }

    for (; i < linesize; i++)
    {

        if (header_value[i] == 0x3B)
        {

            if (statetemp == 0)
            {
                callposttype(buffer_value);
            }
            else if (statetemp == 1)
            {
                peer->chartset = buffer_value;
                statetemp      = 0;
            }
            else if (statetemp == 2)
            {
                temp_post_data->boundary = buffer_value;
                statetemp                = 0;
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
            peer->chartset = buffer_value;
        }
        else if (statetemp == 2)
        {
            temp_post_data->boundary = buffer_value;
        }
        else
        {
            callposttype(buffer_value);
        }
    }
}

void httpparse::getrange(std::string_view header_value)
{
    unsigned int j = 0, linesize;
    linesize       = header_value.size();
    std::string buffer_value;
    for (; j < linesize; j++)
    {
        if (header_value[j] == 0x3D)
        {
            j++;
            break;
        }
        buffer_value.push_back(header_value[j]);
    }

    if (str_casecmp(buffer_value, "bytes"))
    {
        // state.rangebytes = true;
        peer->state.rangebytes = true;
    }
    buffer_value.clear();
    bool ismuilt = false;
    for (; j < linesize; j++)
    {
        if (header_value[j] == 0x2C)
        {
            j++;
            ismuilt = true;
            break;
        }
        if (header_value[j] == 0x2D)
        {

            long long tm = 0;
            for (unsigned int qi = 0; qi < buffer_value.size(); qi++)
            {
                if (buffer_value[qi] < 0x3A && buffer_value[qi] > 0x2F)
                {
                    tm = tm * 10 + (buffer_value[qi] - 0x30);
                }
                if (qi >= 14)
                {
                    error = 40024;
                    return;
                }
            }
            // state.rangebegin = tm;
            peer->state.rangebegin = tm;
            buffer_value.clear();
            continue;
        }
        buffer_value.push_back(header_value[j]);
    }

    if (j == header_value.length() || ismuilt)
    {
        long long tm = 0;
        for (unsigned int qi = 0; qi < buffer_value.size(); qi++)
        {
            if (buffer_value[qi] < 0x3A && buffer_value[qi] > 0x2F)
            {
                tm = tm * 10 + (buffer_value[qi] - 0x30);
            }
            if (qi >= 14)
            {
                error = 40091;
                return;
            }
        }
        // state.rangeend = tm;
        peer->state.rangeend = tm;
    }
}

void httpparse::process_header_line(std::string_view line_str)
{
    if (!isfinish_url)
    {
        methodprocess(line_str);
        isfinish_url = true;
        if (error > 0)
        {
            return;
        }
        return;
    }
    std::string_view header_key;
    std::string_view header_value;
    for (unsigned int j = 0; j < line_str.size(); j++)
    {
        if (line_str[j] == ':')
        {
            header_key = line_str.substr(0, j);
            j          = j + 1;
            while (j < line_str.size() && line_str[j] == ' ')
                ++j;
            header_value = line_str.substr(j);
            break;
        }
    }
    if (header_key.size() == 0)
    {
        error = 40003;
        return;
    }

    if (header_key.size() > 72)
    {
        error = 40004;
        return;
    }

    std::string key;
    key.resize(header_key.size());
    std::transform(header_key.begin(), header_key.end(), key.begin(), [](unsigned char c)
                   { return std::tolower(c); });
    peer->header.insert_or_assign(std::move(key), header_value);

    switch (header_key.size())
    {
    case 24:
        if (str_casecmp(header_key, "Sec-WebSocket-Extensions"))
        {
            if (websocket == nullptr)
            {
                websocket = std::make_unique<websocket_t>();
            }
            getwebsocketextensions(header_value);
        }
        break;
    case 21:
        if (str_casecmp(header_key, "Sec-WebSocket-Version"))
        {
            if (websocket == nullptr)
            {
                websocket = std::make_unique<websocket_t>();
            }
            websocket->version = (unsigned char)header_valuetoint(header_value);
        }
        break;
    case 19:
        if (str_casecmp(header_key, "If-Unmodified-Since"))
        {
            getifunmodifiedsince(header_value);
        }
        break;
    case 17:

        switch (header_key[0])
        {
        case 's':
        case 'S':
            if (str_casecmp(header_key, "Sec-WebSocket-Key"))
            {
                if (websocket == nullptr)
                {
                    websocket = std::make_unique<websocket_t>();
                }
                websocket->key = header_value;
            }
            break;
        case 'i':
        case 'I':
            if (str_casecmp(header_key, "If-Modified-Since"))
            {
                getifmodifiedsince(header_value);
            }
            break;
        default:;
        }
        break;
    case 15:
        if (header_key[7] == 'E' || header_key[7] == 'e')
        {
            if (str_casecmp(header_key, "Accept-Encoding"))
            {
                getacceptencoding(header_value);
                break;
            }
        }
        else if (header_key[7] == 'L' || header_key[7] == 'l')
        {
            if (str_casecmp(header_key, "Accept-Language"))
            {
                getacceptlanguage(header_value);
                break;
            }
        }

        if (str_casecmp(header_key, "Accept-Encoding"))
        {
            getacceptencoding(header_value);
        }
        if (str_casecmp(header_key, "Accept-Language"))
        {
            getacceptlanguage(header_value);
        }
        // Accept-Language

        break;
    case 14:
        if (str_casecmp(header_key, "Content-Length"))
        {
            peer->content_length = header_valuetoint(header_value);
        }
        break;
    case 13:
        if (str_casecmp(header_key, "If-None-Match"))
        {
            getifnonematch(header_value);
        }
        break;
    case 12:
        if (str_casecmp(header_key, "Content-Type"))
        {

            getcontenttype(header_value);
        }
        break;
    case 10:
        switch (header_key[0])
        {
        case 'c':
        case 'C':
            if (str_casecmp(header_key, "Connection"))
            {
                getconnection(header_value);
                peer->keepalive = peer->state.keepalive;
            }
            break;
        case 'u':
        case 'U':
            // if (str_casecmp(header_key, "User-Agent"))
            {
                // useragent = header_value;
                // peer->header["User-Agent"] = header_value;
            }
            break;
        }
        break;
    case 8:
        if (str_casecmp(header_key, "If-Match"))
        {
            getifnonematch(header_value);
        }
        break;
    case 7:
        if (str_casecmp(header_key, "Upgrade"))
        {
            getupgrade(header_value);
        }
        break;
    case 6:

        switch (header_key[0])
        {
        case 'c':
        case 'C':
            if (str_casecmp(header_key, "Cookie"))
            {
                getcookie(header_value);
            }
            break;
        case 'a':
        case 'A':
            if (str_casecmp(header_key, "Accept"))
            {

                getaccept(header_value);
            }
            break;
        }
        break;
    case 5:
        if (str_casecmp(header_key, "Range"))
        {
            getrange(header_value);
        }
        break;
    case 4:
        if (str_casecmp(header_key, "Host"))
        {
            getheaderhost(header_value);
        }
        break;
    }
}

void httpparse::getaccept(std::string_view header_value)
{
    unsigned int i = 0, linesize;
    linesize       = header_value.size();

    std::string buffer_value;

    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C || header_value[i] == 0x3B)
        {
            switch (buffer_value.length())
            {
            case 10:
                if (buffer_value[6] == 'a' && buffer_value[7] == 'v' && buffer_value[8] == 'i' &&
                    buffer_value[9] == 'f')
                {
                    peer->state.avif = true;
                }
                else if (buffer_value[6] == 'w' && buffer_value[7] == 'e' && buffer_value[8] == 'b' &&
                         buffer_value[9] == 'p')
                {
                    peer->state.webp = true;
                }
                break;
            case 16:
                //application/json
                if (str_casecmp(buffer_value, "application/json"))
                {
                    peer->state.accept_json = true;
                }
                break;
            case 15:
                //application/xml
                if (str_casecmp(buffer_value, "application/xml"))
                {
                    peer->state.accept_xml = true;
                }
                break;
            case 9:
                //text/json
                if (str_casecmp(buffer_value, "text/json"))
                {
                    peer->state.accept_xml = true;
                }
                break;
            case 8:
                //text/xml
                if (str_casecmp(buffer_value, "text/xml"))
                {
                    peer->state.accept_xml = true;
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
            if (buffer_value[6] == 'a' && buffer_value[7] == 'v' && buffer_value[8] == 'i' && buffer_value[9] == 'f')
            {
                peer->state.avif = true;
            }
            else if (buffer_value[6] == 'w' && buffer_value[7] == 'e' && buffer_value[8] == 'b' &&
                     buffer_value[9] == 'p')
            {
                peer->state.webp = true;
            }
            break;
        case 16:
            //application/json
            if (str_casecmp(buffer_value, "application/json"))
            {
                peer->state.accept_json = true;
            }
            break;
        case 15:
            //application/xml
            if (str_casecmp(buffer_value, "application/xml"))
            {
                peer->state.accept_xml = true;
            }
            break;
        case 9:
            //text/json
            if (str_casecmp(buffer_value, "text/json"))
            {
                peer->state.accept_xml = true;
            }
            break;
        case 8:
            //text/xml
            if (str_casecmp(buffer_value, "text/xml"))
            {
                peer->state.accept_xml = true;
            }
            break;
        default:;
        }
    }
}

void httpparse::getacceptlanguage(std::string_view header_value)
{
    unsigned int i = 0;
    for (; i < header_value.size() && i < 7; i++)
    {
        if (header_value[i] == 0x2C)
        {
            break;
        }
        peer->state.language[i] = header_value[i];
    }
    for (; i < 8; i++)
    {
        peer->state.language[i] = 0x00;
    }
}
void httpparse::getifunmodifiedsince(std::string_view header_value) { peer->state.ifunmodifiedsince = strgmttotime(header_value); }
void httpparse::getifmodifiedsince(std::string_view header_value) { peer->state.ifmodifiedsince = strgmttotime(header_value); }
void httpparse::getifnonematch(std::string_view header_value)
{
    unsigned int i = 0;
    peer->etag.clear();
    if (header_value.size() < 2)
    {
        error = 40071;
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
            peer->etag.push_back(header_value[i]);
        }
    }
}
void httpparse::getwebsocketextensions(std::string_view header_value)
{
    if (websocket == nullptr)
    {
        return;
    }

    switch (header_value.length())
    {
    case 22:
        if (str_casecmp(header_value, "x-webkit-deflate-frame"))
        {
            websocket->deflateframe = true;
            websocket->deflate      = true;
        }
        break;
    case 18:

        if (str_casecmp(header_value, "permessage-deflate"))
        {
            websocket->permessagedeflate = true;
            websocket->deflate           = true;
        }
        break;
    case 16:
        if (str_casecmp(header_value, "perframe-deflate"))
        {
            websocket->perframedeflate = true;
            websocket->deflate         = true;
        }
        break;
    default:;
    }
    if (websocket->deflate)
    {
        return;
    }

    if (header_value.find("server_max_window_bits") != std::string::npos)
    {
        unsigned int pos_b = header_value.find("server_max_window_bits") + 20;
        unsigned int j     = pos_b;
        websocket->bits    = 0;
        for (; pos_b < header_value.size(); pos_b++)
        {
            if (header_value[pos_b] == '=')
            {
                pos_b++;
                for (; pos_b < header_value.size(); pos_b++)
                {
                    if (header_value[pos_b] == 0x20)
                    {
                        continue;
                    }
                    break;
                }

                for (; pos_b < header_value.size(); pos_b++)
                {
                    if (header_value[pos_b] >= '0' && header_value[pos_b] <= '9')
                    {
                        if (websocket->bits > 15)
                        {
                            break;
                        }
                        websocket->bits = websocket->bits * 10 + (header_value[pos_b] - '0');
                        continue;
                    }
                    break;
                }
                break;
            }
            j++;
            if (j > 5)
            {
                break;
            }
        }
        //has server_max_window_bits
        if (websocket->bits == 0)
        {
            websocket->bits = 1;
        }
    }

    if (header_value.find("permessage-deflate") != std::string::npos)
    {
        websocket->permessagedeflate = true;
        websocket->deflate           = true;
        return;
    }
    if (header_value.find("perframe-deflate") != std::string::npos)
    {
        websocket->perframedeflate = true;
        websocket->deflate         = true;
        return;
    }
    if (header_value.find("x-webkit-deflate-frame") != std::string::npos)
    {
        websocket->deflateframe = true;
        websocket->deflate      = true;
        return;
    }
}

void httpparse::getupgrade(std::string_view header_value)
{

    if (header_value.size() > 9)
    {
        if (header_value.back() == 0x20)
        {
            header_value.remove_suffix(1);
        }
    }

    if (str_casecmp(header_value, "websocket"))
    {
        // state.websocket = true;
        peer->state.websocket = true;
    }
    else if (str_casecmp(header_value, "h2c"))
    {
        // state.h2c = true;
        peer->state.h2c = true;
    }
}

void httpparse::getconnection(std::string_view header_value)
{
    if (header_value.size() < 2)
    {
        error = 40072;
        return;
    }
    if (header_value[0] == 'K' || header_value[0] == 'k')
    {
        if (header_value[1] == 'e' || header_value[1] == 'E')
        {
            peer->state.keepalive = true;
            return;
        }
    }
    if (str_casecmp(header_value, "keep-alive"))
    {
        peer->state.keepalive = true;
        return;
    }
    if (str_casecmp(header_value, "Upgrade"))
    {
        peer->state.upgradeconnection = true;
        return;
    }
    if (header_value[0] == 'c' || header_value[0] == 'C')
    {
        if (header_value[1] == 'l' || header_value[1] == 'L')
        {
            peer->state.keepalive = false;
        }
    }
}

void httpparse::getacceptencoding(std::string_view header_value)
{
    unsigned int i = 0, linesize;
    linesize       = header_value.size();
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
                    // state.br = true;
                    peer->state.br = true;
                }
                break;
            case 4:
                if (buffer_value[0] == 'g')
                {
                    // state.gzip = true;
                    peer->state.gzip = true;
                }
                else if (buffer_value[0] == 'z')
                {
                    peer->state.zstd = true;
                }
                break;
            case 7:
                if (buffer_value[0] == 'd')
                {
                    // state.deflate = true;
                    peer->state.deflate = true;
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
                // state.br = true;
                peer->state.br = true;
            }
            break;
        case 4:
            if (buffer_value[0] == 'g')
            {
                // state.gzip = true;
                peer->state.gzip = true;
            }
            else if (buffer_value[0] == 'z')
            {
                peer->state.zstd = true;
            }
            break;
        case 7:
            if (buffer_value[0] == 'd')
            {
                // state.deflate = true;
                peer->state.deflate = true;
            }
            break;
        default:;
        }
        buffer_value.clear();
    }
}

void httpparse::getcookie(std::string_view header_value)
{
    unsigned int i = 0, linesize;
    linesize       = header_value.size();
    std::string buffer_key;
    std::string buffer_value;
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
            // cookie[buffer_key] = buffer_value;
            if (buffer_key.size() > 72)
            {
                error = 40073;
                return;
            }
            peer->cookie[buffer_key] = buffer_value;
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
        buffer_value = http::url_decode(buffer_value.data(), buffer_value.length());
        // cookie[buffer_key] = buffer_value;
        if (buffer_key.size() > 72)
        {
            error = 40074;
            return;
        }
        peer->cookie[buffer_key] = buffer_value;
    }
    else
    {
        if (buffer_key.size() > 72)
        {
            error = 40075;
            return;
        }
        if (buffer_key.size() > 0)
        {
            // cookie[buffer_key] = "";
            peer->cookie[buffer_key] = "";
        }
        buffer_key.clear();
    }
}

void httpparse::getheaderhost(std::string_view header_value)
{
    unsigned int ioffset    = 0, linesize;
    linesize                = header_value.size();
    unsigned char ishasport = 0;
    port                    = 0;
    // check hostport 检查有没有定制端口
    peer->host.clear();
    for (; ioffset < linesize; ioffset++)
    {
        if (header_value[ioffset] == 0x3A)
        {
            ishasport = 1;
            ioffset++;
            break;
        }
        else if (header_value[ioffset] >= '0' && header_value[ioffset] <= '9')
        {
            peer->host.push_back(header_value[ioffset]);
        }
        else if (header_value[ioffset] >= 'a' && header_value[ioffset] <= 'z')
        {
            peer->host.push_back(header_value[ioffset]);
        }
        else if (header_value[ioffset] >= 'A' && header_value[ioffset] <= 'Z')
        {
            peer->host.push_back(header_value[ioffset] + 32);
        }
        else if (header_value[ioffset] == '.' || header_value[ioffset] == '-')
        {
            peer->host.push_back(header_value[ioffset]);
        }
        else if (header_value[ioffset] == '[')
        {
            if (peer->host.size() > 0)
            {
                error = 40076;
                return;
            }

            ioffset++;
            for (; ioffset < linesize; ioffset++)
            {
                if (header_value[ioffset] >= '0' && header_value[ioffset] <= '9')
                {
                    peer->host.push_back(header_value[ioffset]);
                }
                else if (header_value[ioffset] >= 'a' && header_value[ioffset] <= 'f')
                {
                    peer->host.push_back(header_value[ioffset]);
                }
                else if (header_value[ioffset] >= 'A' && header_value[ioffset] <= 'F')
                {
                    peer->host.push_back(header_value[ioffset] + 32);
                }
                else if (header_value[ioffset] == ':')
                {
                    peer->host.push_back(header_value[ioffset]);
                }
                else if (header_value[ioffset] == ']')
                {
                    ioffset++;
                    if (ioffset < linesize && header_value[ioffset] == 0x3A)
                    {
                        ishasport = 1;
                        ioffset++;
                        break;
                    }
                    break;
                }
                else
                {
                    error = 40077;
                    return;
                }
            }
        }
        else
        {
            error = 40078;
            return;
        }
    }
    if (peer->host.size() > 72)
    {
        error = 40079;
        return;
    }
    peer->find_host_index();
    if (ishasport == 1)
    {
        for (; ioffset < linesize; ioffset++)
        {
            if (header_value[ioffset] < 0x3A && header_value[ioffset] > 0x2F)
            {
                port = port * 10 + (header_value[ioffset] - 0x30);
            }
            if (port > 65535)
            {
                error = 40080;
                return;
            }
        }
    }
    // state.port = port;
    peer->state.port = port;
}

void httpparse::read_http_header_block(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int pos_m = readoffset;

    if (pre_http_header.size() > 0)
    {
        pre_http_header.clear();
        if (readoffset < buffersize && buffer[readoffset] == '\n')
        {
            pos_m++;
            readoffset = pos_m;
            if (header_line.size() == 0)
            {
                isfinish_header = true;
                return;
            }
            process_header_line(header_line);
            header_line.clear();
        }
        else
        {
            error = 40001;
            return;
        }
    }

    for (; pos_m < buffersize; pos_m++)
    {
        if (buffer[pos_m] == '\r')
        {
            unsigned int j = pos_m;
            j++;
            if (j < buffersize && buffer[j] == '\n')
            {
                header_line.append((char *)&buffer[readoffset], pos_m - readoffset);
                readoffset = j + 1;
                if (header_line.size() == 0)
                {
                    isfinish_header = true;
                    return;
                }
                http_content_length += header_line.size();

                process_header_line(header_line);
                header_line.clear();
                return;
            }
            else
            {
                pre_http_header = "\r";
                header_line.append((char *)&buffer[readoffset], pos_m - readoffset);
                readoffset = buffersize;
                http_content_length += header_line.size();
                return;
            }
        }
    }
    header_line.append((char *)&buffer[readoffset], pos_m - readoffset);
    readoffset = buffersize;

    if (header_line.size() > CONST_HTTP_HEADER_BODY_SIZE)
    {
        error = 40002;
        return;
    }
}

void httpparse::post_form_to_postfield(std::string_view form_post_name, std::string_view form_post_value)
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
        peer->post[form_post_name] = form_post_value;
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
                                error = 40022;
                                return;
                            }

                            peer->post[objname].set_object();
                            unsigned int iii = peer->post[objname].size();
                            key1name         = std::to_string(iii);
                            peer->post[objname][key1name].set_object();

                            iii      = peer->post[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            peer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40023;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40025;
                                return;
                            }

                            peer->post[objname].set_object();
                            unsigned int iii = peer->post[objname].size();
                            key1name         = std::to_string(iii);
                            peer->post[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            peer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40033;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40037;
                                return;
                            }

                            peer->post[objname].set_object();
                            peer->post[objname][key1name].set_object();

                            unsigned int iii = peer->post[objname][key1name].size();
                            key2name         = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            peer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {

                            if (objname.size() > 48)
                            {
                                error = 40039;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40040;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40041;
                                return;
                            }

                            peer->post[objname].set_object();
                            peer->post[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            peer->post[objname][key1name].push(key2name, std::move(objtemp));
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
                            error = 40042;
                            return;
                        }

                        peer->post[objname].set_object();
                        unsigned int iii = peer->post[objname].size();
                        key1name         = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp = form_post_value;

                        peer->post[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40043;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40044;
                            return;
                        }
                        peer->post[objname].set_object();

                        http::obj_val objtemp;
                        objtemp = form_post_value;

                        peer->post[objname].push(key1name, std::move(objtemp));
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
        peer->post[form_post_name] = form_post_value;
    }
}

void httpparse::post_www_form_urlencoded(std::string_view raw_post_urlencode_content)
{
    std::string temp_value;
    std::string buffer_key;
    std::string field_value;
    unsigned int qsize    = raw_post_urlencode_content.size();
    unsigned char partype = 0;
    unsigned int j        = 0;
    unsigned int jj       = 0;
    for (j = 0; j < qsize; j++)
    {
        if (raw_post_urlencode_content[j] == 0x3D)
        {
            buffer_key = http::url_decode(temp_value.data(), temp_value.length());
            temp_value.clear();
            partype = 1;
            jj      = 0;
            continue;
        }
        else if (raw_post_urlencode_content[j] == 0x26)
        {
            field_value = http::url_decode(temp_value.data(), temp_value.length());
            if (buffer_key.size() > 72)
            {
                error = 40083;
                return;
            }
            post_form_to_postfield(buffer_key, field_value);
            temp_value.clear();
            field_value.clear();
            partype = 2;
            jj      = 0;
            continue;
        }
        temp_value.push_back(raw_post_urlencode_content[j]);

        if (partype == 0 || partype == 2)
        {
            //key name too long
            if (jj > 72)
            {
                error = 40084;
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
            error = 40085;
            return;
        }
        post_form_to_postfield(buffer_key, field_value);
    }
    else if (partype == 2)
    {
        buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        field_value.clear();
        if (buffer_key.size() > 72)
        {
            error = 40086;
            return;
        }
        post_form_to_postfield(buffer_key, field_value);
    }
    else if (temp_value.size() > 0)
    {
        buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        field_value.clear();
        if (buffer_key.size() > 72)
        {
            error = 40087;
            return;
        }
        post_form_to_postfield(buffer_key, field_value);
    }
}

void httpparse::read_rawfile_formdata(std::string_view raw_http_post_data)
{
    if (!uprawfile)
    {
        server_loaclvar &localvar    = get_server_global_var();
        std::string fieldheader_temp = "rawcontent" + std::to_string(http::timeid()) + rand_string(6, 0);

        std::string temp_filename = localvar.temp_path;// + "temp/";
        temp_filename.append(std::to_string(std::hash<std::string>{}(fieldheader_temp)));

        std::unique_ptr<std::FILE, int (*)(FILE *)> fpa(std::fopen(temp_filename.c_str(), "wb"), std::fclose);
        if (fpa)
        {
            uprawfile = std::move(fpa);
        }
        else
        {
            error = 40093;
            return;
        }
        peer->files["rawcontent"].set_object();
        peer->files["rawcontent"]["filename"] = "rawcontent";
        peer->files["rawcontent"]["name"]     = "rawcontent";
        peer->files["rawcontent"]["tempfile"] = temp_filename;
        peer->files["rawcontent"]["type"]     = "";
        peer->files["rawcontent"]["size"]     = 0;
        peer->files["rawcontent"]["error"]    = 0;
    }

    if (uprawfile == nullptr)
    {
        return;
    }
    if (uprawfile)
    {
        size_t n = fwrite(raw_http_post_data.data(), 1, raw_http_post_data.size(), uprawfile.get());
        if (n != raw_http_post_data.size())
        {
            uprawfile.reset();
            error = 40094;
        }
    }

    if (http_content_length >= peer->content_length)
    {
        uprawfile.reset();
        peer->files["rawcontent"]["size"] = http_content_length;
        peer->isfinish                    = true;
    }
}

void httpparse::multipart_post_file_field()
{
    std::string objname;
    bool isgroup = true;

    for (unsigned int j = 0; j < temp_post_data->field_name.length(); j++)
    {
        if (temp_post_data->field_name[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < temp_post_data->field_name.length(); n++)
            {
                if (temp_post_data->field_name[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (temp_post_data->field_name[n] == '[')
                {

                    break;
                }
                if (temp_post_data->field_name[n] != 0x22)
                {
                    key1name.push_back(temp_post_data->field_name[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {
                unsigned int m = n;
                if (n < temp_post_data->field_name.length())
                {
                    if (temp_post_data->field_name[m] == '[')
                    {
                        m += 1;
                        for (; m < temp_post_data->field_name.length(); m++)
                        {
                            if (temp_post_data->field_name[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (temp_post_data->field_name[m] == '[')
                            {

                                break;
                            }
                            if (temp_post_data->field_name[m] != 0x22)
                            {
                                key2name.push_back(temp_post_data->field_name[m]);
                            }
                        }

                        if (ishaskey2 && m == temp_post_data->field_name.length())
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
                                error = 40046;
                                return;
                            }

                            peer->files[objname].set_object();
                            unsigned int iii = peer->files[objname].size();
                            key1name         = std::to_string(iii);
                            peer->files[objname][key1name].set_object();

                            iii      = peer->files[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data->filename;
                            objtemp["name"]     = temp_post_data->field_name;
                            objtemp["tempfile"] = temp_post_data->temp_filename;
                            objtemp["type"]     = temp_post_data->mimetype;
                            objtemp["size"]     = temp_post_data->cur_length;
                            objtemp["error"]    = 0;

                            peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40047;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40048;
                                return;
                            }

                            peer->files[objname].set_object();
                            unsigned int iii = peer->files[objname].size();
                            key1name         = std::to_string(iii);
                            peer->files[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data->filename;
                            objtemp["name"]     = temp_post_data->field_name;
                            objtemp["tempfile"] = temp_post_data->temp_filename;
                            objtemp["type"]     = temp_post_data->mimetype;
                            objtemp["size"]     = temp_post_data->cur_length;
                            objtemp["error"]    = 0;

                            peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40049;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40050;
                                return;
                            }

                            peer->files[objname].set_object();
                            peer->files[objname][key1name].set_object();

                            unsigned iii = peer->files[objname][key1name].size();
                            key2name     = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data->filename;
                            objtemp["name"]     = temp_post_data->field_name;
                            objtemp["tempfile"] = temp_post_data->temp_filename;
                            objtemp["type"]     = temp_post_data->mimetype;
                            objtemp["size"]     = temp_post_data->cur_length;
                            objtemp["error"]    = 0;

                            peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {

                            if (objname.size() > 48)
                            {
                                error = 40051;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40052;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40053;
                                return;
                            }

                            peer->files[objname].set_object();
                            peer->files[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data->filename;
                            objtemp["name"]     = temp_post_data->field_name;
                            objtemp["tempfile"] = temp_post_data->temp_filename;
                            objtemp["type"]     = temp_post_data->mimetype;
                            objtemp["size"]     = temp_post_data->cur_length;
                            objtemp["error"]    = 0;

                            peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == temp_post_data->field_name.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        if (objname.size() > 48)
                        {
                            error = 40054;
                            return;
                        }

                        peer->files[objname].set_object();
                        unsigned iii = peer->files[objname].size();
                        key1name     = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp.set_object();
                        objtemp["filename"] = temp_post_data->filename;
                        objtemp["name"]     = temp_post_data->field_name;
                        objtemp["tempfile"] = temp_post_data->temp_filename;
                        objtemp["type"]     = temp_post_data->mimetype;
                        objtemp["size"]     = temp_post_data->cur_length;
                        objtemp["error"]    = 0;

                        peer->files[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40055;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40056;
                            return;
                        }
                        peer->files[objname].set_object();

                        http::obj_val objtemp;
                        objtemp.set_object();
                        objtemp["filename"] = temp_post_data->filename;
                        objtemp["name"]     = temp_post_data->field_name;
                        objtemp["tempfile"] = temp_post_data->temp_filename;
                        objtemp["type"]     = temp_post_data->mimetype;
                        objtemp["size"]     = temp_post_data->cur_length;
                        objtemp["error"]    = 0;

                        peer->files[objname].push(key1name, std::move(objtemp));
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
                objname.push_back(temp_post_data->field_name[j]);
            }
        }
        else
        {
            objname.push_back(temp_post_data->field_name[j]);
        }
    }
    if (isgroup)
    {
        peer->files[temp_post_data->field_name].set_object();
        peer->files[temp_post_data->field_name]["filename"] = temp_post_data->filename;
        peer->files[temp_post_data->field_name]["name"]     = temp_post_data->field_name;
        peer->files[temp_post_data->field_name]["tempfile"] = temp_post_data->temp_filename;
        peer->files[temp_post_data->field_name]["type"]     = temp_post_data->mimetype;
        peer->files[temp_post_data->field_name]["size"]     = temp_post_data->cur_length;
        peer->files[temp_post_data->field_name]["error"]    = 0;
    }
}

void httpparse::post_multipart_itemcontent_append()
{
    //only process upload file
    if (temp_post_data->temp_filename.size() > 0)
    {
        if (temp_post_data->fp)
        {
            if (temp_post_data->field_item.size() == 0)
            {
                return;
            }
            temp_post_data->cur_length += temp_post_data->field_item.size();
            size_t n = fwrite(&temp_post_data->field_item[0], 1, temp_post_data->field_item.size(), temp_post_data->fp.get());
            if (n != temp_post_data->field_item.size())
            {
                error = 40038;
                temp_post_data->fp.reset();
                std::remove(temp_post_data->temp_filename.c_str());// 再删除孤儿文件
                temp_post_data->field_item.clear();
                return;
            }
            temp_post_data->field_item.clear();
            return;
        }
    }
}
void httpparse::post_multipart_itemcontent(bool isfilefull)
{
    if (temp_post_data->field_item.size() == 0)
    {
        return;
    }

    unsigned int pos_m = 0;
    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] == '\r' || temp_post_data->field_item[pos_m] == '\n' || temp_post_data->field_item[pos_m] == '-')
        {
            continue;
        }
        break;
    }
    //Content-Disposition
    std::string keyname_field;
    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] == ':')
        {
            break;
        }
        keyname_field.push_back(temp_post_data->field_item[pos_m]);
        if (keyname_field.size() > 64)
        {
            error = 40045;
            return;
        }
    }

    if (pos_m >= temp_post_data->field_item.size())
    {
        return;
    }

    if (!str_casecmp(keyname_field, "Content-Disposition"))
    {
        error = 40026;
        return;
    }

    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] == ';')
        {
            pos_m++;
            break;
        }
    }

    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] != ' ')
        {
            break;
        }
    }
    keyname_field.clear();
    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] == '=')
        {
            pos_m++;
            break;
        }
        keyname_field.push_back(temp_post_data->field_item[pos_m]);
        if (keyname_field.size() > 72)
        {
            error = 40027;
            return;
        }
    }
    if (!str_casecmp(keyname_field, "name"))
    {
        error = 40092;
        return;
    }
    //fieldname
    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] == ' ' || temp_post_data->field_item[pos_m] == '"')
        {
            continue;
        }
        else if (temp_post_data->field_item[pos_m] == '=')
        {
            continue;
        }
        break;
    }

    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] == '"')
        {
            pos_m++;
            break;
        }
        temp_post_data->field_name.push_back(temp_post_data->field_item[pos_m]);
        if (temp_post_data->field_name.size() > 512)
        {
            error = 40028;
            return;
        }
    }

    //filename
    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] != ' ')
        {
            break;
        }
    }
    if (pos_m >= temp_post_data->field_item.size())
    {
        error = 40029;
        return;
    }

    if (temp_post_data->field_item[pos_m] == ';')
    {
        temp_post_data->isfile = true;
        pos_m++;
        keyname_field.clear();
        for (; pos_m < temp_post_data->field_item.size(); pos_m++)
        {
            if (temp_post_data->field_item[pos_m] != ' ')
            {
                break;
            }
        }

        for (; pos_m < temp_post_data->field_item.size(); pos_m++)
        {
            if (temp_post_data->field_item[pos_m] == '=')
            {
                pos_m++;
                break;
            }
            keyname_field.push_back(temp_post_data->field_item[pos_m]);
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

        if (pos_m >= temp_post_data->field_item.size())
        {
            error = 40032;
            return;
        }

        for (; pos_m < temp_post_data->field_item.size(); pos_m++)
        {
            if (temp_post_data->field_item[pos_m] != ' ')
            {
                break;
            }
        }

        for (; pos_m < temp_post_data->field_item.size(); pos_m++)
        {
            if (temp_post_data->field_item[pos_m] == '"')
            {
                pos_m++;
                break;
            }
        }

        for (; pos_m < temp_post_data->field_item.size(); pos_m++)
        {
            if (temp_post_data->field_item[pos_m] == '"')
            {
                pos_m++;
                break;
            }
            temp_post_data->filename.push_back(temp_post_data->field_item[pos_m]);
            if (temp_post_data->filename.size() > 512)
            {
                error = 40034;
                return;
            }
        }
        for (; pos_m < temp_post_data->field_item.size(); pos_m++)
        {
            if (temp_post_data->field_item[pos_m] == '\r')
            {
                pos_m++;
                if (pos_m < temp_post_data->field_item.size())
                {
                    if (temp_post_data->field_item[pos_m] == '\n')
                    {
                        pos_m++;
                    }
                }
                break;
            }
        }
        keyname_field.clear();
        for (; pos_m < temp_post_data->field_item.size(); pos_m++)
        {
            if (temp_post_data->field_item[pos_m] == '\r')
            {
                pos_m++;
                if (pos_m < temp_post_data->field_item.size())
                {
                    if (temp_post_data->field_item[pos_m] == '\n')
                    {
                        pos_m++;
                    }
                }
                break;
            }
            keyname_field.push_back(temp_post_data->field_item[pos_m]);
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
                temp_post_data->mimetype.push_back(keyname_field[i]);
            }
        }
    }
    else
    {
        if (temp_post_data->field_item[pos_m] == '\r')
        {
            pos_m++;
            if (pos_m < temp_post_data->field_item.size())
            {
                if (temp_post_data->field_item[pos_m] == '\n')
                {
                    pos_m++;
                }
            }
        }
    }
    keyname_field.clear();
    for (; pos_m < temp_post_data->field_item.size(); pos_m++)
    {
        if (temp_post_data->field_item[pos_m] == '\r')
        {
            pos_m++;
            if (pos_m < temp_post_data->field_item.size())
            {
                if (temp_post_data->field_item[pos_m] == '\n')
                {
                    pos_m++;
                }
            }
            break;
        }
    }

    std::string_view keyvalue_field(&temp_post_data->field_item[pos_m], temp_post_data->field_item.size() - pos_m);
    if (temp_post_data->isfile)
    {
        server_loaclvar &localvar     = get_server_global_var();
        temp_post_data->temp_filename = localvar.temp_path;
        if (temp_post_data->temp_filename.size() > 0 && temp_post_data->temp_filename.back() != '/')
        {
            temp_post_data->temp_filename.push_back('/');
        }

        temp_post_data->temp_filename = temp_post_data->temp_filename + std::to_string(http::timeid()) + rand_string(6, 0) + "_" + std::to_string(peer->content_length);
        std::unique_ptr<std::FILE, int (*)(FILE *)> fpa(std::fopen(temp_post_data->temp_filename.c_str(), "wb"), std::fclose);
        if (fpa)
        {
            temp_post_data->fp         = std::move(fpa);
            temp_post_data->cur_length = keyvalue_field.size();
            size_t n                   = fwrite(&keyvalue_field[0], 1, keyvalue_field.size(), temp_post_data->fp.get());
            if (n != keyvalue_field.size())
            {
                error = 40036;
                temp_post_data->fp.reset();
                std::remove(temp_post_data->temp_filename.c_str());// 再删除孤儿文件
                return;
            }
        }
        if (isfilefull)
        {
            multipart_post_file_field();
            temp_post_data->fp.reset();
            temp_post_data->isfile = false;
            temp_post_data->temp_filename.clear();
            temp_post_data->filename.clear();
        }
    }
    else
    {
        post_form_to_postfield(temp_post_data->field_name, keyvalue_field);
    }
}

void httpparse::reset_uploadfile()
{
    if (temp_post_data->temp_filename.size() > 0)
    {
        if (temp_post_data->fp)
        {
            post_multipart_itemcontent_append();
            multipart_post_file_field();
        }
    }
    temp_post_data->isfile = false;
    temp_post_data->temp_filename.clear();
    temp_post_data->field_name.clear();
    temp_post_data->filename.clear();
    temp_post_data->fp.reset();
}

void httpparse::post_multipart_formdata()
{
    unsigned int pos_m = temp_post_data->field_offset;

    //处理上次剩余的
    if (temp_post_data->pre_content.size() > 0)
    {
        if (temp_post_data->pre_content.size() == 1)
        {
            bool isnotmatch = true;
            if (temp_post_data->content[pos_m] == '-')
            {
                //说明已经命中两个，接着直接匹配boundary
                pos_m++;
                if ((pos_m + temp_post_data->boundary.size()) > temp_post_data->content.size())
                {
                    //两次数量小于boundary.size()
                    temp_post_data->field_offset = temp_post_data->content.size();
                    return;
                }
                unsigned int i = 0;
                for (; i < temp_post_data->boundary.size(); i++)
                {
                    if (temp_post_data->content[pos_m] != temp_post_data->boundary[i])
                    {
                        pos_m = temp_post_data->field_offset;
                        break;
                    }
                    pos_m++;
                }
                if (i == temp_post_data->boundary.size())
                {
                    isnotmatch = false;
                }
            }
            //说明是假匹配
            temp_post_data->pre_content.clear();
            if (isnotmatch)
            {
                temp_post_data->field_item.push_back('-');
                //重新开始当作没有发生过
                pos_m = temp_post_data->field_offset;
            }
            else
            {
                temp_post_data->field_offset = pos_m;
                if (temp_post_data->field_item.size() > 1)
                {
                    temp_post_data->field_item.resize(temp_post_data->field_item.size() - 2);
                }
                reset_uploadfile();
                post_multipart_itemcontent(true);
                temp_post_data->field_item.clear();
                if ((pos_m + 1) < temp_post_data->content.size())
                {
                    if (temp_post_data->content[pos_m] == '-')
                    {
                        if (temp_post_data->content[pos_m + 1] == '-')
                        {
                            peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data->field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data->content[pos_m] == '\r')
                    {
                        if (temp_post_data->content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data->field_offset = pos_m + 1;
                        }
                    }
                }
                return;
            }
        }
        else if (temp_post_data->pre_content.size() == 2)
        {
            bool isnotmatch = true;
            if ((pos_m + temp_post_data->boundary.size()) > temp_post_data->content.size())
            {
                //两次数量小于boundary.size()
                temp_post_data->field_offset = temp_post_data->content.size();
                return;
            }
            unsigned int i = 0;
            for (; i < temp_post_data->boundary.size(); i++)
            {
                if (temp_post_data->content[pos_m] != temp_post_data->boundary[i])
                {
                    pos_m = temp_post_data->field_offset;
                    break;
                }
                pos_m++;
            }
            if (i == temp_post_data->boundary.size())
            {
                isnotmatch = false;
            }

            //说明是假匹配
            temp_post_data->pre_content.clear();
            if (isnotmatch)
            {
                //需要恢复2个
                temp_post_data->field_item.push_back('-');
                temp_post_data->field_item.push_back('-');
                //重新开始当作没有发生过
                pos_m = temp_post_data->field_offset;
            }
            else
            {
                temp_post_data->field_offset = pos_m;
                if (temp_post_data->field_item.size() > 1)
                {
                    temp_post_data->field_item.resize(temp_post_data->field_item.size() - 2);
                }

                reset_uploadfile();
                post_multipart_itemcontent(true);
                temp_post_data->field_item.clear();
                if ((pos_m + 1) < temp_post_data->content.size())
                {
                    if (temp_post_data->content[pos_m] == '-')
                    {
                        if (temp_post_data->content[pos_m + 1] == '-')
                        {
                            peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data->field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data->content[pos_m] == '\r')
                    {
                        if (temp_post_data->content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data->field_offset = pos_m + 1;
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

            if ((pos_m + temp_post_data->boundary.size()) > temp_post_data->content.size())
            {
                temp_post_data->field_offset = temp_post_data->content.size();
                return;
            }
            unsigned int i = temp_post_data->pre_content.size() - 2;
            for (; i < temp_post_data->boundary.size(); i++)
            {
                if (temp_post_data->content[pos_m] != temp_post_data->boundary[i])
                {
                    pos_m = temp_post_data->field_offset;
                    break;
                }
                pos_m++;
            }
            if (i == temp_post_data->boundary.size())
            {
                isnotmatch = false;
            }

            //说明是假匹配
            if (isnotmatch)
            {
                //恢复所有temp_post_data->pre_content
                temp_post_data->field_item.append(temp_post_data->pre_content);
                //重新开始当作没有发生过
                pos_m = temp_post_data->field_offset;
                temp_post_data->pre_content.clear();
            }
            else
            {
                temp_post_data->pre_content.clear();
                temp_post_data->field_offset = pos_m;

                if (temp_post_data->field_item.size() > 1)
                {
                    temp_post_data->field_item.resize(temp_post_data->field_item.size() - 2);
                }

                reset_uploadfile();
                post_multipart_itemcontent(true);
                temp_post_data->field_item.clear();
                if ((pos_m + 1) < temp_post_data->content.size())
                {
                    if (temp_post_data->content[pos_m] == '-')
                    {
                        if (temp_post_data->content[pos_m + 1] == '-')
                        {
                            peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data->field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data->content[pos_m] == '\r')
                    {
                        if (temp_post_data->content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data->field_offset = pos_m + 1;
                        }
                    }
                }
                return;
            }
        }
    }

    temp_post_data->pre_content.clear();
    for (; pos_m < temp_post_data->content.size(); pos_m++)
    {
        if (temp_post_data->content[pos_m] == '-')
        {
            unsigned int j = pos_m;
            j++;
            if (j < temp_post_data->content.size())
            {
                if (temp_post_data->content[j] == '-')
                {
                    j++;
                    if (j < temp_post_data->content.size())
                    {
                        //主要是检查是否匹配或最后
                        unsigned int i = 0;
                        for (; i < temp_post_data->boundary.size();)
                        {
                            if (temp_post_data->content[j] != temp_post_data->boundary[i])
                            {
                                break;
                            }
                            j++;
                            i++;
                            if (j >= temp_post_data->content.size())
                            {
                                break;
                            }
                        }
                        if (i == temp_post_data->boundary.size())
                        {

                            temp_post_data->field_item.append(&temp_post_data->content[temp_post_data->field_offset], pos_m - temp_post_data->field_offset);

                            if (temp_post_data->field_item.size() > 1)
                            {
                                temp_post_data->field_item.resize(temp_post_data->field_item.size() - 2);
                            }

                            reset_uploadfile();
                            post_multipart_itemcontent(true);
                            temp_post_data->field_offset = j;
                            temp_post_data->field_item.clear();

                            if (j + 1 < temp_post_data->content.size())
                            {
                                if (temp_post_data->content[j] == '-' && temp_post_data->content[j + 1] == '-')
                                {
                                    temp_post_data->field_offset = j + 1;
                                    peer->isfinish               = true;
                                }
                                else if (temp_post_data->content[j] == '\r' && temp_post_data->content[j + 1] == '\n')
                                {
                                    temp_post_data->field_offset = j + 1;
                                }
                            }

                            return;
                        }
                        if (j >= temp_post_data->content.size())
                        {
                            //例外已经到头了
                            //让进入下一轮
                            temp_post_data->pre_content.append(&temp_post_data->content[pos_m], temp_post_data->content.size() - pos_m);
                            temp_post_data->field_item.append(&temp_post_data->content[temp_post_data->field_offset], pos_m - temp_post_data->field_offset);
                            temp_post_data->field_offset = temp_post_data->content.size();
                            return;
                        }
                    }
                    else
                    {
                        temp_post_data->pre_content.push_back('-');
                        temp_post_data->pre_content.push_back('-');
                        temp_post_data->field_item.append(&temp_post_data->content[temp_post_data->field_offset], pos_m - temp_post_data->field_offset);
                        temp_post_data->field_offset = temp_post_data->content.size();
                        return;
                    }
                }
            }
            else
            {
                temp_post_data->pre_content.push_back('-');
                temp_post_data->field_item.append(&temp_post_data->content[temp_post_data->field_offset], pos_m - temp_post_data->field_offset);
                temp_post_data->field_offset = temp_post_data->content.size();
                return;
            }
        }
    }
    temp_post_data->field_item.append(&temp_post_data->content[temp_post_data->field_offset], pos_m - temp_post_data->field_offset);
    temp_post_data->field_offset = pos_m;
}

void httpparse::read_http_post_block(const unsigned char *buffer, unsigned int buffersize)
{
    if (peer->compress == 10)
    {
        if (peer->content_length > CONST_PHP_BODY_POST_SIZE)
        {
            error = 40005;
            return;
        }
        if (peer->output.size() > peer->content_length)
        {
            error = 40006;
            return;
        }

        peer->output.append((char *)&buffer[readoffset], (buffersize - readoffset));
        readoffset = buffersize;
        if (peer->output.size() >= peer->content_length)
        {
            peer->isfinish = true;
        }
    }
    else
    {
        http_content_length += (buffersize - readoffset);

        if (posttype == 0)
        {
            //fix weixin browser
            if (readoffset < buffersize)
            {
                char first = buffer[readoffset];
                if (first == '-')
                {
                    posttype = 2;// multipart
                }
                else if (first == '{' || first == '[')
                {
                    posttype = 3;// json
                }
                else if (first == '<')
                {
                    posttype = 4;// xml
                }
                else
                {
                    posttype = 5;// fallback: raw
                }
            }
        }

        switch (posttype)
        {
        case 1:
            // x-www-form-urlencoded
            peer->rawcontent.append((char *)&buffer[readoffset], (buffersize - readoffset));
            readoffset = buffersize;
            if (peer->rawcontent.size() >= peer->content_length)
            {
                post_www_form_urlencoded(peer->rawcontent);
                peer->isfinish = true;
            }

            if (error > 0)
            {
                return;
            }
            break;
        case 2:
            // multipart/form-data
            if (temp_post_data == nullptr)
            {
                temp_post_data = std::make_unique<HTTP_POST_DATA_T>();
            }

            temp_post_data->content      = std::string_view((const char *)&buffer[readoffset], buffersize - readoffset);
            readoffset                   = buffersize;
            temp_post_data->field_offset = 0;
            for (; temp_post_data->field_offset < temp_post_data->content.size();)
            {
                post_multipart_formdata();
            }

            if (temp_post_data->isfile && temp_post_data->field_item.size() > 3145728)
            {
                //及时消化掉大文件中间内容，尽快保存到文件
                post_multipart_itemcontent_append();
                temp_post_data->field_item.clear();
            }
            else if (temp_post_data->field_item.size() > 3145728)
            {
                if (!temp_post_data->isfile)
                {
                    std::string_view check_filename = std::string_view(temp_post_data->field_item.data(), 80);
                    size_t pos                      = check_filename.find("filename");
                    if (pos != std::string::npos)
                    {
                        reset_uploadfile();
                        post_multipart_itemcontent(false);
                        temp_post_data->field_item.clear();
                    }
                }
            }

            if (peer->isfinish)
            {
                reset_uploadfile();
            }

            if (error > 0)
            {
                return;
            }
            break;
        case 3:
            // json
            peer->rawcontent.append((char *)&buffer[readoffset], (buffersize - readoffset));
            readoffset = buffersize;
            if (peer->rawcontent.size() >= peer->content_length)
            {
                peer->json.from_json(peer->rawcontent);
                peer->isfinish = true;
            }

            if (error > 0)
            {
                return;
            }

            if (peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
            {
                error = 40081;
                return;
            }
            break;
        case 4:
            // xml
            peer->rawcontent.append((char *)&buffer[readoffset], (buffersize - readoffset));
            readoffset = buffersize;
            if (error > 0)
            {
                return;
            }
            if (peer->rawcontent.size() >= peer->content_length)
            {
                peer->isfinish = true;
            }
            if (peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
            {
                error = 40082;
                return;
            }
            break;
        case 5:
            // octet-stream
            read_rawfile_formdata(std::string_view((const char *)&buffer[readoffset], buffersize - readoffset));
            readoffset = buffersize;
            if (http_content_length >= peer->content_length)
            {
                peer->isfinish = true;
            }
            break;
        }

        if (http_content_length > CONST_HTTP_BODY_POST_SIZE)
        {
            error = 40089;
            return;
        }
    }
}

void httpparse::process(const unsigned char *buffer, unsigned int buffersize)
{
    readoffset = 0;
    for (; readoffset < buffersize;)
    {
        if (isfinish_header)
        {
            read_http_post_block(buffer, buffersize);
        }
        else
        {
            //full http header block
            read_http_header_block(buffer, buffersize);
            if (http_content_length > CONST_HTTP_HEADER_BODY_SIZE)
            {
                error = 40088;
                return;
            }
            if (isfinish_header)
            {
                if (method == HEAD_METHOD::POST || method == HEAD_METHOD::QUERY)
                {
                    error = peer->check_upload_limit();
                }
                else
                {
                    peer->isfinish = true;
                }
                peer->isuse_fastcgi();
                http_content_length = 0;
            }
        }

        if (error > 0)
        {
            break;
        }
    }
}

bool httpparse::getfinish()
{
    return peer->isfinish;
}

void httpparse::clear()
{
    isfinish_header = false;
    isfinish_url    = false;

    posttype          = 0;
    http_action_setup = 0;
    headerfinish      = 0;
    port              = 0;

    http_content_length = 0;
    header_line.clear();
    pre_http_header.clear();

    readoffset = 0;
    uprawfile.reset(nullptr);
    error  = 0;
    method = HEAD_METHOD::UNKNOW;

    if (websocket != nullptr)
    {
        websocket->deflate           = false;
        websocket->permessagedeflate = false;
        websocket->perframedeflate   = false;
        websocket->deflateframe      = false;
        websocket->isopen            = false;
        websocket->version           = 0x00;
        websocket->key.clear();
        websocket->ext.clear();
    }

    if (temp_post_data != nullptr)
    {
        temp_post_data->isfile     = false;
        temp_post_data->exp_length = 0;//需要的长度
        temp_post_data->cur_length = 0;//当前长度
        temp_post_data->content    = std::string_view();
        temp_post_data->chartset.clear();
        temp_post_data->mimetype.clear();
        temp_post_data->boundary.clear();
        temp_post_data->pre_content.clear();
        temp_post_data->filename.clear();
        temp_post_data->field_name.clear();
        temp_post_data->field_item.clear();
        temp_post_data->temp_filename.clear();
        temp_post_data->fp.reset();
    }
}

}// namespace http
