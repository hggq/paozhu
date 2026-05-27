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
    class socket_api
    {
    public:
        socket_api(unsigned int t, unsigned int m, unsigned int g, unsigned char s) : state(s), timeloop_num(t), myid(m), groupid(g){}
        virtual void on_open()                    = 0;
        virtual void on_close()                    = 0;
        virtual asio::awaitable<void> async_on_message(const unsigned char *buffer, unsigned int readoffset, unsigned int readnum)
        {
            for(; readoffset < readnum; readoffset++)
            {
                content.push_back(buffer[readoffset]);
            }
            co_return;
        }
        virtual void run_loop()                  = 0;
        virtual asio::awaitable<void> async_run_loop()                  = 0;
        virtual asio::awaitable<void> async_process(const unsigned char *buffer, unsigned int readoffset, unsigned int readnum)
        {
            if(isbegin)
            {
                if(isbody)
                {
                    co_await async_on_message(buffer, readoffset, readnum);
                    co_return;
                }
                else
                {
                    if(cur_state == 1)
                    {
                        //continu read keyname;
                        for(; readoffset < readnum; readoffset++)
                        {
                            if(buffer[readoffset]=='=')
                            {
                                readoffset++;
                                for(; readoffset < readnum; readoffset++)
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
                                if(readoffset == readnum)
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
                        
                        if(readoffset == readnum)
                        {
                            cur_state = 1;
                            co_return;
                        }

                        if(read_key.size() > 0)
                        {
                            header.push_back({read_key,read_value}); 
                        }

                        if(readoffset < readnum && buffer[readoffset]==0x0A)
                        {
                            readoffset++;
                            if(buffer[readoffset]==0x0A)
                            {
                                readoffset++;
                                isbody = true;
                            }
                        }

                        if(isbody && readoffset < readnum)
                        {
                            co_await async_on_message(buffer, readoffset, readnum);
                        }
                    }
                    else if(cur_state == 2)
                    {
                        //continu read value;
                        for(; readoffset < readnum; readoffset++)
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
                        
                        if(readoffset == readnum)
                        {
                            cur_state = 1;
                            co_return;
                        }

                        for(; readoffset < readnum; readoffset++)
                        {
                            if(buffer[readoffset]=='=')
                            {
                                readoffset++;
                                for(; readoffset < readnum; readoffset++)
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
                                if(readoffset == readnum)
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
                        
                        if(readoffset == readnum)
                        {
                            cur_state = 1;
                            co_return;
                        }

                        if(read_key.size() > 0)
                        {
                            header.push_back({read_key,read_value}); 
                        }
                        
                        if(readoffset < readnum && buffer[readoffset]==0x0A)
                        {
                            readoffset++;
                            if(buffer[readoffset]==0x0A)
                            {
                                readoffset++;
                                isbody = true;
                            }
                        }

                        if(isbody && readoffset < readnum)
                        {
                            co_await async_on_message(buffer, readoffset, readnum);
                        }
                    }
                }
            }
            else
            {
                isbegin = true;
                cur_state = 0;

                if(readoffset < readnum)
                {
                    if(buffer[readoffset]=='?')
                    {
                        readoffset++;
                        for(; readoffset < readnum; readoffset++)
                        {
                            if(buffer[readoffset]=='=')
                            {
                                readoffset++;
                                for(; readoffset < readnum; readoffset++)
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
                                if(readoffset == readnum)
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
                        
                        if(readoffset == readnum)
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

                    if(readoffset < readnum)
                    {
                        co_await async_on_message(buffer, readoffset, readnum);
                    }
                }
                else
                {
                    co_return;
                }
            }
        }
        virtual ~socket_api(){};

        bool isclose = false;
        bool isbegin = false;
        bool isbody = false;
        bool isfinish = false;
        unsigned char state;
        unsigned char cur_state;
        unsigned int timeloop_num;
        unsigned int myid=0;
        unsigned int groupid = 0;
        unsigned int current_time=0;

        std::string error_msg;
        std::string url;
        std::string content;
        std::string read_key;   //read temp var in the header
        std::string read_value;
        std::shared_ptr<client_session> session_sock;
        std::vector<socket_api_data_t> header;
    };
    typedef std::map<std::string, std::function<std::shared_ptr<socket_api>(unsigned int myid_,unsigned int groupid_)>> HTTP_SOCKET_REG;
    HTTP_SOCKET_REG &get_http_socket_reg();

}

#endif