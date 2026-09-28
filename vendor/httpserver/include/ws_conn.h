//
// ws_conn.h — WebSocket 出站帧发送
//
// 所有帧经 serialize_frame 编码后通过 client_session::post_write 入发送环，
// 由 ring_client_server 统一写出，避免并发写同一 socket。
//

#ifndef PROJECT_WS_CONN_H
#define PROJECT_WS_CONN_H

#include <string>
#include <string_view>
#include "ws_wire.h"

namespace http
{
class client_session;

namespace ws
{

// 编码帧并入环，返回 false 表示环满或连接已关闭；rsv=1 打 RSV1（压缩标记）
bool post_send_frame(client_session &session, opcode op, std::string_view payload, bool fin = true,
                     unsigned char rsv = 0);

// text/binary：已协商 permessage-deflate 且载荷 > CONST_WEBSOCKET_DEFLATE_MIN_SIZE 时
// 单消息 raw deflate 并置 RSV1；压缩不划算则原样发送（RSV1=0，RFC 7692 允许）
bool post_send_text(client_session &session, std::string_view payload);

bool post_send_binary(client_session &session, std::string_view payload);

bool post_send_close(client_session &session, uint16_t code = 1000, std::string_view reason = {});

bool post_send_ping(client_session &session, std::string_view payload = "ping");

bool post_send_pong(client_session &session, std::string_view payload);

}// namespace ws
}// namespace http

#endif// PROJECT_WS_CONN_H
