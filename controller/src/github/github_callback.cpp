// GitHub 回调：验票据 → 占位行 → 换令牌 → 拿身份 → 落最终态。
//
// state 的校验口径是"与 session 里那一枚逐字节相等"，这比逐段校验更强：三段分别对齐
// adminid、client_id、随机数，而整体相等一次就把三者一起证明了。分段的用处在这里——
// 它把 client_id 解出来，好去查这一行是不是已经绑过。
//
// 校验通过后立刻抹掉 session 里的票据、verifier 与暂存密文：同一枚 code 不能第二次提交，
// 而暂存的那枚密文没有理由在换完令牌后还留在会话里。
#include <string>

#include "acme_crypto.hpp"
#include "cost_define.h"
#include "datetime.h"
#include "func.h"
#include "github_callback.h"
#include "github_api.h"
#include "github_config.h"
#include "github_crypto.h"
#include "github_oauth.h"
#include "github_store.h"
#include "httppeer.h"

namespace http
{
//@urlpath(admin_islogin,github/callback)
std::string github_callback(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    std::string code     = client.get["code"].to_string();
    std::string state    = client.get["state"].to_string();
    std::string user_err = client.get["error"].to_string();

    if (!user_err.empty())
    {
        // 用户在 GitHub 那页点了取消。这时候什么都没发生，库里不该留下任何痕迹。
        client.goto_url("/github/list", 3, "authorization cancelled");
        return "";
    }
    if (code.empty() || state.empty())
    {
        client.goto_url("/github/list", 3, "callback without code or state");
        return "";
    }

    unsigned long long adminid   = (unsigned long long)client.session["adminid"].to_int();
    unsigned long long companyid = (unsigned long long)client.session["userid"].to_int();

    std::string want_state    = client.session["gh_state"].to_string();
    std::string verifier      = client.session["gh_code_verifier"].to_string();
    std::string cipher_b64    = client.session["gh_cipher_b64"].to_string();
    std::string redirect_uri  = client.session["gh_redirect_uri"].to_string();
    unsigned long long expire = (unsigned long long)client.session["gh_state_expire"].to_int();

    if (want_state.empty() || cipher_b64.empty())
    {
        client.goto_url("/github/list", 3, "binding session expired, please submit again");
        return "";
    }
    unsigned long long now = (unsigned long long)timeid();
    if (now >= expire)
    {
        client.goto_url("/github/list", 3, "binding ticket expired, please submit again");
        return "";
    }
    if (state != want_state)
    {
        // 不匹配就是不该收到这一趟回调（CSRF / 串号 / 别的浏览器）。中止，不解释差异在哪，
        // 免得这条页面变成"你提交的 state 长这样"的预言机。
        client.goto_url("/github/list", 3, "state mismatch");
        return "";
    }

    std::string cipher = b64url::decode(cipher_b64);
    if (cipher.empty())
    {
        client.goto_url("/github/list", 3, "stored secret unreadable");
        return "";
    }

    unsigned long long state_adminid = 0;
    std::string client_id;
    if (!github::state_shape_ok(state, state_adminid, client_id) || state_adminid != adminid)
    {
        client.goto_url("/github/list", 3, "state shape invalid");
        return "";
    }

    // 一次性：往下就是真活儿了，票据用不上第二次。
    client.session["gh_state"]         = "";
    client.session["gh_state_expire"]  = 0;
    client.session["gh_code_verifier"] = "";
    client.session["gh_cipher_b64"]    = "";
    client.session["gh_redirect_uri"]  = "";
    client.save_session();

    auto cfg = github::read_github_settings();
    if (!cfg.key_ok)
    {
        client.goto_url("/github/list", 3, "server has no crypto key configured");
        return "";
    }

    // 这一行是不是已经绑过，决定失败分支往哪儿收：新插的占位行留 state=4 当痕迹，
    // 既有绑定要回到改前状态——"重新授权"这个动作本身不该有能力杀死一条正常同步的绑定。
    github::gh_account row;
    bool existed              = github::load_account_by_key(adminid, client_id, row);
    unsigned long long id     = 0;
    unsigned char prior_state = 0;

    if (existed)
    {
        id          = row.id;
        prior_state = row.token_state;
    }
    else
    {
        // 先插占位行再换令牌：换成功但最终落库崩了，这枚令牌就永远没人知道它存在过。
        id = github::insert_account(client_id, cipher, redirect_uri, adminid, companyid, now);
        if (id == 0)
        {
            client.goto_url("/github/list", 3, "cannot write binding row");
            return "";
        }
    }

    std::string plain_secret;
    std::string error;
    {
        // decrypt_secret_of 要的是"这一行的密文"，重绑时手上那枚新密文才是这次要用的那一份，
        // 所以临时摆一个只填了凭据的形状给它，不去改写库里的行。
        github::gh_account holder;
        holder.id            = id;
        holder.client_secret = cipher;
        if (!github::decrypt_secret_of(cfg, holder, plain_secret, error))
        {
            if (existed)
            {
                github::set_state(id, prior_state, "secret decrypt failed");
            }
            else
            {
                github::set_state(id, 4, "secret decrypt failed");
            }
            client.goto_url("/github/list", 3, "cannot decrypt stored secret");
            return "";
        }
    }

    // 换令牌用的 redirect_uri 必须是刚才发 authorize 时那一枚（GitHub 要逐字节相同），
    // 也就是 session 里这份；库里旧行的那一份可能已经跟着 app 设置改过。
    github::token_answer tk;
    bool exchanged = github::exchange_code(cfg, client_id, plain_secret, code, redirect_uri, verifier, tk);
    plain_secret.clear();
    if (!exchanged || !tk.ok || tk.access_token.empty())
    {
        std::string why = tk.error.empty() ? std::string("token exchange failed") : github::scrub_error_text(tk.error);
        if (existed)
        {
            // 旧令牌一个字节都不动，状态回到改前——用户粘错一次 secret 不该打死现有绑定。
            github::set_state(id, prior_state, "rebind failed: " + why);
        }
        else
        {
            github::set_state(id, 4, "exchange failed: " + why);
        }
        client.goto_url("/github/list", 3, "token exchange failed");
        return "";
    }

    // 身份三列：拿不到也不阻断绑定（scope 被用户改小时 /user 会 403），留空并记一句。
    github::gh_user_info who;
    bool who_ok = false;
    {
        std::string user_error;
        who_ok = github::fetch_user(cfg, tk.access_token, who, user_error);
    }
    if (!who_ok && existed)
    {
        who.user_id = row.gh_user_id;
        who.login   = row.gh_login;
        who.avatar  = row.gh_avatar;
    }

    // 新密文只在换令牌成功的那一次跟着写进去；失败的趟子上库里那一行还是原样。
    std::string cipher_field = existed ? cipher : std::string("");
    if (!github::save_bound(id, cipher_field, who, tk, now))
    {
        if (existed)
        {
            github::set_state(id, prior_state, "save bound row failed");
        }
        else
        {
            github::set_state(id, 4, "save bound row failed");
        }
        cipher.clear();
        client.goto_url("/github/list", 3, "cannot save binding");
        return "";
    }
    cipher.clear();

    if (!who_ok)
    {
        github::set_state(id, 1, "user info unavailable");
    }

    // next_sync_time 已经由 save_bound 记成 now，下一拍就同步一次。
    client.goto_url("/github/list", 3, "bound");
    return "";
}
}// namespace http
