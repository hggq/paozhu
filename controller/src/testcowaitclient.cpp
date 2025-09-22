#include "httppeer.h"
#include "orm.h"
#include "func.h"
#include "testcowaitclient.h"
#include "client_context.h"
#include "httpclient.h"
namespace http
{
//@urlpath(null,testcowaitclient4)
std::string testhttpclient_cowait_php(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  test testhttpclient_cowait_body";
    //client_context &temp_io_context = get_client_context_obj();

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get("https://www.php.net/docs.php");
    a->add_header("Connection", "keep-alive");
    a->send();
    client << a->get_header();
    if (a->get_status() == 200)
    {
        //  client << a->getBody();
        std::cout << "https://www.php.net/manual/zh/copyright.php" << std::endl;
        a->get("https://www.php.net/manual/zh/copyright.php");
        a->add_header("Connection", "Close");
        a->send();
        if (a->get_status() == 200)
        {
            client << a->get_body();
        }
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
    httppeer &client = peer->get_peer();
    client << "hello world!  test testhttpclient_cowait_body";
    //client_context &temp_io_context = get_client_context_obj();

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get("https://gcc.gnu.org/gcc-12/changes.html");
    a->send();
    if (a->get_status() == 200)
    {
        client << a->get_body();
    }
    else
    {
        client << "+++++not content++++";
    }
    return "";
}

//@urlpath(null,testcowaitclient21)
asio::awaitable<std::string> testhttpclient21_cowait_body(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world! co_await test testhttpclient_cowait_body";
    //client_context &temp_io_context = get_client_context_obj();

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get("https://gcc.gnu.org/gcc-12/changes.html");
    co_await a->async_send();
    if (a->get_status() == 200)
    {
        client << a->get_body();
    }
    else
    {
        client << "+++++not content++++";
    }
    co_return "";
}

//@urlpath(null,testcowaitclient22)
asio::awaitable<std::string> testhttpclient22_cowait_body(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world! co_await test localhost http://127.0.0.1addpost.html ";
    //client_context &temp_io_context = get_client_context_obj();

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get("http://127.0.0.1/addpost.html");
    co_await a->async_send();
    if (a->get_status() == 200)
    {
        client << a->get_body();
    }
    else
    {
        client << "+++++not content++++";
    }
    co_return "";
}

//@urlpath(null,testcowaitclient5)
std::string testhttpclient_cowait_post(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  test testhttpclient_cowait_body";
    //client_context &temp_io_context = get_client_context_obj();

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get("http://www.xxxxxx.net/");
    a->add_header("Connection", "keep-alive");
    a->send();
    if (a->get_status() == 200)
    {
        //  client << a->getBody();
        std::cout << "http://www.xxxxxx.net/login.php" << std::endl;
        a->post("http://www.xxxxxx.net/login.php");
        a->add_header("Connection", "keep-alive");
        a->data["user_name"] = "admin";
        a->data["user_pass"] = "123456";
        a->data["action"]    = "login";
        a->send();
        if (a->get_status() == 200)
        {
            //client << a->getBody();
            std::cout << "http://www.xxxxxx.net/main.php" << std::endl;
            a->get("http://www.xxxxxx.net/main.php");
            a->requst_clear();
            a->add_cookie("PHPSESSID", a->get_cookie("PHPSESSID"));
            a->add_header("Connection", "Close");

            a->send();
            if (a->get_status() == 200)
            {
                std::cout << a->get_body() << std::endl;
                //client << a->getBody();
            }
        }
    }
    else
    {
        client << a->error_msg;
        client << "+++++not content++++";
    }
    return "";
}

//@urlpath(null,testcowaitclient2)
std::string testhttpclient_cowait_urls(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
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
            a->add_header("Connection", "keep-alive");
        }
        a->onload = [](const std::string &respbody, std::shared_ptr<http::client> a) -> void
        {
            if (a->host == "www.php.net" && a->path == "/docs.php")
            {
                a->get("https://www.php.net/manual/zh/copyright.php");
                http::client_context &client_context_in = get_client_context_obj();
                a->add_header("Connection", "Close");
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
    httppeer &client = peer->get_peer();
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
            client_context.get_ctx(),
            [](std::shared_ptr<http::client> a) -> asio::awaitable<void>
            {
                co_await a->async_send();
            }(a),
            asio::detached);
    }
    return "";
}

//@urlpath(null,testclientgetrange)
std::string testhttpclient_get_range(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  test testhttpclient_cowait_body";

    client_context &client_context = get_client_context_obj();
    std::vector<std::string> urls  = {"https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz",
                                      "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang+llvm-18.1.8-arm64-apple-macos11.tar.xz"};

    for (unsigned int i = 0; i < urls.size(); i++)
    {
        std::shared_ptr<http::client> a = std::make_shared<http::client>();
        a->get(urls[i]);
        a->add_header("Range", "bytes=100-13919154");
        client << "<p>" << i << "</p>";
        a->onload = [](const std::string &respbody, std::shared_ptr<http::client> a) -> void
        {
            std::cout << a->host << std::endl;
        };
        // co_spawn(
        //     client_context.ioc,
        //     [](std::shared_ptr<http::client> a) -> asio::awaitable<void>
        //     {
        //         co_await a->async_send();
        //     }(a),
        //     asio::detached);
        client_context.add_http_task(a);
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return "";
}

//@urlpath(null,downfilelist)
std::string testhttpclient_downfilelist(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<html><head>";
    std::vector<std::string> urls = {"https://www.xxx.com/api/articleall?page=","&limit=10&order=time&category=0"};

    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    std::string tempurl;
    unsigned int i = 1;
 
    for ( ; i < 2; i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        tempurl.clear();
        tempurl.append(urls[0]);
        tempurl.append(std::to_string(i));
        tempurl.append(urls[1]);
        a->requst_clear();
        a->state.json.clear();
        std::cout<<tempurl<<std::endl;
        a->get_json(tempurl);
        a->add_header("Connection", "keep-alive");
        a->send();
        std::cout<<"----begin header----\n"<<std::endl;
        std::cout<<a->get_header();
        std::cout<<"----end header----\n"<<std::endl;

        if (a->get_status() == 200)
        {
            std::cout<<"resp ok"<<std::endl;
            if(a->state.isjson ==1)
            {
                std::cout<<"begin json"<<std::endl;
                if(a->state.json["code"].to_int()==0)
                {
                    if(a->state.json["data"].is_array())
                    {
                        std::cout<<"is_array"<<std::endl;
                        unsigned int j=0;
                        for(;j<a->state.json["data"].size();j++)
                        {
                           std::cout<<"fid:" <<a->state.json["data"][j]["aritcleid"].to_string()<<std::endl;
                        }

                        if(j>0)
                        {
                            client <<"<meta http-equiv=\"refresh\" content=\"1;url=/downfilelist\">";
                            client << "</head></html>";
                        }

                    }
                    else if(a->state.json["data"].is_obj())
                    {
                    
                        std::cout<<"is_obj"<<std::endl;
                    }
                    std::cout << a->state.content << std::endl;
                    continue;
                }
                std::cout<<"json error!"<<std::endl;
                break;
            }
            else 
            {
                std::cout<<"not json"<<std::endl;
                 
                break;
            }
        }
        std::cout<<"get_status!"<<a->get_status()<<std::endl;
        break;
    }
    return "";
}

//@urlpath(null,downfilecontent)
std::string testhttpclient_getdownfile(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<html><head>";

    //std::vector<std::string> urls  = {"https://gcc.gnu.org/gcc-12/changes.html", "https://www.php.net/docs.php", "https://gcc.gnu.org/gcc-13/changes.html"};
    std::vector<std::string> urls = {"https://www.xxx.com/api/upload?did="};

    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    std::string tempurl;
    unsigned int fid = 1;

    if(fid == 0)
    {
        return "";
    }

    std::string fileurl=urls[0];
    fileurl.append(std::to_string(fid));
    std::cout<<"----begin----\n"<<fileurl<<std::endl;
    a->get(fileurl);
    a->add_header("User-Agent","Mozilla/5.0 AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/57.0.2987.132 MQQBrowser/6.2 TBS/043807 Mobile Safari/537.36 MicroMessenger/6.6.1.1220(0x26060135) miniProgram");
    
    a->onheader= [](const char *buffer, unsigned int buffersize, unsigned int code) -> bool
    {
        std::cout<<"----begin header--"<<std::to_string(code)<<"--"<<std::endl;
        for(unsigned int i=0;i<buffersize;i++)
        {
            std::cout<<buffer[i];
        }
        std::cout<<"\n----end header----"<<std::endl;
        return false;
    };
    a->onrequest=[](std::string &str) -> void
    {
        std::cout<<"----begin request--"<<std::endl;
        std::cout<<str;
        std::cout<<"\n----end request----"<<std::endl;
    };
    //a->timeout(10);
    //a->set_limit_time(800);
    a->send();

    if (a->get_status() == 200)
    {
        std::cout<<"----get ok----\n"<<std::endl;
        if(a->state.istxt==false)
        {
            if(a->state.page.tempfile.size()>0)
            {
                std::cout<<"filename:"<<a->state.page.filename<<std::endl;
                fileurl = a->state.page.filename;
 
                client << "<html><head>";
                client <<"<meta http-equiv=\"refresh\" content=\"1;url=/downfilecontent\">";
                client << "</head>";
                client << "<body>";
                client << fileurl;
                client << "</body>";
                client << "</head></html>";
                return "";
            }
        }
    }
    client << "<html><body>";
    client <<" error! ";
    client << "</body></html>";
    return "";
}

}// namespace http
