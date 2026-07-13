/**
 * acme_config.h
 * ACME 配置加载
 * 实现完整的证书申请流程：
 * 加载配置，找出域名配置，当在server.conf里面配置
 * acme = 1 时候这个域名将自动申请域名或检查过期时间
 * 具体实施有acme.conf 配置文件指定周期申请
 *
 *   目录发现 → 创建账号(EAB) → 新建订单 → 获取授权 →
 *   HTTP-01 挑战 → 响应挑战 → 提交 CSR → 下载证书
 *   更新 acme.conf 配置，主要是保存   eab_kid eab_hmac_key;
 *   cert 目录保存在 项目根目录
 *   证书保存在conf目录或/usr/local/etc/paozhu目录
 * 2026-07-13
 */
#pragma once
#ifndef HTTP_ACME_CONFIG
#define HTTP_ACME_CONFIG
#include "parse_ini.h"

namespace http
{
//在此加载 conf/acme.conf 文件，当申请成功后需要保存 eab_kid eab_hmac_key 到 conf/acme.conf
//里面 [zerossl] 部分 eab_kid eab_hmac_key
//就是有这个方法
class acme_config
{
  public:
    acme_config();
    ~acme_config();

    //加载 acme.conf 配置文件
    bool load(const std::string &filename);

    //保存 eab_kid 和 eab_hmac_key 到 acme.conf 的 [zerossl] 部分
    bool save_eab_credentials(const std::string &eab_kid, const std::string &eab_hmac_key);

    //获取配置值
    std::string get_server_url();
    std::string get_eab_api();
    std::string get_eab_kid();
    std::string get_eab_hmac_key();
    std::string get_email();
    std::string get_acme_path();
    //取得剩余天数 不小于2天，不大于100天
    unsigned char get_days_remain();
    //域名cert申请成功保存目录
    std::string get_domain_cert_save_path();
    //判断 eab 凭证是否存在
    bool has_eab_credentials();

  public:
    parse_ini ini_conf;
    std::string config_filename;
    std::string message_debug;
};
}// namespace http
#endif