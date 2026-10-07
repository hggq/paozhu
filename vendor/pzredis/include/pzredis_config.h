#ifndef PZ_REDIS_CONFIG_H
#define PZ_REDIS_CONFIG_H
/*
 * pzredis 配置加载（conf/redis.conf）
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 *
 * 段名即连接名，与 orm.conf 的 db tag 同理。
 * 整个头文件在 ENABLE_REDIS 关闭时为空，调用方用 #ifdef ENABLE_REDIS 决定是否使用。
 */
#include <asio.hpp>
#include <asio/io_context.hpp>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "redis_conn.h"
#include "parse_ini.h"

#ifdef ENABLE_REDIS

namespace pz
{
namespace redis
{

struct redis_config_t
{
    http::parse_ini data;
    std::string file;
    bool loaded = false;

    // 重新解析文件；成功会把已缓存的连接配置一起清空（配置变了，缓存必须跟着作废）
    bool load(const std::string &filename);
    bool is_load() const { return loaded; }
    bool has(const std::string &section) const;
    // 解析某一段。段不存在，或 host/port/dbindex/timeout 这类数字字段写了但不是数字 ⇒ nullopt，
    // 并打一行日志说明是哪一段、哪个字段。不会退回默认值：连到错误的端口比连不上更难查。
    std::optional<conn_config_t> get(const std::string &section = "default") const;
    // 读某一段里任意一个字段的原文（不属于 conn_config_t 的配置走这里，例如连接池的 maxpool）。
    // 段或字段不存在返回空串，不抛。
    std::string raw_field(const std::string &section, const std::string &key) const;
    std::vector<std::string> sections() const;

    // 缓存：get/redis_conf 反复访问时避免每次重解析（只缓存解析成功的段）
    mutable std::map<std::string, conn_config_t> cache_;
};

redis_config_t &get_redis_config();

// 启动加载入口（httpserver::run() 调用）：
// server_conf_file 为 server.conf 全路径，从中取目录拼出 redis.conf（如 /etc/paozhu/conf/redis.conf）。
// load 成功后初始化并启动连接池。
bool load_redis_config(asio::io_context &server_ioc, const std::string &server_conf_file);

// 取某段连接配置（懒取 + 缓存）。段名拼错或段里有坏掉的数字字段 ⇒ nullopt。
std::optional<conn_config_t> redis_conf(const std::string &section = "default");

// 改了 conf 之后手动重载（会重建连接池的 section 配置，不立即断旧连接）
bool reload_redis_config(asio::io_context &server_ioc);

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS
#endif
