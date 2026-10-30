// 列表页：本租户的绑定行，加上被点中那一行的仓库清单。
//
// 为什么页面读库而不是当场打 GitHub：一页一请求的话，5000 次/小时/令牌的额度会被自己的
// 后台和页面一起刷穿，而且页面快慢从此绑在 GitHub 的可用性上。仓库清单只由 /github/sync 写。
//
// 回显面上没有 access_token、refresh_token、client_secret 这三列——列表页的 val 是会原样
// 进 HTML 的，少一列就少一条把凭据带出去的通路。last_error 是例外：它在落库前已经过
// 一遍敏感片段过滤（scrub_error_text），剩下的只有状态码和 GitHub 的错误名。
#include <string>
#include <vector>

#include "datetime.h"
#include "func.h"
#include "github_config.h"
#include "github_page.h"
#include "github_store.h"
#include "httppeer.h"

namespace http
{
//@urlpath(admin_islogin,github/list)
std::string github_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    // 租户 id 取自登录时写下的 userid：那一格存的就是 sysuser.companyid，
    // session 里没有 companyid 这个键，照它读会恒为 0。
    unsigned long long companyid = (unsigned long long)client.session["userid"].to_int();
    unsigned long long adminid   = (unsigned long long)client.session["adminid"].to_int();

    auto cfg = github::read_github_settings();

    client.val["key_ok"] = cfg.key_ok;
    // quickbind 一键授权入口：只有默认 OAuth App 三键都配好了才显示按钮，
    // 否则点进去必然被拒（handler 里有同样的 default_app_ready 检查）。
    client.val["quickbind_ok"] = (unsigned int)github::default_app_ready(cfg);
    client.val["adminid"]      = adminid;
    client.val["companyid"]    = companyid;
    // 表单里的 scope 提示直接回显 conf 的原文，与发出去的请求差一个 normalize_scopes，
    // 页面只当说明文字用。
    client.val["scopes"] = cfg.default_scopes;

    std::vector<github::gh_account> accounts;
    github::list_accounts(companyid, accounts);

    client.val["accounts"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < accounts.size(); i++)
    {
        temp.clear();
        temp["id"]                = accounts[i].id;
        temp["client_id"]         = accounts[i].client_id;
        temp["redirect_uri"]      = accounts[i].redirect_uri;
        temp["gh_user_id"]        = accounts[i].gh_user_id;
        temp["gh_login"]          = accounts[i].gh_login;
        temp["gh_avatar"]         = accounts[i].gh_avatar;
        temp["scopes"]            = accounts[i].scopes;
        temp["token_state"]       = (unsigned int)accounts[i].token_state;
        temp["isopen"]            = (unsigned int)accounts[i].isopen;
        temp["access_expires_at"] = accounts[i].access_expires_at;
        temp["next_sync_time"]    = accounts[i].next_sync_time;
        temp["last_sync_time"]    = accounts[i].last_sync_time;
        temp["last_sync_ok"]      = (unsigned int)accounts[i].last_sync_ok;
        temp["sync_fails"]        = accounts[i].sync_fails;
        temp["last_error"]        = accounts[i].last_error;
        // 页面上一眼能看懂的时间，省得模板里自己算。
        temp["next_sync_text"] = accounts[i].next_sync_time > 0 ? get_date("%Y-%m-%d %H:%M:%S", (unsigned int)accounts[i].next_sync_time) : std::string("");
        temp["last_sync_text"] = accounts[i].last_sync_time > 0 ? get_date("%Y-%m-%d %H:%M:%S", (unsigned int)accounts[i].last_sync_time) : std::string("");
        client.val["accounts"].push(temp);
    }

    // 仓库清单：点哪一行看哪一行；没点就不查，列表页首屏不必为每行各发一条语句。
    unsigned long long picked  = (unsigned long long)client.get["id"].to_int();
    bool show_deleted          = client.get["show_deleted"].to_int() != 0;
    client.val["picked_id"]    = picked;
    client.val["show_deleted"] = show_deleted;
    client.val["repos"].set_array();
    if (picked > 0)
    {
        // 只允许看自己租户的行：id 是请求参数，直接拿它查表等于把别人的仓库清单端给访客。
        bool mine = false;
        for (unsigned int i = 0; i < accounts.size(); i++)
        {
            if (accounts[i].id == picked)
            {
                mine = true;
                break;
            }
        }
        std::vector<github::gh_repo_row> repos;
        if (mine && github::list_repos(picked, show_deleted, repos))
        {
            obj_val row;
            for (unsigned int i = 0; i < repos.size(); i++)
            {
                row.clear();
                row["gh_repo_id"]       = repos[i].item.gh_repo_id;
                row["full_name"]        = repos[i].item.full_name;
                row["is_private"]       = repos[i].item.is_private;
                row["description"]      = repos[i].item.description;
                row["primary_language"] = repos[i].item.primary_language;
                row["stars"]            = repos[i].item.stars;
                row["pushed_at"]        = repos[i].item.pushed_at;
                row["pushed_text"]      = repos[i].item.pushed_at > 0 ? get_date("%Y-%m-%d %H:%M:%S", repos[i].item.pushed_at) : std::string("");
                row["is_deleted"]       = repos[i].is_deleted;
                client.val["repos"].push(row);
            }
        }
    }

    peer->view("github/list");
    return "";
}
}// namespace http
