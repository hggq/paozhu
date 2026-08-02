#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_httppeersend.h"
#include "func.h"
#include <memory>
#include <string>
#include <fstream>
#include <vector>

namespace http
{

//@urlpath(null,test_httppeersend)
std::string test_httppeersend(std::shared_ptr<httppeer> peer)
{
    server_loaclvar &static_server_var = get_server_global_var();
    if (static_server_var.log_path.size() < 5)
    {
        peer->type("text/plain");
        peer->status(500);
        peer->send_make_header();
        peer->send_make_body("错误：log_path 未设置\n");
        peer->send_make_end();
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
        peer->send_make_header();
        peer->send_make_body("文件不存在: " + file_conf);
        peer->send_make_end();
        return "";
    }

    // 获取文件大小，设置 Content-Length
    f.seekg(0, std::ios::end);
    auto file_size = f.tellg();
    f.seekg(0, std::ios::beg);

    if (file_size == -1)
    {
        peer->type("text/plain");
        peer->status(500);
        peer->send_make_header();
        peer->send_make_body("读取文件大小失败\n");
        peer->send_make_end();
        return "";
    }

    peer->type("application/pdf");
    peer->status(200);
    peer->length(file_size);  // 关键：设置 Content-Length
    peer->send_make_header();

    // 分块发送文件
    std::vector<char> buf(8192);
    while (f)
    {
        f.read(buf.data(), buf.size());
        auto n = f.gcount();
        if (n > 0)
        {
            peer->send_make_body(std::string_view(buf.data(), n));
        }
    }

    peer->send_make_end();
    return "";
}

//@urlpath(null,test_httppeersend_text)
std::string test_httppeersend_text(std::shared_ptr<httppeer> peer)
{
    // Content-Length 发送纯文本
    std::string content = "Hello! 这是通过 Content-Length 模式发送的文本内容。\n"
                          "第一行\n第二行\n第三行\n";

    peer->type("text/plain; charset=utf-8");
    peer->status(200);
    peer->length(content.size());
    peer->send_make_header();
    peer->send_make_body(content);
    peer->send_make_end();
    return "";
}

//@urlpath(null,test_httppeersend_async)
asio::awaitable<std::string> test_httppeersend_async(std::shared_ptr<httppeer> peer)
{
    server_loaclvar &static_server_var = get_server_global_var();
    if (static_server_var.log_path.size() < 5)
    {
        peer->type("text/plain");
        peer->status(500);
        co_await peer->async_send_make_header();
        co_await peer->async_send_make_body("错误：log_path 未设置\n");
        co_await peer->async_send_make_end();
        co_return "";
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
        co_await peer->async_send_make_header();
        co_await peer->async_send_make_body("文件不存在: " + file_conf);
        co_await peer->async_send_make_end();
        co_return "";
    }

    f.seekg(0, std::ios::end);
    auto file_size = f.tellg();
    f.seekg(0, std::ios::beg);

    if (file_size == -1)
    {
        peer->type("text/plain");
        peer->status(500);
        co_await peer->async_send_make_header();
        co_await peer->async_send_make_body("读取文件大小失败\n");
        co_await peer->async_send_make_end();
        co_return "";
    }

    peer->type("application/pdf");
    peer->status(200);
    peer->length(file_size);
    co_await peer->async_send_make_header();

    std::vector<char> buf(8192);
    while (f)
    {
        f.read(buf.data(), buf.size());
        auto n = f.gcount();
        if (n > 0)
        {
            co_await peer->async_send_make_body(std::string_view(buf.data(), n));
        }
    }

    co_await peer->async_send_make_end();
    co_return "";
}

}// namespace http
