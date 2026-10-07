#ifndef PZ_REDIS_CLIENT_H
#define PZ_REDIS_CLIENT_H
/*
 * pz::redis::redis_client —— 绑到一个连接段（redis.conf 的 [default] / [client1] …）的 redis 操作入口
 * author Huang ziquan (黄自权)
 * date 2026-10-02
 *
 * 这个对象**不持连接**，它持的是"段名 + 段对象"。连接在每条命令内部借、发完就还：
 *   sec->get_conn() -> command(args) -> sec->back_conn(conn)
 * 段对象（redis_pool_section_t，一个段名一个实例，连接归它管）在构造时查一次登记表并缓存下来，
 * 所以命令路径上不再回连接池查表；构造那会儿段还不存在（池没加载 / 段名拼错）时，每次调用回查一次，
 * 于是热更新补上的段同一个长寿命对象也能用上。缓存只在构造时写一次，之后只读 ⇒ 多线程共用一个
 * redis_client 也不会有人在写它。
 * 所以：
 *  - 它可以拷贝、可以赋值、可以放进长寿命容器、析构什么都不用做（没有东西要归还）；
 *  - 同一时刻不会有两条命令共用一条连接（连接被摘出 idle 的那一刻起，独占者就是这次调用）；
 *  - 两条命令**不保证**落在同一条连接上，所以"必须同连接"的写法这里发不出去：
 *    WATCH/MULTI/EXEC、先 SETNX 再 EXPIRE 的两步锁、BLPOP 之类阻塞命令的会话，
 *    要那些就自己拿一条 redis_conn_base 长期持有（自管连接），别用这个类。
 *  - 锁请走单命令：str_set(k, v, {"NX","EX","30"}) —— 加锁和设期限在同一条命令里原子完成。
 *
 * 命名：同步版裸名，协程版 async_ 前缀，与 redis_conn_base 的 command_obj / async_command_obj 同一口径。
 * 同步版在**调用者线程**上借连接，idle 空时那次借包含一次阻塞 connect，所以它只能在业务线程里调
 * （`std::string` 控制器本来就在业务线程池 worker 上）；协程控制器里调同步版就是占住协程线程去睡。
 *
 * 这一层只做三件事：把命令名和参数拼好、交给连接池、把回包原样搬回来。
 *  - 每个方法都是一行转发，不做任何 key 位置判断（key 位只认 redis_conn.cpp 里那一张 kKeyPositions 表，
 *    这里再判一次就是两张表，迟早对不齐）；
 *  - 不解析值、不转 C++ 类型，返回的就是 redis_conn_base::to_obj_val 出来的 obj_val。
 *    RESP2（pzredis 从不发 HELLO 3）下的形状规矩：bulk/simple/verbatim/error ⇒ 字符串，`:` ⇒ 整数，
 *    数组元素各自成元素，`$-1` 与 `_` ⇒ null。
 *    所以 hash_getall 回来是 [f1,v1,f2,v2] 的扁平数组，不是对象 —— 要对象形态请调用方自己配对。
 *
 * 原命令的可选项一律透传（SET 的 NX/XX/EX/PX/KEEPTTL/GET、HSET 的 NX/XX），这一层不做子集挑选，
 * 挑了就是把功能面收窄。**落点按原命令自己的语法，不是本层挑的口径**：
 *   SET  key value [NX|XX] [EX|PX ...] [KEEPTTL] [GET]   ⇒ 可选项在 value 之后（尾巴）
 *   HSET key [NX|XX] field value                          ⇒ 可选项在 field 之前（夹在 key 后面）
 * 所以 rc.str_set(k,v,{"NX","EX","30"}) == SET k v NX EX 30，
 *    rc.hash_set(k,f,v,{"NX"})          == HSET k NX f v。
 * 注：本机 redis 8.8.0(homebrew, git_dirty) 这个构建对 HSET 的 NX/XX 一律回
 * "ERR wrong number of arguments for 'hset' command"（两个位置都试过），上游 4.0 起是支持的；
 * 那是本机服务端的构建问题，本层只保证"按上游语法拼"。字段级"不存在才写"请用 HSETNX（本机可用）。
 *
 * 一处必须知道的限制（连接池的口径，不是本层加的）：
 * 池没加载 / 段名不存在 / 连接没建成 ⇒ 回 null；键不存在 ⇒ 服务端也回 null。两种"没有"在这里不可分。
 * key_ttl / key_expire / key_persist / key_exists 这四个整数口径也一样：没送出去时 to_int() 得 0，
 * 而 0 不是服务端 TTL 会产生的数（TTL 只回 -2 / -1 / 正数），EXPIRE 与 PERSIST 的 0 则与"没送出去"同形。
 * 要分开就用 redis_pool::async_exec()（它回 std::optional<reply_t>，nullopt 才是"没送出去"）。
 *
 * 服务端回 -ERR（比如对 String 键执行 HSET 的 WRONGTYPE）会变成一个普通字符串
 * （"WRONGTYPE Operation against a key ..."），与"值恰好长这样的 bulk"也不可分。
 *
 * 用法（协程里）：
 *   pz::redis::redis_client rc("default");
 *   co_await rc.async_str_set("k", "v", {"NX", "EX", "30"});   // 加锁：一条命令原子完成
 *   co_await rc.async_str_get("k");
 *   co_await rc.async_key_ttl("k");
 *   co_await rc.async_str_del({"k"});
 */
#ifdef ENABLE_REDIS

#include <asio.hpp>
#include <string>
#include <vector>

#include "redis_pool.h"

namespace pz
{
namespace redis
{
class redis_client
{
  public:
    // 无参 = 用 redis.conf 的 [default] 段；换段用 redis_client rc("client1")。
    // 第二个参数是命令级 key 前缀：非空 ⇒ 每条命令都用它加前缀；空串 ⇒ 回落到该段的 prefix。
    // 前缀只由连接层按 kKeyPositions 加在"确知是 key 的参数位"上，业务侧写的仍是裸键名。
    // 构造函数不建连接；段名不存在不是错误，是"第一次调用回 null"（池还没加载时也一样）。
    redis_client() : sec_(get_redis_pool().section(section_)) {}
    explicit redis_client(std::string section, std::string prefix = "")
        : section_(std::move(section)), prefix_(std::move(prefix)),
          sec_(get_redis_pool().section(section_)) {}

    const std::string &section() const { return section_; }
    // 只读回显：空串的意思是"本对象不覆盖、吃段配置"，不是"不加前缀"。
    const std::string &key_prefix() const { return prefix_; }

  private:
    // 段对象：构造时那一次查表的结果，命令路径上直接用；没查到（段名当时不存在 / 池没加载）就
    // 每次回查一次而不写成员 —— 构造之后没人写它，所以多线程共用一个对象也没有写冲突。
    std::shared_ptr<redis_pool_section_t> tag_sec() const
    {
        if (sec_)
            return sec_;
        return get_redis_pool().section(section_);
    }

  public:
    // ===== String =====
    http::obj_val str_set(const std::string &key, const std::string &value, const std::vector<std::string> &opts = {});
    http::obj_val str_get(const std::string &key);
    // keys 为空时发出去的是没有参数的 DEL，服务端回 -ERR 字符串；本层不替你挡。
    http::obj_val str_del(const std::vector<std::string> &keys);

    // ===== Hash =====
    http::obj_val hash_set(const std::string &key, const std::string &field, const std::string &value, const std::vector<std::string> &opts = {});
    http::obj_val hash_get(const std::string &key, const std::string &field);
    http::obj_val hash_getall(const std::string &key);

    // ===== List =====
    // 索引与区间一律 long long，负数按 redis 自己的口径直送（-1 = 最后一个元素），本层不做换算。
    // 空 values 发出去的是没有元素的 LPUSH，服务端回 -ERR 字符串 —— 与 str_del 同理，本层不替你挡。
    http::obj_val list_push_left(const std::string &key, const std::vector<std::string> &values);
    http::obj_val list_push_right(const std::string &key, const std::vector<std::string> &values);
    // count = 0 表示不发 count 参数。真发 LPOP key 0 服务端不报错，回空数组，
    // 和省略时的 bulk、以及"键不存在"的 null 是三种形状，所以这个参数必须由本层省掉而不是传下去。
    http::obj_val list_pop_left(const std::string &key, unsigned int count = 0);
    http::obj_val list_pop_right(const std::string &key, unsigned int count = 0);
    http::obj_val list_length(const std::string &key);
    http::obj_val list_range(const std::string &key, long long start, long long stop);
    http::obj_val list_at(const std::string &key, long long index);
    http::obj_val list_set_at(const std::string &key, long long index, const std::string &value);
    http::obj_val list_trim(const std::string &key, long long start, long long stop);
    // where 是 "BEFORE" / "AFTER"，方向关键字一律透传，本层不校验拼写。
    http::obj_val list_insert(const std::string &key, const std::string &where, const std::string &pivot, const std::string &value);
    // src_dir / dst_dir 是 "LEFT" / "RIGHT"。
    http::obj_val list_move(const std::string &src, const std::string &dst, const std::string &src_dir, const std::string &dst_dir);

    // ===== Stream（第 6 型：消息队列）=====
    // 回包一律是 RESP2 的嵌套数组原样搬回来（to_obj_val 递归），本层不配对、不转对象：
    //   XADD ⇒ bulk 字符串 id "毫秒-序号"；NOMKSTREAM 打在不存在的键上 ⇒ null（XADD 也能为空）
    //   XRANGE ⇒ [[id,[f,v,f,v]],…]，字段段是扁平交替
    //   XREAD / XREADGROUP ⇒ [[流名,[[id,[f,v]],…]],…]，**没有新消息时是 null，不是空数组**
    //   XACK / XDEL / XLEN ⇒ 整数（XACK 只数"真在 PEL 里被收掉的条数"）
    //   XPENDING 摘要 ⇒ [条数, 最小 id, 最大 id, [[消费者名, "条数"],…]]
    //     —— 每消费者条数是 bulk **字符串** "4"，不是整数；区间形式才是 [[id, 消费者, idle整数, 条数整数],…]
    // 回包里的流名是服务端给的完整物理名（段配了 prefix 就带着它），不做剥除。
    // 不发 BLOCK（与阻塞族同一口径）：这个类每命令借还，BLOCK 期间那条连接被独占，
    // 而 XREAD BLOCK 0 实测会一直挂到别人 XADD 为止。队列消费靠"上一次的 id + 轮询"。
    // 同理注意：XREAD 的 '$' 只在配合 BLOCK 时才有意义，不带 BLOCK 传 '$' 恒回 null，
    // 所以轮询要传 "0-0" 或上一次拿到的最大 id。
    // fields 是扁平的 f,v,f,v；奇数长度本层不拦，服务端报 wrong number of arguments（与空 values 同理）。
    http::obj_val stream_add(const std::string &key, const std::vector<std::string> &fields, const std::string &id = "*", const std::vector<std::string> &opts = {});
    http::obj_val stream_len(const std::string &key);
    // start / end 是服务端自己的写法（"-"、"+"、"0-0"、"(1-1" 排除左边界都行），一律透传。
    // count = 0 表示不发 count 参数。这个参数和 LPOP 那个同源但**不同形**：XRANGE 要写成 "COUNT n"，
    // 只发数字服务端回 ERR syntax error；而真发 COUNT 0 不报错、回空数组 —— 和"区间里没东西"同形，
    // 所以省略必须由本层做，不能把 0 传下去。
    http::obj_val stream_range(const std::string &key, const std::string &start, const std::string &end, unsigned int count = 0);
    // keys 与 ids 是平行数组，长度不等本层不拦，服务端报 Unbalanced 'xread'。opts 装 "COUNT"、"10" 这类。
    http::obj_val stream_read(const std::vector<std::string> &keys, const std::vector<std::string> &ids, const std::vector<std::string> &opts = {});
    // id 是组的起点："0-0" 收历史消息，'$' 只收新建组之后的新消息（要 MKSTREAM 时把它放进 opts，
    // 但 XGROUP 的选项落在 id **之后**，所以 opts 走的是尾巴不是中段）。组名已存在回 BUSYGROUP 字符串。
    http::obj_val stream_group_create(const std::string &key, const std::string &group, const std::string &id, const std::vector<std::string> &opts = {});
    // ids 一般就是 {">"}（'> 只能配 GROUP 用，XREAD 里传它会报参数错）。组不存在回 NOGROUP 字符串，
    // 所以建组是这条和 stream_ack 的前提。NOACK 之类放 opts。
    http::obj_val stream_read_group(const std::string &group, const std::string &consumer, const std::vector<std::string> &keys, const std::vector<std::string> &ids, const std::vector<std::string> &opts = {});
    // 必须带组名：漏了就是服务端的参数错字符串。同一条 id 重复 ack 回 0。
    http::obj_val stream_ack(const std::string &key, const std::string &group, const std::vector<std::string> &ids);
    // 摘要形式不带 tail；区间形式把 [IDLE ms] start end count [consumer] 整段按原语法放进 tail。
    http::obj_val stream_pending(const std::string &key, const std::string &group, const std::vector<std::string> &tail = {});
    // XINFO 的三个子命令（STREAM / GROUPS / CONSUMERS）key 都在 args[2]，所以一条方法按原语法发：
    // 命令实际是 XINFO <sub> <key> [tail…]，sub 一律透传、本层不校验拼写。
    // CONSUMERS 的组名放 tail —— 它不是键，服务端 COMMAND GETKEYS 实测 XINFO CONSUMERS kk gg 只回 kk，
    // 所以配了 prefix 的段里它不会被加前缀（加了就回 NOGROUP，这条正好是它的运行时口）。
    // 回包 RESP2 形状：STREAM 是扁平的 field,value,… 交替数组；GROUPS / CONSUMERS 是"数组套扁平数组"。
    http::obj_val stream_info(const std::string &key, const std::string &sub, const std::vector<std::string> &tail = {});

    // ===== Set =====
    // 成员位永远不是 key（本机逐条 COMMAND GETKEYS 实测：SADD/SREM/SMISMEMBER/SPOP/SRANDMEMBER 都只吃
    // args[1]，SMOVE 吃 args[1..2]，SUNION/SINTER/SDIFF 整串都是 key），所以带 prefix 的段里
    // set_add(k, {"1"}) 写进去的成员就是裸 "1"，不会被加前缀。
    // 一条都不走整数收口：SCARD/SREM/SMOVE/SISMEMBER 的 0 是服务端真会产生的数，收口就把"没送出去"混进真值。
    // SADD/SREM/SMISMEMBER 传空 members 发出去的是零成员命令，服务端回 -ERR 字符串（与 str_del 同理，本层不挡）。
    http::obj_val set_add(const std::string &key, const std::vector<std::string> &members);
    http::obj_val set_remove(const std::string &key, const std::vector<std::string> &members);
    // 缺失键回**空数组**（不是 null），所以"键不存在"和"键在但没成员"在这里同形（后者不可达：弹空即删键）。
    http::obj_val set_members(const std::string &key);
    http::obj_val set_card(const std::string &key);
    // 1 是成员、0 不是（键不存在也是 0，整数口径不区分）。
    http::obj_val set_is_member(const std::string &key, const std::string &member);
    // 逐个成员各回一个 0/1，顺序就是传入顺序；键不存在回 [0,0,…]（按成员数），不是 null。
    http::obj_val set_are_members(const std::string &key, const std::vector<std::string> &members);
    // count = 0 表示不发 count 参数，而这个参数必须能省：**SPOP k 回 bulk 单条，SPOP k 1 回 1 元素数组**，
    // 两者形状不同；真发 SPOP k 0 服务端不报错、回空数组且一个成员都不删（实测），是静默 no-op。
    // 负数 count（"允许重复地抽 n 个"）在这条签名下发不出去，需要它走 command()。
    http::obj_val set_pop(const std::string &key, unsigned int count = 0);
    // 与 set_pop 同一条形状规矩：无 count 是 bulk、带 count 是数组、count 0 是静默 no-op；且不删成员。
    http::obj_val set_random_member(const std::string &key, unsigned int count = 0);
    // 1 搬成了、0 没搬（成员不在、或源键不存在）。src==dst 且成员在时**回 1 且内容不变**（实测 no-op，
    // 不是老版本的"source and destination object must be different"参数错）。
    http::obj_val set_move(const std::string &src, const std::string &dst, const std::string &member);
    // 三条多键读命令：keys 里的缺失键当空集（SUNION 照并其余、SINTER 直接空、SDIFF 首键缺失则结果为空）。
    // keys 为空发出去的是零参数命令，服务端回 -ERR 字符串。
    http::obj_val set_union(const std::vector<std::string> &keys);
    http::obj_val set_intersection(const std::vector<std::string> &keys);
    http::obj_val set_difference(const std::vector<std::string> &keys);

    // Set 的"键段"一族：三个 *STORE + SINTERCARD。
    // Set 的三个 *STORE 的语法是 SUNIONSTORE dst key [key…]，**没有 numkeys 这个参数**：
    // 本机 redis 8.8 实测 COMMAND GETKEYS SUNIONSTORE d s t 把 d/s/t 三个全当键返回，
    // 而 ZUNIONSTORE 同样的发法不吃那个数字 —— 所以它们和 SINTER/SDIFF/SUNION 一样整串都是键位。
    // 回包一律是整数：目标集写完之后的基数（差/交为空就是 0，"没送出去"也是 0，见开头那条说明）。
    http::obj_val set_union_store(const std::string &dst, const std::vector<std::string> &keys);
    http::obj_val set_intersect_store(const std::string &dst, const std::vector<std::string> &keys);
    http::obj_val set_difference_store(const std::string &dst, const std::vector<std::string> &keys);
    // SINTERCARD 才是真带 numkeys 的那条：数字位由本方法按 keys.size() 现算，其后紧跟的才是键段，
    // 段之后的一切（"LIMIT" 与它的数值）不会被当成键。keys 与数字必然同源，所以数字错了只会是
    // 服务端响（"Number of keys can't be greater than number of args" / "syntax error"，均实测）。
    // 空 keys 发出去的是 SINTERCARD 0，服务端回 -ERR 字符串（实测），本层不替你挡。
    // LIMIT 0 是"不限制"而不是"取 0 条"（实测 SINTERCARD 1 a LIMIT 0 == 不带 LIMIT 的 3）。
    http::obj_val set_intersect_card(const std::vector<std::string> &keys,
                                     const std::vector<std::string> &tail = {});

    // ===== 键级（string/list/set/zset/hash/stream 都适用，所以不挂 str_/hash_ 前缀）=====
    // TTL：-2 键不存在（含已过期）、-1 存在但永不过期、>0 剩余秒数。
    long long key_ttl(const std::string &key);
    // EXPIRE / PERSIST：1 改成功、0 没改（键不存在，或本来就没期限）。
    bool key_expire(const std::string &key, unsigned int sec);
    bool key_persist(const std::string &key);
    // EXISTS：1 在、0 不在。
    bool key_exists(const std::string &key);
    // TYPE：键不存在回 "none"，其余是 string / list / set / zset / hash / stream。
    // 它回的是文本不是整数，所以走 obj_val，不进上面那四个的整数口径。
    http::obj_val key_type(const std::string &key);

    // ===== 没被上面覆盖的命令，直接按参数位发（key 位表照样在 redis_conn.cpp 那一张里生效）=====
    http::obj_val command(const std::vector<std::string> &args);

    // ===== 协程版：与上面逐一对应 =====
    asio::awaitable<http::obj_val> async_str_set(const std::string &key, const std::string &value, const std::vector<std::string> &opts = {});
    asio::awaitable<http::obj_val> async_str_get(const std::string &key);
    asio::awaitable<http::obj_val> async_str_del(const std::vector<std::string> &keys);
    asio::awaitable<http::obj_val> async_hash_set(const std::string &key, const std::string &field, const std::string &value, const std::vector<std::string> &opts = {});
    asio::awaitable<http::obj_val> async_hash_get(const std::string &key, const std::string &field);
    asio::awaitable<http::obj_val> async_hash_getall(const std::string &key);
    asio::awaitable<http::obj_val> async_list_push_left(const std::string &key,
                                                        const std::vector<std::string> &values);
    asio::awaitable<http::obj_val> async_list_push_right(const std::string &key,
                                                         const std::vector<std::string> &values);
    asio::awaitable<http::obj_val> async_list_pop_left(const std::string &key, unsigned int count = 0);
    asio::awaitable<http::obj_val> async_list_pop_right(const std::string &key, unsigned int count = 0);
    asio::awaitable<http::obj_val> async_list_length(const std::string &key);
    asio::awaitable<http::obj_val> async_list_range(const std::string &key, long long start, long long stop);
    asio::awaitable<http::obj_val> async_list_at(const std::string &key, long long index);
    asio::awaitable<http::obj_val> async_list_set_at(const std::string &key, long long index, const std::string &value);
    asio::awaitable<http::obj_val> async_list_trim(const std::string &key, long long start, long long stop);
    asio::awaitable<http::obj_val> async_list_insert(const std::string &key, const std::string &where, const std::string &pivot, const std::string &value);
    asio::awaitable<http::obj_val> async_list_move(const std::string &src, const std::string &dst, const std::string &src_dir, const std::string &dst_dir);
    asio::awaitable<http::obj_val> async_stream_add(const std::string &key,
                                                    const std::vector<std::string> &fields,
                                                    const std::string &id                = "*",
                                                    const std::vector<std::string> &opts = {});
    asio::awaitable<http::obj_val> async_stream_len(const std::string &key);
    asio::awaitable<http::obj_val> async_stream_range(const std::string &key, const std::string &start, const std::string &end, unsigned int count = 0);
    asio::awaitable<http::obj_val> async_stream_read(const std::vector<std::string> &keys,
                                                     const std::vector<std::string> &ids,
                                                     const std::vector<std::string> &opts = {});
    asio::awaitable<http::obj_val> async_stream_group_create(const std::string &key, const std::string &group, const std::string &id, const std::vector<std::string> &opts = {});
    asio::awaitable<http::obj_val> async_stream_read_group(const std::string &group,
                                                           const std::string &consumer,
                                                           const std::vector<std::string> &keys,
                                                           const std::vector<std::string> &ids,
                                                           const std::vector<std::string> &opts = {});
    asio::awaitable<http::obj_val> async_stream_ack(const std::string &key, const std::string &group, const std::vector<std::string> &ids);
    asio::awaitable<http::obj_val> async_stream_pending(const std::string &key, const std::string &group, const std::vector<std::string> &tail = {});
    asio::awaitable<http::obj_val> async_stream_info(const std::string &key, const std::string &sub, const std::vector<std::string> &tail = {});
    asio::awaitable<http::obj_val> async_set_union_store(const std::string &dst,
                                                         const std::vector<std::string> &keys);
    asio::awaitable<http::obj_val> async_set_intersect_store(const std::string &dst,
                                                             const std::vector<std::string> &keys);
    asio::awaitable<http::obj_val> async_set_difference_store(const std::string &dst,
                                                              const std::vector<std::string> &keys);
    asio::awaitable<http::obj_val> async_set_intersect_card(const std::vector<std::string> &keys,
                                                            const std::vector<std::string> &tail = {});
    asio::awaitable<http::obj_val> async_set_add(const std::string &key,
                                                 const std::vector<std::string> &members);
    asio::awaitable<http::obj_val> async_set_remove(const std::string &key,
                                                    const std::vector<std::string> &members);
    asio::awaitable<http::obj_val> async_set_members(const std::string &key);
    asio::awaitable<http::obj_val> async_set_card(const std::string &key);
    asio::awaitable<http::obj_val> async_set_is_member(const std::string &key, const std::string &member);
    asio::awaitable<http::obj_val> async_set_are_members(const std::string &key,
                                                         const std::vector<std::string> &members);
    asio::awaitable<http::obj_val> async_set_pop(const std::string &key, unsigned int count = 0);
    asio::awaitable<http::obj_val> async_set_random_member(const std::string &key, unsigned int count = 0);
    asio::awaitable<http::obj_val> async_set_move(const std::string &src, const std::string &dst, const std::string &member);
    asio::awaitable<http::obj_val> async_set_union(const std::vector<std::string> &keys);
    asio::awaitable<http::obj_val> async_set_intersection(const std::vector<std::string> &keys);
    asio::awaitable<http::obj_val> async_set_difference(const std::vector<std::string> &keys);
    asio::awaitable<long long> async_key_ttl(const std::string &key);
    asio::awaitable<bool> async_key_expire(const std::string &key, unsigned int sec);
    asio::awaitable<bool> async_key_persist(const std::string &key);
    asio::awaitable<bool> async_key_exists(const std::string &key);
    asio::awaitable<http::obj_val> async_key_type(const std::string &key);
    asio::awaitable<http::obj_val> async_command(const std::vector<std::string> &args);

  private:
    std::string section_ = "default";
    // 命令级 key 前缀覆盖：空串 = 不覆盖，每条命令回落到该段配置的 prefix（两态，没有"显式不加前缀"）。
    // 只有构造时能设它，没有 setter —— 换前缀等于换租户，该换新对象。
    std::string prefix_;
    // 构造时解析到的段对象（可能为空，见 tag_sec()）
    std::shared_ptr<redis_pool_section_t> sec_;
};

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS
#endif// PZ_REDIS_CLIENT_H
