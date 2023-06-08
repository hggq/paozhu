#pragma once
#ifndef FRAME_SERVERCONFIG_H
#define FRAME_SERVERCONFIG_H
#include <string>
#include <map>
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
    struct site_host_info_t{
        std::string wwwpath;
        std::string aliaspath;
        std::string mainhost;
        std::string rewrite_404_action;
        std::string certificate_file;
        std::string privateKey_file;
        std::vector<std::string> action_404_lists;
        unsigned int rewrite404=0;
        bool isrewrite=false;
        bool http2_enable=false;
    };
    class serverconfig
    {

    public:
        std::string getsitepath(const std::string &);
        std::string getsitewwwpath(unsigned int);
        std::tuple<unsigned int,std::string> gethost404(const std::string &host);
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
        bool reloadmysql = true;
        bool reloadserverconfig = true;
        unsigned int siteusehtmlchachetime = 0;
        bool siteusehtmlchache = false;
        bool isallnothttp2=true;
        std::map<std::string,SSL_CTX*> g_ctxMap;
        std::map<unsigned long long,bool> domain_http2;
        std::vector<struct site_host_info_t> sitehostinfos;
        std::map<std::string,unsigned int> host_toint;
    };

}
#endif