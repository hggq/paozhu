#ifndef PZ_MQTT_SUBPUB_REG_H
#define PZ_MQTT_SUBPUB_REG_H
/*
 * MQTT 长连接客户端注册工厂 map
 * 完全镜像 sock_subpub_reg.h / ws_subpub_reg.h
 */
#include <functional>
#include <map>
#include <memory>
#include <string>

#ifdef ENABLE_MQTT_CLIENT

#include "mqtt_subpub_client.h"

namespace http
{

using MQTT_SUBPUB_FACTORY = std::function<std::shared_ptr<mqtt_subpub_client>()>;
using MQTT_SUBPUB_REG     = std::map<std::string, MQTT_SUBPUB_FACTORY>;

MQTT_SUBPUB_REG &get_mqtt_subpub_reg();

}// namespace http

#endif// ENABLE_MQTT_CLIENT
#endif
