#pragma once
#ifndef FRAME_SERVERCONFIG_H
#define FRAME_SERVERCONFIG_H
#include <string>
#include <string_view>
#include <map>
#include <mutex>
#include <tuple>
#include <vector>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <unordered_set>
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
    // cors_domain：配置里的原始串（逗号分隔），仅保留用于日志/兼容查看
    std::string cors_domain;
    // cors_expose_headers：跨域响应里允许页面 JS 读取的响应头（Access-Control-Expose-Headers）。
    // 未配置就不发这个头：发 "*" 等于把服务端所有响应头都开放给页面脚本
    std::string cors_expose_headers;
    // cors_credentials：跨域响应是否带 Access-Control-Allow-Credentials（默认 false，不发该头）。
    // 只在命中白名单、ACAO 回显具体 Origin 时才发：ACAO "*" 与 credentials 按规范互斥，
    // 浏览器对 "*"+credentials 一律硬拒，那种组合发了等于没发
    bool cors_credentials = false;
    // cors_allow_methods：预检放行的方法列表，决定 Access-Control-Allow-Methods 响应头，
    // 同时也是校验请求头 Access-Control-Request-Method 的名单（两处同一份，否则预检能过、
    // 响应头却不认）。缺省四项里 QUERY 是 HTTP METHOD-QUERY 的新方法，框架按
    // HEAD_METHOD::QUERY 与 GET/POST 并列解析，所以名单里必须有它。配置写了就整表覆盖。
    std::vector<std::string> cors_allow_methods{"POST", "GET", "OPTIONS", "QUERY"};
    // 以下三项由加载期解析 cors_domain 得到，请求期只做 O(1) 查表，见 cors_allow_origin()
    std::vector<std::string> cors_domain_list;          // 切分后的配置条目
    std::unordered_set<std::string> cors_origin_allowed;// 归一化（全小写）后可直接比较的 origin
    bool cors_allow_all = false;                        // 放开全部：只有 cors_domain 显式写了 "*" 才置真
    bool is_cors        = false;                        // 用户是否显式配置了 cors_domain（含 "*"）；false=默认拒绝，加载期告警
    std::string themes;
    std::string themes_url;
    std::vector<std::string> action_404_lists;
    std::vector<std::string> action_pre_lists;
    std::vector<std::string> action_after_lists;
    std::vector<std::string> static_pre_lists;
    std::vector<std::pair<std::string, std::string>> rewrite_php_lists;
    unsigned long long siteid     = 0;
    unsigned long long groupid    = 0;
    unsigned int rewrite404       = 0;
    unsigned int upload_max_size  = 0;
    unsigned int slot_id          = 0;// 反向保存 注册函数槽位
    unsigned int usehtmlcachetime = 0;
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
    bool is_usehtmlcache   = false;

    /// @brief 依据本站点 CORS 配置，算出应写入 Access-Control-Allow-Origin 的值。
    /// @param request_origin 请求头 Origin 的原值，无该头时传空串
    /// @return 未配置或配置为 '*' → "*"；命中白名单 → 请求 Origin 的原值
    ///         （规范要求逐字节相等，不能用配置值回填）；白名单未命中 → 空串，调用方不要写该响应头。
    /// @note 配置串的切分、补 scheme、大小写归一化都在加载期完成，这里只是一次哈希查表。
    std::string cors_allow_origin(std::string_view request_origin) const;
};
class serverconfig
{

  public:
    std::string getsitepath(const std::string &);
    std::string getsitewwwpath(unsigned int);
    std::tuple<unsigned int, std::string> gethost404(const std::string &host);
    bool loadserverglobalconfig();
#ifdef ENABLE_WEBPAY
    // 加载 conf/webpay.conf 到 get_webpay_config() 单例，filename 为空时用 configpath + "webpay.conf"
    // 文件缺失返回 false，不影响服务启动（仅 ENABLE_WEBPAY 时存在）
    bool load_webpay_file(const std::string &filename = "");
#endif
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
    std::string cors_domain;
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

    unsigned int rate_limit_second_num1     = 20 ;
    unsigned int rate_limit_second_num2     = 5 ;

    unsigned int acme_every_day_time = 7;
    unsigned int acme_every_num      = 5;

    unsigned int ocsp_interval_time = 14400;

    std::map<std::string, SSL_CTX *> g_ctxMap;
    std::map<unsigned long long, bool> domain_http2;
    std::vector<struct site_host_info_t> sitehostinfos;
    std::map<std::string, unsigned int> host_toint;

    // OCSP Stapling: domain -> DER-encoded OCSP response
    // 快照是不可变 map，写侧 copy-on-write；ocsp_cache_mutex_ 既保护指针交换，
    // 也把 set_ocsp_staple 的"读—改—写"整段串行化（写侧是 detach 线程，可能重叠）。
    // 读侧在锁内只拷一个 shared_ptr 就出来，之后查的是自己那份快照。
    // 不用 std::atomic<std::shared_ptr>：Apple clang 21 / libc++ 210106 尚未实现该特化，
    // 而 std::atomic_load 自由函数在 libstdc++ 15 上是 -Wdeprecated-declarations。
    std::shared_ptr<const std::unordered_map<std::string, std::vector<uint8_t>>> ocsp_cache_;
    std::mutex ocsp_cache_mutex_;
};

}// namespace http
#endif
