/**
 * acme_run.h
 * acme_client 使用例子文件
 *
 * 2026-07-13
 */
#pragma once
#ifndef HTTP_ACME_RUN
#define HTTP_ACME_RUN
#include "acme_config.h"
#include "http_domain.h"
namespace http
{
//在此加载 conf/acme.conf 文件，当申请成功后需要保存 eab_kid eab_hmac_key 到 conf/acme.conf
//里面 [zerossl] 部分 eab_kid eab_hmac_key
//就是有这个方法
class acme_run
{
  public:
    //几个方法
    acme_run();
    void set_webroot(const std::string &);
    void set_email(const std::string &);
    void set_acme_config_path(const std::string &);
    bool load_acme_config(const std::string &filename);
    cert_validity_t check_domain_cert_pem(const std::string &domainname);
    bool run();//执行申请流程 单线程
  public:
    std::string acme_config_path;//acme_config_path + "/acme.conf" 没有创建一个
    std::string cert_path;       //域名申请成功保存的目录
    acme_config acme_ini;
    std::string email;    //conf/acme.conf email
    std::string acme_path;//conf/acme.conf acme_path
    std::vector<std::string> domains;
    std::string webroot;
    std::string output_dir;        //acme_path + "/" + domains[0]
    unsigned char days_remain = 10;//days_remaining 剩余的天数，才更新
    std::string message_debug;     //过程信息
    bool isforce = false;
};
}// namespace http
#endif