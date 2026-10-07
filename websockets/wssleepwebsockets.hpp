#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "websockets.h"

namespace http
{

namespace ws_probe_detail
{
// 把"当前是哪条线程在执行我"变成一个可以对比的字符串。
// 两条钩子（下面那个同步版、那个协程版）都会把这个指纹回显给客户端，
// 拿两边的取值范围对一下，就知道各自的代码到底跑在哪一类线程上。
inline std::string tid_text()
{
    return std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
}
}// namespace ws_probe_detail

// -----------------------------------------------------------------------------
// 演示：websocket 的业务钩子里放一段阻塞，会不会把这条连接后面的消息一起堵住。
//
// 结论是「不会，只堵住自己这一条消息」。websocket 是一条双工连接，收帧和跑业务是两件事：
// 框架收到数据消息后把它交给业务线程池去执行，然后立刻回去继续收后面的帧，
// 所以业务钩子睡得再久，连接也不会停，先到的消息排在这条连接自己的接收队列里等着。
//
// 这一份代码注册了两个名字，类的实现完全相同，只是构造时那个 bool 开关不同：
//   /wssleep     同步钩子 onmessage()，走业务线程池
//   /wssleepco   协程钩子 async_onmessage()，走 io/协程线程
// 两条放在一起，是为了拿后者的线程指纹当对照，看这两类钩子的执行位置本来就不一样。
//
// 自己动手验一遍（任意 websocket 客户端都行，比如 wscat -c ws://127.0.0.1/wssleep）：
//   1. 连上 /wssleep
//   2. 发一条以 SLEEP 开头的消息，例如 `SLEEP-first`，这条会在钩子里睡 3 秒
//   3. 不要等回显，马上再发第二条 `second`
//   4. 先看第二条几乎立刻就回来了，再等第一条的 3 秒回显
//   5. 第一条的回显大致是 `SLEEP-first tid=83… seq=0 q=1`
//
// 回显末尾三个字段就是这场演示要看的东西：
//   tid= 执行这条钩子的线程指纹（换一条消息可能就换了一条池线程）
//   seq= 这条消息进接收队列时拿到的序号，用它确认没有消息被跳过
//   q=   取走这一条之后队列里还剩几条；第一条回显里 q>=1，说明那条阻塞的 3 秒里，
//        第二条帧确实已经被读进来了 —— 这就是"入站没有被串行"的直接证据
// -----------------------------------------------------------------------------
class wssleepwebsockets : public websockets_api
{
  public:
    wssleepwebsockets(unsigned int m, unsigned int g, bool coro) : websockets_api(8, m, g, 0)
    {
        isco     = coro;
        isloopco = false;
    }
    ~wssleepwebsockets() {}

    // 同步钩子：框架把这条连接已经收到的数据消息攒在 content_list 里，
    // 这里取一条出来处理。注意先把消息取走、立刻解锁，再往下睡 ——
    // 带着这把锁去睡，读循环就没法把新到的帧放进来了。
    void onmessage() override
    {
        auto self = shared_from_this();
        std::unique_lock<std::mutex> lock(content_list_mutex);
        if (content_list.empty())
        {
            return;
        }
        auto msg = std::move(content_list.front());
        content_list.pop_front();
        lock.unlock();

        // 故意阻塞 3 秒，这段等待就是这场演示的窗口：
        // 期间读循环照常把后面的帧收进 content_list，所以回显里的 q= 会 >=1。
        if (msg.value.rfind("SLEEP", 0) == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        }
        self->send(msg.value + " tid=" + ws_probe_detail::tid_text() +
                   " seq=" + std::to_string(msg.seqid) +
                   " q=" + std::to_string(self->queue_items()));
    }

    // 协程版钩子（注册名 /wssleepco 走这里）：消息由框架直接递进来，不用自己去队列里取，
    // 所以这条不阻塞 —— 它的用途是给上面那个同步版提供一份线程指纹对照。
    // 业务把阻塞调用写进这里同样会拖住协程线程，演示刻意不做。
    asio::awaitable<void> async_onmessage(websockets_data_list_t &&msg) override
    {
        auto self = shared_from_this();
        self->send(msg.value + " tid=" + ws_probe_detail::tid_text());
        co_return;
    }

    // 下面这几个钩子这场演示用不上，但它们在 websockets.h 里是纯虚的，
    // 不给实现这个类就编不过；isco / isloopco 必须在构造时定好，框架造完对象就按它们分派。
    void onopen() override {}
    asio::awaitable<void> async_onopen() override { co_return; }
    void onclose() override { isclose = true; }
    asio::awaitable<void> async_onclose() override { co_return; }
    void onpong() override {}
    void run_loop() override {}
    asio::awaitable<void> async_run_loop() override { co_return; }
};

}// namespace http
