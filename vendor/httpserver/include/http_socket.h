#ifndef __HTTP_OBJ_SOCKET_API_H
#define __HTTP_OBJ_SOCKET_API_H

#include <vector>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include "request.h"
#include "httppeer.h"
#include "client_session.h"

namespace http
{
    struct socket_api_data_t
    {
        std::string name;
        std::string value;
    };
    class socket_api : public std::enable_shared_from_this<socket_api> 
    {
    public:
        socket_api(unsigned int t, unsigned int m, unsigned int g, unsigned char s, std::shared_ptr<client_session> s_sock) : state(s), timeloop_num(t), myid(m), groupid(g),session_sock(s_sock),notify_timer_(s_sock->strand_){}
        virtual void on_open()                    = 0;
        virtual asio::awaitable<void> async_on_open()= 0;
        virtual void on_close()                    = 0;
        virtual asio::awaitable<void> async_on_message(std::string &&buffer)
        {
            unsigned int readoffset = 0;
            for(; readoffset < buffer.size(); readoffset++)
            {
                if(buffer[readoffset]=='\n')
                {
                    break;
                }
            }
            co_return;
        }
        virtual void run_loop()                  = 0;
        virtual asio::awaitable<void> async_run_loop()                  = 0;
        asio::awaitable<void> wait_up() 
        {
            notify_timer_.expires_after(std::chrono::hours(24 * 365));
            co_await notify_timer_.async_wait(asio::as_tuple(asio::use_awaitable));
        }
        asio::awaitable<void> loop_process() 
        {
            auto self = shared_from_this();
            while (!isclose)
            {
                std::string buffer;
                {
                    std::unique_lock<std::mutex> lock(mtx_list_lock);
                    if (content_list.empty()) {
                        lock.unlock(); 
                        co_await wait_up();
                        continue;
                    }
                    buffer = std::move(content_list.front());
                    content_list.pop_front();
                }

                if(isbody)
                {
                    co_await async_on_message(std::move(buffer));
                }
                else
                {
                    co_await async_process(std::move(buffer));
                }
                
            }
            co_return;
        }
        virtual asio::awaitable<void> async_process(std::string &&buffer)
        {
            unsigned int readoffset = 0;
            if(isbegin)
            {
                if(isbody)
                {
                    co_await async_on_message(std::move(buffer));
                    co_return;
                }
                else
                {
                    if(cur_state == 1)
                    {
                        //continu read keyname;
                        for(; readoffset < buffer.size(); readoffset++)
                        {
                            if(buffer[readoffset]=='=')
                            {
                                readoffset++;
                                for(; readoffset < buffer.size(); readoffset++)
                                {
                                    if(buffer[readoffset]=='&')
                                    {
                                        header.push_back({read_key,read_value}); 
                                        break;
                                    }
                                    else if(buffer[readoffset]==0x0A)
                                    {
                                        if(read_key.size() > 0)
                                        {
                                            header.push_back({read_key,read_value}); 
                                        }
                                        break;
                                    }
                                    read_value.push_back(buffer[readoffset]);
                                }
                                if(readoffset == buffer.size())
                                {
                                    cur_state = 2;
                                    co_return;
                                }

                                read_key.clear();
                                read_value.clear();
                                if(buffer[readoffset]==0x0A)
                                {
                                    break;
                                }
                                continue;
                            }
                            else if(buffer[readoffset]==0x0A)
                            {
                                break;
                            }
                            read_key.push_back(buffer[readoffset]);
                        }
                        
                        if(readoffset == buffer.size())
                        {
                            cur_state = 1;
                            co_return;
                        }

                        if(read_key.size() > 0)
                        {
                            header.push_back({read_key,read_value}); 
                        }

                        if(readoffset < buffer.size() && buffer[readoffset]==0x0A)
                        {
                            readoffset++;
                            if(buffer[readoffset]==0x0A)
                            {
                                readoffset++;
                                isbody = true;
                            }
                        }

                        if(isbody && readoffset < buffer.size())
                        {
                            std::string temp_aa;
                            temp_aa.append(buffer.data() + readoffset,buffer.size() - readoffset);
                            co_await async_on_message(std::move(temp_aa));
                        }
                    }
                    else if(cur_state == 2)
                    {
                        //continu read value;
                        for(; readoffset < buffer.size(); readoffset++)
                        {
                            if(buffer[readoffset]=='&')
                            {
                                if(read_key.size() > 0)
                                {
                                    header.push_back({read_key,read_value});
                                }
                                readoffset++; 
                                cur_state = 1;
                                //keep level 1
                                break;
                            }
                            else if(buffer[readoffset]==0x0A)
                            {
                                if(read_key.size() > 0)
                                {
                                    header.push_back({read_key,read_value}); 
                                }
                                break;
                            }
                            read_value.push_back(buffer[readoffset]);
                        }
                        
                        if(readoffset == buffer.size())
                        {
                            cur_state = 1;
                            co_return;
                        }

                        for(; readoffset < buffer.size(); readoffset++)
                        {
                            if(buffer[readoffset]=='=')
                            {
                                readoffset++;
                                for(; readoffset < buffer.size(); readoffset++)
                                {
                                    if(buffer[readoffset]=='&')
                                    {
                                        header.push_back({read_key,read_value}); 
                                        break;
                                    }
                                    else if(buffer[readoffset]==0x0A)
                                    {
                                        if(read_key.size() > 0)
                                        {
                                            header.push_back({read_key,read_value}); 
                                        }
                                        break;
                                    }
                                    read_value.push_back(buffer[readoffset]);
                                }
                                if(readoffset == buffer.size())
                                {
                                    cur_state = 2;
                                    co_return;
                                }

                                read_key.clear();
                                read_value.clear();
                                if(buffer[readoffset]==0x0A)
                                {
                                    break;
                                }
                                continue;
                            }
                            else if(buffer[readoffset]==0x0A)
                            {
                                break;
                            }
                            read_key.push_back(buffer[readoffset]);
                        }
                        
                        if(readoffset == buffer.size())
                        {
                            cur_state = 1;
                            co_return;
                        }

                        if(read_key.size() > 0)
                        {
                            header.push_back({read_key,read_value}); 
                        }
                        
                        if(readoffset < buffer.size() && buffer[readoffset]==0x0A)
                        {
                            readoffset++;
                            if(buffer[readoffset]==0x0A)
                            {
                                readoffset++;
                                isbody = true;
                            }
                        }

                        if(isbody && readoffset < buffer.size())
                        {
                            std::string temp_aa;
                            temp_aa.append(buffer.data() + readoffset,buffer.size() - readoffset);
                            co_await async_on_message(std::move(temp_aa));
                        }
                    }
                }
            }
            else
            {
                isbegin = true;
                cur_state = 0;

                if(readoffset < buffer.size())
                {
                    if(buffer[readoffset]=='?')
                    {
                        readoffset++;
                        for(; readoffset < buffer.size(); readoffset++)
                        {
                            if(buffer[readoffset]=='=')
                            {
                                readoffset++;
                                for(; readoffset < buffer.size(); readoffset++)
                                {
                                    if(buffer[readoffset]=='&')
                                    {
                                        header.push_back({read_key,read_value}); 
                                        break;
                                    }
                                    else if(buffer[readoffset]==0x0A)
                                    {
                                        if(read_key.size() > 0)
                                        {
                                            header.push_back({read_key,read_value}); 
                                        }
                                        break;
                                    }
                                    read_value.push_back(buffer[readoffset]);
                                }
                                if(readoffset == buffer.size())
                                {
                                    cur_state = 2;
                                    co_return;
                                }

                                read_key.clear();
                                read_value.clear();
                                if(buffer[readoffset]==0x0A)
                                {
                                    break;
                                }
                                continue;
                            }
                            else if(buffer[readoffset]==0x0A)
                            {
                                break;
                            }
                            read_key.push_back(buffer[readoffset]);
                        }
                        
                        if(readoffset == buffer.size())
                        {
                            cur_state = 1;
                            co_return;
                        }
                        if(read_key.size() > 0)
                        {
                            header.push_back({read_key,read_value}); 
                        }
                    }
                    
                    if(buffer[readoffset]==0x0A)
                    {
                        readoffset++;
                        if(buffer[readoffset]==0x0A)
                        {
                            readoffset++;
                            isbody = true;
                        }
                    }
                    else if(buffer[readoffset]==0x20)
                    {
                        //the space end header
                        readoffset++;
                        isbody = true;
                    }
                    else 
                    {
                        isclose = true;
                        co_return;
                    }

                    if(readoffset < buffer.size())
                    {
                        std::string temp_aa;
                        temp_aa.append(buffer.data() + readoffset,buffer.size() - readoffset);
                        co_await async_on_message(std::move(temp_aa));
                    }
                }
                else
                {
                    co_return;
                }
            }
        }
        virtual ~socket_api()
        {
            isclose = true;
            notify_timer_.cancel();
        };
        void push(std::string&& item) 
        {
            std::unique_lock<std::mutex> lock(mtx_list_lock);
            content_list.push_back(std::move(item));
            notify_timer_.cancel();
        }
public:
        bool isclose = false;
        bool isbegin = false;
        bool isbody = false;
        bool isfinish = false;
        bool isco = false;
        bool isloopco = false;
        unsigned char state;
        unsigned char cur_state;
        unsigned int timeloop_num;
        unsigned int myid=0;
        unsigned int groupid = 0;
        unsigned int current_time=0;

        std::string error_msg;
        std::string url;
        std::string host;
        std::string read_key;   //read temp var in the header
        std::string read_value;
        std::shared_ptr<client_session> session_sock;
        std::vector<socket_api_data_t> header;
        std::list<std::string> content_list;
        std::mutex mtx_list_lock;
        asio::steady_timer notify_timer_;
    };
    typedef std::map<std::string, std::function<std::shared_ptr<socket_api>(unsigned int myid_, unsigned int groupid_, std::shared_ptr<client_session>)>> HTTP_SOCKET_REG;
    HTTP_SOCKET_REG &get_http_socket_reg();

}

#endif