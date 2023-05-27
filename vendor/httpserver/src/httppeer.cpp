/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file httppeer.cpp
 *  @date 2022-11-12
 *
 *  http client peer file
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
#include <concepts>
#include <string_view>
#include <type_traits>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include "request.h"
#include "datetime.h"
#include "client_session.h"
#include "viewso_param.h"
#include "httppeer.h"
#include "serverconfig.h"
#include "http2_frame.h"
#include "http2_huffman.h"
#include "terminal_color.h"
#ifdef ENABLE_BOOST
#include "loadmodule.h"
#include "loadviewso.h"
#include "http_so_common_api.h"
#endif
#include "viewmethold_reg.h"
#include "debug_log.h"
#include "server_localvar.h"
#include "pzcache.h"
namespace http
{

std::map<std::string, regmethold_t> _http_regmethod_table;
void make_404_content(std::shared_ptr<httppeer> peer)
{
    peer->output = "<h3>404 Not Found</h3>";
    peer->output.append("<hr /><p>File: " + peer->urlpath + " </p>");
    peer->status(404);
    peer->type("text/html; charset=utf-8");
}
void httppeer::clear_timeloop_task() { timeloop_num = 0; }
unsigned int httppeer::get_timeloop_count() { return timecount_num; }
void httppeer::add_timeloop_count(unsigned int a)
{
    if (a == 0)
    {
        timecount_num = 0;
    }
    else
    {
        timecount_num++;
    }
}
unsigned char httppeer::add_timeloop_task(const std::string &path_method, unsigned int count_id)
{
    linktype      = 7;
    timeloop_num  = count_id;
    timecount_num = 1;
    if (pathinfos.size() == 0)
    {
        pathinfos.push_back(path_method);
    }
    else
    {
        pathinfos[0] = path_method;
    }
    std::string temptaskhash = path_method;
    temptaskhash.append(url);
    std::size_t temp_name_id = std::hash<std::string>{}(temptaskhash);
    std::ostringstream oss;
    oss << temp_name_id;
    etag = oss.str();
    return linktype;
}
void httppeer::send(const std::string &a)
{
    if (httpv == 2)
    {
        return;
    }
    if (socket_session)
    {
        socket_session->send_data(a);
    }
}
void httppeer::parse_session_file(std::string &sessionfile)
{
    std::string root_path;
    server_loaclvar &localvar = get_server_global_var();
    root_path                 = localvar.temp_path;
    sessionfile.append("_sess");
    root_path.append(sessionfile);

    struct stat sessfileinfo;
    unsigned long long tempsesstime = 0;
    unsigned long long vistsesstime = 0;
    memset(&sessfileinfo, 0, sizeof(sessfileinfo));
    if (stat(root_path.c_str(), &sessfileinfo) == 0)
    {
        if (sessfileinfo.st_mode & S_IFREG)
        {
            tempsesstime = sessfileinfo.st_mtime;
            vistsesstime = sessfileinfo.st_atime;
        }
    }

    unsigned long long reseetime = timeid();

    if (reseetime > (vistsesstime + 5400))
    {
        sessionfile = cookie.get(COOKIE_SESSION_NAME);
        cookie.set(COOKIE_SESSION_NAME, sessionfile, 7200, "/", host);
        send_cookie.set(COOKIE_SESSION_NAME, sessionfile, 7200, "/", host);
    }

    if (tempsesstime > 0 && tempsesstime == sessionfile_time)
    {
        return;
    }
    int fd = open(root_path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        // perror("open");
        return;
    }

    // 锁住整个文件
    struct flock lock = {};
    lock.l_type       = F_RDLCK;
    lock.l_whence     = 0;
    lock.l_start      = 0;
    lock.l_len        = 0;

    lock.l_pid = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        return;
    }
    int filelen = lseek(fd, 0L, SEEK_END);
    sessionfile.clear();
    sessionfile.resize(filelen);
    lseek(fd, 0L, SEEK_SET);
    int readsize = read(fd, sessionfile.data(), filelen);
    if (readsize > 0)
    {
        sessionfile.resize(readsize);
        session.from_json(sessionfile);
    }

    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {

        return;
    }
    close(fd);
    sessionfile_time = tempsesstime;
}
void httppeer::parse_session_memory(std::string &sessionfile_id)
{
    pzcache<OBJ_VALUE> &temp_cache = pzcache<OBJ_VALUE>::conn();
    temp_cache.update(sessionfile_id, 3600);
    session = temp_cache.get(sessionfile_id);
}
std::string httppeer::get_session_id()
{
    if (cookie.check(COOKIE_SESSION_NAME))
    {
        return cookie.get(COOKIE_SESSION_NAME);
    }
    return "";
}
void httppeer::set_session_id(const std::string &a)
{
    cookie.set(COOKIE_SESSION_NAME, a, 7200, "/", host);
    send_cookie.set(COOKIE_SESSION_NAME, a, timeid() + 7200 * 12, "/", host);
    parse_session();
}
void httppeer::parse_session()
{

    if (cookie.check(COOKIE_SESSION_NAME))
    {
        server_loaclvar &localvar = get_server_global_var();
        std::string sessionfile   = cookie.get(COOKIE_SESSION_NAME);
        if (sessionfile.empty())
        {
            return;
        }
        for (unsigned int i = 0; i < sessionfile.size(); i++)
        {
            if (sessionfile[i] == '/')
            {
                cookie.set(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, "/", host);
                send_cookie.set(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, "/", host);
                return;
            }
        }
        switch (localvar.session_type)
        {
        case 0: parse_session_file(sessionfile); break;
        case 1: parse_session_memory(sessionfile); break;
        default: parse_session_file(sessionfile); break;
        }
    }
}
void httppeer::save_session()
{
    server_loaclvar &localvar = get_server_global_var();
    std::string sessionfile;
    if (cookie.check(COOKIE_SESSION_NAME))
    {
        sessionfile = cookie.get(COOKIE_SESSION_NAME);
    }

    if (sessionfile.size() > 0)
    {
        unsigned int j = 0;
        for (unsigned int i = 0; i < sessionfile.size(); i++)
        {
            if (sessionfile[i] == '.' || sessionfile[i] == '/')
            {
                continue;
            }
            sessionfile[j] = sessionfile[i];
            j++;
        }

        if (j > 0)
        {
            sessionfile.resize(j);
        }
        else
        {
            sessionfile.clear();
        }
    }

    if (sessionfile.empty())
    {
        sessionfile =
            client_ip + std::to_string(client_port) + std::to_string(timeid()) + std::to_string(rand_range(1000, 9999));
        sessionfile = std::to_string(std::hash<std::string>{}(sessionfile));
        cookie.set(COOKIE_SESSION_NAME, sessionfile, 7200, "/", host);
        send_cookie.set(COOKIE_SESSION_NAME, sessionfile, 7200, "/", host);
    }
    if (localvar.session_type == 1)
    {
        save_session_memory(sessionfile);
    }
    else
    {
        save_session_file(sessionfile);
    }
}
void httppeer::save_session_memory(std::string &sessionfile)
{
    pzcache<OBJ_VALUE> &temp_cache = pzcache<OBJ_VALUE>::conn();
    temp_cache.save(sessionfile, session, 3600, true);
}
void httppeer::save_session_file(std::string &sessionfile)
{

    std::string root_path;
    // serverconfig &sysconfigpath = getserversysconfig();
    server_loaclvar &localvar = get_server_global_var();
    root_path                 = localvar.temp_path;

    sessionfile.append("_sess");
    root_path.append(sessionfile);

    int fd = open(root_path.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd == -1)
    {
        // perror("open");
        return;
    }
    // 锁住整个文件
    struct flock lock = {};
    lock.l_type       = F_WRLCK;
    lock.l_whence     = 0;
    lock.l_start      = 0;
    lock.l_len        = 0;

    lock.l_pid = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        return;
    }

    sessionfile = session.to_json();

    ssize_t n = write(fd, sessionfile.data(), sessionfile.size());
    if (n > 0)
    {
        n = 0;
    }
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {

        return;
    }
    close(fd);

    sessionfile_time = timeid();
}
void httppeer::clear_session()
{
    // serverconfig &sysconfigpath = getserversysconfig();
    if (cookie.check(COOKIE_SESSION_NAME))
    {
        std::string root_path;
        server_loaclvar &localvar = get_server_global_var();
        root_path                 = localvar.temp_path;

        std::string sessionfile = cookie.get(COOKIE_SESSION_NAME);
        if (sessionfile.empty())
        {
            return;
        }
        if (localvar.session_type == 1)
        {
            pzcache<OBJ_VALUE> &temp_cache = pzcache<OBJ_VALUE>::conn();
            temp_cache.remove(sessionfile);
            return;
        }

        sessionfile.append("_sess");
        root_path.append(sessionfile);

        struct stat sessfileinfo;
        // unsigned long long tempsesstime = 0;
        memset(&sessfileinfo, 0, sizeof(sessfileinfo));
        if (stat(root_path.c_str(), &sessfileinfo) == 0)
        {

            if (sessfileinfo.st_mode & S_IFREG)
            {
                remove(root_path.c_str());
                sessionfile_time = 0;
                session.clear();
                cookie.set(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, "/", host);
                send_cookie.set(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, "/", host);
            }
        }
        else
        {
            return;
        }
    }
}
std::string httppeer::get_sitepath() { return getserversysconfig().getsitepath(host); }
std::string httppeer::get_hosturl()
{
    std::string tempurl;
    if (isssl)
    {
        tempurl.append("https://");
    }
    else
    {
        tempurl.append("http://");
    }
    tempurl.append(host);
    if (isssl)
    {
        if (server_port != 443)
        {
            tempurl.push_back(':');
            tempurl.append(std::to_string(server_port));
        }
    }
    else
    {
        if (server_port != 80)
        {
            tempurl.push_back(':');
            tempurl.append(std::to_string(server_port));
        }
    }
    return tempurl;
}
bool httppeer::isshow_directory()
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.map_value.find(host) != sysconfigpath.map_value.end())
    {

        if (sysconfigpath.map_value[host]["directorylist"].empty())
        {
            if (sysconfigpath.map_value["default"]["directorylist"].size() > 0 &&
                sysconfigpath.map_value["default"]["directorylist"][0] == '1')
            {
                return true;
            }
        }
        else if (sysconfigpath.map_value["default"]["directorylist"].size() > 0 &&
                 sysconfigpath.map_value[host]["directorylist"][0] == '1')
        {
            return true;
        }
    }
    else
    {
        if (sysconfigpath.map_value["default"]["directorylist"].size() > 0 &&
            sysconfigpath.map_value["default"]["directorylist"][0] == '1')
        {
            return true;
        }
    }
    return false;
}
unsigned char httppeer::get_fileinfo()
{

    std::string filenamebase, fileexttype;
    serverconfig &sysconfigpath = getserversysconfig();

    sendfilename = sitepath;
    if (sendfilename.size() > 0)
    {
        if (sendfilename.back() != '/')
        {
            sendfilename.push_back('/');
        }
    }
    else
    {
        sendfilename.push_back('/');
    }
    for (unsigned int i = 0; i < pathinfos.size(); i++)
    {
        if (i > 0)
        {
            sendfilename.append("/");
        }
        sendfilename.append(pathinfos[i]);
    }

    if (pathinfos.size() > 0)
    {
        filenamebase = pathinfos.back();
    }
    else
    {
        filenamebase.clear();
    }

    unsigned int filebasesize   = filenamebase.size();
    unsigned int filenameoffset = 0;

    if (filebasesize > 0)
    {
        for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
        {
            if (filenamebase[filenameoffset] == '.')
            {
                break;
            }
        }
        filenameoffset += 1;
        for (; filenameoffset < filebasesize; filenameoffset++)
        {
            fileexttype.push_back(filenamebase[filenameoffset]);
        }
    }

    memset(&fileinfo, 0, sizeof(fileinfo));
    sendfiletype = 0;
    if (stat(sendfilename.c_str(), &fileinfo) == 0)
    {

        if (fileinfo.st_mode & S_IFDIR)
        {
            sendfiletype             = 2;
            unsigned int nowpathsize = sendfilename.size();
            if (sendfilename.size() > 0 && sendfilename.back() != '/')
            {
                sendfilename.push_back('/');
            }
            sendfilename.append(sysconfigpath.map_value["default"]["index"]);
            memset(&fileinfo, 0, sizeof(fileinfo));
            if (stat(sendfilename.c_str(), &fileinfo) == 0)
            {
                if (fileinfo.st_mode & S_IFREG)
                {
                    sendfiletype = 1;
                    fileexttype  = "html";
                }
            }
            if (sendfiletype == 2)
            {
                sendfilename.resize(nowpathsize);
            }
        }
        else if (fileinfo.st_mode & S_IFREG)
        {
            sendfiletype = 1;
            // use cahce html ,modulepath same urlpath
            // sample: /module/method/202204/22333.html

            // if(isusehtmlcache&&header->pathinfo.size()>2&&chachefiletime>10&&chachefiletime<(http::timeid()-(unsigned
            // long)fileinfo.st_mtime)){
            //     pathtype=3;
            // }
            if (sysconfigpath.siteusehtmlchache && pathinfos.size() > 2 && sysconfigpath.siteusehtmlchachetime > 10 &&
                sysconfigpath.siteusehtmlchachetime < (timeid() - (unsigned long)fileinfo.st_mtime))
            {
                sendfiletype = 3;
            }
        }
    }
    return sendfiletype;
}
std::shared_ptr<httppeer> httppeer::get_ptr() { return shared_from_this(); }
void httppeer::send_files(std::string filename)
{
    // socket_session->send_data("hello world!");
}
unsigned int httppeer::get_status() { return status_code; }
void httppeer::status(unsigned int http_code) { status_code = http_code; }
void httppeer::type(const std::string &a) { content_type = a; }
bool httppeer::isset_type() { return content_type.size() > 0 ? true : false; }
void httppeer::set_header(const std::string &a, const std::string &v)
{
    if (httpv == 2)
    {
        std::string temp = a;
        std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
        if (http2_header_codes_table[temp] > 0)
        {
            http2_send_header[http2_header_codes_table[temp]] = v;
        }
        else
        {
            send_header[temp] = v;
        }
    }
    else
    {
        send_header[a] = v;
    }
}
void httppeer::set_cookie(std::string key,
                          std::string val,
                          long long exptime,
                          std::string domain,
                          std::string path,
                          bool secure,
                          bool httponly,
                          std::string issamesite)
{
    cookie.set(key, val, exptime, domain, path, secure, httponly, issamesite);
    send_cookie.set(key, val, exptime, domain, path, secure, httponly, issamesite);
}
std::list<std::string> httppeer::cookietoheader()
{
    std::list<std::string> headerlists;
    std::string temph;
    for (auto &iter : send_cookie)
    {
        temph.clear();
        std::string key        = iter.first;
        std::string domain     = send_cookie.getDomain(key);
        std::string domainpath = send_cookie.getPath(key);
        unsigned long timeexp  = send_cookie.getExpires(key);
        unsigned char issecure = send_cookie.getSecure(key);
        unsigned char isonly   = send_cookie.getHttponly(key);

        std::string samesite = send_cookie.getSamesite(key);

        temph.append(url_encode(iter.first.data(), iter.first.size()));
        temph.push_back('=');
        temph.append(url_encode(iter.second.data(), iter.second.size()));

        if (timeexp > 0 && timeexp < 63072000)
        {
            timeexp = timeid() + timeexp;
        }

        key.clear();
        if (timeexp > 0)
        {
            key = get_gmttime((unsigned long long)timeexp);
            temph.append("; Expires=");
            temph.append(key);
        }

        if (domain.size() > 1)
        {
            temph.append("; Domain=");
            temph.append(domain);
        }

        if (domainpath.size() > 0)
        {
            temph.append("; Path=");
            temph.append(domainpath);
        }

        if (issecure > 0)
        {
            temph.append("; Secure");
        }

        if (isonly > 0)
        {
            temph.append("; HttpOnly");
        }
        if (!samesite.empty())
        {
            switch (samesite[0])
            {
            case 'L':
            case 'l': temph.append("; SameSite=Lax"); break;
            case 'S':
            case 's': temph.append("; SameSite=Strict"); break;
            }
        }

        headerlists.emplace_back(temph);
    }
    return headerlists;
}
void httppeer::scheme(unsigned char code) { isssl = (code == 1) ? 1 : 0; }
void httppeer::length(unsigned long long num) { content_length = num; }
std::string httppeer::make_http1_header()
{
    std::string http1header;
    http1header.append("HTTP/1.1 ");
    http1header.append(std::to_string(status_code));
    http1header.push_back(' ');
    http1header.append(http_status_static_table[status_code]);
    http1header.append("\r\n");

    if (ischunked)
    {
        http1header.append("Transfer-Encoding: chunked\r\n");
    }
    else
    {
        http1header.append("Content-Length: ");
        http1header.append(std::to_string(content_length));
        http1header.append("\r\n");
    }
    if (compress == 1)
    {
        http1header.append("Content-Encoding: ");
        http1header.append("gzip");
        http1header.append("\r\n");
    }
    else if (compress == 2)
    {
        http1header.append("Content-Encoding: ");
        http1header.append("br");
        http1header.append("\r\n");
    }
    if (content_type.size() > 0)
    {
        http1header.append("Content-Type: ");
        http1header.append(content_type);
        http1header.append("\r\n");
    }

    if (keeplive)
    {
        http1header.append("Connection: Keep-Alive\r\n");
    }
    else
    {
        http1header.append("Connection: close\r\n");
    }

    if (send_header.size() > 0)
    {
        for (auto [first, second] : send_header)
        {
            http1header.append(first);
            http1header.append(": ");
            http1header.append(second);
            http1header.append("\r\n");
        }
    }
    std::list<std::string> cookielist = cookietoheader();

    if (cookielist.size() > 0)
    {
        for (; cookielist.size() > 0;)
        {
            http1header.append("Set-Cookie: ");
            http1header.append(cookielist.front());
            http1header.append("\r\n");
            cookielist.pop_front();
        }
    }
    return http1header;
}
std::string httppeer::make_http2_header(unsigned char flag_code)
{
    std::string http2header;

    http2header.resize(9);
    http2header[3]        = 0x01;
    unsigned int streamid = stream_id;
    http2header[8]        = streamid & 0xFF;
    streamid              = streamid >> 8;
    http2header[7]        = streamid & 0xFF;
    streamid              = streamid >> 8;
    http2header[6]        = streamid & 0xFF;
    streamid              = streamid >> 8;
    http2header[5]        = streamid & 0xFF;

    make_http2_headers_static(http2header, status_code);
    set_http2_headers_flag(http2header, HTTP2_HEADER_END_HEADERS | flag_code);
    make_http2_headers_item3(http2header, HTTP2_CODE_content_length, content_length);

    make_http2_headers_item3(http2header, HTTP2_CODE_content_type, content_type);

    if (compress == 1)
    {
        make_http2_headers_item3(http2header, HTTP2_CODE_content_encoding, "gzip");
    }
    else if (compress == 2)
    {
        make_http2_headers_item3(http2header, HTTP2_CODE_content_encoding, "br");
    }

    if (http2_send_header.size() > 0)
    {
        for (auto [first, second] : http2_send_header)
        {
            make_http2_headers_item3(http2header, (unsigned char)first, second);
        }
    }

    if (send_header.size() > 0)
    {
        for (auto [first, second] : send_header)
        {
            make_http2_headers_item3(http2header, first, second);
        }
    }
    std::list<std::string> cookielist = cookietoheader();

    if (cookielist.size() > 0)
    {
        for (; cookielist.size() > 0;)
        {
            make_http2_headers_item3(http2header, HTTP2_CODE_set_cookie, cookielist.front());
            cookielist.pop_front();
        }
    }

    set_http2_headers_size(http2header, (unsigned int)http2header.size() - 9);
    return http2header;
}
void httppeer::goto_url(const std::string &url, unsigned char second, const std::string &msg)
{
    output.append("<html><head><meta http-equiv=\"refresh\" content=\"");
    output.append(std::to_string(second));
    output.append(";url=");
    output.append(url);
    output.append("\"></head><body>");
    if (msg.size() > 0)
    {
        output.append("<div style=\"margin-top:20px;margin-left: auto;margin-right: auto;with:60%\"><h3>");
        output.append(msg);
        output.append("</h3></div>");
    }
    output.append("</body></html>");
}

httppeer &httppeer::operator<<(OBJ_VALUE &a)
{
    output.append(a.to_string());
    return *this;
}
httppeer &httppeer::operator<<(const std::string &a)
{
    output.append(a);
    return *this;
}
httppeer &httppeer::operator<<(std::string &&a)
{
    output.append(a);
    return *this;
}
httppeer &httppeer::operator<<(std::string &a)
{
    output.append(a);
    return *this;
}
httppeer &httppeer::operator<<(std::string_view a)
{
    output.append(a);
    return *this;
}
httppeer &httppeer::operator<<(char a)
{
    output.push_back(a);
    return *this;
}
httppeer &httppeer::operator<<(unsigned char a)
{
    output.push_back(a);
    return *this;
}
httppeer &httppeer::operator<<(const char *a)
{
    output.append(a);
    return *this;
}
httppeer &httppeer::operator<<(unsigned int a)
{
    output.append(std::to_string(a));
    return *this;
}
httppeer &httppeer::operator<<(int a)
{
    output.append(std::to_string(a));
    return *this;
}
httppeer &httppeer::operator<<(unsigned long long a)
{
    output.append(std::to_string(a));
    return *this;
}
httppeer &httppeer::operator<<(unsigned long a)
{
    output.append(std::to_string(a));
    return *this;
}
httppeer &httppeer::operator<<(long long a)
{
    output.append(std::to_string(a));
    return *this;
}
httppeer &httppeer::operator<<(float a)
{
    output.append(std::to_string(a));
    return *this;
}
httppeer &httppeer::operator<<(double a)
{
    output.append(std::to_string(a));
    return *this;
}
template <typename T> httppeer &httppeer::operator<<(VALNUM_T auto a)
{
    output.append(std::to_string(a));
    return *this;
}

template <typename T> httppeer &httppeer::operator<<(T a)
{
    output.append(a.to_string());
    return *this;
}
template <typename T> httppeer &httppeer::operator<<(T *a)
{
    output.append(a->to_string());
    return *this;
}

httppeer &httppeer::getpeer() { return *this; }
httppeer &httppeer::out(std::string a)
{
    output.append(a);
    return *this;
}
httppeer &httppeer::out(const std::string &a)
{
    output.append(a);
    return *this;
}

void httppeer::view(const std::string &a)
{
    try
    {
        struct view_param tempvp(get, post, cookie, session);

        serverconfig &sysconfigpath = getserversysconfig();
        if (sysconfigpath.map_value.find(host) != sysconfigpath.map_value.end())
        {
            if (sysconfigpath.map_value[host].find("viewsopath") != sysconfigpath.map_value[host].end())
            {
                tempvp.viewsopath = sysconfigpath.map_value[host]["viewsopath"];
            }
        }
        if (tempvp.viewsopath.empty())
        {
            tempvp.viewsopath = sysconfigpath.map_value["default"]["viewsopath"];
        }
        if (tempvp.viewsopath.size() > 0 && tempvp.viewsopath.back() != '/')
        {
            tempvp.viewsopath.push_back('/');
        }

        std::string tempp = tempvp.viewsopath;
        tempp.append(a);

        try
        {
            VIEW_REG &viewreg = get_viewmetholdreg();
            if (viewreg.find(a) != viewreg.end())
            {
                output.append(viewreg[a](tempvp, val));
                return;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return;
        }
#ifdef ENABLE_BOOST
        if (isso)
        {
            if (clientapi::get().map_value.find(host) != clientapi::get().map_value.end())
            {
                if (clientapi::get().map_value[host].find("viewsopath") != clientapi::get().map_value[host].end())
                {
                    tempvp.viewsopath = clientapi::get().map_value[host]["viewsopath"];
                }
            }
            if (tempvp.viewsopath.empty())
            {
                tempvp.viewsopath = clientapi::get().map_value["default"]["viewsopath"];
            }
            if (tempvp.viewsopath.size() > 0 && tempvp.viewsopath.back() != '/')
            {
                tempvp.viewsopath.push_back('/');
            }
            tempp = tempvp.viewsopath;
            tempp.append(a);
            output.append(clientapi::get().api_loadview(tempp)(tempvp, val));
        }
        else
        {
            output.append(loadviewso(tempp)(tempvp, val));
        }
#endif
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
void httppeer::view(const std::string &a, OBJ_VALUE &b)
{
    try
    {
        struct view_param tempvp(get, post, cookie, session);

        serverconfig &sysconfigpath = getserversysconfig();
        if (sysconfigpath.map_value.find(host) != sysconfigpath.map_value.end())
        {
            if (sysconfigpath.map_value[host].find("viewsopath") != sysconfigpath.map_value[host].end())
            {
                tempvp.viewsopath = sysconfigpath.map_value[host]["viewsopath"];
            }
        }
        if (tempvp.viewsopath.empty())
        {
            tempvp.viewsopath = sysconfigpath.map_value["default"]["viewsopath"];
        }
        if (tempvp.viewsopath.size() > 0 && tempvp.viewsopath.back() != '/')
        {
            tempvp.viewsopath.push_back('/');
        }
        std::string tempp = tempvp.viewsopath;
        tempp.append(a);
        try
        {
            VIEW_REG &viewreg = get_viewmetholdreg();
            if (viewreg.find(a) != viewreg.end())
            {
                output.append(viewreg[a](tempvp, b));
                return;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return;
        }
#ifdef ENABLE_BOOST
        if (isso)
        {
            if (clientapi::get().map_value.find(host) != clientapi::get().map_value.end())
            {
                if (clientapi::get().map_value[host].find("viewsopath") != clientapi::get().map_value[host].end())
                {
                    tempvp.viewsopath = clientapi::get().map_value[host]["viewsopath"];
                }
            }
            if (tempvp.viewsopath.empty())
            {
                tempvp.viewsopath = clientapi::get().map_value["default"]["viewsopath"];
            }
            if (tempvp.viewsopath.size() > 0 && tempvp.viewsopath.back() != '/')
            {
                tempvp.viewsopath.push_back('/');
            }
            tempp = tempvp.viewsopath;
            tempp.append(a);
            output.append(clientapi::get().api_loadview(tempp)(tempvp, b));
        }
        else
        {
            output.append(loadviewso(tempp)(tempvp, b));
        }
#endif
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
std::string httppeer::fetchview(const std::string &a)
{
    struct view_param tempvp(get, post, cookie, session);
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.map_value.find(host) != sysconfigpath.map_value.end())
    {
        if (sysconfigpath.map_value[host].find("viewsopath") != sysconfigpath.map_value[host].end())
        {
            tempvp.viewsopath = sysconfigpath.map_value[host]["viewsopath"];
        }
    }
    if (tempvp.viewsopath.empty())
    {
        tempvp.viewsopath = sysconfigpath.map_value["default"]["viewsopath"];
    }
    if (tempvp.viewsopath.size() > 0 && tempvp.viewsopath.back() != '/')
    {
        tempvp.viewsopath.push_back('/');
    }
    std::string tempp = tempvp.viewsopath;
    tempp.append(a);
    try
    {
        VIEW_REG &viewreg = get_viewmetholdreg();
        if (viewreg.find(a) != viewreg.end())
        {
            return viewreg[a](tempvp, val);
        }
        else
        {
            return "";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return "";
    }
#ifdef ENABLE_BOOST
    try
    {

        if (isso)
        {
            if (clientapi::get().map_value.find(host) != clientapi::get().map_value.end())
            {
                if (clientapi::get().map_value[host].find("viewsopath") != clientapi::get().map_value[host].end())
                {
                    tempvp.viewsopath = clientapi::get().map_value[host]["viewsopath"];
                }
            }
            if (tempvp.viewsopath.empty())
            {
                tempvp.viewsopath = clientapi::get().map_value["default"]["viewsopath"];
            }
            if (tempvp.viewsopath.size() > 0 && tempvp.viewsopath.back() != '/')
            {
                tempvp.viewsopath.push_back('/');
            }
            tempp = tempvp.viewsopath;
            tempp.append(a);
            return (clientapi::get().api_loadview(tempp)(tempvp, val));
        }
        else
        {

            return loadviewso(tempp)(tempvp, val);
        }
    }
    catch (const std::exception &e)
    {
        return "";
    }
#endif
}
std::string httppeer::fetchview(const std::string &a, OBJ_VALUE &b)
{
    struct view_param tempvp(get, post, cookie, session);
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.map_value.find(host) != sysconfigpath.map_value.end())
    {
        if (sysconfigpath.map_value[host].find("viewsopath") != sysconfigpath.map_value[host].end())
        {
            tempvp.viewsopath = sysconfigpath.map_value[host]["viewsopath"];
        }
    }
    if (tempvp.viewsopath.empty())
    {
        tempvp.viewsopath = sysconfigpath.map_value["default"]["viewsopath"];
    }
    if (tempvp.viewsopath.size() > 0 && tempvp.viewsopath.back() != '/')
    {
        tempvp.viewsopath.push_back('/');
    }
    std::string tempp = tempvp.viewsopath;
    tempp.append(a);
    try
    {
        VIEW_REG &viewreg = get_viewmetholdreg();
        if (viewreg.find(a) != viewreg.end())
        {
            return viewreg[a](tempvp, b);
        }
        else
        {
            return "";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return "";
    }
#ifdef ENABLE_BOOST
    try
    {
        if (isso)
        {
            if (clientapi::get().map_value.find(host) != clientapi::get().map_value.end())
            {
                if (clientapi::get().map_value[host].find("viewsopath") != clientapi::get().map_value[host].end())
                {
                    tempvp.viewsopath = clientapi::get().map_value[host]["viewsopath"];
                }
            }
            if (tempvp.viewsopath.empty())
            {
                tempvp.viewsopath = clientapi::get().map_value["default"]["viewsopath"];
            }
            if (tempvp.viewsopath.size() > 0 && tempvp.viewsopath.back() != '/')
            {
                tempvp.viewsopath.push_back('/');
            }
            tempp = tempvp.viewsopath;
            tempp.append(a);
            return (clientapi::get().api_loadview(tempp)(tempvp, b));
        }
        else
        {
            return loadviewso(tempp)(tempvp, b);
        }
    }
    catch (const std::exception &e)
    {
        return "";
    }
#endif
}
void httppeer::out_json(OBJ_VALUE &a)
{
    content_type = "application/json";
    output       = a.to_json();
}
void httppeer::out_json()
{
    content_type = "application/json";
    if (output.size() < 1)
    {
        output = val.to_json();
    }
}
void httppeer::json_type() { content_type = "application/json"; }
void httppeer::cors_domain(const std::string &name, const std::string &header_v)
{
    if (httpv == 2)
    {

        if (http2_header_codes_table["access-control-allow-origin"] > 0)
        {
            http2_send_header[http2_header_codes_table["access-control-allow-origin"]] = name;
        }
        else
        {
            send_header["access-control-allow-origin"] = name;
        }
        if (header_v.size() > 0)
        {
            send_header["access-control-allow-headers"] = header_v;
        }
        else
        {
            send_header["access-control-allow-headers"] = "*";
        }
    }
    else
    {
        send_header["Access-Control-Allow-Origin"] = name;
        if (header_v.size() > 0)
        {
            send_header["Access-Control-Allow-Headers"] = header_v;
        }
        else
        {
            send_header["Access-Control-Allow-Headers"] = "*";
        }
    }
}
void httppeer::cors_method(const std::string &header_v)
{
    if (httpv == 2)
    {
        if (header_v.size() > 0)
        {
            send_header["access-control-expose-headers"] = header_v;
        }
        else
        {
            send_header["access-control-expose-headers"] = "*";
        }
        send_header["access-control-max-age"]       = "86400";
        send_header["access-control-allow-methods"] = "POST, GET, OPTIONS";
    }
    else
    {
        if (header_v.size() > 0)
        {
            send_header["Access-Control-Expose-Headers"] = header_v;
        }
        else
        {
            send_header["Access-Control-Expose-Headers"] = "*";
        }
        send_header["Access-Control-Max-Age"]       = "86400";
        send_header["Access-Control-Allow-Methods"] = "POST, GET, OPTIONS";
    }
}
void httppeer::push_path_method(const std::string &m_name) { path_method_names.push(m_name); }
std::string httppeer::pop_path_method()
{
    if (path_method_names.empty())
    {
        return "";
    }
    std::string tempmethod = path_method_names.top();
    path_method_names.pop();
    return tempmethod;
}
}// namespace http
