#ifndef PZ_REDIS_POOL_H
#define PZ_REDIS_POOL_H
/*
 * pzredis 连接池（共用框架 io_context）
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 * updated 2026-10-03
 *
 * 设计目标：HTTP 协程永不阻塞。
 *  - 所有 socket 都跑在框架传进来的 asio::io_context 上，不再起独立 worker 线程。
 *  - async_exec / async_publish / subscribe 全部用 redis_conn_base 的协程版，不走 sync command。
 *
 * 两个类的分工（照 orm_conn_pool.h 的形状：一个 tag 一个池对象，连接归池对象管）：
 *  - redis_pool_section_t：一个段名（= redis.conf 的一个段）一个实例，**管连接**。
 *    配置、maxpool、idle 表、借出/归还/保活都在它身上，一把自己的锁。
 *  - redis_pool：段名 → 段对象的登记表 + 保活周期 + 协程入口。它不碰连接。
 * 参考的 orm_conn_pool 用 shared_ptr 持连接，所以它必须再加一个 pooled_ 归属位防重复归还；
 * 这里连接是 unique_ptr，所有权本身就是归属位（复制不过去），也就没有那个位要维护。
 */
#include <asio.hpp>
#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>
#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "redis_conn.h"
#include "redis_pubsub.h"
#include "pzredis_config.h"

#ifdef ENABLE_REDIS

namespace pz
{
namespace redis
{

// 池侧读数（进程累计）。counters() 那五个数都在连接层，池这一层的两件事原先没有任何读数：
// 超限借用要重连、回池时被静默关闭；协程路排在框架 io_context 的队列上。
struct pool_counters
{
    std::atomic<unsigned long long> created{0};         // 段对象新建的连接条数
    std::atomic<unsigned long long> over_limit_close{0};// idle 已满，回池/保活时被直接关闭的条数
    std::atomic<unsigned int> busy{0};                  // 当前借出未还的连接条数
    std::atomic<unsigned int> peak_busy{0};             // busy 的峰值（高水位，只升不降）
    std::atomic<unsigned int> pending{0};               // 协程路已投递、还没跑完的命令条数（排队 + 在跑）
    std::atomic<unsigned int> peak_pending{0};          // pending 的峰值
};

// tag 管理类：一个段名一个实例。成员照 orm_conn_pool 的先例公开（它的池成员也是 public），
// 但连接的取用只走下面四个方法 —— 借还的记账和判活都在方法里，绕过它们就等于绕过计数。
struct redis_pool_section_t
{
    std::string name;
    // 建连接用的 io_context（框架传进来的那个）。init() 里随段一起定死，之后不改。
    asio::io_context *ioc = nullptr;
    conn_config_t cfg;// 只在 mu 里读写
    // 钳的是 idle 的条数，不是在途借用。
    // 锁规矩：写它只发生在 init()/update()，那里是"先握 sections_mu_ 再握本段 mu"；
    // 所以读它的人只要握着这两把里的任意一把，就必然和写者互斥（run() 读的是登记表那一把）。
    // 借还路径上的判断（idle.size() < maxpool）读的是本段这把锁。
    unsigned int maxpool = 5;
    std::list<std::unique_ptr<redis_conn_base>> idle;
    std::mutex mu;// 就这一把：护 cfg / maxpool / idle

    // 热更新：cfg 与 maxpool 一起换。已经借出去的连接不受影响（它连的是旧地址，用完自然废弃）。
    void update(const conn_config_t &newcfg, unsigned int newmaxpool);
    // subscribe 要一份配置快照，但不能拿着段锁去建连接（锁内阻塞 = 整段等一条 socket）
    conn_config_t take_config();

    // 摘 idle 里第一条活着的；没有就新建（不钳在途数量）。
    // 两条路并存：
    //   sync  get_conn()     — 阻塞 connect + sync command，给业务线程池里的 sync handler 用
    //   async async_get_conn — 协程 async_connect，给 HTTP 协程用，不阻塞事件循环
    std::unique_ptr<redis_conn_base> get_conn();
    asio::awaitable<std::unique_ptr<redis_conn_base>> async_get_conn();
    // 归还：脏/死的直接丢弃；idle 满的就地关闭并计 over_limit_close。所有权交回来，调用方手里就空了
    void back_conn(std::unique_ptr<redis_conn_base> conn);
    // 保活：每拍从 idle 队头摘一小批发 PING，活着的放回队尾（仍受 maxpool 约束）
    asio::awaitable<void> async_ping_idle();
    // 一拍验几条。整批摘走会把全段 idle 扣在这条协程里一整个 PING 的时间，期间谁也借不到。
    // 摘队头 + 放回队尾 = 下一拍自动换一批；从队尾摘则每拍都验同一批新回池的，老那一头永远轮不到。
    static constexpr std::size_t kPingIdleBatch = 4;
};

class redis_pool
{
  public:
    // 初始化：按 conf 建/更新登记表里的段对象（在 server.cpp run() 的 load_redis_config 里调）
    // 存一份框架 io_context 指针供段对象建 socket 用
    void init(asio::io_context &server_ioc, const redis_config_t &conf);
    // 启动保活协程（一次性，直接 co_spawn 到框架 io_context 上）
    void start_keepalive();

    bool is_loaded() const { return loaded_.load(); }

    // 段对象入口。redis_client 构造时拿一次、之后每条命令直接用它（稳态零查表）；
    // 只有段名的调用方（控制器）走下面那组 string 重载，它们各查一次表再交给段对象版。
    std::shared_ptr<redis_pool_section_t> section(const std::string &name);

    // 协程侧入口：全部走 redis_conn_base 的 async_*，不阻塞框架事件循环
    // pfx = 命令级 key 前缀覆盖（空串 = 用段配置）；只段对象入口收它，按段名的重载一律走段配置。
    asio::awaitable<std::optional<reply_t>> async_exec(std::shared_ptr<redis_pool_section_t> sec,
                                                       std::vector<std::string> args,
                                                       std::string pfx = "");
    asio::awaitable<std::optional<reply_t>> async_exec(const std::string &name,
                                                       const std::vector<std::string> &args);
    asio::awaitable<http::obj_val> async_exec_obj(std::shared_ptr<redis_pool_section_t> sec,
                                                  std::vector<std::string> args,
                                                  std::string pfx = "");
    asio::awaitable<http::obj_val> async_exec_obj(const std::string &name,
                                                  const std::vector<std::string> &args);

    // 区分"没送出去"与"服务端回 null"：
    //   nullopt                = 传输失败（池未加载 / 段名不存在 / 连接没建成）
    //   std::optional 有值     = 已送达；其内 obj_val.is_null()==true 才是服务端真·null（键不存在）
    // 不关心区分的调用点仍用 async_exec_obj（回 null），本组为新增、向后兼容。
    asio::awaitable<std::optional<http::obj_val>>
    async_exec_obj_ex(std::shared_ptr<redis_pool_section_t> sec,
                      std::vector<std::string> args,
                      std::string pfx = "");
    asio::awaitable<std::optional<http::obj_val>>
    async_exec_obj_ex(const std::string &name, const std::vector<std::string> &args);

    // 发布：异步 PUBLISH，返回接收者数量（-1 表示失败）
    asio::awaitable<long long> async_publish(std::shared_ptr<redis_pool_section_t> sec,
                                             std::string channel,
                                             std::string payload);
    asio::awaitable<long long> async_publish(const std::string &name,
                                             const std::string &channel,
                                             const std::string &payload);

    // 订阅：在框架 io_context 上 co_spawn 连接 + 订阅，返回订阅句柄。
    // 消息回调在框架 io_context 的某个 worker 线程触发；句柄可以复制，最后一份析构（或句柄 stop()）即结束订阅。
    // 段不存在（或池没起来）时返回空句柄（valid() 为 false）。
    redis_subscription subscribe(std::shared_ptr<redis_pool_section_t> sec,
                                 const std::vector<std::string> &channels,
                                 message_cb cb);
    redis_subscription subscribe(const std::string &name,
                                 const std::vector<std::string> &channels,
                                 message_cb cb);
    redis_subscription psubscribe(std::shared_ptr<redis_pool_section_t> sec,
                                  const std::string &pattern,
                                  pmessage_cb cb);
    redis_subscription psubscribe(const std::string &name,
                                  const std::string &pattern,
                                  pmessage_cb cb);

    // 池侧读数（进程累计）：超限借用要重连、回池时被静默关闭，协程路在框架 io_context 上排队，
    // 这三件事原先在进程内完全看不见，只能从服务端的 total_connections_received 反推。
    static pool_counters &counters();
    // 框架 io_context 的 worker 线程数。asio 的 io_context 不报自己跑在几条线程上，池也不数线程，
    // 所以这个数由 httpserver 起完那批线程时告知一次（server.cpp run()）；没告知就是 0。
    unsigned int worker_count() const { return worker_count_.load(); }
    void set_worker_count(unsigned int n) { worker_count_.store(n); }
    // 框架共享 io_context 指针。controller 侧需要用它来 co_spawn 绕过 HTTP 的 strand 串行化。
    asio::io_context *get_io_context() const { return server_ioc_; }

  private:
    // 空闲连接保活：每 kKeepAliveIntervalSec 秒逐段调一次 async_ping_idle()
    asio::awaitable<void> keepalive_();

    // 保活周期（秒）。redis.conf 目前没有对应配置项，运行时改不了。
    static constexpr unsigned int kKeepAliveIntervalSec = 30;

    // 登记表：只护"有哪些段"，连接不在这把锁后面
    std::map<std::string, std::shared_ptr<redis_pool_section_t>> sections_;
    std::mutex sections_mu_;
    asio::io_context *server_ioc_ = nullptr;// 框架 io_context，不再独立
    std::atomic<unsigned int> worker_count_{0};

    std::atomic<bool> loaded_{false};
};

redis_pool &get_redis_pool();

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS
#endif
