// 过期行清理：两条路由，一条是间隔任务的本体，一条是把它登记下去的开关。
//
// 为什么要清理：oauth_code 一次授权一行、oauth_refresh 每次轮换一行，两张都是只增的流水表。
// 没人删就是一年千万行，uq_code_hash / uq_token_hash 跟着涨。
//
// 过期行的删除门槛取"这一行自己的 expires_at 早于 now - CONST_OAUTH_PURGE_KEEP"，而不是"已撤销就删"——
// revoked 的 refresh 行是重放检测的证据，提前删掉就再也认不出"同一枚码被换了第二次"。
//
// 登记口只有这一处，且必须 return "@timeloop"：peer->add_timeloop_task() 只改这条 peer 的调度字段，
// 真正进 clientlooptasks 表的是路由链上的 frametasks_timeloop（router.cpp 判到
// chain_act::run_timeloop 才按名字调它）。少了那个 "@timeloop"，任务表里没有这一条，
// 日志里也不会报错——只是永远不会跑。先例 controller/src/testaddclienttask.cpp:29。

#include <string>

#include "cost_define.h"
#include "datetime.h"
#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_audit.h"
#include "oauth/oauth_store.h"
#include "oauth_purge.h"

namespace http
{
// 一拍里把两张表各扫一遍，每张最多 CONST_OAUTH_PURGE_MAX_ROUNDS 轮。
// 一轮删不满 CONST_OAUTH_PURGE_BATCH 就收手，所以没有积压的一拍只发两条各删 0 行的 DELETE。
//@urlpath(null,oauth/purge)
std::string oauth_purge(std::shared_ptr<httppeer> peer)
{
    // 保留期比 refresh 的 TTL 长，所以这一刀永远碰不到"还在重放检测窗口里"的行。
    long long cutoff = (long long)timeid() - (long long)CONST_OAUTH_PURGE_KEEP;

    unsigned int codes_deleted     = 0;
    unsigned int refreshes_deleted = 0;
    for (unsigned int round = 0; round < CONST_OAUTH_PURGE_MAX_ROUNDS; ++round)
    {
        unsigned int n = oauth::purge_codes_before(cutoff, CONST_OAUTH_PURGE_BATCH);
        codes_deleted += n;
        if (n < CONST_OAUTH_PURGE_BATCH)
        {
            break;
        }
    }
    for (unsigned int round = 0; round < CONST_OAUTH_PURGE_MAX_ROUNDS; ++round)
    {
        unsigned int n = oauth::purge_refreshes_before(cutoff, CONST_OAUTH_PURGE_BATCH);
        refreshes_deleted += n;
        if (n < CONST_OAUTH_PURGE_BATCH)
        {
            break;
        }
    }
    // 只进日志：这条既可能是间隔任务的一拍，也可能是有人直接 GET 了一下这页，两种都要留数。
    if (codes_deleted > 0 || refreshes_deleted > 0)
    {
        OAUTH_AUDIT("purge cutoff:" + std::to_string(cutoff) + " codes:" + std::to_string(codes_deleted) +
                    " refreshes:" + std::to_string(refreshes_deleted));
    }
    return "";
}

// 登记这条间隔任务。闸门与证书下载页同一个口径（ip_is_local），任务本体那条路由不设 HTTPS 闸：
// 间隔任务复跑时拿到的是当初登记它的那次请求的对象，是它而不是当前连接决定 is_ssl()，
// 加了闸就等于把清理绑在登记那一次走的明文还是密文上。
// 直接命中它只是多做一轮已经到期的删除，那条语句本身就是幂等的。
//@urlpath(null,oauth/purge/arm)
std::string oauth_purge_arm(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    if (!ip_is_local(client.client_ip))
    {
        client << "oauth purge task not registered (local-only)";
        return "";
    }
    peer->add_timeloop_task("oauth/purge", CONST_OAUTH_PURGE_INTERVAL);
    client << "oauth purge task registered, every " << CONST_OAUTH_PURGE_INTERVAL << "s";
    return "@timeloop";
}
}// namespace http
