#ifndef PZ_SOCKETS_CLIENT_CONFIG_H
#define PZ_SOCKETS_CLIENT_CONFIG_H
/*
 * WebSocket 长连接客户端配置加载（conf/sockets.conf）
 * 完全镜像 pzredis_config.h — 段名即连接名。
 *
 * ENABLE_SOCKETS_CLIENT 关闭时整个文件空。
 */
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "parse_ini.h"// 复用 INI 解析层

#ifdef ENABLE_SOCKETS_CLIENT

namespace http
{

struct sock_conn_config_t
{
    std::string host;// 默认 127.0.0.1
    unsigned short port = 80;
    std::string url;// 默认 "/"
    bool isssl             = false;
    unsigned short sslport = 443;
    bool insecure          = false;
    std::string ca_file;
    std::string sni;
    std::string header;
    unsigned int timeout_sec = 10;
};

struct sockets_config_t
{
    parse_ini data;
    std::string file;
    bool loaded = false;

    bool load(const std::string &filename);
    bool is_load() const { return loaded; }
    bool has(const std::string &section) const;
    std::optional<sock_conn_config_t> get(const std::string &section = "default") const;
    std::string raw_field(const std::string &section, const std::string &key) const;
    std::vector<std::string> sections() const;

    mutable std::map<std::string, sock_conn_config_t> cache_;
};

sockets_config_t &get_sockets_config();

bool load_sockets_config(const std::string &server_conf_file);
bool reload_sockets_config();
std::optional<sock_conn_config_t> sock_conf(const std::string &section = "default");

}// namespace http

#endif// ENABLE_SOCKETS_CLIENT
#endif
