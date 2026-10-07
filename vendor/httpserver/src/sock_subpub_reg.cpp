/*
 * WebSocket 长连接客户端注册工厂 singleton
 */
#ifdef ENABLE_SOCKETS_CLIENT
#include "sock_subpub_reg.h"

namespace http
{
SOCK_SUBPUB_REG &get_sock_subpub_reg()
{
    static SOCK_SUBPUB_REG instance;
    return instance;
}
}// namespace http
#endif
