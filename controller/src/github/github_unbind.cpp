// 解绑：先在库里立旗，再去 GitHub 吊销，最后才删本地行。
//
// 顺序是反过来的话会漏：先删本地行，一旦吊销调用失败（5xx、超时、网络错），
// 我们连"该用哪一枚 client_secret 再试一次"都查不到了——本地无据、GitHub 侧令牌照旧活着。
// 所以 ① UPDATE token_state=4 让定时任务的 SELECT IN (1,2) 自然跳过这一行，
// ② 用这一行自己的 secret 做 HTTP Basic 去 DELETE /applications/{client_id}/token，
// ③ 只有确认吊销（200/204）或 GitHub 说本来就没有（404/422，重试永远失败）才删本地行，
// ④ 5xx/超时保留 state=4 的行，给页面一个重试入口。
//
// 出参只有 code/msg 两个键：令牌、secret、密文一个都不回显。
#include <string>

#include "github_config.h"
#include "github_oauth.h"
#include "github_store.h"
#include "github_unbind.h"
#include "httppeer.h"
#include "terminal_color.h"

namespace http
{
//@urlpath(admin_isloginjson,github/unbind)
std::string github_unbind(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned long long id = (unsigned long long)client.post["id"].to_int();
    if (id == 0)
    {
        client.val["code"] = 0;
        client.val["msg"]  = "id required";
        client.out_json();
        return "";
    }

    github::gh_account row;
    if (!github::load_account(id, row))
    {
        client.val["code"] = 0;
        client.val["msg"]  = "binding row not found";
        client.out_json();
        return "";
    }
    // id 是请求参数，不校验归属就等于谁都能凭一个自增号解掉别家租户的绑定。
    unsigned long long companyid = (unsigned long long)client.session["userid"].to_int();
    if (row.companyid != companyid)
    {
        client.val["code"] = 0;
        client.val["msg"]  = "not your binding";
        client.out_json();
        return "";
    }

    if (!github::mark_unbinding(id))
    {
        client.val["code"] = 0;
        client.val["msg"]  = "cannot mark unbinding";
        client.out_json();
        return "";
    }

    // 手上就没有令牌（占位行、换令牌失败的行）：GitHub 侧无可吊销的东西，删本地行不会泄漏什么。
    if (row.access_token.empty())
    {
        bool removed = github::delete_account(id);
        client.val["code"] = removed ? 1 : 0;
        client.val["msg"]  = removed ? "unbound" : "cannot delete local row";
        client.out_json();
        return "";
    }

    auto cfg = github::read_github_settings();

    std::string plain_secret;
    std::string error;
    if (!cfg.key_ok || !github::decrypt_secret_of(cfg, row, plain_secret, error))
    {
        plain_secret.clear();
        // 解不出 secret 就没法吊销。行留在 state=4：定时任务不再碰它，但 GitHub 那枚令牌还活着，
        // 这一条必须让人看得见、能重试，不能假装解绑成功。
        client.val["code"] = 0;
        client.val["msg"]  = "cannot decrypt secret, token still active at github";
        client.out_json();
        return "";
    }

    unsigned int status = 0;
    auto outcome = github::revoke_token(cfg, row.client_id, plain_secret, row.access_token, status);
    plain_secret.clear();

    if (outcome == github::revoke_outcome::failed)
    {
        DEBUG_LOG("[github] unbind id:%llu revoke failed http:%u", id, status);
        client.val["code"]   = 0;
        client.val["status"] = status;
        client.val["msg"]    = "github revoke failed, binding kept for retry";
        client.out_json();
        return "";
    }

    bool removed = github::delete_account(id);
    if (!removed)
    {
        // GitHub 侧已经吊销干净，本地行却删不掉——记一行 state=4 的僵尸，比反过来安全得多。
        DEBUG_LOG("[github] unbind id:%llu revoke ok but local delete failed", id);
        client.val["code"] = 0;
        client.val["msg"]  = "token revoked, local row delete failed";
        client.out_json();
        return "";
    }
    if (outcome == github::revoke_outcome::already_gone)
    {
        // 404/422：GitHub 早就没有这枚令牌（用户在 app 后台撤过、或 secret 换过）。重试也不会变，
        // 所以本地行照删，只在日志里留一句，免得下一个人以为吊销真打过一次。
        DEBUG_LOG("[github] unbind id:%llu nothing to revoke http:%u", id, status);
    }
    client.val["code"] = 1;
    client.val["msg"]  = "unbound";
    client.out_json();
    return "";
}
} // namespace http
