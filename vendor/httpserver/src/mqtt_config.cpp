/*
 * MQTT 长连接客户端配置加载实现
 * 镜像 sockets_config.cpp
 */
#include "mqtt_config.h"

#ifdef ENABLE_MQTT_CLIENT

#include <filesystem>
#include <iostream>
#include <limits>
#include <mutex>
#include <optional>
#include <stdexcept>

#include "func.h"// str2uint64_strict

namespace http
{
namespace fs = std::filesystem;

namespace
{
bool parse_bool(const std::string &v)
{
    return v == "1" || v == "true" || v == "True" || v == "TRUE" || v == "On" || v == "ON";
}

std::string field_str(const http::ini_section_value_t &sec, const std::string &key)
{
    auto found = sec.try_find(key);
    return found.second ? *found.first : std::string();
}

template <typename T>
T field_num(const http::ini_section_value_t &sec, const std::string &key, T fallback, std::string &bad_key)
{
    std::string v = field_str(sec, key);
    if (v.empty())
        return fallback;
    unsigned long long n = 0;
    if (!http::str2uint64_strict(v, n) ||
        n > static_cast<unsigned long long>(std::numeric_limits<T>::max()))
    {
        if (bad_key.empty())
            bad_key = key;
        return fallback;
    }
    return static_cast<T>(n);
}

std::mutex &conf_mu()
{
    static std::mutex mu;
    return mu;
}

void report_bad_section(const std::string &file, const std::string &section, const std::string &why)
{
    std::cerr << "[mqtt_client] section [" << section << "] unusable in "
              << (file.empty() ? "mqtt.conf" : file)
              << ": " << why << std::endl;
}

std::optional<mqtt_conn_config_t> parse_section(const http::parse_ini &ini,
                                                const std::string &file,
                                                const std::string &section)
{
    auto [sec, found] = ini.config.try_section(section);
    if (!found || sec == nullptr)
    {
        report_bad_section(file, section, "section not found");
        return std::nullopt;
    }
    mqtt_conn_config_t cfg;
    std::string bad_key;
    std::string host = field_str(*sec, "host");
    if (!host.empty())
        cfg.host = host;
    cfg.port        = field_num<unsigned short>(*sec, "port", cfg.port, bad_key);
    cfg.clientid    = field_str(*sec, "clientid");
    cfg.username    = field_str(*sec, "username");
    cfg.password    = field_str(*sec, "password");
    cfg.url         = field_str(*sec, "url");
    cfg.isssl       = parse_bool(field_str(*sec, "isssl"));
    cfg.sslport     = field_num<unsigned short>(*sec, "sslport", cfg.sslport, bad_key);
    cfg.insecure    = parse_bool(field_str(*sec, "insecure"));
    cfg.ca_file     = field_str(*sec, "ca_file");
    cfg.sni         = field_str(*sec, "sni");
    cfg.timeout_sec = field_num<unsigned int>(*sec, "timeout", cfg.timeout_sec, bad_key);
    cfg.keepalive   = field_num<uint16_t>(*sec, "keepalive", cfg.keepalive, bad_key);

    if (!cfg.clientid.empty())
    {
        // 允许空字符串作为合法 clientid（MQTT 协议允许），但更常见是必填
    }

    if (!bad_key.empty())
    {
        report_bad_section(file, section, "field " + bad_key + " is not a number");
        return std::nullopt;
    }
    return cfg;
}
}// namespace

mqtt_config_t &get_mqtt_config()
{
    static mqtt_config_t instance;
    return instance;
}

bool mqtt_config_t::load(const std::string &filename)
{
    std::lock_guard<std::mutex> lk(conf_mu());
    loaded = false;
    file.clear();
    cache_.clear();
    data.config.clear();
    if (filename.empty())
        return false;
    std::error_code ec;
    if (!fs::exists(filename, ec) || !fs::is_regular_file(filename, ec))
        return false;
    try
    {
        data.parse_file(filename);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[mqtt_client] load config fail: " << filename << " " << e.what() << std::endl;
        return false;
    }
    file   = filename;
    loaded = true;
    return true;
}

bool mqtt_config_t::has(const std::string &section) const
{
    std::lock_guard<std::mutex> lk(conf_mu());
    return data.config.try_section(section).second;
}

std::optional<mqtt_conn_config_t> mqtt_config_t::get(const std::string &section) const
{
    std::lock_guard<std::mutex> lk(conf_mu());
    return parse_section(data, file, section);
}

std::string mqtt_config_t::raw_field(const std::string &section, const std::string &key) const
{
    std::lock_guard<std::mutex> lk(conf_mu());
    auto [sec, found] = data.config.try_section(section);
    if (!found || sec == nullptr)
        return std::string();
    return field_str(*sec, key);
}

std::vector<std::string> mqtt_config_t::sections() const
{
    std::vector<std::string> out;
    std::lock_guard<std::mutex> lk(conf_mu());
    for (const auto &sec : data.config)
        if (!sec.name.empty())
            out.push_back(sec.name);
    return out;
}

std::optional<mqtt_conn_config_t> mqtt_conf(const std::string &section)
{
    auto &cfg = get_mqtt_config();
    std::lock_guard<std::mutex> lk(conf_mu());
    auto it = cfg.cache_.find(section);
    if (it != cfg.cache_.end())
        return it->second;
    auto one = parse_section(cfg.data, cfg.file, section);
    if (one)
        cfg.cache_.emplace(section, *one);
    return one;
}

bool load_mqtt_config(const std::string &server_conf_file)
{
    std::string dir;
    std::size_t pos = server_conf_file.find_last_of("/\\");
    if (pos != std::string::npos)
        dir = server_conf_file.substr(0, pos + 1);
    std::string path = dir + "mqtt.conf";
    std::error_code ec;
    if (!fs::exists(path, ec))
    {
        std::string fallback = "./conf/mqtt.conf";
        if (fs::exists(fallback, ec))
            path = fallback;
    }
    auto &cfg = get_mqtt_config();
    if (!cfg.load(path))
    {
        std::cerr << "[mqtt_client] " << path << " not found or parse fail, mqtt client disabled" << std::endl;
        return false;
    }
    std::cerr << "[mqtt_client] loaded " << cfg.sections().size() << " sections from " << path << std::endl;
    return true;
}

bool reload_mqtt_config()
{
    auto &cfg = get_mqtt_config();
    std::string path;
    {
        std::lock_guard<std::mutex> lk(conf_mu());
        if (!cfg.loaded)
            return false;
        path = cfg.file;
    }
    return cfg.load(path);
}

}// namespace http

#endif// ENABLE_MQTT_CLIENT
