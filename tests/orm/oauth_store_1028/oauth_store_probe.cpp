// 三张 oauth 表的读写在线探针：连 conf/orm.conf 的 [cms] 真库，跑 libs/oauth/oauth_store.cpp
// 里碰库的那批函数（client 读写、授权码、refresh 轮换与撤销、过期行清理）。
//
// 数据面纪律：这个台子只往 oauth_client / oauth_code / oauth_refresh 写，且每一行的
// client_id 都以 "probe-" 开头；开头断言无残留、结尾按前缀删干净并再断言归零。这三张表
// 目前只有本功能会写，但**别**把清理改成"全表 delete"——将来表里有真客户端时那就成了删生产数据。
#include <asio.hpp>

#include <atomic>
#include <cstdio>
#include <ctime>
#include <string>
#include <thread>
#include <vector>

#include "acme_crypto.hpp"
#include "cost_define.h"
#include "oauth_store.h"
#include "orm.h"
#include "orm_conn_pool.h"

static int g_pass = 0;
static int g_fail = 0;

static void expect(bool ok, const char *name, const std::string &detail = "")
{
    if (ok)
    {
        ++g_pass;
        std::printf("PASS %s\n", name);
    }
    else
    {
        ++g_fail;
        std::printf("FAIL %s %s\n", name, detail.c_str());
    }
}

[[maybe_unused]] static void reading(const char *name, const std::string &v)
{
    std::printf("READ %s=%s\n", name, v.c_str());
}

[[maybe_unused]] static void reading(const char *name, unsigned long long v)
{
    std::printf("READ %s=%llu\n", name, v);
}

[[maybe_unused]] static void reading(const char *name, long long v)
{
    std::printf("READ %s=%lld\n", name, v);
}

namespace oauth = http::oauth;

static const char *kClientA = "probe-client-a";
static const char *kClientB = "probe-client-b";

// ---------------------------------------------------------------------------
// 行数读数（只读，不判据）
// ---------------------------------------------------------------------------
static unsigned long long count_where_client_id()
{
    auto cl = orm::cms::OauthClient();
    unsigned long long c = cl.count();
    if (cl.iserror)
        return 0xFFFFFFFFu;
    return c;
}

static unsigned long long code_rows_for(const std::string &client_id)
{
    auto m = orm::cms::OauthCode();
    m.where("client_id", client_id);
    unsigned long long c = m.count();
    return m.iserror ? 0xFFFFFFFFu : c;
}

static unsigned long long refresh_rows_for(const std::string &client_id)
{
    auto m = orm::cms::OauthRefresh();
    m.where("client_id", client_id);
    unsigned long long c = m.count();
    return m.iserror ? 0xFFFFFFFFu : c;
}

static unsigned long long refresh_revoked_in_family(unsigned int family_id)
{
    auto m = orm::cms::OauthRefresh();
    m.where("family_id", (long long)family_id).where("revoked", 1);
    unsigned long long c = m.count();
    return m.iserror ? 0xFFFFFFFFu : c;
}

static unsigned long long refresh_total_revoked()
{
    auto m = orm::cms::OauthRefresh();
    m.where("revoked", 1);
    unsigned long long c = m.count();
    return m.iserror ? 0xFFFFFFFFu : c;
}

static void snapshot(const char *phase)
{
    reading("clients_total", count_where_client_id());
    reading("code_rows_probe_a", code_rows_for(kClientA));
    reading("refresh_rows_probe_a", refresh_rows_for(kClientA));
    std::printf("SNAPSHOT %s\n", phase);
}

// ---------------------------------------------------------------------------
// 夹具：直接落库，不走 store 的 add_client——add_client 自己生成口令、只写散列，而夹具要落
// 指定散列、NULL、''、family_id=0 这些 store 拒发的形状
// ---------------------------------------------------------------------------
static bool fixture_client(const std::string &client_id, bool secret_is_null,
                           const std::string &secret, const std::string &uris, bool confidential,
                           bool isopen = true)
{
    auto m = orm::cms::OauthClient();
    m.data.client_id = client_id;
    if (secret_is_null)
    {
        m.data.set_null(orm::cms::oauth_client_info::null_col::client_secret);
    }
    else
    {
        m.data.client_secret = secret;
    }
    m.data.redirect_uris = uris;
    m.data.name = "probe";
    m.data.is_confidential = confidential ? 1 : 0;
    m.data.isopen = isopen ? 1 : 0;
    auto [effect, new_id] = m.insert();
    return effect == 1 && new_id > 0;
}

static bool fixture_code(const std::string &client_id, const std::string &code_hash, long long expires_at)
{
    auto m = orm::cms::OauthCode();
    m.data.code_hash = code_hash;
    m.data.client_id = client_id;
    m.data.user_id = 7;
    m.data.redirect_uri = "https://probe.example/cb";
    m.data.scope = "openid";
    m.data.code_challenge = "E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM";
    m.data.expires_at = expires_at;
    m.data.used = 0;
    auto [effect, new_id] = m.insert();
    return effect == 1 && new_id > 0;
}

// family_id 可以是 0：store 的 issue_refresh 拒发这种行，正因如此只能从这里造
static bool fixture_refresh(unsigned int family_id, const std::string &client_id,
                            const std::string &token_hash, long long expires_at)
{
    auto m = orm::cms::OauthRefresh();
    m.data.family_id = family_id;
    m.data.client_id = client_id;
    m.data.token_hash = token_hash;
    m.data.user_id = 7;
    m.data.scope = "openid";
    m.data.expires_at = expires_at;
    m.data.revoked = 0;
    auto [effect, new_id] = m.insert();
    return effect == 1 && new_id > 0;
}

// ---------------------------------------------------------------------------
// 清理：只删 probe- 前缀的行
// ---------------------------------------------------------------------------
static void purge_probe_rows()
{
    {
        auto m = orm::cms::OauthClient();
        m.whereLikeRight("client_id", "probe-").remove();
    }
    {
        auto m = orm::cms::OauthCode();
        m.whereLikeRight("client_id", "probe-").remove();
    }
    {
        auto m = orm::cms::OauthRefresh();
        m.whereLikeRight("client_id", "probe-").remove();
    }
}

static bool probe_prefix_left()
{
    unsigned long long left = 0;
    {
        auto m = orm::cms::OauthClient();
        m.whereLikeRight("client_id", "probe-");
        left += m.iserror ? 1 : m.count();
    }
    {
        auto m = orm::cms::OauthCode();
        m.whereLikeRight("client_id", "probe-");
        left += m.iserror ? 1 : m.count();
    }
    {
        auto m = orm::cms::OauthRefresh();
        m.whereLikeRight("client_id", "probe-");
        left += m.iserror ? 1 : m.count();
    }
    return left != 0;
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    asio::io_context ioc;
    orm::init_orm_conn_pool(ioc, "conf/orm.conf");
    auto pool = orm::get_orm_conn_pool_obj()["cms"];
    if (!pool)
    {
        std::printf("FAIL pool_cms no [cms] section in conf/orm.conf\n");
        return 1;
    }
    std::thread io_thread(
        [&ioc]()
        {
            auto guard = asio::make_work_guard(ioc);
            ioc.run();
        });

    // ---- 前置：没有上一轮的残留才继续
    purge_probe_rows();
    expect(!probe_prefix_left(), "pre.no_probe_residue");

    // ---- 1. purge.boundary：先跑，此时表里只有本臂自己插的行
    {
        const long long base = (long long)time(nullptr) - 10000;
        const std::string hash_at = "probe-code-at";
        const std::string hash_old = "probe-code-old";
        bool f1 = fixture_code(kClientA, hash_at, base);
        bool f2 = fixture_code(kClientA, hash_old, base - 1);
        expect(f1 && f2, "fixture.code_two_rows");
        expect(code_rows_for(kClientA) == 2, "fixture.code_rows_visible");

        // cutoff == base：expires_at < cutoff，所以 base 那一行不许被删，只删 base-1
        unsigned int d0 = oauth::purge_codes_before(base, 0);
        expect(d0 == 0, "purge.codes_max_rows_zero");
        expect(code_rows_for(kClientA) == 2, "purge.codes_zero_limit_keeps_rows");

        unsigned int d1 = oauth::purge_codes_before(base, 100);
        expect(d1 == 1, "purge.codes_strict_lt", "d1=" + std::to_string(d1));
        expect(code_rows_for(kClientA) == 1, "purge.codes_keeps_boundary_row");

        unsigned int d2 = oauth::purge_codes_before(base + 1, 100);
        expect(d2 == 1, "purge.codes_boundary_row_goes_at_cutoff_plus_one",
               "d2=" + std::to_string(d2));
        expect(code_rows_for(kClientA) == 0, "purge.codes_table_empty_after");

        const std::string rh_at = "probe-refresh-at";
        const std::string rh_old = "probe-refresh-old";
        expect(fixture_refresh(900001, kClientA, rh_at, base) &&
                   fixture_refresh(900002, kClientA, rh_old, base - 1),
               "fixture.refresh_two_rows");
        expect(oauth::purge_refreshes_before(base, 0) == 0, "purge.refreshes_max_rows_zero");
        expect(refresh_rows_for(kClientA) == 2, "purge.refreshes_zero_limit_keeps_rows");
        unsigned int r1 = oauth::purge_refreshes_before(base, 100);
        expect(r1 == 1, "purge.refreshes_strict_lt", "r1=" + std::to_string(r1));
        unsigned int r2 = oauth::purge_refreshes_before(base + 1, 100);
        expect(r2 == 1, "purge.refreshes_boundary_row_goes", "r2=" + std::to_string(r2));
        expect(refresh_rows_for(kClientA) == 0, "purge.refreshes_empty_after");
    }
    purge_probe_rows();

    // ---- 2. client.*
    {
        oauth::client_record rec;
        expect(!oauth::load_client("probe-no-such-client", rec), "client.missing_row");
        expect(!rec.found && rec.client_id.empty() && rec.secret_hash.empty() &&
                   rec.redirect_uris.empty() && rec.name.empty() && rec.confidential,
               "client.missing_row_outputs_default");

        expect(fixture_client("probe-null-secret", true, "", "https://p/cb", true),
               "fixture.client_null_secret");
        expect(fixture_client("probe-empty-secret", false, "", "https://p/cb", true),
               "fixture.client_empty_secret");

        oauth::client_record nul_row, empty_row;
        bool ok_nul = oauth::load_client("probe-null-secret", nul_row);
        bool ok_emp = oauth::load_client("probe-empty-secret", empty_row);
        expect(ok_nul && ok_emp, "client.both_rows_load");
        expect(nul_row.secret_is_null, "client.null_vs_empty.null_row_flagged",
               "secret_is_null=" + std::to_string((int)nul_row.secret_is_null));
        expect(!empty_row.secret_is_null, "client.null_vs_empty.empty_row_not_flagged",
               "secret_is_null=" + std::to_string((int)empty_row.secret_is_null));
        expect(nul_row.secret_hash.empty() && empty_row.secret_hash.empty(),
               "client.both_secrets_read_as_empty_string");
        expect(nul_row.confidential && nul_row.redirect_uris == "https://p/cb",
               "client.other_fields_read");

        // 公开客户端：is_confidential=0 且 secret 为 NULL
        expect(fixture_client("probe-public", true, "", "https://p/cb", false),
               "fixture.client_public");
        oauth::client_record pub;
        expect(oauth::load_client("probe-public", pub) && !pub.confidential && pub.secret_is_null,
               "client.public_reads_not_confidential");
    }

    // ---- 2.2 add_client.*（store 唯一的 client 写入口）
    {
        std::string secret;
        expect(oauth::add_client("probe-ac-secret", "AC", "https://p/ac", true, "", "", true, secret),
               "add_client.confidential_ok");
        expect(secret.size() == 43, "add_client.secret_is_43_chars", "len=" + std::to_string(secret.size()));
        oauth::client_record rec;
        expect(oauth::load_client("probe-ac-secret", rec) && rec.found, "add_client.row_reads_back");
        expect(rec.secret_hash == oauth::lowerhex_sha256(secret), "add_client.stores_hash_not_plain",
               "hash=" + rec.secret_hash);
        expect(!rec.secret_is_null && rec.confidential && rec.name == "AC" &&
                   rec.redirect_uris == "https://p/ac",
               "add_client.other_columns_landed");
        // 策略串留空就是留空（不设上限），不是被写成一个默认白名单
        expect(rec.allowed_scopes.empty() && rec.grant_types.empty() && rec.isopen,
               "add_client.empty_policy_stays_empty",
               "scopes=[" + rec.allowed_scopes + "] grants=[" + rec.grant_types + "]");

        // 三列原样往返：大小写必须逐字节回来（这一条同时证明列级 utf8mb4_bin 真落了）
        std::string policy_secret;
        expect(oauth::add_client("probe-ac-policy", "PC", "https://p/pc", true,
                                 "Admin openid", "refresh_token,authorization_code", true, policy_secret),
               "add_client.policy_ok");
        oauth::client_record pol;
        expect(oauth::load_client("probe-ac-policy", pol) &&
                   pol.allowed_scopes == "Admin openid" &&
                   pol.grant_types == "refresh_token,authorization_code" && pol.isopen,
               "client.columns_roundtrip",
               "scopes=[" + pol.allowed_scopes + "] grants=[" + pol.grant_types + "]");

        // 公开客户端那一列必须是真 NULL：'' 和"散列过的空串"在库里长得一样
        std::string pub_secret = "not-empty-before-the-call";
        expect(oauth::add_client("probe-ac-public", "AP", "https://p/ap", false, "", "", true, pub_secret),
               "add_client.public_ok");
        expect(pub_secret.empty(), "add_client.public_returns_no_secret", "secret=" + pub_secret);
        oauth::client_record pub2;
        expect(oauth::load_client("probe-ac-public", pub2) && pub2.secret_is_null && !pub2.confidential,
               "add_client.public_stores_real_null");

        // 注册即停用：写进去的是 0，不是列默认的 1
        std::string closed_secret;
        expect(oauth::add_client("probe-ac-closed", "CL", "https://p/cl", true, "", "", false, closed_secret),
               "add_client.closed_ok");
        oauth::client_record closed_row;
        expect(oauth::load_client("probe-ac-closed", closed_row) && !closed_row.isopen,
               "add_client.isopen_zero_stored");

        // 唯一键：同一个 client_id 第二次注册 ⇒ false，且第一次那行不能被改写
        std::string again;
        expect(!oauth::add_client("probe-ac-secret", "dup", "https://p/other", true, "", "", true, again),
               "add_client.duplicate_id_rejected");
        expect(again.empty(), "add_client.duplicate_returns_no_secret", "secret=" + again);
        oauth::client_record after_dup;
        expect(oauth::load_client("probe-ac-secret", after_dup) &&
                   after_dup.secret_hash == rec.secret_hash &&
                   after_dup.redirect_uris == "https://p/ac",
               "add_client.duplicate_leaves_first_row");

        // 入参闸门：这几条都不该发语句，所以都查无此行
        std::string gate;
        expect(!oauth::add_client("", "n", "https://p/g", true, "", "", true, gate), "add_client.empty_id_rejected");
        expect(!oauth::add_client(std::string(CONST_OAUTH_CLIENT_ID_MAX_LEN + 1, 'x'), "n",
                                  "https://p/g", true, "", "", true, gate),
               "add_client.overlong_id_rejected");
        expect(!oauth::add_client("probe-ac-overname",
                                  std::string(CONST_OAUTH_CLIENT_NAME_MAX_LEN + 1, 'y'),
                                  "https://p/g", true, "", "", true, gate),
               "add_client.overlong_name_rejected");
        expect(!oauth::add_client("probe-ac-no-uri", "n", "", true, "", "", true, gate),
               "add_client.empty_uris_rejected");
        // 恰好越过列宽一格的两条（255/127 是列声明值）
        expect(!oauth::add_client("probe-ac-longscope", "n", "https://p/g", true,
                                  std::string(CONST_OAUTH_CLIENT_SCOPES_MAX_LEN + 1, 's'), "", true, gate),
               "add_client.overlong_scopes_rejected");
        expect(!oauth::add_client("probe-ac-longgrant", "n", "https://p/g", true, "",
                                  std::string(CONST_OAUTH_CLIENT_GRANTS_MAX_LEN + 1, 'g'), true, gate),
               "add_client.overlong_grants_rejected");
        // 恰好在界内的长度要放过（否则闸门写紧了也没人发现）
        expect(oauth::add_client("probe-ac-exactscope", "n", "https://p/g", true,
                                 std::string(CONST_OAUTH_CLIENT_SCOPES_MAX_LEN, 's'), "", true, gate),
               "add_client.scopes_at_limit_accepted");
        expect(oauth::add_client("probe-ac-exactgrant", "n", "https://p/g", true, "",
                                 std::string(CONST_OAUTH_CLIENT_GRANTS_MAX_LEN, 'g'), true, gate),
               "add_client.grants_at_limit_accepted");
        // scheme 闸门（R-11）：离线台那 17 条只测纯函数，add_client 里那段循环撤掉它们照样全绿，
        // 所以这三条是 store 侧门"接进 add_client"的判据。第二条是正向下界——只允许 https 的写法会把它打红；
        // 第三条测逐项：只看首项的写法放得过它。
        expect(!oauth::add_client("probe-ac-badscheme", "n", "http://evil.example/cb", true, "", "", true, gate),
               "add_client.plaintext_non_loopback_rejected");
        expect(oauth::add_client("probe-ac-loopback", "n", "http://127.0.0.1:8080/cb", true, "", "", true, gate),
               "add_client.loopback_plaintext_accepted");
        expect(!oauth::add_client("probe-ac-mixedscheme", "n", "https://p/ok,http://evil.example/cb", true, "", "", true, gate),
               "add_client.each_item_checked");
        oauth::client_record gate_row;
        expect(!oauth::load_client("probe-ac-overname", gate_row) &&
                   !oauth::load_client("probe-ac-no-uri", gate_row) &&
                   !oauth::load_client("probe-ac-longscope", gate_row) &&
                   !oauth::load_client("probe-ac-longgrant", gate_row) &&
                   !oauth::load_client("probe-ac-badscheme", gate_row) &&
                   !oauth::load_client("probe-ac-mixedscheme", gate_row),
               "add_client.gated_calls_wrote_nothing");
    }

    // ---- 2.5 authenticate_client.*（token 与 revoke 两个端点共用的那一份判定）
    {
        // 库里的 secret 存的是**摘要**，所以夹具要落 sha256 的小写 hex，不是明文口令
        const std::string secret = "probe-shared-secret";
        const std::string digest = oauth::lowerhex_sha256(secret);
        expect(digest.size() == 64, "fixture.secret_digest_is_64_hex");

        expect(fixture_client(kClientA, false, digest, "https://p/cb", true),
               "fixture.client_secreted");
        expect(fixture_client("probe-no-secret", true, "", "https://p/cb", true),
               "fixture.client_confidential_without_secret");

        const std::string basic_ok =
            "Basic " + http::b64url::encode(std::string(kClientA) + ":" + secret);
        const std::string basic_other =
            "Basic " + http::b64url::encode(std::string(kClientB) + ":" + secret);

        // 1) 只有 Authorization 头：body 里不给 client_id，也要认得出是谁
        {
            oauth::client_record rec;
            std::string          authed;
            expect(oauth::authenticate_client(basic_ok, "", "", rec, authed), "auth.basic_only_passes");
            expect(authed == kClientA, "auth.basic_only_reports_header_id", "authed=" + authed);
            expect(rec.found && rec.client_id == kClientA, "auth.basic_only_fills_record");
        }
        // 2) 只有表单：client_secret_post
        {
            oauth::client_record rec;
            std::string          authed;
            expect(oauth::authenticate_client("", kClientA, secret, rec, authed), "auth.post_only_passes");
            expect(authed == kClientA, "auth.post_only_reports_body_id", "authed=" + authed);
        }
        // 3) 口令错 ⇒ false，且 out_client_id 保持空（端点不能变成 client_id 探针）
        {
            oauth::client_record rec;
            std::string          authed;
            const std::string basic_bad =
                "Basic " + http::b64url::encode(std::string(kClientA) + ":wrong-secret");
            expect(!oauth::authenticate_client(basic_bad, "", "", rec, authed), "auth.wrong_secret_rejected");
            expect(authed.empty(), "auth.wrong_secret_reports_no_id", "authed=" + authed);
        }
        // 4) 头的形状不对 ⇒ 拒绝，且不回落到 body 里的明文口令（这一条测的就是那半句注释）
        {
            oauth::client_record rec;
            std::string          authed;
            expect(!oauth::authenticate_client("Bearer sometoken", kClientA, secret, rec, authed),
                   "auth.non_basic_header_rejected");
            expect(!oauth::authenticate_client("Basic @@@not-base64@@@", kClientA, secret, rec, authed),
                   "auth.unparsable_basic_rejected");
            expect(authed.empty(), "auth.bad_header_reports_no_id");
        }
        // 5) 两处都给了 id 且不相等 ⇒ 拒绝（"A 的口令配 B 的 client_id"）
        {
            oauth::client_record rec;
            std::string          authed;
            expect(!oauth::authenticate_client(basic_other, kClientA, secret, rec, authed),
                   "auth.id_mismatch_rejected");
            expect(authed.empty(), "auth.id_mismatch_reports_no_id", "authed=" + authed);
        }
        // 6) 查无此客户端 ⇒ false，与"口令错"同一个形状（不给枚举留缝隙）
        {
            oauth::client_record rec;
            std::string          authed;
            const std::string basic_unknown =
                "Basic " + http::b64url::encode(std::string("probe-no-such-client") + ":" + secret);
            expect(!oauth::authenticate_client(basic_unknown, "", "", rec, authed), "auth.unknown_client_rejected");
            expect(!rec.found && rec.client_id.empty(), "auth.unknown_client_record_stays_default");
            expect(!oauth::authenticate_client("", "", "", rec, authed), "auth.no_id_at_all_rejected");
        }
        // 7) 保密客户端但库里没配口令：NULL 与空串两条来源都一律拒
        {
            oauth::client_record rec;
            std::string          authed;
            const std::string basic_nosecret =
                "Basic " + http::b64url::encode(std::string("probe-no-secret") + ":" + secret);
            expect(!oauth::authenticate_client(basic_nosecret, "", "", rec, authed),
                   "auth.null_secret_never_authenticates");
            expect(!oauth::authenticate_client("", "probe-no-secret", secret, rec, authed),
                   "auth.null_secret_post_shape_never_authenticates");
            expect(!oauth::authenticate_client("", "probe-no-secret", "", rec, authed),
                   "auth.empty_secret_still_rejected");
        }
        // 8) 公开客户端：只给 id 就过；任何形式的口令都拒
        {
            oauth::client_record rec;
            std::string          authed;
            expect(oauth::authenticate_client("", "probe-public", "", rec, authed), "auth.public_id_only_passes");
            expect(authed == "probe-public", "auth.public_reports_loaded_id", "authed=" + authed);

            oauth::client_record rec2;
            std::string          authed2;
            expect(!oauth::authenticate_client("", "probe-public", "whatever", rec2, authed2),
                   "auth.public_rejects_body_secret");
            expect(authed2.empty(), "auth.public_secret_submission_reports_no_id", "authed=" + authed2);

            oauth::client_record rec3;
            std::string          authed3;
            const std::string basic_public =
                "Basic " + http::b64url::encode(std::string("probe-public") + ":whatever");
            expect(!oauth::authenticate_client(basic_public, "", "", rec3, authed3),
                   "auth.public_rejects_basic_header");
        }
        // 9) 停用的客户端：口令对也拒，且对外形状与"查无此客户端"逐项相同
        {
            expect(fixture_client("probe-closed", false, digest, "https://p/cb", true, false),
                   "fixture.client_closed");
            oauth::client_record loaded;
            expect(oauth::load_client("probe-closed", loaded) && loaded.found && !loaded.isopen,
                   "client.isopen_zero_reads_as_closed");
            const std::string basic_closed =
                "Basic " + http::b64url::encode(std::string("probe-closed") + ":" + secret);
            oauth::client_record closed_out;
            std::string          closed_authed = "sentinel";
            expect(!oauth::authenticate_client(basic_closed, "", "", closed_out, closed_authed),
                   "client.isopen_zero_refused");
            // 公开客户端也只靠 isopen：不认证口令的那条路同样要被挡住
            expect(fixture_client("probe-closed-public", true, "", "https://p/cb", false, false),
                   "fixture.client_closed_public");
            oauth::client_record closed_pub_out;
            std::string          closed_pub_authed = "sentinel";
            expect(!oauth::authenticate_client("", "probe-closed-public", "", closed_pub_out, closed_pub_authed),
                   "client.isopen_zero_refused_for_public");
            // 同形：两者都只有 false + 空 out_id，端点看不到任何可区分的痕迹
            oauth::client_record ghost_out;
            std::string          ghost_authed = "sentinel";
            const std::string    basic_ghost =
                "Basic " + http::b64url::encode(std::string("probe-no-such-client") + ":" + secret);
            bool ghost_ok = oauth::authenticate_client(basic_ghost, "", "", ghost_out, ghost_authed);
            expect(!ghost_ok && closed_authed.empty() && closed_pub_authed.empty() && ghost_authed.empty(),
                   "client.isopen_zero_same_shape_as_missing",
                   "closed=[" + closed_authed + "] pub=[" + closed_pub_authed + "] ghost=[" + ghost_authed + "]");
        }
    }

    // ---- 3. code.*
    {
        const long long exp = (long long)time(nullptr) + 300;
        std::string plain = oauth::issue_code(kClientA, 42, "https://probe.example/cb",
                                              "openid profile", "E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM",
                                              exp);
        expect(!plain.empty(), "code.issue_returns_plaintext");
        expect(plain.size() == 43, "code.plaintext_is_43_chars", std::to_string(plain.size()));
        expect(code_rows_for(kClientA) == 1, "code.one_row_landed");

        oauth::code_record cr;
        expect(oauth::find_code(plain, cr), "code.roundtrip_found");
        expect(cr.client_id == kClientA && cr.user_id == 42 &&
                   cr.redirect_uri == "https://probe.example/cb" && cr.scope == "openid profile" &&
                   cr.code_challenge == "E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM" &&
                   cr.expires_at == exp && !cr.used,
               "code.roundtrip_six_fields_byte_equal");

        // 库里存的是摘要，不是明文
        {
            auto m = orm::cms::OauthCode();
            m.where("client_id", kClientA);
            m.fetch_one();
            expect(m.data.code_hash != plain && m.data.code_hash.size() == 64,
                   "code.stored_as_64_char_hex_digest", m.data.code_hash);
        }

        expect(!oauth::find_code("not-a-real-code", cr), "code.miss_returns_false");

        // CAS：两次消费只有第一次成功
        expect(oauth::consume_code(plain), "code.consume_first_wins");
        expect(!oauth::consume_code(plain), "code.consume_second_loses");
        oauth::code_record after;
        expect(oauth::find_code(plain, after) && after.used, "code.used_flag_set_in_db");

        // R-10：码行到 refresh 族的连线。绑族只发生在 used=1 之后，所以先绑成真的族根、
        // 再拿 0 去绑、最后验未消费的那行绑不上——这三条是 bind_code_family 的 WHERE 与 0 闸门的形状。
        expect(after.family_id == 0, "code.family_id_zero_before_bind",
               "family_id=" + std::to_string(after.family_id));
        unsigned int fam_root_row = 0;
        std::string  fam_token    = oauth::issue_refresh(0, kClientA, 42, "openid",
                                                         (long long)time(nullptr) + 3600, fam_root_row);
        expect(!fam_token.empty() && fam_root_row > 0, "code.bind_fixture_family",
               "row=" + std::to_string(fam_root_row));
        // 顺序是这两条臂的牙所在：先绑成真的族根，再拿 0 去绑。反过来的话，撤掉 0 闸门的那次
        // `UPDATE ... SET family_id=0` 落在一行 family_id 已经是 0 的码上＝零行变更，
        // `bind_code_family()` 照样返回 false，`bind_zero_refused` 与 `bind_zero_leaves_row` 一起假绿。
        expect(oauth::bind_code_family(plain, fam_root_row), "code.bind_after_consume_ok");
        oauth::code_record bound;
        expect(oauth::find_code(plain, bound) && bound.family_id == fam_root_row,
               "code.family_id_roundtrip", "got=" + std::to_string(bound.family_id));
        expect(!oauth::bind_code_family(plain, 0), "code.bind_zero_refused");
        // 0 被拒之后那一格不能被写脏
        oauth::code_record after_zero;
        expect(oauth::find_code(plain, after_zero) && after_zero.family_id == fam_root_row,
               "code.bind_zero_leaves_row", "got=" + std::to_string(after_zero.family_id));
        std::string p3 = oauth::issue_code(kClientA, 44, "https://probe.example/cb", "openid",
                                           "challenge-for-bind-unconsumed",
                                           (long long)time(nullptr) + 300);
        expect(!p3.empty(), "code.bind_unconsumed_fixture");
        expect(!oauth::bind_code_family(p3, fam_root_row), "code.bind_unconsumed_refused");
        // 绑上之后撤族撤得到：这一条是"重放撤族"在 store 侧的下界（端点那条腿要真发 HTTP 请求）。
        // 这个族里只有上面那一行夹具，所以恰好 1 行；第二次调用必须报 0，否则这条断言没有牙。
        unsigned int swept_once = oauth::revoke_refresh_family(fam_root_row);
        unsigned int swept_twice = oauth::revoke_refresh_family(fam_root_row);
        expect(swept_once == 1 && swept_twice == 0, "code.bind_family_is_revocable",
               "once=" + std::to_string(swept_once) + " twice=" + std::to_string(swept_twice));

        // 同码并发消费：两个线程各拿一次 CAS，恰好一个 true
        {
            std::string p2 = oauth::issue_code(kClientA, 43, "https://probe.example/cb",
                                               "openid", "challenge-for-concurrency",
                                               (long long)time(nullptr) + 300);
            expect(!p2.empty(), "code.concurrent_fixture");
            std::atomic<bool> go{false};
            std::atomic<int> wins{0};
            auto worker = [&]()
            {
                while (!go.load())
                {
                    std::this_thread::yield();
                }
                if (oauth::consume_code(p2))
                    ++wins;
            };
            std::thread t1(worker), t2(worker);
            go.store(true);
            t1.join();
            t2.join();
            expect(wins.load() == 1, "code.concurrent_exactly_one_winner",
                   "wins=" + std::to_string(wins.load()));
        }

        // 空明文不许去打库（token_hash_hex 的闸门）
        expect(!oauth::consume_code(""), "code.consume_empty_refused");
        oauth::code_record blank;
        expect(!oauth::find_code("", blank), "code.find_empty_refused");
        // 空 client_id 没有闸门，这是刻意测出来的现实而不是断言：调用方手里必然已经
        // load_client 成功过。只登记，不断言——将来真加了闸门这条会变，也不该被判红。
        {
            std::string junk = oauth::issue_code("", 1, "https://x/cb", "", "challenge", exp);
            std::printf("NOTE code.issue_empty_client | returns_code=%d rows_inserted=%d\n",
                        (int)!junk.empty(), (int)code_rows_for(""));
            if (!junk.empty())
            {
                auto m = orm::cms::OauthCode();
                m.where("code_hash", http::oauth::lowerhex_sha256(junk));
                m.remove();
            }
        }
    }

    // ---- 4. refresh.*
    {
        const long long exp = (long long)time(nullptr) + 3600;
        unsigned int root_row = 0;
        std::string root = oauth::issue_refresh(0, kClientA, 7, "openid", exp, root_row);
        expect(!root.empty() && root_row > 0, "refresh.root_issued",
               "row=" + std::to_string(root_row));

        oauth::refresh_record rr;
        expect(oauth::find_refresh(root, rr), "refresh.root_found");
        expect(rr.family_id == root_row && rr.family_id != 0, "refresh.family_backfilled_to_own_id",
               "family=" + std::to_string(rr.family_id) + " row=" + std::to_string(root_row));
        expect(rr.row_id == root_row && rr.client_id == kClientA && rr.user_id == 7 &&
                   rr.scope == "openid" && rr.expires_at == exp && !rr.revoked,
               "refresh.roundtrip_fields");

        unsigned int child_row = 0;
        std::string child = oauth::issue_refresh(rr.family_id, kClientA, 7, "openid", exp, child_row);
        expect(!child.empty(), "refresh.child_issued");
        oauth::refresh_record child_rec;
        expect(oauth::find_refresh(child, child_rec) && child_rec.family_id == rr.family_id,
               "refresh.child_shares_family");

        // 轮换 CAS：同一行第二次撤销必败（重放信号）
        expect(oauth::revoke_refresh_row(root_row), "refresh.rotate_first_wins");
        expect(!oauth::revoke_refresh_row(root_row), "refresh.rotate_second_loses");
        oauth::refresh_record revoked;
        expect(oauth::find_refresh(root, revoked) && revoked.revoked, "refresh.row_marked_revoked");
        expect(!oauth::revoke_refresh_row(0), "refresh.revoke_row_zero_refused");

        // 撤整族：族 A 三行全撤，族 B 不受影响
        {
            unsigned int a_root_row = 0, a1 = 0, a2 = 0, b_root_row = 0;
            std::string a_root = oauth::issue_refresh(0, kClientB, 8, "openid", exp, a_root_row);
            oauth::refresh_record a_root_rec;
            expect(!a_root.empty() && oauth::find_refresh(a_root, a_root_rec),
                   "refresh.familyA_fixture");
            std::string a_c1 = oauth::issue_refresh(a_root_rec.family_id, kClientB, 8, "openid", exp, a1);
            std::string a_c2 = oauth::issue_refresh(a_root_rec.family_id, kClientB, 8, "openid", exp, a2);
            std::string b_root = oauth::issue_refresh(0, "probe-client-c", 9, "openid", exp, b_root_row);
            oauth::refresh_record b_rec;
            expect(!a_c1.empty() && !a_c2.empty() && !b_root.empty() &&
                       oauth::find_refresh(b_root, b_rec),
                   "refresh.familyB_fixture");

            unsigned int fam_a = a_root_rec.family_id;
            unsigned int fam_b = b_rec.family_id;
            expect(fam_a != fam_b && fam_a != 0 && fam_b != 0, "refresh.families_are_distinct");
            expect(refresh_revoked_in_family(fam_a) == 0, "refresh.familyA_starts_clean");

            unsigned int swept = oauth::revoke_refresh_family(fam_a);
            expect(swept == 3, "refresh.familyA_three_rows_revoked", "swept=" + std::to_string(swept));
            expect(refresh_revoked_in_family(fam_a) == 3, "refresh.familyA_still_three_after");
            expect(refresh_revoked_in_family(fam_b) == 0, "refresh.familyB_untouched");
            // 再撤一次：全已撤销 ⇒ 0 行
            expect(oauth::revoke_refresh_family(fam_a) == 0, "refresh.familyA_second_sweep_zero");
        }

        // family_id=0 的拒发口：只能从夹具造出这种行
        {
            expect(fixture_refresh(0, "probe-client-d", "probe-zero-family-token", exp),
                   "fixture.refresh_zero_family_row");
            unsigned long long before = refresh_total_revoked();
            expect(oauth::revoke_refresh_family(0) == 0, "refresh.zero_family_refused");
            unsigned long long after = refresh_total_revoked();
            expect(after == before, "refresh.zero_family_changes_nothing",
                   "before=" + std::to_string(before) + " after=" + std::to_string(after));
            // 这一行是夹具直接落的，token_hash 不是 store 算出来的摘要，所以只能按列读回
            {
                auto m = orm::cms::OauthRefresh();
                m.where("token_hash", "probe-zero-family-token");
                m.fetch_one();
                expect(!m.iserror && m.data.family_id == 0 && m.data.revoked == 0,
                       "refresh.zero_family_row_still_live",
                       "family=" + std::to_string(m.data.family_id) +
                           " revoked=" + std::to_string((int)m.data.revoked));
            }
        }

        // RFC 7009：撤必须带 client_id
        {
            unsigned int row = 0;
            std::string tok = oauth::issue_refresh(0, kClientA, 11, "openid", exp, row);
            expect(!tok.empty(), "refresh.cross_client_fixture");
            expect(!oauth::revoke_refresh_by_token(tok, kClientB), "refresh.other_client_cannot_revoke");
            oauth::refresh_record still;
            expect(oauth::find_refresh(tok, still) && !still.revoked, "refresh.row_still_live_after_wrong_client");
            expect(!oauth::revoke_refresh_by_token(tok, ""), "refresh.empty_client_id_refused");
            expect(oauth::revoke_refresh_by_token(tok, kClientA), "refresh.own_client_revokes");
            expect(!oauth::revoke_refresh_by_token(tok, kClientA), "refresh.second_revoke_loses");
            expect(!oauth::revoke_refresh_by_token("", kClientA), "refresh.empty_token_refused");
        }
    }

    // ---- 4.5 introspect.*（opaque 那一支要查 oauth_refresh，所以只能在真库上跑）----
    {
        // opaque 分支不碰公钥（只有 JWT 那一支验签），这里只要非空就能过 store 的"配置坏了"闸。
        const std::string kNoKeyNeeded = "unused-by-opaque-branch";
        const char *      kIss         = "https://probe.example";
        const long long   exp          = (long long)time(nullptr) + 3600;

        unsigned int live_row = 0;
        std::string  live     = oauth::issue_refresh(0, kClientA, 7, "openid", exp, live_row);
        expect(!live.empty() && live.size() == 43 && live.find('.') == std::string::npos,
               "introspect.opaque_fixture_is_43_no_dot", "len=" + std::to_string(live.size()));

        oauth::introspect_record ir;
        // 正向下界：活着的那一行必须 active:true，且每一格都来自库里的列
        expect(oauth::introspect_token(kClientA, kNoKeyNeeded, kIss, 30, live, ir) && ir.active &&
                   ir.token_type == "refresh_token" && ir.client_id == kClientA && ir.sub == "7" &&
                   ir.scope == "openid" && ir.exp == exp,
               "introspect.opaque_active_all_fields",
               "type=" + ir.token_type + " cid=" + ir.client_id + " sub=" + ir.sub +
                   " scope=" + ir.scope + " exp=" + std::to_string(ir.exp));
        // oauth_refresh 没有签发时刻列 ⇒ iss/aud/jti/iat 只能是空，端点据此不发这四格
        expect(ir.iss.empty() && ir.aud.empty() && ir.jti.empty() && ir.iat == 0,
               "introspect.opaque_no_issued_at_columns",
               "iss=" + ir.iss + " aud=" + ir.aud + " jti=" + ir.jti + " iat=" + std::to_string(ir.iat));

        // 查无此令牌：形状合法、库里没有 ⇒ active:false，与"别人的令牌"同一格答案
        {
            std::string ghost = oauth::random_token();
            expect(!ghost.empty() && ghost != live, "introspect.ghost_fixture_distinct");
            expect(oauth::introspect_token(kClientA, kNoKeyNeeded, kIss, 30, ghost, ir) && !ir.active &&
                       ir.client_id.empty() && ir.sub.empty(),
                   "introspect.unknown_token_inactive", "cid=" + ir.client_id);
        }

        // 跨 client：B 的令牌由 A 来问 ⇒ active:false（不是 403，不给"属于别人"的任何提示）
        {
            unsigned int b_row = 0;
            std::string  b_tok = oauth::issue_refresh(0, kClientB, 8, "openid", exp, b_row);
            expect(!b_tok.empty(), "introspect.cross_client_fixture");
            expect(oauth::introspect_token(kClientA, kNoKeyNeeded, kIss, 30, b_tok, ir) && !ir.active &&
                       ir.sub.empty(),
                   "introspect.other_client_inactive", "sub=" + ir.sub);
            // 同一枚令牌由它自己的主人来问就是活的：证明上一条红的是"跨 client"，不是"这枚令牌不活"
            expect(oauth::introspect_token(kClientB, kNoKeyNeeded, kIss, 30, b_tok, ir) && ir.active &&
                       ir.client_id == kClientB && ir.sub == "8",
                   "introspect.own_client_sees_it_active", "cid=" + ir.client_id + " sub=" + ir.sub);
        }

        // 已撤销 ⇒ active:false
        {
            unsigned int row = 0;
            std::string  tok = oauth::issue_refresh(0, kClientA, 12, "openid", exp, row);
            expect(!tok.empty() && oauth::revoke_refresh_row(row), "introspect.revoke_fixture");
            expect(oauth::introspect_token(kClientA, kNoKeyNeeded, kIss, 30, tok, ir) && !ir.active,
                   "introspect.revoked_inactive");
        }

        // 已过期 ⇒ active:false（expires_at 由 issue_refresh 原样落库，它不拒过去的值）
        {
            unsigned int row = 0;
            std::string  tok =
                oauth::issue_refresh(0, kClientA, 13, "openid", (long long)time(nullptr) - 60, row);
            expect(!tok.empty(), "introspect.expired_fixture");
            expect(oauth::introspect_token(kClientA, kNoKeyNeeded, kIss, 30, tok, ir) && !ir.active,
                   "introspect.expired_inactive");
        }

        // issuer 那一格对 opaque 分支不起作用：opaque 令牌里没有 iss 可比。
        // 这条登记现状（不是"设计如此"的保证），免得将来有人以为它也在管 opaque。
        expect(oauth::introspect_token(kClientA, kNoKeyNeeded, "", 30, live, ir) && ir.active,
               "introspect.opaque_ignores_issuer");

        // 空公钥 ⇒ 判不了（false），哪怕问的是一枚 opaque 令牌：配置坏了不许被答成 active:false
        expect(!oauth::introspect_token(kClientA, "", kIss, 30, live, ir) && !ir.active,
               "introspect.opaque_no_pubkey_cannot_judge");
    }

    // ---- 5. 清理与残留断言
    snapshot("before_cleanup");
    purge_probe_rows();
    expect(!probe_prefix_left(), "cleanup.no_probe_residue_left");
    snapshot("after_cleanup");

    std::printf("Results: %d PASS %d FAIL\n", g_pass, g_fail);
    ioc.stop();
    io_thread.join();
    return g_fail == 0 ? 0 : 1;
}
