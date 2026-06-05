#include "http_socket_client.h"
#include "client_context.h"

namespace http
{
 socket_client::socket_client():strand_(asio::make_strand(*(get_client_context_obj().ioc))){};
 socket_client::~socket_client()
{
    if(data != nullptr)
    {
        std::free(data);
    }
}
void socket_client::reset()
{
    iserror = false;
    isssl = false;
    isbody =false;
    isfinish = false;
    iswait_exit = false;
    exptime = 0;
    cur_process_type = 0;
    offsetnum = 0;
    port=0;
    val_size = 0;
    timeout_end = 0;

    url.clear();
    host.clear();
    error_msg.clear();
 
    parameter.clear();
    close_connect();
    sock.reset();
    sslsock.reset();
    ssl_context.reset();
}

 void socket_client::set_header(std::string_view name,std::string_view value)
{
    socket_parameter_t a;
    a.name = name;
    a.value = value;
    parameter.emplace_back(a);
}
void socket_client::set_port(unsigned int n)
{
    port = n;
}
void socket_client::set_host(std::string_view name)
{
    host = name;
}

void socket_client::set_url(std::string_view name)
{
    if(name.size() > 7)
    {
        if(name[0]=='h' && name[1]=='t' && name[2]=='t' && name[3]=='p' && name[4]==':' && name[5]=='/'  && name[6]=='/'  )
        {
            std::string temp_str;
            unsigned int i=7;
            for(; i < name.size(); i++)
            {
                if(name[i] == '/')
                {
                    if(temp_str.size() > 0)
                    {
                        host = temp_str;
                    }
                    temp_str.clear();
                    break;
                }
                else if(name[i] == ':')
                {
                    if(temp_str.size() > 0)
                    {
                        host = temp_str;
                    }
                    temp_str.clear();
                    port = 0;
                    for(; i < name.size(); i++)
                    {
                        if(name[i] == '/')
                        {
                            break;
                        }
                        if(name[i] >= '0' && name[i] <= '9')
                        {
                            port = port * 10 + (name[i]- '0');
                        }
                    }    
                    break;
                }
                temp_str.push_back(name[i]);
            }
            for(; i < name.size(); i++)
            {
                if(url.size() == 0 && name[i] == '/')
                {
                    continue;
                }
                url.push_back(name[i]);
            }
            if(port == 0)
            {
                port = 80;
            }
        }
        else if(name[0]=='h' && name[1]=='t' && name[2]=='t' && name[3]=='p'  && name[4]=='s' && name[5]==':' && name[6]=='/'  && name[7]=='/'  )
        {
            std::string temp_str;
            unsigned int i=8;
            isssl = true;
            for(; i < name.size(); i++)
            {
                if(name[i] == '/')
                {
                    if(temp_str.size() > 0)
                    {
                        host = temp_str;
                    }
                    
                    break;
                }
                else if(name[i] == ':')
                {
                    if(temp_str.size() > 0)
                    {
                        host = temp_str;
                    }
                    
                    port = 0;
                    for(; i < name.size(); i++)
                    {
                        if(name[i] == '/')
                        {
                            break;
                        }
                        if(name[i] >= '0' && name[i] <= '9')
                        {
                            port = port * 10 + (name[i]- '0');
                        }
                    }    
                    break;
                }
                temp_str.push_back(name[i]);
            }
            for(; i < name.size(); i++)
            {
                if(url.size() == 0 && name[i] == '/')
                {
                    continue;
                }
                url.push_back(name[i]);
            }
            if(port == 0)
            {
                port = 443;
            } 
        }
        else if(name[0]>='0' && name[0] <= '9')
        {
            std::string temp_str;
            unsigned int i=0;
            for(; i < name.size(); i++)
            {
                if(name[i] == '/')
                {
                    if(temp_str.size() > 0)
                    {
                        host = temp_str;
                    }
                    temp_str.clear();
                    break;
                }
                else if(name[i] == ':')
                {
                    if(temp_str.size() > 0)
                    {
                        host = temp_str;
                    }
                    temp_str.clear();
                    port = 0;
                    for(; i < name.size(); i++)
                    {
                        if(name[i] == '/')
                        {
                            break;
                        }
                        if(name[i] >= '0' && name[i] <= '9')
                        {
                            port = port * 10 + (name[i]- '0');
                        }
                    }    
                    break;
                }
                temp_str.push_back(name[i]);
            }
            for(; i < name.size(); i++)
            {
                if(url.size() == 0 && name[i] == '/')
                {
                    continue;
                }
                url.push_back(name[i]);
            }
            if(port == 0)
            {
                port = 80;
            }
        }
        else
        {
            url = name;
        }
    }
    else
    {
        url = name;
    }
}

asio::awaitable<bool> socket_client::async_init_https_sock()
{
    //auto executor = co_await asio::this_coro::executor;
    ssl_context   = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    sslsock       = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(strand_, *ssl_context);
    ssl_context->set_default_verify_paths();

    asio::ip::tcp::resolver resolver(strand_);
    SSL_set_tlsext_host_name(sslsock->native_handle(), host.c_str());

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    auto endpoints                 = co_await resolver.async_resolve(host, std::to_string(port), asio::use_awaitable);
    for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
    {
        std::tie(ec) = co_await sslsock->lowest_layer().async_connect(*iter, tuple_awaitable);
        if (ec)
        {
            continue;
        }
        break;
    }
    if (ec)
    {
        error_msg = host + " ssl async_connect error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }


    sslsock->lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    ssl_context->set_verify_mode(asio::ssl::verify_peer);
    ssl_context->set_verify_callback(asio::ssl::host_name_verification(host));

    std::tie(ec) = co_await sslsock->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
    if (ec)
    {
        error_msg = host + " ssl handshake error! ";
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}

asio::awaitable<bool> socket_client::async_init_http_sock()
{
    error_msg.clear();
    //auto executor = co_await asio::this_coro::executor;
    //strand_(asio::make_strand(executor));
    //asio::ip::tcp::resolver resolver(executor);
    asio::ip::tcp::resolver resolver(strand_);

    sock = std::make_shared<asio::ip::tcp::socket>(strand_);

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    auto endpoints                 = co_await resolver.async_resolve(host, std::to_string(port), asio::use_awaitable);

    for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
    {
        std::tie(ec) = co_await sock->async_connect(*iter, tuple_awaitable);
        if (ec)
        {
            continue;
        }
        break;
    }
 

    if (ec)
    {
        error_msg = ec.message();
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}

asio::awaitable<bool> socket_client::async_connect(std::string_view name,unsigned int time_out_num)
{
    set_url(name);
    exptime = time_out_num;
    co_return co_await async_connect();
}

asio::awaitable<bool> socket_client::async_connect()
{
    bool isinit = false;

    if(url.size()< 5)
    {
        iserror = true;
        error_msg = "url empty";
        co_return false;
    }

    if(port == 0)
    {
        iserror = true;
        error_msg = "port empty";
        co_return false;
    }

    if (isssl)
    {
        isinit = co_await async_init_https_sock();
    }
    else
    {
        isinit = co_await async_init_http_sock();
    }

    if(!isinit)
    {
        iserror = true;
        error_msg = "async init socket error";
        co_return false;
    }

    if (exptime > 30)
    {
        exptime = 30;
    }
    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.socket_timeout_lists.push_back(shared_from_this());
            temp_io_context.timeout_condition.notify_one();
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror = true;
            co_return false;
        }
    }
 
    co_return true;
} 

asio::awaitable<bool> socket_client::async_tcp_connect(std::string_view name,unsigned int time_out_num)
{
    set_url(name);
    exptime = time_out_num;
    co_return co_await async_tcp_connect();
}

asio::awaitable<bool> socket_client::async_tcp_connect()
{
    bool isinit = false;

    if(url.size()< 5)
    {
        iserror = true;
        error_msg = "url empty";
        co_return false;
    }

    if(port == 0)
    {
        iserror = true;
        error_msg = "port empty";
        co_return false;
    }

    if (isssl)
    {
        isinit = co_await async_init_https_sock();
    }
    else
    {
        isinit = co_await async_init_http_sock();
    }

    if(!isinit)
    {
        iserror = true;
        error_msg = "async init socket error";
        co_return false;
    }

    if (exptime > 30)
    {
        exptime = 30;
    }
    if (exptime > 0)
    {
        set_timeout(exptime);
        client_context &temp_io_context = get_client_context_obj();
        try
        {
            temp_io_context.socket_timeout_lists.push_back(shared_from_this());
            temp_io_context.timeout_condition.notify_one();
        }
        catch (const std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg = e.what();
            iserror = true;
            co_return false;
        }
    }
    std::string _send_content="tcp "+url;
    _send_content.push_back(0x0A);
    _send_content.push_back(0x0A);
    unsigned int n = co_await async_write(_send_content);
    if(n > 0)
    {
        co_return true;
    }
    else
    {
        co_return false;
    }
}

asio::awaitable<unsigned int> socket_client::async_read(unsigned char *buffer_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        co_return 0;
    }

    if(iserror)
    {
        socket_read_lock.clear();
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await sslsock->async_read_some(asio::buffer(buffer_data, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await sock->async_read_some(asio::buffer(buffer_data, buffersize), asio::use_awaitable);
        }
        socket_read_lock.clear();
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }
    socket_read_lock.clear();
    co_return 0;
}

asio::awaitable<unsigned int> socket_client::async_read(std::string &buffer_data)
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        co_return 0;
    }

    if(iserror)
    {
        socket_read_lock.clear();
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await sslsock->async_read_some(asio::buffer(buffer_data), asio::use_awaitable);
        }
        else
        {
            n = co_await sock->async_read_some(asio::buffer(buffer_data), asio::use_awaitable);
        }
        socket_read_lock.clear();
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }
    socket_read_lock.clear();
    co_return 0;
}

asio::awaitable<unsigned int> socket_client::async_write(unsigned char *data, unsigned int buffersize)
{

    if(iserror)
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await asio::async_write(*sslsock, asio::buffer(data, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*sock, asio::buffer(data, buffersize), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }

    co_return 0;
}

asio::awaitable<unsigned int> socket_client::async_write(std::string_view value)
{

    if(iserror)
    {
        co_return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }

    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = co_await asio::async_write(*sslsock, asio::buffer(value), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*sock, asio::buffer(value), asio::use_awaitable);
        }
        co_return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }

    co_return 0;
}

//synchronous
unsigned int socket_client::write(unsigned char *data, unsigned int buffersize)
{
    if(iserror)
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = asio::write(*sslsock, asio::buffer(data, buffersize));
        }
        else
        {
            n = asio::write(*sock, asio::buffer(data, buffersize));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }

    return 0;
}

unsigned int socket_client::write(std::string_view value)
{
    if(iserror)
    {
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }

    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = asio::write(*sslsock, asio::buffer(value));
        }
        else
        {
            n = asio::write(*sock, asio::buffer(value));
        }
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }

    return 0;
}

unsigned int socket_client::read(unsigned char *buffer_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        return 0;
    }

    if(iserror)
    {
        socket_read_lock.clear();
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = sslsock->read_some(asio::buffer(buffer_data, buffersize));
        }
        else
        {
            n = sock->read_some(asio::buffer(buffer_data, buffersize));
        }
        socket_read_lock.clear();
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }
    socket_read_lock.clear();
    return 0;
}

unsigned int socket_client::read(std::string &buffer_data)
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        return 0;
    }

    if(iserror)
    {
        socket_read_lock.clear();
        return 0;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        if (isssl)
        {
            n = sslsock->read_some(asio::buffer(buffer_data));
        }
        else
        {
            n = sock->read_some(asio::buffer(buffer_data));
        }
        socket_read_lock.clear();
        return n;
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }
    socket_read_lock.clear();
    return 0;
}

void socket_client::close_connect()
{
    if (isssl)
    {
        if (sslsock)
        {
            if (sslsock->lowest_layer().is_open())
            {
                sslsock->lowest_layer().cancel(ec);
                sslsock->lowest_layer().close(ec);
            }
        }
    }
    else
    {
        if (sock && sock->is_open())
        {
            sock->close(ec);
        }
    }
}

void socket_client::run_loop()
{
    auto self = shared_from_this();
    if(data == nullptr)
    {
        data = static_cast<unsigned char*>(std::malloc(512 * sizeof(unsigned char)));
    }
    for(;;)
    {
        if(iserror)
        {
            return;
        }
        if (exptime > 0)
        {
            reset_timeout();
        }
        unsigned int n = 0;
        unsigned char data[512];
        try
        {

            if (isssl)
            {
                if (sslsock->lowest_layer().is_open())
                {
                    n = sslsock->read_some(asio::buffer(data,512));
                }
                else
                {
                    return;
                }
            }
            else
            {
                if (sock->is_open())
                {
                   n = sock->read_some(asio::buffer(data,512));
                }
                else
                {
                    return;
                }
            }

            if(run_loop_fun != nullptr)
            {
                run_loop_fun(self,n);
            }
            else if(async_run_loop_fun != nullptr)
            {
                asio::co_spawn(strand_, [self, n]() mutable
                 { return self->async_run_loop_fun(self, n); },
                 asio::detached);
            }
            else
            {
                return;
            }
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg  = e.what();
            iserror = true;
            return;
        }
    }
    return;
}

asio::awaitable<void> socket_client::async_run_loop()
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        co_return;
    }

    auto self = shared_from_this();
    if(data == nullptr)
    {
        data = static_cast<unsigned char*>(std::malloc(512 * sizeof(unsigned char)));
    }
    for(;;)
    {
        if(iserror)
        {
            socket_read_lock.clear();
            co_return;
        }
        if (exptime > 0)
        {
            reset_timeout();
        }
        unsigned int n = 0;
        try
        {
            if (isssl)
            {
                n = co_await sslsock->async_read_some(asio::buffer(data,512), asio::use_awaitable);
            }
            else
            {
                n = co_await sock->async_read_some(asio::buffer(data,512), asio::use_awaitable);
            }
            
            if(run_loop_fun != nullptr)
            {
                run_loop_fun(self,n);
            }
            else if(async_run_loop_fun != nullptr)
            {
                co_await async_run_loop_fun(self,n);
            }
            else
            {
                socket_read_lock.clear();
                co_return;
            }
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg  = e.what();
            iserror = true;
            socket_read_lock.clear();
            co_return;
        }
    }
    co_return;
}

}// namespace http
