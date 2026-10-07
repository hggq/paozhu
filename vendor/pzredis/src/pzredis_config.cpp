/*
 * pzredis 配置加载实现
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 */
#include "pzredis_config.h"

#ifdef ENABLE_REDIS

#include <filesystem>
#include <iostream>
#include <limits>
#include <mutex>
#include <optional>
#include <stdexcept>

#include "func.h"           // str2uint64_strict
#include "server_localvar.h"// http::server_loaclvar / get_server_global_var
#include "redis_pool.h"     // load_redis_config 需要初始化连接池

namespace pz
{
namespace redis
{
namespace fs = std::filesystem;

namespace
{
// 接受 1/true/TRUE/On/ON
bool parse_bool(const std::string &v)
{
    return v == "1" || v == "true" || v == "True" || v == "TRUE" || v == "On" || v == "ON";
}

// 段里没写这一行时，ini 的 const operator[] 是抛 out_of_range 的：整段解析会在那一行停下，
// 排在它后面的字段全部退回默认值。所以逐字段用 try_find 取，缺行只影响这一个字段。
std::string field_str(const http::ini_section_value_t &sec, const std::string &key)
{
    auto found = sec.try_find(key);
    return found.second ? *found.first : std::string();
}

// 数字字段：没写这行 ⇒ 用传入的默认值；写了但不是纯数字（或超出本字段装得下的范围）⇒ 把字段名记进
// bad_key，由调用方整段失败。端口/库号/期限这种数值字段，静默换成默认值比直接失败危险得多
//（"port = abc" 连到 6379 是最难查的一种错）。
// 解析本身用框架的 str2uint64_strict（func.h）：只收纯数字，空格/负号/十六进制一律不算，
// 比 std::stoull 的前缀解析严格。
template <typename T>
T field_num(const http::ini_section_value_t &sec, const std::string &key, T fallback, std::string &bad_key)
{
    std::string v = field_str(sec, key);
    if (v.empty())
        return fallback;
    unsigned long long n = 0;
    if (!http::str2uint64_strict(v, n) || n > static_cast<unsigned long long>(std::numeric_limits<T>::max()))
    {
        if (bad_key.empty())
            bad_key = key;
        return fallback;
    }
    return static_cast<T>(n);
}

// 配置对象唯一的读写锁：业务线程随时可能读某一段（redis_conf 的缓存未命中就要读 ini 正文），
// 而 reload 会把整份 ini 换掉。缓存表、ini 正文、文件名都围着这一把锁，读侧不会看到半新半旧。
std::mutex &conf_mu()
{
    static std::mutex mu;
    return mu;
}

// 整段解析失败统一从这里出去：把段名、文件和原因一起打出来，
// 别让线上只剩下"redis 连不上"这一句。
void report_bad_section(const std::string &file, const std::string &section, const std::string &why)
{
    std::cerr << "[redis] section [" << section << "] unusable in " << (file.empty() ? "redis.conf" : file)
              << ": " << why << std::endl;
}

// 真正干活的解析，调用方必须已经持有 conf_mu()（get() 与 redis_conf() 都走它，避免嵌套加锁）。
std::optional<conn_config_t> parse_section(const http::parse_ini &ini, const std::string &file, const std::string &section)
{
    // 用 try_section / try_find 而不是 operator[]：const 版 operator[] 查不到就抛 out_of_range，
    // 而段里少写一个可选字段是常事，绝不能让它把它后面的字段一起吞掉（与 webpay_config 同一个取法）
    auto [sec, found] = ini.config.try_section(section);
    if (!found || sec == nullptr)
    {
        report_bad_section(file, section, "section not found");
        return std::nullopt;
    }

    conn_config_t cfg;
    std::string bad_key;
    std::string host = field_str(*sec, "host");
    if (!host.empty())
        cfg.host = host;
    cfg.port        = field_num<unsigned short>(*sec, "port", cfg.port, bad_key);
    cfg.username    = field_str(*sec, "username");
    cfg.password    = field_str(*sec, "password");
    cfg.dbindex     = field_num<unsigned int>(*sec, "dbindex", cfg.dbindex, bad_key);
    cfg.isssl       = parse_bool(field_str(*sec, "isssl"));
    cfg.insecure    = parse_bool(field_str(*sec, "insecure"));
    cfg.ca_file     = field_str(*sec, "ca_file");
    cfg.sni         = field_str(*sec, "sni");
    cfg.timeout_sec = field_num<unsigned int>(*sec, "timeout", cfg.timeout_sec, bad_key);
    cfg.prefix      = field_str(*sec, "prefix");
    cfg.isdebug     = parse_bool(field_str(*sec, "isdebug"));

    if (!bad_key.empty())
    {
        report_bad_section(file, section, "field " + bad_key + " is not a number");
        return std::nullopt;
    }
    return cfg;
}
}// namespace

redis_config_t &get_redis_config()
{
    static redis_config_t instance;
    return instance;
}

bool redis_config_t::load(const std::string &filename)
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
        data.parse_file(filename);// 打不开会抛 runtime_error
    }
    catch (const std::exception &e)
    {
        std::cerr << "[redis] load config fail: " << filename << " " << e.what() << std::endl;
        return false;
    }

    file   = filename;
    loaded = true;
    return true;
}

bool redis_config_t::has(const std::string &section) const
{
    std::lock_guard<std::mutex> lk(conf_mu());
    return data.config.try_section(section).second;
}

std::optional<conn_config_t> redis_config_t::get(const std::string &section) const
{
    std::lock_guard<std::mutex> lk(conf_mu());
    return parse_section(data, file, section);
}

std::string redis_config_t::raw_field(const std::string &section, const std::string &key) const
{
    std::lock_guard<std::mutex> lk(conf_mu());
    auto [sec, found] = data.config.try_section(section);
    if (!found || sec == nullptr)
        return std::string();
    return field_str(*sec, key);
}

std::vector<std::string> redis_config_t::sections() const
{
    std::vector<std::string> out;
    std::lock_guard<std::mutex> lk(conf_mu());
    for (const auto &sec : data.config)
    {
        if (!sec.name.empty())
            out.push_back(sec.name);
    }
    return out;
}

std::optional<conn_config_t> redis_conf(const std::string &section)
{
    auto &cfg = get_redis_config();
    std::lock_guard<std::mutex> lk(conf_mu());
    auto it = cfg.cache_.find(section);
    if (it != cfg.cache_.end())
        return it->second;
    // 只缓存解析成功的段：拼错段名不会被记成"永久失败"，改对 conf 并 reload 之后立刻生效
    auto one = parse_section(cfg.data, cfg.file, section);
    if (one)
        cfg.cache_.emplace(section, *one);
    return one;
}

bool load_redis_config(asio::io_context &server_ioc, const std::string &server_conf_file)
{
    // 从 server.conf 全路径取目录，拼出同目录下的 redis.conf
    std::string dir;
    std::size_t pos = server_conf_file.find_last_of("/\\");
    if (pos != std::string::npos)
        dir = server_conf_file.substr(0, pos + 1);
    std::string path = dir + "redis.conf";

    redis_config_t &cfg = get_redis_config();
    if (!cfg.load(path))
    {
        std::cerr << "[redis] " << path << " not found or parse fail, redis disabled" << std::endl;
        return false;
    }

    get_redis_pool().init(server_ioc, cfg);
    get_redis_pool().start_keepalive();
    return true;
}

bool reload_redis_config(asio::io_context &server_ioc)
{
    redis_config_t &cfg = get_redis_config();
    std::string path;
    {
        std::lock_guard<std::mutex> lk(conf_mu());
        if (!cfg.loaded)
            return false;
        path = cfg.file;
    }
    if (!cfg.load(path))// load 会连着缓存表一起清空
        return false;
    get_redis_pool().init(server_ioc, cfg);
    // reload 不重复启动 keepalive：start_keepalive 只在首次 load 时调一次就够
    return true;
}

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS
