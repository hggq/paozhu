#pragma once
// MQTT 控制报文拆帧（服务端 mqtt_session 与客户端 mqtt_client 共用）
// 三态语义：仅 need_more 才继续读；malformed / too_large 是终态，
// 应回 DISCONNECT(0x81 / 0x95) 并断链（spec §2.1.4 / §3.1.4.7）。

#include <cstddef>
#include <cstdint>

namespace http
{

enum class mqtt_frame_state
{
    complete,
    need_more,
    malformed,
    too_large
};

struct mqtt_frame_view
{
    uint8_t        fixed    = 0;
    const uint8_t *body     = nullptr;
    size_t         body_len = 0;
    size_t         total    = 0;// 整包字节数（固定头 + 变长头 + body）
};

// 从 [data, data+len) 头部尝试拆一帧；max_packet 为本连接单帧上限
// （协议硬上限 268435455 已内含；声明超上限的帧头读完即返回 too_large，不等 body）
inline mqtt_frame_state mqtt_frame_parse(const uint8_t *data, size_t len, size_t max_packet,
                                         mqtt_frame_view &out)
{
    if (len < 2) return mqtt_frame_state::need_more;

    uint32_t remaining  = 0;
    uint32_t multiplier = 1;
    size_t   off        = 1;
    for (int i = 0; i < 4; ++i)
    {
        if (off >= len) return mqtt_frame_state::need_more;// varint 未收完

        const uint8_t b = data[off++];
        remaining += static_cast<uint32_t>(b & 0x7F) * multiplier;
        if ((b & 0x80) == 0)
        {
            if (remaining > 0x0FFFFFFFu) return mqtt_frame_state::malformed;
            if (static_cast<size_t>(remaining) > max_packet) return mqtt_frame_state::too_large;
            if (len - off < remaining) return mqtt_frame_state::need_more;

            out.fixed    = data[0];
            out.body     = data + off;
            out.body_len = remaining;
            out.total    = off + remaining;
            return mqtt_frame_state::complete;
        }
        multiplier *= 128;
    }
    return mqtt_frame_state::malformed;// 第 4 字节仍带续位
}

}// namespace http
