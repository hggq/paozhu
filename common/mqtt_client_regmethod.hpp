#pragma once
/*
 * MQTT 长连接客户端业务注册入口（出站 — 连接外部 broker）
 *
 * 业务侧的 mqtt_subpub_client 子类写在各自业务目录里，
 * 在这个文件里 include 并 emplace 进注册表。
 * server.cpp 启动时遍历注册表，对每个客户端 co_spawn async_mqtt_subpub_loop：
 *   async_tcp_connect → async_mqtt_connect → pump 回调分发 → 断连重连循环
 * websocket_loop 每秒一拍扫 mqtt_clients，按 durtime 分频调 run_loop/async_run_loop。
 *
 * 线程模型：完全对称 ws_subpub_client / sock_subpub_client
 *   is_coroutine_ = false  →  收到 broker PUBLISH 丢 clientrunpool
 *   is_coroutine_ = true   →  收到 broker PUBLISH co_spawn 到 io_context 新协程
 */
#ifdef ENABLE_MQTT_CLIENT

#include "mqtt_subpub_reg.h"

#include "mqtt/echo_mqtt_client.hpp"
#include "mqtt/echo_mqtt_client_co.hpp"

namespace http
{

inline void _initmqttsubpubregto(http::MQTT_SUBPUB_REG &reg)
{
    // reg.emplace("my_mqtt_client",
    //             [] { return std::make_shared<my_ns::my_mqtt_client>(); });

    // 框架级验证客户端（跑通后业务侧自行增删）
    // 注: conf/mqtt.conf [default] 的 host/port 指向外部 broker（如 EMQX / Mosquitto）
    reg.emplace("echo_mqtt_client",
                [] { return std::make_shared<mqtt_framework_test::echo_mqtt_client>(); });
    reg.emplace("echo_mqtt_client_co",
                [] { return std::make_shared<mqtt_framework_test::echo_mqtt_client_co>(); });
}

} // namespace http

#endif // ENABLE_MQTT_CLIENT
