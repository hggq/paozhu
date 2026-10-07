#ifndef PZ_SOCK_SUBPUB_REG_H
#define PZ_SOCK_SUBPUB_REG_H
/*
 * WebSocket 长连接客户端注册工厂 map
 * 完全镜像 redis_subpub_reg.h
 */
#include <functional>
#include <map>
#include <memory>
#include <string>

#ifdef ENABLE_SOCKETS_CLIENT

#include "sock_subpub_client.h"

namespace http
{

using SOCK_SUBPUB_FACTORY = std::function<std::shared_ptr<sock_subpub_client>()>;
using SOCK_SUBPUB_REG     = std::map<std::string, SOCK_SUBPUB_FACTORY>;

SOCK_SUBPUB_REG &get_sock_subpub_reg();

}// namespace http

#endif// ENABLE_SOCKETS_CLIENT
#endif
