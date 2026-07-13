/**
 * acme_config.cpp
 * ACME 配置加载与保存
 * 加载 conf/acme.conf 文件，提取配置信息
 * 当申请成功后保存 eab_kid eab_hmac_key 到 conf/acme.conf [zerossl] 部分
 * 2026-07-13
 */
#include "acme_config.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace http
{
acme_config::acme_config() {}
acme_config::~acme_config() {}

// ============================================================
//  加载 acme.conf 配置文件
// ============================================================
bool acme_config::load(const std::string &filename)
{
    config_filename = filename;
    if (!std::filesystem::exists(filename))
    {
        return false;
    }
    ini_conf.parse_file(filename);

    return true;
}

// ============================================================
//  保存 eab_kid 和 eab_hmac_key 到 acme.conf 的 [zerossl] 部分
//  使用 parse_ini 的 add_value 方法，保留原文注释和格式
// ============================================================
bool acme_config::save_eab_credentials(const std::string &eab_kid, const std::string &eab_hmac_key)
{
    if (config_filename.empty())
    {
        std::cerr << "[ACME] 配置文件路径为空，无法保存 EAB 凭证\n";
        return false;
    }

    bool ok1 = ini_conf.add_value("zerossl", "eab_kid", eab_kid);
    bool ok2 = ini_conf.add_value("zerossl", "eab_hmac_key", eab_hmac_key);

    if (ok1 && ok2)
    {
        return true;
    }

    return false;
}

// ============================================================
//  获取配置值
// ============================================================
std::string acme_config::get_server_url()
{
    return ini_conf.get_value("zerossl", "server_url", "https://acme.zerossl.com/v2/DV90");
}

std::string acme_config::get_eab_api()
{
    return ini_conf.get_value("zerossl", "eab_api", "https://api.zerossl.com/acme/eab-credentials-email");
}

std::string acme_config::get_eab_kid()
{
    return ini_conf.get_value("zerossl", "eab_kid", "");
}

std::string acme_config::get_eab_hmac_key()
{
    return ini_conf.get_value("zerossl", "eab_hmac_key", "");
}

std::string acme_config::get_email()
{
    // 优先取 [info] 段的 email，没有则取 [zerossl] 段的
    std::string mail = ini_conf.get_value("info", "email", "");
    if (mail.empty())
    {
        mail = ini_conf.get_value("zerossl", "email", "");
    }
    return mail;
}

unsigned char acme_config::get_days_remain()
{
    std::string temp_day = ini_conf.get_value("info", "days_remain", "10");
    unsigned char days   = 0;

    if (temp_day.size() > 1)
    {
        if (temp_day[0] >= '0' && temp_day[0] <= '9')
        {
            days = temp_day[0] - '0';
        }
        if (temp_day[1] >= '0' && temp_day[1] <= '9')
        {
            days = days * 10 + temp_day[1] - '0';
        }
    }

    if (days > 89)
    {
        days = 89;
    }

    if (days < 2)
    {
        days = 2;
    }

    return days;
}

std::string acme_config::get_domain_cert_save_path()
{
    return ini_conf.get_value("info", "cert_path", "");
}

std::string acme_config::get_acme_path()
{
    return ini_conf.get_value("info", "acme_path", "");
}

// ============================================================
//  判断 eab 凭证是否存在
// ============================================================
bool acme_config::has_eab_credentials()
{
    return !get_eab_kid().empty() && !get_eab_hmac_key().empty();
}

}// namespace http
