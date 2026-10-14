#ifndef __HTTP_POOL_STEP_H
#define __HTTP_POOL_STEP_H

#include <exception>
#include <functional>
#include <memory>
#include <string>

#include <asio.hpp>
#include <asio/io_context.hpp>

namespace http
{

class httppeer;

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
// 零参版给不持 httppeer 的调用方用：mqtt/socket 的同步钩子走 bool/void 两个入口
// （server.cpp 的 co_sync_hook_bool / co_sync_hook_void）。它是框架对外的 API，保留不删。
// HTTP 路由的池步骤走下面的 peer 版：peer 是必传实参，池任务从入队到协程恢复全程持有它。
// 路由侧不要退回「零参版 + 闭包捕获 peer」的写法——那样 peer 的引用在池任务闭包析构时就断了，
// 发送线程可能拿到已经释放的 peer。
asio::awaitable<pool_text> co_pool_run_step(std::function<std::string()> fn,
                                            asio::use_awaitable_t<> h = {});
asio::awaitable<pool_bool> co_pool_run_bool(std::function<bool()> fn,
                                            asio::use_awaitable_t<> h = {});
asio::awaitable<pool_fail> co_pool_run_void(std::function<void()> fn,
                                            asio::use_awaitable_t<> h = {});

// peer 版：业务函数直接吃 peer，同时 peer 作为任务的一等参数贯穿
// 「入队 → 池内执行 → finish → 唤醒 op → 协程在 strand 上恢复完成」整条链，
// 链尾那份引用由会话 strand 线程释放（持有关系写在 pool_step.cpp）。
// 注册表里 pre/regfun 是按值形参（httppeer.h:328/331），装进引用形参的 std::function 合法；
// fn(peer) 时业务函数自己再收一份按值副本, 使用旧流程，协程返回自己的帧线程继续。
asio::awaitable<pool_text> co_pool_run_step(std::function<std::string(std::shared_ptr<httppeer> &)> fn,
                                            std::shared_ptr<httppeer> peer,
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
