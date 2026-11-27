#ifndef HTTP2_BORROW_STATE_H__
#define HTTP2_BORROW_STATE_H__

#include <asio/async_result.hpp>
#include <asio/awaitable.hpp>
#include <asio/executor.hpp>
#include <asio/post.hpp>
#include <asio/use_awaitable.hpp>
#include <memory>
#include <functional>
#include <mutex>
#include <utility>

namespace http
{

struct h2_borrow_state : std::enable_shared_from_this<h2_borrow_state>
{
    explicit h2_borrow_state(asio::any_io_executor executor) : wait_executor(std::move(executor)) {}

    bool add(unsigned int count)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (done || cancelled || count == 0) return false;
        pending += count;
        return true;
    }

    void release(unsigned int count = 1)
    {
        finish(count, false);
    }

    void cancel()
    {
        finish(0, true);
    }

    asio::awaitable<bool> wait()
    {
        co_await asio::this_coro::executor;
        auto token = asio::use_awaitable_t<>{};
        co_await asio::async_initiate<decltype(token), void()>(
            [self = shared_from_this()](auto &&handler) mutable
            {
                auto completion = std::make_shared<std::decay_t<decltype(handler)>>(
                    std::move(handler));
                std::function<void()> wake;
                {
                    std::lock_guard<std::mutex> lock(self->mutex);
                    if (!self->done && !self->cancelled)
                    {
                        self->continuation = [completion]() mutable
                        {
                            (*completion)();
                        };
                    }
                    else
                    {
                        wake = [completion]() mutable
                        {
                            (*completion)();
                        };
                    }
                }
                if (wake) wake();
            },
            token);
        std::lock_guard<std::mutex> lock(mutex);
        co_return !cancelled;
    }

  private:
    void finish(unsigned int count, bool value)
    {
        std::function<void()> wake;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (done || cancelled || (!value && count == 0)) return;
            if (value || count >= pending)
            {
                done      = true;
                cancelled = value;
                pending   = 0;
                wake      = std::move(continuation);
                continuation = nullptr;
            }
            else
            {
                pending -= count;
            }
        }
        if (wake) asio::post(wait_executor, std::move(wake));
    }

    asio::any_io_executor wait_executor;
    std::mutex mutex;
    unsigned int pending = 0;
    bool done = false;
    bool cancelled = false;
    std::function<void()> continuation;
};

}// namespace http
#endif
