//
// ws_wire.h — WebSocket 帧编解码
//
// 提供帧头解析、消息聚合、出站帧编码等基础能力。
// 不依赖 asio，可在任意线程调用，调用方负责并发安全。
//

#ifndef PROJECT_WS_WIRE_H
#define PROJECT_WS_WIRE_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

namespace http
{
namespace ws
{

// 限额默认值（实际运行限额来自 common/cost_define.h 编译期常量，经 set_limits 注入）
constexpr uint64_t kMaxFramePayload          = (1ull << 63) - 1;   // RFC 理论上限
constexpr uint64_t kDefaultSpillThreshold    = 2ull * 1024 * 1024;  // 超过 2MB 落盘
constexpr size_t   kDefaultBufferLimit       = 64ull * 1024 * 1024; // 暂存区上限 64MB
constexpr uint64_t kDefaultMaxFramePayload   = 16ull * 1024 * 1024; // 单帧上限 16MB
constexpr uint64_t kDefaultMaxMessagePayload = 16ull * 1024 * 1024; // 单条消息上限 16MB

// 不变式：落盘阈值 < 单帧上限 <= 消息上限
static_assert(kDefaultSpillThreshold < kDefaultMaxFramePayload &&
              kDefaultMaxFramePayload <= kDefaultMaxMessagePayload,
              "ws limits invariant: spill_threshold < max_frame <= max_message");

// WebSocket 帧操作码
enum class opcode : unsigned char
{
    continuation = 0x00,
    text         = 0x01,
    binary       = 0x02,
    close        = 0x08,
    ping         = 0x09,
    pong         = 0x0A,
};

// 临时文件（RAII）。unlink 模式下关闭即删；named 模式保留文件名供业务层访问。
class temp_file_handle
{
  public:
    temp_file_handle();
    ~temp_file_handle() = default;

    // 创建并打开临时文件。unlink_after_create=true 时创建后立即 unlink。
    bool open(const std::string &dir, bool unlink_after_create = true);

    bool write(const void *data, size_t len);

    void close();

    bool               is_open() const { return fp_ != nullptr; }
    uint64_t           size() const { return size_; }
    std::FILE         *get() const { return fp_.get(); }
    const std::string &filename() const { return filename_; }

  private:
    std::unique_ptr<std::FILE, void (*)(std::FILE *)> fp_;
    uint64_t    size_ = 0;
    std::string filename_;
};

// 已解析的帧头
struct frame_header
{
    bool          fin         = false;
    unsigned char rsv         = 0;          // RSV1|RSV2|RSV3 低 3 位
    opcode        op          = opcode::continuation;
    bool          masked      = false;
    uint64_t      payload_len = 0;
    unsigned char mask_key[4] = {0, 0, 0, 0};
    size_t        header_len  = 0;          // 帧头总字节数
};

// 增量帧头解析：feed 喂入数据，返回已消费字节数；complete 为 true 时 header() 可用。
class header_parser
{
  public:
    void reset();

    size_t feed(const unsigned char *data, size_t len);

    bool               complete() const { return complete_; }
    bool               error() const { return error_; }
    const frame_header &header() const { return hdr_; }

  private:
    size_t needed() const; // 还需多少字节才能凑齐帧头

    std::string  buf_;
    frame_header hdr_;
    bool         complete_ = false;
    bool         error_    = false;
};

// 带容量上限的暂存缓冲区。append 返回超出上限的字节数（0 表示全部接受）。
class input_buffer
{
  public:
    explicit input_buffer(size_t limit = kDefaultBufferLimit);

    size_t append(const unsigned char *data, size_t len);

    void consume(size_t n); // 丢弃前 n 字节

    const std::string &data() const { return buf_; }
    unsigned char     *writable_data() { return reinterpret_cast<unsigned char *>(buf_.data()); }
    size_t             size() const { return buf_.size(); }
    bool               empty() const { return buf_.empty(); }
    void               clear() { buf_.clear(); }

  private:
    std::string buf_;
    size_t      limit_;
};

// UTF-8 流式校验器（RFC 3629）：逐块喂入，跨读、跨分片保持状态。
// 拒绝：非法前导/截断序列、过长编码、代理对 U+D800-DFFF、>U+10FFFF、
// 消息末尾残留半截序列（由调用方在消息结束处查 complete()）。
class utf8_stream_checker
{
  public:
    bool feed(const unsigned char *data, size_t len);
    // 消息结束时不得有未完成的多字节序列
    bool complete() const { return pending_ == 0; }
    void reset();

  private:
    unsigned char pending_ = 0; // 当前序列还差几个后续字节
    unsigned char total_   = 0; // 当前序列总长（过长编码下限用）
    unsigned int  cp_      = 0; // 累积码点
};

// 一次性校验（close 原因等短缓冲）
inline bool utf8_valid(std::string_view s)
{
    utf8_stream_checker c;
    return c.feed(reinterpret_cast<const unsigned char *>(s.data()), s.size()) && c.complete();
}

// permessage-deflate（RFC 7692）入站解压：raw deflate（窗口 15）流式 inflate。
// 协商参数固定 client_no_context_takeover——每条消息是独立 deflate 流：
// 消息末帧喂入时内部追加 0x00 0x00 0xff 0xff 同步标记强制冲净，随后 inflateReset。
// sink 返回 false（聚合器超限等）立即停止喂入；zlib 报错置 failed()，调用方回 Close(1007)。
class permessage_inflate
{
  public:
    permessage_inflate();
    ~permessage_inflate();
    permessage_inflate(const permessage_inflate &)            = delete;
    permessage_inflate &operator=(const permessage_inflate &) = delete;

    // 喂入本帧的压缩载荷字节；msg_fin=true 表示本帧是消息末帧。
    bool feed(const unsigned char *data, size_t len, bool msg_fin,
              bool (*sink)(const unsigned char *out, size_t n, void *ud), void *ud);

    bool failed() const { return failed_; }
    void reset();

  private:
    bool pump(bool (*sink)(const unsigned char *out, size_t n, void *ud), void *ud, int flush);

    struct inflate_state_t;
    inflate_state_t *st_;
    bool             failed_ = false;
    bool             ended_  = false;// 本条流已见 BFINAL（Z_STREAM_END），末帧无需再补同步标记
};

// 单条消息独立 raw deflate（出站，配合 server_no_context_takeover）。
// 返回 false 表示 zlib 失败或未变小；out 为压缩字节（已按惯例剥离尾部 0x00 0x00 0xff 0xff）。
bool raw_deflate_once(std::string_view in, std::string &out);

// 消息聚合器：把多个分片帧合并成一条完整消息，超过阈值时落盘。
// 只处理数据帧，控制帧由调用方自行处理（不要喂入本类）。
class message_assembler
{
  public:
    explicit message_assembler(uint64_t spill_threshold = kDefaultSpillThreshold,
                               uint64_t max_total       = kDefaultMaxMessagePayload);
    // 未交付给业务的消息（收一半断连）落盘文件在这里回收；已交付的不碰
    ~message_assembler();

    void set_limits(uint64_t spill_threshold, uint64_t max_total);

    void reset();

    // 开始一帧。仅接受数据帧，返回 false 表示协议错误。
    bool begin_frame(bool fin, opcode op, unsigned char rsv);

    // 喂入本帧载荷（已解掩码），可多次调用。
    bool push_payload(const unsigned char *data, size_t len);

    // 帧载荷喂完后调用，fin=1 时消息完成。
    bool end_frame();

    bool          error() const { return error_; }
    bool          error_too_big() const { return too_big_; }
    // 文本消息载荷不是合法 UTF-8（RFC 6455 §5.5/7.1.7），调用方应回 Close(1007)
    bool          error_invalid_utf8() const { return invalid_utf8_; }
    // 落盘等内部失败（非对端过错），调用方应回 Close(1011)
    bool          error_internal() const { return internal_; }
    bool          finished() const { return finished_; }
    bool          in_message() const { return in_message_; }
    opcode        message_opcode() const { return opcode_; }
    unsigned char message_rsv() const { return rsv_; }
    uint64_t      message_size() const { return total_size_; }
    bool          is_spilled() const { return spilled_; }

    // 取出内存消息（仅未落盘时）
    std::string take_payload() { return std::move(mem_payload_); }

    // 落盘消息的文件名（仅落盘时），文件已 flush，业务层负责读取和删除
    std::string message_filename() const;

    temp_file_handle &file() { return file_; }

    // 消息取走后重置，准备接收下一条
    void consume_message();

  private:
    bool            in_message_  = false;
    bool            spilled_     = false;
    bool            error_       = false;
    bool            too_big_     = false;
    bool            internal_    = false;
    bool            invalid_utf8_ = false;
    bool            finished_    = false;
    bool            current_fin_ = false;
    opcode          opcode_      = opcode::continuation;
    unsigned char   rsv_         = 0;
    uint64_t        total_size_  = 0;
    uint64_t        spill_threshold_;
    uint64_t        max_total_   = kDefaultMaxMessagePayload;
    std::string     mem_payload_;
    temp_file_handle file_;
    utf8_stream_checker utf8_;   // 仅 text 消息启用，跨分片保持状态
};

// 出站帧编码（服务端→客户端，不掩码）；rsv 为帧头 3 位 RSV 字段值（RSV1=4，即比特 0x40，
// permessage-deflate 压缩标记；RSV2=2/RSV3=1 保留必须为 0）
std::string serialize_frame(opcode op, std::string_view payload, bool fin = true,
                            unsigned char rsv = 0);

std::string make_close_frame(uint16_t code = 1000, std::string_view reason = {});

std::string make_ping_frame(std::string_view payload = "ping");

std::string make_pong_frame(std::string_view payload);

}// namespace ws
}// namespace http

#endif// PROJECT_WS_WIRE_H
