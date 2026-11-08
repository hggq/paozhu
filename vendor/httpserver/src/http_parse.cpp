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
    if (header_temp.empty())
    {
        // 空 key 存进 get 以后谁也取不到（?=b、a=1& 都会走到这里），直接丢弃
        return;
    }
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

            // [ 后没找到对应的 ] — 畸形 key: a[ / a[b[ / a[bc 等
            if (!ishaskey)
            {
                error = 40007;
                return;
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
                            std::size_t iii = peer->get[objname].size();
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
                            std::size_t iii = peer->get[objname].size();
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

                            std::size_t iii = peer->get[objname][key1name].size();
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
                        std::size_t iii = peer->get[objname].size();
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
                else if (header_temp[n] != '[')
                {
                    // 解析完一级 key 后既不是二级 [ 也不是行尾 — 畸形 key: a[]x] / a[b]c 等
                    error = 40007;
                    return;
                }
                // else header_temp[n] == '[' — 进入第二级 key 解析 (保持原逻辑)
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

// multipart boundary 长度上限（RFC 2046 规定 ≤70，取 72），顺带剥掉可选的引号。
// 终止符要逐位置做前缀比较，开销随 boundary 长度线性增长，所以必须限长、不能原样收下。
static bool boundary_normalize(std::string &out, const std::string &raw)
{
    std::string temp = raw;
    if (temp.size() >= 2 && temp.front() == '"' && temp.back() == '"')
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
    if (header_key.empty())
    {
        // 请求行以空格开头：这一行根本不是 method，属于格式错误而不是未实现的方法。
        error = 40066;
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
            method_unknown = true;
        }
        break;
    case 'd':
    case 'D':
        if (str_casecmp(header_key, "delete"))
        {
            method = HEAD_METHOD::DELETE;
        }
        else
        {
            // 与 default 分支同一口径：D 开头但不是 DELETE 就是无法识别的方法
            method_unknown = true;
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
            method_unknown = true;
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
            method_unknown = true;
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
            method_unknown = true;
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
            method_unknown = true;
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
            method_unknown = true;
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
            method_unknown = true;
        }
        break;
    default:
        method_unknown = true;
    }
    if (method_unknown)
    {
        // 方法名认不出来：不落 peer->method，也不解析这一行的后续部分，
        // 由 server 回 501 并关闭连接（这一类方法的正文没人消费）。
        return;
    }
    peer->method = static_cast<unsigned char>(method);
    // 这里只落一个标记：请求方法是否为 OPTIONS，供 server 分派到 send_cors_domain()
    // 处理预检（预检响应头由该函数按站点白名单输出，解析期不预置 ACAO）
    peer->iscors = (method == HEAD_METHOD::OPTIONS);
    // 解析结果写入 state.keepalive，响应头(peer->keepalive)与 server 循环里"是否关闭连接"的依据从此取同一值。
    {
        // 版本 token 必须是请求行末尾、紧跟最后一个空格的 8 字节，且只接受
        // HTTP/1.1 / HTTP/1.0。整行 find("HTTP/1.") 会被 URL 内 "HTTP/1.1" 伪造，
        // 也会把 HTTP/1.9 静默当 1.0。
        std::string_view ver;
        size_t vsp = contentline.find_last_of(' ');
        if (vsp != std::string_view::npos && vsp + 1 < contentline.size())
        {
            ver = contentline.substr(vsp + 1);
        }
        if (ver == "HTTP/1.1")
        {
            peer->state.version   = 0x11;
            peer->state.keepalive = true;
        }
        else if (ver == "HTTP/1.0")
        {
            peer->state.version   = 0x10;
            peer->state.keepalive = false;
        }
        // 其它（无版本 token / HTTP/1.9 / 非法）：保持 headstate_t 默认 keepalive=false
    }
    peer->keepalive = peer->state.keepalive;
    headerstep      = 1;
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

    // 按原始 / 逐段切分，每段交给 http::url_segments_normalize()（func.cpp 统一实现）
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
                if (!url_segments_normalize(peer->pathinfos, header_temp))
                {
                    error = 40095;
                    return;
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
        if (!url_segments_normalize(peer->pathinfos, header_temp))
        {
            error = 40095;
            return;
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

    // 最后一段含 '.' 标记为文件请求；.php 结尾标 compress=10 交给 loop 处理 —
    // last segment has '.' means file-like URL; .php suffix marks fastcgi candidate
    if (!peer->pathinfos.empty())
    {
        auto const &last = peer->pathinfos.back();
        peer->isfile     = (last.find('.') != std::string::npos);
#ifdef ENABLE_FASTCGI
        // 超轻量：只比后缀 4 字符，不查路由表不查磁盘 —
        // ultra-light: only check suffix, no route lookup no disk stat

        if (last.size() > 4 && last[last.size() - 1] == 'p' && last[last.size() - 2] == 'h' &&
            last[last.size() - 3] == 'p' && last[last.size() - 4] == '.')
        {
            peer->compress = 10;
        }
#endif
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
                if (partype == 1)
                {
                    // 值里的 '=' 按普通字符保留：跳过它会让 ?a=b=c 切成 get["b"]="c"、丢掉后半截 token
                    header_value.push_back(header_key[j]);
                    continue;
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
                // 同上：partype 为 0/2 时这条 token 没见过 '='，它是裸 key、值为空。
                if (partype == 1)
                {
                    header_temp = http::url_decode(header_value.data(), header_value.length());
                }
                else
                {
                    url_keyname = http::url_decode(header_value.data(), header_value.length());
                    header_temp.clear();
                }

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
    // media type 只允许识别一次：第一个分号之前，或整段没有分号时的末尾。
    // 与 HTTP/2 的 getcontenttype 同口径：charset / boundary 之外的后续参数段
    // 必须跳过，否则会二次调用 callposttype 把已判定的 posttype 覆盖掉。
    bool media_type_done = false;

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
                // RFC 9110 §8.3.1：media type 之后可跟任意参数，框架只认 charset / boundary。
                // 未知参数段不能当新的 media type 再喂给 callposttype：它的 default 分支
                // 会把 posttype / peer->posttype 覆盖成 5（raw 落盘），multipart 表单就此失效。
                if (!media_type_done)
                {
                    callposttype(buffer_value);
                    media_type_done = true;
                }
            }
            else if (statetemp == 1)
            {
                peer->chartset = buffer_value;
                statetemp      = 0;
            }
            else if (statetemp == 2)
            {
                if (!boundary_normalize(temp_post_data->boundary, buffer_value))
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
    // Content-Type 扫描结束；boundary 参数声明了但值为空也视为非法
    if (statetemp == 2)
    {
        if (!boundary_normalize(temp_post_data->boundary, buffer_value))
        {
            error = 40099;
            return;
        }
    }
    else if (buffer_value.size() > 0)
    {
        if (statetemp == 1)
        {
            peer->chartset = buffer_value;
        }
        else if (!media_type_done)
        {
            // 整段没有分号（如 "application/json"）时才在这里识别 media type；
            // 已经识别过的尾部是未知参数段，直接忽略。
            callposttype(buffer_value);
        }
    }
}

void httpparse::getrange(std::string_view header_value)
{
    if (parse_range_header(header_value, peer->state) == range_parse_t::syntax_error)
    {
        error = 40091;
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

    if (header_key.size() > http::field_name_max_length)
    {
        error = 40004;
        return;
    }

    // field-name 必须是 RFC 7230 token。带空格/制表符的头名会让下面按长度分派的
    // switch 全部落空——CL、TE 这类决定请求边界的头被无声忽略，本端算出的边界
    // 就和前置代理不一致。表在 http_header.h，h2 用的是同一份。
    if (!http::is_valid_field_name(header_key))
    {
        error = 40003;
        return;
    }

    auto trim_ows = [](std::string_view value)
    {
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
            value.remove_prefix(1);
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t'))
            value.remove_suffix(1);
        return value;
    };

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
        case 't':
        case 'T':
            if (str_casecmp(header_key, "Transfer-Encoding"))
            {
                // 只接受单一 chunked；gzip、identity、多值编码一律拒绝。
                // 与 Content-Length 并存按 RFC 9112 §6.1 拒绝，避免 TE/CL 口径分叉。
                // Accept a single chunked coding only; gzip, identity and lists are rejected.
                // Reject when Content-Length is also present (RFC 9112 §6.1), no TE/CL divergence.
                if (has_content_length || request_chunked)
                {
                    error = 40233;
                    return;
                }
                if (!str_casecmp(trim_ows(header_value), "chunked"))
                {
                    error = 40098;
                    return;
                }
                // Transfer-Encoding 只在 HTTP/1.1 定义，1.0 请求一律拒绝
                // Transfer-Encoding is defined for HTTP/1.1 only: reject 1.0 chunked
                if (peer->state.version != 0x11)
                {
                    error = 40098;
                    return;
                }
                request_chunked = true;
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
            // 严格解析：跳过非数字字符（"5, 5"→55）、接受负号、把超限值静默改成 0，
            // 都会让声明的 body 长度和实际消费的字节对不上 → 报文错位。
            if (has_content_length)
            {
                error = 40096;
                return;
            }
            // chunked 请求的 body 边界由分块定义，再出现 Content-Length 属两套口径并存，拒绝。
            // A chunked request's body boundary comes from the chunking; a Content-Length here
            // would be a second, conflicting boundary, so reject.
            if (request_chunked)
            {
                error = 40233;
                return;
            }
            has_content_length         = true;
            unsigned long long temp_cl = 0;
            if (!str2uint64_strict(header_value, temp_cl))
            {
                error = 40096;
                return;
            }
            if (temp_cl > CONST_HTTP_BODY_POST_SIZE)
            {
                error = 40097;
                return;
            }
            peer->content_length = temp_cl;
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
        case 'o':
        case 'O':
            if (str_casecmp(header_key, "origin"))
            {
                // 跨域请求才会走到这里，Origin 值就在手边：Host 已解析完（绝大多数情况）
                // 就直接判定，请求期不再有任何 CORS 判断，也不用回 header 里查 origin
                peer->cors_origin_process(header_value);
            }
            break;
        case 'e':
        case 'E':
            if (str_casecmp(header_key, "Expect"))
            {
                // 只认 100-continue；是否真的发 100 由 server 按版本与 body 情况决定
                // Only 100-continue is recognized; whether to send 100 is decided by server
                // from the protocol version and body presence
                if (str_casecmp(trim_ows(header_value), "100-continue"))
                {
                    expect_continue = true;
                }
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
                // 只看 [6..9] 会被 application/xxxavif 等 10 字节串误命中, 必须确认前缀是 image/
                if (str_casecmp(buffer_value, "image/avif"))
                {
                    peer->state.avif = true;
                }
                else if (str_casecmp(buffer_value, "image/webp"))
                {
                    peer->state.webp = true;
                }
                break;
            case 16:
                if (str_casecmp(buffer_value, "application/json"))
                {
                    peer->state.accept_json = true;
                }
                break;
            case 15:
                if (str_casecmp(buffer_value, "application/xml"))
                {
                    peer->state.accept_xml = true;
                }
                break;
            case 9:
                if (str_casecmp(buffer_value, "text/json"))
                {
                    peer->state.accept_json = true;
                }
                break;
            case 8:
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
            // 只看 [6..9] 会被 application/xxxavif 等 10 字节串误命中, 必须确认前缀是 image/
            // 主循环内已用 str_casecmp 做全串比较, 这里的末尾分支必须一致。
            if (str_casecmp(buffer_value, "image/avif"))
            {
                peer->state.avif = true;
            }
            else if (str_casecmp(buffer_value, "image/webp"))
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
                peer->state.accept_json = true;
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
    // RFC 7230：Upgrade 是逗号分隔的 token 列表（"websocket"、"h2c" 可并存）。
    // 按逗号逐个 token 比较：整串比较只认得单一值，覆盖不了带 OWS 或列表的值。
    size_t start = 0;
    while (start <= header_value.size())
    {
        size_t comma           = header_value.find(',', start);
        std::string_view token = (comma == std::string_view::npos) ? header_value.substr(start) : header_value.substr(start, comma - start);
        while (!token.empty() && (token.front() == ' ' || token.front() == '\t'))
            token.remove_prefix(1);
        while (!token.empty() && (token.back() == ' ' || token.back() == '\t'))
            token.remove_suffix(1);

        if (str_casecmp(token, "websocket"))
        {
            peer->state.websocket = true;
        }
        else if (str_casecmp(token, "h2c"))
        {
            peer->state.h2c = true;
        }
        if (comma == std::string_view::npos)
            break;
        start = comma + 1;
    }
}

void httpparse::getconnection(std::string_view header_value)
{
    if (header_value.size() < 2)
    {
        error = 40072;
        return;
    }
    // RFC 7230 6.1：Connection 是逗号分隔 token 列表——"keep-alive, Upgrade" 是
    // 浏览器 WS 握手的标准写法，整串比较 + 前缀短路会丢掉 Upgrade token。
    size_t start = 0;
    while (start <= header_value.size())
    {
        size_t comma           = header_value.find(',', start);
        std::string_view token = (comma == std::string_view::npos) ? header_value.substr(start) : header_value.substr(start, comma - start);
        while (!token.empty() && (token.front() == ' ' || token.front() == '\t'))
            token.remove_prefix(1);
        while (!token.empty() && (token.back() == ' ' || token.back() == '\t'))
            token.remove_suffix(1);

        if (str_casecmp(token, "keep-alive"))
        {
            peer->state.keepalive = true;
        }
        else if (str_casecmp(token, "upgrade"))
        {
            peer->state.upgradeconnection = true;
        }
        else if (str_casecmp(token, "close"))
        {
            peer->state.keepalive = false;
        }
        // 其余 token（TE 扩展名等）忽略
        if (comma == std::string_view::npos)
            break;
        start = comma + 1;
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
            // cookie[buffer_key] = buffer_value;
            if (buffer_key.size() > 72)
            {
                error = 40073;
                return;
            }
            if (buffer_key.empty())
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
        if (buffer_key.empty())
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
            bool ipv6closed = false;
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
                    ipv6closed = true;
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
            if (!ipv6closed)
            {
                error = 40077;// IPv6 字面量未闭合 ']'
                return;
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
    // Origin 早于 Host 到达时（HTTP/1 不保证头顺序），cors_origin_process() 因 host 未定
    // 而挂起，这里在 host_index 确定后补判一次。OPTIONS 预检的 Allow-Origin 由
    // send_cors_domain() 按站点白名单重新判定并覆盖，不受此处影响。
    if (peer->cors_origin_pending)
    {
        peer->cors_origin_pending = false;
        auto iter                 = peer->header.find("origin");
        if (iter != peer->header.end())
        {
            peer->cors_origin_allow(iter->second);
        }
    }
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
            // 补记上一包里残留的 "\r" 与本包的 "\n"，否则 16KB 头部上限口径偏移
            http_content_length += 2;
            if (header_line.size() == 0)
            {
                if (!isfinish_url)
                {
                    // 请求行还没解析到，说明这是消息之前的空行。RFC 9112 允许忽略它，
                    // 但忽略不等于凭空生成一条 method/URL/host 全空的请求。CR+LF 已记账。
                    return;
                }
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
        if (buffer[pos_m] == '\n')
        {
            // 行终止只认 CRLF。能走到这里的 LF，前面必然不是同一行里的 CR——
            // CRLF 在上一轮就 return 了，跨包那个 LF 由上面的分支吃掉。裸 LF 留在
            // 行里会被原样带进 URL 和访问日志，等于让客户端往日志里加行。
            error = 40223;
            return;
        }
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
                    if (!isfinish_url)
                    {
                        // 同上：前导空行只跳过，不派发。这一行的 CRLF 还没记账，这里补上。
                        http_content_length += 2;
                        return;
                    }
                    isfinish_header = true;
                    return;
                }
                http_content_length += header_line.size() + 2;

                process_header_line(header_line);
                header_line.clear();
                return;
            }
            else if (j < buffersize)
            {
                // CR 后面本包里已经有字节、却不是 LF：不是合法行终止。
                // 不能像下面那样把 CR 当"待定"存起来——那会顺手吞掉本包余下的字节。
                error = 40222;
                return;
            }
            else
            {
                // CR 正好是本包最后一个字节，是不是 CRLF 要等下一包首字节才能判。
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
                            std::size_t iii = peer->post[objname].size();
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
                            std::size_t iii = peer->post[objname].size();
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

                            std::size_t iii = peer->post[objname][key1name].size();
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
                        std::size_t iii = peer->post[objname].size();
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
            if (partype == 1)
            {
                // 值里的 '=' 按普通字符保留：再切一次会把 a=b=c 存成 post["b"]="c"、丢掉前半个 key
                temp_value.push_back(raw_post_urlencode_content[j]);
                continue;
            }
            buffer_key = http::url_decode(temp_value.data(), temp_value.length());
            temp_value.clear();
            partype = 1;
            jj      = 0;
            continue;
        }
        else if (raw_post_urlencode_content[j] == 0x26)
        {
            // partype 决定 temp_value 的身份：1 时它是 '=' 之后的值，key 早在 buffer_key 里；
            // 0 或 2 时这条 token 没见过 '='，它是裸 key、值为空——与函数尾部同一口径。
            if (partype == 1)
            {
                field_value = http::url_decode(temp_value.data(), temp_value.length());
            }
            else
            {
                buffer_key    = http::url_decode(temp_value.data(), temp_value.length());
                field_value.clear();
            }
            if (buffer_key.size() > 72)
            {
                error = 40083;
                return;
            }
            if (!buffer_key.empty())
            {
                post_form_to_postfield(buffer_key, field_value);
            }
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
        if (!buffer_key.empty())
        {
            post_form_to_postfield(buffer_key, field_value);
        }
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
        if (!buffer_key.empty())
        {
            post_form_to_postfield(buffer_key, field_value);
        }
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
        if (!buffer_key.empty())
        {
            post_form_to_postfield(buffer_key, field_value);
        }
    }
}

void httpparse::read_rawfile_formdata(std::string_view raw_http_post_data)
{
    if (!uprawfile)
    {
        server_loaclvar &localvar = get_server_global_var();
        // 落盘名统一由 make_http_temp_raw_name() 生成：文件名带 pzraw_ 前缀，
        // 该前缀是 httpwatch 周期清理时识别「框架自己的临时文件」的唯一依据，
        // 不要在此自行拼接文件名，否则文件会长期留在 temp_path 里。
        std::string temp_filename = localvar.temp_path;
        temp_filename.append(http::make_http_temp_raw_name());

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
                            std::size_t iii = peer->files[objname].size();
                            key1name         = std::to_string(iii);
                            peer->files[objname][key1name].set_object();

                            iii      = peer->files[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = safe_filename(temp_post_data->filename);
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
                            std::size_t iii = peer->files[objname].size();
                            key1name         = std::to_string(iii);
                            peer->files[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = safe_filename(temp_post_data->filename);
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

                            std::size_t iii = peer->files[objname][key1name].size();
                            key2name     = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = safe_filename(temp_post_data->filename);
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
                            objtemp["filename"] = safe_filename(temp_post_data->filename);
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
                        std::size_t iii = peer->files[objname].size();
                        key1name     = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp.set_object();
                        objtemp["filename"] = safe_filename(temp_post_data->filename);
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
                        objtemp["filename"] = safe_filename(temp_post_data->filename);
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
        peer->files[temp_post_data->field_name]["filename"] = safe_filename(temp_post_data->filename);
        peer->files[temp_post_data->field_name]["name"]     = temp_post_data->field_name;
        peer->files[temp_post_data->field_name]["tempfile"] = temp_post_data->temp_filename;
        peer->files[temp_post_data->field_name]["type"]     = temp_post_data->mimetype;
        peer->files[temp_post_data->field_name]["size"]     = temp_post_data->cur_length;
        peer->files[temp_post_data->field_name]["error"]    = 0;
    }
}

//大文件正文中途落盘时，先把 field_item 末尾两个字节留在内存里。
//分隔符前面必定是 CRLF，这两个字节一旦写进文件，下一包的 "--" 就回看不到：
//回看全空会把它们当成正文（文件末尾多出两个字节），只看见 LF 那半个又会把
//真分隔符判成普通数据吞进文件里，后面的字段一起丢掉。
static std::string take_delim_crlf_tail(std::string &field_item)
{
    if (field_item.size() < 2)
    {
        return std::string();
    }
    std::string carry = field_item.substr(field_item.size() - 2, 2);
    field_item.resize(field_item.size() - 2);
    return carry;
}

// 串比较（忽略大小写），长度由 a 决定，b 是字面量。
static bool head_ci_equal(std::string_view a, const char *b)
{
    for (size_t i = 0; i < a.size(); i++)
    {
        if (b[i] == '\0')
        {
            return false;
        }
        char c = a[i];
        if (c >= 'A' && c <= 'Z')
        {
            c = static_cast<char>(c + 32);
        }
        if (c != b[i])
        {
            return false;
        }
    }
    return b[a.size()] == '\0';
}

// 判断 field_item 开头这一段是不是"声明了 filename 的 part 头"。
// 走字节顺序必须和 post_multipart_itemcontent 的头解析一致（参数名上限 72、
// name 值上限 512、名字与 '=' 之间不许有空格），否则判真之后正式解析会半路报错。
// 只看传进来的这一段，超出即判否；判否什么都不做，交给正常解析。
// 128：与 h2 (http2_parse.cpp) 同名分支同宽，改动需两边同步。
static bool head_is_file_part(std::string_view head)
{
    size_t i = 0;
    while (i < head.size() && (head[i] == '\r' || head[i] == '\n' || head[i] == '-'))
    {
        i++;
    }

    // Content-Disposition 允许不在第一行：逐行扫到头区结束（空行），
    // 找到后把 head 收窄到该行，后续参数解析的边界就只到行尾，不会跨头。
    bool found = false;
    while (i < head.size())
    {
        size_t line_begin = i;
        while (i < head.size() && head[i] != '\r' && head[i] != '\n')
        {
            i++;
        }
        size_t line_end = i;
        if (line_end == line_begin)
        {
            break;
        }

        size_t colon = line_begin;
        while (colon < line_end && head[colon] != ':')
        {
            colon++;
        }
        if (colon < line_end && (colon - line_begin) <= 72
            && head_ci_equal(head.substr(line_begin, colon - line_begin), "Content-Disposition"))
        {
            head  = head.substr(line_begin, line_end - line_begin);
            i     = colon - line_begin + 1;
            found = true;
            break;
        }
        if (i < head.size() && head[i] == '\r')
        {
            i++;
        }
        if (i < head.size() && head[i] == '\n')
        {
            i++;
        }
    }
    if (!found)
    {
        return false;
    }

    size_t begin = i;
    while (i < head.size() && head[i] != ';')
    {
        i++;
    }
    if (i >= head.size())
    {
        return false;
    }
    i++;
    while (i < head.size() && head[i] == ' ')
    {
        i++;
    }

    begin = i;
    while (i < head.size() && head[i] != '=')
    {
        i++;
    }
    if (i >= head.size() || (i - begin) > 72 || !head_ci_equal(head.substr(begin, i - begin), "name"))
    {
        return false;
    }
    i++;

    while (i < head.size() && (head[i] == ' ' || head[i] == '"' || head[i] == '='))
    {
        i++;
    }
    begin = i;
    while (i < head.size() && head[i] != '"')
    {
        i++;
    }
    if (i >= head.size() || (i - begin) > 512)
    {
        return false;
    }
    i++;

    while (i < head.size() && head[i] == ' ')
    {
        i++;
    }
    if (i >= head.size() || head[i] != ';')
    {
        return false;
    }
    i++;
    while (i < head.size() && head[i] == ' ')
    {
        i++;
    }

    begin = i;
    while (i < head.size() && head[i] != '=')
    {
        i++;
    }
    if (i >= head.size() || (i - begin) > 72)
    {
        return false;
    }
    return head_ci_equal(head.substr(begin, i - begin), "filename");
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
    // part 头区一次扫完：定位 Content-Disposition（允许不在第一行），只认 Content-Type
    // 收 mimetype，其余头行整行丢弃（RFC 7578 §4.8）。上限：单行 128 字节、
    // CD + 16 个额外头、头区总计 1 KB；超限报 40059。
    std::string keyname_field;
    unsigned int cd_value_pos = 0;// CD 值起点（':' 之后），供下面参数解析复用
    unsigned int cd_line_end   = 0;// CD 行尾（'\r' 的位置），参数解析不得越界
    bool         cd_found      = false;

    {
        unsigned int header_count = 0;
        unsigned int header_bytes = 0;
        for (;;)
        {
            keyname_field.clear();
            bool is_closed        = false;
            unsigned int line_end = pos_m;
            while (pos_m < temp_post_data->field_item.size())
            {
                if (temp_post_data->field_item[pos_m] == '\r')
                {
                    line_end = pos_m;
                    pos_m++;
                    if (pos_m < temp_post_data->field_item.size() && temp_post_data->field_item[pos_m] == '\n')
                    {
                        pos_m++;
                    }
                    is_closed = true;
                    break;
                }
                keyname_field.push_back(temp_post_data->field_item[pos_m]);
                pos_m++;
                if (keyname_field.size() > 128)
                {
                    error = 40059;
                    return;
                }
            }
            // 空行 = part 头结束；越界未闭合则按当前位置进正文（与旧行为一致）
            if (keyname_field.empty() || !is_closed)
            {
                break;
            }
            header_bytes += static_cast<unsigned int>(keyname_field.size()) + 2;
            if (header_bytes > 1024 || ++header_count > 17)
            {
                error = 40059;
                return;
            }
            unsigned int name_end = 0;
            for (; name_end < keyname_field.size(); name_end++)
            {
                if (keyname_field[name_end] == ':')
                {
                    break;
                }
            }
            if (name_end < keyname_field.size())
            {
                std::string_view name(keyname_field.data(), name_end);
                if (str_casecmp(name, "Content-Disposition"))
                {
                    if (name_end > 72)
                    {
                        error = 40045;
                        return;
                    }
                    if (!cd_found)
                    {
                        cd_line_end  = line_end;
                        cd_value_pos = line_end - static_cast<unsigned int>(keyname_field.size()) + name_end + 1;
                        cd_found     = true;
                    }
                }
                else if (str_casecmp(name, "Content-Type"))
                {
                    unsigned int i = name_end + 1;
                    for (; i < keyname_field.size(); i++)
                    {
                        if (keyname_field[i] != ' ')
                        {
                            break;
                        }
                    }
                    for (; i < keyname_field.size(); i++)
                    {
                        temp_post_data->mimetype.push_back(keyname_field[i]);
                    }
                }
            }
        }
    }

    unsigned int body_pos = pos_m;
    if (!cd_found)
    {
        error = 40026;
        return;
    }
    pos_m = cd_value_pos;

    for (; pos_m < cd_line_end; pos_m++)
    {
        if (temp_post_data->field_item[pos_m] == ';')
        {
            pos_m++;
            break;
        }
    }
    if (pos_m >= cd_line_end)
    {
        error = 40204;
        return;
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
            // 72：与 h2 (http2_parse.cpp) 同名分支同值，改动需两边同步
            if (keyname_field.size() > 72)
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
    pos_m = body_pos;

    std::string_view keyvalue_field(&temp_post_data->field_item[pos_m], temp_post_data->field_item.size() - pos_m);
    if (temp_post_data->isfile)
    {
        server_loaclvar &localvar     = get_server_global_var();
        temp_post_data->temp_filename = localvar.temp_path;
        if (temp_post_data->temp_filename.size() > 0 && temp_post_data->temp_filename.back() != '/')
        {
            temp_post_data->temp_filename.push_back('/');
        }

        temp_post_data->temp_filename = temp_post_data->temp_filename + http::make_http_temp_upload_name(peer->content_length);
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
        else
        {
            // 落不了盘就没有这个文件；报错退出，别让下面把 size=0 和不存在的路径写进 files。
            error = 40226;
            return;
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
    else if (isfilefull)
    {
        // 只有整段读完（isfilefull）才当作文本域写入；中途探测时这一段还不完整，
        // 写进去会截断，剩下的字节又没人接，所以这里宁可不写。
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

    //看 "--" 前面第 dist 个字节是不是 want：1 是，0 不是，-1 看不见。
    //这两个字节可能还在本包的段里，也可能已经进了 field_item（段起点之前），
    //再往前（跨包、或大文件已刷盘）就取不到，只能按 -1 处理。
    auto prev_is = [&](unsigned int dist, char want) -> int
    {
        unsigned int seg = pos_m - temp_post_data->field_offset;
        if (dist <= seg)
        {
            return (temp_post_data->content[pos_m - dist] == want) ? 1 : 0;
        }
        unsigned int back = dist - seg;
        if (temp_post_data->field_item.size() < back)
        {
            return -1;
        }
        return (temp_post_data->field_item[temp_post_data->field_item.size() - back] == want) ? 1 : 0;
    };
    //跨包那几条支路里 "--" 存在 pre_content 中，前面的字节只剩 field_item 尾部
    auto tail_is = [&](unsigned int back, char want) -> int
    {
        if (temp_post_data->field_item.size() < back)
        {
            return -1;
        }
        return (temp_post_data->field_item[temp_post_data->field_item.size() - back] == want) ? 1 : 0;
    };

    //处理上次剩余的
    if (temp_post_data->pre_content.size() > 0)
    {
        if (temp_post_data->pre_content.size() == 1)
        {
            bool isnotmatch = true;
            bool isrunout   = false;
            if (temp_post_data->content[pos_m] == '-')
            {
                //说明已经命中两个，接着直接匹配boundary
                pos_m++;
                unsigned int i = 0;
                for (; i < temp_post_data->boundary.size(); i++)
                {
                    if (pos_m >= temp_post_data->content.size())
                    {
                        isrunout = true;
                        break;
                    }
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
            //横线前面有 LF 但没有 CR（tail_cr!=1，含 0 和 -1）或明确不是换行（tail_lf=0）时置为假匹配
            if (!isnotmatch && (tail_is(1, '\n') == 0 || (tail_is(1, '\n') == 1 && tail_is(2, '\r') != 1)))
            {
                isnotmatch = true;
            }
            if (isrunout)
            {
                //本包在边界文本中间用完了：横线连同已经对上的部分先留在 pre_content，
                //下一包接着比，不能就地丢掉这些字节。
                temp_post_data->pre_content.append(
                    temp_post_data->content.substr(temp_post_data->field_offset, pos_m - temp_post_data->field_offset));
                temp_post_data->field_offset = temp_post_data->content.size();
                return;
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
                if (tail_is(1, '\n') == 1 && tail_is(2, '\r') == 1 && temp_post_data->field_item.size() > 1)
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
            bool isrunout   = false;
            unsigned int i  = 0;
            for (; i < temp_post_data->boundary.size(); i++)
            {
                if (pos_m >= temp_post_data->content.size())
                {
                    isrunout = true;
                    break;
                }
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
            //横线前面有 LF 但没有 CR（tail_cr!=1，含 0 和 -1）或明确不是换行（tail_lf=0）时置为假匹配
            if (!isnotmatch && (tail_is(1, '\n') == 0 || (tail_is(1, '\n') == 1 && tail_is(2, '\r') != 1)))
            {
                isnotmatch = true;
            }
            if (isrunout)
            {
                //本包在边界文本中间用完了：已经对上的部分先留在 pre_content，下一包接着比
                temp_post_data->pre_content.append(
                    temp_post_data->content.substr(temp_post_data->field_offset, pos_m - temp_post_data->field_offset));
                temp_post_data->field_offset = temp_post_data->content.size();
                return;
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
                if (tail_is(1, '\n') == 1 && tail_is(2, '\r') == 1 && temp_post_data->field_item.size() > 1)
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
            bool isrunout   = false;

            //pre_content 里 "--" 后面那几个字节已经和 boundary 对上了，从剩下的接着比
            unsigned int i = temp_post_data->pre_content.size() - 2;
            for (; i < temp_post_data->boundary.size(); i++)
            {
                if (pos_m >= temp_post_data->content.size())
                {
                    isrunout = true;
                    break;
                }
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
            //横线前面有 LF 但没有 CR（tail_cr!=1，含 0 和 -1）或明确不是换行（tail_lf=0）时置为假匹配
            if (!isnotmatch && (tail_is(1, '\n') == 0 || (tail_is(1, '\n') == 1 && tail_is(2, '\r') != 1)))
            {
                isnotmatch = true;
            }
            if (isrunout)
            {
                //本包在边界文本中间用完了：已经对上的部分先留在 pre_content，下一包接着比
                temp_post_data->pre_content.append(
                    temp_post_data->content.substr(temp_post_data->field_offset, pos_m - temp_post_data->field_offset));
                temp_post_data->field_offset = temp_post_data->content.size();
                return;
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

                if (tail_is(1, '\n') == 1 && tail_is(2, '\r') == 1 && temp_post_data->field_item.size() > 1)
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
                        // 分隔符的 '--' 前面必须是 CRLF 或整体不可见（body 开头、跨包、大文件刷盘）。
                        // 明确有 LF 但没有 CR（dash_lf=1, dash_cr!=1，含 0 和 -1）拒绝；
                        // 明确都不是（dash_lf=0）也拒绝。
                        // 整体不可见（dash_lf=-1）时放行——第一个 boundary 就是 body 开头。
                        int dash_lf = prev_is(1, '\n');
                        int dash_cr = prev_is(2, '\r');
                        if (dash_lf == 0 || (dash_lf == 1 && dash_cr != 1))
                        {
                            continue;// 普通数据，继续扫描，后续 append 会把它并回 field_item
                        }

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

                            // 确认前面是 CRLF 才剥掉这两个字节
                            if (dash_lf == 1 && dash_cr == 1 && temp_post_data->field_item.size() > 1)
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
                                    // 跳过结束标记的两个 '-'：j 指向第一个 '-', j+1 指向第二个 '-'
                                    temp_post_data->field_offset = j + 2;
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
    if (request_chunked)
    {
        // chunked 的 body 边界来自分块本身，不走 Content-Length 计数
        // A chunked body's boundary comes from the chunking, not from a Content-Length count
        read_http_chunked_block(buffer, buffersize);
        return;
    }
    // 只取 body 还缺的字节，不把读包剩余部分整块 append 进 body。
    unsigned int avail = (readoffset < buffersize) ? (buffersize - readoffset) : 0;
    if (peer->content_length > 0)
    {
#ifdef ENABLE_FASTCGI
        unsigned long long already = (peer->compress == 10) ? peer->output.size() : http_content_length;
#else
        unsigned long long already = http_content_length;
#endif
        if (already >= peer->content_length)
        {
            avail = 0;
        }
        else if ((peer->content_length - already) < avail)
        {
            avail = static_cast<unsigned int>(peer->content_length - already);
        }
    }
    if (peer->content_length == 0 || avail == 0)
    {
        // 没有声明 body（或已消费完）：不再消费字节，直接判定请求完成。
        // 否则同一读包内的残留字节会被当成 body 判为未知类型而落盘，既丢数据又产生孤儿文件。
        peer->isfinish = true;
        return;
    }

#ifdef ENABLE_FASTCGI
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

        peer->output.append((char *)&buffer[readoffset], avail);
        readoffset += avail;
        if (peer->output.size() >= peer->content_length)
        {
            peer->isfinish = true;
        }
    }
    else
#endif
    {
        http_content_length += avail;
        bool is_final = http_content_length >= peer->content_length;
        append_post_body(&buffer[readoffset], avail, is_final);
        readoffset += avail;
        if (error > 0)
        {
            return;
        }
        if (http_content_length > CONST_HTTP_BODY_POST_SIZE)
        {
            error = 40089;
            return;
        }
    }
}

// 把一段已确认属于本次请求的正文喂给 posttype 解析。
// is_final 表示这是本次请求的最后一段：urlencoded/json/xml/raw 只在这时才整体解析，
// 否则会把半截正文当完整正文交给业务。
// Feeds a confirmed slice of this request's body to the posttype parsers.
// is_final means the last slice: urlencoded/json/xml/raw only parse as a whole on it,
// otherwise a half body would be handed to business code as if complete.
void httpparse::append_post_body(const unsigned char *data, unsigned int len, bool is_final)
{
    if (posttype == 0 && len > 0)
    {
        char first = (char)data[0];
        if (first == '-')
        {
            // 走到这里说明请求没带 Content-Type，头里也就没有 boundary 可用。
            // 没有 boundary 还按 multipart 解，正文里每个 "--" 都会被当成分隔符；
            // 退回原始正文分支，body 原样落盘给业务自己读。
            if (temp_post_data != nullptr && !temp_post_data->boundary.empty())
            {
                posttype = 2;// multipart
            }
            else
            {
                posttype = 5;// fallback: raw
            }
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

    switch (posttype)
    {
    case 1:
        // x-www-form-urlencoded
        peer->rawcontent.append((char *)data, len);
        if (is_final)
        {
            post_www_form_urlencoded(peer->rawcontent);
            peer->isfinish = true;
        }
        break;
    case 2:
        // multipart/form-data
        if (temp_post_data == nullptr)
        {
            temp_post_data = std::make_unique<HTTP_POST_DATA_T>();
        }
        // 无 boundary：匹配循环会把任意 '--' 当成分隔符，必须显式报错。
        if (temp_post_data->boundary.empty())
        {
            error = 40099;
            return;
        }

        if (request_chunked && chunked_payload_closed)
        {
            if (is_final)
            {
                reset_uploadfile();
            }
            break;
        }

        temp_post_data->content       = std::string_view((const char *)data, len);
        temp_post_data->field_offset  = 0;
        for (; temp_post_data->field_offset < temp_post_data->content.size();)
        {
            post_multipart_formdata();
            if (error > 0)
            {
                break;
            }
            if (request_chunked && peer->isfinish)
            {
                peer->isfinish               = false;
                chunked_payload_closed       = true;
                temp_post_data->field_offset = temp_post_data->content.size();
                reset_uploadfile();
                break;
            }
        }

        // 3145728（3MB）：与 h2 (http2_parse.cpp) 同名分支同值，改动需两边同步。
        if (temp_post_data->isfile && temp_post_data->field_item.size() > 3145728)
        {
            //及时消化掉大文件中间内容，尽快保存到文件
            if (temp_post_data->fp)
            {
                std::string carry = take_delim_crlf_tail(temp_post_data->field_item);
                post_multipart_itemcontent_append();
                temp_post_data->field_item = std::move(carry);
            }
        }
        else if (temp_post_data->field_item.size() > 3145728)
        {
            if (!temp_post_data->isfile)
            {
                // 头还没被认出来就攒过了阈值：先看头部是否声明 filename，
                // 判真才转成文件流式落盘；判否什么都不动，等正常解析。
                std::string_view check_head = std::string_view(temp_post_data->field_item.data(),
                                                               std::min<size_t>(temp_post_data->field_item.size(), 128));
                if (head_is_file_part(check_head))
                {
                    reset_uploadfile();
                    std::string carry = take_delim_crlf_tail(temp_post_data->field_item);
                    post_multipart_itemcontent(false);
                    if (temp_post_data->isfile)
                    {
                        temp_post_data->field_item = std::move(carry);
                    }
                    else
                    {
                        // 正式解析没认成文件，把摘掉的尾部放回，一段字节都不丢
                        temp_post_data->field_item.append(carry);
                    }
                }
            }
        }

        // 边界文本正好断在包尾时，它后面那两条横线 "-" "-" 分在两个包里，结束标记认不出来，
        // 请求就会一直等下去。声明的 body 字节已经全部读完，不会再有属于本次请求的字节了，这里收尾。
        // 到这里还没认到闭合分隔符，说明 body 被截断：最后一段的头部没解析过，整段丢掉，
        // 不送进解析器——送进去要么把这个请求判成坏请求，要么交出一个值末尾多两字节的字段。
        if (!peer->isfinish && error == 0 && is_final && !request_chunked)
        {
            peer->isfinish = true;
        }

        // chunked multipart 走到终止块仍没见到闭合 boundary：尾段按上面的既有口径丢弃，
        // 但不改状态码，只留诊断标志，避免业务拿到“部分字段”却毫无感知。
        // fastcgi 的 .php 请求不走本函数（正文原样交给 PHP），因此不会置位。
        // Chunked multipart ended with a terminal chunk but no closing boundary: the trailing
        // part is dropped as before; only a diagnostic flag is set (no status-code change).
        if (error == 0 && is_final && request_chunked && !chunked_payload_closed)
        {
            peer->multipart_truncated = true;
        }

        if (peer->isfinish || (request_chunked && is_final))
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
        peer->rawcontent.append((char *)data, len);
        if (is_final)
        {
            try
            {
                peer->json.from_json(peer->rawcontent);
            }
            catch (...)
            {
                // 畸形 / 超长 JSON 会抛 json_parse_error；必须就地消化，
                // 否则异常沿协程栈上抛会丢弃甚至崩溃该连接处理协程（DoS 向量）。
                // 复用 40081（JSON 体积错误）作为解析失败的统一出口，已由
                // server 侧 http1_send_bad_request 映射为 400 响应。
                error = 40081;
                return;
            }
            peer->isfinish = true;
        }

        if (peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
        {
            error = 40081;
            return;
        }
        break;
    case 4:
        // xml
        peer->rawcontent.append((char *)data, len);
        if (is_final)
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
        if (len > 0)
        {
            read_rawfile_formdata(std::string_view((const char *)data, len));
            if (error > 0)
            {
                return;
            }
        }
        if (is_final)
        {
            // 收尾由这里统一做：chunked 没有 Content-Length，只能在终止块到达时判完整
            // Finalization happens here: chunked has no Content-Length, so completeness is
            // only known when the terminal chunk arrives
            if (uprawfile)
            {
                uprawfile.reset();
            }
            peer->files["rawcontent"]["size"] = request_chunked ? chunk_total : http_content_length;
            peer->isfinish                    = true;
        }
        break;
    }
}

// Transfer-Encoding: chunked 的请求体解码。
// 只有终止块与 trailer 段都结束才置 isfinish；失败按 error 走既有 400 + 关连接路径。
// Decodes a Transfer-Encoding: chunked request body.
// Only a terminal chunk followed by the trailer section sets isfinish; failures use the
// existing error -> 400 + connection close path.
void httpparse::read_http_chunked_block(const unsigned char *buffer, unsigned int buffersize)
{
    unsigned int p = readoffset;
    while (p < buffersize && error == 0 && chunk_state != 5)
    {
        unsigned char c = buffer[p];
        switch (chunk_state)
        {
        case 0:// chunk-size 行，可带 ;ext
            if (c == '\r')
            {
                if (!chunk_size_has_digit)
                {
                    error = 40229;
                    break;
                }
                ++p;
                chunk_state = 1;
                break;
            }
            if (c == '\n')
            {
                // 行尾必须有 CR，裸 LF 视为畸形
                error = 40231;
                break;
            }
            if (chunk_line_bytes >= 1024)
            {
                error = 40229;
                break;
            }
            ++chunk_line_bytes;
            if (c == ';')
            {
                // chunk-ext 整体忽略，只保留对 CR 的定位
                chunk_in_ext = true;
                ++p;
                break;
            }
            if (chunk_in_ext)
            {
                ++p;
                break;
            }
            if (!((c >= '0' && c <= '9') || ((c | 0x20) >= 'a' && (c | 0x20) <= 'f')))
            {
                error = 40229;
                break;
            }
            {
                unsigned int v = (c <= '9') ? (unsigned int)(c - '0') : (unsigned int)((c | 0x20) - 'a' + 10);
                chunk_size_has_digit = true;
                chunk_size_tmp       = chunk_size_tmp * 16 + v;
                if (chunk_size_tmp > CONST_HTTP_BODY_POST_SIZE)
                {
                    error = 40089;
                    break;
                }
            }
            ++p;
            break;
        case 1:// chunk-size 行后的 LF
            if (c != '\n')
            {
                error = 40231;
                break;
            }
            ++p;
            chunk_line_bytes = 0;
            chunk_in_ext     = false;
            if (chunk_size_tmp == 0)
            {
                chunk_remaining = 0;
                chunk_state     = 4;// 终止块，转 trailer 段
            }
            else
            {
                chunk_remaining = chunk_size_tmp;
                chunk_state     = 2;
            }
            break;
        case 2:// chunk 数据
        {
            unsigned int n = buffersize - p;
            if ((unsigned long long)n > chunk_remaining)
            {
                n = static_cast<unsigned int>(chunk_remaining);
            }
            if (n > 0)
            {
                if (!chunked_payload_closed)
                {
#ifdef ENABLE_FASTCGI
                    if (peer->compress == 10)
                    {
                        // .php 走 FastCGI：正文进 peer->output，CONTENT_LENGTH 取它的 size
                        // .php goes through FastCGI: body into peer->output, CONTENT_LENGTH is its size
                        if (peer->output.size() + n > CONST_PHP_BODY_POST_SIZE)
                        {
                            error = 40005;
                            break;
                        }
                        peer->output.append((char *)&buffer[p], n);
                    }
                    else
#endif
                    {
                        append_post_body(buffer + p, n, false);
                    }
                    if (error > 0)
                    {
                        break;
                    }
                }
                p += n;
                chunk_remaining -= n;
                chunk_total += n;
                // chunk_total 统计的是 HTTP 消息体字节数（含闭合 boundary 之后的 epilogue）。
                // 与 Content-Length 路径“按声明 body 长度判限额”的口径一致（头阶段判定见头解析里的
                // check_upload_limit）；它不是表单有效载荷长度。若改成只按有效载荷计，同一条超限报文
                // 会出现“chunked 放行、CL 拒绝”的绕过面，因此这里维持 body 字节口径。
                // chunk_total counts HTTP message-body octets, including bytes after the closing
                // multipart boundary (epilogue). This matches the Content-Length path, which judges
                // the limit by declared body length; it is NOT the form payload length. Counting only
                // the payload would let an over-limit body pass via chunked while CL still rejects it.
                // 解码后的累计正文是上限的唯一依据：没有 Content-Length 可参考
                if (chunk_total > CONST_HTTP_BODY_POST_SIZE)
                {
                    error = 40089;
                    break;
                }
                unsigned int site_rc = peer->check_upload_limit_size(chunk_total);
                if (site_rc > 0)
                {
                    error = site_rc;
                    break;
                }
            }
            if (chunk_remaining == 0)
            {
                chunk_state = 3;
            }
        }
        break;
        case 3:// chunk 数据后的 CR
            if (c != '\r')
            {
                error = 40230;
                break;
            }
            ++p;
            chunk_state = 6;
            break;
        case 6:// chunk 数据后的 LF
            if (c != '\n')
            {
                error = 40230;
                break;
            }
            ++p;
            chunk_size_tmp       = 0;
            chunk_size_has_digit = false;
            chunk_line_bytes     = 0;
            chunk_in_ext         = false;
            chunk_state          = 0;
            break;
        case 4:// trailer 行
            if (c == '\r')
            {
                ++p;
                chunk_state = 7;
                break;
            }
            if (c == '\n')
            {
                error = 40232;
                break;
            }
            if (c < 0x20 && c != '\t')
            {
                error = 40232;
                break;
            }
            ++chunk_trailer_bytes;
            ++chunk_trailer_line;
            if (chunk_trailer_bytes > 16384)
            {
                error = 40232;
                break;
            }
            chunk_trailer_current.push_back(static_cast<char>(c));
            ++p;
            break;
        case 7:// trailer 行的 LF
            if (c != '\n')
            {
                error = 40232;
                break;
            }
            ++p;
            if (chunk_trailer_line == 0)
            {
                chunk_state = 5;// 空行：trailer 段结束
                break;
            }
            // trailer 字段校验后丢弃，不并入 peer->header，避免覆盖同名请求头
            {
                std::string_view line(chunk_trailer_current);
                std::size_t colon = line.find(':');
                if (colon == std::string_view::npos || colon == 0 || colon > http::field_name_max_length ||
                    !http::is_valid_field_name(line.substr(0, colon)))
                {
                    error = 40232;
                    break;
                }
            }
            chunk_trailer_current.clear();
            chunk_trailer_line = 0;
            chunk_state        = 4;
            break;
        case 5:// 已完成：不能继续吞字节，同包里后续字节属于下一条请求
            break;
        default:
            break;
        }
        if (chunk_state == 5 || error > 0)
        {
            break;
        }
    }
    readoffset = p;
    if (error > 0)
    {
        return;
    }
    if (chunk_state == 5)
    {
        // 终止块与 trailer 都在，此时累计正文才算完整
        peer->content_length       = chunk_total;
        peer->ishas_content_length = true;
#ifdef ENABLE_FASTCGI
        if (peer->compress != 10)
#endif
        {
            unsigned char dummy = 0;
            append_post_body(&dummy, 0, true);
        }
        if (error == 0)
        {
            peer->isfinish = true;
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
            unsigned int before_offset = readoffset;
            read_http_post_block(buffer, buffersize);
            if (peer->isfinish || error > 0 || readoffset == before_offset)
            {
                // 本次请求的 body 已消费完（或本包内已无可消费字节），
                // 余下字节不再并入本次请求，也不再往下解析；readoffset 停在残留开头，
                // 是留着下一轮接着解还是直接丢弃，由调用方决定。
                break;
            }
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
                // CORS：OPTIONS 预检的 ACAO 由 send_cors_domain() 按白名单判定输出；
                // 普通请求的 ACAO 在解析到 origin 头时就判定了（process_header_line 的头名长度 6 分支），
                // Origin 早于 Host 时由 getheaderhost() 补判，这里不再需要任何 CORS 处理
                // 响应头用的 peer->keepalive 取自 state.keepalive，两处关闭判断用同一个值，不各看一半
                peer->keepalive = peer->state.keepalive;
                if (method == HEAD_METHOD::POST || method == HEAD_METHOD::QUERY)
                {
                    if (!request_chunked)
                    {
                        // chunked 没有 Content-Length，头阶段无法判限额，改由解码累计量判。
                        // chunked has no Content-Length, so the limit is judged by decoded total later.
                        error = peer->check_upload_limit();
                        if (error > 0)
                        {
                            return;
                        }
                        if (peer->content_length == 0)
                        {
                            peer->isfinish = true;
                        }
                    }
                }
                else if (method == HEAD_METHOD::PUT || method == HEAD_METHOD::DELETE ||
                         method == HEAD_METHOD::TRACE || method == HEAD_METHOD::CONNECT)
                {
                    // 有意不支持的方法：明确回 405 并关闭连接，避免未消费的 body
                    // 被当成下一个请求解析（实现层残留）
                    method_not_allowed = true;
                    return;
                }
                else
                {
                    peer->isfinish = true;
                    if (peer->content_length > 0 || request_chunked)
                    {
                        // GET/HEAD/OPTIONS 等声明了 body（或用了 chunked）：框架不消费它。
                        // 关闭连接而不是复用，避免剩余字节在下一轮被当成新请求解析。
                        // A GET/HEAD/OPTIONS with a body (or chunked): the framework does not
                        // consume it, so close instead of reusing to avoid request smuggling.
                        peer->state.keepalive = false;
                        peer->keepalive       = false;
                    }
                }
                // isuse_fastcgi() 已删除：路由预查挪到 loop 里，php 处理挪到 server 新方法
                http_content_length = 0;
            }
        }

        if (error > 0 || method_unknown)
        {
            // 方法名认不出来时不再解释这一条消息剩下的行：停在这里，readoffset 就停在
            // 出错那一行末尾，与格式错误（error）同一走法。
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

    has_content_length = false;
    method_not_allowed = false;
    method_unknown     = false;

    posttype          = 0;
    http_action_setup = 0;
    headerfinish      = 0;
    port              = 0;

    http_content_length = 0;
    // chunked 解码状态必须逐请求清零，否则上一条请求的分块进度会串到下一条
    // Reset chunked decode state per request, otherwise progress leaks into the next one
    request_chunked       = false;
    expect_continue       = false;
    continue_sent         = false;
    chunk_state           = 0;
    chunk_remaining       = 0;
    chunk_total           = 0;
    chunk_size_tmp        = 0;
    chunk_line_bytes      = 0;
    chunk_trailer_bytes   = 0;
    chunk_trailer_line    = 0;
    chunk_in_ext          = false;
    chunk_size_has_digit  = false;
    chunked_payload_closed = false;
    chunk_trailer_current.clear();
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
