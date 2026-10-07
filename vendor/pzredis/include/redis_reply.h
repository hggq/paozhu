#ifndef PZ_REDIS_REPLY_H
#define PZ_REDIS_REPLY_H
/*
 * pzredis protocol layer (pure RESP parser, no IO / no asio)
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 */
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace pz
{
namespace redis
{

// 一次 feed() 的解析结果
enum class parse_status
{
    ok,       // 成功解析出一个完整的 reply
    need_more,// 数据不完整，等更多字节再喂（调用方保留已喂数据，consumed 回滚到起始）
    error     // 协议错误 / 超限制
};

// RESP / RESP3 回复类型（覆盖常用 + RESP3 安全兜底）
enum class reply_type : uint8_t
{
    simple_string = '+',
    error         = '-',
    integer       = ':',
    bulk_string   = '$',
    array         = '*',
    double_value  = ',',
    boolean       = '#',
    big_number    = '(',
    null          = '_',
    verbatim      = '=',// 格式：$len\r\nfmt:data\r\n
    map           = '%',// RESP3 的 (key,value) 成对帧：本项目按协议错拒绝，不产出这个类型
    set           = '~',// RESP3 的集合帧：同样按协议错拒绝
    push          = '>',// 同 array（服务端推送，二期 pubsub 用）
    blob_error    = '!'
};

struct reply_t
{
    reply_type type = reply_type::simple_string;
    std::string str_value;
    long long int_value = 0;
    double double_value = 0.0;
    bool bool_value     = false;
    bool is_null        = false;
    std::vector<reply_t> array_value;

    bool is_error() const { return type == reply_type::error || type == reply_type::blob_error; }
    bool is_ok() const { return type == reply_type::simple_string && str_value == "OK"; }
};

// 纯函数式、无状态解析器：一段数据里解析出一个完整 reply。
// 调用方负责累积缓冲（半包时把新读到的字节 append 后再重喂整块）。
class reply_parser_t
{
  public:
    static constexpr unsigned kMaxDepth   = 32;              // 嵌套深度上限
    static constexpr std::size_t kMaxBulk = 64 * 1024 * 1024;// 单 bulk 上限，防 OOM

    // 进程累计：被显式判成协议错的 RESP3 独有帧数（map '%' 与 set '~'）。
    // 本项目从不发 HELLO 3，正常对端不会送来这两种帧；收到就只有两种解释——
    // 对面不是 Redis，或者字节流早就串了。按 array 收下会拿出一个"形状像样、内容不对"的
    // 结果，所以宁可判错并留一条能看见的读数。
    static std::atomic<unsigned long long> &resp3_reject();

    // 解析一个 reply；consumed 为 [in/out]，成功前进到 reply 之后，need_more/error 回滚到起始。
    parse_status feed(const unsigned char *data, std::size_t length, std::size_t &consumed, reply_t &out, unsigned depth = 0);
    void reset() {}

  private:
    bool read_line(const unsigned char *data, std::size_t length, std::size_t &offset, std::string &line);
    bool try_parse_longlong(const std::string &s, long long &val);
    bool try_parse_double(const std::string &s, double &val);

    // 数组 / 推送统一解析（元素个数 = 帧里那个长度）
    parse_status parse_collection(const unsigned char *data, std::size_t length, std::size_t &consumed, reply_t &out, unsigned depth);
    // 类 bulk 解析（bulk_string / verbatim / blob_error）
    parse_status parse_bulk_like(const unsigned char *data, std::size_t length, std::size_t &consumed, reply_t &out, reply_type t);
};

}// namespace redis
}// namespace pz

#endif
