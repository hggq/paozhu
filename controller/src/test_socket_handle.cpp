#include <chrono>
#include <thread>
#include "httppeer.h"
#include "test_socket_handle.h"
#include "http_socket_client.h"


namespace http
{
//@urlpath(null,test_socket_client)
asio::awaitable<std::string> test_socket_client(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " hello world! this is a test test_socket_client function. ";

    std::shared_ptr<http::socket_client> a = std::make_shared<http::socket_client>();

    std::string send_content;
    bool isok = co_await a->async_connect("http://127.0.0.1:80/mytestsocket",30);
    // a->set_url("127.0.0.1/mytestsocket");
    // a->set_port(80);
    //bool isok = co_await a->async_connect();
    
    if(!isok)
    {
        client << " <hr> async_connect error.";
        co_return "";
    }
    send_content="tcp mytestsocket";
    send_content.push_back(0x0A);
    send_content.push_back(0x0A);
    send_content.append("test socket client");
    client << " <hr> ";
    unsigned int n = co_await a->async_write(send_content);
    send_content.clear();
 
    unsigned char recv_data[512];
    n = co_await a->async_read(recv_data,512);
 
    recv_data[511]=0x00;
    send_content.clear();
    send_content.append((char *)recv_data,0,n);
    client << send_content;

    a->durtime = 12;
    a->async_dur_time_loop_fun = [](std::shared_ptr<http::socket_client> b)-> asio::awaitable<void> 
                {
                        std::string send_content="time client loop mytestsocket";
                        co_await b->async_write(send_content);
                        std::cout<<"~~~~~~~~~~~~~~~~~~~"<<std::endl;
                        co_return;
                 };

    a->async_run_loop_fun = [](std::shared_ptr<http::socket_client> b,unsigned int readnum)-> asio::awaitable<void> 
                {
                        for(unsigned int i=0; i < readnum; i++)
                        {
                            std::cout<<b->data[i];
                        }
                        std::cout<<std::endl;
                        co_return;
                 };
    co_spawn(a->strand_, [a]() mutable
                 { return a->async_run_loop(); },
                 asio::detached);
    co_return "";
}

 

}//namespace http
