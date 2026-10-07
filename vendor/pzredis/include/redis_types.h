#ifndef PZ_REDIS_TYPES_H
#define PZ_REDIS_TYPES_H
/*
 * pzredis 五种基本类型的便利入口（第一批：String + Hash）
 * author Huang ziquan (黄自权)
 * date 2026-10-02
 *
 * 这一层只是"把命令名和参数拼好，交给连接池发出去"：
 *  - 每个方法都是一行转发，不做任何 key 位置判断（key 位只认 redis_conn.cpp 里那一张 kKeyPositions 表，
 *    这里再判一次就是两张表，迟早对不齐）；
 *  - 不解析值、不转 C++ 类型，返回的就是 redis_conn_base::to_obj_val 出来的 obj_val。
 *    RESP2（pzredis 从不发 HELLO 3）下的形状规矩：bulk/simple/verbatim/error ⇒ 字符串，`:` ⇒ 整数，
 *    `*2\r\n$-1` 这种数组元素各自成元素，`$-1` 与 `_` ⇒ null。
 *    所以 HGETALL 回来是 [f1,v1,f2,v2] 的扁平数组，不是对象 —— 要对象形态请调用方自己配对。
 *
 * 原命令的可选项一律透传（SET 的 NX/XX/EX/PX/KEEPTTL/GET、HSET 的 NX/XX），这一层不做子集挑选，
 * 挑了就是把功能面收窄。落点按原命令的语法，不是本层定的口径：
 *   SET  key value [NX|XX] [EX|PX ...] [KEEPTTL] [GET]   ⇒ 可选项在 value 之后（尾巴）
 *   HSET key [NX|XX] field value                          ⇒ 可选项在 field 之前（夹在 key 后面）
 * 所以 async_str_set(sec,k,v,{"NX","EX","30"}) == SET k v NX EX 30，
 *    async_hash_set(sec,k,f,v,{"NX"})          == HSET k NX f v。
 * 注：本机 redis 8.8.0(homebrew, git_dirty) 这个构建对 HSET 的 NX/XX 一律回
 * "ERR wrong number of arguments for 'hset' command"（两个位置都试过），上游 4.0 起是支持的；
 * 那是本机服务端的构建问题，本层只保证"按上游语法拼"。字段级"不存在才写"请用 HSETNX（本机可用）。
 *
 * 命名：本层这些入口都是协程（co_await 返回 obj_val），所以一律 async_ 前缀，
 * 与 redis_conn_base 的 async_command_obj / async_exec_obj 同一口径；裸名（str_set、hash_get…）
 * 归 redis_client 的同步方法，在调用者线程上借连接、发完就还，所以只能在可以阻塞的线程里调。
 *
 * 一处必须知道的限制（不是本层的缺陷，是 async_exec_obj 的口径）：
 * 池没加载 / 段名不存在 / 连接没建成 ⇒ 回 null；键不存在 ⇒ 服务端也回 null。两种"没有"在这里不可分。
 * 要分就用 redis_pool::async_exec()（它回 std::optional<reply_t>，nullopt 才是"没送出去"）。
 *
 * 服务端回 -ERR（比如对 String 键执行 HSET 的 WRONGTYPE）会变成一个普通字符串
 * （"WRONGTYPE Operation against a key ..."），与"值恰好长这样的 bulk"也不可分。
 */
#include <asio.hpp>
#include <optional>
#include <string>
#include <vector>

#include "request.h"
#include "redis_conn.h"

#ifdef ENABLE_REDIS

namespace pz
{
namespace redis
{

// ===== String =====
// SET key value [opts...]；opts 原样接在参数尾巴上（{"NX"}、{"EX","30"}、{"XX","KEEPTTL"}…）。
// 成功回 "OK"；NX/XX 没命中回 null；带 GET 时回旧值。
asio::awaitable<http::obj_val> async_str_set(const std::string &section, const std::string &key, const std::string &value, const std::vector<std::string> &opts = {});
// GET key；键不存在（或值是别的类型，服务端回 -ERR）见上面两条说明。
asio::awaitable<http::obj_val> async_str_get(const std::string &section, const std::string &key);
// GET key 的"可区分"版本：返回 std::optional<http::obj_val>。
//   nullopt                = 传输失败（池未加载 / 段名不存在 / 连接没建成）
//   有值且 is_null()==true = 键不存在（服务端真·null）
//   有值且非 null           = 取到的值
asio::awaitable<std::optional<http::obj_val>> async_str_get_ex(const std::string &section, const std::string &key);
// DEL key [key...]；回被删掉的键个数（整数），一个都没删成回 0。
// keys 为空时发出去的是没有参数的 DEL，服务端回 -ERR 字符串；本层不替你挡。
asio::awaitable<http::obj_val> async_str_del(const std::string &section, const std::vector<std::string> &keys);

// ===== Hash =====
// HSET key [opts...] field value；新建字段回 1，覆盖已有字段回 0（值仍然被改）。
// 一次多个字段请直接走 async_exec_obj(sec, {"HSET", key, f1, v1, f2, v2})，本层只管单字段。
asio::awaitable<http::obj_val> async_hash_set(const std::string &section, const std::string &key, const std::string &field, const std::string &value, const std::vector<std::string> &opts = {});
// HGET key field；字段不存在回 null。
asio::awaitable<http::obj_val> async_hash_get(const std::string &section, const std::string &key, const std::string &field);
// HGETALL key；回扁平数组 [f1,v1,f2,v2,...]（RESP2 下服务端就是扁平发的），键不存在回空数组。
asio::awaitable<http::obj_val> async_hash_getall(const std::string &section, const std::string &key);

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS
#endif
