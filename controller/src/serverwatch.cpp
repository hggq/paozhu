// serverwatch.cpp — 服务自检路由，原先由 httpserver::httpwatch_register_builtin_routes() 在
// httpwatch() 线程里注册，现在走正常的 controller 注解注册口（注册键带 '/'，URL 面可直接命中）。
//
// frametasks_timeloop 这一条不只是 URL：router 在链上判到 chain_act::run_timeloop 时是按名字
// 找它的（router.cpp 的 call_sync_regfun / find_sitecontent）。这个文件被删掉的话间隔任务不再
// 登记，只留一条 "frametasks_timeloop not registered" 日志。

#include <chrono>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <asio.hpp>
#include "func.h"
#include "httppeer.h"
#include "router.h"
#include "server.h"
#include "server_localvar.h"
#include "terminal_color.h"
#include "version.h"

namespace http
{

//@urlpath(null,paozhu_status)
std::string paozhu_status(std::shared_ptr<httppeer> peer)
{
    httpserver &app  = get_server_app();
    httppeer &client = peer->get_peer();
    client << "<h3 align=\"center\">";
    client << "<span style=\"font-size:2em\">Paozhu</h3> <p align=\"center\">Version ";
    client << (PAOZHU_VERSION / 100000);
    client << "." << (PAOZHU_VERSION / 100 % 1000);
    client << "." << (PAOZHU_VERSION % 100);
    client << "</p>";

    int isshow = -1;
    if (client.get.isset("show_visitinfo"))
    {
        isshow                             = client.get["show_visitinfo"].to_int();
        server_loaclvar &static_server_var = get_server_global_var();
        if (isshow == 1 && static_server_var.debug_enable)
        {
            static_server_var.show_visitinfo = true;
            client << "<p>";
            client << "online:" << app.total_count.load() << " ";
            try
            {
                client << "]</p>";
                client << app.clientrunpool.printthreads(true);
            }
            catch (...)
            {
                client << "<p>exception</p>";
            }
        }
        else
        {
            static_server_var.show_visitinfo = false;
        }
    }
    return "";
}

//@urlpath(null,paozhu_routes)
std::string paozhu_routes(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // 这张表导出每一条路由的名字、前置钩子和注册位置（文件:行号）。任何来源都能读到它，
    // 等于把站点内部的路径清单双手递出去。所以只放行本机/内网来源（ip_is_local()，
    // 与证书下载页同一个闸门）；client_ip 取套接字对端地址，伪造请求头换不来本机身份。
    // 拒绝时正文与"本构建没编这一功能"那批桩逐字节一致：连"这条诊断路由存在"也不透露。
    if (!ip_is_local(client.client_ip))
    {
        client.status(404);
        client.val.set_object();
        client.val["error"] = "not found";
        client.out_json();
        return "";
    }
    server_loaclvar &static_server_var = get_server_global_var();
    if (!static_server_var.debug_enable)
    {
        client << "debug_enable off";
        return "";
    }
    std::string filter;
    if (client.get.isset("name"))
    {
        filter = client.get["name"].to_string();
    }
    peer->type("text/plain; charset=utf-8");
    client << get_router().routes_text(filter);
    return "";
}

//@urlpath(null,frametasks_timeloop)
std::string frametasks_timeloop(std::shared_ptr<httppeer> peer)
{
    if (peer->linktype != 7)
    {
        DEBUG_LOG("timeloop enqueue skipped, linktype:%u", (unsigned int)peer->linktype);
        return "";
    }
    httpserver &app = get_server_app();
    if (peer->timeloop_taskname.size() > 0 && peer->etag.size() > 0)
    {
        std::string temptaskhash = peer->timeloop_taskname;
        temptaskhash.append(peer->url);
        std::size_t temp_name_id = std::hash<std::string>{}(temptaskhash);
        std::ostringstream oss;
        oss << temp_name_id;
        temptaskhash = oss.str();
        if (temptaskhash == peer->etag)
        {
            bool isintask = true;
            {
                std::lock_guard<std::mutex> lk(app.clientlooptasks_mutex);
                for (auto iter = app.clientlooptasks.begin(); iter != app.clientlooptasks.end();)
                {
                    if (iter->first == temp_name_id)
                    {
                        isintask = false;
                        break;
                    }
                    ++iter;
                }
                if (isintask)
                    app.clientlooptasks.push_back({temp_name_id, peer});
            }
            if (isintask)
            {
                app.websocketcondition.notify_one();
                DEBUG_LOG("timeloop task queued:%s every:%u", peer->timeloop_taskname.c_str(), peer->timeloop_num.load());
            }
        }
    }
    return "";
}

}// namespace http
