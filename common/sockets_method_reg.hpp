#pragma once
#include <map>
#include "httppeer.h"
#include "client_session.h"
#include "http_socket.h"
#include "my_test_socket.hpp"

namespace http
{
void _inithttpsocketmethodregto(HTTP_SOCKET_REG &methodcallback)
{

    methodcallback.emplace("mytestsocket", [](unsigned int myid_,unsigned int groupid_, std::shared_ptr<client_session> s_sock) -> std::shared_ptr<socket_api>
                           { return std::make_shared<my_test_socket>(myid_, groupid_, s_sock); });

}

}// namespace http