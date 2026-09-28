#pragma once
#include "mqtt_reg.h"
#include "mqtt_session.h"
#include "mqtt/my_test_mqtt.hpp"
#include "mqtt/async_my_test_mqtt.hpp"

namespace http
{

inline void _initmqttmethodregto(MQTT_REG &reg)
{
    // reg_key = "mydevice"     → Client ID: "mydevice@@@group-device"
    reg.emplace("mydevice", [](const mqtt_client_info &info, std::shared_ptr<mqtt_session> s) {
        return std::make_shared<my_test_mqtt>(info, s);
    });

    // reg_key = "mydeviceco"   → 协程版 demo（is_coroutine() == true）
    reg.emplace("mydeviceco", [](const mqtt_client_info &info, std::shared_ptr<mqtt_session> s) {
        return std::make_shared<async_my_test_mqtt>(info, s);
    });
}

}// namespace http
