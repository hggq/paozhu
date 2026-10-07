#pragma once

// Socket 业务示例的同步钩子版，和 my_test_socket.hpp / async_my_test_socket.hpp 成对读：
// 那两份的报文都进 async_on_message（协程钩子，内联跑在会话协程上，钩子多久这条连接的读就停多久）；
// 这一份把 issyncmsg 置成 true，报文改走「接收队列 + 同步钩子」——读环只负责把读到的字节收进队列，
// 钩子在业务线程池上执行，读环投完就走，所以钩子里放阻塞调用也不会把这条连接按住。

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include "httppeer.h"
#include "http_socket.h"

namespace http
{

namespace sync_sock_probe_detail
{
// 线程身份的十进制指纹。算法与 my_test_socket.hpp 里的 sock_probe_detail 逐字相同 ——
// 回显里的 tid= 要能和另外几条演示的 tid= 直接对上（同一个数代表同一条线程），
// 换了算法两边就再也对不上，所以这里照抄而不是另起一份。
inline std::string tid_text(std::thread::id id)
{
    return std::to_string(std::hash<std::thread::id>{}(id));
}
inline std::string tid_now()
{
    return tid_text(std::this_thread::get_id());
}
// 单调时钟的毫秒值。和框架填进 msg.arrived_ms 的那个是同一个钟，两边相减才是排队时长。
inline unsigned long long now_ms()
{
    return static_cast<unsigned long long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
}
// 取命令词后面紧着的那串数字，遇到第一个非数字就停：一次读上来的可能是粘在一起的好几条数据，
// 后面的数字跟这条命令没关系。一个数字都没有时返回 false，由调用方自己定默认值。
inline bool parse_count_after(const std::string &text, std::size_t prefix_len, unsigned long long &out)
{
    out        = 0;
    bool found = false;
    for (std::size_t i = prefix_len; i < text.size(); i++)
    {
        if (text[i] < '0' || text[i] > '9')
        {
            break;
        }
        out   = out * 10 + (unsigned long long)(text[i] - '0');
        found = true;
    }
    return found;
}
}// namespace sync_sock_probe_detail

// -----------------------------------------------------------------------------
// 演示：socket（tcp 长连接）的业务钩子里放一段阻塞，这条连接后面的数据会不会被一起堵住。
//
// 结论是「读不会停，处理按到达顺序一片接一片」。同步钩子内框架把这条连接读到的数据攒在
// 它自己的接收队列里，交给业务线程池执行，读环投完就回去收下一片，所以钩子睡 1.5 秒，
// 这 1.5 秒里对端发来的数据照样被读上来、照样排在队列里等着。
//
// 和 websocket 的差别只有一处：websocket 的入站是一条条完整的帧，可以并发派发；
// socket 的入站是字节流上的一片（一条消息可能被读成两片，几条短的也可能粘成一片），
// 两片同时交给业务就会把这条流咬乱。所以同一条连接同一时刻只跑一个钩子，队列严格按到达
// 顺序出片；不同连接之间仍然完全并行。
//
// 自己动手验一遍（任意 tcp 客户端都行，连上后先写握手再发数据）：
//   1. 发握手 `tcp mytestsocketsync/777` + 换行，再跟一个空行（少了那个空行服务端会一直等）
//   2. 发一条以 SLEEP 开头的行，例如 `SLEEP-first`，这一条会在钩子里睡 1.5 秒
//   3. 不等回显，马上再发第二条 `second`
//   4. 两条回显都会立刻出现在连接上（第二条要等第一条的 1.5 秒跑完才轮到它）
//
// 每行回显末尾这几个字段就是这场演示要看的东西：
//   seq=  这一片进接收队列时拿到的序号，用它确认没有片被跳过、也没有并发插队
//   got=  这一片被服务端读上来的时刻（毫秒）
//   did=  处理它的钩子开始跑的时刻；did= 减 got= 就是这一片在队列里排了多久
//   tid=  执行这条钩子的线程指纹（同步钩子在业务线程池上，不是 io 线程，也不是协程线程）
//   q=    这一片处理完时队列里还剩几片：处理期间对端发来的片如果被读进来了，这里就 >=1
//   held= 这一片准备发出去时，出站那侧还压着多少字节没投出去
//   retry= 到目前为止重投成功的拍数，非零就说明发送环满过一次
//   data= 这一片的原文，原样带回；把各行 data= 依次拼起来应当等于发出去的全部字节
//
// 要睡多久可以自己定：以 SLEEP 开头、后面紧着毫秒数就行，`SLEEP`（不带数字）按 1500 毫秒算，
// 上限钳在 15000 毫秒 —— 这一段阻塞占的是业务线程池里的一条线程，别拿它睡太久。
//
// -----------------------------------------------------------------------------
// 出站这一半走的是另一条路：send() 把数据投进这条连接的发送环（16 格，实际装 15 片），
// 由环上一个消费者协程串行写出去。为什么要环：报文钩子在业务线程池上、周期钩子 run_loop()
// 在框架的定时线程上，两边都能发数据，同时裸写同一个 socket 就会把两段字节咬交错。
//
// 环满了 send() 就返回 false，那一片还在调用方手里。这份演示把它压着，等下一拍（一秒一拍）
// 由 run_loop() 再投一次；压着存货的时候不再单独投新的，新的跟在它后面一起走，顺序就不会破。
//
// 两个词可以自己试：
//   `TICK5`   往后 5 拍（每拍 1 秒）各发一行 `tick=...`。再发一条 `SLEEP3000`，
//             会看到 tick= 那几行在 SLEEP 的回显之前就陆续到达 —— 出站不排在入站钩子后面，
//             这条连接是收、发各自往前走的。
//   `RING40`  当场发 40 行出去（每行填到 32 KB，上限 120 行）。要看见背压就发完立刻把客户端
//             的读停住（不 recv）：消费者的 write 停在 socket 上，十几片之内环就满，
//             被拒的那些会压在 pending 里等重投；等你重新开始读，这 40 行按原顺序一整块回来，
//             回显里 retry= 也 >=1，连接关闭时那条日志里 refuse= 同样 >=1。
//             行小是看不到的 —— 内核发送缓冲先替你把几个字节吞了，环永远不会满。
// 命令后面的数字要紧贴在词后面（`RING40`、`TICK5`），理由和 SLEEP 一样：
// 一次读上来的可能是粘在一起的好几条数据，中间隔了空格这串数字就不归这条命令了。
// -----------------------------------------------------------------------------
class sync_my_test_socket : public socket_api
{
  public:
    // socket_api(1, ...) 的第一个参数是 timeloop_num：定时线程上按 fps % timeloop_num == 0
    // 那一拍调一次周期钩子，而 fps 每秒才加一 —— 所以它的单位是秒，写 7 就是每 7 秒才重投一次。
    // isloopco 保持 false：周期钩子走同步的 run_loop()，它跑在定时线程上，里面不许阻塞。
    sync_my_test_socket(unsigned int m, unsigned int g, std::shared_ptr<client_session> s_sock)
        : socket_api(1, m, g, 0, s_sock)
    {
        issyncmsg = true;
        isco      = false;
        isloopco  = false;
    }
    ~sync_my_test_socket() override
    {
        isclose = true;
    }

  public:
    void on_open() override {}
    asio::awaitable<void> async_on_open() override { co_return; }
    void on_close() override { isclose = true; }
    asio::awaitable<void> async_on_close() override { co_return; }

    // 周期钩子：跑在框架的定时线程上（既不是这条连接的 strand，也不是业务线程池）。
    // 每一拍先把手里压着没投出去的字节再投一次，投掉了才允许这一拍的周期行走前面。
    void run_loop() override
    {
        std::lock_guard<std::mutex> lk(pending_mutex);

        if (!pending_line.empty())
        {
            const std::string waiting = pending_line;
            if (send(waiting))
            {
                // waiting 是这一刻之前的全部存货，所以从头上把它切掉；
                // 切完之后如果还剩下这期间新压进来的尾巴，下一拍接着投。
                pending_line.erase(0, waiting.size());
                retry_done++;
            }
            else
            {
                // 环还是满的：这一拍连周期行也不投，免得排在前面的字节被后来者越过。
                return;
            }
        }

        if (tick_left == 0)
        {
            return;
        }
        tick_left--;
        tick_done++;
        std::string line;
        line.append("tick=").append(std::to_string(tick_done)).append(" tid=").append(sync_sock_probe_detail::tid_now()).append(" left=").append(std::to_string(tick_left)).append(" retry=").append(std::to_string(retry_done)).append(" data=tick\n");
        emit_locked(line);
    }
    asio::awaitable<void> async_run_loop() override
    {
        run_loop();
        co_return;
    }

  private:
    // 出站手里压着、还没投进环的字节。on_message 在业务线程池上跑、run_loop 在定时线程上跑，
    // 两边都会碰这几格，所以统一由 pending_mutex 圈住。
    std::mutex pending_mutex;
    std::string pending_line;
    unsigned long long retry_done      = 0;// 重投成功的拍数
    unsigned long long tick_left       = 0;// 还欠几拍周期行
    unsigned long long tick_done       = 0;// 已经发出几拍周期行
    unsigned long long pending_dropped = 0;// 因 pending 超 kPendingLimit 被丢的片数

    // 手里压着、还没投进环的字节上界。环持续满时若不做这个闸，pending_line 会无限增长
    // 把内存吃爆（RING120 × 32KB 一次就能攒到 ~3.9MB，业务持续发就持续涨）。
    static constexpr std::size_t kPendingLimit = 1024 * 1024;// 1 MiB

    // 投一片出去，进不了环就压在手里。调用前必须已经持有 pending_mutex。
    void emit_locked(std::string &line)
    {
        if (!pending_line.empty())
        {
            // 前面还有存货，这一条只能跟在它后面 —— 单独先发就把这条流的顺序破了。
            // 超过上界则宁肯丢这一片也别让 pending 无界增长（极端背压时才到）。
            if (pending_line.size() + line.size() > kPendingLimit)
            {
                pending_dropped++;
                return;
            }
            pending_line.append(line);
            return;
        }
        if (!send(line))
        {
            if (line.size() > kPendingLimit)
            {
                pending_dropped++;
                return;
            }
            pending_line = line;
        }
    }
    void emit(std::string &line)
    {
        std::lock_guard<std::mutex> lk(pending_mutex);
        emit_locked(line);
    }

  public:
    // 入站的同步钩子：一条连接同一时刻只有一个这样的任务在跑，msg 已经是队列的队头那片。
    // 这里是业务线程池上的普通线程，可以放开手做阻塞的事（查库、调下游、睡一觉都行），
    // 它按住的是池里的一条线程，不是这条连接的读循环。
    void on_message(socket_data_list_t &&msg) override
    {
        const unsigned long long seq        = msg.seqid;
        const unsigned long long arrived_ms = msg.arrived_ms;
        const unsigned long long begin_ms   = sync_sock_probe_detail::now_ms();
        std::string data                    = std::move(msg.value);

        // 想让钩子当场抛一次就发 "THROW"：框架接住、记一条日志、把单飞门开回来，
        // 这条连接接着收发——业务抛异常不该比业务返回失败造成更大的破坏。
        if (data.rfind("THROW", 0) == 0)
        {
            throw std::runtime_error("sync socket demo: on_message throws on purpose");
        }

        if (data.rfind("SLEEP", 0) == 0)
        {
            // 只认 SLEEP 后面紧着的那串数字，没给就按 1500 毫秒算，上限钳在 15000 毫秒。
            unsigned long long sleep_ms = 0;
            if (!sync_sock_probe_detail::parse_count_after(data, 5, sleep_ms))
            {
                sleep_ms = 1500;
            }
            if (sleep_ms > 15000)
            {
                sleep_ms = 15000;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        }

        if (isclose || !session_sock)
        {
            return;
        }
        // 这一片已经处理完了，此刻队列里还剩几片就是"刚才那段处理期间读进来的存货"。
        const unsigned long long queued = queue_items();

        // 两个让出站自己动起来命令：TICK 只在钩子里记账，周期行归 run_loop() 一拍一拍发；
        // RING 当场连发若干行，用来把发送环顶满，看 send() 返回 false 之后数据怎么自己接住。
        if (data.rfind("TICK", 0) == 0)
        {
            unsigned long long n = 0;
            sync_sock_probe_detail::parse_count_after(data, 4, n);
            if (n > 100000)
            {
                n = 100000;
            }
            std::lock_guard<std::mutex> lk(pending_mutex);
            tick_left += n;
        }
        if (data.rfind("RING", 0) == 0)
        {
            unsigned long long n = 0;
            sync_sock_probe_detail::parse_count_after(data, 4, n);
            if (n > 120)
            {
                n = 120;
            }
            // 每行填到 32 KB：发送环只有 16 格（实际装 15 片），对端不读的时候消费者会
            // 停在 socket 上，几片之内环就满 —— 行太小（比如一行几个字节）内核发送缓冲
            // 先吃得下全部，就永远看不到 send() 返回 false。
            const std::string ring_pad(32768, 'x');
            for (unsigned long long i = 1; i <= n; i++)
            {
                std::string body = "R" + std::to_string(i) + ring_pad;
                std::string ring_line;
                ring_line.append("ring=").append(std::to_string(i)).append(" tid=").append(sync_sock_probe_detail::tid_now()).append(" len=").append(std::to_string(body.size())).append(" data=").append(body).append("\n");
                emit(ring_line);
            }
        }

        unsigned long long held    = 0;
        unsigned long long retried = 0;
        {
            std::lock_guard<std::mutex> lk(pending_mutex);
            held    = pending_line.size();
            retried = retry_done;
        }
        std::string line;
        line.append("seq=").append(std::to_string(seq)).append(" got=").append(std::to_string(arrived_ms)).append(" did=").append(std::to_string(begin_ms)).append(" tid=").append(sync_sock_probe_detail::tid_now()).append(" q=").append(std::to_string(queued)).append(" held=").append(std::to_string(held)).append(" retry=").append(std::to_string(retried)).append(" len=").append(std::to_string(data.size())).append(" data=").append(data);
        line.push_back('\n');
        // 同步钩子跑在池线程上，出站照样只用 send()：入环这一步不阻塞调用方，
        // 环满了这一片就压在 pending_line 里，等定时线程那一拍重投。
        emit(line);
    }
};

}// namespace http
