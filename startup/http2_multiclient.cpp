// 简易 HTTP/2 压测客户端：明文 h2c 直连（先发表节串 PRI * HTTP/2.0），仅依赖 asio，不链框架源码。
// 请求头用 HPACK 字面量（never indexed）编码，不做 Huffman；
// 响应头块不整体解码，只看首字节：0x88 是静态表第 8 项 :status 200（本框架就这么编码）。
//
// 编译（仓库根目录执行，产物落 bin/）：
//   g++ -std=c++20 -O0 -g -DASIO_STANDALONE -Iasio startup/http2_multiclient.cpp -o bin/http2_multiclient -pthread
//
// 用法：
//   ./bin/http2_multiclient --host 127.0.0.1 --port 80 --conns 1 --streams 2000 --inflight 100 \
//       --path /admin/isloginjson --authority www.hggq.com --deadline 60000 --settings-timeout 5000
//
// 每条连接常驻 inflight 条在途流，一条流收到 END_STREAM 就补开一条，直到开完 --streams 条；
// 在途上限取 --inflight 与服务端 SETTINGS_MAX_CONCURRENT_STREAMS 的较小值。
// --settings-timeout 是握手阶段超时：连上后这么多毫秒没收到服务端 SETTINGS 就判该连接失败并快速收尾，
// 不必干等总 --deadline（默认 5000ms，传 0 关闭、只受 --deadline 约束）。
// 想压多条连接传 --conns N，或并跑多个本程序进程。
// POST 正文模式（测 multipart 上传这类要靠 DATA 帧切片的请求）：
//   ./bin/http2_multiclient --port 18890 --path /tfilepost --method POST \
//       --content-type 'multipart/form-data; boundary=XXXX' --body-file /abs/body.bin \
//       --frame-size 997 --dump-body --conns 1 --streams 3 --inflight 1
// --frame-size 决定每个 DATA 帧的负载字节数（服务端 SETTINGS_MAX_FRAME_SIZE 会再钳一次），
// 传小值就能把分隔符、字段名切成跨帧。
// 正文按发送窗口（连接级和流级各自一份）分批发，窗口空了就等服务端 WINDOW_UPDATE 续发，
// 所以正文没有长度上限，多条在途流也能各发各的；代价是 --deadline 要按上传带宽留够。
// 想精确控制"刷盘缝"落在正文哪个偏移，用 --inflight 1 配 --frame-size：
// 每帧一次解析，累计到货字节数就是 frame_size 的整数倍（减去 HEADERS 不占正文）。
// --dump-body 把每条完成流的响应体打出来（单条最多 8192 字节），配合 grep 判字段值。
// 输出一律用 endl 冲刷：stdout 重定向到文件是块缓冲，不冲刷则进程挂住时一行都看不到。
//
// 输出行：START / SETTINGS-TIMEOUT(握手超时，仅发生时) / PROGRESS(每秒每连接) / CONN(每连接汇总)
//         / BODY(响应体形状分布) / BODY-DUMP(--dump-body 时每条完成流的响应体)
//         / LATE(HEADERS/DATA 落在已关闭或本端没开过的流上，仅 late>0 时出现)
//         / STUCK(每条没回完的流收到了什么，仅 stuck>0 时出现)
//         / SUMMARY / REQUESTS path=… count=…（与服务端命中日志对账取这条 count）/ VERDICT PASS|FAIL。
// 退出码：0 = stuck、rst、late 全零且无连接中途断；1 = 连不上；2 = 参数非法；
// 3 = 有流没回完、被 RST、收到迟到帧或连接中断（含握手超时）。GOAWAY 不算失败（正常收尾服务端就发它），
// 被它覆盖的流计入 never_served，恒等式：completed + rst + never_served + stuck = opened。

#include <asio.hpp>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr const char *h2_preface     = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
constexpr std::size_t h2_preface_len = 24;

enum h2_frame_type : unsigned char
{
    FR_DATA          = 0x0,
    FR_HEADERS       = 0x1,
    FR_PUSH_PROMISE  = 0x2,
    FR_RST_STREAM    = 0x3,
    FR_SETTINGS      = 0x4,
    FR_PING          = 0x6,
    FR_GOAWAY        = 0x7,
    FR_WINDOW_UPDATE = 0x8,
    FR_CONTINUATION  = 0xA
};
constexpr unsigned char FR_FLAG_END_STREAM  = 0x1;
constexpr unsigned char FR_FLAG_ACK         = 0x1;
constexpr unsigned char FR_FLAG_END_HEADERS = 0x4;

// 连接级接收窗口目标值。初值恒为 65535（RFC 9113 §6.9.1），SETTINGS 改不了，只能用流 0 的 WINDOW_UPDATE 抬。
constexpr unsigned int h2_conn_window_target = 16777215;

// --dump-body 时单条流最多留这么多响应体字节；判 multipart 时正文可能整段回显，
// 所以上限要盖得住一条 4 万余字的上传，否则判据只能看到截断的前缀。
constexpr std::size_t body_dump_cap = 131072;

struct options
{
    std::string host               = "127.0.0.1";
    std::string port               = "80";
    std::string path               = "/admin/isloginjson";
    std::string authority          = "127.0.0.1";
    unsigned int conns             = 1;
    unsigned int streams           = 2000;
    unsigned int inflight          = 100;
    unsigned long long deadline_ms = 60000;
    // 握手超时：连上后等服务端 SETTINGS 的最长毫秒数；0 = 不单独限制，交给总 --deadline
    unsigned int settings_timeout_ms = 5000;
    // POST 正文：--body-file 非空即每条流都发这份正文，按 --frame-size 切成多个 DATA 帧
    std::string method       = "GET";
    std::string content_type;
    std::string body_file;
    unsigned int frame_size  = 16384;
    bool dump_body           = false;
    // POST 正文默认带 Content-Length；传 --send-cl 0 就不带，用来看服务端怎么收尾这条流
    bool send_cl = true;
    std::string body;// main() 读进来的正文内容
};

unsigned int be_to_uint(const unsigned char *p)
{
    return (static_cast<unsigned int>(p[0]) << 24) | (static_cast<unsigned int>(p[1]) << 16) |
           (static_cast<unsigned int>(p[2]) << 8) | static_cast<unsigned int>(p[3]);
}

std::vector<unsigned char> uint_to_be(unsigned int v)
{
    std::vector<unsigned char> out(4);
    out[0] = (v >> 24) & 0xFF;
    out[1] = (v >> 16) & 0xFF;
    out[2] = (v >> 8) & 0xFF;
    out[3] = v & 0xFF;
    return out;
}

std::vector<unsigned char> build_frame(unsigned char type, unsigned char flags, unsigned int stream_id, const std::vector<unsigned char> &payload)
{
    std::vector<unsigned char> frame(9 + payload.size());
    frame[0]                       = (payload.size() >> 16) & 0xFF;
    frame[1]                       = (payload.size() >> 8) & 0xFF;
    frame[2]                       = payload.size() & 0xFF;
    frame[3]                       = type;
    frame[4]                       = flags;
    std::vector<unsigned char> sid = uint_to_be(stream_id & 0x7FFFFFFF);
    std::memcpy(&frame[5], sid.data(), 4);
    std::memcpy(&frame[9], payload.data(), payload.size());
    return frame;
}

// 请求头块。索引取自 vendor/httpserver/include/http2_frame.h 里那张 http2_header_static_table
// （服务端解码用的就是它；下标与 RFC 7541 附录 A 并不一致：那里 content-type 是 24，这里是 31，
// 表里多了 accept-ranges/allow/cache-control 等项，所以本端必须按框架的表取号）：
// 2=:method GET、3=:method POST、6=:scheme http 用纯索引；
// :authority、:path、content-type、content-length 用 never-indexed 字面量。
// 值长度只用 1 字节，所以 path/authority/content-type 各自限制在 126 字节内；高位不置 1 表示未做 Huffman。
//
// 服务端读名字索引只用低 4 位前缀（http2_parse.cpp 的 0001xxxx 分支：`& 0x0F` 加续字节），
// 而 RFC 7541 里这一位是 7 位前缀，所以 15 及以上的号必须走续字节，不能塞进一个字节。
void push_never_indexed(std::vector<unsigned char> &out, unsigned int name_index)
{
    if (name_index < 15)
    {
        out.push_back(static_cast<unsigned char>(0x10 | name_index));
        return;
    }
    out.push_back(0x1F);
    name_index -= 15;
    while (name_index >= 128)
    {
        out.push_back(static_cast<unsigned char>(0x80 | (name_index & 0x7F)));
        name_index >>= 7;
    }
    out.push_back(static_cast<unsigned char>(name_index));
}

std::vector<unsigned char> make_request_header_block(const options &opt)
{
    std::vector<unsigned char> block;
    if (opt.method == "POST")
    {
        block.push_back(0x83);// :method POST
    }
    else
    {
        block.push_back(0x82);// :method GET
    }
    block.push_back(0x86);// :scheme http
    block.push_back(0x11);// :authority，never indexed，名字取静态表 1
    block.push_back(static_cast<unsigned char>(opt.authority.size()));
    block.insert(block.end(), opt.authority.begin(), opt.authority.end());
    block.push_back(0x14);// :path，never indexed，名字取静态表 4
    block.push_back(static_cast<unsigned char>(opt.path.size()));
    block.insert(block.end(), opt.path.begin(), opt.path.end());
    if (!opt.content_type.empty())
    {
        push_never_indexed(block, 31);// content-type，框架静态表 31
        block.push_back(static_cast<unsigned char>(opt.content_type.size()));
        block.insert(block.end(), opt.content_type.begin(), opt.content_type.end());
    }
    if (!opt.body.empty() && opt.send_cl)
    {
        std::string cl = std::to_string(opt.body.size());
        push_never_indexed(block, 28);// content-length，框架静态表 28
        block.push_back(static_cast<unsigned char>(cl.size()));
        block.insert(block.end(), cl.begin(), cl.end());
    }
    return block;
}

// 响应体里可能有非打印字节（正文被当成字段值回显时就会），打之前转义成 \xHH，
// 让 BODY-DUMP 恒是一行纯 ASCII，判据才能按行取。bytes= 仍是转义前的真实长度。
std::string escape_body(const std::string &in)
{
    static const char hex[] = "0123456789abcdef";
    std::string out;
    out.reserve(in.size());
    for (unsigned char b : in)
    {
        if (b >= 0x20 && b < 0x7F)
        {
            out.push_back(static_cast<char>(b));
        }
        else
        {
            out.push_back('\\');
            out.push_back('x');
            out.push_back(hex[b >> 4]);
            out.push_back(hex[b & 0x0F]);
        }
    }
    return out;
}

std::vector<unsigned char> make_window_update(unsigned int stream_id, unsigned int increment)
{
    std::vector<unsigned char> payload = uint_to_be(increment);
    return build_frame(FR_WINDOW_UPDATE, 0, stream_id, payload);
}

struct h2conn
{
    h2conn(asio::io_context &io, const options &opt, unsigned int id) : sock(io), opt(opt), id(id) {}

    asio::ip::tcp::socket sock;
    const options &opt;
    unsigned int id;

    unsigned int inflight_limit = 100;
    // 服务端 SETTINGS_MAX_FRAME_SIZE(0x5)：本端发 DATA 时按它再钳一次；缺省按 RFC 允许的下限 16384
    unsigned int max_frame_size = 16384;
    // 发送窗口：连接级初值恒 65535（SETTINGS 改不了，只能靠对端 WINDOW_UPDATE 抬）；
    // 流级初值 = 对端 SETTINGS_INITIAL_WINDOW_SIZE(0x4)，服务端改这个值时要在已开流上按差量调整。
    long long conn_send_window = 65535;
    long long stream_send_init = 65535;
    std::map<unsigned int, long long> stream_send_window;
    // 窗口不够时排在这里的正文还没发完，收到 WINDOW_UPDATE 或 SETTINGS 后续发
    struct pending_body
    {
        unsigned int sid  = 0;
        std::size_t offset = 0;
    };
    std::deque<pending_body> send_queue;

    // 服务端 SETTINGS 到货前不开流：在途上限要以服务端广告的 MAX_CONCURRENT_STREAMS 为准，开超了会被 RST
    bool server_settings_seen     = false;
    unsigned int next_stream_id   = 1;
    unsigned int opened           = 0;
    unsigned int completed        = 0;
    unsigned int got_rst          = 0;
    unsigned int status_200       = 0;
    unsigned int status_other     = 0;
    unsigned int never_served     = 0;
    unsigned long long data_bytes = 0;
    // 发送侧读数：正文实际写出去多少字节、收到多少条 WINDOW_UPDATE（连接级/流级各一）。
    // 卡在窗口上时（正文没发完又没收到响应）只有这组读数能分清是本端停手还是对端不回。
    unsigned long long body_sent    = 0;
    unsigned int wu_conn            = 0;
    unsigned int wu_stream          = 0;
    // 流已关闭但正文还没发完的字节数（服务端提前 END_STREAM 或 RST 才有值）
    unsigned long long body_undelivered = 0;
    bool got_goaway               = false;
    // GOAWAY 读数：错误码、最后一个已处理流号、附带调试文本（框架会把错码文本塞在 payload 尾部）
    unsigned int goaway_code      = 0;
    unsigned int goaway_last_sid  = 0;
    std::string goaway_debug;
    bool broken                   = false;
    std::string broken_msg;

    std::map<unsigned int, std::chrono::steady_clock::time_point> live_streams;

    // 每条流的到货明细，流关闭后也保留：卡在 deadline 时要能报出该流收到过什么
    struct stream_seen
    {
        unsigned int headers     = 0;
        unsigned int data_frames = 0;
        unsigned int data_len    = 0;
        unsigned int last_flags  = 0;
        bool end_stream          = false;
        std::string body;// 仅 --dump-body 时填充，最多 body_dump_cap 字节
    };
    std::map<unsigned int, stream_seen> stream_detail;

    // 迟到帧：HEADERS/DATA 落在已关闭的流、或本端从没开过的流号上，单独计数
    unsigned int late_frames = 0;
    std::vector<std::string> late_detail;
    // 连接建立时刻，迟到帧要能报出发生在第几秒、开过第几条流之后
    std::chrono::steady_clock::time_point conn_t0 = std::chrono::steady_clock::now();

    std::deque<unsigned char> inbox;
    std::vector<unsigned char> rbuf = std::vector<unsigned char>(65536);

    // 同步写：本工具单线程驱动 io_context，读回调里也可能回帧（PING/WINDOW_UPDATE），
    // 不存在并发；发送阻塞时最坏由 --deadline 兜底
    void send_raw(std::vector<unsigned char> &&bytes)
    {
        if (broken)
        {
            return;
        }
        asio::error_code ec;
        asio::write(sock, asio::buffer(bytes.data(), bytes.size()), ec);
        if (ec)
        {
            broken     = true;
            broken_msg = "write: " + ec.message();
        }
    }

    void start()
    {
        std::vector<unsigned char> out;
        out.insert(out.end(), h2_preface, h2_preface + h2_preface_len);
        // SETTINGS_INITIAL_WINDOW_SIZE(0x4)=16M：流级接收窗口从默认 64K 抬高，压大响应时不被自己的窗口卡住
        std::vector<unsigned char> setting_payload = {0x00, 0x04, 0x00, 0xFF, 0xFF, 0xFF};
        std::vector<unsigned char> setting         = build_frame(FR_SETTINGS, 0, 0, setting_payload);
        out.insert(out.end(), setting.begin(), setting.end());
        std::vector<unsigned char> wu = make_window_update(0, h2_conn_window_target - 65535);
        out.insert(out.end(), wu.begin(), wu.end());
        send_raw(std::move(out));
        arm_read();
        // 开流由主循环驱动：每轮都调 open_more()，服务端 SETTINGS 一到就补开
    }

    void open_more()
    {
        while (!broken && server_settings_seen && !got_goaway && opened < opt.streams &&
               live_streams.size() < inflight_limit)
        {
            unsigned int sid = next_stream_id;
            next_stream_id += 2;
            std::vector<unsigned char> block = make_request_header_block(opt);
            if (opt.body.empty())
            {
                // 无正文：HEADERS 自带 END_STREAM 就是完整请求
                send_raw(build_frame(FR_HEADERS, FR_FLAG_END_HEADERS | FR_FLAG_END_STREAM, sid, block));
            }
            else
            {
                send_raw(build_frame(FR_HEADERS, FR_FLAG_END_HEADERS, sid, block));
                if (broken)
                {
                    return;
                }
                stream_send_window[sid] = stream_send_init;
                send_queue.push_back(pending_body{sid, 0});
            }
            if (broken)
            {
                return;
            }
            live_streams[sid] = std::chrono::steady_clock::now();
            opened++;
        }
        flush_send();
    }

    // 按发送窗口把排队的正文一段段发出去：连接级窗口空了整体停手，
    // 某条流自己的窗口空了只跳过它，别的流照发。窗口回来时由 WINDOW_UPDATE/SETTINGS 再调这里。
    void flush_send()
    {
        while (!broken && !send_queue.empty())
        {
            bool progress = false;
            for (auto iter = send_queue.begin(); iter != send_queue.end();)
            {
                if (conn_send_window <= 0)
                {
                    break;
                }
                auto witer = stream_send_window.find(iter->sid);
                if (witer == stream_send_window.end() || witer->second <= 0)
                {
                    ++iter;
                    continue;
                }
                unsigned int chunk = std::min(opt.frame_size, max_frame_size);
                std::size_t left   = opt.body.size() - iter->offset;
                std::size_t allow  = static_cast<std::size_t>(std::min<long long>(
                    conn_send_window, std::min<long long>(witer->second, chunk)));
                unsigned int n = static_cast<unsigned int>(std::min(allow, left));
                if (n == 0)
                {
                    ++iter;
                    continue;
                }
                std::vector<unsigned char> payload(opt.body.begin() + iter->offset,
                                                   opt.body.begin() + iter->offset + n);
                unsigned char flags = (iter->offset + n >= opt.body.size()) ? FR_FLAG_END_STREAM : 0;
                send_raw(build_frame(FR_DATA, flags, iter->sid, payload));
                if (broken)
                {
                    return;
                }
                conn_send_window -= n;
                witer->second -= n;
                body_sent += n;
                iter->offset += n;
                progress      = true;
                if (iter->offset >= opt.body.size())
                {
                    stream_send_window.erase(iter->sid);
                    iter = send_queue.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
            if (!progress)
            {
                return;
            }
        }
    }

    void close_stream(unsigned int sid, bool reset)
    {
        auto iter = live_streams.find(sid);
        if (iter == live_streams.end())
        {
            return;
        }
        live_streams.erase(iter);
        // 流已经关了（服务端提前回完、或被 RST），正文没发完的那部分不能再发：
        // 往已关闭的流上写 DATA 是协议违例。这种流单独计数，否则判据会把"半截请求"当成上传成功。
        stream_send_window.erase(sid);
        for (auto qiter = send_queue.begin(); qiter != send_queue.end();)
        {
            if (qiter->sid == sid)
            {
                body_undelivered += opt.body.size() - qiter->offset;
                qiter            = send_queue.erase(qiter);
            }
            else
            {
                ++qiter;
            }
        }
        if (reset)
        {
            got_rst++;
        }
        else
        {
            completed++;
            if (opt.dump_body)
            {
                auto d = stream_detail.find(sid);
                std::string view = (d == stream_detail.end()) ? std::string() : d->second.body;
                std::cout << "BODY-DUMP conn=" << id << " sid=" << sid << " bytes=" << view.size()
                          << " total=" << ((d == stream_detail.end()) ? 0u : d->second.data_len)
                          << " body=" << escape_body(view) << std::endl;
            }
        }
    }

    void arm_read()
    {
        if (broken)
        {
            return;
        }
        sock.async_read_some(
            asio::buffer(rbuf.data(), rbuf.size()),
            [this](const asio::error_code &ec, std::size_t n)
            {
                if (ec)
                {
                    if (ec != asio::error::operation_aborted)
                    {
                        broken     = true;
                        broken_msg = "read: " + ec.message();
                    }
                    return;
                }
                inbox.insert(inbox.end(), rbuf.begin(), rbuf.begin() + n);
                drain_frames();
                arm_read();
            });
    }

    void drain_frames()
    {
        while (inbox.size() >= 9)
        {
            unsigned int length = (static_cast<unsigned int>(inbox[0]) << 16) | (static_cast<unsigned int>(inbox[1]) << 8) |
                                  static_cast<unsigned int>(inbox[2]);
            unsigned char type  = inbox[3];
            unsigned char flags = inbox[4];
            // inbox 是 deque，逐字节取流号：不能对 &inbox[5] 按 4 字节连读，
            // 相邻下标可能跨内存块，越界读到下一块的堆元数据
            unsigned int sid = (static_cast<unsigned int>(inbox[5]) << 24) |
                               (static_cast<unsigned int>(inbox[6]) << 16) |
                               (static_cast<unsigned int>(inbox[7]) << 8) |
                               static_cast<unsigned int>(inbox[8]);
            sid &= 0x7FFFFFFF;
            if (inbox.size() < 9 + length)
            {
                return;
            }
            std::vector<unsigned char> payload(inbox.begin() + 9, inbox.begin() + 9 + length);
            inbox.erase(inbox.begin(), inbox.begin() + 9 + length);
            handle_frame(type, flags, sid, payload, length);
        }
    }

    void note_late(unsigned char type, unsigned int sid, unsigned int length, unsigned int flags)
    {
        if (live_streams.count(sid) != 0)
        {
            return;
        }
        late_frames++;
        if (late_detail.size() >= 10)
        {
            return;
        }
        // 同时报序号和连接内毫秒时刻，便于判断迟到帧是序号触发还是时间触发
        unsigned long long at_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - conn_t0).count();
        std::string why = "frame=" + std::to_string(type) + " sid=" + std::to_string(sid) + " len=" +
                          std::to_string(length) + " flags=" + std::to_string(flags) +
                          " at_opened=" + std::to_string(opened) + " at_ms=" + std::to_string(at_ms);
        auto d = stream_detail.find(sid);
        if (d == stream_detail.end())
        {
            why += " prev=never_opened";
        }
        else
        {
            why += " prev_frames=" + std::to_string(d->second.data_frames) + " prev_len=" +
                   std::to_string(d->second.data_len) + " prev_end=" + (d->second.end_stream ? "1" : "0");
        }
        late_detail.push_back(std::move(why));
    }

    void handle_frame(unsigned char type, unsigned char flags, unsigned int sid, const std::vector<unsigned char> &payload, unsigned int length)
    {
        switch (type)
        {
        case FR_SETTINGS:
            if ((flags & FR_FLAG_ACK) == 0)
            {
                // 只认带参数的那一条；纯 ACK 没有载荷，收到它说明带参数的那条早已到过
                server_settings_seen = true;
                std::vector<unsigned char> empty;
                send_raw(build_frame(FR_SETTINGS, FR_FLAG_ACK, 0, empty));
            }
            for (unsigned int i = 0; i + 6 <= length; i += 6)
            {
                unsigned int ident = (static_cast<unsigned int>(payload[i]) << 8) | payload[i + 1];
                unsigned int value = be_to_uint(&payload[i + 2]);
                if (ident == 0x3 && value > 0)
                {
                    // SETTINGS_MAX_CONCURRENT_STREAMS(0x3)：在途上限取配置值与服务端广告值的较小者
                    inflight_limit = std::min(opt.inflight, value);
                }
                else if (ident == 0x4)
                {
                    // SETTINGS_INITIAL_WINDOW_SIZE(0x4)：改的是流级发送窗口的基准，
                    // 已开的流要按差量一起调（RFC 9113 §6.9.2），连接级窗口不受 SETTINGS 影响
                    long long delta = static_cast<long long>(value) - stream_send_init;
                    stream_send_init = static_cast<long long>(value);
                    for (auto &sw : stream_send_window)
                    {
                        sw.second += delta;
                    }
                }
                else if (ident == 0x5 && value > 0)
                {
                    // SETTINGS_MAX_FRAME_SIZE(0x5)：单个 DATA 帧负载的硬上限
                    max_frame_size = value;
                }
            }
            flush_send();
            break;

        case FR_HEADERS:
        {
            note_late(type, sid, length, flags);
            stream_seen &sd = stream_detail[sid];
            sd.headers++;
            sd.last_flags = flags;
            // 不解 HPACK：首字节 0x88 即本框架写出的 :status 200 静态索引
            if (length > 0 && payload[0] == 0x88)
            {
                status_200++;
            }
            else
            {
                status_other++;
            }
            if (flags & FR_FLAG_END_STREAM)
            {
                sd.end_stream = true;
                close_stream(sid, false);
            }
            break;
        }
        case FR_DATA:
        {
            note_late(type, sid, length, flags);
            data_bytes += length;
            stream_seen &sd = stream_detail[sid];
            sd.data_frames++;
            sd.data_len += length;
            sd.last_flags = flags;
            if (opt.dump_body && sd.body.size() < body_dump_cap)
            {
                std::size_t take = std::min(body_dump_cap - sd.body.size(), payload.size());
                sd.body.append(reinterpret_cast<const char *>(payload.data()), take);
            }
            if (length > 0)
            {
                // 消费多少还多少：流级和连接级窗口各补一次
                send_raw(make_window_update(sid, length));
                send_raw(make_window_update(0, length));
            }
            if (flags & FR_FLAG_END_STREAM)
            {
                sd.end_stream = true;
                close_stream(sid, false);
            }
            break;
        }
        case FR_RST_STREAM:
            // RST 是幂等的关闭信号，重复或迟到的 RST 不计 late，close_stream 找不到流会直接忽略
            close_stream(sid, true);
            break;
        case FR_GOAWAY:
            got_goaway = true;
            if (length >= 8)
            {
                // 把服务端给的所以然留在读数里：光知道"连接被拆"判不出是哪类协议违例
                goaway_code = be_to_uint(payload.data() + 4);
                goaway_last_sid = be_to_uint(payload.data()) & 0x7FFFFFFFu;
                if (length > 8)
                {
                    goaway_debug.assign(reinterpret_cast<const char *>(payload.data() + 8), length - 8);
                }
                std::cout << "GOAWAY conn=" << id << " last_sid=" << goaway_last_sid
                          << " code=0x" << std::hex << goaway_code << std::dec
                          << (goaway_debug.empty() ? "" : " debug=\"" + goaway_debug + "\"") << std::endl;
                // last_sid 最高位是保留位，比较前清掉（RFC 9113 §6.8）；
                // 流号大于它的在途流服务端承诺不会处理，计入 never_served
                for (auto iter = live_streams.begin(); iter != live_streams.end();)
                {
                    if (iter->first > goaway_last_sid)
                    {
                        iter = live_streams.erase(iter);
                        never_served++;
                    }
                    else
                    {
                        ++iter;
                    }
                }
            }
            break;
        case FR_PING:
            if ((flags & FR_FLAG_ACK) == 0)
            {
                send_raw(build_frame(FR_PING, FR_FLAG_ACK, 0, payload));
            }
            break;
        case FR_WINDOW_UPDATE:
        {
            if (length < 4)
            {
                break;
            }
            // 最高位是保留位，比较前清掉（RFC 9113 §6.9）
            unsigned int increment = be_to_uint(payload.data()) & 0x7FFFFFFFu;
            if (sid == 0)
            {
                conn_send_window += increment;
                wu_conn++;
            }
            else
            {
                auto witer = stream_send_window.find(sid);
                if (witer != stream_send_window.end())
                {
                    witer->second += increment;
                }
                wu_stream++;
            }
            flush_send();
            break;
        }
        case FR_PUSH_PROMISE:
        case FR_CONTINUATION:
        default:
            break;
        }
    }

    // 握手超时检查：返回 true 表示本次调用刚判定超时（供主循环打一条即时日志）。
    // 每条连接按自己的 conn_t0 独立计时；超时直接置 broken，finished() 随即为真、读数算 broken 失败。
    bool check_settings_timeout()
    {
        if (server_settings_seen || broken || opt.settings_timeout_ms == 0)
        {
            return false;
        }
        unsigned long long ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - conn_t0)
                .count();
        if (ms < opt.settings_timeout_ms)
        {
            return false;
        }
        broken     = true;
        broken_msg = "settings timeout: peer SETTINGS not received within " +
                     std::to_string(opt.settings_timeout_ms) + " ms";
        return true;
    }

    bool finished() const
    {
        // GOAWAY 只表示不再收新流（opened 再也到不了 streams），在途的流仍要等它回完；
        // 回不完就是 stuck，由 --deadline 兜住
        if (broken)
        {
            return true;
        }
        return (opened >= opt.streams || got_goaway) && live_streams.empty();
    }
};

// 严格解析十进制无符号整数：空串、负号、非数字、超上限一律拒绝
bool parse_uint(const std::string &s, unsigned long long max, unsigned long long &out)
{
    if (s.empty())
    {
        return false;
    }
    unsigned long long v = 0;
    for (char ch : s)
    {
        if (ch < '0' || ch > '9')
        {
            return false;
        }
        v = v * 10 + static_cast<unsigned long long>(ch - '0');
        if (v > max)
        {
            return false;
        }
    }
    out = v;
    return true;
}

bool parse_options(int argc, char **argv, options &opt)
{
    // argv[0] 之后必须是 key value 成对出现，落单的 key 直接判非法
    if (argc % 2 == 0)
    {
        std::cout << "BAD-ARG dangling argument" << std::endl;
        return false;
    }
    for (int i = 1; i + 1 < argc; i += 2)
    {
        std::string key        = argv[i];
        std::string value      = argv[i + 1];
        unsigned long long num = 0;
        if (key == "--host")
            opt.host = value;
        else if (key == "--port")
            opt.port = value;
        else if (key == "--path")
            opt.path = value;
        else if (key == "--authority")
            opt.authority = value;
        else if (key == "--conns" && parse_uint(value, 0xFFFFFFFFULL, num))
            opt.conns = static_cast<unsigned int>(num);
        else if (key == "--streams" && parse_uint(value, 0xFFFFFFFFULL, num))
            opt.streams = static_cast<unsigned int>(num);
        else if (key == "--inflight" && parse_uint(value, 0xFFFFFFFFULL, num))
            opt.inflight = static_cast<unsigned int>(num);
        else if (key == "--deadline" && parse_uint(value, 0xFFFFFFFFFFFFFFFFULL, num))
            opt.deadline_ms = num;
        else if (key == "--settings-timeout" && parse_uint(value, 0xFFFFFFFFULL, num))
            opt.settings_timeout_ms = static_cast<unsigned int>(num);
        else if (key == "--method")
            opt.method = value;
        else if (key == "--content-type")
            opt.content_type = value;
        else if (key == "--body-file")
            opt.body_file = value;
        else if (key == "--frame-size" && parse_uint(value, 0xFFFFFFFFULL, num) && num > 0)
            opt.frame_size = static_cast<unsigned int>(num);
        else if (key == "--dump-body" && parse_uint(value, 1, num))
            opt.dump_body = (num == 1);
        else if (key == "--send-cl" && parse_uint(value, 1, num))
            opt.send_cl = (num == 1);
        else
        {
            std::cout << "BAD-ARG " << key << " " << value << std::endl;
            return false;
        }
    }
    if (opt.conns == 0 || opt.streams == 0 || opt.inflight == 0)
    {
        std::cout << "BAD-ARG conns/streams/inflight must be > 0" << std::endl;
        return false;
    }
    if (opt.path.size() < 1 || opt.path.size() > 126 || opt.path[0] != '/')
    {
        std::cout << "BAD-ARG path must start with / and be 1..126 bytes (single-byte HPACK length)" << std::endl;
        return false;
    }
    if (opt.authority.size() < 1 || opt.authority.size() > 126)
    {
        std::cout << "BAD-ARG authority must be 1..126 bytes" << std::endl;
        return false;
    }
    if (opt.method != "GET" && opt.method != "POST")
    {
        std::cout << "BAD-ARG method must be GET or POST" << std::endl;
        return false;
    }
    if (opt.content_type.size() > 126)
    {
        std::cout << "BAD-ARG content-type must be 0..126 bytes (single-byte HPACK length)" << std::endl;
        return false;
    }
    if (!opt.body_file.empty() && opt.method != "POST")
    {
        std::cout << "BAD-ARG body-file needs --method POST" << std::endl;
        return false;
    }
    if (opt.method == "POST" && !opt.body_file.empty())
    {
        if (opt.content_type.empty())
        {
            std::cout << "BAD-ARG POST body needs --content-type" << std::endl;
            return false;
        }
    }
    return true;
}
}// namespace

int main(int argc, char **argv)
{
    options opt;
    if (!parse_options(argc, argv, opt))
    {
        return 2;
    }
    if (!opt.body_file.empty())
    {
        std::ifstream in(opt.body_file, std::ios::binary);
        if (!in)
        {
            std::cout << "FATAL open body-file " << opt.body_file << std::endl;
            return 1;
        }
        opt.body.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        std::cout << "BODY loaded bytes=" << opt.body.size() << " frame_size=" << opt.frame_size
                  << " path=" << opt.path << std::endl;
    }

    asio::io_context io;
    asio::error_code ec;
    asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve(opt.host, opt.port, ec);
    if (ec || endpoints.empty())
    {
        std::cout << "FATAL resolve " << ec.message() << std::endl;
        return 1;
    }

    std::vector<std::unique_ptr<h2conn>> conns;
    for (unsigned int i = 0; i < opt.conns; i++)
    {
        conns.push_back(std::make_unique<h2conn>(io, opt, i + 1));
        conns.back()->sock.connect(*endpoints.begin(), ec);
        if (ec)
        {
            std::cout << "FATAL connect " << ec.message() << std::endl;
            return 1;
        }
        conns.back()->sock.set_option(asio::ip::tcp::no_delay(true), ec);
        conns.back()->start();
    }

    auto begin_time = std::chrono::steady_clock::now();
    auto last_beat  = begin_time;
    std::cout << "START conns=" << opt.conns << " streams_per_conn=" << opt.streams
              << " inflight=" << opt.inflight << " settings_timeout_ms=" << opt.settings_timeout_ms
              << " method=" << opt.method << " body_bytes=" << opt.body.size()
              << " frame_size=" << opt.frame_size << " dump_body=" << (opt.dump_body ? 1 : 0)
              << " path=" << opt.path << " host=" << opt.authority
              << " target=" << opt.host << ":" << opt.port << std::endl;

    while (true)
    {
        io.run_for(std::chrono::milliseconds(20));
        for (auto &c : conns)
        {
            c->open_more();
            if (c->check_settings_timeout())
            {
                std::cout << "SETTINGS-TIMEOUT conn=" << c->id
                          << " timeout_ms=" << opt.settings_timeout_ms << std::endl;
            }
        }

        auto now                   = std::chrono::steady_clock::now();
        unsigned long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin_time).count();
        if (now - last_beat >= std::chrono::seconds(1))
        {
            last_beat = now;
            for (auto &c : conns)
            {
                std::cout << "PROGRESS conn=" << c->id << " opened=" << c->opened << " completed=" << c->completed
                          << " in_flight=" << c->live_streams.size() << " rst=" << c->got_rst
                          << " never_served=" << c->never_served << " t_ms=" << elapsed << std::endl;
            }
        }

        bool all_done = true;
        for (auto &c : conns)
        {
            if (!c->finished())
                all_done = false;
        }
        if (all_done)
        {
            break;
        }
        if (opt.deadline_ms > 0 && elapsed >= opt.deadline_ms)
        {
            std::cout << "DEADLINE reached t_ms=" << elapsed << std::endl;
            break;
        }
    }

    unsigned int total_opened = 0, total_completed = 0, total_rst = 0, total_stuck = 0, total_never = 0;
    unsigned int total_200 = 0, total_other_status = 0, total_late = 0;
    unsigned long long total_data = 0;
    bool any_broken               = false;
    bool any_goaway               = false;
    for (auto &c : conns)
    {
        std::cout << "CONN id=" << c->id << " opened=" << c->opened << " completed=" << c->completed
                  << " status200=" << c->status_200 << " status_other=" << c->status_other
                  << " rst=" << c->got_rst << " never_served=" << c->never_served
                  << " stuck=" << c->live_streams.size() << " data_bytes=" << c->data_bytes
                  << (opt.body.empty() ? "" : " body_sent=" + std::to_string(c->body_sent) + "/" +
                                                    std::to_string(opt.body.size()) +
                                                    " wu_conn=" + std::to_string(c->wu_conn) +
                                                    " wu_stream=" + std::to_string(c->wu_stream))
                  << (c->body_undelivered > 0 ? " body_undelivered=" + std::to_string(c->body_undelivered) : "")
                  << (c->broken ? " broken=\"" + c->broken_msg + "\"" : "")
                  << (c->got_goaway ? " goaway=1" : "") << std::endl;
        // 响应体形状分布：定长 JSON 只该有一种 (帧数,字节数) 组合；
        // 出现第二种说明某条流被写了两次体，或体被拆成多帧
        std::map<std::pair<unsigned int, unsigned int>, unsigned int> body_dist;
        for (auto &kv : c->stream_detail)
        {
            body_dist[std::make_pair(kv.second.data_frames, kv.second.data_len)]++;
        }
        for (auto &bd : body_dist)
        {
            std::cout << "BODY conn=" << c->id << " data_frames=" << bd.first.first
                      << " data_len=" << bd.first.second << " streams=" << bd.second << std::endl;
        }
        if (c->late_frames > 0)
        {
            std::cout << "LATE conn=" << c->id << " frames=" << c->late_frames << std::endl;
            for (auto &s : c->late_detail)
            {
                std::cout << "LATE conn=" << c->id << " " << s << std::endl;
            }
        }
        // 每条没回完的流报告到货形状：
        //   headers=1 data_frames=0                        有响应头、DATA 从没来
        //   headers=1 data_frames=1 data_len=定长 end=0    体写完了但没给 END_STREAM
        //   headers=0                                      服务端完全没处理过这条流
        for (auto &kv : c->live_streams)
        {
            auto d                   = c->stream_detail.find(kv.first);
            unsigned int got_headers = (d == c->stream_detail.end() ? 0 : d->second.headers);
            unsigned int got_data    = (d == c->stream_detail.end() ? 0 : d->second.data_frames);
            unsigned int got_len     = (d == c->stream_detail.end() ? 0 : d->second.data_len);
            unsigned int last_flags  = (d == c->stream_detail.end() ? 0 : d->second.last_flags);
            bool ended               = (d != c->stream_detail.end() && d->second.end_stream);
            std::cout << "STUCK conn=" << c->id << " sid=" << kv.first << " headers=" << got_headers
                      << " data_frames=" << got_data << " data_len=" << got_len
                      << " end_stream=" << (ended ? 1 : 0) << " last_flags=0x" << std::hex << last_flags
                      << std::dec << std::endl;
        }
        total_opened += c->opened;
        total_completed += c->completed;
        total_rst += c->got_rst;
        total_stuck += c->live_streams.size();
        total_never += c->never_served;
        total_200 += c->status_200;
        total_other_status += c->status_other;
        total_data += c->data_bytes;
        total_late += c->late_frames;
        if (c->broken)
        {
            any_broken = true;
        }
        if (c->got_goaway)
        {
            any_goaway = true;
        }
    }
    unsigned long long wall_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin_time).count();
    std::cout << "SUMMARY opened=" << total_opened << " completed=" << total_completed
              << " status200=" << total_200 << " status_other=" << total_other_status
              << " rst=" << total_rst << " never_served=" << total_never << " stuck=" << total_stuck
              << " late=" << total_late << " data_bytes=" << total_data << " wall_ms=" << wall_ms << std::endl;
    // 单独一行计数，用于与服务端日志的路由命中次数对账
    std::cout << "REQUESTS path=" << opt.path << " count=" << total_opened << std::endl;

    // 任一不为零即本趟没跑干净：stuck=收了流没回完、rst=被 RST、late=收到迟到帧、broken=连接中断。
    // opened=0（例如对端一直不发 SETTINGS，deadline 空转）同样算失败，不能把"什么都没测到"当 PASS。
    // GOAWAY 不算失败：正常收尾就会发，被它覆盖的流计入 never_served，账面仍然配平。
    bool clean = (total_stuck == 0 && total_rst == 0 && total_late == 0 && !any_broken && total_opened > 0);
    std::cout << "VERDICT " << (clean ? "PASS" : "FAIL")
              << " stuck=" << total_stuck << " rst=" << total_rst << " late=" << total_late
              << " broken=" << (any_broken ? 1 : 0)
              << " goaway=" << (any_goaway ? 1 : 0) << std::endl;

    for (auto &c : conns)
    {
        asio::error_code ignore;
        c->sock.close(ignore);
    }
    return clean ? 0 : 3;
}
