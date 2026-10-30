#ifndef __GITHUB_STORE_H__
#define __GITHUB_STORE_H__

#include <string>
#include <vector>

#include "github_api.h"
#include "github_config.h"
#include "github_oauth.h"

namespace http
{
namespace github
{

// github_account 一行的搬运形状。
// 为什么不用生成的 meta 直接当参数类型：meta 的字段是库的列（tinyint→char、
// smallint→短整型），调用方拿到它们就得自己补类型转换与越界处理；这一格只把
// 用得上的几列摆成稳定的类型，读写的语句留在本文件里。
struct gh_account
{
    unsigned long long id        = 0;
    unsigned long long adminid   = 0;
    unsigned long long companyid = 0;
    std::string client_id;
    // AES-256-GCM 的密文原文（IV+tag+正文）。它只在栈上停留，绝不写日志、绝不回显。
    std::string client_secret;
    std::string redirect_uri;

    unsigned long long gh_user_id = 0;
    std::string gh_login;
    std::string gh_avatar;

    std::string access_token;
    std::string refresh_token;
    std::string scopes;
    // 0 = 永不过期（协议第一态）/ 没有 refresh，两种含义各自列里都有说明。
    unsigned long long access_expires_at  = 0;
    unsigned long long refresh_expires_at = 0;

    // 0=换令牌中 1=可用 2=将/已过期且有 refresh 3=死票 4=解绑或换令牌失败
    unsigned char token_state = 0;
    unsigned char isopen      = 1;

    unsigned long long next_sync_time = 0;
    unsigned long long last_sync_time = 0;
    unsigned char last_sync_ok        = 0;
    unsigned long long sync_fails     = 0;
    std::string last_error;
};

// github_repo 的一行给页面看的形状：软删旗要跟数据一起走，否则"显示已删除"开关没法渲染。
struct gh_repo_row
{
    gh_repo_item item;
    bool is_deleted = false;
};

// ---------------------------------------------------------------------------
// client_secret 的两把闸 + 加解密
// ---------------------------------------------------------------------------

// 明文过 CONST_GH_SECRET_MAX_LEN（入口闸），密文过 CONST_GH_CIPHER_MAX_LEN（列宽闸）。
// 密文 = 明文 + 12(IV) + 16(tag)，所以两把闸必须都量：只量明文的话 128 字节的明文
// 出 156 字节的密文，列装得下；但有人把列改成更短或把明文闸调大时，只有密文闸拦得住截断。
// cfg.key_ok 为 false 时直接失败——不落一条"加密失败但存了空串"的行。
bool encrypt_secret_for_store(const github_settings &cfg,
                              const std::string &plain,
                              std::string &cipher_out,
                              std::string &error);

bool decrypt_secret_of(const github_settings &cfg,
                       const gh_account &row,
                       std::string &plain_out,
                       std::string &error);

// ---------------------------------------------------------------------------
// 账号表
// ---------------------------------------------------------------------------

// 查不到返回 false（out 会被清干净），调用方据此区分"没有这一行"和"有但停用"。
bool load_account(unsigned long long id, gh_account &out);
bool load_account_by_key(unsigned long long adminid, const std::string &client_id, gh_account &out);

// 列表页：只看本租户的行。companyid 取自 session["userid"]，不是 session["companyid"]。
bool list_accounts(unsigned long long companyid, std::vector<gh_account> &out);

// 占位行：token_state=0（换令牌中）。返回新行 id，0=没插进去。
// 为什么先插再换：换令牌成功但落库崩了，那枚令牌就永远没人知道它存在；
// 反过来先留一条"令牌中"的痕迹，失败了它还在，排查有东西可看。
unsigned long long insert_account(const std::string &client_id,
                                  const std::string &cipher,
                                  const std::string &redirect_uri,
                                  unsigned long long adminid,
                                  unsigned long long companyid,
                                  unsigned long long now);

// 只改状态与 last_error；换令牌失败与"用户拒绝"两条出口用它。
bool set_state(unsigned long long id, unsigned char state, const std::string &error);

// 覆盖某一行的密文（重绑时把新 secret 写进去）。轮换密钥后只能靠这一条救回来。
bool set_cipher(unsigned long long id, const std::string &cipher);

// 6g 的最终态：令牌、scope、两枚到期时间、state=1，并把 next_sync_time 记为 now
// （下一拍就同步一次）。cipher 传空串＝不覆盖这一列（重绑失败恢复时用它保旧密文）。
bool save_bound(unsigned long long id,
                const std::string &cipher,
                const gh_user_info &who,
                const token_answer &tk,
                unsigned long long now);

// 定时那一拍刷新成功后的写回：新的 access、两枚到期时间、scope，state 归 1。
// 响应没带回 refresh 时（GitHub 会轮换，但没带回归档也要能跑）保留库里那一枚——
// 把它写成空串等于下一拍再也刷不动，好端端的一枚票被我们自己做死。
bool save_refreshed(unsigned long long id, const token_answer &tk, unsigned long long now);

// 每拍的预扫：把临近过期的活票翻成 2（拉进本批优先刷），把已过期且无 refresh 的翻成 3
// （永久死票——不翻就会每一拍都捞到它，永远不收敛）。
// near_expiry = access_expires_at 的非 0 上界（now + 间隔）。两个计数出参给日志用。
void presweep_expiring(unsigned long long now,
                       unsigned long long near_expiry,
                       unsigned int &flipped_to_refresh,
                       unsigned int &flipped_dead);

// 到期批：token_state IN (1,2) AND isopen=1 AND next_sync_time<=now，按 next_sync_time 升序，
// 最多 limit_n 行（一轮最多 N 个账号串着跑）。
bool fetch_due_batch(unsigned long long now, unsigned int limit_n, std::vector<gh_account> &out);

// 先推再做：这一轮中途崩了，下一轮不会又捞到同一批，把一次失败变成反复重试。
bool push_next_sync(unsigned long long id, unsigned long long when);

// 成功记账：last_sync_ok=1、sync_fails=0、last_sync_time=now、next_sync_time=next。
// note 用来记"这一轮翻页被截断，仓库清单可能不完整"（C-11 甲）——正常成功传空串，清空旧笔记。
bool record_success(unsigned long long id,
                    unsigned long long now,
                    unsigned long long next,
                    const std::string &note);

// 失败记账：sync_fails+1，next_sync_time 按 fails 指数推后，封顶 cfg.sync_backoff_max。
bool record_fail(unsigned long long id, const github_settings &cfg, unsigned long long now, const std::string &error);

// 限流记账：推到 Reset 之后（Reset 不可信或已过去时按一个间隔推）。
// 它也算 sync_fails+1——限流不是错误但它是"这一轮没干完"，退避同一个数不会有害。
bool record_rate_limited(unsigned long long id,
                         unsigned long long now,
                         long long reset_at,
                         const github_settings &cfg);

// 401：这枚票在 GitHub 侧已经没了，定时任务再打一百次也不会自己活。
bool mark_token_dead(unsigned long long id, const std::string &error);

// 解绑第一步：先 UPDATE 4，让定时任务的 SELECT IN (1,2) 自然跳过这一行。
bool mark_unbinding(unsigned long long id);

// 只有 GitHub 侧确认吊销（或本来就没有）之后才允许调它。
bool delete_account(unsigned long long id);

// ---------------------------------------------------------------------------
// 仓库表
// ---------------------------------------------------------------------------

// 逐条 UPDATE ... WHERE account_id AND gh_repo_id；改动为 0 行才 INSERT。
// 不先读一遍表：那要多一次查询，而"改不动就插"这一条已经把 upsert 覆盖完了。
// 更新的列里带 is_deleted=0、deleted_at=0——用户在 GitHub 上把仓库改回可见，这一行要跟着回来。
bool upsert_repos(unsigned long long account_id,
                  const std::vector<gh_repo_item> &items,
                  unsigned long long now,
                  unsigned int &written);

// 把"表里有、本轮没拉到"的行标成已删除。present_ids 为本轮拉到的 gh_repo_id 全集。
// 调用方必须先确认这一轮自然翻完了（最后一页条数 < per_page）——被页数上界截断的一轮
// 里"没拉到"是截断，不是删除，标下去就是"第 301 个以后的仓库凭空消失"。
bool soft_delete_missing(unsigned long long account_id,
                         const std::vector<unsigned long long> &present_ids,
                         unsigned long long now,
                         unsigned int &marked);

bool list_repos(unsigned long long account_id, bool show_deleted, std::vector<gh_repo_row> &out);

}// namespace github
}// namespace http
#endif
