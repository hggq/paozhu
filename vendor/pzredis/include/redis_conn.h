#ifndef PZ_REDIS_CONN_H
#define PZ_REDIS_CONN_H
/*
 * pzredis connection layer (sync + coroutine)
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 *
 * 同步方法（connect/command/ping/close）只允许在**可以阻塞**的线程上调用 —— 现在就是框架的业务
 * 线程池（`std::string` 控制器跑在那里），池自己没有人手线程了。
 * 禁止在跑 io_context 的那批线程上调用：这里的 socket 挂在框架传进来的共享 io_context 上，
 * 占住那条线程等于占住它名下的一批会话。
 * 业务侧请统一走 redis_pool::async_exec() / async_exec_obj()。
 *
 * 期限（timeout_sec）是"读/写各自的期限"，同步路和协程路同一口径：
 * 同步侧 —— asio 的同步 read_some 不吃内核 SO_RCVTIMEO（实测 >6s 不返回），所以连接一建立就把 socket
 * 切成 non_blocking，TLS 握手和之后的读写都由本类自己的"歇拍轮询 + 期限"循环兜住。
 * 协程侧 —— TLS 握手、发命令、收回复各自 race 一只 timeout_sec 的表；timeout_sec == 0 才不建表。
 * 唯一不设限的是订阅消息泵：推送帧本来就要长期等服务端，给它加期限等于把空闲订阅打成永久失败。
 * 订阅确认帧另算：redis_subscriber 自己那一只表管"确认等不齐就算这次订阅失败"。
 */
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "redis_reply.h"
#include "request.h"

namespace pz
{
namespace redis
{

// 错误码（替代原魔法数字）
enum class errc : unsigned short
{
    ok       = 0,
    resolve  = 1,
    connect  = 2,
    auth     = 10,
    select   = 11,
    write    = 20,
    read     = 21,
    protocol = 30,
    bad_args = 40,
    timeout  = 50
};

// 连接配置（段名 → 一个 conn_config_t，与 orm.conf 的 db tag 同理）
struct conn_config_t
{
    std::string host    = "127.0.0.1";
    unsigned short port = 6379;
    std::string username;// Redis 6 ACL（空=经典单密码）
    std::string password;
    unsigned int dbindex = 0;
    bool isssl           = false;
    bool insecure        = false;// 跳过证书 + 主机名校验（仅内网自签用）
    std::string ca_file;
    // TLS 握手里要登记的域名（Server Name Indication）。服务端靠它挑证书，
    // 连 IP 或连接用的 host 不在证书里时，就得在这里填证书里那个名字；填了之后证书主机名校验也按它。
    // 留空则握手不上报任何名字（和没有这个字段时一样），主机名校验退回用 host。
    std::string sni;
    unsigned int timeout_sec = 3;// 读/写各自的期限，同步与协程两条路都吃这个数；0 = 不设限
    std::string prefix;          // key 前缀（只对"确知是 key 的参数位"生效，见 apply_prefix 的 key 位表）
                                 // 这是段配置的默认值；调用方可以用 command(..., pfx) 逐条覆盖，空串表示不覆盖。
    bool isdebug = false;
};

// 单次发送命令的最大参数个数（MSET/DEL 这类批量命令按参数计，256 会静默截不了）
constexpr std::size_t kMaxArgs = 4096;

// 分支触发计数（进程累计，用于确认这条支路真的被走到，也用于线上诊断）
struct conn_counters
{
    std::atomic<unsigned long long> timeout{0};    // TLS 握手/读/写到期
    std::atomic<unsigned long long> poisoned{0};   // 连接被判脏的次数
    std::atomic<unsigned long long> prefix_skip{0};// 带 numkeys 的命令参数不足或 numkeys 不可解析，这次没加前缀
    std::atomic<unsigned long long> arg_reject{0}; // 组包被拒（参数个数 / 字节数 / 命令名含 CR/LF）
    std::atomic<unsigned long long> stale_drop{0}; // 脏/死连接被丢弃（没有回池）
};

class redis_conn_base
{
  public:
    explicit redis_conn_base(asio::io_context &ioc);
    ~redis_conn_base();

    // ===== 同步：调用者必须独占这条连接，并且在**能阻塞**的线程上调用 =====
    // 连接摘出 idle 之后独占者就是调用方；两条命令同时在同一条连接上读写会串包。
    // 这一组走的是带期限的非阻塞轮询（deadline 来自 cfg.timeout_sec），所以 socket 属于哪个
    // io_context、那个 io_context 是不是正在别的线程上跑都无关 —— 只要没有 async op 挂在它上面。
    // 池的协程通路（async_exec）不经过这一组 —— 它自己 co_await 异步读写；这一组的调用者是
    // redis_client 的同步入口，在调用者线程上就地借连接、发命令。TLS 握手吃上面那个期限；
    // TCP 建连那一步没有期限 —— 网络不通时占住调用线程。
    bool connect(const conn_config_t &cfg);
    // pfx 是"命令级前缀覆盖"：空串 = 不覆盖，用本连接所属段的 cfg_.prefix（只有两态，没有"显式不加前缀"）。
    // 加前缀这件事全层只有一个落点：apply_prefix，而它只被 prepare_command 调，同步/协程都从那儿走。
    std::optional<reply_t> command(const std::vector<std::string> &args, const std::string &pfx = "");
    bool ping();
    void close();
    bool connected() const;

    // ===== 协程：可在 HTTP 协程上下文中使用（基于 asio 非阻塞 socket）=====
    // 这一组的握手/发/收也各吃一份 cfg.timeout_sec 期限（订阅消息泵除外，它就该一直等）；
    // 和同步侧一样还没有期限的只剩 TCP 建连那一步。
    asio::awaitable<bool> async_connect(const conn_config_t &cfg);
    // 协程形参里的 pfx 一律按值：协程帧要持有它，挂起后调用方的临时串可能先没了。
    asio::awaitable<std::optional<reply_t>> async_command(const std::vector<std::string> &args,
                                                          std::string pfx = "");
    asio::awaitable<bool> async_ping();
    asio::awaitable<void> async_close();

    // ===== 便利封装 =====
    http::obj_val command_obj(const std::vector<std::string> &args, const std::string &pfx = "");
    asio::awaitable<http::obj_val> async_command_obj(const std::vector<std::string> &args, std::string pfx = "");
    // 按模式扫键。pattern 是"相对于 key 前缀"的模式：配置里设了 prefix 时，
    // 这里会自动补成 prefix+pattern，所以扫不到别的租户的键；返回的是服务端的完整键名（含前缀）。
    asio::awaitable<http::obj_val> async_scan_keys(std::string_view pattern = "*", unsigned int count = 100);

    // ===== 发布（channel 不加 key 前缀，区别于普通 key 命令）=====
    long long publish(const std::string &channel, const std::string &payload);
    asio::awaitable<long long> async_publish(const std::string &channel, const std::string &payload);

    // 脏连接：一次同步读写/协议出错后 RESP 同步已不可信，绝不回池复用
    // （独占性由连接池的 idle 表保证：谁把连接摘出 idle，谁才是它此刻唯一的使用者）
    bool is_poisoned() const { return poisoned_.load(); }

    static conn_counters &counters();

    // ===== 静态工具 =====
    // 组包；被拒时 err 给出具体原因（个数 / 字节数 / 命令名含 CR/LF），不再和"空参数"混成一串
    static std::string make_command(const std::vector<std::string> &args, std::string *err = nullptr);
    static http::obj_val to_obj_val(const reply_t &reply);

    errc last_error() const { return last_err_; }
    const std::string &last_error_msg() const { return last_msg_; }

  protected:
    // 供 redis_subscriber 复用底层收发（订阅连接独占，单连接内唯一 reader = pump）
    // due = 这一次收/发的绝对期限，由调用点的语义决定：命令-应答传 make_deadline()，
    // 要长期等的（订阅消息泵）传 std::chrono::steady_clock::time_point::max()＝不设限。
    asio::awaitable<std::optional<reply_t>> async_read_reply(std::chrono::steady_clock::time_point due);
    asio::awaitable<bool> async_write_raw(const std::string &buf, std::chrono::steady_clock::time_point due);
    std::chrono::steady_clock::time_point make_deadline() const;
    asio::io_context *raw_ioc() const { return io_ctx_; }
    const conn_config_t &config() const { return cfg_; }
    bool ssl_mode() const { return sock_type_ == 2; }

  private:
    std::optional<reply_t> read_reply_sync(const std::chrono::steady_clock::time_point &due);

    // 有期限的同步读：返回读到的字节数；ec==would_block 表示期限已到
    std::size_t read_some_bounded(unsigned char *dst, std::size_t cap, const std::chrono::steady_clock::time_point &due, asio::error_code &ec);
    // 有期限的同步全量写
    bool write_all_bounded(const std::string &buf, const std::chrono::steady_clock::time_point &due, asio::error_code &ec);
    // 有期限的同步 TLS 握手：对端不回 TLS 报文时按 due 收住
    asio::error_code handshake_bounded(const std::chrono::steady_clock::time_point &due);

    // pfx = 生效前缀（调用方负责把"空覆盖回落成 cfg_.prefix"这一步做掉），空 = 这次不加前缀。
    void apply_prefix(std::vector<std::string> &args, const std::string &pfx) const;
    void mark_poisoned(errc code, const std::string &msg);

    // 命令前置：状态检查 + 组装 send_buf_。失败返回 false（last_err_/last_msg_ 已设）。
    bool prepare_command(const std::vector<std::string> &args, const std::string &pfx = "");

    // socket 类型（0=未连, 1=tcp, 2=ssl）
    unsigned char sock_type_ = 0;

    std::atomic<bool> poisoned_{false};

    asio::io_context *io_ctx_ = nullptr;
    std::unique_ptr<asio::ip::tcp::socket> socket_;
    std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket_;
    std::shared_ptr<asio::ssl::context> ssl_context_;

    std::vector<unsigned char> read_buf_;// 累积缓冲（处理半包/粘包）
    std::size_t parse_pos_ = 0;
    std::string send_buf_;// 待发送（替代原单一 send_data 成员）
    reply_parser_t parser_;

    conn_config_t cfg_;
    asio::error_code ec_;
    std::atomic<bool> isclose_{true};
    errc last_err_ = errc::ok;
    std::string last_msg_;
};

}// namespace redis
}// namespace pz

#endif
