#include <map>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <functional>

#include "router.h"
#include "httppeer.h"
#include "serverconfig.h"
#include "server.h"
#include "pool_step.h"
#include "func.h"
#include "debug_log.h"

namespace http
{

router &get_router()
{
    static router instance;
    return instance;
}

// 站点钩子（method_pre / method_after）单次请求最多执行 15 条（界值来自 size() < 16 的写法）。
// 越界时 run_site_hooks 一条都不跑，所以启动期必须把越界报出来（见 validate_site_hooks）
constexpr size_t site_hook_limit = 15;

namespace
{

enum class chain_act : unsigned char
{
    stop,       // 链到此结束，已写出的 output 原样返回
    run_regfun, // 前置放行，继续执行本条目的主 handler
    jump,       // 换成另一个注册名接着跑
    run_timeloop// 主 handler 要求进入间隔任务
};

// 链上一步（pre / regfun）返回值到动作的映射，全项目唯一的解释点。
// 语义：
//   pre    返回 ""        = 拒绝，主 handler 不执行
//   pre    返回 "ok"      = 放行
//   pre    返回当前路由名 = 放行（自跳转保护）
//   pre    返回其它名字   = 跳转
//   regfun 返回 "" / "exit" = 结束
//   regfun 返回 "T"        = 执行 frametasks_timeloop 后结束
//   regfun 返回其它名字    = 跳转
// 「pre 返回自己那条名字」= 放行。名字带不带开头的 '/' 指的是同一条路由，所以这里两种写法都要认得：
// 只比字符串的话，pre 返回裸名会被当成跳转、跳回自己，一路跑到 32 步上限才停
bool same_route_name(const std::string &ret, const std::string &cur_path)
{
    std::string_view a = ret;
    std::string_view b = cur_path;
    if (!a.empty() && a.front() == '/')
        a.remove_prefix(1);
    if (!b.empty() && b.front() == '/')
        b.remove_prefix(1);
    return !a.empty() && str_casecmp(a, b);
}

chain_act classify_step(const std::string &ret, bool is_pre, const std::string &cur_path)
{
    if (is_pre)
    {
        if (ret.empty())
            return chain_act::stop;
        if (ret.size() == 2 && str_casecmp(ret, "ok"))
            return chain_act::run_regfun;
        if (same_route_name(ret, cur_path))
            return chain_act::run_regfun;
        if (ret.size() == 4 && str_casecmp(ret, "exit"))
            return chain_act::stop;
        return chain_act::jump;
    }
    if (ret.empty())
        return chain_act::stop;
    if (ret.size() == 4 && str_casecmp(ret, "exit"))
        return chain_act::stop;
    if (ret.size() == 1 && ret[0] == 'T')
        return chain_act::run_timeloop;
    return chain_act::jump;
}

void fail_500(std::shared_ptr<httppeer> &peer, const std::string &reason)
{
    DEBUG_LOG("sync handler exception: %s", reason.c_str());
    peer->status(500);
    peer->output = "Internal Server Error <hr />";
    // 5XX 把原因直接显示给调用者：固定前缀 + 异常文本
    peer->output.append(reason);
}

void fail_503(std::shared_ptr<httppeer> &peer)
{
    DEBUG_LOG("sync handler rejected by thread pool");
    peer->status(503);
    peer->output = "server busy";
}

int checked_idx(unsigned int idx)
{
    if (idx >= _handlers.size())
        return -1;
    return static_cast<int>(idx);
}

}// namespace

// — 兜底 walk 的前缀表：由短到长，最多 6 段；超过 6 段的深路径靠 lookup_exact 原样命中，
//    更深的前缀参数请改用 query（?aa=222&bb=444） —
// 6 是硬上限而不是约定：段数不钳死的话，前缀表长度和 find 次数都跟着 URL 段数走，
// 一条几百段的 URL 就能把这点开销按连接数放大 —
// 空 pathinfos 什么前缀都不给：站点首页的注册名 "home" 归 resolve_idx 的尾巴查（见 fallback_404_home）
// 所有前缀统一带前导 '/'，和注册 key 对齐 — all prefixed with '/' to match registration keys
void router::build_prefixes(std::vector<std::string> const &pathinfos, std::vector<std::string> &prefixes)
{
    prefixes.clear();
    // 空段表（urlpath 就是 "/"）必须在这里退出：下面是 prefixes.resize(0) 之后直接写
    // prefixes[0]，空 vector 的 data() 是 nullptr，写进去就是一次空指针赋值。
    // 请求 "/" 且 wwwpath 目录不在盘上（或整站没配静态根）时 get_fileinfo() 返回 0，
    // resolve_idx 会带着空段表走到这里 —— 早退之后前缀表为空，walk 自然无命中，
    // 由 resolve_idx 的尾巴 fallback_404_home 接着查注册的 "/home"
    if (pathinfos.empty())
        return;
    unsigned int count = pathinfos.size() < 6 ? (unsigned int)pathinfos.size() : 6;
    prefixes.resize(count);
    prefixes[0] = "/";
    prefixes[0].append(pathinfos[0]);
    for (unsigned int i = 1; i < count; ++i)
    {
        prefixes[i] = prefixes[i - 1];
        prefixes[i].push_back('/');
        prefixes[i].append(pathinfos[i]);
    }
}

unsigned int router::host_slot(unsigned int host_index)
{
    serverconfig const &cfg = getserversysconfig();
    if (host_index >= cfg.sitehostinfos.size())
        return 0;
    unsigned int slot = cfg.sitehostinfos[host_index].slot_id;
    if (slot >= _slots.size())
        return 0;
    return slot;
}

unsigned int router::peer_slot(std::shared_ptr<httppeer> &peer)
{
    return host_slot(peer->host_index);
}

// — 在一个 slot 里原样查一个 key；命中回填注册名 —
int router::find_in_slot(unsigned int slot, const std::string &key, std::string &matched_path)
{
    if (slot >= _slots.size())
        return -1;
    auto it = _slots[slot].path_map.find(key);
    if (it == _slots[slot].path_map.end())
        return -1;
    matched_path = key;
    return checked_idx(it->second);
}

// — 注册名查找：本站 slot → 全局 slot 0 各一次原样查，都没中且名字不带开头的 '/' 时补一个 '/'
//    再走一遍同样的两级；matched_name 回填真正命中的键名 —
// 注册表的键一律带 '/'（reg_add 写入时规范化，注解口和手写口同一个规则），
// 而 conf 里的钩子名、业务 push_flow
// 和 handler 返回的跳转名允许两种写法 ⇒ 只有这一侧收两种写法。
// URL 那三个查表口（lookup_exact / lookup_registered / lookup_key_in_slots）不走这里，
// 免得给请求路径补斜杠改变精确匹配语义
int router::find_name_in_slots(unsigned int slot, const std::string &name, std::string &matched_name)
{
    if (name.empty())
        return -1;

    int idx = find_in_slot(slot, name, matched_name);
    if (idx < 0 && slot != 0)
        idx = find_in_slot(0, name, matched_name);
    if (idx >= 0 || name[0] == '/')
        return idx;

    std::string slashed = "/" + name;
    idx                 = find_in_slot(slot, slashed, matched_name);
    if (idx < 0 && slot != 0)
        idx = find_in_slot(0, slashed, matched_name);
    if (idx >= 0)
    {
        DEBUG_LOG("route name matched with a prepended slash:%s", name.c_str());
    }
    return idx;
}

// — 整条 URL 原样注册的精确匹配：一次拼接，本站 slot → 全局 slot 0 各一次 find —
// 每请求第一道判定：直接用 peer->urlpath（解析层已组装成 `/seg1/seg2` 格式），
// 空 urlpath（"/"）也照原样查 —— 站点首页的注册名 "/home" 挪到链尾巴 fallback_404_home 查，
// 这样静态 index.html 和 home 注册的先后仍由 loop 的磁盘流程决定
int router::lookup_exact(std::shared_ptr<httppeer> &peer, std::string &matched_path)
{
    std::string const &key = peer->urlpath;

    unsigned int slot = peer_slot(peer);
    int idx           = find_in_slot(slot, key, matched_path);
    if (idx < 0 && slot != 0)
        idx = find_in_slot(0, key, matched_path);
    return idx;
}

// — 404 兜底位的一条前缀 walk：由短到长（先 user，再 user/info），最深 6 段；
//    外层是 slot（本站 → slot 0），所以本站的长前缀压得过全局的短前缀；表里没有就返回 -1，不做任何回落 —
// fill_args=false 时纯只读；true 时把命中前缀之后的剩余段按注册时的 urlpath 名字表填进 peer->get
int router::lookup_registered(std::shared_ptr<httppeer> &peer, std::string &matched_path, bool fill_args)
{
    std::vector<std::string> prefixes;
    build_prefixes(peer->pathinfos, prefixes);
    matched_path.clear();

    unsigned int slot = peer_slot(peer);
    int matched_plen  = 0;

    // 一个 slot 内由短到长；matched_plen 是命中前缀的下标，参数段从这一段往后填
    auto walk_one_slot = [&](unsigned int s) -> int
    {
        for (int plen = 0; plen < (int)prefixes.size(); ++plen)
        {
            int hit = find_in_slot(s, prefixes[plen], matched_path);
            if (hit >= 0)
            {
                matched_plen = plen;
                return hit;
            }
        }
        return -1;
    };

    int idx = walk_one_slot(slot);
    if (idx < 0 && slot != 0)
        idx = walk_one_slot(0);
    if (idx < 0)
        return -1;

    if (fill_args)
    {
        auto up = _urlpath_map.find((unsigned int)idx);
        if (up != _urlpath_map.end())
        {
            auto const &names = up->second;
            for (unsigned int m = (unsigned int)matched_plen; m < peer->pathinfos.size(); ++m)
            {
                if (m < names.size() && !names[m].empty())
                {
                    peer->get[names[m]] = peer->pathinfos[m];
                }
            }
        }
    }
    return idx;
}

// — 链的尾巴：空 pathinfos（urlpath 就是 "/"）查站点首页的注册名 "/home"，其余查注册的 "/404"，
//    先本站 slot 再全局 slot 0，都没有返回 -1 由调用方出框架 404 —
// 表里的键一律带 '/'（reg_add 写入时规范化），所以这里直接用带斜杠的写法，第一轮 find 就命中
int router::fallback_404_home(std::shared_ptr<httppeer> &peer, std::string &matched_path)
{
    std::string const &key = peer->pathinfos.empty() ? std::string("/home") : std::string("/404");
    return find_name_in_slots(peer_slot(peer), key, matched_path);
}

// — 注册名精确探测：本站 slot → 全局 slot 0，命中返回 handler 下标，没命中返回 -1 —
// 走 find_in_slot 所以顺带吃到 checked_idx 的越界门禁（表里有条目、_handlers 里没有对应下标时算没命中）；
// 不建前缀表、不回落：前缀参数路由（user/info/:userid）和空 pathinfos 的 "home" 都归 resolve_idx 那条链 —
// 命中的注册名就是传入的 key，调用方（httppeer::prefetch_routing）已把 urlpath 原地换成它，
// 所以这里不必再带出一个 matched 名
int router::lookup_key_in_slots(const std::string &key, unsigned int host_index)
{
    if (key.empty())
        return -1;

    unsigned int slot = host_slot(host_index);
    std::string matched;
    int idx = find_in_slot(slot, key, matched);
    if (idx < 0 && slot != 0)
        idx = find_in_slot(0, key, matched);
    return idx;
}

// — 路由查找：先原样精确匹配，未命中才走 404 兜底位的前缀 walk，最后才是链尾巴的 home/404 —
// known_idx >= 0 表示 loop 的路由预查已经命中过同一条精确注册（prefetch_routing 把下标和注册名
// 一起带回来了，注册名就是 peer->urlpath），这里直接进链，不再把同一个 key 查第二遍；
// 默认 -1 给没有预查的调用点（rpc 通路、同步链）走完整查找
int router::resolve_idx(std::shared_ptr<httppeer> &peer, std::string &matched_path, int known_idx, bool exact_checked)
{
    if (known_idx >= 0)
    {
        matched_path = peer->urlpath;
        return known_idx;
    }

    // exact_checked=true：loop 的 prefetch_routing 已经对当前 urlpath 做过一次完全相同的原样精确查表
    // （lookup_key_in_slots，本站 slot → 全局 slot 0 各一次 find）并确认未命中，这里不必再付一次 hash 查找。
    // 注意 urlpath 在预查剥扩展名分支若未命中会被原地还原成原值，所以和本处的 lookup_exact(urlpath) 一致。
    int idx = exact_checked ? -1 : lookup_exact(peer, matched_path);
    // — walk 只站在兜底位：本站有静态根、且磁盘上这条 URL 是目录时不抢静态页，直接进 404 链 —
    if (idx < 0 && (peer->sitepath.empty() || peer->sendfiletype != 2))
    {
        idx = lookup_registered(peer, matched_path, true);
    }
    if (idx < 0)
        idx = fallback_404_home(peer, matched_path);
    return idx;
}

// — 跳转目标 / 钩子名查找：本 domain slot → slot 0，带不带开头的 '/' 都认 —
int router::find_sitecontent(std::shared_ptr<httppeer> &peer, const std::string &sitecontent)
{
    if (sitecontent.empty() || _slots.empty())
        return -1;
    std::string matched;
    return find_name_in_slots(peer_slot(peer), sitecontent, matched);
}

// — 编排栈排空：业务在 handler 里 push_flow("名字")，等被依赖的那条跑完（返回空串）后由这里弹出执行 —
// 弹出取栈尾（pop_flow 用 back），所以后压的先执行；一个 handler 执行完只弹一个，
// 剩下的等它自己那次「返回空串」再弹，直到栈空或整链触顶。
// is_coro_driver=false（同步链）时，弹到协程条目就停下：请求已经跑到链尾、正文可能已写出，
// 把它转成 404 响应比跳过这段编排更糟
int router::pop_flow_index(std::shared_ptr<httppeer> &peer, std::string &cur_path, bool is_coro_driver)
{
    if (!peer->flow_method || peer->flow_method->empty())
        return -1;

    std::string name = peer->pop_flow();
    std::string matched;
    int idx = find_name_in_slots(peer_slot(peer), name, matched);
    if (idx < 0)
    {
        DEBUG_LOG("flow stack name not registered:%s", name.c_str());
        return -1;
    }
    if (!is_coro_driver && _handlers[idx].reg_kind == fn_kind::coro)
    {
        DEBUG_LOG("flow stack pops a coro route on sync chain, stop drain:%s", name.c_str());
        return -1;
    }
    cur_path = matched;
    return idx;
}

// — 只跑一个注册名的 sync regfun（static_pre 防盗链、站点钩子共用入口）—
// 协程条目在这条通路上没人能等它：调用点在 ThreadPool 上、没有协程帧，返回空串等于让
// 防盗链静默失效，所以这里给一个非空哨兵，让「绑错类型」显式地按拒绝处理
std::string router::call_sync_regfun(std::shared_ptr<httppeer> &peer, const std::string &name)
{
    if (name.empty())
        return "";
    int idx = find_sitecontent(peer, name);
    if (idx < 0)
        return "";

    auto const &h = _handlers[idx];
    if (h.reg_kind == fn_kind::sync && h.regfun)
        return h.regfun(peer);
    if (h.reg_kind == fn_kind::coro)
    {
        DEBUG_LOG("sync regfun entry is a coro route, refuse:%s", name.c_str());
        return "coro-refused";
    }
    return "";
}

// — 协程调用点用的注册名入口（static_pre 防盗链）：sync 步交给业务线程池，coro 步就地 co_await —
// 调用方协程会挂着等这个任务跑完，所以 peer 在这个任务期间由它独占，和 co_run_site_hooks 同一套规矩。
// 未注册返回空串（放行）；池没接单或业务抛出都回非空哨兵——静态文件防盗链在"拿不到结论"时
// 只能是拒绝，不能放行。
asio::awaitable<std::string> router::co_call_regfun(std::shared_ptr<httppeer> &peer, const std::string &name)
{
    if (name.empty())
        co_return "";
    int idx = find_sitecontent(peer, name);
    if (idx < 0)
        co_return "";

    auto const &h = _handlers[idx];
    if (h.reg_kind == fn_kind::sync && h.regfun)
    {
        auto step = co_await co_pool_run_step(
            std::function<std::string()>([fn = h.regfun, peer]()
                                         { return fn(peer); }),
            asio::use_awaitable);
        if (step.eptr)
        {
            DEBUG_LOG("sync regfun threw on pool, refuse:%s %s", name.c_str(), exception_text(step.eptr).c_str());
            co_return "pool-error";
        }
        if (step.rejected)
        {
            DEBUG_LOG("sync regfun refused by pool, refuse:%s", name.c_str());
            co_return "pool-rejected";
        }
        co_return step.ret;
    }
    if (h.reg_kind == fn_kind::coro && h.co_regfun)
    {
        DEBUG_LOG("regfun entry is a coro route, await it:%s", name.c_str());
        std::string ret;
        try
        {
            ret = co_await h.co_regfun(peer);
        }
        catch (...)
        {
            // 同步那一支的抛出由池的记账带回来，这一支没人包，不接住就一路抛回请求协程：
            // 连接被拆掉、一个字节都没发，比 403 更难查
            DEBUG_LOG("coro regfun threw, refuse:%s %s", name.c_str(), exception_text(std::current_exception()).c_str());
            co_return "coro-error";
        }
        co_return ret;
    }
    co_return "";
}

// — 站点钩子：只跑 regfun，不跑钩子自身的 pre；前置钩子返回 "exit" 终止整链 —
bool router::run_site_hooks(std::shared_ptr<httppeer> &peer,
                            std::vector<std::string> const &lists,
                            bool is_pre)
{
    if (lists.empty() || lists.size() > site_hook_limit)
        return false;
    for (auto const &name : lists)
    {
        std::string ret = call_sync_regfun(peer, name);
        if (is_pre && ret.size() == 4 && str_casecmp(ret, "exit"))
            return true;
    }
    return false;
}

asio::awaitable<bool> router::co_run_site_hooks(std::shared_ptr<httppeer> &peer,
                                                std::vector<std::string> const &lists,
                                                bool is_pre)
{
    if (lists.empty() || lists.size() > site_hook_limit)
        co_return false;

    for (auto const &name : lists)
    {
        int idx = find_sitecontent(peer, name);
        if (idx < 0)
            continue;

        auto const &h = _handlers[idx];
        std::string ret;
        if (h.reg_kind == fn_kind::sync && h.regfun)
        {
            auto step = co_await co_pool_run_step(
                std::function<std::string()>([fn = h.regfun, peer]()
                                             { return fn(peer); }),
                asio::use_awaitable);
            if (step.eptr)
            {
                fail_500(peer, exception_text(step.eptr));
                co_return true;
            }
            if (step.rejected)
            {
                fail_503(peer);
                co_return true;
            }
            ret = step.ret;
        }
        else if (h.reg_kind == fn_kind::coro && h.co_regfun)
        {
            ret = co_await h.co_regfun(peer);
        }
        if (is_pre && ret.size() == 4 && str_casecmp(ret, "exit"))
            co_return true;
    }
    co_return false;
}

// — 同步整链（原先由 ThreadPool::http_clientrun 调用，该调用点已 #if 0，见 threadpool.cpp 里停用的 http_clientrun 段）—
// 当前无调用者、只作备查；重新启用前须先与 co_resolve() 对齐，分叉点见 router.h 里 resolve() 的声明注释
std::string router::resolve(std::shared_ptr<httppeer> peer)
{
    serverconfig const &cfg      = getserversysconfig();
    site_host_info_t const *site = nullptr;
    if (peer->host_index < cfg.sitehostinfos.size())
    {
        site = &cfg.sitehostinfos[peer->host_index];
    }

    if (site != nullptr && site->is_method_pre)
    {
        if (run_site_hooks(peer, site->action_pre_lists, true))
            return "";
    }

    std::string cur_path;
    int idx = resolve_idx(peer, cur_path);
    if (idx < 0)
    {
        make_404_content(peer);
        return "";
    }

    std::string sitecontent;
    unsigned int safety_counter = 0;
    while (safety_counter++ < 32)
    {
        auto const &h = _handlers[idx];

        if (h.pre_kind == fn_kind::sync && h.pre)
        {
            sitecontent   = h.pre(peer);
            chain_act act = classify_step(sitecontent, true, cur_path);
            if (act == chain_act::stop)
            {
                sitecontent.clear();
                break;
            }
            if (act == chain_act::jump)
            {
                int next = find_sitecontent(peer, sitecontent);
                if (next < 0)
                    break;// 跳转名没注册：交回调用方并记日志
                cur_path = sitecontent;
                sitecontent.clear();
                idx = next;
                continue;
            }
        }
        else if (h.pre_kind == fn_kind::coro && h.co_pre)
        {
            // 同步链等不了协程 pre，跳过它等于让鉴权没跑就执行 handler；
            // 按「本条在同步表里不存在」处置，和下面协程 handler 的处理方式一致
            DEBUG_LOG("sync dispatch cannot await coro pre, refuse:%s", cur_path.c_str());
            make_404_content(peer);
            sitecontent.clear();
            break;
        }

        if (h.reg_kind == fn_kind::sync && h.regfun)
        {
            sitecontent = h.regfun(peer);
        }
        else if (h.reg_kind == fn_kind::coro)
        {
            // 同步入口跑不了协程 handler，按「该路由没有注册」处置（记日志 + 404）
            DEBUG_LOG("sync dispatch on coro route:%s", cur_path.c_str());
            make_404_content(peer);
            sitecontent.clear();
            break;
        }
        else
        {
            sitecontent.clear();
            break;
        }

        chain_act act = classify_step(sitecontent, false, cur_path);
        if (act == chain_act::stop)
        {
            // 空串=主 handler 正常完成，业务压栈的编排从这里接着弹；"exit" 是显式终止，不弹
            if (sitecontent.empty())
            {
                int next = pop_flow_index(peer, cur_path, false);
                if (next >= 0)
                {
                    sitecontent.clear();
                    idx = next;
                    continue;
                }
            }
            sitecontent.clear();
            break;
        }
        if (act == chain_act::run_timeloop)
        {
            sitecontent.clear();
            call_sync_regfun(peer, "frametasks_timeloop");
            break;
        }
        int next = find_sitecontent(peer, sitecontent);
        if (next < 0)
            break;// sitecontent 作为未消费的跳转名交回调用方
        cur_path = sitecontent;
        sitecontent.clear();
        idx = next;
    }
    // 触顶和 break 用同一个计数器区分：出循环时还等于 32 是主动 break，33 才是把步数用完
    if (safety_counter > 32)
    {
        DEBUG_LOG("route chain step limit reached:%s", cur_path.c_str());
    }

    if (site != nullptr && site->is_method_after)
    {
        run_site_hooks(peer, site->action_after_lists, false);
    }
    return sitecontent;
}

// — 协程整链（http1loop / http2loop / rpc）：sync 步经 ThreadPool，coro 步就地 co_await —
// known_idx >= 0 是 loop 的路由预查（httppeer::prefetch_routing）带回来的 handler 下标，
// 链直接从那条注册开始跑；rpc 通路没有预查，用默认 -1 走完整查找
asio::awaitable<std::string> router::co_resolve(std::shared_ptr<httppeer> peer, int known_idx)
{
    serverconfig const &cfg      = getserversysconfig();
    site_host_info_t const *site = nullptr;
    if (peer->host_index < cfg.sitehostinfos.size())
    {
        site = &cfg.sitehostinfos[peer->host_index];
    }

    if (site != nullptr && site->is_method_pre)
    {
        if (co_await co_run_site_hooks(peer, site->action_pre_lists, true))
            co_return "";
    }

    // 消费预查留下的「精确表已查且未命中」标志：两个 http loop 都先跑过 prefetch_routing 再进这里，
    // 标志是可靠的；rpc / 其它没跑预查的入口标志为 false（或上一次已被复位），仍走完整查找。
    // 复位必须在这里做一次，否则同一条 peer 复用（keep-alive、同连接切协议）会把旧标志带到下一请求。
    // 前提是这段窗口里没人改写 urlpath：站点 pre 钩子和 static_pre 钩子都不许改，改了这里跳过的就是
    // 新串该做的那次精确查表。今天改 urlpath 的只有 handler，它们跑在 resolve_idx 之后。
    bool exact_checked        = peer->route_exact_checked;
    peer->route_exact_checked = false;

    std::string cur_path;
    int idx = resolve_idx(peer, cur_path, known_idx, exact_checked);
    if (idx < 0)
    {
        make_404_content(peer);
        co_return "";
    }

    std::string sitecontent;
    unsigned int safety_counter = 0;
    while (safety_counter++ < 32)
    {
        auto const &h = _handlers[idx];

        bool has_pre = false;
        if (h.pre_kind == fn_kind::sync && h.pre)
        {
            has_pre   = true;
            auto step = co_await co_pool_run_step(
                std::function<std::string()>([fn = h.pre, peer]()
                                             { return fn(peer); }),
                asio::use_awaitable);
            if (step.eptr)
            {
                fail_500(peer, exception_text(step.eptr));
                co_return "";
            }
            if (step.rejected)
            {
                fail_503(peer);
                co_return "";
            }
            sitecontent = step.ret;
        }
        else if (h.pre_kind == fn_kind::coro && h.co_pre)
        {
            has_pre = true;
            try
            {
                sitecontent = co_await h.co_pre(peer);
            }
            catch (const std::exception &e)
            {
                fail_500(peer, e.what());
                co_return "";
            }
            catch (...)
            {
                fail_500(peer, "unknown exception");
                co_return "";
            }
        }

        if (has_pre)
        {
            chain_act act = classify_step(sitecontent, true, cur_path);
            if (act == chain_act::stop)
            {
                sitecontent.clear();
                break;
            }
            if (act == chain_act::jump)
            {
                int next = find_sitecontent(peer, sitecontent);
                if (next < 0)
                    break;// 跳转名没注册：交回调用方并记日志
                cur_path = sitecontent;
                sitecontent.clear();
                idx = next;
                continue;
            }
        }

        bool has_reg = false;
        if (h.reg_kind == fn_kind::sync && h.regfun)
        {
            has_reg   = true;
            auto step = co_await co_pool_run_step(
                std::function<std::string()>([fn = h.regfun, peer]()
                                             { return fn(peer); }),
                asio::use_awaitable);
            if (step.eptr)
            {
                fail_500(peer, exception_text(step.eptr));
                co_return "";
            }
            if (step.rejected)
            {
                fail_503(peer);
                co_return "";
            }
            sitecontent = step.ret;
        }
        else if (h.reg_kind == fn_kind::coro && h.co_regfun)
        {
            has_reg = true;
            try
            {
                sitecontent = co_await h.co_regfun(peer);
            }
            catch (const std::exception &e)
            {
                fail_500(peer, e.what());
                co_return "";
            }
            catch (...)
            {
                fail_500(peer, "unknown exception");
                co_return "";
            }
        }
        if (!has_reg)
        {
            sitecontent.clear();
            break;
        }

        chain_act act = classify_step(sitecontent, false, cur_path);
        if (act == chain_act::stop)
        {
            // 与同步链同形：空串=正常完成才弹编排栈，"exit" 不弹
            if (sitecontent.empty())
            {
                int next = pop_flow_index(peer, cur_path, true);
                if (next >= 0)
                {
                    sitecontent.clear();
                    idx = next;
                    continue;
                }
            }
            sitecontent.clear();
            break;
        }
        if (act == chain_act::run_timeloop)
        {
            sitecontent.clear();
            int loop_idx = find_sitecontent(peer, "frametasks_timeloop");
            if (loop_idx < 0)
            {
                DEBUG_LOG("frametasks_timeloop not registered");
                break;
            }
            auto const &loop_h = _handlers[loop_idx];
            if (loop_h.reg_kind == fn_kind::sync && loop_h.regfun)
            {
                auto step = co_await co_pool_run_step(
                    std::function<std::string()>([fn = loop_h.regfun, peer]()
                                                 { return fn(peer); }),
                    asio::use_awaitable);
                if (step.eptr)
                    fail_500(peer, exception_text(step.eptr));
                else if (step.rejected)
                    fail_503(peer);
            }
            break;
        }
        int next = find_sitecontent(peer, sitecontent);
        if (next < 0)
            break;
        cur_path = sitecontent;
        sitecontent.clear();
        idx = next;
    }

    // 触顶和 break 用同一个计数器区分：出循环时还等于 32 是主动 break，33 才是把步数用完
    if (safety_counter > 32)
    {
        DEBUG_LOG("route chain step limit reached:%s", cur_path.c_str());
    }

    if (site != nullptr && site->is_method_after)
    {
        co_await co_run_site_hooks(peer, site->action_after_lists, false);
    }
    if (!sitecontent.empty())
    {
        DEBUG_LOG("route chain left unconsumed:%s", sitecontent.c_str());
    }
    co_return sitecontent;
}

// — 间隔任务：按任务名精确执行 sync regfun，不跑 pre、不回落 404/home —
void router::run_timeloop_task(std::shared_ptr<httppeer> &peer, const std::string &taskname)
{
    int idx = find_sitecontent(peer, taskname);
    if (idx < 0)
    {
        DEBUG_LOG("timeloop task not registered:%s", taskname.c_str());
        return;
    }
    auto const &h = _handlers[idx];
    if (h.reg_kind == fn_kind::sync && h.regfun)
    {
        (void)h.regfun(peer);
        return;
    }
    DEBUG_LOG("timeloop task is not a sync handler:%s", taskname.c_str());
}

// — 站点钩子存在性校验：conf 解析期注册表还是空的，只能在这里补 —
void router::validate_site_hooks()
{
    serverconfig &cfg = getserversysconfig();

    for (size_t i = 0; i < cfg.sitehostinfos.size(); ++i)
    {
        site_host_info_t &site = cfg.sitehostinfos[i];

        // 和请求期同一个查法（find_name_in_slots：两级 slot + 带不带 '/' 两种写法 + checked_idx 界值），
        // 免得启动期说"注册了"而请求期查不到 ⇒ static_pre 变成静默放行
        auto registered = [slot = site.slot_id](const std::string &name) -> bool
        {
            if (name.empty())
                return false;
            std::string matched;
            return find_name_in_slots(slot, name, matched) >= 0;
        };

        auto prune = [&registered](std::vector<std::string> &lists)
        {
            std::vector<std::string> kept;
            kept.reserve(lists.size());
            for (auto const &name : lists)
            {
                if (registered(name))
                    kept.push_back(name);
                else
                    DEBUG_LOG("site hook not registered, dropped:%s", name.c_str());
            }
            lists.swap(kept);
        };

        // static_pre_lists 是 URL 前缀名单，不是注册名，只有 static_pre_method 需要校验
        site.is_static_pre = !site.static_pre_method.empty() && registered(site.static_pre_method);
        if (!site.is_static_pre && !site.static_pre_method.empty())
        {
            DEBUG_LOG("static_pre method not registered:%s", site.static_pre_method.c_str());
        }

        prune(site.action_pre_lists);
        site.is_method_pre = !site.action_pre_lists.empty();

        prune(site.action_after_lists);
        site.is_method_after = !site.action_after_lists.empty();

        // 越界时 run_site_hooks / co_run_site_hooks 整表不跑，method_pre 里的鉴权会静默失效；
        // 这是请求期行为，启动期是唯一能把话说清楚的时机
        if (site.action_pre_lists.size() > site_hook_limit)
        {
            fprintf(stderr, "[ROUTE-WARN] site %s method_pre has %zu hooks, over %zu, none of them run\n", site.mainhost.c_str(), site.action_pre_lists.size(), site_hook_limit);
        }
        if (site.action_after_lists.size() > site_hook_limit)
        {
            fprintf(stderr, "[ROUTE-WARN] site %s method_after has %zu hooks, over %zu, none of them run\n", site.mainhost.c_str(), site.action_after_lists.size(), site_hook_limit);
        }
    }
}

// — 路由表导出：把一条 URL 对回「哪一行注册、绑哪个函数」—
namespace
{
const char *kind_text(fn_kind k)
{
    if (k == fn_kind::sync)
        return "sync";
    if (k == fn_kind::coro)
        return "coro";
    return "none";
}
}// namespace

std::string router::routes_text(const std::string &filter)
{
    std::vector<std::string> slotname(_slots.size());
    for (auto const &kv : _site_to_slot)
    {
        if (kv.second < slotname.size())
            slotname[kv.second] = kv.first;
    }

    std::ostringstream oss;
    unsigned long long shown = 0;
    unsigned long long total = 0;
    for (unsigned int s = 0; s < _slots.size(); ++s)
    {
        std::string sname = (s == 0 ? "global" : (slotname[s].empty() ? "?" : slotname[s]));
        for (auto const &item : _slots[s].path_map)
        {
            ++total;
            if (!filter.empty() && item.first != filter)
                continue;
            ++shown;

            oss << "slot=" << s << "(" << sname << ")  path=" << item.first
                << "  idx=" << item.second;
            int idx = checked_idx(item.second);
            if (idx < 0)
            {
                oss << "  handler=OUT-OF-RANGE\n";
                continue;
            }
            auto const &h = _handlers[idx];
            oss << "  pre=" << kind_text(h.pre_kind) << "  reg=" << kind_text(h.reg_kind);

            auto up = _urlpath_map.find((unsigned int)idx);
            if (up != _urlpath_map.end())
            {
                bool first = true;
                for (auto const &nm : up->second)
                {
                    if (nm.empty())
                        continue;
                    if (first)
                    {
                        oss << "  urlpath=";
                        first = false;
                    }
                    else
                    {
                        oss << ",";
                    }
                    oss << nm;
                }
            }
            oss << "  reg@ " << h.reg_file << ":" << h.reg_line << "  [" << h.reg_fn << "]\n";
        }
    }
    oss << "shown " << shown << " total " << total << " handlers " << _handlers.size()
        << " slots " << _slots.size() << " dropped " << _route_reg_dropped
        << " dup " << _route_reg_dup << "\n";
    return oss.str();
}

void force_pool_reject_once()
{
    get_server_app().clientrunpool.force_reject_sync_once();
}

void force_pool_reject_conn_once()
{
    get_server_app().clientrunpool.force_reject_conn_once();
}

}// namespace http
