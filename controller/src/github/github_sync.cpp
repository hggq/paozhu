// 定时那一拍：捞到期账号 → 刷令牌 → 翻页写仓库 → 只在自然翻完时做软删对比。
//
// 形状照 controller/src/oauth/oauth_purge.cpp：一条任务本体 + 一条把它登记下去的开关，
// 开关那条必须 return "@timeloop"——peer->add_timeloop_task() 只改这条 peer 的调度字段，
// 真正进 clientlooptasks 表的是路由链上的 frametasks_timeloop。少了那个哨兵，任务表里没有这一条。
//
// 三条纪律：
// 1 先推 next_sync_time 再打 GitHub。这一轮中途崩了，下一轮不会又捞到同一批，把一次失败变成反复重试。
// 2 401 与"403 且额度见底"分开走。前者是死票，再打一百次也不会自己活；后者是排队，推到 Reset 之后。
//   把两者并成一支，就等于一次限流能把一条正常绑定永久打死。
// 3 软删只在最后一页返回条数 < per_page 的那一轮做。翻页上界截断的那一轮里"没拉到"是截断不是删除，
//   标下去就是"第 301 个以后的仓库凭空消失"。
//
// 令牌与 secret 都不进日志：DEBUG_LOG 那几条只摆计数与 id。
#include <string>
#include <vector>

#include "cost_define.h"
#include "datetime.h"
#include "func.h"
#include "github_api.h"
#include "github_config.h"
#include "github_oauth.h"
#include "github_store.h"
#include "github_sync.h"
#include "httppeer.h"
#include "terminal_color.h"

namespace http
{
//@urlpath(null,github/sync)
std::string github_sync(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    // 闸门与登记页同一个口径。间隔任务那一拍放行：它复跑的是当初登记它的那个对象，
    // 那次请求已经被 github_sync_arm() 的 ip_is_local() 拦过一道，而任务本体上挂的
    // client_ip 不是这一拍发起方的（框架内部的 rpc 循环会造出一个 client_ip 为空的 peer，
    // 它照样走路由链）。只写 ip_is_local() 会把定时器那一拍一起拦掉。
    if (peer->linktype != 7 && !ip_is_local(client.client_ip))
    {
        client << "github sync is local-trigger only";
        return "@exit";
    }

    auto cfg = github::read_github_settings();

    // conf 里把这两个数写成 0 会得到"每一拍都重捞同一批、一拍一次请求都不发"的怪状态；
    // 回落编译期默认，节拍和上界至少有一个是合理的。
    unsigned int interval  = cfg.sync_interval > 0 ? cfg.sync_interval : CONST_GH_SYNC_INTERVAL;
    unsigned int max_pages = cfg.sync_max_pages > 0 ? cfg.sync_max_pages : CONST_GH_SYNC_MAX_PAGES;
    unsigned int batch     = cfg.sync_batch > 0 ? cfg.sync_batch : CONST_GH_SYNC_BATCH;

    unsigned long long now = (unsigned long long)timeid();

    // 加密密钥不可用就整轮跳过：不解密、不发 HTTP、也不要把行误标成死票。
    if (!cfg.key_ok)
    {
        DEBUG_LOG("[github] sync skipped, crypto key unavailable");
        return "";
    }

    // 状态机前置：把临近过期的活票翻成 2（拉进本批优先刷），把已过期且无 refresh 的翻成 3。
    // 少了后一条，那一行会每拍都被捞到、每拍都刷不动，永远不收敛。
    unsigned int to_refresh = 0;
    unsigned int to_dead    = 0;
    github::presweep_expiring(now, now + interval, to_refresh, to_dead);

    std::vector<github::gh_account> due;
    if (!github::fetch_due_batch(now, batch, due))
    {
        DEBUG_LOG("[github] sync batch read failed");
        return "";
    }

    unsigned int synced = 0;
    for (unsigned int i = 0; i < due.size(); i++)
    {
        github::gh_account &row = due[i];

        // 先推再做。
        github::push_next_sync(row.id, now + interval);

        std::string token = row.access_token;
        bool stale        = (row.access_expires_at != 0 && row.access_expires_at <= now);
        if (stale)
        {
            if (row.refresh_token.empty())
            {
                // 预扫已经翻过一遍，这里是兜底：没有 refresh 的过期票只能由用户重新授权救回来。
                github::mark_token_dead(row.id, "expired without refresh");
                continue;
            }
            std::string plain_secret;
            std::string error;
            if (!github::decrypt_secret_of(cfg, row, plain_secret, error))
            {
                plain_secret.clear();
                // 解不开多半是 master key 换了。按失败退避，不要标死——行还在，配回原密钥就能续上。
                github::record_fail(row.id, cfg, now, "secret decrypt failed");
                continue;
            }
            github::token_answer tk;
            bool refreshed = github::refresh_access(cfg, row.client_id, plain_secret, row.refresh_token, tk);
            plain_secret.clear();
            if (!refreshed || !tk.ok || tk.access_token.empty())
            {
                // refresh 被拒＝这枚票在 GitHub 侧已经没了（用户撤了授权、或 refresh 也过期）。
                github::mark_token_dead(row.id, "refresh rejected");
                continue;
            }
            if (!github::save_refreshed(row.id, tk, now))
            {
                github::record_fail(row.id, cfg, now, "save refresh failed");
                continue;
            }
            token = tk.access_token;
        }
        if (token.empty())
        {
            github::record_fail(row.id, cfg, now, "no access token");
            continue;
        }

        std::vector<github::gh_repo_item> items;
        bool natural_end = false;
        bool aborted     = false;
        for (unsigned int page = 1; page <= max_pages; page++)
        {
            github::gh_page_result pr = github::fetch_repos_page(cfg, token, page);
            github::gh_api_state st   = pr.state();
            if (st == github::gh_api_state::ok)
            {
                items.insert(items.end(), pr.items.begin(), pr.items.end());
                if (pr.items.size() < (size_t)CONST_GH_REPO_PER_PAGE)
                {
                    natural_end = true;
                    break;
                }
                continue;
            }
            aborted = true;
            if (st == github::gh_api_state::token_dead)
            {
                github::mark_token_dead(row.id, "http 401");
            }
            else if (st == github::gh_api_state::rate_limited)
            {
                // 限流不是错误，是排队：推到 X-RateLimit-Reset 之后，下一拍自然绕开它。
                github::record_rate_limited(row.id, now, pr.ratelimit_reset, cfg);
            }
            else
            {
                github::record_fail(row.id, cfg, now, pr.error);
            }
            break;
        }
        token.clear();

        // 已经翻到的先落库：中途失败也不丢这几页的活儿，upsert 幂等，下一轮还会覆盖。
        if (!items.empty())
        {
            unsigned int written = 0;
            github::upsert_repos(row.id, items, now, written);
        }
        if (aborted)
        {
            continue;
        }

        std::string note;
        if (natural_end)
        {
            std::vector<unsigned long long> present;
            present.reserve(items.size());
            for (unsigned int k = 0; k < items.size(); k++)
            {
                present.push_back(items[k].gh_repo_id);
            }
            unsigned int marked = 0;
            github::soft_delete_missing(row.id, present, now, marked);
        }
        else
        {
            // 被页数上界截断的这一轮跳过软删，改在 last_error 留一句"清单可能不完整"。
            note = "repo list truncated at " + std::to_string(max_pages) + " pages";
        }
        github::record_success(row.id, now, now + interval, note);
        synced++;
    }

    if (synced > 0 || to_refresh > 0 || to_dead > 0)
    {
        DEBUG_LOG("[github] sync batch:%u synced:%u to_refresh:%u to_dead:%u",
                  (unsigned int)due.size(),
                  synced,
                  to_refresh,
                  to_dead);
    }
    return "";
}

// 登记这条间隔任务。闸门与证书下载页、oauth 清理页同一个口径（ip_is_local）。
// 任务本体那条路由不设 HTTPS 闸：间隔任务复跑时拿到的是当初登记它的那次请求的对象，
// 是它而不是当前连接决定 is_ssl()。
// 本功能的部署前置条件是"HTTP 服务单实例"：每个进程都有自己的 tick 线程和自己的任务表，
// 两个实例都登记就会各自捞同一批到期行、各自打 GitHub。
//@urlpath(null,github/sync/arm)
std::string github_sync_arm(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    if (!ip_is_local(client.client_ip))
    {
        client << "github sync task not registered (local-only)";
        return "";
    }
    auto cfg              = github::read_github_settings();
    unsigned int interval = cfg.sync_interval > 0 ? cfg.sync_interval : CONST_GH_SYNC_INTERVAL;
    peer->add_timeloop_task("github/sync", interval);
    client << "github sync task registered, every " << interval << "s";
    return "@timeloop";
}
}// namespace http
