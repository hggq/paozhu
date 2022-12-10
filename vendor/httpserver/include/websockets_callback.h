#ifndef __FRAME_WEBSOCKETS_CALLBACK_H__
#define __FRAME_WEBSOCKETS_CALLBACK_H__

#include <iostream>
#include <string>
#include <map>
#include <functional>
#include "request.h"
#include "httppeer.h"
#include "websockets.h"

namespace http
{
    typedef  std::map<std::string,std::function<std::shared_ptr<websockets_api>(std::weak_ptr<httppeer>)>> WEBSOCKET_REG;
    WEBSOCKET_REG &get_websocket_config();
}
#endif
