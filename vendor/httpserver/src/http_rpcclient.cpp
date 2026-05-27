/**
 *  @copyright copyright 2026, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file http rpc client.cpp
 *  @date 2026-05-10
 *
 *  http rpc client file
 *
 *
 */
#include <memory> 
#include "server_localvar.h" 
#include "http_rpcclient.h"
#include "client_context.h"

namespace http
{
rpc_client::rpc_client(): uprawfile(nullptr, std::fclose), strand_(asio::make_strand(*(get_client_context_obj().ioc))) { iserror = false; }
rpc_client::~rpc_client()
{
    if(data != nullptr)
    {
        std::free(data);
    }
}
void rpc_client::set_header(std::string_view name,std::string_view value)
{
    rpc_parameter_t a;
    a.name = name;
    a.value = value;
    parameter.emplace_back(a);
}

void rpc_client::set_host(std::string_view name)
{
    host = name;
}

void rpc_client::close_connect()
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

void rpc_client::set_url(std::string_view name)
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
                url.push_back(name[i]);
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
                url.push_back(name[i]);
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
asio::awaitable<void> rpc_client::async_send()
{
    std::string send_content;
    send_content.append("rpc");
    send_content.push_back(0x20);
    send_content.append(url);
    send_content.push_back(0x0A);
    send_content.append("HOST ");
    
    unsigned short value_size = host.size();
    unsigned char a = value_size & 0xFF;
    
    value_size = value_size & 0xFF00;
    unsigned char b = value_size >> 8;
    send_content.push_back(b);
    send_content.push_back(a);

    send_content.append(host);
    send_content.push_back(0x0A);

    send_content.append("size ");

    if(content.size() <=  0xFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = content.size();
        unsigned char a = value_size & 0xFF;
        value_size = value_size & 0xFF00;
        unsigned char b = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
    }
    else if(content.size() <=  0xFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x04);

        unsigned int value_size = content.size();
        unsigned char a;
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else if(content.size() <=  0xFFFFFFFFFF)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x05);

        unsigned long long value_size = content.size();
        unsigned char a;
        a = (value_size & 0xFF00000000) >> 32;
        send_content.push_back(a);
        a = (value_size & 0xFF000000) >> 24;
        send_content.push_back(a);
        a = (value_size & 0xFF0000) >> 16;
        send_content.push_back(a);
        a = (value_size & 0xFF00) >> 8;
        send_content.push_back(a);
        a = value_size & 0xFF;
        send_content.push_back(a);
    }
    else
    {
        iserror = true;
        co_return;
    }
    send_content.push_back(0x0A);

    send_content.append("type ");

    if(content_type.size() ==0 )
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if(content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");               
            iserror = true;
            co_return;
        }

        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = content_type.size();
        unsigned char a = value_size & 0xFF;
        value_size = value_size & 0xFF00;
        unsigned char b = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
    }
    send_content.push_back(0x0A);

    for(unsigned int i=0; i < parameter.size(); i++)
    {
        send_content.append(parameter[i].name);
        send_content.push_back(0x20);
        unsigned short value_size = parameter[i].value.size();
        unsigned char a = value_size & 0xFF;
        value_size = value_size & 0xFF00;
        unsigned char b = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(parameter[i].value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
    send_content.append(content);

    if(isssl)
    {
        if(port == 0)
        {
            port = 443;
        }
        co_await async_ssl_send_data(send_content);
    }
    else
    {
        if(port == 0)
        {
            port = 80;
        }
        co_await async_send_data(send_content);
    }
    co_return;
}

void rpc_client::send()
{

} 

asio::awaitable<bool> rpc_client::async_init_https_sock()
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

asio::awaitable<bool> rpc_client::async_init_http_sock()
{
    error_msg.clear();
    //auto executor = co_await asio::this_coro::executor;
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

asio::awaitable<void> rpc_client::async_ssl_send_data(std::string_view send_content)
{
    try
    {
        if (!sslsock)
        {
            bool isinit = co_await async_init_https_sock();
            if (!isinit)
            {
                co_return;
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
                    temp_io_context.rpc_timeout_lists.push_back(shared_from_this());
                    temp_io_context.timeout_condition.notify_one();
                }
                catch (const std::exception &e)
                {
                    DEBUG_LOG("Exception: %s", e.what());
                    error_msg = e.what();
                    iserror = true;
                }
            }
            if(iserror)
            {
                co_return;
            }

            unsigned int n=0;
            n = co_await async_write(*sslsock, asio::buffer(send_content), asio::use_awaitable);
            unsigned char data[2052];
            
            bool begin_data=true;
            isbody = false;
            while (true)
            {
                memset(data, 0x00, 2048);
                if (exptime > 0)
                {
                    reset_timeout();
                }
                if (isbody && page.read_size < 2048)
                {
                    n = co_await sslsock->async_read_some(asio::buffer(data, page.read_size), asio::use_awaitable);
                }
                else
                {
                    n = co_await sslsock->async_read_some(asio::buffer(data, 2048), asio::use_awaitable);
                }
                offsetnum = 0;
                if (n == 0)
                {
                    break;
                }
                if(begin_data)
                {
                    begin_data = false;
                    process(data,n);
                }
                else
                {
                    process_append(data,n);
                }

                if (isfinish)
                {
                    break;
                }
                if(iserror)
                {
                    break;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }

    co_return;
}


asio::awaitable<void> rpc_client::async_send_data(std::string_view send_content)
{
    try
    {
        if (!sock)
        {
            bool isinit = co_await async_init_http_sock();
            if (!isinit)
            {
                co_return;
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
                    temp_io_context.rpc_timeout_lists.push_back(shared_from_this());
                    temp_io_context.timeout_condition.notify_one();
                }
                catch (const std::exception &e)
                {
                    DEBUG_LOG("Exception: %s", e.what());
                    error_msg = e.what();
                    iserror = true;
                }
            }
            if(iserror)
            {
                co_return;
            }

            co_await asio::async_write(*sock, asio::buffer(send_content), asio::use_awaitable);
            unsigned char data[2052];
            unsigned int n=0;
            bool begin_data=true;
            isbody = false;
            while (true)
            {
                memset(data, 0x00, 2048);
                if (exptime > 0)
                {
                    reset_timeout();
                }
                if (isbody && page.read_size < 2048)
                {
                    n = co_await sock->async_read_some(asio::buffer(data, page.read_size), asio::use_awaitable);
                }
                else
                {
                    n = co_await sock->async_read_some(asio::buffer(data, 2048), asio::use_awaitable);
                }
                offsetnum = 0;
                if (n == 0)
                {
                    break;
                }
                if(begin_data)
                {
                    begin_data = false;
                    process(data,n);
                }
                else
                {
                    process_append(data,n);
                }

                if (isfinish)
                {
                    break;
                }
                if(iserror)
                {
                    break;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        DEBUG_LOG("Exception: %s", e.what());
        error_msg  = e.what();
        iserror = true;
    }

    co_return;
}


void rpc_client::process_headkv()
{
    if(read_key.size()==6 && read_key == "cookie")
    {
        read_key.clear();
        unsigned int i = 0;
        for(; i < read_value.size(); i++)
        {
            if(read_value[i]=='=')
            {
                break;
            }
            read_key.push_back(read_value[i]);  
        }

        if(read_key.empty())
        {
            //ignore
            return;
        }

        if(read_value[i]=='=')
        {
            i++;
            rpc_parameter_t a;
            if(i < read_value.size())
            {
                a.name = read_key;
                a.value = read_value.substr(i);
            }
            else
            {
                //empty
                a.name = read_key;
                a.value = "";
            }
            page.cookie.emplace_back(a);
        }
    }
    else if(read_key.size()==4 && read_key == "type")
    {
        page.content_type = read_value;
    }
    else if(read_key.size()==5 && read_key == "chunk")
    {
        if(read_value =="none")
        {
            page.ischunked = false;
        }
        else
        {
            if(read_value.size()>0)
            {
                page.ischunked = true;
            }
            else
            {
                page.ischunked = false;
            }
        }
    }
    else if(read_key.size()==6 && read_key == "encode")
    {
        page.encode_type = read_value;
    }
    else if(read_key.size()==4 && read_key == "size")
    {
        unsigned int i = 0;
        page.page_size = 0;
        for(; i < read_value.size(); i++)
        {
            page.page_size = (page.page_size <<8) + read_value[i];
        }
        content_size = page.page_size;
        page.read_size = page.page_size;
        if(content_size > 68719476735)
        {
            //to long long(>15G)
            iserror = true;
            return;
        }
    }
    else
    {
        rpc_parameter_t a;
        a.name = read_key;
        a.value = read_value;
        page.header.emplace_back(a);
    }
}
void rpc_client::process_value(const unsigned char *buffer, unsigned int readnum)
{
    for(; offsetnum < readnum; offsetnum++)
    {
        read_value.push_back(buffer[offsetnum]);
        val_size--; 
        if(val_size == 0)
        {
            cur_process_type = 11;
            offsetnum++;
            return;
        }
    }
}   

void rpc_client::process_append(const unsigned char *buffer, unsigned int readnum)
{
    if(iserror)
    {
        return;
    }

    if(isbody)
    {
        if(page.ischunked)
        {
            process_body_chunk(buffer,readnum); 
        }
        else
        {
            process_body(buffer,readnum);  
        }
    }
    else
    {
        process_parameter(buffer,readnum);
    }    
}

void rpc_client::process(const unsigned char *buffer, unsigned int readnum)
{
    offsetnum = 0;
    if(readnum < 6)
    {
        error_msg = "too short response";
        iserror = true;
        return;
    }

    if(buffer[0] == 'r' && buffer[1] == 'p' && buffer[2] == 'c'  && buffer[3] == 0x20 )
    {
         if( buffer[4] >='0' &&  buffer[4] <='9' && buffer[5] >='0' &&  buffer[5] <='9' && buffer[6] >='0' &&  buffer[6] <='9') 
         {
            page.status_code = buffer[4] - '0';
            page.status_code = page.status_code * 10;
            page.status_code = page.status_code + (buffer[5] - '0');
            page.status_code = page.status_code * 10;
            page.status_code = page.status_code + (buffer[6] - '0');
         }
         else
         {
            error_msg = "header info error";
            iserror = true;
            return;
         }  
         offsetnum = 7;
         if(offsetnum < readnum && buffer[offsetnum] == 0x20 )
         {
            for(; offsetnum < readnum; offsetnum++)
            {
                if(buffer[offsetnum] == 0x0A)
                {
                    break;
                }
                page.status_msg.push_back(buffer[offsetnum]); 
            }    

            if(buffer[offsetnum] != 0x0A)
            {
                error_msg = "header info error";
                iserror = true;
                return;
            }
            offsetnum++;
            //process parameter
            process_parameter(buffer,readnum);
            if(isbody)
            {
                if(page.ischunked)
                {
                    process_body_chunk(buffer,readnum); 
                }
                else
                {
                    process_body(buffer,readnum);  
                }
            }
         } 
         else
         {
            error_msg = "header info error";
            iserror = true;
            return;
         }  
    }
    else
    {
        error_msg = "header info error";
        iserror = true;
        return;
    }    
}

void rpc_client::process_parameter(const unsigned char *buffer, unsigned int readnum)
{
    if(cur_process_type == 0)
    {
        for(unsigned int i = 0; offsetnum < readnum; offsetnum++)
        {
            if(buffer[offsetnum]==0x20)
            {
                break;
            }
            read_key.push_back(buffer[offsetnum]);
            i++;
            if(i > 128)
            {
                iserror = true;
                return;
            }
        }
        
        if(read_key.size() > 128)
        {
            iserror = true;
            return;
        }

        if(buffer[offsetnum]==0x20)
        {
           offsetnum++;  
        }
        else
        {
            // not finished
            cur_process_type = 0;
            return;
        }

        
        if(offsetnum < readnum)
        {
            val_size = buffer[offsetnum];
            offsetnum++;
            if(offsetnum < readnum)
            {
                val_size =(val_size << 8) + buffer[offsetnum];
                cur_process_type = 10;
                offsetnum++;
                process_value(buffer,readnum);
            }
            else
            {
                // not finished
                cur_process_type = 1;
                return;
            }
        }
        else
        {
            // not finished
            cur_process_type = 2;
            return;
        }
        
    }
    else if(cur_process_type == 1)
    {
        if(offsetnum < readnum)
        {
            val_size = (val_size << 8) + buffer[offsetnum];
            cur_process_type = 10;
            offsetnum++;
            process_value(buffer,readnum);
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }
    else if(cur_process_type == 2)
    {
        if(offsetnum < readnum)
        {
            val_size = buffer[offsetnum];
            offsetnum++;
            if(offsetnum < readnum)
            {
                val_size = (val_size << 8) + buffer[offsetnum];
                cur_process_type = 10;
                offsetnum++;
                process_value(buffer,readnum);
            }
            else
            {
                // not finished
                iserror = true;
                return;
            }
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }
    else if(cur_process_type == 10)
    {
        cur_process_type = 10;
        process_value(buffer,readnum);
    }
    else if(cur_process_type == 12)
    {
        //值还没有读到空格 The value has not yet reached the space
        if(buffer[offsetnum]==0x0A)
        {
            offsetnum++;  
        }
        else
        {
            iserror = true;
            return;
        }
        cur_process_type = 0;
        //重新开始kv read kv 
    }
   
    if(cur_process_type == 11)
    {
        if(offsetnum < readnum)
        {
            if(buffer[offsetnum]==0x0A)
            {
                offsetnum++;  
            }
            process_headkv();
            cur_process_type = 0;
            read_key.clear();
            read_value.clear();
        }
        else if(offsetnum == readnum)
        {
            process_headkv();
            cur_process_type = 12;
            read_key.clear();
            read_value.clear();
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }

    if(offsetnum < readnum)
    {
        //test the end
        if(buffer[offsetnum]==0x0A)
        {
            offsetnum++;
            isbody = true;
            return; 
        }
    }
    else
    {
        //the end
        return; 
    }

    // next
    process_parameter(buffer,readnum);

}
void rpc_client::process_body_chunk(const unsigned char *buffer, unsigned int readnum)
{
    if(isfinish)
    {
        return;
    }
    if(iserror)
    {
        return;
    }
    if(offsetnum < readnum)
    {
        if(page.read_size == 0)
        {
            page.read_size =  buffer[offsetnum];
            page.read_size = page.read_size << 8;
            offsetnum++;
            if(offsetnum < readnum)
            {
                page.read_size =page.read_size  |  buffer[offsetnum];
                offsetnum++;
            }
            if(page.read_size == 0)
            {
                isfinish = true;
                page.content_length = page.content.size();
                return;
            }
        }

        if(offsetnum < readnum)
        {
            unsigned int temp_size = readnum - offsetnum;
            if(temp_size > page.read_size)
            {
                page.content.append((char *)&buffer[offsetnum], page.read_size);
                page.read_size = 0;
                offsetnum = offsetnum + page.read_size;
                if(chunk_process  != nullptr)
                {
                    chunk_process(shared_from_this());
                }
                process_body_chunk(buffer,readnum);
                return;
            }
            else
            {
                page.content.append((char *)&buffer[offsetnum], temp_size);
                if(chunk_process  != nullptr)
                {
                    chunk_process(shared_from_this());
                }
            }
        }
        
        page.read_size = page.read_size - (readnum - offsetnum);
        if(page.read_size > 68719476735)
        {
            // too conent
            iserror = true;
            return;
        }
    }
    else if(offsetnum == readnum)
    {
        return;
    }
    else
    {
        //read error
        iserror = true;
        return;
    }
}
void rpc_client::process_body(const unsigned char *buffer, unsigned int readnum)
{
    if(isfinish)
    {
        return;
    }
    if(iserror)
    {
        return;
    }


    if(page.page_size == 0)
    {
        isfinish = true;
        return;
    } 
    if(page.page_size > 65536)
    {
        if(uprawfile == nullptr)
        {
            server_loaclvar &localvar = get_server_global_var();
            read_key = std::to_string(timeid()) + std::to_string(std::hash<std::string>{}(url)) + std::to_string(rand_range(1000, 9999));

            read_value = localvar.temp_path;
            read_value.append(std::to_string(std::hash<std::string>{}(read_key)));

            // uprawfile = fopen(upfile.tempfile.c_str(), "wb");
            uprawfile.reset(fopen(read_value.c_str(), "wb"));
 
            page.tempfile = read_value;
 
            page.isfile = true;
        }

        if(uprawfile == nullptr)
        {
            //
            iserror = true;
            return;
        }
        if(offsetnum < readnum)
        {
            fwrite(&buffer[offsetnum], (readnum - offsetnum), 1, uprawfile.get());
        }
        else if(offsetnum == readnum)
        {
            return;
        }
        else
        {
            //
            iserror = true;
            return;
        }
        page.read_size =  page.read_size - (readnum - offsetnum);
        if( page.read_size > 68719476735)
        {
            // too conent
            iserror = true;
            uprawfile.reset(nullptr);
            return;
        }
        if( page.read_size == 0)
        {
            uprawfile.reset(nullptr);
            isfinish = true;
            return;
        }
    }
    else
    {
        if(offsetnum < readnum)
        {
            page.content.append((char *)&buffer[offsetnum], (readnum - offsetnum));
            if(content_size >= 0 && page.content.size() == static_cast<size_t>(content_size))
            {
                isfinish = true;
                page.content_length = content_size;
                return;
            }
        }
        else if(offsetnum == readnum)
        {
            return;
        }
        else
        {
            //read error
            iserror = true;
            return;
        }

        page.read_size = page.read_size - (readnum - offsetnum);
        if(page.read_size > 68719476735)
        {
            // too conent
            iserror = true;
            return;
        }
        
        if(page.read_size == 0)
        {
            isfinish = true;
            page.content_length = content_size;
            return;
        }
    }
}

void rpc_client::run_loop()
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

            std::cout<<"++++++async_read_some+++++++"<<std::endl;
            if(run_loop_fun != nullptr)
            {
                run_loop_fun(self,n);
            }
            else if(async_run_loop_fun != nullptr)
            {
                co_spawn(strand_, [self, n]() mutable
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

asio::awaitable<void> rpc_client::async_run_loop()
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
            std::cout<<"++++++async_read_some+++++++"<<std::endl;
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
                co_return;
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


} //end http