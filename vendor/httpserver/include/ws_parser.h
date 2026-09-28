//
// ws_parser.h — WebSocket 入站帧解析
//
// 组合 header_parser + message_assembler + input_buffer，
// 喂入原始字节，产出完整消息或控制帧。
// 单线程使用，不依赖 asio。
//

#ifndef PROJECT_WS_PARSER_H
#define PROJECT_WS_PARSER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "websockets.h"
#include "ws_wire.h"

namespace http
{
namespace ws
{

// 控制帧事件（close/ping/pong）
struct control_event
{
    opcode      op;
    std::string payload;
};

// 入站解析器：喂入字节，产出消息或控制帧，内部用队列缓存。
class ws_parser
{
  public:
    ws_parser()  = default;
    ~ws_parser();

    // 设置限额（取自 common/cost_define.h 编译期常量，调用一次）
    void set_limits(uint64_t max_frame_payload, uint64_t max_message_payload, uint64_t spill_threshold);

    // permessage-deflate 已协商：允许数据帧首帧 RSV1，载荷按 no-context 流 inflate（须在首次 feed 前设置）
    void set_deflate_ext(bool enabled) { deflate_ext_ = enabled; }

    // 喂入原始字节，循环解析完整帧。返回 false 表示协议错误，应断开连接。
    bool feed(const unsigned char *data, size_t len);

    bool has_message() const { return !message_queue_.empty(); }
    bool has_control() const { return !control_queue_.empty(); }
    bool error() const { return error_; }
    // feed 失败后应回送的 Close 状态码：1002 协议错误 / 1007 文本非 UTF-8 或解压失败 / 1009 超限 / 1011 内部错误
    unsigned short error_code() const { return error_code_; }

    websockets_data_list_t pop_message();

    control_event pop_control();

    void reset();

  private:
    // 置错误并返回 false，code 为对端应收到的 Close 状态码
    bool fail(unsigned short code);

    // 删除队列中尚未交付的落盘消息对应的临时文件
    void discard_queued_files();

    // inflate 输出直灌聚合器（解压后字节参与聚合/限额/UTF-8 口径）
    static bool assembler_sink(const unsigned char *out, size_t n, void *ud);

    input_buffer      input_;
    header_parser     header_parser_;
    message_assembler assembler_;
    permessage_inflate inflator_;

    std::vector<websockets_data_list_t> message_queue_;
    std::vector<control_event>          control_queue_;

    uint64_t        max_frame_payload_ = kDefaultMaxFramePayload;
    bool            deflate_ext_       = false; // permessage-deflate 已协商
    bool            msg_deflated_      = false; // 当前消息处于压缩流中
    bool            error_             = false;
    unsigned short  error_code_        = 1002;
    bool            header_complete_   = false;
    uint64_t        frame_payload_left_ = 0;
    bool            frame_is_control_  = false;
    std::string     ctl_payload_;
    unsigned char   frame_mask_key_[4] = {0, 0, 0, 0};
    unsigned int    frame_mask_offset_ = 0;
    size_t          frame_fed_         = 0; // 本帧已喂给聚合/解压的载荷字节（0 字节末分片补冲净用）
    bool            frame_fin_         = false; // 当前帧 FIN 位（载荷消费段引用）
    unsigned int    next_seq_id_       = 0;
};

}// namespace ws
}// namespace http

#endif// PROJECT_WS_PARSER_H
