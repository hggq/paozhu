// 绑定：表单提交 → 暂存凭据 → 302 到 GitHub。
//
// 这一页只有 POST 一条路：表单和列表共用 view/github/list.html，GET 过来直接回列表页，
// 免得同一段"读列表"的代码在两个文件里各写一份（两份会漂）。
//
// client_secret 不在这里落库。此刻还没换得 code，这组凭据能不能用是未知的；用户关掉浏览器
// 就该什么都不留下——session 里的暂存值随会话过期自然消失，比"库里多一坨垃圾凭据 + 一个清理任务"便宜。
//
// 暂存前先 b64url：密文是原始二进制，而 session 的序列化只把 0x00-0x1F 转成 \u00XX，
// 其余字节原样写出，落盘就是一串非法 UTF-8。
#include <string>

#include "acme_crypto.hpp"
#include "cost_define.h"
#include "datetime.h"
#include "func.h"
#include "github_bind.h"
#include "github_config.h"
#include "github_oauth.h"
#include "github_store.h"
#include "http_header.h"
#include "httppeer.h"

namespace http
{
//@urlpath(admin_islogin,github/bind)
std::string github_bind(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    if (client.method != static_cast<unsigned char>(HEAD_METHOD::POST))
    {
        client.goto_url("/github/list");
        return "";
    }

    auto cfg = github::read_github_settings();
    if (!cfg.key_ok)
    {
        // 没有可用的 master key 就加密不了 client_secret。宁可拒服务，也不要落一行解不开的密文。
        client.goto_url("/github/list", 3, "server has no crypto key configured");
        return "";
    }

    unsigned long long adminid = (unsigned long long)client.session["adminid"].to_int();

    std::string client_id   = str_trim(client.post["client_id"].to_string());
    std::string plain_secret = client.post["client_secret"].to_string();
    std::string redirect_uri = str_trim(client.post["redirect_uri"].to_string());

    if (client_id.empty() || plain_secret.empty() || redirect_uri.empty())
    {
        client.goto_url("/github/list", 3, "client_id / client_secret / redirect_uri are all required");
        return "";
    }
    // 三把长度闸各自对着一列：超了宁可拒收，也不要 MySQL 截断——
    // client_id 被截就永远查不到这一行，secret 被截就换不成令牌，redirect_uri 被截每次都以同一个错失败。
    if (client_id.size() > (size_t)CONST_GH_CLIENT_ID_MAX_LEN)
    {
        client.goto_url("/github/list", 3, "client_id too long");
        return "";
    }
    if (redirect_uri.size() > (size_t)CONST_GH_REDIRECT_MAX_LEN)
    {
        client.goto_url("/github/list", 3, "redirect_uri too long");
        return "";
    }
    if (plain_secret.size() > (size_t)cfg.secret_max_len)
    {
        client.goto_url("/github/list", 3, "client_secret too long");
        return "";
    }

    std::string cipher;
    std::string error;
    if (!github::encrypt_secret_for_store(cfg, plain_secret, cipher, error))
    {
        plain_secret.clear();
        client.goto_url("/github/list", 3, "encrypt client_secret failed");
        return "";
    }
    plain_secret.clear();

    std::string state = github::make_state(adminid, client_id);
    if (state.empty())
    {
        client.goto_url("/github/list", 3, "random source failed");
        return "";
    }
    // PKCE：随机源失败就退成"这一趟不带 PKCE"，而不是降级到时间戳拼一枚看起来像 verifier 的东西。
    std::string verifier  = github::make_code_verifier();
    std::string challenge = verifier.empty() ? std::string("") : github::pkce_challenge(verifier);

    unsigned long long now = (unsigned long long)timeid();

    client.session["gh_state"]         = state;
    client.session["gh_state_expire"]  = now + cfg.state_ttl;
    client.session["gh_code_verifier"] = verifier;
    client.session["gh_cipher_b64"]    = b64url::encode(cipher);
    client.session["gh_redirect_uri"]  = redirect_uri;
    client.save_session();
    // 明文口令级的东西只留在 session 里那一枚密文，栈上这份现在已经是垃圾。
    cipher.clear();

    std::string url;
    if (!github::build_authorize_url(cfg.auth_base,
                                     client_id,
                                     redirect_uri,
                                     cfg.default_scopes,
                                     state,
                                     challenge,
                                     url))
    {
        // 拼不出 authorize URL 时把刚写下的暂存一并抹掉，否则这一组 state 会在 session 里
        // 悬到过期，用户下一次提交又叠一层。
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
} // namespace http
