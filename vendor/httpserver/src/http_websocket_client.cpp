#include <random>
#include <openssl/sha.h>//for SHA1
#include "http_websocket_client.h"
#include "client_context.h"
#include "base64.h"
#include "func.h"
#include "atomic_guard.h"
#include "gzip.h"

namespace http
{
 websocket_client::websocket_client():strand_(asio::make_strand(*(get_client_context_obj().ioc))){};
 websocket_client::~websocket_client()
{
    if(data != nullptr)
    {
        std::free(data);
    }
}
void websocket_client::reset()
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
    ready_state = 0;

    url.clear();
    host.clear();
    error_msg.clear();
 
    parameter.clear();
    close_connect();
    sock.reset();
    sslsock.reset();
    ssl_context.reset();
}

void websocket_client::set_deflate(bool isstatus)
{
    open_deflate = isstatus;
}
void websocket_client::set_header(std::string_view name,std::string_view value)
{
    websocket_parameter_t a;
    a.name = name;
    a.value = value;
    parameter.emplace_back(a);
}
void websocket_client::set_port(unsigned int n)
{
    port = n;
}
void websocket_client::set_host(std::string_view name)
{
    host = name;
}

void websocket_client::set_url(std::string_view name)
{
    if(name.size() > 7)
    {
        if(name[0]=='w' && name[1]=='s' && name[2]==':' && name[3]=='/'  && name[4]=='/')
        {
            std::string temp_str;
            unsigned int i=5;
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
                url.push_back(name[i]);
            }
            if(port == 0)
            {
                port = 80;
            }
        }
        else if(name[0]=='w' && name[1]=='s' && name[2]=='s' && name[3]==':' && name[4]=='/'  && name[5]=='/')
        {
            std::string temp_str;
            unsigned int i=6;
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
                url.push_back(name[i]);
            }
            if(port == 0)
            {
                port = 80;
            }
        }
        else
        {
            error_msg = "url formatting error";
            iserror = true;
        }
    }
    else
    {
        error_msg = "url too short";
        iserror = true;
    }
}

asio::awaitable<bool> websocket_client::async_init_https_sock()
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

asio::awaitable<bool> websocket_client::async_init_http_sock()
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
asio::awaitable<bool> websocket_client::async_connect(std::string_view url_,unsigned int time_out_num)
{
    set_url(url_);
    exptime = time_out_num;
    co_return co_await async_connect();
}

asio::awaitable<bool> websocket_client::async_connect()
{
    bool isinit = false;

    if(url.size()< 5)
    {
        iserror = true;
        error_msg = "url empty";
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
            temp_io_context.websocket_timeout_lists.push_back(shared_from_this());
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
    isinit = co_await websocket_handshake();
    co_return isinit;
}

bool websocket_client::add_client_task_loop()
{
    client_context &temp_io_context = get_client_context_obj();
    try
    {
        temp_io_context.websocket_timeout_lists.push_back(shared_from_this());
        temp_io_context.timeout_condition.notify_one();
        return true;
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg = e.what();
        iserror = true;
        return false;
    }
}

asio::awaitable<unsigned int> websocket_client::async_read(unsigned char *read_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        co_return 0;
    }
    atomic_guard guard{socket_read_lock};

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
            n = co_await sslsock->async_read_some(asio::buffer(read_data, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await sock->async_read_some(asio::buffer(read_data, buffersize), asio::use_awaitable);
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

asio::awaitable<unsigned int> websocket_client::async_read(std::string &read_data)
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        co_return 0;
    }
    atomic_guard guard{socket_read_lock};
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
        if(read_data.size()==0)
        {
            read_data.resize(1024);
        }

        if (isssl)
        {
            n = co_await sslsock->async_read_some(asio::buffer(read_data), asio::use_awaitable);
        }
        else
        {
            n = co_await sock->async_read_some(asio::buffer(read_data), asio::use_awaitable);
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

asio::awaitable<unsigned int> websocket_client::async_write(unsigned char *data_out, unsigned int buffersize)
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
            n = co_await asio::async_write(*sslsock, asio::buffer(data_out, buffersize), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*sock, asio::buffer(data_out, buffersize), asio::use_awaitable);
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

asio::awaitable<unsigned int> websocket_client::async_write(std::string_view value)
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
unsigned int websocket_client::write(unsigned char *data_out, unsigned int buffersize)
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
            n = asio::write(*sslsock, asio::buffer(data_out, buffersize));
        }
        else
        {
            n = asio::write(*sock, asio::buffer(data_out, buffersize));
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

unsigned int websocket_client::write(std::string_view value)
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

unsigned int websocket_client::read(unsigned char *buffer_data, unsigned int buffersize)
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        return 0;
    }
    atomic_guard guard{socket_read_lock};
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
            n = sslsock->read_some(asio::buffer(buffer_data, buffersize));
        }
        else
        {
            n = sock->read_some(asio::buffer(buffer_data, buffersize));
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

unsigned int websocket_client::read(std::string &buffer_data)
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        return 0;
    }
    atomic_guard guard{socket_read_lock};
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
            n = sslsock->read_some(asio::buffer(buffer_data));
        }
        else
        {
            n = sock->read_some(asio::buffer(buffer_data));
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

void websocket_client::close_connect()
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

void websocket_client::run_loop()
{
    auto self = shared_from_this();
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        return;
    }
    atomic_guard guard{socket_read_lock};

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

            process_data(data, n);

            if(recv_data.isfinish)
            {
                if(run_loop_fun != nullptr)
                {
                    run_loop_fun(self);
                }
                else if(async_run_loop_fun != nullptr)
                {
                    asio::co_spawn(strand_, [self, pack_data =recv_data]() mutable
                    { return self->async_run_loop_fun(self, pack_data); },
                    asio::detached);
                    
                }
                reset_recv_status();
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

asio::awaitable<void> websocket_client::async_run_loop()
{
    auto self = shared_from_this();
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        co_return;
    }
    atomic_guard guard{socket_read_lock};

    if(data == nullptr)
    {
        data = static_cast<unsigned char*>(std::malloc(512 * sizeof(unsigned char)));
    }
    for(;;)
    {
        if(iserror)
        {
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

            process_data(data, n);

            if(recv_data.isfinish)
            {
                if(run_loop_fun != nullptr)
                {
                    run_loop_fun(self);
                }
                else if(async_run_loop_fun != nullptr)
                {
                    asio::co_spawn(strand_, [self, pack_data =recv_data]() mutable
                    { return self->async_run_loop_fun(self, pack_data); },
                    asio::detached);
                    
                }
                reset_recv_status();
            }
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg  = e.what();
            iserror = true;
            co_return;
        }
    }
    co_return;
}

 
std::string websocket_client::make_http_header()
{
    if(url.size() < 1 )
    {
        error_msg = "url to short";
        iserror = true;
        return "";
    }
    if(host.size() < 1 )
    {
        error_msg = "connect host to short";
        iserror = true;
        return "";
    }

    std::string send_header_content;

    send_header_content="GET ";
    send_header_content.append(url);
    send_header_content.append(" HTTP/1.1\r\nHost: ");
    send_header_content.append(host);
    send_header_content.append("\r\n");

    if(parameter.size()>0)
    {
        for(unsigned int i=0;i < parameter.size(); i++)
        {
            send_header_content.append(parameter[i].name);
            send_header_content.append(": ");
            send_header_content.append(parameter[i].value);
            send_header_content.append("\r\n");
        }
    }
    else
    {
        send_header_content.append("User-Agent: paozhu\r\nAccept: */*\r\nAccept-Language: en-US\r\n");
        if(open_deflate)
        {
            send_header_content.append("Accept-Encoding: gzip, deflate\r\n");
        }
    }

    send_header_content.append("Sec-WebSocket-Version: 13\r\n");
    if(open_deflate)
    {
        send_header_content.append("Sec-WebSocket-Extensions: permessage-deflate\r\n");
    }

    send_header_content.append("Sec-WebSocket-Key: ");


    std::random_device rd;
    std::mt19937 gen(rd());
    //定义均匀整数分布，范围是 [0, 255]（闭区间，包含两端）
    std::uniform_int_distribution<int> dis(0, 255);

    for(unsigned int i=0; i<16; i++)
    {
        key_str[i] = dis(gen);
    }
    //base64
    send_header_content.append(base64_encode((const char *)key_str,16,false));
    send_header_content.append("\r\n");

    send_header_content.append("Connection: Upgrade\r\n");
    send_header_content.append("Pragma: no-cache\r\n");
    send_header_content.append("Cache-Control: no-cache\r\n");
    send_header_content.append("Upgrade: websocket\r\n");
    send_header_content.append("\r\n");
    return send_header_content;
}

bool websocket_client::process_handshake(unsigned char *read_data, unsigned int readnum)
{
    offsetnum = 0;
    if(offsetnum >= readnum)
    {
        iserror = true;
        return false;
    }
    for(;offsetnum < readnum; offsetnum++)
    {
        if(read_data[offsetnum] == 0x20)
        {
            break;
        }
    }
    
    if(read_data[offsetnum] == 0x20)
    {
        offsetnum++;
    } 
    else 
    {
        iserror = true;
        return false;
    }
 
    if(read_data[offsetnum] =='1' && read_data[offsetnum+1] =='0' && read_data[offsetnum+2] =='1')
    {
        offsetnum = offsetnum + 3;
        for(;offsetnum < readnum; offsetnum++)
        {
            if(read_data[offsetnum] == '\r')
            {
                offsetnum++;
                if(offsetnum < readnum && read_data[offsetnum] == '\n')
                {
                    break;
                }
            }
        }
        if(read_data[offsetnum] == '\n')
        {
            offsetnum++;
        } 
        else 
        {
            iserror = true;
            return false;
        }

        unsigned int begin_data_pos = offsetnum;
        for(;offsetnum < readnum; offsetnum++)
        {
            if(read_data[offsetnum] == '\r')
            {
                offsetnum++;
                if(offsetnum < readnum && read_data[offsetnum] == '\n')
                {
                    process_header(read_data,begin_data_pos,offsetnum);
                    begin_data_pos = offsetnum + 1;
                    continue;
                }
            }
        }
    }
    else 
    {
        iserror = true;
        return false;
    }

    //handshake
    bool isok = false;
    if(parameter.size() > 0)
    {
        for(unsigned int i=0; i < parameter.size(); i++)
        {
            if(str_casecmp(parameter[i].name,"Sec-WebSocket-Accept"))
            {
                // 使用请求传过来的KEY+协议字符串，先用SHA1加密然后使用base64编码算出一个应答的KEY
                std::string magicKey = base64_encode((const char *)key_str,16,false);
                magicKey.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

                unsigned char digest[SHA_DIGEST_LENGTH];
                SHA1((unsigned char *)magicKey.c_str(), magicKey.length(), (unsigned char *)&digest);

                magicKey.clear();
                magicKey = base64_encode((char *)digest, SHA_DIGEST_LENGTH, false);

                if(str_casecmp(parameter[i].value, magicKey))
                {
                    isok = true;
                }
                else
                {
                    error_msg = "response Handshake error";
                }
            }
            else if(str_casecmp(parameter[i].name,"Sec-WebSocket-Extensions"))
            {
                if (parameter[i].value.find("permessage-deflate") != std::string::npos)
                {
                    if(open_deflate)
                    {
                        isdeflate = true;
                    }
                }
            }
        }
    }
    else
    {
        error_msg = "response header empty";
    }
    return isok;
}

void websocket_client::process_header(unsigned char *read_data, unsigned int data_bein, unsigned int data_end)
{
    struct websocket_parameter_t temp_kv;
    for(;data_bein < data_end; data_bein++)
    {
        if(read_data[data_bein] == ':')
        {
            break;
        }
        else if(read_data[data_bein] == '\r')
        {
            break;
        }
        temp_kv.name.push_back(read_data[data_bein]);
    }

    if(read_data[data_bein] == ':')
    {
        data_bein++;
        for(;data_bein < data_end; data_bein++)
        {
            if(read_data[data_bein] != 0x20)
            {
                break;
            }
        }
        for(;data_bein < data_end; data_bein++)
        {
            if(read_data[data_bein] == '\r')
            {
                break;
            }
            temp_kv.value.push_back(read_data[data_bein]);
        }

        parameter.push_back(temp_kv);
    }
    else
    {
        return;
    }
}

 
asio::awaitable<bool> websocket_client::websocket_handshake()
{
    auto self = shared_from_this();
    if(data == nullptr)
    {
        data = static_cast<unsigned char*>(std::malloc(512 * sizeof(unsigned char)));
    }
  
    if(iserror)
    {
        error_msg = " is has error";
        co_return false;
    }
    if (exptime > 0)
    {
        reset_timeout();
    }
    unsigned int n = 0;
    try
    {
        std::string send_hand_header = make_http_header();
        if(iserror)
        {
            error_msg = " make_http_header error";
            co_return false;
        }

        if (isssl)
        {
            if (sslsock->lowest_layer().is_open())
            {
                n = co_await asio::async_write(*sslsock, asio::buffer(send_hand_header), asio::use_awaitable);
            }
            else
            {
                error_msg = " sslsock async_write error";
                co_return false;
            }
        }
        else
        {
            if (sock->is_open())
            {
                n = co_await asio::async_write(*sock, asio::buffer(send_hand_header), asio::use_awaitable);
            }
            else
            {
                error_msg = " sock async_write error";
                co_return false;
            }
        }

        if (isssl)
        {
            if (sslsock->lowest_layer().is_open())
            {
                n = sslsock->read_some(asio::buffer(data,512));
            }
            else
            {
                error_msg = " sslsock read_some error";
                co_return false;
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
                error_msg = " sock read_some error";
                co_return false;
            }
        }

        if(process_handshake(data, n))
        {
            co_return true;
        }
        else
        {
            error_msg.append(" process_handshake error");
            co_return false;
        }

    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
        co_return false;
    }
 
    co_return false;
}

std::string websocket_client::make_pong()
{
    std::string reping;
    make_mark();
    reping.resize(10);
    reping[0] = 0x8A;
    reping[1] = 0x84;
    reping[2] = mask_key[0];
    reping[3] = mask_key[1];
    reping[4] = mask_key[2];
    reping[5] = mask_key[3];

    reping[6] = 'p' ^ mask_key[0];
    reping[7] = 'o' ^ mask_key[1];
    reping[8] = 'n' ^ mask_key[2];
    reping[9] = 'g' ^ mask_key[3];
    return reping;
}
std::string websocket_client::make_ping()
{
    std::string reping;
    make_mark();
    reping.resize(10);
    reping[0] = 0x89;
    reping[1] = 0x84;
    reping[2] = mask_key[0];
    reping[3] = mask_key[1];
    reping[4] = mask_key[2];
    reping[5] = mask_key[3];

    reping[6] = 'p' ^ mask_key[0];
    reping[7] = 'i' ^ mask_key[1];
    reping[8] = 'n' ^ mask_key[2];
    reping[9] = 'g' ^ mask_key[3];
 
    return reping;
}

int websocket_client::make_ws_data(char *msg, unsigned int msgLen, std::string &outBuf)
{
    make_ws_header(msgLen, outBuf, 0x02);
    for (unsigned int i = 0; i < msgLen; i++)
    {
        unsigned char ta = msg[i] ^ mask_key[i%4];
        outBuf.push_back(ta);
    }
    return outBuf.size();
}
int websocket_client::make_ws_text(char *msg, unsigned int msgLen, std::string &outBuf)
{
    make_ws_header(msgLen, outBuf, 0x01);
    for (unsigned int i = 0; i < msgLen; i++)
    {
        unsigned char ta = msg[i] ^ mask_key[i%4];
        outBuf.push_back(ta);
    }
    return outBuf.size();
}

int websocket_client::make_ws_data(std::string_view msg, std::string &outBuf)
{
    make_ws_header(msg.length(), outBuf, 0x02);
    for (unsigned int i = 0; i < msg.length(); i++)
    {
        unsigned char ta = msg[i] ^ mask_key[i%4];
        outBuf.push_back(ta);
    }
    return outBuf.size();
}
int websocket_client::make_ws_text(std::string_view msg, std::string &outBuf)
{
    make_ws_header(msg.length(), outBuf, 0x01);
    for (unsigned int i = 0; i < msg.length(); i++)
    {
        unsigned char ta = msg[i] ^ mask_key[i%4];
        outBuf.push_back(ta);
    }
    return outBuf.size();
}
int websocket_client::make_ws_header(unsigned int len, std::string &header, unsigned char t)
{
    make_mark();
    switch (t)
    {
    case 0x01:
        header.push_back(0x81);
        break;
    case 0x02:
        header.push_back(0x82);
        break;
    case 0x80:
        header.push_back(0x80);
        break;
    case 0x81:
        header.push_back(0x81);
        break;
    case 0x82:
        header.push_back(0x82);
        break;
    default:
        header.push_back(t);
    }

    if (len <= 125)
    {
        header.push_back(0x80 | len);
    }
    else if (len <= 65535)
    {
        header.push_back(0xFE);                // 16 bit length follows
        header.push_back(((len >> 8) & 0xFF));// leftmost first
        header.push_back((len & 0xFF));
    }
    else// >2^16-1 (   )
    {
        header.push_back(0xFF);// 64 bit length follows
        unsigned long long real_length = len;
 
        for (int i = 8; i > 0; i--)
        {
            header.push_back(((real_length >> (8 * (i-1))) & 0xFF));
        }
    }

    for(unsigned int i=0; i<4; i++)
    {
        header.push_back(mask_key[i]);
    }
    return header.size();
}

void websocket_client::make_mark()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    //定义均匀整数分布，范围是 [0, 255]（闭区间，包含两端）
    std::uniform_int_distribution<int> dis(0, 255);

    for(unsigned int i=0; i<4; i++)
    {
        mask_key[i] = dis(gen);
    }
}

unsigned long long websocket_client::first_parsedata(unsigned char *inputdata, unsigned int buffersize)
{
    if(buffersize < 2)
    {
        iserror = true;
        error_msg = "Pack is bad";
        return 0;
    }
    recv_data.fin           = inputdata[0] >> 7 & 0x01;
    recv_data.opcode        = inputdata[0] & 0x0F;
    recv_data.mask          = inputdata[1] >> 7 & 0x01;

    unsigned char fixlength = inputdata[0] >> 6 & 0x01;

    if(fixlength > 0)
    {
        recv_data.isdeflate = true;
    }
    fixlength = inputdata[1] & 0x7F;

    unsigned int pos                  = 2;
    recv_data.isfinish                = false;
    recv_data.length = 0;
    if (recv_data.fin > 0)
    {
        recv_data.isclose_stream = true;
    }

    switch (recv_data.opcode)
    {
    case 0x00:
        break;
    case 0x01:
    case 0x02:
        break;
    case 0x08: 
    case 0x09:
    case 0x0A:
        recv_data.isfinish = true;
        break;             
    default:
        iserror = true;
        return 0;
        break;
    }

    // unsigned char mask_key[4];
    if (fixlength < 126)
    {
        recv_data.length = fixlength;
    }
    else if (fixlength == 126)
    {
        recv_data.length = inputdata[pos] << 8 | (unsigned char)inputdata[pos + 1];
        pos += 2;
    }
    else if (fixlength == 127)
    {
        recv_data.length = (unsigned char)inputdata[pos];
        recv_data.length = recv_data.length << 8 | (unsigned char)inputdata[pos + 1];
        recv_data.length = recv_data.length << 8 | (unsigned char)inputdata[pos + 2];
        recv_data.length = recv_data.length << 8 | (unsigned char)inputdata[pos + 3];
        recv_data.length = recv_data.length << 8 | (unsigned char)inputdata[pos + 4];
        recv_data.length = recv_data.length << 8 | (unsigned char)inputdata[pos + 5];
        recv_data.length = recv_data.length << 8 | (unsigned char)inputdata[pos + 6];
        recv_data.length = recv_data.length << 8 | (unsigned char)inputdata[pos + 7];
        pos += 8;
    }
    if (recv_data.mask == 1)
    {
        recv_data.mask_key[0] = inputdata[pos];
        recv_data.mask_key[1] = inputdata[pos + 1];
        recv_data.mask_key[2] = inputdata[pos + 2];
        recv_data.mask_key[3] = inputdata[pos + 3];
        pos += 4;
    }
    
    if (recv_data.mask == 1)
    {
        for (; pos < buffersize; pos++)
        {
            unsigned char nn = pos % 4;
            char b           = inputdata[pos] ^ mask_key[nn];
            recv_data.content.push_back(b);
            recv_data.read_length++;
        }
    }
    else
    {
        for (; pos < buffersize; pos++)
        {
            recv_data.content.push_back(inputdata[pos]);
            recv_data.read_length++;
        }
    }
    recv_data.total_length += recv_data.read_length;
    if (recv_data.read_length >= recv_data.length)
    {
        if(recv_data.isclose_stream)
        {
            recv_data.isfinish      = true;
        }
    }

    return recv_data.read_length;
}
unsigned long long websocket_client::append_parsedata(unsigned char *inputdata, unsigned int buffersize)
{
    if (recv_data.mask == 1)
    {
        for (unsigned int j = 0; j < buffersize; j++)
        {
            unsigned char nn = j % 4;
            char b           = inputdata[j] ^ mask_key[nn];
            recv_data.content.push_back(b);
        }
    }
    else
    {
        for (unsigned int j = 0; j < buffersize; j++)
        {
            recv_data.content.push_back(inputdata[j]);
        }
    }
    recv_data.read_length = recv_data.read_length + buffersize;
    recv_data.total_length += recv_data.read_length;
    if (recv_data.read_length >= recv_data.length)
    {
        if(recv_data.isclose_stream)
        {
            recv_data.isfinish      = true;
        }
    }
    return recv_data.read_length;
}

unsigned int websocket_client::process_data(unsigned char *inputdata, unsigned int buffersize)
{
    if(recv_data.opcode ==0)
    {
        return first_parsedata(inputdata, buffersize);
    }
    else
    {
       return append_parsedata(inputdata, buffersize);
    }
    if(recv_data.total_length > 16777216)
    {
        iserror = true;
        error_msg = "Recv data too long";
    }
}

asio::awaitable<void> websocket_client::async_recv_finish()
{
    if(async_recv_finish_fun != nullptr)
    {
       co_await async_recv_finish_fun(shared_from_this());
    }
    co_return;
}

void websocket_client::reset_recv_status()
{
    recv_data.isfile = false;
    recv_data.isfinish = false;
    recv_data.isdeflate = false;
    recv_data.isclose_stream = false;
    recv_data.fin = 0;
    recv_data.opcode = 0;      
    recv_data.mask = 0; 
    recv_data.length = 0;
    recv_data.read_length = 0;
    recv_data.content.clear();
}
asio::awaitable<unsigned int> websocket_client::async_text_write(std::string_view value)
{
    std::string outdata;
    make_ws_text(value,outdata);
    unsigned int n = co_await async_write(outdata);   
    co_return n;
}
asio::awaitable<unsigned int> websocket_client::async_data_write(std::string_view value)
{
    std::string outdata;
    make_ws_data(value,outdata);
    unsigned int n = co_await async_write(outdata);   
    co_return n;
}

asio::awaitable<unsigned int> websocket_client::async_text_read()
{
    if (socket_read_lock.test_and_set()) 
    {
        error_msg = "Other socket read is set";
        iserror = true;
        co_return 0;
    }
    atomic_guard guard{socket_read_lock};
    reset_recv_status();
    auto self = shared_from_this();
    if(data == nullptr)
    {
        data = static_cast<unsigned char*>(std::malloc(512 * sizeof(unsigned char)));
    }
    for(;;)
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
                n = co_await sslsock->async_read_some(asio::buffer(data,512), asio::use_awaitable);
            }
            else
            {
                n = co_await sock->async_read_some(asio::buffer(data,512), asio::use_awaitable);
            }
            process_data(data, n);

            if(recv_data.isfinish)
            {
                co_return recv_data.opcode;
            } 
        }
        catch (std::exception &e)
        {
            DEBUG_LOG("Exception: %s", e.what());
            error_msg  = e.what();
            iserror = true;
            co_return 0;
        }
    }
    co_return 0;
}

bool websocket_client::un_pack(const std::string &content, std::string &outBuf)
{
    return http::uncompress(content,outBuf);
}

bool websocket_client::un_pack(std::string &outBuf)
{
    return http::uncompress(recv_data.content,outBuf);
}
int websocket_client::compress(const std::string &pack_data,std::string &outBuf)
{
    if (http::compress(pack_data.data(), pack_data.size(), outBuf, Z_DEFAULT_COMPRESSION) ==
                Z_OK)
    {
        return 0;
    }
    return 1;
}

}// namespace http
