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
template <typename OUT, typename FN>
asio::awaitable<OUT> run_one_step_on_pool(FN fn, asio::use_awaitable_t<> h)
{
    auto initiate = [fn = std::move(fn)](
                        asio::detail::awaitable_handler<asio::any_io_executor, OUT> &&handler) mutable
    {
        auto handler_ptr = std::make_shared<
            asio::detail::awaitable_handler<asio::any_io_executor, OUT>>(std::move(handler));

        // awaitable_handler 在哪个线程被调用，协程就在哪个线程续跑；唤醒回到协程自己的执行域
        // （会话 strand），业务线程只跑 fn()。
        asio::any_io_executor wakeup_ex = handler_ptr->get_executor();

        auto finish = [wakeup_ex, handler_ptr](OUT out) mutable
        {
            asio::dispatch(wakeup_ex,
                           [handler_ptr, out = std::move(out)]() mutable
                           { (*handler_ptr)(std::move(out)); });
        };

        bool accepted = get_server_app().clientrunpool.add_sync_task(
            [fn = std::move(fn), finish]() mutable
            {
                OUT out;
                try
                {
                    if constexpr (std::is_same_v<OUT, pool_bool>)
                        out.value = fn();
                    else if constexpr (std::is_same_v<OUT, pool_text>)
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
            OUT out;
            out.rejected = true;
            finish(std::move(out));
        }
    };
    return asio::async_initiate<asio::use_awaitable_t<>, void(OUT)>(initiate, h);
}

}// namespace

asio::awaitable<pool_text> co_pool_run_step(std::function<std::string()> fn, asio::use_awaitable_t<> h)
{
    return run_one_step_on_pool<pool_text>(std::move(fn), h);
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
