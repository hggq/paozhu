#include <iostream>
#include <string>
#include <map>
#include <functional>
#include "request.h"
#include "httppeer.h"
#include "websockets.h"
#include "websockets_callback.h"

namespace http
{
WEBSOCKET_REG &get_websocket_reg()
{
    static WEBSOCKET_REG instance;
    return instance;
}
}// namespace http
