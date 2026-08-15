/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 * @Description: orm_conn_link_t implementation
 */
#include "orm_common.h"

namespace orm
{

std::string to_escape(std::string_view val, bool no_backslash_escapes)
{
    std::string temp;
    temp.reserve(val.size() + val.size() / 8);

    for (char c : val)
    {
        switch (c)
        {
        case '\'':
            // NO_BACKSLASH_ESCAPES 模式下用 '' 转义；否则用 \'
            temp += no_backslash_escapes ? "''" : "\\'";
            break;
        case '"': temp += no_backslash_escapes ? "\"\"" : "\\\""; break;
        case '\\': temp += no_backslash_escapes ? "\\" : "\\\\"; break;
        case '\0': temp += "\\0"; break;
        case '\n': temp += "\\n"; break;
        case '\r': temp += "\\r"; break;
        case '\x1a': temp += "\\Z"; break;
        default: temp.push_back(c); break;
        }
    }
    return temp;
}

std::unique_ptr<orm_conn_link_t> orm_conn_link_t::create(asio::io_context &ioc, DB_TYPE type)
{
    auto link     = std::make_unique<orm_conn_link_t>();
    link->io_ctx  = &ioc;
    link->db_type = type;
    link->socket  = std::make_unique<asio::ip::tcp::socket>(ioc);
    return link;
}

void orm_conn_link_t::close()
{
    if (sslsocket)
    {
        asio::error_code ignore_ec;
        sslsocket->lowest_layer().cancel(ignore_ec);
        sslsocket->lowest_layer().close(ignore_ec);
        sslsocket.reset();
    }
    if (socket && socket->is_open())
    {
        asio::error_code ignore_ec;
        socket->close(ignore_ec);
        socket.reset();
    }
    if (localsocket && localsocket->is_open())
    {
        asio::error_code ignore_ec;
        localsocket->close(ignore_ec);
        localsocket.reset();
    }
    ssl_context.reset();
}

bool orm_conn_link_t::is_closed() const
{
    if (sslsocket)
    {
        return !sslsocket->lowest_layer().is_open();
    }
    if (socket)
    {
        return !socket->is_open();
    }
    if (localsocket)
    {
        return !localsocket->is_open();
    }
    return true;
}

}// namespace orm
