/*
 * test_redis_client.cpp — pzredis 功能验证 (controller)
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 *
 * 未开启 -DENABLE_REDIS 时，本文件的五个路由仍然存在，但一律按"路由不存在"回 404：
 * 控制器是由构建系统无条件登记进来的，函数体不能整文件关掉，否则链接阶段找不到符号；
 * 回 404 而不是回一句"未编译 redis 支持"，是为了不把"这个二进制编了哪些功能"摆到公开路由面上。
 * 路由：
 *   redis/parse    纯协议解析自测（不连服务端）
 *   redis/pool     协程路径：get_redis_pool().async_exec_obj(...)（业务推荐入口）
 *   redis/sync     同步接口演示（在后台线程跑，避免阻塞事件循环）
 *   redis/pubsub   发布订阅演示
 *   redis/hardfix  回归自检台（组包 / key 前缀 / 超时与脏连接 / TLS 握手期限 / 三态可分）
 */

#include <atomic>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "httppeer.h"
#ifdef ENABLE_REDIS
#include "redis_reply.h"
#include "redis_conn.h"
#include "redis_pubsub.h"
#include "redis_pool.h"
#include "pzredis_config.h"
#endif// ENABLE_REDIS

namespace http
{
#ifdef ENABLE_REDIS
// ==================== 订阅测试共享状态（redis worker 线程回调写，HTTP 线程读）====================
namespace
{
struct pubsub_test_ctx
{
    std::mutex mu;
    // 句柄（不是 subscriber 本体）：复制安全，最后一份析构或 stop() 才结束订阅
    std::map<std::string, pz::redis::redis_subscription> subs;
    std::vector<std::string> received;
};
pubsub_test_ctx g_pubsub;
}// namespace
#endif// ENABLE_REDIS

// ==================== redis/parse：协议层自测 ====================
//@urlpath(null,redis/parse)
std::string test_redis_parse(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    client.val.set_array();

    const char *samples[] = {
        "+OK\r\n",
        "-ERR unknown command\r\n",
        ":42\r\n",
        ":-1\r\n",
        "$5\r\nhello\r\n",
        "$-1\r\n",
        "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n",
        ",3.14\r\n",
        "#t\r\n",
        "_\r\n",
        "*2\r\n*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n$3\r\nbaz\r\n",
        // RESP3 独有帧：本项目不发 HELLO 3，这两条按协议错拒绝（见 redis/hardfix 的 P 组）
        "%1\r\n+a\r\n+b\r\n",
        "~1\r\n+a\r\n"};

    pz::redis::reply_parser_t parser;
    for (const char *s : samples)
    {
        std::string buf(s);
        std::size_t consumed = 0;
        pz::redis::reply_t reply;
        auto st = parser.feed(reinterpret_cast<const unsigned char *>(buf.data()), buf.size(), consumed, reply);

        http::obj_val item;
        item.set_object();
        item["status"]   = (st == pz::redis::parse_status::ok) ? std::string("ok") : (st == pz::redis::parse_status::need_more ? std::string("need_more") : std::string("error"));
        item["consumed"] = static_cast<long long>(consumed);
        item["reply"]    = pz::redis::redis_conn_base::to_obj_val(reply).to_json();
        client.val.push(item);
    }
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif// ENABLE_REDIS

    client.out_json();
    return "";
}

// ==================== redis/pool：协程 + 连接池（推荐） ====================
//@urlpath(null,redis/pool)
asio::awaitable<std::string> test_redis_pool(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    client.val.set_object();

    if (!pz::redis::get_redis_pool().is_loaded())
    {
        client.val["error"] = "redis pool not loaded (ENABLE_REDIS off or conf/redis.conf missing)";
        client.out_json();
        co_return "";
    }

    auto set_r = co_await pz::redis::get_redis_pool().async_exec_obj(
        "default",
        std::vector<std::string>{"SET", "pzredis_test", "hello_world"});
    client.val["set"] = set_r.to_json();

    auto get_r = co_await pz::redis::get_redis_pool().async_exec_obj(
        "default",
        std::vector<std::string>{"GET", "pzredis_test"});
    client.val["get"] = get_r.to_json();

    auto keys = co_await pz::redis::get_redis_pool().async_exec_obj(
        "default",
        std::vector<std::string>{"KEYS", "pzredis_*"});
    client.val["keys"] = keys.to_json();
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif// ENABLE_REDIS

    client.out_json();
    co_return "";
}

// ==================== redis/sync：同步接口演示 ====================
// 同步 connect()/command() 会占住调用线程直到期限，所以不能跑在事件循环线程上。
// 本路由是 std::string 控制器，router 分派同步步时已经把它交给业务线程池
// （router.cpp 里的 co_pool_run_step），函数体就地阻塞即可，不需要再换一次线程。
//@urlpath(null,redis/sync)
std::string test_redis_sync(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    client.val.set_object();

    std::string result;
    try
    {
        asio::io_context ioc;
        pz::redis::redis_conn_base conn(ioc);
        auto cfg = pz::redis::redis_conf("default");
        if (!cfg)
        {
            result = "config section [default] unusable, see server stderr log";
        }
        else if (!conn.connect(*cfg))
        {
            result = "connect fail, errc=" + std::to_string(static_cast<int>(conn.last_error())) +
                     " msg=" + conn.last_error_msg();
        }
        else
        {
            auto r = conn.command({"PING"});
            if (r)
                result = "PING -> " + r->str_value;
            else
                result = "ping fail, errc=" + std::to_string(static_cast<int>(conn.last_error()));
        }
    }
    catch (const std::exception &e)
    {
        result = std::string("exception: ") + e.what();
    }

    client.val["result"] = result;
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif// ENABLE_REDIS

    client.out_json();
    return "";
}

// ==================== redis/pubsub：发布订阅（推荐入口 pool.subscribe + pool.async_publish）====================
// 用法：
//   订阅：redis/pubsub?mode=sub&ch=test_chan          （ch 可用逗号列多个：&ch=a,b）
//   重复名字：redis/pubsub?mode=sub&ch=x,x            （订完用 mode=state 看确认有没有等齐）
//   发布：redis/pubsub?mode=pub&ch=test_chan&msg=hello
//   取消息：redis/pubsub?mode=poll     （拿到订阅回调收到的消息）
//   看状态：redis/pubsub?mode=state    （每条订阅的 state/频道数；0=idle 1=connecting 2=subscribed 3=stopped）
//   退订：redis/pubsub?mode=unsub&ch=test_chan   （显式 stop()）
//   丢句柄：redis/pubsub?mode=drop&ch=test_chan  （不调 stop()，只让最后一份句柄析构）
//@urlpath(null,redis/pubsub)
asio::awaitable<std::string> test_redis_pubsub(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    client.val.set_object();

    if (!pz::redis::get_redis_pool().is_loaded())
    {
        client.val["error"] = "redis pool not loaded (ENABLE_REDIS off or conf/redis.conf missing)";
        client.out_json();
        co_return "";
    }

    std::string mode = client.get["mode"].to_string();
    std::string ch   = client.get["ch"].to_string();
    if (ch.empty())
        ch = "pzredis_pubsub_test";

    if (mode == "sub")
    {
        // ch 允许用逗号列多个频道：?mode=sub&ch=a,b 订两个；?mode=sub&ch=a,a 故意重复，
        // 服务端对同一批 SUBSCRIBE 里的重名只回一格确认帧，订完用 mode=state 看它有没有等齐。
        std::vector<std::string> chans;
        std::string one;
        for (char c : ch)
        {
            if (c == ',')
            {
                if (!one.empty())
                    chans.push_back(one);
                one.clear();
            }
            else
                one.push_back(c);
        }
        if (!one.empty())
            chans.push_back(one);

        auto sub = pz::redis::get_redis_pool().subscribe(
            "default",
            chans,
            [](const std::string &channel, const std::string &payload)
            {
                std::lock_guard<std::mutex> lk(g_pubsub.mu);
                g_pubsub.received.push_back(channel + " => " + payload);
            });
        if (sub.valid())
        {
            std::lock_guard<std::mutex> lk(g_pubsub.mu);
            g_pubsub.subs[ch] = sub;
            client.val["ok"]  = 1;
            client.val["msg"] = "subscribed to " + ch;
            // 订阅是在 redis worker 上起的，这里只报"发起时请求了几个名字"；
            // 去重后的实际频道数看 mode=state。
            client.val["asked"] = static_cast<long long>(chans.size());
        }
        else
        {
            client.val["error"] = "subscribe failed (section missing?)";
        }
    }
    else if (mode == "pub")
    {
        std::string msg = client.get["msg"].to_string();
        if (msg.empty())
            msg = "hello-" + std::to_string(static_cast<long long>(std::time(nullptr)));
        long long n             = co_await pz::redis::get_redis_pool().async_publish("default", ch, msg);
        client.val["receivers"] = n;
        client.val["channel"]   = ch;
        client.val["payload"]   = msg;
    }
    else if (mode == "poll")
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));// 等 redis worker 线程回调触发
        std::lock_guard<std::mutex> lk(g_pubsub.mu);
        client.val["received"].set_array();
        for (auto &m : g_pubsub.received)
            client.val["received"].push(m);
        g_pubsub.received.clear();
    }
    else if (mode == "state")
    {
        // 订阅是在 redis worker 上 co_spawn 起来的，发起时还在 connecting；mode=state 读的是实时状态。
        // 0=idle 1=connecting 2=subscribed 3=stopped
        std::lock_guard<std::mutex> lk(g_pubsub.mu);
        client.val["subs"].set_object();
        for (auto &kv : g_pubsub.subs)
        {
            client.val["subs"][kv.first]["state"]    = static_cast<long long>(kv.second.state());
            client.val["subs"][kv.first]["channels"] = static_cast<long long>(kv.second.channel_count());
            client.val["subs"][kv.first]["patterns"] = static_cast<long long>(kv.second.pattern_count());
        }
        // 顺手带一份进程累计的连接级计数：空闲这段时间里 timeout 有没有增长，正好能证明
        // 消息泵没被期限打断 —— 只看 state 分不出"期限到点"和"服务端断了连接"这两种死法。
        auto &cnt = pz::redis::redis_conn_base::counters();
        http::obj_val totals;
        totals.set_object();
        totals["timeout"]            = static_cast<long long>(cnt.timeout.load());
        totals["poisoned"]           = static_cast<long long>(cnt.poisoned.load());
        totals["stale_drop"]         = static_cast<long long>(cnt.stale_drop.load());
        client.val["counters_total"] = totals;
    }
    else if (mode == "unsub")
    {
        std::lock_guard<std::mutex> lk(g_pubsub.mu);
        auto it = g_pubsub.subs.find(ch);
        if (it != g_pubsub.subs.end())
        {
            it->second.stop();// 投递到 redis worker 上关闭连接，pump 随后自然结束
            g_pubsub.subs.erase(it);
            client.val["ok"]  = 1;
            client.val["msg"] = "unsubscribed " + ch;
        }
        else
        {
            client.val["msg"] = "no active subscription for " + ch;
        }
    }
    else if (mode == "drop")
    {
        // 只把这一份句柄丢掉、不调 stop()：验"最后一份句柄析构 ⇒ 连接真的收摊"。
        // 服务端那边订阅者连接应当少一条，PUBSUB CHANNELS 里也不该再有这个频道。
        std::lock_guard<std::mutex> lk(g_pubsub.mu);
        auto it = g_pubsub.subs.find(ch);
        if (it != g_pubsub.subs.end())
        {
            g_pubsub.subs.erase(it);
            client.val["ok"]  = 1;
            client.val["msg"] = "handle dropped for " + ch;
        }
        else
        {
            client.val["msg"] = "no active subscription for " + ch;
        }
    }
    else
    {
        client.val["usage"] = "mode=sub|pub|poll|state|unsub|drop";
    }
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif// ENABLE_REDIS

    client.out_json();
    co_return "";
}

// ==================== redis/hardfix：回归自检台 ====================
// 用法：redis/hardfix   （返回 checks 明细 + 分支计数增量）
//
// 五组检查：A 组包 / B 值里带换行 / C key 前缀只落在 key 位 / D 读写期限与脏连接 /
// E 三种失败态可分 + 配置面（段名拼错、数字字段坏了、段里少写一行）。
// 不依赖外部假服务端：D 组自己 bind 一个只握手、永不回包的端口（端口号 0 让内核分配），
// 这就是一个进程内的"黑洞"。
#ifdef ENABLE_REDIS
namespace
{
struct hardfix_bag
{
    struct item
    {
        std::string name;
        bool pass;
        std::string detail;
    };
    std::vector<item> checks;

    void add(const std::string &name, bool pass, const std::string &detail = "")
    {
        checks.push_back({name, pass, detail});
    }
};

// 明细要进 JSON，控制字符先换成可见写法（断言本身比较的是原始字节）
std::string hardfix_show(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s)
    {
        if (c == '\r')
            out += "\\r";
        else if (c == '\n')
            out += "\\n";
        else if (c == '\0')
            out += "\\0";
        else
            out += c;
    }
    return out;
}

// ---- A 组：纯组包，不碰网络 ----
void hardfix_build_checks(hardfix_bag &bag)
{
    auto &cnt                  = pz::redis::redis_conn_base::counters();
    unsigned long long reject0 = cnt.arg_reject.load();

    std::string err;
    bool empty_ok = pz::redis::redis_conn_base::make_command({}, &err).empty();
    bag.add("A1-empty-args", empty_ok && err == "empty args", "err=" + err);

    std::vector<std::string> too_many(pz::redis::kMaxArgs + 1, "x");
    too_many[0] = "DEL";
    err.clear();
    std::string big  = pz::redis::redis_conn_base::make_command(too_many, &err);
    std::string want = "args count " + std::to_string(pz::redis::kMaxArgs + 1) +
                       " > limit " + std::to_string(pz::redis::kMaxArgs);
    bag.add("A2-args-count", big.empty() && err == want, "err=" + err);

    err.clear();
    std::string badname = pz::redis::redis_conn_base::make_command({"GE\r\nT", "k"}, &err);
    bag.add("A3-command-name-crlf", badname.empty() && err == "command name contains CR/LF", "err=" + err);

    // 值里有 CR/LF 是合法数据：帧按长度前缀分段，值原样带过去
    const std::string weird = "line1\r\nline2\r\nINCR pzj:evil\r\n";
    err.clear();
    std::string frame     = pz::redis::redis_conn_base::make_command({"SET", "pzj:crlf", weird}, &err);
    std::string want_head = "*3\r\n$3\r\nSET\r\n$8\r\npzj:crlf\r\n$" + std::to_string(weird.size()) + "\r\n";
    bag.add("A4-value-crlf-kept",
            !frame.empty() && err.empty() &&
                frame.compare(0, want_head.size(), want_head) == 0 &&
                frame.find(weird) != std::string::npos,
            "head=" + hardfix_show(want_head));

    std::vector<std::string> del300;
    del300.push_back("DEL");
    for (unsigned int i = 0; i < 300; ++i)
        del300.push_back("pzj:k" + std::to_string(i));
    std::string del_frame = pz::redis::redis_conn_base::make_command(del300);
    bag.add("A5-300-keys-del", del_frame.compare(0, 6, "*301\r\n") == 0, "head=" + del_frame.substr(0, 6));

    bag.add("A6-arg-reject-counted", cnt.arg_reject.load() - reject0 >= 3, "delta=" + std::to_string(cnt.arg_reject.load() - reject0));
}

// ---- P 组：RESP3 独有帧的显式拒绝，纯解析不碰网络 ----
// 本项目从不发 HELLO 3，所以 map '%' / set '~' 只能来自"对面不是 Redis"或"字节流早就串了"。
// 按 array 收下等于拿出一个形状像样、内容不对的结果，所以判协议错。这两条还要钉住
// consumed 必须回滚到起始：调用方拿到 error 是把连接作废，回滚了才不会留下"已经消费掉一半"的假象。
void hardfix_resp3_status_name(pz::redis::parse_status st, std::string &out)
{
    out = (st == pz::redis::parse_status::ok) ? "ok" : (st == pz::redis::parse_status::need_more) ? "need_more" :
                                                                                                    "error";
}

void hardfix_resp3_checks(hardfix_bag &bag)
{
    pz::redis::reply_parser_t parser;
    std::string st_name;

    auto feed_one = [&](const std::string &buf, pz::redis::reply_t &reply) -> pz::redis::parse_status
    {
        std::size_t consumed = 0;
        auto st              = parser.feed(reinterpret_cast<const unsigned char *>(buf.data()), buf.size(), consumed, reply);
        hardfix_resp3_status_name(st, st_name);
        st_name += " consumed=" + std::to_string(static_cast<long long>(consumed)) + "/" +
                   std::to_string(static_cast<long long>(buf.size()));
        return st;
    };

    const unsigned long long rej0 = pz::redis::reply_parser_t::resp3_reject().load();
    pz::redis::reply_t reply;

    auto st_map = feed_one("%1\r\n+a\r\n+b\r\n", reply);
    bag.add("P1-map-frame-rejected", st_map == pz::redis::parse_status::error, st_name);

    auto st_set = feed_one("~1\r\n+a\r\n", reply);
    bag.add("P2-set-frame-rejected", st_set == pz::redis::parse_status::error, st_name);

    // 对照用例：同样形状的 array / push 必须照旧解析成功，证明拒绝只收在 % 和 ~ 两扇门上，
    // 没有顺手把整个集合族一起关掉。
    auto st_arr = feed_one("*2\r\n+a\r\n+b\r\n", reply);
    bag.add("P3-array-still-parses",
            st_arr == pz::redis::parse_status::ok && reply.type == pz::redis::reply_type::array &&
                reply.array_value.size() == 2,
            st_name + " elems=" + std::to_string(static_cast<long long>(reply.array_value.size())));

    auto st_push = feed_one(">2\r\n+a\r\n+b\r\n", reply);
    bag.add("P4-push-still-parses",
            st_push == pz::redis::parse_status::ok && reply.type == pz::redis::reply_type::push &&
                reply.array_value.size() == 2,
            st_name + " type=" + std::to_string(static_cast<int>(reply.type)));

    // 嵌在 array 里的 map：外层要跟着判错并整帧回滚，不能交出"array 里有 0 个元素"这种半套结果
    auto st_nested = feed_one("*1\r\n%1\r\n+a\r\n+b\r\n", reply);
    bag.add("P5-nested-map-in-array-rejected",
            st_nested == pz::redis::parse_status::error && reply.array_value.empty(),
            st_name);

    const unsigned long long rej_delta = pz::redis::reply_parser_t::resp3_reject().load() - rej0;
    bag.add("P6-resp3-reject-counted", rej_delta >= 3, "delta=" + std::to_string(static_cast<long long>(rej_delta)));
}

// ---- C 组：key 前缀只加在"确知是 key 的参数位" ----
// 写命令走带 prefix 的连接，核对走不带 prefix 的连接：断言里的键名是手写的真实键名，
// 不是把 prefix 再拼一次，否则前缀加错位置也会被"拼两次"掩盖。
void hardfix_prefix_checks(hardfix_bag &bag)
{
    try
    {
        asio::io_context ioc;
        auto plain_opt = pz::redis::redis_conf("default");
        if (!plain_opt)
        {
            bag.add("C0-config-section", false, "redis_conf(\"default\") not available, see server log");
            return;
        }
        pz::redis::conn_config_t plain = *plain_opt;
        plain.prefix.clear();
        plain.timeout_sec                 = 3;
        pz::redis::conn_config_t prefixed = plain;
        prefixed.prefix                   = "pzjpre:";

        pz::redis::redis_conn_base w(ioc);
        pz::redis::redis_conn_base v(ioc);
        if (!w.connect(prefixed) || !v.connect(plain))
        {
            bag.add("C0-connect", false, w.last_error_msg() + " / " + v.last_error_msg());
            return;
        }

        auto exists = [&](const std::string &k) -> long long
        {
            auto r = v.command({"EXISTS", k});
            if (!r || r->is_error())
                return -1;
            return r->int_value;
        };
        auto get_str = [&](const std::string &k, std::string &out) -> bool
        {
            auto r = v.command({"GET", k});
            if (!r || r->is_error() || r->is_null)
                return false;
            out = r->str_value;
            return true;
        };
        auto ok_reply = [&](const std::optional<pz::redis::reply_t> &r)
        {
            return static_cast<bool>(r) && !r->is_error();
        };
        // 集合基数：用**不带前缀**的连接按手写物理键名读，所以它证的是"命令到底写了哪个物理键"
        auto scard = [&](const std::string &k) -> long long
        {
            auto r = v.command({"SCARD", k});
            if (!r || r->is_error())
                return -1;
            return r->int_value;
        };

        std::string got;
        bag.add("C1-set-key-prefixed",
                ok_reply(w.command({"SET", "c1", "v1"})) && exists("pzjpre:c1") == 1 && exists("c1") == 0,
                "pzjpre:c1=" + std::to_string(exists("pzjpre:c1")) + " c1=" + std::to_string(exists("c1")));
        bag.add("C2-value-not-prefixed", get_str("pzjpre:c1", got) && got == "v1", "got=" + got);

        w.command({"SET", "c2", "v2"});
        ok_reply(w.command({"DEL", "c1", "c2"}));// kAll：参数位全是 key
        bag.add("C3-del-all-keys", exists("pzjpre:c1") == 0 && exists("pzjpre:c2") == 0, "pzjpre:c1=" + std::to_string(exists("pzjpre:c1")));

        ok_reply(w.command({"MSET", "p1", "v1", "p2", "v2"}));// kPairs：只有奇数位是 key
        bag.add("C4-mset-key-slots",
                exists("pzjpre:p1") == 1 && exists("pzjpre:p2") == 1 && exists("pzjpre:v1") == 0,
                "p1=" + std::to_string(exists("pzjpre:p1")) + " v1=" + std::to_string(exists("pzjpre:v1")));

        // kScriptNumkeys：args[1] 是脚本正文，绝不能当 key 加前缀
        auto evalr = w.command({"EVAL", "return redis.call('SET',KEYS[1],ARGV[1])", "1", "k9", "v9"});
        bag.add("C5-eval-script-untouched",
                ok_reply(evalr) && get_str("pzjpre:k9", got) && got == "v9" &&
                    exists("pzjpre:return redis.call('SET',KEYS[1],ARGV[1])") == 0,
                "got=" + got + " err=" + hardfix_show(evalr ? evalr->str_value : std::string("no reply")));

        v.command({"SADD", "pzjpre:x", "a", "b"});
        v.command({"SADD", "pzjpre:y", "a", "c"});
        // Set 的三个 *STORE 是 `SINTERSTORE dst key [key…]`，**没有 numkeys 那一格**：
        // 本机实测 COMMAND GETKEYS SUNIONSTORE d s t 把 d、s、t 三格全当键返回（ZUNIONSTORE 同发法不吃那个数字）。
        // 所以这里不能照 ZUNIONSTORE 的写法发 `SUNIONSTORE d 2 x y`：那个 2 在 Set 语法里是一把**源键**
        // 而不是个数，键 "2" 不存在 ⇒ 交集/差集必然归零（SINTERSTORE/SDIFFSTORE 各回 0），
        // 看起来倒像是"本机 redis-server 8.8.0(git_dirty) 的 SINTERSTORE / SDIFFSTORE 本身算错"。
        // 按 Set 自己的语法发就是并 3、交 1、差 1，服务端一直是对的；三条的基数同时证目的键与每个源键都带上了前缀。
        auto sunion = w.command({"SUNIONSTORE", "d", "x", "y"});
        auto sinter = w.command({"SINTERSTORE", "di", "x", "y"});
        auto sdiff  = w.command({"SDIFFSTORE", "dd", "x", "y"});
        bag.add("C6-store-key-slots-are-all-keys",
                ok_reply(sunion) && exists("pzjpre:d") == 1 && scard("pzjpre:d") == 3 &&
                    scard("pzjpre:di") == 1 && scard("pzjpre:dd") == 1,
                "union=" + std::to_string(scard("pzjpre:d")) + " inter=" + std::to_string(scard("pzjpre:di")) +
                    " diff=" + std::to_string(scard("pzjpre:dd")) +
                    " err=" + hardfix_show(sunion && sunion->is_error() ? sunion->str_value : ""));

        // 源键的名字**就叫 "7"**：key 位上出现纯数字是合法键名（这一点和 ZUNIONSTORE 的 numkeys 位同形）。
        // 不带前缀的连接先把物理键 pzjpre:7 建成 {n7}，再由带前缀的连接做 SUNIONSTORE dnum 7 x y：
        // 按"整串都是键"发 ⇒ 基数 4；按 numkeys 去解析那个 7 ⇒ 发出去的是 SUNIONSTORE pzjpre:dnum 7 pzjpre:x pzjpre:y，
        // 裸名 7 没人读得到 ⇒ 基数 3。两种发法命令都返回成功，差别只在结果集合的内容里，只能从不带前缀的连接外侧去数基数（页内两边是对称的，分不出来）。
        // 用 7 不用 2 是因为下面 C12 断言 pzjpre:2 不存在（C12 证的正是"数字位不被当成键"）。
        v.command({"SADD", "pzjpre:7", "n7"});
        auto sunum = w.command({"SUNIONSTORE", "dnum", "7", "x", "y"});
        bag.add("C6b-numeric-named-source-key-is-a-key",
                ok_reply(sunum) && scard("pzjpre:dnum") == 4 && scard("pzjpre:7") == 1,
                "dnum=" + std::to_string(scard("pzjpre:dnum")) + " seven=" + std::to_string(scard("pzjpre:7")) +
                    " err=" + hardfix_show(sunum && sunum->is_error() ? sunum->str_value : ""));

        v.command({"SET", "pzjpre:sx", "abc"});
        auto bitop = w.command({"BITOP", "AND", "bd", "sx"});// kAfterOp：args[1] 是操作符，其后才是 key
        bag.add("C7-bitop-skips-operator",
                ok_reply(bitop) && exists("pzjpre:bd") == 1 && exists("pzjpre:AND") == 0,
                "bd=" + std::to_string(exists("pzjpre:bd")) + " err=" + hardfix_show(bitop && bitop->is_error() ? bitop->str_value : ""));

        v.command({"SET", "pzjpre:zz", "1"});
        auto keys       = w.command({"KEYS", "pzjpre:*"});// kNone：pattern 不是 key
        bool pattern_ok = keys && !keys->is_error() && keys->type == pz::redis::reply_type::array &&
                          keys->array_value.size() >= 1;
        bag.add("C8-keys-pattern-not-prefixed", pattern_ok, "n=" + std::to_string(keys && keys->type == pz::redis::reply_type::array ? keys->array_value.size() : 0));

        // numkeys 缺失 / 不可解析：只记一次 prefix_skip，绝不能越界读参数
        // 两条都必须是**真带 numkeys 位**的命令：Set 的 *STORE 已经在 kAll 上（它没有那一格），
        // 拿它来拨这个计数器就只是碰巧 —— 所以这一条改用 ZUNIONSTORE（kDestNumkeys 只剩它和 ZINTERSTORE）。
        auto &cnt                = pz::redis::redis_conn_base::counters();
        unsigned long long skip0 = cnt.prefix_skip.load();
        w.command({"ZUNIONSTORE", "d"});       // 参数不够，取不到 numkeys
        w.command({"EVAL", "return 1", "abc"});// numkeys 不是数字
        bag.add("C9-numkeys-guard-counted", cnt.prefix_skip.load() - skip0 >= 2, "delta=" + std::to_string(cnt.prefix_skip.load() - skip0));

        // ---- 以下每条对应 key 位表里一种参数形状（这些形状早先都归错过类）----

        // kFirst2：LCS 的两格参数位都是 key（早先归成 kNone 时两把键都裸发，读不到带前缀的那对）。
        // LCS 给的是最长公共**子序列**（"abcd"/"abxd" ⇒ abd，不是子串），期望值按服务端语义写。
        // 裸名字上另外写一对不同内容的键，是为了让漏加前缀时答案是"另一个值"而不是空串。
        v.command({"SET", "pzjpre:lc1", "abcd"});
        v.command({"SET", "pzjpre:lc2", "abxd"});
        v.command({"SET", "lc1", "a1x"});// 不带前缀时这对键的 LCS 是 "ax"
        v.command({"SET", "lc2", "ax9"});
        auto lcs = w.command({"LCS", "lc1", "lc2"});
        bag.add("C10-lcs-both-slots", ok_reply(lcs) && lcs->str_value == "abd", "got=" + (lcs ? hardfix_show(lcs->str_value) : std::string("<no reply>")) + " want=abd(bare-keys-would-give=ax)");
        v.command({"DEL", "lc1", "lc2"});

        // kAfterOp：OBJECT 的第 1 格是子命令，只有它之后才是 key
        v.command({"SET", "pzjpre:ob", "12345"});
        auto obj = w.command({"OBJECT", "ENCODING", "ob"});
        bag.add("C11-object-skips-subcmd",
                ok_reply(obj) && (obj->str_value == "int" || obj->str_value == "embstr"),
                "got=" + (obj ? obj->str_value : std::string("<no reply>")));

        // kNumkeysAt1：ZUNION 这类 args[1] 是键的个数，数字本身绝不能被当成 key 加前缀
        v.command({"ZADD", "pzjpre:za", "1", "m1"});
        v.command({"ZADD", "pzjpre:zb", "2", "m2"});
        auto un = w.command({"ZUNION", "2", "za", "zb"});
        bag.add("C12-zunion-numkeys-slot",
                ok_reply(un) && un->type == pz::redis::reply_type::array && un->array_value.size() == 2 &&
                    exists("pzjpre:2") == 0,
                "n=" + std::to_string(un && un->type == pz::redis::reply_type::array ? static_cast<long long>(un->array_value.size()) : -1) +
                    " pzjpre:2=" + std::to_string(exists("pzjpre:2")));

        // kStoreKey：SORT ... STORE 的目的键也是 key，而 "STORE" 这个关键字不是
        v.command({"RPUSH", "pzjpre:sr", "2", "1", "3"});
        auto srt = w.command({"SORT", "sr", "STORE", "srdst"});
        bag.add("C13-sort-store-key",
                ok_reply(srt) && srt->int_value == 3 && exists("pzjpre:srdst") == 1 && exists("pzjpre:STORE") == 0,
                "stored=" + std::to_string(srt && !srt->is_error() ? srt->int_value : -1) +
                    " dst=" + std::to_string(exists("pzjpre:srdst")) +
                    " pzjpre:STORE=" + std::to_string(exists("pzjpre:STORE")));

        // kAllButLast：BLPOP 的最后一格是超时，前面到倒数第二格才是 key
        v.command({"LPUSH", "pzjpre:bl", "hit"});
        auto bl = w.command({"BLPOP", "bl", "1"});
        bag.add("C14-blpop-last-is-timeout",
                ok_reply(bl) && bl->array_value.size() == 2 &&
                    bl->array_value[0].str_value == "pzjpre:bl" && bl->array_value[1].str_value == "hit",
                "reply=" + (bl && bl->type == pz::redis::reply_type::array && bl->array_value.size() == 2 ? bl->array_value[0].str_value + "/" + bl->array_value[1].str_value : std::string("<not a 2-array>")));

        // kNumkeysAt2：BLMPOP 前面有"超时 + 键个数"两格数字，都在 key 之前
        v.command({"RPUSH", "pzjpre:blm", "x"});
        auto lm = w.command({"BLMPOP", "1", "1", "blm", "LEFT"});
        bag.add("C15-blmpop-two-counts",
                ok_reply(lm) && !lm->is_null && lm->array_value.size() == 2 &&
                    lm->array_value[0].str_value == "pzjpre:blm",
                "n=" + std::to_string(lm && lm->type == pz::redis::reply_type::array ? static_cast<long long>(lm->array_value.size()) : -1) +
                    " err=" + hardfix_show(lm && lm->is_error() ? lm->str_value : std::string("")));

        // kAll 的 WATCH：只有盯住带前缀的真实键名，另一条连接改这个键才会让事务作废。
        // 早先 WATCH 归成 kNone（盯的是裸名），另一条连接改带前缀的键时 EXEC 照常返回排队结果 ⇒ 漏加前缀时这条检查必定失败。
        w.command({"WATCH", "wa"});
        v.command({"SET", "pzjpre:wa", "1"});
        w.command({"MULTI"});
        w.command({"GET", "wa"});
        auto exec = w.command({"EXEC"});
        bag.add("C16-watch-prefix-aborts-tx", exec && exec->is_null && !exec->is_error(), "is_null=" + std::to_string(exec && exec->is_null ? 1 : 0) + " err=" + hardfix_show(exec && exec->is_error() ? exec->str_value : std::string("")));

        // ---- kFirst2 的"键搬家"三兄弟：RENAME / RENAMENX / COPY ----
        // 本机 redis 8.8 实测 COMMAND GETKEYS RENAME a b、RENAMENX a b、COPY a b 都报 a、b 两把，
        // 而 COPY a b DB 1 只报 a、b —— DB 后面那个数字是库号，不是 key。
        // 这三条原先不在表里，落默认的 kSingle 就是"只给源键加前缀"：目的键裸着搬出去，
        // 键等于离开了命名空间，而服务端两边都回 OK/1，页面上一个错都看不见。
        v.command({"SET", "pzjpre:rn1", "carry"});
        got.clear();// get_str 读不到时不改 out：先清空，失败时 val= 就是空值，不会把上一条检查留下的旧值当成刚读到的
        auto rn = w.command({"RENAME", "rn1", "rn2"});
        bag.add("C17-rename-moves-both-keys",
                ok_reply(rn) && exists("pzjpre:rn1") == 0 && exists("pzjpre:rn2") == 1 &&
                    get_str("pzjpre:rn2", got) && got == "carry",
                "src=" + std::to_string(exists("pzjpre:rn1")) + " dst=" + std::to_string(exists("pzjpre:rn2")) +
                    " val=" + got + " err=" + hardfix_show(rn && rn->is_error() ? rn->str_value : std::string("")));

        // RENAMENX 的语义是"目的键已存在就什么都不做"，所以故意在**裸名**上放一把挡箭键：
        // 两格都带前缀 ⇒ 目的位 pzjpre:rn3 是空的，搬成并返回 1，挡箭键原样不动；
        // 只给源键加前缀 ⇒ 撞上的正是那把挡箭键，返回 0。
        v.command({"SET", "rn3", "bare-keep"});
        v.command({"SET", "pzjpre:rn4", "mv"});
        std::string bare_dst;
        got.clear();
        auto rx = w.command({"RENAMENX", "rn4", "rn3"});
        bag.add("C18-renamenx-both-slots",
                ok_reply(rx) && rx->int_value == 1 && exists("pzjpre:rn3") == 1 && exists("pzjpre:rn4") == 0 &&
                    get_str("pzjpre:rn3", got) && got == "mv" && get_str("rn3", bare_dst) &&
                    bare_dst == "bare-keep",
                "reply=" + std::to_string(rx && !rx->is_error() ? rx->int_value : -1) +
                    " pfx_dst=" + std::to_string(exists("pzjpre:rn3")) + " bare_dst=" + bare_dst);
        v.command({"DEL", "rn3"});// 挡箭键是本页造的裸名，收尾那遍只扫 pzjpre:*，得自己擦

        // COPY 与 RENAME 同形，差别在源键留在原地：目的位裸发的话，带前缀的名字下永远读不到那份拷贝。
        v.command({"SET", "pzjpre:cp1", "orig"});
        auto cp = w.command({"COPY", "cp1", "cp2"});
        bag.add("C19-copy-clones-both-keys",
                ok_reply(cp) && cp->int_value == 1 && exists("pzjpre:cp1") == 1 &&
                    exists("pzjpre:cp2") == 1 && get_str("pzjpre:cp2", got) && got == "orig",
                "reply=" + std::to_string(cp && !cp->is_error() ? cp->int_value : -1) +
                    " src=" + std::to_string(exists("pzjpre:cp1")) + " dst=" + std::to_string(exists("pzjpre:cp2")));

        // COPY 尾部的 `DB <库号>`：加前缀就成 ERR value is not an integer or out of range。
        // 目的库取 1（本机 databases = 16，这一页的检查只碰 0 库），好处是"目的键到底写的哪个名字"能在 1 库里正面读出来。
        // 收尾那遍只扫 0 库，所以这里自己开一条不带前缀的短连接去核对并擦掉那一把键。
        v.command({"SET", "pzjpre:cp5", "dbcopy"});
        pz::redis::redis_conn_base d1(ioc);
        if (!d1.connect(plain))
        {
            bag.add("C20-copy-db-slot-is-not-a-key", false, "db1 conn: " + d1.last_error_msg());
        }
        else
        {
            d1.command({"SELECT", "1"});
            auto pre  = d1.command({"EXISTS", "pzjpre:cp6"});
            auto cpdb = w.command({"COPY", "cp5", "cp6", "DB", "1"});
            auto post = d1.command({"EXISTS", "pzjpre:cp6"});
            bag.add("C20-copy-db-slot-is-not-a-key",
                    pre && !pre->is_error() && pre->int_value == 0 && ok_reply(cpdb) &&
                        cpdb->int_value == 1 && post && !post->is_error() && post->int_value == 1,
                    "pre=" + std::to_string(pre && !pre->is_error() ? pre->int_value : -1) +
                        " reply=" + std::to_string(cpdb && !cpdb->is_error() ? cpdb->int_value : -1) +
                        " post=" + std::to_string(post && !post->is_error() ? post->int_value : -1) +
                        " err=" + hardfix_show(cpdb && cpdb->is_error() ? cpdb->str_value : std::string("")));
            d1.command({"DEL", "pzjpre:cp6"});// 只删这一发刚造出来的那把
            d1.close();
        }

        // kStoreKey 的 STORE 由上面 C13（SORT）证，这一条补同一分支里的另一个关键字 STOREDIST：
        // 本机实测只有 GEORADIUS 家族收它（ZUNIONSTORE/ZINTERSTORE 带 STOREDIST 直接 ERR syntax error，
        // 所以那个分支没有去扫），而服务端把 STORE / STOREDIST 后面那一格都算作 key。
        v.command({"DEL", "grds"});// 万一上一次运行失败过：残留的裸名会把这条其实已经改对的检查一直卡在失败上
        v.command({"GEOADD", "pzjpre:geo", "15", "37", "pal", "15.09", "37.5", "cat"});
        auto gr = w.command({"GEORADIUS", "geo", "15", "37", "200", "km", "STOREDIST", "grds"});
        bag.add("C21-georadius-storedist-key",
                ok_reply(gr) && gr->int_value == 2 && exists("pzjpre:grds") == 1 && exists("grds") == 0 &&
                    exists("pzjpre:STOREDIST") == 0,
                "n=" + std::to_string(gr && !gr->is_error() ? gr->int_value : -1) +
                    " pfx=" + std::to_string(exists("pzjpre:grds")) + " bare=" + std::to_string(exists("grds")) +
                    " err=" + hardfix_show(gr && gr->is_error() ? gr->str_value : std::string("")));

        // 收尾：把 pzjpre:* 全删掉（用不带前缀的连接，按真实键名删）
        auto left = v.command({"KEYS", "pzjpre:*"});
        if (left && left->type == pz::redis::reply_type::array && !left->array_value.empty())
        {
            std::vector<std::string> del = {"DEL"};
            for (const auto &k : left->array_value)
                del.push_back(k.str_value);
            v.command(del);
        }
        w.close();
        v.close();
    }
    catch (const std::exception &e)
    {
        bag.add("C-exception", false, e.what());
    }
}

// ---- D 组：读写期限 + 超时后必须判脏 ----
void hardfix_timeout_checks(hardfix_bag &bag)
{
    using clock = std::chrono::steady_clock;
    try
    {
        auto &cnt              = pz::redis::redis_conn_base::counters();
        unsigned long long to0 = cnt.timeout.load();
        unsigned long long po0 = cnt.poisoned.load();

        auto cfg_opt = pz::redis::redis_conf("default");
        if (!cfg_opt)
        {
            bag.add("D0-config-section", false, "redis_conf(\"default\") not available, see server log");
            return;
        }

        asio::io_context ioc;
        // 黑洞：只 listen，从不 accept，也永不回包。握手由内核完成，写也成功，读等到期限为止。
        asio::ip::tcp::acceptor hole(ioc);
        asio::error_code bec;
        hole.open(asio::ip::tcp::v4(), bec);
        hole.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), 0), bec);
        hole.listen(8, bec);
        if (bec)
        {
            bag.add("D1-blackhole-listen", false, bec.message());
            return;
        }
        bag.add("D1-blackhole-listen", true, "port=" + std::to_string(hole.local_endpoint().port()));

        // AUTH/SELECT 也得让黑洞吞掉，否则测的是握手期而不是命令期，所以这两项清空
        pz::redis::conn_config_t cfg = *cfg_opt;
        cfg.port                     = hole.local_endpoint().port();
        cfg.username.clear();
        cfg.password.clear();
        cfg.dbindex = 0;
        cfg.isssl   = false;
        cfg.prefix.clear();
        cfg.timeout_sec = 2;

        pz::redis::redis_conn_base conn(ioc);
        if (!conn.connect(cfg))
        {
            bag.add("D2-ping-bounded", false, "connect fail: " + conn.last_error_msg());
            hole.close();
            return;
        }
        auto t0      = clock::now();
        auto r       = conn.command({"PING"});
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();
        bag.add("D2-ping-bounded", !r && ms >= 1900 && ms <= 6000, "elapsed_ms=" + std::to_string(ms));
        bag.add("D3-timeout-error-code",
                conn.last_error() == pz::redis::errc::timeout && conn.last_error_msg().find("timeout") != std::string::npos,
                "errc=" + std::to_string(static_cast<int>(conn.last_error())) + " msg=" + conn.last_error_msg());
        bag.add("D4-poisoned-refuses-next",
                conn.is_poisoned() && !conn.connected(),
                "poisoned=" + std::to_string(conn.is_poisoned() ? 1 : 0));

        auto t1       = clock::now();
        auto r2       = conn.command({"PING"});
        long long ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t1).count();
        // 第二条命令必须立刻被拒（不再等一个期限），而且不能把上一条的残留当成回复
        bag.add("D5-second-cmd-fails-fast",
                !r2 && ms2 < 200 && conn.last_error_msg() == "connection poisoned",
                "elapsed_ms=" + std::to_string(ms2) + " msg=" + conn.last_error_msg());
        bag.add("D6-timeout-counters",
                cnt.timeout.load() - to0 >= 1 && cnt.poisoned.load() - po0 >= 1,
                "timeout+=" + std::to_string(cnt.timeout.load() - to0) +
                    " poisoned+=" + std::to_string(cnt.poisoned.load() - po0));

        conn.close();
        hole.close();
    }
    catch (const std::exception &e)
    {
        bag.add("D-exception", false, e.what());
    }
}

// ---- H 组：TLS 握手的期限（同步 connect 与协程 async_connect 各一条通路）----
// 黑洞 = 只 listen、永不 accept 的端口：ClientHello 写得出去，对端一个字节都不回。
// 这正是线上最常见的误配形状（isssl=1 打到了不回 TLS 报文的端口），早先两条通路都没有期限，会永久悬挂。
struct hs_probe_res
{
    std::atomic<bool> finished{false};
    bool ok              = false;
    int errc_val         = 0;
    long long elapsed_ms = 0;
    std::string msg;
};

// 协程探针：cfg 按值传，协程帧要自己养住这份参数
asio::awaitable<void> hardfix_async_handshake(std::shared_ptr<asio::io_context> ioc,
                                              pz::redis::conn_config_t cfg,
                                              std::shared_ptr<hs_probe_res> res)
{
    using clock = std::chrono::steady_clock;
    pz::redis::redis_conn_base conn(*ioc);
    const auto t0   = clock::now();
    res->ok         = co_await conn.async_connect(cfg);
    res->elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();
    res->errc_val   = static_cast<int>(conn.last_error());
    res->msg        = conn.last_error_msg();
    res->finished.store(true);
}

void hardfix_handshake_checks(hardfix_bag &bag)
{
    using clock = std::chrono::steady_clock;
    try
    {
        auto &cnt    = pz::redis::redis_conn_base::counters();
        auto cfg_opt = pz::redis::redis_conf("default");
        if (!cfg_opt)
        {
            bag.add("H0-config-section", false, "redis_conf(\"default\") not available, see server log");
            return;
        }

        asio::io_context ioc;
        asio::ip::tcp::acceptor hole(ioc);
        asio::error_code bec;
        hole.open(asio::ip::tcp::v4(), bec);
        hole.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), 0), bec);
        hole.listen(8, bec);
        if (bec)
        {
            bag.add("H1-blackhole-listen", false, bec.message());
            return;
        }

        pz::redis::conn_config_t cfg = *cfg_opt;
        cfg.host                     = "127.0.0.1";
        cfg.port                     = hole.local_endpoint().port();
        cfg.username.clear();
        cfg.password.clear();
        cfg.dbindex = 0;
        cfg.prefix.clear();
        cfg.ca_file.clear();
        cfg.sni.clear();
        cfg.insecure    = true;// 握手期本来就到不了校验证书那一步，不依赖证书文件
        cfg.isssl       = true;
        cfg.timeout_sec = 2;

        const unsigned long long to0 = cnt.timeout.load();
        const unsigned long long po0 = cnt.poisoned.load();
        const unsigned long long sd0 = cnt.stale_drop.load();

        pz::redis::redis_conn_base conn(ioc);
        const auto t0      = clock::now();
        const bool synced  = conn.connect(cfg);
        const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();
        bag.add("H1-sync-handshake-bounded", !synced && ms >= 1900 && ms <= 6000, "elapsed_ms=" + std::to_string(ms) + " port=" + std::to_string(cfg.port));
        bag.add("H2-sync-handshake-timeout-code",
                conn.last_error() == pz::redis::errc::timeout &&
                    conn.last_error_msg().find("handshake") != std::string::npos,
                "errc=" + std::to_string(static_cast<int>(conn.last_error())) +
                    " msg=" + conn.last_error_msg());
        // 握手到点算进 timeout，但不算"脏连接"：连接从没握成，没有残留字节要防
        bag.add("H3-handshake-timeout-not-poison",
                cnt.timeout.load() - to0 == 1 && cnt.poisoned.load() - po0 == 0 &&
                    cnt.stale_drop.load() - sd0 == 0,
                "timeout+=" + std::to_string(cnt.timeout.load() - to0) +
                    " poisoned+=" + std::to_string(cnt.poisoned.load() - po0) +
                    " stale_drop+=" + std::to_string(cnt.stale_drop.load() - sd0));

        // H4：isssl=1 打到明文 redis（真实误配原样）。这条检查要的是"必须在期限内返回"，不是"必须算作期限"：
        // 明文服务端若用一个非 TLS 的报文回话，握手立刻失败（errc::connect）同样是正确行为。
        auto plain_opt = pz::redis::redis_conf("client1");
        if (!plain_opt)
        {
            bag.add("H4-config-client1", false, "redis_conf(\"client1\") not available");
        }
        else
        {
            pz::redis::conn_config_t plain = *plain_opt;
            plain.host                     = "127.0.0.1";
            plain.username.clear();
            plain.password.clear();
            plain.dbindex = 0;
            plain.prefix.clear();
            plain.ca_file.clear();
            plain.sni.clear();
            plain.insecure    = true;
            plain.isssl       = true;
            plain.timeout_sec = 2;
            pz::redis::redis_conn_base pconn(ioc);
            const auto t1       = clock::now();
            const bool pok      = pconn.connect(plain);
            const long long pms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t1).count();
            const auto pec      = pconn.last_error();
            bag.add("H4-plaintext-port-still-bounded",
                    !pok && pms <= 6000 &&
                        (pec == pz::redis::errc::timeout || pec == pz::redis::errc::connect) &&
                        !pconn.connected(),
                    "elapsed_ms=" + std::to_string(pms) +
                        " errc=" + std::to_string(static_cast<int>(pec)) + " msg=" + pconn.last_error_msg());
            pconn.close();
        }

        // H5：握手失败的连接不能被认为还能用；随后的命令要立刻被拒，而不是再等一个期限
        const auto t2       = clock::now();
        auto r              = conn.command({"PING"});
        const long long rms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t2).count();
        bag.add("H5-failed-handshake-conn-refuses-cmd",
                !r && !conn.connected() && !conn.is_poisoned() && rms < 200,
                "elapsed_ms=" + std::to_string(rms) + " msg=" + conn.last_error_msg());
        conn.close();

        // H6：协程通路（订阅走的就是这一条）在同一个黑洞上也要按期返回
        auto hioc = std::make_shared<asio::io_context>();
        auto res  = std::make_shared<hs_probe_res>();
        asio::co_spawn(*hioc, hardfix_async_handshake(hioc, cfg, res), asio::detached);
        std::thread runner([hioc]()
                           { hioc->run(); });
        bool ready    = false;
        const auto t3 = clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t3).count() < 8000)
        {
            if (res->finished.load())
            {
                ready = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        hioc->stop();// 兜不住时也只让这条线程停在 run() 里；ioc 和 res 由 shared_ptr 养着，不会悬空
        runner.join();
        bag.add("H6-async-handshake-bounded",
                ready && !res->ok && res->elapsed_ms >= 1900 && res->elapsed_ms <= 6000 &&
                    res->errc_val == static_cast<int>(pz::redis::errc::timeout),
                "returned=" + std::to_string(ready ? 1 : 0) +
                    " elapsed_ms=" + std::to_string(res->elapsed_ms) +
                    " errc=" + std::to_string(res->errc_val) + " msg=" + res->msg);

        hole.close();
    }
    catch (const std::exception &e)
    {
        bag.add("H-exception", false, e.what());
    }
}

// ---- I 组：协程通路的读/写期限（同步侧的同类检查是 D 组，写法照 H6）----
// 黑洞 = 只 listen、永不 accept 的端口：命令写得出去（内核收着），一个字节都不回。
// 早先 async_command 的收发都是无限 co_await ⇒ 第一条 PING 就永久挂着，
// 所以这一组每一条在那种写法下都是 returned=0 直接失败，而不是"耗时偏长"。
struct cmd_probe_res
{
    std::atomic<bool> finished{false};
    bool connected       = false;
    bool got_reply       = false;
    long long elapsed_ms = 0;
    int errc_val         = 0;
    std::string msg;
    bool poisoned             = false;
    bool second_rejected_fast = false;
    long long second_ms       = 0;
    std::string second_msg;
};

struct pub_probe_res
{
    std::atomic<bool> finished{false};
    bool connected       = false;
    long long n          = -2;
    long long elapsed_ms = 0;
    std::string msg;
};

struct late_probe_res
{
    std::atomic<bool> finished{false};
    bool got_reply       = false;
    long long elapsed_ms = 0;
    std::string msg;
};

// cfg 按值传：协程帧要自己养住这份参数（H6 同一个道理）。
asio::awaitable<void> hardfix_async_cmd_probe(std::shared_ptr<asio::io_context> ioc,
                                              pz::redis::conn_config_t cfg,
                                              std::shared_ptr<cmd_probe_res> res)
{
    using clock = std::chrono::steady_clock;
    pz::redis::redis_conn_base conn(*ioc);
    res->connected = co_await conn.async_connect(cfg);
    if (!res->connected)
    {
        res->msg = "connect fail: " + conn.last_error_msg();
        res->finished.store(true);
        co_return;
    }
    const auto t0   = clock::now();
    auto r          = co_await conn.async_command({"PING"});
    res->elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();
    res->got_reply  = static_cast<bool>(r);
    res->errc_val   = static_cast<int>(conn.last_error());
    res->msg        = conn.last_error_msg();
    res->poisoned   = conn.is_poisoned();

    // 判脏之后第二条必须立刻被拒（不再等一整个期限），也不能把残留当成回复
    const auto t1             = clock::now();
    auto r2                   = co_await conn.async_command({"PING"});
    res->second_ms            = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t1).count();
    res->second_rejected_fast = (!r2 && res->second_ms < 300);
    res->second_msg           = conn.last_error_msg();
    res->finished.store(true);
    co_return;
}

asio::awaitable<void> hardfix_async_pub_probe(std::shared_ptr<asio::io_context> ioc,
                                              pz::redis::conn_config_t cfg,
                                              std::shared_ptr<pub_probe_res> res)
{
    using clock = std::chrono::steady_clock;
    pz::redis::redis_conn_base conn(*ioc);
    res->connected = co_await conn.async_connect(cfg);
    if (!res->connected)
    {
        res->msg = "connect fail: " + conn.last_error_msg();
        res->finished.store(true);
        co_return;
    }
    const auto t0   = clock::now();
    res->n          = co_await conn.async_publish("pzj:hole", "x");
    res->elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();
    res->msg        = conn.last_error_msg();
    res->finished.store(true);
    co_return;
}

asio::awaitable<void> hardfix_async_late_probe(std::shared_ptr<asio::io_context> ioc,
                                               pz::redis::conn_config_t cfg,
                                               std::shared_ptr<late_probe_res> res)
{
    using clock = std::chrono::steady_clock;
    pz::redis::redis_conn_base conn(*ioc);
    if (!co_await conn.async_connect(cfg))
    {
        res->msg = "connect fail: " + conn.last_error_msg();
        res->finished.store(true);
        co_return;
    }
    const auto t0   = clock::now();
    auto r          = co_await conn.async_command({"PING"});
    res->elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();
    res->got_reply  = static_cast<bool>(r) && !r->is_error();
    if (!res->got_reply)
        res->msg = conn.last_error_msg();
    res->finished.store(true);
    co_return;
}

// 迟到的回复：accept 之后先睡 300ms 再把 +PONG 写出去，然后自己收摊（关闭即 EOF）。
// 这条是给 timeout_sec == 0 用的：0 要是被当成"0 秒期限"，客户端在 300ms 之前就报超时了。
// 和探针同一个 io_context、同一条 runner 线程，所以没有任何阻塞调用。
asio::awaitable<void> hardfix_late_server(std::shared_ptr<asio::io_context> ioc,
                                          std::shared_ptr<asio::ip::tcp::acceptor> acc)
{
    asio::steady_timer late_timer(*ioc);
    asio::ip::tcp::socket s(*ioc);
    co_await acc->async_accept(s, asio::as_tuple(asio::use_awaitable));
    // 故意迟到 300ms：timeout_sec = 0 那一条要的就是"期限正好取 0 时还等不等得到回复"
    late_timer.expires_after(std::chrono::milliseconds(300));
    co_await late_timer.async_wait(asio::as_tuple(asio::use_awaitable));
    const std::string pong = "+PONG\r\n";
    co_await asio::async_write(s, asio::buffer(pong), asio::as_tuple(asio::use_awaitable));
    // 写完先别关：给对端留出把这一包读走的时间（关闭即 EOF，早关会把它挤掉）
    late_timer.expires_after(std::chrono::milliseconds(100));
    co_await late_timer.async_wait(asio::as_tuple(asio::use_awaitable));
    co_return;
}

// 在私有 io_context 上把一条探针协程跑到结束，最多等 wait_ms；返回"协程自己跑完了没有"。
// extra 是跟着一起 spawn 的配角（假服务端），不被计时。
// 到点就 stop()：run() 返回、runner 线程退出，那条没跑完的协程连同它的 socket 留在帧里 ——
// 这正是"永久悬挂"的形状。这里不替它回收：没跑完这件事要靠 returned=0 原样报出来。
bool run_async_probe(std::shared_ptr<asio::io_context> ioc,
                     const std::function<asio::awaitable<void>()> &body,
                     const std::atomic<bool> &flag,
                     long long wait_ms,
                     const std::function<asio::awaitable<void>()> &extra = nullptr)
{
    using clock = std::chrono::steady_clock;
    // 每一趟都得先 restart：上一趟结束时 ioc->stop() 把停止标记留着了，而 asio 的 run()
    // 在带标记的 io_context 上立刻返回 ⇒ 这条探针协程一行都不会跑，打出来就是
    // returned=0 / elapsed_ms=0 / n=默认值 —— 错在夹具自己，不在产品。
    ioc->restart();
    asio::co_spawn(*ioc, body(), asio::detached);
    if (extra)
        asio::co_spawn(*ioc, extra(), asio::detached);
    std::thread runner([ioc]()
                       { ioc->run(); });
    bool ready    = false;
    const auto t0 = clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count() < wait_ms)
    {
        if (flag.load())
        {
            ready = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    ioc->stop();
    runner.join();
    return ready;
}

void hardfix_async_deadline_checks(hardfix_bag &bag)
{
    try
    {
        auto cfg_opt = pz::redis::redis_conf("default");
        if (!cfg_opt)
        {
            bag.add("I0-config-section", false, "redis_conf(\"default\") not available, see server log");
            return;
        }
        auto &cnt                    = pz::redis::redis_conn_base::counters();
        const unsigned long long to0 = cnt.timeout.load();

        auto ioc = std::make_shared<asio::io_context>();
        asio::ip::tcp::acceptor hole(*ioc);
        asio::error_code bec;
        hole.open(asio::ip::tcp::v4(), bec);
        hole.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), 0), bec);
        hole.listen(8, bec);
        if (bec)
        {
            bag.add("I1-blackhole-listen", false, bec.message());
            return;
        }
        bag.add("I1-blackhole-listen", true, "port=" + std::to_string(hole.local_endpoint().port()));

        // AUTH/SELECT 也要让黑洞吞掉，否则测的是握手期而不是命令期（D 组同一口径）
        pz::redis::conn_config_t cfg = *cfg_opt;
        cfg.port                     = hole.local_endpoint().port();
        cfg.username.clear();
        cfg.password.clear();
        cfg.dbindex = 0;
        cfg.isssl   = false;
        cfg.prefix.clear();
        cfg.timeout_sec = 2;

        // I2..I5：协程 command 在黑洞上按期返回、报 timeout、判脏、第二条立刻被拒
        auto cres          = std::make_shared<cmd_probe_res>();
        const bool c_ready = run_async_probe(ioc, [ioc, cfg, cres]()
                                             { return hardfix_async_cmd_probe(ioc, cfg, cres); },
                                             cres->finished,
                                             8000);
        bag.add("I2-async-cmd-bounded",
                c_ready && !cres->got_reply && cres->elapsed_ms >= 1900 && cres->elapsed_ms <= 6000,
                "returned=" + std::to_string(c_ready ? 1 : 0) +
                    " connected=" + std::to_string(cres->connected ? 1 : 0) +
                    " elapsed_ms=" + std::to_string(cres->elapsed_ms));
        bag.add("I3-async-cmd-timeout-errc",
                c_ready && cres->errc_val == static_cast<int>(pz::redis::errc::timeout) &&
                    cres->msg.find("timeout") != std::string::npos,
                "errc=" + std::to_string(cres->errc_val) + " msg=" + cres->msg);
        bag.add("I4-async-cmd-poisoned",
                c_ready && cres->poisoned,
                "poisoned=" + std::to_string(cres->poisoned ? 1 : 0));
        bag.add("I5-async-second-cmd-fails-fast",
                c_ready && cres->second_rejected_fast && cres->second_msg == "connection poisoned",
                "elapsed_ms=" + std::to_string(cres->second_ms) + " msg=" + cres->second_msg);

        // I6：发布通路（async_publish 自己那份读写，不经 async_command）同样要有期限
        auto pres          = std::make_shared<pub_probe_res>();
        const bool p_ready = run_async_probe(ioc, [ioc, cfg, pres]()
                                             { return hardfix_async_pub_probe(ioc, cfg, pres); },
                                             pres->finished,
                                             8000);
        bag.add("I6-async-publish-bounded",
                p_ready && pres->n == -1 && pres->elapsed_ms >= 1900 && pres->elapsed_ms <= 6000,
                "returned=" + std::to_string(p_ready ? 1 : 0) +
                    " n=" + std::to_string(pres->n) + " elapsed_ms=" + std::to_string(pres->elapsed_ms));

        // 两条黑洞检查各记一次 timeout；下界是固定量 2，不是"比 0 大"
        bag.add("I7-timeout-counter-plus",
                cnt.timeout.load() - to0 >= 2,
                "timeout+=" + std::to_string(cnt.timeout.load() - to0));

        hole.close();

        // I8：timeout_sec = 0 仍然等于"不设限"—— 迟到的回复要等得到
        asio::ip::tcp::acceptor late(*ioc);
        asio::error_code lec;
        late.open(asio::ip::tcp::v4(), lec);
        late.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), 0), lec);
        late.listen(1, lec);
        if (lec)
        {
            bag.add("I8-zero-timeout-waits-late-reply", false, "late listen: " + lec.message());
            return;
        }
        pz::redis::conn_config_t lcfg = cfg;
        lcfg.port                     = late.local_endpoint().port();
        lcfg.timeout_sec              = 0;
        // 跑对了只要 300ms 多一点，所以这里 4s 的等待上限本身就是这条检查的断言
        auto acc_sh        = std::make_shared<asio::ip::tcp::acceptor>(std::move(late));
        auto lres          = std::make_shared<late_probe_res>();
        const bool l_ready = run_async_probe(
            ioc,
            [ioc, lcfg, lres]()
            { return hardfix_async_late_probe(ioc, lcfg, lres); },
            lres->finished,
            4000,
            [ioc, acc_sh]()
            { return hardfix_late_server(ioc, acc_sh); });
        bag.add("I8-zero-timeout-waits-late-reply",
                l_ready && lres->got_reply && lres->elapsed_ms >= 250,
                "returned=" + std::to_string(l_ready ? 1 : 0) +
                    " got=" + std::to_string(lres->got_reply ? 1 : 0) +
                    " elapsed_ms=" + std::to_string(lres->elapsed_ms) + " msg=" + lres->msg);
    }
    catch (const std::exception &e)
    {
        bag.add("I-exception", false, e.what());
    }
}

// ---- J 组：保活一拍只摘一小批（async_ping_idle 的分批 + 摘队头）----
// 自己搭一个段对象（成员全公开，池本身就是这么用的），idle 里放 6 条指向黑洞的连接，
// 连拍三回：第一拍只能验掉 kPingIdleBatch 条、剩下的留在队里，第二拍验掉余下的，第三拍空转。
// 早先的写法是"整批 swap 走"⇒ 第一拍就会把 6 条全摘掉，idle1 直接归 0（下面几条全部失败）。
struct pi_probe_res
{
    std::atomic<bool> finished{false};
    unsigned int idle_before = 0;
    unsigned int idle1       = 0;
    unsigned int idle2       = 0;
    unsigned int idle3       = 0;
    long long drop1          = 0;
    long long drop2          = 0;
    long long ms1            = 0;
    long long ms3            = 0;
    std::size_t batch        = 0;
    unsigned int made        = 0;
    std::string err;
};

constexpr unsigned int kPiIdleCount = 6;

asio::awaitable<void> hardfix_ping_idle_probe(std::shared_ptr<asio::io_context> ioc,
                                              pz::redis::conn_config_t cfg,
                                              std::shared_ptr<pi_probe_res> res)
{
    using clock = std::chrono::steady_clock;
    pz::redis::redis_pool_section_t sec;
    // 这个段是本函数私有的：填 cfg / maxpool 和读 idle.size() 都在这一个协程里做，
    // 没有第二个执行者（真实的段由池持有、读写都在 mu 里）。
    sec.name    = "er1hole";
    sec.ioc     = ioc.get();
    sec.cfg     = cfg;
    sec.maxpool = kPiIdleCount;

    // 先全部借出来攥在手里，再一次性还回去。
    // 不能"借一条还一条"地填：借是从队头拿、还是往队尾放，那样每还一条下一轮又立刻被借走，
    // 六圈下来池里只剩一条 —— J1 就会失败在夹具自己数错上，而不是产品上。
    // 先借后还也把 busy 的加减配对了：back_conn 每还一条减一次，凭空还池会把全局的 busy 计数抹低。
    std::vector<std::unique_ptr<pz::redis::redis_conn_base>> held;
    held.reserve(kPiIdleCount);
    unsigned int made = 0;
    for (unsigned int i = 0; i < kPiIdleCount; ++i)
    {
        auto c = co_await sec.async_get_conn();
        if (!c)
        {
            res->err = "async_get_conn returned null at i=" + std::to_string(i);
            break;
        }
        held.push_back(std::move(c));
        ++made;
    }
    for (auto &c : held)
        sec.back_conn(std::move(c));
    held.clear();
    res->idle_before = static_cast<unsigned int>(sec.idle.size());
    res->made        = made;

    auto &pcnt                   = pz::redis::redis_conn_base::counters();
    const unsigned long long sd0 = pcnt.stale_drop.load();
    const auto t1                = clock::now();
    co_await sec.async_ping_idle();
    res->ms1   = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t1).count();
    res->drop1 = static_cast<long long>(pcnt.stale_drop.load() - sd0);
    res->idle1 = static_cast<unsigned int>(sec.idle.size());

    const unsigned long long sd1 = pcnt.stale_drop.load();
    co_await sec.async_ping_idle();
    res->drop2 = static_cast<long long>(pcnt.stale_drop.load() - sd1);
    res->idle2 = static_cast<unsigned int>(sec.idle.size());

    // 空 idle 的一拍：不该摘到东西，也不该等任何东西
    const auto t3 = clock::now();
    co_await sec.async_ping_idle();
    res->ms3   = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t3).count();
    res->idle3 = static_cast<unsigned int>(sec.idle.size());

    res->batch = pz::redis::redis_pool_section_t::kPingIdleBatch;
    res->finished.store(true);
    co_return;
}

void hardfix_ping_idle_checks(hardfix_bag &bag)
{
    auto cfg_opt = pz::redis::redis_conf("default");
    if (!cfg_opt)
    {
        bag.add("J0-config-section", false, "redis_conf(\"default\") not available, see server log");
        return;
    }
    auto ioc = std::make_shared<asio::io_context>();
    asio::ip::tcp::acceptor hole(*ioc);
    asio::error_code bec;
    hole.open(asio::ip::tcp::v4(), bec);
    hole.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), 0), bec);
    // backlog 给到 16：6 条连接都停在"内核已握手、没人 accept"的队列里，队列要是刚好卡满，
    // 多出来的一条会被丢掉，J1 就会失败在夹具自己不通上，而不是产品上。
    hole.listen(16, bec);
    if (bec)
    {
        bag.add("J0-blackhole-listen", false, bec.message());
        return;
    }
    pz::redis::conn_config_t cfg = *cfg_opt;
    cfg.port                     = hole.local_endpoint().port();
    cfg.username.clear();
    cfg.password.clear();
    cfg.dbindex = 0;
    cfg.isssl   = false;
    cfg.prefix.clear();
    cfg.timeout_sec = 1;

    // 一拍的期望摘批数：min(池里的条数, kPingIdleBatch)；整批 swap 走的写法会在这里露出来
    const unsigned int first = kPiIdleCount < static_cast<unsigned int>(pz::redis::redis_pool_section_t::kPingIdleBatch) ? kPiIdleCount : static_cast<unsigned int>(pz::redis::redis_pool_section_t::kPingIdleBatch);
    const unsigned int rest  = kPiIdleCount - first;
    // 一条黑洞上的 ping 花掉的就是读期限（写立刻成功）：一拍串行验 first 条 ⇒ 一拍 ≈ first × timeout_sec。
    // J5 于是同时钉住两件事：这一拍确实有期限（不是无限挂着），且它验的是"一批"而不是"整池"。
    const long long per_cmd_ms = static_cast<long long>(cfg.timeout_sec) * 1000;

    auto res         = std::make_shared<pi_probe_res>();
    const bool ready = run_async_probe(ioc, [ioc, cfg, res]()
                                       { return hardfix_ping_idle_probe(ioc, cfg, res); },
                                       res->finished,
                                       20000);
    hole.close();

    const std::string base_detail = "returned=" + std::to_string(ready ? 1 : 0) +
                                    " made=" + std::to_string(res->made) +
                                    " idle=" + std::to_string(res->idle_before) +
                                    " batch=" + std::to_string(res->batch) + " err=" + res->err;
    // 夹具自己这一项必须先立住：填不进 6 条 idle，后面几项"摘了几条"就没有意义
    bag.add("J1-idle-filled", ready && res->idle_before == kPiIdleCount, base_detail);
    bag.add("J2-tick-one-batch",
            ready && res->idle1 == rest && res->drop1 >= static_cast<long long>(first),
            "idle1=" + std::to_string(res->idle1) + " want=" + std::to_string(rest) +
                " stale_drop+=" + std::to_string(res->drop1) + " tick1_ms=" + std::to_string(res->ms1) +
                " " + base_detail);
    bag.add("J3-tick-the-rest",
            ready && res->idle2 == 0 && res->drop2 >= static_cast<long long>(rest),
            "idle2=" + std::to_string(res->idle2) +
                " stale_drop+=" + std::to_string(res->drop2) + " " + base_detail);
    bag.add("J4-empty-tick-instant",
            ready && res->idle3 == 0 && res->ms3 < 300,
            "idle3=" + std::to_string(res->idle3) + " tick3_ms=" + std::to_string(res->ms3));
    // 黑洞上的 ping 有期限 ⇒ 一拍 ≈ first × timeout_sec（上下都留余量）；
    // 上限另钉一条"不到整池的耗时"：早先整批 swap 走的写法一拍就是 6 条 ⇒ J5 必定失败。
    bag.add("J5-ticks-return-bounded",
            ready && res->ms1 >= static_cast<long long>(first) * (per_cmd_ms * 9 / 10) &&
                res->ms1 <= static_cast<long long>(first) * (per_cmd_ms * 3 / 2) + 200 &&
                res->ms1 < static_cast<long long>(kPiIdleCount) * (per_cmd_ms * 9 / 10),
            "tick1_ms=" + std::to_string(res->ms1) + " want>=" +
                std::to_string(static_cast<long long>(first) * (per_cmd_ms * 9 / 10)) + " want<" +
                std::to_string(static_cast<long long>(kPiIdleCount) * (per_cmd_ms * 9 / 10)) +
                " " + base_detail);
}

// 期望条数：A 6 + C 22 + D 6 + H 6 + I 8 + J 5 + B 5 + E 7 + P 6 = 71。少一条就说明有检查项被静默跳过。
constexpr unsigned int kHardfixExpectedChecks = 71;
}     // namespace
#endif// ENABLE_REDIS

//@urlpath(null,redis/hardfix)
asio::awaitable<std::string> test_redis_hardfix(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    client.val.set_object();

    if (!pz::redis::get_redis_pool().is_loaded())
    {
        client.val["error"] = "redis pool not loaded (ENABLE_REDIS off or conf/redis.conf missing)";
        client.out_json();
        co_return "";
    }

    auto &cnt                               = pz::redis::redis_conn_base::counters();
    const unsigned long long c_timeout      = cnt.timeout.load();
    const unsigned long long c_poisoned     = cnt.poisoned.load();
    const unsigned long long c_prefix_skip  = cnt.prefix_skip.load();
    const unsigned long long c_arg_reject   = cnt.arg_reject.load();
    const unsigned long long c_stale_drop   = cnt.stale_drop.load();
    const unsigned long long c_resp3_reject = pz::redis::reply_parser_t::resp3_reject().load();

    hardfix_bag bag;
    hardfix_build_checks(bag);
    hardfix_resp3_checks(bag);

    // C/D/H/I/J 五组用的是同步接口或自带私有 io_context 的阻塞探针，一条 PING 就要占住线程到
    // 期限为止，所以丢到后台线程去，与 B/E 的协程检查并行；等它在下面 blocking_arm.get() 那里。
    // H 组三条黑洞检查各等满一个 2s 期限（握手到点），I 组两条黑洞检查各 2s + 一条 0.3s，
    // J 组两拍共 6 次 1s 期限 ⇒ 整个后台那一趟约 6 + 4.5 + 6 ≈ 17s（I6 再占一个 2s 期限）。
    hardfix_bag net_bag;
    auto blocking_arm = std::async(std::launch::async, [&net_bag]()
                                   {
        hardfix_prefix_checks(net_bag);
        hardfix_timeout_checks(net_bag);
        hardfix_handshake_checks(net_bag);
        hardfix_async_deadline_checks(net_bag);
        hardfix_ping_idle_checks(net_bag); });

    // ---- B 组：带换行的值走业务路径（连接池 + 协程）----
    const std::string weird = "line1\r\nline2\r\nINCR pzj:evil\r\n";
    const std::string binary("a\0b\0c", 5);
    auto &pool = pz::redis::get_redis_pool();

    auto setr = co_await pool.async_exec("default", {"SET", "pzj:crlf", weird});
    bag.add("B1-set-crlf-value", static_cast<bool>(setr) && !setr->is_error(), "err=" + (setr && setr->is_error() ? setr->str_value : std::string("")));
    auto getr = co_await pool.async_exec("default", {"GET", "pzj:crlf"});
    bag.add("B2-get-crlf-byte-exact",
            getr && !getr->is_null && getr->str_value == weird,
            "got=" + hardfix_show(getr && !getr->is_null ? getr->str_value : std::string("<null>")));
    auto evil = co_await pool.async_exec("default", {"EXISTS", "pzj:evil"});
    bag.add("B3-injected-command-not-run", evil && !evil->is_error() && evil->int_value == 0, "pzj:evil=" + std::to_string(evil && evil->type == pz::redis::reply_type::integer ? evil->int_value : -1));
    auto bset = co_await pool.async_exec("default", {"SET", "pzj:bin", binary});
    bag.add("B4-set-binary-with-nul", static_cast<bool>(bset) && !bset->is_error(), "");
    auto bget = co_await pool.async_exec("default", {"GET", "pzj:bin"});
    bag.add("B5-get-binary-byte-exact",
            bget && !bget->is_null && bget->str_value == binary,
            "len=" + std::to_string(bget && !bget->is_null ? static_cast<long long>(bget->str_value.size()) : -1));

    // ---- E 组：nil / integer 0 / 传输层失败 三态必须互相分得开 ----
    co_await pool.async_exec("default", {"DEL", "pzj:absent"});
    auto nil_r = co_await pool.async_exec("default", {"GET", "pzj:absent"});
    bag.add("E1-nil", nil_r && nil_r->is_null, "");
    auto zero_r = co_await pool.async_exec("default", {"DEL", "pzj:absent"});
    bag.add("E2-integer-zero",
            zero_r && zero_r->type == pz::redis::reply_type::integer && zero_r->int_value == 0,
            "");
    auto dead_r = co_await pool.async_exec("no_such_section", {"PING"});
    bag.add("E3-transport-failure", !dead_r.has_value(), "");
    bag.add("E4-three-states-distinct",
            nil_r && nil_r->is_null && zero_r && zero_r->type == pz::redis::reply_type::integer && !dead_r.has_value(),
            "");

    // ---- 配置面（E5..E7）：拼错段名 / 数字字段坏了 / 段里少写一行 ----
    // E5 用线上 conf 的真名改一个字母（conf/redis.conf 里确实有 [client1]）：早先它会静默兜底成
    // 127.0.0.1:6379，也就是一次没人知道的连接。
    bag.add("E5-typo-section-not-fallback", !pz::redis::redis_conf("clinet1").has_value(), "redis_conf(\"clinet1\") must be empty");
    // E6/E7 读一份只给这两条检查用的临时 conf：它不在服务启动路径上，读完即删。
    // 单独开一份是因为要验的两种坏法都不该写进 conf/redis.conf（那份就是线上在用的）。
    {
        const std::string probe_path = client.get_sitepath() + "/temp/pzredis_conf_probe.conf";
        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(probe_path).parent_path(), ec);
        {
            std::ofstream out(probe_path, std::ios::binary | std::ios::trunc);
            out << "[ok]\nhost = 127.0.0.1\nport = 6379\nprefix = pfxok:\n\n"
                   "[bad]\nhost = 127.0.0.1\nport = 637x\n\n"
                   "[half]\nhost = 127.0.0.1\nport = 6379\ntimeout = 7\nprefix = pfxhalf:\n";
        }
        pz::redis::redis_config_t probe;
        const bool probe_loaded = probe.load(probe_path);
        auto ok_cfg             = probe.get("ok");
        auto bad_cfg            = probe.get("bad");
        // [half] 故意不写 username/ca_file/sni 这些可选行：早先的取值用的是 const operator[]，
        // 查不到就抛，于是那一行之后的 timeout/prefix 全退回默认值——少写一行会连累后面的字段。
        auto half_cfg = probe.get("half");
        bag.add("E6-bad-number-field-fails-only-that-section",
                probe_loaded && ok_cfg && ok_cfg->prefix == "pfxok:" && !bad_cfg.has_value(),
                "loaded=" + std::to_string(probe_loaded ? 1 : 0) +
                    " file_exists=" + std::to_string(std::filesystem::exists(probe_path, ec) ? 1 : 0) +
                    " ok=" + std::to_string(ok_cfg ? 1 : 0) + " bad=" + std::to_string(bad_cfg ? 1 : 0));
        bag.add("E7-field-after-missing-one-still-read",
                half_cfg && half_cfg->prefix == "pfxhalf:" && half_cfg->timeout_sec == 7,
                "prefix=" + (half_cfg ? half_cfg->prefix : std::string("<no cfg>")) +
                    " timeout=" + std::to_string(half_cfg ? static_cast<long long>(half_cfg->timeout_sec) : -1));
        std::filesystem::remove(probe_path, ec);
    }

    co_await pool.async_exec("default", {"DEL", "pzj:crlf", "pzj:bin", "pzj:evil", "pzj:absent"});

    // 等后台那一组跑完再汇总（C 组自己已经把 pzjpre:* 删干净了）
    blocking_arm.get();
    for (auto &c : net_bag.checks)
        bag.checks.push_back(std::move(c));

    // ---- 汇总 ----
    unsigned int pass_n = 0;
    client.val["checks"].set_array();
    for (const auto &c : bag.checks)
    {
        if (c.pass)
            ++pass_n;
        http::obj_val one;
        one.set_object();
        one["name"]   = c.name;
        one["pass"]   = c.pass ? 1 : 0;
        one["detail"] = c.detail;
        client.val["checks"].push(one);
    }
    client.val["total"]          = static_cast<long long>(bag.checks.size());
    client.val["pass"]           = static_cast<long long>(pass_n);
    client.val["fail"]           = static_cast<long long>(bag.checks.size() - pass_n);
    client.val["expected_total"] = static_cast<long long>(kHardfixExpectedChecks);
    client.val["all_pass"]       = (pass_n == kHardfixExpectedChecks) ? 1 : 0;

    http::obj_val delta;
    delta.set_object();
    delta["timeout"]      = static_cast<long long>(cnt.timeout.load() - c_timeout);
    delta["poisoned"]     = static_cast<long long>(cnt.poisoned.load() - c_poisoned);
    delta["prefix_skip"]  = static_cast<long long>(cnt.prefix_skip.load() - c_prefix_skip);
    delta["arg_reject"]   = static_cast<long long>(cnt.arg_reject.load() - c_arg_reject);
    delta["stale_drop"]   = static_cast<long long>(cnt.stale_drop.load() - c_stale_drop);
    delta["resp3_reject"] = static_cast<long long>(pz::redis::reply_parser_t::resp3_reject().load() - c_resp3_reject);
    http::obj_val totals;
    totals.set_object();
    totals["timeout"]            = static_cast<long long>(cnt.timeout.load());
    totals["poisoned"]           = static_cast<long long>(cnt.poisoned.load());
    totals["prefix_skip"]        = static_cast<long long>(cnt.prefix_skip.load());
    totals["arg_reject"]         = static_cast<long long>(cnt.arg_reject.load());
    totals["stale_drop"]         = static_cast<long long>(cnt.stale_drop.load());
    totals["resp3_reject"]       = static_cast<long long>(pz::redis::reply_parser_t::resp3_reject().load());
    client.val["counters_delta"] = delta;
    client.val["counters_total"] = totals;
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif// ENABLE_REDIS

    client.out_json();
    co_return "";
}

}// namespace http
