/**
 * acme_client.cpp
 * ACME 协议客户端
 * 实现完整的证书申请流程：
 *   目录发现 → 创建账号(EAB) → 新建订单 → 获取授权 →
 *   HTTP-01 挑战 → 响应挑战 → 提交 CSR → 下载证书
 * 2026-07-12
 */

#include "crypto_ptr.h"
#include "acme_crypto.hpp"
#include "acme_client.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>

namespace http
{
acme_client::acme_client(const std::string &acme_server_url,
                         const std::string &eab_kid,
                         const std::string &eab_hmac_key,
                         const std::string &email)
    : server_url_(acme_server_url), eab_kid_(eab_kid), eab_hmac_key_(eab_hmac_key), email_(email)
{
    // 生成账号 EC P-256 密钥对
    account_key_ = ec_crypto::generate_p256();
}

// ========================================================
//  Step 1: 获取 ACME 目录
// ========================================================
acme_directory_t acme_client::fetch_directory()
{
    std::ostringstream oss;
    oss << "  [ACME] 获取目录: " << server_url_ << "\n";

    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->get(server_url_);
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->send();

    if (https->get_status() != 200)
        throw std::runtime_error("目录获取失败: HTTP " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());

    if (https->page.isjson == 1)
    {
        dir_.new_nonce   = https->page.json["newNonce"].to_string();
        dir_.new_account = https->page.json["newAccount"].to_string();
        dir_.new_order   = https->page.json["newOrder"].to_string();
    }
    else
    {
        throw std::runtime_error("目录获取失败: JSON " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());
    }

    oss << "  [ACME] newNonce:   " << dir_.new_nonce << "\n";
    oss << "  [ACME] newAccount: " << dir_.new_account << "\n";
    oss << "  [ACME] newOrder:   " << dir_.new_order << "\n";
    message_debug.append(oss.str());
    return dir_;
}

// ========================================================
//  Step 2: 获取 Nonce
// ========================================================
std::string acme_client::fetch_nonce()
{
    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->get(dir_.new_nonce);
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->send();

    if (https->get_status() < 200 || https->get_status() > 299)
        throw std::runtime_error("获取 Nonce: HTTP " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());

    std::string nonce = https->page.header["Replay-Nonce"];
    if (nonce.empty())
        throw std::runtime_error("未获取到 Replay-Nonce");
    return nonce;
}

// ========================================================
//  Step 3: 创建账号 (带 EAB)
// ========================================================
std::string acme_client::create_account()
{
    std::ostringstream oss;
    oss << "\n  [ACME] 创建账号 (EAB 绑定)...\n";

    std::string nonce = fetch_nonce();
    std::string jwk   = jws::make_jwk(account_key_.get());

    // 构造 EAB JWS (HS256)
    std::string eab_jws = jws::make_eab_jws(
        jwk,
        eab_kid_,
        eab_hmac_key_,
        dir_.new_account);

    // 构造 payload
    auto payload = R"({"contact":["mailto:)" + email_ +
                   R"("],"termsOfServiceAgreed":true,"externalAccountBinding":)" + eab_jws + "}";

    std::string body = jws::make_signed_body(
        payload,
        dir_.new_account,
        nonce,
        account_key_.get());

    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->post(dir_.new_account);
    https->add_header("Content-Type", "application/jose+json");
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->set_body(body);
    https->send();

    if (https->get_status() != 200 && https->get_status() != 201)
        throw std::runtime_error("账号创建失败: HTTP " + std::to_string(https->get_status()) + " body: " + https->get_body());

    // 从 Location header 获取 kid
    kid_ = https->page.header["Location"];
    oss << "  [ACME] 账号 kid: " << kid_ << "\n";
    message_debug.append(oss.str());
    return kid_;
}

// ========================================================
//  Step 4: 新建证书订单
// ========================================================
std::string acme_client::new_order(const std::vector<std::string> &domains)
{
    std::ostringstream oss;
    oss << "\n  [ACME] 新建订单...\n";

    std::string nonce = fetch_nonce();

    // 构造 identifiers
    std::string ids_arr;
    for (size_t i = 0; i < domains.size(); ++i)
    {
        if (i > 0)
            ids_arr += ",";
        ids_arr += R"({"type":"dns","value":")" + domains[i] + R"("})";
    }
    auto ids = R"({"identifiers":[)" + ids_arr + "]}";

    std::string body = jws::make_signed_body(
        ids,
        dir_.new_order,
        nonce,
        account_key_.get(),
        kid_);

    // auto url = parse_url(dir_.new_order);
    // auto res = https_.post(url.host, url.port, url.target, body);

    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->post(dir_.new_order);
    https->add_header("Content-Type", "application/jose+json");
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->set_body(body);
    https->send();

    if (https->get_status() != 201)
        throw std::runtime_error("订单创建失败: HTTP " + std::to_string(https->get_status()) + " body: " + https->get_body());

    order_url_ = https->page.header["Location"];

    if (https->page.isjson == 1)
    {
        finalize_url_ = https->page.json["finalize"].to_string();

        if (https->page.json["authorizations"].is_array())
        {
            authz_url_ = https->page.json["authorizations"][0].to_string();
        }
    }
    else
    {
        throw std::runtime_error("订单创建失败: JSON " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());
    }

    // // 提取 finalize URL
    // finalize_url_ = json_util::get_string(res.body, "finalize");

    // // 提取 authorizations 数组中第一个
    // authz_url_ = extract_first_authz(res.body);

    oss << "  [ACME] 订单 URL:    " << order_url_ << "\n";
    oss << "  [ACME] 授权 URL:    " << authz_url_ << "\n";
    oss << "  [ACME] Finalize URL: " << finalize_url_ << "\n";
    message_debug.append(oss.str());
    return order_url_;
}

// ========================================================
//  Step 5: 获取授权 & 提取 HTTP-01 挑战
// ========================================================
http01_challenge acme_client::get_http01_challenge()
{
    std::ostringstream oss;
    oss << "\n  [ACME] 获取授权挑战...\n";

    std::string nonce = fetch_nonce();
    std::string body  = jws::make_signed_body(
        "",
        authz_url_,
        nonce,
        account_key_.get(),
        kid_);

    // auto url = parse_url(authz_url_);
    // auto res = https_.post(url.host, url.port, url.target, body);

    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->post(authz_url_);
    https->add_header("Content-Type", "application/jose+json");
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->set_body(body);
    https->send();

    if (https->get_status() != 200)
        throw std::runtime_error("获取授权失败: HTTP " + std::to_string(https->get_status()) + " body: " + https->get_body());

    if (https->page.isjson == 1)
    {
    }
    else
    {
        throw std::runtime_error("获取授权失败: JSON " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());
    }
    // 查找 http-01 类型的挑战

    http01_challenge c;
    if (https->page.json["challenges"].is_array())
    {
        auto http01 = https->page.json["challenges"][0];
        c.token     = http01["type"].to_string();
        if (c.token == "http-01")
        {
            c.token = http01["token"].to_string();
            c.url   = http01["url"].to_string();
        }
        else
        {
            if (https->page.json["challenges"].size() == 2)
            {
                auto http02 = https->page.json["challenges"][1];
                c.token     = http02["type"].to_string();
                if (c.token == "http-01")
                {
                    c.token = http02["token"].to_string();
                    c.url   = http02["url"].to_string();
                }
            }
        }
    }

    // std::string challenge = json_util::find_in_array(
    //     res.body, "challenges", "type", "http-01");

    // if (challenge.empty())
    //     throw std::runtime_error("未找到 http-01 挑战");

    // http01_challenge c;
    // c.token = json_util::get_string(challenge, "token");
    // c.url   = json_util::get_string(challenge, "url");

    oss << "  [ACME] HTTP-01 token: " << c.token << "\n";
    oss << "  [ACME] Challenge URL: " << c.url << "\n";
    message_debug.append(oss.str());
    return c;
}

// ========================================================
//  Step 6: 准备 HTTP-01 验证文件
// ========================================================
void acme_client::prepare_http01(const std::string &webroot, const std::string &token)
{
    std::ostringstream oss;
    oss << "\n  [ACME] 准备 HTTP-01 验证文件...\n";

    // key authorization = token + "." + JWK Thumbprint
    std::string thumbprint = jws::jwk_thumbprint(account_key_.get());
    std::string key_auth   = token + "." + thumbprint;

    oss << "  [ACME] JWK Thumbprint: " << thumbprint << "\n";
    oss << "  [ACME] Key Authorization: " << key_auth << "\n";

    // 写入 webroot/.well-known/acme-challenge/<token>
    std::string challenge_dir = webroot + "/.well-known/acme-challenge";
    fs::create_directories(challenge_dir);

    std::string file_path = challenge_dir + "/" + token;
    std::ofstream ofs(file_path);
    if (!ofs)
        throw std::runtime_error("无法创建验证文件: " + file_path);
    ofs << key_auth;
    ofs.close();

    oss << "  [ACME] 验证文件已写入: " << file_path << "\n";
    message_debug.append(oss.str());
}

// ========================================================
//  Step 7: 响应挑战 (通知 ACME 服务器验证)
// ========================================================
void acme_client::respond_challenge(const std::string &challenge_url)
{
    std::ostringstream oss;
    oss << "\n  [ACME] 响应挑战...\n";

    std::string nonce = fetch_nonce();
    std::string body  = jws::make_signed_body(
        "{}",
        challenge_url,
        nonce,
        account_key_.get(),
        kid_);

    // auto url = parse_url(challenge_url);
    // auto res = https_.post(url.host, url.port, url.target, body);
    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->post(challenge_url);
    https->add_header("Content-Type", "application/jose+json");
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->set_body(body);
    https->send();

    if (https->get_status() != 200)
        throw std::runtime_error("响应挑战失败: HTTP " + std::to_string(https->get_status()) + " body: " + https->get_body());

    oss << "  [ACME] 挑战已提交，等待验证...\n";
    message_debug.append(oss.str());
}

// ========================================================
//  Step 8: 轮询授权状态
// ========================================================
void acme_client::poll_authorization(int max_retries, int interval_sec)
{
    std::ostringstream oss;
    oss << "  [ACME] 轮询授权状态";

    for (int i = 0; i < max_retries; ++i)
    {
        oss << ".";

        std::string nonce = fetch_nonce();
        std::string body  = jws::make_signed_body(
            "",
            authz_url_,
            nonce,
            account_key_.get(),
            kid_);

        // auto url = parse_url(authz_url_);
        // auto res = https_.post(url.host, url.port, url.target, body);

        std::shared_ptr<http::client> https = std::make_shared<http::client>();
        https->post(authz_url_);
        https->add_header("Content-Type", "application/jose+json");
        https->add_header("Connection", "close");
        https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
        https->timeout(60);
        https->set_body(body);
        https->send();

        if (https->get_status() != 200)
            throw std::runtime_error("授权验证失败: HTTP " + std::to_string(https->get_status()) + " body: " + https->get_body());

        if (https->page.isjson == 1)
        {
        }
        else
        {
            throw std::runtime_error("授权验证失败: JSON " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());
        }

        std::string status = https->page.json["status"].to_string();
        //std::string status = json_util::get_string(res.body, "status");

        if (status == "valid")
        {
            oss << " 验证通过!\n";
            message_debug.append(oss.str());
            return;
        }
        if (status == "invalid")
        {
            oss << " 验证失败!\n";
            message_debug.append(oss.str());
            std::string err = json_util::get_string(https->get_body(), "detail");
            throw std::runtime_error("授权验证失败: " + err +
                                     "\n响应: " + https->get_body());
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval_sec));
    }
    message_debug.append(oss.str());
    throw std::runtime_error("授权轮询超时");
}

// ========================================================
//  Step 9: 生成私钥 + CSR，提交 finalize
// ========================================================
EVP_PKEY_ptr acme_client::finalize_order(const std::vector<std::string> &domains,
                                         const std::string &output_dir)
{
    std::ostringstream oss;
    oss << "\n  [ACME] 生成证书私钥 + CSR...\n";

    // 生成证书私钥 (EC P-256, OpenSSL 3.0 EVP API)
    EVP_PKEY_ptr cert_pkey = ec_crypto::generate_p256();

    // 保存私钥 PEM
    std::string privkey_pem = ec_crypto::export_pem(cert_pkey.get());
    write_file(output_dir + "/privkey.pem", privkey_pem);
    oss << "  [ACME] 私钥已保存: " << output_dir << "/privkey.pem\n";

    // 生成 CSR
    std::string csr_pem = csr::generate_csr_pem(cert_pkey.get(), domains);

    // CSR PEM -> DER -> base64url
    std::string csr_b64 = csr::pem_to_der_b64url(csr_pem);

    oss << "  [ACME] CSR 已生成 (" << domains.size() << " 个域名)\n";

    // 提交 finalize
    oss << "  [ACME] 提交 CSR (finalize)...\n";

    std::string nonce = fetch_nonce();
    auto payload      = R"({"csr":")" + csr_b64 + R"("})";

    std::string body = jws::make_signed_body(
        payload,
        finalize_url_,
        nonce,
        account_key_.get(),
        kid_);

    // auto url = parse_url(finalize_url_);
    // auto res = https_.post(url.host, url.port, url.target, body);

    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->post(finalize_url_);
    https->add_header("Content-Type", "application/jose+json");
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->set_body(body);
    https->send();

    if (https->get_status() != 200)
        throw std::runtime_error("Finalize 失败: HTTP " + std::to_string(https->get_status()) + " body: " + https->get_body());

    message_debug.append(oss.str());
    return cert_pkey;
}

void acme_client::write_file(const std::string &path, const std::string &content)
{
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream ofs(path);
    if (!ofs)
        throw std::runtime_error("无法写入文件: " + path);
    ofs << content;
}

// 从 PEM 链中提取 CA 证书 (去掉第一个叶子证书)
std::string acme_client::extract_chain(const std::string &pem)
{
    auto first_end = pem.find("-----END CERTIFICATE-----");
    if (first_end == std::string::npos)
        return {};
    first_end += std::string("-----END CERTIFICATE-----").size();
    while (first_end < pem.size() && pem[first_end] == '\n')
        first_end++;
    return pem.substr(first_end);
}

// ========================================================
//  Step 10: 轮询订单状态 + 下载证书
// ========================================================
void acme_client::download_certificate(const std::string &output_dir, int max_retries, int interval_sec)
{
    std::ostringstream oss;
    oss << "  [ACME] 等待证书签发";

    std::string cert_url;

    for (int i = 0; i < max_retries; ++i)
    {
        oss << ".";

        std::string nonce = fetch_nonce();
        std::string body  = jws::make_signed_body(
            "",
            order_url_,
            nonce,
            account_key_.get(),
            kid_);

        // auto url = parse_url(order_url_);
        // auto res = https_.post(url.host, url.port, url.target, body);

        std::shared_ptr<http::client> https = std::make_shared<http::client>();
        https->post(order_url_);
        https->add_header("Content-Type", "application/jose+json");
        https->add_header("Connection", "close");
        https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
        https->timeout(60);
        https->set_body(body);
        https->send();

        if (https->get_status() < 200 || https->get_status() > 299)
            throw std::runtime_error("证书签发失败: HTTP " + std::to_string(https->get_status()) + " body: " + https->get_body());

        if (https->page.isjson == 1)
        {
            //std::string status = json_util::get_string(res.body, "status");
            std::string status = https->page.json["status"].to_string();

            if (status == "valid")
            {

                cert_url = https->page.json["certificate"].to_string();// json_util::get_string(res.body, "certificate");
                oss << " 证书已签发!\n";
                break;
            }
            if (status == "invalid")
            {
                throw std::runtime_error("订单状态 invalid: " + https->get_body());
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval_sec));
    }

    if (cert_url.empty())
        throw std::runtime_error("证书签发超时");

    // 下载证书
    oss << "  [ACME] 下载证书: " << cert_url << "\n";

    std::string nonce = fetch_nonce();
    std::string body  = jws::make_signed_body(
        "",
        cert_url,
        nonce,
        account_key_.get(),
        kid_);

    // auto url = parse_url(cert_url);
    // oss<<" host: "<< url.host<<std::endl;
    // oss<<" port: "<< url.port<<std::endl;
    oss << " cert_url: " << cert_url << std::endl;
    oss << " body: " << body << std::endl;

    // auto res = https_.post(url.host, url.port, url.target, body,
    //                        "application/jose+json");

    std::shared_ptr<http::client> https_down = std::make_shared<http::client>();
    https_down->post(cert_url);
    https_down->add_header("Content-Type", "application/jose+json");
    https_down->add_header("Connection", "close");
    https_down->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https_down->timeout(60);
    https_down->set_body(body);
    https_down->send();

    if (https_down->get_status() != 200)
        throw std::runtime_error("证书下载失败: HTTP " + std::to_string(https_down->get_status()) + " body: " + https_down->get_body());

    // 保存证书
    write_file(output_dir + "/cert.pem", https_down->get_body());
    oss << "  [ACME] 证书已保存: " << output_dir << "/cert.pem\n";

    // 提取 CA chain (cert.pem 中第二个证书开始)
    auto chain = extract_chain(https_down->get_body());
    write_file(output_dir + "/chain.pem", chain);
    write_file(output_dir + "/fullchain.pem", https_down->get_body());
    oss << "  [ACME] CA 链已保存: " << output_dir << "/chain.pem\n";
    oss << "  [ACME] 全链已保存: " << output_dir << "/fullchain.pem\n";
    message_debug.append(oss.str());
}

// ========================================================
//  保存账号密钥 (可选)
// ========================================================
void acme_client::save_account_key(const std::string &path)
{
    std::ostringstream oss;
    std::string pem = ec_crypto::export_pem(account_key_.get());
    write_file(path, pem);
    oss << "  [ACME] 账号密钥已保存: " << path << "\n";
    message_debug.append(oss.str());
}
}// namespace http