#include "http_socket.h"
#include "http2_frame.h"
#include "client_session.h"
#include "http2_parse.h"
#include "terminal_color.h"
#include "http2_define.h"
#include "http2_huffman.h"
#include "https_brotli.h"
#include "unicode.h"
#include "directory_fun.h"
#include "http_header.h"
#include "clientdatacache.h"
#include "http2_ring_queue.h"
#include "http2_send_queue.h"
#include "debug_log.h"
#include "base64.h"

namespace http
{

// Build RFC 7540 Section 3.2.1 HTTP2-Settings header value.
// Payload must match the SETTINGS frame sent by co_send_setting().
static std::string make_h2c_switch101_response()
{
    // SETTINGS frame payload used in co_send_setting():
    // SETTINGS_MAX_CONCURRENT_STREAMS = 100
    // SETTINGS_INITIAL_WINDOW_SIZE    = 16777215
    const unsigned char settings_payload[] = {
        0x00,
        0x03,
        0x00,
        0x00,
        0x00,
        0x64,
        0x00,
        0x04,
        0x00,
        0xFF,
        0xFF,
        0xFF};

    std::string settings_value = http::base64_encode(
        reinterpret_cast<const char *>(settings_payload),
        sizeof(settings_payload),
        1);

    std::string response = "HTTP/1.1 101 Switching Protocols\r\n";
    response.append("Connection: Upgrade\r\n");
    response.append("Upgrade: h2c\r\n");
    response.append("HTTP2-Settings: ");
    response.append(settings_value);
    response.append("\r\n\r\n");
    return response;
}

client_session::client_session(asio::io_context &io_context) : strand_(asio::make_strand(io_context))
{
    auto &cc    = get_client_data_cache();
    _cache_data = cc.get_data_ptr();
}

client_session::~client_session()
{
    isclose = true;
    if (isssl)
    {
        if (sslsocket && sslsocket->lowest_layer().is_open())
        {
            sslsocket->lowest_layer().cancel(ec);
            sslsocket->lowest_layer().close(ec);
        }
    }
    else
    {
        if (socket && socket->is_open())
        {
            socket->cancel(ec);
            socket->close(ec);
        }
    }

    if (_cache_data != nullptr)
    {
        auto &cc = get_client_data_cache();
        cc.back_data_ptr(_cache_data);
    }

    if (http2_ring_queue)
    {
        auto &cc = get_http2_ring_queue_obj();
        cc.back_cache_ptr(std::move(http2_ring_queue));
        http2_ring_queue = nullptr;
    }
}
asio::awaitable<bool> client_session::read_some(unsigned int &readnum, std::string &log_item)
{
    auto self = shared_from_this();
    try
    {
        if (isclose || iserror)
        {
            co_return true;
        }
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                readnum = co_await sslsocket->async_read_some(asio::buffer(_cache_data, 4096), asio::redirect_error(asio::use_awaitable, ec));
            }
            else
            {
                isclose = true;
            }
        }
        else
        {
            if (socket->is_open())
            {
                readnum = co_await socket->async_read_some(asio::buffer(_cache_data, 4096), asio::redirect_error(asio::use_awaitable, ec));
            }
            else
            {
                isclose = true;
            }
        }

        if (ec)
        {
            DEBUG_LOG("read_some exception %s", ec.message().c_str());
            log_item.append(ec.message());
            isclose = true;
            iserror = true;
            co_return true;
        }
        if (isclose || iserror)
        {
            co_return true;
        }
        co_return false;
    }
    catch (const std::exception &e)
    {

        for (readnum = 0; readnum < 128; readnum++)
        {
            if (e.what()[readnum] == 0x00)
            {
                break;
            }
            _cache_data[readnum] = e.what()[readnum];
        }
        isclose = true;
        iserror = true;
        cancel();
        co_return true;
    }
    catch (...)
    {
        isclose = true;
        iserror = true;
        cancel();
        co_return true;
    }
    co_return false;
}

asio::awaitable<bool> client_session::read_first(unsigned int &readnum)
{
    auto self = shared_from_this();
    try
    {
        if (isclose || iserror)
        {
            co_return true;
        }
        if (isssl)
        {
            readnum = co_await sslsocket->async_read_some(asio::buffer(_cache_data, 4096), asio::redirect_error(asio::use_awaitable, ec));
        }
        else
        {
            readnum = co_await socket->async_read_some(asio::buffer(_cache_data, 4096), asio::redirect_error(asio::use_awaitable, ec));
        }

        if (ec)
        {
            DEBUG_LOG("read_some exception %s", ec.message().c_str());
            for (readnum = 0; readnum < 128; readnum++)
            {
                if (ec.message()[readnum] == 0x00)
                {
                    break;
                }
                _cache_data[readnum] = ec.message()[readnum];
            }
            isclose = true;
            iserror = true;
            co_return true;
        }
        if (isclose || iserror)
        {
            co_return true;
        }
        co_return false;
    }
    catch (const std::exception &e)
    {

        for (readnum = 0; readnum < 128; readnum++)
        {
            if (e.what()[readnum] == 0x00)
            {
                break;
            }
            _cache_data[readnum] = e.what()[readnum];
        }
        isclose = true;
        iserror = true;
        cancel();
        co_return true;
    }
    catch (...)
    {
        isclose = true;
        iserror = true;
        cancel();
        co_return true;
    }
    co_return false;
}

asio::awaitable<bool> client_session::read_at_least(unsigned int need, unsigned int &readnum, unsigned int limit)
{
    auto self = shared_from_this();
    try
    {
        if (limit > CACHE_DATA_LENGTH)
        {
            limit = CACHE_DATA_LENGTH;
        }
        if (need > limit)
        {
            need = limit;
        }
        if (readnum >= need)
        {
            co_return false;
        }

        // readnum 每轮至少推进 1 字节，最多迭代 CACHE_DATA_LENGTH 次，不会空转
        while (readnum < need)
        {
            if (isclose || iserror)
            {
                co_return true;
            }

            std::size_t got = 0;
            if (isssl)
            {
                if (sslsocket == nullptr || !sslsocket->lowest_layer().is_open())
                {
                    isclose = true;
                    co_return true;
                }
                got = co_await sslsocket->async_read_some(asio::buffer(_cache_data + readnum, limit - readnum), asio::redirect_error(asio::use_awaitable, ec));
            }
            else
            {
                if (socket == nullptr || !socket->is_open())
                {
                    isclose = true;
                    co_return true;
                }
                got = co_await socket->async_read_some(asio::buffer(_cache_data + readnum, limit - readnum), asio::redirect_error(asio::use_awaitable, ec));
            }

            if (ec)
            {
                DEBUG_LOG("read_at_least exception %s", ec.message().c_str());
                isclose = true;
                iserror = true;
                co_return true;
            }
            if (got == 0)
            {
                // 无错误却读回 0 字节，只可能是对端已关闭
                isclose = true;
                iserror = true;
                co_return true;
            }
            readnum += static_cast<unsigned int>(got);
        }
        co_return false;
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("read_at_least std::exception %s", e.what());
        isclose = true;
        iserror = true;
        cancel();
        co_return true;
    }
    catch (...)
    {
        isclose = true;
        iserror = true;
        cancel();
        co_return true;
    }
}

asio::awaitable<bool> client_session::read_socket(unsigned int &readnum, std::string &log_item)
{
    auto self = shared_from_this();
    try
    {
        if (isclose || iserror)
        {
            co_return true;
        }
        log_item.resize(4096);
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                readnum = co_await sslsocket->async_read_some(asio::buffer(log_item), asio::redirect_error(asio::use_awaitable, ec));
            }
            else
            {
                isclose = true;
            }
        }
        else
        {
            if (socket->is_open())
            {
                readnum = co_await socket->async_read_some(asio::buffer(log_item), asio::redirect_error(asio::use_awaitable, ec));
            }
            else
            {
                isclose = true;
            }
        }

        if (ec)
        {
            DEBUG_LOG("read_some exception %s", ec.message().c_str());
            log_item.append(ec.message());
            isclose = true;
            iserror = true;
            co_return true;
        }
        if (isclose || iserror)
        {
            co_return true;
        }
        log_item.resize(readnum);
        co_return false;
    }
    catch (const std::exception &e)
    {
        log_item.append(e.what());
        isclose = true;
        iserror = true;
        cancel();
        co_return true;
    }
    catch (...)
    {
        isclose = true;
        iserror = true;
        cancel();
        co_return true;
    }
    co_return false;
}

std::shared_ptr<client_session> client_session::get_ptr() { return shared_from_this(); }

unsigned int client_session::send_writer(const std::string &msg)
{
    if (isclose)
    {
        return 0;
    }
    try
    {
        unsigned int n = 0;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                n = asio::write(*sslsocket, asio::buffer(msg));
            }
            else
            {
                isclose = true;
                return 0;
            }
        }
        else
        {
            if (socket->is_open())
            {
                n = asio::write(*socket, asio::buffer(msg));
            }
            else
            {
                isclose = true;
                return 0;
            }
        }
        return n;
    }
    catch (std::exception &)
    {
        isclose = true;
        iserror = true;
        cancel();
        return 0;
    }
}

unsigned int client_session::send_writer(std::string_view msg)
{
    if (isclose)
    {
        return 0;
    }
    try
    {
        unsigned int n = 0;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                n = asio::write(*sslsocket, asio::buffer(msg));
            }
            else
            {
                isclose = true;
                return 0;
            }
        }
        else
        {
            if (socket->is_open())
            {
                n = asio::write(*socket, asio::buffer(msg));
            }
            else
            {
                isclose = true;
                return 0;
            }
        }
        return n;
    }
    catch (std::exception &)
    {
        isclose = true;
        iserror = true;
        cancel();
        return 0;
    }
}

bool client_session::isopensocket()
{

    try
    {
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (socket->is_open())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    catch (std::exception &)
    {
        return false;
    }
}

bool client_session::send_switch101()
{
    try
    {
        std::string tempswitch = make_h2c_switch101_response();
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                asio::write(*sslsocket, asio::buffer(tempswitch));
            }
            else
            {
                isclose = true;
                return false;
            }
        }
        else
        {
            if (socket->is_open())
            {
                asio::write(*socket, asio::buffer(tempswitch));
            }
            else
            {
                isclose = true;
                return false;
            }
        }
        return true;
    }
    catch (std::exception &)
    {
        isclose = true;
        iserror = true;
        cancel();
        return false;
    }
}

asio::awaitable<bool> client_session::co_send_switch101()
{
    if (isclose)
    {
        co_return false;
    }
    try
    {
        std::string tempswitch = make_h2c_switch101_response();

        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                co_await asio::async_write(*sslsocket, asio::buffer(tempswitch), asio::use_awaitable);
            }
            else
            {
                isclose = true;
                co_return false;
            }
        }
        else
        {
            if (socket->is_open())
            {
                co_await asio::async_write(*socket, asio::buffer(tempswitch), asio::use_awaitable);
            }
            else
            {
                isclose = true;
                co_return false;
            }
        }
        co_return true;
    }
    catch (...)
    {
        isclose = true;
        iserror = true;
        cancel();
    }
    co_return false;
}

asio::awaitable<void> client_session::co_send_setting()
{
    unsigned char _recvack[] = {0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x64, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xFF};
    http2_ring_queue->push(_recvack, 21);
    co_return;
}

bool client_session::http2_send_enddata(unsigned int s_stream_id)
{
    unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    _recvack[8]              = s_stream_id & 0xFF;
    s_stream_id              = s_stream_id >> 8;
    _recvack[7]              = s_stream_id & 0xFF;
    s_stream_id              = s_stream_id >> 8;
    _recvack[6]              = s_stream_id & 0xFF;
    s_stream_id              = s_stream_id >> 8;
    _recvack[5]              = s_stream_id & 0x7F;

    // 返回值交给调用点：END_STREAM 丢了这条流永远不结束，调用方必须重投。
    return http2_ring_queue->push(_recvack, 9);
}

void client_session::http2_send_rst_stream(unsigned int s_stream_id, unsigned int stream_error_code)
{
    unsigned char _recvack[] = {0x00, 0x00, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    _recvack[8]              = s_stream_id & 0xFF;
    s_stream_id              = s_stream_id >> 8;
    _recvack[7]              = s_stream_id & 0xFF;
    s_stream_id              = s_stream_id >> 8;
    _recvack[6]              = s_stream_id & 0xFF;
    s_stream_id              = s_stream_id >> 8;
    _recvack[5]              = s_stream_id & 0x7F;

    _recvack[12]      = stream_error_code & 0xFF;
    stream_error_code = stream_error_code >> 8;
    _recvack[11]      = stream_error_code & 0xFF;
    stream_error_code = stream_error_code >> 8;
    _recvack[10]      = stream_error_code & 0xFF;
    stream_error_code = stream_error_code >> 8;
    _recvack[9]       = stream_error_code & 0xFF;

    // 重投要把调用方变成阻塞/排队语义，而环满通常发生在连接正在收尾时；
    // 这里只记账并留日志，不改变调用形状。
    bool rst_pushed = http2_ring_queue->push(_recvack, 13);
    if (!rst_pushed)
    {
        http2_ring_queue_drop_count++;
        LOG_ERROR << " http2 control frame dropped, ring full, type:rst" << LOG_END;
    }
}
asio::awaitable<void> client_session::http2_send_ping()
{
    unsigned char _recvack[] =
        {0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    http2_ring_queue->push(_recvack, 17);

    co_return;
}

void client_session::send_ping()
{
    unsigned char _recvack[] =
        {0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    http2_ring_queue->push(_recvack, 17);
}

bool client_session::send_zero_data(unsigned int stream_id)
{
    // 必须是栈上缓冲。函数级 static 全进程只有一份，而发送线程有 2 个：
    // 一个线程刚写完 stream_id，另一个线程就把它覆盖，
    // 「空 DATA + END_STREAM」会被发到另一条流上（A 流永不结束、B 流被截断）。
    unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};

    _recvack[8] = stream_id & 0xFF;
    stream_id   = stream_id >> 8;
    _recvack[7] = stream_id & 0xFF;
    stream_id   = stream_id >> 8;
    _recvack[6] = stream_id & 0xFF;
    stream_id   = stream_id >> 8;
    _recvack[5] = stream_id & 0x7F;

    return http2_ring_queue->push(_recvack, 9);
}

void client_session::send_goaway(unsigned int last_stream_id, unsigned int error_code)
{
    // RFC 7540 §6.8 GOAWAY 帧
    //   帧头(9B): length=8, type=0x07, flags=0, stream_id=0
    //   载荷(8B): last_stream_id(4B) + error_code(4B), network byte order (big-endian)
    // 大端 = 高位字节在前，参考本文件 send_window_update_conn 的正确写法
    unsigned char _recvack[17] = {
        0x00, 0x00, 0x08,  // 长度 = 8
        0x07,               // GOAWAY 帧类型
        0x00,               // flags = 0
        0x00, 0x00, 0x00, 0x00,  // stream_id = 0
        0x00, 0x00, 0x00, 0x00,  // last_stream_id (big-endian 32bit)
        0x00, 0x00, 0x00, 0x00   // error_code (big-endian 32bit)
    };
    // 载荷从字节 9 起（0..8 是 9 字节帧头），下标照 send_window_update_conn 的口径来
    // last_stream_id: 从高字节到低字节（big-endian）
    _recvack[9]  = (last_stream_id >> 24) & 0xFF;
    _recvack[10] = (last_stream_id >> 16) & 0xFF;
    _recvack[11] = (last_stream_id >> 8)  & 0xFF;
    _recvack[12] =  last_stream_id        & 0xFF;
    // error_code: 同上
    _recvack[13] = (error_code >> 24) & 0xFF;
    _recvack[14] = (error_code >> 16) & 0xFF;
    _recvack[15] = (error_code >> 8)  & 0xFF;
    _recvack[16] =  error_code        & 0xFF;

    if (!http2_ring_queue->push(_recvack, 17))
    {
        http2_ring_queue_drop_count++;
        LOG_ERROR << " http2 control frame dropped, ring full, type:goway" << LOG_END;
    }
}

asio::awaitable<void> client_session::async_send_goway(unsigned int last_stream_id, unsigned int error_code)
{
    send_goaway(last_stream_id, error_code);
    co_return;
}

void client_session::send_recv_setting()
{
    unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00};
    http2_ring_queue->push(_recvack, 9);
}
void client_session::send_window_update_conn(unsigned int up_num)
{
    // 单帧：stream id = 0（连接级）。RFC 9113 §6.9.2：连接级窗口初值恒为 65535，
    // 且只能通过本帧改变，SETTINGS_INITIAL_WINDOW_SIZE 对它无效。
    // 增量必须是「本端真实消费掉的字节数」——每次整笔重发整个窗口目标值的话，
    // 对端连接窗口会线性增长并顶穿 2^31-1，对端只能按 FLOW_CONTROL_ERROR 断连。
    unsigned char _recvack[] = {0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    _recvack[12]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[11]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[10]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[9]              = up_num & 0xFF;

    // 丢了只是拖慢对端继续发 body（对端下次 DATA 前会等自己的窗口），不重试。
    if (!http2_ring_queue->push(_recvack, 13))
    {
        http2_ring_queue_drop_count++;
    }
}

void client_session::send_window_update_stream(unsigned int stmid, unsigned int up_num)
{
    // 单帧：stream id = stmid（流级）。只影响这一条流，不得顺带改动连接级窗口。
    unsigned char _recvack[] = {0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    _recvack[12]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[11]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[10]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[9]              = up_num & 0xFF;

    _recvack[8] = stmid & 0xFF;
    stmid       = stmid >> 8;
    _recvack[7] = stmid & 0xFF;
    stmid       = stmid >> 8;
    _recvack[6] = stmid & 0xFF;
    stmid       = stmid >> 8;
    // RFC 9113 §6.1：流标识符最高位是保留位，发送时必须为 0
    _recvack[5] = stmid & 0x7F;

    if (!http2_ring_queue->push(_recvack, 13))
    {
        http2_ring_queue_drop_count++;
    }
}

unsigned int client_session::send_writer(const unsigned char *buffer, unsigned int buffersize)
{
    if (isclose)
    {
        return 0;
    }

    try
    {
        unsigned int n = 0;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                n = asio::write(*sslsocket, asio::buffer(buffer, buffersize));
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if (socket->is_open())
            {
                n = asio::write(*socket, asio::buffer(buffer, buffersize));
            }
            else
            {
                return 0;
            }
        }
        return n;
    }
    catch (std::exception &)
    {
        isclose = true;
        iserror = true;
        cancel();
        return 0;
    }
}

void client_session::waituphttp2()
{
    try
    {
        std::unique_lock lk(waituphttp2_mutex);
        http2_need_wakeup = false;
        if (user_code_handler_call.size() > 0)
        {
            auto handle = std::move(user_code_handler_call.front());
            user_code_handler_call.pop_front();
            lk.unlock();
            asio::dispatch(strand_,
                           [handler = std::move(handle)]() mutable -> void
                           {
                               handler(1);
                           });

            DEBUG_LOG("peer_session user_code_handler_call return");
        }
        else
        {
            lk.unlock();
        }
    }
    catch (...)
    {
        DEBUG_LOG("peer_session user_code_handler_call error");
    }
}

void client_session::flush_parked_send()
{
    // 先立闸门再摘表：冲刷之后发送线程若还想挂起，会走「直接回池」分支，
    // 不会再留下一张永远没人来收的表。
    send_park_closed.store(true);

    std::list<std::shared_ptr<http2_send_data_t>> drained;
    auto &send_queue_obj = get_http2_send_queue();
    if (!send_queue_obj.detach_parked(this, drained))
    {
        return;
    }
    for (auto &sp : drained)
    {
        send_queue_obj.back_cache_ptr(sp);
    }
}

asio::awaitable<unsigned int> client_session::async_send_writer(const unsigned char *buffer, unsigned int buffersize)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    try
    {
        if (buffersize == 0)
        {
            co_return 0;
        }
        unsigned int n = 0;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                n = co_await asio::async_write(*sslsocket, asio::buffer(buffer, buffersize), asio::use_awaitable);
            }
            else
            {
                isclose = true;
            }
        }
        else
        {
            if (socket->is_open())
            {
                n = co_await asio::async_write(*socket, asio::buffer(buffer, buffersize), asio::use_awaitable);
            }
            else
            {
                isclose = true;
            }
        }
        co_return n;
    }
    catch (...)
    {
        isclose = true;
        iserror = true;
    }
    co_return 0;
}

asio::awaitable<unsigned int> client_session::async_send_writer(std::string_view msg)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    try
    {
        if (msg.size() == 0)
        {
            co_return 0;
        }
        unsigned int n = 0;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                n = co_await asio::async_write(*sslsocket, asio::buffer(msg), asio::use_awaitable);
            }
            else
            {
                isclose = true;
            }
        }
        else
        {
            if (socket->is_open())
            {
                n = co_await asio::async_write(*socket, asio::buffer(msg), asio::use_awaitable);
            }
            else
            {
                isclose = true;
            }
        }
        co_return n;
    }
    catch (...)
    {
        isclose = true;
        iserror = true;
        cancel();
    }
    co_return 0;
}

asio::awaitable<unsigned int> client_session::async_send_writer(const std::string &msg)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    try
    {
        if (msg.size() == 0)
        {
            co_return 0;
        }
        unsigned int n = 0;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                n = co_await asio::async_write(*sslsocket, asio::buffer(msg), asio::use_awaitable);
            }
            else
            {
                isclose = true;
            }
        }
        else
        {
            if (socket->is_open())
            {
                n = co_await asio::async_write(*socket, asio::buffer(msg), asio::use_awaitable);
            }
            else
            {
                isclose = true;
            }
        }
        co_return n;
    }
    catch (...)
    {
        isclose = true;
        iserror = true;
        cancel();
    }
    co_return 0;
}

void client_session::cancel()
{
    flush_parked_send();
    if (isssl)
    {
        sslsocket->lowest_layer().cancel(ec);
    }
    else
    {
        socket->cancel(ec);
    }
    isclose = true;
}

void client_session::half_stop()
{
    flush_parked_send();
    if (iserror)
    {
        isclose = true;
    }
}
asio::awaitable<std::string> client_session::async_stop()
{
    DEBUG_LOG("socket async_stop");
    std::string temp_msg;
    isclose = true;
    flush_parked_send();
    // 唤醒挂起中的发送消费者协程，让它看到 isclose 后退出
    waituphttp2();
    try
    {
        asio::error_code ec_a;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().cancel(ec_a);
                if (ec_a)
                {
                    temp_msg = temp_msg + ec_a.message();
                }

                if (!half_close)
                {
                    half_close = true;
                    temp_msg   = temp_msg + " SSL_Shutdown next time";
                    temp_msg.append("\n");
                    co_return temp_msg;
                }

                asio::error_code shutdown_ec;
                try
                {
                    co_await sslsocket->async_shutdown(asio::redirect_error(asio::use_awaitable, shutdown_ec));
                }
                catch (...)
                {
                }

                if (shutdown_ec)
                {
                    temp_msg = temp_msg + " SSL_Shutdown_Error: " + shutdown_ec.message();
                }
                else
                {
                    temp_msg = temp_msg + " SSL_Shutdown end time";
                }
                sslsocket->lowest_layer().close(ec_a);
                if (ec_a)
                {
                    temp_msg = temp_msg + ec_a.message();
                }
            }
        }
        else
        {
            if (socket->is_open())
            {
                socket->cancel(ec_a);
                if (ec_a)
                {
                    temp_msg = temp_msg + ec_a.message();
                }
                socket->close(ec_a);
                if (ec_a)
                {
                    temp_msg = temp_msg + ec_a.message();
                }
            }
        }
    }
    catch (const std::system_error &e)
    {
        iserror  = true;
        temp_msg = temp_msg + e.what();
    }
    catch (std::exception &e)
    {
        temp_msg = temp_msg + e.what();
        iserror  = true;
    }
    catch (...)
    {
        DEBUG_LOG("socket exp ");
        temp_msg = temp_msg + " exception ";
        iserror  = true;
    }
    temp_msg.append("\n");
    co_return temp_msg;
}

void client_session::stop()
{
    DEBUG_LOG("socket stop");
    isclose = true;
    flush_parked_send();
    // 唤醒挂起中的发送消费者协程，让它看到 isclose 后退出
    waituphttp2();
    try
    {
        asio::error_code ec_b;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().cancel(ec_b);
                sslsocket->lowest_layer().close(ec_b);
            }
            if (ec_b)
            {
                iserror = true;
                return;
            }
        }
        else
        {
            if (socket->is_open())
            {
                socket->cancel(ec_b);
                socket->close(ec_b);
            }
            if (ec_b)
            {
                iserror = true;
                return;
            }
        }
    }
    catch (const std::system_error &e)
    {
        iserror = true;
    }
    catch (std::exception &e)
    {
        iserror = true;
    }
    catch (...)
    {
        DEBUG_LOG("socket exp ");
        iserror = true;
    }
}

std::string client_session::getremoteip()
{
    if (client_ip.size() > 2)
    {
        return client_ip;
    }
    if (iserror)
    {
        return "";
    }
    asio::ip::tcp::endpoint ep;
    if (isssl)
    {
        ep = sslsocket->lowest_layer().remote_endpoint(ec);
    }
    else
    {
        ep = socket->remote_endpoint(ec);
    }
    if (ec)
    {
        iserror   = true;
        isclose   = true;
        client_ip = ec.message();

        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().cancel(ec);
                sslsocket->lowest_layer().close(ec);
            }
        }
        else
        {
            if (socket->is_open())
            {
                socket->cancel(ec);
                socket->close(ec);
            }
        }

        return "";
    }
    asio::ip::address addr = ep.address();
    if (addr.is_v6())
    {
        auto v6_addr = addr.to_v6();
        if (v6_addr.is_v4_mapped())
        {
            client_ip = asio::ip::make_address_v4(asio::ip::v4_mapped, v6_addr).to_string();
        }
        else
        {
            client_ip = v6_addr.to_string();
        }
    }
    else if (addr.is_v4())
    {
        client_ip = addr.to_v4().to_string();
    }

    return client_ip;
}

unsigned int client_session::getremoteport()
{
    if (client_port > 1)
    {
        return client_port;
    }
    if (iserror)
    {
        return 0;
    }
    asio::ip::tcp::endpoint ep;
    if (isssl)
    {
        ep = sslsocket->lowest_layer().remote_endpoint(ec);
    }
    else
    {
        ep = socket->remote_endpoint(ec);
    }

    if (ec)
    {
        iserror   = true;
        isclose   = true;
        client_ip = ec.message();

        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().cancel(ec);
                sslsocket->lowest_layer().close(ec);
            }
        }
        else
        {
            if (socket->is_open())
            {
                socket->cancel(ec);
                socket->close(ec);
            }
        }

        return 0;
    }
    client_port = ep.port();
    return client_port;
}

std::string client_session::getlocalip()
{
    if (iserror)
    {
        return "";
    }

    asio::ip::tcp::endpoint ep;
    if (isssl)
    {
        ep = sslsocket->lowest_layer().local_endpoint(ec);
    }
    else
    {
        ep = socket->local_endpoint(ec);
    }
    if (ec)
    {
        iserror = true;
        isclose = true;

        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().cancel(ec);
                sslsocket->lowest_layer().close(ec);
            }
        }
        else
        {
            if (socket->is_open())
            {
                socket->cancel(ec);
                socket->close(ec);
            }
        }

        return "";
    }

    return ep.address().to_string();
}

unsigned int client_session::getlocalport()
{
    unsigned int server_port = 0;
    if (iserror)
    {
        return 0;
    }

    asio::ip::tcp::endpoint ep;
    if (isssl)
    {
        ep = sslsocket->lowest_layer().local_endpoint(ec);
    }
    else
    {
        ep = socket->local_endpoint(ec);
    }

    if (ec)
    {
        iserror = true;
        isclose = true;

        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().cancel(ec);
                sslsocket->lowest_layer().close(ec);
            }
        }
        else
        {
            if (socket->is_open())
            {
                socket->cancel(ec);
                socket->close(ec);
            }
        }

        return 0;
    }
    server_port = ep.port();
    return server_port;
}

bool client_session::post_write(std::string_view msg)
{
    if (isclose || iserror || !http2_ring_queue)
    {
        return false;
    }
    // 条数闸：16 槽环最多积压 15 条；字节闸：累计积压过 LIMIT 才拒，环空时单帧放行
    unsigned long long backlog = http2_ring_queue->bytes.load(std::memory_order_relaxed);
    unsigned int backlog_cnt   = http2_ring_queue->has_size();
    if (backlog_cnt + 1 >= http2_ring_queue->capacity_)
    {
        http2_ring_overflow_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    if (backlog > 0 && backlog + msg.size() > MQTT_SEND_RING_BYTE_LIMIT)
    {
        http2_ring_overflow_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    if (!http2_ring_queue->push(reinterpret_cast<const unsigned char *>(msg.data()),
                                static_cast<unsigned int>(msg.size())))
    {
        http2_ring_overflow_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    http2_ring_queue->bytes.fetch_add(msg.size(), std::memory_order_relaxed);
    if (http2_need_wakeup)
    {
        waituphttp2();
    }
    return true;
}

bool client_session::post_write(const unsigned char *buf, unsigned int len)
{
    if (isclose || iserror || !buf)
        return false;
    return post_write(std::string_view(reinterpret_cast<const char *>(buf), len));
}

}// namespace http
