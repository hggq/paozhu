/*
 * pz::redis_client 的方法体 —— 每个都是一行"拼命令 + 发出去"
 * author Huang ziquan (黄自权)
 * date 2026-10-02
 */
#include "redis_client.h"

#ifdef ENABLE_REDIS

namespace pz
{
namespace redis
{

namespace
{
// 拼参数：命令名 + 前段 + 调用方透传的可选项 + 后段。
// 可选项落在哪一格由原命令的语法决定（SET 在 value 之后，HSET 在 field 之前），不是本层挑的口径。
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

// 可省略的 count 参数：0 = 不发（LPOP/RPOP/ZPOPMIN 这类命令不带 count 时回单值/bulk，
// 带上 count 时回数组，两种回包形状不同；而 "LPOP key 0" 服务端不报错、回空数组 —— 那是第三种形状）。
std::vector<std::string> count_arg(unsigned int count)
{
    std::vector<std::string> out;
    if (count > 0)
        out.push_back(std::to_string(count));
    return out;
}

// 同样省略 count、但要带关键字的版本：XRANGE/XREVRANGE 写的是 "COUNT n"，
// 只发数字的话服务端报 syntax error（实测回的是错误字符串，不是空数组）。
std::vector<std::string> count_kw_arg(unsigned int count)
{
    std::vector<std::string> out;
    if (count > 0)
    {
        out.emplace_back("COUNT");
        out.push_back(std::to_string(count));
    }
    return out;
}

// 带 numkeys 的命令（SINTERCARD 那一族）：数字位由键数现算，紧跟其后才是键段。
// 收成一处是为了"数字和键的个数同源"——分开写迟早漂移，漂了服务端回
// "Number of keys can't be greater than number of args"（实测），而键段被截短的那位则静默不加前缀。
std::vector<std::string> numkeys_head(const std::vector<std::string> &keys)
{
    std::vector<std::string> out;
    out.reserve(1 + keys.size());
    out.push_back(std::to_string(keys.size()));
    for (const auto &one : keys)
        out.push_back(one);
    return out;
}

// 借一条连接、只借到这一次调用为止。析构里必还，命令抛出也还 ——
// 还不了的连接会被下一个借用者读到残留字节，所以归还这件事不能靠调用点自觉。
// conn 是 unique_ptr：所有权本身就是"这条连接归谁"的记录，所以不需要额外的归属位，
// 重复归还这个动作在类型上就写不出来（拿 lvalue 去 back_conn 编译不过）。
class borrowed_conn
{
  public:
    explicit borrowed_conn(std::shared_ptr<redis_pool_section_t> sec_ref) : sec(std::move(sec_ref))
    {
        if (sec)
            conn = sec->get_conn();
    }
    ~borrowed_conn()
    {
        if (conn)
            sec->back_conn(std::move(conn));
    }
    borrowed_conn(const borrowed_conn &)            = delete;
    borrowed_conn &operator=(const borrowed_conn &) = delete;
    borrowed_conn(borrowed_conn &&)                 = delete;
    borrowed_conn &operator=(borrowed_conn &&)      = delete;

    explicit operator bool() const { return static_cast<bool>(conn); }
    redis_conn_base &operator*() const { return *conn; }

  private:
    std::shared_ptr<redis_pool_section_t> sec;
    std::unique_ptr<redis_conn_base> conn;
};

// 同步通路：借 → 发 → 还，全在调用者线程上。
// 借不到（段对象为空 / idle 空且新建连接失败）回 null，与"键不存在"同形。
// 形参顺序是"上下文在前、命令在后"：sec 与 pfx 都来自调用者这个对象，args 才是本次要发的。
// pfx 传空串 = 不发覆盖，由连接层回落到段配置（回落只在 redis_conn_base::prepare_command 那一行）。
http::obj_val send_sync(const std::shared_ptr<redis_pool_section_t> &sec, const std::string &pfx, const std::vector<std::string> &args)
{
    borrowed_conn bc(sec);
    if (!bc)
    {
        http::obj_val n;
        n.set_null();
        return n;
    }
    return (*bc).command_obj(args, pfx);
}

// 协程侧的 pfx 按值：这条协程要跨过挂起点持有它。
asio::awaitable<http::obj_val> send_async(std::shared_ptr<redis_pool_section_t> sec, std::string pfx, std::vector<std::string> args)
{
    co_return co_await get_redis_pool().async_exec_obj(std::move(sec), std::move(args), std::move(pfx));
}

// 整数口径的命令（TTL / EXISTS / EXPIRE / PERSIST）：把 obj_val 收成整数。
// "没送出去"在这里一律得 0（服务端 TTL 不会回 0），与 redis_client 头注释里说的那条限制一致。
asio::awaitable<long long> send_async_int(std::shared_ptr<redis_pool_section_t> sec, std::string pfx, std::vector<std::string> args)
{
    auto v = co_await get_redis_pool().async_exec_obj(std::move(sec), std::move(args), std::move(pfx));
    co_return v.to_int();
}

long long send_sync_int(const std::shared_ptr<redis_pool_section_t> &sec, const std::string &pfx, const std::vector<std::string> &args)
{
    return send_sync(sec, pfx, args).to_int();
}

}// namespace

// ===== String =====
http::obj_val redis_client::str_set(const std::string &key, const std::string &value, const std::vector<std::string> &opts)
{
    return send_sync(tag_sec(), prefix_, join("SET", {key, value}, opts, {}));
}

http::obj_val redis_client::str_get(const std::string &key)
{
    return send_sync(tag_sec(), prefix_, join("GET", {key}, {}, {}));
}

http::obj_val redis_client::str_del(const std::vector<std::string> &keys)
{
    return send_sync(tag_sec(), prefix_, join("DEL", keys, {}, {}));
}

// ===== Hash =====
http::obj_val redis_client::hash_set(const std::string &key, const std::string &field, const std::string &value, const std::vector<std::string> &opts)
{
    return send_sync(tag_sec(), prefix_, join("HSET", {key}, opts, {field, value}));
}

http::obj_val redis_client::hash_get(const std::string &key, const std::string &field)
{
    return send_sync(tag_sec(), prefix_, join("HGET", {key, field}, {}, {}));
}

http::obj_val redis_client::hash_getall(const std::string &key)
{
    return send_sync(tag_sec(), prefix_, join("HGETALL", {key}, {}, {}));
}

// ===== List =====
http::obj_val redis_client::list_push_left(const std::string &key, const std::vector<std::string> &values)
{
    return send_sync(tag_sec(), prefix_, join("LPUSH", {key}, {}, values));
}

http::obj_val redis_client::list_push_right(const std::string &key, const std::vector<std::string> &values)
{
    return send_sync(tag_sec(), prefix_, join("RPUSH", {key}, {}, values));
}

http::obj_val redis_client::list_pop_left(const std::string &key, unsigned int count)
{
    return send_sync(tag_sec(), prefix_, join("LPOP", {key}, {}, count_arg(count)));
}

http::obj_val redis_client::list_pop_right(const std::string &key, unsigned int count)
{
    return send_sync(tag_sec(), prefix_, join("RPOP", {key}, {}, count_arg(count)));
}

http::obj_val redis_client::list_length(const std::string &key)
{
    return send_sync(tag_sec(), prefix_, join("LLEN", {key}, {}, {}));
}

http::obj_val redis_client::list_range(const std::string &key, long long start, long long stop)
{
    return send_sync(tag_sec(), prefix_, join("LRANGE", {key, std::to_string(start), std::to_string(stop)}, {}, {}));
}

http::obj_val redis_client::list_at(const std::string &key, long long index)
{
    return send_sync(tag_sec(), prefix_, join("LINDEX", {key, std::to_string(index)}, {}, {}));
}

http::obj_val redis_client::list_set_at(const std::string &key, long long index, const std::string &value)
{
    return send_sync(tag_sec(), prefix_, join("LSET", {key, std::to_string(index), value}, {}, {}));
}

http::obj_val redis_client::list_trim(const std::string &key, long long start, long long stop)
{
    return send_sync(tag_sec(), prefix_, join("LTRIM", {key, std::to_string(start), std::to_string(stop)}, {}, {}));
}

http::obj_val redis_client::list_insert(const std::string &key, const std::string &where, const std::string &pivot, const std::string &value)
{
    return send_sync(tag_sec(), prefix_, join("LINSERT", {key, where, pivot, value}, {}, {}));
}

http::obj_val redis_client::list_move(const std::string &src, const std::string &dst, const std::string &src_dir, const std::string &dst_dir)
{
    return send_sync(tag_sec(), prefix_, join("LMOVE", {src, dst, src_dir, dst_dir}, {}, {}));
}

// ===== Stream =====
http::obj_val redis_client::stream_add(const std::string &key, const std::vector<std::string> &fields, const std::string &id, const std::vector<std::string> &opts)
{
    // XADD key [NOMKSTREAM] [MAXLEN|MINID …] id field value …：可选项夹在 key 与 id 之间（join 的中段位），
    // id 打头字段段落在尾巴 —— 落点是原命令自己的语法，不是本层挑的。
    std::vector<std::string> tail;
    tail.reserve(1 + fields.size());
    tail.emplace_back(id);
    tail.insert(tail.end(), fields.begin(), fields.end());
    return send_sync(tag_sec(), prefix_, join("XADD", {key}, opts, tail));
}

http::obj_val redis_client::stream_len(const std::string &key)
{
    return send_sync(tag_sec(), prefix_, join("XLEN", {key}, {}, {}));
}

http::obj_val redis_client::stream_range(const std::string &key, const std::string &start, const std::string &end, unsigned int count)
{
    return send_sync(tag_sec(), prefix_, join("XRANGE", {key, start, end}, {}, count_kw_arg(count)));
}

http::obj_val redis_client::stream_read(const std::vector<std::string> &keys, const std::vector<std::string> &ids, const std::vector<std::string> &opts)
{
    std::vector<std::string> tail;
    tail.reserve(1 + keys.size() + ids.size());
    tail.emplace_back("STREAMS");
    tail.insert(tail.end(), keys.begin(), keys.end());
    tail.insert(tail.end(), ids.begin(), ids.end());
    return send_sync(tag_sec(), prefix_, join("XREAD", {}, opts, tail));
}

http::obj_val redis_client::stream_group_create(const std::string &key, const std::string &group, const std::string &id, const std::vector<std::string> &opts)
{
    // XGROUP CREATE key group id [MKSTREAM]：选项在 id **之后**，所以 opts 走尾巴，不走 join 的中段位
    std::vector<std::string> tail{group, id};
    tail.insert(tail.end(), opts.begin(), opts.end());
    return send_sync(tag_sec(), prefix_, join("XGROUP", {"CREATE", key}, {}, tail));
}

http::obj_val redis_client::stream_read_group(const std::string &group, const std::string &consumer, const std::vector<std::string> &keys, const std::vector<std::string> &ids, const std::vector<std::string> &opts)
{
    std::vector<std::string> tail;
    tail.reserve(1 + keys.size() + ids.size());
    tail.emplace_back("STREAMS");
    tail.insert(tail.end(), keys.begin(), keys.end());
    tail.insert(tail.end(), ids.begin(), ids.end());
    return send_sync(tag_sec(), prefix_, join("XREADGROUP", {"GROUP", group, consumer}, opts, tail));
}

http::obj_val redis_client::stream_ack(const std::string &key, const std::string &group, const std::vector<std::string> &ids)
{
    return send_sync(tag_sec(), prefix_, join("XACK", {key, group}, {}, ids));
}

http::obj_val redis_client::stream_pending(const std::string &key, const std::string &group, const std::vector<std::string> &tail)
{
    return send_sync(tag_sec(), prefix_, join("XPENDING", {key, group}, {}, tail));
}

http::obj_val redis_client::stream_info(const std::string &key, const std::string &sub, const std::vector<std::string> &tail)
{
    // sub 在 key 前面，所以 key 落在 args[2]（与 XGROUP 同样的位置）
    return send_sync(tag_sec(), prefix_, join("XINFO", {sub, key}, {}, tail));
}

// ===== Set =====
// 单键族与三条多键读命令。成员位/count 位一律落在 join 的尾巴段，键位在 head 段 ——
// 这个分法来自本机逐条 COMMAND GETKEYS 的实测结果，不是本层挑的口径（键位表在 redis_conn.cpp，只有一张）。
http::obj_val redis_client::set_add(const std::string &key, const std::vector<std::string> &members)
{
    return send_sync(tag_sec(), prefix_, join("SADD", {key}, {}, members));
}

http::obj_val redis_client::set_remove(const std::string &key, const std::vector<std::string> &members)
{
    return send_sync(tag_sec(), prefix_, join("SREM", {key}, {}, members));
}

http::obj_val redis_client::set_members(const std::string &key)
{
    return send_sync(tag_sec(), prefix_, join("SMEMBERS", {key}, {}, {}));
}

http::obj_val redis_client::set_card(const std::string &key)
{
    return send_sync(tag_sec(), prefix_, join("SCARD", {key}, {}, {}));
}

http::obj_val redis_client::set_is_member(const std::string &key, const std::string &member)
{
    return send_sync(tag_sec(), prefix_, join("SISMEMBER", {key, member}, {}, {}));
}

http::obj_val redis_client::set_are_members(const std::string &key, const std::vector<std::string> &members)
{
    return send_sync(tag_sec(), prefix_, join("SMISMEMBER", {key}, {}, members));
}

// count 那一格用 List 轮同一个 count_arg（0 就不发），因为 SPOP 的两条形状实测不同：
// 不带 count 回 bulk、带 count 回数组（哪怕 count=1），而真发 0 是"回空数组 + 一格不删"的静默 no-op。
http::obj_val redis_client::set_pop(const std::string &key, unsigned int count)
{
    return send_sync(tag_sec(), prefix_, join("SPOP", {key}, {}, count_arg(count)));
}

http::obj_val redis_client::set_random_member(const std::string &key, unsigned int count)
{
    return send_sync(tag_sec(), prefix_, join("SRANDMEMBER", {key}, {}, count_arg(count)));
}

http::obj_val redis_client::set_move(const std::string &src, const std::string &dst, const std::string &member)
{
    return send_sync(tag_sec(), prefix_, join("SMOVE", {src, dst}, {}, {member}));
}

http::obj_val redis_client::set_union(const std::vector<std::string> &keys)
{
    return send_sync(tag_sec(), prefix_, join("SUNION", keys, {}, {}));
}

http::obj_val redis_client::set_intersection(const std::vector<std::string> &keys)
{
    return send_sync(tag_sec(), prefix_, join("SINTER", keys, {}, {}));
}

http::obj_val redis_client::set_difference(const std::vector<std::string> &keys)
{
    return send_sync(tag_sec(), prefix_, join("SDIFF", keys, {}, {}));
}

// "键段"那一族：三条 *STORE + SINTERCARD。
// 这三条的语法是 SUNIONSTORE dst key [key…]，**没有 numkeys 这个参数**（实测两族形状不同，
// 见 redis_conn.cpp 键位表里它们与 SINTER/SDIFF/SUNION 同归 kAll），所以整串参数都是键位。
http::obj_val redis_client::set_union_store(const std::string &dst, const std::vector<std::string> &keys)
{
    return send_sync(tag_sec(), prefix_, join("SUNIONSTORE", {dst}, {}, keys));
}

http::obj_val redis_client::set_intersect_store(const std::string &dst, const std::vector<std::string> &keys)
{
    return send_sync(tag_sec(), prefix_, join("SINTERSTORE", {dst}, {}, keys));
}

http::obj_val redis_client::set_difference_store(const std::string &dst, const std::vector<std::string> &keys)
{
    return send_sync(tag_sec(), prefix_, join("SDIFFSTORE", {dst}, {}, keys));
}

// 这一条才是真带 numkeys 的：数字位按 keys.size() 现算，其后一切（"LIMIT" 和它的数值）都落在线外。
http::obj_val redis_client::set_intersect_card(const std::vector<std::string> &keys,
                                               const std::vector<std::string> &tail)
{
    return send_sync(tag_sec(), prefix_, join("SINTERCARD", numkeys_head(keys), {}, tail));
}

// ===== 键级 =====
long long redis_client::key_ttl(const std::string &key)
{
    return send_sync_int(tag_sec(), prefix_, join("TTL", {key}, {}, {}));
}

bool redis_client::key_expire(const std::string &key, unsigned int sec)
{
    return send_sync_int(tag_sec(), prefix_, join("EXPIRE", {key, std::to_string(sec)}, {}, {})) != 0;
}

bool redis_client::key_persist(const std::string &key)
{
    return send_sync_int(tag_sec(), prefix_, join("PERSIST", {key}, {}, {})) != 0;
}

bool redis_client::key_exists(const std::string &key)
{
    return send_sync_int(tag_sec(), prefix_, join("EXISTS", {key}, {}, {})) != 0;
}

http::obj_val redis_client::key_type(const std::string &key)
{
    return send_sync(tag_sec(), prefix_, join("TYPE", {key}, {}, {}));
}

http::obj_val redis_client::command(const std::vector<std::string> &args)
{
    return send_sync(tag_sec(), prefix_, args);
}

// ===== 协程版 =====
asio::awaitable<http::obj_val> redis_client::async_str_set(const std::string &key, const std::string &value, const std::vector<std::string> &opts)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SET", {key, value}, opts, {}));
}

asio::awaitable<http::obj_val> redis_client::async_str_get(const std::string &key)
{
    co_return co_await send_async(tag_sec(), prefix_, join("GET", {key}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_str_del(const std::vector<std::string> &keys)
{
    co_return co_await send_async(tag_sec(), prefix_, join("DEL", keys, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_hash_set(const std::string &key, const std::string &field, const std::string &value, const std::vector<std::string> &opts)
{
    co_return co_await send_async(tag_sec(), prefix_, join("HSET", {key}, opts, {field, value}));
}

asio::awaitable<http::obj_val> redis_client::async_hash_get(const std::string &key, const std::string &field)
{
    co_return co_await send_async(tag_sec(), prefix_, join("HGET", {key, field}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_hash_getall(const std::string &key)
{
    co_return co_await send_async(tag_sec(), prefix_, join("HGETALL", {key}, {}, {}));
}

// ===== List（协程版）=====
asio::awaitable<http::obj_val> redis_client::async_list_push_left(const std::string &key,
                                                                  const std::vector<std::string> &values)
{
    co_return co_await send_async(tag_sec(), prefix_, join("LPUSH", {key}, {}, values));
}

asio::awaitable<http::obj_val> redis_client::async_list_push_right(const std::string &key,
                                                                   const std::vector<std::string> &values)
{
    co_return co_await send_async(tag_sec(), prefix_, join("RPUSH", {key}, {}, values));
}

asio::awaitable<http::obj_val> redis_client::async_list_pop_left(const std::string &key, unsigned int count)
{
    co_return co_await send_async(tag_sec(), prefix_, join("LPOP", {key}, {}, count_arg(count)));
}

asio::awaitable<http::obj_val> redis_client::async_list_pop_right(const std::string &key, unsigned int count)
{
    co_return co_await send_async(tag_sec(), prefix_, join("RPOP", {key}, {}, count_arg(count)));
}

asio::awaitable<http::obj_val> redis_client::async_list_length(const std::string &key)
{
    co_return co_await send_async(tag_sec(), prefix_, join("LLEN", {key}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_list_range(const std::string &key, long long start, long long stop)
{
    co_return co_await send_async(
        tag_sec(),
        prefix_,
        join("LRANGE", {key, std::to_string(start), std::to_string(stop)}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_list_at(const std::string &key, long long index)
{
    co_return co_await send_async(tag_sec(), prefix_, join("LINDEX", {key, std::to_string(index)}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_list_set_at(const std::string &key, long long index, const std::string &value)
{
    co_return co_await send_async(tag_sec(), prefix_, join("LSET", {key, std::to_string(index), value}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_list_trim(const std::string &key, long long start, long long stop)
{
    co_return co_await send_async(
        tag_sec(),
        prefix_,
        join("LTRIM", {key, std::to_string(start), std::to_string(stop)}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_list_insert(const std::string &key, const std::string &where, const std::string &pivot, const std::string &value)
{
    co_return co_await send_async(tag_sec(), prefix_, join("LINSERT", {key, where, pivot, value}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_list_move(const std::string &src, const std::string &dst, const std::string &src_dir, const std::string &dst_dir)
{
    co_return co_await send_async(tag_sec(), prefix_, join("LMOVE", {src, dst, src_dir, dst_dir}, {}, {}));
}

// ===== Stream（协程版）=====
asio::awaitable<http::obj_val> redis_client::async_stream_add(const std::string &key,
                                                              const std::vector<std::string> &fields,
                                                              const std::string &id,
                                                              const std::vector<std::string> &opts)
{
    std::vector<std::string> tail;
    tail.reserve(1 + fields.size());
    tail.emplace_back(id);
    tail.insert(tail.end(), fields.begin(), fields.end());
    co_return co_await send_async(tag_sec(), prefix_, join("XADD", {key}, opts, tail));
}

asio::awaitable<http::obj_val> redis_client::async_stream_len(const std::string &key)
{
    co_return co_await send_async(tag_sec(), prefix_, join("XLEN", {key}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_stream_range(const std::string &key, const std::string &start, const std::string &end, unsigned int count)
{
    co_return co_await send_async(tag_sec(), prefix_, join("XRANGE", {key, start, end}, {}, count_kw_arg(count)));
}

asio::awaitable<http::obj_val> redis_client::async_stream_read(const std::vector<std::string> &keys,
                                                               const std::vector<std::string> &ids,
                                                               const std::vector<std::string> &opts)
{
    std::vector<std::string> tail;
    tail.reserve(1 + keys.size() + ids.size());
    tail.emplace_back("STREAMS");
    tail.insert(tail.end(), keys.begin(), keys.end());
    tail.insert(tail.end(), ids.begin(), ids.end());
    co_return co_await send_async(tag_sec(), prefix_, join("XREAD", {}, opts, tail));
}

asio::awaitable<http::obj_val> redis_client::async_stream_group_create(const std::string &key,
                                                                       const std::string &group,
                                                                       const std::string &id,
                                                                       const std::vector<std::string> &opts)
{
    std::vector<std::string> tail{group, id};
    tail.insert(tail.end(), opts.begin(), opts.end());
    co_return co_await send_async(tag_sec(), prefix_, join("XGROUP", {"CREATE", key}, {}, tail));
}

asio::awaitable<http::obj_val> redis_client::async_stream_read_group(const std::string &group,
                                                                     const std::string &consumer,
                                                                     const std::vector<std::string> &keys,
                                                                     const std::vector<std::string> &ids,
                                                                     const std::vector<std::string> &opts)
{
    std::vector<std::string> tail;
    tail.reserve(1 + keys.size() + ids.size());
    tail.emplace_back("STREAMS");
    tail.insert(tail.end(), keys.begin(), keys.end());
    tail.insert(tail.end(), ids.begin(), ids.end());
    co_return co_await send_async(tag_sec(), prefix_, join("XREADGROUP", {"GROUP", group, consumer}, opts, tail));
}

asio::awaitable<http::obj_val> redis_client::async_stream_ack(const std::string &key, const std::string &group, const std::vector<std::string> &ids)
{
    co_return co_await send_async(tag_sec(), prefix_, join("XACK", {key, group}, {}, ids));
}

asio::awaitable<http::obj_val> redis_client::async_stream_pending(const std::string &key, const std::string &group, const std::vector<std::string> &tail)
{
    co_return co_await send_async(tag_sec(), prefix_, join("XPENDING", {key, group}, {}, tail));
}

asio::awaitable<http::obj_val> redis_client::async_stream_info(const std::string &key, const std::string &sub, const std::vector<std::string> &tail)
{
    co_return co_await send_async(tag_sec(), prefix_, join("XINFO", {sub, key}, {}, tail));
}

// ===== Set（协程版）=====
asio::awaitable<http::obj_val> redis_client::async_set_add(const std::string &key,
                                                           const std::vector<std::string> &members)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SADD", {key}, {}, members));
}

asio::awaitable<http::obj_val> redis_client::async_set_remove(const std::string &key,
                                                              const std::vector<std::string> &members)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SREM", {key}, {}, members));
}

asio::awaitable<http::obj_val> redis_client::async_set_members(const std::string &key)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SMEMBERS", {key}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_set_card(const std::string &key)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SCARD", {key}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_set_is_member(const std::string &key,
                                                                 const std::string &member)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SISMEMBER", {key, member}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_set_are_members(const std::string &key,
                                                                   const std::vector<std::string> &members)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SMISMEMBER", {key}, {}, members));
}

asio::awaitable<http::obj_val> redis_client::async_set_pop(const std::string &key, unsigned int count)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SPOP", {key}, {}, count_arg(count)));
}

asio::awaitable<http::obj_val> redis_client::async_set_random_member(const std::string &key, unsigned int count)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SRANDMEMBER", {key}, {}, count_arg(count)));
}

asio::awaitable<http::obj_val> redis_client::async_set_move(const std::string &src, const std::string &dst, const std::string &member)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SMOVE", {src, dst}, {}, {member}));
}

asio::awaitable<http::obj_val> redis_client::async_set_union(const std::vector<std::string> &keys)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SUNION", keys, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_set_intersection(const std::vector<std::string> &keys)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SINTER", keys, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_set_difference(const std::vector<std::string> &keys)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SDIFF", keys, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_set_union_store(const std::string &dst,
                                                                   const std::vector<std::string> &keys)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SUNIONSTORE", {dst}, {}, keys));
}

asio::awaitable<http::obj_val> redis_client::async_set_intersect_store(const std::string &dst,
                                                                       const std::vector<std::string> &keys)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SINTERSTORE", {dst}, {}, keys));
}

asio::awaitable<http::obj_val> redis_client::async_set_difference_store(const std::string &dst,
                                                                        const std::vector<std::string> &keys)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SDIFFSTORE", {dst}, {}, keys));
}

asio::awaitable<http::obj_val> redis_client::async_set_intersect_card(const std::vector<std::string> &keys,
                                                                      const std::vector<std::string> &tail)
{
    co_return co_await send_async(tag_sec(), prefix_, join("SINTERCARD", numkeys_head(keys), {}, tail));
}

asio::awaitable<long long> redis_client::async_key_ttl(const std::string &key)
{
    co_return co_await send_async_int(tag_sec(), prefix_, join("TTL", {key}, {}, {}));
}

asio::awaitable<bool> redis_client::async_key_expire(const std::string &key, unsigned int sec)
{
    auto n = co_await send_async_int(tag_sec(), prefix_, join("EXPIRE", {key, std::to_string(sec)}, {}, {}));
    co_return n != 0;
}

asio::awaitable<bool> redis_client::async_key_persist(const std::string &key)
{
    auto n = co_await send_async_int(tag_sec(), prefix_, join("PERSIST", {key}, {}, {}));
    co_return n != 0;
}

asio::awaitable<bool> redis_client::async_key_exists(const std::string &key)
{
    auto n = co_await send_async_int(tag_sec(), prefix_, join("EXISTS", {key}, {}, {}));
    co_return n != 0;
}

asio::awaitable<http::obj_val> redis_client::async_key_type(const std::string &key)
{
    co_return co_await send_async(tag_sec(), prefix_, join("TYPE", {key}, {}, {}));
}

asio::awaitable<http::obj_val> redis_client::async_command(const std::vector<std::string> &args)
{
    co_return co_await send_async(tag_sec(), prefix_, args);
}

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS
