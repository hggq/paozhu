// 一键绑定：从 conf 读默认 OAuth App → 加密 → session 暂存 → 302 到 GitHub。
//
// 和 github_bind.cpp 的区别只有"凭据来源"：表单 vs conf。session 里写的键（gh_state /
// gh_cipher_b64 / gh_redirect_uri / gh_code_verifier / gh_state_expire）与 POST 路径完全一致，
// 所以 callback handler 零改动——它只从 session 读暂存，不关心是谁写进去的。
//
// 只有 admin 自己没在 conf/github.conf 填 default_client_id / default_client_secret /
// default_redirect_uri 时这个路由才不可用（default_app_ready 返回 false）。
#include <string>

#include "acme_crypto.hpp"
#include "func.h"
#include "github_config.h"
#include "github_quickbind.h"
#include "github_oauth.h"
#include "github_store.h"
#include "httppeer.h"

namespace http
{
//@urlpath(admin_islogin,github/quickbind)
std::string github_quickbind(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto cfg = github::read_github_settings();
    if (!github::default_app_ready(cfg))
    {
        // 服务器没配默认 OAuth App → 回列表页，显示"管理员还没准备好"。
        client.goto_url("/github/list", 3, "admin has not configured the default GitHub OAuth App yet");
        return "";
    }

    unsigned long long adminid      = (unsigned long long)client.session["adminid"].to_int();
    const std::string &client_id    = cfg.default_client_id;
    const std::string &plain_secret = cfg.default_client_secret;
    const std::string &redirect_uri = cfg.default_redirect_uri;

    std::string cipher;
    std::string error;
    if (!github::encrypt_secret_for_store(cfg, plain_secret, cipher, error))
    {
        // master key 刚才 default_app_ready 已经通过了，这一趟加密失败只可能是密文超长或随机源崩——
        // 后者比前者概率更低。不论哪种，都回列表页打一行，让管理员知道 quickbind 这条路径坏了。
        client.goto_url("/github/list", 3, "encrypt default client_secret failed: " + error);
        return "";
    }

    std::string state = github::make_state(adminid, client_id);
    if (state.empty())
    {
        client.goto_url("/github/list", 3, "random source failed");
        return "";
    }
    std::string verifier  = github::make_code_verifier();
    std::string challenge = verifier.empty() ? std::string("") : github::pkce_challenge(verifier);

    unsigned long long now = (unsigned long long)timeid();

    client.session["gh_state"]         = state;
    client.session["gh_state_expire"]  = now + cfg.state_ttl;
    client.session["gh_code_verifier"] = verifier;
    client.session["gh_cipher_b64"]    = b64url::encode(cipher);
    client.session["gh_redirect_uri"]  = redirect_uri;
    client.save_session();

    std::string url;
    if (!github::build_authorize_url(cfg.auth_base,
                                     client_id,
                                     redirect_uri,
                                     cfg.default_scopes,
                                     state,
                                     challenge,
                                     url))
    {
        // 和 bind 路径同一条：拼不出 authorize URL → 把刚写下去的暂存一起抹掉，别在 session 里悬到过期。
        client.session["gh_state"]         = "";
        client.session["gh_state_expire"]  = 0;
        client.session["gh_code_verifier"] = "";
        client.session["gh_cipher_b64"]    = "";
        client.session["gh_redirect_uri"]  = "";
        client.save_session();
        client.goto_url("/github/list", 3, "cannot build authorize url (check auth_base in conf)");
        return "";
    }

    client.status(302);
    client.set_header("Location", url);
    return "";
}
}// namespace http
