#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_chunked.h"
#include "func.h"
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>

namespace http
{

//@urlpath(null,test_chunked)
std::string test_chunked(std::shared_ptr<httppeer> peer)
{
    peer->type("text/html; charset=utf-8");
    peer->status(200);
    peer->send_chunk_header();

    peer->send_chunk_body("<html><body>\n");
    peer->send_chunk_body("<h1>Chunked Transfer 测试</h1>\n");

    for (int i = 1; i <= 5; i++)
    {
        std::string line = "<p>第 " + std::to_string(i) +
                           " 行：这是通过 chunked transfer 发送的内容。</p>\n";
        peer->send_chunk_body(line);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    peer->send_chunk_body("</body></html>");
    peer->send_chunk_end();
    return "";
}

//@urlpath(null,test_chunkedfile)
std::string test_chunkedfile(std::shared_ptr<httppeer> peer)
{
    server_loaclvar &static_server_var = get_server_global_var();
    if (static_server_var.log_path.size() < 5)
    {
        peer->type("text/plain");
        peer->status(500);
        peer->send_chunk_header();
        peer->send_chunk_body("错误：log_path 未设置\n");
        peer->send_chunk_end();
        return "";
    }

    std::string file_conf = dir_name(static_server_var.log_path);
    if (file_conf.size() > 0 && file_conf.back() != '/')
    {
        file_conf.push_back('/');
    }
    file_conf.append("docs/webpdf_tutorial.pdf");

    std::ifstream f(file_conf, std::ios::binary);
    if (!f.is_open())
    {
        peer->type("text/plain");
        peer->status(404);
        peer->send_chunk_header();
        peer->send_chunk_body("文件不存在: " + file_conf);
        peer->send_chunk_end();
        return "";
    }

    peer->type("application/pdf");
    peer->status(200);
    peer->send_chunk_header();

    std::vector<char> buf(8192);
    while (f)
    {
        f.read(buf.data(), buf.size());
        auto n = f.gcount();
        if (n > 0)
        {
            peer->send_chunk_body(std::string_view(buf.data(), n));
        }
    }

    peer->send_chunk_end();
    return "";
}

//@urlpath(null,test_chunked_async)
asio::awaitable<std::string> test_chunked_async(std::shared_ptr<httppeer> peer)
{
    peer->type("text/html; charset=utf-8");
    peer->status(200);
    co_await peer->async_send_chunk_header();

    co_await peer->async_send_chunk_body("<html><body>\n");
    co_await peer->async_send_chunk_body("<h1>Chunked Async 测试</h1>\n");

    for (int i = 1; i <= 3; i++)
    {
        std::string line = "<p>异步第 " + std::to_string(i) + " 行</p>\n";
        co_await peer->async_send_chunk_body(line);
    }

    co_await peer->async_send_chunk_body("</body></html>");
    co_await peer->async_send_chunk_end();

    co_return "";
}

}// namespace http
