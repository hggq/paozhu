#include "mqtt_reg.h"
#include "mqtt_method_reg.hpp"

namespace http
{

MQTT_REG &get_mqtt_reg()
{
    static MQTT_REG instance;
    static bool inited = []() {
        _initmqttmethodregto(instance);
        return true;
    }();
    (void)inited;
    return instance;
}

} // namespace http
