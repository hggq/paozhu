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
#include "debug_log.h"
#include "base64.h"
#include <algorithm>

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
        if (readnum == 0)
        {
            // 无错误却读回 0 字节，只可能是对端已关闭
            // Zero bytes without error only means the peer has closed.
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
    // 本端发给对端的 SETTINGS：MAX_CONCURRENT_STREAMS(0x03)=100，
    // INITIAL_WINDOW_SIZE(0x04)=0xFFFFFF。
    // 100 是对外的承诺，入站另有本端自己的资源兜底 CONST_HTTP2_MAX_STREAMS=512
    // （readheaders() 里那条 http_data.size() 判定）：守承诺的对端最多开到 100，
    // 无视承诺的直接被 512 拦住，所以承诺值不需要跟兜底值同源。
    unsigned char _recvack[] = {0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x64, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xFF};
    if (!http2_ring_queue->push(_recvack, 21))
    {
        http2_ring_queue_drop_count++;
    }
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
    // 必须是栈上缓冲。函数级 static 全进程只有一份，多个连接同时调用会互相覆盖，
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

asio::awaitable<bool> client_session::flush_send_ring()
{
    // 有界排空：见 client_session.h 注释。不写 socket（写者是 ring_client_server 与
    // 内联直发协程，共用 FIFO 写锁），只等——每轮 sleep 让出 strand，
    // 消费者的 async_write 完成事件才能被这个 strand 串行跑出来。
    // 环空即「全部环内帧已写出」：pop 在 async_write 完成之后（见 ring_client_server 消费序）。
    if (http2_ring_queue == nullptr)
    {
        co_return true;
    }
    // 空环快路径：绝大多数响应到这里环已排空，省掉每响应一次 timer 构造。
    if (http2_ring_queue->has_size() == 0)
    {
        co_return true;
    }
    // strand_ 本身作 timer 的 executor：到期回调仍在本连接 strand 串行域内跑，
    // 不能用 strand_.context()——那是 execution_context，没有 get_executor，编不过。
    asio::steady_timer drain_timer(strand_);
    // 2 秒上限、20 毫秒粒度：本地环排空通常在毫秒级；客户端停读时不无限拖住 teardown。
    for (unsigned int tick = 0; tick < 100; tick++)
    {
        // 消费者（ring_client_server）遇 isclose/iserror 会退出，环再无人排空；
        // socket 已被关同理。这几种情况立刻放弃等待，不空转满 2 秒。
        if (isclose || iserror)
        {
            co_return false;
        }
        bool sock_open = isssl ? (sslsocket && sslsocket->lowest_layer().is_open()) : (socket && socket->is_open());
        if (!sock_open)
        {
            co_return false;
        }
        if (http2_ring_queue->has_size() == 0)
        {
            co_return true;
        }
        drain_timer.expires_after(std::chrono::milliseconds(20));
        co_await drain_timer.async_wait(asio::use_awaitable);
    }
    // 超时仍未排空：调用方不能接着直写 HEADERS，否则会抢在环内 SETTINGS 前面。
    co_return http2_ring_queue->has_size() == 0;
}

asio::awaitable<void> client_session::async_send_goway(unsigned int last_stream_id, unsigned int error_code)
{
    // GOAWAY 经写锁直接写出，不走环，环满也不会丢。
    unsigned char fr[17] = {0x00, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    fr[9]                = (last_stream_id >> 24) & 0xFF;
    fr[10]               = (last_stream_id >> 16) & 0xFF;
    fr[11]               = (last_stream_id >> 8) & 0xFF;
    fr[12]               = last_stream_id & 0xFF;
    fr[13]               = (error_code >> 24) & 0xFF;
    fr[14]               = (error_code >> 16) & 0xFF;
    fr[15]               = (error_code >> 8) & 0xFF;
    fr[16]               = error_code & 0xFF;
    if (co_await http2_write_lock())
    {
        asio::error_code wec;
        if (isssl)
            co_await asio::async_write(*sslsocket, asio::buffer(fr, 17), asio::redirect_error(asio::use_awaitable, wec));
        else
            co_await asio::async_write(*socket, asio::buffer(fr, 17), asio::redirect_error(asio::use_awaitable, wec));
        http2_write_unlock();
    }
    // 关闭冲刷：置 closing + cancel 全部等待者，内联发送协程在循环边界静音退出。
    http2_close_waiters();
    co_return;
}

void client_session::send_recv_setting()
{
    unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00};
    if (!http2_ring_queue->push(_recvack, 9))
    {
        http2_ring_queue_drop_count++;
    }
}

bool client_session::push_window_update_conn(unsigned int up_num)
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

    return http2_ring_queue->push(_recvack, 13);
}

bool client_session::push_window_update_stream(unsigned int stmid, unsigned int up_num)
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

    return http2_ring_queue->push(_recvack, 13);
}

void client_session::grant_conn_window(unsigned int up_num)
{
    if (up_num == 0)
    {
        return;
    }
    // 一笔补量 = 把对端恢复到本端计数 FULL 所需的量，所以欠账之和不会超过 FULL，
    // 合帧重投也就不需要按 2^31-1 钳制。
    conn_grant_pending += up_num;
    if (push_window_update_conn(conn_grant_pending))
    {
        conn_grant_pending = 0;
        return;
    }
    http2_window_update_requeue_count++;
}

void client_session::grant_stream_window(unsigned int stmid, unsigned int up_num)
{
    if (up_num == 0)
    {
        return;
    }
    unsigned int &pending = stream_grant_pending[stmid];
    pending += up_num;
    if (push_window_update_stream(stmid, pending))
    {
        stream_grant_pending.erase(stmid);
        return;
    }
    http2_window_update_requeue_count++;
}

void client_session::flush_pending_window_update()
{
    if (conn_grant_pending > 0)
    {
        if (!push_window_update_conn(conn_grant_pending))
        {
            // 环又满了：整笔留在账上，下一个腾出的槽位再来一次
            http2_window_update_requeue_count++;
            return;
        }
        conn_grant_pending = 0;
    }
    for (auto iter = stream_grant_pending.begin(); iter != stream_grant_pending.end();)
    {
        if (!push_window_update_stream(iter->first, iter->second))
        {
            http2_window_update_requeue_count++;
            break;
        }
        iter = stream_grant_pending.erase(iter);
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

void client_session::wake_ring_consumer()
{
    try
    {
        std::unique_lock lk(wake_ring_consumer_mutex);
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

// ---- h2 直发原语 ----
// 都在本连接 strand 上跑：容器不用锁，cancel 和 async_wait 同一个 executor。

asio::awaitable<bool> client_session::http2_write_lock()
{
    if (isclose || iserror)
        co_return false;
    asio::steady_timer timer(co_await asio::this_coro::executor);
    // 协程在等待中被销毁时局部 timer 析构，队列里的裸指针会悬垂：
    // 析构即摘除；若已被选为交接对象，把锁传下去，不然 busy 永真。
    struct waiter_guard
    {
        client_session *sess;
        asio::steady_timer *t;
        ~waiter_guard()
        {
            sess->http2_write_waiters.remove(t);
            if (sess->http2_write_next_owner == t)
            {
                sess->http2_write_next_owner = nullptr;
                sess->http2_write_unlock();
            }
        }
    } waiter_guard_{this, &timer};
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WRITE_LOCK_TIMEOUT);
    while (http2_write_busy)
    {
        if (isclose || iserror || http2_closing.load(std::memory_order_relaxed))
            co_return false;
        const auto left = deadline - std::chrono::steady_clock::now();
        if (left <= std::chrono::steady_clock::duration::zero())
        {
            // FIFO 交接不会饿死等待者：等满预算说明持锁者的 async_write 卡死，
            // 连接写路径已死。在这里集中判定并叫醒所有人，调用方见 false 即退出，
            // 不再逐条 RST 同连接的健康流。不关 socket：慢写可能仍在途，
            // 收尾交给正常 teardown 路径。
            isclose = true;
            http2_close_waiters();
            wake_ring_consumer();
            co_return false;
        }
        http2_write_waiters.push_back(&timer);
        timer.expires_after(left);
        auto [wait_ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        http2_write_waiters.remove(&timer);
        if (http2_write_next_owner == &timer)
        {
            // 被交接选中就拿着锁，不管 ec 是什么都要处置：超时或关闭时把锁传下去或释放，
            // 不然 busy 永真、队列已空，这条连接就写死了。
            http2_write_next_owner = nullptr;
            if (!http2_closing.load(std::memory_order_relaxed) && wait_ec == asio::error::operation_aborted)
                co_return true;
            http2_write_unlock();
            if (wait_ec != asio::error::operation_aborted)
            {
                // 交接选中与自然到期撞在同一拍：预算同样已耗尽，按写停滞收尾。
                isclose = true;
                http2_close_waiters();
                wake_ring_consumer();
            }
            co_return false;
        }
        if (isclose || iserror || http2_closing.load(std::memory_order_relaxed))
            co_return false;
        if (wait_ec != asio::error::operation_aborted)
            co_return false;
        // 不是交接的 cancel：回循环重判
    }
    http2_write_busy = true;
    co_return true;
}

void client_session::http2_write_unlock()
{
    if (!http2_write_waiters.empty())
    {
        // busy 不置 false：所有权交接给队首，它的续体醒来即持锁
        http2_write_next_owner = http2_write_waiters.front();
        http2_write_waiters.pop_front();
        http2_write_next_owner->cancel();
    }
    else
    {
        http2_write_busy = false;
    }
}

asio::awaitable<bool> client_session::http2_wait_window_episode(unsigned int sid, const std::atomic_bool &dead)
{
    asio::steady_timer timer(co_await asio::this_coro::executor);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
    // 注销两边都清：timer 可能换过登记边，残留条目会被 cancel 成悬垂指针。
    auto unregister = [&]()
    {
        auto it = http2_stream_window_waiters.find(sid);
        if (it != http2_stream_window_waiters.end() && it->second == &timer)
            http2_stream_window_waiters.erase(it);
        auto &v = http2_conn_window_waiters;
        v.erase(std::remove(v.begin(), v.end(), &timer), v.end());
    };
    // 协程在等待中被销毁时局部 timer 析构，两个容器里的裸指针会悬垂：析构即摘除。
    // 正常路径已手动 unregister，重复调用是幂等的。
    struct unregister_guard
    {
        decltype(unregister) &fn;
        ~unregister_guard()
        {
            fn();
        }
    } unregister_guard_{unregister};
    for (;;)
    {
        if (http2_closing.load(std::memory_order_relaxed) || dead.load(std::memory_order_relaxed))
            co_return false;
        const auto left = deadline - std::chrono::steady_clock::now();
        if (left <= std::chrono::steady_clock::duration::zero())
            co_return false;
        // 补查两级窗口：连接级见底登记连接级，否则看流级（条目缺失按
        // remote_initial_window_size 判）；有额度直接返回让调用方 reserve。
        const bool conn_empty = window_update_num.load() <= has_send_update_num.load();
        if (!conn_empty)
        {
            std::lock_guard<std::mutex> lk(stream_send_window_mutex);
            auto it = stream_send_window.find(sid);
            unsigned int avail = (it == stream_send_window.end()) ? remote_initial_window_size.load() : it->second;
            if (avail > 0)
                co_return true;
        }
        unregister();
        if (conn_empty)
            http2_conn_window_waiters.push_back(&timer);
        else
            http2_stream_window_waiters[sid] = &timer;
        timer.expires_after(left);
        auto [wait_ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        unregister();
        if (http2_closing.load(std::memory_order_relaxed) || dead.load(std::memory_order_relaxed))
            co_return false;
        if (wait_ec != asio::error::operation_aborted)
        {
            // 自然到期与 WU 到达可能撞在同一拍：返回前再查一次两级窗口，
            // 有额度就放调用方去 reserve，不误撤流。
            const bool conn_empty = window_update_num.load() <= has_send_update_num.load();
            if (!conn_empty)
            {
                std::lock_guard<std::mutex> lk(stream_send_window_mutex);
                auto it = stream_send_window.find(sid);
                unsigned int avail = (it == stream_send_window.end()) ? remote_initial_window_size.load() : it->second;
                if (avail > 0)
                    co_return true;
            }
            co_return false;// 预算睡满 → 调用方撤流
        }
        // aborted（WU/IWS/dead cancel）→ 回循环顶重查额度与剩余预算
    }
}

void client_session::http2_cancel_conn_waiters()
{
    // 按登记序 cancel ⇒ 续体按 FIFO 醒来，队首先 reserve
    for (auto *t : http2_conn_window_waiters)
        t->cancel();
}

void client_session::http2_cancel_stream_waiter(unsigned int sid)
{
    auto it = http2_stream_window_waiters.find(sid);
    if (it != http2_stream_window_waiters.end())
        it->second->cancel();
}

void client_session::http2_cancel_all_stream_waiters()
{
    for (auto &kv : http2_stream_window_waiters)
        kv.second->cancel();
}

void client_session::http2_close_waiters()
{
    http2_closing.store(true, std::memory_order_relaxed);
    for (auto *t : http2_conn_window_waiters)
        t->cancel();
    for (auto &kv : http2_stream_window_waiters)
        kv.second->cancel();
    for (auto *t : http2_write_waiters)
        t->cancel();
}

asio::awaitable<bool> client_session::http2_send_rst_direct(unsigned int s_stream_id, unsigned int stream_error_code)
{
    unsigned char fr[13] = {0x00, 0x00, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    fr[5]                = (s_stream_id >> 24) & 0x7F;
    fr[6]                = (s_stream_id >> 16) & 0xFF;
    fr[7]                = (s_stream_id >> 8) & 0xFF;
    fr[8]                = s_stream_id & 0xFF;
    fr[9]                = (stream_error_code >> 24) & 0xFF;
    fr[10]               = (stream_error_code >> 16) & 0xFF;
    fr[11]               = (stream_error_code >> 8) & 0xFF;
    fr[12]               = stream_error_code & 0xFF;
    if (!co_await http2_write_lock())
        co_return false;
    asio::error_code wec;
    if (isssl)
        co_await asio::async_write(*sslsocket, asio::buffer(fr, 13), asio::redirect_error(asio::use_awaitable, wec));
    else
        co_await asio::async_write(*socket, asio::buffer(fr, 13), asio::redirect_error(asio::use_awaitable, wec));
    http2_write_unlock();
    co_return !wec;
}

bool client_session::push_rst_stream(unsigned int sid, unsigned int code)
{
    unsigned char fr[13] = {0x00, 0x00, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    fr[5]                = (sid >> 24) & 0x7F;
    fr[6]                = (sid >> 16) & 0xFF;
    fr[7]                = (sid >> 8) & 0xFF;
    fr[8]                = sid & 0xFF;
    fr[9]                = (code >> 24) & 0xFF;
    fr[10]               = (code >> 16) & 0xFF;
    fr[11]               = (code >> 8) & 0xFF;
    fr[12]               = code & 0xFF;
    return http2_ring_queue->push(fr, 13);
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
    http2_closing.store(true, std::memory_order_relaxed);
    http2_close_waiters();
    // 唤醒挂起中的发送消费者协程，让它看到 isclose 后退出
    wake_ring_consumer();
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
    // 跨线程调用只置原子旗（cancel 必须在 strand 上，留给 async_stop/等待者预算到期）；
    // 内联发送协程最迟 10 秒预算到即见旗退出。
    http2_closing.store(true, std::memory_order_relaxed);
    // 唤醒挂起中的发送消费者协程，让它看到 isclose 后退出
    wake_ring_consumer();
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
    // 条数闸：环最多积压 capacity_ - 1 条；字节闸：累计积压过 LIMIT 才拒，环空时单帧放行
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
    // 先记账再入环：帧一旦发布到环上，消费者就可能写完并做减法；若在 push 之后才累加，
    // 消费者读到的旧值会让这一帧的字节永久留在 bytes 里，累计过闸后本连接再也写不出去。
    http2_ring_queue->bytes.fetch_add(msg.size(), std::memory_order_relaxed);
    if (!http2_ring_queue->push(reinterpret_cast<const unsigned char *>(msg.data()),
                                static_cast<unsigned int>(msg.size())))
    {
        http2_ring_queue->bytes.fetch_sub(msg.size(), std::memory_order_relaxed);
        http2_ring_overflow_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    if (http2_need_wakeup)
    {
        wake_ring_consumer();
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
