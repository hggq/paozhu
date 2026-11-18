#ifndef HTTP2_SENDER_COUNT_GUARD_H__
#define HTTP2_SENDER_COUNT_GUARD_H__

#include "client_session.h"
#include "server.h"

namespace http
{

// HTTP/2 发送协程计数 RAII：构造 +1、析构 -1。
// 会话级计数用于收尾日志，全进程计数用于状态页。
// 必须在第一个 co_await 之前构造，覆盖 flush / 停等 / 写帧全过程。
// HTTP/2 sender coroutine count guard: +1 on ctor, -1 on dtor.
// Per-session for teardown logs, per-process for status page.
// Construct before the first co_await so the guard covers flush / stall / frame writes.
struct http2_sender_count_guard
{
    std::atomic<unsigned int> &per_session;
    std::atomic<unsigned int> &per_process;

    explicit http2_sender_count_guard(client_session *sess)
        : per_session(sess->http2_active_senders),
          per_process(get_server_app().http2_active_senders_total)
    {
        per_session.fetch_add(1, std::memory_order_relaxed);
        per_process.fetch_add(1, std::memory_order_relaxed);
    }
    ~http2_sender_count_guard()
    {
        per_session.fetch_sub(1, std::memory_order_relaxed);
        per_process.fetch_sub(1, std::memory_order_relaxed);
    }
    http2_sender_count_guard(const http2_sender_count_guard &)            = delete;
    http2_sender_count_guard &operator=(const http2_sender_count_guard &) = delete;
};

}// namespace http

#endif// HTTP2_SENDER_COUNT_GUARD_H__
