//
// ws_handshake.h — WebSocket 握手层
//
// 校验升级请求、生成 101 响应或错误响应。纯函数，无状态。
//

#ifndef PROJECT_WS_HANDSHAKE_H
#define PROJECT_WS_HANDSHAKE_H

#include <string>
#include <string_view>
#include "http_header.h"

namespace http
{
namespace ws
{

// 校验握手请求，通过返回 true，否则返回 false（调用方应返回 400）。
bool validate(const websocket_t &ws, bool has_upgrade, bool has_connection_upgrade,
              unsigned char method);

// 计算 Sec-WebSocket-Accept 的值
std::string compute_accept_key(std::string_view client_key);

// 生成 101 Switching Protocols 响应
std::string make_101(std::string_view client_key, bool deflate = false);

// 握手失败时的 HTTP 错误响应（400、404 或 426）
std::string make_error(int status = 400);

}// namespace ws
}// namespace http

#endif// PROJECT_WS_HANDSHAKE_H
