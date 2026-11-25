#ifndef HTTP2_BORROW_STATE_H__
#define HTTP2_BORROW_STATE_H__

#include <asio/async_result.hpp>
#include <asio/awaitable.hpp>
#include <asio/executor.hpp>
#include <asio/post.hpp>
#include <asio/use_awaitable.hpp>
#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace http
{

struct h2_borrow_state : std::enable_shared_from_this<h2_borrow_state>
{
    explicit h2_borrow_state(asio::any_io_executor executor) : wait_executor(std::move(executor)) {}

    bool add(unsigned int count)
    {
        if (count == 0)
            return true;
        std::lock_guard<std::mutex> lock(mutex);
        if (cancelled)
            return false;
        pending += count;
        return true;
    }

    void release(unsigned int count = 1)
    {
        if (count == 0)
            return;
        std::function<void()> wake;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (cancelled)
                return;
            assert(pending >= count);
            pending -= count;
            if (pending != 0 || !continuation)
                return;
            wake = std::move(continuation);
            continuation = nullptr;
        }
        wake();
    }

    void cancel()
    {
        std::function<void()> wake;
        {
            std::lock_guard<std::mutex> lock(mutex);
            cancelled = true;
            pending   = 0;
            if (!continuation)
                return;
            wake = std::move(continuation);
            continuation = nullptr;
        }
        wake();
    }

    asio::awaitable<bool> wait()
    {
        co_await asio::this_coro::executor;
        co_await asio::async_initiate<void(bool)>(
            [self = shared_from_this()](auto &&handler) mutable
            {
                auto completion = std::make_shared<std::decay_t<decltype(handler)>>(std::move(handler));
                auto wake = [self, completion]()
                {
                    bool value = false;
                    {
                        std::lock_guard<std::mutex> lock(self->mutex);
                        value = !self->cancelled;
                    }
                    asio::post(self->wait_executor,
                               [completion, value]()
                               {
                                   (*completion)(value);
                               });
                };
                bool ready = false;
                {
                    std::lock_guard<std::mutex> lock(self->mutex);
                    ready = self->pending == 0 || self->cancelled;
                    if (!ready)
                        self->continuation = wake;
                }
                if (ready)
                    wake();
            },
            asio::use_awaitable);
        std::lock_guard<std::mutex> lock(mutex);
        co_return !cancelled;
    }

  private:
    asio::any_io_executor wait_executor;
    std::mutex mutex;
    unsigned int pending = 0;
    bool cancelled       = false;
    std::function<void()> continuation;
};

}// namespace http
#endif
