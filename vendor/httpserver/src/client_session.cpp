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

unsigned int client_session::send_writer(const std::string &msg)
{
    if (isclose)
    {
        return 0;
    }
    if (httpv == 2)
    {
        return http2_send_content_frame(msg);
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
    if (httpv == 2)
    {
        return http2_send_content_frame(msg);
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
    fr[9]                = (last_stream_id >> 24) & 0xFF;
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
    // 关闭冲刷：置 closing + cancel 全部等待者，发送生产者在循环边界静音退出。
    http2_close_waiters();
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
    if (httpv == 2)
    {
        return http2_send_content_frame(std::string_view(reinterpret_cast<const char *>(buffer), buffersize));
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
        auto timer = std::move(http2_consumer_timer);
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
        if (timer)
        {
            asio::dispatch(timer->get_executor(), [timer = std::move(timer)]() mutable
                           { timer->cancel(); });
        }
    }
    catch (...)
    {
        DEBUG_LOG("peer_session user_code_handler_call error");
    }
}

void client_session::http2_register_pending_peer(const std::shared_ptr<httppeer> &peer)
{
    {
        std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
        if (peer->h2send.registered)
            return;
        peer->h2send.registered = true;
        http2_pending_peers.push_back(pending_h2_peer{peer, false, {}});
    }
    wake_ring_consumer();
}

std::shared_ptr<httppeer> client_session::http2_take_next_peer()
{
    std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
    const std::size_t scan = http2_pending_peers.size();
    for (std::size_t i = 0; i < scan; ++i)
    {
        auto item = std::move(http2_pending_peers.front());
        http2_pending_peers.pop_front();
        if (item.peer->isclose.load(std::memory_order_relaxed))
        {
            item.peer->h2send.registered = false;
            if (item.peer->h2send.completion)
                item.peer->h2send.completion->cancel();
            continue;
        }
        if (item.blocked)
        {
            http2_pending_peers.push_back(std::move(item));
            continue;
        }
        http2_active_peer = item.peer;
        return item.peer;
    }
    return nullptr;
}

void client_session::http2_requeue_peer(const std::shared_ptr<httppeer> &peer, bool blocked)
{
    if (blocked && peer->h2send.blocked_deadline.time_since_epoch().count() == 0)
        peer->h2send.blocked_deadline = std::chrono::steady_clock::now() +
                                        std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
    if (!blocked)
        peer->h2send.blocked_deadline = {};
    {
        std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
        if (http2_active_peer == peer)
            http2_active_peer.reset();
        http2_pending_peers.push_back(pending_h2_peer{peer, blocked, peer->h2send.blocked_deadline});
    }
}

bool client_session::http2_begin_peer_refill(const std::shared_ptr<httppeer> &peer)
{
    std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
    if (!peer->h2send.registered || peer->h2send.refill_pending ||
        peer->isclose.load(std::memory_order_relaxed) || isclose.load(std::memory_order_relaxed) ||
        http2_closing.load(std::memory_order_relaxed))
        return false;
    if (http2_active_peer == peer)
        http2_active_peer.reset();
    peer->h2send.refill_pending = true;
    http2_refilling_peers.emplace(peer->stream_id, peer);
    return true;
}

void client_session::http2_finish_peer_refill(const std::shared_ptr<httppeer> &peer, bool ready)
{
    std::shared_ptr<h2_borrow_state> completion;
    bool queued = false;
    {
        std::lock_guard<std::mutex> dl(http2_dead_streams_mutex);
        std::lock_guard<std::mutex> pl(http2_pending_peers_mutex);
        auto it = http2_refilling_peers.find(peer->stream_id);
        if (it == http2_refilling_peers.end() || it->second != peer)
            return;
        http2_refilling_peers.erase(it);
        peer->h2send.refill_pending = false;
        if (ready && peer->h2send.registered && !peer->isclose.load(std::memory_order_relaxed) &&
            !isclose.load(std::memory_order_relaxed) && !http2_closing.load(std::memory_order_relaxed) &&
            http2_dead_streams.find(peer->stream_id) == http2_dead_streams.end())
        {
            try
            {
                http2_pending_peers.push_back(pending_h2_peer{peer, false, {}});
                queued = true;
            }
            catch (...)
            {
            }
        }
        if (!queued)
        {
            peer->h2send.registered = false;
            peer->h2send.blocked_deadline = {};
            completion = peer->h2send.completion;
            http2_try_reap_dead_mark_locked(peer->stream_id);
        }
    }
    if (queued)
        wake_ring_consumer();
    else if (completion)
        completion->cancel();
}

bool client_session::http2_has_refilling_peers()
{
    std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
    return !http2_refilling_peers.empty();
}

void client_session::http2_complete_active_peer(const std::shared_ptr<httppeer> &peer, bool success)
{
    std::shared_ptr<h2_borrow_state> completion;
    {
        std::lock_guard<std::mutex> dl(http2_dead_streams_mutex);
        std::lock_guard<std::mutex> pl(http2_pending_peers_mutex);
        if (http2_active_peer == peer)
            http2_active_peer.reset();
        peer->h2send.registered = false;
        peer->h2send.blocked_deadline = {};
        completion = peer->h2send.completion;
        http2_try_reap_dead_mark_locked(peer->stream_id);
    }
    if (completion)
    {
        if (success)
            completion->release();
        else
            completion->cancel();
    }
}

void client_session::http2_cancel_pending_peer(unsigned int sid)
{
    std::shared_ptr<h2_borrow_state> completion;
    {
        std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
        for (auto it = http2_pending_peers.begin(); it != http2_pending_peers.end(); ++it)
        {
            if (it->peer->stream_id != sid)
                continue;
            it->peer->h2send.registered = false;
            completion = std::move(it->peer->h2send.completion);
            http2_pending_peers.erase(it);
            break;
        }
        auto refill = http2_refilling_peers.find(sid);
        if (refill != http2_refilling_peers.end())
            refill->second->isclose = true;
    }
    if (completion)
        completion->cancel();
    wake_ring_consumer();
}

void client_session::http2_cancel_active_peer()
{
    std::shared_ptr<httppeer> active;
    {
        std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
        active = http2_active_peer;
    }
    if (active)
        http2_complete_active_peer(active, false);
}

void client_session::http2_cancel_all_pending_peers(bool wake)
{
    // 活跃 peer 的合并写入仍引用 output，必须等写入退出后才能取消完成通知。
    std::deque<pending_h2_peer> peers;
    {
        std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
        peers.swap(http2_pending_peers);
        for (const auto &item : http2_refilling_peers)
            item.second->isclose = true;
    }
    for (auto &item : peers)
    {
        item.peer->h2send.registered = false;
        if (item.peer->h2send.completion)
            item.peer->h2send.completion->cancel();
    }
    if (wake)
        wake_ring_consumer();
}

void client_session::http2_begin_content_stream(unsigned int sid)
{
    std::lock_guard<std::mutex> lk(http2_dead_streams_mutex);
    ++http2_content_producers[sid];
}

void client_session::http2_end_content_stream(unsigned int sid)
{
    std::lock_guard<std::mutex> dl(http2_dead_streams_mutex);
    auto it = http2_content_producers.find(sid);
    if (it != http2_content_producers.end())
    {
        if (--it->second == 0)
            http2_content_producers.erase(it);
    }
    std::lock_guard<std::mutex> pl(http2_pending_peers_mutex);
    http2_try_reap_dead_mark_locked(sid);
}

client_session::h2_content_producer_guard::h2_content_producer_guard(client_session *session, unsigned int stream_id)
    : sess(session), sid(stream_id)
{
    sess->http2_begin_content_stream(sid);
}

client_session::h2_content_producer_guard::~h2_content_producer_guard()
{
    sess->http2_end_content_stream(sid);
}

std::vector<std::shared_ptr<httppeer>> client_session::http2_cancel_expired_peers()
{
    std::vector<std::shared_ptr<httppeer>> expired;
    const auto now = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
        for (auto it = http2_pending_peers.begin(); it != http2_pending_peers.end();)
        {
            if (!it->blocked || it->deadline == std::chrono::steady_clock::time_point{} || now < it->deadline)
            {
                ++it;
                continue;
            }
            it->peer->isclose = true;
            it->peer->h2send.registered = false;
            if (it->peer->h2send.completion)
                it->peer->h2send.completion->cancel();
            expired.push_back(it->peer);
            it = http2_pending_peers.erase(it);
        }
    }
    return expired;
}

void client_session::http2_unblock_pending_peers(bool connection, unsigned int sid)
{
    bool changed = false;
    {
        std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
        for (auto &item : http2_pending_peers)
        {
            if (item.blocked && (connection || item.peer->stream_id == sid))
            {
                item.blocked = false;
                item.deadline = {};
                item.peer->h2send.blocked_deadline = {};
                changed = true;
            }
        }
    }
    if (changed)
        wake_ring_consumer();
}

bool client_session::http2_has_pending_peers()
{
    std::lock_guard<std::mutex> dl(http2_dead_streams_mutex);
    std::lock_guard<std::mutex> pl(http2_pending_peers_mutex);
    return http2_active_peer != nullptr || !http2_pending_peers.empty() ||
           !http2_refilling_peers.empty() || !http2_content_producers.empty();
}

bool client_session::http2_has_queued_peers()
{
    std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
    return http2_active_peer != nullptr || !http2_pending_peers.empty();
}

bool client_session::http2_has_runnable_peer()
{
    std::lock_guard<std::mutex> lk(http2_pending_peers_mutex);
    for (const auto &item : http2_pending_peers)
    {
        if (!item.blocked && !item.peer->isclose.load(std::memory_order_relaxed))
            return true;
    }
    return false;
}

asio::awaitable<void> client_session::http2_wait_send_event(std::chrono::nanoseconds wait_time)
{
    auto timer = std::make_shared<asio::steady_timer>(co_await asio::this_coro::executor);
    timer->expires_after(wait_time);
    {
        std::lock_guard<std::mutex> lk(wake_ring_consumer_mutex);
        http2_need_wakeup = true;
        http2_consumer_timer = timer;
        if ((http2_ring_queue && http2_ring_queue->has_size() != 0) || http2_has_runnable_peer())
        {
            http2_need_wakeup = false;
            http2_consumer_timer.reset();
            co_return;
        }
    }
    auto [wait_ec] = co_await timer->async_wait(asio::as_tuple(asio::use_awaitable));
    (void)wait_ec;
    std::lock_guard<std::mutex> lk(wake_ring_consumer_mutex);
    if (http2_consumer_timer == timer)
        http2_consumer_timer.reset();
    http2_need_wakeup = false;
}

// ---- h2 停等原语 ----
// 都在本连接 strand 上跑：容器不用锁，cancel 和 async_wait 同一个 executor。

asio::awaitable<bool> client_session::http2_wait_window_episode(unsigned int sid, const std::atomic_bool &dead)
{
    asio::steady_timer timer(co_await asio::this_coro::executor);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
    // 注销两边都清：timer 可能换过登记边，残留条目会被 cancel 成悬垂指针。
    auto unregister = [&]()
    {
        auto range = http2_stream_window_waiters.equal_range(sid);
        for (auto it = range.first; it != range.second; )
        {
            if (it->second == &timer)
                it = http2_stream_window_waiters.erase(it);  // multimap::erase returns successor
            else
                ++it;
        }
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
            auto it            = stream_send_window.find(sid);
            unsigned int avail = (it == stream_send_window.end()) ? remote_initial_window_size.load() : it->second;
            if (avail > 0)
                co_return true;
        }
        unregister();
        if (conn_empty)
            http2_conn_window_waiters.push_back(&timer);
        else
            http2_stream_window_waiters.emplace(sid, &timer);
        timer.expires_after(left);
        auto [wait_ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        unregister();
        if (http2_closing.load(std::memory_order_relaxed) || dead.load(std::memory_order_relaxed))
            co_return false;
        if (wait_ec != asio::error::operation_aborted)
        {
            // 自然到期与 WU 到达可能撞在同一拍：返回前再查一次两级窗口，
            // 有额度就放调用方去 reserve，不误撤流。
            const bool still_conn_empty = window_update_num.load() <= has_send_update_num.load();
            if (!still_conn_empty)
            {
                std::lock_guard<std::mutex> lk(stream_send_window_mutex);
                auto it            = stream_send_window.find(sid);
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
    http2_unblock_pending_peers(true, 0);
    http2_wake_sync_window_waiters();
    if (http2_has_pending_peers())
        wake_ring_consumer();
}

void client_session::http2_cancel_stream_waiter(unsigned int sid)
{
    auto range = http2_stream_window_waiters.equal_range(sid);
    for (auto it = range.first; it != range.second; ++it)
        it->second->cancel();
    http2_unblock_pending_peers(false, sid);
    http2_wake_sync_window_waiters();
    if (http2_has_pending_peers())
        wake_ring_consumer();
}

void client_session::http2_cancel_all_stream_waiters()
{
    for (auto &kv : http2_stream_window_waiters)
        kv.second->cancel();
    http2_unblock_pending_peers(true, 0);
    http2_wake_sync_window_waiters();
}

void client_session::http2_close_waiters()
{
    http2_closing.store(true, std::memory_order_relaxed);
    for (auto *t : http2_conn_window_waiters)
        t->cancel();
    for (auto &kv : http2_stream_window_waiters)
        kv.second->cancel();
    http2_wake_sync_window_waiters();
    http2_cancel_all_pending_peers();
}

void client_session::http2_wake_sync_window_waiters()
{
    // 只取 sync 世代锁：等待者挂 CV 时也只持这把锁（reserve 在锁外做），
    // 通知方可能持 stream_send_window_mutex，但等待者绝不持世代锁去取它，无环。
    {
        std::lock_guard<std::mutex> lk(http2_sync_window_mutex);
        ++http2_sync_window_gen;
    }
    http2_sync_window_cv.notify_all();
}

asio::awaitable<bool> client_session::http2_send_rst(unsigned int s_stream_id, unsigned int stream_error_code)
{
    // RST 入控制环，满则就地重试：起始 1ms，按积压自适应（醒早翻倍/睡过头减半），2 秒预算。
    asio::steady_timer timer(co_await asio::this_coro::executor);
    unsigned int wait_interval_us = CONST_HTTP2_RING_FULL_WAIT_BASE;
    const auto deadline           = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    for (;;)
    {
        if (isclose || iserror)
            co_return false;
        if (push_rst_stream(s_stream_id, stream_error_code))
            co_return true;
        http2_ring_queue_drop_count++;
        if (std::chrono::steady_clock::now() >= deadline)
            co_return false;
        const unsigned char backlog = http2_ring_queue->has_size();
        wait_interval_us            = http2_ring_wait_adjust_us(wait_interval_us, backlog);
        // 有空槽就立即重试打满，不睡；只有环仍满才按自适应间隔等。
        if (backlog < CONST_HTTP2_RING_QUEUE_CAPACITY - 1)
            continue;
        timer.expires_after(std::chrono::microseconds(wait_interval_us));
        auto [wait_ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        (void)wait_ec;
    }
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
    if (!push_control_frame(fr, 13))
        return false;
    if (http2_need_wakeup)
        wake_ring_consumer();
    return true;
}

bool client_session::push_control_frame(const unsigned char *fr, unsigned int n)
{
    // 控制帧与流式帧共用喂入锁，但不检查流式帧积压上限：持锁直推 FIFO 环，
    // 环满时返回 false，由调用方重试。
    std::lock_guard<std::mutex> feed(http2_feed_mutex);
    return http2_ring_queue->push(fr, n);
}

// ---- 单一 FIFO：HTTP/2 控制帧与流式响应帧 ----
// 流式 DATA 分片并预留窗口后入环，HEADERS 帧链整块入环；发送协程优先消费控制帧。

static unsigned int h2_frame_sid(std::string_view f)
{
    return (static_cast<unsigned int>(static_cast<unsigned char>(f[5])) << 24) |
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

void client_session::http2_mark_stream_dead(unsigned int sid)
{
    {
        std::lock_guard<std::mutex> lk(http2_dead_streams_mutex);
        http2_dead_streams.insert(sid);
    }
    http2_cancel_pending_peer(sid);
    std::lock_guard<std::mutex> dl(http2_dead_streams_mutex);
    std::lock_guard<std::mutex> pl(http2_pending_peers_mutex);
    http2_try_reap_dead_mark_locked(sid);
}

bool client_session::http2_stream_is_dead(unsigned int sid)
{
    std::lock_guard<std::mutex> lk(http2_dead_streams_mutex);
    return http2_dead_streams.find(sid) != http2_dead_streams.end();
}

void client_session::http2_try_reap_dead_mark_locked(unsigned int sid)
{
    if (http2_dead_streams.find(sid) == http2_dead_streams.end())
        return;
    if (http2_content_producers.find(sid) != http2_content_producers.end())
        return;
    auto it = http2_inflight_frames.find(sid);
    if (it != http2_inflight_frames.end() && it->second > 0)
        return;
    if (it != http2_inflight_frames.end())
        http2_inflight_frames.erase(it);
    if ((http2_active_peer && http2_active_peer->stream_id == sid) ||
        http2_refilling_peers.find(sid) != http2_refilling_peers.end() ||
        std::any_of(http2_pending_peers.begin(), http2_pending_peers.end(),
                    [sid](const pending_h2_peer &item) { return item.peer->stream_id == sid; }))
        return;
    http2_dead_streams.erase(sid);
}

void client_session::http2_note_frame_dequeued(unsigned int sid, unsigned char type)
{
    std::lock_guard<std::mutex> dl(http2_dead_streams_mutex);
    std::lock_guard<std::mutex> pl(http2_pending_peers_mutex);
    if (sid != 0 && (type == 0x00 || type == 0x01))
    {
        auto it = http2_inflight_frames.find(sid);
        if (it != http2_inflight_frames.end())
        {
            if (--it->second == 0)
                http2_inflight_frames.erase(it);
        }
    }
    http2_try_reap_dead_mark_locked(sid);
}

void client_session::http2_content_stream_failed(unsigned int sid, unsigned long long refund_bytes, bool rst)
{
    if (refund_bytes > 0)
    {
        get_server_app().http2_refund_send_window(this, sid, refund_bytes);
    }
    http2_mark_stream_dead(sid);
    {
        std::lock_guard<std::mutex> lk(stream_send_window_mutex);
        stream_send_window.erase(sid);
    }
    if (rst && !isclose.load(std::memory_order_relaxed) && !http2_closing.load(std::memory_order_relaxed))
    {
        // 控制环满时一次 push 就丢 RST，这条流会既无响应也无收尾。改用带 2 秒
        // 自适应重试的 http2_send_rst。本函数是 strand 上的同步函数，用 co_spawn
        // 投递到同一 strand，不跨线程碰等待者容器；环满不再等于「连接过载」，
        // 流已标死，同连接其它流照常服务。
        auto self = get_ptr();
        asio::co_spawn(strand_,
                       [self, sid]() -> asio::awaitable<void>
                       {
                           if (!co_await self->http2_send_rst(sid, CONST_HTTP2_STREAM_ERROR_CANCEL))
                           {
                               DEBUG_LOG("http2 RST dropped after retry sid=%u", sid);
                           }
                       },
                       asio::detached);
    }
    {
        std::lock_guard<std::mutex> dl(http2_dead_streams_mutex);
        std::lock_guard<std::mutex> pl(http2_pending_peers_mutex);
        http2_try_reap_dead_mark_locked(sid);
    }
}

// sync 生产者（业务池线程）的失败收尾：先标死流挡住自己后续帧，再把退款/撤流投递到
// strand——http2_refund_send_window 会 cancel 窗口停等者，那些容器是 strand 专属的。
static void h2_content_fail_post(client_session *sess, unsigned int sid, unsigned long long refund_bytes, bool rst)
{
    sess->http2_mark_stream_dead(sid);
    auto self = sess->get_ptr();
    asio::post(sess->strand_, [self, sid, refund_bytes, rst]()
               { self->http2_content_stream_failed(sid, refund_bytes, rst); });
}

// 裸内容帧入环，一次尝试：owned 非空时移动槽内存储，否则直接拷贝调用方帧头。
// 环满 / 流已死 / 连接已关都返回 false。调用方决定 retry 策略和预算。
bool client_session::http2_try_push_content_raw(std::string_view frame, std::string *owned, unsigned int sid)
{
    if (isclose.load(std::memory_order_relaxed) ||
        http2_closing.load(std::memory_order_relaxed) || http2_stream_is_dead(sid))
        return false;
    bool pushed = false;
    {
        std::lock_guard<std::mutex> feed(http2_feed_mutex);
        if (isclose.load(std::memory_order_relaxed) ||
            http2_closing.load(std::memory_order_relaxed) || http2_stream_is_dead(sid))
            return false;
        // 必须先登记再发布 tail_：消费者不取喂入锁，可能在下一次调度前就出队。
        std::lock_guard<std::mutex> dl(http2_dead_streams_mutex);
        const bool counted = frame.size() >= 9 && sid != 0 &&
                             (static_cast<unsigned char>(frame[3]) == 0x00 ||
                              static_cast<unsigned char>(frame[3]) == 0x01);
        if (counted)
            ++http2_inflight_frames[sid];
        if (owned)
            pushed = http2_ring_queue->push(std::move(*owned));
        else
            pushed = http2_ring_queue->push(reinterpret_cast<const unsigned char *>(frame.data()),
                                               static_cast<unsigned int>(frame.size()));
        if (!pushed && counted)
        {
            auto it = http2_inflight_frames.find(sid);
            if (--it->second == 0)
                http2_inflight_frames.erase(it);
        }
    }
    if (pushed)
    {
        // 唤醒放在喂入锁外，避免持锁 dispatch 就地复活消费者后重入喂入锁。
        if (http2_need_wakeup)
            wake_ring_consumer();
        return true;
    }
    http2_ring_overflow_count++;
    return false;
}

bool client_session::http2_push_content_raw_sync(std::string &&frame, unsigned int sid)
{
    return http2_try_push_content_raw(frame, &frame, sid);
}

bool client_session::http2_push_content_raw_sync(const char *frame, unsigned int frame_size, unsigned int sid)
{
    return http2_try_push_content_raw(std::string_view(frame, frame_size), nullptr, sid);
}
// async 版：timer 挂在本连接 strand 上，每次让出都让消费者协程的写完成推进，不会互锁。
// 注：h2_content_fail_post 从业务线程直接调 http2_mark_stream_dead，打破 strand 串行不变量，
// 喂入锁内必须复查死流（与 sync 版 http2_push_content_raw_sync 对齐）。
asio::awaitable<bool> client_session::http2_push_content_raw(std::string frame, unsigned int sid)
{
    asio::steady_timer timer(co_await asio::this_coro::executor);
    unsigned int wait_interval_us = CONST_HTTP2_RING_FULL_WAIT_BASE;
    const auto deadline           = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
    for (;;)
    {
        if (isclose.load(std::memory_order_relaxed) ||
            http2_closing.load(std::memory_order_relaxed) || http2_stream_is_dead(sid))
            co_return false;
        if (http2_try_push_content_raw(frame, &frame, sid))
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

asio::awaitable<bool> client_session::http2_push_content_raw(const char *frame, unsigned int frame_size, unsigned int sid)
{
    asio::steady_timer timer(co_await asio::this_coro::executor);
    unsigned int wait_interval_us = CONST_HTTP2_RING_FULL_WAIT_BASE;
    const auto deadline           = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
    for (;;)
    {
        if (isclose.load(std::memory_order_relaxed) ||
            http2_closing.load(std::memory_order_relaxed) || http2_stream_is_dead(sid))
            co_return false;
        if (http2_try_push_content_raw(std::string_view(frame, frame_size), nullptr, sid))
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

unsigned int client_session::http2_send_content_frame(std::string_view frame)
{
    // 自保活：sync 路径的 CV 停等/重试循环期间若 peer 引用断链，session 不能析构。
    // Self-preserve: keep session alive during sync wait_for / retry loops.
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
        h2_content_producer_guard producer_guard{this, sid};
        if (http2_stream_is_dead(sid))
            return 0;
        std::string headers_frame(frame);
        if (http2_push_content_raw_sync(std::move(headers_frame), sid))
            return static_cast<unsigned int>(frame.size());
        // retry：环满 sleep 等消费者腾槽，死流/连接关不 retry。push 拒收保留源缓冲，
        // 复用同一帧。HEADERS 丢了响应就没有头，宁可等满预算再撤也不能第一次 false 就丢。
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
        for (;;)
        {
            if (http2_stream_is_dead(sid) || isclose.load(std::memory_order_relaxed) ||
                http2_closing.load(std::memory_order_relaxed))
                return 0;
            if (std::chrono::steady_clock::now() >= deadline)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (http2_push_content_raw_sync(std::move(headers_frame), sid))
                return static_cast<unsigned int>(frame.size());
        }
        if (!http2_stream_is_dead(sid) && !isclose.load(std::memory_order_relaxed) &&
            !http2_closing.load(std::memory_order_relaxed))
            h2_content_fail_post(this, sid, 0, true);
        return 0;
    }
    // DATA（含零长 END_STREAM）：payload 直入，拆片/窗口记账在 payload 函数里。
    if (frame.size() - 9 != payload_len)
        return 0;
    return http2_send_data_payload(sid, frame.substr(9), fflags);
}

unsigned int client_session::http2_send_data_payload(unsigned int sid, std::string_view payload, unsigned char end_flags)
{
    // 自保活：sync 路径的 CV 停等（wait_for）期间 session 不能析构。
    // Self-preserve: keep session alive during sync window wait_for.
    auto self = shared_from_this();
    if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        return 0;
    if (http2_ring_queue == nullptr)
        return 0;

    h2_content_producer_guard producer_guard{this, sid};

    // 零长 DATA（END_STREAM 收尾）：不占窗口额度，整帧入环。
    if (payload.empty())
    {
        if (http2_stream_is_dead(sid))
            return 0;
        char header[9];
        h2_write_data_header(header, sid, end_flags, 0);
        if (http2_push_content_raw_sync(header, sizeof(header), sid))
            return 9;
        // retry：END_STREAM 丢了流永远不结束，必须等。帧头固定，重试直接复用。
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
        for (;;)
        {
            if (http2_stream_is_dead(sid) || isclose.load(std::memory_order_relaxed) ||
                http2_closing.load(std::memory_order_relaxed))
                return 0;
            if (std::chrono::steady_clock::now() >= deadline)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (http2_push_content_raw_sync(header, sizeof(header), sid))
                return 9;
        }
        if (!http2_stream_is_dead(sid) && !isclose.load(std::memory_order_relaxed) &&
            !http2_closing.load(std::memory_order_relaxed))
            h2_content_fail_post(this, sid, 0, true);
        return 0;
    }

    auto &app             = get_server_app();
    // 内部用 64 位累加避免 >4GB 回绕，返回处按函数签名收窄。
    unsigned long long accepted = 0;
    size_t off            = 0;
    while (off < payload.size())
    {
        if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed) ||
            http2_stream_is_dead(sid))
            return static_cast<unsigned int>(accepted);
        const unsigned int cap =
            std::min(remote_max_frame_size.load(), static_cast<unsigned int>(CONST_HTTP2_SEND_FRAME_BUF));
        const size_t want      = std::min(payload.size() - off, static_cast<size_t>(cap));
        unsigned long long got = app.http2_reserve_send_window(this, sid, want);
        if (got == 0)
        {
            // 同步窗口停等：挂 CV 等唤醒边（WU/IWS/退款/关闭都会递增世代并 notify），
            // CONST_HTTP2_WINDOW_STALL_TIMEOUT 秒预算；到期撤流（RST CANCEL），连接保留。
            const auto deadline =
                std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
            for (;;)
            {
                if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed) ||
                    http2_stream_is_dead(sid))
                    return static_cast<unsigned int>(accepted);
                const auto left = deadline - std::chrono::steady_clock::now();
                if (left <= std::chrono::steady_clock::duration::zero())
                {
                    if (!http2_stream_is_dead(sid) && !isclose.load(std::memory_order_relaxed) &&
                        !http2_closing.load(std::memory_order_relaxed))
                        h2_content_fail_post(this, sid, 0, true);
                    return static_cast<unsigned int>(accepted);
                }
                // 先快照世代再 reserve：reserve 与挂起之间到达的唤醒已体现在世代里，
                // wait_for 的谓词立即为真，不会丢唤醒。reserve 不持世代锁（锁序无环）。
                unsigned long long seen;
                {
                    std::lock_guard<std::mutex> wl(http2_sync_window_mutex);
                    seen = http2_sync_window_gen;
                }
                got = app.http2_reserve_send_window(this, sid, want);
                if (got > 0)
                    break;
                std::unique_lock<std::mutex> wl(http2_sync_window_mutex);
                http2_sync_window_cv.wait_for(wl, left, [this, seen]
                                              { return http2_sync_window_gen != seen; });
            }
        }
        const bool last   = (off + got >= payload.size());
        std::string chunk = h2_make_data_chunk(sid, last ? end_flags : static_cast<unsigned char>(end_flags & ~0x01), payload.data() + off, static_cast<unsigned int>(got));
        if (!http2_push_content_raw_sync(std::move(chunk), sid))
        {
            // retry：环满 sleep 等消费者腾槽，死流/连接关直接撤流退款。
            // push 拒收保留源缓冲，复用 chunk；重试成功落回正常记账继续外层循环。
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(CONST_HTTP2_WINDOW_STALL_TIMEOUT);
            bool pushed         = false;
            for (;;)
            {
                const bool gone = http2_stream_is_dead(sid) || isclose.load(std::memory_order_relaxed) ||
                                  http2_closing.load(std::memory_order_relaxed);
                if (gone)
                {
                    h2_content_fail_post(this, sid, got, false);
                    return static_cast<unsigned int>(accepted);
                }
                if (std::chrono::steady_clock::now() >= deadline)
                    break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (http2_push_content_raw_sync(std::move(chunk), sid))
                {
                    pushed = true;
                    break;
                }
            }
            if (!pushed)
            {
                // 预算耗尽：退款 got + RST
                h2_content_fail_post(this, sid, got, true);
                return static_cast<unsigned int>(accepted);
            }
        }
        off += static_cast<size_t>(got);
        accepted += 9 + got;
        time_limit.store(timeid() + 16);
    }
    return static_cast<unsigned int>(accepted);
}

asio::awaitable<unsigned int> client_session::async_http2_send_content_frame(std::string_view frame)
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
        h2_content_producer_guard producer_guard{this, sid};
        if (http2_stream_is_dead(sid))
            co_return 0;
        if (co_await http2_push_content_raw(std::string(frame), sid))
            co_return static_cast<unsigned int>(frame.size());
        if (!http2_stream_is_dead(sid) && !isclose.load(std::memory_order_relaxed) &&
            !http2_closing.load(std::memory_order_relaxed))
            http2_content_stream_failed(sid, 0, true);
        co_return 0;
    }
    if (frame.size() - 9 != payload_len)
        co_return 0;
    co_return co_await async_http2_send_data_payload(sid, frame.substr(9), fflags);
}

asio::awaitable<unsigned int> client_session::async_http2_send_data_payload(unsigned int sid, std::string_view payload, unsigned char end_flags)
{
    auto self = shared_from_this();
    http2_sender_count_guard count_guard(this);
    if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        co_return 0;
    if (http2_ring_queue == nullptr)
        co_return 0;

    h2_content_producer_guard producer_guard{this, sid};

    if (payload.empty())
    {
        if (http2_stream_is_dead(sid))
            co_return 0;
        char header[9];
        h2_write_data_header(header, sid, end_flags, 0);
        if (co_await http2_push_content_raw(header, sizeof(header), sid))
            co_return 9;
        if (!http2_stream_is_dead(sid) && !isclose.load(std::memory_order_relaxed) &&
            !http2_closing.load(std::memory_order_relaxed))
            http2_content_stream_failed(sid, 0, true);
        co_return 0;
    }

    auto &app             = get_server_app();
    // 内部用 64 位累加避免 >4GB 回绕，返回处按函数签名收窄。
    unsigned long long accepted = 0;
    size_t off            = 0;
    // 流级死活的判据走 http2_dead_streams（cleanup_stream 会 cancel 停等者叫醒本协程），
    // 所以传给 episode 的 dead 旗恒 false，closing 由 episode 内部判。
    const std::atomic_bool nodead{false};
    while (off < payload.size())
    {
        if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed) ||
            http2_stream_is_dead(sid))
            co_return static_cast<unsigned int>(accepted);
        const unsigned int cap =
            std::min(remote_max_frame_size.load(), static_cast<unsigned int>(CONST_HTTP2_SEND_FRAME_BUF));
        const size_t want      = std::min(payload.size() - off, static_cast<size_t>(cap));
        unsigned long long got = app.http2_reserve_send_window(this, sid, want);
        if (got == 0)
        {
            if (!co_await http2_wait_window_episode(sid, nodead))
            {
                if (!http2_stream_is_dead(sid) && !isclose.load(std::memory_order_relaxed) &&
                    !http2_closing.load(std::memory_order_relaxed))
                    http2_content_stream_failed(sid, 0, true);
                co_return static_cast<unsigned int>(accepted);
            }
            continue;
        }
        const bool last   = (off + got >= payload.size());
        std::string chunk = h2_make_data_chunk(sid, last ? end_flags : static_cast<unsigned char>(end_flags & ~0x01), payload.data() + off, static_cast<unsigned int>(got));
        if (!co_await http2_push_content_raw(std::move(chunk), sid))
        {
            const bool gone = http2_stream_is_dead(sid) || isclose.load(std::memory_order_relaxed) ||
                              http2_closing.load(std::memory_order_relaxed);
            http2_content_stream_failed(sid, got, !gone);
            co_return static_cast<unsigned int>(accepted);
        }
        off += static_cast<size_t>(got);
        accepted += 9 + got;
        time_limit.store(timeid() + 16);
    }
    co_return static_cast<unsigned int>(accepted);
}

asio::awaitable<unsigned int> client_session::async_send_writer(const unsigned char *buffer, unsigned int buffersize)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    if (httpv == 2)
    {
        co_return co_await async_http2_send_content_frame(std::string_view(reinterpret_cast<const char *>(buffer), buffersize));
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
    if (httpv == 2)
    {
        co_return co_await async_http2_send_content_frame(msg);
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

asio::awaitable<unsigned int> client_session::async_send_writer(const std::string &msg)
{
    auto self = shared_from_this();
    if (isclose)
    {
        co_return 0;
    }
    if (httpv == 2)
    {
        co_return co_await async_http2_send_content_frame(msg);
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
