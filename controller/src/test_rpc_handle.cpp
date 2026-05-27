#include <chrono>
#include <thread>
#include "httppeer.h"
#include "test_rpc_handle.h"
#include "http_rpcclient.h"


namespace http
{
//@urlpath(null,test_rpcclient)
asio::awaitable<std::string> test_rpcclient(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " hello world! this is a test test_rpcclient function. ";

    std::shared_ptr<http::rpc_client> a = std::make_shared<http::rpc_client>();

    a->set_url("http://127.0.0.1/test_rpcserver");
    co_await a->async_send();
    client << " <hr> ";
    client << a->page.content;

    co_return "";
}

//@urlpath(null,test_rpcclientssl)
asio::awaitable<std::string> test_rpcclientssl(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " hello world! this is a test test_rpcclientssl function. ";

    std::shared_ptr<http::rpc_client> a = std::make_shared<http::rpc_client>();

    a->set_url("https://www.xx.com/test_rpcserver");
    co_await a->async_send();
    client << " <hr> ";
    client << a->page.content;

    co_return "";
}

//@urlpath(null,test_rpcserver)
asio::awaitable<std::string> test_rpcserver(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "rpc server!";
    co_return "";
}



//@urlpath(null,test_rpc_chunkc)
asio::awaitable<std::string> test_rpc_chunkc(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " hello world! this is a test test_rpcclient function. ";

    std::shared_ptr<http::rpc_client> a = std::make_shared<http::rpc_client>();

    a->set_url("http://127.0.0.1/test_rpc_chunks");
    a->chunk_process =[](std::shared_ptr<http::rpc_client> a)->void {
             if(a->page.page_size > 0)
             {

             }
    };
    co_await a->async_send();
    client << " <hr> ";
    client << a->page.content;

    co_return "";
}


//@urlpath(null,test_rpc_chunks)
asio::awaitable<std::string> test_rpc_chunks(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "rpc server!";
    co_return "";
}

}//namespace http
