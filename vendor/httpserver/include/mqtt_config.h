#ifndef PZ_MQTT_CLIENT_CONFIG_H
#define PZ_MQTT_CLIENT_CONFIG_H
/*
 * MQTT 长连接客户端配置加载（conf/mqtt.conf）
 * 镜像 sockets_config.h — 段名即连接名。
 *
 * ENABLE_MQTT_CLIENT 关闭时整个文件空。
 */
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <cstdint>

#include "parse_ini.h"

#ifdef ENABLE_MQTT_CLIENT

namespace http
{

struct mqtt_conn_config_t
{
    std::string host;// 默认 127.0.0.1
    unsigned short port = 1883;
    std::string clientid;// 客户端唯一标识（必填）
    std::string username;
    std::string password;
    std::string url;// 备用（暂用不上）
    bool isssl             = false;
    unsigned short sslport = 8883;
    bool insecure          = false;
    std::string ca_file;
    std::string sni;
    unsigned int timeout_sec = 10;
    uint16_t keepalive       = 60;
};

struct mqtt_config_t
{
    parse_ini data;
    std::string file;
    bool loaded = false;

    bool load(const std::string &filename);
    bool is_load() const { return loaded; }
    bool has(const std::string &section) const;
    std::optional<mqtt_conn_config_t> get(const std::string &section = "default") const;
    std::string raw_field(const std::string &section, const std::string &key) const;
    std::vector<std::string> sections() const;

    mutable std::map<std::string, mqtt_conn_config_t> cache_;
};

mqtt_config_t &get_mqtt_config();

bool load_mqtt_config(const std::string &server_conf_file);
bool reload_mqtt_config();
std::optional<mqtt_conn_config_t> mqtt_conf(const std::string &section = "default");

}// namespace http

#endif// ENABLE_MQTT_CLIENT
#endif
