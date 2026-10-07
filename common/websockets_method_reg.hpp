#pragma once
#include <map>
#include "httppeer.h"
#include "websockets.h"
#include "loopwebsockets.hpp"
#include "loopwebsockets_co.hpp"
#include "wssleepwebsockets.hpp"
#include "websockets_callback.h"
namespace http
{
void _initwebsocketmethodregto(WEBSOCKET_REG &methodcallback)
{

    methodcallback.emplace("wstest", [](unsigned int m, unsigned int g) -> std::shared_ptr<websockets_api>
                           { return std::make_shared<loopwebsockets>(m,g); });

    // websocket 钩子跑在协程线程上的版本（isco=true + isloopco=true）
    methodcallback.emplace("wstestco", [](unsigned int m, unsigned int g) -> std::shared_ptr<websockets_api>
                           { return std::make_shared<loopwebsockets_co>(m,g); });

    // 演示「业务钩子阻塞时，这条连接的入站不会被串行」：钩子里睡 3 秒，
    // 回显带 tid= / seq= / q= 三个字段。用法和各字段的含义都写在 wssleepwebsockets.hpp 顶部。
    // 同步钩子版：业务代码交给业务线程池执行，连接照常被框架读下去。
    methodcallback.emplace("wssleep", [](unsigned int m, unsigned int g) -> std::shared_ptr<websockets_api>
                           { return std::make_shared<wssleepwebsockets>(m, g, false); });
    // 同一份代码的协程钩子版：回显的 tid= 用来和上面那条对照，确认两类钩子本来就在不同线程上。
    methodcallback.emplace("wssleepco", [](unsigned int m, unsigned int g) -> std::shared_ptr<websockets_api>
                           { return std::make_shared<wssleepwebsockets>(m, g, true); });

}

}// namespace http