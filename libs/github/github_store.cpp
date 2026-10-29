// github_account / github_repo 的读写。
// 这一层只搬数据：不发 HTTP、不解密之外的业务判断、也不把库的错误原文往外传。
//
// 为什么这一层的语句一律走 exec_*（预编译）而不是文本协议：
// client_secret 那一列存的是 AES-GCM 的密文，里面是任意字节。文本协议那条路的转义
// 会把值首尾的空格/Tab/CR/LF 一起削掉（obj_val::to_sql_mysql_escape 先 trim 再转义），
// 于是大约每 25 枚密文里就有一枚被削短一两个字节——表现是"绑定时好、下次解不开"。
// 预编译走参数绑定，长度由协议自己带着，二进制原样进原样出。
#include "github_store.h"

#include <algorithm>
#include <exception>

#include "cost_define.h"
#include "github_crypto.h"
#include "orm.h"

namespace http
{
namespace github
{

namespace
{
// 一行的列搬到搬运形状；生成的 meta 里 tinyint 落在 char、smallint 落在短整型，
// 这里一次性收到调用方用得上的宽度。
void fill_account(const orm::cms::github_account_info::meta &row, gh_account &out)
{
    out.id                  = (unsigned long long)row.id;
    out.adminid             = (unsigned long long)row.adminid;
    out.companyid           = (unsigned long long)row.companyid;
    out.client_id           = row.client_id;
    out.client_secret       = row.client_secret;
    out.redirect_uri        = row.redirect_uri;
    out.gh_user_id          = row.gh_user_id;
    out.gh_login            = row.gh_login;
    out.gh_avatar           = row.gh_avatar;
    out.access_token        = row.access_token;
    out.refresh_token       = row.refresh_token;
    out.scopes              = row.scopes;
    out.access_expires_at   = row.access_expires_at;
    out.refresh_expires_at  = row.refresh_expires_at;
    out.token_state         = (unsigned char)row.token_state;
    out.isopen              = (unsigned char)row.isopen;
    out.next_sync_time      = row.next_sync_time;
    out.last_sync_time      = row.last_sync_time;
    out.last_sync_ok        = (unsigned char)row.last_sync_ok;
    out.sync_fails          = (unsigned long long)row.sync_fails;
    out.last_error          = row.last_error;
}

// 状态 + 错误文本这一对写法的公共部分。id 为 0 时不发语句——
// exec_update 在没有 where 且主键不大于 0 时会拒绝执行并把 iserror 立起来，白搭一次连接。
bool write_state(unsigned long long id, unsigned char state, const std::string &error)
{
    if (id == 0)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.data.token_state = state;
    model.data.last_error  = scrub_error_text(error);
    model.where("id", id);
    model.exec_update("token_state,last_error");
    return model.iserror == 0;
}
} // namespace

bool encrypt_secret_for_store(const github_settings &cfg,
                              const std::string     &plain,
                              std::string           &cipher_out,
                              std::string           &error)
{
    cipher_out.clear();
    if (!cfg.key_ok)
    {
        error = "crypto key not configured";
        return false;
    }
    if (plain.empty())
    {
        error = "client secret empty";
        return false;
    }
    // 明文闸：入口就拒，不留给库截断。
    if (plain.size() > (size_t)cfg.secret_max_len)
    {
        error = "client secret too long";
        return false;
    }
    std::string cipher;
    try
    {
        cipher = gcm_encrypt(plain, cfg.crypto_key);
    }
    catch (const std::exception &)
    {
        // gcm_encrypt 抛的是"随机源失败/钥匙长度不对"，三种都不该降级成明文入库。
        error = "encrypt failed";
        return false;
    }
    // 密文闸：varbinary(255) 装不下就是装不下，别交给库报错。
    if (cipher.size() > (size_t)CONST_GH_CIPHER_MAX_LEN)
    {
        error = "encrypted secret over column width";
        return false;
    }
    cipher_out = std::move(cipher);
    return true;
}

bool decrypt_secret_of(const github_settings &cfg, const gh_account &row, std::string &plain_out, std::string &error)
{
    plain_out.clear();
    if (!cfg.key_ok)
    {
        error = "crypto key not configured";
        return false;
    }
    if (row.client_secret.empty())
    {
        error = "client secret missing";
        return false;
    }
    try
    {
        plain_out = gcm_decrypt(row.client_secret, cfg.crypto_key);
    }
    catch (const std::exception &)
    {
        // tag 校验不过与"库里这格不是密文"都落这里；不把 OpenSSL 的原文往外传。
        plain_out.clear();
        error = "client secret cannot be decrypted";
        return false;
    }
    if (plain_out.empty())
    {
        error = "client secret empty";
        return false;
    }
    return true;
}

bool load_account(unsigned long long id, gh_account &out)
{
    out = gh_account();
    if (id == 0)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.where("id", id);
    if (model.exec_one() == 0 || model.iserror != 0)
    {
        return false;
    }
    fill_account(model.data, out);
    return true;
}

bool load_account_by_key(unsigned long long adminid, const std::string &client_id, gh_account &out)
{
    out = gh_account();
    if (adminid == 0 || client_id.empty())
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.where("adminid", adminid);
    model.where("client_id", client_id);
    if (model.exec_one() == 0 || model.iserror != 0)
    {
        return false;
    }
    fill_account(model.data, out);
    return true;
}

bool list_accounts(unsigned long long companyid, std::vector<gh_account> &out)
{
    out.clear();
    if (companyid == 0)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.where("companyid", companyid);
    model.desc("id");
    model.exec_fetch();
    if (model.iserror != 0)
    {
        return false;
    }
    out.reserve(model.record.size());
    for (auto &row : model.record)
    {
        gh_account one;
        fill_account(row, one);
        out.push_back(std::move(one));
    }
    return true;
}

unsigned long long insert_account(const std::string &client_id,
                                 const std::string &cipher,
                                 const std::string &redirect_uri,
                                 unsigned long long adminid,
                                 unsigned long long companyid,
                                 unsigned long long now)
{
    if (client_id.empty() || adminid == 0)
    {
        return 0;
    }
    auto model   = orm::cms::GithubAccount();
    model.data.adminid      = (unsigned int)adminid;
    model.data.companyid    = (unsigned int)companyid;
    model.data.client_id    = client_id;
    model.data.client_secret = cipher;
    model.data.redirect_uri = redirect_uri;
    model.data.gh_user_id   = 0;
    model.data.gh_login     = "";
    model.data.gh_avatar    = "";
    // 三枚令牌列在这一刻都还没有值。写空串而不是 NULL：库里"有没有令牌"看 token_state，
    // 让 NULL 与 '' 两套真相并存是上一轮 ORM 位图那件事的重演。
    model.data.access_token  = "";
    model.data.refresh_token = "";
    model.data.scopes        = "";
    model.data.access_expires_at  = 0;
    model.data.refresh_expires_at = 0;
    model.data.token_state        = 0;  // 换令牌中
    model.data.isopen             = 1;
    model.data.next_sync_time     = 0;
    model.data.last_sync_time     = 0;
    model.data.last_sync_ok       = 0;
    model.data.sync_fails         = 0;
    model.data.last_error         = "";
    model.data.addtime            = (unsigned int)now;
    model.data.updatetime         = (unsigned int)now;
    auto [effect, newid] = model.exec_insert();
    if (model.iserror != 0 || effect == 0)
    {
        return 0;
    }
    return (unsigned long long)newid;
}

bool set_state(unsigned long long id, unsigned char state, const std::string &error)
{
    return write_state(id, state, error);
}

bool set_cipher(unsigned long long id, const std::string &cipher)
{
    if (id == 0 || cipher.empty() || cipher.size() > (size_t)CONST_GH_CIPHER_MAX_LEN)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.data.client_secret = cipher;
    model.where("id", id);
    model.exec_update("client_secret");
    return model.iserror == 0;
}

bool save_bound(unsigned long long id,
                const std::string  &cipher,
                const gh_user_info &who,
                const token_answer &tk,
                unsigned long long  now)
{
    if (id == 0 || !tk.ok || tk.access_token.empty())
    {
        return false;
    }
    if (tk.access_token.size() > (size_t)CONST_GH_TOKEN_MAX_LEN)
    {
        return false;
    }
    if (!tk.refresh_token.empty() && tk.refresh_token.size() > (size_t)CONST_GH_TOKEN_MAX_LEN)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.data.access_token = tk.access_token;
    // 空串＝这一枚令牌不能刷新（协议第一态），不是"这次没带回、旧的还能用"——
    // 刷新那一拍单独走 save_refreshed，那里才需要保留库里旧的一枚。
    model.data.refresh_token = tk.refresh_token;
    model.data.scopes        = tk.scopes;
    model.data.access_expires_at  = tk.access_expires_in > 0 ? (unsigned int)(now + (unsigned long long)tk.access_expires_in) : 0;
    model.data.refresh_expires_at = tk.refresh_expires_in > 0 ? (unsigned int)(now + (unsigned long long)tk.refresh_expires_in) : 0;
    model.data.token_state        = 1;
    model.data.next_sync_time     = (unsigned int)now;
    model.data.last_sync_ok       = 0;
    model.data.last_error         = "";
    model.data.updatetime         = (unsigned int)now;
    model.data.gh_user_id         = who.user_id;
    model.data.gh_login           = who.login;
    model.data.gh_avatar          = who.avatar;
    if (model.data.gh_avatar.size() > (size_t)CONST_GH_AVATAR_MAX_LEN)
    {
        model.data.gh_avatar.erase(CONST_GH_AVATAR_MAX_LEN);
    }

    std::string fields =
        "access_token,refresh_token,scopes,access_expires_at,refresh_expires_at,token_state,next_sync_time,"
        "last_sync_ok,last_error,updatetime,gh_user_id,gh_login,gh_avatar";
    if (!cipher.empty() && cipher.size() <= (size_t)CONST_GH_CIPHER_MAX_LEN)
    {
        // 重绑时把新 secret 一起覆盖掉：轮换过密钥的 app 只有靠这一次写入才能救回来。
        model.data.client_secret = cipher;
        fields                   = "client_secret," + fields;
    }
    model.where("id", id);
    model.exec_update(fields);
    return model.iserror == 0;
}

bool save_refreshed(unsigned long long id, const token_answer &tk, unsigned long long now)
{
    if (id == 0 || !tk.ok || tk.access_token.empty())
    {
        return false;
    }
    if (tk.access_token.size() > (size_t)CONST_GH_TOKEN_MAX_LEN)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.data.access_token = tk.access_token;
    model.data.scopes       = tk.scopes;
    // 刷新回来的时长是"从现在起"，所以到期时间一律按 now 重算——库里那一格是上一轮的 now+ttl，
    // 直接沿用的话每次刷新都只往前推一点点，实际寿命比协议写的短。
    model.data.access_expires_at  = tk.access_expires_in > 0 ? (unsigned int)(now + (unsigned long long)tk.access_expires_in) : 0;
    model.data.refresh_expires_at = tk.refresh_expires_in > 0 ? (unsigned int)(now + (unsigned long long)tk.refresh_expires_in) : 0;
    model.data.token_state        = 1;
    model.data.last_error         = "";
    model.data.updatetime         = (unsigned int)now;

    std::string fields =
        "access_token,scopes,access_expires_at,refresh_expires_at,token_state,last_error,updatetime";
    if (!tk.refresh_token.empty())
    {
        if (tk.refresh_token.size() > (size_t)CONST_GH_TOKEN_MAX_LEN)
        {
            return false;
        }
        // 带回新的一枚才一起写；没带回时这一列根本不进 SET，库里那枚继续可用。
        model.data.refresh_token = tk.refresh_token;
        fields                   = "refresh_token," + fields;
    }
    model.where("id", id);
    model.exec_update(fields);
    return model.iserror == 0;
}

void presweep_expiring(unsigned long long now,
                       unsigned long long near_expiry,
                       unsigned int      &flipped_to_refresh,
                       unsigned int      &flipped_dead)
{
    flipped_to_refresh = 0;
    flipped_dead       = 0;
    if (near_expiry <= now)
    {
        return;
    }
    // 1 → 2：临近过期且手里有 refresh。有 refresh 的判据用"这一列非空串"，
    // 不用 IS NOT NULL——占位行写入的是 ''，两者都要算"没有 refresh"。
    {
        auto model = orm::cms::GithubAccount();
        model.data.token_state = 2;
        model.where("token_state", 1);
        model.where("access_expires_at", orm::wq::bt, 0);
        model.where("access_expires_at", orm::wq::le, (unsigned int)near_expiry);
        model.whereNE("refresh_token", "");
        model.exec_update("token_state");
        if (model.iserror == 0)
        {
            flipped_to_refresh = model.effect();
        }
    }
    // 1 → 3：已过期且没有 refresh，永久死票。不翻它就会每一拍都捞到同一行、每一拍都不收敛。
    {
        auto model = orm::cms::GithubAccount();
        model.data.token_state = 3;
        model.data.last_error  = "expired without refresh";
        model.where("token_state", 1);
        model.where("access_expires_at", orm::wq::bt, 0);
        model.where("access_expires_at", orm::wq::le, (unsigned int)now);
        model.where("refresh_token", "");
        model.exec_update("token_state,last_error");
        if (model.iserror == 0)
        {
            flipped_dead = model.effect();
        }
    }
}

bool fetch_due_batch(unsigned long long now, unsigned int limit_n, std::vector<gh_account> &out)
{
    out.clear();
    if (limit_n == 0)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    // 状态 0（换令牌中）与 4（解绑/失败）天然不进批：前者还没令牌，后者不该再打 GitHub。
    model.whereIn("token_state", "1,2");
    model.where("isopen", 1);
    model.where("next_sync_time", orm::wq::le, (unsigned int)now);
    model.asc("next_sync_time");
    model.limit(0, limit_n);
    model.exec_fetch();
    if (model.iserror != 0)
    {
        return false;
    }
    out.reserve(model.record.size());
    for (auto &row : model.record)
    {
        gh_account one;
        fill_account(row, one);
        out.push_back(std::move(one));
    }
    return true;
}

bool push_next_sync(unsigned long long id, unsigned long long when)
{
    if (id == 0)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.data.next_sync_time = (unsigned int)when;
    model.where("id", id);
    model.exec_update("next_sync_time");
    return model.iserror == 0;
}

bool record_success(unsigned long long id, unsigned long long now, unsigned long long next, const std::string &note)
{
    if (id == 0)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.data.last_sync_time = (unsigned int)now;
    model.data.last_sync_ok   = 1;
    model.data.sync_fails     = 0;
    model.data.next_sync_time = (unsigned int)next;
    // note 非空＝这一轮翻页被页数上界截断，清单可能不完整；空串把上一轮的旧笔记抹掉，
    // 否则一次截断会一直挂在页面上，看起来像"每次都缺"。
    model.data.last_error     = scrub_error_text(note);
    model.data.updatetime     = (unsigned int)now;
    model.where("id", id);
    model.exec_update("last_sync_time,last_sync_ok,sync_fails,next_sync_time,last_error,updatetime");
    return model.iserror == 0;
}

namespace
{
// 指数退避的间隔本身：fails 每加一翻一倍，封顶在 cfg.sync_backoff_max。
// fails 先钳到 16 再移位——那一格是库里的数，越界之前必须先变成有界的。
unsigned long long backoff_seconds(unsigned long long fails, const github_settings &cfg)
{
    unsigned long long base = cfg.sync_interval > 0 ? cfg.sync_interval : (unsigned long long)CONST_GH_SYNC_INTERVAL;
    unsigned long long cap  = cfg.sync_backoff_max > 0 ? cfg.sync_backoff_max : (unsigned long long)CONST_GH_SYNC_BACKOFF_MAX;
    unsigned long long step = fails > 16 ? 16 : fails;
    unsigned long long wait = base << step;
    return wait > cap ? cap : wait;
}

// sync_fails 那一列是 smallint unsigned；钳在列的界里再写，别让它回绕。
// 回绕的代价不是"数字难看"，是退避档位会从最长那一档掉回最短那一档，正好在故障期打得更勤。
unsigned int clamped_fails(unsigned long long fails)
{
    return fails > (unsigned long long)CONST_GH_SYNC_FAILS_MAX ? (unsigned int)CONST_GH_SYNC_FAILS_MAX
                                                                : (unsigned int)fails;
}
} // namespace

bool record_fail(unsigned long long id, const github_settings &cfg, unsigned long long now, const std::string &error)
{
    if (id == 0)
    {
        return false;
    }
    gh_account row;
    if (!load_account(id, row))
    {
        return false;
    }
    unsigned long long fails  = row.sync_fails + 1;
    unsigned long long next   = now + backoff_seconds(fails, cfg);
    auto              model   = orm::cms::GithubAccount();
    model.data.sync_fails     = clamped_fails(fails);
    model.data.next_sync_time = (unsigned int)next;
    model.data.last_sync_ok   = 0;
    model.data.last_error     = scrub_error_text(error);
    model.data.updatetime     = (unsigned int)now;
    model.where("id", id);
    model.exec_update("sync_fails,next_sync_time,last_sync_ok,last_error,updatetime");
    return model.iserror == 0;
}

bool record_rate_limited(unsigned long long id, unsigned long long now, long long reset_at, const github_settings &cfg)
{
    if (id == 0)
    {
        return false;
    }
    // 限流不是错误，是排队：推到 Reset 之后再拍下一轮。Reset 缺失或已经过去时
    // 按一个正常间隔推，别把它当 0 用（那等于"下一拍再来一次"，正好撞上额度没恢复的窗口）。
    unsigned long long base   = cfg.sync_interval > 0 ? cfg.sync_interval : (unsigned long long)CONST_GH_SYNC_INTERVAL;
    unsigned long long next   = now + base;
    if (reset_at > (long long)now)
    {
        next = (unsigned long long)reset_at + 1;
    }
    gh_account row;
    if (!load_account(id, row))
    {
        return false;
    }
    unsigned long long fails = row.sync_fails + 1;
    auto              model  = orm::cms::GithubAccount();
    model.data.sync_fails     = clamped_fails(fails);
    model.data.next_sync_time = (unsigned int)next;
    model.data.last_error     = scrub_error_text("rate limited");
    model.data.updatetime     = (unsigned int)now;
    model.where("id", id);
    model.exec_update("sync_fails,next_sync_time,last_error,updatetime");
    return model.iserror == 0;
}

bool mark_token_dead(unsigned long long id, const std::string &error)
{
    return write_state(id, 3, error);
}

bool mark_unbinding(unsigned long long id)
{
    return write_state(id, 4, "unbinding");
}

bool delete_account(unsigned long long id)
{
    if (id == 0)
    {
        return false;
    }
    auto model = orm::cms::GithubAccount();
    model.where("id", id);
    model.exec_remove();
    if (model.iserror != 0)
    {
        return false;
    }
    // 仓库缓存跟着一起删：这一行的令牌已经没了，留着的清单只会让人以为还在同步。
    auto repos = orm::cms::GithubRepo();
    repos.where("account_id", id);
    repos.exec_remove();
    return repos.iserror == 0;
}

bool upsert_repos(unsigned long long account_id, const std::vector<gh_repo_item> &items, unsigned long long now,
                  unsigned int &written)
{
    written = 0;
    if (account_id == 0 || items.empty())
    {
        return true;
    }
    // 先取一次本账号已有的 repo_id：有了它才不用靠"UPDATE 影响 0 行"来判新行——
    // MySQL 对"值没变"的更新同样报 0，靠它路由会把一条没变化的仓库重复插一遍，
    // 然后撞 uq_account_repo。
    std::vector<unsigned long long> existing;
    {
        auto model = orm::cms::GithubRepo();
        model.select("gh_repo_id");
        model.where("account_id", account_id);
        model.exec_fetch();
        if (model.iserror != 0)
        {
            return false;
        }
        existing.reserve(model.record.size());
        for (auto &row : model.record)
        {
            existing.push_back(row.gh_repo_id);
        }
    }

    auto model = orm::cms::GithubRepo();
    for (auto &item : items)
    {
        bool is_new = std::find(existing.begin(), existing.end(), item.gh_repo_id) == existing.end();
        model.clear();
        model.data.full_name         = item.full_name;
        model.data.is_private        = item.is_private ? 1 : 0;
        model.data.description       = item.description;
        if (model.data.description.size() > (size_t)CONST_GH_REPO_DESC_MAX_LEN)
        {
            model.data.description.erase(CONST_GH_REPO_DESC_MAX_LEN);
        }
        model.data.primary_language = item.primary_language;
        model.data.stars            = (unsigned int)item.stars;
        model.data.pushed_at        = item.pushed_at;
        model.data.synced_at        = (unsigned int)now;
        // 上一轮被标成已删除、这一轮又出现了（改名、转回可见）：跟着把旗收回。
        model.data.is_deleted       = 0;
        model.data.deleted_at       = 0;
        if (is_new)
        {
            model.data.account_id = (unsigned int)account_id;
            model.data.gh_repo_id = item.gh_repo_id;
            model.exec_insert();
            existing.push_back(item.gh_repo_id);
        }
        else
        {
            model.where("account_id", account_id);
            model.where("gh_repo_id", item.gh_repo_id);
            model.exec_update("full_name,is_private,description,primary_language,stars,pushed_at,synced_at,"
                              "is_deleted,deleted_at");
        }
        if (model.iserror != 0)
        {
            return false;
        }
        written++;
    }
    return true;
}

bool soft_delete_missing(unsigned long long account_id, const std::vector<unsigned long long> &present_ids,
                         unsigned long long now, unsigned int &marked)
{
    marked = 0;
    if (account_id == 0)
    {
        return false;
    }
    // 本账号当前未删的行。present_ids 是"本轮自然翻完拉到的全集"，两者取差就是"这轮没出现"。
    std::vector<unsigned long long> live;
    {
        auto model = orm::cms::GithubRepo();
        model.select("gh_repo_id");
        model.where("account_id", account_id);
        model.where("is_deleted", 0);
        model.exec_fetch();
        if (model.iserror != 0)
        {
            return false;
        }
        live.reserve(model.record.size());
        for (auto &row : model.record)
        {
            live.push_back(row.gh_repo_id);
        }
    }
    if (live.empty())
    {
        return true;
    }
    std::vector<unsigned long long> missing;
    for (auto id : live)
    {
        if (std::find(present_ids.begin(), present_ids.end(), id) == present_ids.end())
        {
            missing.push_back(id);
        }
    }
    if (missing.empty())
    {
        return true;
    }
    auto model = orm::cms::GithubRepo();
    model.data.is_deleted = 1;
    model.data.deleted_at = (unsigned int)now;
    if (present_ids.empty())
    {
        // 本轮一条都没拉到（用户在 GitHub 上删光了，或转成了我们看不到 private 的 scope）。
        // 这里不能拼 NOT IN ()：空列表是非法 SQL，而语义就是"全部缺席"。
        model.where("account_id", account_id);
        model.where("is_deleted", 0);
    }
    else
    {
        model.where("account_id", account_id);
        model.whereIn("gh_repo_id", orm::wq::notin, missing);
    }
    model.exec_update("is_deleted,deleted_at");
    if (model.iserror != 0)
    {
        return false;
    }
    marked = model.effect();
    return true;
}

bool list_repos(unsigned long long account_id, bool show_deleted, std::vector<gh_repo_row> &out)
{
    out.clear();
    if (account_id == 0)
    {
        return false;
    }
    auto model = orm::cms::GithubRepo();
    model.where("account_id", account_id);
    if (!show_deleted)
    {
        model.where("is_deleted", 0);
    }
    model.desc("stars");
    model.exec_fetch();
    if (model.iserror != 0)
    {
        return false;
    }
    out.reserve(model.record.size());
    for (auto &row : model.record)
    {
        gh_repo_row one;
        one.item.gh_repo_id        = row.gh_repo_id;
        one.item.full_name         = row.full_name;
        one.item.is_private        = row.is_private != 0;
        one.item.description       = row.description;
        one.item.primary_language  = row.primary_language;
        one.item.stars             = row.stars;
        one.item.pushed_at         = row.pushed_at;
        one.is_deleted             = row.is_deleted != 0;
        out.push_back(std::move(one));
    }
    return true;
}

} // namespace github
} // namespace http
