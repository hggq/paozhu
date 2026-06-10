#include <chrono>
#include <thread>
#include "httppeer.h"
#include "test_websocket_handle.h"
#include "http_websocket_client.h"


namespace http
{
//@urlpath(null,test_websocket_client)
asio::awaitable<std::string> test_websocket_client(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " hello world! this is a test test_socket_client function. ";

    std::shared_ptr<http::websocket_client> a = std::make_shared<http::websocket_client>();

    std::string send_content;
    bool isok = co_await a->async_connect("ws://127.0.0.1:80/wstest");
    // a->set_url("127.0.0.1/wstest");
    // a->set_port(80);
    //bool isok = co_await a->async_connect();
    
    if(!isok)
    {
        client << " <hr> async_connect error.";
        co_return "";
    }

    send_content="websocket client";
    // std::string outdata;
    // a->make_ws_text(send_content,outdata);
    // unsigned int n = co_await a->async_write(outdata);
    unsigned int n = co_await a->async_text_write(send_content);
    client << " <hr >send:"<<n;

    n = co_await a->async_text_read();
    client << "  "<< a->recv_data.content;
    a->reset_recv_status();
    //end echo http client
    //Let the websocket client run alone in the background

    a->async_dur_time_loop_fun = [](std::shared_ptr<websocket_client> b)-> asio::awaitable<void> {
                            std::string send_content="websocket client loop";
                            std::string outdata;
                            b->make_ws_text(send_content,outdata);
                            unsigned int n = co_await b->async_write(outdata);
                            if(n == outdata.size())
                            {
                                DEBUG_LOG("async_dur_time_loop_fun:%s",send_content.c_str());
                            }
                            co_return;
                         };

    //read loop                     
    a->async_recv_finish_fun = [](std::shared_ptr<websocket_client> b)-> asio::awaitable<void> {
                            if(b->recv_data.length > 0)
                            {

                            }
                            co_return;
                         };                     
    a->async_run_loop_fun = [](std::shared_ptr<websocket_client> b, unsigned int n)-> asio::awaitable<void> {
                            
                            b->process_data(b->data, n);

                            if(b->recv_data.isfinish)
                            {
                                DEBUG_LOG("async_run_loop_fun:%s",b->recv_data.content.c_str());
                                if(b->async_recv_finish_fun != nullptr)
                                {
                                    co_await b->async_recv_finish_fun(b);
                                }
                                b->reset_recv_status();
                            }

                            co_return;
                        };
    
    co_spawn(a->strand_, [a]() mutable
                 { return a->async_run_loop(); },
                 asio::detached);
    //if not set time out, must add to client task loop             
    a->add_client_task_loop();             
    co_return "";
}

 

}//namespace http
