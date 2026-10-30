/**
 * jwt_smoke.cpp — minimal functional verification for http::jwt
 * Compile: g++ -std=c++20 -I. -I./vendor/httpserver/include tests/test_jwt_smoke.cpp vendor/httpserver/src/jwt.cpp vendor/httpserver/src/request.cpp -lcrypto -lssl -o jwt_smoke
 *          (request.cpp carries obj_val::from_json, which jwt.cpp asks for to confirm section syntax;
 *           -I. is for common/cost_define.h, which holds the token length cap)
 * Run:     ./jwt_smoke
 */

#include "jwt.h"
#include "common/cost_define.h"
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <chrono>
#include <thread>

using namespace http;
using namespace http::jwt;

int main()
{
    int pass = 0, fail = 0;
    auto check = [&](const char *name, bool ok) {
        if (ok) { pass++; std::cout << "  PASS: " << name << "\n"; }
        else    { fail++; std::cout << "  FAIL: " << name << "\n"; }
    };

    // ============================================================
    // 1) HS256 sign -> verify round-trip
    // ============================================================
    std::cout << "[1] HS256 round-trip\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        const std::string header_json = R"({"alg":"HS256"})";
        const std::string payload_json = R"({"sub":"user1","exp":)" +
            std::to_string(std::time(nullptr) + 3600) + R"(})";

        std::string token = jwt::encode(header_json, payload_json, key, algorithm::HS256);
        check("HS256 token has 3 segments",
              token.find('.') != std::string::npos &&
              token.find('.', token.find('.')+1) != std::string::npos);

        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("HS256 verify ok", vr.err == verify_error::ok);
        check("HS256 alg field", vr.alg == "HS256");
        check("HS256 payload contains sub", vr.payload_json.find("user1") != std::string::npos);
    }

    // ============================================================
    // 2) HS256 tampered signature -> signature_mismatch
    // ============================================================
    std::cout << "[2] HS256 tamper detection\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        std::string token = jwt::encode(
            R"({"alg":"HS256"})",
            R"({"sub":"x"})",
            key,
            algorithm::HS256);
        // flip one char in signature
        auto last_dot = token.rfind('.');
        token[last_dot + 1] = (token[last_dot + 1] == 'A' ? 'B' : 'A');
        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("HS256 tamper rejected", vr.err == verify_error::signature_mismatch);
    }

    // ============================================================
    // 3) HS256 wrong key -> signature_mismatch
    // ============================================================
    std::cout << "[3] HS256 wrong key\n";
    {
        const std::string key1 = "key-one-32bytes----XXXXXXXXXXXXX";
        const std::string key2 = "key-two-32bytes-----XXXXXXXXXXXX";
        std::string token = jwt::encode(
            R"({"alg":"HS256"})", R"({"sub":"x"})", key1, algorithm::HS256);
        auto vr = jwt::verify(token, key2, algorithm::HS256);
        check("HS256 wrong key rejected", vr.err == verify_error::signature_mismatch);
    }

    // ============================================================
    // 4) ES256 sign -> verify with EC key pair
    // ============================================================
    std::cout << "[4] ES256 round-trip\n";
    {
        // Generate ephemeral P-256 key pair, export PEM strings
        http::EVP_PKEY_ptr priv = http::ec_crypto::generate_p256();
        std::string priv_pem = http::ec_crypto::export_pem(priv.get());

        // Extract public key PEM by exporting a public-key-only PEM
        // PEM_write_bio_PUBKEY writes just the public portion
        BIO_ptr mem(BIO_new(BIO_s_mem()));
        PEM_write_bio_PUBKEY(mem.get(), priv.get());
        char *data = nullptr;
        long len = BIO_get_mem_data(mem.get(), &data);
        std::string pub_pem(data, static_cast<size_t>(len));

        // Sign with private key
        std::string token = jwt::encode(
            R"({"alg":"ES256"})",
            R"({"sub":"ec-user","exp":)" + std::to_string(std::time(nullptr) + 3600) + R"(})",
            priv_pem,
            algorithm::ES256);
        check("ES256 token has 3 segments",
              std::count(token.begin(), token.end(), '.') == 2);

        // Verify with public key
        auto vr = jwt::verify(token, pub_pem, algorithm::ES256);
        check("ES256 verify ok", vr.err == verify_error::ok);
        check("ES256 alg field", vr.alg == "ES256");
        check("ES256 payload", vr.payload_json.find("ec-user") != std::string::npos);
    }

    // ============================================================
    // 5) ES256 wrong public key -> signature_mismatch
    // ============================================================
    std::cout << "[5] ES256 wrong key\n";
    {
        auto kp1 = http::ec_crypto::generate_p256();
        auto kp2 = http::ec_crypto::generate_p256();
        std::string pem1 = http::ec_crypto::export_pem(kp1.get());

        BIO_ptr mem(BIO_new(BIO_s_mem()));
        PEM_write_bio_PUBKEY(mem.get(), kp2.get());
        char *data = nullptr;
        long len = BIO_get_mem_data(mem.get(), &data);
        std::string pem2_pub(data, static_cast<size_t>(len));

        std::string token = jwt::encode(
            R"({"alg":"ES256"})", R"({"sub":"x"})", pem1, algorithm::ES256);

        auto vr = jwt::verify(token, pem2_pub, algorithm::ES256);
        check("ES256 wrong pubkey rejected", vr.err == verify_error::signature_mismatch);
    }

    // ============================================================
    // 6) alg:none rejection
    // ============================================================
    std::cout << "[6] alg:none rejection\n";
    {
        // Manually build a token with alg:none (sign input is irrelevant since alg is none)
        std::string header_b64 = http::b64url::encode(R"({"alg":"none"})");
        std::string payload_b64 = http::b64url::encode(R"({"sub":"ghost"})");
        std::string token = header_b64 + "." + payload_b64 + ".";
        // Use HS256 as expected alg — alg:none must be rejected regardless
        auto vr = jwt::verify(token, "whatever-key", algorithm::HS256);
        check("alg:none rejected", vr.err == verify_error::alg_none_rejected);
    }

    // ============================================================
    // 7) token expiration (exp < now)
    // ============================================================
    std::cout << "[7] exp expired\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        std::string payload = R"({"sub":"x","exp":)" +
            std::to_string(std::time(nullptr) - 100) + R"(})";
        std::string token = jwt::encode(R"({"alg":"HS256"})", payload, key, algorithm::HS256);
        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("exp expired", vr.err == verify_error::token_expired);
    }

    // ============================================================
    // 8) exp + leeway tolerance
    // ============================================================
    std::cout << "[8] exp with leeway\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        std::string payload = R"({"sub":"x","exp":)" +
            std::to_string(std::time(nullptr) - 50) + R"(})";
        std::string token = jwt::encode(R"({"alg":"HS256"})", payload, key, algorithm::HS256);
        // leeway 0 -> expired; leeway 100 -> ok
        auto vr_tight = jwt::verify(token, key, algorithm::HS256, 0);
        auto vr_loose = jwt::verify(token, key, algorithm::HS256, 100);
        check("exp expired without leeway", vr_tight.err == verify_error::token_expired);
        check("exp ok with sufficient leeway", vr_loose.err == verify_error::ok);
    }

    // ============================================================
    // 9) malformed token
    // ============================================================
    std::cout << "[9] malformed token\n";
    {
        auto vr1 = jwt::verify("only-one-dot", "k", algorithm::HS256);
        check("1-segment rejected", vr1.err == verify_error::malformed_token);
        auto vr2 = jwt::verify("a.b", "k", algorithm::HS256);
        check("2-segment rejected", vr2.err == verify_error::malformed_token);
        auto vr3 = jwt::verify("a.b.c.d", "k", algorithm::HS256);
        check("4-segment rejected", vr3.err == verify_error::malformed_token);
    }

    // ============================================================
    // 10) decode_without_verify (doesn't check signature or alg)
    // ============================================================
    std::cout << "[10] decode_without_verify\n";
    {
        // even alg:none should parse here (intentional, for gateway routing)
        std::string header_b64 = http::b64url::encode(R"({"alg":"none"})");
        std::string payload_b64 = http::b64url::encode(R"({"sub":"ghost"})");
        std::string token = header_b64 + "." + payload_b64 + ".";
        auto vr = jwt::decode_without_verify(token);
        check("decode_without_verify reports parsed but unverified",
              vr.err == verify_error::parsed_but_unverified);
        check("alg field parsed as none", vr.alg == "none");
        check("payload parsed", vr.payload_json.find("ghost") != std::string::npos);
    }

    // ============================================================
    // 11) nbf not yet valid
    // ============================================================
    std::cout << "[11] nbf not yet valid\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        std::string payload = R"({"sub":"x","nbf":)" +
            std::to_string(std::time(nullptr) + 3600) + R"(})";
        std::string token = jwt::encode(R"({"alg":"HS256"})", payload, key, algorithm::HS256);
        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("nbf not-yet-valid rejected", vr.err == verify_error::token_not_yet_valid);
    }

    // Helper: generate RSA 2048 key pair, return (priv_pem, pub_pem)
    auto gen_rsa = []() -> std::pair<std::string, std::string> {
        EVP_PKEY *raw = EVP_PKEY_Q_keygen(nullptr, nullptr, "RSA", (size_t)2048);
        if (!raw) throw std::runtime_error("RSA keygen failed");
        http::EVP_PKEY_ptr key(raw);

        std::string priv_pem = http::ec_crypto::export_pem(key.get());

        BIO_ptr mem(BIO_new(BIO_s_mem()));
        PEM_write_bio_PUBKEY(mem.get(), key.get());
        char *data = nullptr;
        long len = BIO_get_mem_data(mem.get(), &data);
        std::string pub_pem(data, static_cast<size_t>(len));
        return {priv_pem, pub_pem};
    };

    // ============================================================
    // 12) RS256 sign -> verify with RSA key pair
    // ============================================================
    std::cout << "[12] RS256 round-trip\n";
    {
        auto [priv_pem, pub_pem] = gen_rsa();

        std::string token = jwt::encode(
            R"({"alg":"RS256"})",
            R"({"sub":"rsa-user","exp":)" + std::to_string(std::time(nullptr) + 3600) + R"(})",
            priv_pem,
            algorithm::RS256);
        check("RS256 token has 3 segments", std::count(token.begin(), token.end(), '.') == 2);

        auto vr = jwt::verify(token, pub_pem, algorithm::RS256);
        check("RS256 verify ok", vr.err == verify_error::ok);
        check("RS256 alg field", vr.alg == "RS256");
        check("RS256 payload", vr.payload_json.find("rsa-user") != std::string::npos);
    }

    // ============================================================
    // 13) RS256 wrong public key -> signature_mismatch
    // ============================================================
    std::cout << "[13] RS256 wrong key\n";
    {
        auto [priv1, _] = gen_rsa();
        auto [__pub1, pub2] = gen_rsa();

        std::string token = jwt::encode(
            R"({"alg":"RS256"})", R"({"sub":"x"})", priv1, algorithm::RS256);

        auto vr = jwt::verify(token, pub2, algorithm::RS256);
        check("RS256 wrong pubkey rejected", vr.err == verify_error::signature_mismatch);
    }

    // ============================================================
    // 14) RS256 tampered signature -> signature_mismatch
    // ============================================================
    std::cout << "[14] RS256 tamper detection\n";
    {
        auto [priv_pem, pub_pem] = gen_rsa();
        std::string token = jwt::encode(
            R"({"alg":"RS256"})", R"({"sub":"x"})", priv_pem, algorithm::RS256);

        // flip char near the middle of signature — RSA sig is long, safe to tamper
        auto last_dot = token.rfind('.');
        token[last_dot + 20] = (token[last_dot + 20] == 'A' ? 'B' : 'A');

        auto vr = jwt::verify(token, pub_pem, algorithm::RS256);
        check("RS256 tamper rejected", vr.err == verify_error::signature_mismatch);
    }

    // ============================================================
    // 15) PS256 sign -> verify with RSA key pair
    // ============================================================
    std::cout << "[15] PS256 round-trip\n";
    {
        auto [priv_pem, pub_pem] = gen_rsa();

        std::string token = jwt::encode(
            R"({"alg":"PS256"})",
            R"({"sub":"pss-user","exp":)" + std::to_string(std::time(nullptr) + 3600) + R"(})",
            priv_pem,
            algorithm::PS256);
        check("PS256 token has 3 segments", std::count(token.begin(), token.end(), '.') == 2);

        auto vr = jwt::verify(token, pub_pem, algorithm::PS256);
        check("PS256 verify ok", vr.err == verify_error::ok);
        check("PS256 alg field", vr.alg == "PS256");
        check("PS256 payload", vr.payload_json.find("pss-user") != std::string::npos);
    }

    // ============================================================
    // 16) PS256 wrong public key -> signature_mismatch
    // ============================================================
    std::cout << "[16] PS256 wrong key\n";
    {
        auto [priv1, _] = gen_rsa();
        auto [__pub1, pub2] = gen_rsa();

        std::string token = jwt::encode(
            R"({"alg":"PS256"})", R"({"sub":"x"})", priv1, algorithm::PS256);

        auto vr = jwt::verify(token, pub2, algorithm::PS256);
        check("PS256 wrong pubkey rejected", vr.err == verify_error::signature_mismatch);
    }

    // ============================================================
    // 17) alg whitelist blocks cross-alg verification
    //     (RS256 token with expected_alg=PS256 should hit alg_mismatch,
    //      NOT signature_mismatch — padding never gets a chance to run)
    // ============================================================
    std::cout << "[17] alg whitelist blocks cross-alg verify\n";
    {
        auto [priv_pem, pub_pem] = gen_rsa();
        std::string token = jwt::encode(
            R"({"alg":"RS256"})", R"({"sub":"x"})", priv_pem, algorithm::RS256);

        auto vr = jwt::verify(token, pub_pem, algorithm::PS256);
        check("RS256 token verified as PS256 -> alg_mismatch (whitelist first)",
              vr.err == verify_error::alg_mismatch);
    }

    // ============================================================
    // 18) RSA key type mismatch: use RSA private key to sign ES256
    //     encode() detects alg vs key type via OpenSSL DigestSignInit failure
    // ============================================================
    std::cout << "[18] alg vs key-type mismatch (RSA key for ES256)\n";
    {
        auto [rsa_priv, _] = gen_rsa();
        bool caught = false;
        try
        {
            jwt::encode(
                R"({"alg":"ES256"})", R"({"sub":"x"})", rsa_priv, algorithm::ES256);
        }
        catch (const std::exception &)
        {
            caught = true;
        }
        check("RSA key rejected for ES256", caught);
    }

    // ============================================================
    // 19) RSA key type mismatch: use ES256 key to sign RS256
    // ============================================================
    std::cout << "[19] alg vs key-type mismatch (EC key for RS256)\n";
    {
        http::EVP_PKEY_ptr ec_priv = http::ec_crypto::generate_p256();
        std::string ec_pem = http::ec_crypto::export_pem(ec_priv.get());

        bool caught = false;
        try
        {
            jwt::encode(
                R"({"alg":"RS256"})", R"({"sub":"x"})", ec_pem, algorithm::RS256);
        }
        catch (const std::exception &)
        {
            caught = true;
        }
        check("EC key rejected for RS256", caught);
    }

    // ============================================================
    // 20) encode throws when header.alg is MISSING
    // ============================================================
    std::cout << "[20] encode: header.alg missing throws\n";
    {
        bool caught = false;
        try
        {
            jwt::encode(
                R"({"some":"field"})", R"({"sub":"x"})", "k", algorithm::HS256);
        }
        catch (const std::exception &)
        {
            caught = true;
        }
        check("header.alg missing throws", caught);
    }

    // ============================================================
    // 21) encode throws when header.alg != requested algorithm
    // ============================================================
    std::cout << "[21] encode: header.alg mismatch throws\n";
    {
        bool caught = false;
        try
        {
            // header says ES256 but we request HS256
            jwt::encode(
                R"({"alg":"ES256"})", R"({"sub":"x"})", "key", algorithm::HS256);
        }
        catch (const std::exception &)
        {
            caught = true;
        }
        check("header.alg != parameter alg throws", caught);
    }

    // ============================================================
    // 22) typ:"JWT" auto-injected when missing
    // ============================================================
    std::cout << "[22] typ auto-injection\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        // No "typ" in header
        std::string token = jwt::encode(
            R"({"alg":"HS256"})",
            R"({"sub":"typ-test"})",
            key,
            algorithm::HS256);
        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("typ auto-injected -> verify ok", vr.err == verify_error::ok);
        check("typ present in decoded header", vr.header_json.find("\"typ\"") != std::string::npos);
        check("typ value is JWT", vr.header_json.find("\"JWT\"") != std::string::npos);
    }

    // ============================================================
    // 23) existing typ NOT overwritten by auto-injection
    // ============================================================
    std::cout << "[23] typ NOT overwritten when already present\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        std::string token = jwt::encode(
            R"({"alg":"HS256","typ":"custom"})",
            R"({"sub":"x"})",
            key,
            algorithm::HS256);
        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("existing typ preserved -> verify ok", vr.err == verify_error::ok);
        // Should NOT contain a second "typ" key; original "custom" should still be there
        check("original typ value retained", vr.header_json.find("\"custom\"") != std::string::npos);
    }

    // ============================================================
    // 24) RSA 4096 round-trip (RS256 + PS256)
    // ============================================================
    std::cout << "[24] RSA-4096 round-trip (RS256 + PS256)\n";
    {
        EVP_PKEY *raw = EVP_PKEY_Q_keygen(nullptr, nullptr, "RSA", (size_t)4096);
        if (!raw)
        {
            check("RSA-4096 keygen available", false);
        }
        else
        {
            http::EVP_PKEY_ptr key4096(raw);
            std::string priv_pem = http::ec_crypto::export_pem(key4096.get());

            BIO_ptr mem(BIO_new(BIO_s_mem()));
            PEM_write_bio_PUBKEY(mem.get(), key4096.get());
            char *data = nullptr;
            long len = BIO_get_mem_data(mem.get(), &data);
            std::string pub_pem(data, static_cast<size_t>(len));

            std::string exp = std::to_string(std::time(nullptr) + 3600);
            std::string rsa_payload = R"({"sub":"rsa4096","exp":)" + exp + R"(})";

            std::string token_rs = jwt::encode(
                R"({"alg":"RS256"})", rsa_payload, priv_pem, algorithm::RS256);
            auto vr_rs = jwt::verify(token_rs, pub_pem, algorithm::RS256);
            check("RSA-4096 RS256 verify ok", vr_rs.err == verify_error::ok);

            std::string token_ps = jwt::encode(
                R"({"alg":"PS256"})", rsa_payload, priv_pem, algorithm::PS256);
            auto vr_ps = jwt::verify(token_ps, pub_pem, algorithm::PS256);
            check("RSA-4096 PS256 verify ok", vr_ps.err == verify_error::ok);
        }
    }

    // ============================================================
    // 25) verify side: EC pubkey used as RS256 key -> signature_mismatch
    //     (OpenSSL DigestVerifyInit + RSA padding on EC key fails verification)
    // ============================================================
    std::cout << "[25] verify: EC pubkey as RS256 key -> signature_mismatch\n";
    {
        auto [rsa_priv, rsa_pub] = gen_rsa();
        http::EVP_PKEY_ptr ec_kp = http::ec_crypto::generate_p256();
        BIO_ptr mem(BIO_new(BIO_s_mem()));
        PEM_write_bio_PUBKEY(mem.get(), ec_kp.get());
        char *data = nullptr;
        long len = BIO_get_mem_data(mem.get(), &data);
        std::string ec_pub(data, static_cast<size_t>(len));

        std::string token = jwt::encode(
            R"({"alg":"RS256"})", R"({"sub":"x"})", rsa_priv, algorithm::RS256);

        // EC pubkey in RSA verify: import_pub_pem succeeds, then
        // EVP_PKEY_CTX_set_rsa_padding fails inside rsa_verify -> signature_mismatch
        auto vr = jwt::verify(token, ec_pub, algorithm::RS256);
        check("EC pubkey rejected for RS256 verify",
              vr.err == verify_error::signature_mismatch);
    }

    // ============================================================
    // 26) verify side: RSA pubkey used as ES256 key -> key_mismatch
    // ============================================================
    std::cout << "[26] verify: RSA pubkey as ES256 key -> key_mismatch\n";
    {
        // Generate EC key pair: priv for ES256 sign, pub for later test
        http::EVP_PKEY_ptr ec_kp = http::ec_crypto::generate_p256();
        std::string ec_priv = http::ec_crypto::export_pem(ec_kp.get());

        // RSA pubkey (from gen_rsa, second element) to use as mismatched verify key
        auto [_rsa_priv, rsa_pub] = gen_rsa();

        std::string ec_token = jwt::encode(
            R"({"alg":"ES256"})", R"({"sub":"x"})", ec_priv, algorithm::ES256);

        // An RSA key is not on any curve, so the ES256 branch reports it as the key error it
        // is; before the curve was asked, this same pair only ever produced signature_mismatch.
        auto vr = jwt::verify(ec_token, rsa_pub, algorithm::ES256);
        check("RSA pubkey rejected for ES256 verify",
              vr.err == verify_error::key_mismatch);
    }

    // ============================================================
    // 27) header is not a base64url segment -> invalid_base64
    //     The segment alphabet is checked before decoding, so garbage never
    //     reaches the alg lookup.
    // ============================================================
    std::cout << "[27] header all-illegal base64url\n";
    {
        // '@' is in neither the base64url nor the padded-base64 alphabet
        std::string bad_header = "@@@@";  // 4 chars, all illegal
        std::string valid_payload = http::b64url::encode(R"({"sub":"x"})");
        std::string valid_sig = "abc";

        std::string token = bad_header + "." + valid_payload + "." + valid_sig;
        auto vr = jwt::verify(token, "k", algorithm::HS256);
        check("illegal base64 header rejected",
              vr.err == verify_error::invalid_base64);
    }

    // ============================================================
    // 28) exp=0 present -> expired (RFC 7519 NumericDate), exp absent -> no check
    // ============================================================
    std::cout << "[28] exp=0 is a date, not \"no limit\"\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        // exp and nbf written out as 0 are the epoch, so the token is long expired
        std::string payload = R"({"sub":"x","exp":0,"nbf":0})";
        std::string token = jwt::encode(R"({"alg":"HS256"})", payload, key, algorithm::HS256);
        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("exp=0 treated as expired", vr.err == verify_error::token_expired);
        check("exp=0 reported as 0", vr.exp == 0);

        // No exp/nbf at all -> nothing to time-check
        std::string payload_no_time = R"({"sub":"x"})";
        std::string token2 = jwt::encode(R"({"alg":"HS256"})", payload_no_time, key, algorithm::HS256);
        auto vr2 = jwt::verify(token2, key, algorithm::HS256);
        check("missing exp/nbf -> no time check", vr2.err == verify_error::ok);
    }

    // ============================================================
    // 29) HS256 with SHORT key (< SHA-256 block size, HMAC spec allows)
    // ============================================================
    std::cout << "[29] HS256 short key (< 32 bytes)\n";
    {
        const std::string short_key = "tiny";  // 4 bytes, HMAC-SHA256 accepts any length
        std::string token = jwt::encode(
            R"({"alg":"HS256"})", R"({"sub":"short-key-user"})", short_key, algorithm::HS256);
        auto vr = jwt::verify(token, short_key, algorithm::HS256);
        check("HS256 with short key verify ok", vr.err == verify_error::ok);
    }

    // ============================================================
    // 30) verify result completeness: multiple claims all parsed correctly
    // ============================================================
    std::cout << "[30] verify_result field completeness\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        time_t now = std::time(nullptr);
        std::string payload =
            R"({"iss":"test-issuer","sub":"user123","aud":"my-api",)"
            R"("exp":)" + std::to_string(now + 3600) +
            R"(,"iat":)" + std::to_string(now) +
            R"(,"nbf":)" + std::to_string(now - 60) +
            R"(,"jti":"unique-jti-value"})";
        std::string token = jwt::encode(
            R"({"alg":"HS256","kid":"key-001"})",
            payload,
            key,
            algorithm::HS256);
        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("multi-claim token verify ok", vr.err == verify_error::ok);
        check("alg parsed as HS256", vr.alg == "HS256");
        check("header contains kid", vr.header_json.find("key-001") != std::string::npos);
        check("header contains alg", vr.header_json.find("HS256") != std::string::npos);
        check("header contains typ (auto-injected)", vr.header_json.find("\"typ\"") != std::string::npos);
        check("payload contains iss", vr.payload_json.find("test-issuer") != std::string::npos);
        check("payload contains sub", vr.payload_json.find("user123") != std::string::npos);
        check("payload contains aud", vr.payload_json.find("my-api") != std::string::npos);
        check("payload contains jti", vr.payload_json.find("unique-jti-value") != std::string::npos);
        check("exp correctly extracted", static_cast<time_t>(vr.exp) == now + 3600);
        check("iat correctly extracted", static_cast<time_t>(vr.iat) == now);
        check("nbf correctly extracted", static_cast<time_t>(vr.nbf) == now - 60);
    }

    // ============================================================
    // 31) payload tampering -> signature_mismatch (swap sub value)
    // ============================================================
    std::cout << "[31] payload tampering -> signature_mismatch\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        std::string token = jwt::encode(
            R"({"alg":"HS256"})", R"({"sub":"alice"})", key, algorithm::HS256);

        // payload = second segment — manually replace the b64 value
        auto first_dot = token.find('.');
        auto last_dot = token.rfind('.');
        std::string tampered_payload_b64 = http::b64url::encode(R"({"sub":"bob"})");
        std::string tampered = token.substr(0, first_dot + 1) +
                               tampered_payload_b64 +
                               token.substr(last_dot);

        auto vr = jwt::verify(tampered, key, algorithm::HS256);
        check("tampered payload rejected", vr.err == verify_error::signature_mismatch);
    }

    // ============================================================
    // 32) claims read from the top level only, and only as numbers
    // ============================================================
    std::cout << "[32] top-level claim scanner\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        const time_t past = std::time(nullptr) - 100;
        const time_t future = std::time(nullptr) + 3600;
        const std::string s_past = std::to_string(static_cast<long long>(past));
        const std::string s_future = std::to_string(static_cast<long long>(future));

        auto hs = [&](const std::string &payload)
        {
            std::string token = jwt::encode(R"({"alg":"HS256"})", payload, key, algorithm::HS256);
            return jwt::verify(token, key, algorithm::HS256);
        };

        // A quoted or null expiry is not "no expiry" — the check would silently vanish
        check("quoted exp rejected",
              hs("{\"exp\":\"" + s_past + "\"}").err == verify_error::invalid_claims);
        check("null exp rejected",
              hs("{\"exp\":null}").err == verify_error::invalid_claims);

        // 32-bit readers lose this value; time_t holds it
        auto vr_big = hs("{\"exp\":2500000000}");
        check("exp 2500000000 verifies ok", vr_big.err == verify_error::ok);
        check("exp 2500000000 value kept", vr_big.exp == 2500000000);

        // A nested same-named member neither counts nor shadows the top level
        auto vr_nested_only = hs("{\"meta\":{\"exp\":" + s_past + "}}");
        check("nested exp ignored", vr_nested_only.err == verify_error::ok);
        check("nested exp not extracted", vr_nested_only.exp == 0);
        auto vr_nested_shadow = hs("{\"meta\":{\"exp\":" + s_future + "},\"exp\":" + s_past + "}");
        check("top-level exp wins over nested", vr_nested_shadow.err == verify_error::token_expired);
        check("top-level exp value used", vr_nested_shadow.exp == past);

        // control arms: plain numbers still behave as before
        check("numeric past exp still expired",
              hs("{\"exp\":" + s_past + "}").err == verify_error::token_expired);
        auto vr_frac = hs("{\"exp\":" + s_past + ".5}");
        check("fractional exp still expired", vr_frac.err == verify_error::token_expired);
        check("fractional exp truncated", vr_frac.exp == past);

        // exponent form is not read as a date
        check("exponent exp rejected", hs("{\"exp\":1.0e9}").err == verify_error::invalid_claims);

        // decode_without_verify uses the same reader, so nested alg cannot be reported
        {
            std::string pls = http::b64url::encode(R"({"sub":"x"})");
            std::string hdr = http::b64url::encode(R"({"meta":{"alg":"ES256"},"alg":"HS256"})");
            auto dv = jwt::decode_without_verify(hdr + "." + pls + ".abc");
            check("decode reads top-level alg", dv.alg == "HS256");

            std::string hdr2 = http::b64url::encode(R"({"meta":{"alg":"HS256"},"typ":"JWT"})");
            auto dv2 = jwt::decode_without_verify(hdr2 + "." + pls + ".abc");
            check("decode reports empty alg when only nested", dv2.alg.empty());
        }

        // The signature is judged before the claims, so an unauthenticated caller cannot
        // probe the claim reader through the error code
        {
            std::string token = jwt::encode(
                R"({"alg":"HS256"})", "{\"exp\":\"x\"}", key, algorithm::HS256);
            auto first_dot = token.find('.');
            auto last_dot = token.rfind('.');
            std::string swapped_payload_b64 = http::b64url::encode("{\"exp\":null}");
            std::string tampered = token.substr(0, first_dot + 1) +
                                   swapped_payload_b64 +
                                   token.substr(last_dot);
            auto vr = jwt::verify(tampered, key, algorithm::HS256);
            check("signature checked before claims",
                  vr.err == verify_error::signature_mismatch);
        }
    }

    // ============================================================
    // 33) empty HMAC secret refused on both sides
    // ============================================================
    std::cout << "[33] empty HMAC secret\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";

        bool caught = false;
        std::string what;
        try
        {
            jwt::encode(R"({"alg":"HS256"})", R"({"sub":"x"})", "", algorithm::HS256);
        }
        catch (const std::invalid_argument &e)
        {
            caught = true;
            what = e.what();
        }
        check("encode with empty secret throws", caught);
        check("throw text names the empty secret", what.find("empty HMAC secret") != std::string::npos);

        // A token signed with a real key must not pass against an empty one
        std::string token = jwt::encode(R"({"alg":"HS256"})", R"({"sub":"x"})", key, algorithm::HS256);
        auto vr = jwt::verify(token, "", algorithm::HS256);
        check("verify with empty secret -> key_mismatch", vr.err == verify_error::key_mismatch);

        // Only the empty string is refused: a short secret still works
        std::string tiny_token = jwt::encode(
            R"({"alg":"HS256"})", R"({"sub":"x"})", "tiny", algorithm::HS256);
        check("4-byte secret still verifies",
              jwt::verify(tiny_token, "tiny", algorithm::HS256).err == verify_error::ok);

        // Non-HMAC algorithms keep their own failure text
        bool ec_caught = false;
        std::string ec_what;
        try
        {
            jwt::encode(R"({"alg":"ES256"})", R"({"sub":"x"})", "", algorithm::ES256);
        }
        catch (const std::invalid_argument &e)
        {
            ec_caught = true;
            ec_what = e.what();
        }
        check("ES256 with empty key reports the PEM failure",
              ec_caught && ec_what.find("private key") != std::string::npos);
    }

    // ============================================================
    // 34) encode requires a JSON object for both sections
    // ============================================================
    std::cout << "[34] encode JSON shape gate\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";

        auto refused = [&](const std::string &hdr, const std::string &payload) -> bool
        {
            try
            {
                jwt::encode(hdr, payload, key, algorithm::HS256);
                return false;
            }
            catch (const std::invalid_argument &)
            {
                return true;
            }
        };

        check("plain string payload refused", refused(R"({"alg":"HS256"})", "not-json-at-all"));
        check("truncated payload refused", refused(R"({"alg":"HS256"})", R"({"exp":)"));
        check("array payload refused", refused(R"({"alg":"HS256"})", R"([{"a":1}])"));
        check("non-object header refused", refused("HS256", R"({"sub":"x"})"));
        check("braces inside a string value are fine",
              !refused(R"({"alg":"HS256"})", R"({"note":"a{b}c"})"));
        check("empty object payload is signable", !refused(R"({"alg":"HS256"})", "{}"));

        std::string token = jwt::encode(
            R"({"alg":"HS256"})", R"({"note":"a{b}c{\"d}"})", key, algorithm::HS256);
        check("signed token with bracey value verifies",
              jwt::verify(token, key, algorithm::HS256).err == verify_error::ok);
    }

    // ============================================================
    // 35) structural gates in verify: object shape, crit, duplicated members
    // ============================================================
    std::cout << "[35] verify structural gates\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";

        auto hs = [&](const std::string &hdr, const std::string &payload)
        {
            std::string token = jwt::encode(hdr, payload, key, algorithm::HS256);
            return jwt::verify(token, key, algorithm::HS256);
        };

        // A hand-built section that is not an object is refused before the signature is
        // looked at, so the short signature here never gets used
        std::string hdr_b64 = b64url::encode(R"({"alg":"HS256"})");
        std::string arr_b64 = b64url::encode(R"([{"exp":1}])");
        auto vr_arr = jwt::verify(hdr_b64 + "." + arr_b64 + ".abc", key, algorithm::HS256);
        check("array payload refused as malformed json",
              vr_arr.err == verify_error::malformed_json);

        std::string plain_hdr = b64url::encode("HS256");
        std::string obj_b64   = b64url::encode(R"({"exp":9999999999})");
        auto vr_hdr = jwt::verify(plain_hdr + "." + obj_b64 + ".abc", key, algorithm::HS256);
        check("non-object header refused as malformed json",
              vr_hdr.err == verify_error::malformed_json);

        // control: same signature length, but both sections are objects -> the gate stays out
        auto vr_shape_control = jwt::verify(hdr_b64 + "." + obj_b64 + ".abc", key, algorithm::HS256);
        check("object sections reach the signature check",
              vr_shape_control.err == verify_error::signature_mismatch);

        // crit names an extension this implementation cannot understand, so it ends the
        // verification even with a perfectly valid signature
        auto vr_crit = hs(R"({"alg":"HS256","crit":["b64u"]})", R"({"sub":"x"})");
        check("crit header refused", vr_crit.err == verify_error::unsupported_critical);
        auto vr_crit_nested = hs(R"({"alg":"HS256","meta":{"crit":["b64u"]}})", R"({"sub":"x"})");
        check("nested crit is not a critical header", vr_crit_nested.err == verify_error::ok);
        auto vr_no_crit = hs(R"({"alg":"HS256"})", R"({"sub":"x"})");
        check("header without crit still verifies", vr_no_crit.err == verify_error::ok);

        // Two members with the same name: which one wins has no agreed answer, so the
        // verifier refuses instead of picking the one this scanner happens to read first
        auto vr_dup_alg = hs(R"({"alg":"HS256","alg":"HS256"})", R"({"sub":"x"})");
        check("duplicated alg refused", vr_dup_alg.err == verify_error::alg_mismatch);
        const time_t future = std::time(nullptr) + 3600;
        const std::string s_future = std::to_string(static_cast<long long>(future));
        auto vr_dup_exp = hs(R"({"alg":"HS256"})",
                             "{\"exp\":" + s_future + ",\"exp\":" + s_future + "}");
        check("duplicated exp refused even when identical",
              vr_dup_exp.err == verify_error::invalid_claims);
        auto vr_single_exp = hs(R"({"alg":"HS256"})", "{\"exp\":" + s_future + "}");
        check("one exp of the same value still verifies", vr_single_exp.err == verify_error::ok);
    }

    // ============================================================
    // 36) a closed-off tail does not discard what was already read
    // ============================================================
    std::cout << "[36] scanner tail with an unterminated string\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        const time_t      future = std::time(nullptr) + 3600;
        const std::string s_future = std::to_string(static_cast<long long>(future));

        std::string hdr_b64 = b64url::encode(R"({"alg":"HS256"})");

        std::string cut_b64 = b64url::encode("{\"exp\":" + s_future + ",\"note\":\"unclosed");
        auto vr_cut = jwt::verify(hdr_b64 + "." + cut_b64 + ".abc", key, algorithm::HS256);
        check("verify still refuses the unclosed payload",
              vr_cut.err == verify_error::malformed_json);

        // The parse-only reader keeps whatever it read before the tail, so exp stays put
        auto dv_cut = jwt::decode_without_verify(hdr_b64 + "." + cut_b64 + ".abc");
        check("decode keeps exp read before an unclosed tail", dv_cut.exp == future);
        check("decode still reports parsed but unverified",
              dv_cut.err == verify_error::parsed_but_unverified);

        // The duplicates counted before the tail are reported too, so a repeated claim is
        // still judged illegal rather than quietly resolved to the first one
        std::string dup_cut_b64 = b64url::encode(
            "{\"exp\":" + s_future + ",\"exp\":" + s_future + ",\"note\":\"unclosed");
        auto dv_dup_cut = jwt::decode_without_verify(hdr_b64 + "." + dup_cut_b64 + ".abc");
        check("decode drops a claim whose duplicates were counted before the tail",
              dv_dup_cut.exp == 0);

        // No member at all before the tail: still nothing to hand back
        std::string only_tail_b64 = b64url::encode("{\"note\":\"unclosed");
        auto dv_only = jwt::decode_without_verify(hdr_b64 + "." + only_tail_b64 + ".abc");
        check("decode reports 0 when only the tail was scanned", dv_only.exp == 0);
    }

    // ============================================================
    // 37) typ injection when the header object is padded with whitespace
    // ============================================================
    std::cout << "[37] typ injection with whitespace in the header\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        std::string token = jwt::encode(
            R"({ "alg":"HS256" })",
            R"({"sub":"ws-header"})",
            key,
            algorithm::HS256);
        auto vr = jwt::verify(token, key, algorithm::HS256);
        check("whitespace-padded header verifies", vr.err == verify_error::ok);
        const size_t first_typ = vr.header_json.find("\"typ\"");
        check("whitespace-padded header got exactly one typ",
              first_typ != std::string::npos &&
              vr.header_json.find("\"typ\"", first_typ + 1) == std::string::npos);
        check("typ appended as a member, never as an empty-list comma",
              vr.header_json.find("{ ,") == std::string::npos &&
              vr.header_json.find(",\"typ\"") != std::string::npos);

        // 空白填满的空对象没有任何成员，也就没声明 alg：encode() 在写 typ 之前就先拒了
        bool caught = false;
        try
        {
            jwt::encode(R"({ })", R"({"sub":"x"})", key, algorithm::HS256);
        }
        catch (const std::exception &)
        {
            caught = true;
        }
        check("empty-after-trim header never reaches typ injection", caught);
    }

    // ============================================================
    // 38) three gates: section syntax, signature alphabet, error-path payload
    // ============================================================
    std::cout << "[38] syntax gate, signature alphabet, error path keeps no text\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        const time_t      future = std::time(nullptr) + 3600;
        const std::string s_future = std::to_string(static_cast<long long>(future));

        // A member with no colon is not one JSON document: the section used to pass the
        // brace-balance shape check, the scanner found no exp in it, and the token only
        // failed at the signature.
        std::string hdr_b64      = http::b64url::encode(R"({"alg":"HS256"})");
        std::string nocolon_b64  = http::b64url::encode(R"({"exp" "x"})");
        auto vr_nocolon = jwt::verify(hdr_b64 + "." + nocolon_b64 + ".abc", key, algorithm::HS256);
        check("a member without a colon is refused as malformed json",
              vr_nocolon.err == verify_error::malformed_json);
        check("a section refused on syntax is not handed back as text",
              vr_nocolon.header_json.empty() && vr_nocolon.payload_json.empty());

        // The signature does not cover the tampered payload, so the verdict is reported and
        // the bytes the caller has not authenticated are not.
        std::string live = jwt::encode(R"({"alg":"HS256"})",
                                      "{\"exp\":" + s_future + ",\"sub\":\"alice\"}",
                                      key,
                                      algorithm::HS256);
        const size_t last_dot = live.rfind('.');
        std::string tampered  = live.substr(0, last_dot + 1) +
                                http::b64url::encode("{\"exp\":" + s_future + ",\"sub\":\"bob\"}");
        auto vr_tamper = jwt::verify(tampered, key, algorithm::HS256);
        check("tampered payload still reports a signature mismatch",
              vr_tamper.err == verify_error::signature_mismatch);
        check("an error result carries no section text",
              vr_tamper.header_json.empty() && vr_tamper.payload_json.empty());
        check("an error result keeps the diagnostics",
              vr_tamper.alg == "HS256" && static_cast<time_t>(vr_tamper.exp) == future);

        // Completing the padding of the signature segment folded back into the very same
        // signature bytes and used to verify. How many characters do that depends on the
        // segment length, so the fixture computes it instead of hard-coding one '='.
        const std::string sig_b64 = live.substr(last_dot + 1);
        const size_t      pad     = (4 - sig_b64.size() % 4) % 4;
        check("this algorithm leaves a padding fixture to fold with", pad > 0);
        if (pad > 0)
        {
            auto vr_pad = jwt::verify(live + std::string(pad, '='), key, algorithm::HS256);
            check("a padded signature segment is refused as invalid base64",
                  vr_pad.err == verify_error::invalid_base64);
        }
        check("the same token with no padding still verifies",
              jwt::verify(live, key, algorithm::HS256).err == verify_error::ok);
    }

    std::cout << "[39] token length cap, ES256 curve, escaped member names\n";
    {
        const std::string key = "my-32-byte-secret-key---XXXXXX";
        const time_t      past   = std::time(nullptr) - 3600;
        const time_t      future = std::time(nullptr) + 3600;
        const std::string s_past   = std::to_string(static_cast<long long>(past));
        const std::string s_future = std::to_string(static_cast<long long>(future));

        // ---- the whole token is bounded before any segment is decoded ----
        // 界上的令牌必须照常可用，所以这里把填充调到 token.size() 恰好等于界，再+1 去看它被拒。
        // The cap has to leave a real token alone, so the fixture is sized to land exactly on it
        // and then one byte past it.
        auto token_of_size = [&](size_t want) -> std::string
        {
            size_t pad = want;
            std::string tok;
            for (int guard = 0; guard < 64; ++guard)
            {
                const std::string payload =
                    std::string("{\"s\":\"") + std::string(pad, 'a') + "\"}";
                tok = jwt::encode(R"({"alg":"HS256"})", payload, key, algorithm::HS256);
                const long long diff = static_cast<long long>(want)
                                     - static_cast<long long>(tok.size());
                if (diff == 0)
                    return tok;
                if (static_cast<long long>(pad) + diff < 0)
                    break;
                pad = static_cast<size_t>(static_cast<long long>(pad) + diff);
            }
            return "";
        };
        const std::string at_cap   = token_of_size(CONST_JWT_TOKEN_MAX_LEN);
        const std::string over_cap = token_of_size(CONST_JWT_TOKEN_MAX_LEN + 1);
        check("a fixture can be sized to the cap exactly",
              at_cap.size() == CONST_JWT_TOKEN_MAX_LEN);
        check("a token at the cap still verifies",
              jwt::verify(at_cap, key, algorithm::HS256).err == verify_error::ok);
        check("one byte over the cap is refused before any segment is decoded",
              over_cap.size() == CONST_JWT_TOKEN_MAX_LEN + 1 &&
              jwt::verify(over_cap, key, algorithm::HS256).err == verify_error::token_too_long);

        // ---- ES256 names its curve ----
        auto ec_pub_pem = [](int nid) -> std::string
        {
            std::string pem;
            EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
            EVP_PKEY *k = nullptr;
            if (ctx)
            {
                EVP_PKEY_keygen_init(ctx);
                EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, nid);
                EVP_PKEY_keygen(ctx, &k);
                EVP_PKEY_CTX_free(ctx);
            }
            if (k)
            {
                BIO *mem = BIO_new(BIO_s_mem());
                PEM_write_bio_PUBKEY(mem, k);
                char *data = nullptr;
                const long len = BIO_get_mem_data(mem, &data);
                pem.assign(data, static_cast<size_t>(len));
                BIO_free(mem);
                EVP_PKEY_free(k);
            }
            return pem;
        };
        http::EVP_PKEY_ptr signer = http::ec_crypto::generate_p256();
        std::string signer_pub;
        {
            BIO_ptr mem(BIO_new(BIO_s_mem()));
            PEM_write_bio_PUBKEY(mem.get(), signer.get());
            char *data = nullptr;
            const long len = BIO_get_mem_data(mem.get(), &data);
            signer_pub.assign(data, static_cast<size_t>(len));
        }
        const std::string es_token = jwt::encode(
            R"({"alg":"ES256"})", R"({"sub":"x"})",
            http::ec_crypto::export_pem(signer.get()), algorithm::ES256);
        check("ES256 refuses a P-384 public key as a key problem",
              jwt::verify(es_token, ec_pub_pem(NID_secp384r1), algorithm::ES256).err
                  == verify_error::key_mismatch);
        // secp256k1 与 P-256 同宽 256 位：认曲线而不是认位数，这一条就是两者的分界。
        // Same field width as P-256, different curve — this is why the check is a name, not a bit count.
        check("ES256 refuses secp256k1, the other 256-bit curve",
              jwt::verify(es_token, ec_pub_pem(NID_secp256k1), algorithm::ES256).err
                  == verify_error::key_mismatch);
        check("ES256 keeps reporting a different P-256 key as a signature mismatch",
              jwt::verify(es_token, ec_pub_pem(NID_X9_62_prime256v1), algorithm::ES256).err
                  == verify_error::signature_mismatch);

        // ---- a member name is read as JSON spells it ----
        // {"\u0065xp":...} 的键名就是 exp：改前扫描器看不见它，签名一过就当"永不过期"收下了。
        // The name of {"\u0065xp":...} is exp; the literal scanner used to miss it, so a passed
        // signature plus "no exp" made an expired token live.
        check("an escaped exp name is still the exp claim",
              jwt::verify(jwt::encode(R"({"alg":"HS256"})",
                                      "{\"\\u0065xp\":" + s_past + "}", key, algorithm::HS256),
                          key, algorithm::HS256).err == verify_error::token_expired);
        check("every escape in the name still lands on exp",
              jwt::verify(jwt::encode(R"({"alg":"HS256"})",
                                      "{\"\\u0065\\u0078\\u0070\":" + s_past + "}",
                                      key, algorithm::HS256),
                          key, algorithm::HS256).err == verify_error::token_expired);
        // \u0045 是大写 E：解码后是另一个成员，不该被当成 exp。
        // \u0045 is a capital E: decoded it names a different member, which must not read as exp.
        check("a name that decodes to something else names no claim",
              jwt::verify(jwt::encode(R"({"alg":"HS256"})",
                                      "{\"\\u0045xp\":" + s_past + "}", key, algorithm::HS256),
                          key, algorithm::HS256).err == verify_error::ok);
        check("an escaped name and its literal twin count as one repeated claim",
              jwt::verify(jwt::encode(R"({"alg":"HS256"})",
                                      "{\"exp\":" + s_future + ",\"\\u0065xp\":" + s_future + "}",
                                      key, algorithm::HS256),
                          key, algorithm::HS256).err == verify_error::invalid_claims);
        const std::string esc_typ_header = R"({"alg":"HS256","\u0074yp":"JWT"})";
        const std::string esc_typ_token =
            jwt::encode(esc_typ_header, "{\"exp\":" + s_future + "}", key, algorithm::HS256);
        check("an escaped typ name is already a typ, so encode adds no second one",
              http::b64url::decode(esc_typ_token.substr(0, esc_typ_token.find('.')))
                  == esc_typ_header);
    }

    std::cout << "\n========================================\n"
              << "Results: " << pass << " passed, " << fail << " failed\n"
              << "========================================\n";
    return fail == 0 ? 0 : 1;
}
