#include "http_socket.h"
#include "http2_frame.h"
#include "client_session.h"
#include "http2_parse.h"
#include "terminal_color.h"
#include "http2_define.h"
#include "http2_huffman.h"
#include "https_brotli.h"
#include "unicode.h"
#include "directory_fun.h"
#include "http_header.h"
#include "clientdatacache.h"
namespace http
{
client_session::client_session()
{
    auto &cc    = get_client_data_cache();
    _cache_data = cc.get_data_ptr();
    // cache_back_obj.setptr(_cache_data);
}

client_session::~client_session()
{
    if (_cache_data != nullptr)
    {
        auto &cc = get_client_data_cache();
        cc.back_data_ptr(_cache_data);
    }
}
asio::awaitable<bool>  client_session::read_some(unsigned int& readnum,std::string &log_item)
{
    memset(_cache_data, 0x00, 4096);
    try
    {
        if (isssl)
        {
            readnum = co_await sslsocket->async_read_some(asio::buffer(_cache_data, 4096),asio::use_awaitable);
        }
        else
        {
            readnum = co_await socket->async_read_some(asio::buffer(_cache_data, 4096),asio::use_awaitable);
        }
    }
    catch (const std::exception &e)
    {
        log_item.append(e.what());
        DEBUG_LOG("read_some exception");
        co_return true;
    }
    co_return false;
}
void client_session::clsoesend(asio::io_context &ioc)
{
    try
    {
        std::unique_lock<std::mutex> lock(pop_user_handleer_mutex);
        if (user_code_handler_call.size() > 0)
        {
            //auto ex = asio::get_associated_executor(user_code_handler_call.front());
            asio::dispatch(ioc,
                           [handler = std::move(user_code_handler_call.front())]() mutable -> void
                           {
                               /////////////
                               handler(1);
                               //////////
                           });
            user_code_handler_call.pop_front();
            DEBUG_LOG("peer_session user_code_handler_call return");
        }
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("peer_session user_code_handler_call error");
    }
}
std::shared_ptr<client_session> client_session::get_ptr() { return shared_from_this(); }

bool client_session::send_data(const std::string &msg)
{
    if (isclose)
    {
        return false;
    }
    try
    {
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                asio::write(*sslsocket, asio::buffer(msg));
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (socket->is_open())
            {
                asio::write(*socket, asio::buffer(msg));
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    catch (std::exception &)
    {
        return false;
    }
}
bool client_session::isopensocket()
{

    try
    {
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (socket->is_open())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    catch (std::exception &)
    {
        return false;
    }
}

bool client_session::send_switch101()
{
    try
    {
        std::string tempswitch = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: h2c\r\n\r\n";
        if (isssl)
        {
            asio::write(*sslsocket, asio::buffer(tempswitch));
        }
        else
        {

            asio::write(*socket, asio::buffer(tempswitch));
        }
        return true;
    }
    catch (std::exception &)
    {
        return false;
    }
}
bool client_session::send_setting()
{

    try
    {
        unsigned char _setting[] = {0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x64, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xFF};
        if (sendtype)
        {
            other_msg.append((char *)_setting, 21);
            sendother = true;
        }
        else
        {
            sendtype = true;
            if (isssl)
            {
                asio::write(*sslsocket, asio::buffer(_setting, 21));
            }
            else
            {
                asio::write(*socket, asio::buffer(_setting, 21));
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                http2_send_data_loop();
            }
        }
        //setting_lists.append((char *)_setting, 21);

        return true;
    }
    catch (std::exception &)
    {
        sendtype = false;
        return false;
    }
}

asio::awaitable<void> client_session::co_send_setting()
{

    try
    {
        unsigned char _setting[] = {0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x64, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xFF};

        sendtype = true;
        if (isssl)
        {
            co_await asio::async_write(*sslsocket, asio::buffer(_setting, 21), asio::use_awaitable);
        }
        else
        {
            co_await asio::async_write(*socket, asio::buffer(_setting, 21), asio::use_awaitable);
        }
        sendtype = false;
        co_return;
    }
    catch (std::exception &)
    {
        stop();
        co_return;
    }
}
bool client_session::send_enddata(unsigned int s_stream_id)
{

    try
    {
        unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
        _recvack[8]              = s_stream_id & 0xFF;
        s_stream_id              = s_stream_id >> 8;
        _recvack[7]              = s_stream_id & 0xFF;
        s_stream_id              = s_stream_id >> 8;
        _recvack[6]              = s_stream_id & 0xFF;
        s_stream_id              = s_stream_id >> 8;
        _recvack[5]              = s_stream_id & 0x7F;

        if (sendtype)
        {
            other_msg.append((char *)_recvack, 9);
            sendother = true;
        }
        else
        {
            sendtype = true;
            if (isssl)
            {
                asio::write(*sslsocket, asio::buffer(_recvack, 9));
            }
            else
            {
                asio::write(*socket, asio::buffer(_recvack, 9));
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                http2_send_data_loop();
            }
        }
        //setting_lists.append((char *)_recvack, 9);

        return true;
    }
    catch (std::exception &)
    {
        sendtype = false;
        return false;
    }
}

asio::awaitable<void> client_session::co_send_enddata(unsigned int s_stream_id)
{
    try
    {
        unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
        _recvack[8]              = s_stream_id & 0xFF;
        s_stream_id              = s_stream_id >> 8;
        _recvack[7]              = s_stream_id & 0xFF;
        s_stream_id              = s_stream_id >> 8;
        _recvack[6]              = s_stream_id & 0xFF;
        s_stream_id              = s_stream_id >> 8;
        _recvack[5]              = s_stream_id & 0x7F;

        if (sendtype)
        {
            other_msg.append((char *)_recvack, 9);
            sendother = true;
        }
        else
        {
            sendtype = true;
            if (isssl)
            {
                co_await asio::async_write(*sslsocket, asio::buffer(_recvack, 9), asio::use_awaitable);
            }
            else
            {

                co_await asio::async_write(*socket, asio::buffer(_recvack, 9), asio::use_awaitable);
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                co_await http2_send_data_loop_co();
            }
        }
        //setting_lists.append((char *)_recvack, 9);
        co_return;
    }
    catch (std::exception &)
    {
        sendtype = false;
        co_return;
    }
}

bool client_session::send_ping()
{
    try
    {
        unsigned char _recvack[] =
            {0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        sendtype = true;
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                asio::write(*sslsocket, asio::buffer(_recvack, 17));
            }
        }
        else
        {
            if (socket->is_open())
            {
                asio::write(*socket, asio::buffer(_recvack, 17));
            }
        }
        sendtype = false;
        return true;
    }
    catch (std::exception &)
    {
        sendtype = false;
        return false;
    }
}


asio::awaitable<void> client_session::co_send_goway()
{
    try
    {
        unsigned char _recvack[] ={0x00, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        if (isclose)
        {
            co_return;
        }

        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                co_await asio::async_write(*sslsocket, asio::buffer(_recvack, 17), asio::use_awaitable);
            }
        }
        else
        {
            if (socket->is_open())
            {
                co_await asio::async_write(*socket, asio::buffer(_recvack, 17), asio::use_awaitable);
            }
        }
    }
    catch (...)
    {
        stop();
    }
    co_return;
}

bool client_session::send_goway()
{

    try
    {
        unsigned char _recvack[] =
            {0x00, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        if (sendtype)
        {
            other_msg.append((char *)_recvack, 17);
            sendother = true;
        }
        else
        {
            sendtype = true;
            if (isssl)
            {
                if (sslsocket->lowest_layer().is_open())
                {
                    asio::write(*sslsocket, asio::buffer(_recvack, 17));
                }
            }
            else
            {
                if (socket->is_open())
                {
                    asio::write(*socket, asio::buffer(_recvack, 17));
                }
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                http2_send_data_loop();
            }
        }
        //setting_lists.append((char *)_recvack, 17);
        return true;
    }
    catch (std::exception &)
    {
        sendtype = false;
        return false;
    }
}
bool client_session::send_recv_setting()
{

    try
    {
        unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00};

        if (sendtype)
        {
            other_msg.append((char *)_recvack, 9);
            sendother = true;
        }
        else
        {
            sendtype = true;
            if (isssl)
            {
                asio::write(*sslsocket, asio::buffer(_recvack, 9));
            }
            else
            {
                asio::write(*socket, asio::buffer(_recvack, 9));
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                http2_send_data_loop();
            }
        }
        return true;
    }
    catch (std::exception &)
    {
        sendtype = false;
        return false;
    }
}
void client_session::send_window_update(unsigned int up_num, unsigned int stmid)
{
    unsigned char _recvack[] = {0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    _recvack[12]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[11]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[10]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[9]              = up_num & 0xFF;

    std::string msg;
    msg.reserve(32);
    msg.resize(13);
    for (int i = 0; i < 13; i++)
    {
        msg[i] = _recvack[i];
    }

    _recvack[8] = stmid & 0xFF;
    stmid       = stmid >> 8;
    _recvack[7] = stmid & 0xFF;
    stmid       = stmid >> 8;
    _recvack[6] = stmid & 0xFF;
    stmid       = stmid >> 8;
    _recvack[5] = stmid & 0xFF;

    for (int i = 0; i < 13; i++)
    {
        msg.push_back(_recvack[i]);
    }

    if (sendtype)
    {
        other_msg.append(msg);
        sendother = true;
    }
    else
    {
        try
        {
            sendtype = true;
            if (isssl)
            {
                asio::write(*sslsocket, asio::buffer(msg));
            }
            else
            {
                asio::write(*socket, asio::buffer(msg));
            }
            sendtype = false;
        }
        catch (std::exception &)
        {
            sendtype = false;
            return;
        }
        if (setting_lists.size() > 0)
        {
            http2_send_data_loop();
        }
    }
}
void client_session::recv_window_update(unsigned int up_num, unsigned int stmid)
{
    unsigned char _recvack[] = {0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    _recvack[12]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[11]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[10]             = up_num & 0xFF;
    up_num                   = up_num >> 8;
    _recvack[9]              = up_num & 0xFF;

    std::string msg;
    msg.reserve(32);
    msg.resize(13);
    for (int i = 0; i < 13; i++)
    {
        msg[i] = _recvack[i];
    }

    _recvack[8] = stmid & 0xFF;
    stmid       = stmid >> 8;
    _recvack[7] = stmid & 0xFF;
    stmid       = stmid >> 8;
    _recvack[6] = stmid & 0xFF;
    stmid       = stmid >> 8;
    _recvack[5] = stmid & 0xFF;

    // msg.resize(13);
    for (int i = 0; i < 13; i++)
    {
        msg.push_back(_recvack[i]);
    }

    if (sendtype)
    {
        other_msg.append(msg);
        sendother = true;
    }
    else
    {
        try
        {
            sendtype = true;
            if (isssl)
            {
                asio::write(*sslsocket, asio::buffer(msg));
            }
            else
            {
                asio::write(*socket, asio::buffer(msg));
            }
            sendtype = false;
        }
        catch (std::exception &)
        {
            sendtype = false;
            return;
        }
        if (setting_lists.size() > 0)
        {
            http2_send_data_loop();
        }
    }
}

bool client_session::send_data(const unsigned char *buffer, unsigned int buffersize)
{
    if (isclose)
    {
        return false;
    }

    try
    {
        if (isssl)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                asio::write(*sslsocket, asio::buffer(buffer, buffersize));
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (socket->is_open())
            {
                asio::write(*socket, asio::buffer(buffer, buffersize));
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    catch (std::exception &)
    {
        return false;
    }
}

void client_session::http2_send_data(std::string_view msg)
{
    setting_lists.push(std::string{msg});
    if (!sendtype)
    {
        http2_send_data_loop();
    }
}
void client_session::http2_send_data_loop()
{
    try
    {
        while (true)
        {
            if (setting_lists.size() > 0)
            {
                if (sendtype)
                {
                    return;
                }
                else
                {
                    sendtype = true;
                }
                auto buffer = std::move(setting_lists.front());
                setting_lists.pop();
                if (isssl)
                {
                    asio::write(*sslsocket, asio::buffer(buffer));
                }
                else
                {
                    asio::write(*socket, asio::buffer(buffer));
                }
                sendtype = false;
            }
            else
            {
                sendtype = false;
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        stop();
    }
}
asio::awaitable<void> client_session::http2_send_data_loop_co()
{
    try
    {
        while (true)
        {
            if (setting_lists.size() > 0)
            {

                if (sendtype)
                {
                    co_return;
                }
                else
                {
                    sendtype = true;
                }
                auto buffer = std::move(setting_lists.front());
                setting_lists.pop();
                if (isssl)
                {
                    co_await asio::async_write(*sslsocket, asio::buffer(buffer), asio::use_awaitable);
                }
                else
                {
                    co_await asio::async_write(*socket, asio::buffer(buffer), asio::use_awaitable);
                }
                sendtype = false;
            }
            else
            {
                sendtype = false;
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        stop();
    }
    co_return;
}

asio::awaitable<void> client_session::loopwriter(const unsigned char *buffer, unsigned int buffersize)
{
    try
    {
        if (buffersize == 0)
        {
            co_return;
        }
        if (isssl)
        {
            co_await asio::async_write(*sslsocket, asio::buffer(buffer, buffersize), asio::use_awaitable);
        }
        else
        {
            co_await asio::async_write(*socket, asio::buffer(buffer, buffersize), asio::use_awaitable);
        }
    }
    catch (...)
    {
        stop();
    }
}


asio::awaitable<void> client_session::http2_send_writer(std::string_view msg)
{
    if (isclose)
    {
        co_return;
    }
    try
    {
        if (msg.size() == 0)
        {
            co_return;
        }

        if (sendtype)
        {
            setting_lists.push(std::string{msg});
            co_return;
        }

        if (isssl)
        {
            sendtype = true;
            co_await asio::async_write(*sslsocket, asio::buffer(msg), asio::use_awaitable);
            if (sendother)
            {
                co_await asio::async_write(*sslsocket, asio::buffer(other_msg), asio::use_awaitable);
                other_msg.clear();
                sendother = false;
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                co_await http2_send_data_loop_co();
            }
            co_return;
        }
        else
        {
            sendtype = true;
            co_await asio::async_write(*socket, asio::buffer(msg), asio::use_awaitable);
            if (sendother)
            {
                co_await asio::async_write(*socket, asio::buffer(other_msg), asio::use_awaitable);
                other_msg.clear();
                sendother = false;
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                co_await http2_send_data_loop_co();
            }
            co_return;
        }
    }
    catch (...)
    {
        stop();
    }
    co_return;
}

void client_session::http2_pool_send_data(std::string_view msg)
{
    if (isclose)
    {
        return;
    }
    try
    {
        if (msg.size() == 0)
        {
            return;
        }

        if (sendtype)
        {
            setting_lists.push(std::string{msg});
            return;
        }

        if (isssl)
        {
            sendtype = true;
            asio::write(*sslsocket, asio::buffer(msg));
            if (sendother)
            {
                asio::write(*sslsocket, asio::buffer(other_msg));
                other_msg.clear();
                sendother = false;
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                http2_send_data_loop();
            }
            return;
        }
        else
        {
            sendtype = true;
            asio::write(*socket, asio::buffer(msg));
            if (sendother)
            {
                asio::write(*socket, asio::buffer(other_msg));
                other_msg.clear();
                sendother = false;
            }
            sendtype = false;
            if (setting_lists.size() > 0)
            {
                http2_send_data_loop();
            }
            return;
        }
    }
    catch (...)
    {
        stop();
    }
    return;
}

asio::awaitable<void> client_session::send_writer(const unsigned char *buffer, unsigned int buffersize)
{
    if (isclose)
    {
        co_return;
    }
    try
    {
        if (buffersize == 0)
        {
            co_return;
        }
        if (isssl)
        {
            co_await asio::async_write(*sslsocket, asio::buffer(buffer, buffersize), asio::use_awaitable);
        }
        else
        {
            co_await asio::async_write(*socket, asio::buffer(buffer, buffersize), asio::use_awaitable);
        }
    }
    catch (...)
    {
        stop();
    }
}

asio::awaitable<void> client_session::send_writer(const std::string &msg)
{
    if (isclose)
    {
        co_return;
    }
    try
    {
        if (msg.size() == 0)
        {
            co_return;
        }
        if (isssl)
        {
            co_await asio::async_write(*sslsocket, asio::buffer(msg), asio::use_awaitable);
        }
        else
        {
            co_await asio::async_write(*socket, asio::buffer(msg), asio::use_awaitable);
        }
    }
    catch (...)
    {
        stop();
    }
}

asio::awaitable<void> client_session::co_send_writer(const unsigned char *buffer, unsigned int buffersize)
{
    if (isclose)
    {
        co_return;
    }
    try
    {
        if (buffersize == 0)
        {
            co_return;
        }
        if (isssl)
        {
            co_await asio::async_write(*sslsocket, asio::buffer(buffer, buffersize), asio::use_awaitable);
        }
        else
        {
            co_await asio::async_write(*socket, asio::buffer(buffer, buffersize), asio::use_awaitable);
        }
    }
    catch (...)
    {
        stop();
    }
}

asio::awaitable<void> client_session::co_send_writer(const std::string &msg)
{
    if (isclose)
    {
        co_return;
    }
    try
    {
        if (msg.size() == 0)
        {
            co_return;
        }
        if (isssl)
        {
            co_await asio::async_write(*sslsocket, asio::buffer(msg), asio::use_awaitable);
        }
        else
        {
            co_await asio::async_write(*socket, asio::buffer(msg), asio::use_awaitable);
        }
    }
    catch (...)
    {
        stop();
    }
}

void client_session::stop()
{
    DEBUG_LOG("socket stop");    
    isclose = true;
    try
    {
        if (isssl)
        {
            asio::error_code ec;
            sslsocket->shutdown(ec);
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().close();
            }
        }
        else
        {
            socket->close();
        }
    }
    catch (...)
    {
        DEBUG_LOG("socket exp ");  
    }
    // timer_.cancel();
}

std::string client_session::getremoteip()
{
    if (isssl)
    {
        client_ip = sslsocket->lowest_layer().remote_endpoint().address().to_string();
    }
    else
    {
        client_ip = socket->remote_endpoint().address().to_string();
    }
    return client_ip;
}
unsigned int client_session::getremoteport()
{
    if (isssl)
    {
        client_port = sslsocket->lowest_layer().remote_endpoint().port();
    }
    else
    {
        client_port = socket->remote_endpoint().port();
    }
    return client_port;
}
std::string client_session::getlocalip()
{
    if (isssl)
    {
        server_ip = sslsocket->lowest_layer().local_endpoint().address().to_string();
    }
    else
    {
        server_ip = socket->local_endpoint().address().to_string();
    }
    return server_ip;
}
unsigned int client_session::getlocalport()
{
    if (isssl)
    {
        server_port = sslsocket->lowest_layer().local_endpoint().port();
    }
    else
    {
        server_port = socket->local_endpoint().port();
    }
    return server_port;
}

}// namespace http
