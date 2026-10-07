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

#ifndef _MSC_VER
#include <sys/fcntl.h>
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif
#include "cost_define.h"
#include "request.h"
#include "datetime.h"
#include "session_id.h"
#include "client_session.h"
#include "viewso_param.h"
#include "httppeer.h"
#include "router.h"
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
#include "func.h"

namespace http
{

// — v6 router —
std::vector<reg_methold_mid_t> _handlers;
std::vector<SiteSlot> _slots;
std::map<unsigned int, std::vector<std::string>> _urlpath_map;
std::map<std::string, unsigned int> _site_to_slot;
unsigned int _route_reg_dropped = 0;
unsigned int _route_reg_dup     = 0;

// — v6 helper —
// router 自己分配 slot_id，serverconfig 只管 host_toint（真实请求域名 → host_index）
// 初始化顺序：init_path() 先读 conf → router_init_sites() 再分配 slot
// alias_domain 语义：多个真实域名（mainhost）统一归属到一个注册标识，不受域名影响
// 因此 slot_id 按 alias_domain（有就用 alias_domain，没就用 mainhost）做唯一分组分配
void router_init_sites()
{
    auto &cfg = getserversysconfig();

    // 0 号 slot 固定是全局兜底
    _site_to_slot.clear();
    _site_to_slot[""]  = 0;
    _site_to_slot["*"] = 0;

    // 先收集所有 "site key"（alias_domain 优先，没有用 mainhost）
    // 按出现顺序去重，第一个 key 固定 slot 1
    std::vector<std::string> unique_keys;
    unique_keys.reserve(cfg.sitehostinfos.size());
    for (auto const &info : cfg.sitehostinfos)
    {
        std::string key = info.alias_domain.empty() ? info.mainhost : info.alias_domain;
        if (key.empty())
            continue;
        if (_site_to_slot.find(key) == _site_to_slot.end())
        {
            unsigned int slot  = unique_keys.size() + 1;// 从 1 开始，0 留给全局
            _site_to_slot[key] = slot;
            unique_keys.push_back(key);
        }
    }

    _slots.resize(unique_keys.size() + 1);// slot 0 (global) + unique_keys

    // 把 slot_id 写回每个 host_index 对应的 sitehostinfo
    for (unsigned int i = 0; i < cfg.sitehostinfos.size(); ++i)
    {
        auto const &info             = cfg.sitehostinfos[i];
        std::string key              = info.alias_domain.empty() ? info.mainhost : info.alias_domain;
        auto it                      = _site_to_slot.find(key);
        cfg.sitehostinfos[i].slot_id = (it != _site_to_slot.end()) ? it->second : 0;
    }
}

// 未知 site 返回 -1，由 reg_add 决定丢弃：静默落进 slot 0 会把只有某个站点配的
// 路由暴露给所有 vhost，还会顶掉同名全局路由
int site_to_slot(const std::string &site)
{
    if (site.empty())
        return 0;
    auto it = _site_to_slot.find(site);
    if (it != _site_to_slot.end())
        return static_cast<int>(it->second);
    return -1;
}

// 注册表的键一律以 '/' 开头。@urlpath 生成口已经强制前置（autopickmethod.cpp 的二次规范化），
// 手写 REG_* 口没有；而 URL 面查表按原样匹配、不给请求路径补斜杠（见 router::lookup_exact 的注释），
// 所以漏一个斜杠的那条注册打不到，只会静默 404。两个入口（reg_add 写表、reg_urlpath 按键回查）
// 必须走同一个规范化，否则混着写的时候参数名表会挂不上。
// 空串原样留着：请求 "/" 组装出的 urlpath 就是空串，注册 "" 是给站点根用的。
static std::string norm_route_key(const std::string &path)
{
    if (path.empty() || path[0] == '/')
        return path;
    return "/" + path;
}

unsigned int reg_add(const std::string &site, const std::string &path, reg_methold_mid_t &&entry)
{
    std::string key = norm_route_key(path);
    int slot        = site_to_slot(site);
    if (slot < 0 || static_cast<size_t>(slot) >= _slots.size())
    {
        ++_route_reg_dropped;
        // stderr 而不是 DEBUG_LOG：这条是「你 conf 里没有这个站点段」的部署诊断，
        // Release 也要看得见，且 stdout 重定向到文件时是块缓冲，启动期这几行会压在缓冲区里
        fprintf(stderr, "[ROUTE-WARN] reg site not in conf, route dropped:site=%s path=%s reg@%s:%u\n", site.c_str(), key.c_str(), entry.reg_file, entry.reg_line);
        return (unsigned int)-1;
    }
    unsigned int idx = (unsigned int)_handlers.size();
    auto &tbl        = _slots[static_cast<size_t>(slot)].path_map;
    auto ins         = tbl.emplace(key, idx);
    if (!ins.second)
    {
        // 重名：先 emplace 探测、失败就不 push —— 这样不会产生「挂在 _handlers 里却没任何
        // path_map 指向」的孤儿 entry（那种 entry 会让 routes_text() 末尾的 handlers N 大于真实
        // 路由数 total，排查时误读为「有路由没挂上」）。
        auto const &kept = _handlers[ins.first->second];
        ++_route_reg_dup;
        fprintf(stderr, "[ROUTE-WARN] duplicate route name kept first:slot=%d path=%s keep reg@%s:%u drop reg@%s:%u\n", slot, key.c_str(), kept.reg_file, kept.reg_line, entry.reg_file, entry.reg_line);
        return ins.first->second;
    }
    _handlers.push_back(std::move(entry));
    return idx;
}

// 同名路径的重复注册只留下第一条 handler（见 reg_add），但两条注解的参数名表都会走到这里：
// 后一份属于被丢弃的那条注解，覆盖进去就让存活的 handler 按错名字填参，所以只认第一次
static void set_urlpath_once(unsigned int idx, std::vector<std::string> &&names, const std::string &site, const std::string &path)
{
    auto ins = _urlpath_map.emplace(idx, std::move(names));
    if (!ins.second)
    {
        fprintf(stderr, "[ROUTE-WARN] duplicate urlpath name table kept first:idx=%u site=%s path=%s\n", idx, site.c_str(), path.c_str());
    }
}

void reg_urlpath(unsigned int idx, std::vector<std::string> names)
{
    if (names.empty())
        return;
    set_urlpath_once(idx, std::move(names), "", "");
}
void reg_urlpath(const std::string &site, const std::string &path, std::vector<std::string> names)
{
    if (names.empty())
        return;
    // 与 reg_add 同一个规范化：这里按 path 文本回查刚写进去的键，两边拼写不一致就静默挂不上参数名表
    std::string key = norm_route_key(path);
    int slot        = site_to_slot(site);
    if (slot < 0 || static_cast<size_t>(slot) >= _slots.size())
        return;// reg_add 已为这条报过警

    auto &tbl = _slots[static_cast<size_t>(slot)].path_map;
    auto it   = tbl.find(key);
    if (it == tbl.end() && slot != 0)
    {
        auto &global = _slots[0].path_map;
        auto git     = global.find(key);
        if (git != global.end())
            set_urlpath_once(git->second, std::move(names), site, key);
        return;
    }
    if (it != tbl.end())
        set_urlpath_once(it->second, std::move(names), site, key);
}
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
    // 任务名不再写进 pathinfos：那是解析器每个请求重填的路由段表（见头文件里的说明）
    timeloop_taskname        = path_method;
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
        socket_session->send_writer(a);
    }
}
void httppeer::flush_out()
{
    // current empty chunk out
    send_header["Transfer-Encoding"] = "chunked";
    ischunked                        = true;
    if (httpv == 2)
    {
        return;
    }
    if (socket_session)
    {
        return;
    }
}
void httppeer::parse_session_file(const std::string &sessionfile)
{
    std::string root_path, temp_session_file;
    server_loaclvar &localvar = get_server_global_var();
    root_path                 = localvar.temp_path;
    if (root_path.size() > 0 && root_path.back() != '/')
    {
        root_path.push_back('/');
    }
    //sessionfile.append("_sess");
    for (unsigned int i = 0; i < sessionfile.size(); i++)
    {
        if (sessionfile[i] > 0x2F && sessionfile[i] < 0x3A)
        {
            root_path.push_back(sessionfile[i]);
        }
        else if (sessionfile[i] > 0x40 && sessionfile[i] < 0x5B)
        {
            root_path.push_back(sessionfile[i]);
        }
        else if (sessionfile[i] > 0x60 && sessionfile[i] < 0x7B)
        {
            root_path.push_back(sessionfile[i]);
        }
        else if (sessionfile[i] == '-')
        {
            root_path.push_back(sessionfile[i]);
        }
        else if (sessionfile[i] == '_')
        {
            root_path.push_back(sessionfile[i]);
        }
    }
    //root_path.append(sessionfile);
    root_path.append("_sess");

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
        temp_session_file = cookie.get(COOKIE_SESSION_NAME);
        // cookie.set(COOKIE_SESSION_NAME, sessionfile, 7200, "/", host);
        // send_cookie.set(COOKIE_SESSION_NAME, sessionfile, 7200, "/", host);
        set_cookie(COOKIE_SESSION_NAME, temp_session_file, 30000, host, "/", false, true, session_samesite());
    }

    if (tempsesstime > 0 && tempsesstime == sessionfile_time)
    {
        return;
    }
#ifndef _MSC_VER
    int fd = open(root_path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        // perror("open");
        return;
    }

#ifndef _WIN32
    // 锁住整个文件
    struct flock lock = {};
    lock.l_type       = F_RDLCK;
    lock.l_whence     = 0;
    lock.l_start      = 0;
    lock.l_len        = 0;

    lock.l_pid = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        close(fd);
        return;
    }
#else
    // lock file by using win32 api
    auto native_handle = (HANDLE)_get_osfhandle(fd);
    if (!LockFile(native_handle, 0, 0, 0, MAXDWORD))
    {
        close(fd);
        return;
    }
#endif

    int filelen = lseek(fd, 0L, SEEK_END);
    if (filelen > 0)
    {
        temp_session_file.clear();
        temp_session_file.resize(filelen);
        lseek(fd, 0L, SEEK_SET);
        int readsize = read(fd, temp_session_file.data(), filelen);
        if (readsize > 0)
        {
            temp_session_file.resize(readsize);
            // session 文件内容损坏时不得抛出异常, 否则会中断整个请求连接
            try
            {
                session.from_json(temp_session_file);
            }
            catch (...)
            {
                session.clear();
            }
        }
    }

#ifndef _WIN32
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        close(fd);
        return;
    }
#else
    if (!UnlockFile(native_handle, 0, 0, 0, MAXDWORD))
    {
        close(fd);
        return;
    }
#endif

    close(fd);
#endif

    sessionfile_time = tempsesstime;
}
void httppeer::parse_session_memory(const std::string &sessionfile_id)
{
    http::pzcache<http::obj_val> &temp_cache = http::pzcache<http::obj_val>::conn();
    temp_cache.update(sessionfile_id, 30000);
    try
    {
        session = temp_cache.get(sessionfile_id);
    }
    catch (const char *e)
    {
        session.clear();
    }
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
    // cookie.set(COOKIE_SESSION_NAME, a, 7200, "/", host);
    // send_cookie.set(COOKIE_SESSION_NAME, a, timeid() + 7200 * 12, "/", host);
    set_cookie(COOKIE_SESSION_NAME, a, (timeid() + 7200 * 12), host, "/", false, true, session_samesite());
    parse_session();
}
void httppeer::parse_session()
{
    if (cookie.empty())
    {
        return;
    }
    if (cookie.check(COOKIE_SESSION_NAME))
    {
        std::string sessionfile = cookie.get(COOKIE_SESSION_NAME);
        if (sessionfile.empty())
        {
            return;
        }
        //
        // for (unsigned int i = 0; i < sessionfile.size(); i++)
        // {
        //     if (sessionfile[i] == '/')
        //     {
        //         // cookie.set(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, "/", host);
        //         // send_cookie.set(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, "/", host);
        //         set_cookie(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, host, "/");
        //         return;
        //     }
        // }
        server_loaclvar &localvar = get_server_global_var();
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
            // Safe filtering allowed: A-Z a-z 0-9 _-
            unsigned char c = sessionfile[i];
            if ((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || c == '-')
            {
                sessionfile[j] = sessionfile[i];
                j++;
            }
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
        // sessionfile =
        //     client_ip + std::to_string(client_port) + std::to_string(rand_range(1000, 9999)) + std::to_string(timeid()) + std::to_string(rand_range(1000, 9999));
        // sessionfile = std::to_string(std::hash<std::string>{}(sessionfile));
        // cookie.set(COOKIE_SESSION_NAME, sessionfile, 7200, "/", host);
        // send_cookie.set(COOKIE_SESSION_NAME, sessionfile, 7200, "/", host);
        sessionfile.append(std::to_string(client_port));
        sessionfile.append(get_rand_session_id());
        set_cookie(COOKIE_SESSION_NAME, sessionfile, 30000, host, "/", false, true, session_samesite());
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
void httppeer::save_session_memory(const std::string &sessionfile)
{
    pzcache<obj_val> &temp_cache = pzcache<obj_val>::conn();
    temp_cache.save(sessionfile, session, 30000, true);
}
void httppeer::save_session_file(const std::string &sessionfile)
{

    std::string root_path, temp_session_file;
    // serverconfig &sysconfigpath = getserversysconfig();
    server_loaclvar &localvar = get_server_global_var();
    root_path                 = localvar.temp_path;

    // sessionfile.append("_sess");
    root_path.append(sessionfile);
    root_path.append("_sess");
#ifndef _MSC_VER
    int fd = open(root_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
    {
        // perror("open");
        return;
    }

#ifndef _WIN32
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
#else
    // lock file by using win32 api
    auto native_handle = (HANDLE)_get_osfhandle(fd);
    if (!LockFile(native_handle, 0, 0, 0, MAXDWORD))
    {
        return;
    }
#endif

    temp_session_file = session.to_json();

    ssize_t n = write(fd, temp_session_file.data(), temp_session_file.size());
    if (n > 0)
    {
        n = 0;
    }

#ifndef _WIN32
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {

        return;
    }
#else
    if (!UnlockFile(native_handle, 0, 0, 0, MAXDWORD))
    {
        return;
    }
#endif

    close(fd);
#endif
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
        if (sessionfile.size() > 0)
        {
            unsigned int j = 0;
            for (unsigned int i = 0; i < sessionfile.size(); i++)
            {
                // Safe filtering allowed: A-Z a-z 0-9 _-
                unsigned char c = sessionfile[i];
                if ((c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9') ||
                    c == '_' || c == '-')
                {
                    sessionfile[j] = sessionfile[i];
                    j++;
                }
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
            return;
        }
        if (localvar.session_type == 1)
        {
            pzcache<obj_val> &temp_cache = pzcache<obj_val>::conn();
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
                // cookie.set(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, "/", host);
                // send_cookie.set(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, "/", host);
                set_cookie(COOKIE_SESSION_NAME, sessionfile, timeid() - 7200, host, "/", false, true, session_samesite());
            }
        }
        else
        {
            return;
        }
    }
}
std::string httppeer::get_sitepath() { return getserversysconfig().getsitewwwpath(host_index); }
unsigned long long httppeer::get_siteid()
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (host_index >= sysconfigpath.sitehostinfos.size())
    {
        return 0;
    }
    return sysconfigpath.sitehostinfos[host_index].siteid;
}
unsigned long long httppeer::get_groupid()
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (host_index >= sysconfigpath.sitehostinfos.size())
    {
        return 0;
    }
    return sysconfigpath.sitehostinfos[host_index].groupid;
}
std::string httppeer::get_theme()
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (host_index >= sysconfigpath.sitehostinfos.size())
    {
        return "";
    }
    return sysconfigpath.sitehostinfos[host_index].themes;
}
std::string httppeer::get_themeurl()
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (host_index >= sysconfigpath.sitehostinfos.size())
    {
        return "";
    }
    return sysconfigpath.sitehostinfos[host_index].themes_url;
}
void httppeer::theme_view(const std::string &a)
{
    std::string view_name;
    serverconfig &sysconfigpath = getserversysconfig();
    if (host_index >= sysconfigpath.sitehostinfos.size())
    {
        view_name = a;
    }
    else
    {
        if (sysconfigpath.sitehostinfos[host_index].themes.size() > 0)
        {
            view_name = sysconfigpath.sitehostinfos[host_index].themes;
            view_name.push_back('/');
        }

        view_name.append(a);
    }

    struct view_param tempvp(get, post, cookie, session);
    if (!isso)
    {
        try
        {
            VIEW_REG &viewreg = get_viewmetholdreg();
            auto viter        = viewreg.find(view_name);
            if (viter != viewreg.end())
            {
                output.append(viter->second(tempvp, val));
                return;
            }
            output.append("Not Found View ");
            output.append(view_name);
        }
        catch (const std::exception &e)
        {
            output.append(std::string(e.what()));
            return;
        }
    }
    return;
}
bool httppeer::is_ssl()
{
    if (isssl)
    {
        return true;
    }
    else
    {
        return false;
    }
}
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
unsigned int httppeer::check_upload_limit()
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.sitehostinfos[host_index].is_limit_upload && content_length > sysconfigpath.sitehostinfos[host_index].upload_max_size)
    {
        DEBUG_LOG("upload file size limit %lld", content_length);
        return 403;
    }
    return 0;
}
void httppeer::cors_origin_process(std::string_view request_origin)
{
    // host 还没解析到时按协议分流：
    //   h1：HTTP/1 不保证头顺序，Origin 早于 Host 是合法的，此刻 host_index 仍是回落值，
    //        按它判定会拿错站点白名单，所以只挂标记，由 getheaderhost() 解出 Host 后补判。
    //   h2：RFC 7540 要求伪头排在普通头之前，走到这里要么客户端违序、要么整单没带
    //       :authority，两种情况路由都按回落的默认站点（index 0）走，CORS 跟着同一口径当场判定。
    if (host.empty() && httpv != 2)
    {
        cors_origin_pending = true;
        return;
    }
    cors_origin_allow(request_origin);
}
void httppeer::cors_origin_allow(std::string_view request_origin)
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (host_index >= sysconfigpath.sitehostinfos.size())
    {
        return;
    }
    auto &site_info          = sysconfigpath.sitehostinfos[host_index];
    std::string allow_origin = site_info.cors_allow_origin(request_origin);
    if (allow_origin.empty())
    {
        return;
    }
    // 解析期就把响应头设好：请求期不再做任何 CORS 判断，也没有临时变量要传
    set_header("Access-Control-Allow-Origin", allow_origin);
    // Expose-Headers 是实际响应头，告诉浏览器哪些响应头可被 JS 读取，
    // 只在普通跨域请求命中白名单时输出，预检响应里不带。
    // 取值来自站点配置 cors_expose_headers，不再无条件 "*"：
    // 通配会把服务端所有响应头（含内部用的）都暴露给页面 JS，未配置就不发这个头，
    // 业务确实需要时再配置或自己 set_header()。
    // 注：未配置 cors_domain 的站点是默认拒绝（allow_origin 为空，上面直接 return，一个 CORS 头都不发），
    //     哪些站点没配已在 loadserversconfig() 末尾点名告警一次，
    //     此处不再每请求 fprintf + fflush（那是同步写 stderr 的热路径）
    if (site_info.cors_expose_headers.size() > 0)
    {
        set_header("Access-Control-Expose-Headers", site_info.cors_expose_headers);
    }
    // 白名单命中时取值来自请求 Origin，必须声明 Vary；
    // '*' 是常量、不随请求变化，加了反而会让 CDN 把同一份缓存按 Origin 拆成多份
    if (allow_origin != "*")
    {
        add_vary("Origin");
        // 凭证与"回显具体 Origin"是绑定的：ACAO "*" 配 Allow-Credentials 会被浏览器硬拒，
        // 所以只在命中白名单这一支发，站点开关是 cors_credentials（默认关）
        if (site_info.cors_credentials)
        {
            set_header("Access-Control-Allow-Credentials", "true");
        }
    }
}

bool httppeer::find_host_index()
{
    serverconfig &sysconfigpath = getserversysconfig();
    auto hiter                  = sysconfigpath.host_toint.find(host);
    bool found                  = false;
    if (hiter != sysconfigpath.host_toint.end())
    {
        host_index = hiter->second;
        if (host_index >= sysconfigpath.sitehostinfos.size())
        {
            host_index = 0;
        }
        else
        {
            found = true;
        }
    }
    else
    {
        // 未命中：回落到 index 0（default 站点），避免 keep-alive 复用时沿用
        // 上一个请求的 host_index 导致站点配置（CORS 白名单、sitepath 等）串用
        host_index = 0;
    }
    // 本函数只负责定位 host_index。Origin 早于 Host 到达时 cors_origin_process() 会挂起，
    // 由 h1 的 getheaderhost() 在 host_index 确定后补判（h2 的 :authority 必在普通头之前，
    // 挂起分支带 httpv 守卫，h2 走的是当场判定）。
    return found;
}
// stat_is_regfile 已移到 httppeer.h inline 公共函数 — moved to httppeer.h

// html 缓存过期判定，界值只写这一份（prefetch_routing 的落盘检查和 get_fileinfo 共用）：
// 开关 usehtmlcache + TTL 下限 10 秒（防 0/1 秒把每个请求都打回动态链）。
// 两个值按站读：站点段没写就继承 [default]（serverconfig.cpp 的 tempinfo_default）。
static bool html_cache_expired(const struct stat &st, const site_host_info_t &site)
{
    return site.is_usehtmlcache && site.usehtmlcachetime > 10 &&
           site.usehtmlcachetime < (timeid() - (unsigned long)st.st_mtime);
}

// 路由预查：带点的 URL 先落盘查一次，再两段查表（原样名 → 去掉扩展名的同名）。
// 命中返回 handler 下标（>=0），loop 带着它直接进动态链，不必再把同一个 key 查第二遍；
// 未命中、或盘上那个文件还在有效期内（让位静态文件）都返回 -1。
// 命中时 pathinfos/urlpath 原地改成注册名，所以返回的下标和 peer->urlpath 始终对得上同一条注册。
int httppeer::prefetch_routing()
{
    // 每一次预查都先清掉上一请求留下的那份 stat（keep-alive 复用同一个 peer）
    prefetch_statfile.clear();
    // 与 prefetch_statfile 同节奏复位：route_exact_checked 只在下面「确实查过原样精确表并确认未命中」
    // 的分支才置 true；空 pathinfos / 先落盘命中 这两个提前返回的出口都保持 false，
    // 让 resolve_idx 仍走完整查找（那两条出口本来就没做过表查找）。
    route_exact_checked = false;
    // 空 pathinfos（urlpath 就是 "/"）让位磁盘：wwwpath 下有没有 index.html 由 get_fileinfo() 判，
    // 磁盘没有才由动态链尾巴按注册名 "home" 命中；这里提前命中等于把站点首页换成路由
    if (pathinfos.empty())
    {
        return -1;
    }

    // 第一道：带点的 URL 先看盘。文件还在有效期内就一次表都不查，直接让位 get_fileinfo() 发文件；
    // 已过期或盘上没有（含目录）才继续查表，命中的注册名走动态链重生成。
    // sitepath 为空（连 [default] 都没写静态根）时无盘可查；跑分构建（-DENABLE_BENCHMARK=ON）整段不进，
    // 不背这次 stat：注册路由直接赢。
#ifndef BENCHMARK
    serverconfig &sysconfigpath = getserversysconfig();
    site_host_info_t *siteinfo =
        host_index < sysconfigpath.sitehostinfos.size() ? &sysconfigpath.sitehostinfos[host_index] : nullptr;
    if (isfile && siteinfo != nullptr && siteinfo->is_usehtmlcache && !sitepath.empty())
    {
        std::string diskfile = sitepath;
        if (diskfile.back() != '/')
        {
            diskfile.push_back('/');
        }
        // urlpath 就是 '/' + join(pathinfos,"/")，所以去头斜杠接在 sitepath 后面 == sendfilename
        diskfile.append(urlpath, 1, std::string::npos);

        memset(&fileinfo, 0, sizeof(fileinfo));
        if (stat(diskfile.c_str(), &fileinfo) == 0 && (fileinfo.st_mode & S_IFREG) &&
            !html_cache_expired(fileinfo, *siteinfo))
        {
            prefetch_statfile = diskfile;
            return -1;
        }
    }
#endif

    int idx = get_router().lookup_key_in_slots(urlpath, host_index);
    // 置位前已经做过「原样精确」查表：lookup_key_in_slots 内部就是 本域名 slot → 全局 slot 0 的两级回落
    // （与 resolve_idx 里被跳过的 lookup_exact 完全等价），返回 -1 即"两级都没命中"这一条布尔事实，
    // 不需要区分落在哪一级——resolve_idx 据此跳过一次重复的两级精确查即可；命中（idx>=0）由 known_idx
    // 早退接管，不会走到跳过分支。正确性依赖两者始终等价，若要给 lookup_exact 加第三级 slot 需同步处理。
    route_exact_checked = true;
    if (idx >= 0)
    {
        return idx;
    }
    if (!isfile)
    {
        return -1;
    }

    // 第二道：把最后一段的扩展名去掉再查一次（/about.html ↔ 注册名 /about）
    auto &last_seg = pathinfos.back();
    auto dot       = last_seg.find('.');
    if (dot == std::string::npos || dot == 0)
    {
        return -1;
    }
    std::string stripped = last_seg.substr(0, dot);
    std::string orig     = last_seg;
    auto apply_name      = [&last_seg, this](const std::string &name)
    {
        last_seg   = name;
        auto slash = urlpath.rfind('/');
        if (slash != std::string::npos)
        {
            urlpath.resize(slash + 1);
            urlpath.append(name);
        }
    };
    apply_name(stripped);
    idx = get_router().lookup_key_in_slots(urlpath, host_index);
    if (idx < 0)
    {
        // 没命中，恢复带点的原名，磁盘那条路照旧
        apply_name(orig);
    }
    return idx;
}

unsigned char httppeer::get_fileinfo()
{
    serverconfig &sysconfigpath = getserversysconfig();
    // 越界守卫：host_index 来自 find_host_index()，正常永远 < size；但 conf 一个站点段都没有
    // （sitehostinfos 为空）时它回落的 0 也是越界。先判一下，越界直接报"非文件"，让请求走
    // 动态链 / 404，而不是解引用一个不存在的 vector 元素（下面的 977 行对 sitehostinfos[0]
    // 的访问另有 !empty() 守卫，这里不重复）。
    if (host_index >= sysconfigpath.sitehostinfos.size())
        return 0;
    auto &hostinfo = sysconfigpath.sitehostinfos[host_index];

    //host 段没写 wwwpath 时 sitepath 是空的，跟着 default 站走；
    //留空会拼出 "/xxx" 这种绝对路径，直接 stat 到文件系统根上
    if (sitepath.empty() && !sysconfigpath.sitehostinfos.empty())
    {
        sitepath = sysconfigpath.sitehostinfos[0].wwwpath;
    }
    //default 段也没有静态根：这条 URL 不在磁盘上判，交回动态链（home → 404 → 框架 404）
    if (sitepath.empty())
    {
        sendfiletype = 0;
        return sendfiletype;
    }

    //full static file path: sitepath + url paths
    sendfilename = sitepath;
    if (sendfilename.size() == 0 || sendfilename.back() != '/')
    {
        sendfilename.push_back('/');
    }
    // urlpath 本身就是两个 parser 用 '/' + join(pathinfos, "/") 拼出来的那一条
    // （http_parse.cpp:585-589、http2_parse.cpp:465-469），逐段再拼一遍是同一件事做第二遍；
    // prefetch 落盘那次也是这么接的，两处路径都由同一个来源拼出来，不会各拼各的
    // pathinfos 为空时不接：HPACK 静态表 index 4/5 那条捷径（http2_parse.cpp:1850-1856）只给
    // urlpath 赋值、不跑 path_process，此时 urlpath 可以是 "/index.html" 而段表是空的，
    // 接上去就绕过了"目录 + document_index"那个分支 —— 保持与逐段循环相同的边界
    if (!pathinfos.empty() && urlpath.size() > 1)
    {
        sendfilename.append(urlpath, 1, std::string::npos);
    }

    // 预查「先落盘」那次已经 stat 过同一个文件，fileinfo 就是它的结果；那次已经用同一个
    // html_cache_expired 判过没过期，所以这里直接当普通文件成立，不必 stat 也不必重判第二遍
    bool already_stat = !prefetch_statfile.empty() && prefetch_statfile == sendfilename;
    if (!already_stat)
    {
        memset(&fileinfo, 0, sizeof(fileinfo));
    }
    sendfiletype = already_stat ? 1 : 0;
    if (!already_stat && stat(sendfilename.c_str(), &fileinfo) == 0)
    {
        if (fileinfo.st_mode & S_IFDIR)
        {
            //directory, try default index file
            sendfiletype             = 2;
            unsigned int nowpathsize = sendfilename.size();
            if (sendfilename.size() > 0 && sendfilename.back() != '/')
            {
                sendfilename.push_back('/');
            }
            sendfilename.append(hostinfo.document_index.empty() ? "index.html" : hostinfo.document_index);
            memset(&fileinfo, 0, sizeof(fileinfo));
            if (stat(sendfilename.c_str(), &fileinfo) == 0)
            {
                if (fileinfo.st_mode & S_IFREG)
                {
                    sendfiletype = 1;
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
            // 段数门槛 >3 是这条「路由注册在父路径、生成文件在深路径」形状的专属门禁（剥名那组
            // 同名对已由 prefetch_routing 的同一份过期判定处理）；界值见 html_cache_expired。
            // 扩展名只认小写 ".html"：这条机制管的就是生成的 html 缓存页，深路径放的其他文件
            // （jpg/css/js/字体…）没有注册名能接，判过期等于把盘上有、动态没人接的文件打成 404。
            // 大小写不同（.HTML）算没命中，往"发文件"那侧回落，方向是安全的。
            // pathinfos.size() > 3 必须先判：段表为空时 back() 是越界读，size 判断要放在最前
            if (pathinfos.size() > 3 && pathinfos.back().size() > 5 &&
                pathinfos.back().compare(pathinfos.back().size() - 5, 5, ".html") == 0 &&
                html_cache_expired(fileinfo, hostinfo))
            {
                //cache html expired, use urlpath function
                sendfiletype = 3;
            }
        }
    }
    if (sendfiletype == 0 && hostinfo.isrewrite)
    {
        if (hostinfo.rewrite404 == 1)
        {
            //static 404 rewrite, check multi 404 rewrite
            if (hostinfo.action_404_lists.size() > 0 && pathinfos.size() > 0)
            {
                for (unsigned int k = 0; k < hostinfo.action_404_lists.size(); k++)
                {
                    if (pathinfos[0].size() <= hostinfo.action_404_lists[k].size() && str_cmp_pre(hostinfo.action_404_lists[k], pathinfos[0], pathinfos[0].size()))
                    {
                        sendfilename = sitepath;
                        sendfilename.append(hostinfo.action_404_lists[k]);
                        memset(&fileinfo, 0, sizeof(fileinfo));
                        if (stat(sendfilename.c_str(), &fileinfo) == 0)
                        {
                            if (fileinfo.st_mode & S_IFREG)
                            {
                                sendfiletype = 1;
                                return sendfiletype;
                            }
                        }
                    }
                }
            }
            sendfilename = sitepath;
            if (hostinfo.rewrite_404_action.size() > 0)
            {
                sendfilename.append(hostinfo.rewrite_404_action);
                memset(&fileinfo, 0, sizeof(fileinfo));
                if (stat(sendfilename.c_str(), &fileinfo) == 0)
                {
                    if (fileinfo.st_mode & S_IFREG)
                    {
                        sendfiletype = 1;
                    }
                }
            }
        }
        else if (hostinfo.rewrite404 == 2)
        {
            //dynamic method, use urlpath function
            if (hostinfo.action_404_lists.size() > 0 && pathinfos.size() > 0)
            {
                for (unsigned int k = 0; k < hostinfo.action_404_lists.size(); k++)
                {
                    if (pathinfos[0].size() > 2 && hostinfo.action_404_lists[k].size() > 2 && str_cmp_pre(hostinfo.action_404_lists[k], pathinfos[0], 3))
                    {
                        if (pathinfos.size() < 20)
                        {
                            pathinfos.insert(pathinfos.begin(), hostinfo.action_404_lists[k]);
                            sendfiletype = 3;
                            return sendfiletype;
                        }
                    }
                }
            }

            if (hostinfo.rewrite_404_action.size() > 0)
            {
                if (pathinfos.size() < 20)
                {
                    pathinfos.insert(pathinfos.begin(), hostinfo.rewrite_404_action);
                    sendfiletype = 3;
                }
            }
        }
    }
    return sendfiletype;
}
std::shared_ptr<httppeer> httppeer::get_ptr() { return shared_from_this(); }
void httppeer::send_files(std::string filename)
{
    if (filename.size() > 0)
    {
    }
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

// 追加式写 Vary：Vary 一行里是多个字段名，覆盖式 set_header 会让 CORS 声明的 Origin 与
// 业务/压缩声明的字段互相吃掉。这里保留已有文本，只把缺的字段补到行尾；
// 字段名按 RFC 9110 大小写不敏感，所以比对用小写，写出沿用传入的原样拼写
void httppeer::add_vary(std::string_view values)
{
    // 读回响应里已有的 Vary：h2 命中 HPACK 静态表时它落在索引槽，不在 send_header
    std::string merged;
    if (httpv == 2)
    {
        auto slot = http2_send_header.find(HTTP2_CODE_vary);
        if (slot != http2_send_header.end())
        {
            merged = slot->second;
        }
    }
    else
    {
        auto iter = send_header.find("Vary");
        if (iter != send_header.end())
        {
            merged = iter->second;
        }
    }

    std::vector<std::string> have;
    auto collect = [&have](std::string_view list)
    {
        std::size_t pos = 0;
        while (pos < list.size())
        {
            std::size_t comma = list.find(',', pos);
            if (comma == std::string_view::npos)
            {
                comma = list.size();
            }
            std::string_view item = list.substr(pos, comma - pos);
            while (!item.empty() && (item.front() == ' ' || item.front() == '\t'))
            {
                item.remove_prefix(1);
            }
            while (!item.empty() && (item.back() == ' ' || item.back() == '\t'))
            {
                item.remove_suffix(1);
            }
            if (!item.empty())
            {
                std::string key(item);
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                have.emplace_back(std::move(key));
            }
            pos = comma + 1;
        }
    };
    collect(merged);

    std::size_t pos = 0;
    while (pos < values.size())
    {
        std::size_t comma = values.find(',', pos);
        if (comma == std::string_view::npos)
        {
            comma = values.size();
        }
        std::string_view item = values.substr(pos, comma - pos);
        while (!item.empty() && (item.front() == ' ' || item.front() == '\t'))
        {
            item.remove_prefix(1);
        }
        while (!item.empty() && (item.back() == ' ' || item.back() == '\t'))
        {
            item.remove_suffix(1);
        }
        if (!item.empty())
        {
            std::string key(item);
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            if (std::find(have.begin(), have.end(), key) == have.end())
            {
                if (!merged.empty())
                {
                    merged.append(", ");
                }
                merged.append(item);
                have.emplace_back(std::move(key));
            }
        }
        pos = comma + 1;
    }

    if (!merged.empty())
    {
        set_header("Vary", merged);
    }
}

std::string httppeer::get_header(std::string_view key_name)
{
    std::string key;
    key.resize(key_name.size());
    std::transform(key_name.begin(), key_name.end(), key.begin(), ::tolower);

    auto iter = header.find(key);
    if (iter != header.end())
    {
        return iter->second;
    }
    return "";
}

void httppeer::set_cookie(std::string key,
                          std::string value,
                          long long exptime,
                          std::string domain,
                          std::string path,
                          bool secure,
                          bool httponly,
                          std::string issamesite)
{
    // SameSite=None 必须与 Secure 成对出现：只带 None 的 Set-Cookie 会被浏览器直接丢掉，
    // 所以这里替调用方补上 Secure（属性表也存补过的值，别出现两处属性不一致）
    if (!issamesite.empty() && (issamesite[0] == 'N' || issamesite[0] == 'n'))
    {
        secure = true;
    }
    cookie.set(key, value, exptime, domain, path, secure, httponly, issamesite);
    //send_cookie.set(key, val, exptime, domain, path, secure, httponly, issamesite);

    std::string temph;
    temph.append(url_encode(key.data(), key.size()));
    temph.push_back('=');
    temph.append(url_encode(value.data(), value.size()));

    if (exptime > 0 && exptime < 63072000)
    {
        exptime = timeid() + exptime;
    }

    if (exptime > 0)
    {
        temph.append("; Expires=");
        temph.append(get_gmttime((unsigned long long)exptime));
    }

    if (domain.size() > 1)
    {
        temph.append("; Domain=");
        temph.append(domain);
    }

    if (path.size() > 0)
    {
        temph.append("; Path=");
        temph.append(path);
    }

    if (secure)
    {
        temph.append("; Secure");
    }

    if (httponly)
    {
        temph.append("; HttpOnly");
    }
    if (!issamesite.empty())
    {
        switch (issamesite[0])
        {
        case 'L':
        case 'l': temph.append("; SameSite=Lax"); break;
        case 'S':
        case 's': temph.append("; SameSite=Strict"); break;
        case 'N':
        case 'n': temph.append("; SameSite=None"); break;
        }
    }

    send_cookie_lists.emplace_back(temph);
}

// 会话 cookie 的 SameSite 取值：站点开了 cors_credentials（准备接带凭证的跨域请求）就必须给 None，
// 否则浏览器按缺省的 Lax 处理，跨站 XHR 压根不会带这个 cookie，ACAO 与 Allow-Credentials 说得再对也没用。
// 只在这一支给 None，且要求当前是 HTTPS：明文收到的 Secure cookie 浏览器会直接丢弃，
// 本地 http 开发环境的登录态会被这个改动弄坏。没开凭证的站点维持现状（不发 SameSite 属性）
std::string httppeer::session_samesite()
{
    if (!is_ssl())
    {
        return "";
    }
    serverconfig &sysconfigpath = getserversysconfig();
    if (host_index < sysconfigpath.sitehostinfos.size() && sysconfigpath.sitehostinfos[host_index].cors_credentials)
    {
        return "None";
    }
    return "";
}

std::list<std::string> httppeer::cookietoheader()
{
    // std::string temph;
    // for (auto &iter : send_cookie)
    // {
    //     temph.clear();
    //     std::string key        = iter.first;
    //     std::string domain     = send_cookie.getDomain(key);
    //     std::string domainpath = send_cookie.getPath(key);
    //     unsigned long timeexp  = send_cookie.getExpires(key);
    //     unsigned char issecure = send_cookie.getSecure(key);
    //     unsigned char isonly   = send_cookie.getHttponly(key);

    //     std::string samesite = send_cookie.getSamesite(key);

    //     temph.append(url_encode(iter.first.data(), iter.first.size()));
    //     temph.push_back('=');
    //     temph.append(url_encode(iter.second.data(), iter.second.size()));

    //     if (timeexp > 0 && timeexp < 63072000)
    //     {
    //         timeexp = timeid() + timeexp;
    //     }

    //     key.clear();
    //     if (timeexp > 0)
    //     {
    //         key = get_gmttime((unsigned long long)timeexp);
    //         temph.append("; Expires=");
    //         temph.append(key);
    //     }

    //     if (domain.size() > 1)
    //     {
    //         temph.append("; Domain=");
    //         temph.append(domain);
    //     }

    //     if (domainpath.size() > 0)
    //     {
    //         temph.append("; Path=");
    //         temph.append(domainpath);
    //     }

    //     if (issecure > 0)
    //     {
    //         temph.append("; Secure");
    //     }

    //     if (isonly > 0)
    //     {
    //         temph.append("; HttpOnly");
    //     }
    //     if (!samesite.empty())
    //     {
    //         switch (samesite[0])
    //         {
    //         case 'L':
    //         case 'l': temph.append("; SameSite=Lax"); break;
    //         case 'S':
    //         case 's': temph.append("; SameSite=Strict"); break;
    //         }
    //     }

    //     send_cookie_lists.emplace_back(temph);
    // }
    return send_cookie_lists;
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
        // 合并 Vary：压缩固定声明 Accept-Encoding，若 send_header 里还有 Vary: Origin
        // 则合并为一行，避免同一响应出现两个 Vary 头
        std::string vary_val = "Accept-Encoding";
        auto vary_iter       = send_header.find("Vary");
        if (vary_iter != send_header.end())
        {
            vary_val.append(", ");
            vary_val.append(vary_iter->second);
            send_header.erase(vary_iter);
        }
        http1header.append("Vary: ");
        http1header.append(vary_val);
        http1header.append("\r\n");
    }
    else if (compress == 2)
    {
        http1header.append("Content-Encoding: ");
        http1header.append("br");
        http1header.append("\r\n");
        std::string vary_val = "Accept-Encoding";
        auto vary_iter       = send_header.find("Vary");
        if (vary_iter != send_header.end())
        {
            vary_val.append(", ");
            vary_val.append(vary_iter->second);
            send_header.erase(vary_iter);
        }
        http1header.append("Vary: ");
        http1header.append(vary_val);
        http1header.append("\r\n");
    }
    if (content_type.size() > 0)
    {
        http1header.append("Content-Type: ");
        http1header.append(content_type);
        http1header.append("\r\n");
    }

    if (keepalive)
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
    //std::list<std::string> cookielist = cookietoheader();

    if (send_cookie_lists.size() > 0)
    {
        for (auto iter = send_cookie_lists.begin(); iter != send_cookie_lists.end(); iter++)
        {
            http1header.append("Set-Cookie: ");
            http1header.append(*iter);
            http1header.append("\r\n");
        }
    }
    http1header.append("Server: Paozhu\r\n");
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
    make_http2_headers_item3(http2header, "server", "Paozhu");
    if (send_header.size() > 0)
    {
        for (auto [first, second] : send_header)
        {
            make_http2_headers_item3(http2header, first, second);
        }
    }
    //std::list<std::string> cookielist = cookietoheader();
    if (send_cookie_lists.size() > 0)
    {
        for (auto iter = send_cookie_lists.begin(); iter != send_cookie_lists.end(); iter++)
        {
            make_http2_headers_item3(http2header, HTTP2_CODE_set_cookie, *iter);
            //cookielist.pop_front();
        }
    }

    set_http2_headers_size(http2header, (unsigned int)http2header.size() - 9);
    DEBUG_LOG("make_http2_header end");
    return http2header;
}
void httppeer::goto_url(const std::string &gourl, unsigned char second, const std::string &msg)
{
    output.append("<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"");
    output.append(std::to_string(second));
    output.append(";url=");
    output.append(gourl);
    output.append("\"></head><body>");
    if (msg.size() > 0)
    {
        output.append("<div style=\"margin-top:20px;margin-left: auto;margin-right: auto;with:60%\"><h3>");
        output.append(msg);
        output.append("</h3></div>");
    }
    output.append("</body></html>");
}

httppeer &httppeer::operator<<(obj_val &a)
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
template <typename T>
httppeer &httppeer::operator<<(VALNUM_T auto a)
{
    output.append(std::to_string(a));
    return *this;
}

template <typename T>
httppeer &httppeer::operator<<(T a)
{
    output.append(a.to_string());
    return *this;
}
template <typename T>
httppeer &httppeer::operator<<(T *a)
{
    output.append(a->to_string());
    return *this;
}

httppeer &httppeer::get_peer() { return *this; }
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
    struct view_param tempvp(get, post, cookie, session);
    if (!isso)
    {
        try
        {
            VIEW_REG &viewreg = get_viewmetholdreg();
            auto viter        = viewreg.find(a);
            if (viter != viewreg.end())
            {
                output.append(viter->second(tempvp, val));
                return;
            }
            output.append("Not Found View ");
            output.append(a);
        }
        catch (const std::exception &e)
        {
            output.append(std::string(e.what()));
            return;
        }
    }
#ifdef ENABLE_BOOST
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
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
#endif
    return;
}
void httppeer::view(const std::string &a, obj_val &b)
{
    struct view_param tempvp(get, post, cookie, session);
    if (!isso)
    {
        try
        {
            VIEW_REG &viewreg = get_viewmetholdreg();
            auto viter        = viewreg.find(a);
            if (viter != viewreg.end())
            {
                output.append(viter->second(tempvp, b));
                return;
            }
            output.append("Not Found View ");
            output.append(a);
        }
        catch (const std::exception &e)
        {
            output.append(std::string(e.what()));
            return;
        }
    }
#ifdef ENABLE_BOOST
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
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
#endif
    return;
}
std::string httppeer::fetchview(const std::string &a)
{
    struct view_param tempvp(get, post, cookie, session);
    if (!isso)
    {
        try
        {
            VIEW_REG &viewreg = get_viewmetholdreg();
            auto viter        = viewreg.find(a);
            if (viter != viewreg.end())
            {
                return viter->second(tempvp, val);
            }
            output.append("Not Found View ");
            output.append(a);
        }
        catch (const std::exception &e)
        {
            output.append(std::string(e.what()));
            return "";
        }
    }
#ifdef ENABLE_BOOST
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
    return "";
}
std::string httppeer::fetchview(const std::string &a, obj_val &b)
{
    struct view_param tempvp(get, post, cookie, session);
    if (!isso)
    {
        try
        {
            VIEW_REG &viewreg = get_viewmetholdreg();
            auto viter        = viewreg.find(a);
            if (viter != viewreg.end())
            {
                return viter->second(tempvp, b);
            }
            output.append("Not Found View ");
            output.append(a);
        }
        catch (const std::exception &e)
        {
            output.append(std::string(e.what()));
            return "";
        }
    }
#ifdef ENABLE_BOOST
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
    return "";
}
void httppeer::out_json(obj_val &a)
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
    // 三个响应头统一交给 set_header() 写：h1 原样落 send_header；h2 命中 HPACK 静态表的
    // （ACAO 落索引槽 20、Vary 落索引槽 59）写 http2_send_header，其余落 send_header。
    // 自己拼容器名就会绕过这张表：send_header["vary"] 与解析期 cors_origin_process()
    // 经 set_header("Vary") 写的索引槽 59 各算一处 —— 响应里会出现两条 Vary。
    set_header("Access-Control-Allow-Origin", name);
    // header_v 为空按放行全部请求头处理，默认发 "*"
    set_header("Access-Control-Allow-Headers", header_v.size() > 0 ? header_v : "*");
    // ACAO 取值依赖请求 Origin，必须声明 Vary，否则会被缓存/CDN 串用
    add_vary("Origin");
}
void httppeer::cors_method(const std::string &header_v)
{
    // 预检响应头：Allow-Methods 按站点配置，Max-Age 缓存预检结果。
    // Expose-Headers 属实际响应头，不在预检里输出，改由普通请求的 cors_origin_process() 设置。
    // Allow-Methods 的值取自本站点的 cors_allow_methods，与 send_cors_domain() 校验
    // Access-Control-Request-Method 用的是同一份列表，改配置两处一起变
    std::string allow_methods;
    serverconfig &sysconfigpath = getserversysconfig();
    if (host_index < sysconfigpath.sitehostinfos.size())
    {
        for (auto &method_item : sysconfigpath.sitehostinfos[host_index].cors_allow_methods)
        {
            if (!allow_methods.empty())
            {
                allow_methods.append(", ");
            }
            allow_methods.append(method_item);
        }
    }
    // header_v 是业务要放行的请求头列表（apicrudtest 里写的是 "origin, x-requested-with"）：
    // 预检响应必须回 Allow-Headers 浏览器才会放行，之前这个入参被实现丢在一边，
    // 控制器里那几处 cors_method("origin, x-requested-with") 等于没生效
    if (header_v.size() > 0)
    {
        set_header("Access-Control-Allow-Headers", header_v);
    }
    // 站点把列表配空＝一个方法都不放行，这时发一条空的 Allow-Methods 没有意义，
    // 预检也会在 send_cors_domain() 用同一份名单校验时被判失败
    if (!allow_methods.empty())
    {
        if (httpv == 2)
        {
            send_header["access-control-max-age"]       = "86400";
            send_header["access-control-allow-methods"] = allow_methods;
        }
        else
        {
            send_header["Access-Control-Max-Age"]       = "86400";
            send_header["Access-Control-Allow-Methods"] = allow_methods;
        }
    }
}
void httppeer::push_flow(const std::string &m_name)
{
    if (!flow_method)
    {
        flow_method = std::make_unique<std::list<std::string>>();
    }
    flow_method->push_back(m_name);
}
void httppeer::push_front_flow(const std::string &m_name)
{
    if (!flow_method)
    {
        flow_method = std::make_unique<std::list<std::string>>();
    }
    flow_method->push_front(m_name);
}
std::string httppeer::pop_flow()
{
    if (!flow_method)
    {
        return "";
    }
    if (flow_method->empty())
    {
        return "";
    }
    std::string tempmethod = flow_method->back();
    flow_method->pop_back();
    return tempmethod;
}
void httppeer::clsoesend()
{
    try
    {
        std::unique_lock<std::mutex> lock(pop_user_handleer_mutex);
        if (user_code_handler_call.size() > 0)
        {
            asio::dispatch(socket_session->strand_,
                           [handler = std::move(user_code_handler_call.front())]() mutable -> void
                           {
                               /////////////
                               handler(0);
                               //////////
                           });
            user_code_handler_call.pop_front();
            DEBUG_LOG("httppeer user_code_handler_call return");
        }
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("httppeer user_code_handler_call error");
    }
}

void httppeer::clear()
{
    host.clear();
    ;
    url.clear();
    ;
    urlpath.clear();
    ;
    querystring.clear();
    ;

    content_type.clear();
    ;
    etag.clear();
    ;

    chartset.clear();
    rawcontent.clear();
    rawcontent.shrink_to_fit();
    header.clear();
    get.clear();
    post.clear();
    files.clear();
    json.clear();
    val.clear();
    cookie.clear();
    pathinfos.clear();

    issendheader = false;
    ischunked    = false;
    isfinish     = false;
    issend       = false;
    isclose      = false;
    keepalive    = true;
    isso         = false;
    iscors       = false;
    // 必须清：挂起标记只有 h1 的 getheaderhost() 会消费，头块中途报错弃单时走不到那里，
    // 残留到下一个请求会把上一个 Origin 的白名单判定结果串到新请求上
    cors_origin_pending = false;
    // host_index 同步复位：未配置域名的请求 find_host_index() 会回落到 0，
    // 这里再兜底一次，防止 keep-alive 复用上一请求的站点配置
    host_index = 0;

    posttype = 0;
    compress = 0;

    stream_id   = 0;
    status_code = 0;
    // 挂着间隔任务（timeloop_num>0）的 peer，这三个调度字段不属于"本次请求的状态"：
    // keep-alive 的每一次请求结束都会调这里，无条件清零等于把业务刚登记的间隔任务抹掉，
    // 下一秒的 tick 就把这条任务从 clientlooptasks 里 erase——登记和执行两头都还在，任务本身留不下来。
    // 摘掉任务由业务自己调 clear_timeloop_task()（或跑够拍数后 timecount_num 归 0）来做。
    if (timeloop_num == 0)
    {
        timecount_num = 0;
    }
    request_time = 0;
    //unsigned int time_limit             = 0;
    content_length   = 0;
    sessionfile_time = 0;
    upload_length    = 0;

    state.gzip              = false;
    state.deflate           = false;
    state.br                = false;
    state.avif              = false;
    state.webp              = false;
    state.zstd              = false;
    state.keepalive         = false;
    state.websocket         = false;
    state.upgradeconnection = false;
    state.rangebytes        = false;
    state.range_suffix      = false;
    state.range_has_end     = false;
    state.language[0]       = {0};
    state.version           = 0;
    state.port              = 0;
    state.ifmodifiedsince   = 0;
    state.rangebegin        = 0;
    state.rangeend          = 0;

    session.clear();

    //cookie send_cookie;
    send_cookie_lists.clear();
    send_header.clear();
    http2_send_header.clear();

    output.clear();
    output.shrink_to_fit();
    sitepath.clear();
    sendfilename.clear();

    sendfiletype = 0;
    if (timeloop_num == 0)
    {
        // 投递侧按 linktype==7 认领这条任务，清了它就变成"表里有任务、池里没人跑"。
        linktype = 0;
    }
    method = 0;
    httpv  = 0;
    // 两个 parser 都只在「段表非空」时给 isfile 赋值（http_parse.cpp:603、http2_parse.cpp:486），
    // 所以空段表请求（"/"）会沿用上一请求的取值。keep-alive 复用同一个 peer 时，
    // 这条链是 "/" → prefetch_routing 在 pathinfos.empty() 处提前 return，恰好读不到它才没出事。
    // 在这里复位，把「靠调用顺序才正确」换成「字段本身可信」：段表为空就是没有末段，
    // 也就不是文件型 URL，false 是它该有的值。
    isfile = false;

    // 路由预查留下的两份请求态跟着复位。prefetch_routing 入口也清一次，这里是第二道：
    // 一个「不进预查也能被读到」的字段，可信度不该建立在调用顺序上（同上面 isfile 那条理由）。
    prefetch_statfile.clear();
    route_exact_checked = false;

    user_code_handler_call.clear();
    flow_method.reset();
}

range_parse_t parse_range_header(std::string_view header_value, headstate_t &state)
{
    // 按 RFC 7233 解析首个 range 体（多段 range 只取首段）。
    // 语法不合法时清掉 rangebytes：h1 侧调用点会把它转成 400，h2 侧没有单条头
    // 错误通道，只能按「忽略该头」处理，留着 rangebytes 会发出没有 Content-Range 的 206。
    unsigned int j        = 0;
    unsigned int linesize = header_value.size();
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
    if (!str_casecmp(buffer_value, "bytes"))
    {
        return range_parse_t::not_bytes;// 非 bytes 单位，忽略该头
    }

    unsigned int digits = 0;
    auto parse_number   = [&](unsigned long long &out) -> bool
    {
        bool any = false;
        for (; j < linesize; j++)
        {
            if (header_value[j] < 0x30 || header_value[j] > 0x39)
            {
                break;
            }
            if ((++digits) > 14)
            {
                return false;
            }
            out = out * 10 + static_cast<unsigned long long>(header_value[j] - 0x30);
            any = true;
        }
        return any;
    };

    // 单元循环只吞掉了 '=' 之前的空白；'=' 之后与 '-' 两侧同样是 OWS，一并容忍，
    // 否则 "bytes = 0-999" 会从「忽略该头」变成「语法不合法」，反而把 h1 打回 400。
    auto skip_ows = [&]()
    {
        while (j < linesize && header_value[j] == 0x20)
        {
            j++;
        }
    };

    state.rangebytes = true;
    skip_ows();
    if (j < linesize && header_value[j] == 0x2D)
    {
        // "bytes=-N"：取末尾 N 字节，N 暂存 rangeend，由 server 侧换算成绝对范围
        j++;
        skip_ows();
        if (!parse_number(state.rangeend))
        {
            state.rangebytes = false;
            return range_parse_t::syntax_error;
        }
        state.range_suffix  = true;
        state.range_has_end = true;
        return range_parse_t::applied;
    }

    if (!parse_number(state.rangebegin))
    {
        state.rangebytes = false;
        return range_parse_t::syntax_error;
    }
    skip_ows();
    if (j < linesize && header_value[j] == 0x2D)
    {
        j++;
        skip_ows();
        if (j < linesize && header_value[j] >= 0x30 && header_value[j] <= 0x39)
        {
            if (!parse_number(state.rangeend))
            {
                state.rangebytes = false;
                return range_parse_t::syntax_error;
            }
            state.range_has_end = true;
        }
    }
    return range_parse_t::applied;
}

std::string httppeer::make_http2_data(unsigned int sid, std::string_view payload, bool is_end)
{
    std::string frame(9, '\0');

    // payload length (3 bytes, big-endian)
    unsigned int len = payload.size();
    frame[2]         = len & 0xFF;
    len >>= 8;
    frame[1] = len & 0xFF;
    len >>= 8;
    frame[0] = len & 0xFF;

    // frame type: 0x00 = DATA
    frame[3] = 0x00;

    // flags: 0x01 = END_STREAM
    frame[4] = is_end ? 0x01 : 0x00;

    // stream_id (4 bytes, big-endian)
    unsigned int sid_temp = sid;
    frame[8]              = sid_temp & 0xFF;
    sid_temp >>= 8;
    frame[7] = sid_temp & 0xFF;
    sid_temp >>= 8;
    frame[6] = sid_temp & 0xFF;
    sid_temp >>= 8;
    frame[5] = sid_temp & 0xFF;

    frame.append(payload);
    return frame;
}

// ---------------- 常规发送（Content-Length） ----------------
void httppeer::send_make_header()
{
    status(200);
    if (!isset_type())
    {
        type("application/octet-stream");
    }
    set_header("Cache-Control", "no-cache");
    set_header("Date", get_gmttime());

    std::string header_str;
    if (httpv == 2)
    {
        header_str = make_http2_header(0);
    }
    else
    {
        header_str = make_http1_header();
        header_str.append("\r\n");
    }

    if (socket_session)
    {
        socket_session->send_writer(header_str);
    }
}

void httppeer::send_make_body(std::string_view data)
{
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, data, false);
        if (socket_session)
        {
            socket_session->send_writer(frame);
        }
    }
    else
    {
        if (socket_session)
        {
            socket_session->send_writer(data);
        }
    }
}

void httppeer::send_make_end()
{
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, "", true);
        if (socket_session)
        {
            socket_session->send_writer(frame);
        }
    }
    // HTTP/1.1: Content-Length 模式下，浏览器按长度读取完毕自动结束，无需额外操作
    // 发完后置 ischunked = true，让 server.cpp 那边直接返回
    ischunked = true;
}

asio::awaitable<void> httppeer::async_send_make_header()
{
    status(200);
    if (!isset_type())
    {
        type("application/octet-stream");
    }
    set_header("Cache-Control", "no-cache");
    set_header("Date", get_gmttime());

    std::string header_str;
    if (httpv == 2)
    {
        header_str = make_http2_header(0);
    }
    else
    {
        header_str = make_http1_header();
        header_str.append("\r\n");
    }

    if (socket_session)
    {
        co_await socket_session->async_send_writer(header_str);
    }
    co_return;
}

asio::awaitable<void> httppeer::async_send_make_body(std::string_view data)
{
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, data, false);
        if (socket_session)
        {
            co_await socket_session->async_send_writer(frame);
        }
    }
    else
    {
        if (socket_session)
        {
            co_await socket_session->async_send_writer(data);
        }
    }
    co_return;
}

asio::awaitable<void> httppeer::async_send_make_end()
{
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, "", true);
        if (socket_session)
        {
            co_await socket_session->async_send_writer(frame);
        }
    }
    ischunked = true;
    co_return;
}

// ---------------- 流式发送（chunked transfer） ----------------
void httppeer::send_chunk_header()
{
    status(200);
    if (!isset_type())
    {
        type("application/octet-stream");
    }
    set_header("Cache-Control", "no-cache");
    set_header("Date", get_gmttime());
    ischunked = true;

    std::string header_str;
    if (httpv == 2)
    {
        header_str = make_http2_header(0);
    }
    else
    {
        header_str = make_http1_header();
        header_str.append("\r\n");
    }

    if (socket_session)
    {
        socket_session->send_writer(header_str);
    }
    ischunked = false;
}

void httppeer::send_chunk_body(std::string_view data)
{
    if (data.empty())
    {
        return;
    }
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, data, false);
        if (socket_session)
        {
            socket_session->send_writer(frame);
        }
    }
    else
    {
        std::string chunk;
        chunk.reserve(data.size() + 16);
        std::ostringstream ss;
        ss << std::hex << data.size();
        chunk.append(ss.str());
        chunk.append("\r\n");
        chunk.append(data);
        chunk.append("\r\n");

        if (socket_session)
        {
            socket_session->send_writer(chunk);
        }
    }
}

void httppeer::send_chunk_end()
{
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, "", true);
        if (socket_session)
        {
            socket_session->send_writer(frame);
        }
    }
    else
    {
        if (socket_session)
        {
            socket_session->send_writer(std::string_view("0\r\n\r\n"));
        }
    }
    ischunked = true;
}

asio::awaitable<void> httppeer::async_send_chunk_header()
{
    status(200);
    if (!isset_type())
    {
        type("application/octet-stream");
    }
    set_header("Cache-Control", "no-cache");
    set_header("Date", get_gmttime());
    ischunked = true;

    std::string header_str;
    if (httpv == 2)
    {
        header_str = make_http2_header(0);
    }
    else
    {
        header_str = make_http1_header();
        header_str.append("\r\n");
    }

    if (socket_session)
    {
        co_await socket_session->async_send_writer(header_str);
    }
    ischunked = false;
    co_return;
}

asio::awaitable<void> httppeer::async_send_chunk_body(std::string_view data)
{
    if (data.empty())
    {
        co_return;
    }
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, data, false);
        if (socket_session)
        {
            co_await socket_session->async_send_writer(frame);
        }
    }
    else
    {
        std::string chunk;
        chunk.reserve(data.size() + 16);
        std::ostringstream ss;
        ss << std::hex << data.size();
        chunk.append(ss.str());
        chunk.append("\r\n");
        chunk.append(data);
        chunk.append("\r\n");

        if (socket_session)
        {
            co_await socket_session->async_send_writer(chunk);
        }
    }
    co_return;
}

asio::awaitable<void> httppeer::async_send_chunk_end()
{
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, "", true);
        if (socket_session)
        {
            co_await socket_session->async_send_writer(frame);
        }
    }
    else
    {
        if (socket_session)
        {
            co_await socket_session->async_send_writer(std::string_view("0\r\n\r\n"));
        }
    }
    ischunked = true;
    co_return;
}

asio::awaitable<void> httppeer::async_make_sse_header()
{
    status(200);
    type("text/event-stream");
    set_header("Cache-Control", "no-cache");
    set_header("Date", get_gmttime());
    set_header("Last-Modified", get_gmttime((unsigned long long)fileinfo.st_mtime));
    if (!etag.empty())
    {
        set_header("ETag", etag);
    }
    ischunked = true;
    // Transfer-Encoding: chunked 和 Connection 由 make_http1_header 根据 ischunked/keepalive 自动添加

    std::string header_str;
    if (httpv == 2)
    {
        header_str = make_http2_header(0);
    }
    else
    {
        header_str = make_http1_header();
        header_str.append("\r\n");
    }

    if (socket_session)
    {
        co_await socket_session->async_send_writer(header_str);
    }
    co_return;
}

void httppeer::make_sse_header()
{
    status(200);
    type("text/event-stream");
    set_header("Cache-Control", "no-cache");
    set_header("Date", get_gmttime());
    set_header("Last-Modified", get_gmttime((unsigned long long)fileinfo.st_mtime));
    if (!etag.empty())
    {
        set_header("ETag", etag);
    }
    ischunked = true;

    std::string header_str;
    if (httpv == 2)
    {
        header_str = make_http2_header(0);
    }
    else
    {
        header_str = make_http1_header();
        header_str.append("\r\n");
    }

    if (socket_session)
    {
        socket_session->send_writer(header_str);
    }
}

asio::awaitable<void> httppeer::async_make_sse_body(std::string_view data)
{
    std::string sse_data;
    sse_data.reserve(data.size() + 8);
    sse_data.append("data: ");
    sse_data.append(data);
    sse_data.append("\n\n");

    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, sse_data, false);
        if (socket_session)
        {
            co_await socket_session->async_send_writer(frame);
        }
    }
    else
    {
        std::string chunk;
        chunk.reserve(sse_data.size() + 16);
        std::ostringstream ss;
        ss << std::hex << sse_data.size();
        chunk.append(ss.str());
        chunk.append("\r\n");
        chunk.append(sse_data);
        chunk.append("\r\n");

        if (socket_session)
        {
            co_await socket_session->async_send_writer(chunk);
        }
    }
    co_return;
}

void httppeer::make_sse_body(std::string_view data)
{
    std::string sse_data;
    sse_data.reserve(data.size() + 8);
    sse_data.append("data: ");
    sse_data.append(data);
    sse_data.append("\n\n");

    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, sse_data, false);
        if (socket_session)
        {
            socket_session->send_writer(frame);
        }
    }
    else
    {
        std::string chunk;
        chunk.reserve(sse_data.size() + 16);
        std::ostringstream ss;
        ss << std::hex << sse_data.size();
        chunk.append(ss.str());
        chunk.append("\r\n");
        chunk.append(sse_data);
        chunk.append("\r\n");

        if (socket_session)
        {
            socket_session->send_writer(chunk);
        }
    }
}

asio::awaitable<void> httppeer::async_make_sse_end()
{
    if (httpv == 2)
    {
        // 发送空 DATA 帧，END_STREAM 标志
        std::string frame = make_http2_data(stream_id, "", true);
        if (socket_session)
        {
            co_await socket_session->async_send_writer(frame);
        }
    }
    else
    {
        // HTTP/1.1 chunked 结束标记
        if (socket_session)
        {
            co_await socket_session->async_send_writer(std::string_view("0\r\n\r\n"));
        }
    }
    co_return;
}

void httppeer::make_sse_end()
{
    if (httpv == 2)
    {
        std::string frame = make_http2_data(stream_id, "", true);
        if (socket_session)
        {
            socket_session->send_writer(frame);
        }
    }
    else
    {
        if (socket_session)
        {
            socket_session->send_writer(std::string_view("0\r\n\r\n"));
        }
    }
}

}// namespace http
