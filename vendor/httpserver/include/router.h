#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <asio/awaitable.hpp>

#include "httppeer.h"

namespace http
{

class router
{
  public:
    // — 路由查找：先精确匹配，未命中才走 404 兜底位的前缀 walk，最后查尾巴的 home/404；
    //    命中返回 handler idx，并回填命中的注册名；都没有返回 -1 —
    // known_idx >= 0 是 loop 预查（httppeer::prefetch_routing）已经命中的那条注册，直接进链不再重查；
    // exact_checked=true 表示预查已对当前 urlpath 做过原样精确查表并确认未命中，跳过重复的 lookup_exact
    // （rpc / 同步链不跑预查，co_resolve 会把它复位成 false 走完整查找）
    int resolve_idx(std::shared_ptr<httppeer> &peer, std::string &matched_path, int known_idx = -1, bool exact_checked = false);

    // — 注册名精确探测：返回 handler 下标（>=0 命中），没命中返回 -1 —
    // 纯查表，一次 map find，不建前缀表、不回落；命中的注册名就是传入的 key
    // host_index 对应的 slot 就是 alias_domain（conf 里写了 alias_domain 用它，没写用 mainhost）
    //    分组出来的本站 slot —
    int lookup_key_in_slots(const std::string &key, unsigned int host_index);

    // — 跳转目标 / 钩子名查找：本 domain slot → slot 0，找不到返回 -1 —
    int find_sitecontent(std::shared_ptr<httppeer> &peer, const std::string &sitecontent);

    std::vector<reg_methold_mid_t> const &handlers() const { return _handlers; }

    // — 路由表导出：filter 非空时只留精确同名注册名 —
    // 每条打印 slot/path/idx、pre 与 reg 的种类、urlpath 参数名，以及注册点 file:line，
    // 用来把一条 URL 直接对回「哪一行注册、绑哪个函数」。
    std::string routes_text(const std::string &filter);

    // — 只跑一个注册名的 sync regfun（同步链的站点钩子、timeloop 用）—
    // 未注册返回空串（调用方按放行处理）；注册的是协程条目时返回非空哨兵 "coro-refused"，
    // 因为这条通路在 ThreadPool 上、没有协程帧，没人能等它，返回空串等于让鉴权静默失效
    std::string call_sync_regfun(std::shared_ptr<httppeer> &peer, const std::string &name);

    // — 同上，但调用方是协程（static_pre 防盗链）：sync 条目交给业务线程池，coro 条目就地 co_await —
    // 未注册同样返回空串（放行）；池拒收或业务抛出返回非空哨兵，让调用方按拒绝处理（fail-closed）
    asio::awaitable<std::string> co_call_regfun(std::shared_ptr<httppeer> &peer, const std::string &name);

    // — 同步整链：只跑 fn_kind::sync，站点钩子照常执行 —
    // 返回值：空 = 结束（输出在 peer->output）；非空 = 链跳到了本表没有的名字
    // 协程 pre / 协程 handler 在这条链上都按「本条不存在」处理（记日志 + 404），不会跳过 pre 直接跑 handler
    //
    // ⚠ 当前无调用者：唯一的调用点 ThreadPool::http_clientrun() 已连同整段被 #if 0 停用
    // （threadpool.cpp 里那段 #if 0），这条链现在只是备查，保留是为了将来可能重新接线。
    // 重新启用前必须先与 co_resolve() 对齐以下三处分叉，否则同一条路由在两条链上表现不一致：
    //   1. 协程 pre：这里记日志 + make_404() 直接回 404（本函数 pre_kind==coro 分支），co_resolve 是 co_await 后放行；
    //   2. 协程 handler：这里同样回 404（本函数 reg_kind==coro 分支），co_resolve 是就地 co_await；
    //   3. 编排栈排空：这里 pop_flow_index(..., is_coro_driver=false) 弹到协程条目就停止排空
    //      （见 pop_flow_index 的说明），co_resolve 会把它切进协程 driver 继续跑。
    std::string resolve(std::shared_ptr<httppeer> peer);

    // — 协程整链：sync 步经 ThreadPool 执行，coro 步就地 co_await —
    // 返回值语义与 resolve() 相同，但三个调用点（http1loop / http2loop / rpc）都不消费它：
    // 链跳到表外名字时只在本文件记 DEBUG_LOG，正文仍按已写出的 output 收尾
    // 两个 http loop 把预查命中的 idx 传进来，rpc 通路没有预查、用默认值走完整查找
    asio::awaitable<std::string> co_resolve(std::shared_ptr<httppeer> peer, int known_idx = -1);

    // — 间隔任务：按任务名精确执行 sync regfun，不跑 pre、不回落 404/home —
    void run_timeloop_task(std::shared_ptr<httppeer> &peer, const std::string &taskname);

    // — conf 解析期只登记钩子名，注册全部完成之后再校验存在性，并核对条数是否越过钩子执行上限 —
    void validate_site_hooks();

  private:
    // 兜底 walk 的前缀表：由短到长（user、user/info、user/info/7…），最多 6 段
    static void build_prefixes(std::vector<std::string> const &pathinfos, std::vector<std::string> &prefixes);
    // host_index → 本站 slot（越界或 slot 不存在都回落全局 slot 0）
    static unsigned int host_slot(unsigned int host_index);
    static unsigned int peer_slot(std::shared_ptr<httppeer> &peer);

    // 在一个 slot 里原样查一个 key，命中就把注册名回填进 matched_path
    static int find_in_slot(unsigned int slot, const std::string &key, std::string &matched_path);

    // 注册名（钩子名、跳转名、home/404）查找：本站 slot → 全局 slot 0，先按原样查，
    // 未命中且名字不以 '/' 开头时补一个 '/' 再查一次；matched_name 回填真正命中的那个键名。
    // URL 面（lookup_exact / lookup_registered / lookup_key_in_slots）不走这里，保持严格原样匹配。
    static int find_name_in_slots(unsigned int slot, const std::string &name, std::string &matched_name);

    // 整条 URL 原样注册的精确匹配：直接用 peer->urlpath，本站 slot → 全局 slot 0 各一次 find。
    // 正常 URL 只付这一次代价，段数再多也不建中间前缀，所以深路径的原样注册不受 6 段上限影响
    static int lookup_exact(std::shared_ptr<httppeer> &peer, std::string &matched_path);

    // 404 兜底位的前缀 walk：由短到长，最深 6 段；外层是 slot（本站 → slot 0），
    // 所以本站的长前缀压得过全局的短前缀；fill_args=true 时把命中前缀之后的段按名字表填进 peer->get。
    // 段数不校验：少给的段留空，多给的段直接丢掉，依赖参数非空的 handler 得自己判空
    static int lookup_registered(std::shared_ptr<httppeer> &peer, std::string &matched_path, bool fill_args);
    // 链尾巴的回落：空 pathinfos 查注册名 "home"（站点首页），其余查注册的 "404"
    static int fallback_404_home(std::shared_ptr<httppeer> &peer, std::string &matched_path);

    // 编排栈排空：主 handler 正常完成（返回空串）后弹出一个注册名接着跑，直到栈空。
    // 栈空时只读一个 unique_ptr；弹出的名没注册时记日志返回 -1，不改状态码。
    // is_coro_driver=false 表示当前是同步链，弹到协程条目只记日志并结束排空
    int pop_flow_index(std::shared_ptr<httppeer> &peer, std::string &cur_path, bool is_coro_driver);

    // 站点钩子：逐个查 handler 只跑 regfun；返回 true 表示整链应当终止
    bool run_site_hooks(std::shared_ptr<httppeer> &peer, std::vector<std::string> const &lists, bool is_pre);
    asio::awaitable<bool> co_run_site_hooks(std::shared_ptr<httppeer> &peer,
                                            std::vector<std::string> const &lists,
                                            bool is_pre);
};

router &get_router();

// 测试钩子：让 clientrunpool 下次 add_sync_task 被拒绝（触发 fail_503）。
// 一次性消费，自动复位，不影响后续请求。
// 开关是进程级的：并发下有别的请求先到就会把它消费掉，只在串行用例里有效
void force_pool_reject_once();

// lane B（conn_tasks，ws 数据消息这类投完就走的任务）的同形钩子，另一把开关：
// 不复用上面那把，否则 HTTP 的 503 通路和 ws 的丢弃通路会互相消费。
// 同样是一次性、进程级，只在串行用例里有效。
void force_pool_reject_conn_once();

}// namespace http
#endif
