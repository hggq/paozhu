#pragma once
#ifndef FRAME_SERVERCONFIG_H
#define FRAME_SERVERCONFIG_H
#include <string>
#include <map>
#include <tuple>
#include <vector>
#include "request.h"
#include "client_session.h"
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
    std::vector<std::string> action_404_lists;
    std::vector<std::string> action_pre_lists;
    std::vector<std::string> action_after_lists;
    std::vector<std::string> static_pre_lists;
    std::vector<std::pair<std::string, std::string>> rewrite_php_lists;
    unsigned int rewrite404 = 0;
    bool isuse_php          = false;
    bool isrewrite          = false;
    bool http2_enable       = false;
    bool is_method_pre      = false;
    bool is_method_after    = false;
    bool is_static_pre      = false;
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
    SSL_CTX *getctx(std::string filename);
    SSL_CTX *getdefaultctx();

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
    bool reloadmysql                   = true;
    bool reloadserverconfig            = true;
    unsigned int siteusehtmlchachetime = 0;
    bool siteusehtmlchache             = false;
    bool isallnothttp2                 = true;
    std::map<std::string, SSL_CTX *> g_ctxMap;
    std::map<unsigned long long, bool> domain_http2;
    std::vector<struct site_host_info_t> sitehostinfos;
    std::map<std::string, unsigned int> host_toint;
};

}// namespace http
#endif
