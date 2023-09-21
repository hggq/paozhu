#include "httppeer.h"
#include "func.h"
#include "testcowaitclient.h"
#include "client_context.h"
#include "httpclient.h"
namespace http
{
//@urlpath(null,testcowaitclient4)
std::string testhttpclient_cowait_php(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "hello world!  test testhttpclient_cowait_body";
    //client_context &temp_io_context = get_client_context_obj();

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get("https://www.php.net/docs.php");
    a->send();
    client << a->getHeader();
    if (a->getStatus() == 200)
    {
        client << a->getBody();
    }
    else
    {
        client << "+++++not content++++";
    }
    return "";
}
//@urlpath(null,testcowaitclient1)
std::string testhttpclient_cowait_body(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "hello world!  test testhttpclient_cowait_body";
    //client_context &temp_io_context = get_client_context_obj();

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get("https://gcc.gnu.org/gcc-12/changes.html");
    a->send();
    if (a->getStatus() == 200)
    {
        client << a->getBody();
    }
    else
    {
        client << "+++++not content++++";
    }
    return "";
}
//@urlpath(null,testcowaitclient2)
std::string testhttpclient_cowait_urls(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "hello world!  test testhttpclient_cowait_body";

    http::client_context &client_context = get_client_context_obj();
    //std::vector<std::string> urls  = {"https://gcc.gnu.org/gcc-12/changes.html", "https://www.php.net/docs.php", "https://gcc.gnu.org/gcc-13/changes.html"};
    std::vector<std::string> urls = {"https://www.php.net/docs.php"};

    for (unsigned int i = 0; i < urls.size(); i++)
    {
        std::shared_ptr<http::client> a = std::make_shared<http::client>();
        a->get(urls[i]);
        if (a->host == "www.php.net")
        {
            a->addheader("Connection", "keep-alive");
        }
        a->onload = [](const std::string &respbody, std::shared_ptr<http::client> a) -> void
        {
            if (a->host == "www.php.net" && a->path == "/docs.php")
            {
                a->get("https://www.php.net/manual/zh/copyright.php");
                http::client_context &client_context_in = get_client_context_obj();
                a->addheader("Connection", "Close");
                client_context_in.add_http_task(a);
            }
            else if (a->host == "www.php.net" && a->path == "/manual/zh/copyright.php")
            {
                std::cout << respbody << std::endl;
            }
        };
        client_context.add_http_task(a);
    }
    return "";
}
//@urlpath(null,testcowaitclient3)
std::string testhttpclient_cowait_spawn(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "hello world!  test testhttpclient_cowait_body";

    client_context &client_context = get_client_context_obj();
    std::vector<std::string> urls  = {"https://gcc.gnu.org/gcc-12/changes.html", "https://gcc.gnu.org/gcc-13/changes.html", "https://www.php.net/docs.php"};

    for (unsigned int i = 0; i < urls.size(); i++)
    {
        std::shared_ptr<http::client> a = std::make_shared<http::client>();
        a->get(urls[i]);
        a->onload = [](const std::string &respbody, std::shared_ptr<http::client> a) -> void
        {
            std::cout << respbody << "\n"
                      << a->host << std::endl;
        };
        co_spawn(
            client_context.ioc,
            [](std::shared_ptr<http::client> a) -> asio::awaitable<void>
            {
                co_await a->co_send();
            }(a),
            asio::detached);
    }
    return "";
}
}// namespace http