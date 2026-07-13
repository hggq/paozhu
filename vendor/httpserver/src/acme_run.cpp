/**
 * acme_run.cpp
 * ACME 证书申请入口
 *
 * 流程:
 *   1. 加载 conf/acme.conf 配置文件
 *   2. 提取配置信息 (email, acme_path, server_url, eab_kid, eab_hmac_key)
 *   3. 检查 output_dir/cert.pem 是否存在且未过期 (剩余 >10 天则跳过)
 *   4. 若需要申请，执行完整 ACME 流程
 *   5. 申请成功后保存 eab_kid eab_hmac_key 回 acme.conf
 *
 * 2026-07-13
 */
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include "httpclient.h"
#include "acme_run.h"
#include "http_domain.h"
#include "acme_crypto.hpp"
#include "acme_client.h"
#include "terminal_color.h"

namespace http
{
// ============================================================
//  构造函数
// ============================================================
acme_run::acme_run()
    : acme_config_path("conf"), webroot("www/default")
{
}

// ============================================================
//  设置 Web 根目录 (用于 HTTP-01 验证)
// ============================================================
void acme_run::set_webroot(const std::string &path)
{
    webroot = path;
}

// ============================================================
//  设置邮箱
// ============================================================
void acme_run::set_email(const std::string &mail)
{
    email = mail;
}

// ============================================================
//  设置 acme.conf 所在目录路径
// ============================================================
void acme_run::set_acme_config_path(const std::string &path)
{
    acme_config_path = path;
    // 确保路径末尾没有多余的 '/'
    if (!acme_config_path.empty() && acme_config_path.back() == '/')
    {
        acme_config_path.pop_back();
    }
}

// ============================================================
//  加载 acme.conf 配置文件，提取配置信息
// ============================================================
bool acme_run::load_acme_config(const std::string &filename)
{
    std::string filepath = filename;
    if (filepath.empty())
    {
        filepath = acme_config_path + "/acme.conf";
    }
    std::ostringstream oss;
    if (!acme_ini.load(filepath))
    {
        oss << "[ACME] 无法加载配置文件: " << filepath << "\n";
        message_debug.append(oss.str());
        return false;
    }

    // 从配置中提取信息
    if (email.empty())
    {
        email = acme_ini.get_email();
    }
    if (acme_path.empty())
    {
        acme_path = acme_ini.get_acme_path();
    }

    days_remain = acme_ini.get_days_remain();
    cert_path   = acme_ini.get_domain_cert_save_path();

    oss << "[ACME] 配置信息:\n";
    oss << "  email:      " << email << "\n";
    oss << "  acme_path:  " << acme_path << "\n";
    oss << "  server_url: " << acme_ini.get_server_url() << "\n";
    oss << "  eab_kid:    " << (acme_ini.has_eab_credentials() ? "(已配置)" : "(未配置)") << "\n";
    message_debug.append(oss.str());
    return true;
}

// ============================================================
//  检查域名证书的有效期
//  先查 domainname/cert.pem，不存在则尝试 www.domainname/cert.pem
//  返回 cert_validity_t 结构体
//  如果证书文件不存在，返回 is_expired=true 的占位值
// ============================================================
cert_validity_t acme_run::check_domain_cert_pem(const std::string &domainname)
{
    if (domainname.empty())
    {
        throw std::runtime_error("域名为空");
    }

    std::string path = acme_path.empty() ? "certs" : acme_path;

    // 先检查 domainname/cert.pem
    std::string cert_save_path = path + "/" + domainname + "/cert.pem";

    if (!std::filesystem::exists(cert_save_path))
    {
        // 不存在则尝试 www.domainname/cert.pem
        if (domainname.size() > 4 && domainname.substr(0, 4) != "www.")
        {
            std::string www_path = path + "/www." + domainname + "/cert.pem";
            if (std::filesystem::exists(www_path))
            {
                cert_save_path = www_path;
            }
            else
            {
                // 都不存在，返回已过期的占位值
                cert_validity_t cv;
                cv.not_before     = 0;
                cv.not_after      = 0;
                cv.days_remaining = 0;
                cv.is_expired     = true;
                return cv;
            }
        }
        else
        {
            // 已是 www. 开头或太短，直接返回占位值
            cert_validity_t cv;
            cv.not_before     = 0;
            cv.not_after      = 0;
            cv.days_remaining = 0;
            cv.is_expired     = true;
            return cv;
        }
    }

    return get_cert_validity(cert_save_path);
}

// ============================================================
//  执行 ACME 证书申请流程 (单线程)
//  返回 true 表示成功
// ============================================================
bool acme_run::run()
{
    std::ostringstream oss;
    try
    {
        // 检查域名列表
        if (domains.empty())
        {
            oss << "[ACME] 错误: 未设置域名\n";
            message_debug.append(oss.str());
            return false;
        }

        std::string primary_domain = domains[0];

        // 输出目录: acme_path/<primary_domain>/
        if (acme_path.empty())
        {
            acme_path = "certs";
        }
        output_dir = acme_path + "/" + primary_domain;

        // 创建输出目录
        std::filesystem::create_directories(output_dir);

        // ====================================================
        //  检查现有证书是否即将过期 (剩余 >10 天则跳过申请)
        // ====================================================
        try
        {
            cert_validity_t cv = check_domain_cert_pem(primary_domain);

            struct tm tm_val{};
#ifdef _WIN32
            gmtime_s(&tm_val, &cv.not_after);
#else
            gmtime_r(&cv.not_after, &tm_val);
#endif
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", &tm_val);
            std::string isodatetime = std::string(buf);

            oss << "[ACME] 现有证书过期时间: " << isodatetime << "\n";
            if (!cv.is_expired)
            {
                oss << "[ACME] 现有证书剩余天数: " << cv.days_remaining << " 保存目录:" << output_dir << "\n";

                if (isforce)
                {
                    oss << "[ACME] 证书重新申请新证书...\n";
                }
                else
                {
                    if (cv.days_remaining > days_remain)
                    {
                        oss << "[ACME] 证书仍在有效期内 (剩余 "
                            << cv.days_remaining << " 天)，无需申请\n";
                        message_debug.append(oss.str());
                        return true;
                    }
                    oss << "[ACME] 证书即将过期，开始重新申请...\n";
                }
            }
            else
            {
                oss << "[ACME] 证书不存在或已过期，开始申请...\n";
            }
        }
        catch (const std::exception &e)
        {
            oss << "[ACME] 检查证书有效期失败: " << e.what() << "\n";
            oss << "[ACME] 将继续申请新证书...\n";
        }

        // ====================================================
        //  打印申请信息
        // ====================================================
        oss << "\n";
        oss << "==========================================\n";
        oss << "  ZeroSSL ACME 证书申请\n";
        oss << "==========================================\n";
        oss << "  邮箱:    " << email << "\n";
        oss << "  域名:    ";
        for (size_t i = 0; i < domains.size(); ++i)
        {
            if (i > 0)
                oss << ", ";
            oss << domains[i];
        }
        oss << "\n";
        oss << "  Webroot: " << webroot << "\n";
        oss << "  输出:    " << output_dir << "\n";
        oss << "  ACME:    " << acme_ini.get_server_url() << "\n";
        oss << "==========================================\n\n";

        // ====================================================
        //  Step 0: 获取或使用已有的 EAB 凭证
        // ====================================================
        std::string eab_kid      = acme_ini.get_eab_kid();
        std::string eab_hmac_key = acme_ini.get_eab_hmac_key();
        bool need_save_eab       = false;

        if (eab_kid.empty() || eab_hmac_key.empty())
        {
            oss << "[ACME] EAB 凭证未配置，通过邮箱申请...\n";
            eab_credentials_t eab = fetch_eab_by_email(acme_ini.get_eab_api(), email);
            eab_kid               = eab.eab_kid;
            eab_hmac_key          = eab.eab_hmac_key;
            need_save_eab         = true;
            oss << "[ACME] EAB 凭证获取成功\n";
            oss << "  eab_kid:      " << eab_kid << "\n";
            oss << "  eab_hmac_key: " << eab_hmac_key << "\n";
        }
        else
        {
            oss << "[ACME] 使用已有 EAB 凭证\n";
        }

        // ====================================================
        //  Step 1: 创建 ACME 客户端
        // ====================================================
        oss << "[ACME] Step 1: 初始化 ACME 客户端...\n";
        acme_client acme(
            acme_ini.get_server_url(),
            eab_kid,
            eab_hmac_key,
            email);

        // ====================================================
        //  Step 2: 获取 ACME 目录
        // ====================================================
        oss << "[ACME] Step 2: 获取 ACME 目录...\n";
        acme.fetch_directory();
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  Step 3: 创建账号 (带 EAB 绑定)
        // ====================================================
        oss << "[ACME] Step 3: 创建 ACME 账号...\n";
        acme.create_account();
        acme.save_account_key(acme_path + "/account_key.pem");
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  Step 4: 新建证书订单
        // ====================================================
        oss << "[ACME] Step 4: 新建证书订单...\n";
        acme.new_order(domains);
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  Step 5: 获取 HTTP-01 挑战
        // ====================================================
        oss << "[ACME] Step 5: 获取 HTTP-01 挑战...\n";
        auto challenge = acme.get_http01_challenge();
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  Step 6: 准备验证文件
        // ====================================================
        oss << "[ACME] Step 6: 写入 HTTP-01 验证文件...\n";
        acme.prepare_http01(webroot, challenge.token);
        oss << acme.message_debug;
        acme.message_debug.clear();

        oss << "\n";
        oss << "[ACME] 请确保 Web 服务器可访问验证文件:\n";
        oss << "  curl http://" << primary_domain
            << "/.well-known/acme-challenge/" << challenge.token << "\n\n";

        // ====================================================
        //  Step 7: 响应挑战
        // ====================================================
        oss << "[ACME] Step 7: 通知 ACME 服务器验证...\n";
        acme.respond_challenge(challenge.url);
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  Step 8: 等待授权验证通过
        // ====================================================
        oss << "[ACME] Step 8: 等待域名验证...\n";
        acme.poll_authorization(60, 5);
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  Step 9: 生成 CSR + finalize
        // ====================================================
        oss << "[ACME] Step 9: 生成 CSR 并提交...\n";
        acme.finalize_order(domains, output_dir);
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  Step 10: 下载证书
        // ====================================================
        oss << "[ACME] Step 10: 下载证书...\n";
        acme.download_certificate(output_dir, 20, 10);
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  保存 EAB 凭证到 acme.conf (如果是新获取的)
        // ====================================================
        if (need_save_eab)
        {
            oss << "\n[ACME] 保存 EAB 凭证到 acme.conf...\n";
            acme_ini.save_eab_credentials(eab_kid, eab_hmac_key);
        }
        oss << acme.message_debug;
        acme.message_debug.clear();

        // ====================================================
        //  完成
        // ====================================================
        oss << "\n";
        oss << "==========================================\n";
        oss << "  证书申请成功!\n";
        oss << "==========================================\n";
        oss << "\n";
        oss << "  输出文件:\n";
        oss << "    " << acme_path << "/account_key.pem  - ACME 账号私钥 (所有域名共用)\n";
        oss << "    " << output_dir << "/privkey.pem      - 证书私钥\n";
        oss << "    " << output_dir << "/cert.pem         - 服务器证书\n";
        oss << "    " << output_dir << "/chain.pem        - CA 中间证书链\n";
        oss << "    " << output_dir << "/fullchain.pem    - 证书 + CA 链\n";
        oss << "\n";
        message_debug.append(oss.str());
        return true;
    }
    catch (std::exception const &e)
    {
        oss << message_debug;
        oss << "[ACME] 程序异常: " << e.what() << "\n";

        // 打印 OpenSSL 错误栈
        unsigned long err;
        while ((err = ERR_get_error()) != 0)
        {
            char buf[256];
            ERR_error_string_n(err, buf, sizeof(buf));
            oss << "[ACME] OpenSSL: " << buf << "\n";
        }
        message_debug.append(oss.str());
        return false;
    }
}

}// namespace http