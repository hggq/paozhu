/*
 * WebSocket 长连接客户端注册工厂 singleton
 */
#ifdef ENABLE_WEBSOCKETS_CLIENT
#include "ws_subpub_reg.h"

namespace http
{
WS_SUBPUB_REG &get_ws_subpub_reg()
{
    static WS_SUBPUB_REG instance;
    return instance;
}
}// namespace http
#endif
