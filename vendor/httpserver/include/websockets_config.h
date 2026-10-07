#ifndef PZ_WEBSOCKETS_CONFIG_H
#define PZ_WEBSOCKETS_CONFIG_H
/*
 * WebSocket 长连接客户端配置加载（conf/websockets.conf）
 * 完全镜像 pzredis_config.h — 段名即连接名。
 *
 * ENABLE_WEBSOCKETS_CLIENT 关闭时整个文件空。
 */
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "parse_ini.h"

#ifdef ENABLE_WEBSOCKETS_CLIENT

namespace http
{

struct ws_conn_config_t
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

struct websockets_config_t
{
    parse_ini data;
    std::string file;
    bool loaded = false;

    bool load(const std::string &filename);
    bool is_load() const { return loaded; }
    bool has(const std::string &section) const;
    std::optional<ws_conn_config_t> get(const std::string &section = "default") const;
    std::string raw_field(const std::string &section, const std::string &key) const;
    std::vector<std::string> sections() const;

    mutable std::map<std::string, ws_conn_config_t> cache_;
};

websockets_config_t &get_websockets_config();

bool load_websockets_config(const std::string &server_conf_file);
bool reload_websockets_config();
std::optional<ws_conn_config_t> ws_conf(const std::string &section = "default");

}// namespace http

#endif// ENABLE_WEBSOCKETS_CLIENT
#endif
