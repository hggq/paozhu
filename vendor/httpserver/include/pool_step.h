#ifndef __HTTP_POOL_STEP_H
#define __HTTP_POOL_STEP_H

#include <exception>
#include <functional>
#include <string>

#include <asio.hpp>
#include <asio/io_context.hpp>

namespace http
{

// 一次「把同步业务函数交给业务线程池（clientrunpool）跑」的结局。
// eptr = 业务函数抛出；rejected = 池没接单（停机或扩容中）。
struct pool_fail
{
    std::exception_ptr eptr;
    bool rejected = false;
};
struct pool_bool : pool_fail
{
    bool value = false;
};
struct pool_text : pool_fail
{
    std::string ret;
};

// 三个入口只在协程里 co_await：fn() 跑在业务池线程，续体回到本协程的执行域。
// 默认参数用 {} 而非 asio::use_awaitable：两者等价，调用点少写一个实参。
// 池没接单时不会把协程挂住——以 rejected=true 就地完成，调用方必须读这个标志。
asio::awaitable<pool_text> co_pool_run_step(std::function<std::string()> fn,
                                            asio::use_awaitable_t<> h = {});
asio::awaitable<pool_bool> co_pool_run_bool(std::function<bool()> fn,
                                            asio::use_awaitable_t<> h = {});
asio::awaitable<pool_fail> co_pool_run_void(std::function<void()> fn,
                                            asio::use_awaitable_t<> h = {});

// eptr 的文本形态；空指针返回空串
std::string exception_text(const std::exception_ptr &eptr);

// conn_tasks：连接类任务（ws 数据消息）投完就走 —— 调用方不 await、不等完成，
// 所以双工连接的读循环不会被一个慢钩子按住。
// tag 必须是静态字符串常量（诊断页把它写进取货线程的 url 字段）。
// 返回 false = 池没接单（停机/扩容中，或测试钩子），这条任务已计数并丢弃，不会执行。
bool post_conn_step(const char *tag, std::function<void()> fn);

}// namespace http

#endif
