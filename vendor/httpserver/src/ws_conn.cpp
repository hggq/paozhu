#include "ws_conn.h"
#include "client_session.h"
#include "cost_define.h"

namespace http
{
namespace ws
{

bool post_send_frame(client_session &session, opcode op, std::string_view payload, bool fin, unsigned char rsv)
{
    if (session.isclose || session.iserror)
        return false;
    std::string frame = serialize_frame(op, payload, fin, rsv);
    return session.post_write(frame);
}

bool post_send_text(client_session &session, std::string_view payload)
{
    if (session.ws_deflate && payload.size() > CONST_WEBSOCKET_DEFLATE_MIN_SIZE)
    {
        std::string zbuf;
        if (raw_deflate_once(payload, zbuf))
            return post_send_frame(session, opcode::text, zbuf, true, 4);
        // 压缩不划算或 zlib 失败：按 RSV1=0 原样发送
    }
    return post_send_frame(session, opcode::text, payload);
}

bool post_send_binary(client_session &session, std::string_view payload)
{
    // 出站压缩只对 text 开启，binary 不压缩
    return post_send_frame(session, opcode::binary, payload);
}

bool post_send_close(client_session &session, uint16_t code, std::string_view reason)
{
    std::string body;
    body.push_back(static_cast<char>((code >> 8) & 0xFF));
    body.push_back(static_cast<char>(code & 0xFF));
    // RFC 6455 §5.5：控制帧载荷 ≤125 字节，状态码占 2 字节，原因最多 123 字节。
    // §7.1.7 又要求原因必须是合法 UTF-8，所以非法输入整段丢弃只发 2 字节状态码；
    // 合法输入按码点边界回退截断，避免把末尾的多字节字符切成一半发出去。
    if (!reason.empty() && utf8_valid(reason))
    {
        size_t cut = reason.size();
        while (cut > 123)// 回退到上一个完整字符起点
        {
            --cut;
            while (cut > 0 && ((unsigned char)reason[cut] & 0xC0) == 0x80)
                --cut;
        }
        body.append(reason.substr(0, cut));
    }
    return post_send_frame(session, opcode::close, body);
}

bool post_send_ping(client_session &session, std::string_view payload)
{
    if (payload.size() > 125)
        payload = payload.substr(0, 125);
    return post_send_frame(session, opcode::ping, payload);
}

bool post_send_pong(client_session &session, std::string_view payload)
{
    if (payload.size() > 125)
        payload = payload.substr(0, 125);
    return post_send_frame(session, opcode::pong, payload);
}

}// namespace ws
}// namespace http
