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
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include "datetime.h"
// #include "md5.hpp"
// #include "mime.hpp"
#include "unicode.h"
// #include "cookie.hpp"
#include "urlcode.h"
#include "request.h"
// #include "WebSocket.h"
#include "http_parse.h"
#include "client_session.h"
#include "httppeer.h"
#include "server_localvar.h"

namespace http
{

httpparse::httpparse() : uprawfile(nullptr, &std::fclose) { error = 0; }
// void httpparse::setsession(std::shared_ptr<client_session> peer)
// {
//       peer_session = peer->get_ptr();
// }
void httpparse::setpeer(std::shared_ptr<httppeer> hpeer) { peer = hpeer->get_ptr(); }
unsigned int httpparse::header_valuetoint()
{
    unsigned int temp = 0;
    for (unsigned int qi = 0; qi < header_value.size(); qi++)
    {
        if (header_value[qi] < 0x3A && header_value[qi] > 0x2F)
        {
            temp = temp * 10 + (header_value[qi] - 0x30);
        }
    }
    return temp;
}
unsigned int httpparse::checkheadend(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;
    if (headendhitnum == 2)
    {
        if (buffer[i] == 0x0D && i < buffersize)
        {
            i++;
            headendhitnum++;
            if (buffer[i] == 0x0A && i < buffersize)
            {
                i++;
                headendhitnum++;
            }
        }
    }
    else
    {
        if (buffer[i] == 0x0A && i < buffersize)
        {
            i++;
            headendhitnum++;
            if (buffer[i] == 0x0D && i < buffersize)
            {
                i++;
                headendhitnum++;
                if (buffer[i] == 0x0A && i < buffersize)
                {
                    i++;
                    headendhitnum++;
                }
            }
        }
    }
    return headendhitnum;
}
void httpparse::procssparamter()
{
    std::string objname;
    bool isgroup = true;
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
                            http::OBJ_ARRAY objtemp;
                            objtemp.push(header_input);
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            peer->get[objname].push(std::move(objtemp1));
                        }
                        else
                        {
                            http::OBJ_VALUE objtemp;
                            objtemp[key2name] = header_input;
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            peer->get[objname].push(std::move(objtemp1));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            peer->get[objname][key1name].set_array();
                            peer->get[objname][key1name] = header_input;
                        }
                        else
                        {

                            peer->get[objname][key1name][key2name] = header_input;
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
                        peer->get[objname].set_array();
                        peer->get[objname].push(header_input);
                    }
                    else
                    {
                        peer->get[objname].push(key1name, header_input);
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
void httpparse::procssxformurlencoded()
{
    std::string objname;
    bool isgroup = true;
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
                            http::OBJ_ARRAY objtemp;
                            objtemp.push(header_input);
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            peer->post[objname].push(std::move(objtemp1));
                        }
                        else
                        {
                            http::OBJ_VALUE objtemp;
                            objtemp[key2name] = header_input;
                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            peer->post[objname].push(std::move(objtemp1));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            peer->post[objname][key1name].set_array();
                            peer->post[objname][key1name] = header_input;
                        }
                        else
                        {

                            peer->post[objname][key1name][key2name] = header_input;
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
                        peer->post[objname].set_array();
                        peer->post[objname].push(header_input);
                    }
                    else
                    {
                        peer->post[objname].push(key1name, header_input);
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
        peer->post[header_temp] = header_input;
    }
}
bool httpparse::checkmethod()
{
    unsigned int ioffset = 0, linesize;
    unsigned int j       = 0;
    linesize             = contentline.size();
    header_key.clear();
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
            return false;
        }
    }
    switch (header_key[0])
    {
    case 'c':
    case 'C':
        if (strcasecmp(header_key.c_str(), "CONNECT") == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
        break;
    case 'h':
    case 'H':
        if (strcasecmp(header_key.c_str(), "HEAD") == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
        break;
    case 'g':
    case 'G':
        if (strcasecmp(header_key.c_str(), "GET") == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
        break;
    case 'p':
    case 'P':

        if (strcasecmp(header_key.c_str(), "POST") == 0)
        {
            return true;
        }
        else if (strcasecmp(header_key.c_str(), "PUT") == 0)
        {
            return true;
        }
        else
        {
            return false;
        }

        break;
    case 'o':
    case 'O':
        if (strcasecmp(header_key.c_str(), "OPTIONS") == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
        break;
    case 't':
    case 'T':
        if (strcasecmp(header_key.c_str(), "TRACE") == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
        break;
    default: return false;
    }
    return true;
}
void httpparse::methodprocess()
{
    unsigned int ioffset = 0, linesize;
    unsigned int j       = 0;
    linesize             = contentline.size();
    header_key.clear();
    header_value.clear();

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
            error = 6;
            break;
        }
    }
    if (header_key.size() > 8)
    {
        error = 6;
        return;
        ;
    }
    method = HEAD_METHOD::UNKNOW;
    switch (header_key[0])
    {
    case 'c':
    case 'C':
        if (strcasecmp(header_key.c_str(), "CONNECT") == 0)
        {
            method = HEAD_METHOD::CONNECT;
        }
        else
        {
            error = 6;
        }
        break;
    case 'h':
    case 'H':
        if (strcasecmp(header_key.c_str(), "HEAD") == 0)
        {
            method = HEAD_METHOD::HEAD;
        }
        else
        {
            error = 6;
        }
        break;
    case 'g':
    case 'G':
        if (strcasecmp(header_key.c_str(), "GET") == 0)
        {
            method = HEAD_METHOD::GET;
        }
        else
        {
            error = 6;
        }
        break;
    case 'p':
    case 'P':

        if (strcasecmp(header_key.c_str(), "POST") == 0)
        {
            method = HEAD_METHOD::POST;
        }
        else if (strcasecmp(header_key.c_str(), "PUT") == 0)
        {
            method = HEAD_METHOD::PUT;
        }
        else
        {
            error = 6;
        }

        break;
    case 'o':
    case 'O':
        if (strcasecmp(header_key.c_str(), "OPTIONS") == 0)
        {
            method = HEAD_METHOD::OPTIONS;
        }
        else
        {
            error = 6;
        }
        break;
    case 't':
    case 'T':
        if (strcasecmp(header_key.c_str(), "TRACE") == 0)
        {
            method = HEAD_METHOD::TRACE;
        }
        else
        {
            error = 6;
        }
        break;
    default: method = HEAD_METHOD::UNKNOW; error = 6;
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
    buffer_key.clear();
    // pathinfo.clear();
    //  url.clear();
    peer->pathinfos.clear();
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
            if (buffer_key.size() > 0)
            {
                if (buffer_key.size() == 2 && buffer_key[0] == '.' && buffer_key[1] == '.')
                {
                    if (peer->pathinfos.size() > 0)
                    {
                        peer->pathinfos.pop_back();
                    }
                }
                else if (buffer_key.size() == 1 && buffer_key[0] == '.')
                {
                }
                else
                {
                    peer->pathinfos.emplace_back(http::url_decode(buffer_key.data(), buffer_key.length()));
                }
                buffer_key.clear();
            }
        }
        else
        {
            buffer_key.push_back(contentline[ioffset]);
        }
        header_key.push_back(contentline[ioffset]);
        header_value.push_back(contentline[ioffset]);
    }
    if (buffer_key.size() > 0)
    {
        if (buffer_key.size() == 2 && buffer_key[0] == '.' && buffer_key[1] == '.')
        {
            if (peer->pathinfos.size() > 0)
            {
                peer->pathinfos.pop_back();
            }
        }
        else if (buffer_key.size() == 1 && buffer_key[0] == '.')
        {
        }
        else
        {
            peer->pathinfos.emplace_back(http::url_decode(buffer_key.data(), buffer_key.length()));
        }
    }
    peer->header["urlpath"] = header_key;
    if (header_key.size() > 0)
    {
        if (header_key.size() == 1 && header_key[0] == '/')
        {
            peer->urlpath = "/";
        }
        else
        {
            peer->urlpath = http::url_decode(header_key.data(), header_key.length());
        }
    }
    else
    {
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
        header_value.push_back(0x3F);
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
            header_key.push_back(contentline[ioffset]);
            header_value.push_back(contentline[ioffset]);
        }
    }

    peer->header["url"]         = header_value;
    peer->header["querystring"] = header_key;
    peer->url                   = http::url_decode(header_value.data(), header_value.length());
    peer->querystring           = http::url_decode(header_key.data(), header_key.length());

    if (headerstep == 7)
    {
        // parameter
        header_value.clear();
        header_temp.clear();
        header_input.clear();
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
                header_temp = http::url_decode(header_value.data(), header_value.length());
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
                header_input = http::url_decode(header_value.data(), header_value.length());
                procssparamter();
                header_value.clear();
                partype = 2;
                continue;
            }
            header_value.push_back(header_key[j]);
        }
        if (partype == 1)
        {
            header_input = http::url_decode(header_value.data(), header_value.length());
            procssparamter();
        }
        else if (partype == 2)
        {
            header_temp = http::url_decode(header_value.data(), header_value.length());
            header_input.clear();
            procssparamter();
        }
        else if (header_value.size() > 0)
        {
            header_temp = http::url_decode(header_value.data(), header_value.length());
            header_input.clear();
            procssparamter();
        }
    }
    peer->httpv = 1;
    // for (; ioffset < linesize; ioffset++)
    // {
    //       if (contentline[ioffset] == 0x20)
    //       {
    //             continue;
    //       }
    //       break;
    // }
    // httpversion = 0;
    // version.clear();
    // for (; ioffset < linesize; ioffset++)
    // {
    //       if (contentline[ioffset] >= '0' && contentline[ioffset] <= '9')
    //       {
    //             if (httpversion < 25)
    //                   httpversion = httpversion * 10 + (contentline[ioffset] - '0');
    //       }
    //       //version.push_back(contentline[ioffset]);
    // }
    headerstep = 8;
}
void httpparse::callposttype()
{
    switch (buffer_value.size())
    {
    case 33:
        if (strcasecmp(buffer_value.c_str(), "application/x-www-form-urlencoded") == 0)
        {
            poststate.type     = "application/x-www-form-urlencoded";
            poststate.posttype = 1;
            peer->posttype     = 1;
            return;
        }
        break;
    case 24:
        if (strcasecmp(buffer_value.c_str(), "application/octet-stream") == 0)
        {
            poststate.type     = "application/octet-stream";
            poststate.posttype = 5;
            peer->posttype     = 5;
            return;
        }
        break;
    case 19:
        if (strcasecmp(buffer_value.c_str(), "multipart/form-data") == 0)
        {
            poststate.type     = "multipart/form-data";
            poststate.posttype = 2;
            peer->posttype     = 2;
            return;
        }
        break;
    case 16:
        if (strcasecmp(buffer_value.c_str(), "application/json") == 0)
        {
            poststate.type     = "application/json";
            poststate.posttype = 3;
            peer->posttype     = 3;
            return;
        }
        break;
    case 15:
        if (strcasecmp(buffer_value.c_str(), "application/xml") == 0)
        {
            poststate.type     = "application/xml";
            poststate.posttype = 4;
            peer->posttype     = 4;
            return;
        }
    case 6:
        if (strcasecmp(buffer_value.c_str(), "binary") == 0)
        {
            poststate.type     = "binary";
            poststate.posttype = 6;
            peer->posttype     = 5;
            return;
        }
        break;
    default:
        poststate.type     = "raw";
        poststate.posttype = 7;
        peer->posttype     = 5;
        return;
    }
}
void httpparse::getcontenttype()
{
    unsigned int i = 0, linesize;
    linesize       = header_value.size();
    buffer_value.clear();
    unsigned char statetemp = 0;
    poststate.posttype      = 0;
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
                poststate.chartset = buffer_value;
                peer->chartset     = buffer_value;
                statetemp          = 0;
            }
            else if (statetemp == 2)
            {
                poststate.boundary = buffer_value;
                statetemp          = 0;
            }
            /////////////////////
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3D)
        {
            if (strcasecmp(buffer_value.c_str(), "charset") == 0)
            {
                buffer_value.clear();
                statetemp = 1;
                continue;
            }
            else if (strcasecmp(buffer_value.c_str(), "boundary") == 0)
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
            poststate.chartset = buffer_value;
            peer->chartset     = buffer_value;
        }
        else if (statetemp == 2)
        {
            poststate.boundary = buffer_value;
        }
        else
        {
            callposttype();
        }
    }
}
void httpparse::getrange()
{
    unsigned int j = 0, linesize;
    linesize       = header_value.size();
    buffer_value.clear();
    for (; j < linesize; j++)
    {
        if (header_value[j] == 0x3D)
        {
            j++;
            break;
        }
        buffer_value.push_back(header_value[j]);
    }

    if (strcasecmp(buffer_value.c_str(), "bytes") == 0)
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
        }
        // state.rangeend = tm;
        peer->state.rangeend = tm;
    }
}
void httpparse::readheaderline(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;
    if (headendhitnum > 0)
    {
        headendhitnum = checkheadend(buffer, buffersize);
    }
    if (headendhitnum > 3)
    {
        headerfinish = 1;
        readoffset   = i;
        buffer_value.clear();
        header_key.clear();
        header_value.clear();
        header_temp.clear();
        header_input.clear();
        buffer_key.clear();
        headendhitnum = 0;
        return;
    }
    headendhitnum = 0;

    int lineend = 0;
    for (; i < buffersize; i++)
    {
        if (buffer[i] == 0x0D)
        {
            lineend = 1;
            break;
        }
        contentline.push_back(buffer[i]);
    }
    if (lineend == 1)
    {

        if (contentline.size() > 0)
        {

            // headerrawcontent.emplace_back(contentline);
            if (headerstep == 0 && checkmethod())
            {
                methodprocess();
            }
            else
            {

                header_key.clear();
                header_value.clear();
                bool isfirst;
                isfirst = true;
                for (unsigned int j = 0; j < contentline.size(); j++)
                {
                    if (isfirst && contentline[j] == ':')
                    {
                        header_key = header_value;
                        j++;
                        for (; j < contentline.size(); j++)
                        {
                            if (contentline[j] != 0x20)
                            {
                                j -= 1;
                                break;
                            }
                        }
                        header_value.clear();
                        isfirst = false;
                        continue;
                    }
                    if (isfirst && contentline[j] == 0x20)
                    {
                        continue;
                    }
                    header_value.push_back(contentline[j]);
                }
                if (header_key.size() > 0)
                {
                    // header[header_key] = header_value;
                    peer->header[header_key] = header_value;
                    switch (header_key.size())
                    {
                    case 24:
                        if (strcasecmp(header_key.c_str(), "Sec-WebSocket-Extensions") == 0)
                        {

                            getwebsocketextensions();
                        }
                        break;
                    case 21:
                        if (strcasecmp(header_key.c_str(), "Sec-WebSocket-Version") == 0)
                        {

                            peer->websocket.version = (unsigned char)header_valuetoint();
                        }
                        break;
                    case 19:
                        if (strcasecmp(header_key.c_str(), "If-Unmodified-Since") == 0)
                        {
                            getifunmodifiedsince();
                        }
                        break;
                    case 17:

                        switch (header_key[0])
                        {
                        case 's':
                        case 'S':
                            if (strcasecmp(header_key.c_str(), "Sec-WebSocket-Key") == 0)
                            {

                                peer->websocket.key = header_value;
                            }
                            break;
                        case 'i':
                        case 'I':
                            if (strcasecmp(header_key.c_str(), "If-Modified-Since") == 0)
                            {

                                getifmodifiedsince();
                            }
                            break;
                        default:;
                        }
                        break;
                    case 15:
                        if (header_key[7] == 'E')
                        {
                            if (strcasecmp(header_key.c_str(), "Accept-Encoding") == 0)
                            {
                                getacceptencoding();
                                break;
                            }
                        }
                        else if (header_key[7] == 'L')
                        {
                            if (strcasecmp(header_key.c_str(), "Accept-Language") == 0)
                            {
                                getacceptlanguage();
                                break;
                            }
                        }

                        if (strcasecmp(header_key.c_str(), "Accept-Encoding") == 0)
                        {
                            getacceptencoding();
                        }
                        if (strcasecmp(header_key.c_str(), "Accept-Language") == 0)
                        {
                            getacceptlanguage();
                        }
                        // Accept-Language

                        break;
                    case 14:
                        if (strcasecmp(header_key.c_str(), "Content-Length") == 0)
                        {
                            poststate.content_length = header_valuetoint();
                            // peer_session->stream[1].content_length=poststate.content_length;
                            peer->content_length = poststate.content_length;
                        }
                        break;
                    case 13:
                        if (strcasecmp(header_key.c_str(), "If-None-Match") == 0)
                        {
                            getifnonematch();
                        }
                        break;
                    case 12:
                        if (strcasecmp(header_key.c_str(), "Content-Type") == 0)
                        {

                            getcontenttype();
                        }
                        break;
                    case 10:
                        switch (header_key[0])
                        {
                        case 'c':
                        case 'C':
                            if (strcasecmp(header_key.c_str(), "Connection") == 0)
                            {
                                getconnection();
                            }
                            break;
                        case 'u':
                        case 'U':
                            if (strcasecmp(header_key.c_str(), "User-Agent") == 0)
                            {
                                // useragent = header_value;
                                peer->header["user-agent"] = header_value;
                            }
                            break;
                        }
                        break;
                    case 8:
                        if (strcasecmp(header_key.c_str(), "If-Match") == 0)
                        {

                            getifnonematch();
                        }
                        break;
                    case 7:
                        if (strcasecmp(header_key.c_str(), "Upgrade") == 0)
                        {

                            getupgrade();
                        }
                        break;
                    case 6:

                        switch (header_key[0])
                        {
                        case 'c':
                        case 'C':
                            if (strcasecmp(header_key.c_str(), "Cookie") == 0)
                            {

                                getcookie();
                            }
                            break;
                        case 'a':
                        case 'A':
                            if (strcasecmp(header_key.c_str(), "Accept") == 0)
                            {

                                getaccept();
                            }
                            break;
                        }
                        break;
                    case 5:
                        if (strcasecmp(header_key.c_str(), "Range") == 0)
                        {
                            getrange();
                        }
                        break;
                    case 4:
                        if (strcasecmp(header_key.c_str(), "Host") == 0)
                        {
                            getheaderhost();
                        }
                        break;
                    }
                }
            }
        }
        contentline.clear();
        headendhitnum++;
        i++;
        if (buffer[i] == 0x0A && i < buffersize)
        {
            i++;
            headendhitnum++;
            if (buffer[i] == 0x0D && i < buffersize)
            {
                i++;
                headendhitnum++;
                if (buffer[i] == 0x0A && i < buffersize)
                {
                    i++;
                    headendhitnum++;
                }
            }
        }

        if (headendhitnum > 3)
        {
            headerfinish = 1;
            readoffset   = i;
            buffer_value.clear();
            header_key.clear();
            header_value.clear();
            header_temp.clear();
            header_input.clear();
            buffer_key.clear();
            headendhitnum = 0;
            return;
        }
    }
    readoffset = i;
}
void httpparse::getaccept()
{
    unsigned int i = 0, linesize;
    linesize       = header_value.size();

    buffer_value.clear();

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
        default:;
        }
    }
}
void httpparse::getacceptlanguage()
{
    unsigned int i = 0;
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] == 0x2C)
        {
            break;
        }
        peer->state.language[i] = header_value[i];
        if (i > 6)
        {
            break;
        }
    }
    for (; i < 8; i++)
    {
        peer->state.language[i] = 0x00;
    }
}
void httpparse::getifunmodifiedsince() { peer->state.ifmodifiedsince = strgmttotime(header_value); }
void httpparse::getifmodifiedsince() { peer->state.ifmodifiedsince = strgmttotime(header_value); }
void httpparse::getifnonematch()
{
    unsigned int i = 0;
    peer->etag.clear();
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
void httpparse::getwebsocketextensions()
{
    switch (header_value.length())
    {
    case 22:
        if (strcasecmp(header_value.c_str(), "x-webkit-deflate-frame") == 0)
        {
            peer->websocket.deflateframe = true;
            peer->websocket.deflate      = true;
        }
        break;
    case 18:

        if (strcasecmp(header_value.c_str(), "permessage-deflate") == 0)
        {
            peer->websocket.permessagedeflate = true;
            peer->websocket.deflate           = true;
        }
        break;
    case 16:
        if (strcasecmp(header_value.c_str(), "perframe-deflate") == 0)
        {
            peer->websocket.perframedeflate = true;
            peer->websocket.deflate         = true;
        }
        break;
    default:;
    }
    if (peer->websocket.deflate)
    {
        return;
    }
    if (header_value.find("permessage-deflate") != std::string::npos)
    {
        peer->websocket.permessagedeflate = true;
        peer->websocket.deflate           = true;
        return;
    }
    if (header_value.find("perframe-deflate") != std::string::npos)
    {
        peer->websocket.perframedeflate = true;
        peer->websocket.deflate         = true;
        return;
    }
    if (header_value.find("x-webkit-deflate-frame") != std::string::npos)
    {
        peer->websocket.deflateframe = true;
        peer->websocket.deflate      = true;
        return;
    }
}
void httpparse::getupgrade()
{

    if (header_value.size() > 9)
    {
        if (header_value.back() == 0x20)
        {
            header_value.pop_back();
        }
    }

    if (strcasecmp(header_value.c_str(), "websocket") == 0)
    {
        // state.websocket = true;
        peer->state.websocket = true;
    }
    else if (strcasecmp(header_value.c_str(), "h2c") == 0)
    {
        // state.h2c = true;
        peer->state.h2c = true;
    }
}
void httpparse::getconnection()
{
    if (strcasecmp(header_value.c_str(), "keep-alive") == 0)
    {
        peer->state.keepalive = true;
    }
    if (strcasecmp(header_value.c_str(), "Upgrade") == 0)
    {
        peer->state.upgradeconnection = true;
    }
    if (header_value[0] == 'c' || header_value[0] == 'C')
    {
        if (header_value[1] == 'l' || header_value[1] == 'L')
        {
            peer->state.keepalive = false;
        }
    }
}
void httpparse::getacceptencoding()
{
    unsigned int i = 0, linesize;
    linesize       = header_value.size();
    buffer_key.clear();
    buffer_value.clear();

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
void httpparse::getcookie()
{
    unsigned int i = 0, linesize;
    linesize       = header_value.size();
    buffer_key.clear();
    buffer_value.clear();
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
        peer->cookie[buffer_key] = buffer_value;
    }
    else
    {
        if (buffer_key.size() > 0)
        {
            // cookie[buffer_key] = "";
            peer->cookie[buffer_key] = "";
        }
        buffer_key.clear();
    }
}
void httpparse::getheaderhost()
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
        peer->host.push_back(header_value[ioffset]);
    }
    std::transform(peer->host.begin(), peer->host.end(), peer->host.begin(), ::tolower);
    // peer_session->stream[1].host=host;
    // peer->host = host;
    if (ishasport == 1)
    {
        for (; ioffset < linesize; ioffset++)
        {
            if (header_value[ioffset] < 0x3A && header_value[ioffset] > 0x2F)
            {
                port = port * 10 + (header_value[ioffset] - 0x30);
            }
        }
    }
    // state.port = port;
    peer->state.port = port;
}
void httpparse::readboundaryline(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i       = readoffset;
    unsigned int isbound = 0;
    for (; i < buffersize; i++)
    {
        if ((buffer[i] != 0x0D) && (buffer[i] != 0x0A))
        {

            break;
        }
    }
    for (; i < buffersize; i++)
    {
        if (buffer[i] == 0x0D)
        {
            isbound = 1;
            break;
        }
        buffer_key.push_back(buffer[i]);
    }
    if (isbound == 1)
    {
        std::string bi;

        bi = buffer_key.substr(2);
        if (bi == poststate.boundary)
        {

            postfieldtype = 2;
        }
        i++;
        if (buffer[i] == 0x0A && i < buffersize)
        {
            i++;
        }
    }
    readoffset = i;
}
void httpparse::readformfilename(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i       = readoffset;
    unsigned int isbound = 0;
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
    else
    {
        for (; i < buffersize; i++)
        {
            if ((buffer[i] != 0x0D) && (buffer[i] != 0x0A))
            {

                break;
            }
        }
    }

    if (headendhitnum > 3)
    {
        isbound = 1;
    }
    else
    {
        i = readoffset;
    }
    headendhitnum = 0;

    if (isbound != 1)
    {

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

            buffer_key.push_back(buffer[i]);
        }
    }
    readoffset = i;
    if (isbound == 1)
    {
        std::string fieldname;

        header_temp.clear();
        unsigned int jj = 0;
        for (; jj < buffer_key.size(); jj++)
        {
            if (buffer_key[jj] == 0x0D || buffer_key[jj] == 0x0A)
            {
                header_temp.clear();
                continue;
            }
            if (buffer_key[jj] == ';')
            {
                header_temp.clear();
                continue;
            }
            if (buffer_key[jj] == 0x20)
            {
                header_temp.clear();
                continue;
            }
            if (buffer_key[jj] == ':')
            {
                if (strcasecmp(header_temp.c_str(), "Content-Type") == 0)
                {
                    unsigned int nmpp = jj + 1;
                    for (; nmpp < buffer_key.size(); nmpp++)
                    {
                        if (buffer_key[nmpp] != 0x20)
                        {

                            break;
                        }
                    }
                    upfile.type.clear();
                    for (; nmpp < buffer_key.size(); nmpp++)
                    {
                        if (buffer_key[nmpp] == 0x20 || buffer_key[nmpp] == 0x0D)
                        {
                            break;
                        }
                        upfile.type.push_back(buffer_key[nmpp]);
                    }
                    jj = nmpp;
                }
                header_temp.clear();
                continue;
            }
            if (buffer_key[jj] == 0x3D)
            {
                if (header_temp == "name")
                {
                    unsigned int mm = jj;
                    mm += 1;
                    if (buffer_key[mm] == '"')
                    {
                        mm += 1;
                    }
                    header_key.clear();
                    for (; mm < buffer_key.size(); mm++)
                    {
                        if (buffer_key[mm] == '"')
                        {
                            break;
                        }
                        header_key.push_back(buffer_key[mm]);
                    }
                    if (header_key.size() > 0)
                    {
                        if (postfieldtype < 7)
                        {
                            postfieldtype = 7;
                        }
                    }
                    else
                    {
                        if (postfieldtype < 7)
                        {
                            postfieldtype = 7;
                        }
                        header_key = std::to_string(timeid()) + std::to_string(rand_range(100, 999));
                        header_key = std::to_string(std::hash<std::string>{}(header_key));
                    }

                    if (buffer_key[mm] == '"')
                    {
                        mm += 1;
                    }
                    jj = mm;
                }
                else if (header_temp == "filename")
                {
                    unsigned int mm = jj;
                    mm += 1;
                    if (buffer_key[mm] == '"')
                    {
                        mm += 1;
                    }
                    header_value.clear();
                    for (; mm < buffer_key.size(); mm++)
                    {
                        if (buffer_key[mm] == '"')
                        {
                            break;
                        }
                        header_value.push_back(buffer_key[mm]);
                    }

                    if (postfieldtype < 8)
                    {
                        postfieldtype = 9;
                    }

                    if (buffer_key[mm] == '"')
                    {
                        mm += 1;
                    }
                    jj = mm;
                }
                header_temp.clear();
                continue;
            }
            header_temp.push_back(buffer_key[jj]);
        }
        header_temp.clear();
        if (postfieldtype == 9)
        {
            server_loaclvar &localvar = get_server_global_var();
            upfile.filename           = header_value;
            upfile.name               = header_key;

            header_temp =
                upfile.filename + std::to_string(timeid()) + header_key + std::to_string(rand_range(100, 999));

            upfile.tempfile = localvar.temp_path;
            upfile.tempfile.append(std::to_string(std::hash<std::string>{}(header_temp)));

            // uprawfile = fopen(upfile.tempfile.c_str(), "wb");
            uprawfile.reset(fopen(upfile.tempfile.c_str(), "wb"));
            if (!uprawfile)
            {
                upfile.tempfile.append("_t");
                // uprawfile = fopen(upfile.tempfile.c_str(), "wb");
                uprawfile.reset(fopen(upfile.tempfile.c_str(), "wb"));
                if (!uprawfile)
                {
                    error         = 3;
                    postfieldtype = 12;
                }
            }
            upfile.size = 0;
            header_temp.clear();
        }
        changetype = 0;

        header_input.clear();
        buffer_key.clear();
    }
}

void httpparse::readformfielditem(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i       = readoffset;
    unsigned int isbound = 0;
    unsigned int j       = 0;
    if (changetype > 0 || buffer_key.size() > 0)
    {
        j = i;
        // algin pre match
        if (buffer_key.size() < 5)
        {
            j          = j + (4 - buffer_key.size());
            changetype = 0;
        }

        for (; changetype < poststate.boundary.size(); changetype++)
        {
            if (buffer[j] != poststate.boundary[changetype])
            {
                isbound = 2;
                break;
            }
            j++;
        }
        if (isbound == 0)
        {
            changetype    = 0;
            postfieldtype = 2;
            i             = j;
            header_temp   = header_key;
            procssxformurlencoded();
            header_key.clear();
            header_value.clear();
            header_temp.clear();
            header_input.clear();
            buffer_key.clear();

            headerstep    = 0;
            headendhitnum = 0;

            if (buffer[i] == 0x2D && buffer[i + 1] == 0x2D)
            {
                headerfinish = 2;
                i += 2;
                readoffset = i;

                return;
            }
            if (buffer[i] == 0x0D && i < buffersize)
            {
                i++;
            }
            if (buffer[i] == 0x0A && i < buffersize)
            {
                i++;
            }

            readoffset = i;
            return;
        }

        header_input.append((char *)&buffer_key[0], buffer_key.size());
        buffer_key.clear();
    }
    changetype = 0;
    j          = 0;
    for (; i < buffersize; i++)
    {
        if (buffer[i] == 0x0D)
        {

            buffer_key.clear();
            buffer_key.push_back(0x0D);
            j = i + 1;
            if (j < buffersize && buffer[j] == 0x0A)
            {
                j++;

                buffer_key.push_back(0x0A);
                if (j < buffersize && buffer[j] == 0x2D)
                {
                    j++;

                    buffer_key.push_back(0x2D);
                    if (j < buffersize && buffer[j] == 0x2D)
                    {
                        j++;

                        buffer_key.push_back(0x2D);
                        for (unsigned int nnn = 0; nnn < poststate.boundary.size(); nnn++)
                        {
                            if (buffer[j] != poststate.boundary[nnn])
                            {
                                isbound = 2;
                                break;
                            }
                            j += 1;
                            buffer_key.push_back(buffer[j]);
                            if (j >= buffersize)
                            {
                                changetype = nnn + 1;
                                isbound    = 1;
                                break;
                            }
                        }
                        if (isbound == 0)
                        {
                            changetype    = 0;
                            postfieldtype = 2;

                            header_input.append((char *)&buffer[readoffset], (i - readoffset));
                            i           = j;
                            header_temp = header_key;
                            procssxformurlencoded();
                            header_key.clear();
                            header_value.clear();
                            header_temp.clear();
                            header_input.clear();
                            buffer_key.clear();

                            headerstep    = 0;
                            headendhitnum = 0;
                            if (buffer[i] == 0x2D && buffer[i + 1] == 0x2D)
                            {
                                headerfinish = 2;
                                i += 2;

                                if (buffer[i] == 0x0D && i < buffersize)
                                {
                                    i++;
                                }
                                if (buffer[i] == 0x0A)
                                {
                                    i++;
                                }
                                readoffset = i;
                                return;
                            }
                            if (buffer[i] == 0x0D && i < buffersize)
                            {
                                i++;
                            }
                            if (buffer[i] == 0x0A)
                            {
                                i++;
                            }

                            readoffset = i;
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
            headerstep = 0;
            changetype = 0;
            buffer_key.clear();
        }
    }

    header_input.append((char *)&buffer[readoffset], (i - readoffset));
    readoffset = buffersize;
}
void httpparse::procssformfile()
{
    std::string objname;
    bool isgroup = true;
    for (unsigned int j = 0; j < upfile.name.length(); j++)
    {
        if (upfile.name[j] == '[')
        {
            std::string key1name;

            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < upfile.name.length(); n++)
            {
                if (upfile.name[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (upfile.name[n] == '[')
                {

                    break;
                }
                if (upfile.name[n] != 0x22)
                {
                    key1name.push_back(upfile.name[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < upfile.name.length())
                {
                    if (upfile.name[m] == '[')
                    {
                        m += 1;
                        for (; m < upfile.name.length(); m++)
                        {
                            if (upfile.name[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (upfile.name[m] == '[')
                            {

                                break;
                            }
                            if (upfile.name[m] != 0x22)
                            {
                                key2name.push_back(upfile.name[m]);
                            }
                        }

                        if (ishaskey2 && m == upfile.name.length())
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
                            objtemp["name"]     = upfile.name;
                            objtemp["filename"] = upfile.filename;
                            objtemp["tempfile"] = upfile.tempfile;
                            objtemp["type"]     = upfile.type;
                            objtemp["size"]     = upfile.size;
                            objtemp["error"]    = upfile.error;

                            http::OBJ_ARRAY objtemp1;
                            objtemp1.push(std::move(objtemp));
                            peer->files[objname].push(std::move(objtemp1));
                        }
                        else
                        {

                            http::OBJ_VALUE objtemp;
                            objtemp[key2name].set_array();
                            objtemp[key2name]["name"]     = upfile.name;
                            objtemp[key2name]["filename"] = upfile.filename;
                            objtemp[key2name]["tempfile"] = upfile.tempfile;
                            objtemp[key2name]["type"]     = upfile.type;
                            objtemp[key2name]["size"]     = upfile.size;
                            objtemp[key2name]["error"]    = upfile.error;
                            peer->files[objname].set_array();
                            peer->files[objname].push(std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {

                            peer->files[objname][key1name].set_array();
                            http::OBJ_VALUE objtemp;
                            objtemp.set_array();
                            objtemp["name"]     = upfile.name;
                            objtemp["filename"] = upfile.filename;
                            objtemp["tempfile"] = upfile.tempfile;
                            objtemp["type"]     = upfile.type;
                            objtemp["size"]     = upfile.size;
                            objtemp["error"]    = upfile.error;

                            peer->files[objname][key1name] = objtemp;
                        }
                        else
                        {

                            peer->files[objname][key1name][key2name].set_array();
                            peer->files[objname][key1name][key2name]["name"]     = upfile.name;
                            peer->files[objname][key1name][key2name]["filename"] = upfile.filename;
                            peer->files[objname][key1name][key2name]["tempfile"] = upfile.tempfile;
                            peer->files[objname][key1name][key2name]["type"]     = upfile.type;
                            peer->files[objname][key1name][key2name]["size"]     = upfile.size;
                            peer->files[objname][key1name][key2name]["error"]    = upfile.error;
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == upfile.name.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        peer->files[objname].set_array();
                        http::OBJ_VALUE objtemp;
                        objtemp.set_array();
                        objtemp["name"]      = upfile.name;
                        objtemp["filename"]  = upfile.filename;
                        objtemp["tempfile"]  = upfile.tempfile;
                        objtemp["type"]      = upfile.type;
                        objtemp["size"]      = upfile.size;
                        objtemp["error"]     = upfile.error;
                        peer->files[objname] = objtemp;
                    }
                    else
                    {
                        // files[objname].push(key1name,"");
                        peer->files[objname][key1name].set_array();
                        peer->files[objname][key1name]["name"]     = upfile.name;
                        peer->files[objname][key1name]["filename"] = upfile.filename;
                        peer->files[objname][key1name]["tempfile"] = upfile.tempfile;
                        peer->files[objname][key1name]["type"]     = upfile.type;
                        peer->files[objname][key1name]["size"]     = upfile.size;
                        peer->files[objname][key1name]["error"]    = upfile.error;
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
                objname.push_back(upfile.name[j]);
            }
        }
        else
        {
            objname.push_back(upfile.name[j]);
        }
    }
    if (isgroup)
    {
        // files[upfile.name]=buffer_value;
        peer->files[upfile.name].set_array();
        peer->files[upfile.name]["name"]     = upfile.name;
        peer->files[upfile.name]["filename"] = upfile.filename;
        peer->files[upfile.name]["tempfile"] = upfile.tempfile;
        peer->files[upfile.name]["type"]     = upfile.type;
        peer->files[upfile.name]["size"]     = upfile.size;
        peer->files[upfile.name]["error"]    = upfile.error;
    }
    upfile.name.clear();
    upfile.filename.clear();
    upfile.tempfile.clear();
    upfile.type.clear();
    upfile.size  = 0;
    upfile.error = 0;
}
void httpparse::readformfilecotent(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i       = readoffset;
    unsigned int isbound = 0;
    unsigned int j       = 0;
    if (changetype > 0 || header_input.size() > 0)
    {
        j = i;
        // algin pre match
        if (header_input.size() < 5)
        {
            j          = j + (4 - header_input.size());
            changetype = 0;
        }
        for (; changetype < poststate.boundary.size(); changetype++)
        {
            if (buffer[j] != poststate.boundary[changetype])
            {
                isbound = 2;
                break;
            }
            j++;
        }
        if (isbound == 0)
        {
            changetype    = 0;
            postfieldtype = 2;
            header_input.clear();
            i = j;
            if (uprawfile)
            {
                // fclose(uprawfile);
                // uprawfile = NULL;
                uprawfile.reset(nullptr);
            }
            procssformfile();
            header_key.clear();
            header_value.clear();
            header_temp.clear();
            header_input.clear();
            buffer_key.clear();

            headerstep = 0;

            if (buffer[i] == 0x2D && buffer[i + 1] == 0x2D)
            {
                headerfinish = 2;
                i += 2;
                readoffset    = i;
                headendhitnum = 0;
                return;
            }
            if (buffer[i] == 0x0D && i < buffersize)
            {
                i++;
                headendhitnum++;
            }
            if (buffer[i] == 0x0A && i < buffersize)
            {
                i++;
                headendhitnum++;
            }

            readoffset = i;
            return;
        }

        upfile.size += header_input.size();
        fwrite(&header_input[0], header_input.size(), 1, uprawfile.get());
        header_input.clear();
    }
    changetype = 0;
    for (; i < buffersize; i++)
    {
        if (buffer[i] == 0x0D)
        {
            headerstep = 1;
            header_input.clear();
            header_input.push_back(0x0D);
            unsigned int j = i + 1;
            if (j < buffersize && buffer[j] == 0x0A)
            {
                j++;
                headerstep += 1;
                header_input.push_back(0x0A);
                if (j < buffersize && buffer[j] == 0x2D)
                {
                    j++;
                    headerstep += 1;
                    header_input.push_back(0x2D);
                    if (j < buffersize && buffer[j] == 0x2D)
                    {
                        j++;
                        headerstep += 1;
                        header_input.push_back(0x2D);
                        for (unsigned int nnn = 0; nnn < poststate.boundary.size(); nnn++)
                        {
                            if (buffer[j] != poststate.boundary[nnn])
                            {
                                isbound = 2;
                                break;
                            }
                            j += 1;
                            header_input.push_back(buffer[j]);
                            if (j >= buffersize)
                            {
                                changetype = nnn + 1;
                                isbound    = 1;
                                break;
                            }
                        }
                        if (isbound == 0)
                        {

                            changetype    = 0;
                            postfieldtype = 2;
                            header_input.clear();

                            fwrite(&buffer[readoffset], (i - readoffset), 1, uprawfile.get());
                            upfile.size += (i - readoffset);
                            i = j;
                            if (uprawfile)
                            {
                                // fclose(uprawfile);
                                // uprawfile = NULL;
                                uprawfile.reset(nullptr);
                            }
                            procssformfile();
                            header_key.clear();
                            header_value.clear();
                            header_temp.clear();
                            header_input.clear();
                            buffer_key.clear();

                            headerstep = 0;
                            // i++;
                            if (buffer[i] == 0x2D && buffer[i + 1] == 0x2D)
                            {
                                headerfinish = 2;
                                i += 2;
                                readoffset    = i;
                                headendhitnum = 0;
                                return;
                            }
                            if (buffer[i] == 0x0D && i < buffersize)
                            {
                                i++;
                                headendhitnum++;
                            }
                            if (buffer[i] == 0x0A)
                            {
                                i++;
                                headendhitnum++;
                            }

                            readoffset = i;
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
            header_input.clear();
            headerstep = 0;
            changetype = 0;
        }
    }
    fwrite(&buffer[readoffset], (i - readoffset), 1, uprawfile.get());
    upfile.size += (i - readoffset);
    readoffset = buffersize;
}
void httpparse::readmultipartformdata(const unsigned char *buffer, unsigned int buffersize)
{

    for (; readoffset < buffersize;)
    {
        switch (postfieldtype)
        {
        case 0:
            // 开头
            readboundaryline(buffer, buffersize);
            headendhitnum = 0;
            header_key.clear();
            header_value.clear();
            header_temp.clear();
            header_input.clear();
            buffer_key.clear();
            break;
        case 2: readformfilename(buffer, buffersize); break;
        case 7: readformfielditem(buffer, buffersize); break;
        case 9: readformfilecotent(buffer, buffersize); break;
        default: readoffset++;
        }
    }
}
void httpparse::readformjson(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;
    for (; i < buffersize; i++)
    {
        buffer_value.push_back(buffer[i]);
    }
    readoffset = i;
    if ((buffer_value.size() + 2) >= poststate.content_length)
    {
        headerfinish = 2;
        peer->json.from_json(buffer_value);
        peer->rawcontent = buffer_value;
    }
}
void httpparse::readformurlencoded(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;
    for (; i < buffersize; i++)
    {
        buffer_value.push_back(buffer[i]);
    }
    readoffset = i;
    if ((buffer_value.size() + 2) >= poststate.content_length)
    {
        headerfinish = 2;

        header_key.clear();
        header_value.clear();
        header_temp.clear();
        header_input.clear();
        unsigned int qsize    = buffer_value.size();
        unsigned char partype = 0;
        unsigned int j        = 0;
        for (j = 0; j < qsize; j++)
        {
            if (buffer_value[j] == 0x3D)
            {
                header_temp = http::url_decode(header_value.data(), header_value.length());
                header_value.clear();
                partype = 1;
                continue;
            }
            else if (buffer_value[j] == 0x26)
            {
                header_input = http::url_decode(header_value.data(), header_value.length());
                procssxformurlencoded();
                header_value.clear();
                partype = 2;
                continue;
            }
            header_value.push_back(buffer_value[j]);
        }
        if (partype == 1)
        {
            header_input = http::url_decode(header_value.data(), header_value.length());
            procssxformurlencoded();
        }
        else if (partype == 2)
        {
            header_temp = http::url_decode(header_value.data(), header_value.length());
            header_input.clear();
            procssxformurlencoded();
        }
        else if (header_value.size() > 0)
        {
            header_temp = http::url_decode(header_value.data(), header_value.length());
            header_input.clear();
            procssxformurlencoded();
        }
    }
}
void httpparse::readformxml(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;
    for (; i < buffersize; i++)
    {
        buffer_value.push_back(buffer[i]);
    }
    readoffset = i;
    if ((buffer_value.size() + 2) >= poststate.content_length)
    {
        headerfinish     = 2;
        peer->rawcontent = buffer_value;
    }
}
void httpparse::readformraw(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;

    if (!uprawfile)
    {
        server_loaclvar &localvar = get_server_global_var();

        srand((int)time(0));
        header_temp     = "application/octet-stream" + std::to_string(timeid()) + std::to_string(rand());
        upfile.tempfile = localvar.temp_path;
        upfile.tempfile.append(std::to_string(std::hash<std::string>{}(header_temp)));

        // uprawfile = fopen(upfile.tempfile.c_str(), "wb");
        uprawfile.reset(fopen(upfile.tempfile.c_str(), "wb"));
        if (!uprawfile)
        {
            upfile.tempfile.append("_t");
            // uprawfile = fopen(upfile.tempfile.c_str(), "wb");
            uprawfile.reset(fopen(upfile.tempfile.c_str(), "wb"));
            if (!uprawfile)
            {
                error = 3;
            }
        }
        upfile.size = 0;
    }
    if (i < buffersize && uprawfile)
    {
        unsigned int tempnum = buffersize - i;
        fwrite(&buffer[i], tempnum, 1, uprawfile.get());
        upfile.size = tempnum;
    }

    if ((upfile.size + 2) >= poststate.content_length)
    {
        if (uprawfile)
        {
            // fclose(uprawfile);
            uprawfile.reset(nullptr);
        }
        // uprawfile = NULL;
        peer->files["tempraw"].set_array();
        peer->files["tempraw"]["name"]     = "tempraw";
        peer->files["tempraw"]["filename"] = "";
        peer->files["tempraw"]["tempfile"] = upfile.tempfile;
        peer->files["tempraw"]["type"]     = "application/octet-stream";
        peer->files["tempraw"]["size"]     = upfile.size;
        peer->files["tempraw"]["error"]    = upfile.error;
        headerfinish                       = 2;
    }
}

void httpparse::process(const unsigned char *buffer, unsigned int buffersize)
{
    readoffset = 0;
    if (error > 0)
    {
        return;
    }
    if (headerfinish == 0)
    {
        for (; readoffset < buffersize;)
        {

            readheaderline(buffer, buffersize);
            if (error > 0)
            {
                break;
            }
            if (headerfinish == 1)
            {
                break;
            }
        }
    }
    if (method == HEAD_METHOD::POST && headerfinish == 1 && error == 0)
    {
        switch (poststate.posttype)
        {
        case 1:
            // x-www-form-urlencoded
            readformurlencoded(buffer, buffersize);
            break;
        case 2:
            // multipart/form-data-
            readmultipartformdata(buffer, buffersize);
            break;
        case 3:
            // json
            readformjson(buffer, buffersize);
            break;
        case 4:
            // xml
            readformxml(buffer, buffersize);
            break;
        case 5:
            // octet-stream
            readformraw(buffer, buffersize);
            break;
        }
        for (; readoffset < buffersize;)
        {
            readoffset++;
        }
    }
}
bool httpparse::getfinish()
{
    if (method == HEAD_METHOD::POST)
    {
        if (headerfinish == 2)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    if (headerfinish == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void httpparse::clear()
{
    peer->state.gzip              = false;
    peer->state.deflate           = false;
    peer->state.br                = false;
    peer->state.avif              = false;
    peer->state.webp              = false;
    peer->state.keepalive         = false;
    peer->state.websocket         = false;
    peer->state.upgradeconnection = false;
    peer->state.rangebytes        = false;
    peer->state.language[0]       = {0};
    peer->state.version           = 0;
    peer->state.port              = 0;
    peer->state.ifmodifiedsince   = 0;
    peer->state.rangebegin        = 0;
    peer->state.rangeend          = 0;
    peer->keepalive               = false;
    // headerrawcontent.clear();
    peer->send_header.clear();
    peer->send_cookie.clear();
    peer->header.clear();
    peer->pathinfos.clear();
    peer->querystring.clear();
    peer->urlpath.clear();
    readoffset    = 0;
    headendhitnum = 0;
    postfieldtype = 0;
    headerstep    = 0;

    peer->host.clear();
    peer->etag.clear();
    peer->output.clear();
    peer->val.clear();
    peer->post.clear();
    peer->get.clear();
    peer->files.clear();
    peer->json.clear();

    uprawfile.reset(nullptr);
    error = 0;
    peer->cookie.clear();
    method                            = HEAD_METHOD::UNKNOW;
    headerfinish                      = 0;
    peer->httpv                       = 1;
    peer->isso                        = false;
    peer->compress                    = 0;
    peer->websocket.deflate           = false;
    peer->websocket.permessagedeflate = false;
    peer->websocket.perframedeflate   = false;
    peer->websocket.deflateframe      = false;
    peer->websocket.isopen            = false;
    peer->websocket.version           = 0x00;
    peer->websocket.key.clear();
    peer->websocket.ext.clear();
}

}// namespace http
