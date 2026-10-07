// =============================================================================
// 任务编排与协程/Sync 混合路由测试
// Orchestration flow & coro/sync mixed router test cases
// =============================================================================
//
// 【教程 / Tutorial】
//
// 每个请求进来后，router::co_resolve 按「pre → regfun → 编排栈」跑一条链。
// 链上每步可以是 sync 也可以是 coro，框架自动桥接：
//
//   sync  handler → co_pool_run_step → ThreadPool
//   coro handler → 直接 co_await
//
// 业务代码在 handler 里 push_flow("名字") 把后续要跑的路由压到编排栈，
// 等当前 handler return ""（正常结束）之后，框架从栈顶弹一个出来接着跑，
// 直到栈空。exit 返回值则立即终止，不弹栈。
//
//   push_flow(...)       → 压栈，后压的先出（LIFO）
//   push_front_flow(...) → 压栈底，先压的先出（FIFO）
//   return ""            → 正常结束，继续弹栈
//   return "exit"        → 显式终止，不弹栈
//   return "别的名字"     → 直接跳到那个注册名（不经过栈）
//
// 【什么时候用 / When to use】
//
// 1. 等某个异步条件就绪再跑主逻辑（先 push 自己，再跳条件检查；第二次进来
//    时条件已满足 → 这就是 fx/depa + fx/depb 的"依赖等待"模式）
//    Wait for an async condition before running main logic.
//
// 2. 一个主 handler 后接一串收尾步骤，不想写成超长同步代码，按职责拆开：
//    先跑 A，A 里压 B、C，框架自动按序弹出执行。
//    Decompose a long handler into focused steps, queued via push_flow.
//
// 3. pre 是 coro（要 co_await 外部鉴权接口）但 regfun 是 sync（老逻辑），
//    或者反过来——框架两种都能接，业务随便写。
//    Mix coro and sync freely; the bridge is automatic.
//
// 【安全 / Safety】
//
//   - 自引用 push_flow 会触发安全计数器（32 步截断），不会无限循环。
//   - push 不存在的路由名被静默忽略（不会改判 404）。
//   - 异常通路：sync throw → co_pool_run_step 内部 catch → 500；
//     coro throw → co_resolve 里 try/catch → 500。
//     ThreadPool 满时 add_sync_task 返回 false → 503。
//
// =============================================================================

#include <atomic>
#include <chrono>
#include <thread>

#include "httppeer.h"
#include "pool_step.h"
#include "server.h"
#include "testflowstack.h"
#include "router.h"

namespace http
{

// =============================================================================
// 基础编排（sync-only，演示 flow_method 栈行为）
// Basic orchestration in pure sync.
// =============================================================================

// @urlpath(null,fx/preonly)
// demo: pre-only handler, returns "ok" = proceed
// 演示：只有 pre 没有 regfun，pre 返回 "ok" 继续，返回 "" 拒绝
std::string testfxpreonly(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "PRESEEN;";// 标记 pre 被执行
    return "ok";         // 放行
}

// @urlpath(testfxpreonly,fx/guarded)
// demo: pre-only handler, returns "" = refuse / stop the chain
// 演示：pre 返回空串 = 拒绝主 handler，output 里能看到 preonly 被执行了但 guarded 没继续
std::string testfxguarded(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "G;";
    return "";
}

// @urlpath(null,fx/a)
// demo: multiple push_flow / push_front_flow patterns via query param m
// 演示：编排栈 LIFO / FIFO / 嵌套 各种压栈顺序
//   m=1 → 压 b，出 A;B;
//   m=2 → 先压 b 再压 c，栈顶是 c，出 A;C;B;（LIFO）
//   m=3 → push_flow(b) + push_front_flow(c)，c 在底 b 在上，出 A;B;C;（FIFO）
std::string testfxa(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string m    = client.get["m"].to_string();
    if (m == "1")
    {
        client.push_flow("fx/b");
    }
    else if (m == "2")
    {
        client.push_flow("fx/b");
        client.push_flow("fx/c");
    }
    else if (m == "3")
    {
        client.push_flow("fx/b");
        client.push_front_flow("fx/c");
    }
    else if (m == "4")
    {
        client.push_flow("fx/b");
    }
    else if (m == "5")
    {
        client.push_flow("fx/plain");
        client.push_flow("fx/guarded");
    }
    client << "A;";
    return "";// 正常 return 空串才会触发栈弹出
}

// @urlpath(null,fx/b)
// demo: handler can keep pushing after being popped — nested push
// 演示：自己被弹出来执行，里面还可以再压别的，形成嵌套编排
std::string testfxb(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "B;";
    if (client.get["pushd"].to_string() == "1")
    {
        client.push_flow("fx/d");// 自己压自己的下游
    }
    return "";
}

// @urlpath(null,fx/c)
// simple leaf handler, no push
// 演示：叶子节点，只输出
std::string testfxc(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "C;";
    return "";
}

// @urlpath(null,fx/d)
// simple leaf handler, no push
std::string testfxd(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "D;";
    return "";
}

// @urlpath(null,fx/plain)
// simple leaf handler, no push
std::string testfxplain(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "PLAIN;";
    return "";
}

// @urlpath(null,fx/loop)
// demo: self-referencing push — triggers safety counter (32 steps)
// 演示：自引用 push 不会无限循环，被安全计数器截断
std::string testfxloop(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.push_flow("fx/loop");// 压自己
    client << "L;";
    return "";
}

// @urlpath(null,fx/gone)
// demo: push a non-registered name — silently ignored, status 200
// 演示：push 不存在的路由名不抛错、不改判 404，只是那条编排不执行
std::string testfxgone(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.push_flow("no_such_step");
    client << "GONE;";
    return "";
}

// @urlpath(null,fx/exit)
// demo: return "exit" = explicit stop, do NOT drain the flow stack
// 演示：显式终止 vs 正常 return 空串的区别——栈里可能还有东西但就是不弹
std::string testfxexit(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.push_flow("fx/b");// 压了但不会弹
    client << "EXIT;";
    return "exit";// 显式终止
}

// @urlpath(null,fx/depa)
// demo: dependency-wait pattern — self-push + conditional jump
// 演示：「等条件就绪再跑」——第一次进来 output 里还没 B;，压自己跳 depb；
//       depb 把 B; 写进 output，第二次 depa 进来看到 B; 就知道依赖完成
// 应用场景：先跑鉴权/远程调用/读缓存，完了再回来继续主流程
std::string testfxdepa(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (peer->output.find("B;") == std::string::npos)
    {
        client.push_flow("fx/depa");// 压自己，让自己第二次被弹出来
        return "fx/depb";           // 立即跳去跑依赖
    }
    client << "A2;";// 第二次进来，依赖已就绪
    return "";
}

// @urlpath(null,fx/depb)
// depa 的依赖任务：把 B; 写进 output
std::string testfxdepb(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "B;";
    return "";
}

// =============================================================================
// coro 基础：sync/coro 混合编排
// Basic coro handlers & sync/coro mixed orchestration.
// =============================================================================

// @urlpath(null,fx/coro)
// demo: coro handler pushes a sync route — mixed works both ways
// 演示：coro regfun 里 push_flow("fx/b")（sync handler），链能正确桥接
asio::awaitable<std::string> testfxcoro(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "CORO;";
    client.push_flow("fx/b");// 推一个 sync handler
    co_return "";
}

// @urlpath(null,fx/coroa)
// demo: coro → push coro (pure coro chain)
// 演示：coro regfun 里 push_flow 另一个 coro
asio::awaitable<std::string> testfxcoroa(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "COROA;";
    client.push_flow("fx/corob");// coro → push coro
    co_return "";
}

// @urlpath(null,fx/corob)
// leaf coro handler
asio::awaitable<std::string> testfxcorob(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "COROB;";
    co_return "";
}

// @urlpath(null,fx/sync_push_coro)
// demo: sync → push coro (bridge into coro world)
// 演示：sync handler 里 push_flow 一个 coro——框架会把链切换进协程 driver
std::string testfxsyncpushcoro(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "SYNC;";
    client.push_flow("fx/coroc");// sync → push coro
    return "";
}

// @urlpath(null,fx/coroc)
// coro handler reached from sync-initial chain
asio::awaitable<std::string> testfxcoroc(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "COROC;";
    co_return "";
}

// =============================================================================
// coro pre → sync regfun 组合 + push 链
// coro pre + sync regfun, with an extra push after reg runs.
// =============================================================================

// @urlpath(null,fx/coro_pre)
// demo: coro pre —— co_await 外部鉴权接口，放行返回 "ok"
// 演示：coro pre 放行/拒绝语义跟 sync pre 一样——空串 = 拒绝
asio::awaitable<std::string> testfxcoropre(std::shared_ptr<httppeer> peer)
{
    co_return "ok";
}

// @urlpath(testfxcoropre,fx/coro_pre_sync)
// sync regfun reached after a coro pre
// coro pre 放行后 sync regfun 被 ThreadPool 执行
std::string testfxcoropresync(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "SYNC_AFTER_CORO_PRE;";
    client.push_flow("fx/corob");// sync → push coro
    return "";
}

// =============================================================================
// sync → coro → sync 三段跨类型混合
// Three-step mixed chain: sync → coro → sync.
// =============================================================================

// @urlpath(null,fx/mix_chain)
std::string testfxmixchain_sync(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "MIX_S;";
    client.push_flow("fx/mix_coro");// sync → push coro
    return "";
}

// @urlpath(null,fx/mix_coro)
asio::awaitable<std::string> testfxmixchain_coro(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "MIX_C;";
    client.push_flow("fx/mix_tail");// coro → push sync
    co_return "";
}

// @urlpath(null,fx/mix_tail)
std::string testfxmixchain_tail(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "MIX_T;";
    return "";
}

// =============================================================================
// 异常通道：sync/coro throw → fail_500
// Exception paths: throw → fail_500, pre throw skips reg.
// =============================================================================

// @urlpath(null,fx/exc/sync_reg)
// sync regfun throw → co_pool_run_step catch → fail_500
std::string testfxexc_sync_reg(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    throw std::runtime_error("bad sync reg");
    client << "testfxexc_sync_reg";
    return "";
}

// @urlpath(null,fx/exc/sync_pre)
// sync pre throw → regfun 不应执行
std::string testfxexc_sync_pre(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    throw std::runtime_error("bad sync pre");
    client << "testfxexc_sync_pre";
    return "";
}

// @urlpath(testfxexc_sync_pre,fx/exc/sync_pre_reg)
// 只是占位，sync pre throw 后这里不会执行
std::string testfxexc_sync_pre_reg(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "SHOULD_NOT_REACH;";
    return "";
}

// @urlpath(null,fx/exc/coro_reg)
// coro regfun throw → router try/catch (修复后) → fail_500
asio::awaitable<std::string> testfxexc_coro_reg(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    throw std::runtime_error("bad coro reg");
    client << "testfxexc_coro_reg;";
    co_return "";
}

// @urlpath(null,fx/exc/coro_pre)
// coro pre throw → regfun 不应执行
asio::awaitable<std::string> testfxexc_coro_pre(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    throw std::runtime_error("bad coro pre");
    client << "testfxexc_coro_pre;";
    co_return "";
}

// @urlpath(testfxexc_coro_pre,fx/exc/coro_pre_reg)
// 占位；coro pre throw 后这里不会执行
std::string testfxexc_coro_pre_reg(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "SHOULD_NOT_REACH_EITHER;";
    return "";
}

// @urlpath(null,fx/exc/coro_unknown)
// throw const char* → catch(...) → "unknown exception"
asio::awaitable<std::string> testfxexc_coro_unknown(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    throw "raw string exception";// const char*, not std::exception
    client << "testfxexc_coro_unknown;";
    co_return "";
}

// =============================================================================
// 503 通道：add_sync_task reject → fail_503
// Sync pool reject — triggered via test hook, auto-reset.
// =============================================================================

// @urlpath(null,fx/exc/reject503_pre)
// coro pre (no threadpool) safely sets the reject flag, then returns ok.
// coro pre 不进 ThreadPool，开 reject 一次性开关最安全
asio::awaitable<std::string> testfxexc_reject503_pre(std::shared_ptr<httppeer> peer)
{
    force_pool_reject_once();// next add_sync_task fails; atomic exchange auto-resets
    co_return "ok";
}

// @urlpath(testfxexc_reject503_pre,fx/exc/reject503)
// sync regfun — add_sync_task rejects → fail_503("server busy")
// sync regfun 被派单时 add_sync_task 返回 false → fail_503
std::string testfxexc_reject503(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "SHOULD_NOT_REACH_503;";// never reached
    return "";
}

// @urlpath(null,fx/exc/after503)
// verify the one-shot flag is consumed — subsequent sync handlers run fine
// 验证一次性开关已自动复位
std::string testfxexc_after503(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "AFTER503_OK;";
    return "";
}

// =============================================================================
// 业务线程池两条队列的演示台，和 /wssleep 那条 websocket 演示配套看。
//
// 业务线程池取货时有两条队列：
//   lane A（sync_tasks） —— HTTP 请求链的每一步都从这里排队，优先级最高；
//                            mqtt / socket 那些"要等钩子结果才能发协议报文"的同步钩子、
//                            以及 static_pre 这类同步前置钩子也走这一条
//   lane B（conn_tasks） —— websocket 这类投完就走的长连接钩子从这里排队
// lane A 一直有货时，lane B 大约每 10 拍才被看一眼（不这样写的话，持续的 HTTP 流量会让
// 长连接的钩子永远排不上）；lane A 空下来，lane B 就无条件继续服务。
//
// 下面四个路由把这两条队列变成可以用 curl 直接按的开关和计数：
//   /fx/lane/b?n=30&ms=10   往 lane B 投 30 格任务，每格睡 10ms，投完立刻返回
//   /fx/lane/a?n=30&ms=100  往 lane A 投 30 格，每格睡 100ms；这里是协程里一步步 await 的，
//                           所以要等 30 格全跑完这个请求才有回显
//   /fx/lane/read           两边各实际跑过多少格、累计睡了多少毫秒，外加池子的实时状态
//   /fx/lane/rejectonce     让池子下一次拒绝 lane B 的投递，试一下"池子不接单"时会发生什么
//   /fx/loop/slow?sec=1&ms=2500&beat=4
//                           登记一条间隔任务，每拍睡 2.5 秒而间隔只有 1 秒——
//                           看 /fx/lane/read 的 loop_peak 是否始终是 1（单飞门）、loop_skipped 是否开始涨
// 想从零开始数：/fx/lane/read?reset=1 只把下面这四个计数清零，
// 而 conn_dropped 是池子自己的进程级累计量，清不掉，所以要按增量来看。
// =============================================================================

namespace fxlane
{
// 这四个计数是"任务真的被执行了"的证据：写入只发生在池线程跑闭包的时候，
// 所以 HTTP 侧只能看到计数，不能凭返回码推断任务跑没跑。
std::atomic<unsigned long long> conn_done{0};// lane B 实际跑过的格数
std::atomic<unsigned long long> conn_ms{0};  // lane B 累计睡掉的毫秒数
std::atomic<unsigned long long> sync_done{0};// lane A 实际跑过的格数
std::atomic<unsigned long long> sync_ms{0};  // lane A 累计睡掉的毫秒数
// 间隔任务（/fx/loop/slow）的三个计数：跑了几拍、此刻几条同时在跑、历史最宽到几条。
// loop_peak 是那条单飞门的计数：它大于 1 就说明同一个任务被两个池线程同时跑过。
std::atomic<unsigned long long> loop_runs{0};
std::atomic<unsigned long long> loop_live{0};
std::atomic<unsigned long long> loop_peak{0};
// 这两格是任务的参数，不是请求的参数：间隔任务本体在池线程上跑，那时这次请求的
// client.get 已经被下一次请求（或同一个连接的 clear()）清掉了，读它只会读到 0。
std::atomic<unsigned long long> loop_ms_cfg{0};  // 每一拍睡多少毫秒
std::atomic<unsigned long long> loop_beat_cfg{0};// 跑够几拍自己收摊
}// namespace fxlane

// @urlpath(null,fx/lane/b)
// ?n=格数 &ms=每格睡多少毫秒。投完 n 格立刻返回，不等它们跑完 ——
// 这就是"投递方不被业务拖住"的形状；真实业务里投这条队列的是 websocket 的收帧循环。
// 返回值里 requested 是你要的格数，posted 是池子实际接下的格数（两个不等就说明池子在拒单）。
std::string testfxlaneb(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int n   = client.get["n"].to_int();
    unsigned int ms  = client.get["ms"].to_int();

    unsigned int posted = 0;
    for (unsigned int i = 0; i < n; i++)
    {
        if (post_conn_step("fx.laneb",
                           [ms]()
                           {
                               fxlane::conn_done += 1;
                               if (ms > 0)
                               {
                                   std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                                   fxlane::conn_ms += ms;
                               }
                           }))
        {
            posted += 1;
        }
    }

    client.val["requested"] = n;
    client.val["posted"]    = posted;
    client.out_json();
    return "";
}

// @urlpath(null,fx/lane/read)
// 看数用的路由，返回一段 JSON：
//   conn_done / conn_ms   lane B 实际跑过的格数、累计睡掉的毫秒数
//   sync_done / sync_ms   lane A 同上
//   conn_dropped          池子拒绝过多少次 lane B 的投递（进程级累计，reset 也清不掉它）
//   sync_dropped          同上，lane A 的那一份：投进去的闭包被拒了几次
//   loop_skipped          间隔任务被跳过的拍数（同一条还在跑，这一拍不重复投）
//   loop_ticks            每秒拍扫过间隔任务表多少次；登记了任务而它不涨，说明拍没走到这一段
//   loop_tasks            此刻间隔任务表里有几条任务
//   loop_runs             /fx/loop/slow 那个间隔任务实际跑了几拍
//   loop_live             此刻有几拍正在跑
//   loop_peak             历史上最多同时跑过几拍；大于 1 就是单飞门没夹住
//   tasknum               另一条队列 clienttasks 的排队长度（框架给出站客户端用的，跟这两条车道无关）
//   livenum               此刻正在池线程上执行的格数
//   threadnum             业务线程池现在的线程条数
// ?reset=1 先把上面四个计数清零，适合"先清零再动手"的单独试验；并发压测时别带它。
std::string testfxlaneread(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    httpserver &app  = get_server_app();

    if (client.get["reset"].to_int() == 1)
    {
        fxlane::conn_done = 0;
        fxlane::conn_ms   = 0;
        fxlane::sync_done = 0;
        fxlane::sync_ms   = 0;
    }

    client.val["conn_done"]    = (unsigned int)(fxlane::conn_done.load() & 0xffffffff);
    client.val["conn_ms"]      = (unsigned int)(fxlane::conn_ms.load() & 0xffffffff);
    client.val["sync_done"]    = (unsigned int)(fxlane::sync_done.load() & 0xffffffff);
    client.val["sync_ms"]      = (unsigned int)(fxlane::sync_ms.load() & 0xffffffff);
    client.val["conn_dropped"] = app.clientrunpool.getconndropped();
    client.val["sync_dropped"] = app.clientrunpool.getsyncdropped();
    client.val["loop_skipped"] = (unsigned int)(app.clientloop_skipped.load() & 0xffffffff);
    client.val["loop_ticks"]   = (unsigned int)(app.clientloop_ticks.load() & 0xffffffff);
    {
        std::lock_guard<std::mutex> lk(app.clientlooptasks_mutex);
        client.val["loop_tasks"] = (unsigned int)app.clientlooptasks.size();
    }
    client.val["loop_runs"] = (unsigned int)(fxlane::loop_runs.load() & 0xffffffff);
    client.val["loop_live"] = (unsigned int)(fxlane::loop_live.load() & 0xffffffff);
    client.val["loop_peak"] = (unsigned int)(fxlane::loop_peak.load() & 0xffffffff);
    client.val["tasknum"]   = app.clientrunpool.gettasknum();
    client.val["livenum"]   = app.clientrunpool.getlivenum();
    client.val["threadnum"] = app.clientrunpool.getpoolthreadnum();
    client.out_json();
    return "";
}

// @urlpath(null,fx/lane/a)
// 给 lane A 制造持续有货的那一路：协程里连续 n 步 co_await co_pool_run_void(...)，
// 每步睡 ms，所以这一步从投递到跑完期间 lane A 都不空。
// 拿来和 /fx/lane/b 对照着看：先开着这条，再投 lane B，就能观察到
// "lane A 有货时 lane B 大约每 10 拍才被服务一次"这个比例（跑完 lane A 再投 lane B，
// 那份 1/10 就量不出来了，因为 lane A 一空 lane B 就变成无条件服务）。
// 回显：steps=要跑的格数，ms=每格睡多少毫秒，elapsed_ms=这一路总耗时，bad_steps=被拒单或抛异常的次数。
asio::awaitable<std::string> testfxlanea(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int n   = client.get["n"].to_int();
    unsigned int ms  = client.get["ms"].to_int();
    if (n == 0)
        n = 1;
    if (n > 2000)
        n = 2000;

    unsigned int bad = 0;
    auto t0          = std::chrono::steady_clock::now();
    for (unsigned int i = 0; i < n; i++)
    {
        auto out = co_await co_pool_run_void(
            std::function<void()>(
                [ms]()
                {
                    fxlane::sync_done += 1;
                    if (ms > 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                        fxlane::sync_ms += ms;
                    }
                }));
        // 拒单/异常都算没跑成，但不能让这条路由把协程挂住，计数后继续下一步
        if (out.rejected || out.eptr)
            bad += 1;
    }
    unsigned long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::steady_clock::now() - t0)
                                     .count();

    client.val["steps"]      = n;
    client.val["ms"]         = ms;
    client.val["elapsed_ms"] = (unsigned int)(elapsed & 0xffffffff);
    client.val["bad_steps"]  = bad;
    client.out_json();
    co_return "";
}

// @urlpath(null,fx/lane/rejectonce)
// 让业务线程池的下一次 lane B 投递被拒绝，然后立刻返回。开关是一次性的，用完自动复位。
// 这条路由自己既不占 lane A 的步、也不投 lane B，所以被拒的一定是你紧接着做的那一下
// —— 例如随后从客户端发来的一条 websocket 消息。
// 拒掉的是"这一次执行的机会"，不是那条消息本身：消息还留在这条连接的接收队列里，
// 等下一次触发时被一起带出来，所以表现是回显整体往后挪一格、四条都在、顺序不变，
// 最后那一条要再发一条消息（或让连接上还有后续触发）才会推出来。
// 想确认没有丢东西：看 /fx/lane/read 的 conn_dropped 涨了 1，而连接没被断开。
asio::awaitable<std::string> testfxlanerejectonce(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    force_pool_reject_conn_once();
    client << "CONN_REJECT_ARMED;";
    co_return "";
}

// =============================================================================
// 间隔任务（框架的每秒拍会把登记过的任务投给业务线程池）
// Interval task: the per-second tick posts registered peers to the business pool.
// =============================================================================

// @urlpath(null,fx/loop/slow)
// ?sec=每隔几秒跑一次（默认 1） &ms=每一拍睡多少毫秒（默认 0） &beat=跑几拍之后自己收摊（默认 3）
// 这条路由回 "T" 就是把当前请求的 peer 登记成间隔任务，之后框架每隔 sec 秒投一拍。
// ms 大于 sec×1000 就是故意让每一拍比自己的间隔还慢：没有单飞门的话拍数会越积越多、
// 同一个任务同时有好几个执行者；有了它则始终只有一个在跑，抢不到的那拍直接跳过，
// 计数看 /fx/lane/read 的 loop_peak（应为 1）、loop_skipped（应大于 0）、loop_runs（应等于 beat）。
std::string testfxloopslow(std::shared_ptr<httppeer> peer)
{
    httppeer &client  = peer->get_peer();
    unsigned int sec  = client.get["sec"].to_int();
    unsigned int ms   = client.get["ms"].to_int();
    unsigned int beat = client.get["beat"].to_int();
    if (sec == 0)
        sec = 1;
    if (beat == 0)
        beat = 3;

    fxlane::loop_runs = 0;
    fxlane::loop_live = 0;
    fxlane::loop_peak = 0;
    // 每一拍睡多少毫秒、跑几拍收摊，存在这里而不是让任务本体去读 client.get：
    // client.get 是"本次请求"的状态，请求一结束就被复位（httppeer::clear()），
    // 而任务本体是在之后的某一拍、另一个线程上跑的。
    fxlane::loop_ms_cfg   = ms > 60000 ? 60000 : ms;
    fxlane::loop_beat_cfg = beat;

    peer->add_timeloop_task("fxlooptick", sec);

    client.val["sec"]  = sec;
    client.val["ms"]   = ms;
    client.val["beat"] = beat;
    client.out_json();
    return "T";
}

// @urlpath(null,fxlooptick)
// 间隔任务本体：框架的每秒拍把它投给业务线程池，池线程按登记时写下的任务名调起这个函数。
// 名字要和 add_timeloop_task() 里传的字符串一字不差（这里传的是 "fxlooptick"）。
// 跑够 beat 拍就 clear_timeloop_task() 把自己摘掉，不然这条任务会一直占着表。
std::string testfxlooptick(std::shared_ptr<httppeer> peer)
{
    unsigned int ms   = (unsigned int)(fxlane::loop_ms_cfg.load() & 0xffffffff);
    unsigned int beat = (unsigned int)(fxlane::loop_beat_cfg.load() & 0xffffffff);
    if (beat == 0)
        beat = 3;

    unsigned long long now  = (fxlane::loop_live += 1);
    unsigned long long peak = fxlane::loop_peak.load();
    while (now > peak && !fxlane::loop_peak.compare_exchange_strong(peak, now))
    {
    }
    fxlane::loop_runs += 1;

    if (ms > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    fxlane::loop_live -= 1;

    if (fxlane::loop_runs.load() >= beat)
    {
        peer->clear_timeloop_task();
    }
    return "";
}

}// namespace http
