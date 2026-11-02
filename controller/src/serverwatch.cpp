// serverwatch.cpp — 服务自检路由，原先由 httpserver::httpwatch_register_builtin_routes() 在
// httpwatch() 线程里注册，现在走正常的 controller 注解注册口（注册键带 '/'，URL 面可直接命中）。
//
// frametasks_timeloop 这一条不只是 URL：router 在链上判到 chain_act::run_timeloop 时是按名字
// 找它的（router.cpp 的 call_sync_regfun / find_sitecontent）。这个文件被删掉的话间隔任务不再
// 登记，只留一条 "frametasks_timeloop not registered" 日志。

#include <chrono>
#include <cstdio>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <asio.hpp>
#include <mutex>
#include "func.h"
#include "httppeer.h"
#include "router.h"
#include "server.h"
#include "server_localvar.h"
#include "terminal_color.h"
#include "version.h"
#include "get_rss_memory.h"
#include "orm_conn_pool.h"

namespace http
{

namespace
{
// 复用 server.cpp 的 ORM 连接查看入口：遍历 orm::get_orm_conn_pool_obj() 取每个 tag 的连接池用量。
// edit/select 两把锁在 orm_conn_pool.h 里是 mysql/pg/sqlite 共用的，锁 edit 即可读到全部 edit 连接。
void collect_orm_rows(std::string &html_rows, std::string &json_items)
{
    std::map<std::string, std::shared_ptr<orm::orm_conn_pool>> &ormpools = orm::get_orm_conn_pool_obj();
    bool first = true;
    for (auto &kv : ormpools)
    {
        auto &p = kv.second;
        // conf_data 可能有两份（edit/select），展示优先取有 host 的那份
        // conf_data may have two slots (edit/select); pick the one that has host filled in
        orm::orm_conn_t *cfg = &p->conf_data[0];
        if (cfg->host.empty() && !p->conf_data[1].host.empty())
            cfg = &p->conf_data[1];

        std::size_t edit_n = 0, select_n = 0;
        {
            std::lock_guard<std::mutex> lk(p->conn_edit_mutex);
            edit_n = p->mysql_edit_pool.size() + p->pg_edit_pool.size() + (p->sqlite_edit_conn ? 1 : 0);
        }
        {
            std::lock_guard<std::mutex> lk(p->conn_select_mutex);
            select_n = p->mysql_select_pool.size() + p->pg_select_pool.size() + (p->sqlite_select_conn ? 1 : 0);
        }

        std::string tag    = kv.first;
        std::string host   = cfg->host.empty() ? "(local)" : cfg->host;
        std::string dbname = cfg->dbname;
        std::string dbtype = cfg->dbtype.empty() ? "?" : cfg->dbtype;
        const std::string &err = p->error_msg;

        std::string h_tag    = html_escape(tag);
        std::string h_dbtype = html_escape(dbtype);
        std::string h_host   = html_escape(host);
        std::string h_port   = html_escape(cfg->port);
        std::string h_dbname = html_escape(dbname);
        std::string h_err    = html_escape(err);

        // HTML 表行：所有外部注入字段先转义再拼
        char buf[768];
        std::snprintf(buf, sizeof(buf),
                      "<tr><td>%s</td><td>%s</td><td>%s:%s</td><td>%s</td>"
                      "<td>%zu</td><td>%zu</td><td>%s</td></tr>",
                      h_tag.c_str(), h_dbtype.c_str(), h_host.c_str(), h_port.c_str(),
                      h_dbname.c_str(), edit_n, select_n,
                      err.empty() ? "—" : ("<span class=\"err\">" + h_err + "</span>").c_str());
        html_rows += buf;

        // JSON：所有字符串字段 escape 后再拼
        std::string j_tag    = json_escape(tag);
        std::string j_dbtype = json_escape(dbtype);
        std::string j_host   = json_escape(host);
        std::string j_port   = json_escape(cfg->port);
        std::string j_dbname = json_escape(dbname);
        std::string j_err    = json_escape(err);

        if (!first)
            json_items += ",";
        first = false;
        json_items += "{\"tag\":\"" + j_tag + "\",\"dbtype\":\"" + j_dbtype +
                      "\",\"host\":\"" + j_host + "\",\"port\":\"" + j_port +
                      "\",\"dbname\":\"" + j_dbname +
                      "\",\"edit\":" + std::to_string(edit_n) +
                      ",\"select\":" + std::to_string(select_n) +
                      ",\"err\":\"" + j_err + "\"}";
    }
}
}// namespace

//@urlpath(null,metrics)
std::string paozhu_metrics(std::shared_ptr<httppeer> peer)
{
    httpserver &app  = get_server_app();
    httppeer &client = peer->get_peer();

    bool as_json = client.get.isset("json") && client.get["json"].to_int() != 0;

    unsigned long long self_rss   = get_rss_kb();
    unsigned long long total_mem  = get_total_memory_kb();
    unsigned int       mem_pct    = total_mem ? (unsigned int)(self_rss * 100 / total_mem) : 0;

    unsigned int total_c  = app.total_count.load();
    unsigned int live_c   = app.live_link_count.load();
    unsigned int h2_min   = app.http2_minute_count.load();
    unsigned int h2_tot   = app.total_http2_count.load();
    unsigned int h1_tot   = app.total_http1_count.load();
    unsigned int mqtt_on  = 0;
    {
        std::lock_guard<std::mutex> lk(app.mqtt_sessions_mutex);
        mqtt_on = (unsigned int)app.mqtt_sessions.size();
    }

    bool        rate_on      = app.rate_limit_status.load();
    unsigned int rate_q      = app.rate_queue_count.load();
    unsigned int rate_lim    = app.rate_limited_count.load();
    unsigned int rate_exp    = app.rate_expired_count.load();
    unsigned int rate_admit  = app.rate_limit_second_time_num.load();

    unsigned int th_live  = app.clientrunpool.getlivenum();
    unsigned int th_task  = app.clientrunpool.gettasknum();
    unsigned int th_pool  = app.clientrunpool.getpoolthreadnum();
    unsigned int th_run   = (unsigned int)app.runthreads.size();
    unsigned int th_ws    = (unsigned int)app.websocketthreads.size();
    unsigned int th_h2    = (unsigned int)app.http2_send_data_threads.size();

    unsigned int loop_ticks = app.clientloop_ticks.load();
    unsigned int loop_skip  = app.clientloop_skipped.load();
    std::size_t  loop_tasks = 0;
    {
        std::lock_guard<std::mutex> lk(app.clientlooptasks_mutex);
        loop_tasks = app.clientlooptasks.size();
    }

    server_loaclvar &gvar = get_server_global_var();

    std::string orm_html_rows, orm_json_items;
    collect_orm_rows(orm_html_rows, orm_json_items);

    if (as_json)
    {
        client.set_header("Content-Type", "application/json; charset=utf-8");
        std::ostringstream js;

        int sess_page = 1;
        if (client.get.isset("page"))
        {
            sess_page = client.get["page"].to_int();
            if (sess_page < 1) sess_page = 1;
        }
        std::vector<std::shared_ptr<client_session>> sess_snap;
        {
            std::lock_guard<std::mutex> lk(app.socket_session_lists_mutex);
            for (auto &wp : app.socket_session_lists)
            {
                if (auto sp = wp.lock()) sess_snap.push_back(std::move(sp));
            }
        }
        const std::size_t SESS_PAGE = 100;
        std::size_t sess_total = sess_snap.size();
        std::size_t sess_pages = sess_total ? (sess_total + SESS_PAGE - 1) / SESS_PAGE : 1;
        if ((std::size_t)sess_page > sess_pages) sess_page = (int)sess_pages;
        std::size_t sess_start = (std::size_t)(sess_page - 1) * SESS_PAGE;
        std::size_t sess_end   = std::min(sess_start + SESS_PAGE, sess_total);

        js << "{";
        js << "\"version\":" << PAOZHU_VERSION << ",";
        js << "\"memory\":{\"rss_kb\":" << self_rss << ",\"total_kb\":" << total_mem << ",\"pct\":" << mem_pct << "},";
        js << "\"connections\":{\"total_accepted\":" << total_c << ",\"live\":" << live_c
           << ",\"http1\":" << h1_tot << ",\"http2\":" << h2_tot
           << ",\"http2_minute\":" << h2_min << ",\"mqtt\":" << mqtt_on << "},";
        js << "\"rate_limit\":{\"enabled\":" << (rate_on ? 1 : 0) << ",\"queue\":" << rate_q
           << ",\"limited\":" << rate_lim << ",\"expired\":" << rate_exp << ",\"admit_per_sec\":" << rate_admit << "},";
        js << "\"threadpool\":{\"live\":" << th_live << ",\"tasks\":" << th_task << ",\"pool_threads\":" << th_pool
           << ",\"acceptor\":" << th_run << ",\"websocket\":" << th_ws << ",\"h2_send\":" << th_h2 << "},";
        js << "\"timeloop\":{\"ticks\":" << loop_ticks << ",\"skipped\":" << loop_skip << ",\"tasks\":" << loop_tasks << "},";
        js << "\"orm\":[" << orm_json_items << "],";
        js << "\"sessions\":[";
        {
            bool sfirst = true;
            for (std::size_t i = sess_start; i < sess_end; ++i)
            {
                auto &s = sess_snap[i];
                if (!sfirst) js << ",";
                sfirst = false;
                js << "{\"ip\":\"" << json_escape(s->client_ip) << "\",\"port\":" << s->client_port
                   << ",\"proto\":" << (s->httpv == 2 ? 2 : (s->httpv == 1 ? 1 : 0))
                   << ",\"ssl\":" << (s->isssl ? 1 : 0) << "}";
            }
        }
        js << "],";
        js << "\"session_total\":" << sess_total << ",\"session_page\":" << sess_page
           << ",\"session_pages\":" << sess_pages << ",";
        js << "\"flags\":{\"http2_enable\":" << (gvar.http2_enable ? 1 : 0)
           << ",\"debug\":" << (gvar.debug_enable ? 1 : 0)
           << ",\"daemon\":" << (gvar.deamon_enable ? 1 : 0)
           << ",\"session_type\":" << (int)gvar.session_type << "}";
        js << "}";
        client << js.str();
        return "";
    }

    // ===== HTML5 仪表盘 =====
    client << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\">";
    client << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
    client << "<title>Paozhu Metrics</title>";
    client << "<style>"
           << ":root{--bg1:#eef2f7;--bg2:#f5f6f8;--ink:#1f2937;--muted:#64748b;--card:#fff;"
           << "--line:#eef0f3;--head:#334155;--zebra:#f8fafc;"
           << "--c-mem:#3b82f6;--c-conn:#8b5cf6;--c-rl:#f59e0b;--c-thr:#10b981;--c-orm:#ef4444;--c-flag:#6366f1;"
           << "--ok:#10b981;--warn:#f59e0b;--bad:#ef4444;--off:#94a3b8}"
           << "*{box-sizing:border-box}"
           << "body{margin:0;min-height:100vh;font-family:system-ui,-apple-system,'Segoe UI',Arial,sans-serif;"
           << "color:var(--ink);background:linear-gradient(180deg,var(--bg1),var(--bg2))}"
           << "header.topbar{display:flex;flex-wrap:wrap;align-items:baseline;gap:8px 16px;"
           << "background:linear-gradient(90deg,#0ea5e9,#6366f1);color:#fff;padding:22px 24px;"
           << "border-radius:0 0 14px 14px;box-shadow:0 2px 8px rgba(0,0,0,.15)}"
           << "header.topbar h1{margin:0;font-size:1.5em;letter-spacing:.5px}"
           << "header.topbar p{margin:0;opacity:.92;font-size:.9em}"
           << "header.topbar a{color:#fff;text-decoration:underline}"
           << "main.wrap{max-width:980px;margin:0 auto;padding:20px 16px 40px;"
           << "display:flex;flex-direction:column;gap:16px}"
           << "section.card{background:var(--card);border-radius:12px;box-shadow:0 1px 4px rgba(0,0,0,.08);"
           << "padding:14px 18px;transition:box-shadow .2s}"
           << "section.card:hover{box-shadow:0 4px 14px rgba(0,0,0,.12)}"
           << "section.card>h2{margin:0 0 10px;font-size:1.05em;padding-left:10px;border-left:4px solid #ccc}"
           << ".sec-mem>h2{border-color:var(--c-mem);color:var(--c-mem)}"
           << ".sec-conn>h2{border-color:var(--c-conn);color:var(--c-conn)}"
           << ".sec-rl>h2{border-color:var(--c-rl);color:var(--c-rl)}"
           << ".sec-thr>h2{border-color:var(--c-thr);color:var(--c-thr)}"
           << ".sec-orm>h2{border-color:var(--c-orm);color:var(--c-orm)}"
 ".sec-flag>h2{border-color:var(--c-flag);color:var(--c-flag)}"
           << ".sec-sess>h2{border-color:#0ea5e9;color:#0369a1}"
           << "table{width:100%;border-collapse:collapse;border-radius:8px;overflow:hidden}"
           << "th,td{border-bottom:1px solid var(--line);padding:8px 10px;text-align:left;font-size:.9em}"
           << "th{background:var(--head);color:#fff;font-weight:600}"
           << "tbody tr:nth-child(even){background:var(--zebra)}"
           << ".pill{display:inline-block;padding:2px 10px;border-radius:12px;font-size:.8em;color:#fff;font-weight:600}"
           << ".ok{background:var(--ok)} .warn{background:var(--warn)} .bad{background:var(--bad)}"
           << ".on{background:var(--ok)} .off{background:var(--off)}"
           << ".err{color:var(--bad);font-weight:600}"
           << ".vbad{color:var(--bad);font-weight:700} .vwarn{color:var(--warn);font-weight:700}"
           << "pre{background:#0f172a;color:#e2e8f0;padding:12px;border-radius:8px;overflow:auto;font-size:.82em}"
           << ".muted{color:var(--muted);font-size:.85em;margin:6px 0}"
           << ".pager{display:flex;flex-wrap:wrap;gap:6px;margin-top:10px;align-items:center}"
           << ".pager a{display:inline-block;padding:4px 10px;border-radius:8px;background:#eef2f7;color:#334155;text-decoration:none;font-size:.85em}"
           << ".pager a.cur{background:#6366f1;color:#fff}"
           << "footer{text-align:center;color:var(--muted);font-size:.8em;padding:12px}"
           << "@media (max-width:600px){main.wrap{padding:12px}th,td{padding:6px 8px}}"
           << "</style></head><body>";
    client << "<header class=\"topbar\"><h1>Paozhu Metrics</h1><p>Version "
           << (PAOZHU_VERSION / 100000) << "." << (PAOZHU_VERSION / 100 % 1000) << "." << (PAOZHU_VERSION % 100)
           << " &nbsp;·&nbsp; <a href=\"?json=1\">View JSON</a></p></header>";
    client << "<main class=\"wrap\">";

    // 内存
    const char *mem_cls = mem_pct > 80 ? "bad" : (mem_pct > 50 ? "warn" : "ok");
    client << "<section class=\"card sec-mem\"><h2>Memory</h2><table><tr><th>Process RSS</th><th>System Total</th><th>Usage</th></tr><tr>"
           << "<td>" << (self_rss / 1024) << " MB</td><td>" << (total_mem / 1024) << " MB</td>"
           << "<td><span class=\"pill " << mem_cls << "\">" << mem_pct << "%</span></td></tr></table></section>";

    // 连接
    client << "<section class=\"card sec-conn\"><h2>Connections</h2><table>"
           << "<tr><th>Total Accepted</th><th>Live Links</th><th>HTTP/1 Cumulative</th><th>HTTP/2 Cumulative</th><th>HTTP/2 /60s</th><th>MQTT Online</th></tr><tr>"
           << "<td>" << total_c << "</td><td>" << live_c << "</td><td>" << h1_tot << "</td><td>" << h2_tot
           << "</td><td>" << h2_min << "</td><td>" << mqtt_on << "</td></tr></table></section>";

    // 限流
    const char *rl_cls = rate_on ? "warn" : "ok";
    const char *rq_cls = rate_q > 0 ? "vwarn" : "";
    client << "<section class=\"card sec-rl\"><h2>Rate Limit</h2><table>"
           << "<tr><th>Status</th><th>Queue</th><th>Limited</th><th>Expired</th><th>Admit (/s)</th></tr><tr>"
           << "<td><span class=\"pill " << rl_cls << "\">" << (rate_on ? "ON" : "OFF") << "</span></td>"
           << "<td class=\"" << rq_cls << "\">" << rate_q << "</td><td>" << rate_lim << "</td><td>" << rate_exp << "</td><td>" << rate_admit
           << "</td></tr></table></section>";

    // 线程池
    const char *tb_cls = th_task > 0 ? "vwarn" : "";
    client << "<section class=\"card sec-thr\"><h2>Threads</h2><table>"
           << "<tr><th>Active Workers</th><th>Backlog</th><th>Pool Threads</th><th>Acceptor</th><th>WebSocket</th><th>H2 Send</th></tr><tr>"
           << "<td>" << th_live << "</td><td class=\"" << tb_cls << "\">" << th_task << "</td><td>" << th_pool << "</td><td>" << th_run
           << "</td><td>" << th_ws << "</td><td>" << th_h2 << "</td></tr></table></section>";

    // 间隔任务
    client << "<section class=\"card sec-thr\"><h2>Timeloop</h2><table>"
           << "<tr><th>Ticks</th><th>Skipped</th><th>Tasks</th></tr><tr>"
           << "<td>" << loop_ticks << "</td><td>" << loop_skip << "</td><td>" << loop_tasks << "</td></tr></table></section>";

    // ORM 连接池
    client << "<section class=\"card sec-orm\"><h2>ORM Pools</h2><table>"
           << "<tr><th>Tag</th><th>Type</th><th>Host</th><th>DB</th><th>Edit</th><th>Select</th><th>Error</th></tr>"
           << orm_html_rows << "</table></section>";

    // 全局开关
    const char *fl_on  = gvar.http2_enable ? "on" : "off";
    const char *fl_dbg = gvar.debug_enable ? "on" : "off";
    const char *fl_dmn = gvar.deamon_enable ? "on" : "off";
    client << "<section class=\"card sec-flag\"><h2>Flags</h2><table>"
           << "<tr><th>http2</th><th>debug</th><th>daemon</th><th>session_type</th></tr><tr>"
           << "<td><span class=\"pill " << fl_on << "\">" << (gvar.http2_enable ? "ON" : "OFF") << "</span></td>"
           << "<td><span class=\"pill " << fl_dbg << "\">" << (gvar.debug_enable ? "ON" : "OFF") << "</span></td>"
           << "<td><span class=\"pill " << fl_dmn << "\">" << (gvar.deamon_enable ? "ON" : "OFF") << "</span></td>"
           << "<td>" << (int)gvar.session_type << "</td></tr></table></section>";

    // 保留原有 show_visitinfo 调试开关（仅 debug 模式生效）
    if (client.get.isset("show_visitinfo"))
    {
        int isshow = client.get["show_visitinfo"].to_int();
        if (isshow == 1 && gvar.debug_enable)
        {
            gvar.show_visitinfo = true;
            client << "<section class=\"card sec-thr\"><h2>Worker Threads</h2><pre>";
            try
            {
                client << app.clientrunpool.printthreads(true);
            }
            catch (...)
            {
                client << "exception";
            }
            client << "</pre></section>";
        }
        else
        {
            gvar.show_visitinfo = false;
        }
    }

    // 当前在线连接列表（socket_session_lists，只读快照，不干预真实逻辑）
    int sess_page = 1;
    if (client.get.isset("page"))
    {
        sess_page = client.get["page"].to_int();
        if (sess_page < 1) sess_page = 1;
    }
    std::vector<std::shared_ptr<client_session>> sess_snap;
    {
        std::lock_guard<std::mutex> lk(app.socket_session_lists_mutex);
        for (auto &wp : app.socket_session_lists)
        {
            if (auto sp = wp.lock()) sess_snap.push_back(std::move(sp));
        }
    }
    const std::size_t SESS_PAGE = 100;
    std::size_t sess_total = sess_snap.size();
    std::size_t sess_pages = sess_total ? (sess_total + SESS_PAGE - 1) / SESS_PAGE : 1;
    if ((std::size_t)sess_page > sess_pages) sess_page = (int)sess_pages;
    std::size_t sess_start = (std::size_t)(sess_page - 1) * SESS_PAGE;
    std::size_t sess_end   = std::min(sess_start + SESS_PAGE, sess_total);

    std::string sess_extra;
    if (client.get.isset("show_visitinfo"))
        sess_extra = "&show_visitinfo=" + std::to_string(client.get["show_visitinfo"].to_int());

    client << "<section class=\"card sec-sess\"><h2>Sessions (online)</h2>";
    client << "<p class=\"muted\">Total online: " << sess_total << " &nbsp;·&nbsp; Page " << sess_page << "/" << sess_pages << "</p>";
    client << "<table><tr><th>#</th><th>Port</th><th>Proto</th><th>SSL</th><th>Remote IP</th></tr>";
    for (std::size_t i = sess_start; i < sess_end; ++i)
    {
        auto &s = sess_snap[i];
        const char *proto = s->httpv == 2 ? "HTTP/2" : (s->httpv == 1 ? "HTTP/1" : "?");
        const char *ssl   = s->isssl ? "SSL" : "-";
        client << "<tr><td>" << (i + 1) << "</td><td>" << s->client_port
               << "</td><td>" << proto << "</td><td>" << ssl << "</td><td>" << html_escape(s->client_ip) << "</td></tr>";
    }
    client << "</table>";
    client << "<div class=\"pager\">";
    if (sess_page > 1)
        client << "<a href=\"?page=" << (sess_page - 1) << sess_extra << "\">&laquo; Prev</a>";
    std::size_t p0 = sess_page > 5 ? (std::size_t)sess_page - 5 : 1;
    std::size_t p1 = std::min(p0 + 10, sess_pages);
    for (std::size_t p = p0; p <= p1; ++p)
    {
        if (p == (std::size_t)sess_page)
            client << "<a class=\"cur\" href=\"?page=" << p << sess_extra << "\">" << p << "</a>";
        else
            client << "<a href=\"?page=" << p << sess_extra << "\">" << p << "</a>";
    }
    if (sess_page < (int)sess_pages)
        client << "<a href=\"?page=" << (sess_page + 1) << sess_extra << "\">Next &raquo;</a>";
    client << "</div></section>";

    client << "</main><footer>Paozhu Metrics · Self-monitoring Dashboard</footer></body></html>";
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
