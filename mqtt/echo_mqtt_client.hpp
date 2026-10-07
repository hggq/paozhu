/*
 * 框架级 MQTT broker 出站连接验证客户端（同步版）
 *
 * 连接 conf/mqtt.conf [default] 段指定的外部 broker
 * on_open 里 subscribe 主题，run_loop 周期 publish 测试消息
 * broker 下发的 PUBLISH 走 on_message(topic, payload, qos)
 *
 * 线程模型：
 *   is_coroutine_ = false  →  broker PUBLISH 回调丢 clientrunpool
 *                            run_loop 跑 tick 线程
 */
#pragma once

#include "mqtt_subpub_client.h"
#include <cstdio>
#include <string>

namespace mqtt_framework_test
{

class echo_mqtt_client : public http::mqtt_subpub_client
{
  public:
    echo_mqtt_client()
    {
        is_coroutine_ = false;
        is_loop_co_   = false;
        durtime       = 10;// 每 10 秒跑一轮 run_loop
        loop_num      = 999999;
    }

    std::string section_name() const override { return "echo_sync"; }

    void on_open() override
    {
        std::fprintf(stderr, "[echo_mqtt] ✅ CONNECTED\n");
        std::fflush(stderr);
        // 同步钩子里没有 co_await，subscribe 必须异步调
        // 这里起个独立协程去 subscribe（框架的 io_context）
        auto self = shared_from_this();
        asio::co_spawn(*io_ctx, [self]() -> asio::awaitable<void>
                       {
                auto ok = co_await self->async_subscribe("paozhu/echo/out", 1);
                std::fprintf(stderr, "[echo_mqtt] SUB paozhu/echo/out qos=1 → %s\n",
                             ok ? "OK" : "FAIL");
                std::fflush(stderr);
                co_return; },
                       asio::detached);
    }

    void on_close() override
    {
        std::fprintf(stderr, "[echo_mqtt] ⚠️  DISCONNECTED\n");
        std::fflush(stderr);
    }

    void on_message(std::string_view topic, std::string_view payload, uint8_t qos) override
    {
        std::fprintf(stderr, "[echo_mqtt] 📨  RECV topic=%s qos=%u payload=%s\n", std::string(topic).c_str(), qos, std::string(payload).c_str());
        std::fflush(stderr);
    }

    void run_loop() override
    {
        static int round = 0;
        auto mqc         = conn();
        if (!mqc || !mqc->sock)
            return;
        std::string msg = "MQTT_ECHO #" + std::to_string(++round);
        auto self       = shared_from_this();
        asio::co_spawn(*io_ctx, [self, msg]() -> asio::awaitable<void>
                       {
                auto ok = co_await self->async_publish("paozhu/echo/out", msg, 1);
                std::fprintf(stderr, "[echo_mqtt] 📤  PUB paozhu/echo/out %s → %s\n",
                             msg.c_str(), ok ? "OK" : "FAIL");
                std::fflush(stderr);
                co_return; },
                       asio::detached);
    }
};

}// namespace mqtt_framework_test
