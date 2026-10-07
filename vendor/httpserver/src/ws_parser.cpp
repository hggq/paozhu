#include "ws_parser.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace http
{
namespace ws
{

void ws_parser::set_limits(uint64_t max_frame_payload, uint64_t max_message_payload, uint64_t spill_threshold)
{
    max_frame_payload_ = max_frame_payload;
    assembler_.set_limits(spill_threshold, max_message_payload);
}

bool ws_parser::feed(const unsigned char *data, size_t len)
{
    if (error_)
        return false;

    // 超出暂存区上限 → 1009
    size_t consumed = input_.append(data, len);
    if (consumed > 0)
        return fail(1009);

    while (!input_.empty() && !error_)
    {
        if (!header_complete_)
        {
            // 解析帧头
            size_t n = header_parser_.feed(
                reinterpret_cast<const unsigned char *>(input_.data().data()),
                input_.size());
            input_.consume(n);

            if (header_parser_.error())
                return fail(1002);
            if (!header_parser_.complete())
                break;// 帧头不完整，等更多数据

            const frame_header &h = header_parser_.header();

            // RSV2/RSV3 恒为 0；RSV1（permessage-deflate，字段值 4=0x40 位）仅在已协商、数据帧、
            // 且非消息中间（continuation 帧 RSV 必须为 0）时允许
            bool is_control = (h.op == opcode::close || h.op == opcode::ping || h.op == opcode::pong);
            if (h.rsv & 0x03)
                return fail(1002);
            if (h.rsv & 0x04)
            {
                if (!deflate_ext_ || is_control || assembler_.in_message())
                    return fail(1002);
                msg_deflated_ = true;
            }

            // RFC 6455 5.1：客户端到服务端的帧 MASK 必须为 1
            if (!h.masked)
                return fail(1002);

            // 控制帧载荷上限 125 字节，且必须 FIN=1
            if (is_control && h.payload_len > 125)
                return fail(1002);
            if (is_control && !h.fin)
                return fail(1002);
            // 单帧载荷上限
            if (h.payload_len > max_frame_payload_)
                return fail(1009);

            // 数据帧进入消息聚合；控制帧跳过聚合器
            frame_is_control_ = is_control;
            ctl_payload_.clear();
            if (!is_control && !assembler_.begin_frame(h.fin, h.op, h.rsv))
                return fail(1002);

            frame_payload_left_ = h.payload_len;
            std::memcpy(frame_mask_key_, h.mask_key, 4);
            frame_mask_offset_ = 0;
            frame_fed_         = 0;
            frame_fin_         = h.fin;
            header_complete_   = true;
        }

        // 消费帧载荷
        if (frame_payload_left_ > 0)
        {
            size_t avail = input_.size();
            if (avail == 0)
                break;

            size_t take = std::min<uint64_t>(avail, frame_payload_left_);
            // 就地解掩码，滚动索引避免逐字节取模
            unsigned char *p = input_.writable_data();
            unsigned int k   = frame_mask_offset_ & 3;
            for (size_t i = 0; i < take; ++i)
            {
                p[i] = static_cast<unsigned char>(p[i] ^ frame_mask_key_[k]);
                k    = (k + 1) & 3;
            }
            frame_mask_offset_ += static_cast<unsigned int>(take);

            if (frame_is_control_)
            {
                ctl_payload_.append(reinterpret_cast<const char *>(p), take);
            }
            else
            {
                bool pushed;
                if (msg_deflated_)
                {
                    // 解压后字节进聚合器：限额/落盘/UTF-8 口径全部按原始消息字节计
                    bool msg_fin = frame_fin_ && (take == frame_payload_left_);
                    pushed       = inflator_.feed(p, take, msg_fin, assembler_sink, &assembler_);
                }
                else
                {
                    pushed = assembler_.push_payload(p, take);
                }
                if (!pushed)
                {
                    if (assembler_.error_too_big())
                        return fail(1009);
                    if (assembler_.error_invalid_utf8())
                        return fail(1007);
                    if (assembler_.error_internal())
                        return fail(1011);
                    if (msg_deflated_ && inflator_.failed())
                        return fail(1007);
                    return fail(1002);
                }
            }
            frame_payload_left_ -= take;
            frame_fed_ += take;
            input_.consume(take);
        }

        // 本帧载荷收完
        if (frame_payload_left_ == 0)
        {
            const frame_header &h = header_parser_.header();

            if (frame_is_control_)
            {
                // 控制帧放入控制队列，不经过聚合器
                control_event evt;
                evt.op      = h.op;
                evt.payload = std::move(ctl_payload_);
                ctl_payload_.clear();
                control_queue_.push_back(std::move(evt));
            }
            else
            {
                if (msg_deflated_ && h.fin && frame_fed_ == 0)
                {
                    // 0 字节末分片：补一次 fin 冲净，防 inflate 流残留未冲净输出/未重置
                    static const unsigned char zero[1] = {0};
                    if (!inflator_.feed(zero, 0, true, assembler_sink, &assembler_))
                    {
                        if (assembler_.error_too_big())
                            return fail(1009);
                        if (assembler_.error_invalid_utf8())
                            return fail(1007);
                        if (assembler_.error_internal())
                            return fail(1011);
                        if (inflator_.failed())
                            return fail(1007);
                        return fail(1002);
                    }
                }
                if (!assembler_.end_frame())
                    return fail(assembler_.error_invalid_utf8() ? 1007 : 1002);

                if (assembler_.finished())
                {
                    websockets_data_list_t msg;
                    msg.seqid = next_seq_id_++;
                    if (assembler_.is_spilled())
                    {
                        msg.isfile = true;
                        msg.value  = assembler_.message_filename();
                    }
                    else
                    {
                        msg.isfile = false;
                        msg.value  = assembler_.take_payload();
                    }
                    message_queue_.push_back(std::move(msg));
                    assembler_.consume_message();
                    msg_deflated_ = false;// 解压流已在末帧冲净并重置（no-context）
                }
            }

            // 重置帧状态，准备下一帧
            header_parser_.reset();
            header_complete_    = false;
            frame_payload_left_ = 0;
            frame_is_control_   = false;
        }
    }
    return true;
}

void ws_parser::discard_queued_files()
{
    // 交付路径（ws_conn 读完后自行删文件）之外的唯一持有者是我们；
    // pop_message 移走所有权后本队列不再负责那条
    for (auto &msg : message_queue_)
    {
        if (msg.isfile && !msg.value.empty())
            std::remove(msg.value.c_str());
    }
}

ws_parser::~ws_parser()
{
    discard_queued_files();
}

websockets_data_list_t ws_parser::pop_message()
{
    websockets_data_list_t m = std::move(message_queue_.front());
    message_queue_.erase(message_queue_.begin());
    return m;
}

control_event ws_parser::pop_control()
{
    control_event e = std::move(control_queue_.front());
    control_queue_.erase(control_queue_.begin());
    return e;
}

void ws_parser::reset()
{
    input_.clear();
    header_parser_.reset();
    assembler_.reset();
    inflator_.reset();
    discard_queued_files();
    message_queue_.clear();
    control_queue_.clear();
    error_              = false;
    error_code_         = 1002;
    header_complete_    = false;
    frame_payload_left_ = 0;
    frame_is_control_   = false;
    msg_deflated_       = false;
    frame_fed_          = 0;
    ctl_payload_.clear();
    frame_mask_offset_ = 0;
    next_seq_id_       = 0;
}

bool ws_parser::assembler_sink(const unsigned char *out, size_t n, void *ud)
{
    return static_cast<message_assembler *>(ud)->push_payload(out, n);
}

bool ws_parser::fail(unsigned short code)
{
    error_      = true;
    error_code_ = code;
    return false;
}

}// namespace ws
}// namespace http
