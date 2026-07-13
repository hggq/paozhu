/**
 * acme_client.h
 * ACME 协议客户端
 * 实现完整的证书申请流程：
 *   目录发现 → 创建账号(EAB) → 新建订单 → 获取授权 →
 *   HTTP-01 挑战 → 响应挑战 → 提交 CSR → 下载证书
 * 2026-07-12
 */
#pragma once
#ifndef HTTP_ACME_DOMAIN_CLIENT
#define HTTP_ACME_DOMAIN_CLIENT

#include "crypto_ptr.h"
#include "acme_crypto.hpp"
#include "httpclient.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>

namespace http
{
namespace fs = std::filesystem;
struct acme_directory_t
{
    std::string new_nonce;
    std::string new_account;
    std::string new_order;
};
struct http01_challenge
{
    std::string token;
    std::string url;// challenge URL (用于响应)
};

class acme_client
{
  public:
    acme_client(const std::string &acme_server_url,
                const std::string &eab_kid,
                const std::string &eab_hmac_key,
                const std::string &email);

    // ========================================================
    //  Step 1: 获取 ACME 目录
    // ========================================================
    acme_directory_t fetch_directory();

    // ========================================================
    //  Step 2: 获取 Nonce
    // ========================================================
    std::string fetch_nonce();

    // ========================================================
    //  Step 3: 创建账号 (带 EAB)
    // ========================================================
    std::string create_account();

    // ========================================================
    //  Step 4: 新建证书订单
    // ========================================================
    std::string new_order(const std::vector<std::string> &domains);

    // ========================================================
    //  Step 5: 获取授权 & 提取 HTTP-01 挑战
    // ========================================================
    http01_challenge get_http01_challenge();

    // ========================================================
    //  Step 6: 准备 HTTP-01 验证文件
    // ========================================================
    void prepare_http01(const std::string &webroot, const std::string &token);

    // ========================================================
    //  Step 7: 响应挑战 (通知 ACME 服务器验证)
    // ========================================================
    void respond_challenge(const std::string &challenge_url);

    // ========================================================
    //  Step 8: 轮询授权状态
    // ========================================================
    void poll_authorization(int max_retries = 30, int interval_sec = 5);

    // ========================================================
    //  Step 9: 生成私钥 + CSR 提交 finalize
    // ========================================================
    EVP_PKEY_ptr finalize_order(const std::vector<std::string> &domains, const std::string &output_dir);

    // ========================================================
    //  Step 10: 轮询订单状态 + 下载证书
    // ========================================================
    void download_certificate(const std::string &output_dir, int max_retries = 30, int interval_sec = 5);

    // ========================================================
    //  保存账号密钥 (可选)
    // ========================================================
    void save_account_key(const std::string &path);

    void write_file(const std::string &path, const std::string &content);
    // 从 PEM 链中提取 CA 证书 (去掉第一个叶子证书)
    std::string extract_chain(const std::string &pem);

  public:
    std::string message_debug;

  private:
    std::string server_url_;
    std::string eab_kid_;
    std::string eab_hmac_key_;
    std::string email_;
    EVP_PKEY_ptr account_key_;
    acme_directory_t dir_;
    std::string kid_;
    std::string order_url_;
    std::string authz_url_;
    std::string finalize_url_;
};
}// namespace http
#endif