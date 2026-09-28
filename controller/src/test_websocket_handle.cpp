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
    client << " <hr >send:"<< n;

    n = co_await a->async_text_read();
    if (n > 0)
    {
        client << "  " << a->recv_data.content;
    }
    else
    {
        client << "  <read error: " << a->error_msg << ">";
    }
    a->reset_recv_status();
    // permessage-deflate roundtrip: payloads >256 bytes are auto-compressed on send,
    // echoed back compressed after server-side negotiation, auto-decompressed on receive by
    // the client state machine — we should read back exactly what we sent.
    send_content = "deflate-roundtrip:" + std::string(600, 'p');
    n = co_await a->async_text_write(send_content);
    client << " <hr >deflate send:" << n;
    n = co_await a->async_text_read();
    if (n > 0 && a->recv_data.content == send_content)
    {
        client << "  deflate echo ok, len:" << a->recv_data.content.size();
    }
    else
    {
        client << "  <deflate echo mismatch or read error: " << a->error_msg
               << ", len:" << a->recv_data.content.size() << ">";
    }
    a->reset_recv_status();
    //end echo http client
    //Let the websocket client run alone in the background
    /*
    * Simple usage above — sufficient for typical business code.
    * Advanced usage below: when the HTTP handler returns, the task keeps running
    * on a background task thread on a schedule.
    */

    a->async_dur_time_loop_fun = [](std::shared_ptr<websocket_client> b)-> asio::awaitable<void> {
                            std::string loop_content="websocket client loop";
                            // Route through async_text_write → send queue; bypassing the queue
                            // with a raw async_write can interleave with auto-pong frames.
                            unsigned int loop_n = co_await b->async_text_write(loop_content);
                            if(loop_n > 0)
                            {
                                DEBUG_LOG("async_dur_time_loop_fun:%s",loop_content.c_str());
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
    a->async_run_loop_fun = [](std::shared_ptr<websocket_client> b, [[maybe_unused]] websocket_client::ws_pack_data pack_data)-> asio::awaitable<void> {
                            (void)b;
                            // recv_data is cleared by the framework right after frame dispatch;
                            // read the copy that was captured into pack_data when the coroutine spawned.
                            DEBUG_LOG("async_run_loop_fun:%s",pack_data.content.c_str());
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
