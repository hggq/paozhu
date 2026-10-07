/*
 * MQTT 长连接客户端注册工厂 singleton
 */
#ifdef ENABLE_MQTT_CLIENT
#include "mqtt_subpub_reg.h"

namespace http
{
MQTT_SUBPUB_REG &get_mqtt_subpub_reg()
{
    static MQTT_SUBPUB_REG instance;
    return instance;
}
}// namespace http
#endif
