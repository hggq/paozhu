#ifndef __GITHUB_OAUTH_H__
#define __GITHUB_OAUTH_H__

#include <string>
#include <vector>

#include "github_config.h"

namespace http
{
namespace github
{

// ---------------------------------------------------------------------------
// 纯函数部分：URL 拼装、票据形状、响应解析。这几块是离线判据台真正测的东西。
// ---------------------------------------------------------------------------

// conf 里的 default_scopes 用逗号存，GitHub 的授权请求要空格分隔（响应体里那个
// scope=repo%2Cgist 的逗号是**返回值**的写法，不能反过来当请求格式抄）。
// read:user 无条件附加：没有它 GET /user 回 403，身份三列永远填不上。
std::string normalize_scopes(const std::string &scopes_comma);

// 302 出去的那条 authorize URL。auth_base 过不了 https 闸、必填项为空、或 scope 空的
// 时候返回 false 且 url_out 被清空——调用方据此拒绝跳转。
// scopes_comma 传 conf 里的逗号写法，内部过 normalize_scopes；code_challenge 传空串＝不带 PKCE。
bool build_authorize_url(const std::string &auth_base,
                         const std::string &client_id,
                         const std::string &redirect_uri,
                         const std::string &scopes_comma,
                         const std::string &state,
                         const std::string &code_challenge,
                         std::string       &url_out);

// 三段 [adminid, client_id, rand32] 各自 base64url 后用 '.' 连接。
// 原始随机字节绝不能直接和分隔符拼（32 字节里出现 0x7C 的概率约 12%，一旦命中就破坏分段）；
// base64url 的字母表 A-Za-z0-9-_ 不含 '.'，所以 split('.') 恰好三段。
std::string make_state(unsigned long long adminid, const std::string &client_id);

// state 的形状校验：切出恰好三段、第一段解出全是数字的 adminid、第二段解出非空且只含
// [A-Za-z0-9_-] 且不超过 client_id 列宽的 client_id。**不查库**——首次绑定时占位行还没插。
bool state_shape_ok(const std::string &state, unsigned long long &adminid_out, std::string &client_id_out);

// PKCE 的 verifier：64 个 RFC 7636 unreserved 字符（随机源失败返回空串，不降级到时间戳）。
std::string make_code_verifier();

// challenge = b64url(SHA-256(verifier))，GitHub 只支持 S256。
std::string pkce_challenge(const std::string &code_verifier);

// 令牌端点响应的三态解析。is_json 是框架报的"响应 Content-Type 是不是 json"——
// 忘了带 Accept: application/json 时正文是一坨 urlencoded 文本，框架不抛、json 是空的，
// 所以这一格为 false 时必须走失败分支，不能当"取不到 expires_in"处理。
struct token_answer
{
    bool        ok = false;
    std::string access_token;
    // 空 = 这一枚令牌不能刷新（第一态），不是"没解析出来"。
    std::string refresh_token;
    // GitHub 实到的 scope，空格分隔；与申请的可能不同，存实到的这份。
    std::string scopes;
    // 两个时长（秒），由调用方加成绝对到期时间。0 = 永不过期 / 没有 refresh。
    long long access_expires_in  = 0;
    long long refresh_expires_in = 0;
    // 失败原因。只放错误名与 GitHub 的 error 文本，落库前还要过一遍 scrub_error_text。
    std::string error;
};
token_answer parse_token_body(const std::string &body, bool is_json);

// ---------------------------------------------------------------------------
// 出站部分
// ---------------------------------------------------------------------------

// 拿 code 换令牌（form + Accept: application/json）。
// code_verifier 传空串＝这一趟不带 PKCE（现场核 P-1 判定不支持时的退路）。
// client_secret_plain 只存活于这一次调用的栈帧：绝不写日志、绝不塞进结构体。
bool exchange_code(const github_settings &cfg,
                   const std::string     &client_id,
                   const std::string     &client_secret_plain,
                   const std::string     &code,
                   const std::string     &redirect_uri,
                   const std::string     &code_verifier,
                   token_answer          &out);

// 用 refresh_token 续命。成功时 out.refresh_token 可能带回新的一枚（GitHub 会轮换），
// 空串表示响应没带回、旧的仍然可用——调用方据此决定要不要一起写两列。
bool refresh_access(const github_settings &cfg,
                    const std::string     &client_id,
                    const std::string     &client_secret_plain,
                    const std::string     &refresh_token,
                    token_answer          &out);

// 吊销 GitHub 侧的令牌。GitHub 的 revoke 端点只吃 DELETE + HTTP Basic，所以这一条
// 依赖框架 client 的 del() 动词。
enum class revoke_outcome
{
    revoked,      // 204/200：GitHub 侧已经没有这枚令牌了
    already_gone, // 404/422：本来就没有（用户自己在后台撤过、或 secret 已换）——重试永远失败
    failed        // 5xx/超时/网络错：本地行必须保留，等重试
};
revoke_outcome revoke_token(const github_settings &cfg,
                            const std::string     &client_id,
                            const std::string     &client_secret_plain,
                            const std::string     &access_token,
                            unsigned int          &status_out);

} // namespace github
} // namespace http
#endif
