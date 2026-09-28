#include "ws_handshake.h"
#include <openssl/sha.h>
#include "base64.h"

namespace http
{
namespace ws
{

bool validate(const websocket_t &ws, bool has_upgrade, bool has_connection_upgrade,
              unsigned char method)
{
    // 必须是 GET 请求
    if (method != static_cast<unsigned char>(HEAD_METHOD::GET))
        return false;

    if (!has_upgrade || !has_connection_upgrade)
        return false;

    // Sec-WebSocket-Key 不能为空
    if (ws.key.empty())
        return false;

    // 版本必须为 13
    if (ws.version != 13)
        return false;

    return true;
}

std::string compute_accept_key(std::string_view client_key)
{
    const std::string magic("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    std::string server_key = std::string(client_key) + magic;

    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(server_key.c_str()),
         server_key.length(), digest);

    return base64_encode(reinterpret_cast<char *>(digest), SHA_DIGEST_LENGTH, false);
}

std::string make_101(std::string_view client_key, bool deflate)
{
    std::string response = "HTTP/1.1 101 Switching Protocols\r\n";
    response += "Upgrade: websocket\r\n";
    response += "Connection: Upgrade\r\n";
    response += "Sec-WebSocket-Version: 13\r\n";
    if (deflate)
    {
        // 强制双向 no-context：每条消息是独立 deflate 流，连接不保留压缩上下文
        response += "Sec-WebSocket-Extensions: permessage-deflate; "
                    "server_no_context_takeover; client_no_context_takeover\r\n";
    }
    response += "Sec-WebSocket-Accept: ";
    response += compute_accept_key(client_key);
    response += "\r\n\r\n";
    return response;
}

std::string make_error(int status)
{
    std::string body;
    switch (status)
    {
    case 426:
        body = "HTTP/1.1 426 Upgrade Required\r\n"
               "Sec-WebSocket-Version: 13\r\n"
               "Content-Length: 0\r\n\r\n";
        break;
    case 404:
        body = "HTTP/1.1 404 Not Found\r\n"
               "Content-Length: 0\r\n\r\n";
        break;
    case 400:
    default:
        body = "HTTP/1.1 400 Bad Request\r\n"
               "Content-Length: 0\r\n\r\n";
        break;
    }
    return body;
}

}// namespace ws
}// namespace http
