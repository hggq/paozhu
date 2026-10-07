#include "ws_wire.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <random>
#include <zlib.h>
#include "server_localvar.h"

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#define WS_OPEN ::_open
#define WS_CLOSE ::_close
#define WS_UNLINK ::_unlink
#define WS_FDOPEN ::_fdopen
#define WS_O_CREAT _O_CREAT
#define WS_O_EXCL _O_EXCL
#define WS_O_RDWR _O_RDWR
#define WS_O_BINARY _O_BINARY
#else
#include <fcntl.h>
#include <unistd.h>
#define WS_OPEN ::open
#define WS_CLOSE ::close
#define WS_UNLINK ::unlink
#define WS_FDOPEN ::fdopen
#define WS_O_CREAT O_CREAT
#define WS_O_EXCL O_EXCL
#define WS_O_RDWR O_RDWR
#define WS_O_BINARY 0
#endif

namespace http
{
namespace ws
{

// temp_file_handle

temp_file_handle::temp_file_handle() : fp_(nullptr, [](std::FILE *f)
                                           {
    if (f)
        std::fclose(f); })
{
}

bool temp_file_handle::open(const std::string &dir, bool unlink_after_create)
{
    if (dir.empty())
        return false;

    thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dis;

    char tmpl[256];
    std::snprintf(tmpl, sizeof(tmpl), "%s/ws_%llx_%llx", dir.c_str(), (unsigned long long)time(nullptr), (unsigned long long)dis(rng));

    int fd = WS_OPEN(tmpl, WS_O_CREAT | WS_O_EXCL | WS_O_RDWR | WS_O_BINARY, 0600);
    if (fd < 0)
        return false;

    if (unlink_after_create)
    {
        WS_UNLINK(tmpl);
        filename_.clear();
    }
    else
    {
        filename_ = tmpl;
    }

    std::FILE *f = WS_FDOPEN(fd, "wb+");
    if (!f)
    {
        WS_CLOSE(fd);
        if (!unlink_after_create)
            WS_UNLINK(tmpl);
        return false;
    }
    fp_.reset(f);
    size_ = 0;
    return true;
}

bool temp_file_handle::write(const void *data, size_t len)
{
    if (!fp_)
        return false;
    if (std::fwrite(data, 1, len, fp_.get()) != len)
        return false;
    size_ += len;
    return true;
}

void temp_file_handle::close()
{
    fp_.reset();
    size_ = 0;
}

// header_parser

void header_parser::reset()
{
    buf_.clear();
    complete_ = false;
    error_    = false;
}

size_t header_parser::feed(const unsigned char *data, size_t len)
{
    if (complete_ || error_ || len == 0)
        return 0;

    size_t total = 0;
    while (total < len && !complete_ && !error_)
    {
        size_t need = needed();
        if (need == 0)
            break;
        size_t take = std::min(len - total, need);
        buf_.append(reinterpret_cast<const char *>(data + total), take);
        total += take;

        if (buf_.size() >= 2)
        {
            unsigned char b0 = static_cast<unsigned char>(buf_[0]);
            unsigned char b1 = static_cast<unsigned char>(buf_[1]);
            hdr_.fin         = (b0 >> 7) & 1;
            hdr_.rsv         = (b0 >> 4) & 0x07;
            hdr_.op          = static_cast<opcode>(b0 & 0x0F);
            hdr_.masked      = (b1 >> 7) & 1;

            // 校验操作码合法性
            unsigned char opv = b0 & 0x0F;
            bool valid        = (opv <= 0x02) || (opv >= 0x08 && opv <= 0x0A);
            if (!valid)
            {
                error_ = true;
                break;
            }

            // 计算扩展长度和掩码长度
            unsigned char plen = b1 & 0x7F;
            size_t ext         = (plen < 126) ? 0 : (plen == 126 ? 2 : 8);
            size_t mask_len    = hdr_.masked ? 4 : 0;
            size_t full        = 2 + ext + mask_len;

            if (buf_.size() >= full)
            {
                if (ext == 0)
                {
                    hdr_.payload_len = plen;
                }
                else if (ext == 2)
                {
                    hdr_.payload_len =
                        (static_cast<uint64_t>(static_cast<unsigned char>(buf_[2])) << 8) |
                        static_cast<unsigned char>(buf_[3]);
                    // RFC 6455 5.2：长度必须用最紧凑编码
                    if (hdr_.payload_len < 126)
                    {
                        error_ = true;
                        break;
                    }
                }
                else if (ext == 8)
                {
                    if (static_cast<unsigned char>(buf_[2]) & 0x80)
                    {
                        // 最高位必须为 0
                        error_ = true;
                        break;
                    }
                    uint64_t v = 0;
                    for (int i = 0; i < 8; ++i)
                        v = (v << 8) | static_cast<uint64_t>(static_cast<unsigned char>(buf_[2 + i]));
                    hdr_.payload_len = v;
                    if (hdr_.payload_len <= 65535)
                    {
                        error_ = true;
                        break;
                    }
                }

                if (hdr_.masked)
                {
                    size_t mk        = 2 + ext;
                    hdr_.mask_key[0] = static_cast<unsigned char>(buf_[mk]);
                    hdr_.mask_key[1] = static_cast<unsigned char>(buf_[mk + 1]);
                    hdr_.mask_key[2] = static_cast<unsigned char>(buf_[mk + 2]);
                    hdr_.mask_key[3] = static_cast<unsigned char>(buf_[mk + 3]);
                }
                hdr_.header_len = full;
                complete_       = true;
            }
        }
    }
    return total;
}

size_t header_parser::needed() const
{
    size_t n = buf_.size();
    if (n < 2)
        return 2 - n;
    unsigned char b1   = static_cast<unsigned char>(buf_[1]);
    unsigned char plen = b1 & 0x7F;
    size_t ext         = (plen < 126) ? 0 : (plen == 126 ? 2 : 8);
    size_t mask_len    = ((b1 >> 7) & 1) ? 4 : 0;
    size_t full        = 2 + ext + mask_len;
    return (n >= full) ? 0 : (full - n);
}

// input_buffer

input_buffer::input_buffer(size_t limit) : limit_(limit) {}

size_t input_buffer::append(const unsigned char *data, size_t len)
{
    if (len == 0)
        return 0;
    size_t avail = (buf_.size() + len > limit_) ? (limit_ - buf_.size()) : len;
    if (avail < len)
    {
        buf_.append(reinterpret_cast<const char *>(data), avail);
        return len - avail;
    }
    buf_.append(reinterpret_cast<const char *>(data), len);
    return 0;
}

void input_buffer::consume(size_t n)
{
    if (n >= buf_.size())
        buf_.clear();
    else
        buf_.erase(0, n);
}

// message_assembler

// UTF-8 流式校验器（RFC 3629）

void utf8_stream_checker::reset()
{
    pending_ = 0;
    total_   = 0;
    cp_      = 0;
}

bool utf8_stream_checker::feed(const unsigned char *data, size_t len)
{
    size_t i = 0;
    while (i < len)
    {
        if (pending_ == 0)
        {
            // ASCII 快路径：纯文本消息最常见的形态
            while (i < len && data[i] < 0x80)
                ++i;
            if (i == len)
                return true;
            unsigned char b = data[i];
            if (b >= 0xC2 && b <= 0xDF)
            {
                cp_      = b & 0x1F;
                pending_ = 1;
            }
            else if (b >= 0xE0 && b <= 0xEF)
            {
                cp_      = b & 0x0F;
                pending_ = 2;
            }
            else if (b >= 0xF0 && b <= 0xF4)
            {
                cp_      = b & 0x07;
                pending_ = 3;
            }
            else
            {
                return false;// 后续字节作前导、0xC0/0xC1 恒过长、0xF5-0xFF 超范围
            }
            total_ = static_cast<unsigned char>(pending_ + 1);
            ++i;
            continue;
        }
        unsigned char b = data[i++];
        if ((b & 0xC0) != 0x80)
            return false;// 后续字节缺失，序列在上一个字节就断了
        cp_ = (cp_ << 6) | (b & 0x3F);
        if (--pending_ == 0)
        {
            unsigned int minv = (total_ == 2) ? 0x80u : (total_ == 3 ? 0x800u : 0x10000u);
            if (cp_ < minv)
                return false;// 过长编码
            if (cp_ >= 0xD800u && cp_ <= 0xDFFFu)
                return false;// UTF-8 禁止代理对
            if (cp_ > 0x10FFFFu)
                return false;
        }
    }
    return true;
}

message_assembler::message_assembler(uint64_t spill_threshold, uint64_t max_total)
    : spill_threshold_(spill_threshold), max_total_(max_total)
{
}

void message_assembler::set_limits(uint64_t spill_threshold, uint64_t max_total)
{
    spill_threshold_ = spill_threshold;
    max_total_       = max_total;
}

message_assembler::~message_assembler()
{
    // 句柄仍开着 ⟹ 消息从未交付（consume_message 已 close）；回收该文件
    if (file_.is_open())
    {
        std::string name = file_.filename();
        file_.close();
        if (!name.empty())
            std::remove(name.c_str());
    }
}

void message_assembler::reset()
{
    in_message_ = false;
    total_size_ = 0;
    spilled_    = false;
    opcode_     = opcode::continuation;
    rsv_        = 0;
    mem_payload_.clear();
    if (file_.is_open())
    {
        std::string name = file_.filename();
        file_.close();
        if (!name.empty())
            std::remove(name.c_str());
    }
    error_        = false;
    too_big_      = false;
    internal_     = false;
    invalid_utf8_ = false;
    utf8_.reset();
    finished_ = false;
}

bool message_assembler::begin_frame(bool fin, opcode op, unsigned char rsv)
{
    error_    = false;
    finished_ = false;

    // 控制帧不能进入消息聚合
    if (op == opcode::close || op == opcode::ping || op == opcode::pong)
    {
        error_ = true;
        return false;
    }

    if (!in_message_)
    {
        // 首帧不能是 continuation
        if (op == opcode::continuation)
        {
            error_ = true;
            return false;
        }
        opcode_     = op;
        rsv_        = rsv;
        in_message_ = true;
        if (op == opcode::text)
        {
            utf8_.reset();
            invalid_utf8_ = false;
        }
    }
    else
    {
        // 消息中间帧必须是 continuation
        if (op != opcode::continuation)
        {
            error_ = true;
            return false;
        }
        rsv_ |= rsv;
    }
    current_fin_ = fin;
    return true;
}

bool message_assembler::push_payload(const unsigned char *data, size_t len)
{
    if (error_)
        return false;
    total_size_ += len;
    // 超过消息上限
    if (total_size_ > max_total_)
    {
        error_   = true;
        too_big_ = true;
        return false;
    }
    // text 消息载荷必须是合法 UTF-8（含落盘路径：字节同样先过校验）
    if (opcode_ == opcode::text && !utf8_.feed(data, len))
    {
        error_        = true;
        invalid_utf8_ = true;
        return false;
    }
    // 超过落盘阈值，把内存中已有数据写入临时文件
    if (!spilled_ && total_size_ > spill_threshold_)
    {
        // 落点回退链：temp_path → ./temp → "."
        bool opened              = false;
        const std::string &gpath = get_server_global_var().temp_path;
        if (!gpath.empty())
            opened = file_.open(gpath, /*unlink_after_create=*/false);
        if (!opened)
            opened = file_.open("./temp", /*unlink_after_create=*/false);
        if (!opened)
            opened = file_.open(".", /*unlink_after_create=*/false);
        if (!opened)
        {
            error_    = true;
            internal_ = true;
            return false;
        }
        if (!mem_payload_.empty())
        {
            if (!file_.write(mem_payload_.data(), mem_payload_.size()))
            {
                error_    = true;
                internal_ = true;// 落盘写失败是本地内部过错 → Close 1011，不是协议错误 1002
                return false;
            }
            mem_payload_.clear();
        }
        spilled_ = true;
    }

    if (spilled_)
    {
        if (!file_.write(data, len))
        {
            error_    = true;
            internal_ = true;
            return false;
        }
    }
    else
    {
        mem_payload_.append(reinterpret_cast<const char *>(data), len);
    }
    return true;
}

bool message_assembler::end_frame()
{
    if (error_)
        return false;
    if (current_fin_)
    {
        // 消息末尾不得残留半截多字节序列
        if (opcode_ == opcode::text && !utf8_.complete())
        {
            error_        = true;
            invalid_utf8_ = true;
            return false;
        }
        finished_ = true;
        if (spilled_)
            std::fflush(file_.get());
    }
    return true;
}

std::string message_assembler::message_filename() const
{
    return spilled_ ? file_.filename() : std::string{};
}

void message_assembler::consume_message()
{
    in_message_ = false;
    total_size_ = 0;
    spilled_    = false;
    opcode_     = opcode::continuation;
    rsv_        = 0;
    mem_payload_.clear();
    file_.close();
    finished_ = false;
    too_big_  = false;
    utf8_.reset();
    invalid_utf8_ = false;
}

// permessage-deflate（RFC 7692）

struct permessage_inflate::inflate_state_t
{
    z_stream zs;
    bool inited = false;
};

permessage_inflate::permessage_inflate() : st_(new inflate_state_t())
{
    std::memset(&st_->zs, 0, sizeof(z_stream));
}

permessage_inflate::~permessage_inflate()
{
    if (st_ != nullptr)
    {
        if (st_->inited)
            inflateEnd(&st_->zs);
        delete st_;
    }
}

// 泵干当前可解输出；sink 返回 false 时透传失败（不置 failed_，由调用方查聚合器）
bool permessage_inflate::pump(bool (*sink)(const unsigned char *out, size_t n, void *ud),
                              void *ud,
                              int flush)
{
    z_stream &zs = st_->zs;
    unsigned char ob[16384];
    while (true)
    {
        uInt in_before = zs.avail_in;
        zs.next_out    = ob;
        zs.avail_out   = sizeof(ob);
        int ret        = inflate(&zs, flush);
        size_t made    = sizeof(ob) - zs.avail_out;
        if (made > 0 && !sink(ob, made, ud))
            return false;
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
        {
            failed_ = true;
            return false;
        }
        if (ret == Z_STREAM_END)
        {
            ended_ = true;
            break;
        }
        if (zs.avail_in == in_before && made == 0)
        {
            if (in_before > 0)
            {
                // 输入未耗尽却无法推进：坏流
                failed_ = true;
                return false;
            }
            break;// 输入耗尽，等更多字节
        }
        if (zs.avail_in == 0 && zs.avail_out > 0)
            break;// 本轮已耗尽输入且输出缓冲未填满，无待发数据
    }
    return true;
}

bool permessage_inflate::feed(const unsigned char *data, size_t len, bool msg_fin, bool (*sink)(const unsigned char *out, size_t n, void *ud), void *ud)
{
    if (failed_)
        return false;
    if (!st_->inited)
    {
        // 惰性初始化：未协商压缩的连接不背 inflate 窗口内存
        st_->inited = inflateInit2(&st_->zs, -15) == Z_OK;// raw deflate，窗口 15（不协商缩减）
        if (!st_->inited)
        {
            failed_ = true;
            return false;
        }
    }
    st_->zs.next_in  = const_cast<Bytef *>(data);
    st_->zs.avail_in = static_cast<uInt>(len);
    if (!pump(sink, ud, Z_NO_FLUSH))
        return false;

    if (msg_fin)
    {
        // 对端可能剥离了尾部同步标记（流停在未冲净的块上）：补标记再冲一次。
        // 已见 Z_STREAM_END 的自终结流不能续喂，否则 inflate 报流错误。
        if (!ended_)
        {
            static unsigned char sync[4] = {0x00, 0x00, 0xff, 0xff};
            st_->zs.next_in              = sync;
            st_->zs.avail_in             = 4;
            if (!pump(sink, ud, Z_SYNC_FLUSH))
                return false;
        }
        reset();// no-context：每条消息独立流
    }
    return true;
}

void permessage_inflate::reset()
{
    if (st_->inited)
        inflateReset(&st_->zs);
    failed_ = false;
    ended_  = false;
}

bool raw_deflate_once(std::string_view in, std::string &out)
{
    z_stream zs;
    std::memset(&zs, 0, sizeof(z_stream));
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        return false;

    std::string buf;
    buf.reserve(in.size() / 2 + 64);
    unsigned char ob[16384];
    zs.next_in  = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(in.data()));
    zs.avail_in = static_cast<uInt>(in.size());
    bool err    = false;
    do
    {
        zs.next_out  = ob;
        zs.avail_out = sizeof(ob);
        int ret      = deflate(&zs, Z_FINISH);
        buf.append(reinterpret_cast<char *>(ob), sizeof(ob) - zs.avail_out);
        if (ret == Z_STREAM_ERROR)
        {
            err = true;
            break;
        }
    } while (zs.avail_out == 0);
    deflateEnd(&zs);
    if (err || buf.size() >= in.size())
        return false;
    // 按惯例剥离尾部空块标记
    if (buf.size() >= 4 &&
        std::memcmp(buf.data() + buf.size() - 4, "\x00\x00\xff\xff", 4) == 0)
        buf.resize(buf.size() - 4);
    out = std::move(buf);
    return true;
}

// 出站帧编码

std::string serialize_frame(opcode op, std::string_view payload, bool fin, unsigned char rsv)
{
    std::string out;
    unsigned char b0 = (fin ? 0x80 : 0x00) | ((rsv & 0x07) << 4) | static_cast<unsigned char>(op);
    out.push_back(static_cast<char>(b0));

    uint64_t len = payload.size();
    if (len <= 125)
    {
        out.push_back(static_cast<char>(len));
    }
    else if (len <= 65535)
    {
        out.push_back(static_cast<char>(126));
        out.push_back(static_cast<char>((len >> 8) & 0xFF));
        out.push_back(static_cast<char>(len & 0xFF));
    }
    else
    {
        out.push_back(static_cast<char>(127));
        for (int i = 7; i >= 0; --i)
            out.push_back(static_cast<char>((len >> (8 * i)) & 0xFF));
    }
    out.append(payload);
    return out;
}

std::string make_close_frame(uint16_t code, std::string_view reason)
{
    std::string body;
    body.push_back(static_cast<char>((code >> 8) & 0xFF));
    body.push_back(static_cast<char>(code & 0xFF));
    body.append(reason);
    return serialize_frame(opcode::close, body);
}

std::string make_ping_frame(std::string_view payload)
{
    return serialize_frame(opcode::ping, payload);
}

std::string make_pong_frame(std::string_view payload)
{
    return serialize_frame(opcode::pong, payload);
}

}// namespace ws
}// namespace http
