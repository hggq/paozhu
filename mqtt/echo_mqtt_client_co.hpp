/*
 * 框架级 MQTT broker 出站连接验证客户端（协程版）
 *
 * 镜像 echo_mqtt_client.hpp，钩子全走协程版。
 * async_on_open 里直接 co_await async_subscribe，
 * async_run_loop 里直接 co_await async_publish。
 */
#pragma once

#include "mqtt_subpub_client.h"
#include <cstdio>
#include <string>

namespace mqtt_framework_test
{

class echo_mqtt_client_co : public http::mqtt_subpub_client
{
  public:
    echo_mqtt_client_co()
    {
        is_coroutine_ = true;
        is_loop_co_   = true;
        durtime       = 10;
        loop_num      = 999999;
    }

    std::string section_name() const override { return "echo_co"; }

    asio::awaitable<void> async_on_open() override
    {
        std::fprintf(stderr, "[echo_mqtt_co] ✅ CONNECTED\n");
        std::fflush(stderr);
        auto ok = co_await async_subscribe("paozhu/echo/out", 1);
        std::fprintf(stderr, "[echo_mqtt_co] SUB paozhu/echo/out qos=1 → %s\n", ok ? "OK" : "FAIL");
        std::fflush(stderr);
        co_return;
    }

    asio::awaitable<void> async_on_close() override
    {
        std::fprintf(stderr, "[echo_mqtt_co] ⚠️  DISCONNECTED\n");
        std::fflush(stderr);
        co_return;
    }

    asio::awaitable<void> async_on_message(std::string_view topic,
                                           std::string_view payload,
                                           uint8_t qos) override
    {
        std::fprintf(stderr, "[echo_mqtt_co] 📨  RECV topic=%s qos=%u payload=%s\n", std::string(topic).c_str(), qos, std::string(payload).c_str());
        std::fflush(stderr);
        co_return;
    }

    asio::awaitable<void> async_run_loop() override
    {
        static int round = 0;
        auto mqc         = conn();
        if (!mqc || !mqc->sock)
            co_return;
        std::string msg = "CO_MQTT_ECHO #" + std::to_string(++round);
        auto ok         = co_await async_publish("paozhu/echo/out", msg, 1);
        std::fprintf(stderr, "[echo_mqtt_co] 📤  PUB paozhu/echo/out %s → %s\n", msg.c_str(), ok ? "OK" : "FAIL");
        std::fflush(stderr);
        co_return;
    }
};

}// namespace mqtt_framework_test
