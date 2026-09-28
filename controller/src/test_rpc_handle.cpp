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
    a->chunk_process =[](std::shared_ptr<http::rpc_client> cli)->void {
             if(cli->page.page_size > 0)
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

//@urlpath(null,test_rpc_binary)
// 二进制帧端到端用例：返回 application/octet-stream，body 含 0x00，
// 验证 build_header 的 size/type 字段与 send_header 自定义头。
asio::awaitable<std::string> test_rpc_binary(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("application/octet-stream");
    // 自定义响应头，验证 build_header 的 key 小写化 / 空格转下划线
    client.send_header["X-Trace-Id"] = "bin-001";
    // 写入含 0x00 的二进制 body
    client.output.append("\x00\x01\x02\x03\x04\x05", 6);
    client.output.push_back((char)0xFF);
    co_return "";
}

//@urlpath(null,test_rpc_binary_echo)
// 二进制回显：把请求 rawcontent 原样写回，验证请求 body 解析与响应构建闭环。
asio::awaitable<std::string> test_rpc_binary_echo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("application/octet-stream");
    client.output = peer->rawcontent;
    co_return "";
}

}//namespace http
