#pragma once
#include <map>
#include "httppeer.h"
#include "websockets.h"
#include "loopwebsockets.hpp"
#include "loopwebsockets_co.hpp"
#include "websockets_callback.h"
namespace http
{
void _initwebsocketmethodregto(WEBSOCKET_REG &methodcallback)
{

    methodcallback.emplace("wstest", [](unsigned int m, unsigned int g) -> std::shared_ptr<websockets_api>
                           { return std::make_shared<loopwebsockets>(m,g); });

    // ws 协程版（isco=true + isloopco=true）
    methodcallback.emplace("wstestco", [](unsigned int m, unsigned int g) -> std::shared_ptr<websockets_api>
                           { return std::make_shared<loopwebsockets_co>(m,g); });

}

}// namespace http