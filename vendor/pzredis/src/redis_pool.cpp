/*
 * pzredis 连接池（共用框架 io_context）
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 * updated 2026-10-03
 */
#include "redis_pool.h"

#ifdef ENABLE_REDIS

#include <asio/co_spawn.hpp>
#include <asio/use_awaitable.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <tuple>
#include <utility>

#include "func.h"// str2uint64_strict（maxpool 的严格数字解析）

namespace pz
{
namespace redis
{

// ===================== 池侧读数 =====================
redis_pool &get_redis_pool()
{
    static redis_pool instance;
    return instance;
}

pool_counters &redis_pool::counters()
{
    static pool_counters instance;
    return instance;
}

namespace
{
// 高水位：只升不降。并发下比较交换会少记几拍，但不会记出一个比真实峰值更大的数。
void bump_peak(std::atomic<unsigned int> &peak, unsigned int now)
{
    unsigned int old = peak.load();
    while (now > old && !peak.compare_exchange_weak(old, now))
    {
    }
}

// 配对计数的减法：两个计数都只由"先加后减"的成对路径写，减到 0 就停手，
// 免得某条重复归还要让读数绕回一个巨大的数（这读数是要上线看的）。
// unique_ptr 让重复归还编译不过，所以这里只是不让读数撒谎的最后一道。
void dec_floor(std::atomic<unsigned int> &v)
{
    unsigned int now = v.load();
    while (now > 0 && !v.compare_exchange_weak(now, now - 1))
    {
    }
}

// 借出登记：busy 只记"被拿走还没还"的条数，还池（back_conn）时减回去。
std::unique_ptr<redis_conn_base> borrow_(std::unique_ptr<redis_conn_base> conn)
{
    unsigned int now = redis_pool::counters().busy.fetch_add(1) + 1;
    bump_peak(redis_pool::counters().peak_busy, now);
    return conn;
}

// 一条已投递、还没跑完的协程路命令（排队 + 在跑都算）。守卫放在 async_exec 自己的协程帧里，
// 帧结束（正常返回或被取消）就减回去。
struct pending_cmd
{
    pending_cmd()
    {
        pool_counters &c = redis_pool::counters();
        bump_peak(c.peak_pending, c.pending.fetch_add(1) + 1);
    }
    ~pending_cmd()
    {
        dec_floor(redis_pool::counters().pending);
    }
    pending_cmd(const pending_cmd &)            = delete;
    pending_cmd &operator=(const pending_cmd &) = delete;
};

// 协程路的归还守卫。busy 只在 back_conn 里减，而 async_command 会抛（协议错、连接错），
// 抛出去的那一条连接就永久漏在池外 —— 所以归还挂在析构上，不挂在调用点。
// conn 是 unique_ptr：所有权本身就是归属记录，重复归还在类型上写不出来。
struct scoped_back_conn
{
    scoped_back_conn(std::shared_ptr<redis_pool_section_t> sec_ref,
                     std::unique_ptr<redis_conn_base> conn_ref)
        : sec(std::move(sec_ref)), conn(std::move(conn_ref))
    {
    }
    ~scoped_back_conn()
    {
        if (conn)
            sec->back_conn(std::move(conn));
    }
    scoped_back_conn(const scoped_back_conn &)            = delete;
    scoped_back_conn &operator=(const scoped_back_conn &) = delete;

    redis_conn_base *operator->() const { return conn.get(); }
    explicit operator bool() const { return static_cast<bool>(conn); }

    std::shared_ptr<redis_pool_section_t> sec;
    std::unique_ptr<redis_conn_base> conn;
};
}// namespace

// ===================== tag 管理类：一个段一个实例，连接归它管 =====================
void redis_pool_section_t::update(const conn_config_t &newcfg, unsigned int newmaxpool)
{
    std::lock_guard<std::mutex> lk(mu);
    cfg     = newcfg;
    maxpool = newmaxpool;
}

conn_config_t redis_pool_section_t::take_config()
{
    std::lock_guard<std::mutex> lk(mu);
    return cfg;
}

// 同步路径：给业务线程池里的 sync handler 用（redis_client::str_set 等）。
// sync get_conn + sync command 会阻塞调用线程，但业务线程池本来就允许阻塞。
std::unique_ptr<redis_conn_base> redis_pool_section_t::get_conn()
{
    {
        std::lock_guard<std::mutex> lk(mu);
        while (!idle.empty())
        {
            auto c = std::move(idle.front());
            idle.pop_front();
            if (c && c->connected())
                return borrow_(std::move(c));
            if (c)
                redis_conn_base::counters().stale_drop.fetch_add(1);
        }
    }
    // idle 空了：新建。cfg 会被 update()（reload 路径）在运行时整体替换，
    // 这里的读在段锁外，所以先 take_config() 拿快照再连。
    // ioc 只在 init() 建对象那一路写一次，之后不再改，可以直接读。
    if (!ioc)
        return nullptr;
    conn_config_t usecfg = take_config();
    auto nc              = std::make_unique<redis_conn_base>(*ioc);
    if (!nc->connect(usecfg))
        return nullptr;
    redis_pool::counters().created.fetch_add(1);
    return borrow_(std::move(nc));
}

// 协程路径：给 HTTP 协程用。async_connect 不阻塞事件循环。
// 注意：这个函数通常不直接调用，而是通过 async_exec co_spawn 到全局 io_context 上运行，
// 这样 resolver.async_resolve 就不会卡在原请求的 strand 上。
asio::awaitable<std::unique_ptr<redis_conn_base>> redis_pool_section_t::async_get_conn()
{
    {
        std::lock_guard<std::mutex> lk(mu);
        while (!idle.empty())
        {
            auto c = std::move(idle.front());
            idle.pop_front();
            if (c && c->connected())
                co_return borrow_(std::move(c));
            if (c)
                redis_conn_base::counters().stale_drop.fetch_add(1);
        }
    }
    if (!ioc)
        co_return nullptr;
    conn_config_t usecfg = take_config();
    auto nc              = std::make_unique<redis_conn_base>(*ioc);
    if (!co_await nc->async_connect(usecfg))
        co_return nullptr;
    redis_pool::counters().created.fetch_add(1);
    co_return borrow_(std::move(nc));
}

void redis_pool_section_t::back_conn(std::unique_ptr<redis_conn_base> conn)
{
    if (!conn)
        return;
    dec_floor(redis_pool::counters().busy);
    if (!conn->connected())
    {
        // 死连接 / 超时或协议出错而判脏的连接都不回池：残留字节会被下一个调用方当成回复读走
        redis_conn_base::counters().stale_drop.fetch_add(1);
        return;
    }
    std::lock_guard<std::mutex> lk(mu);
    if (idle.size() < maxpool)
        idle.push_back(std::move(conn));
    else
        redis_pool::counters().over_limit_close.fetch_add(1);// 超出本段 idle 上限，unique_ptr 析构自动关闭
}

asio::awaitable<void> redis_pool_section_t::async_ping_idle()
{
    std::list<std::unique_ptr<redis_conn_base>> batch;
    {
        std::lock_guard<std::mutex> lk(mu);
        // 只摘队头那几条（借出从队头拿，队头＝最急着被人用走的，也正是该先判活的那几条）。
        // 批外的连接留在 idle 里，本拍照常可被借走。splice 保持原有次序。
        while (batch.size() < kPingIdleBatch && !idle.empty())
            batch.splice(batch.end(), idle, idle.begin());
    }
    std::vector<std::unique_ptr<redis_conn_base>> kept;
    for (auto &c : batch)
    {
        if (!c || !c->connected())
        {
            if (c)
                redis_conn_base::counters().stale_drop.fetch_add(1);
            continue;
        }
        // PING 走的是连接层的协程收发，所以它自带期限（读/写各 timeout_sec）：
        // 对端不回就是这一条被判脏、丢出池，而不是这一拍永远停在 co_await 上。
        auto ok = co_await c->async_ping();
        if (!ok || !c->connected())
            redis_conn_base::counters().stale_drop.fetch_add(1);
        else
            kept.push_back(std::move(c));
    }
    if (kept.empty())
        co_return;
    std::lock_guard<std::mutex> lk(mu);
    for (auto &k : kept)
    {
        if (idle.size() < maxpool)
            idle.push_back(std::move(k));
        else
            redis_pool::counters().over_limit_close.fetch_add(1);// 放回时限额已收紧，这条就地关闭
    }
}

// ===================== 运行时：登记表 + 保活 =====================
void redis_pool::init(asio::io_context &server_ioc, const redis_config_t &conf)
{
    server_ioc_ = &server_ioc;
    // 就地更新：段对象一旦被建出来就不从表里摘除（只 insert，永不 erase），
    // 所以已经持有 shared_ptr<redis_pool_section_t> 的调用方（redis_client）永远看到同一个对象，
    // 热更新改的也正是它。代价：conf 里删掉一段不会让登记表丢掉它，那一段仍然可借，直到进程退出。
    std::lock_guard<std::mutex> lk(sections_mu_);
    for (const auto &name : conf.sections())
    {
        auto cfg = conf.get(name);
        if (!cfg)
            continue;// 坏掉的段：get() 已经把"哪一段、哪个字段、为什么"打进日志了，这里不再重复
        unsigned int mp          = 5;
        const std::string raw_mp = conf.raw_field(name, "maxpool");
        unsigned long long want  = 0;
        if (raw_mp.empty())
            mp = 5;
        else if (!http::str2uint64_strict(raw_mp, want))
        {
            std::cerr << "[redis] section [" << name << "] maxpool='" << raw_mp
                      << "' is not a number, using 5" << std::endl;
            mp = 5;
        }
        else if (want == 0)
        {
            std::cerr << "[redis] section [" << name << "] maxpool=0, using 5" << std::endl;
            mp = 5;
        }
        else if (want > 64)
        {
            std::cerr << "[redis] section [" << name << "] maxpool=" << want << ", clamped to 64" << std::endl;
            mp = 64;
        }
        else
            mp = static_cast<unsigned int>(want);

        auto it = sections_.find(name);
        if (it == sections_.end())
        {
            // 新建：还没有别人能看见这个对象，所以字段直接写，不走 update() 那把段锁
            auto sec     = std::make_shared<redis_pool_section_t>();
            sec->name    = name;
            sec->ioc     = server_ioc_;
            sec->cfg     = *cfg;
            sec->maxpool = mp;
            sections_.emplace(name, std::move(sec));
            continue;
        }
        // 锁序只有一种：sections_mu_ → 段 mu（这里）。借还路径只握段 mu，保活先放掉登记表再握段 mu，
        // 都不反向，所以不会撞。
        it->second->update(*cfg, mp);
    }
    loaded_.store(true);
}

void redis_pool::start_keepalive()
{
    if (!server_ioc_)
        return;
    // 一次性 co_spawn 到框架 io_context 上，保活循环自己退出（进程退出时 io_context 停了自然停）
    asio::co_spawn(*server_ioc_, keepalive_(), asio::detached);
}

std::shared_ptr<redis_pool_section_t> redis_pool::section(const std::string &name)
{
    std::lock_guard<std::mutex> lk(sections_mu_);
    auto it = sections_.find(name);
    if (it == sections_.end())
        return nullptr;
    return it->second;
}

asio::awaitable<void> redis_pool::keepalive_()
{
    // 常驻协程：每 kKeepAliveIntervalSec 秒逐段 ping 一次 idle 连接。
    // 逐段是**串行**的，刻意的：看护循环宁可慢一拍，不为一拍把 CPU 抢给 ping。
    // 代价是一拍的最坏耗时 = 段数 × kPingIdleBatch ×（写期限 + 读期限），默认配置就是每段最坏 4×(3+3) 秒；
    // 到点那几条会被丢出池，下一拍接着验队头剩下的。
    asio::steady_timer timer(*server_ioc_);
    while (loaded_.load())
    {
        timer.expires_after(std::chrono::seconds(kKeepAliveIntervalSec));
        asio::error_code tec;
        std::tie(tec) = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        if (tec || !loaded_.load())
            co_return;

        std::vector<std::shared_ptr<redis_pool_section_t>> targets;
        {
            std::lock_guard<std::mutex> lk(sections_mu_);
            for (auto &kv : sections_)
                targets.push_back(kv.second);
        }
        for (auto &sec : targets)
            co_await sec->async_ping_idle();
    }
    co_return;
}

asio::awaitable<std::optional<reply_t>> redis_pool::async_exec(std::shared_ptr<redis_pool_section_t> sec,
                                                               std::vector<std::string> args,
                                                               std::string pfx)
{
    if (!sec || !loaded_.load())
        co_return std::nullopt;
    pending_cmd queued;
    // 纯协程路径：直接调 async_get_conn + async_command。
    // 全程 co_await 不阻塞 strand，比 co_spawn 快 10%（省掉两次 dispatch + 协程帧）。
    auto conn = co_await sec->async_get_conn();
    if (!conn)
        co_return std::nullopt;
    scoped_back_conn guard{sec, std::move(conn)};
    auto r = co_await guard->async_command(std::move(args), std::move(pfx));
    co_return std::move(r);
}

asio::awaitable<std::optional<reply_t>> redis_pool::async_exec(const std::string &name,
                                                               const std::vector<std::string> &args)
{
    co_return co_await async_exec(section(name), std::vector<std::string>(args));
}

asio::awaitable<http::obj_val> redis_pool::async_exec_obj(std::shared_ptr<redis_pool_section_t> sec,
                                                          std::vector<std::string> args,
                                                          std::string pfx)
{
    auto r = co_await async_exec(std::move(sec), std::move(args), std::move(pfx));
    if (!r)
    {
        http::obj_val n;
        n.set_null();
        co_return n;
    }
    co_return redis_conn_base::to_obj_val(*r);
}

asio::awaitable<http::obj_val> redis_pool::async_exec_obj(const std::string &name,
                                                          const std::vector<std::string> &args)
{
    co_return co_await async_exec_obj(section(name), std::vector<std::string>(args));
}

asio::awaitable<std::optional<http::obj_val>>
redis_pool::async_exec_obj_ex(std::shared_ptr<redis_pool_section_t> sec,
                              std::vector<std::string> args,
                              std::string pfx)
{
    auto r = co_await async_exec(std::move(sec), std::move(args), std::move(pfx));
    if (!r)
        co_return std::nullopt;               // 明确区分：没送出去
    co_return redis_conn_base::to_obj_val(*r);// 送达（可能是服务端真·null）
}

asio::awaitable<std::optional<http::obj_val>>
redis_pool::async_exec_obj_ex(const std::string &name, const std::vector<std::string> &args)
{
    co_return co_await async_exec_obj_ex(section(name), std::vector<std::string>(args));
}

asio::awaitable<long long> redis_pool::async_publish(std::shared_ptr<redis_pool_section_t> sec,
                                                     std::string channel,
                                                     std::string payload)
{
    if (!sec || !loaded_.load())
        co_return -1;
    pending_cmd queued;
    auto conn = co_await sec->async_get_conn();
    if (!conn)
        co_return -1;
    scoped_back_conn guard{sec, std::move(conn)};
    long long n = co_await guard->async_publish(std::move(channel), std::move(payload));
    co_return n;
}

asio::awaitable<long long> redis_pool::async_publish(const std::string &name,
                                                     const std::string &channel,
                                                     const std::string &payload)
{
    co_return co_await async_publish(section(name), std::string(channel), std::string(payload));
}

redis_subscription redis_pool::subscribe(std::shared_ptr<redis_pool_section_t> sec,
                                         const std::vector<std::string> &channels,
                                         message_cb cb)
{
    if (!sec || !loaded_.load())
        return redis_subscription();
    conn_config_t cfg = sec->take_config();// 放开段锁再建连接
    auto sub          = std::make_shared<redis_subscriber>(*server_ioc_);
    // 协程按值捕获 sub：订阅者的强引用在 pump 起来之前由这里供养，
    // 起来之后由 pump 的协程帧供养。业务侧拿到的句柄只负责"最后一份析构 ⇒ 投递 stop"。
    // 订阅协程跑在 sub 的 strand 上，与 pump 串行执行。
    asio::co_spawn(sub->strand(), [sub, cfg, channels, cb]() -> asio::awaitable<void>
                   {
                       if (co_await sub->async_start(cfg))
                           co_await sub->async_subscribe(channels, std::move(cb)); },
                   asio::detached);
    return redis_subscription::attach(sub);
}

redis_subscription redis_pool::subscribe(const std::string &name,
                                         const std::vector<std::string> &channels,
                                         message_cb cb)
{
    return subscribe(section(name), channels, std::move(cb));
}

redis_subscription redis_pool::psubscribe(std::shared_ptr<redis_pool_section_t> sec,
                                          const std::string &pattern,
                                          pmessage_cb cb)
{
    if (!sec || !loaded_.load())
        return redis_subscription();
    conn_config_t cfg = sec->take_config();
    auto sub          = std::make_shared<redis_subscriber>(*server_ioc_);
    asio::co_spawn(sub->strand(), [sub, cfg, pattern, cb]() -> asio::awaitable<void>
                   {
                       if (co_await sub->async_start(cfg))
                           co_await sub->async_psubscribe(pattern, std::move(cb)); },
                   asio::detached);
    return redis_subscription::attach(sub);
}

redis_subscription redis_pool::psubscribe(const std::string &name,
                                          const std::string &pattern,
                                          pmessage_cb cb)
{
    return psubscribe(section(name), pattern, std::move(cb));
}

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS
