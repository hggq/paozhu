#ifndef PZ_WS_SUBPUB_REG_H
#define PZ_WS_SUBPUB_REG_H
/*
 * WebSocket 长连接客户端注册工厂 map
 * 完全镜像 redis_subpub_reg.h
 */
#include <functional>
#include <map>
#include <memory>
#include <string>

#ifdef ENABLE_WEBSOCKETS_CLIENT

#include "ws_subpub_client.h"

namespace http
{

using WS_SUBPUB_FACTORY = std::function<std::shared_ptr<ws_subpub_client>()>;
using WS_SUBPUB_REG     = std::map<std::string, WS_SUBPUB_FACTORY>;

WS_SUBPUB_REG &get_ws_subpub_reg();

}// namespace http

#endif// ENABLE_WEBSOCKETS_CLIENT
#endif
