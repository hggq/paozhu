#pragma once
#include <map>
#include "httppeer.h"
#include "websockets.h"
#include "loopwebsockets.hpp"
#include "websockets_callback.h"
namespace http
{
void _initwebsocketmethodregto(WEBSOCKET_REG &methodcallback)
{

    methodcallback.emplace("wstest", [](unsigned int m, unsigned int g) -> std::shared_ptr<websockets_api>
                           { return std::make_shared<loopwebsockets>(m,g); });
 
}

}// namespace http