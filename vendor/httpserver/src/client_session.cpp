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
#include "send_ring_queue.h"
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

    if (send_ring_queue)
    {
        auto &cc = get_send_ring_pool();
        cc.back_cache_ptr(std::move(send_ring_queue));
        send_ring_queue = nullptr;
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
    if (push_control_frame(_recvack, 21) && http2_need_wakeup)
    {
        wake_ring_consumer();
    }
    co_return;
}

asio::awaitable<bool> client_session::flush_http2_send_queue(bool include_cleanup)
{
    // 有界排空：等控制帧链表和待发送 peer 列表同时空，活跃写入也计入未排空状态。
    // 消费者协程是唯一 socket 写者。
    auto drained = [this, include_cleanup]()
    {
        {
            std::lock_guard<std::mutex> lock{http2_send_control_frame_mutex};
            if (!http2_send_control_frame_list.empty()) return false;
        }
        if (http2_control_write_active.load(std::memory_order_acquire))
            return false;
        return !(include_cleanup ? http2_has_pending_peers() : http2_has_queued_peers());
    };
    // 空链表快路径：绝大多数响应到这里已经排空，省掉每响应一次 timer 构造。
    if (drained())
    {
        co_return true;
    }
    // strand_ 本身作 timer 的 executor：到期回调仍在本连接 strand 串行域内跑，
    // 不能用 strand_.context()——那是 execution_context，没有 get_executor，编不过。
    asio::steady_timer drain_timer(strand_);
    // 2 秒上限、20 毫秒粒度：本地发送队列排空通常在毫秒级；客户端停读时不无限拖住 teardown。
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
    // 超时仍未排空：调用方无法确认握手 SETTINGS 已经写出，不能继续进入业务帧处理。
    co_return drained();
}

asio::awaitable<void> client_session::async_send_goway(unsigned int last_stream_id, unsigned int error_code)
{
    unsigned char fr[17] = {0x00, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    fr[9]                = (last_stream_id >> 24) & 0x7F;
    fr[10]               = (last_stream_id >> 16) & 0xFF;
    fr[11]               = (last_stream_id >> 8) & 0xFF;
    fr[12]               = last_stream_id & 0xFF;
    fr[13]               = (error_code >> 24) & 0xFF;
    fr[14]               = (error_code >> 16) & 0xFF;
    fr[15]               = (error_code >> 8) & 0xFF;
    fr[16]               = error_code & 0xFF;
    // 先等在途响应写出再投 GOAWAY；flush 自带 2 秒预算，排不空也继续通知对端收尾。
    co_await flush_http2_send_queue();
    if (!isclose.load(std::memory_order_relaxed) && !iserror.load(std::memory_order_relaxed) &&
        push_control_frame(fr, sizeof(fr)))
    {
        if (http2_need_wakeup)
            wake_ring_consumer();
        // 调用方随后会 cancel()，必须等 GOAWAY 真正写出或达到排空预算。
        if (!co_await flush_http2_send_queue(false))
        {
            DEBUG_LOG("http2 goaway flush timeout last_sid=%u err=%u", last_stream_id, error_code);
        }
    }
    // 关闭冲刷：置 closing + 撤掉全部 pending peer，发送生产者在循环边界静音退出。
    http2_closing.store(true, std::memory_order_relaxed);
    http2_cancel_all_pending_peers(true);
    co_return;
}

void client_session::send_recv_setting()
{
    unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00};
    if (push_control_frame(_recvack, 9) && http2_need_wakeup)
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
    push_window_update_conn(up_num);
}

bool client_session::push_rst_stream(unsigned int sid, unsigned int code,
                                     const std::shared_ptr<httppeer> &peer)
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
    if (!push_control_frame(frame, sizeof(frame), peer))
        return false;
    if (http2_need_wakeup) wake_ring_consumer();
    return true;
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

bool client_session::http2_send_queue_add(std::shared_ptr<httppeer> peer)
{
    if (!peer || peer->h2send.send_queued || peer->h2send.completion_done || peer->h2send.stream_is_dead ||
        peer->h2send.rst_pending || peer->isclose.load(std::memory_order_relaxed) ||
        isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        return false;

    auto timer = std::make_shared<asio::steady_timer>(strand_);
    timer->expires_after(std::chrono::hours(8));
    peer->h2send.completion_timer = std::move(timer);
    peer->h2send.send_queued = true;

    http2_send_queue.push_back(std::move(peer));
    wake_ring_consumer();
    return true;
}

asio::awaitable<bool> client_session::http2_wait_peer_completion(const std::shared_ptr<httppeer> &peer)
{
    auto timer = peer->h2send.completion_timer;
    if (!timer) co_return false;
    auto [wait_ec] = co_await timer->async_wait(asio::as_tuple(asio::use_awaitable));
    (void)wait_ec;
    if (!peer->h2send.completion_done)
    {
        http2_mark_stream_dead(peer);
        co_return false;
    }
    co_return peer->isfinish && !peer->isclose.load(std::memory_order_relaxed);
}

void client_session::wake_ring_consumer()
{
    auto timer = std::atomic_load(&http2_consumer_timer);
    if (!timer) return;
    auto executor = timer->get_executor();
    asio::post(executor, [timer = std::move(timer)]() mutable { timer->cancel(); });
}

void client_session::http2_mark_peer_rst(const std::shared_ptr<httppeer> &peer, unsigned int error_code)
{
    if (!peer) return;
    auto &state = peer->h2send;
    if (state.completion_done || state.rst_sent || state.rst_pending) return;
    state.rst_pending = true;
    state.rst_error_code = error_code;
    peer->is_rst_stream.store(true, std::memory_order_release);
    if (push_rst_stream(peer->stream_id, error_code, peer))
    {
        state.rst_sent = true;
        state.rst_pending = false;
    }
    else if (!isclose.load(std::memory_order_relaxed) && !http2_closing.load(std::memory_order_relaxed))
    {
        // RST 重试独立于正常 DATA 队列，死流因此不会再次成为可运行 peer。
        ++http2_rst_retry_count;
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
                            if (self->push_rst_stream(peer->stream_id, retry_state.rst_error_code, peer))
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
                    --self->http2_rst_retry_count;
                    if (!retry_state.in_batch)
                        self->http2_complete_active_peer(peer, false);
                },
                asio::detached);
        }
        catch (...)
        {
            state.rst_pending = false;
            --http2_rst_retry_count;
        }
    }
    else state.rst_pending = false;
    wake_ring_consumer();
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
    if (ready && !state.stream_is_dead && !state.rst_pending && !state.completion_done &&
        !peer->isclose.load(std::memory_order_relaxed) &&
        !isclose.load(std::memory_order_relaxed) && !http2_closing.load(std::memory_order_relaxed))
    {
        wake_ring_consumer();
    }
}

void client_session::http2_complete_active_peer(const std::shared_ptr<httppeer> &peer, bool success)
{
    auto &state = peer->h2send;
    std::shared_ptr<asio::steady_timer> completion_timer;
    {
        std::lock_guard<std::mutex> lock(h2_send_window_mutex);
        if (state.completion_done) return;
        http2_send_reserved.fetch_sub(std::min<unsigned long long>(
            state.send_reserved, http2_send_reserved.load(std::memory_order_relaxed)),
            std::memory_order_relaxed);
        state.in_batch = false;
        state.send_reserved = 0;
        state.completion_done = true;
        peer->isfinish = success;
        completion_timer = state.completion_timer;
    }
    if (completion_timer) completion_timer->cancel();
}

void client_session::http2_cancel_all_pending_peers(bool wake)
{
    while (!http2_send_queue.empty())
    {
        auto it = http2_send_queue.begin();
        auto peer = *it;
        http2_send_queue.erase(it);
        peer->h2send.send_queued = false;
        http2_complete_active_peer(peer, false);
    }
    if (wake) wake_ring_consumer();
}

bool client_session::http2_has_pending_peers()
{
    return !http2_send_queue.empty() ||
           http2_rst_retry_count.load(std::memory_order_acquire) > 0 ||
           http2_control_cleanup_count.load(std::memory_order_acquire) > 0;
}

bool client_session::http2_has_queued_peers()
{
    return !http2_send_queue.empty() || http2_rst_retry_count.load(std::memory_order_acquire) > 0;
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

static std::int64_t h2_window_remaining(std::int64_t total_window, std::uint64_t sent_bytes)
{
    if (total_window <= 0) return total_window;
    const auto granted = static_cast<std::uint64_t>(total_window);
    if (granted >= sent_bytes)
    {
        const auto remaining = granted - sent_bytes;
        return remaining > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())
                   ? std::numeric_limits<std::int64_t>::max()
                   : static_cast<std::int64_t>(remaining);
    }
    const auto deficit = sent_bytes - granted;
    if (deficit > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
        return std::numeric_limits<std::int64_t>::min();
    return -static_cast<std::int64_t>(deficit);
}

static std::int64_t h2_window_visible(std::int64_t total_window,
                                      std::uint64_t sent_bytes,
                                      std::uint64_t reserved)
{
    const auto remaining = h2_window_remaining(total_window, sent_bytes);
    if (remaining <= 0) return 0;
    const auto available = static_cast<std::uint64_t>(remaining);
    if (available <= reserved) return 0;
    return static_cast<std::int64_t>(available - reserved);
}

bool client_session::http2_apply_initial_window_size(unsigned int size, const http2parse &parser)
{
    std::lock_guard<std::mutex> lock(h2_send_window_mutex);
    const auto old_size = remote_initial_window_size.load(std::memory_order_relaxed);
    const auto delta = static_cast<std::int64_t>(size) - static_cast<std::int64_t>(old_size);
    if (delta != 0)
    {
        auto valid = [delta](const httppeer *peer) {
            const auto remaining = h2_window_remaining(peer->h2send.send_window,
                                                       peer->h2send.send_totoal_bytes);
            if (delta > 0 && remaining > CONST_HTTP2_MAX_WINDOW - delta) return false;
            if (peer->h2send.send_window < -delta) return false;
            return true;
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
    remote_initial_window_size.store(size, std::memory_order_relaxed);
    wake_ring_consumer();
    return true;
}

bool client_session::push_control_frame(const unsigned char *fr, unsigned int n,
                                        const std::shared_ptr<httppeer> &peer)
{
    if (!fr || n == 0 || isclose.load(std::memory_order_relaxed) ||
        iserror.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        return false;
    h2_control_frame_t item;
    item.stream_peer = peer;
    item.content.assign(fr, fr + n);
    std::lock_guard<std::mutex> lock{http2_send_control_frame_mutex};
    if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        return false;
    http2_send_control_frame_list.push_back(std::move(item));
    http2_control_frames_ready.store(true, std::memory_order_release);
    return true;
}

// ---- HTTP/2 控制帧与 httppeer 同步流式帧链表 ----
// 流式 DATA 分片并预留窗口后入链表，HEADERS 帧链整块入链表；发送协程优先消费控制帧。

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
        state.control_cleanup_pending = true;
        ++http2_control_cleanup_count;
        peer->isclose.store(true, std::memory_order_release);
    }
    if (!state.in_batch && !state.rst_pending)
        http2_complete_active_peer(peer, false);
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

// 裸内容帧入链表，一次尝试：owned 非空时拷贝其存储，否则直接拷贝调用方帧头。
// 流已死 / 连接已关返回 false；链表不做容量判断。
bool client_session::http2_try_push_content_raw(std::string_view frame, std::string *owned,
                                                const std::shared_ptr<httppeer> &peer)
{
    if (!peer || frame.size() < 9 || isclose.load(std::memory_order_relaxed) ||
        http2_closing.load(std::memory_order_relaxed) || peer->h2send.stream_is_dead)
        return false;
    h2_control_frame_t item;
    item.stream_peer = peer;
    if (owned)
        item.content.assign(owned->begin(), owned->end());
    else
        item.content.assign(frame.begin(), frame.end());
    peer->h2_control_frames_pending.fetch_add(1, std::memory_order_release);
    try
    {
        std::lock_guard<std::mutex> lock{http2_send_control_frame_mutex};
        if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed) ||
            peer->h2send.stream_is_dead)
        {
            peer->h2_control_frames_pending.fetch_sub(1, std::memory_order_release);
            return false;
        }
        http2_send_control_frame_list.push_back(std::move(item));
        http2_control_frames_ready.store(true, std::memory_order_release);
    }
    catch (...)
    {
        peer->h2_control_frames_pending.fetch_sub(1, std::memory_order_release);
        throw;
    }
    if (http2_need_wakeup)
        wake_ring_consumer();
    return true;
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
asio::awaitable<bool> client_session::http2_push_content_raw(std::string frame,
                                                              std::shared_ptr<httppeer> peer)
{
    co_return http2_try_push_content_raw(frame, &frame, peer);
}

asio::awaitable<bool> client_session::http2_push_content_raw(const char *frame, unsigned int frame_size,
                                                              std::shared_ptr<httppeer> peer)
{
    co_return http2_try_push_content_raw(std::string_view(frame, frame_size), nullptr, peer);
}

unsigned int client_session::http2_send_content_frame(std::string_view frame,
                                                       const std::shared_ptr<httppeer> &peer)
{
    // 自保活：sync 路径入控制帧链表期间若 peer 引用断链，session 不能析构。
    // Self-preserve: keep session alive during sync control-frame enqueue loops.
    auto self = shared_from_this();
    if (isclose.load(std::memory_order_relaxed) || http2_closing.load(std::memory_order_relaxed))
        return 0;
    if (frame.size() < 9)
        return 0;
    const unsigned int sid         = h2_frame_sid(frame);
    const unsigned char ftype      = static_cast<unsigned char>(frame[3]);
    const unsigned char fflags     = static_cast<unsigned char>(frame[4]);
    const unsigned int payload_len = h2_frame_payload_len(frame);

    // HEADERS 帧链：整块入控制帧链表，不做窗口记账。
    if (ftype != 0x00)
    {
        if (!peer || peer->stream_id != sid) return 0;
        if (peer->h2send.stream_is_dead)
            return 0;
        std::string headers_frame(frame);
        if (http2_push_content_raw_sync(std::move(headers_frame), peer))
        {
            peer->h2send.headers_sent.store(true, std::memory_order_release);
            if (fflags & 0x01) peer->h2_end_stream_queued.store(true, std::memory_order_release);
            return static_cast<unsigned int>(frame.size());
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

    // 零长 DATA（END_STREAM 收尾）：不占窗口额度，整帧入链表。
    if (payload.empty())
    {
        if (peer->h2send.stream_is_dead)
            return 0;
        char header[9];
        h2_write_data_header(header, sid, end_flags, 0);
        if (!http2_push_content_raw_sync(header, sizeof(header), peer))
        {
            if (!peer->h2send.stream_is_dead && !isclose.load(std::memory_order_relaxed) &&
                !http2_closing.load(std::memory_order_relaxed))
                h2_content_fail(this, peer, true);
            return 0;
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
        unsigned long long got = 0;
        {
            std::lock_guard<std::mutex> lock(h2_send_window_mutex);
            if (!peer->h2send.stream_is_dead)
            {
                const auto stream_available = h2_window_visible(peer->h2send.send_window,
                                                                peer->h2send.send_totoal_bytes,
                                                                peer->h2send.send_reserved);
                const auto conn_available = h2_window_visible(http2_send_window.load(std::memory_order_relaxed),
                                                              http2_send_totoal_bytes.load(std::memory_order_relaxed),
                                                              http2_send_reserved.load(std::memory_order_relaxed));
                const auto available = std::min(stream_available, conn_available);
                if (available > 0)
                {
                    got = std::min<unsigned long long>(want, static_cast<unsigned long long>(available));
                    peer->h2send.send_reserved += got;
                    http2_send_reserved.fetch_add(got, std::memory_order_relaxed);
                }
            }
        }
        // 非阻塞：窗口不够就把已收字节数还给调用方，由它决定稍后再发；停等只在 ring loop。
        if (got == 0)
            return static_cast<unsigned int>(accepted);
        const bool last   = (off + got >= payload.size());
        std::string chunk = h2_make_data_chunk(sid, last ? end_flags : static_cast<unsigned char>(end_flags & ~0x01), payload.data() + off, static_cast<unsigned int>(got));
        if (!http2_push_content_raw_sync(std::move(chunk), peer))
        {
            const bool gone = peer->h2send.stream_is_dead || isclose.load(std::memory_order_relaxed) ||
                              http2_closing.load(std::memory_order_relaxed);
            if (!gone)
                h2_content_fail(this, peer, true);
            return static_cast<unsigned int>(accepted);
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
    if (frame.size() < 9)
        co_return 0;
    const unsigned int sid         = h2_frame_sid(frame);
    const unsigned char ftype      = static_cast<unsigned char>(frame[3]);
    const unsigned char fflags     = static_cast<unsigned char>(frame[4]);
    const unsigned int payload_len = h2_frame_payload_len(frame);

    if (ftype != 0x00)
    {
        if (!peer || peer->stream_id != sid) co_return 0;
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
        unsigned long long got = 0;
        {
            std::lock_guard<std::mutex> lock(h2_send_window_mutex);
            if (!peer->h2send.stream_is_dead)
            {
                const auto stream_available = h2_window_visible(peer->h2send.send_window,
                                                                peer->h2send.send_totoal_bytes,
                                                                peer->h2send.send_reserved);
                const auto conn_available = h2_window_visible(http2_send_window.load(std::memory_order_relaxed),
                                                              http2_send_totoal_bytes.load(std::memory_order_relaxed),
                                                              http2_send_reserved.load(std::memory_order_relaxed));
                const auto available = std::min(stream_available, conn_available);
                if (available > 0)
                {
                    got = std::min<unsigned long long>(want, static_cast<unsigned long long>(available));
                    peer->h2send.send_reserved += got;
                    http2_send_reserved.fetch_add(got, std::memory_order_relaxed);
                }
            }
        }
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
    // HTTP/2 响应协程最迟 10 秒预算到即见旗退出。
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
    if (isclose || iserror || !send_ring_queue)
    {
        return false;
    }
    if (msg.size() > std::numeric_limits<unsigned int>::max())
    {
        return false;
    }

    auto *q = send_ring_queue.get();
    const unsigned long long size = static_cast<unsigned long long>(msg.size());
    unsigned long long backlog = q->bytes.load(std::memory_order_relaxed);
    for (;;)
    {
        if (backlog > 0 && (backlog > SEND_RING_BYTE_LIMIT ||
                            size > SEND_RING_BYTE_LIMIT - backlog))
        {
            send_ring_overflow_count.fetch_add(1, std::memory_order_relaxed);
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
        send_ring_overflow_count.fetch_add(1, std::memory_order_relaxed);
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
