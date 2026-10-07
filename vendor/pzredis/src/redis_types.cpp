/*
 * pzredis 五种基本类型的便利入口（第一批：String + Hash）
 * author Huang ziquan (黄自权)
 * date 2026-10-02
 */
#include "redis_types.h"

#ifdef ENABLE_REDIS

#include "redis_pool.h"

namespace pz
{
namespace redis
{

namespace
{
// 拼参数：命令名 + 前段 + 调用方透传的可选项 + 后段。
// 可选项落在哪个参数位由原命令的语法决定（SET 在 value 之后，HSET 在 field 之前），不是本层挑的口径。
// 越界的参数个数由 redis_conn_base::make_command 拒（kMaxArgs），本层不再自设上限。
std::vector<std::string> join(const char *cmd, const std::vector<std::string> &head, const std::vector<std::string> &opts, const std::vector<std::string> &tail)
{
    std::vector<std::string> out;
    out.reserve(1 + head.size() + opts.size() + tail.size());
    out.emplace_back(cmd);
    for (const auto &h : head)
        out.push_back(h);
    for (const auto &o : opts)
        out.push_back(o);
    for (const auto &t : tail)
        out.push_back(t);
    return out;
}

asio::awaitable<http::obj_val> send(const std::string &section, std::vector<std::string> args)
{
    co_return co_await get_redis_pool().async_exec_obj(section, args);
}

// 问题 2 示范：async_exec_obj_ex 把"传输失败(nullopt)"与"服务端真 null(有值且 is_null)"分开
asio::awaitable<std::optional<http::obj_val>> send_ex(const std::string &section, std::vector<std::string> args)
{
    co_return co_await get_redis_pool().async_exec_obj_ex(section, args);
}
}// namespace

asio::awaitable<http::obj_val> async_str_set(const std::string &section, const std::string &key, const std::string &value, const std::vector<std::string> &opts)
{
    co_return co_await send(section, join("SET", {key, value}, opts, {}));
}

asio::awaitable<http::obj_val> async_str_get(const std::string &section, const std::string &key)
{
    co_return co_await send(section, join("GET", {key}, {}, {}));
}

// 问题 2 示范：可区分的 GET。调用方示例：
//   auto r = co_await async_str_get_ex(sec, key);
//   if (!r)            { /* 池未加载 / 段不存在 / 连接失败 —— 基础设施问题，应告警 */ }
//   else if (r->is_null()) { /* 键不存在，正常业务分支 */ }
//   else               { /* 取到值：*r */ }
asio::awaitable<std::optional<http::obj_val>> async_str_get_ex(const std::string &section, const std::string &key)
{
    co_return co_await send_ex(section, join("GET", {key}, {}, {}));
}

asio::awaitable<http::obj_val> async_str_del(const std::string &section, const std::vector<std::string> &keys)
{
    co_return co_await send(section, join("DEL", keys, {}, {}));
}

asio::awaitable<http::obj_val> async_hash_set(const std::string &section, const std::string &key, const std::string &field, const std::string &value, const std::vector<std::string> &opts)
{
    co_return co_await send(section, join("HSET", {key}, opts, {field, value}));
}

asio::awaitable<http::obj_val> async_hash_get(const std::string &section, const std::string &key, const std::string &field)
{
    co_return co_await send(section, join("HGET", {key, field}, {}, {}));
}

asio::awaitable<http::obj_val> async_hash_getall(const std::string &section, const std::string &key)
{
    co_return co_await send(section, join("HGETALL", {key}, {}, {}));
}

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS
