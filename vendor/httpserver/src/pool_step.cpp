#include "pool_step.h"

#include <memory>
#include <type_traits>
#include <utility>

#include "server.h"

namespace http
{

namespace
{

// 全项目唯一的一份「交给 clientrunpool 跑一步」实现：router 的 sync handler、mqtt 的同步钩子、
// socket 的 on_open / on_close 都走这里，拒单就地完成也写在这里，调用方漏不掉。
//
// 机制：fn() 在池线程跑，结果交给 finish；finish 用 asio::dispatch 把唤醒操作投回协程自己的
// 执行域（会话 strand）。awaitable_handler 在哪个线程被调用，协程就在哪个线程续跑，
// 所以业务线程只跑业务函数，续体和响应发送都落在 strand 上。
// 池没接单（停机或扩容中）时立刻就地调用 finish 并把 rejected 置真：静默丢任务等于让这条协程
// 永久挂起，peer 与 h2 stream 都不会回收。
//
// 两个模板的差别只在要不要携带 peer：
//   run_one_step_on_pool（零参版）——任务只带 fn，给不持 httppeer 的调用方用，行为一直未变。
//   run_one_step_with_peer（peer 版）——peer 由任务闭包、finish、唤醒 op 依次各自持有，
//   池任务闭包在池线程析构之后，队列里排队的唤醒 op 仍替协程帧持着一份引用，
//   最后一份在 strand 线程释放。httppeer 同时被发送线程使用
//   （http2_loop_send_sequence 与 http2_send_queue_loop 读 sq_obj->peer / sp->peer），
//   把释放点固定到 strand 线程，发送线程就不会拿到已经释放的对象。
//
// 模板参数叫 Outcome 不叫 OUT：在 Windows 上 <asio.hpp> 会经 asio/detail/socket_types.hpp
// 把 windows.h 拉进这个 TU，而 windows.h 把 OUT 定义成了空宏，于是 asio::awaitable<OUT>
// 变成 awaitable<>、is_same_v<OUT, pool_bool> 变成 <, pool_bool>，
// MSVC 报的是 C2976 too few template arguments 加一串 'out': undeclared identifier。
// 同一个宏簇还有 IN / OPTIONAL，别用它们当标识符。
template <typename Outcome, typename FN>
asio::awaitable<Outcome> run_one_step_on_pool(FN fn, asio::use_awaitable_t<> h)
{
    auto initiate = [fn = std::move(fn)](
                        asio::detail::awaitable_handler<asio::any_io_executor, Outcome> &&handler) mutable
    {
        auto handler_ptr = std::make_shared<
            asio::detail::awaitable_handler<asio::any_io_executor, Outcome>>(std::move(handler));

        // awaitable_handler 在哪个线程被调用，协程就在哪个线程续跑；唤醒回到协程自己的执行域
        // （会话 strand），业务线程只跑 fn()。
        asio::any_io_executor wakeup_ex = handler_ptr->get_executor();

        auto finish = [wakeup_ex, handler_ptr](Outcome out) mutable
        {
            asio::dispatch(wakeup_ex,
                           [handler_ptr, out = std::move(out)]() mutable
                           {
                               (*handler_ptr)(std::move(out));
                           });
        };

        bool accepted = get_server_app().clientrunpool.add_sync_task(
            [fn = std::move(fn), finish]() mutable
            {
                Outcome out;
                try
                {
                    if constexpr (std::is_same_v<Outcome, pool_bool>)
                        out.value = fn();
                    else if constexpr (std::is_same_v<Outcome, pool_text>)
                        out.ret = fn();
                    else
                        fn();
                }
                catch (...)
                {
                    out.eptr = std::current_exception();
                }
                finish(std::move(out));
            });

        if (!accepted)
        {
            // 池在停机或扩容中：就地完成 awaitable。静默丢任务等于让这条协程永久挂起，
            // peer 与 h2 stream 都不会回收。
            Outcome out;
            out.rejected = true;
            finish(std::move(out));
        }
    };
    return asio::async_initiate<asio::use_awaitable_t<>, void(Outcome)>(initiate, h);
}

// peer 版：与上面同一条机制，只多一件事——peer 由任务闭包、finish、唤醒 op 顺次各自持有，
// 于是「池任务闭包在池线程析构」到「协程在 strand 上恢复完」这段窗口里恒有 ≥1 份帧侧之外的强引用，
// 最后一份也在 strand 线程释放，发送线程（队列槽那一份）不可能撞上已释放对象。
template <typename Outcome, typename FN>
asio::awaitable<Outcome> run_one_step_with_peer(FN fn, std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h)
{
    auto initiate = [fn = std::move(fn), peer = std::move(peer)](
                        asio::detail::awaitable_handler<asio::any_io_executor, Outcome> &&handler) mutable
    {
        auto handler_ptr = std::make_shared<
            asio::detail::awaitable_handler<asio::any_io_executor, Outcome>>(std::move(handler));

        asio::any_io_executor wakeup_ex = handler_ptr->get_executor();

        // peer 进 finish：dispatch 把 op 投出去之后，池线程那侧的任务闭包马上析构，
        // 队列里排队的唤醒 op 仍替协程帧兜着一份强引用，那份到 op 在 strand 上析构才放。
        auto finish = [wakeup_ex, handler_ptr, peer](Outcome out) mutable
        {
            asio::dispatch(wakeup_ex,
                           [handler_ptr, peer, out = std::move(out)]() mutable
                           {
                               (*handler_ptr)(std::move(out));
                           });
        };

        // 这里对 peer 取副本而不是 move：发起侧和任务闭包各持一份，闭包那份撑到池线程执行完，
        // 之后 strand 上还有 finish 与唤醒 op 两份。代价是每步多一对原子 inc/dec。
        bool accepted = get_server_app().clientrunpool.add_sync_task(
            [fn = std::move(fn), peer, finish]() mutable
            {
                Outcome out;
                try
                {
                    if constexpr (std::is_same_v<Outcome, pool_bool>)
                        out.value = fn(peer);
                    else if constexpr (std::is_same_v<Outcome, pool_text>)
                        out.ret = fn(peer);
                    else
                        fn(peer);
                }
                catch (...)
                {
                    out.eptr = std::current_exception();
                }
                finish(std::move(out));
            });

        if (!accepted)
        {
            // 拒单：finish 在发起侧同步走 dispatch，peer 在 op 里，协程不挂死、503 路径上
            // peer 的释放仍落在 strand 线程。
            Outcome out;
            out.rejected = true;
            finish(std::move(out));
        }
    };
    return asio::async_initiate<asio::use_awaitable_t<>, void(Outcome)>(initiate, h);
}

}// namespace

asio::awaitable<pool_text> co_pool_run_step(std::function<std::string()> fn, asio::use_awaitable_t<> h)
{
    return run_one_step_on_pool<pool_text>(std::move(fn), h);
}

asio::awaitable<pool_text> co_pool_run_step(std::function<std::string(std::shared_ptr<httppeer> &)> fn,
                                            std::shared_ptr<httppeer> peer,
                                            asio::use_awaitable_t<> h)
{
    return run_one_step_with_peer<pool_text>(std::move(fn), std::move(peer), h);
}

asio::awaitable<pool_bool> co_pool_run_bool(std::function<bool()> fn, asio::use_awaitable_t<> h)
{
    return run_one_step_on_pool<pool_bool>(std::move(fn), h);
}

asio::awaitable<pool_fail> co_pool_run_void(std::function<void()> fn, asio::use_awaitable_t<> h)
{
    return run_one_step_on_pool<pool_fail>(std::move(fn), h);
}

bool post_conn_step(const char *tag, std::function<void()> fn)
{
    return get_server_app().clientrunpool.add_conn_task(tag, std::move(fn));
}

std::string exception_text(const std::exception_ptr &eptr)
{
    if (!eptr)
        return "";
    try
    {
        std::rethrow_exception(eptr);
    }
    catch (const std::exception &e)
    {
        return e.what();
    }
    catch (...)
    {
        return "unknown exception";
    }
}

}// namespace http
