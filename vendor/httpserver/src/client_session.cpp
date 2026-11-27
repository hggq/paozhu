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
#include "http2_sender_count_guard.h"
#include "debug_log.h"
#include "base64.h"
#include "server.h"
#include "datetime.h"
#include <algorithm>
#include <cassert>
#include <limits>

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
    http2_cancel_all_pending_peers(false);
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
                readnum = co_await sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::redirect_error(asio::use_awaitable, ec));
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
                readnum = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::redirect_error(asio::use_awaitable, ec));
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
            readnum = co_await sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::redirect_error(asio::use_awaitable, ec));
        }
        else
        {
            readnum = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::redirect_error(asio::use_awaitable, ec));
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

unsigned int client_session::send_writer(const std::string &msg, const std::shared_ptr<httppeer> &peer)
{
    if (isclose)
    {
        return 0;
    }
    if (httpv == 2)
    {
        return http2_send_content_frame(msg, peer);
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

unsigned int client_session::send_writer(std::string_view msg, const std::shared_ptr<httppeer> &peer)
{
    if (isclose)
    {
        return 0;
    }
    if (httpv == 2)
    {
        return http2_send_content_frame(msg, peer);
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
    // 本端发给对端的 SETTINGS：MAX_CONCURRENT_STREAMS(0x03)=CONST_HTTP2_MAX_CONCURRENT_STREAMS，
    // INITIAL_WINDOW_SIZE(0x04)=0xFFFFFF。
    // 承诺值和入站执行同源：超过承诺的流按流级 REFUSED_STREAM 拒绝（http2_parse.cpp），
    // CONST_HTTP2_MAX_STREAMS=2048 只是头部块未完成等异常路径的连接级兜底。
    unsigned char _recvack[] = {0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, CONST_HTTP2_MAX_CONCURRENT_STREAMS, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xFF};
    if (!push_control_frame(_recvack, 21))
    {
        http2_ring_queue_drop_count++;
    }
    // 生产者契约：消费者可能已挂起（注册后的复查只封住「先入队」那一侧），
    // 入队成功后必须查 need_wakeup 唤醒，否则 SETTINGS 拖到下一次活动才写出。
    // 入队失败不唤：环满时消费者不可能在空环上挂起。
    else if (http2_need_wakeup)
    {
        wake_ring_consumer();
    }
    co_return;
}

asio::awaitable<void> client_session::http2_send_ping()
{
    unsigned char _recvack[] =
        {0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (push_control_frame(_recvack, 17) && http2_need_wakeup)
        wake_ring_consumer();
    co_return;
}

void client_session::send_ping()
{
    unsigned char _recvack[] =
        {0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (push_control_frame(_recvack, 17) && http2_need_wakeup)
        wake_ring_consumer();
}

asio::awaitable<bool> client_session::flush_send_ring(bool include_producers)
{
    // 有界排空：等 FIFO 环和待发送 peer 列表同时空，活跃写入也计入未排空状态。
    // 消费者协程是唯一 socket 写者。
    auto drained = [this, include_producers]()
    {
        if (http2_fifo_write_active.load(std::memory_order_acquire))
            return false;
        if (http2_ring_queue != nullptr && http2_ring_queue->has_size() != 0)
            return false;
        return !(include_producers ? http2_has_pending_peers() : http2_has_queued_peers());
    };
    // 空环快路径：绝大多数响应到这里环已排空，省掉每响应一次 timer 构造。
    if (drained())
    {
        co_return true;
    }
    // strand_ 本身作 timer 的 executor：到期回调仍在本连接 strand 串行域内跑，
    // 不能用 strand_.context()——那是 execution_context，没有 get_executor，编不过。
    asio::steady_timer drain_timer(strand_);
    // 2 秒上限、20 毫秒粒度：本地环排空通常在毫秒级；客户端停读时不无限拖住 teardown。
    for (unsigned int tick = 0; tick < 100; tick++)
    {
        // 消费者遇 isclose/iserror 会退出，环再无人排空；
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
        if (drained())
        {
            co_return true;
        }
        drain_timer.expires_after(std::chrono::milliseconds(20));
        co_await drain_timer.async_wait(asio::use_awaitable);
    }
    // 超时仍未排空：调用方不能接着直写 HEADERS，否则会抢在环内 SETTINGS 前面。
    co_return drained();
}

asio::awaitable<void> client_session::async_send_goway(unsigned int last_stream_id, unsigned int error_code)
{
    // GOAWAY 入控制环，环满就地自适应重试，2 秒预算——客户端停读时不无限拖住 teardown。
    unsigned char fr[17] = {0x00, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    fr[9]                = (last_stream_id >> 24) & 0x7F;
    fr[10]               = (last_stream_id >> 16) & 0xFF;
    fr[11]               = (last_stream_id >> 8) & 0xFF;
    fr[12]               = last_stream_id & 0xFF;
    fr[13]               = (error_code >> 24) & 0xFF;
    fr[14]               = (error_code >> 16) & 0xFF;
    fr[15]               = (error_code >> 8) & 0xFF;
    fr[16]               = error_code & 0xFF;
    // 先等在途响应写出再投 GOAWAY：发送协程优先处理控制帧，GOAWAY 入环后会
    // 抢在 FIFO 排队帧之前上线；部分客户端收到非零错误码的 GOAWAY 后立即停读。
    // flush 自带 2 秒预算，排不空也继续投 GOAWAY，由它通知对端收尾。
    co_await flush_send_ring();
    asio::steady_timer timer(co_await asio::this_coro::executor);
    unsigned int wait_interval_us = CONST_HTTP2_RING_FULL_WAIT_BASE;
    const auto deadline           = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    for (;;)
    {
        if (isclose || iserror)
            break;
        if (push_control_frame(fr, 17))
        {
            if (http2_need_wakeup)
                wake_ring_consumer();
            // 等 GOAWAY 真的写出再返回：调用方随后 cancel()，不等就把环里的
            // GOAWAY 与在途响应一起杀掉。超时未排空时后续 cancel() 仍会丢帧，
            // 记一条日志留诊断线索。
            if (!co_await flush_send_ring(false))
            {
                DEBUG_LOG("http2 goaway flush timeout last_sid=%u err=%u", last_stream_id, error_code);
            }
            break;
        }
        http2_ring_queue_drop_count++;
        if (std::chrono::steady_clock::now() >= deadline)
            break;
        const unsigned char backlog = http2_ring_queue->has_size();
        wait_interval_us            = http2_ring_wait_adjust_us(wait_interval_us, backlog);
        // 有空槽就立即重试打满，不睡；只有环仍满才按自适应间隔等。
        if (backlog < CONST_HTTP2_RING_QUEUE_CAPACITY - 1)
            continue;
        timer.expires_after(std::chrono::microseconds(wait_interval_us));
        auto [wait_ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        (void)wait_ec;
    }
    // 关闭冲刷：置 closing + 撤掉全部 pending peer，发送生产者在循环边界静音退出。
    http2_closing.store(true, std::memory_order_relaxed);
    http2_cancel_all_pending_peers(true);
    co_return;
}

void client_session::send_recv_setting()
{
    unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00};
    if (!push_control_frame(_recvack, 9))
    {
        http2_ring_queue_drop_count++;
    }
    else if (http2_need_wakeup)
    {
        wake_ring_consumer();
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

    if (!push_control_frame(_recvack, 13))
        return false;
    if (http2_need_wakeup)
        wake_ring_consumer();
    return true;
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

    if (!push_control_frame(_recvack, 13))
        return false;
    if (http2_need_wakeup)
        wake_ring_consumer();
    return true;
}

void client_session::grant_conn_window(unsigned int up_num)
{
    if (up_num == 0) return;
    h2_conn_send.conn_grant_pending += up_num;
    if (push_window_update_conn(h2_conn_send.conn_grant_pending))
        h2_conn_send.conn_grant_pending = 0;
    else
        http2_window_update_requeue_count++;
}

bool client_session::push_rst_stream(unsigned int sid, unsigned int code)
{
    if (sid == 0 || isclose.load(std::memory_order_relaxed) ||
        http2_closing.load(std::memory_order_relaxed)) return false;
    unsigned char frame[13] = {0x00, 0x00, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    frame[5]  = static_cast<unsigned char>((sid >> 24) & 0x7F);
    frame[6]  = static_cast<unsigned char>((sid >> 16) & 0xFF);
    frame[7]  = static_cast<unsigned char>((sid >> 8) & 0xFF);
    frame[8]  = static_cast<unsigned char>(sid & 0xFF);
    frame[9]  = static_cast<unsigned char>((code >> 24) & 0xFF);
    frame[10] = static_cast<unsigned char>((code >> 16) & 0xFF);
    frame[11] = static_cast<unsigned char>((code >> 8) & 0xFF);
    frame[12] = static_cast<unsigned char>(code & 0xFF);
    if (!push_control_frame(frame, sizeof(frame)))
    {
        http2_ring_queue_drop_count++;
        return false;
    }
    if (http2_need_wakeup) wake_ring_consumer();
    return true;
}

void client_session::http2_enqueue_grant(httppeer *peer)
{
    // 流级前置条件（stream_dead / queued / isclose）由 httppeer::grant_recv_window
    // 在调用前自行判定，connection 层只管把 peer 挂入连接级 grant 链表头。
    peer->h2recv.grant_prev = h2_conn_send.grant_tail;
    if (h2_conn_send.grant_tail) h2_conn_send.grant_tail->h2recv.grant_next = peer;
    else h2_conn_send.grant_head = peer;
    h2_conn_send.grant_tail = peer;
    h2_conn_send.grant_count++;
    peer->h2recv.grant_queued = true;
}

void client_session::http2_unlink_grant(httppeer *peer)
{
    if (!peer->h2recv.grant_queued) return;
    if (peer->h2recv.grant_prev) peer->h2recv.grant_prev->h2recv.grant_next = peer->h2recv.grant_next;
    else h2_conn_send.grant_head = peer->h2recv.grant_next;
    if (peer->h2recv.grant_next) peer->h2recv.grant_next->h2recv.grant_prev = peer->h2recv.grant_prev;
    else h2_conn_send.grant_tail = peer->h2recv.grant_prev;
    h2_conn_send.grant_count--;
    // prev / next / queued / grant_pending 的清零交给 httppeer::grant_self_unlink / reset_recv_grant_pending。
}

void client_session::flush_pending_window_update()
{
    if (h2_conn_send.conn_grant_pending > 0)
    {
        if (!push_window_update_conn(h2_conn_send.conn_grant_pending))
        {
            http2_window_update_requeue_count++;
            return;
        }
        h2_conn_send.conn_grant_pending = 0;
    }
    while (h2_conn_send.grant_head)
    {
        auto *peer = h2_conn_send.grant_head;
        if (!push_window_update_stream(peer->stream_id, peer->h2recv.grant_pending))
        {
            http2_window_update_requeue_count++;
            break;
        }
        peer->grant_self_unlink();
    }
}

unsigned int client_session::send_writer(const unsigned char *buffer, unsigned int buffersize,
                                         const std::shared_ptr<httppeer> &peer)
{
    if (isclose)
    {
        return 0;
    }
    if (httpv == 2)
    {
        return http2_send_content_frame(std::string_view(reinterpret_cast<const char *>(buffer), buffersize), peer);
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
    auto timer = std::atomic_load(&http2_consumer_timer);
    if (!timer) return;
    auto executor = timer->get_executor();
    asio::post(executor, [timer = std::move(timer)]() mutable { timer->cancel(); });
}

void client_session::http2_enqueue_peer(const std::shared_ptr<httppeer> &peer)
{
    if (!peer) return;
    auto &state = peer->h2send;
    if (state.send_queued || state.in_batch || state.completion_done ||
        state.stream_is_dead || state.rst_pending || peer->isclose.load(std::memory_order_relaxed) ||
        isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed)) return;
    state.queue_owner = peer;
    state.send_prev = h2_conn_send.send_tail;
    if (h2_conn_send.send_tail) h2_conn_send.send_tail->h2send.send_next = peer.get();
    else h2_conn_send.send_head = peer.get();
    h2_conn_send.send_tail = peer.get();
    h2_conn_send.send_count++;
    state.send_queued = true;
    wake_ring_consumer();
}

void client_session::http2_unlink_peer(httppeer *peer)
{
    auto &state = peer->h2send;
    if (!state.send_queued) return;
    if (state.send_prev) state.send_prev->h2send.send_next = state.send_next;
    else h2_conn_send.send_head = state.send_next;
    if (state.send_next) state.send_next->h2send.send_prev = state.send_prev;
    else h2_conn_send.send_tail = state.send_prev;
    h2_conn_send.send_count--;
    state.send_prev = nullptr;
    state.send_next = nullptr;
    state.send_queued = false;
    state.queue_owner.reset();
}

void client_session::http2_mark_peer_rst(const std::shared_ptr<httppeer> &peer, unsigned int error_code)
{
    if (!peer) return;
    auto &state = peer->h2send;
    if (state.completion_done || state.rst_sent || state.rst_pending) return;
    state.rst_pending = true;
    state.rst_error_code = error_code;
    peer->is_rst_stream.store(true, std::memory_order_release);
    http2_unlink_peer(peer.get());
    peer->grant_self_unlink();
    if (push_rst_stream(peer->stream_id, error_code))
    {
        state.rst_sent = true;
        state.rst_pending = false;
    }
    else if (!isclose.load(std::memory_order_relaxed) && !http2_closing.load(std::memory_order_relaxed))
    {
        // RST 重试独立于正常 DATA 队列，死流因此不会再次成为可运行 peer。
        ++h2_conn_send.rst_retry_count;
        try
        {
            asio::co_spawn(strand_,
                [self = shared_from_this(), peer]() mutable -> asio::awaitable<void>
                {
                    auto &retry_state = peer->h2send;
                    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
                    asio::steady_timer timer(co_await asio::this_coro::executor);
                    try
                    {
                        while (retry_state.rst_pending && !retry_state.rst_sent &&
                               !retry_state.completion_done &&
                               peer->is_rst_stream.load(std::memory_order_acquire) &&
                               !self->isclose.load(std::memory_order_relaxed) &&
                               !self->http2_closing.load(std::memory_order_relaxed))
                        {
                            if (self->push_rst_stream(peer->stream_id, retry_state.rst_error_code))
                            {
                                retry_state.rst_sent = true;
                                break;
                            }
                            if (std::chrono::steady_clock::now() >= deadline) break;
                            timer.expires_after(std::chrono::milliseconds(1));
                            auto [wait_ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
                            (void)wait_ec;
                        }
                    }
                    catch (...)
                    {
                    }
                    retry_state.rst_pending = false;
                    --self->h2_conn_send.rst_retry_count;
                    if (!retry_state.in_batch)
                    {
                        self->http2_refund_peer_window(peer.get());
                        self->http2_complete_active_peer(peer, false);
                    }
                },
                asio::detached);
        }
        catch (...)
        {
            state.rst_pending = false;
            --h2_conn_send.rst_retry_count;
        }
    }
    else state.rst_pending = false;
    wake_ring_consumer();
}

std::shared_ptr<httppeer> client_session::http2_take_next_peer()
{
    auto *peer = h2_conn_send.send_head;
    if (!peer) return nullptr;
    auto result = peer->h2send.queue_owner.lock();
    http2_unlink_peer(peer);
    return result;
}

bool client_session::http2_peer_is_blocked(const std::shared_ptr<httppeer> &peer)
{
    auto &state = peer->h2send;
    if (state.stream_is_dead || peer->isclose.load(std::memory_order_relaxed) || state.rst_pending) return false;
    if (state.refill_pending || state.refill_running) return true;
    if (!state.headers_sent)
    {
        if (state.end_on_headers || !peer->output.empty() || peer->h2file) return false;
        return true;
    }
    if (state.end_on_headers ||
        (state.sent_bytes == peer->output.size() && (!peer->h2file || peer->h2file->remaining == 0)))
        return false;
    return http2_peer_window_available(peer.get()) <= 0 || http2_conn_window_available() == 0;
}

bool client_session::http2_begin_peer_refill(const std::shared_ptr<httppeer> &peer)
{
    auto &state = peer->h2send;
    if (state.refill_pending || state.refill_running || state.stream_is_dead || state.rst_pending ||
        state.completion_done || peer->isclose.load(std::memory_order_relaxed)) return false;
    state.refill_pending = true;
    return true;
}

void client_session::http2_finish_peer_refill(const std::shared_ptr<httppeer> &peer, bool ready)
{
    auto &state = peer->h2send;
    state.refill_pending = false;
    state.refill_running = false;
    state.blocked = !ready;
    state.blocked_deadline = {};
    if (ready && !state.stream_is_dead && !state.rst_pending && !state.completion_done &&
        !peer->isclose.load(std::memory_order_relaxed) &&
        !isclose.load(std::memory_order_relaxed) && !http2_closing.load(std::memory_order_relaxed))
    {
        if (state.send_queued) http2_unlink_peer(peer.get());
        http2_enqueue_peer(peer);
    }
}

void client_session::http2_complete_active_peer(const std::shared_ptr<httppeer> &peer, bool success)
{
    auto &state = peer->h2send;
    std::shared_ptr<h2_borrow_state> completion;
    {
        std::lock_guard<std::mutex> lock(h2_send_window_mutex);
        if (state.completion_done) return;
        http2_unlink_peer(peer.get());
        peer->grant_self_unlink();
        h2_conn_send.reserved -= std::min<unsigned long long>(
            state.send_reserved, h2_conn_send.reserved);
        state.in_batch = false;
        state.blocked = false;
        state.send_reserved = 0;
        state.completion_done = true;
        peer->isfinish = success;
        completion = state.completion;
    }
    if (completion)
    {
        if (success) completion->release();
        else completion->cancel();
    }
}

void client_session::http2_cancel_all_pending_peers(bool wake)
{
    while (h2_conn_send.send_head)
    {
        auto peer = http2_take_next_peer();
        if (peer) http2_complete_active_peer(peer, false);
    }
    while (h2_conn_send.grant_head)
    {
        auto *peer = h2_conn_send.grant_head;
        peer->grant_self_unlink();
    }
    h2_conn_send.conn_grant_pending = 0;
    if (wake) wake_ring_consumer();
}

void client_session::http2_begin_content_stream(const std::shared_ptr<httppeer> &peer)
{
    if (!peer) return;
    peer->h2_producer_count.fetch_add(1, std::memory_order_release);
    h2_conn_send.active_producers.fetch_add(1, std::memory_order_release);
}

void client_session::http2_end_content_stream(const std::shared_ptr<httppeer> &peer)
{
    if (!peer) return;
    assert(peer->h2_producer_count.load(std::memory_order_acquire) > 0);
    peer->h2_producer_count.fetch_sub(1, std::memory_order_release);
    assert(h2_conn_send.active_producers.load(std::memory_order_acquire) > 0);
    h2_conn_send.active_producers.fetch_sub(1, std::memory_order_release);
}

client_session::h2_content_producer_guard::h2_content_producer_guard(
    client_session *session, const std::shared_ptr<httppeer> &stream_peer)
    : sess(session), peer(stream_peer)
{
    sess->http2_begin_content_stream(peer);
}

client_session::h2_content_producer_guard::~h2_content_producer_guard()
{
    sess->http2_end_content_stream(peer);
}

void client_session::http2_expire_blocked_peers()
{
    const auto now = std::chrono::steady_clock::now();
    for (auto *p = h2_conn_send.send_head; p;)
    {
        auto *next = p->h2send.send_next;
        auto peer = p->h2send.queue_owner.lock();
        if (peer && peer->h2send.blocked &&
            peer->h2send.blocked_deadline != std::chrono::steady_clock::time_point{} &&
            now >= peer->h2send.blocked_deadline)
        {
            http2_mark_peer_rst(peer, CONST_HTTP2_STREAM_ERROR_CANCEL);
            http2_mark_stream_dead(peer);
        }
        p = next;
    }
}

bool client_session::http2_has_pending_peers()
{
    return h2_conn_send.send_count > 0 || h2_conn_send.active_producers > 0 ||
           h2_conn_send.rst_retry_count > 0 || h2_conn_send.fifo_cleanup_count > 0;
}

bool client_session::http2_has_queued_peers()
{
    return h2_conn_send.send_count > 0 || h2_conn_send.rst_retry_count > 0;
}

bool client_session::http2_has_runnable_peer()
{
    for (auto *p = h2_conn_send.send_head; p; p = p->h2send.send_next)
    {
        auto peer = p->h2send.queue_owner.lock();
        if (peer && !http2_peer_is_blocked(peer)) return true;
    }
    return false;
}

asio::awaitable<void> client_session::http2_wait_send_event(std::chrono::nanoseconds wait_time)
{
    auto timer = std::make_shared<asio::steady_timer>(co_await asio::this_coro::executor);
    timer->expires_after(wait_time);
    std::atomic_store(&http2_consumer_timer, timer);
    http2_need_wakeup.store(true, std::memory_order_release);
    auto [wait_ec] = co_await timer->async_wait(asio::as_tuple(asio::use_awaitable));
    http2_need_wakeup.store(false, std::memory_order_release);
    std::atomic_store(&http2_consumer_timer, std::shared_ptr<asio::steady_timer>{});
    (void)wait_ec;
}

std::int64_t client_session::http2_conn_window_available() const
{
    std::lock_guard<std::mutex> lock(h2_send_window_mutex);
    return http2_conn_window_available_locked();
}

std::int64_t client_session::http2_conn_window_available_locked() const
{
    const auto available = h2_conn_send.window - static_cast<std::int64_t>(h2_conn_send.reserved);
    return available > 0 ? available : 0;
}

std::int64_t client_session::http2_peer_window_available(const httppeer *peer) const
{
    if (!peer) return 0;
    std::lock_guard<std::mutex> lock(h2_send_window_mutex);
    return peer->h2send.send_window - static_cast<std::int64_t>(peer->h2send.send_reserved);
}

unsigned long long client_session::http2_try_reserve_send_window(httppeer *peer,
                                                                  unsigned long long want,
                                                                  bool allow_sub_frame)
{
    if (!peer || want == 0) return 0;
    std::lock_guard<std::mutex> lock(h2_send_window_mutex);
    if (peer->h2send.stream_is_dead) return 0;
    const auto stream_available = peer->h2send.send_window - static_cast<std::int64_t>(peer->h2send.send_reserved);
    const auto conn_available = http2_conn_window_available_locked();
    const auto available = std::min(stream_available, conn_available);
    if (available <= 0) return 0;
    const auto cap = h2_conn_send.max_frame_size;
    if (!allow_sub_frame && want <= cap && static_cast<unsigned long long>(available) < want) return 0;
    const auto take = std::min<unsigned long long>(want, static_cast<unsigned long long>(available));
    peer->h2send.send_reserved += take;
    h2_conn_send.reserved += take;
    return take;
}

void client_session::http2_refund_peer_window(httppeer *peer)
{
    if (!peer) return;
    std::lock_guard<std::mutex> lock(h2_send_window_mutex);
    h2_conn_send.reserved -= std::min<unsigned long long>(
        peer->h2send.send_reserved, h2_conn_send.reserved);
    peer->h2send.send_reserved = 0;
}

void client_session::http2_commit_conn_window(unsigned long long bytes)
{
    if (bytes == 0) return;
    std::lock_guard<std::mutex> lock(h2_send_window_mutex);
    bytes = std::min<unsigned long long>(bytes, h2_conn_send.reserved);
    h2_conn_send.reserved -= bytes;
    h2_conn_send.window -= static_cast<std::int64_t>(bytes);
}

bool client_session::http2_apply_initial_window_size(unsigned int size, const http2parse &parser)
{
    std::lock_guard<std::mutex> lock(h2_send_window_mutex);
    const auto old_size = h2_conn_send.initial_window_size;
    const auto delta = static_cast<std::int64_t>(size) - static_cast<std::int64_t>(old_size);
    if (delta != 0)
    {
        auto valid = [delta](const httppeer *peer) {
            return peer->h2send.send_window + delta <= CONST_HTTP2_MAX_WINDOW;
        };
        for (auto &item : parser.http_data)
            if (!item.second->h2send.stream_is_dead && !valid(item.second.get())) return false;
        for (auto &item : parser.http_data_weak)
            if (auto peer = item.second.lock(); peer && !peer->h2send.stream_is_dead && !valid(peer.get())) return false;

        auto apply = [delta](httppeer *peer) {
            if (!peer->h2send.stream_is_dead) peer->h2send.send_window += delta;
        };
        for (auto &item : parser.http_data) apply(item.second.get());
        for (auto &item : parser.http_data_weak)
            if (auto peer = item.second.lock()) apply(peer.get());
    }
    h2_conn_send.initial_window_size = size;
    remote_initial_window_size.store(size, std::memory_order_relaxed);
    wake_ring_consumer();
    return true;
}

bool client_session::push_control_frame(const unsigned char *fr, unsigned int n)
{
    return http2_ring_queue->push(fr, n);
}

// ---- 单一 FIFO：HTTP/2 控制帧与流式响应帧 ----
// 流式 DATA 分片并预留窗口后入环，HEADERS 帧链整块入环；发送协程优先消费控制帧。

static unsigned int h2_frame_sid(std::string_view f)
{
    return (static_cast<unsigned int>(static_cast<unsigned char>(f[5]) & 0x7F) << 24) |
           (static_cast<unsigned int>(static_cast<unsigned char>(f[6])) << 16) |
           (static_cast<unsigned int>(static_cast<unsigned char>(f[7])) << 8) |
           static_cast<unsigned int>(static_cast<unsigned char>(f[8]));
}
static unsigned int h2_frame_payload_len(std::string_view f)
{
    return (static_cast<unsigned int>(static_cast<unsigned char>(f[0])) << 16) |
           (static_cast<unsigned int>(static_cast<unsigned char>(f[1])) << 8) |
           static_cast<unsigned int>(static_cast<unsigned char>(f[2]));
}
void h2_write_data_header(char *header, unsigned int sid, unsigned char flags, unsigned int len)
{
    header[0] = static_cast<char>((len >> 16) & 0xFF);
    header[1] = static_cast<char>((len >> 8) & 0xFF);
    header[2] = static_cast<char>(len & 0xFF);
    header[3] = 0x00;
    header[4] = static_cast<char>(flags);
    header[5] = static_cast<char>((sid >> 24) & 0x7F);
    header[6] = static_cast<char>((sid >> 16) & 0xFF);
    header[7] = static_cast<char>((sid >> 8) & 0xFF);
    header[8] = static_cast<char>(sid & 0xFF);
}
std::string h2_make_data_chunk(unsigned int sid, unsigned char flags, const char *payload, unsigned int len)
{
    std::string chunk;
    chunk.resize(9 + static_cast<size_t>(len));
    h2_write_data_header(chunk.data(), sid, flags, len);
    if (len > 0)
        std::memcpy(&chunk[9], payload, len);
    return chunk;
}

void client_session::http2_mark_stream_dead(const std::shared_ptr<httppeer> &peer)
{
    if (!peer) return;
    auto &state = peer->h2send;
    if (!state.stream_is_dead)
    {
        state.stream_is_dead = true;
        state.fifo_cleanup_pending = true;
        ++h2_conn_send.fifo_cleanup_count;
        state.blocked = false;
        state.blocked_deadline = {};
        peer->isclose.store(true, std::memory_order_release);
        http2_unlink_peer(peer.get());
        peer->grant_self_unlink();
    }
    if (!state.in_batch && !state.rst_pending)
    {
        http2_refund_peer_window(peer.get());
        http2_complete_active_peer(peer, false);
    }
    wake_ring_consumer();
}

void client_session::http2_content_stream_failed(const std::shared_ptr<httppeer> &peer, bool rst)
{
    if (!peer) return;
    if (rst && !isclose.load(std::memory_order_relaxed) && !http2_closing.load(std::memory_order_relaxed))
        http2_mark_peer_rst(peer, CONST_HTTP2_STREAM_ERROR_CANCEL);
    http2_mark_stream_dead(peer);
}

static void h2_content_fail(client_session *sess, const std::shared_ptr<httppeer> &peer, bool rst)
{
    auto self = sess->shared_from_this();
    asio::post(sess->strand_, [self = std::move(self), peer, rst]() mutable
    {
        self->http2_content_stream_failed(peer, rst);
    });
}

// 裸内容帧入环，一次尝试：owned 非空时移动槽内存储，否则直接拷贝调用方帧头。
// 环满 / 流已死 / 连接已关都返回 false。调用方决定 retry 策略和预算。
bool client_session::http2_try_push_content_raw(std::string_view frame, std::string *owned,
                                                const std::shared_ptr<httppeer> &peer)
{
    if (!peer || isclose.load(std::memory_order_relaxed) ||
        http2_closing.load(std::memory_order_relaxed) || peer->h2send.stream_is_dead)
        return false;
    bool pushed = false;
    peer->h2_fifo_frames_pending.fetch_add(1, std::memory_order_release);
    if (owned)
        pushed = http2_ring_queue->push(std::move(*owned));
    else
        pushed = http2_ring_queue->push(reinterpret_cast<const unsigned char *>(frame.data()),
                                           static_cast<unsigned int>(frame.size()));
    if (pushed)
    {
        if (http2_need_wakeup)
            wake_ring_consumer();
        return true;
    }
    peer->h2_fifo_frames_pending.fetch_sub(1, std::memory_order_release);
    http2_ring_overflow_count++;
    return false;
}

bool client_session::http2_push_content_raw_sync(std::string &&frame, const std::shared_ptr<httppeer> &peer)
{
    return http2_try_push_content_raw(frame, &frame, peer);
}

bool client_session::http2_push_content_raw_sync(const char *frame, unsigned int frame_size,
                                                 const std::shared_ptr<httppeer> &peer)
{
    return http2_try_push_content_raw(std::string_view(frame, frame_size), nullptr, peer);
}
// async 版：timer 挂在本连接 strand 上，每次让出都让消费者协程的写完成推进，不会互锁。
// 失败收尾可从业务线程直接标死流，喂入锁内必须复查死流。
asio::awaitable<bool> client_session::http2_push_content_raw(std::string frame,
                                                              std::shared_ptr<httppeer> peer)
{
    asio::steady_timer timer(co_await asio::this_coro::executor);
    unsigned int wait_interval_us = CONST_HTTP2_RING_FULL_WAIT_BASE;
    const auto deadline           = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
    for (;;)
    {
        if (!peer || isclose.load(std::memory_order_relaxed) ||
            http2_closing.load(std::memory_order_relaxed) || peer->h2send.stream_is_dead)
            co_return false;
        if (http2_try_push_content_raw(frame, &frame, peer))
            co_return true;
        if (std::chrono::steady_clock::now() >= deadline)
            co_return false;
        const unsigned char backlog = http2_ring_queue->has_size();
        wait_interval_us            = http2_ring_wait_adjust_us(wait_interval_us, backlog);
        if (backlog < CONST_HTTP2_RING_QUEUE_CAPACITY - 1)
            continue;
        timer.expires_after(std::chrono::microseconds(wait_interval_us));
        auto [wait_ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        (void)wait_ec;
    }
}

asio::awaitable<bool> client_session::http2_push_content_raw(const char *frame, unsigned int frame_size,
                                                              std::shared_ptr<httppeer> peer)
{
    asio::steady_timer timer(co_await asio::this_coro::executor);
    unsigned int wait_interval_us = CONST_HTTP2_RING_FULL_WAIT_BASE;
    const auto deadline           = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
    for (;;)
    {
        if (!peer || isclose.load(std::memory_order_relaxed) ||
            http2_closing.load(std::memory_order_relaxed) || peer->h2send.stream_is_dead)
            co_return false;
        if (http2_try_push_content_raw(std::string_view(frame, frame_size), nullptr, peer))
            co_return true;
        if (std::chrono::steady_clock::now() >= deadline)
            co_return false;
        const unsigned char backlog = http2_ring_queue->has_size();
        wait_interval_us            = http2_ring_wait_adjust_us(wait_interval_us, backlog);
        if (backlog < CONST_HTTP2_RING_QUEUE_CAPACITY - 1)
            continue;
        timer.expires_after(std::chrono::microseconds(wait_interval_us));
        auto [wait_ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        (void)wait_ec;
    }
}

unsigned int client_session::http2_send_content_frame(std::string_view frame,
                                                       const std::shared_ptr<httppeer> &peer)
{
    // 自保活：sync 路径的入环重试循环期间若 peer 引用断链，session 不能析构。
    // Self-preserve: keep session alive during sync push retry loops.
    auto self = shared_from_this();
    if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        return 0;
    if (frame.size() < 9 || http2_ring_queue == nullptr)
        return 0;
    const unsigned int sid         = h2_frame_sid(frame);
    const unsigned char ftype      = static_cast<unsigned char>(frame[3]);
    const unsigned char fflags     = static_cast<unsigned char>(frame[4]);
    const unsigned int payload_len = h2_frame_payload_len(frame);

    // HEADERS 帧链：整块入环，不做窗口记账。
    if (ftype != 0x00)
    {
        if (!peer || peer->stream_id != sid) return 0;
        h2_content_producer_guard producer_guard{this, peer};
        if (peer->h2send.stream_is_dead)
            return 0;
        std::string headers_frame(frame);
        if (http2_push_content_raw_sync(std::move(headers_frame), peer))
        {
            peer->h2send.headers_sent.store(true, std::memory_order_release);
            if (fflags & 0x01) peer->h2_end_stream_queued.store(true, std::memory_order_release);
            return static_cast<unsigned int>(frame.size());
        }
        // retry：环满 sleep 等消费者腾槽，死流/连接关不 retry。push 拒收保留源缓冲，
        // 复用同一帧。HEADERS 丢了响应就没有头，宁可等满预算再撤也不能第一次 false 就丢。
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
        for (;;)
        {
            if (peer->h2send.stream_is_dead || isclose.load(std::memory_order_relaxed) ||
                http2_closing.load(std::memory_order_relaxed))
                return 0;
            if (std::chrono::steady_clock::now() >= deadline)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (http2_push_content_raw_sync(std::move(headers_frame), peer))
            {
                peer->h2send.headers_sent.store(true, std::memory_order_release);
                if (fflags & 0x01) peer->h2_end_stream_queued.store(true, std::memory_order_release);
                return static_cast<unsigned int>(frame.size());
            }
        }
        if (!peer->h2send.stream_is_dead && !isclose.load(std::memory_order_relaxed) &&
            !http2_closing.load(std::memory_order_relaxed))
            h2_content_fail(this, peer, true);
        return 0;
    }
    // DATA（含零长 END_STREAM）：payload 直入，拆片/窗口记账在 payload 函数里。
    if (frame.size() - 9 != payload_len)
        return 0;
    const auto accepted = http2_send_data_payload(peer, frame.substr(9), fflags);
    return peer && accepted == payload_len && !peer->h2send.stream_is_dead
               ? static_cast<unsigned int>(frame.size())
               : 0;
}

unsigned int client_session::http2_send_data_payload(const std::shared_ptr<httppeer> &peer,
                                                     std::string_view payload, unsigned char end_flags)
{
    if (!peer) return 0;
    const unsigned int sid = peer->stream_id;
    // 失败收尾只更新锁保护状态，session 由调用方保持存活。
    auto self = shared_from_this();
    if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        return 0;
    if (http2_ring_queue == nullptr)
        return 0;

    h2_content_producer_guard producer_guard{this, peer};

    // 零长 DATA（END_STREAM 收尾）：不占窗口额度，整帧入环；入环失败按预算重试。
    if (payload.empty())
    {
        if (peer->h2send.stream_is_dead)
            return 0;
        char header[9];
        h2_write_data_header(header, sid, end_flags, 0);
        const auto push_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
        while (!http2_push_content_raw_sync(header, sizeof(header), peer))
        {
            if (peer->h2send.stream_is_dead || isclose.load(std::memory_order_relaxed) ||
                http2_closing.load(std::memory_order_relaxed))
                return 0;
            if (std::chrono::steady_clock::now() >= push_deadline)
            {
                h2_content_fail(this, peer, true);
                return 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (end_flags & 0x01) peer->h2_end_stream_queued.store(true, std::memory_order_release);
        return 0;
    }

    // 内部用 64 位累加避免 >4GB 回绕，返回处按函数签名收窄。
    unsigned long long accepted = 0;
    size_t off            = 0;
    while (off < payload.size())
    {
        if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed) ||
            peer->h2send.stream_is_dead)
            return static_cast<unsigned int>(accepted);
        const unsigned int cap =
            std::min(remote_max_frame_size.load(), static_cast<unsigned int>(CONST_HTTP2_SEND_FRAME_BUF));
        const size_t want      = std::min(payload.size() - off, static_cast<size_t>(cap));
        unsigned long long got = http2_try_reserve_send_window(peer.get(), want, true);
        // 非阻塞：窗口不够就把已收字节数还给调用方，由它决定稍后再发；停等只在 ring loop。
        if (got == 0)
            return static_cast<unsigned int>(accepted);
        const bool last   = (off + got >= payload.size());
        std::string chunk = h2_make_data_chunk(sid, last ? end_flags : static_cast<unsigned char>(end_flags & ~0x01), payload.data() + off, static_cast<unsigned int>(got));
        const auto push_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
        while (!http2_push_content_raw_sync(std::move(chunk), peer))
        {
            const bool gone = peer->h2send.stream_is_dead || isclose.load(std::memory_order_relaxed) ||
                              http2_closing.load(std::memory_order_relaxed);
            if (gone)
                return static_cast<unsigned int>(accepted);
            if (std::chrono::steady_clock::now() >= push_deadline)
            {
                h2_content_fail(this, peer, true);
                return static_cast<unsigned int>(accepted);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        off += static_cast<size_t>(got);
        accepted += got;
        if (last && (end_flags & 0x01))
            peer->h2_end_stream_queued.store(true, std::memory_order_release);
        time_limit.store(timeid() + 16);
    }
    return static_cast<unsigned int>(accepted);
}

asio::awaitable<unsigned int> client_session::async_http2_send_content_frame(
    std::string_view frame, std::shared_ptr<httppeer> peer)
{
    auto self = shared_from_this();
    http2_sender_count_guard count_guard(this);
    if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        co_return 0;
    if (frame.size() < 9 || http2_ring_queue == nullptr)
        co_return 0;
    const unsigned int sid         = h2_frame_sid(frame);
    const unsigned char ftype      = static_cast<unsigned char>(frame[3]);
    const unsigned char fflags     = static_cast<unsigned char>(frame[4]);
    const unsigned int payload_len = h2_frame_payload_len(frame);

    if (ftype != 0x00)
    {
        if (!peer || peer->stream_id != sid) co_return 0;
        h2_content_producer_guard producer_guard{this, peer};
        if (peer->h2send.stream_is_dead)
            co_return 0;
        if (co_await http2_push_content_raw(std::string(frame), peer))
        {
            peer->h2send.headers_sent.store(true, std::memory_order_release);
            if (fflags & 0x01) peer->h2_end_stream_queued.store(true, std::memory_order_release);
            co_return static_cast<unsigned int>(frame.size());
        }
        if (!peer->h2send.stream_is_dead && !isclose.load(std::memory_order_relaxed) &&
            !http2_closing.load(std::memory_order_relaxed))
            http2_content_stream_failed(peer, true);
        co_return 0;
    }
    if (frame.size() - 9 != payload_len)
        co_return 0;
    const auto accepted = co_await async_http2_send_data_payload(peer, frame.substr(9), fflags);
    co_return peer && accepted == payload_len && !peer->h2send.stream_is_dead
                   ? static_cast<unsigned int>(frame.size())
                   : 0;
}

asio::awaitable<unsigned int> client_session::async_http2_send_data_payload(
    std::shared_ptr<httppeer> peer, std::string_view payload, unsigned char end_flags)
{
    if (!peer) co_return 0;
    const unsigned int sid = peer->stream_id;
    auto self = shared_from_this();
    http2_sender_count_guard count_guard(this);
    if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        co_return 0;
    if (http2_ring_queue == nullptr)
        co_return 0;

    h2_content_producer_guard producer_guard{this, peer};

    if (payload.empty())
    {
        if (peer->h2send.stream_is_dead)
            co_return 0;
        char header[9];
        h2_write_data_header(header, sid, end_flags, 0);
        if (co_await http2_push_content_raw(header, sizeof(header), peer))
        {
            if (end_flags & 0x01) peer->h2_end_stream_queued.store(true, std::memory_order_release);
            co_return 0;
        }
        if (!peer->h2send.stream_is_dead && !isclose.load(std::memory_order_relaxed) &&
            !http2_closing.load(std::memory_order_relaxed))
            http2_content_stream_failed(peer, true);
        co_return 0;
    }

    // 内部用 64 位累加避免 >4GB 回绕，返回处按函数签名收窄。
    unsigned long long accepted = 0;
    size_t off            = 0;
    auto stall_deadline   = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
    while (off < payload.size())
    {
        if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed) ||
            peer->h2send.stream_is_dead)
            co_return static_cast<unsigned int>(accepted);
        const unsigned int cap =
            std::min(remote_max_frame_size.load(), static_cast<unsigned int>(CONST_HTTP2_SEND_FRAME_BUF));
        const size_t want      = std::min(payload.size() - off, static_cast<size_t>(cap));
        unsigned long long got = http2_try_reserve_send_window(peer.get(), want, true);
        if (got == 0)
        {
            if (std::chrono::steady_clock::now() >= stall_deadline)
            {
                http2_content_stream_failed(peer, true);
                co_return static_cast<unsigned int>(accepted);
            }
            asio::steady_timer retry(co_await asio::this_coro::executor);
            retry.expires_after(std::chrono::milliseconds(1));
            co_await retry.async_wait(asio::use_awaitable);
            continue;
        }
        const bool last   = (off + got >= payload.size());
        std::string chunk = h2_make_data_chunk(sid, last ? end_flags : static_cast<unsigned char>(end_flags & ~0x01), payload.data() + off, static_cast<unsigned int>(got));
        if (!co_await http2_push_content_raw(std::move(chunk), peer))
        {
            const bool gone = peer->h2send.stream_is_dead || isclose.load(std::memory_order_relaxed) ||
                              http2_closing.load(std::memory_order_relaxed);
            http2_content_stream_failed(peer, !gone);
            co_return static_cast<unsigned int>(accepted);
        }
        off += static_cast<size_t>(got);
        accepted += got;
        if (last && (end_flags & 0x01))
            peer->h2_end_stream_queued.store(true, std::memory_order_release);
        stall_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
        time_limit.store(timeid() + 16);
    }
    co_return static_cast<unsigned int>(accepted);
}

asio::awaitable<unsigned int> client_session::async_send_writer(const unsigned char *buffer, unsigned int buffersize,
                                                                 std::shared_ptr<httppeer> peer)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    if (httpv == 2)
    {
        co_return co_await async_http2_send_content_frame(
            std::string_view(reinterpret_cast<const char *>(buffer), buffersize), peer);
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

asio::awaitable<unsigned int> client_session::async_send_writer(std::string_view msg,
                                                                 std::shared_ptr<httppeer> peer)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    if (httpv == 2)
    {
        co_return co_await async_http2_send_content_frame(msg, peer);
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

asio::awaitable<unsigned int> client_session::async_send_writer(std::string_view head, std::string_view body)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    try
    {
        if (head.size() == 0 && body.size() == 0)
        {
            co_return 0;
        }
        std::array<asio::const_buffer, 2> bufs{asio::buffer(head), asio::buffer(body)};
        unsigned int n = 0;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                n = co_await asio::async_write(*sslsocket, bufs, asio::use_awaitable);
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
                n = co_await asio::async_write(*socket, bufs, asio::use_awaitable);
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

asio::awaitable<unsigned int> client_session::async_send_writer(const std::string &msg,
                                                                 std::shared_ptr<httppeer> peer)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    if (httpv == 2)
    {
        co_return co_await async_http2_send_content_frame(msg, peer);
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
    http2_cancel_all_pending_peers(false);
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
    if (msg.size() > std::numeric_limits<unsigned int>::max())
    {
        return false;
    }

    auto *q = http2_ring_queue.get();
    const unsigned long long size = static_cast<unsigned long long>(msg.size());
    unsigned long long backlog = q->bytes.load(std::memory_order_relaxed);
    for (;;)
    {
        if (backlog > 0 && (backlog > MQTT_SEND_RING_BYTE_LIMIT ||
                            size > MQTT_SEND_RING_BYTE_LIMIT - backlog))
        {
            http2_ring_overflow_count.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        if (q->bytes.compare_exchange_weak(backlog, backlog + size,
                                           std::memory_order_release,
                                           std::memory_order_relaxed))
        {
            break;
        }
    }

    // 先记账再入环：帧一旦发布到环上，消费者就可能写完并做减法；若在 push 之后才累加，
    // 消费者读到的旧值会让这一帧的字节永久留在 bytes 里，累计过闸后本连接再也写不出去。
    if (!q->push(reinterpret_cast<const unsigned char *>(msg.data()),
                 static_cast<unsigned int>(size)))
    {
        q->bytes.fetch_sub(size, std::memory_order_relaxed);
        http2_ring_overflow_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    if (http2_need_wakeup)
    {
        wake_ring_consumer();
    }
    return true;
}

bool client_session::post_write(const unsigned char *buf, std::size_t len)
{
    if (isclose || iserror || !buf || len > std::numeric_limits<unsigned int>::max())
        return false;
    return post_write(std::string_view(reinterpret_cast<const char *>(buf), len));
}

}// namespace http
