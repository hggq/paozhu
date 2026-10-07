/*
 * pzredis connection layer implementation
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 */
#include "redis_conn.h"

#include <asio/experimental/awaitable_operators.hpp>

#include <openssl/ssl.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <tuple>
#include <utility>

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <poll.h>
#endif

namespace pz
{
namespace redis
{

namespace
{
// 参数里 "哪些位置是 key"。表里没有的命令按 kSingle 处理（arg1 是 key）——这正是绝大多数命令的形状；
// 被显式列出来的是 arg1 之外还有 key、或 arg1 根本不是 key 的那些。
enum class kp : unsigned char
{
    kSingle,       // 只有 args[1] 是 key：GET SET SETEX SETNX INCR EXPIRE TTL LPUSH ZADD HSET ...
    kAll,          // args[1..N] 全是 key：DEL UNLINK EXISTS TOUCH MGET SINTER SDIFF SUNION WATCH
    kFirst2,       // args[1..2] 是 key，后面是成员/方向：SMOVE RPOPLPUSH LMOVE BLMOVE LCS ZRANGESTORE
    kPairs,        // 奇数位是 key：MSET MSETNX
    kAfterOp,      // args[1] 是操作符，args[2..N] 是 key：BITOP OBJECT
    kDestNumkeys,  // args[1] 目标 key，args[2] 是 numkeys，其后 numkeys 个是源 key：ZUNIONSTORE ZINTERSTORE
    kScriptNumkeys,// args[1] 是脚本，args[2] 是 numkeys，其后 numkeys 个是 KEYS：EVAL/EVALSHA/FCALL
    kAllButLast,   // args[1..N-1] 是 key，最后一格是超时：BLPOP BRPOP BZPOPMIN BZPOPMAX
    kNumkeysAt1,   // args[1] 是 numkeys，其后 numkeys 个是 key：ZUNION ZINTER ZDIFF SINTERCARD LMPOP ZMPOP
    kNumkeysAt2,   // args[1] 是超时，args[2] 是 numkeys，其后 numkeys 个是 key：BLMPOP BZMPOP
    kStoreKey,     // args[1] 是 key，关键字 STORE / STOREDIST 后面那一格也是 key：SORT GEORADIUS
    kArgIsSecond,  // args[1] 是子命令，args[2] 才是 key：XGROUP XINFO
    kStreams,      // 关键字 STREAMS 之后的**前一半**是 key，后一半是 id：XREAD
    kGroupStreams, // 同 kStreams，但 GROUP g c 三格固定在前，从 args[4] 起才找 STREAMS：XREADGROUP
    kNone          // 明确不加前缀：KEYS/SCAN 的参数是 pattern，不是 key
};

const std::vector<std::pair<const char *, kp>> kKeyPositions = {
    {"DEL", kp::kAll}, {"UNLINK", kp::kAll}, {"EXISTS", kp::kAll}, {"TOUCH", kp::kAll}, {"MGET", kp::kAll}, {"SINTER", kp::kAll}, {"SDIFF", kp::kAll}, {"SUNION", kp::kAll}, {"WATCH", kp::kAll},
    // Set 的三个 *STORE 是 `SUNIONSTORE dst key [key…]`，**没有 numkeys 这个参数**：
    // 本机 redis 8.8 实测 COMMAND GETKEYS SUNIONSTORE d s t 把 d、s、t 全当键返回，
    // 而同一发法在 ZUNIONSTORE 上不吃那个数字 —— 两族语法不同形，别照 Zset 抄。
    // 键名恰好是纯数字时（源键完全可以叫 "1"），照 numkeys 去解析它就不是"少加一格"而是静默走错键。
    {"SINTERSTORE", kp::kAll},
    {"SUNIONSTORE", kp::kAll},
    {"SDIFFSTORE", kp::kAll},
    {"SMOVE", kp::kFirst2},
    {"RPOPLPUSH", kp::kFirst2},
    {"LMOVE", kp::kFirst2},
    {"BLMOVE", kp::kFirst2},
    {"LCS", kp::kFirst2},
    {"ZRANGESTORE", kp::kFirst2},
    {"GEOSEARCHSTORE", kp::kFirst2},
    // 键搬家的那三条也是"两个键"：本机 redis 8.8 实测 COMMAND GETKEYS RENAME a b / RENAMENX a b /
    // COPY a b 都报 a、b，而 COPY a b DB 3 只报 a、b —— DB 后面那个数字不是 key，别顺手加到 args[3] 上。
    // 落默认的 kSingle 只给源键加前缀，等于把键搬出命名空间。
    {"RENAME", kp::kFirst2},
    {"RENAMENX", kp::kFirst2},
    {"COPY", kp::kFirst2},
    {"MSET", kp::kPairs},
    {"MSETNX", kp::kPairs},
    {"BITOP", kp::kAfterOp},
    {"OBJECT", kp::kAfterOp},
    // 带 numkeys 的只有 Zset 那两个 *STORE 和 EVAL 家族
    {"ZUNIONSTORE", kp::kDestNumkeys},
    {"ZINTERSTORE", kp::kDestNumkeys},
    {"EVAL", kp::kScriptNumkeys},
    {"EVALSHA", kp::kScriptNumkeys},
    {"FCALL", kp::kScriptNumkeys},
    {"BLPOP", kp::kAllButLast},
    {"BRPOP", kp::kAllButLast},
    {"BZPOPMIN", kp::kAllButLast},
    {"BZPOPMAX", kp::kAllButLast},
    {"ZUNION", kp::kNumkeysAt1},
    {"ZINTER", kp::kNumkeysAt1},
    {"ZDIFF", kp::kNumkeysAt1},
    {"SINTERCARD", kp::kNumkeysAt1},
    {"LMPOP", kp::kNumkeysAt1},
    {"ZMPOP", kp::kNumkeysAt1},
    {"BLMPOP", kp::kNumkeysAt2},
    {"BZMPOP", kp::kNumkeysAt2},
    {"SORT", kp::kStoreKey},
    {"GEORADIUS", kp::kStoreKey},
    {"GEORADIUSBYMEMBER", kp::kStoreKey},
    // Stream 族只有这四条不在默认的 kSingle 上（本机 redis 8.8 逐条 COMMAND GETKEYS 核对过）：
    // XADD/XLEN/XRANGE/XACK/XPENDING/XCLAIM/XAUTOCLAIM/XDEL/XTRIM 的 key 就是 args[1]，
    // 且 XPENDING/XCLAIM 的 group 名、XCLAIM 的 consumer 名都**不是** key，给它们加前缀就找不着组了。
    {"XGROUP", kp::kArgIsSecond},
    {"XINFO", kp::kArgIsSecond},
    {"XREAD", kp::kStreams},
    {"XREADGROUP", kp::kGroupStreams},
    {"KEYS", kp::kNone},
    {"SCAN", kp::kNone},
    {"ECHO", kp::kNone},
    {"AUTH", kp::kNone},
    {"SELECT", kp::kNone},
    {"CONFIG", kp::kNone},
    {"INFO", kp::kNone},
    {"CLIENT", kp::kNone},
    {"PUBLISH", kp::kNone},
    {"SUBSCRIBE", kp::kNone},
    {"UNSUBSCRIBE", kp::kNone},
    {"PSUBSCRIBE", kp::kNone},
    {"PUNSUBSCRIBE", kp::kNone},
    {"MULTI", kp::kNone},
    {"EXEC", kp::kNone},
    {"DISCARD", kp::kNone},
    {"UNWATCH", kp::kNone},
    {"FLUSHALL", kp::kNone},
    {"FLUSHDB", kp::kNone},
    {"SCRIPT", kp::kNone},
    {"ACL", kp::kNone},
    // FUNCTION 的参数 1 是函数正文不是 key（服务端 COMMAND GETKEYS 也报"无 key"）；
    // 它不在表里就会走默认的 kSingle，把正文加前缀改坏。fcall 的 KEYS 段另有 kScriptNumkeys。
    {"FUNCTION", kp::kNone}};

kp lookup_keypos(const std::string &cmd)
{
    for (const auto &e : kKeyPositions)
        if (cmd == e.first)
            return e.second;
    return kp::kSingle;
}

std::string toupper_cmd(std::string s)
{
    for (auto &c : s)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

bool has_crlf(const std::string &s)
{
    return s.find('\r') != std::string::npos || s.find('\n') != std::string::npos;
}

// numkeys 参数：解析失败返回 -1（调用方计一次 prefix_skip，不加前缀）
long long parse_numkeys(const std::string &s)
{
    try
    {
        std::size_t pos = 0;
        long long n     = std::stoll(s, &pos);
        if (pos != s.size() || n < 0)
            return -1;
        return n;
    }
    catch (...)
    {
        return -1;
    }
}

// numkeys 落在 args[numkeys_at]，它后面紧跟 numkeys 格是 key。
// 参数不够或 numkeys 不是非负整数 ⇒ 一格都不加前缀，计一次 prefix_skip（宁可少加不可加错）。
void prefix_numkeys_block(std::vector<std::string> &out, std::size_t numkeys_at, const std::string &prefix)
{
    if (out.size() <= numkeys_at)
    {
        redis_conn_base::counters().prefix_skip.fetch_add(1);
        return;
    }
    long long n = parse_numkeys(out[numkeys_at]);
    if (n < 0)
    {
        redis_conn_base::counters().prefix_skip.fetch_add(1);
        return;
    }
    for (long long i = 0; i < n && numkeys_at + 1 + static_cast<std::size_t>(i) < out.size(); ++i)
    {
        std::size_t at = numkeys_at + 1 + static_cast<std::size_t>(i);
        out[at]        = prefix + out[at];
    }
}

// SORT / GEORADIUS 家族的 `STORE 目标键` 与 `STOREDIST 目标键`。
// 关键字**整格精确比较**：用前缀匹配的话 "STORE" 会把 "STOREDIST" 一起吃掉，目标键反而漏加。
// 两种关键字都扫（GEORADIUS 允许同时带 STORE 和 STOREDIST），命中就加、不 break。
// search_from 由调用方给"关键字可能从哪一格出现"，免得把源键名当成关键字。
// ZUNIONSTORE / ZINTERSTORE 不走这里：本机 redis 8.8 实测这两条带 STOREDIST 直接 ERR syntax error，
// 服务端收不下的写法扫了也到不了键。
void prefix_store_targets(std::vector<std::string> &out, std::size_t search_from, const std::string &prefix)
{
    for (std::size_t i = search_from; i + 1 < out.size(); ++i)
    {
        std::string up = toupper_cmd(out[i]);
        if (up == "STORE" || up == "STOREDIST")
        {
            // 分两句写，不要 `out[++i] = prefix + out[i]`：C++17 起赋值右边先算，
            // 那样读到的 out[i] 还是关键字本身，等于把 "STORE" 写进了目标键那一格。
            ++i;
            out[i] = prefix + out[i];
        }
    }
}

// XREAD / XREADGROUP：键名段在关键字 STREAMS 之后，且和 id 段是**平行数组**
// （STREAMS k1 k2 id1 id2），所以只有"其后 half 格"是 key。
// search_from 是查找起点：XREAD 从 args[1] 起，XREADGROUP 的 GROUP <组名> <消费者> 三格固定在前，
// 从 args[4] 起才找 —— 否则组名恰好叫 STREAMS 时会命中组名那一格。
// 找不到 STREAMS、或其后格数是 0/奇数（服务端自己会报 Unbalanced 'xread'）⇒ 一格不加 + 计 prefix_skip，
// 与 prefix_numkeys_block 同一口径：宁可少加，不可加错（加到 id 段上就是改了读游标）。
void prefix_streams_block(std::vector<std::string> &out, std::size_t search_from, const std::string &prefix)
{
    std::size_t sep = out.size();
    for (std::size_t i = search_from; i < out.size(); ++i)
    {
        if (toupper_cmd(out[i]) == "STREAMS")
        {
            sep = i;
            break;
        }
    }
    std::size_t tail = sep + 1 < out.size() ? out.size() - sep - 1 : 0;
    if (sep == out.size() || tail == 0 || tail % 2 != 0)
    {
        redis_conn_base::counters().prefix_skip.fetch_add(1);
        return;
    }
    for (std::size_t i = sep + 1; i < sep + 1 + tail / 2; ++i)
        out[i] = prefix + out[i];
}

// timeout_sec == 0 表示"不设限"：期限取远未来，轮询循环只会被数据或 isclose_ 叫醒
std::chrono::steady_clock::time_point deadline_for(unsigned int sec)
{
    if (sec == 0)
        return std::chrono::steady_clock::time_point::max();
    return std::chrono::steady_clock::now() + std::chrono::seconds(sec);
}

// 距期限还剩多少毫秒；-1 = 不设限。deadline_for(0) 给的是 time_point::max()，
// 直接和 now() 做差会溢出，所以先把它单独认出来。
long long remaining_ms(const std::chrono::steady_clock::time_point &due)
{
    if (due == std::chrono::steady_clock::time_point::max())
        return -1;
    auto now = std::chrono::steady_clock::now();
    if (now >= due)
        return 0;
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(due - now).count();
    return ms;
}

// 会阻塞到描述符就绪或期限到点；true = 就绪（再调一次那个同步操作就会往前走），
// false = 期限已到 / fd 出错，调用方照 would_block 那条分支收口。
// 用它取代 yield() 空转：期限检查点还是 would_block（blocking 描述符上 asio 的同步
// read_some 不吃内核 SO_RCVTIMEO，见 connect() 里那段注释），但等的时候不烧 CPU。
enum class fd_evt : unsigned char
{
    read,
    write
};

// 一次内核等待的三种结局。分成"期限到点"和"被打断"两类，是为了让 wait_fd 的期限循环
// 只有一处：平台差异全收在 fd_poll 里，重试规则和 ms 换算不在两份代码里各写一遍。
enum class poll_out : unsigned char
{
    ready,  // 描述符就绪
    expired,// 期限到点，或者 fd 本身出错（调用方按超时收口）
    intr    // 被信号/中断打断，重试
};

// poll 和 WSAPoll 的 timeout 都是 int 毫秒，上限 2^31-1 ms ≈ 24.8 天。
// 换算必须先钳位：conf 里一个过大的 timeout（读进 timeout_sec 时它是合法的 unsigned int）
// 截成 int 会变**负数**，而负数在 poll 的口径里是"不设限" —— 期限会静默消失，
// 看起来配置生效了，实际那条读写永久挂住。-1 只留给 remaining_ms 那条真正的"不设限"。
int poll_timeout_ms(long long ms)
{
    constexpr long long int_max_ms = 2147483647LL;
    if (ms < 0)
        return -1;
    return static_cast<int>(ms > int_max_ms ? int_max_ms : ms);
}

#if defined(_WIN32)
poll_out fd_poll(long long fd, fd_evt evt, long long ms)
{
    WSAPOLLFD pfd;
    pfd.fd      = static_cast<SOCKET>(static_cast<uintptr_t>(fd));
    pfd.events  = static_cast<SHORT>((evt == fd_evt::read) ? POLLIN : POLLOUT);
    pfd.revents = 0;
    // WSAPoll 自 Vista 起就有；本项目的 Windows 档在 CMake 里定到 _WIN32_WINNT=0x0601
    int r = ::WSAPoll(&pfd, 1, poll_timeout_ms(ms));
    if (r > 0)
        return poll_out::ready;
    if (r == SOCKET_ERROR && WSAGetLastError() == WSAEINTR)
        return poll_out::intr;
    return poll_out::expired;// r==0 是期限到点；非中断的出错也按到点收口
}
#else
poll_out fd_poll(long long fd, fd_evt evt, long long ms)
{
    struct pollfd pfd;
    pfd.fd      = static_cast<int>(fd);
    pfd.events  = (evt == fd_evt::read) ? POLLIN : POLLOUT;
    pfd.revents = 0;
    int r       = ::poll(&pfd, 1, poll_timeout_ms(ms));
    if (r > 0)
        return poll_out::ready;
    if (r < 0 && errno == EINTR)
        return poll_out::intr;
    return poll_out::expired;// r==0 是期限到点；非 EINTR 的出错也按到点收口
}
#endif

bool wait_fd(long long fd, fd_evt evt, const std::chrono::steady_clock::time_point &due)
{
    for (;;)
    {
        long long ms = remaining_ms(due);
        if (ms == 0)
            return false;
        poll_out r = fd_poll(fd, evt, ms);
        if (r == poll_out::ready)
            return true;
        if (r == poll_out::intr)
            continue;
        return false;
    }
}

// 协程 TLS 握手的期限那一侧：true = 确实是期限到点，false = 握手先成了、这一侧被取消
asio::awaitable<bool> hs_deadline_hit(asio::steady_timer &t)
{
    asio::error_code tec;
    co_await t.async_wait(asio::redirect_error(asio::use_awaitable, tec));
    co_return !tec;
}
}// namespace

conn_counters &redis_conn_base::counters()
{
    static conn_counters instance;
    return instance;
}

void redis_conn_base::mark_poisoned(errc code, const std::string &msg)
{
    last_err_ = code;
    if (!msg.empty())
        last_msg_ = msg;
    if (!poisoned_.load())
    {
        poisoned_.store(true);
        counters().poisoned.fetch_add(1);
    }
}

redis_conn_base::redis_conn_base(asio::io_context &ioc) : io_ctx_(&ioc) {}

redis_conn_base::~redis_conn_base()
{
    close();
}

// ===================== make_command（D1 二进制安全 + D7 拒绝原因分态）=====================
std::string redis_conn_base::make_command(const std::vector<std::string> &args, std::string *err)
{
    auto reject = [&](const std::string &why) -> std::string
    {
        counters().arg_reject.fetch_add(1);
        if (err)
            *err = why;
        return "";
    };

    if (args.empty())
        return reject("empty args");
    if (args.size() > kMaxArgs)
        return reject("args count " + std::to_string(args.size()) + " > limit " + std::to_string(kMaxArgs));
    // 只有命令名是协议位。值里有 CR/LF 是合法数据：RESP 按长度前缀分帧，实测
    // 写入 "line1\r\nline2\r\nINCR evil\r\n" 后逐字节回读一致，且 INCR evil 没有被当成命令执行。
    if (has_crlf(args[0]))
        return reject("command name contains CR/LF");

    unsigned long long bytes = 0;
    for (const auto &a : args)
    {
        bytes += a.size();
        if (bytes > reply_parser_t::kMaxBulk)
            return reject("total bytes " + std::to_string(bytes) + " > limit " + std::to_string(reply_parser_t::kMaxBulk));
    }

    std::string cmd;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "*%zu\r\n", args.size());
    cmd += buf;
    for (const auto &arg : args)
    {
        std::snprintf(buf, sizeof(buf), "$%zu\r\n", arg.size());
        cmd += buf;
        cmd += arg;
        cmd += "\r\n";
    }
    if (err)
        err->clear();
    return cmd;
}

// ===================== key 前缀（D2 显式 key 位表，不再"除命令名外全部加"）=====================
// pfx 由调用方给"生效前缀"：命令级覆盖非空就用它，否则用段配置。这里收到空串就是真的不加。
void redis_conn_base::apply_prefix(std::vector<std::string> &out, const std::string &pfx) const
{
    if (pfx.empty() || out.size() < 2)
        return;
    auto add = [&](std::size_t i)
    {
        if (i < out.size())
            out[i] = pfx + out[i];
    };
    switch (lookup_keypos(toupper_cmd(out[0])))
    {
    case kp::kAll:
        for (std::size_t i = 1; i < out.size(); ++i)
            add(i);
        break;
    case kp::kFirst2:
        add(1);
        add(2);
        break;
    case kp::kPairs:
        for (std::size_t i = 1; i < out.size(); i += 2)
            add(i);
        break;
    case kp::kAfterOp:
        for (std::size_t i = 2; i < out.size(); ++i)
            add(i);
        break;
    case kp::kDestNumkeys:
        add(1);
        prefix_numkeys_block(out, 2, pfx);
        break;
    case kp::kScriptNumkeys:
        // args[1] 是脚本正文，绝不能当 key 加前缀；只有 KEYS 段加
        prefix_numkeys_block(out, 2, pfx);
        break;
    case kp::kNumkeysAt1:
        prefix_numkeys_block(out, 1, pfx);
        break;
    case kp::kNumkeysAt2:
        // args[1] 是超时（秒），不是 key
        prefix_numkeys_block(out, 2, pfx);
        break;
    case kp::kAllButLast:
        // 最后一格是超时，按服务端形状它不算 key
        for (std::size_t i = 1; i + 1 < out.size(); ++i)
            add(i);
        break;
    case kp::kStoreKey:
        // args[1] 是源键；STORE / STOREDIST 后面那一格是被写入的目标键
        add(1);
        prefix_store_targets(out, 2, pfx);
        break;
    case kp::kArgIsSecond:
        // args[1] 是 CREATE / STREAM / GROUPS / CONSUMERS 这类子命令名，key 在它后面
        add(2);
        break;
    case kp::kStreams:
        prefix_streams_block(out, 1, pfx);
        break;
    case kp::kGroupStreams:
        prefix_streams_block(out, 4, pfx);
        break;
    case kp::kNone:
        break;
    case kp::kSingle:
    default:
        add(1);
        break;
    }
}

// ===================== 同步连接 =====================
bool redis_conn_base::connect(const conn_config_t &cfg)
{
    last_err_ = errc::ok;
    last_msg_.clear();
    // 重连必须从干净状态开始：脏位、占位、上一轮的残留缓冲都要归零
    poisoned_.store(false);
    read_buf_.clear();
    parse_pos_ = 0;
    cfg_       = cfg;
    if (!socket_)
        socket_ = std::make_unique<asio::ip::tcp::socket>(*io_ctx_);

    asio::ip::tcp::resolver resolver(*io_ctx_);
    asio::ip::tcp::resolver::results_type endpoints;
    try
    {
        endpoints = resolver.resolve(cfg.host, std::to_string(cfg.port));// 抛异常兜底
    }
    catch (const std::exception &e)
    {
        last_err_ = errc::resolve;
        last_msg_ = e.what();
        isclose_.store(true);
        return false;
    }

    asio::connect(*socket_, endpoints, ec_);
    if (ec_)
    {
        last_err_ = errc::connect;
        last_msg_ = ec_.message();
        isclose_.store(true);
        return false;
    }
    socket_->set_option(asio::ip::tcp::no_delay(true));
    sock_type_ = 1;

    // 同步路径的期限靠"歇拍轮询"实现，需要描述符处于 non_blocking：
    // 实测 asio 的同步 read_some 不吃内核 SO_RCVTIMEO（设 2s 仍 >6s 不返回），而 non_blocking 下立刻返回 would_block。
    // TLS 握手和读写用的是同一批 asio 同步操作，所以这一步必须赶在握手之前——
    // 留在握手之后就等于握手期还是一次阻塞 OS recv，对端不回 TLS 报文就永久挂住（timeout_sec 管不上）。
    // 实测 O_NONBLOCK 熬得过 std::move 进 ssl stream。
    // 协程路径（async_*）不受影响：asio 的异步操作本来就跑在非阻塞描述符上。
    // sync path 想要的是"直接阻塞、不空转"，但期限检查点只能长在 would_block 上：
    // 描述符一旦是 blocking，read_some 就卡在 OS 里，read_some_bounded 的 now()>=due 那次比较
    // 永远到不了（timeout_sec 与 counters().timeout 双双失效）。
    // 所以这里保持 non_blocking，空转的那一拍交给 wait_fd() 的内核等待去做。
    asio::error_code nb_ec;
    socket_->non_blocking(true, nb_ec);

    if (cfg.isssl)
    {
        // 证书里的域名和连接用的 host 常常不是一个（连 IP、或走内网别名），这时在 conf 里登记 sni：
        // 握手时上报的就是它，校验证书也按它。sni 留空则一个字节都不多发（老行为），
        // 主机名校验退回用 host —— 把 "127.0.0.1" 这类 IP 当 server name 发出去是不合规的。
        const std::string verify_host = cfg.sni.empty() ? cfg.host : cfg.sni;
        try
        {
            ssl_context_ = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
            if (!cfg.insecure)// 先配置 verify 再 handshake + 主机名校验
            {
                if (!cfg.ca_file.empty())
                    ssl_context_->load_verify_file(cfg.ca_file);
                else
                    ssl_context_->set_default_verify_paths();
                ssl_context_->set_verify_mode(asio::ssl::verify_peer);
                ssl_context_->set_verify_callback(asio::ssl::host_name_verification(verify_host));
            }
            else
            {
                ssl_context_->set_verify_mode(asio::ssl::verify_none);
            }
            // release() 交出的描述符先由一个有主的 unique_ptr 握住，再 move 进 ssl stream：
            // stream 的构造本身也可能抛（分配、SSL 对象创建），抛了要有人把那个 fd 关掉。
            // move 之后源对象持的是空描述符，出作用域不会重复关。
            std::unique_ptr<asio::ip::tcp::socket> plain_sock(socket_.release());
            sslsocket_ = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(*plain_sock), *ssl_context_);
        }
        catch (const std::exception &e)
        {
            last_err_    = errc::connect;
            last_msg_    = std::string("tls init failed: ") + e.what();
            ssl_context_ = nullptr;
            isclose_.store(true);
            return false;
        }
        sslsocket_->lowest_layer().set_option(asio::ip::tcp::no_delay(true));
        if (!cfg.sni.empty())
            SSL_set_tlsext_host_name(sslsocket_->native_handle(), cfg.sni.c_str());
        sock_type_ = 2;
        ec_        = handshake_bounded(deadline_for(cfg.timeout_sec));
        if (ec_)
        {
            // 对端不回 TLS 报文才算期限；证书/主机名/协议错误是立刻失败，不占满一个期限
            if (ec_ == asio::error::timed_out)
            {
                last_err_ = errc::timeout;
                last_msg_ = "ssl handshake timeout after " + std::to_string(cfg.timeout_sec) + "s";
                counters().timeout.fetch_add(1);
            }
            else
            {
                last_err_ = errc::connect;
                last_msg_ = ec_.message();
            }
            isclose_.store(true);
            return false;
        }
    }

    isclose_.store(false);

    if (!cfg.password.empty())// AUTH 校验 reply
    {
        std::optional<reply_t> r;
        if (cfg.username.empty())
            r = command({"AUTH", cfg.password});
        else
            r = command({"AUTH", cfg.username, cfg.password});
        if (!r || r->is_error())
        {
            last_err_ = errc::auth;
            if (r)
                last_msg_ = r->str_value;
            isclose_.store(true);
            return false;
        }
    }
    if (cfg.dbindex > 0)
    {
        auto r = command({"SELECT", std::to_string(cfg.dbindex)});
        if (!r || r->is_error())
        {
            last_err_ = errc::select;
            if (r)
                last_msg_ = r->str_value;
            isclose_.store(true);
            return false;
        }
    }
    return true;
}

// ===================== 协程连接 =====================
asio::awaitable<bool> redis_conn_base::async_connect(const conn_config_t &cfg)
{
    last_err_ = errc::ok;
    last_msg_.clear();
    poisoned_.store(false);
    read_buf_.clear();
    parse_pos_ = 0;
    cfg_       = cfg;
    if (!socket_)
        socket_ = std::make_unique<asio::ip::tcp::socket>(*io_ctx_);

    asio::ip::tcp::resolver resolver(*io_ctx_);
    auto [resolve_ec, endpoints] = co_await resolver.async_resolve(cfg.host, std::to_string(cfg.port), asio::as_tuple(asio::use_awaitable));
    if (resolve_ec)
    {
        last_err_ = errc::resolve;
        last_msg_ = resolve_ec.message();
        isclose_.store(true);
        co_return false;
    }
    bool linked = false;
    for (auto &ep : endpoints)// 每个 endpoint 推进迭代器，避免死循环
    {
        std::tie(ec_) = co_await socket_->async_connect(ep, asio::as_tuple(asio::use_awaitable));
        if (!ec_)
        {
            linked = true;
            break;
        }
    }
    if (!linked)
    {
        last_err_ = errc::connect;
        last_msg_ = ec_.message();
        isclose_.store(true);
        co_return false;
    }
    socket_->set_option(asio::ip::tcp::no_delay(true));
    sock_type_ = 1;
    // 与同步 connect() 一致：必须置 non_blocking。异步连接会进入和同步连接同一个 idle 池，
    // 若留作 blocking，同步命令路径借到它后 read_some 卡在 OS 里不返回 would_block，
    // read_some_bounded 的期限检查（now()>=due）永远到不了，timeout_sec 失效、业务线程永久挂死。
    // 异步收发操作在非阻塞描述符上行为不变。
    {
        asio::error_code nb_ec;
        socket_->non_blocking(true, nb_ec);
    }

    if (cfg.isssl)
    {
        // 同同步版：sni 登记了才上报，留空一个字节都不多发；主机名校验按 verify_host
        const std::string verify_host = cfg.sni.empty() ? cfg.host : cfg.sni;
        try
        {
            ssl_context_ = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
            if (!cfg.insecure)
            {
                if (!cfg.ca_file.empty())
                    ssl_context_->load_verify_file(cfg.ca_file);
                else
                    ssl_context_->set_default_verify_paths();
                ssl_context_->set_verify_mode(asio::ssl::verify_peer);
                ssl_context_->set_verify_callback(asio::ssl::host_name_verification(verify_host));
            }
            else
            {
                ssl_context_->set_verify_mode(asio::ssl::verify_none);
            }
            // 与同步版同一口径：release() 交出的描述符先由有主的 unique_ptr 握住再 move 进
            // ssl stream，构造抛了也不会留下一个没人关的 fd。
            std::unique_ptr<asio::ip::tcp::socket> plain_sock(socket_.release());
            sslsocket_ = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(*plain_sock), *ssl_context_);
        }
        catch (const std::exception &e)
        {
            last_err_    = errc::connect;
            last_msg_    = std::string("tls init failed: ") + e.what();
            ssl_context_ = nullptr;
            isclose_.store(true);
            co_return false;
        }
        if (!cfg.sni.empty())
            SSL_set_tlsext_host_name(sslsocket_->native_handle(), cfg.sni.c_str());
        sock_type_ = 2;
        if (cfg.timeout_sec == 0)
        {
            std::tie(ec_) = co_await sslsocket_->async_handshake(asio::ssl::stream_base::client,
                                                                 asio::as_tuple(asio::use_awaitable));
        }
        else
        {
            // 握手和一只 timeout_sec 的表同时跑，先到者说了算。表先到时 asio 会取消握手并等它收摊
            // （实测：被取消的那侧确实以 operation_aborted 结束，reactor 上不留残留 op）。
            using namespace asio::experimental::awaitable_operators;
            asio::steady_timer hs_timer(*io_ctx_);
            hs_timer.expires_after(std::chrono::seconds(cfg.timeout_sec));
            auto first = co_await (
                sslsocket_->async_handshake(asio::ssl::stream_base::client,
                                            asio::as_tuple(asio::use_awaitable)) ||
                hs_deadline_hit(hs_timer));
            ec_ = first.index() == 0 ? std::get<0>(std::get<0>(first)) : asio::error::timed_out;
        }
        if (ec_)
        {
            // 与同步侧同一套口径：期限到点报 timeout，其余算连接阶段失败
            if (ec_ == asio::error::timed_out)
            {
                last_err_ = errc::timeout;
                last_msg_ = "ssl handshake timeout after " + std::to_string(cfg.timeout_sec) + "s";
                counters().timeout.fetch_add(1);
            }
            else
            {
                last_err_ = errc::connect;
                last_msg_ = ec_.message();
            }
            isclose_.store(true);
            co_return false;
        }
    }

    isclose_.store(false);

    if (!cfg.password.empty())
    {
        std::vector<std::string> a = cfg.username.empty() ? std::vector<std::string>{"AUTH", cfg.password} : std::vector<std::string>{"AUTH", cfg.username, cfg.password};
        auto r                     = co_await async_command(a);
        if (!r || r->is_error())
        {
            last_err_ = errc::auth;
            if (r)
                last_msg_ = r->str_value;
            isclose_.store(true);
            co_return false;
        }
    }
    if (cfg.dbindex > 0)
    {
        auto r = co_await async_command({"SELECT", std::to_string(cfg.dbindex)});
        if (!r || r->is_error())
        {
            last_err_ = errc::select;
            if (r)
                last_msg_ = r->str_value;
            isclose_.store(true);
            co_return false;
        }
    }
    co_return true;
}

// ===================== 有期限的同步读写（D3）=====================
std::size_t redis_conn_base::read_some_bounded(unsigned char *dst, std::size_t cap, const std::chrono::steady_clock::time_point &due, asio::error_code &out_ec)
{
    while (true)
    {
        if (sock_type_ == 1)
        {
            std::size_t n = socket_->read_some(asio::buffer(dst, cap), out_ec);
            if (!out_ec)
                return n;
        }
        else if (sock_type_ == 2)
        {
            std::size_t n = sslsocket_->read_some(asio::buffer(dst, cap), out_ec);
            if (!out_ec)
                return n;
        }
        else
        {
            out_ec = asio::error::not_connected;
            return 0;
        }

        if (out_ec != asio::error::would_block && out_ec != asio::error::try_again)
            return 0;// 真错误（eof / connection reset ...），交调用方分辨
        if (isclose_.load())
        {
            out_ec = asio::error::operation_aborted;
            return 0;
        }
        if (std::chrono::steady_clock::now() >= due)
            return 0;// out_ec 仍是 would_block == 期限已到
        // 等可读或期限到点，不空转。TLS 的 native_handle() 是 SSL*，fd 在 lowest_layer 上。
        long long fd = (sock_type_ == 1) ? static_cast<long long>(socket_->native_handle()) : static_cast<long long>(sslsocket_->lowest_layer().native_handle());
        if (!wait_fd(fd, fd_evt::read, due))
            return 0;// 期限已到（out_ec 仍是 would_block）
    }
}

bool redis_conn_base::write_all_bounded(const std::string &buf, const std::chrono::steady_clock::time_point &due, asio::error_code &out_ec)
{
    std::size_t sent = 0;
    while (sent < buf.size())
    {
        auto remaining = asio::buffer(buf.data() + sent, buf.size() - sent);
        std::size_t n  = 0;
        if (sock_type_ == 1)
            n = socket_->write_some(remaining, out_ec);
        else if (sock_type_ == 2)
            n = sslsocket_->write_some(remaining, out_ec);
        else
        {
            out_ec = asio::error::not_connected;
            return false;
        }
        if (!out_ec)
        {
            sent += n;
            continue;
        }
        if (out_ec != asio::error::would_block && out_ec != asio::error::try_again)
            return false;
        if (isclose_.load())
        {
            out_ec = asio::error::operation_aborted;
            return false;
        }
        if (std::chrono::steady_clock::now() >= due)
            return false;// 期限已到（对端不收 / 发送缓冲满）
        // 等可写或期限到点，不空转（fd 同读侧：TLS 走 lowest_layer）
        long long fd = (sock_type_ == 1) ? static_cast<long long>(socket_->native_handle()) : static_cast<long long>(sslsocket_->lowest_layer().native_handle());
        if (!wait_fd(fd, fd_evt::write, due))
            return false;
    }
    out_ec.clear();
    return true;
}

// ===================== 有期限的同步 TLS 握手 =====================
// non_blocking 上 asio::ssl::stream::handshake 看不到报文立刻 would_block，
// 所以和读写一样走"歇拍轮询 + 期限"。
// 用 SSL_want_write 判定本次握手想要的方向（读 or 写），精确等待对应事件。
asio::error_code redis_conn_base::handshake_bounded(const std::chrono::steady_clock::time_point &due)
{
    asio::error_code hec;
    while (true)
    {
        hec.clear();
        sslsocket_->handshake(asio::ssl::stream_base::client, hec);
        if (!hec)
            return hec;// 握完了
        if (hec != asio::error::would_block && hec != asio::error::try_again)
            return hec;// 真失败，立刻返回
        // 握手期连接独占，isclose_ 本来就是 true，这里不看它。
        if (std::chrono::steady_clock::now() >= due)
            return asio::error::timed_out;
        // 按 SSL 实际想要的方向等。
        SSL *ssl    = sslsocket_->native_handle();
        fd_evt want = SSL_want_write(ssl) ? fd_evt::write : fd_evt::read;
        if (!wait_fd(static_cast<long long>(sslsocket_->lowest_layer().native_handle()), want, due))
            return asio::error::timed_out;
    }
}

std::optional<reply_t> redis_conn_base::read_reply_sync(const std::chrono::steady_clock::time_point &due)
{
    reply_t reply;
    while (true)
    {
        std::size_t consumed = 0;
        parse_status st      = parser_.feed(read_buf_.data() + parse_pos_,
                                       read_buf_.size() - parse_pos_,
                                       consumed,
                                       reply);
        if (st == parse_status::ok)
        {
            parse_pos_ += consumed;
            if (parse_pos_ > 0)
            {
                read_buf_.erase(read_buf_.begin(), read_buf_.begin() + parse_pos_);
                parse_pos_ = 0;
            }
            return reply;
        }
        if (st == parse_status::error)
        {
            // 帧已经不完整/不合法：残留字节必须作废，否则下一条命令会解析到上一条的尾巴
            mark_poisoned(errc::protocol, "protocol parse error");
            return std::nullopt;
        }
        if (isclose_.load())
        {
            mark_poisoned(errc::read, "connection closed");
            return std::nullopt;
        }
        unsigned char tmp[16384];
        asio::error_code iec;
        std::size_t n = read_some_bounded(tmp, sizeof(tmp), due, iec);
        if (n == 0)
        {
            if (iec == asio::error::would_block)
            {
                counters().timeout.fetch_add(1);
                mark_poisoned(errc::timeout, "read timeout after " + std::to_string(cfg_.timeout_sec) + "s");
            }
            else if (iec)
                mark_poisoned(errc::read, iec.message());
            else
                mark_poisoned(errc::read, "peer closed without data");
            return std::nullopt;
        }
        read_buf_.insert(read_buf_.end(), tmp, tmp + n);
        if (read_buf_.size() - parse_pos_ > reply_parser_t::kMaxBulk)
        {
            mark_poisoned(errc::protocol, "reply exceeds kMaxBulk");
            return std::nullopt;
        }
    }
}

std::optional<reply_t> redis_conn_base::command(const std::vector<std::string> &args, const std::string &pfx)
{
    if (!prepare_command(args, pfx))
        return std::nullopt;
    asio::error_code wec;
    if (!write_all_bounded(send_buf_, deadline_for(cfg_.timeout_sec), wec))
    {
        if (wec == asio::error::would_block)
        {
            counters().timeout.fetch_add(1);
            mark_poisoned(errc::timeout, "write timeout after " + std::to_string(cfg_.timeout_sec) + "s");
        }
        else
            mark_poisoned(errc::write, wec.message());
        return std::nullopt;
    }
    return read_reply_sync(deadline_for(cfg_.timeout_sec));
}

bool redis_conn_base::prepare_command(const std::vector<std::string> &args, const std::string &pfx)
{
    last_err_ = errc::ok;
    last_msg_.clear();
    if (isclose_.load() || sock_type_ == 0)
    {
        last_err_ = errc::connect;
        last_msg_ = "not connected";
        return false;
    }
    if (poisoned_.load())
    {
        last_err_ = errc::connect;
        last_msg_ = "connection poisoned";
        return false;
    }
    std::vector<std::string> real = args;
    // 回落只在这一行：命令级覆盖是空串才吃段配置。加前缀没有第二个入口。
    apply_prefix(real, pfx.empty() ? cfg_.prefix : pfx);
    std::string build_err;
    send_buf_ = make_command(real, &build_err);
    if (send_buf_.empty())
    {
        last_err_ = errc::bad_args;
        last_msg_ = build_err;
        return false;
    }
    return true;
}

bool redis_conn_base::ping()
{
    auto r = command({"PING"});
    return r && !r->is_error();
}

// ===================== 协程收发 =====================
std::chrono::steady_clock::time_point redis_conn_base::make_deadline() const
{
    return deadline_for(cfg_.timeout_sec);
}

asio::awaitable<std::optional<reply_t>> redis_conn_base::async_read_reply(std::chrono::steady_clock::time_point due)
{
    reply_t reply;
    // 期限由调用点给：due == time_point::max() 就是"不设限"的情况（timeout_sec == 0 时
    // make_deadline() 也给出它）。这一支不建表也不 race，直接 co_await 那一发读，
    // 与加期限之前的行为逐字节同一条路 —— 订阅消息泵长期等推送靠的就是它。
    // 有期限时整条回复共用这一个绝对到期时刻：循环里每次等的是"距 due 还剩多少"，
    // 不是每读一次重新给 timeout_sec 秒（与同步版 read_reply_sync(due) 同口径）。
    asio::steady_timer read_timer(*io_ctx_);
    const bool bounded = (due != std::chrono::steady_clock::time_point::max());
    if (bounded)
        read_timer.expires_at(due);
    while (true)
    {
        std::size_t consumed = 0;
        parse_status st      = parser_.feed(read_buf_.data() + parse_pos_,
                                       read_buf_.size() - parse_pos_,
                                       consumed,
                                       reply);
        if (st == parse_status::ok)
        {
            parse_pos_ += consumed;
            if (parse_pos_ > 0)
            {
                read_buf_.erase(read_buf_.begin(), read_buf_.begin() + parse_pos_);
                parse_pos_ = 0;
            }
            co_return reply;
        }
        if (st == parse_status::error)
        {
            last_err_ = errc::protocol;
            last_msg_ = "protocol parse error";
            co_return std::nullopt;
        }
        if (isclose_.load())
        {
            last_err_ = errc::read;
            last_msg_ = "connection closed";
            co_return std::nullopt;
        }
        unsigned char tmp[16384];
        std::size_t n = 0;
        try
        {
            asio::awaitable<std::size_t> reading = (sock_type_ == 1) ? socket_->async_read_some(asio::buffer(tmp, sizeof(tmp)), asio::use_awaitable) : sslsocket_->async_read_some(asio::buffer(tmp, sizeof(tmp)), asio::use_awaitable);
            if (!bounded)
            {
                n = co_await std::move(reading);
            }
            else
            {
                using namespace asio::experimental::awaitable_operators;
                // 表先到时 asio 会取消那一发读并等它收摊（被取消的一侧以 operation_aborted 结束，
                // reactor 上不留残留 op），所以复用同一只表进下一次循环是安全的。
                auto first = co_await (std::move(reading) || hs_deadline_hit(read_timer));
                if (first.index() == 1)
                {
                    counters().timeout.fetch_add(1);
                    last_err_ = errc::timeout;
                    last_msg_ = "read timeout after " + std::to_string(cfg_.timeout_sec) + "s";
                    co_return std::nullopt;
                }
                n = std::get<0>(first);
            }
        }
        catch (const std::exception &e)
        {
            last_err_ = errc::read;
            last_msg_ = e.what();
            co_return std::nullopt;
        }
        if (n == 0)
        {
            last_err_ = errc::read;
            co_return std::nullopt;
        }
        read_buf_.insert(read_buf_.end(), tmp, tmp + n);
        if (read_buf_.size() - parse_pos_ > reply_parser_t::kMaxBulk)
        {
            last_err_ = errc::protocol;
            co_return std::nullopt;
        }
    }
}

asio::awaitable<std::optional<reply_t>> redis_conn_base::async_command(const std::vector<std::string> &args,
                                                                       std::string pfx)
{
    if (!prepare_command(args, pfx))
        co_return std::nullopt;
    // 写和读各取一份期限，与同步版 command()（write_all_bounded + read_reply_sync 各一次
    // deadline_for(timeout_sec)）逐字同口径。
    if (!(co_await async_write_raw(send_buf_, make_deadline())))
    {
        mark_poisoned(last_err_, last_msg_);// 写失败/到点都可能只发了一半
        co_return std::nullopt;
    }
    auto r = co_await async_read_reply(make_deadline());
    if (!r)
        mark_poisoned(last_err_, last_msg_);
    co_return r;
}

asio::awaitable<bool> redis_conn_base::async_ping()
{
    auto r = co_await async_command({"PING"});
    co_return (r && !r->is_error());
}

// ===================== 原始写（供 redis_subscriber 复用）=====================
asio::awaitable<bool> redis_conn_base::async_write_raw(const std::string &buf, std::chrono::steady_clock::time_point due)
{
    if (sock_type_ != 1 && sock_type_ != 2)
        co_return false;
    // 到点就是"这一发写出去多少不可知"：调用方按写失败收口，连接判脏（见 async_command / async_publish）
    asio::steady_timer write_timer(*io_ctx_);
    const bool bounded = (due != std::chrono::steady_clock::time_point::max());
    if (bounded)
        write_timer.expires_at(due);
    try
    {
        asio::awaitable<std::size_t> writing = (sock_type_ == 1) ? asio::async_write(*socket_, asio::buffer(buf), asio::use_awaitable) : asio::async_write(*sslsocket_, asio::buffer(buf), asio::use_awaitable);
        if (!bounded)
            co_await std::move(writing);
        else
        {
            using namespace asio::experimental::awaitable_operators;
            auto first = co_await (std::move(writing) || hs_deadline_hit(write_timer));
            if (first.index() == 1)
            {
                counters().timeout.fetch_add(1);
                last_err_ = errc::timeout;
                last_msg_ = "write timeout after " + std::to_string(cfg_.timeout_sec) + "s";
                co_return false;
            }
        }
    }
    catch (const std::exception &e)
    {
        last_err_ = errc::write;
        last_msg_ = e.what();
        co_return false;
    }
    co_return true;
}

// ===================== 发布（PUBLISH，channel 不加 key 前缀）=====================
long long redis_conn_base::publish(const std::string &channel, const std::string &payload)
{
    last_err_ = errc::ok;
    last_msg_.clear();
    if (isclose_.load() || sock_type_ == 0)
    {
        last_err_ = errc::connect;
        last_msg_ = "not connected";
        return -1;
    }
    if (poisoned_.load())
    {
        last_err_ = errc::connect;
        last_msg_ = "connection poisoned";
        return -1;
    }
    std::string build_err;
    // channel 不进 key 位表（PUBLISH 的 arg1 是频道名），直接组包，不加前缀
    send_buf_ = make_command({"PUBLISH", channel, payload}, &build_err);
    if (send_buf_.empty())
    {
        last_err_ = errc::bad_args;
        last_msg_ = build_err;
        return -1;
    }
    asio::error_code wec;
    if (!write_all_bounded(send_buf_, deadline_for(cfg_.timeout_sec), wec))
    {
        if (wec == asio::error::would_block)
        {
            counters().timeout.fetch_add(1);
            mark_poisoned(errc::timeout, "write timeout after " + std::to_string(cfg_.timeout_sec) + "s");
        }
        else
            mark_poisoned(errc::write, wec.message());
        return -1;
    }
    auto r = read_reply_sync(deadline_for(cfg_.timeout_sec));
    if (!r)
        return -1;// 置脏与计数在 read_reply_sync 内
    if (r->type != reply_type::integer)
    {
        last_err_ = errc::protocol;
        return -1;
    }
    return r->int_value;
}

asio::awaitable<long long> redis_conn_base::async_publish(const std::string &channel, const std::string &payload)
{
    last_err_ = errc::ok;
    last_msg_.clear();
    if (isclose_.load() || sock_type_ == 0)
    {
        last_err_ = errc::connect;
        last_msg_ = "not connected";
        co_return -1;
    }
    if (poisoned_.load())
    {
        last_err_ = errc::connect;
        last_msg_ = "connection poisoned";
        co_return -1;
    }
    std::string build_err;
    std::string cmd = make_command({"PUBLISH", channel, payload}, &build_err);
    if (cmd.empty())
    {
        last_err_ = errc::bad_args;
        last_msg_ = build_err;
        co_return -1;
    }
    if (!(co_await async_write_raw(cmd, make_deadline())))
        mark_poisoned(last_err_, last_msg_);// 写失败可能只发了一半
    else
    {
        auto r = co_await async_read_reply(make_deadline());
        if (!r)
            mark_poisoned(last_err_, last_msg_);
        else if (r->type != reply_type::integer)
        {
            last_err_ = errc::protocol;
            co_return -1;
        }
        else
            co_return r->int_value;
    }
    co_return -1;
}

// ===================== 关闭 =====================
// 不发 QUIT：socket 已是 non_blocking，Quit 的写在对端不收时只会得到 would_block，
// 而服务端看到连接断开效果与 QUIT 相同。
void redis_conn_base::close()
{
    bool was_open = !isclose_.load();
    isclose_.store(true);// 先置位，轮询循环里的 isclose_ 检查据此退出
    if (was_open)
    {
        try
        {
            if (sock_type_ == 1)
            {
                if (socket_ && socket_->is_open())
                {
                    socket_->cancel(ec_);
                    socket_->close(ec_);
                }
            }
            else if (sock_type_ == 2)
            {
                if (sslsocket_ && sslsocket_->lowest_layer().is_open())
                {
                    sslsocket_->lowest_layer().cancel(ec_);
                    sslsocket_->lowest_layer().close(ec_);
                }
            }
        }
        catch (...)
        {
        }
    }
    sock_type_ = 0;
    read_buf_.clear();
    parse_pos_ = 0;
    poisoned_.store(false);
}

asio::awaitable<void> redis_conn_base::async_close()
{
    // 挂起的异步读要先 cancel，否则 close 之后那个 handler 才拿到 operation_aborted
    try
    {
        if (sock_type_ == 1)
        {
            if (socket_)
                socket_->cancel(ec_);
        }
        else if (sock_type_ == 2)
        {
            if (sslsocket_)
                sslsocket_->lowest_layer().cancel(ec_);
        }
    }
    catch (...)
    {
    }
    close();
    co_return;
}

bool redis_conn_base::connected() const
{
    if (isclose_.load() || poisoned_.load())
        return false;
    if (sock_type_ == 1)
        return socket_ && socket_->is_open();
    if (sock_type_ == 2)
        return sslsocket_ && sslsocket_->lowest_layer().is_open();
    return false;
}

// ===================== 便利封装 =====================
http::obj_val redis_conn_base::to_obj_val(const reply_t &reply)
{
    http::obj_val result;
    if (reply.is_null)
    {
        result.set_null();
        return result;
    }
    switch (reply.type)
    {
    case reply_type::simple_string:
    case reply_type::error:
    case reply_type::bulk_string:
    case reply_type::verbatim:
    case reply_type::big_number:
        result = reply.str_value;
        break;
    case reply_type::integer:
        result = reply.int_value;
        break;
    case reply_type::double_value:
        result = reply.double_value;
        break;
    case reply_type::boolean:
        result = reply.bool_value;
        break;
    case reply_type::array:
    case reply_type::push:
    {
        result.set_array();
        for (const auto &e : reply.array_value)
            result.push(to_obj_val(e));
        break;
    }
    default:
        result.set_null();
        break;
    }
    return result;
}

http::obj_val redis_conn_base::command_obj(const std::vector<std::string> &args, const std::string &pfx)
{
    auto r = command(args, pfx);
    if (!r)
    {
        http::obj_val n;
        n.set_null();
        return n;
    }
    return to_obj_val(*r);
}

asio::awaitable<http::obj_val> redis_conn_base::async_command_obj(const std::vector<std::string> &args,
                                                                  std::string pfx)
{
    auto r = co_await async_command(args, std::move(pfx));
    if (!r)
    {
        http::obj_val n;
        n.set_null();
        co_return n;
    }
    co_return to_obj_val(*r);
}

asio::awaitable<http::obj_val> redis_conn_base::async_scan_keys(std::string_view pattern, unsigned int count)
{
    // SCAN 的 MATCH 是模式、不是键名，所以 apply_prefix 的 key 位表不碰它（那是对的：
    // 表里加了前缀反而会把通配符喂进键名）。这里自己把前缀补到模式前面，
    // 于是只在本租户的键空间里枚举；返回的是服务端给的完整键名（带前缀），不做剥除。
    std::string match = cfg_.prefix.empty() ? std::string(pattern) : cfg_.prefix + std::string(pattern);

    http::obj_val result;
    result.set_array();
    std::string cursor  = "0";
    unsigned int rounds = 0;
    while (rounds++ < 1000)// 轮次上限：游标一直不回 0 时不能无限扫下去
    {
        auto r = co_await async_command({"SCAN", cursor, "MATCH", match, "COUNT", std::to_string(count)});
        if (!r || r->type != reply_type::array || r->array_value.size() < 2)
            break;
        cursor = r->array_value[0].str_value;
        if (r->array_value[1].type == reply_type::array)
        {
            for (const auto &k : r->array_value[1].array_value)
                result.push(to_obj_val(k));
        }
        if (cursor == "0")
            break;
    }
    co_return result;
}

}// namespace redis
}// namespace pz
