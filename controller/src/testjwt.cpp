// testjwt.cpp — JWT 公共类的演示路由：一次请求把签发、验签、时效、错误码全部走一遍。
// HMAC 密钥是页面上写死的演示值，EC/RSA 密钥每次请求现生成，所以页面里的 token 出去就没法再验回来。

#include <ctime>
#include <string>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "httppeer.h"
#include "jwt.h"

namespace http
{

//@urlpath(null,testjwt)
std::string testjwt(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("text/html; charset=UTF-8");

    using namespace http::jwt;

    const std::string secret = "paozhu-demo-hmac-secret";
    const time_t        now  = std::time(nullptr);

    unsigned int passed = 0;
    unsigned int failed = 0;

    auto line = [&](bool ok, const std::string &what, const std::string &detail = "")
    {
        ok ? ++passed : ++failed;
        client << (ok ? "PASS  " : "FAIL  ") << what;
        if (detail.size() > 0)
        {
            client << "  |  " << detail;
        }
        client << "\n";
    };

    client << "<pre>Paozhu JWT demo\n";
    client << "============================================================\n\n";
    client << "[1] HS256 sign & verify\n";

    std::string token;
    try
    {
        token = jwt::encode(R"({"alg":"HS256"})",
                            R"({"sub":"demo","iat":)" + std::to_string(now) +
                                R"(,"exp":)" + std::to_string(now + 3600) + R"(})",
                            secret,
                            algorithm::HS256);
    }
    catch (const std::exception &e)
    {
        line(false, "encode HS256", e.what());
    }
    line(token.size() > 0, "encode HS256 returns a token", "len=" + std::to_string(token.size()));

    auto vr = jwt::verify(token, secret, algorithm::HS256);
    line(vr.err == verify_error::ok, "verify with the same secret",
         "err=" + std::to_string(static_cast<int>(vr.err)) +
             " exp=" + std::to_string(static_cast<long long>(vr.exp)));
    line(vr.header_json.find("\"typ\"") != std::string::npos, "typ:JWT auto-injected",
         vr.header_json);

    std::string tampered = token;
    size_t      dot      = tampered.rfind('.');
    if (dot + 1 < tampered.size())
    {
        tampered[dot + 1] = (tampered[dot + 1] == 'A') ? 'B' : 'A';
    }
    vr = jwt::verify(tampered, secret, algorithm::HS256);
    line(vr.err == verify_error::signature_mismatch, "one signature byte flipped is rejected",
         "err=" + std::to_string(static_cast<int>(vr.err)) +
             " exp=" + std::to_string(static_cast<long long>(vr.exp)));

    vr = jwt::verify(token, "another-secret-value", algorithm::HS256);
    line(vr.err == verify_error::signature_mismatch, "wrong secret is rejected",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    vr = jwt::verify(token, secret, algorithm::ES256);
    line(vr.err == verify_error::alg_mismatch, "expected-alg whitelist",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    std::string none_token = b64url::encode(R"({"alg":"none","typ":"JWT"})") + "." +
                             b64url::encode(R"({"sub":"x"})") + ".";
    vr = jwt::verify(none_token, secret, algorithm::HS256);
    line(vr.err == verify_error::alg_none_rejected, "alg:none is rejected",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    vr = jwt::verify("@@@@@.@@@@@.@@@@@", secret, algorithm::HS256);
    line(vr.err == verify_error::invalid_base64, "non-base64url segments",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    auto dv = jwt::decode_without_verify(token);
    line(dv.alg == "HS256" && dv.err == verify_error::parsed_but_unverified,
         "decode_without_verify reads alg", "alg=" + dv.alg);

    client << "\n[2] exp / nbf claims\n";

    std::string past_token = jwt::encode(R"({"alg":"HS256"})",
                                         R"({"sub":"demo","exp":)" + std::to_string(now - 10) +
                                             R"(})",
                                         secret,
                                         algorithm::HS256);
    vr = jwt::verify(past_token, secret, algorithm::HS256);
    line(vr.err == verify_error::token_expired, "exp in the past",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    vr = jwt::verify(past_token, secret, algorithm::HS256, 60);
    line(vr.err == verify_error::ok, "same token inside 60s leeway",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    std::string future_token = jwt::encode(R"({"alg":"HS256"})",
                                           R"({"sub":"demo","nbf":)" +
                                               std::to_string(now + 3600) + R"(})",
                                           secret,
                                           algorithm::HS256);
    vr = jwt::verify(future_token, secret, algorithm::HS256);
    line(vr.err == verify_error::token_not_yet_valid, "nbf in the future",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    std::string zero_token =
        jwt::encode(R"({"alg":"HS256"})", R"({"sub":"demo","exp":0})", secret, algorithm::HS256);
    vr = jwt::verify(zero_token, secret, algorithm::HS256);
    line(vr.err == verify_error::token_expired, "exp=0 means 1970, so expired",
         "err=" + std::to_string(static_cast<int>(vr.err)) +
             " exp=" + std::to_string(static_cast<long long>(vr.exp)));

    std::string text_token =
        jwt::encode(R"({"alg":"HS256"})", R"({"sub":"demo","exp":"2000000000"})", secret,
                    algorithm::HS256);
    vr = jwt::verify(text_token, secret, algorithm::HS256);
    line(vr.err == verify_error::invalid_claims, "exp as a JSON string is refused, not ignored",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    std::string nested_token = jwt::encode(
        R"({"alg":"HS256"})",
        R"({"sub":"demo","meta":{"exp":)" + std::to_string(now - 10) + R"(}})",
        secret,
        algorithm::HS256);
    vr = jwt::verify(nested_token, secret, algorithm::HS256);
    line(vr.err == verify_error::ok, "a nested exp is not a top-level claim",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    client << "\n[3] bad key or bad JSON on the signing side\n";

    bool threw = false;
    try
    {
        jwt::encode(R"({"alg":"HS256"})", R"({"sub":"demo"})", "", algorithm::HS256);
    }
    catch (const std::exception &)
    {
        threw = true;
    }
    line(threw, "empty HMAC secret cannot sign");

    vr = jwt::verify(token, "", algorithm::HS256);
    line(vr.err == verify_error::key_mismatch, "empty HMAC secret cannot verify",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    threw = false;
    try
    {
        jwt::encode(R"({"alg":"HS256"})", "not-json-at-all", secret, algorithm::HS256);
    }
    catch (const std::exception &)
    {
        threw = true;
    }
    line(threw, "payload that is not a JSON object is refused");

    client << "\n[4] structural gates: object shape, crit, repeated members, cut-off tail\n";

    // 这两条在验签之前判，所以签名段随便填什么都到不了 HMAC
    std::string hdr_b64 = b64url::encode(R"({"alg":"HS256"})");
    std::string arr_b64 =
        b64url::encode(R"([{"exp":)" + std::to_string(static_cast<long long>(now - 10)) + R"(}])");
    vr = jwt::verify(hdr_b64 + "." + arr_b64 + ".abc", secret, algorithm::HS256);
    line(vr.err == verify_error::malformed_json, "array payload refused before the signature",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    const std::string future_s = std::to_string(static_cast<long long>(now + 3600));
    std::string cut_b64 =
        b64url::encode(R"({"exp":)" + future_s + R"(,"note":"unclosed)");
    vr = jwt::verify(hdr_b64 + "." + cut_b64 + ".abc", secret, algorithm::HS256);
    line(vr.err == verify_error::malformed_json, "unclosed string refused by the object gate",
         "err=" + std::to_string(static_cast<int>(vr.err)));
    // decode_without_verify 不判读，只解析：闭合之前读到的 exp 要原样交出去
    auto dv_cut = jwt::decode_without_verify(hdr_b64 + "." + cut_b64 + ".abc");
    line(dv_cut.exp == now + 3600, "decode keeps the exp read before an unclosed tail",
         "exp=" + std::to_string(static_cast<long long>(dv_cut.exp)) +
             " err=" + std::to_string(static_cast<int>(dv_cut.err)));

    std::string crit_token = jwt::encode(R"({"alg":"HS256","crit":["b64u"]})",
                                         R"({"sub":"demo"})",
                                         secret,
                                         algorithm::HS256);
    vr = jwt::verify(crit_token, secret, algorithm::HS256);
    line(vr.err == verify_error::unsupported_critical, "header with crit refused",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    std::string dup_token = jwt::encode(R"({"alg":"HS256"})",
                                        R"({"exp":)" + future_s + R"(,"exp":)" + future_s + R"(})",
                                        secret,
                                        algorithm::HS256);
    vr = jwt::verify(dup_token, secret, algorithm::HS256);
    line(vr.err == verify_error::invalid_claims, "repeated exp refused even when both values match",
         "err=" + std::to_string(static_cast<int>(vr.err)));

    client << "\n[5] ES256 (key pair generated for this request)\n";
    try
    {
        EVP_PKEY_ptr kp = ec_crypto::generate_p256();
        std::string  priv_pem = ec_crypto::export_pem(kp.get());
        BIO_ptr      mem(BIO_new(BIO_s_mem()));
        PEM_write_bio_PUBKEY(mem.get(), kp.get());
        char *pub_data = nullptr;
        long  pub_len  = BIO_get_mem_data(mem.get(), &pub_data);
        std::string pub_pem(pub_data, static_cast<size_t>(pub_len));

        std::string ec_token = jwt::encode(
            R"({"alg":"ES256"})",
            R"({"sub":"demo","exp":)" + std::to_string(now + 3600) + R"(})",
            priv_pem,
            algorithm::ES256);
        vr = jwt::verify(ec_token, pub_pem, algorithm::ES256);
        line(vr.err == verify_error::ok, "ES256 sign & verify",
             "err=" + std::to_string(static_cast<int>(vr.err)));

        std::string ec_bad = ec_token;
        size_t      ec_dot = ec_bad.rfind('.');
        ec_bad[ec_dot + 1] = (ec_bad[ec_dot + 1] == 'A') ? 'B' : 'A';
        vr = jwt::verify(ec_bad, pub_pem, algorithm::ES256);
        line(vr.err == verify_error::signature_mismatch, "ES256 tampered signature",
             "err=" + std::to_string(static_cast<int>(vr.err)));

        vr = jwt::verify(ec_token, pub_pem, algorithm::HS256);
        line(vr.err == verify_error::alg_mismatch, "ES256 token refused for HS256",
             "err=" + std::to_string(static_cast<int>(vr.err)));
    }
    catch (const std::exception &e)
    {
        line(false, "ES256 round-trip", e.what());
    }

    if (client.get.isset("rsa") && client.get["rsa"].to_int() == 1)
    {
        client << "\n[6] RS256 / PS256 (RSA-2048 generated for this request)\n";
        try
        {
            EVP_PKEY *raw = EVP_PKEY_Q_keygen(nullptr, nullptr, "RSA", static_cast<size_t>(2048));
            if (raw == nullptr)
            {
                throw std::runtime_error("RSA keygen failed");
            }
            EVP_PKEY_ptr kp(raw);
            std::string  priv_pem = ec_crypto::export_pem(kp.get());
            BIO_ptr      mem(BIO_new(BIO_s_mem()));
            PEM_write_bio_PUBKEY(mem.get(), kp.get());
            char *pub_data = nullptr;
            long  pub_len  = BIO_get_mem_data(mem.get(), &pub_data);
            std::string pub_pem(pub_data, static_cast<size_t>(pub_len));

            std::string payload =
                R"({"sub":"demo","exp":)" + std::to_string(now + 3600) + R"(})";

            std::string rs_token = jwt::encode(R"({"alg":"RS256"})", payload, priv_pem,
                                               algorithm::RS256);
            vr = jwt::verify(rs_token, pub_pem, algorithm::RS256);
            line(vr.err == verify_error::ok, "RS256 sign & verify",
                 "err=" + std::to_string(static_cast<int>(vr.err)));

            std::string rs_bad = rs_token;
            size_t      rs_dot = rs_bad.rfind('.');
            rs_bad[rs_dot + 1] = (rs_bad[rs_dot + 1] == 'A') ? 'B' : 'A';
            vr = jwt::verify(rs_bad, pub_pem, algorithm::RS256);
            line(vr.err == verify_error::signature_mismatch, "RS256 tampered signature",
                 "err=" + std::to_string(static_cast<int>(vr.err)));

            std::string ps_token = jwt::encode(R"({"alg":"PS256"})", payload, priv_pem,
                                               algorithm::PS256);
            vr = jwt::verify(ps_token, pub_pem, algorithm::PS256);
            line(vr.err == verify_error::ok, "PS256 sign & verify",
                 "err=" + std::to_string(static_cast<int>(vr.err)));

            vr = jwt::verify(ps_token, pub_pem, algorithm::RS256);
            line(vr.err == verify_error::alg_mismatch, "PS256 token refused for RS256",
                 "err=" + std::to_string(static_cast<int>(vr.err)));
        }
        catch (const std::exception &e)
        {
            line(false, "RSA round-trip", e.what());
        }
    }
    else
    {
        client << "\n[5] skipped — add ?rsa=1 to run the RS256 / PS256 pair\n";
    }

    client << "\n============================================================\n";
    client << "Results: " << passed << " passed / " << failed << " failed\n";
    client << "</pre>\n";
    return "";
}

}// namespace http
