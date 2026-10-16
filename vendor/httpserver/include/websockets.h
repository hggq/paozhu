#ifndef BOOST_DLL_MY_WEBSOCKET_API_HPP
#define BOOST_DLL_MY_WEBSOCKET_API_HPP

//[plugapi
#include <string>
#include <string_view>
#include <memory>
#include <mutex>
#include <atomic>
#include "client_session.h"
#include "ws_conn.h"
#include "cost_define.h"

namespace http
{
struct websockets_api_data_t
{
    std::string name;
    std::string value;
};
struct websockets_data_list_t
{
    bool isfile        = false;
    bool isdeflate     = false;
    unsigned int seqid = 0;
    std::string value;
};
class websockets_api : public std::enable_shared_from_this<websockets_api>
{
  public:
    websockets_api(unsigned char t, unsigned int m, unsigned int g, unsigned char s) : state(s), durtime(t), myid(m), groupid(g) {}
    virtual void onopen()                                                       = 0;
    virtual asio::awaitable<void> async_onopen()                                = 0;
    virtual void onmessage()                                                    = 0;
    virtual asio::awaitable<void> async_onmessage(websockets_data_list_t &&msg) = 0;
    virtual void run_loop()                                                     = 0;
    virtual asio::awaitable<void> async_run_loop()                              = 0;
    //  virtual void timeloop(clientpeer*) = 0;
    virtual void onclose()                        = 0;
    virtual asio::awaitable<void> async_onclose() = 0;
    // 保活钩子在连接的收帧循环（strand）上同步调用：只该做记账，做重活会卡住这条连接的收帧。
    // 入站 PING：对端要我们回 pong，pong 由框架回，payload 是 ping 的原文
    virtual void onping([[maybe_unused]] std::string_view payload) {}
    // 入站 PONG：服务端 ping 的回应，payload 是回应原文，可用来算 RTT
    virtual void onpong([[maybe_unused]] std::string_view payload) {}
    // 旧名字：历史上收到 PING 时调它，现在和 onping 同一拍触发，等业务适配类迁完再删
    virtual void onpong() {}
    virtual ~websockets_api() {}

    // ====== 模式（对齐 mqtt_api.h 的 is_coroutine() 风格）======
    // is_coroutine()=true  → 框架调用 async_onopen/async_onmessage/async_onclose
    // is_coroutine()=false → 框架调用 onopen/onmessage/onclose
    // 默认返回 isco 字段值，子类可在构造函数中设置 isco，也可直接覆盖此虚方法。
    virtual bool is_coroutine() const { return isco; }

    // run_loop 的协程开关：true → async_run_loop()，false → run_loop()
    virtual bool is_loop_coroutine() const { return isloopco; }
    // 入队带水位闸：条数和字节两把尺子在同一把锁里一起判，界值和口径与协程侧的
    // ws_inflight_reserve 一致（都是"已达上限即拒收"，所以最多允许一条消息冲出上限）。
    // 返回 false 表示越限未入队，msg 未被移动，调用方等一会可以拿同一条重试。
    bool push(websockets_data_list_t &&msg)
    {
        std::unique_lock<std::mutex> lock(content_list_mutex);
        if (content_list.size() >= CONST_WEBSOCKET_QUEUE_HIGH_ITEMS ||
            queue_bytes_locked() >= CONST_WEBSOCKET_QUEUE_HIGH_BYTES)
        {
            return false;
        }
        content_list.push_back(std::move(msg));
        return true;
    }
    unsigned long long queue_items() const
    {
        std::unique_lock<std::mutex> lock(content_list_mutex);
        return content_list.size();
    }
    // 队列里的载荷字节。非维护计数：content_list 由业务代码自行 pop_front，任何累减式记账都会漂，
    // 所以每次现场遍历求和（条子上限 256，遍历最长 256 项）。
    unsigned long long queue_bytes() const
    {
        std::unique_lock<std::mutex> lock(content_list_mutex);
        return queue_bytes_locked();
    }

  private:
    // 求和本身不判水位：调用方必须已持有 content_list_mutex
    unsigned long long queue_bytes_locked() const
    {
        unsigned long long bytes = 0;
        for (auto &it : content_list)
        {
            bytes += it.value.size();
        }
        return bytes;
    }

  public:
    // 统一出站接口 — 全部走发送环，ring_client_server 为唯一写者。
    // 禁止业务代码直接调用 session_sock->send_writer / async_send_writer 发送帧。
    bool send(std::string_view payload)
    {
        if (!session_sock)
            return false;
        return ws::post_send_text(*session_sock, payload);
    }
    bool send_binary(std::string_view payload)
    {
        if (!session_sock)
            return false;
        return ws::post_send_binary(*session_sock, payload);
    }
    bool send_close(uint16_t code = 1000, std::string_view reason = {})
    {
        if (!session_sock)
            return false;
        return ws::post_send_close(*session_sock, code, reason);
    }
    bool send_ping(std::string_view payload = "ping")
    {
        if (!session_sock)
            return false;
        return ws::post_send_ping(*session_sock, payload);
    }

  public:
    bool isclose      = false;
    bool iserror      = false;
    bool isfile       = false;
    bool isco         = false;
    bool isloopco     = false;
    bool open_deflate = false;
    // 协程模式（is_coroutine()=true）没有 content_list，每收一条消息就 co_spawn 一个
    // async_onmessage——在途协程本身就是那条无界队列（业务里任何 co_await 都会让它长起来）。
    // 由 server.cpp 的派发包装协程成对 reserve/release，作为该模式的在途水位尺子。
    std::atomic<unsigned long long> inflight_dispatch = 0;
    std::atomic<unsigned long long> inflight_bytes    = 0;
    // 服务端保活 ping 的下一次到期时刻（epoch 秒），0 = 还没登记。
    // 只有 websocket_loop 定时 tick 那一拍读写它（单线程），所以是普通整数。
    // 放在 websockets_api（per-WS-conn）而非 client_session（HTTP/1/2/WS 共用骨架）。
    // Next server-keepalive ping deadline (epoch seconds), 0 = not yet registered.
    // Single-thread access from websocket_loop tick, plain int is enough.
    // Lives on websockets_api (per-WS-conn), not the shared HTTP/1/2/WS client_session skeleton.
    unsigned int ws_next_ping = 0;
    // 最近一次收到入站字节的时刻（epoch 秒），含数据帧、pong、close 等所有控制帧。
    // 读循环在 strand 上 store，定时 tick 线程 load——两边不同线程，所以放成原子。
    // 0 = 还没收到过任何字节（首拍默认值，此时不做超时判定，给对端留初始握手窗口）。
    // 放在 websockets_api（per-WS-connection）里而不是 client_session（HTTP/1/2/WS 共用），
    // 避免污染 HTTP 骨架结构。
    // Epoch seconds of last inbound byte (data / pong / close, all control frames).
    // Strand reader stores, tick thread loads — cross-thread access requires atomic.
    // Lives on websockets_api (per-WS-conn) rather than client_session (shared HTTP/1/2/WS
    // skeleton) to keep HTTP structs lean.
    std::atomic<unsigned int> ws_last_rx_seen{0};
    unsigned char state;
    unsigned int durtime  = 8;
    unsigned int loop_num = 1;
    unsigned int myid     = 0;
    unsigned int groupid  = 0;
    unsigned int siteid   = 0;
    std::string url;
    std::string host;
    std::list<websockets_data_list_t> content_list;
    std::vector<websockets_api_data_t> header;
    std::shared_ptr<client_session> session_sock = nullptr;
    // mutable：queue_items()/queue_bytes() 是只读探测，但要和业务的 pop_front 抢同一把锁
    mutable std::mutex content_list_mutex;
};
}// namespace http
#endif