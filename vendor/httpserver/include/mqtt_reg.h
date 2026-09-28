#ifndef HTTP_MQTT_REG_H
#define HTTP_MQTT_REG_H

#include <functional>
#include <map>
#include <memory>

#include "mqtt_api.h"
#include "mqtt_frame.h"
#include "mqtt_session.h"

namespace http
{

// 工厂签名: (client_info, session) → shared_ptr<mqtt_api>
typedef std::map<std::string,
                 std::function<std::shared_ptr<mqtt_api>(const mqtt_client_info &,
                                                         std::shared_ptr<mqtt_session>)>>
    MQTT_REG;

MQTT_REG &get_mqtt_reg();

}// namespace http

#endif
