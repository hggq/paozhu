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
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>

namespace http
{
acme_client::acme_client(const std::string &acme_server_url,
                         const std::string &eab_kid,
                         const std::string &eab_hmac_key,
                         const std::string &email,
                         std::ostringstream &oss_)
    : server_url_(acme_server_url), eab_kid_(eab_kid), eab_hmac_key_(eab_hmac_key), email_(email), oss(oss_)
{
    // 生成账号 EC P-256 密钥对
    account_key_ = ec_crypto::generate_p256();
}

// ========================================================
//  Step 1: 获取 ACME 目录
// ========================================================
acme_directory_t acme_client::fetch_directory()
{
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
    return kid_;
}

// ========================================================
//  🔑 Step 4: 新建证书订单 (修复：保存所有 Authz URL)
// ========================================================
std::string acme_client::new_order(const std::vector<std::string> &domains)
{
    oss << "\n  [ACME] 新建订单...\n";
    std::string nonce = fetch_nonce();

    std::string ids_arr;
    for (size_t i = 0; i < domains.size(); ++i)
    {
        if (i > 0)
            ids_arr += ",";
        ids_arr += R"({"type":"dns","value":")" + domains[i] + R"("})";
    }
    auto ids = R"({"identifiers":[)" + ids_arr + "]}";

    std::string body = jws::make_signed_body(ids, dir_.new_order, nonce, account_key_.get(), kid_);

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

    if (https->page.isjson != 1)
        throw std::runtime_error("订单创建失败: 非JSON响应 " + https->get_body());

    finalize_url_ = https->page.json["finalize"].to_string();

    // 🔑 修复：提取 ALL authorization URLs
    authz_urls_.clear();

    if (https->page.json["authorizations"].is_array())
    {
        auto authz_arr = https->page.json["authorizations"];// 拷贝一份非const副本
        for (size_t i = 0; i < authz_arr.size(); ++i)
        {
            authz_urls_.push_back(authz_arr[i].to_string());
        }
    }

    oss << "  [ACME] 订单 URL:    " << order_url_ << "\n";
    oss << "  [ACME] Finalize URL: " << finalize_url_ << "\n";
    oss << "  [ACME] 授权数量:   " << authz_urls_.size() << "\n";
    for (size_t i = 0; i < authz_urls_.size(); ++i)
    {
        oss << "  [ACME] Authz[" << i << "]: " << authz_urls_[i] << "\n";
    }

    return order_url_;
}

// ========================================================
//  🔑 新增：一键完成所有域名的 Challenge + 轮询
//  封装原 Step 5 ~ Step 8，支持多域名共用 webroot
// ========================================================
void acme_client::verify_all_domains(const std::string &webroot, int max_retries, int interval_sec)
{
    if (authz_urls_.empty())
        throw std::runtime_error("verify_all_domains: authz_urls_ 为空，请先调用 new_order()");

    oss << "\n  [ACME] === 开始验证 " << authz_urls_.size() << " 个域名 ===\n";

    // --- Phase A: 逐个获取 Challenge → 写入文件 → 触发验证 ---
    for (size_t i = 0; i < authz_urls_.size(); ++i)
    {
        const auto &authz_url = authz_urls_[i];
        oss << "\n  [ACME] --- 域名 " << (i + 1) << "/" << authz_urls_.size() << " ---\n";

        // 1. POST-as-GET 获取该 Authz 详情
        std::string nonce = fetch_nonce();
        std::string body  = jws::make_signed_body("", authz_url, nonce, account_key_.get(), kid_);

        auto https = std::make_shared<http::client>();
        https->post(authz_url);
        https->add_header("Content-Type", "application/jose+json");
        https->add_header("Connection", "close");
        https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
        https->timeout(60);
        https->set_body(body);
        https->send();

        if (https->get_status() != 200 || !https->page.isjson)
            throw std::runtime_error("获取授权失败: " + https->get_body());

        // 2. 遍历查找 http-01 challenge（修复原来硬编码 [0]/[1] 的问题）
        std::string token, chall_url;

        if (https->page.json["challenges"].is_array())
        {
            auto challenges = https->page.json["challenges"];// 拷贝一份非const副本
            for (size_t ci = 0; ci < challenges.size(); ++ci)
            {
                if (challenges[ci]["type"].to_string() == "http-01")
                {
                    token     = challenges[ci]["token"].to_string();
                    chall_url = challenges[ci]["url"].to_string();
                    break;
                }
            }
        }

        if (token.empty())
            throw std::runtime_error("未找到 http-01 挑战: " + authz_url);

        oss << "  [ACME] HTTP-01 token: " << token << "\n";
        oss << "  [ACME] Challenge URL: " << chall_url << "\n";

        // 3. 写入验证文件（共用 webroot）
        prepare_http01(webroot, token);

        // 4. 通知 CA 开始验证
        respond_challenge(chall_url);
    }

    oss << "\n  [ACME] 所有域名挑战已提交，等待验证";

    // --- Phase B: 统一轮询所有 Authz 直到全部 valid ---
    bool all_valid = false;
    for (int attempt = 0; attempt < max_retries && !all_valid; ++attempt)
    {
        oss << ".";
        all_valid = true;

        for (const auto &authz_url : authz_urls_)
        {
            std::string nonce = fetch_nonce();
            std::string body  = jws::make_signed_body("", authz_url, nonce, account_key_.get(), kid_);

            auto ar = std::make_shared<http::client>();
            ar->post(authz_url);
            ar->add_header("Content-Type", "application/jose+json");
            ar->add_header("Connection", "close");
            ar->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
            ar->timeout(30);
            ar->set_body(body);
            ar->send();

            if (ar->get_status() != 200 || !ar->page.isjson)
            {
                all_valid = false;
                continue;
            }

            std::string status = ar->page.json["status"].to_string();

            if (status == "invalid")
            {
                std::string detail = "未知错误";
                if (ar->page.json["challenges"].is_array() && ar->page.json["challenges"].size() > 0)
                {
                    auto &err = ar->page.json["challenges"][0]["error"];
                    if (!err.is_null())
                        detail = err["detail"].to_string();
                }
                throw std::runtime_error("域名验证失败: " + detail + " | " + authz_url);
            }

            if (status != "valid")
                all_valid = false;
        }

        if (!all_valid)
            std::this_thread::sleep_for(std::chrono::seconds(interval_sec));
    }

    if (!all_valid)
        throw std::runtime_error("等待域名验证超时(" + std::to_string(max_retries * interval_sec) + "s)");

    oss << " 全部验证通过!\n";
}

// ========================================================
//  Step 6: 准备 HTTP-01 验证文件
// ========================================================
void acme_client::prepare_http01(const std::string &webroot, const std::string &token)
{
    oss << "\n  [ACME] 准备 HTTP-01 验证文件...\n";

    // key authorization = token + "." + JWK Thumbprint
    std::string thumbprint = jws::jwk_thumbprint(account_key_.get());
    std::string key_auth   = token + "." + thumbprint;

    oss << "  [ACME] JWK Thumbprint: " << thumbprint << "\n";
    oss << "  [ACME] Key Authorization: " << key_auth << "\n";

    // 写入 webroot/.well-known/acme-challenge/<token>
    std::string challenge_dir = webroot;
    if (challenge_dir.size() > 0 && challenge_dir.back() != '/')
    {
        challenge_dir.push_back('/');
    }
    challenge_dir.append(".well-known/acme-challenge");
    fs::create_directories(challenge_dir);

    std::string file_path = challenge_dir + "/" + token;
    std::ofstream ofs(file_path);
    if (!ofs)
        throw std::runtime_error("无法创建验证文件: " + file_path);
    ofs << key_auth;
    ofs.close();

    oss << "  [ACME] 验证文件已写入: " << file_path << "\n";
}

// ========================================================
//  Step 7: 响应挑战 (通知 ACME 服务器验证)
// ========================================================
void acme_client::respond_challenge(const std::string &challenge_url)
{
    oss << "\n  [ACME] 响应挑战...\n";

    std::string nonce = fetch_nonce();
    std::string body  = jws::make_signed_body(
        "{}",
        challenge_url,
        nonce,
        account_key_.get(),
        kid_);

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
}

// ========================================================
//  Step 9: 生成私钥 + CSR，轮询订单就绪并提交 finalize
// ========================================================
EVP_PKEY_ptr acme_client::finalize_order(const std::vector<std::string> &domains,
                                         const std::string &output_dir)
{
    oss << "\n  [ACME] 生成证书私钥 + CSR...\n";

    // 1. 生成证书私钥 (EC P-256)
    EVP_PKEY_ptr cert_pkey  = ec_crypto::generate_p256();
    std::string privkey_pem = ec_crypto::export_pem(cert_pkey.get());
    write_file(output_dir + "/privkey.pem", privkey_pem);
    oss << "  [ACME] 私钥已保存: " << output_dir << "/privkey.pem\n";

    // 2. 生成 CSR -> DER -> base64url
    std::string csr_pem = csr::generate_csr_pem(cert_pkey.get(), domains);
    std::string csr_b64 = csr::pem_to_der_b64url(csr_pem);
    oss << "  [ACME] CSR 已生成 (" << domains.size() << " 个域名)\n";

    // ========================================================
    //  🔑 3. 获取 Order 详情并提取所有 Authorization URL
    // ========================================================
    oss << "  [ACME] 获取订单授权信息...\n";
    std::string nonce      = fetch_nonce();
    std::string order_body = jws::make_signed_body("", order_url_, nonce, account_key_.get(), kid_);

    auto order_req = std::make_shared<http::client>();
    order_req->post(order_url_);
    order_req->add_header("Content-Type", "application/jose+json");
    order_req->add_header("Connection", "close");
    order_req->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    order_req->timeout(30);
    order_req->set_body(order_body);
    order_req->send();

    if (order_req->get_status() != 200 || !order_req->page.isjson)
    {
        throw std::runtime_error("获取订单详情失败: HTTP " +
                                 std::to_string(order_req->get_status()) +
                                 " body: " + order_req->get_body());
    }

    std::vector<std::string> authz_urls;
    // for (const auto& authz : order_req->page.json["authorizations"]) {
    //     authz_urls.push_back(authz.to_string());
    // }
    auto authz_list = order_req->page.json["authorizations"];
    for (size_t ai = 0; ai < authz_list.size(); ++ai)
    {
        authz_urls.push_back(authz_list[ai].to_string());
    }

    oss << "  [ACME] 共 " << authz_urls.size() << " 个授权需要验证\n";

    // ========================================================
    //  🔑 4. 轮询所有 Authz，直到全部 valid 或出现 invalid
    // ========================================================
    oss << "  [ACME] 等待域名验证通过";
    bool all_valid = false;
    for (int attempt = 0; attempt < 60 && !all_valid; ++attempt)
    {
        oss << ".";
        all_valid = true;

        for (const auto &authz_url : authz_urls)
        {
            std::string an = fetch_nonce();
            std::string ab = jws::make_signed_body("", authz_url, an, account_key_.get(), kid_);

            auto ar = std::make_shared<http::client>();
            ar->post(authz_url);
            ar->add_header("Content-Type", "application/jose+json");
            ar->add_header("Connection", "close");
            ar->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
            ar->timeout(30);
            ar->set_body(ab);
            ar->send();

            if (ar->get_status() != 200 || !ar->page.isjson)
            {
                oss << "\n  [WARN] 查询 Authz 失败: " << ar->get_body() << "\n";
                all_valid = false;
                continue;
            }

            std::string status = ar->page.json["status"].to_string();

            if (status == "invalid")
            {
                // 提取具体失败原因
                std::string detail = "未知错误";
                if (ar->page.json["challenges"].is_array() &&
                    ar->page.json["challenges"].size() > 0)
                {
                    auto &err = ar->page.json["challenges"][0]["error"];
                    if (!err.is_null())
                    {
                        detail = err["detail"].to_string();
                    }
                }
                throw std::runtime_error("域名验证失败: " + detail +
                                         " | Authz: " + authz_url);
            }

            if (status != "valid")
            {
                all_valid = false;
            }
        }

        if (!all_valid)
        {
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }

    if (!all_valid)
    {
        throw std::runtime_error("等待域名验证超时(300s)，请检查 DNS/HTTP 配置");
    }
    oss << " 所有域名验证已通过!\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // ========================================================
    //  5. 确认 Order 状态确实变为 ready
    // ========================================================
    oss << "  [ACME] 确认订单就绪...\n";
    nonce      = fetch_nonce();
    order_body = jws::make_signed_body("", order_url_, nonce, account_key_.get(), kid_);

    auto ready_check = std::make_shared<http::client>();
    ready_check->post(order_url_);
    ready_check->add_header("Content-Type", "application/jose+json");
    ready_check->add_header("Connection", "close");
    ready_check->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    ready_check->timeout(30);
    ready_check->set_body(order_body);
    ready_check->send();

    if (ready_check->get_status() != 200 || !ready_check->page.isjson)
    {
        throw std::runtime_error("确认订单就绪失败: " + ready_check->get_body());
    }

    std::string order_status = ready_check->page.json["status"].to_string();
    if (order_status != "ready")
    {
        throw std::runtime_error("所有 Authz 已 valid 但订单状态仍为 '" +
                                 order_status + "', body: " + ready_check->get_body());
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    // ========================================================
    //  6. 提交 CSR (finalize)
    // ========================================================
    oss << "  [ACME] 提交 CSR (finalize)...\n";
    nonce            = fetch_nonce();
    auto payload     = R"({"csr":")" + csr_b64 + R"("})";
    std::string body = jws::make_signed_body(payload, finalize_url_, nonce, account_key_.get(), kid_);

    auto https = std::make_shared<http::client>();
    https->post(finalize_url_);
    https->add_header("Content-Type", "application/jose+json");
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->set_body(body);
    https->send();

    if (https->get_status() != 200)
    {
        throw std::runtime_error("Finalize 失败: HTTP " + std::to_string(https->get_status()) +
                                 " body: " + https->get_body());
    }

    if (!https->page.isjson)
    {
        throw std::runtime_error("Finalize 响应非JSON: " + https->get_body());
    }

    order_status = https->page.json["status"].to_string();
    oss << "  [ACME] Finalize 响应状态: " << order_status << "\n";

    // ========================================================
    //  7. 如果服务端还在处理中，轮询等待变为 valid
    // ========================================================
    if (order_status == "processing")
    {
        oss << "  [ACME] 证书签发中，等待服务端处理...";
        for (int i = 0; i < 30; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            oss << "." << i;

            std::string pn = fetch_nonce();
            std::string pb = jws::make_signed_body("", order_url_, pn, account_key_.get(), kid_);
            auto hp        = std::make_shared<http::client>();
            hp->post(order_url_);
            hp->add_header("Content-Type", "application/jose+json");
            hp->add_header("Connection", "close");
            hp->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
            hp->timeout(30);
            hp->set_body(pb);
            hp->send();

            if (hp->get_status() == 200 && hp->page.isjson)
            {
                std::string st = hp->page.json["status"].to_string();
                if (st == "valid")
                {
                    oss << " 证书已签发!\n";
                    return cert_pkey;
                }
                if (st == "invalid")
                {
                    throw std::runtime_error("Finalize 后订单变为 invalid: " + hp->get_body());
                }
            }
        }
        throw std::runtime_error("Finalize 后轮询超时(300s)，订单仍处于 processing 状态");
    }

    if (order_status != "valid")
    {
        throw std::runtime_error("Finalize 后订单状态异常: '" + order_status +
                                 "', body: " + https->get_body());
    }

    oss << "  [ACME] Finalize 成功!\n";
    return cert_pkey;
}

void acme_client::write_file(const std::string &path, const std::string &content)
{
    auto parent = fs::path(path).parent_path();
    if (!parent.empty() && !fs::exists(parent))
    {
        fs::create_directories(parent);
        fs::permissions(parent,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all,
                        fs::perm_options::replace);
    }

    std::ofstream ofs(path);
    if (!ofs)
        throw std::runtime_error("无法写入文件: " + path);
    ofs << content;

    fs::permissions(path,
                    fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                    fs::perm_options::replace);
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
            std::string status = https->page.json["status"].to_string();

            if (status == "valid")
            {

                cert_url = https->page.json["certificate"].to_string();
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

    oss << " cert_url: " << cert_url << std::endl;
    oss << " body: " << body << std::endl;

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
}

// ========================================================
//  保存账号密钥 (可选)
// ========================================================
void acme_client::save_account_key(const std::string &path)
{
    std::string pem = ec_crypto::export_pem(account_key_.get());
    write_file(path, pem);
    oss << "  [ACME] 账号密钥已保存: " << path << "\n";
}

// ========================================================
//  从文件加载已有账号密钥
// ========================================================
bool acme_client::load_account_key(const std::string &path)
{
    if (!fs::exists(path))
        return false;

    std::ifstream fin(path);
    if (!fin.is_open())
        return false;

    std::string pem((std::istreambuf_iterator<char>(fin)),
                    std::istreambuf_iterator<char>());
    fin.close();

    if (pem.empty())
        return false;

    account_key_ = ec_crypto::import_pem(pem);
    oss << "  [ACME] 已加载账号密钥: " << path << "\n";
    return true;
}
}// namespace http