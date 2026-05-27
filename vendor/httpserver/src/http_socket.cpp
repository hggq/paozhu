#include "http_socket.h"
 
namespace http
{
HTTP_SOCKET_REG &get_http_socket_reg()
{
    static HTTP_SOCKET_REG instance;
    return instance;
}
}// namespace http
