#pragma once
#ifndef FRAME_SERVERCONFIG_H
#define FRAME_SERVERCONFIG_H
#include <string>
#include <map>
#include <mutex>
#include <tuple>
#include <vector>
#include <atomic>
#include <memory>
#include <unordered_map>
#include "request.h"
#include "client_session.h"
#include "ocsp_check.h"
// #include "Websockets_api.h"

namespace http
{

class serverconfig;
extern std::string server_config_path;
extern std::map<std::string, std::map<std::string, std::string>> loadserversconfig(std::string filename);

serverconfig &getserversysconfig();
struct site_host_info_t
{
    std::string wwwpath;
    std::string aliaspath;
    std::string mainhost;
    std::string fastcgi_host;
    std::string fastcgi_port;
    std::string php_root_document;
    std::string rewrite_404_action;
    std::string certificate_file;
    std::string privateKey_file;
    std::string static_pre_method;
    std::string document_index;
    std::string alias_domain;
    std::string themes;
    std::string themes_url;
    std::vector<std::string> action_404_lists;
    std::vector<std::string> action_pre_lists;
    std::vector<std::string> action_after_lists;
    std::vector<std::string> static_pre_lists;
    std::vector<std::pair<std::string, std::string>> rewrite_php_lists;
    unsigned long long siteid    = 0;
    unsigned long long groupid   = 0;
    unsigned int rewrite404      = 0;
    unsigned int upload_max_size = 0;
    //unsigned int http_header_max_size = 0;
    bool isuse_php         = false;
    bool isrewrite         = false;
    bool http2_enable      = false;
    bool is_method_pre     = false;
    bool is_method_after   = false;
    bool is_static_pre     = false;
    bool is_show_directory = false;
    bool is_limit_upload   = false;
    bool is_close          = false;
    bool is_proxy          = false;
    bool is_acme           = false;
};
class serverconfig
{

  public:
    std::string getsitepath(const std::string &);
    std::string getsitewwwpath(unsigned int);
    std::tuple<unsigned int, std::string> gethost404(const std::string &host);
    bool loadserverglobalconfig();
    bool checkmaindomain(const char *);
    void init_path();
    unsigned char get_co_thread_num();
    unsigned int get_ssl_port();
    unsigned int get_port();
    std::string ssl_chain_file();
    std::string ssl_key_file();
    std::string ssl_dh_file();
    std::string ssl_chain_crt_file();
    SSL_CTX *getctx(std::string filename);
    SSL_CTX *getdefaultctx();
    unsigned int get_hostindex(const std::string &host);
    void clearctx();

    // OCSP Stapling 支持
    // 设置指定域名的 OCSP 装订响应（DER 格式）
    void set_ocsp_staple(const std::string &domain, std::vector<uint8_t> ocsp_der);
    // 获取指定域名的 OCSP 装订响应
    std::vector<uint8_t> get_ocsp_staple(const std::string &domain);
    // 从缓存设置 OCSP staple 到 SSL 对象（用于 SNI 回调，提前设置）
    void set_ocsp_staple_to_ssl(SSL *ssl, const std::string &domain);
    // 启用 SSL_CTX 的 OCSP Stapling 回调
    void enable_ocsp_stapling(SSL_CTX *ctx);

  private:
    // OCSP stapling 回调：OpenSSL 在 TLS 握手时调用，返回预取的 OCSP 响应
    static int ocsp_stapling_cb(SSL *ssl, void *arg);

  public:
    std::string serverpath;
    std::string wwwpath;
    std::string mainhost;
    std::string secondhost;
    // std::map<std::string,std::function<std::shared_ptr<websockets_api>(std::weak_ptr<client_session>)>> websocketmethodcallback;
    std::map<std::string, std::function<std::string(http::client_session &)>> methodcallback;
    std::string configfile;
    std::string configpath;
    std::map<std::string, std::map<std::string, std::string>> map_value;
    bool clear_ctx                     = false;
    bool reloadmysql                   = true;
    bool reloadserverconfig            = true;
    bool siteusehtmlchache             = false;
    bool isallnothttp2                 = true;
    bool is_limit_upload               = false;
    bool ip6_enable                    = false;
    unsigned int siteusehtmlchachetime = 0;
    unsigned int upload_max_size       = 0;
    unsigned int http_header_max_size  = 0;

    unsigned int rate_limit_new_wait_num    = 300;
    unsigned int rate_limit_accept_wait_num = 600;
    unsigned int rate_limit_accept_time     = 500;

    unsigned int acme_every_day_time  = 7;
    unsigned int acme_every_num   = 5;

    unsigned int ocsp_interval_time  = 14400;

    std::map<std::string, SSL_CTX *> g_ctxMap;
    std::map<unsigned long long, bool> domain_http2;
    std::vector<struct site_host_info_t> sitehostinfos;
    std::map<std::string, unsigned int> host_toint;

    // OCSP Stapling: 域名 -> DER 编码的 OCSP 响应
    // 使用 shared_ptr<const unordered_map> + atomic_load/store 实现无锁读写分离
    // 读路径（SNI 回调）完全无锁，写路径（刷新线程）copy-on-write
    std::shared_ptr<const std::unordered_map<std::string, std::vector<uint8_t>>> ocsp_cache_;
};

}// namespace http
#endif
