/*
 * test_redis_types.cpp — Redis 常用类型的"简单用法"演示，全部走 pz::redis_client
 * author Huang ziquan (黄自权)
 * date 2026-10-02
 *
 * 这一页有两个面孔：给人照着抄的用法样例，和自带断言的常驻回归入口。
 * 未开启 -DENABLE_REDIS 时路由仍然存在，只是回一句"未编译 redis 支持"：控制器由构建系统无条件登记进来，
 * 函数体不能整文件关掉，否则链接阶段找不到符号。
 *
 * 路由：
 *   redis/types                          跑六组 + 页级检查（13 + 12 + 18 + 18 + 23 + 10 + 6 = 100 条）
 *   redis/types?grp=string               只跑 String 组（13 条 + 页级 6 条）
 *   redis/types?grp=hash                 只跑 Hash 组（12 条 + 页级 6 条）
 *   redis/types?grp=list                 只跑 List 组（18 条 + 页级 6 条）
 *   redis/types?grp=stream               只跑 Stream 组（18 条 + 页级 6 条）
 *   redis/types?grp=set                  只跑 Set 组（键段 4 条 + 单键族 9 条 + 集合代数 3 条 = 23 条 + 页级 6 条）
 *   redis/types?grp=key                  只跑键级/锁组（10 条 + 页级 6 条）
 *   redis/types?grp=check                同上，只回摘要（不给 checks 明细，省流量）
 *   redis/types?grp=cleanup              只删本页键名表里的键，回残留条数
 *   redis/types?grp=leave                只写表里四把键、不删，供从服务端一侧核对物理键名用
 *   redis/types?sec=client1              换 conf 里另一段跑；那一段带 prefix，所以这一跑同时覆盖前缀通路
 *   redis/types/sync?n=50                同步入口（业务线程上借还），17 条 + 借还循环 + 耗时统计
 *
 * 键空间一律 pzt5: 开头，写死在命令参数里；每组跑完自己删干净，不含 KEYS/SCAN/FLUSHDB。
 * 换 ?sec=client1 时那一段在 conf 里配了 prefix，服务端物理键名会是 prefix+pzt5:... —— 前缀是连接层按
 * kKeyPositions 加的，本页看不见它，所以这里的键名口径不变；物理名要在服务端那一侧另行核对。
 */

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

#include "httppeer.h"
#ifdef ENABLE_REDIS
#include "redis_conn.h"
#include "redis_pool.h"
#include "redis_client.h"
#endif// ENABLE_REDIS

namespace http
{
#ifdef ENABLE_REDIS
namespace
{
// 每组条数固定；少一条就说明有用例被静默跳过（页级检查另算 6 条）
constexpr unsigned int kStringChecks = 13;
constexpr unsigned int kHashChecks   = 12;
constexpr unsigned int kListChecks   = 18;
constexpr unsigned int kStreamChecks = 18;
constexpr unsigned int kSetChecks    = 23;
constexpr unsigned int kKeyChecks    = 10;
constexpr unsigned int kKeyTableSize = 36;
constexpr unsigned int kPageChecks   = 6;
constexpr unsigned int kTypesExpectedChecks =
    kStringChecks + kHashChecks + kListChecks + kStreamChecks + kSetChecks + kKeyChecks + kPageChecks;
// 同步入口那一页 26 条：Y01..Y06 + YL01..YL03 + YS01..YS07 + 收尾的 Y07（共 17 条），
// 再加前缀覆盖那一族 YP01..YP09；不叠页级检查。
// YP 那九条是协程页 ?grp=pfxctor 的同步镜像，条数与断言必须逐条对上。
constexpr unsigned int kSyncChecks = 26;

struct check_item
{
    std::string name;
    bool pass;
    std::string detail;
};

struct types_bag
{
    std::vector<check_item> checks;
    unsigned int pass_n   = 0;
    unsigned int n_string = 0;
    unsigned int n_hash   = 0;
    unsigned int n_list   = 0;
    unsigned int n_stream = 0;
    unsigned int n_set    = 0;
    unsigned int n_key    = 0;

    void add_s(const std::string &name, bool pass, const std::string &detail = "")
    {
        checks.push_back({name, pass, detail});
        n_string++;
        if (pass)
            pass_n++;
    }
    void add_h(const std::string &name, bool pass, const std::string &detail = "")
    {
        checks.push_back({name, pass, detail});
        n_hash++;
        if (pass)
            pass_n++;
    }
    void add_l(const std::string &name, bool pass, const std::string &detail = "")
    {
        checks.push_back({name, pass, detail});
        n_list++;
        if (pass)
            pass_n++;
    }
    // Stream 组用 add_x（x 是键名表里那一批 x_ 键的前缀）
    void add_x(const std::string &name, bool pass, const std::string &detail = "")
    {
        checks.push_back({name, pass, detail});
        n_stream++;
        if (pass)
            pass_n++;
    }
    // Set 的"键段"一族用 add_st（键名表里挂 st_ 前缀那批）
    void add_st(const std::string &name, bool pass, const std::string &detail = "")
    {
        checks.push_back({name, pass, detail});
        n_set++;
        if (pass)
            pass_n++;
    }
    // 键级操作对五种类型通用，所以这一组不分类型前缀
    void add_k(const std::string &name, bool pass, const std::string &detail = "")
    {
        checks.push_back({name, pass, detail});
        n_key++;
        if (pass)
            pass_n++;
    }
    void add_p(const std::string &name, bool pass, const std::string &detail = "")
    {
        checks.push_back({name, pass, detail});
        if (pass)
            pass_n++;
    }
};

// 明细要进 JSON，先换成纯 ASCII 的可见写法（断言本身比的还是原始字节）。
// 这里必须连"非 ASCII 字节"一起转义：实测把 0xFF 原样写进响应体，整页 JSON 就非法了，
// 调用方解析会直接失败，看到的是"什么都没跑"而不是"哪条断言没通过"。
std::string tshow(const std::string &s)
{
    static const char *hexd = "0123456789abcdef";
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s)
    {
        unsigned char b = static_cast<unsigned char>(c);
        if (b == '\r')
            out += "\\r";
        else if (b == '\n')
            out += "\\n";
        else if (b == '\0')
            out += "\\0";
        else if (b == '\\')
            out += "\\\\";
        else if (b < 0x20 || b >= 0x7f)
        {
            out += "\\x";
            out.push_back(hexd[b >> 4]);
            out.push_back(hexd[b & 0x0f]);
        }
        else
            out.push_back(c);
    }
    return out;
}

const char *ttype(const obj_val &v)
{
    switch (v.get_type())
    {
    case obj_type::OBJECT: return "object";
    case obj_type::ARRAY: return "array";
    case obj_type::BOOL: return "bool";
    case obj_type::STRING: return "string";
    case obj_type::DOUBLE: return "double";
    case obj_type::INT: return "int";
    case obj_type::UINT: return "uint";
    case obj_type::LONG: return "long";
    case obj_type::ULONG: return "ulong";
    case obj_type::NIL: return "null";
    }
    return "?";
}

bool is_nil(const obj_val &v) { return v.get_type() == obj_type::NIL; }
bool is_str(const obj_val &v) { return v.get_type() == obj_type::STRING; }
bool is_num(const obj_val &v)
{
    obj_type t = v.get_type();
    return t == obj_type::LONG || t == obj_type::INT || t == obj_type::UINT || t == obj_type::ULONG;
}
bool is_arr(const obj_val &v) { return v.get_type() == obj_type::ARRAY; }

bool starts_with(const std::string &s, const char *p)
{
    std::size_t n = std::string(p).size();
    return s.size() >= n && s.compare(0, n, p) == 0;
}

// RESP2 里 map 就是扁平的 key,value,key,value…（XINFO STREAM 与 GROUPS/CONSUMERS 的每一格都是这个形状）。
// 按字段名取值的这一步属于调用方；返回值的下标，找不到回 -1 —— 不按下标猜是因为这个构建在 "length" 之后
// 还塞了 idmp-*/iids-* 一批私有字段，字段顺序不是可以依赖的口径。
long long flat_pos(const obj_val &arr, const std::string &field)
{
    if (!is_arr(arr))
        return -1;
    for (std::size_t i = 0; i + 1 < arr.size(); i += 2)
    {
        if (arr[i].to_string() == field)
            return static_cast<long long>(i + 1);
    }
    return -1;
}

// 上面那个按下标取值的两种口径：字段不存在时分别回 def 与空串，调用方按"读没读到"判断。
long long flat_int(const obj_val &arr, const std::string &field, long long def = -1)
{
    long long p = flat_pos(arr, field);
    return p >= 0 ? arr[static_cast<std::size_t>(p)].to_int() : def;
}

std::string flat_str(const obj_val &arr, const std::string &field)
{
    long long p = flat_pos(arr, field);
    return p >= 0 ? arr[static_cast<std::size_t>(p)].to_string() : std::string();
}

// RESP2 下 HGETALL 是扁平数组 [f1,v1,f2,v2]，配对成对象这一步属于调用方，不属于 redis 通路层
obj_val flat_pairs_to_object(const obj_val &flat)
{
    obj_val out;
    out.set_object();
    if (!is_arr(flat))
        return out;
    unsigned int n = flat.size();
    for (unsigned int i = 0; i + 1 < n; i += 2)
        out[flat[i].to_string()] = flat[i + 1].to_string();
    return out;
}

// 扁平数组里按字段名找值（字段名可能是二进制，不适合当 JSON 键，所以直接扫偶数位）
bool flat_find(const obj_val &flat, const std::string &field, std::string &value)
{
    if (!is_arr(flat))
        return false;
    unsigned int n = flat.size();
    for (unsigned int i = 0; i + 1 < n; i += 2)
    {
        if (flat[i].to_string() == field)
        {
            value = flat[i + 1].to_string();
            return true;
        }
    }
    return false;
}

// 本页用过的键名只有这一份表：用例和清理都从这儿出，不会漂
struct key_table
{
    std::string s_main, s_nx, s_ex, s_empty, s_bin, s_weird, s_del1, s_del2, s_strtype;
    std::string s_missing, s_missing2, s_noxp, s_lock;
    std::string h_main, h_main2, h_missing, h_weird, h_hastype;
    std::string l_main, l_trim, l_ins, l_src, l_dst, l_bin, l_missing;
    std::string x_main, x_nomk, x_missing, x_strtype;
    std::string st_a, st_b, st_dst, st_wrong, st_missing, st_num1, st_num2;

    std::vector<std::string> all() const
    {
        std::vector<std::string> v;
        for (const std::string *one : {&s_main, &s_nx, &s_ex, &s_empty, &s_bin, &s_weird, &s_del1, &s_del2, &s_strtype, &s_missing, &s_missing2, &s_noxp, &s_lock, &h_main, &h_main2, &h_missing, &h_weird, &h_hastype, &l_main, &l_trim, &l_ins, &l_src, &l_dst, &l_bin, &l_missing, &x_main, &x_nomk, &x_missing, &x_strtype, &st_a, &st_b, &st_dst, &st_wrong, &st_missing, &st_num1, &st_num2})
            v.push_back(*one);
        return v;
    }
};

key_table make_keys()
{
    key_table k;
    k.s_main     = "pzt5:s:main";
    k.s_nx       = "pzt5:s:nx";
    k.s_ex       = "pzt5:s:ex";
    k.s_empty    = "pzt5:s:empty";
    k.s_bin      = "pzt5:s:bin";
    k.s_weird    = "pzt5:s:ace 中文\r\n";// 键名里带空格/多字节/CR/LF：证明组包按字节长度，不按分隔符
    k.s_del1     = "pzt5:s:del1";
    k.s_del2     = "pzt5:s:del2";
    k.s_strtype  = "pzt5:s:strtype";
    k.s_missing  = "pzt5:s:nosuchkey";
    k.s_missing2 = "pzt5:s:nosuchkey2";
    k.s_noxp     = "pzt5:s:noexpire";
    k.s_lock     = "pzt5:s:lock";
    k.h_main     = "pzt5:h:main";
    k.h_main2    = "pzt5:h:main2";
    k.h_missing  = "pzt5:h:nosuchkey";
    k.h_weird    = "pzt5:h:weird";
    k.h_hastype  = "pzt5:h:hastype";
    k.l_main     = "pzt5:l:main";
    k.l_trim     = "pzt5:l:trim";
    k.l_ins      = "pzt5:l:ins";
    k.l_src      = "pzt5:l:src";
    k.l_dst      = "pzt5:l:dst";
    k.l_bin      = "pzt5:l:bin";
    k.l_missing  = "pzt5:l:nosuchkey";
    k.x_main     = "pzt5:x:main";
    k.x_nomk     = "pzt5:x:nomk";
    k.x_missing  = "pzt5:x:nosuchkey";
    k.x_strtype  = "pzt5:x:strtype";// 故意存成 String，用来发 XLEN 拿 WRONGTYPE
    k.st_a       = "pzt5:set:a";
    k.st_b       = "pzt5:set:b";
    k.st_dst     = "pzt5:set:dst";
    k.st_wrong   = "pzt5:set:stringkey";// 故意存成 String，交并差与 SINTERCARD 打它都回 WRONGTYPE
    k.st_missing = "pzt5:set:nosuchkey";
    // 键名就叫 "1" / "2"：Set 的 *STORE 没有 numkeys 参数位，源键位上出现纯数字是**合法键名**。
    // 键位表要是把它当数字解析，就少加一位前缀、命令照样成功、读的是另一个物理键 ⇒ 这两个键名就是那条静默路径的靶子。
    // 本机 db0 实测无裸数字键（100001 个键全是 bench_direct_*），跑完由 ?grp=cleanup 收干净。
    k.st_num1 = "1";
    k.st_num2 = "2";
    return k;
}

// 唯一的数据通路：协程 → redis_client（它每条命令内部向池借还一次）
asio::awaitable<void> cleanup_keys(pz::redis::redis_client &rc, const std::vector<std::string> &keys)
{
    if (keys.empty())
        co_return;
    co_await rc.async_str_del(keys);
    co_return;
}

asio::awaitable<unsigned int> count_existing(pz::redis::redis_client &rc, const std::vector<std::string> &keys)
{
    if (keys.empty())
        co_return 0;
    std::vector<std::string> args = {"EXISTS"};
    for (const auto &k : keys)
        args.push_back(k);
    auto r = co_await rc.async_command(args);
    co_return is_num(r) ? static_cast<unsigned int>(r.to_int()) : 0;
}

// ==================== String：str_set / str_get / str_del ====================
asio::awaitable<void> run_string(pz::redis::redis_client &rc, types_bag &bag, obj_val &readings)
{
    key_table k                 = make_keys();
    const std::string bin_value = std::string("a\r\n\0b\xff", 6);// a CR LF NUL b 0xFF = 6 字节

    // S01 SET 新键 ⇒ OK
    auto s1 = co_await rc.async_str_set(k.s_main, "v1");
    bag.add_s("S01-set-new-ok", is_str(s1) && s1.to_string() == "OK", "got=" + std::string(ttype(s1)) + "(" + tshow(s1.to_string()) + ") want=string(OK)");

    // S02 GET 读回写入值
    auto g1 = co_await rc.async_str_get(k.s_main);
    bag.add_s("S02-get-roundtrip", is_str(g1) && g1.to_string() == "v1", "got=" + std::string(ttype(g1)) + "(" + tshow(g1.to_string()) + ") want=v1");

    // S03 覆盖：回 OK，且读回是新值
    auto s2 = co_await rc.async_str_set(k.s_main, "v2");
    auto g2 = co_await rc.async_str_get(k.s_main);
    bag.add_s("S03-overwrite-ok-and-newvalue",
              is_str(s2) && s2.to_string() == "OK" && is_str(g2) && g2.to_string() == "v2",
              "reply=" + std::string(ttype(s2)) + "(" + tshow(s2.to_string()) + ") readback=" + tshow(g2.to_string()));

    // S04 SET NX 打在已存在键上：回 null（不是 0），且原值一格没动
    auto s3 = co_await rc.async_str_set(k.s_main, "v3", {"NX"});
    auto g3 = co_await rc.async_str_get(k.s_main);
    bag.add_s("S04-set-nx-on-existing-nil-and-unchanged",
              is_nil(s3) && is_str(g3) && g3.to_string() == "v2",
              "reply_type=" + std::string(ttype(s3)) + " readback=" + std::string(ttype(g3)) + "(" + tshow(g3.to_string()) + ")");

    // S05 SET NX 打在不存在的键上：OK
    auto s4 = co_await rc.async_str_set(k.s_nx, "only", {"NX"});
    auto g4 = co_await rc.async_str_get(k.s_nx);
    bag.add_s("S05-set-nx-new-ok", is_str(s4) && s4.to_string() == "OK" && is_str(g4) && g4.to_string() == "only", "reply=" + tshow(s4.to_string()) + " readback=" + tshow(g4.to_string()));

    // S06 SET key val EX 30：TTL 落在 (0,30]
    co_await rc.async_str_set(k.s_ex, "x", {"EX", "30"});
    long long ttl = co_await rc.async_key_ttl(k.s_ex);
    bool ttl_ok   = ttl > 0 && ttl <= 30;
    bag.add_s("S06-set-ex-then-ttl-in-range", ttl_ok, "ttl=" + std::to_string(ttl) + " want=1..30");

    // S07 空串值 vs 键不存在：两者必须分得开（空串是 string，不存在是 null）
    co_await rc.async_str_set(k.s_empty, "");
    auto ge = co_await rc.async_str_get(k.s_empty);
    auto gm = co_await rc.async_str_get(k.s_missing);
    bag.add_s("S07-empty-string-not-nil",
              is_str(ge) && ge.to_string().empty() && is_nil(gm) && ttype(ge) != ttype(gm),
              "empty_value=" + std::string(ttype(ge)) + " missing_key=" + std::string(ttype(gm)));

    // S08 二进制值原样回读（含 CR/LF/NUL/0xFF）
    co_await rc.async_str_set(k.s_bin, bin_value);
    auto gb = co_await rc.async_str_get(k.s_bin);
    bag.add_s("S08-binary-value-roundtrip",
              is_str(gb) && gb.to_string() == bin_value,
              "got=" + tshow(gb.to_string()) + "(" + std::to_string(gb.to_string().size()) + "B) want=" +
                  tshow(bin_value) + "(" + std::to_string(bin_value.size()) + "B)");

    // S09 DEL 单键：回 1，读回 null
    auto d1 = co_await rc.async_str_del({k.s_nx});
    auto g5 = co_await rc.async_str_get(k.s_nx);
    bag.add_s("S09-del-single-one-and-gone", is_num(d1) && d1.to_int() == 1 && is_nil(g5), "del=" + tshow(d1.to_string()) + " after_get=" + std::string(ttype(g5)));

    // S10 DEL 多键（其中一格不存在）：只数个得到的
    co_await rc.async_str_set(k.s_del1, "1");
    co_await rc.async_str_set(k.s_del2, "2");
    auto d2 = co_await rc.async_str_del({k.s_del1, k.s_del2, k.s_missing});
    bag.add_s("S10-del-multi-counts-existing", is_num(d2) && d2.to_int() == 2, "del=" + tshow(d2.to_string()) + " want=2(2 keys exist + 1 missing)");

    // S11 DEL 全不存在：回 0，不是 null
    auto d3 = co_await rc.async_str_del({k.s_missing, k.s_missing2});
    bag.add_s("S11-del-absent-zero-not-nil", is_num(d3) && d3.to_int() == 0, "del=" + std::string(ttype(d3)) + "(" + tshow(d3.to_string()) + ") want=number(0)");

    // S12 对 String 键执行 HSET：服务端 -ERR ⇒ 一个普通字符串。
    // 这一条同时记下"错误面塌成字符串"这个事实：错误文本和"值恰好长这样的 bulk"不可区分。
    co_await rc.async_str_set(k.s_strtype, "plain");
    auto hErr = co_await rc.async_hash_set(k.s_strtype, "f", "v");
    bag.add_s("S12-wrongtype-surfaces-as-string",
              is_str(hErr) && starts_with(hErr.to_string(), "WRONGTYPE"),
              "got=" + std::string(ttype(hErr)) + "(" + tshow(hErr.to_string()) + ") note=error-and-same-text-bulk-indistinguishable");

    // S13 键名里带空格/多字节/CR/LF：组包按字节长度，不按分隔符
    co_await rc.async_str_set(k.s_weird, "w");
    auto gw = co_await rc.async_str_get(k.s_weird);
    bag.add_s("S13-weird-key-name-roundtrip", is_str(gw) && gw.to_string() == "w", "key=" + tshow(k.s_weird) + " got=" + tshow(gw.to_string()));

    readings["string"]["get_weird"]       = gw.to_json();
    readings["string"]["bin_value_shown"] = tshow(bin_value);
    co_return;
}

// ==================== Hash：hash_set / hash_get / hash_getall ====================
asio::awaitable<void> run_hash(pz::redis::redis_client &rc, types_bag &bag, obj_val &readings)
{
    key_table k                 = make_keys();
    const std::string field_bin = std::string("f\r\n\0\xe4\xb8\xad", 7);// f CR LF NUL + UTF-8 "中" = 7 字节

    // H01 HSET 新字段 ⇒ 1
    auto a1 = co_await rc.async_hash_set(k.h_main, "f1", "v1");
    bag.add_h("H01-hset-new-field-one", is_num(a1) && a1.to_int() == 1, "got=" + std::string(ttype(a1)) + "(" + tshow(a1.to_string()) + ") want=1");

    // H02 HSET 覆盖已有字段 ⇒ 0，但值确实被改写（回包数字与写入不同步 ⇒ 断言要看读回的值）
    auto a2 = co_await rc.async_hash_set(k.h_main, "f1", "v2");
    auto b2 = co_await rc.async_hash_get(k.h_main, "f1");
    bag.add_h("H02-hset-overwrite-zero-but-written",
              is_num(a2) && a2.to_int() == 0 && is_str(b2) && b2.to_string() == "v2",
              "reply=" + tshow(a2.to_string()) + " readback=" + std::string(ttype(b2)) + "(" + tshow(b2.to_string()) + ")");

    // H03 HGET 缺字段 ⇒ null
    auto b3 = co_await rc.async_hash_get(k.h_main, "nosuchfield");
    bag.add_h("H03-hget-missing-field-nil", is_nil(b3), "got=" + std::string(ttype(b3)));

    // H04 HGETALL 扁平数组 ⇒ 调用方自己配对成对象
    co_await rc.async_hash_set(k.h_main, "f2", "x");
    auto all4    = co_await rc.async_hash_getall(k.h_main);
    obj_val obj4 = flat_pairs_to_object(all4);
    std::string v_f1, v_f2;
    bool p4 = flat_find(all4, "f1", v_f1) && flat_find(all4, "f2", v_f2);
    bag.add_h("H04-hgetall-flat-even-and-pairs",
              is_arr(all4) && all4.size() == 4 && (all4.size() % 2) == 0 && p4 && v_f1 == "v2" && v_f2 == "x" &&
                  obj4.size() == 2 && obj4["f1"].to_string() == "v2" && obj4["f2"].to_string() == "x",
              "size=" + std::to_string(all4.size()) + " f1=" + tshow(v_f1) + " f2=" + tshow(v_f2) +
                  " raw=" + tshow(all4.to_json()));
    readings["hash"]["main_paired"] = obj4.to_json();

    // H05 HGETALL 不存在的键 ⇒ 空数组（不是 null，和 H03 的"字段不存在"两种形状）
    auto all5 = co_await rc.async_hash_getall(k.h_missing);
    bag.add_h("H05-hgetall-absent-key-empty-array", is_arr(all5) && all5.size() == 0, "got=" + std::string(ttype(all5)) + "(" + tshow(all5.to_json()) + ") want=array(0)");

    // H06 一次多字段（本层只管单字段，多字段走池口一行转发）⇒ 回新建条数
    auto c6   = co_await rc.async_command({"HSET", k.h_main2, "a", "1", "b", "2"});
    auto all6 = co_await rc.async_hash_getall(k.h_main2);
    bag.add_h("H06-hset-multi-field-once-two", is_num(c6) && c6.to_int() == 2 && is_arr(all6) && all6.size() == 4, "reply=" + tshow(c6.to_string()) + " getall_size=" + std::to_string(all6.size()));

    // H07 HSETNX 已有字段 ⇒ 0，且值不动。
    // 这里用 HSETNX 而不是 HSET 的 NX 位：本机 redis 8.8.0 这个构建对 "HSET k NX f v" 和
    // "HSET k f v NX" 两个位置一律回 wrong number of arguments（上游 4.0 起支持），
    // NX 位的透传形状由 H12 验证；参数落点位置从回包看不出来，页内测不到。
    auto d7 = co_await rc.async_command({"HSETNX", k.h_main, "f1", "zzz"});
    auto e7 = co_await rc.async_hash_get(k.h_main, "f1");
    bag.add_h("H07-hsetnx-existing-zero-unchanged",
              is_num(d7) && d7.to_int() == 0 && is_str(e7) && e7.to_string() == "v2",
              "reply=" + tshow(d7.to_string()) + " readback=" + tshow(e7.to_string()));

    // H08 HSETNX 新字段 ⇒ 1，且读得到
    auto d8 = co_await rc.async_command({"HSETNX", k.h_main, "f3", "v3"});
    auto e8 = co_await rc.async_hash_get(k.h_main, "f3");
    bag.add_h("H08-hsetnx-new-one", is_num(d8) && d8.to_int() == 1 && is_str(e8) && e8.to_string() == "v3", "reply=" + tshow(d8.to_string()) + " readback=" + tshow(e8.to_string()));

    // H09 字段名/值都是二进制（CR/LF/NUL/多字节）：原样写、原样读、在扁平数组里扫得回来
    co_await rc.async_hash_set(k.h_weird, field_bin, field_bin);
    auto f9   = co_await rc.async_hash_get(k.h_weird, field_bin);
    auto all9 = co_await rc.async_hash_getall(k.h_weird);
    std::string got9;
    bool scanned = flat_find(all9, field_bin, got9);
    bag.add_h("H09-binary-field-and-value-roundtrip",
              is_str(f9) && f9.to_string() == field_bin && scanned && got9 == field_bin,
              "field_len=" + std::to_string(field_bin.size()) + "B(" + tshow(field_bin) + ") readback=" +
                  std::string(ttype(f9)) + "(" + tshow(f9.to_string()) + ") scanned_from_getall=" + (scanned ? "1" : "0"));

    // H10 hash 键也能用 str_del 删掉（DEL 不认类型）
    auto g10 = co_await rc.async_str_del({k.h_main});
    auto h10 = co_await rc.async_hash_getall(k.h_main);
    bag.add_h("H10-del-hash-key-then-getall-empty",
              is_num(g10) && g10.to_int() == 1 && is_arr(h10) && h10.size() == 0,
              "del=" + tshow(g10.to_string()) + " getall=" + std::string(ttype(h10)) + "(" + std::to_string(h10.size()) + ")");

    // H11 反向类型冲突：对 hash 键执行 GET ⇒ -ERR 塌成字符串
    co_await rc.async_command({"HSET", k.h_hastype, "a", "b"});
    auto i11 = co_await rc.async_str_get(k.h_hastype);
    bag.add_h("H11-get-on-hash-key-wrongtype",
              is_str(i11) && starts_with(i11.to_string(), "WRONGTYPE"),
              "got=" + std::string(ttype(i11)) + "(" + tshow(i11.to_string()) + ")");

    // H12 opts 透传确实送到了服务端：一个不存在的选项必须让命令失败，且那个字段不能被写出来。
    // 反向验证：本层若把 opts 丢了，这里回的是整数 1（HSET 成功建字段）⇒ 这条断言就会失败。
    auto l12 = co_await rc.async_hash_set(k.h_main2, "f_bogus", "v", {"BOGUS"});
    auto m12 = co_await rc.async_hash_get(k.h_main2, "f_bogus");
    bag.add_h("H12-opts-passthrough-reaches-server",
              is_str(l12) && starts_with(l12.to_string(), "ERR") && is_nil(m12),
              "reply=" + std::string(ttype(l12)) + "(" + tshow(l12.to_string()) + ") field=" + std::string(ttype(m12)));

    readings["hash"]["binary_field_getall_flat"] = all9.to_json();
    readings["hash"]["multi_field_getall_flat"]  = all6.to_json();
    co_return;
}

// ==================== List：11 条命令 ====================
// 数组内容拼成可读的一串（明细要用；断言比的还是原始字节，不是这串）
std::string arr_show(const obj_val &arr)
{
    if (!is_arr(arr))
        return std::string("<") + ttype(arr) + ">";
    std::string out;
    for (unsigned int i = 0; i < arr.size(); i++)
    {
        if (i > 0)
            out += ",";
        out += tshow(arr[i].to_string());
    }
    return out;
}

// 集合成员：服务端不保序，所以只比"排序后的成员集"，不比顺序
std::string set_show(const obj_val &arr)
{
    if (!is_arr(arr))
        return std::string("<") + ttype(arr) + ">";
    std::vector<std::string> items;
    for (unsigned int i = 0; i < arr.size(); i++)
        items.push_back(arr[i].to_string());
    std::sort(items.begin(), items.end());
    std::string out;
    for (std::size_t i = 0; i < items.size(); i++)
    {
        if (i > 0)
            out += ",";
        out += tshow(items[i]);
    }
    return out;
}

asio::awaitable<void> run_list(pz::redis::redis_client &rc, types_bag &bag, obj_val &readings)
{
    key_table k              = make_keys();
    const std::string bin_el = std::string("e\r\n\0f\xff", 6);// e CR LF NUL f 0xFF = 6 字节

    // 这一组靠"元素顺序"和"剩余长度"判定，别的组跑完留没留键都不能影响它 ⇒ 先把自己这几个键清干净
    co_await rc.async_str_del({k.l_main, k.l_trim, k.l_ins, k.l_src, k.l_dst, k.l_bin});

    // L01 LPUSH 一次三个 ⇒ 新长度 3；LRANGE 0 -1 回 [c,b,a]。
    // 期望值是实测的（本机 redis 8.8.0）：LPUSH 逐个往头部塞，所以顺序是反的。
    auto p1 = co_await rc.async_list_push_left(k.l_main, {"a", "b", "c"});
    auto r1 = co_await rc.async_list_range(k.l_main, 0, -1);
    bag.add_l("L01-lpush-three-and-lrange-is-reverse-order",
              is_num(p1) && p1.to_int() == 3 && is_arr(r1) && r1.size() == 3 && r1[0].to_string() == "c" &&
                  r1[1].to_string() == "b" && r1[2].to_string() == "a",
              "len=" + tshow(p1.to_string()) + " range=[" + arr_show(r1) + "] want=[c,b,a]");

    // L02 RPUSH 落在尾部 ⇒ 长度 +1，最后一个元素是新值
    auto p2 = co_await rc.async_list_push_right(k.l_main, {"d"});
    auto r2 = co_await rc.async_list_range(k.l_main, 0, -1);
    bag.add_l("L02-rpush-appends-at-tail",
              is_num(p2) && p2.to_int() == 4 && is_arr(r2) && r2.size() == 4 && r2[3].to_string() == "d",
              "len=" + tshow(p2.to_string()) + " range=[" + arr_show(r2) + "] want=[c,b,a,d]");

    // L03 LLEN：存在的键数长度，不存在的键回 0（不是 null —— 和 GET 不存在键的 null 两种形状）
    auto c3a = co_await rc.async_list_length(k.l_main);
    auto c3b = co_await rc.async_list_length(k.l_missing);
    bag.add_l("L03-llen-present-and-absent-zero",
              is_num(c3a) && c3a.to_int() == 4 && is_num(c3b) && c3b.to_int() == 0,
              "present=" + std::string(ttype(c3a)) + "(" + tshow(c3a.to_string()) + ") absent=" +
                  std::string(ttype(c3b)) + "(" + tshow(c3b.to_string()) + ") want=4/0");

    // L04 LINDEX 正负索引：-1 是最后一个（负数按 redis 自己的口径直送，本层不换算）
    auto i4a = co_await rc.async_list_at(k.l_main, 0);
    auto i4b = co_await rc.async_list_at(k.l_main, -1);
    bag.add_l("L04-lindex-head-and-negative-tail",
              is_str(i4a) && i4a.to_string() == "c" && is_str(i4b) && i4b.to_string() == "d",
              "idx0=" + tshow(i4a.to_string()) + " idx-1=" + tshow(i4b.to_string()) + " want=c/d");

    // L05 LINDEX 越界 ⇒ null
    auto i5 = co_await rc.async_list_at(k.l_main, 99);
    bag.add_l("L05-lindex-out-of-range-nil", is_nil(i5), "got=" + std::string(ttype(i5)) + " want=null");

    // L06 LSET 成功 ⇒ +OK 字符串（不是整数 1），且读回新值
    auto s6 = co_await rc.async_list_set_at(k.l_main, 1, "B2");
    auto g6 = co_await rc.async_list_at(k.l_main, 1);
    bag.add_l("L06-lset-ok-string-and-readback",
              is_str(s6) && s6.to_string() == "OK" && is_str(g6) && g6.to_string() == "B2",
              "reply=" + std::string(ttype(s6)) + "(" + tshow(s6.to_string()) + ") readback=" +
                  std::string(ttype(g6)) + "(" + tshow(g6.to_string()) + ")");

    // L07 LSET 越界 ⇒ -ERR 塌成字符串。具体文本按实测只断前缀：
    // 越界回 "index out of range"、键不存在回 "no such key"，两种文本都由服务端决定，锁死文本就是替它写测试。
    auto s7 = co_await rc.async_list_set_at(k.l_main, 99, "x");
    bag.add_l("L07-lset-out-of-range-err-string", is_str(s7) && starts_with(s7.to_string(), "ERR"), "got=" + std::string(ttype(s7)) + "(" + tshow(s7.to_string()) + ") want=ERR*");

    // L08 带 count 的 LPOP ⇒ 数组，按弹出顺序（此时 l_main = [c,B2,a,d]）
    auto po8 = co_await rc.async_list_pop_left(k.l_main, 2);
    bag.add_l("L08-lpop-with-count-is-array",
              is_arr(po8) && po8.size() == 2 && po8[0].to_string() == "c" && po8[1].to_string() == "B2",
              "got=" + std::string(ttype(po8)) + "([" + arr_show(po8) + "]) want=array[c,B2]");

    // L09 不传 count ⇒ bulk（单个值）。这条同时是 count 哨兵的证据：count==0 时该参数整格不发，
    // 真把 0 发出去（LPOP k 0）实测服务端不报错、回的是**空数组** —— 回包形状和"省略"不一样，
    // 所以"0 = 不发"不是偷懒，是这两种形状必须分开。
    auto po9  = co_await rc.async_list_pop_left(k.l_main);
    auto len9 = co_await rc.async_list_length(k.l_main);
    bag.add_l("L09-lpop-omitted-count-is-bulk-not-array",
              is_str(po9) && po9.to_string() == "a" && is_num(len9) && len9.to_int() == 1,
              "got=" + std::string(ttype(po9)) + "(" + tshow(po9.to_string()) + ") remaining=" +
                  tshow(len9.to_string()) + " want=bulk(a)/1");

    // L10 RPOP 弹尾，弹空之后键直接消失（不是"空列表"）：LLEN 0、TYPE none、EXISTS false
    auto po10a = co_await rc.async_list_pop_right(k.l_main);
    auto po10b = co_await rc.async_list_pop_right(k.l_main);
    auto len10 = co_await rc.async_list_length(k.l_main);
    auto t10   = co_await rc.async_key_type(k.l_main);
    bag.add_l("L10-rpop-tail-then-emptied-key-disappears",
              is_str(po10a) && po10a.to_string() == "d" && is_nil(po10b) && is_num(len10) &&
                  len10.to_int() == 0 && is_str(t10) && t10.to_string() == "none",
              "popped=" + tshow(po10a.to_string()) + " next=" + std::string(ttype(po10b)) + " llen=" +
                  tshow(len10.to_string()) + " type=" + tshow(t10.to_string()) + " want=d/nil/0/none");

    // L11 LTRIM ⇒ OK 字符串，且只剩区间里那几个（[4,3,2,1] 留 0..1 ⇒ [4,3]）
    co_await rc.async_list_push_left(k.l_trim, {"1", "2", "3", "4"});
    auto tr11 = co_await rc.async_list_trim(k.l_trim, 0, 1);
    auto r11  = co_await rc.async_list_range(k.l_trim, 0, -1);
    bag.add_l("L11-ltrim-ok-and-range-shrinks",
              is_str(tr11) && tr11.to_string() == "OK" && is_arr(r11) && r11.size() == 2 &&
                  r11[0].to_string() == "4" && r11[1].to_string() == "3",
              "reply=" + std::string(ttype(tr11)) + "(" + tshow(tr11.to_string()) + ") after=[" + arr_show(r11) +
                  "] want=[4,3]");

    // L12 LINSERT BEFORE / AFTER ⇒ 每次回新长度
    co_await rc.async_list_push_left(k.l_ins, {"b"});
    auto in12a = co_await rc.async_list_insert(k.l_ins, "BEFORE", "b", "a");
    auto in12b = co_await rc.async_list_insert(k.l_ins, "AFTER", "b", "c");
    auto r12   = co_await rc.async_list_range(k.l_ins, 0, -1);
    bag.add_l("L12-linsert-before-after-new-length",
              is_num(in12a) && in12a.to_int() == 2 && is_num(in12b) && in12b.to_int() == 3 && is_arr(r12) &&
                  r12.size() == 3 && r12[0].to_string() == "a" && r12[2].to_string() == "c",
              "before=" + tshow(in12a.to_string()) + " after=" + tshow(in12b.to_string()) + " list=[" +
                  arr_show(r12) + "] want=2/3/[a,b,c]");

    // L13 pivot 找不到 ⇒ -1；键不存在 ⇒ 0（实测服务端真会回 0）
    auto in13 = co_await rc.async_list_insert(k.l_ins, "BEFORE", "nosuchpivot", "z");
    bag.add_l("L13-linsert-missing-pivot-minus-one", is_num(in13) && in13.to_int() == -1, "got=" + tshow(in13.to_string()) + " want=-1");

    // L14 键不存在 ⇒ 0。这条用例存在的理由是把"0 是服务端会真产生的数"钉住 ——
    // 所以新命令一律回 obj_val，不走整数收口（走收口的话"没送出去"和这个 0 就分不开）。
    auto in14 = co_await rc.async_list_insert(k.l_missing, "BEFORE", "q", "r");
    bag.add_l("L14-linsert-absent-key-zero-is-a-real-reply", is_num(in14) && in14.to_int() == 0, "got=" + std::string(ttype(in14)) + "(" + tshow(in14.to_string()) + ") want=0");

    // L15 LMOVE src dst LEFT RIGHT ⇒ 回搬过去的那个元素，两边长度各动一格（src 少 1，dst 尾追）
    co_await rc.async_list_push_left(k.l_src, {"s1", "s2"});// [s2,s1]
    co_await rc.async_list_push_left(k.l_dst, {"d1"});      // [d1]
    auto mv15 = co_await rc.async_list_move(k.l_src, k.l_dst, "LEFT", "RIGHT");
    auto ls15 = co_await rc.async_list_length(k.l_src);
    auto ld15 = co_await rc.async_list_range(k.l_dst, 0, -1);
    bag.add_l("L15-lmove-returns-moved-element-and-both-lengths",
              is_str(mv15) && mv15.to_string() == "s2" && is_num(ls15) && ls15.to_int() == 1 && is_arr(ld15) &&
                  ld15.size() == 2 && ld15[1].to_string() == "s2",
              "moved=" + std::string(ttype(mv15)) + "(" + tshow(mv15.to_string()) + ") src_len=" +
                  tshow(ls15.to_string()) + " dst=[" + arr_show(ld15) + "] want=s2/1/[d1,s2]");

    // L16 LRANGE 不存在的键 ⇒ 空数组（不是 null）
    auto r16 = co_await rc.async_list_range(k.l_missing, 0, -1);
    bag.add_l("L16-lrange-absent-key-empty-array-not-nil", is_arr(r16) && r16.size() == 0, "got=" + std::string(ttype(r16)) + "(" + std::to_string(r16.size()) + ") want=array(0)");

    // L17 元素是二进制（CR/LF/NUL/0xFF）：原样写、原样读，组包按字节长度不按分隔符
    co_await rc.async_list_push_left(k.l_bin, {bin_el});
    auto g17 = co_await rc.async_list_at(k.l_bin, 0);
    bag.add_l("L17-binary-element-roundtrip", is_str(g17) && g17.to_string() == bin_el, "got=" + tshow(g17.to_string()) + "(" + std::to_string(g17.to_string().size()) + "B) want=" + tshow(bin_el) + "(" + std::to_string(bin_el.size()) + "B)");

    // L18 对 List 键发 GET ⇒ WRONGTYPE，且 key_type 正向断言它确实是 list（不再只能靠回包文本猜类型）
    auto w18 = co_await rc.async_str_get(k.l_ins);
    auto t18 = co_await rc.async_key_type(k.l_ins);
    bag.add_l("L18-get-on-list-key-wrongtype-and-keytype-list",
              is_str(w18) && starts_with(w18.to_string(), "WRONGTYPE") && is_str(t18) && t18.to_string() == "list",
              "get=" + std::string(ttype(w18)) + "(" + tshow(w18.to_string()) + ") type=" +
                  std::string(ttype(t18)) + "(" + tshow(t18.to_string()) + ") want=WRONGTYPE*/list");

    readings["list"]["l_main_after_lpush"]   = r1.to_json();
    readings["list"]["lmove_dst"]            = ld15.to_json();
    readings["list"]["binary_element_shown"] = tshow(g17.to_string());
    co_return;
}

// ==================== Stream：8 条命令（消息队列）====================
// 这一组里 S09/S10/S12/S14 是**非对称**断言：它们比的是服务端回包里的物理流名和组名记账，
// 不是"本页写进去又读回来"。页内的读写都走同一个段，前缀加错、加漏两边会一起错而查不出来，
// 所以键位面必须有这种"从回包/服务端状态取值"的用例。
asio::awaitable<void> run_stream(pz::redis::redis_client &rc, types_bag &bag, obj_val &readings)
{
    key_table k               = make_keys();
    const std::string grpname = "pzt5g";// 组名不是 key：被加前缀就找不到组（S12/S14 会立刻失败）
    const std::string cname   = "pzt5c";
    const std::string bigid   = "1999999999999-0";

    // 段配了什么前缀得从池里取（断言要用它和回包里的流名比对）
    std::string sec_prefix;
    if (auto s = pz::redis::get_redis_pool().section(rc.section()))
        sec_prefix = s->take_config().prefix;

    // id 拆成可比的数：毫秒段*10000 + 序号段（同毫秒连发时序号才递增，只比毫秒会漏判）
    auto id_num = [](const std::string &s) -> long long
    {
        std::size_t dash = s.find('-');
        if (dash == std::string::npos)
            return -1;
        try
        {
            return std::stoll(s.substr(0, dash)) * 10000 + std::stoll(s.substr(dash + 1));
        }
        catch (...)
        {
            return -1;
        }
    };

    co_await rc.async_str_del({k.x_main, k.x_nomk, k.x_missing, k.x_strtype});

    // S01 XADD 回的是 entry id 文本（"<毫秒>-<序号>"），不是整数
    auto a1         = co_await rc.async_stream_add(k.x_main, {"f", "v"});
    std::string id1 = is_str(a1) ? a1.to_string() : "";
    bag.add_x("S01-xadd-returns-entry-id-text", is_str(a1) && id_num(id1) > 0, "got=" + std::string(ttype(a1)) + "(" + tshow(id1) + ") want=<ms>-<seq>");

    // S02 再写一条 ⇒ id 严格变大（同毫秒时靠序号段）
    auto a2         = co_await rc.async_stream_add(k.x_main, {"f1", "v1", "f2", "v2"});
    std::string id2 = is_str(a2) ? a2.to_string() : "";
    bag.add_x("S02-second-auto-id-strictly-greater", id_num(id2) > id_num(id1), "id1=" + tshow(id1) + " id2=" + tshow(id2));

    // S03 显式大 id 原样收下并原样回
    auto a3 = co_await rc.async_stream_add(k.x_main, {"c", "3"}, bigid);
    bag.add_x("S03-explicit-large-id-accepted-verbatim", is_str(a3) && a3.to_string() == bigid, "got=" + std::string(ttype(a3)) + "(" + tshow(a3.to_string()) + ") want=" + bigid);

    // S04 显式小 id ⇒ -ERR 塌成字符串（不是 null；"没送出去"才是 null）
    auto a4 = co_await rc.async_stream_add(k.x_main, {"z", "9"}, "1-1");
    bag.add_x("S04-explicit-small-id-err-string-not-nil", is_str(a4) && starts_with(a4.to_string(), "ERR"), "got=" + std::string(ttype(a4)) + "(" + tshow(a4.to_string()) + ") want=ERR*");

    // S05 NOMKSTREAM 打在不在的键上 ⇒ null（XADD 也能为空），且 XLEN 那侧数出来是 0 不是 null
    auto a5  = co_await rc.async_stream_add(k.x_nomk, {"f", "v"}, "*", {"NOMKSTREAM"});
    auto l5  = co_await rc.async_stream_len(k.x_nomk);
    auto l5b = co_await rc.async_stream_len(k.x_missing);
    bag.add_x("S05-nomkstream-absent-key-nil-and-xlen-zero",
              is_nil(a5) && is_num(l5) && l5.to_int() == 0 && is_num(l5b) && l5b.to_int() == 0,
              "add=" + std::string(ttype(a5)) + " xlen_nomk=" + tshow(l5.to_string()) +
                  " xlen_missing=" + tshow(l5b.to_string()) + " want=null/0/0");

    // S06 opts 落在 key 与 id 之间才成立：五次带 MAXLEN 2 的写入后只剩 2 条。
    // 拼错位置（比如把 MAXLEN 2 丢到字段段后面）服务端直接报参数错，XLEN 就回不来 2。
    obj_val a6;
    for (unsigned int i = 1; i <= 5; i++)
        a6 = co_await rc.async_stream_add(k.x_nomk, {"i", std::to_string(i)}, "*", {"MAXLEN", "2"});
    auto l6 = co_await rc.async_stream_len(k.x_nomk);
    bag.add_x("S06-xadd-maxlen-opts-land-before-id", is_num(l6) && l6.to_int() == 2, "last_id=" + tshow(a6.to_string()) + " xlen=" + tshow(l6.to_string()) + " want=2");

    // S07 XRANGE 是 [[id,[f,v]],…]：字段段扁平交替，本层不配对
    auto r7  = co_await rc.async_stream_range(k.x_nomk, "-", "+");
    bool ok7 = is_arr(r7) && r7.size() == 2 && is_arr(r7[0]) && r7[0].size() == 2 && is_arr(r7[0][1]) &&
               r7[0][1].size() == 2 && r7[0][1][0].to_string() == "i" && r7[0][1][1].to_string() == "4" &&
               r7[1][1][1].to_string() == "5";
    bag.add_x("S07-xrange-nested-and-fields-flat", ok7, "got=[" + arr_show(r7) + "] size=" + std::to_string(is_arr(r7) ? r7.size() : 0) + " want=2 entries [[i,4],[i,5]]");

    // S08 COUNT 参数的两种取值：count=0 ⇒ 整格不发（回 2 条），count=1 ⇒ 发带关键字的 COUNT 1（回 1 条）。
    // 只发数字会撞 ERR syntax error（实测），把 0 传下去则回空数组、和"区间没东西"同形（也实测）——
    // 两种错法都会被这条用例抓到，因为它同时断言了"1 条"和"全部 2 条"两种结果。
    auto r8    = co_await rc.async_stream_range(k.x_nomk, "-", "+", 1);
    auto r8all = co_await rc.async_stream_range(k.x_nomk, "-", "+");
    bag.add_x("S08-xrange-count-keyword-and-omitted",
              is_arr(r8) && r8.size() == 1 && is_arr(r8all) && r8all.size() == 2,
              "count1=" + std::string(ttype(r8)) + "(" + std::to_string(is_arr(r8) ? r8.size() : 0) +
                  ") all=" + std::to_string(is_arr(r8all) ? r8all.size() : 0) + " want=1/2");

    // S09 XREAD 的回包第一格是**物理流名**（段配了前缀就得带着它）：
    // 这一条同时钉住两件事 —— STREAMS 之后的键名段被加了前缀（加到 COUNT/STREAMS 上这条命令根本发不出去），
    // 以及回包不做前缀剥除。
    auto rd9 = co_await rc.async_stream_read({k.x_main}, {"0-0"});
    bool ok9 = is_arr(rd9) && rd9.size() == 1 && is_arr(rd9[0]) && rd9[0].size() == 2 &&
               rd9[0][0].to_string() == sec_prefix + k.x_main && is_arr(rd9[0][1]) && rd9[0][1].size() == 3 &&
               rd9[0][1][0].size() == 2 && rd9[0][1][0][0].to_string() == id1 &&
               rd9[0][1][0][1][1].to_string() == "v";
    bag.add_x("S09-xread-physical-stream-name-in-reply", ok9, "name=" + (is_arr(rd9) && rd9.size() > 0 && is_arr(rd9[0]) ? tshow(rd9[0][0].to_string()) : std::string("<") + ttype(rd9) + ">") + " want=" + tshow(sec_prefix + k.x_main));

    // S10 '$' 不配 BLOCK 恒为 null（消费靠轮询时必须传 0-0 或上一次的 id —— 这是队列用法上最容易踩的坑）
    auto rd10 = co_await rc.async_stream_read({k.x_main}, {"$"});
    bag.add_x("S10-xread-dollar-without-block-is-nil", is_nil(rd10), "got=" + std::string(ttype(rd10)) + " want=null");

    // S11 建组 OK，重名 BUSYGROUP（组名带没带前缀在这一步看不出来，S12/S14 才看得穿）
    auto g11  = co_await rc.async_stream_group_create(k.x_main, grpname, "0-0");
    auto g11b = co_await rc.async_stream_group_create(k.x_main, grpname, "0-0");
    bag.add_x("S11-xgroup-create-ok-then-busygroup",
              is_str(g11) && g11.to_string() == "OK" && is_str(g11b) && starts_with(g11b.to_string(), "BUSYGROUP"),
              "first=" + std::string(ttype(g11)) + "(" + tshow(g11.to_string()) + ") second=" +
                  std::string(ttype(g11b)) + "(" + tshow(g11b.to_string()) + ")");

    // S12 消费组读 '>' 拿到全部 3 条（组起点是 0-0）。没有新消息时回 null，不是空数组。
    auto rg12 = co_await rc.async_stream_read_group(grpname, cname, {k.x_main}, {">"});
    bool ok12 = is_arr(rg12) && rg12.size() == 1 && is_arr(rg12[0][1]) && rg12[0][1].size() == 3 &&
                rg12[0][0].to_string() == sec_prefix + k.x_main;
    bag.add_x("S12-xreadgroup-gt-delivers-three", ok12, "got=" + std::string(ttype(rg12)) + "(" + std::to_string(is_arr(rg12) ? rg12.size() : 0) + ") want=array(1)/3 entries");

    // S13 再读一次 ⇒ null（PEL 已把这三条记在 pzt5c 名下）
    auto rg13 = co_await rc.async_stream_read_group(grpname, cname, {k.x_main}, {">"});
    bag.add_x("S13-xreadgroup-empty-is-nil-not-array", is_nil(rg13), "got=" + std::string(ttype(rg13)) + " want=null");

    // S14 XPENDING 摘要 [条数, 最小 id, 最大 id, [[消费者, "条数"]]]：
    // 每消费者那一格是 bulk **字符串**（实测 "3"），不是整数 —— 断 is_str 就是钉这一点。
    // 组名/消费者名若被加了前缀，这里会换成 NOGROUP 字符串而不是这份摘要。
    auto pd14 = co_await rc.async_stream_pending(k.x_main, grpname);
    bool ok14 = is_arr(pd14) && pd14.size() == 4 && is_num(pd14[0]) && pd14[0].to_int() == 3 &&
                pd14[1].to_string() == id1 && pd14[2].to_string() == bigid && is_arr(pd14[3]) &&
                pd14[3].size() == 1 && pd14[3][0][0].to_string() == cname && is_str(pd14[3][0][1]) &&
                pd14[3][0][1].to_string() == "3";
    bag.add_x("S14-xpending-summary-shape-and-count-is-string", ok14, "got=" + std::string(ttype(pd14)) + "([" + arr_show(pd14) + "]) want=3/" + tshow(id1) + "/" + bigid + "/[[pzt5c,\"3\"]]");

    // S15 XACK 用真在 PEL 里的 id ⇒ 1，同一条再 ack ⇒ 0，摘要跟着掉到 2
    auto ack15a = co_await rc.async_stream_ack(k.x_main, grpname, {id1});
    auto ack15b = co_await rc.async_stream_ack(k.x_main, grpname, {id1});
    auto pd15   = co_await rc.async_stream_pending(k.x_main, grpname);
    bag.add_x("S15-xack-one-then-zero-and-pending-drops",
              is_num(ack15a) && ack15a.to_int() == 1 && is_num(ack15b) && ack15b.to_int() == 0 && is_arr(pd15) &&
                  is_num(pd15[0]) && pd15[0].to_int() == 2,
              "first=" + tshow(ack15a.to_string()) + " second=" + tshow(ack15b.to_string()) + " pending=" +
                  (is_arr(pd15) ? tshow(pd15[0].to_string()) : std::string(ttype(pd15))) + " want=1/0/2");

    // S16 TYPE 报 stream（对第 6 种类型的正向断言），且对 String 键发 XLEN ⇒ WRONGTYPE
    co_await rc.async_str_set(k.x_strtype, "notastream");
    auto t16 = co_await rc.async_key_type(k.x_main);
    auto w16 = co_await rc.async_stream_len(k.x_strtype);
    bag.add_x("S16-keytype-stream-and-xlen-on-string-wrongtype",
              is_str(t16) && t16.to_string() == "stream" && is_str(w16) && starts_with(w16.to_string(), "WRONGTYPE"),
              "type=" + tshow(t16.to_string()) + " xlen_on_string=" + std::string(ttype(w16)) + "(" +
                  tshow(w16.to_string()) + ")");

    // S17 XINFO STREAM 的回包每格是扁平 map（RESP2 的 map 就是 key,value 交替），本层不配对；
    // length / groups / last-generated-id 三格都要对上现在的真实状态。
    // sub 是透传的：拼错的子命令回来的是服务端那句 ERR，不是本层拦下来的空值。
    auto xf17      = co_await rc.async_stream_info(k.x_main, "STREAM");
    auto xf17bogus = co_await rc.async_stream_info(k.x_main, "NOSUCH");
    bag.add_x("S17-xinfo-stream-flat-map",
              flat_int(xf17, "length") == 3 && flat_int(xf17, "groups") == 1 &&
                  flat_str(xf17, "last-generated-id") == bigid && is_str(xf17bogus) &&
                  starts_with(xf17bogus.to_string(), "ERR"),
              "length=" + std::to_string(flat_int(xf17, "length")) + " groups=" +
                  std::to_string(flat_int(xf17, "groups")) + " lastid=" + tshow(flat_str(xf17, "last-generated-id")) +
                  " bogus=" + std::string(ttype(xf17bogus)) + "(" + tshow(xf17bogus.to_string()) + ")" +
                  " want=3/1/" + bigid + "/ERR*");

    // S18 XINFO GROUPS / CONSUMERS 是"数组套扁平 map"，而 name 两格回的是**裸组名/裸消费者名**：
    // 配了 prefix 的段里 key 被加了前缀（加错位置就查不到这条流），组名没被加（加了就回 NOGROUP）。
    // 这一条是 XINFO 自己的运行时验证点 —— 它和 XGROUP 共用 kArgIsSecond 那一档键位，但落到 args[2] 之后
    // 谁被加前缀、谁不被加，只有这里能同时看见两面。
    auto xf18g         = co_await rc.async_stream_info(k.x_main, "GROUPS");
    auto xf18c         = co_await rc.async_stream_info(k.x_main, "CONSUMERS", {grpname});
    bool one_g         = is_arr(xf18g) && xf18g.size() == 1;
    bool one_c         = is_arr(xf18c) && xf18c.size() == 1;
    std::string g_name = one_g ? flat_str(xf18g[0], "name") : std::string();
    long long g_pend   = one_g ? flat_int(xf18g[0], "pending") : -1;
    std::string c_name = one_c ? flat_str(xf18c[0], "name") : std::string();
    long long c_pend   = one_c ? flat_int(xf18c[0], "pending") : -1;
    bag.add_x("S18-xinfo-groups-consumers-names-not-prefixed",
              g_name == grpname && g_pend == 2 && c_name == cname && c_pend == 2,
              "groups=" + std::to_string(one_g ? 1 : 0) + " gname=" + tshow(g_name) + " gpending=" +
                  std::to_string(g_pend) + " cname=" + tshow(c_name) + " cpending=" + std::to_string(c_pend) +
                  " consumers=" + std::to_string(one_c ? 1 : 0) + " want=" + grpname + "/2/" + cname + "/2");

    readings["stream"]["prefix_in_use"]      = sec_prefix;
    readings["stream"]["x_main_ids"]         = id1 + "," + id2 + "," + bigid;
    readings["stream"]["nomk_after_maxlen2"] = tshow(l6.to_string());
    readings["stream"]["xread_name"] =
        is_arr(rd9) && rd9.size() > 0 && is_arr(rd9[0]) ? rd9[0][0].to_string() : std::string("<") + ttype(rd9) + ">";
    co_return;
}

// ==================== Set：键段那一族（三个 *STORE + SINTERCARD）+ 单键族 9 条 + 集合代数 3 条 ====================
// ST01–ST11 守的是"哪几格算键"这条界：Set 的 *STORE 整串参数都是键（**没有 numkeys 那一格**），
// SINTERCARD 才有数字位，而那个数字是本方法按 keys.size() 现算的。
// 界一旦错位，两种错法症状不同：多算一位服务端响（syntax error / Number of keys…），
// 少算一位（源键名恰好是纯数字时）命令照样成功、读的是另一个物理键 ⇒ 只能比内容才看得见，那就是 ST05。
// ST12–ST23 是单键族与三条多键读命令，其中 ST22：成员位不是 key（本机 GETKEYS 实测），
// ST18：SPOP 的 count=0 必须整格不发出去——真把 0 发出去服务端不报错，回空数组且一个成员都不删。
asio::awaitable<void> run_set(pz::redis::redis_client &rc, types_bag &bag, obj_val &readings)
{
    key_table k = make_keys();

    // 这一组只比"目标集内容"和"基数"，别的组留没留键都不该影响它 ⇒ 先把自己这几个键清干净
    co_await rc.async_str_del({k.st_a, k.st_b, k.st_dst, k.st_wrong, k.st_num1, k.st_num2});

    // a={m1,m2,m3}（多塞一个重复的测去重），b={m2,m3,m4} ⇒ 并 4 / 交 2 / 差 1
    auto add1 = co_await rc.async_command({"SADD", k.st_a, "m1", "m2", "m3", "m3"});
    auto add2 = co_await rc.async_command({"SADD", k.st_b, "m2", "m3", "m4"});
    // ST01 SADD 只数真新增的条数（重复那个不计）—— 后面每条的基数地基
    bag.add_st("ST01-sadd-counts-only-new-members",
               is_num(add1) && add1.to_int() == 3 && is_num(add2) && add2.to_int() == 3,
               "a=" + std::string(ttype(add1)) + "(" + tshow(add1.to_string()) + ") b=" +
                   std::string(ttype(add2)) + "(" + tshow(add2.to_string()) + ") want=3/3");

    // ST02 SUNIONSTORE：回包是写完之后的目标集基数，内容按集合比（服务端不保序）
    auto u1  = co_await rc.async_set_union_store(k.st_dst, {k.st_a, k.st_b});
    auto u1m = co_await rc.async_command({"SMEMBERS", k.st_dst});
    bag.add_st("ST02-union-store-card-and-members",
               is_num(u1) && u1.to_int() == 4 && set_show(u1m) == "m1,m2,m3,m4",
               "card=" + std::string(ttype(u1)) + "(" + tshow(u1.to_string()) + ") members=[" +
                   set_show(u1m) + "] want=[m1,m2,m3,m4]");

    // ST03 SINTERSTORE：交 = {m2,m3}
    co_await rc.async_str_del({k.st_dst});
    auto i1  = co_await rc.async_set_intersect_store(k.st_dst, {k.st_a, k.st_b});
    auto i1m = co_await rc.async_command({"SMEMBERS", k.st_dst});
    bag.add_st("ST03-intersect-store-card-and-members",
               is_num(i1) && i1.to_int() == 2 && set_show(i1m) == "m2,m3",
               "card=" + std::string(ttype(i1)) + "(" + tshow(i1.to_string()) + ") members=[" +
                   set_show(i1m) + "] want=[m2,m3]");

    // ST04 SDIFFSTORE：a 减 b = {m1}。源键顺序有意义 ⇒ 这一条同时证明参数没被重排
    co_await rc.async_str_del({k.st_dst});
    auto d1  = co_await rc.async_set_difference_store(k.st_dst, {k.st_a, k.st_b});
    auto d1m = co_await rc.async_command({"SMEMBERS", k.st_dst});
    bag.add_st("ST04-difference-store-first-key-wins",
               is_num(d1) && d1.to_int() == 1 && set_show(d1m) == "m1",
               "card=" + std::string(ttype(d1)) + "(" + tshow(d1.to_string()) + ") members=[" +
                   set_show(d1m) + "] want=[m1]");

    // ST05 源键名叫 "1" / "2"：Set 的 *STORE 没有 numkeys 参数位，纯数字键名是**合法源键**。
    // 谁要是照 Zset 的形状把它当数字解析，这个键位自己就不加前缀了 ⇒ 少一位、命令仍成功、
    // 读到的是另一个物理键（在没配 prefix 的段上两者同形，所以这条只有比对内容才查得出来）。
    co_await rc.async_command({"SADD", k.st_num1, "u1a"});
    co_await rc.async_command({"SADD", k.st_num2, "u2a"});
    co_await rc.async_str_del({k.st_dst});
    auto n1  = co_await rc.async_set_union_store(k.st_dst, {k.st_num1, k.st_num2});
    auto n1m = co_await rc.async_command({"SMEMBERS", k.st_dst});
    bag.add_st("ST05-numeric-named-source-keys-both-used",
               is_num(n1) && n1.to_int() == 2 && set_show(n1m) == "u1a,u2a",
               "card=" + std::string(ttype(n1)) + "(" + tshow(n1.to_string()) + ") members=[" +
                   set_show(n1m) + "] want=[u1a,u2a]");

    // ST06 SINTERCARD：数字位由 keys.size() 算 ⇒ 交 = 2，整数口径（不是 ERR 字符串）
    auto c6 = co_await rc.async_set_intersect_card({k.st_a, k.st_b});
    bag.add_st("ST06-sintercard-integer-card", is_num(c6) && c6.to_int() == 2, "got=" + std::string(ttype(c6)) + "(" + tshow(c6.to_string()) + ") want=2");

    // ST07 LIMIT 落在键段**之外**：不带 LIMIT 是 2，带就是 1。
    // 数字位多算一位会把 "LIMIT" 吃进键段 ⇒ 服务端回 syntax error（实测），这条当场失败；
    // 而"发了 LIMIT 却不生效"也会失败（2 != 1）⇒ 两个方向都查得出来。
    auto c7 = co_await rc.async_set_intersect_card({k.st_a, k.st_b}, {"LIMIT", "1"});
    bag.add_st("ST07-sintercard-limit-outside-key-block-takes-effect",
               is_num(c7) && c7.to_int() == 1,
               "got=" + std::string(ttype(c7)) + "(" + tshow(c7.to_string()) + ") want=1");

    // ST08 缺键当空集 ⇒ 0，不报错（0 是服务端真会产生的数，所以这些方法一个都不走 int 收口）
    auto c8 = co_await rc.async_set_intersect_card({k.st_a, k.st_missing});
    bag.add_st("ST08-sintercard-missing-key-is-empty-zero", is_num(c8) && c8.to_int() == 0, "got=" + std::string(ttype(c8)) + "(" + tshow(c8.to_string()) + ") want=0");

    // ST09 空 keys ⇒ 发出去的是 SINTERCARD 0，服务端回参数错字符串（实测文本），本层不挡；
    // 且这条用例不该惊动"宁可一格不加"那个计数器（它走的是键位表的正常分支）
    unsigned long long skip_before = pz::redis::redis_conn_base::counters().prefix_skip.load();
    auto c9                        = co_await rc.async_set_intersect_card({});
    unsigned long long skip_after  = pz::redis::redis_conn_base::counters().prefix_skip.load();
    bag.add_st("ST09-sintercard-empty-keys-is-server-error-string",
               is_str(c9) && starts_with(c9.to_string(), "ERR") && skip_after == skip_before,
               "reply=" + std::string(ttype(c9)) + "(" + tshow(c9.to_string()) + ") want=ERR* prefix_skip+=" +
                   std::to_string(skip_after - skip_before));

    // ST10 WRONGTYPE 是正向断言：打 String 键 ⇒ WRONGTYPE 文本，且该键确实是 string（不是"没送出去"）
    co_await rc.async_str_set(k.st_wrong, "plain");
    auto c10 = co_await rc.async_set_intersect_card({k.st_wrong, k.st_a});
    auto t10 = co_await rc.async_key_type(k.st_wrong);
    bag.add_st("ST10-sintercard-wrongtype-on-string-key",
               is_str(c10) && starts_with(c10.to_string(), "WRONGTYPE") && t10.to_string() == "string",
               "reply=" + std::string(ttype(c10)) + "(" + tshow(c10.to_string()) + ") type=" +
                   tshow(t10.to_string()));

    // ST11 store 覆盖既有任意类型键（实测：写完 TYPE 变 set，GET 它回 WRONGTYPE）
    auto u11 = co_await rc.async_set_union_store(k.st_wrong, {k.st_a});
    auto t11 = co_await rc.async_key_type(k.st_wrong);
    bag.add_st("ST11-store-overwrites-key-of-other-type",
               is_num(u11) && u11.to_int() == 3 && t11.to_string() == "set",
               "card=" + std::string(ttype(u11)) + "(" + tshow(u11.to_string()) + ") type=" +
                   tshow(t11.to_string()));

    readings["set"]["union_members"]   = set_show(u1m);
    readings["set"]["numeric_members"] = set_show(n1m);
    readings["set"]["card_no_limit"]   = tshow(c6.to_string());
    readings["set"]["card_limit1"]     = tshow(c7.to_string());
    readings["set"]["empty_keys_reply"] =
        std::string(ttype(c9)) + "(" + tshow(c9.to_string()) + ")";

    // ==================== 单键族 9 条 + 集合代数 3 条 ====================
    // 上面的键段用例已把 st_* 改写（st_dst 是并集结果、st_wrong 变成了 set），所以这一族先重建自己的 fixture。
    co_await rc.async_str_del({k.st_a, k.st_b, k.st_dst});

    // ST12 set_add 只数真新增的条数（重复成员与已存在成员都不计数），card/members 各自对上
    auto a12a = co_await rc.async_set_add(k.st_a, {"m1", "m2", "m3", "m3"});
    auto a12b = co_await rc.async_set_add(k.st_a, {"m1"});
    auto a12c = co_await rc.async_set_card(k.st_a);
    auto a12d = co_await rc.async_set_members(k.st_a);
    bag.add_st("ST12-set_add_counts_only_new_and_card_members_agree",
               is_num(a12a) && a12a.to_int() == 3 && is_num(a12b) && a12b.to_int() == 0 &&
                   is_num(a12c) && a12c.to_int() == 3 && set_show(a12d) == "m1,m2,m3",
               "add=" + tshow(a12a.to_string()) + " add_again=" + tshow(a12b.to_string()) + " card=" +
                   tshow(a12c.to_string()) + " members=[" + set_show(a12d) + "] want=3/0/3/[m1,m2,m3]");

    // ST13 set_is_member：命中 1、未命中 0、缺失键 0（三个都是整数，不是错误字符串）
    auto i13a = co_await rc.async_set_is_member(k.st_a, "m1");
    auto i13b = co_await rc.async_set_is_member(k.st_a, "nope");
    auto i13c = co_await rc.async_set_is_member(k.st_missing, "m1");
    bag.add_st("ST13-set_is_member_one_zero_zero",
               is_num(i13a) && i13a.to_int() == 1 && is_num(i13b) && i13b.to_int() == 0 &&
                   is_num(i13c) && i13c.to_int() == 0,
               "hit=" + std::string(ttype(i13a)) + "(" + tshow(i13a.to_string()) + ") miss=" +
                   tshow(i13b.to_string()) + " missing_key=" + tshow(i13c.to_string()) + " want=1/0/0");

    // ST14 set_are_members：逐个 0/1 且顺序就是传入顺序；缺失键回 [0]（按成员数），不是 null
    auto i14a = co_await rc.async_set_are_members(k.st_a, {"m1", "nope", "m3"});
    auto i14b = co_await rc.async_set_are_members(k.st_missing, {"m1"});
    bag.add_st("ST14-set_are_members_ordered_int_array",
               is_arr(i14a) && i14a.size() == 3 && i14a[0].to_int() == 1 && i14a[1].to_int() == 0 &&
                   i14a[2].to_int() == 1 && is_arr(i14b) && i14b.size() == 1 && i14b[0].to_int() == 0,
               "got=[" + arr_show(i14a) + "] missing=[" + arr_show(i14b) + "] want=[1,0,1]/[0]");

    // ST15 set_remove 只数真删掉的条数（"nope" 不在集合里 ⇒ 回 1），剩余成员读回
    auto r15a = co_await rc.async_set_remove(k.st_a, {"m1", "nope"});
    auto r15b = co_await rc.async_set_card(k.st_a);
    auto r15c = co_await rc.async_set_members(k.st_a);
    bag.add_st("ST15-set_remove_counts_only_real_removals",
               is_num(r15a) && r15a.to_int() == 1 && is_num(r15b) && r15b.to_int() == 2 &&
                   set_show(r15c) == "m2,m3",
               "srem=" + tshow(r15a.to_string()) + " card=" + tshow(r15b.to_string()) + " members=[" +
                   set_show(r15c) + "] want=1/2/[m2,m3]");

    // ST16 set_random_member：不带 count 是 bulk、带 count 是数组，两种发法都不删成员
    // （这条同时验证 "count 参数省略/发出"的形状差 —— 真发 SRANDMEMBER k 0 回的是空数组）
    auto p16a    = co_await rc.async_set_random_member(k.st_a);
    auto p16b    = co_await rc.async_set_random_member(k.st_a, 2);
    auto p16c    = co_await rc.async_set_card(k.st_a);
    bool p16a_in = is_str(p16a) && (p16a.to_string() == "m2" || p16a.to_string() == "m3");
    bool p16b_in = is_arr(p16b) && p16b.size() == 2 &&
                   set_show(p16b).find("m2") != std::string::npos &&
                   set_show(p16b).find("m3") != std::string::npos;
    bag.add_st("ST16-set_random_member_bulk_vs_array_and_not_consuming",
               p16a_in && p16b_in && is_num(p16c) && p16c.to_int() == 2,
               "one=" + std::string(ttype(p16a)) + "(" + tshow(p16a.to_string()) + ") two=" +
                   std::string(ttype(p16b)) + "([" + set_show(p16b) + "]) card=" +
                   tshow(p16c.to_string()) + " want=bulk/array/2");

    // ST17 set_pop：无 count 回 bulk、带 count 回数组，两次弹出把三条全带走（弹空即删键）
    co_await rc.async_set_add(k.st_dst, {"p1", "p2", "p3"});
    auto q17a = co_await rc.async_set_pop(k.st_dst);
    auto q17b = co_await rc.async_set_pop(k.st_dst, 2);
    auto q17c = co_await rc.async_set_card(k.st_dst);
    std::vector<std::string> q17all;
    if (is_str(q17a))
        q17all.push_back(q17a.to_string());
    if (is_arr(q17b))
        for (unsigned int i = 0; i < q17b.size(); i++)
            q17all.push_back(q17b[i].to_string());
    std::sort(q17all.begin(), q17all.end());
    std::string q17joined;
    for (std::size_t i = 0; i < q17all.size(); i++)
        q17joined += (i ? "," : "") + q17all[i];
    bag.add_st("ST17-set_pop_bulk_then_array_consumes_all",
               is_str(q17a) && is_arr(q17b) && q17b.size() == 2 && q17joined == "p1,p2,p3" &&
                   is_num(q17c) && q17c.to_int() == 0,
               "one=" + std::string(ttype(q17a)) + "(" + tshow(q17a.to_string()) + ") two=" +
                   std::string(ttype(q17b)) + "([" + set_show(q17b) + "]) popped=[" + tshow(q17joined) +
                   "] card=" + tshow(q17c.to_string()) + " want=[p1,p2,p3]/0");

    // ST18 count=0 必须整格不发出去：真把 0 发出去服务端不报错，回空数组且一个成员都不删（实测静默 no-op）
    // ⇒ 这条断言的是两件事：形状（bulk 而非数组）**加上**"确实弹掉了"（键随之消失）
    co_await rc.async_set_add(k.st_dst, {"z1"});
    auto q18a = co_await rc.async_set_pop(k.st_dst, 0);
    auto q18b = co_await rc.async_set_card(k.st_dst);
    bag.add_st("ST18-set_pop_count_zero_is_omitted_not_sent",
               is_str(q18a) && q18a.to_string() == "z1" && is_num(q18b) && q18b.to_int() == 0,
               "got=" + std::string(ttype(q18a)) + "(" + tshow(q18a.to_string()) + ") card=" +
                   tshow(q18b.to_string()) + " want=string(z1)/0");

    // ST19 set_move：搬成回 1、成员不在回 0，两边内容各自对上
    // 两条键全部自建，**不依赖前面用例留下的状态**：实测教训是"src_card==0"这条如果依赖
    // ST17/ST18 把 st_dst 弹干净，那么坏掉的是 SPOP 却被 SMOVE 报告 —— 失败会指到错的地方。
    // st_b 同理（族开头的整体删除之后再没写过它）。ST20 接着用这里的 st_b，那是这条用例自己产出的状态。
    co_await rc.async_str_del({k.st_b, k.st_dst});
    co_await rc.async_set_add(k.st_b, {"m2", "m3", "m4"});
    co_await rc.async_set_add(k.st_dst, {"mv"});
    auto v19a = co_await rc.async_set_move(k.st_dst, k.st_b, "mv");
    auto v19b = co_await rc.async_set_move(k.st_dst, k.st_b, "nope");
    auto v19c = co_await rc.async_set_card(k.st_dst);
    auto v19d = co_await rc.async_set_is_member(k.st_b, "mv");
    auto v19e = co_await rc.async_set_members(k.st_b);
    bag.add_st("ST19-set_move_members_and_counts",
               is_num(v19a) && v19a.to_int() == 1 && is_num(v19b) && v19b.to_int() == 0 &&
                   is_num(v19c) && v19c.to_int() == 0 && is_num(v19d) && v19d.to_int() == 1 &&
                   set_show(v19e) == "m2,m3,m4,mv",
               "move=" + tshow(v19a.to_string()) + " miss=" + tshow(v19b.to_string()) + " src_card=" +
                   tshow(v19c.to_string()) + " in_dst=" + tshow(v19d.to_string()) + " dst=[" +
                   set_show(v19e) + "] want=1/0/0/1/[m2,m3,m4,mv]");

    // ST20 src==dst：实测回 **1 且成员还在**（不是老版本的参数错字符串）⇒ 必须是整数 1、基数不变
    auto v20a = co_await rc.async_set_move(k.st_b, k.st_b, "mv");
    auto v20b = co_await rc.async_set_card(k.st_b);
    bag.add_st("ST20-set_move_same_key_is_noop_returns_one",
               is_num(v20a) && v20a.to_int() == 1 && is_num(v20b) && v20b.to_int() == 4,
               "reply=" + std::string(ttype(v20a)) + "(" + tshow(v20a.to_string()) + ") card=" +
                   tshow(v20b.to_string()) + " want=1/4");

    // ST21 三条集合代数读命令：内容按集合比，缺失键当空集（并集不受影响、交集直接空）
    co_await rc.async_str_del({k.st_a, k.st_b});
    co_await rc.async_set_add(k.st_a, {"u1", "u2", "u3"});
    co_await rc.async_set_add(k.st_b, {"u2", "u3", "u4"});
    auto w21a = co_await rc.async_set_union({k.st_a, k.st_b});
    auto w21b = co_await rc.async_set_intersection({k.st_a, k.st_b});
    auto w21c = co_await rc.async_set_difference({k.st_a, k.st_b});
    auto w21d = co_await rc.async_set_intersection({k.st_a, k.st_missing});
    auto w21e = co_await rc.async_set_difference({k.st_missing, k.st_a});
    bag.add_st("ST21-union_intersection_difference_contents",
               set_show(w21a) == "u1,u2,u3,u4" && set_show(w21b) == "u2,u3" &&
                   set_show(w21c) == "u1" && is_arr(w21d) && w21d.size() == 0 &&
                   is_arr(w21e) && w21e.size() == 0,
               "union=[" + set_show(w21a) + "] inter=[" + set_show(w21b) + "] diff=[" +
                   set_show(w21c) + "] inter-missing=" + std::to_string(is_arr(w21d) ? w21d.size() : 0) +
                   " diff-missing-first=" + std::to_string(is_arr(w21e) ? w21e.size() : 0));

    // ST22 成员位长得像 key：成员就叫 "1" / "2"。GETKEYS 实测这些位**不是键**，
    // 谁把 SADD 归进 kAll，带 prefix 的段上这两个成员会被写成 pzpre:1/pzpre:2 ⇒ 这条当场失败
    // （要复核物理键名，可以换一条不带前缀的连接从服务端外面读同一把键）
    co_await rc.async_str_del({k.st_dst});
    auto x22a = co_await rc.async_set_add(k.st_dst, {"1", "2"});
    auto x22b = co_await rc.async_set_members(k.st_dst);
    auto x22c = co_await rc.async_set_is_member(k.st_dst, "1");
    bag.add_st("ST22-numeric_looking_members_stay_members",
               is_num(x22a) && x22a.to_int() == 2 && set_show(x22b) == "1,2" &&
                   is_num(x22c) && x22c.to_int() == 1,
               "add=" + tshow(x22a.to_string()) + " members=[" + set_show(x22b) + "] is_member_1=" +
                   tshow(x22c.to_string()) + " want=2/[1,2]/1");

    // ST23 WRONGTYPE 是正向断言：单键族打 String 键回 WRONGTYPE 文本，且键类型没被改写
    co_await rc.async_str_set(k.s_main, "plain");
    auto y23a = co_await rc.async_set_add(k.s_main, {"boom"});
    auto y23b = co_await rc.async_set_members(k.s_main);
    auto y23c = co_await rc.async_key_type(k.s_main);
    bag.add_st("ST23-wrongtype_on_string_key_for_single_key_ops",
               is_str(y23a) && starts_with(y23a.to_string(), "WRONGTYPE") &&
                   is_str(y23b) && starts_with(y23b.to_string(), "WRONGTYPE") &&
                   y23c.to_string() == "string",
               "sadd=" + std::string(ttype(y23a)) + "(" + tshow(y23a.to_string()) + ") smembers=" +
                   std::string(ttype(y23b)) + "(" + tshow(y23b.to_string()) + ") type=" +
                   tshow(y23c.to_string()));

    readings["set"]["members_after_srem"]          = set_show(r15c);
    readings["set"]["popped_all"]                  = tshow(q17joined);
    readings["set"]["numeric_members_are_members"] = set_show(x22b);
    readings["set"]["same_key_move"]               = std::string(ttype(v20a)) + "(" + tshow(v20a.to_string()) + ")";
    co_return;
}

// ==================== 键级：TTL / EXPIRE / PERSIST / EXISTS + 单命令锁 ====================
asio::awaitable<void> run_key(pz::redis::redis_client &rc, types_bag &bag, obj_val &readings)
{
    key_table k = make_keys();

    // K01 EXISTS 命中的键 ⇒ true（先建一个，别依赖上一组留下的键）
    co_await rc.async_str_set(k.s_noxp, "nope");
    bool e1 = co_await rc.async_key_exists(k.s_noxp);
    bag.add_k("K01-exists-on-present-key", e1, "exists=" + std::string(e1 ? "1" : "0"));

    // K02 EXISTS 不存在的键 ⇒ false
    bool e2 = co_await rc.async_key_exists(k.s_missing);
    bag.add_k("K02-exists-on-missing-key", !e2, "exists=" + std::string(e2 ? "1" : "0"));

    // K03 TTL 不存在的键 ⇒ -2（区别于"命令没送出去"的 0）
    long long t3 = co_await rc.async_key_ttl(k.s_missing);
    bag.add_k("K03-ttl-missing-key-is-minus2", t3 == -2, "ttl=" + std::to_string(t3) + " want=-2");

    // K04 TTL 存在但没期限 ⇒ -1
    long long t4 = co_await rc.async_key_ttl(k.s_noxp);
    bag.add_k("K04-ttl-no-expire-is-minus1", t4 == -1, "ttl=" + std::to_string(t4) + " want=-1");

    // K05 EXPIRE 成功 ⇒ true，且 TTL 真的落进区间
    bool f5      = co_await rc.async_key_expire(k.s_noxp, 30);
    long long t5 = co_await rc.async_key_ttl(k.s_noxp);
    bag.add_k("K05-expire-then-ttl-in-range", f5 && t5 > 0 && t5 <= 30, "expire=" + std::string(f5 ? "1" : "0") + " ttl=" + std::to_string(t5) + " want=1..30");

    // K06 EXPIRE 不存在的键 ⇒ false
    bool f6 = co_await rc.async_key_expire(k.s_missing, 30);
    bag.add_k("K06-expire-on-missing-key-false", !f6, "expire=" + std::string(f6 ? "1" : "0"));

    // K07 PERSIST 去掉期限 ⇒ true，且 TTL 回到 -1
    bool g7      = co_await rc.async_key_persist(k.s_noxp);
    long long t7 = co_await rc.async_key_ttl(k.s_noxp);
    bag.add_k("K07-persist-removes-expire", g7 && t7 == -1, "persist=" + std::string(g7 ? "1" : "0") + " ttl=" + std::to_string(t7) + " want=-1");

    // K08 PERSIST 本来就没期限 ⇒ false
    bool g8 = co_await rc.async_key_persist(k.s_noxp);
    bag.add_k("K08-persist-without-expire-false", !g8, "persist=" + std::string(g8 ? "1" : "0"));

    // K09 锁：SET NX EX 一条命令搞定，抢输的一方不回 OK 也不改写值，
    // 且赢的那一方刚建成就带期限 —— 这正是"先 SETNX 再 EXPIRE"两步写法拿不到的性质。
    auto l9a      = co_await rc.async_str_set(k.s_lock, "ownerA", {"NX", "EX", "30"});
    auto l9b      = co_await rc.async_str_set(k.s_lock, "ownerB", {"NX"});
    auto l9v      = co_await rc.async_str_get(k.s_lock);
    long long l9t = co_await rc.async_key_ttl(k.s_lock);
    bag.add_k("K09-set-nx-ex-single-command-lock",
              is_str(l9a) && l9a.to_string() == "OK" && is_nil(l9b) && is_str(l9v) && l9v.to_string() == "ownerA" &&
                  l9t > 0 && l9t <= 30,
              "first=" + std::string(ttype(l9a)) + "(" + tshow(l9a.to_string()) + ") second=" +
                  std::string(ttype(l9b)) + " holder=" + tshow(l9v.to_string()) + " ttl=" + std::to_string(l9t));

    // K10 解锁后同一个键能被再抢（NX 位没被粘住），且期限跟着新主人走
    auto d10      = co_await rc.async_str_del({k.s_lock});
    auto l10      = co_await rc.async_str_set(k.s_lock, "ownerC", {"NX", "EX", "20"});
    long long t10 = co_await rc.async_key_ttl(k.s_lock);
    bag.add_k("K10-unlock-then-relatch", is_num(d10) && d10.to_int() == 1 && is_str(l10) && l10.to_string() == "OK" && t10 > 0 && t10 <= 20, "del=" + tshow(d10.to_string()) + " relatch=" + tshow(l10.to_string()) + " ttl=" + std::to_string(t10));

    readings["key"]["lock_ttl_after_relatch"] = std::to_string(t10);
    co_return;
}

// ==================== 页级检查（跑完两组之后收尾） ====================
asio::awaitable<void> run_page(pz::redis::redis_client &rc, types_bag &bag, obj_val &readings, const pz::redis::conn_counters &before, bool did_string, bool did_hash, bool did_list, bool did_stream, bool did_set, bool did_key)
{
    key_table k                   = make_keys();
    std::vector<std::string> keys = k.all();

    // P01 池已加载
    bag.add_p("P01-pool-loaded", pz::redis::get_redis_pool().is_loaded(), "is_loaded=" + std::string(pz::redis::get_redis_pool().is_loaded() ? "1" : "0"));

    // P02 键名表非空且无重复（漂了会在这里露出来，而不是在清理那一步静默留下键）
    std::vector<std::string> sorted = keys;
    std::sort(sorted.begin(), sorted.end());
    bool uniq = true;
    for (std::size_t i = 1; i < sorted.size(); i++)
        if (sorted[i] == sorted[i - 1])
            uniq = false;
    bag.add_p("P02-key-table-unique", uniq && keys.size() == kKeyTableSize, "total=" + std::to_string(keys.size()) + " want=" + std::to_string(kKeyTableSize) + " unique=" + (uniq ? "1" : "0"));

    // P03 清完必须一个不剩（数的是服务端实际状态，不是本页的记账）
    co_await cleanup_keys(rc, keys);
    unsigned int leftover = co_await count_existing(rc, keys);
    bag.add_p("P03-no-residual-after-cleanup", leftover == 0, "leftover=" + std::to_string(leftover) + " want=0");

    // P04 一次干净跑不该触发任何错误支路（超时/脏连接/前缀跳过/组包被拒/丢连接）
    const pz::redis::conn_counters &now = pz::redis::redis_conn_base::counters();
    unsigned long long d_timeout        = now.timeout.load() - before.timeout.load();
    unsigned long long d_poisoned       = now.poisoned.load() - before.poisoned.load();
    unsigned long long d_prefix         = now.prefix_skip.load() - before.prefix_skip.load();
    unsigned long long d_reject         = now.arg_reject.load() - before.arg_reject.load();
    unsigned long long d_drop           = now.stale_drop.load() - before.stale_drop.load();
    bag.add_p("P04-clean-run-trips-no-error-branch",
              d_timeout == 0 && d_poisoned == 0 && d_prefix == 0 && d_reject == 0 && d_drop == 0,
              "timeout=" + std::to_string(d_timeout) + " poisoned=" + std::to_string(d_poisoned) +
                  " prefix_skip=" + std::to_string(d_prefix) + " arg_reject=" + std::to_string(d_reject) +
                  " stale_drop=" + std::to_string(d_drop));

    // P05 段名不存在 与 键不存在 在这条通路上同形（如实记下这个事实，不是当它不存在）
    pz::redis::redis_client badrc(rc.section() + "_nosuch_section");
    auto miss_sec = co_await badrc.async_str_get(k.s_main);
    auto miss_key = co_await rc.async_str_get(k.s_missing);
    bag.add_p("P05-bad-section-conflated-with-missing-key", is_nil(miss_sec) && is_nil(miss_key), "bad_section=" + std::string(ttype(miss_sec)) + " missing_key=" + std::string(ttype(miss_key)) + " note=use_async_exec_to_tell_them_apart");

    // P06 六组各自被跑到了（新增支路必须自带触发计数，否则"整组被跳过"也算全部通过）
    unsigned int want_s  = did_string ? kStringChecks : 0;
    unsigned int want_h  = did_hash ? kHashChecks : 0;
    unsigned int want_l  = did_list ? kListChecks : 0;
    unsigned int want_x  = did_stream ? kStreamChecks : 0;
    unsigned int want_st = did_set ? kSetChecks : 0;
    unsigned int want_k  = did_key ? kKeyChecks : 0;
    bag.add_p("P06-group-arm-counts",
              bag.n_string == want_s && bag.n_hash == want_h && bag.n_list == want_l &&
                  bag.n_stream == want_x && bag.n_set == want_st && bag.n_key == want_k,
              "string=" + std::to_string(bag.n_string) + "/" + std::to_string(want_s) +
                  " hash=" + std::to_string(bag.n_hash) + "/" + std::to_string(want_h) +
                  " list=" + std::to_string(bag.n_list) + "/" + std::to_string(want_l) +
                  " stream=" + std::to_string(bag.n_stream) + "/" + std::to_string(want_x) +
                  " set=" + std::to_string(bag.n_set) + "/" + std::to_string(want_st) +
                  " key=" + std::to_string(bag.n_key) + "/" + std::to_string(want_k));

    readings["leftover_after_cleanup"] = static_cast<long long>(leftover);
    co_return;
}

// 池侧统计：created / over_limit_close 是本次请求期间的增量，busy / pending 是当下瞬时值，
// peak_* 是进程启动以来的高水位。这四个数原先在进程里完全看不见，只能靠服务端的
// total_connections_received 反推（"超过 maxpool 的借用每条都在重连"这件事就是这么发现的）。
// 那两个 *_abs 是给外部核对用的：并发段的每个请求都截自己那段窗口，窗口互相重叠，
// 把 per-request 增量加起来必然重复计数（实测一把 40 并发把 117 条新连接加成 3707），
// 只有绝对值跨请求取 max 再相减才是这段爆发真实建了多少条。
void out_pool_counters(obj_val &delta, const pz::redis::pool_counters &p, unsigned long long created_base, unsigned long long close_base)
{
    delta["created"]              = static_cast<long long>(p.created.load() - created_base);
    delta["over_limit_close"]     = static_cast<long long>(p.over_limit_close.load() - close_base);
    delta["created_abs"]          = static_cast<long long>(p.created.load());
    delta["over_limit_close_abs"] = static_cast<long long>(p.over_limit_close.load());
    delta["busy"]                 = static_cast<long long>(p.busy.load());
    delta["peak_busy"]            = static_cast<long long>(p.peak_busy.load());
    delta["pending"]              = static_cast<long long>(p.pending.load());
    delta["peak_pending"]         = static_cast<long long>(p.peak_pending.load());
    delta["workers"]              = static_cast<long long>(pz::redis::get_redis_pool().worker_count());
}
}// namespace
#endif// ENABLE_REDIS

// ==================== redis/types：常用类型用法演示（String + Hash + 键级/锁） ====================
//@urlpath(null,redis/types)
asio::awaitable<std::string> test_redis_types(std::shared_ptr<httppeer> peer)
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

    std::string grp = client.get["grp"].to_string();
    std::string sec = client.get["sec"].to_string();
    if (sec.empty())
        sec = "default";
    pz::redis::redis_client rc(sec);

    std::vector<std::string> keys = make_keys().all();

    // ?ov= 是构造参数传入的键前缀：不带它时是空串 ⇒ 回落到段配置，物理名与不加它时逐字节同形。
    // leave / cleanup 都吃同一个 ov —— 写和删必须走同一个对象，否则删的是另一个物理名，
    // 会在服务器上留下一批 *pctor:* 的残留。
    std::string ov = client.get["ov"].to_string();

    if (grp == "cleanup")
    {
        // 只做清理 + 报残留，不跑断言（其它用例跑完调它复原环境）
        pz::redis::redis_client rcv(sec, ov);
        co_await cleanup_keys(rcv, keys);
        unsigned int leftover  = co_await count_existing(rcv, keys);
        client.val["mode"]     = std::string("cleanup");
        client.val["keys"]     = static_cast<long long>(keys.size());
        client.val["leftover"] = static_cast<long long>(leftover);
        client.val["override"] = ov;
        client.out_json();
        co_return "";
    }

    if (grp == "leave")
    {
        // 只写四把键、**不删**，供从服务端一侧数物理键名 —— 页内的一切都是对称的
        // （写和读走同一条连接、同一个段），前缀加不加都查不出来，所以键名前缀的正向验证只能从服务端取。
        // 用的是键名表里已有的四把，?grp=cleanup 一定收得干净。
        pz::redis::redis_client rcv(sec, ov);
        key_table k = make_keys();
        co_await rcv.async_str_set(k.s_main, "leaveval");
        co_await rcv.async_list_push_left(k.l_main, {"e1"});
        co_await rcv.async_stream_add(k.x_main, {"lv", "1"});
        // 第四把是 Set 的 *STORE 写出来的目标键，源键里有一把的名字就是 "1"。
        // 选这一种形状是因为它是"整串参数都是键"（Set 的 *STORE 没有 numkeys 参数位）唯一能从外面验证的形态：
        // 界一旦错位，要么目标键根本没建成（少了一个物理名），要么并进来的是另一个段的键（成员集不一样），
        // 而这两种在页内都读成"命令成功、值也对"—— 只有 redis-cli 换一条不带前缀的连接才看得见。
        co_await rcv.async_command({"SADD", k.st_a, "lv1", "lv2", "1", "2"});
        co_await rcv.async_command({"SADD", k.st_num1, "lvn"});
        co_await rcv.async_set_union_store(k.st_dst, {k.st_a, k.st_num1});
        // 五把带 pzt5: 名字的键（st_a 是并集的源键，它自己也在物理键名里数得到）+ 一把名字叫 "1" 的源键，
        // 从服务端外侧按这两形计数，应当各是 5 与 1。
        // st_a 里那两个成员的名字**故意也叫 "1" / "2"**：成员位在 GETKEYS 实测里不是 key，
        // 所以从外面（不带前缀的连接）读到的成员必须还是裸 "1"/"2"。谁把 SADD 的键位改宽，
        // 这里就会读成 pzpre:1/pzpre:2 —— 和页内 ST22 断言的是同一件事，只是从服务端外面看。
        unsigned int written   = co_await count_existing(rcv, {k.s_main, k.l_main, k.x_main, k.st_dst, k.st_a});
        client.val["mode"]     = std::string("leave");
        client.val["section"]  = sec;
        client.val["override"] = ov;
        client.val["written"]  = static_cast<long long>(written);
        // 源键名 "1" 自己也是一把键（Set 的 *STORE 里它落在 key 位），单独报一个字段
        unsigned int numkey_left     = co_await count_existing(rcv, {k.st_num1});
        client.val["numkey_written"] = static_cast<long long>(numkey_left);
        client.out_json();
        co_return "";
    }

    if (grp == "skiparm")
    {
        // 故意发一格里 STREAMS 之后是**奇数**格（两个键名一个 id），用来证"宁可一格不加 + 计
        // prefix_skip"这一支路真的可达：正常命令全是偶数格，把奇数判定删掉整套页内断言照样全部通过
        // （tail/2 整除之后同样是一格不加），所以这一支唯一可见的信号就是那个计数器。
        // 服务端自己报 ERR Unbalanced 'xread'...（实测文本），且读命令不建键 ⇒ 不留残留。
        // 只在配了 prefix 的段上才有意义：空前缀在 apply_prefix 第一行就返回，计数器不会动。
        unsigned long long skip_before = pz::redis::redis_conn_base::counters().prefix_skip.load();
        auto bad                       = co_await rc.async_command(
            {"XREAD", "STREAMS", "pzt5:x:skip1", "pzt5:x:skip2", "0-0"});
        unsigned long long skip_after = pz::redis::redis_conn_base::counters().prefix_skip.load();
        unsigned long long d          = skip_after - skip_before;
        bool err_text                 = is_str(bad) && starts_with(bad.to_string(), "ERR Unbalanced");
        client.val["mode"]            = std::string("skiparm");
        client.val["section"]         = sec;
        client.val["skip_delta"]      = static_cast<long long>(d);
        client.val["reply"]           = std::string(ttype(bad)) + "(" + tshow(bad.to_string()) + ")";
        client.val["total"]           = static_cast<long long>(1);
        client.val["pass"]            = static_cast<long long>((d == 1 && err_text) ? 1 : 0);
        client.val["all_pass"]        = (d == 1 && err_text) ? 1 : 0;
        client.out_json();
        co_return "";
    }

    // ==================== 构造参数带前缀 ====================
    // 这一组自己早退，是因为它故意发一条畸形命令去拨 prefix_skip：主流程的页级检查 P04
    // 断的是"干净跑时这些错误计数增量为 0"，两件事写进同一页会互相造成失败（?grp=skiparm 同样单独早退）。
    // pc 带覆盖、rc 是同段不带覆盖的对象，两者的物理键名必须不同名 —— 这就是"覆盖替换段配置、
    // 不是接在它后面"的页内验证；物理名到底长什么样，要换一条不带前缀的连接从服务端外面看。
    if (grp == "pfxctor")
    {
        const std::string pfx = "pctor:";
        pz::redis::redis_client pc(sec, pfx);
        pz::redis::redis_client fb(sec, "");// 显式空覆盖 = 该回落到段配置
        const std::string k01    = "pzt5:pc:01";
        const std::string k03    = "pzt5:pc:03";
        const std::string k04a   = "pzt5:pc:04a";
        const std::string k04b   = "pzt5:pc:04b";
        const std::string k05dst = "pzt5:pc:05dst";
        const std::string k05a   = "pzt5:pc:05a";
        // 源键的名字就是一个数字：键位界错位时它会被当成 numkeys 参数那一格吃掉
        const std::string k05n = "1";
        const std::string k06  = "pzt5:pc:06";
        const std::string k07  = "pzt5:pc:07";// 畸形命令的靶，永远不该建成
        const std::string k08  = "pzt5:pc:08";// 回落对象写的键，物理名跟着段配置走

        types_bag bag;
        const pz::redis::conn_counters &c0 = pz::redis::redis_conn_base::counters();
        unsigned long long q_timeout       = c0.timeout.load();
        unsigned long long q_poisoned      = c0.poisoned.load();
        unsigned long long q_prefix        = c0.prefix_skip.load();
        unsigned long long q_reject        = c0.arg_reject.load();
        unsigned long long q_drop          = c0.stale_drop.load();

        // PC01 单键位往返：只自证"发得出去、回得来"。写和读是同一条带覆盖的连接，前缀加没加它自己
        // 看不见（前缀处理错了这条也照样通过），所以它的价值是给 PC02 当参照：页面对称、错在别处。
        auto p1w = co_await pc.async_str_set(k01, "override");
        auto p1r = co_await pc.async_str_get(k01);
        bag.add_p("PC01-override-set-get-roundtrip",
                  is_str(p1w) && p1w.to_string() == "OK" && is_str(p1r) && p1r.to_string() == "override",
                  "set=" + tshow(p1w.to_string()) + " get=" + tshow(p1r.to_string()));

        // PC02 同段不带覆盖的对象读不到它 ⇒ 覆盖真的换了物理名（不是只在页内自洽）
        auto p2r = co_await rc.async_str_get(k01);
        bool p2e = co_await rc.async_key_exists(k01);
        bag.add_p("PC02-no-override-client-cannot-read", is_nil(p2r) && !p2e, "plain_get=" + std::string(ttype(p2r)) + "(" + tshow(p2r.to_string()) + ")" + " plain_exists=" + std::string(p2e ? "1" : "0"));

        // PC03 成员位不是 key：三格里两格的名字就叫 "1" / "2"，被加前缀的话 SISMEMBER "1" 回 0、
        // 成员集会长成 pctor:1,pctor:2（页内读得出来，因为它查的是裸名）
        auto p3a = co_await pc.async_set_add(k03, {"1", "2", "lv"});
        auto p3i = co_await pc.async_set_is_member(k03, "1");
        auto p3m = co_await pc.async_set_members(k03);
        bag.add_p("PC03-member-positions-not-prefixed",
                  is_num(p3a) && p3a.to_int() == 3 && is_num(p3i) && p3i.to_int() == 1 &&
                      set_show(p3m) == "1,2,lv",
                  "added=" + tshow(p3a.to_string()) + " ismember_1=" + tshow(p3i.to_string()) +
                      " members=[" + set_show(p3m) + "] want=3/1/[1,2,lv]");

        // PC04 两个键位都要加（SMOVE 在表里是 kFirst2）：只加 src 的话成员搬进裸名键，
        // 带覆盖的目标键就是空的；只加 dst 的话 src 读不到、SMOVE 直接回 0。
        co_await pc.async_set_add(k04a, {"m1", "m2"});
        auto p4m  = co_await pc.async_set_move(k04a, k04b, "m1");
        auto p4ca = co_await pc.async_set_card(k04a);
        auto p4cb = co_await pc.async_set_card(k04b);
        auto p4mb = co_await pc.async_set_members(k04b);
        bag.add_p("PC04-both-key-slots-prefixed",
                  is_num(p4m) && p4m.to_int() == 1 && is_num(p4ca) && p4ca.to_int() == 1 &&
                      is_num(p4cb) && p4cb.to_int() == 1 && set_show(p4mb) == "m1",
                  "move=" + tshow(p4m.to_string()) + " src=" + tshow(p4ca.to_string()) +
                      " dst=" + tshow(p4cb.to_string()) + " dstmembers=[" + set_show(p4mb) +
                      "] want=1/1/1/[m1]");

        // PC05 *STORE 没有 numkeys 参数位：dst 打头、其后整串都是键。
        // 谁照 Zset 抄成"args[2] 是数字"，这里会连着失败两条：并集读成空（这条）+ prefix_skip 被拨动（PC09）。
        co_await pc.async_set_add(k05a, {"a1", "a2"});
        co_await pc.async_set_add(k05n, {"n1"});
        auto p5u = co_await pc.async_set_union_store(k05dst, {k05a, k05n});
        auto p5m = co_await pc.async_set_members(k05dst);
        bag.add_p("PC05-store-takes-no-numkeys-slot",
                  is_num(p5u) && p5u.to_int() == 3 && set_show(p5m) == "a1,a2,n1",
                  "card=" + tshow(p5u.to_string()) + " members=[" + set_show(p5m) + "] want=3/[a1,a2,n1]");

        // PC06 键级命令（EXPIRE / TTL）也吃覆盖，且 plain 那一面必须看见 -2（键在它眼里根本不存在）。
        // 这条在两个段上都查得出来：覆盖被忽略时 [default] 上 plain 读到的是同一个键，TTL 就不是 -2。
        auto p6s           = co_await pc.async_str_set(k06, "ttl");
        bool p6e           = co_await pc.async_key_expire(k06, 30);
        long long p6t      = co_await pc.async_key_ttl(k06);
        long long p6tplain = co_await rc.async_key_ttl(k06);
        bag.add_p("PC06-key-ttl-commands-take-override",
                  is_str(p6s) && p6s.to_string() == "OK" && p6e && p6t > 0 && p6t <= 30 && p6tplain == -2,
                  "expire=" + std::string(p6e ? "1" : "0") + " ttl=" + std::to_string(p6t) +
                      " want=1..30 plain_ttl=" + std::to_string(p6tplain) + " want=-2");

        // PC07 覆盖通路上 prefix_skip 可达：故意发一格非数字的 numkeys——
        // "宁可一格不加 + 计一次"那支唯一可见的信号就是计数器，删掉奇数/非法判定整套正常命令照样全部通过。
        // [default] 那一段配置本来就是空前缀，这一支要靠"覆盖非空"才走得到。
        auto p7                   = co_await pc.async_command({"SINTERCARD", "notanumber", k07});
        unsigned long long p7skip = pz::redis::redis_conn_base::counters().prefix_skip.load() - q_prefix;
        bag.add_p("PC07-prefix-skip-reachable-through-override",
                  p7skip == 1 && is_str(p7) && starts_with(p7.to_string(), "ERR"),
                  "skip=" + std::to_string(p7skip) + " want=1 reply=" + std::string(ttype(p7)) +
                      "(" + tshow(p7.to_string()) + ")");

        // PC08 空覆盖 = 回落段配置：fb（显式空串）写的键，不带覆盖的 rc 读得到、带覆盖的 pc 读不到。
        // 这条要在配了 prefix 的段上才查得出来：[default] 那一段配置本来就是空的，"回落"与"空串当有效前缀"同形。
        auto p8w = co_await fb.async_str_set(k08, "fallback");
        auto p8r = co_await rc.async_str_get(k08);
        auto p8p = co_await pc.async_str_get(k08);
        bag.add_p("PC08-empty-override-falls-back-to-section",
                  is_str(p8w) && p8w.to_string() == "OK" && is_str(p8r) && p8r.to_string() == "fallback" &&
                      is_nil(p8p),
                  "set=" + tshow(p8w.to_string()) + " plain=" + tshow(p8r.to_string()) +
                      " want=fallback override_get=" + std::string(ttype(p8p)));

        // 收尾：三个对象各删一遍。带覆盖的对象写进的物理名和不带覆盖的不是同一个，
        // 多删一次不是错（删不存在的键回 0），少删一次就会在服务器上留残留。
        std::vector<std::string> pc_keys = {k01, k03, k04a, k04b, k05dst, k05a, k05n, k06, k07, k08};
        co_await pc.async_str_del(pc_keys);
        co_await rc.async_str_del(pc_keys);
        co_await fb.async_str_del(pc_keys);
        unsigned int left_pc    = co_await count_existing(pc, pc_keys);
        unsigned int left_plain = co_await count_existing(rc, pc_keys);

        // PC09 覆盖通路的卫生：这一组只许 prefix_skip 动一格（PC07 那一次），其余四条连接级错误
        // 计数一格都不许动；两面数到的残留都得是 0。
        {
            const pz::redis::conn_counters &cnt = pz::redis::redis_conn_base::counters();
            long long d_timeout                 = static_cast<long long>(cnt.timeout.load() - q_timeout);
            long long d_poisoned                = static_cast<long long>(cnt.poisoned.load() - q_poisoned);
            long long d_reject                  = static_cast<long long>(cnt.arg_reject.load() - q_reject);
            long long d_drop                    = static_cast<long long>(cnt.stale_drop.load() - q_drop);
            bag.add_p("PC09-override-path-hygiene",
                      d_timeout == 0 && d_poisoned == 0 && d_reject == 0 && d_drop == 0 && p7skip == 1 &&
                          left_pc == 0 && left_plain == 0,
                      "timeout=" + std::to_string(d_timeout) + " poisoned=" + std::to_string(d_poisoned) +
                          " reject=" + std::to_string(d_reject) + " stale_drop=" + std::to_string(d_drop) +
                          " skip=" + std::to_string(p7skip) + " left_override=" + std::to_string(left_pc) +
                          " left_plain=" + std::to_string(left_plain));
        }

        client.val["mode"]    = std::string("pfxctor");
        client.val["section"] = sec;
        // 两个对象的覆盖值一起回显：空串的意思是"本对象不覆盖、吃段配置"，不是"不加前缀"
        client.val["override_prefix"]      = pc.key_prefix();
        client.val["fallback_prefix"]      = fb.key_prefix();
        client.val["keys_written"]         = static_cast<long long>(pc_keys.size());
        client.val["total"]                = static_cast<long long>(bag.checks.size());
        client.val["pass"]                 = static_cast<long long>(bag.pass_n);
        client.val["fail"]                 = static_cast<long long>(bag.checks.size() - bag.pass_n);
        client.val["expected_total"]       = static_cast<long long>(9);
        client.val["all_pass"]             = (bag.checks.size() == 9 && bag.pass_n == 9) ? 1 : 0;
        const pz::redis::conn_counters &qe = pz::redis::redis_conn_base::counters();
        obj_val cd;
        cd.set_object();
        cd["timeout"]                = static_cast<long long>(qe.timeout.load() - q_timeout);
        cd["poisoned"]               = static_cast<long long>(qe.poisoned.load() - q_poisoned);
        cd["prefix_skip"]            = static_cast<long long>(qe.prefix_skip.load() - q_prefix);
        cd["arg_reject"]             = static_cast<long long>(qe.arg_reject.load() - q_reject);
        cd["stale_drop"]             = static_cast<long long>(qe.stale_drop.load() - q_drop);
        client.val["counters_delta"] = cd;

        obj_val arr;
        arr.set_array();
        for (const auto &c : bag.checks)
        {
            obj_val one;
            one.set_object();
            one["name"]   = c.name;
            one["pass"]   = c.pass ? 1 : 0;
            one["detail"] = c.detail;
            arr.push(one);
        }
        client.val["checks"] = arr;
        client.out_json();
        co_return "";
    }

    // 组名打错时六个 do_* 全是 false：期望条数只剩页面级那几条，断言一条类型命令都没发，
    // 页面照样能打出一屏绿并回显 group=<错名>。这里先明确拒掉，让"grp 拼错了"和
    // "这一组全过了"是两种读数；mode 字段就是这条支路的触发计数。
    if (!grp.empty() && grp != "all" && grp != "check" && grp != "string" && grp != "hash" &&
        grp != "list" && grp != "stream" && grp != "set" && grp != "key")
    {
        client.val["mode"]           = std::string("unknown-group");
        client.val["grp"]            = grp;
        client.val["total"]          = 0LL;
        client.val["pass"]           = 0LL;
        client.val["fail"]           = 0LL;
        client.val["expected_total"] = 0LL;
        client.val["all_pass"]       = 0;
        client.val["error"] = std::string("unknown grp; valid: (empty) all check string hash list stream set key, "
                                          "plus one-shot groups cleanup leave skiparm pfxctor");
        client.out_json();
        co_return "";
    }

    const bool do_string = (grp.empty() || grp == "all" || grp == "check" || grp == "string");
    const bool do_hash   = (grp.empty() || grp == "all" || grp == "check" || grp == "hash");
    const bool do_list   = (grp.empty() || grp == "all" || grp == "check" || grp == "list");
    const bool do_stream = (grp.empty() || grp == "all" || grp == "check" || grp == "stream");
    const bool do_set    = (grp.empty() || grp == "all" || grp == "check" || grp == "set");
    const bool do_key    = (grp.empty() || grp == "all" || grp == "check" || grp == "key");
    // 全跑的期望条数就是常量；只跑一组时按组数现算。
    // 常量与组表一旦漂移（新增一组却忘了改常量），全跑时回显的 expected_total 就对不上。
    unsigned int expected = (do_string && do_hash && do_list && do_stream && do_set && do_key) ? kTypesExpectedChecks : ((do_string ? kStringChecks : 0) + (do_hash ? kHashChecks : 0) + (do_list ? kListChecks : 0) + (do_stream ? kStreamChecks : 0) + (do_set ? kSetChecks : 0) + (do_key ? kKeyChecks : 0) + kPageChecks);

    const pz::redis::conn_counters &before  = pz::redis::redis_conn_base::counters();
    unsigned long long c_timeout            = before.timeout.load();
    unsigned long long c_poisoned           = before.poisoned.load();
    unsigned long long c_prefix             = before.prefix_skip.load();
    unsigned long long c_reject             = before.arg_reject.load();
    unsigned long long c_drop               = before.stale_drop.load();
    const pz::redis::pool_counters &pbefore = pz::redis::redis_pool::counters();
    unsigned long long c_created            = pbefore.created.load();
    unsigned long long c_close              = pbefore.over_limit_close.load();

    types_bag bag;
    obj_val readings;
    readings.set_object();
    // 收尾要用"跑之前"的计数，所以先把它们抄成一份局部值（counters() 是同一个对象，不能用引用做差）
    pz::redis::conn_counters snap;
    snap.timeout.store(c_timeout);
    snap.poisoned.store(c_poisoned);
    snap.prefix_skip.store(c_prefix);
    snap.arg_reject.store(c_reject);
    snap.stale_drop.store(c_drop);

    if (do_string)
        co_await run_string(rc, bag, readings);
    if (do_hash)
        co_await run_hash(rc, bag, readings);
    if (do_list)
        co_await run_list(rc, bag, readings);
    if (do_stream)
        co_await run_stream(rc, bag, readings);
    if (do_set)
        co_await run_set(rc, bag, readings);
    if (do_key)
        co_await run_key(rc, bag, readings);
    co_await run_page(rc, bag, readings, snap, do_string, do_hash, do_list, do_stream, do_set, do_key);

    client.val["group"]          = grp.empty() ? std::string("all") : grp;
    client.val["section"]        = sec;
    client.val["total"]          = static_cast<long long>(bag.checks.size());
    client.val["pass"]           = static_cast<long long>(bag.pass_n);
    client.val["fail"]           = static_cast<long long>(bag.checks.size() - bag.pass_n);
    client.val["expected_total"] = static_cast<long long>(expected);
    client.val["all_pass"]       = (bag.checks.size() == expected && bag.pass_n == expected) ? 1 : 0;

    const pz::redis::conn_counters &cnt = pz::redis::redis_conn_base::counters();
    obj_val delta;
    delta.set_object();
    delta["timeout"]     = static_cast<long long>(cnt.timeout.load() - c_timeout);
    delta["poisoned"]    = static_cast<long long>(cnt.poisoned.load() - c_poisoned);
    delta["prefix_skip"] = static_cast<long long>(cnt.prefix_skip.load() - c_prefix);
    delta["arg_reject"]  = static_cast<long long>(cnt.arg_reject.load() - c_reject);
    delta["stale_drop"]  = static_cast<long long>(cnt.stale_drop.load() - c_drop);
    out_pool_counters(delta, pz::redis::redis_pool::counters(), c_created, c_close);
    client.val["counters_delta"] = delta;

    client.val["readings"] = readings;
    if (grp != "check")
    {
        obj_val arr;
        arr.set_array();
        for (const auto &c : bag.checks)
        {
            obj_val one;
            one.set_object();
            one["name"]   = c.name;
            one["pass"]   = c.pass ? 1 : 0;
            one["detail"] = c.detail;
            arr.push(one);
        }
        client.val["checks"] = arr;
    }
    else
    {
        obj_val bad;
        bad.set_array();
        for (const auto &c : bag.checks)
        {
            if (c.pass)
                continue;
            obj_val one;
            one.set_object();
            one["name"]   = c.name;
            one["detail"] = c.detail;
            bad.push(one);
        }
        client.val["failed"] = bad;
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

// ==================== redis/types/sync：同步入口（在业务线程上借还连接）====================
// 这一页存在的理由：协程版走的是池的 async_exec，同步版走的是这个类从业务线程上直接调 get_conn 的路径，
// 借还是否闭合只能实测。n 是一次借还循环的条数（命令数 = 2n），跑完后可以在服务端一侧核对连接数。
//@urlpath(null,redis/types/sync)
std::string test_redis_types_sync(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    client.val.set_object();

    if (!pz::redis::get_redis_pool().is_loaded())
    {
        client.val["error"] = "redis pool not loaded (ENABLE_REDIS off or conf/redis.conf missing)";
        client.out_json();
        return "";
    }

    std::string sec = client.get["sec"].to_string();
    if (sec.empty())
        sec = "default";
    unsigned int n = static_cast<unsigned int>(client.get["n"].to_int());
    if (n == 0)
        n = 50;
    if (n > 200)
        n = 200;

    pz::redis::redis_client rc(sec);
    key_table k                   = make_keys();
    std::vector<std::string> keys = k.all();

    types_bag bag;
    std::string bin_value = "y\r\nz";

    // 借还没闭合的话，最先动的就是这几个计数：脏连接不回池 ⇒ stale_drop 每次借还 +1，
    // 上一次命令的残留字节会被下一次读到 ⇒ poisoned/timeout 涨。所以它们必须全 0。
    const pz::redis::conn_counters &c0 = pz::redis::redis_conn_base::counters();
    unsigned long long k_timeout       = c0.timeout.load();
    unsigned long long k_poisoned      = c0.poisoned.load();
    unsigned long long k_prefix        = c0.prefix_skip.load();
    unsigned long long k_reject        = c0.arg_reject.load();
    unsigned long long k_drop          = c0.stale_drop.load();
    const pz::redis::pool_counters &p0 = pz::redis::redis_pool::counters();
    unsigned long long k_created       = p0.created.load();
    unsigned long long k_close         = p0.over_limit_close.load();

    // 一次借还循环：每条命令自己借一条、发完就还
    std::chrono::steady_clock::time_point t_start = std::chrono::steady_clock::now();
    bool loop_ok                                  = true;
    for (unsigned int i = 0; i < n; i++)
    {
        auto w = rc.str_set(k.s_main, "loop");
        auto r = rc.str_get(k.s_main);
        if (!(is_str(w) && w.to_string() == "OK" && is_str(r) && r.to_string() == "loop"))
            loop_ok = false;
    }
    std::chrono::steady_clock::time_point t_end = std::chrono::steady_clock::now();
    unsigned long long elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
    bag.add_s("Y01-sync-loop-set-get", loop_ok, "n=" + std::to_string(n) + " commands=" + std::to_string(n * 2));

    // 二进制值原样回读（同步版拼包与协程版共用同一条通路）
    rc.str_set(k.s_bin, bin_value);
    auto gb = rc.str_get(k.s_bin);
    bag.add_s("Y02-sync-binary-roundtrip", is_str(gb) && gb.to_string() == bin_value, "got=" + tshow(gb.to_string()));

    // 键级四条：SET EX 之后 TTL 有期限，EXPIRE/PERSIST 各自改得动，EXISTS 分得出有无
    rc.str_set(k.s_noxp, "x", {"EX", "30"});
    long long t1 = rc.key_ttl(k.s_noxp);
    bool p2      = rc.key_persist(k.s_noxp);
    long long t2 = rc.key_ttl(k.s_noxp);
    bool f3      = rc.key_expire(k.s_noxp, 20);
    long long t3 = rc.key_ttl(k.s_noxp);
    bag.add_s("Y03-sync-ttl-persist-expire",
              t1 > 0 && t1 <= 30 && p2 && t2 == -1 && f3 && t3 > 0 && t3 <= 20,
              "ttl_ex=" + std::to_string(t1) + " persist=" + std::string(p2 ? "1" : "0") +
                  " ttl_after_persist=" + std::to_string(t2) + " expire=" + std::string(f3 ? "1" : "0") +
                  " ttl_after_expire=" + std::to_string(t3));

    bool y4a = rc.key_exists(k.s_noxp);
    bool y4b = rc.key_exists(k.s_missing);
    bag.add_s("Y04-sync-exists", y4a && !y4b, "present=" + std::string(y4a ? "1" : "0") + " missing=" + std::string(y4b ? "1" : "0"));

    // Hash：单字段入口 + 扁平数组自己配对
    auto h1        = rc.hash_set(k.h_main, "f1", "v1");
    auto h2        = rc.hash_get(k.h_main, "f1");
    auto h3        = rc.hash_getall(k.h_main);
    obj_val paired = flat_pairs_to_object(h3);
    bag.add_s("Y05-sync-hash-three",
              is_num(h1) && h1.to_int() == 1 && is_str(h2) && h2.to_string() == "v1" && is_arr(h3) &&
                  h3.size() == 2 && paired.size() == 1 && paired["f1"].to_string() == "v1",
              "hset=" + std::string(ttype(h1)) + "(" + tshow(h1.to_string()) + ") hget=" + tshow(h2.to_string()) +
                  " getall_size=" + std::to_string(h3.size()));

    // 锁：一条 SET NX EX 拿到，第二次抢回 null，值还是第一个主人
    // 先清掉这把锁键：上一次运行跑到一半中断会留下脏锁，那时"第一次就抢不到"并不是本层的错
    rc.str_del({k.s_lock});
    auto l1 = rc.str_set(k.s_lock, "ownerA", {"NX", "EX", "30"});
    auto l2 = rc.str_set(k.s_lock, "ownerB", {"NX"});
    auto lv = rc.str_get(k.s_lock);
    bag.add_s("Y06-sync-set-nx-ex-lock",
              is_str(l1) && l1.to_string() == "OK" && is_nil(l2) && is_str(lv) && lv.to_string() == "ownerA",
              "first=" + std::string(ttype(l1)) + "(" + tshow(l1.to_string()) + ") second=" +
                  std::string(ttype(l2)) + " holder=" + tshow(lv.to_string()));

    // ===== List 的同步用例：把 List 命令也放上业务线程的借还通路 =====
    // 上一次运行跑到一半中断也会留下键，而这一族断言比的正是元素顺序和剩余长度 ⇒ 先把自己这几个键清干净
    rc.str_del({k.l_main, k.l_trim, k.l_ins, k.l_src, k.l_dst, k.l_bin});

    // YL01 LPUSH/RPUSH/LLEN/LRANGE/LINDEX 一次搬完；顺序按实测的 [c,b,a,d]（LPUSH 逐个塞头）
    auto yl1a = rc.list_push_left(k.l_main, {"a", "b", "c"});
    auto yl1b = rc.list_push_right(k.l_main, {"d"});
    auto yl1c = rc.list_length(k.l_main);
    auto yl1d = rc.list_range(k.l_main, 0, -1);
    auto yl1e = rc.list_at(k.l_main, -1);
    bag.add_l("YL01-sync-lpush-rpush-llen-lrange-lindex",
              is_num(yl1a) && yl1a.to_int() == 3 && is_num(yl1b) && yl1b.to_int() == 4 &&
                  is_num(yl1c) && yl1c.to_int() == 4 && is_arr(yl1d) && yl1d.size() == 4 &&
                  yl1d[0].to_string() == "c" && yl1d[3].to_string() == "d" && is_str(yl1e) &&
                  yl1e.to_string() == "d",
              "lpush=" + tshow(yl1a.to_string()) + " rpush=" + tshow(yl1b.to_string()) + " llen=" +
                  tshow(yl1c.to_string()) + " range=[" + arr_show(yl1d) + "] idx-1=" + tshow(yl1e.to_string()));

    // YL02 带 count 的 LPOP 回数组、省略 count 的回 bulk。
    // 这个形状差在同步通路上也要断一遍：拼包用的是同一个 count_arg，走的是另一条借还线程。
    auto yl2a = rc.list_pop_left(k.l_main, 2);
    auto yl2b = rc.list_pop_right(k.l_main);
    auto yl2c = rc.list_length(k.l_main);
    bag.add_l("YL02-sync-lpop-count-array-and-omitted-bulk",
              is_arr(yl2a) && yl2a.size() == 2 && yl2a[0].to_string() == "c" && is_str(yl2b) &&
                  yl2b.to_string() == "d" && is_num(yl2c) && yl2c.to_int() == 1,
              "pop2=" + std::string(ttype(yl2a)) + "([" + arr_show(yl2a) + "]) rpop=" +
                  std::string(ttype(yl2b)) + "(" + tshow(yl2b.to_string()) + ") remaining=" +
                  tshow(yl2c.to_string()));

    // YL03 LSET / LTRIM / LINSERT / LMOVE / key_type 各一次
    auto yl3a = rc.list_set_at(k.l_main, 0, "A2");
    auto yl3b = rc.list_trim(k.l_main, 0, 5);
    rc.list_push_left(k.l_ins, {"b"});
    auto yl3c = rc.list_insert(k.l_ins, "BEFORE", "b", "a");
    auto yl3d = rc.list_insert(k.l_ins, "BEFORE", "nosuchpivot", "z");
    rc.list_push_left(k.l_src, {"s1", "s2"});
    rc.list_push_left(k.l_dst, {"d1"});
    auto yl3e = rc.list_move(k.l_src, k.l_dst, "LEFT", "RIGHT");
    auto yl3f = rc.key_type(k.l_main);
    bag.add_l("YL03-sync-lset-ltrim-linsert-lmove-keytype",
              is_str(yl3a) && yl3a.to_string() == "OK" && is_str(yl3b) && yl3b.to_string() == "OK" &&
                  is_num(yl3c) && yl3c.to_int() == 2 && is_num(yl3d) && yl3d.to_int() == -1 &&
                  is_str(yl3e) && yl3e.to_string() == "s2" && is_str(yl3f) && yl3f.to_string() == "list",
              "lset=" + tshow(yl3a.to_string()) + " ltrim=" + tshow(yl3b.to_string()) + " linsert=" +
                  tshow(yl3c.to_string()) + " linsert-miss=" + tshow(yl3d.to_string()) + " lmove=" +
                  tshow(yl3e.to_string()) + " type=" + tshow(yl3f.to_string()));

    // ===== Stream 的同步用例：XADD / XRANGE / XLEN 与消费者组那一族 =====
    // 同理先清自己的键：上一次运行中断会留下带消费者组的流，而 XPENDING 数的正是 PEL 条数
    rc.str_del({k.x_main, k.x_nomk});

    // YS01 XADD 两条 + XLEN + XRANGE 全量：id 递增、字段扁平对回原样
    auto ys1a          = rc.stream_add(k.x_main, {"f", "v"});
    auto ys1b          = rc.stream_add(k.x_main, {"f1", "v1", "f2", "v2"});
    auto ys1c          = rc.stream_len(k.x_main);
    auto ys1d          = rc.stream_range(k.x_main, "-", "+");
    std::string ys1a_s = ys1a.to_string();
    std::string ys1b_s = ys1b.to_string();
    bool ys1_id_shape  = is_str(ys1a) && is_str(ys1b) && ys1a_s.find("-") != std::string::npos &&
                        ys1b_s.find("-") != std::string::npos;
    bag.add_x("YS01-sync-xadd-xlen-xrange",
              ys1_id_shape && is_num(ys1c) && ys1c.to_int() == 2 && is_arr(ys1d) && ys1d.size() == 2 &&
                  ys1d[0][0].to_string() == ys1a_s && ys1d[0][1].size() == 2 &&
                  ys1d[1][1].size() == 4 && ys1d[1][1][3].to_string() == "v2",
              "id1=" + tshow(ys1a.to_string()) + " id2=" + tshow(ys1b.to_string()) + " xlen=" +
                  tshow(ys1c.to_string()) + " range_size=" + std::to_string(ys1d.size()));

    // YS02 opts 夹在 key 与 id 之间：MAXLEN 2 必须真的截断（opts 拼错段位这条会失败）
    auto ys2    = rc.stream_add(k.x_nomk, {"i", "3"}, "*", {"MAXLEN", "2"});
    auto ys2len = rc.stream_len(k.x_nomk);
    auto ys2cnt = rc.stream_range(k.x_nomk, "-", "+", 1);
    bag.add_x("YS02-sync-xadd-maxlen-and-count",
              is_str(ys2) && is_num(ys2len) && ys2len.to_int() == 1 && is_arr(ys2cnt) && ys2cnt.size() == 1,
              "xadd=" + std::string(ttype(ys2)) + "(" + tshow(ys2.to_string()) + ") xlen=" +
                  tshow(ys2len.to_string()) + " xrange_count1=" + std::to_string(ys2cnt.size()));

    // YS03 XGROUP CREATE / XREADGROUP / XPENDING / XACK 一整套：PEL 从 2 条被一次 XACK 两个 id 清成 0 条
    auto ys3g = rc.stream_group_create(k.x_main, "syng", "0-0");
    auto ys3r = rc.stream_read_group("syng", "sync1", {k.x_main}, {">"});
    auto ys3p = rc.stream_pending(k.x_main, "syng");
    std::vector<std::string> ys3ids;
    if (is_arr(ys3r) && ys3r.size() == 1 && is_arr(ys3r[0][1]) && ys3r[0][1].size() == 2)
        for (unsigned int i = 0; i < 2; i++)
            ys3ids.push_back(ys3r[0][1][i][0].to_string());
    auto ys3a        = rc.stream_ack(k.x_main, "syng", ys3ids);
    auto ys3p2       = rc.stream_pending(k.x_main, "syng");
    long long ys3pel = -1, ys3pel2 = -1;
    if (is_arr(ys3p) && is_num(ys3p[0]))
        ys3pel = ys3p[0].to_int();
    if (is_arr(ys3p2) && is_num(ys3p2[0]))
        ys3pel2 = ys3p2[0].to_int();
    bag.add_x("YS03-sync-xgroup-xreadgroup-xpending-xack",
              is_str(ys3g) && ys3g.to_string() == "OK" && ys3ids.size() == 2 && !ys3ids[0].empty() &&
                  ys3pel == 2 && is_num(ys3a) && ys3a.to_int() == 2 && ys3pel2 == 0,
              "create=" + tshow(ys3g.to_string()) + " ids=" + tshow(ys3ids.empty() ? "" : ys3ids[0]) +
                  " pending=" + std::to_string(ys3pel) + " ack=" + tshow(ys3a.to_string()) +
                  " pending_after=" + std::to_string(ys3pel2));

    // YS04 XINFO 三个子命令走同步通路：条目数/组数对上，组名与消费者名是裸名（prefix 只加在 key 位），
    // 而 XACK 之后两侧 pending 都归 0 —— 这条同时验证了"同步通路也真发了 XINFO"。
    auto ys4s             = rc.stream_info(k.x_main, "STREAM");
    auto ys4g             = rc.stream_info(k.x_main, "GROUPS");
    auto ys4c             = rc.stream_info(k.x_main, "CONSUMERS", {"syng"});
    bool ys4_one_g        = is_arr(ys4g) && ys4g.size() == 1;
    bool ys4_one_c        = is_arr(ys4c) && ys4c.size() == 1;
    std::string ys4_gname = ys4_one_g ? flat_str(ys4g[0], "name") : std::string();
    long long ys4_gpend   = ys4_one_g ? flat_int(ys4g[0], "pending") : -1;
    std::string ys4_cname = ys4_one_c ? flat_str(ys4c[0], "name") : std::string();
    long long ys4_cpend   = ys4_one_c ? flat_int(ys4c[0], "pending") : -1;
    bag.add_x("YS04-sync-xinfo-stream-groups-consumers",
              flat_int(ys4s, "length") == 2 && flat_int(ys4s, "groups") == 1 && ys4_gname == "syng" &&
                  ys4_gpend == 0 && ys4_cname == "sync1" && ys4_cpend == 0,
              "length=" + std::to_string(flat_int(ys4s, "length")) + " groups=" +
                  std::to_string(flat_int(ys4s, "groups")) + " gname=" + tshow(ys4_gname) + " gpending=" +
                  std::to_string(ys4_gpend) + " cname=" + tshow(ys4_cname) + " cpending=" +
                  std::to_string(ys4_cpend) + " want=2/1/syng/0/sync1/0");

    // ===== Set 的同步用例：*STORE 整串参数都是键（没有 numkeys 参数位），SINTERCARD 的数字位是现算的 =====
    // 先清自己这几个键：上一次运行中断会留下 st_dst（断言比的正是它的内容）
    rc.str_del({k.st_a, k.st_b, k.st_dst, k.st_num1, k.st_num2});
    rc.command({"SADD", k.st_a, "m1", "m2", "m3"});
    rc.command({"SADD", k.st_b, "m2", "m3", "m4"});
    rc.command({"SADD", k.st_num1, "u1a"});
    rc.command({"SADD", k.st_num2, "u2a"});
    // 并集把键名恰好是 "1" 的那格也当源键：并进来才是对的，被当数字吃掉就少一个成员
    auto ys5u = rc.set_union_store(k.st_dst, {k.st_a, k.st_num1, k.st_b});
    auto ys5m = rc.command({"SMEMBERS", k.st_dst});
    auto ys5c = rc.set_intersect_card({k.st_a, k.st_b});
    auto ys5l = rc.set_intersect_card({k.st_a, k.st_b}, {"LIMIT", "1"});
    bag.add_st("YS05-sync-store_keys_all_and_sintercard_numkeys",
               is_num(ys5u) && ys5u.to_int() == 5 && set_show(ys5m) == "m1,m2,m3,m4,u1a" && is_num(ys5c) &&
                   ys5c.to_int() == 2 && is_num(ys5l) && ys5l.to_int() == 1,
               "card=" + tshow(ys5u.to_string()) + " members=[" + set_show(ys5m) + "] sintercard=" +
                   tshow(ys5c.to_string()) + " limit1=" + tshow(ys5l.to_string()) +
                   " want=5/[m1,m2,m3,m4,u1a]/2/1");

    // ===== Set 单键族的同步用例：这一族也放到业务线程上发一遍 =====
    // YS05 刚把 st_a/st_b/st_dst 用进了并集用例 ⇒ 先清干净再建自己的 fixture（比的是内容，脏键会直接导致失败）
    rc.str_del({k.st_a, k.st_b, k.st_dst});

    // YS06 add/card/members/is_member/are_members/remove 一条链走完，口径与协程页 ST12/ST15 相同
    auto ys6a = rc.set_add(k.st_a, {"a1", "a2", "a3", "a3"});
    auto ys6b = rc.set_card(k.st_a);
    auto ys6c = rc.set_members(k.st_a);
    auto ys6d = rc.set_is_member(k.st_a, "a2");
    auto ys6e = rc.set_is_member(k.st_a, "nope");
    auto ys6f = rc.set_are_members(k.st_a, {"a1", "nope"});
    auto ys6g = rc.set_remove(k.st_a, {"a1", "nope"});
    auto ys6h = rc.set_members(k.st_a);
    bag.add_st("YS06-sync-set-single-key-family",
               is_num(ys6a) && ys6a.to_int() == 3 && is_num(ys6b) && ys6b.to_int() == 3 &&
                   set_show(ys6c) == "a1,a2,a3" && is_num(ys6d) && ys6d.to_int() == 1 &&
                   is_num(ys6e) && ys6e.to_int() == 0 && is_arr(ys6f) && ys6f.size() == 2 &&
                   ys6f[0].to_int() == 1 && ys6f[1].to_int() == 0 && is_num(ys6g) && ys6g.to_int() == 1 &&
                   set_show(ys6h) == "a2,a3",
               "add=" + tshow(ys6a.to_string()) + " card=" + tshow(ys6b.to_string()) + " members=[" +
                   set_show(ys6c) + "] is_member=" + tshow(ys6d.to_string()) + "/" + tshow(ys6e.to_string()) +
                   " are_members=[" + arr_show(ys6f) + "] srem=" + tshow(ys6g.to_string()) + " left=[" +
                   set_show(ys6h) + "] want=3/3/[a1,a2,a3]/1/0/[1,0]/1/[a2,a3]");

    // YS07 形状断言 + 消费断言 + 三条集合代数：
    // SRANDMEMBER 无 count 是 bulk、带 count 是数组且**不删**成员；SPOP 无 count 是 bulk 且**真删**；
    // SMOVE 搬成回 1，再搬同一条回 0（成员已经在 dst 里了）。
    // dst 特意留一个成员 keep：源集合被搬空时服务端会把那个键直接删掉（SPOP 弹空同理），
    // 而收尾的 Y07 数的是"本页写过的 16 把键全被 DEL 清干净"——键自己先没了，DEL 就只有 15。
    auto ys7a = rc.set_random_member(k.st_a);
    auto ys7b = rc.set_random_member(k.st_a, 2);
    auto ys7c = rc.set_card(k.st_a);
    auto ys7d = rc.set_pop(k.st_a);
    auto ys7e = rc.set_card(k.st_a);
    rc.set_add(k.st_b, {"b1", "b2"});
    rc.set_add(k.st_dst, {"mv", "keep"});
    auto ys7f = rc.set_move(k.st_dst, k.st_b, "mv");
    auto ys7g = rc.set_move(k.st_dst, k.st_b, "mv");
    auto ys7h = rc.set_members(k.st_b);
    auto ys7i = rc.set_union({k.st_b, k.st_missing});
    auto ys7j = rc.set_intersection({k.st_a, k.st_b});
    auto ys7k = rc.set_difference({k.st_b, k.st_a});
    bag.add_st("YS07-sync-set_shapes_move_and_algebra",
               is_str(ys7a) && is_arr(ys7b) && ys7b.size() == 2 && is_num(ys7c) && ys7c.to_int() == 2 &&
                   is_str(ys7d) && is_num(ys7e) && ys7e.to_int() == 1 && is_num(ys7f) && ys7f.to_int() == 1 &&
                   is_num(ys7g) && ys7g.to_int() == 0 && set_show(ys7h) == "b1,b2,mv" &&
                   set_show(ys7i) == "b1,b2,mv" && is_arr(ys7j) && ys7j.size() == 0 &&
                   set_show(ys7k) == "b1,b2,mv",
               "rand=" + std::string(ttype(ys7a)) + "([" + set_show(ys7b) + "]) card=" +
                   tshow(ys7c.to_string()) + " pop=" + std::string(ttype(ys7d)) + "(" +
                   tshow(ys7d.to_string()) + ") card=" + tshow(ys7e.to_string()) + " move=" +
                   tshow(ys7f.to_string()) + "/" + tshow(ys7g.to_string()) + " dst=[" + set_show(ys7h) +
                   "] union=[" + set_show(ys7i) + "] inter=" +
                   std::to_string(is_arr(ys7j) ? ys7j.size() : 0) + " diff=[" + set_show(ys7k) +
                   "] want=bulk/2/1/1/0/[b1,b2,mv]x3/0");

    // 收尾：同步 DEL 整张键名表，残留必须是 0（数的是服务端状态）
    // 本页自己写了 16 个键（s_main/s_bin/s_noxp/s_lock/h_main + List 族的 l_main/l_ins/l_src/l_dst
    // + Stream 族的 x_main/x_nomk + Set 族的 st_a/st_b/st_dst/st_num1/st_num2）
    // ⇒ DEL 至少数到 16，静默 no-op 会在这里暴露
    auto dk = rc.str_del(keys);
    std::vector<std::string> exists_args;
    exists_args.emplace_back("EXISTS");
    for (const auto &one : keys)
        exists_args.push_back(one);
    auto left = rc.command(exists_args);
    bag.add_s("Y07-sync-cleanup-no-residual",
              is_num(dk) && dk.to_int() >= 16 && is_num(left) && left.to_int() == 0,
              "del=" + (is_num(dk) ? tshow(dk.to_string()) : std::string(ttype(dk))) + " table=" +
                  std::to_string(keys.size()) + " leftover=" +
                  (is_num(left) ? tshow(left.to_string()) : std::string(ttype(left))));

    // ===== 构造参数带前缀：协程页 ?grp=pfxctor 那九条的同步镜像 =====
    // 断言逐条照抄，只差在"每条命令在业务线程上借还一次"。键名用 pzt5:yp:* 与协程页那组错开，
    // 物理名同样是 pctor:pzt5:yp:*。本组的键不在键名表里，所以 Y07 的 16 把与这里的残留各数各的。
    {
        const std::string pfx = "pctor:";
        pz::redis::redis_client pc(sec, pfx);
        pz::redis::redis_client fb(sec, "");// 显式空覆盖 = 该回落到段配置
        const std::string k01    = "pzt5:yp:01";
        const std::string k03    = "pzt5:yp:03";
        const std::string k04a   = "pzt5:yp:04a";
        const std::string k04b   = "pzt5:yp:04b";
        const std::string k05dst = "pzt5:yp:05dst";
        const std::string k05a   = "pzt5:yp:05a";
        const std::string k05n   = "1";// 源键的名字就是一个数字（键位错位时会被当成 numkeys 吃掉）
        const std::string k06    = "pzt5:yp:06";
        const std::string k07    = "pzt5:yp:07";
        const std::string k08    = "pzt5:yp:08";

        const pz::redis::conn_counters &y0 = pz::redis::redis_conn_base::counters();
        unsigned long long y_timeout       = y0.timeout.load();
        unsigned long long y_poisoned      = y0.poisoned.load();
        unsigned long long y_prefix        = y0.prefix_skip.load();
        unsigned long long y_reject        = y0.arg_reject.load();
        unsigned long long y_drop          = y0.stale_drop.load();

        // YP01 单键位往返（对称，看不见前缀本身；真正的验证点在 YP02/YP06）
        auto y1w = pc.str_set(k01, "override");
        auto y1r = pc.str_get(k01);
        bag.add_p("YP01-override-set-get-roundtrip",
                  is_str(y1w) && y1w.to_string() == "OK" && is_str(y1r) && y1r.to_string() == "override",
                  "set=" + tshow(y1w.to_string()) + " get=" + tshow(y1r.to_string()));

        // YP02 同段不带覆盖的对象读不到它
        auto y2r = rc.str_get(k01);
        bool y2e = rc.key_exists(k01);
        bag.add_p("YP02-no-override-client-cannot-read", is_nil(y2r) && !y2e, "plain_get=" + std::string(ttype(y2r)) + "(" + tshow(y2r.to_string()) + ")" + " plain_exists=" + std::string(y2e ? "1" : "0"));

        // YP03 成员位不是 key
        auto y3a = pc.set_add(k03, {"1", "2", "lv"});
        auto y3i = pc.set_is_member(k03, "1");
        auto y3m = pc.set_members(k03);
        bag.add_p("YP03-member-positions-not-prefixed",
                  is_num(y3a) && y3a.to_int() == 3 && is_num(y3i) && y3i.to_int() == 1 &&
                      set_show(y3m) == "1,2,lv",
                  "added=" + tshow(y3a.to_string()) + " ismember_1=" + tshow(y3i.to_string()) +
                      " members=[" + set_show(y3m) + "] want=3/1/[1,2,lv]");

        // YP04 两个键位都要加（kFirst2）
        pc.set_add(k04a, {"m1", "m2"});
        auto y4m  = pc.set_move(k04a, k04b, "m1");
        auto y4ca = pc.set_card(k04a);
        auto y4cb = pc.set_card(k04b);
        auto y4mb = pc.set_members(k04b);
        bag.add_p("YP04-both-key-slots-prefixed",
                  is_num(y4m) && y4m.to_int() == 1 && is_num(y4ca) && y4ca.to_int() == 1 &&
                      is_num(y4cb) && y4cb.to_int() == 1 && set_show(y4mb) == "m1",
                  "move=" + tshow(y4m.to_string()) + " src=" + tshow(y4ca.to_string()) +
                      " dst=" + tshow(y4cb.to_string()) + " dstmembers=[" + set_show(y4mb) +
                      "] want=1/1/1/[m1]");

        // YP05 *STORE 没有 numkeys 参数位
        pc.set_add(k05a, {"a1", "a2"});
        pc.set_add(k05n, {"n1"});
        auto y5u = pc.set_union_store(k05dst, {k05a, k05n});
        auto y5m = pc.set_members(k05dst);
        bag.add_p("YP05-store-takes-no-numkeys-slot",
                  is_num(y5u) && y5u.to_int() == 3 && set_show(y5m) == "a1,a2,n1",
                  "card=" + tshow(y5u.to_string()) + " members=[" + set_show(y5m) + "] want=3/[a1,a2,n1]");

        // YP06 键级命令吃覆盖，且 plain 那一面看见 -2
        auto y6s           = pc.str_set(k06, "ttl");
        bool y6e           = pc.key_expire(k06, 30);
        long long y6t      = pc.key_ttl(k06);
        long long y6tplain = rc.key_ttl(k06);
        bag.add_p("YP06-key-ttl-commands-take-override",
                  is_str(y6s) && y6s.to_string() == "OK" && y6e && y6t > 0 && y6t <= 30 && y6tplain == -2,
                  "expire=" + std::string(y6e ? "1" : "0") + " ttl=" + std::to_string(y6t) +
                      " want=1..30 plain_ttl=" + std::to_string(y6tplain) + " want=-2");

        // YP07 覆盖通路上 prefix_skip 可达（畸形 numkeys ⇒ 一格不加 + 计一次）
        auto y7                   = pc.command({"SINTERCARD", "notanumber", k07});
        unsigned long long y7skip = pz::redis::redis_conn_base::counters().prefix_skip.load() - y_prefix;
        bag.add_p("YP07-prefix-skip-reachable-through-override",
                  y7skip == 1 && is_str(y7) && starts_with(y7.to_string(), "ERR"),
                  "skip=" + std::to_string(y7skip) + " want=1 reply=" + std::string(ttype(y7)) +
                      "(" + tshow(y7.to_string()) + ")");

        // YP08 空覆盖 = 回落段配置（在配了 prefix 的段上才查得出来；[default] 两种写法同形）
        auto y8w = fb.str_set(k08, "fallback");
        auto y8r = rc.str_get(k08);
        auto y8p = pc.str_get(k08);
        bag.add_p("YP08-empty-override-falls-back-to-section",
                  is_str(y8w) && y8w.to_string() == "OK" && is_str(y8r) && y8r.to_string() == "fallback" &&
                      is_nil(y8p),
                  "set=" + tshow(y8w.to_string()) + " plain=" + tshow(y8r.to_string()) +
                      " want=fallback override_get=" + std::string(ttype(y8p)));

        // 收尾：三个对象各删一遍（带覆盖的对象写进的物理名可能不是本对象原本那一个，多删不是错）
        std::vector<std::string> yp_keys = {k01, k03, k04a, k04b, k05dst, k05a, k05n, k06, k07, k08};
        pc.str_del(yp_keys);
        rc.str_del(yp_keys);
        fb.str_del(yp_keys);
        std::vector<std::string> exists_pc    = {"EXISTS"};
        std::vector<std::string> exists_plain = {"EXISTS"};
        for (const auto &one : yp_keys)
        {
            exists_pc.push_back(one);
            exists_plain.push_back(one);
        }
        auto lefty_pc    = pc.command(exists_pc);
        auto lefty_plain = rc.command(exists_plain);

        // YP09 卫生：只许 prefix_skip 动一格，其余四条连接级计数不许动；两面残留都是 0
        {
            const pz::redis::conn_counters &y1 = pz::redis::redis_conn_base::counters();
            long long d_timeout                = static_cast<long long>(y1.timeout.load() - y_timeout);
            long long d_poisoned               = static_cast<long long>(y1.poisoned.load() - y_poisoned);
            long long d_reject                 = static_cast<long long>(y1.arg_reject.load() - y_reject);
            long long d_drop                   = static_cast<long long>(y1.stale_drop.load() - y_drop);
            long long n_plain                  = is_num(lefty_plain) ? lefty_plain.to_int() : -1;
            long long n_pc                     = is_num(lefty_pc) ? lefty_pc.to_int() : -1;
            bag.add_p("YP09-override-path-hygiene",
                      d_timeout == 0 && d_poisoned == 0 && d_reject == 0 && d_drop == 0 && y7skip == 1 &&
                          n_plain == 0 && n_pc == 0,
                      "timeout=" + std::to_string(d_timeout) + " poisoned=" + std::to_string(d_poisoned) +
                          " reject=" + std::to_string(d_reject) + " stale_drop=" + std::to_string(d_drop) +
                          " skip=" + std::to_string(y7skip) + " left_plain=" + std::to_string(n_plain) +
                          " left_override=" + std::to_string(n_pc));
        }
    }

    client.val["section"]              = sec;
    client.val["loop_n"]               = static_cast<long long>(n);
    client.val["elapsed_ms"]           = static_cast<long long>(elapsed_ms);
    client.val["ms_per_command"]       = static_cast<long long>(elapsed_ms) / static_cast<long long>(n * 2);
    client.val["total"]                = static_cast<long long>(bag.checks.size());
    client.val["pass"]                 = static_cast<long long>(bag.pass_n);
    client.val["fail"]                 = static_cast<long long>(bag.checks.size() - bag.pass_n);
    client.val["expected_total"]       = static_cast<long long>(kSyncChecks);
    client.val["all_pass"]             = (bag.checks.size() == kSyncChecks && bag.pass_n == kSyncChecks) ? 1 : 0;
    const pz::redis::conn_counters &c1 = pz::redis::redis_conn_base::counters();
    obj_val cd;
    cd.set_object();
    cd["timeout"]     = static_cast<long long>(c1.timeout.load() - k_timeout);
    cd["poisoned"]    = static_cast<long long>(c1.poisoned.load() - k_poisoned);
    cd["prefix_skip"] = static_cast<long long>(c1.prefix_skip.load() - k_prefix);
    cd["arg_reject"]  = static_cast<long long>(c1.arg_reject.load() - k_reject);
    cd["stale_drop"]  = static_cast<long long>(c1.stale_drop.load() - k_drop);
    out_pool_counters(cd, pz::redis::redis_pool::counters(), k_created, k_close);
    client.val["counters_delta"] = cd;

    obj_val arr;
    arr.set_array();
    for (const auto &c : bag.checks)
    {
        obj_val one;
        one.set_object();
        one["name"]   = c.name;
        one["pass"]   = c.pass ? 1 : 0;
        one["detail"] = c.detail;
        arr.push(one);
    }
    client.val["checks"] = arr;
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif// ENABLE_REDIS

    client.out_json();
    return "";
}
}// namespace http
