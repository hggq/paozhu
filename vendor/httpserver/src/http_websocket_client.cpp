#include <random>
#include <openssl/sha.h>//for SHA1
#include "http_websocket_client.h"
#include "client_context.h"
#include "base64.h"
#include "func.h"
#include "atomic_guard.h"
#include "gzip.h"
#include "cost_define.h"

namespace http
{
websocket_client::websocket_client() : strand_(asio::make_strand(*(get_client_context_obj().ioc))) {};
websocket_client::~websocket_client()
{
    if (data != nullptr)
    {
        std::free(data);
        data = nullptr;
    }
}
void websocket_client::reset()
{
    iserror               = false;
    isssl                 = false;
    isbody                = false;
    isfinish              = false;
    iswait_exit           = false;
    isclose_recv_         = false;
    exptime               = 0;
    cur_process_type      = 0;
    offsetnum             = 0;
    port                  = 0;
    val_size              = 0;
    timeout_end           = 0;
    ready_state           = 0;
    last_rx_sec           = 0;
    next_ping_sec         = 0;
    handshake_resp_begin_ = 0;

    url.clear();
    host.clear();
    error_msg.clear();

    parameter.clear();
    in_payload_phase_ = false;
    in_hdr_have_      = 0;
    in_hdr_need_      = 2;
    in_fin_           = false;
    in_ctl_           = false;
    in_msg_open_      = false;
    in_payload_left_  = 0;
    in_ctl_buf_.clear();
    stream_pending_.clear();
    in_utf8_.reset();
    close_connect();
    sock.reset();
    sslsock.reset();
    ssl_context.reset();
}

void websocket_client::set_deflate(bool isstatus)
{
    open_deflate = isstatus;
}
void websocket_client::set_header(std::string_view name, std::string_view value)
{
    websocket_parameter_t a;
    a.name  = name;
    a.value = value;
    parameter.emplace_back(a);
}
void websocket_client::add_headers(std::string_view raw)
{
    // 统一分隔符: \r\n → \n, 分号 → \n (INI 里写 \r\n 要转义, 分号更方便)
    std::string s(raw);
    for (auto &c : s)
        if (c == '\r' || c == ';')
            c = '\n';
    std::size_t start = 0;
    while (start <= s.size())
    {
        auto pos         = s.find('\n', start);
        std::string line = (pos == std::string::npos) ? s.substr(start) : s.substr(start, pos - start);
        // trim trailing \0 or whitespace
        while (!line.empty() && (line.back() == '\0' || line.back() == ' ' || line.back() == '\t'))
            line.pop_back();
        auto colon = line.find(':');
        if (colon != std::string::npos && colon > 0)
            set_header(std::string_view(line).substr(0, colon),
                       std::string_view(line).substr(colon + 1));
        if (pos == std::string::npos)
            break;
        start = pos + 1;
    }
}
void websocket_client::set_port(unsigned int n)
{
    port = n;
}
void websocket_client::set_host(std::string_view name)
{
    host = name;
}

void websocket_client::set_url(std::string_view name)
{
    // 纯路径（conf/websockets.conf 的 url 就是这个形状，host/port 由调用方分开给）：
    // 直接收下。少了这一步，"/wstest" 会掉进下面 scheme 解析的 else 分支被判成格式错，
    // 常驻客户端的 url 留空，async_connect 第一句就返回 false —— 这条连接永远建不起来。
    if (!name.empty() && name[0] == '/')
    {
        url = name;
        return;
    }
    if (name.size() <= 7)
    {
        error_msg = "url too short";
        iserror   = true;
        return;
    }
    // 三种可识别形状：ws://host[:port][/路径]、wss:// 同上、以及数字开头的裸 host。
    // 解析全程只用局部量，成功才一次性写入 host/port/url —— 半途失败不会残留上一轮的值，
    // 也不会把两次的路径拼起来（旧写法是 url.push_back 累加）。
    std::string_view rest;
    unsigned int default_port = 80;
    bool ssl                  = false;
    if (name[0] == 'w' && name[1] == 's' && name[2] == ':' && name[3] == '/' && name[4] == '/')
    {
        rest = name.substr(5);
    }
    else if (name[0] == 'w' && name[1] == 's' && name[2] == 's' && name[3] == ':' && name[4] == '/' && name[5] == '/')
    {
        rest         = name.substr(6);
        ssl          = true;
        default_port = 443;
    }
    else if (name[0] >= '0' && name[0] <= '9')
    {
        rest = name;
    }
    else
    {
        error_msg = "url formatting error";
        iserror   = true;
        return;
    }

    std::size_t i = 0;
    std::string temp_host;
    if (i < rest.size() && rest[i] == '[')
    {
        // IPv6 字面值：host 在方括号里，括号内的冒号不是端口分隔符。
        // 入库时剥掉括号——地址解析要的是裸地址，方括号只在写 Host: 请求头时才需要。
        std::size_t close = rest.find(']', i + 1);
        if (close == std::string_view::npos)
        {
            error_msg = "url formatting error";
            iserror   = true;
            return;
        }
        temp_host.assign(rest, i + 1, close - i - 1);
        i = close + 1;
    }
    else
    {
        std::size_t end = rest.find_first_of("/:", i);
        if (end == std::string_view::npos)
        {
            end = rest.size();
        }
        temp_host.assign(rest, i, end - i);
        i = end;
    }
    if (temp_host.empty())
    {
        error_msg = "url formatting error";
        iserror   = true;
        return;
    }
    // 括号收尾后只允许 ':'（端口）、'/'（路径）或直接结束
    unsigned long long temp_port = 0;
    if (i < rest.size() && rest[i] == ':')
    {
        i++;
        for (; i < rest.size() && rest[i] != '/'; i++)
        {
            if (rest[i] < '0' || rest[i] > '9')
            {
                continue;// 端口段的非数字字符跳过（与旧解析一致，不判格式错）
            }
            temp_port = temp_port * 10 + static_cast<unsigned long long>(rest[i] - '0');
        }
        if (temp_port > 65535)
        {
            error_msg = "url port error";
            iserror   = true;
            return;
        }
    }
    else if (i < rest.size() && rest[i] != '/')
    {
        error_msg = "url formatting error";
        iserror   = true;
        return;
    }
    // 没有路径段时按 RFC 6455 §4.1 取 "/"，请求行才是合法的 "GET / HTTP/1.1"
    std::string temp_url;
    if (i < rest.size())
    {
        temp_url.assign(rest, i, rest.size() - i);
    }
    else
    {
        temp_url = "/";
    }

    isssl = ssl;
    host  = temp_host;
    port  = (temp_port == 0) ? default_port : static_cast<unsigned int>(temp_port);
    url   = temp_url;
}

asio::awaitable<bool> websocket_client::async_init_https_sock()
{
    //auto executor = co_await asio::this_coro::executor;
    ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    sslsock     = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(strand_, *ssl_context);
    ssl_context->set_default_verify_paths();

    asio::ip::tcp::resolver resolver(strand_);
    SSL_set_tlsext_host_name(sslsock->native_handle(), host.c_str());

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    auto endpoints                 = co_await resolver.async_resolve(host, std::to_string(port), asio::use_awaitable);
    for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
    {
        std::tie(ec) = co_await sslsock->lowest_layer().async_connect(*iter, tuple_awaitable);
        if (ec)
        {
            ++iter;
            continue;
        }
        break;
    }
    if (ec)
    {
        error_msg = host + " ssl async_connect error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }

    sslsock->lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    ssl_context->set_verify_mode(asio::ssl::verify_peer);
    ssl_context->set_verify_callback(asio::ssl::host_name_verification(host));

    std::tie(ec) = co_await sslsock->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
    if (ec)
    {
        error_msg = host + " ssl handshake error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}

asio::awaitable<bool> websocket_client::async_init_http_sock()
{
    error_msg.clear();
    //auto executor = co_await asio::this_coro::executor;
    //strand_(asio::make_strand(executor));
    //asio::ip::tcp::resolver resolver(executor);
    asio::ip::tcp::resolver resolver(strand_);

    sock = std::make_shared<asio::ip::tcp::socket>(strand_);

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    auto endpoints                 = co_await resolver.async_resolve(host, std::to_string(port), asio::use_awaitable);

    for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
    {
        std::tie(ec) = co_await sock->async_connect(*iter, tuple_awaitable);
        if (ec)
        {
            ++iter;
            continue;
        }
        break;
    }

    if (ec)
    {
        error_msg = ec.message();
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}
asio::awaitable<bool> websocket_client::async_connect(std::string_view url_, unsigned int time_out_num)
{
    set_url(url_);
    exptime = time_out_num;
    co_return co_await async_connect();
}

asio::awaitable<bool> websocket_client::async_connect()
{
    bool isinit = false;

    if (url.empty())
    {
        iserror   = true;
        error_msg = "url empty";
        co_return false;
    }

    if (isssl)
    {
        isinit = co_await async_init_https_sock();
    }
    else
    {
        isinit = co_await async_init_http_sock();
    }

    if (!isinit)
    {
        iserror   = true;
        error_msg = "async init socket error";
        co_return false;
    }

    // timeout==0 不再豁免：始终进入超时链表，由 dur 心跳托管（reset_timeout 对 0 写哨兵大值）
    reset_timeout();
    {
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.add_timeout_list(temp_io_context.websocket_timeout_lists, shared_from_this());
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
            co_return false;
        }
    }
    isinit = co_await websocket_handshake();
    co_return isinit;
}

bool websocket_client::add_client_task_loop()
{
    client_context &temp_io_context = get_client_context_obj();
    try
    {
        auto self = shared_from_this();
        for (auto &wp : temp_io_context.websocket_timeout_lists)
        {
            if (auto p = wp.lock(); p && p.get() == self.get())
            {
                return true;
            }
        }
        temp_io_context.add_timeout_list(temp_io_context.websocket_timeout_lists, self);
        return true;
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
        return false;
    }
}

// 保活判定：由看护线程每秒一拍调用，只碰记账字段，不加锁、不做 IO。
// 返回 1 表示本拍该发一条 ping（由看护线程在锁外经出站队列发出），
// 返回 2 表示连续 3 个周期没有入站字节，已置 iswait_exit 交给看护线程下一拍关连接摘链表。
unsigned int websocket_client::keepalive_check(unsigned int nowsec)
{
    constexpr unsigned int interval = CONST_WEBSOCKET_CLIENT_PING_INTERVAL;
    if constexpr (interval == 0)
    {
        return 0;// 关闭档：这些记账字段全程不被读写
    }
    if (isclose_recv_)
    {
        return 0;// 已经收到对端 close，等回声写完就收摊，不再探测
    }
    unsigned int last = last_rx_sec.load();
    if (last == 0)
    {
        // 第一次被看护线程看到：以本拍为静默起点，避免刚登记就按"连接建立前"的时间判死
        last = nowsec;
        last_rx_sec.store(last);
    }
    if (nowsec >= last + interval * 3)
    {
        // 不写 error_msg：那个 std::string 由连接自己的协程和业务线程读写，看护线程碰它就是数据竞争。
        // 判死只立面旗，拆除沿用既有路径（下一拍 close_erase → close_connect），
        // 业务侧经 on_close/async_on_close 收到收摊通知。
        DEBUG_LOG("websocket client keepalive timeout, idle over %u sec", interval * 3);
        iswait_exit = true;
        return 2;
    }
    if (next_ping_sec == 0)
    {
        next_ping_sec = nowsec + interval;// 第一条 ping 也要等满一个周期
    }
    if (nowsec < next_ping_sec)
    {
        return 0;
    }
    // 不补发欠账：看护线程可能因为进程挂起跳过若干拍，一次一拍会形成 ping 风暴
    next_ping_sec = nowsec + interval;
    return 1;
}

asio::awaitable<unsigned int> websocket_client::async_read(unsigned char *read_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set())
    {
        // 读锁占用是可重试的瞬时冲突，不锁死整条连接
        error_msg = "Other socket read is set";
        co_return 0;
    }
    atomic_guard guard{socket_read_lock};

    if (iserror)
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await sslsock->async_read_some(asio::buffer(read_data, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await sock->async_read_some(asio::buffer(read_data, buffersize), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    co_return 0;
}

asio::awaitable<unsigned int> websocket_client::async_read(std::string &read_data)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        co_return 0;
    }
    atomic_guard guard{socket_read_lock};
    if (iserror)
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (read_data.size() == 0)
        {
            read_data.resize(1024);
        }

        if (isssl)
        {
            n = co_await sslsock->async_read_some(asio::buffer(read_data), asio::use_awaitable);
        }
        else
        {
            n = co_await sock->async_read_some(asio::buffer(read_data), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    co_return 0;
}

asio::awaitable<unsigned int> websocket_client::async_write(unsigned char *data_out, unsigned int buffersize)
{
    if (iserror || (isssl ? (sslsock == nullptr) : (sock == nullptr)))
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await asio::async_write(*sslsock, asio::buffer(data_out, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*sock, asio::buffer(data_out, buffersize), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    co_return 0;
}

asio::awaitable<unsigned int> websocket_client::async_write(std::string_view value)
{
    if (iserror || (isssl ? (sslsock == nullptr) : (sock == nullptr)))
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }

    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await asio::async_write(*sslsock, asio::buffer(value), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*sock, asio::buffer(value), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    co_return 0;
}

// ---------------------------------------------------------------------------
// 出站写队列
//   读侧已有 socket_read_lock 串行化；写侧此前无任何保护，导致自动 pong
//   （经 co_spawn 触发）与业务 async_text_write 可并发写同一 stream，
//   帧字节交错后对端流同步丢失。所有 WebSocket 帧出站必须经 async_send_frame()。
//   队列元素为完整帧。业务协程（服务器 io_context 线程）与 strand 上的 pong
//   来自不同线程，send_queue_/sending_ 由 send_queue_mutex_ 保护；
//   enqueue_frame 返回 true 的一方接管队列消费，同一时刻只有一个 pump 在跑。
// ---------------------------------------------------------------------------
// 队列长度上限：正常使用下 async_text_write 会 await 本次发送，队列深度保持在个位数。
// 队列持续堆积说明对端长期不可写（背压失控）。此时若静默丢弃，会造成帧流中断后
// 对端协议错位，故直接判定连接错误。
static constexpr std::size_t kMaxSendQueueDepth = 1024;

bool websocket_client::enqueue_frame(std::string frame)
{
    std::lock_guard<std::mutex> lock(send_queue_mutex_);
    if (send_queue_.size() >= kMaxSendQueueDepth)
    {
        error_msg = "websocket send queue overflow";
        iserror   = true;
        return false;
    }
    send_queue_.push_back(std::move(frame));
    if (sending_)
    {
        return false;// 已有 pump 协程在消费，它会带走本次追加的帧
    }
    sending_ = true;
    return true;
}

asio::awaitable<void> websocket_client::pump_send_queue()
{
    // 任何退出路径（正常收尾/async_write 抛异常/协程被 strand 取消）都必须清掉
    // 唯一消费者标记 sending_，否则 pump 折损后发送队列永久卡死
    struct sending_guard
    {
        websocket_client *self;
        ~sending_guard()
        {
            std::lock_guard<std::mutex> lock(self->send_queue_mutex_);
            self->send_queue_.clear();// 本 pump 收尾（含被取消），剩余帧作废
            self->sending_ = false;
        }
    } guard{this};

    for (;;)
    {
        std::string frame;
        {
            std::lock_guard<std::mutex> lock(send_queue_mutex_);
            if (send_queue_.empty() || iserror)
            {
                co_return;// 收尾与作废由 guard 统一执行
            }
            frame = std::move(send_queue_.front());
            send_queue_.pop_front();
        }
        co_await async_write(frame);
    }
}

asio::awaitable<unsigned int> websocket_client::async_send_frame(std::string frame)
{
    if (iserror)
    {
        co_return 0;
    }
    const unsigned int queued = static_cast<unsigned int>(frame.size());
    bool mine                 = enqueue_frame(std::move(frame));
    if (!mine && iserror)
    {
        // 队列溢出：这一帧根本没进队列，返回值不能报"发了 queued 字节"
        co_return 0;
    }
    if (mine)
    {
        co_await pump_send_queue();
        if (iserror)
        {
            // 本 pump 因写失败折损，剩余帧（含还没轮到写的这一帧）已被作废清空
            co_return 0;
        }
    }
    // 注：!mine 且未出错时是"别的 pump 协程接管了队列"，本函数不等它写完就返回，
    // 返回值含义是"帧已被队列接走"，不是"已确认写出"。
    co_return queued;
}

//synchronous
unsigned int websocket_client::write(unsigned char *data_out, unsigned int buffersize)
{
    if (iserror || (isssl ? (sslsock == nullptr) : (sock == nullptr)))
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = asio::write(*sslsock, asio::buffer(data_out, buffersize));
        }
        else
        {
            n = asio::write(*sock, asio::buffer(data_out, buffersize));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    return 0;
}

unsigned int websocket_client::write(std::string_view value)
{
    if (iserror || (isssl ? (sslsock == nullptr) : (sock == nullptr)))
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }

    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = asio::write(*sslsock, asio::buffer(value));
        }
        else
        {
            n = asio::write(*sock, asio::buffer(value));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }

    return 0;
}

// 阻塞读：会占住当前线程直到有字节到达。只能在非 io_context 线程上调用，
// 在事件循环线程上调用会冻结该 io_context 上的所有协程。
unsigned int websocket_client::read(unsigned char *buffer_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        return 0;
    }
    atomic_guard guard{socket_read_lock};
    if (iserror)
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = sslsock->read_some(asio::buffer(buffer_data, buffersize));
        }
        else
        {
            n = sock->read_some(asio::buffer(buffer_data, buffersize));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    return 0;
}

unsigned int websocket_client::read(std::string &buffer_data)
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        return 0;
    }
    atomic_guard guard{socket_read_lock};
    if (iserror)
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = sslsock->read_some(asio::buffer(buffer_data));
        }
        else
        {
            n = sock->read_some(asio::buffer(buffer_data));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
    }
    return 0;
}

void websocket_client::close_connect()
{
    // cancel + close 投递到本连接的 strand 上执行，而不是在调用线程直接关：
    // 1) 出站 pump 协程（close 回声、pong、业务写）都在这个 strand 上，strand 队列
    //    先进先出，排到 pump 之后就能等最后一帧写完再关，回声不会被截断；
    // 2) 调用线程（看护线程、业务线程）和 strand 上的在途 async_write 并发操作
    //    同一个 socket 对象是 asio 明确禁止的。
    // 老连接对象的 socket 句柄按值捕获：即使对象复用重连或被析构，这里关掉的仍是
    // 本次拿到的那对 socket，不会误关新连接，也不会漏关。
    asio::post(strand_, [sock = this->sock, sslsock = this->sslsock]()
               {
        std::error_code ec;
        if (sslsock)
        {
            if (sslsock->lowest_layer().is_open())
            {
                sslsock->lowest_layer().cancel(ec);
                sslsock->lowest_layer().close(ec);
            }
        }
        if (sock)
        {
            if (sock->is_open())
            {
                // 先 cancel 再 close，否则 parked 在 async_read 上的常驻协程
                // 可能永远等不到 handler
                sock->cancel(ec);
                sock->close(ec);
            }
        } });
}

// 阻塞读循环，同 read()：只能在非 io_context 线程上调用
void websocket_client::run_loop()
{
    auto self = shared_from_this();
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        return;
    }
    atomic_guard guard{socket_read_lock};

    if (isssl ? (sslsock == nullptr) : (sock == nullptr))
    {
        error_msg = "socket not init";
        iserror   = true;
        return;
    }
    if (data == nullptr)
    {
        data = static_cast<unsigned char *>(std::malloc(512 * sizeof(unsigned char)));
    }
    for (;;)
    {
        if (iserror || isclose_recv_)
        {
            return;
        }
        if (exptime > 0)
        {
            reset_timeout();
        }
        unsigned int n = 0;
        try
        {

            if (isssl)
            {
                if (sslsock->lowest_layer().is_open())
                {
                    n = sslsock->read_some(asio::buffer(data, 512));
                }
                else
                {
                    return;
                }
            }
            else
            {
                if (sock->is_open())
                {
                    n = sock->read_some(asio::buffer(data, 512));
                }
                else
                {
                    return;
                }
            }

            process_data(data, n);

            if (recv_data.isfinish)
            {
                if (run_loop_fun != nullptr)
                {
                    run_loop_fun(self);
                }
                else if (async_run_loop_fun != nullptr)
                {
                    asio::co_spawn(strand_, [self, pack_data = recv_data]() mutable
                                   { return self->async_run_loop_fun(self, pack_data); },
                                   asio::detached);
                }
                reset_recv_status();
            }
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
            return;
        }
    }
    return;
}

asio::awaitable<void> websocket_client::async_run_loop()
{
    auto self = shared_from_this();
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        iserror   = true;
        co_return;
    }
    atomic_guard guard{socket_read_lock};

    if (isssl ? (sslsock == nullptr) : (sock == nullptr))
    {
        error_msg = "socket not init";
        iserror   = true;
        co_return;
    }
    if (data == nullptr)
    {
        data = static_cast<unsigned char *>(std::malloc(512 * sizeof(unsigned char)));
    }
    for (;;)
    {
        if (iserror || isclose_recv_)
        {
            co_return;
        }
        if (exptime > 0)
        {
            reset_timeout();
        }
        unsigned int n = 0;
        try
        {
            if (isssl)
            {
                n = co_await sslsock->async_read_some(asio::buffer(data, 512), asio::use_awaitable);
            }
            else
            {
                n = co_await sock->async_read_some(asio::buffer(data, 512), asio::use_awaitable);
            }

            process_data(data, n);

            if (recv_data.isfinish)
            {
                if (run_loop_fun != nullptr)
                {
                    run_loop_fun(self);
                }
                else if (async_run_loop_fun != nullptr)
                {
                    asio::co_spawn(strand_, [self, pack_data = recv_data]() mutable
                                   { return self->async_run_loop_fun(self, pack_data); },
                                   asio::detached);
                }
                reset_recv_status();
            }
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
            co_return;
        }
    }
    co_return;
}

std::string websocket_client::make_http_header()
{
    if (url.size() < 1)
    {
        error_msg = "url to short";
        iserror   = true;
        return "";
    }
    if (host.size() < 1)
    {
        error_msg = "connect host to short";
        iserror   = true;
        return "";
    }

    std::string send_header_content;

    send_header_content = "GET ";
    send_header_content.append(url);
    send_header_content.append(" HTTP/1.1\r\nHost: ");
    // IPv6 字面值的 host 存的是裸地址（地址解析要用），Host 头里必须加回方括号（RFC 7230 §5.4）
    if (host.find(':') != std::string::npos)
    {
        send_header_content.append("[");
        send_header_content.append(host);
        send_header_content.append("]");
    }
    else
    {
        send_header_content.append(host);
    }
    send_header_content.append("\r\n");

    if (parameter.size() > 0)
    {
        for (unsigned int i = 0; i < parameter.size(); i++)
        {
            send_header_content.append(parameter[i].name);
            send_header_content.append(": ");
            send_header_content.append(parameter[i].value);
            send_header_content.append("\r\n");
        }
    }
    else
    {
        send_header_content.append("User-Agent: paozhu\r\nAccept: */*\r\nAccept-Language: en-US\r\n");
    }

    send_header_content.append("Sec-WebSocket-Version: 13\r\n");
    if (open_deflate)
    {
        // 与服务端对称：强制双向 no-context，每条消息独立 deflate 流；
        // 回包若缺任一参数按协商失败处理（process_handshake 严格校验）
        send_header_content.append(
            "Sec-WebSocket-Extensions: permessage-deflate; server_no_context_takeover; "
            "client_no_context_takeover\r\n");
    }

    send_header_content.append("Sec-WebSocket-Key: ");

    std::random_device rd;
    std::mt19937 gen(rd());
    //定义均匀整数分布，范围是 [0, 255]（闭区间，包含两端）
    std::uniform_int_distribution<int> dis(0, 255);

    for (unsigned int i = 0; i < 16; i++)
    {
        key_str[i] = dis(gen);
    }
    //base64
    send_header_content.append(base64_encode((const char *)key_str, 16, false));
    send_header_content.append("\r\n");

    send_header_content.append("Connection: Upgrade\r\n");
    send_header_content.append("Pragma: no-cache\r\n");
    send_header_content.append("Cache-Control: no-cache\r\n");
    send_header_content.append("Upgrade: websocket\r\n");
    send_header_content.append("\r\n");
    return send_header_content;
}

bool websocket_client::process_handshake(unsigned char *read_data, unsigned int readnum)
{
    offsetnum = 0;
    if (offsetnum >= readnum)
    {
        iserror = true;
        return false;
    }
    for (; offsetnum < readnum; offsetnum++)
    {
        if (read_data[offsetnum] == 0x20)
        {
            break;
        }
    }

    if (offsetnum >= readnum)
    {
        iserror = true;
        return false;
    }
    // 状态行的第一个 token 必须是 "HTTP/1.x"：只查空格后的三个字符会让 "X 101" 也判成握手成功。
    // 这里 offsetnum 就是该 token 的长度（上面那个循环只在遇到空格时停下）。
    if (offsetnum < 8 ||
        !str_casecmp_pre(std::string_view(reinterpret_cast<const char *>(read_data), offsetnum), "HTTP/1."))
    {
        error_msg = "handshake status line error";
        iserror   = true;
        return false;
    }
    offsetnum++;

    if (offsetnum + 3 > readnum)
    {
        iserror = true;
        return false;
    }
    if (read_data[offsetnum] == '1' && read_data[offsetnum + 1] == '0' && read_data[offsetnum + 2] == '1')
    {
        offsetnum = offsetnum + 3;
        for (; offsetnum < readnum; offsetnum++)
        {
            if (read_data[offsetnum] == '\r')
            {
                offsetnum++;
                if (offsetnum < readnum && read_data[offsetnum] == '\n')
                {
                    break;
                }
            }
        }
        if (offsetnum < readnum && read_data[offsetnum] == '\n')
        {
            offsetnum++;
        }
        else
        {
            iserror = true;
            return false;
        }

        unsigned int begin_data_pos = offsetnum;
        for (; offsetnum < readnum; offsetnum++)
        {
            if (read_data[offsetnum] == '\r')
            {
                offsetnum++;
                if (offsetnum < readnum && read_data[offsetnum] == '\n')
                {
                    process_header(read_data, begin_data_pos, offsetnum);
                    begin_data_pos = offsetnum + 1;
                    continue;
                }
            }
        }
    }
    else
    {
        iserror = true;
        return false;
    }

    //handshake
    // 只扫本次握手追加进来的响应头（handshake_resp_begin_ 之后），请求头不参与判定。
    // Accept 与 Extensions 各用一个布尔，谁都不覆盖谁：单个布尔时，Extensions 在前、
    // Accept 在后会把"协商失败"覆盖成成功。
    const unsigned int resp_begin = (handshake_resp_begin_ < parameter.size()) ? handshake_resp_begin_ : static_cast<unsigned int>(parameter.size());
    bool accept_ok                = false;
    // 对端不回 Sec-WebSocket-Extensions ＝ 双方都不压缩，是合法结果；只有回了参数不符的
    // permessage-deflate 才算协商失败
    bool ext_ok      = true;
    bool saw_upgrade = false;
    bool saw_connect = false;
    // Connection 允许 "keepalive, Upgrade" 这类多 token 写法（RFC 7230 §6.1），按 token 逐个比
    auto has_token = [](std::string_view value, std::string_view token) -> bool
    {
        std::size_t start = 0;
        while (start <= value.size())
        {
            std::size_t comma    = value.find(',', start);
            std::size_t stop     = (comma == std::string_view::npos) ? value.size() : comma;
            std::size_t trim_be  = start;
            std::size_t trim_end = stop;
            while (trim_be < trim_end && (value[trim_be] == ' ' || value[trim_be] == '\t'))
            {
                trim_be++;
            }
            while (trim_end > trim_be && (value[trim_end - 1] == ' ' || value[trim_end - 1] == '\t'))
            {
                trim_end--;
            }
            if (str_casecmp(value.substr(trim_be, trim_end - trim_be), token))
            {
                return true;
            }
            if (comma == std::string_view::npos)
            {
                break;
            }
            start = comma + 1;
        }
        return false;
    };
    if (parameter.size() > resp_begin)
    {
        for (unsigned int i = resp_begin; i < parameter.size(); i++)
        {
            if (str_casecmp(parameter[i].name, "Sec-WebSocket-Accept"))
            {
                // 使用请求传过来的KEY+协议字符串，先用SHA1加密然后使用base64编码算出一个应答的KEY
                std::string magicKey = base64_encode((const char *)key_str, 16, false);
                magicKey.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

                unsigned char digest[SHA_DIGEST_LENGTH];
                SHA1((unsigned char *)magicKey.c_str(), magicKey.length(), (unsigned char *)&digest);

                magicKey.clear();
                magicKey = base64_encode((char *)digest, SHA_DIGEST_LENGTH, false);

                // Accept 是 RFC 要求的 MUST 响应头：出现过但值不对，和根本没出现，都是失败
                accept_ok = str_casecmp(parameter[i].value, magicKey);
                if (!accept_ok)
                {
                    error_msg = "response Handshake error";
                }
            }
            else if (str_casecmp(parameter[i].name, "Sec-WebSocket-Extensions"))
            {
                const std::string &ext = parameter[i].value;
                bool offers_deflate    = ext.find("permessage-deflate") != std::string::npos;
                if (offers_deflate && !open_deflate)
                {
                    // 未 offer 却收到 permessage-deflate 应答：对端违规，判握手失败
                    ext_ok    = false;
                    error_msg = "Server negotiated permessage-deflate but it was not offered";
                }
                else if (offers_deflate)
                {
                    // 严格校验双向 no-context：缺任一参数即对端想沿用上下文，
                    // 与本实现的每消息独立流不兼容，按协商失败断连（RFC 6455 §9.1）
                    bool srv_noctx = ext.find("server_no_context_takeover") != std::string::npos;
                    bool cli_noctx = ext.find("client_no_context_takeover") != std::string::npos;
                    if (srv_noctx && cli_noctx)
                    {
                        ext_ok    = true;
                        isdeflate = true;
                    }
                    else
                    {
                        ext_ok    = false;
                        error_msg = "permessage-deflate negotiation mismatch (no-context "
                                    "params required)";
                    }
                }
            }
            else if (str_casecmp(parameter[i].name, "Upgrade"))
            {
                saw_upgrade = has_token(parameter[i].value, "websocket");
            }
            else if (str_casecmp(parameter[i].name, "Connection"))
            {
                saw_connect = has_token(parameter[i].value, "Upgrade");
            }
        }
    }
    else
    {
        error_msg = "response header empty";
    }
    bool isok = accept_ok && ext_ok && saw_upgrade && saw_connect;
    if (!isok && error_msg.empty())
    {
        // 头缺失时给个明确原因，否则调用方只看到 " process_handshake error"
        if (!accept_ok)
        {
            error_msg = "missing Sec-WebSocket-Accept";
        }
        else if (!saw_upgrade)
        {
            error_msg = "missing Upgrade: websocket";
        }
        else if (!saw_connect)
        {
            error_msg = "missing Connection: Upgrade";
        }
        else
        {
            error_msg = "extension negotiation failed";
        }
    }
    return isok;
}

void websocket_client::process_header(unsigned char *read_data, unsigned int data_bein, unsigned int data_end)
{
    struct websocket_parameter_t temp_kv;
    for (; data_bein < data_end; data_bein++)
    {
        if (read_data[data_bein] == ':')
        {
            break;
        }
        else if (read_data[data_bein] == '\r')
        {
            break;
        }
        temp_kv.name.push_back(read_data[data_bein]);
    }

    if (data_bein >= data_end)
    {
        return;// 空行（无 ':' 也无 '\r'），如头部终止符前的残段
    }
    if (read_data[data_bein] == ':')
    {
        data_bein++;
        for (; data_bein < data_end; data_bein++)
        {
            if (read_data[data_bein] != 0x20)
            {
                break;
            }
        }
        for (; data_bein < data_end; data_bein++)
        {
            if (read_data[data_bein] == '\r')
            {
                break;
            }
            temp_kv.value.push_back(read_data[data_bein]);
        }

        parameter.push_back(temp_kv);
    }
    else
    {
        return;
    }
}

asio::awaitable<bool> websocket_client::websocket_handshake()
{
    auto self = shared_from_this();
    // parameter 前半段是请求头（set_header/add_headers），process_header 会把响应头追加在后面；
    // 记下分界，握手校验只扫本次追加的响应头
    handshake_resp_begin_ = static_cast<unsigned int>(parameter.size());
    if (data == nullptr)
    {
        data = static_cast<unsigned char *>(std::malloc(512 * sizeof(unsigned char)));
    }

    if (iserror)
    {
        error_msg = " is has error";
        co_return false;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        std::string send_hand_header = make_http_header();
        if (iserror)
        {
            error_msg = " make_http_header error";
            co_return false;
        }

        if (isssl)
        {
            if (sslsock->lowest_layer().is_open())
            {
                n = co_await asio::async_write(*sslsock, asio::buffer(send_hand_header), asio::use_awaitable);
            }
            else
            {
                error_msg = " sslsock async_write error";
                co_return false;
            }
        }
        else
        {
            if (sock->is_open())
            {
                n = co_await asio::async_write(*sock, asio::buffer(send_hand_header), asio::use_awaitable);
            }
            else
            {
                error_msg = " sock async_write error";
                co_return false;
            }
        }

        // 响应头可能拆包到达：累积读直到出现 \r\n\r\n（总上限 512 字节）
        unsigned int total      = 0;
        unsigned int header_end = 0;
        bool header_done        = false;
        while (!header_done)
        {
            if (total >= 512)
            {
                error_msg = " handshake header too large";
                iserror   = true;
                co_return false;
            }
            if (isssl)
            {
                if (sslsock->lowest_layer().is_open())
                {
                    n = co_await sslsock->async_read_some(asio::buffer(data + total, 512 - total), asio::use_awaitable);
                }
                else
                {
                    error_msg = " sslsock async_read_some error";
                    co_return false;
                }
            }
            else
            {
                if (sock->is_open())
                {
                    n = co_await sock->async_read_some(asio::buffer(data + total, 512 - total), asio::use_awaitable);
                }
                else
                {
                    error_msg = " sock async_read_some error";
                    co_return false;
                }
            }
            if (n == 0)
            {
                error_msg = " handshake closed by peer";
                iserror   = true;
                co_return false;
            }
            total += n;
            // 只需在新旧字节交界处往后找 3 字节即可判定完整终止符
            for (unsigned int j = (total > n && total - n > 3) ? total - n - 3 : 0; j + 3 < total; j++)
            {
                if (data[j] == '\r' && data[j + 1] == '\n' && data[j + 2] == '\r' && data[j + 3] == '\n')
                {
                    header_done = true;
                    header_end  = j + 4;
                    break;
                }
            }
        }

        if (process_handshake(data, total))
        {
            // 101 应答与首帧同包到达时，\r\n\r\n 之后的残包字节喂入帧解析器，
            // 否则这段帧数据被握手路径静默丢弃（残帧不完整时由 process_data 暂存）
            if (total > header_end)
            {
                process_data(data + header_end, total - header_end);
                if (recv_data.isfinish)
                {
                    if (run_loop_fun != nullptr)
                    {
                        run_loop_fun(self);
                    }
                    else if (async_run_loop_fun != nullptr)
                    {
                        asio::co_spawn(strand_, [self, pack_data = recv_data]() mutable
                                       { return self->async_run_loop_fun(self, pack_data); },
                                       asio::detached);
                    }
                    reset_recv_status();
                }
            }
            co_return true;
        }
        else
        {
            error_msg.append(" process_handshake error");
            co_return false;
        }
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror   = true;
        co_return false;
    }

    co_return false;
}

// ---------------------------------------------------------------------------
// 出站帧编码
//   客户端出站帧必须掩码（RFC 6455 §5.1）。服务端 ws_wire.h 的 serialize_frame
// 无掩码参数且声明"服务器→客户端，不掩码"，故客户端自持实现
// ---------------------------------------------------------------------------
bool websocket_client::is_control_frame(ws_opcode op) const
{
    return op == ws_opcode::close || op == ws_opcode::ping || op == ws_opcode::pong;
}

void websocket_client::make_mask_key()
{
    if (mask_key_fixed)
    {
        return;// 测试模式：沿用外部填入的 mask_key，便于用固定向量核对掩码结果
    }
    std::random_device rd;
    for (auto &b : mask_key)
    {
        b = static_cast<unsigned char>(rd());
    }
}

std::string websocket_client::serialize_frame(ws_opcode op, std::string_view payload, bool fin, unsigned char rsv)
{
    // RFC 6455 §5.5：控制帧载荷不得超过 125 字节。
    // 必须截断载荷本身 —— 否则帧头声明的长度小于实际写入字节数，对端读完声明长度后
    // 会把多余字节当成下一帧的帧头，造成流同步丢失（旧 make_pong 就有这个问题）。
    if (is_control_frame(op) && payload.size() > 125)
    {
        payload = payload.substr(0, 125);
    }

    std::string out;
    out.reserve(payload.size() + 14);

    out.push_back(static_cast<char>((fin ? 0x80 : 0x00) | ((rsv & 0x07) << 4) | static_cast<unsigned char>(op)));

    const std::uint64_t len = static_cast<std::uint64_t>(payload.size());
    if (len <= 125)
    {
        out.push_back(static_cast<char>(0x80 | static_cast<unsigned char>(len)));// MASK=1
    }
    else if (len <= 65535)
    {
        out.push_back(static_cast<char>(0x80 | 126));
        out.push_back(static_cast<char>((len >> 8) & 0xFF));
        out.push_back(static_cast<char>(len & 0xFF));
    }
    else
    {
        out.push_back(static_cast<char>(0x80 | 127));
        for (int i = 7; i >= 0; --i)
        {
            out.push_back(static_cast<char>((len >> (8 * i)) & 0xFF));
        }
    }

    // 密钥用局部缓冲：成员 mask_key 会被业务线程与 strand 上的自动 PONG 同时写入，
    // 一旦两次 serialize_frame 交错，帧头里那 4 字节和载荷的 XOR 密钥就可能不是同一份，
    // 对端解出来是乱码。测试模式（mask_key_fixed）仍从成员取值，用固定向量核对掩码结果的通路不变。
    unsigned char key[4];
    if (mask_key_fixed)
    {
        for (int i = 0; i < 4; ++i)
        {
            key[i] = mask_key[i];
        }
    }
    else
    {
        std::random_device rd;
        for (auto &b : key)
        {
            b = static_cast<unsigned char>(rd());
        }
    }
    out.append(reinterpret_cast<const char *>(key), 4);

    // 滚动索引 &3 代替逐字节 %4（与服务端解掩码同型优化）
    unsigned int k = 0;
    for (std::size_t i = 0; i < payload.size(); i++)
    {
        out.push_back(static_cast<char>(static_cast<unsigned char>(payload[i]) ^ key[k]));
        k = (k + 1) & 3;
    }
    return out;
}

std::string websocket_client::make_pong()
{
    return make_pong("");
}

std::string websocket_client::make_pong(std::string_view payload)
{
    return serialize_frame(ws_opcode::pong, payload);
}
// 保活探测帧：空载荷 ping（客户端出站一律带掩码，由 serialize_frame 负责）
std::string websocket_client::make_ping()
{
    return serialize_frame(ws_opcode::ping, std::string_view());
}
// 向后兼容接口：内部转发到 serialize_frame()（README 的 ws client 示例引用了 make_ws_text）。
int websocket_client::make_ws_data(char *msg, unsigned int msgLen, std::string &outBuf)
{
    return make_ws_data(std::string_view(msg, msgLen), outBuf);
}
int websocket_client::make_ws_text(char *msg, unsigned int msgLen, std::string &outBuf)
{
    return make_ws_text(std::string_view(msg, msgLen), outBuf);
}

int websocket_client::make_ws_data(std::string_view msg, std::string &outBuf)
{
    outBuf.append(serialize_frame(ws_opcode::binary, msg));
    return static_cast<int>(outBuf.size());
}
int websocket_client::make_ws_text(std::string_view msg, std::string &outBuf)
{
    outBuf.append(serialize_frame(ws_opcode::text, msg));
    return static_cast<int>(outBuf.size());
}

// ---------------------------------------------------------------------------
// 入站帧流状态机
//   旧 first_parsedata/append_parsedata 把帧边界假定为读边界（一次 read 一帧）：
//   载荷消费不受声明长度截断，一次读到达多帧时下一帧头部被吞进当前消息；
//   帧头跨读、fin=0 的续帧同样直接流错位。现在帧边界与 TCP 读边界解耦：
//   数据消息交付时把本轮未消费字节缓存到 stream_pending_，下次进入优先消费。
//   RFC 6455 §5.1：服务端→客户端帧 MASK 必须为 0，收到 masked 帧按协议错误断连。
//   RSV2/RSV3 必须为 0；RSV1 在已协商 permessage-deflate 时由本状态机自动
//   解压后再交付（recv_data.isdeflate 仅作来源标记），未协商时置 RSV1 断连。
//   控制帧走独立缓冲并内部派发（ping→pong、close→回声后置错），不触碰 recv_data，
//   分片消息进行中途到达的 ping/close 不会破坏组包。
//   入站限额与对端服务端同源：common/cost_define.h 编译期常量。
// ---------------------------------------------------------------------------
static unsigned long long ws_be_read64(const unsigned char *p)
{
    unsigned long long v = 0;
    for (int i = 0; i < 8; i++)
    {
        v = (v << 8) | p[i];
    }
    return v;
}

bool websocket_client::inflate_sink(const unsigned char *out, std::size_t n, void *ud)
{
    return static_cast<websocket_client *>(ud)->on_inflated_bytes(out, n);
}

bool websocket_client::on_inflated_bytes(const unsigned char *out, std::size_t n)
{
    if (recv_data.content.size() + n > CONST_WEBSOCKET_MAX_MESSAGE_SIZE)
    {
        in_msg_too_big_ = true;// 解压后超限（解压炸弹防护），按"消息过长"失败
        return false;
    }
    recv_data.content.append(reinterpret_cast<const char *>(out), n);
    if (recv_data.opcode == 0x01 && !in_utf8_.feed(out, n))
    {
        in_inflate_utf8_bad_ = true;// 解压后的 text 字节非法 UTF-8
        return false;
    }
    return true;
}

unsigned int websocket_client::process_data(unsigned char *inputdata, unsigned int buffersize)
{
    if constexpr (CONST_WEBSOCKET_CLIENT_PING_INTERVAL > 0)
    {
        // 任何入站字节都证明对端还活着（保活 pong 回来的那批也走这里），静默计时从此刻重新起算
        if (buffersize > 0)
        {
            last_rx_sec.store(timeid());
        }
    }
    auto fail = [this](const char *msg) -> unsigned int
    {
        error_msg = msg;
        iserror   = true;
        return 0;
    };

    const unsigned char *p = inputdata;
    unsigned int n         = buffersize;
    std::string joined;
    if (!stream_pending_.empty())
    {
        joined = std::move(stream_pending_);
        stream_pending_.clear();
        joined.append(reinterpret_cast<const char *>(inputdata), buffersize);
        p = reinterpret_cast<const unsigned char *>(joined.data());
        n = static_cast<unsigned int>(joined.size());
    }

    unsigned int pos = 0;
    // in_payload_phase_ 也纳入循环条件：零长末帧（如压缩消息的空收尾片）
    // 恰好落在输入末尾时，头部解析后 pos==n，仍需一次零字节的载荷相位
    // 处理来完成冲净/交付（服务端 ws_parser 同场景有显式守卫）。
    while (pos < n || in_payload_phase_)
    {
        if (!in_payload_phase_)
        {
            // ---- 帧头收集 ----
            while (pos < n && in_hdr_have_ < in_hdr_need_)
            {
                in_hdr_[in_hdr_have_++] = p[pos++];
            }
            if (in_hdr_have_ < in_hdr_need_)
            {
                return static_cast<unsigned int>(recv_data.read_length);// 半截帧头，等下次读
            }
            const unsigned char *h = in_hdr_;
            unsigned char b0       = h[0];
            unsigned char b1       = h[1];
            in_fin_                = (b0 & 0x80) != 0;
            unsigned char op       = static_cast<unsigned char>(b0 & 0x0F);
            if ((b1 & 0x80) != 0)
            {
                return fail("Masked frame from server");
            }
            if ((b0 & 0x30) != 0)
            {
                return fail("RSV2/RSV3 bits must be 0");
            }
            in_frame_rsv1_ = (b0 & 0x40) != 0;
            if (op > 0x02 && op < 0x08)
            {
                return fail("Reserved data opcode");
            }
            if (op > 0x0A)
            {
                return fail("Reserved control opcode");
            }

            unsigned long long len = b1 & 0x7F;
            if (len == 126)
            {
                in_hdr_need_ = 4;
                if (in_hdr_have_ < 4)
                {
                    continue;// 继续收扩展长度字节
                }
                len = (static_cast<unsigned long long>(h[2]) << 8) | h[3];
            }
            else if (len == 127)
            {
                in_hdr_need_ = 10;
                if (in_hdr_have_ < 10)
                {
                    continue;
                }
                len = ws_be_read64(h + 2);
                if (len >> 63)
                {
                    return fail("Invalid 64-bit length");
                }
            }
            in_hdr_have_ = 0;
            in_hdr_need_ = 2;

            in_ctl_ = (op >= 0x08);
            in_op_  = static_cast<ws_opcode>(op);
            if (in_ctl_)
            {
                if (in_frame_rsv1_)
                {
                    return fail("RSV1 set on control frame");
                }
                if (!in_fin_)
                {
                    return fail("Control frame must be FIN");
                }
                if (len > 125)
                {
                    return fail("Control frame too long");
                }
                in_ctl_buf_.clear();
            }
            else
            {
                if (len > CONST_WEBSOCKET_MAX_FRAME_SIZE)
                {
                    return fail("Frame too large");
                }
                if (op == 0x00)
                {
                    if (!in_msg_open_)
                    {
                        return fail("Unexpected continuation frame");
                    }
                    if (in_frame_rsv1_)
                    {
                        return fail("RSV1 set on continuation frame");
                    }
                }
                else
                {
                    if (in_msg_open_)
                    {
                        return fail("Interleaved data frames");
                    }
                    if (in_frame_rsv1_ && !isdeflate)
                    {
                        return fail("RSV1 set but permessage-deflate not negotiated");
                    }
                    recv_data.opcode     = op;
                    recv_data.isdeflate  = in_frame_rsv1_;
                    in_msg_deflated_     = in_frame_rsv1_;
                    in_msg_too_big_      = false;
                    in_inflate_utf8_bad_ = false;
                    if (op == 0x01)
                    {
                        in_utf8_.reset();// 新 text 消息，校验器从头开始
                    }
                }
                // 压缩消息的声明长度是压缩后字节数，与解压后上限不同量纲；
                // 上限改在 inflate sink 里按解压后字节把关（防解压炸弹）。
                if (!in_msg_deflated_ &&
                    recv_data.content.size() + len > CONST_WEBSOCKET_MAX_MESSAGE_SIZE)
                {
                    return fail("Recv data too long");
                }
                in_msg_open_ = true;
            }
            in_payload_left_  = len;
            recv_data.length  = len;// 当前帧声明长度，交付时改写为消息总长
            in_payload_phase_ = true;
            continue;
        }

        // ---- 载荷消费 ----
        unsigned long long avail = n - pos;
        unsigned long long take  = avail < in_payload_left_ ? avail : in_payload_left_;
        if (!in_ctl_ && in_msg_deflated_)
        {
            // 压缩消息：字节先进 inflate 管道，解压结果由 sink 写入 recv_data.content
            // 并做 UTF-8 校验。take==0 也要喂——消息末帧要在此冲净并重置流。
            bool msg_fin = in_fin_ && (take == in_payload_left_);
            if (!in_inflator_.feed(p + pos, static_cast<std::size_t>(take), msg_fin, inflate_sink, this))
            {
                if (in_msg_too_big_)
                {
                    return fail("Recv data too long");
                }
                if (in_inflate_utf8_bad_)
                {
                    return fail("Invalid UTF-8 in text message");
                }
                return fail("Invalid deflate stream in message");
            }
            recv_data.read_length += take;
            pos += static_cast<unsigned int>(take);
            in_payload_left_ -= take;
            if (msg_fin)
            {
                in_msg_deflated_ = false;
            }
        }
        else if (take > 0)
        {
            if (in_ctl_)
            {
                in_ctl_buf_.append(reinterpret_cast<const char *>(p + pos), static_cast<std::size_t>(take));
            }
            else
            {
                recv_data.content.append(reinterpret_cast<const char *>(p + pos), static_cast<std::size_t>(take));
                recv_data.read_length += take;
                // RFC 6455 §5.5/7.1.7：text 消息载荷必须是合法 UTF-8
                if (recv_data.opcode == 0x01 &&
                    !in_utf8_.feed(reinterpret_cast<const unsigned char *>(p + pos),
                                   static_cast<std::size_t>(take)))
                {
                    return fail("Invalid UTF-8 in text message");
                }
            }
            pos += static_cast<unsigned int>(take);
            in_payload_left_ -= take;
        }
        if (in_payload_left_ > 0)
        {
            return static_cast<unsigned int>(recv_data.read_length);// 输入耗尽，帧未完
        }
        in_payload_phase_ = false;

        if (in_ctl_)
        {
            if (in_op_ == ws_opcode::ping)
            {
                auto self        = shared_from_this();
                std::string pong = make_pong(in_ctl_buf_);
                asio::co_spawn(strand_, [self, pong = std::move(pong)]() mutable -> asio::awaitable<void>
                               {
                        // 经出站写队列串行化，避免与业务写并发交错
                        co_await self->async_send_frame(std::move(pong));
                        co_return; },
                               asio::detached);
            }
            else if (in_op_ == ws_opcode::close)
            {
                // 回声 close（携带对端状态码）。iserror 必须等 pump 把回声写完之后再置：
                // pump 的循环头一见 iserror 就收摊，sending_guard 析构会清空队列里的剩余帧，
                // 提前置位等于把这条回声帧丢掉，对端收不到关闭应答。
                auto self = shared_from_this();
                std::string code;
                if (in_ctl_buf_.size() >= 2)
                {
                    code.assign(in_ctl_buf_, 0, 2);
                    // RFC 6455 §7.1.7：close 原因（状态码之后的字节）必须是合法 UTF-8，
                    // 非法按对端协议过错，回声 1002 而非原状态码
                    if (in_ctl_buf_.size() > 2 &&
                        !ws::utf8_valid(std::string_view(in_ctl_buf_).substr(2)))
                    {
                        error_msg = "Invalid UTF-8 in close reason";
                        code.assign("\x03\xEA", 2);
                    }
                }
                std::string echo = serialize_frame(ws_opcode::close, code);
                bool mine        = enqueue_frame(std::move(echo));
                // 收到 close 就不再挂新读：对端可以只发 close 不发 FIN，
                // 那时不设这面旗的读循环会一直 parked 在 async_read_some 上等一个不会来的字节
                isclose_recv_ = true;
                asio::co_spawn(strand_, [self, mine]() mutable -> asio::awaitable<void>
                               {
                        if (mine)
                        {
                            co_await self->pump_send_queue();
                        }
                        self->iserror = true;
                        co_return; },
                               asio::detached);
            }
            // pong：当前无 keepalive 记账需求，忽略
            in_ctl_buf_.clear();
            continue;
        }

        if (in_fin_)
        {
            // text 消息末尾不得残留半截多字节序列
            if (recv_data.opcode == 0x01 && !in_utf8_.complete())
            {
                return fail("Invalid UTF-8 in text message");
            }
            // 消息完整，交付（业务判 recv_data.isfinish）
            in_msg_open_             = false;
            recv_data.fin            = 1;
            recv_data.length         = recv_data.content.size();
            recv_data.read_length    = recv_data.content.size();
            recv_data.total_length   = recv_data.content.size();
            recv_data.isclose_stream = true;
            recv_data.isfinish       = true;
            if (pos < n)
            {
                stream_pending_.assign(reinterpret_cast<const char *>(p + pos), n - pos);
            }
            return static_cast<unsigned int>(recv_data.read_length);
        }
        // fin=0：等下一张续帧头，继续在 while 里消费输入
    }
    return static_cast<unsigned int>(recv_data.read_length);
}

asio::awaitable<void> websocket_client::async_recv_finish()
{
    if (async_recv_finish_fun != nullptr)
    {
        co_await async_recv_finish_fun(shared_from_this());
    }
    co_return;
}

void websocket_client::reset_recv_status()
{
    // 仅供消费方在 isfinish 交付后调用；帧流中间状态（in_hdr_/stream_pending_ 等）不属于这里
    recv_data.isfile         = false;
    recv_data.isfinish       = false;
    recv_data.isdeflate      = false;
    recv_data.isclose_stream = false;
    recv_data.fin            = 0;
    recv_data.opcode         = 0;
    recv_data.length         = 0;
    recv_data.read_length    = 0;
    recv_data.total_length   = 0;
    recv_data.content.clear();
}
// 出站压缩（permessage-deflate 已协商时）：仅 text 开启，binary 不压缩；
// 载荷超过阈值才尝试压缩，raw_deflate_once 在压缩不省时返回 false，此时按原文 RSV1=0 发送。
static std::string make_out_frame(websocket_client &cli, ws_opcode op, std::string_view value)
{
    if (cli.isdeflate && value.size() > CONST_WEBSOCKET_DEFLATE_MIN_SIZE)
    {
        std::string zbuf;
        if (ws::raw_deflate_once(value, zbuf))
        {
            // rsv 字段值 4 = RSV1（0x40 位，RFC 6455/7692）
            return cli.serialize_frame(op, zbuf, true, 4);
        }
    }
    return cli.serialize_frame(op, value);
}
asio::awaitable<unsigned int> websocket_client::async_text_write(std::string_view value)
{
    co_return co_await async_send_frame(make_out_frame(*this, ws_opcode::text, value));
}
asio::awaitable<unsigned int> websocket_client::async_data_write(std::string_view value)
{
    co_return co_await async_send_frame(serialize_frame(ws_opcode::binary, value));
}

asio::awaitable<unsigned int> websocket_client::async_text_read()
{
    if (socket_read_lock.test_and_set())
    {
        error_msg = "Other socket read is set";
        co_return 0;
    }
    atomic_guard guard{socket_read_lock};
    reset_recv_status();
    if (isssl ? (sslsock == nullptr) : (sock == nullptr))
    {
        error_msg = "socket not init";
        iserror   = true;
        co_return 0;
    }
    auto self = shared_from_this();
    if (data == nullptr)
    {
        data = static_cast<unsigned char *>(std::malloc(512 * sizeof(unsigned char)));
    }
    for (;;)
    {
        if (iserror || isclose_recv_)
        {
            co_return 0;
        }
        if (exptime > 0)
        {
            reset_timeout();
        }
        unsigned int n = 0;
        try
        {
            if (isssl)
            {
                n = co_await sslsock->async_read_some(asio::buffer(data, 512), asio::use_awaitable);
            }
            else
            {
                n = co_await sock->async_read_some(asio::buffer(data, 512), asio::use_awaitable);
            }
            process_data(data, n);

            if (recv_data.isfinish)
            {
                co_return recv_data.opcode;
            }
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror   = true;
            co_return 0;
        }
    }
    co_return 0;
}

bool websocket_client::un_pack(const std::string &content, std::string &outBuf)
{
    return http::uncompress(content, outBuf);
}

bool websocket_client::un_pack(std::string &outBuf)
{
    return http::uncompress(recv_data.content, outBuf);
}
int websocket_client::compress(const std::string &pack_data, std::string &outBuf)
{
    if (http::compress(pack_data.data(), pack_data.size(), outBuf, Z_DEFAULT_COMPRESSION) ==
        Z_OK)
    {
        return 0;
    }
    return 1;
}

}// namespace http
