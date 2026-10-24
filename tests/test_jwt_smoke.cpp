/**
 * jwt_smoke.cpp — minimal functional verification for http::jwt
 * Compile: g++ -std=c++20 -I./vendor/httpserver/include jwt_smoke.cpp vendor/httpserver/src/jwt.cpp -lcrypto -lssl -o jwt_smoke
 * Run:     ./jwt_smoke
 */

#include "jwt.h"
#include <openssl/evp.h>
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
        check("decode_without_verify ok despite alg:none", vr.err == verify_error::ok);
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
    // 26) verify side: RSA pubkey used as ES256 key -> signature_mismatch
    // ============================================================
    std::cout << "[26] verify: RSA pubkey as ES256 key -> signature_mismatch\n";
    {
        // Generate EC key pair: priv for ES256 sign, pub for later test
        http::EVP_PKEY_ptr ec_kp = http::ec_crypto::generate_p256();
        std::string ec_priv = http::ec_crypto::export_pem(ec_kp.get());

        // RSA pubkey (from gen_rsa, second element) to use as mismatched verify key
        auto [_rsa_priv, rsa_pub] = gen_rsa();

        std::string ec_token = jwt::encode(
            R"({"alg":"ES256"})", R"({"sub":"x"})", ec_priv, algorithm::ES256);

        // RSA pubkey in ES256 verify: the r||s segment is 256 bytes, so es256_verify
        // refuses at the 64-byte clamp -> signature_mismatch
        auto vr = jwt::verify(ec_token, rsa_pub, algorithm::ES256);
        check("RSA pubkey rejected for ES256 verify",
              vr.err == verify_error::signature_mismatch);
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

    std::cout << "\n========================================\n"
              << "Results: " << pass << " passed, " << fail << " failed\n"
              << "========================================\n";
    return fail == 0 ? 0 : 1;
}
