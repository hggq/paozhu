简体中文 | [English](README_EN.md)

<div align="center">
<img src="https://hggq.github.io/paozhu/images/logo.svg">
</div>

**Paozhu**(炮竹🧨)是一个全栈快速的C++ web framework 开发框架，我们口号是“为 C++ Web 快速开发而生”，集成C++ ORM，开发速度跟脚本语言一样，日写1000行业务代码没有压力，框架集成了WebServer，自己原生解析HTTP/1、HTTP/2、JSON协议，GET、POST全部解析出来，POST几种方式都做了区分，框架自带一个OBJ微型对象，可以存放char
int string float等，框架自动解析URL、POST参数到微型对象里面，使用urlpath映射到函数挂载点，使用协程、线程池、数据库链接池。

QQ群：668296235  
作者也在一线写业务代码，根据经验开发本框架，如果有想法可以加群探讨或issues提问，本框架正缺人手，需要完善C++开发web生态。
controller目录是实际使用例子，还在完善中，目前框架压力测试没有什么问题。  

### 1.特性🔥🔥🔥🔥🔥

✅ 1. 自带json编解码不用第三方库，标准json支持  
✅ 2. 支持多域名网站  
✅ 3. 支持多域名ssl 服务端  
✅ 4. 支持http/1.1、http/2协议  
✅ 5. 支持WebSocket服务端和客户端，双工收发  
✅ 6. 框架自带WebSocket推送，支持定时推送到WebSocket客户端  
✅ 7. 支持httpclient get post，同步异步、协程模式，数据采集  
✅ 8. 框架自带ORM,使用链接池方式，目前支持MySQL，支持协程和同步模式  
✅ 9. 框架自带线程池，和用户代码运行的线程池  
✅10. 框架使用asio自带的协程  
✅11. 框架特色是I/O 使用协程池 用户代码运行使用线程池,类似GO那种调度，只是针对http请求调度  
✅12. 框架支持普通文件gzip、br,并支持缓存到磁盘，下次不用cpu再压缩  
✅13. URL、POST和上传文件,解析结果client.get[] client.post[] client.files[]方式获取内容  
✅14. 自带sendmail类库  
✅15. 生成二维码(qrcode),需要gd、qrencode库  
✅16. 插件化编程，热动态更新，使用动态库方式  
✅17. 框架内置通用数据缓存模块，ORM结果缓存，提高并发能力  
✅18. 框架controller目录注解功能，方便添加URL路由映射，降低入门心智  
✅19. 结构和类注解JSON功能，使用json_encode json_decode操作复杂C++结构体和JSON互转，可以参考Wiki  
✅20. 提供一个完整admin后台管理框架（见后面图片）, 访问URL为 /admin/main  
✅21. 支持PHP-FPM fastcgi运行模式，代替Apache做PHP前端，让PHP程序员平稳过渡到 C++ 开发模式  
✅22. 内置微信小程序支付功能  
✅23. 支持Socket服务端和客户端，支持Socket ssl连接  
✅24. 支持RPC服务端和客户端，使用http注册的URL函数  
✅25. 框架支持流量限制，60秒内平滑移动平均线二段式流量限制，配置参数在server.conf  
✅26. 框架内置ACME协议，自动续订SSL证书并生效  
✅27. 整合服务器端OCSP证书装订  
✅28. 简单的Excel文件xlsx读写支持  

### 2.运行环境

- C++ standard request c++20
- asio openssl zlib brotli [可选](gd qrencode)
- Supports Linux MacOS Windows

### 3.环境配置

#### 3.1MacOS

- 系统要求：

```
>= BigSur
```

- 安装必要库

```bash
brew install asio
brew install cmake
brew install openssl
brew install zlib
brew install brotli

[可选安装] brew install gd
[可选安装] brew install qrencode
```

#### 3.2 Ubuntu && Fedora

- 系统要求：

```
Ubuntu>= 22.04 , test on Fedora38 需要按装asan（sudo yum install libasan）
```

```
sudo apt-get install cmake
sudo apt-get install -y brotli  libbrotli-dev zlib1g-dev
sudo apt-get install libgd-dev
sudo apt-get install qrencode libqrencode-dev
sudo apt-get install openssl libssl-dev
sudo apt-get install mysql-server
 
```

- 环境配置  
  [MacOS 环境配置](https://hggq.github.io/paozhudocs/macos.html)  
  [Ubuntu 环境配置](https://hggq.github.io/paozhudocs/ubuntu.html)  
  [RockyLinux AlmaLinux 环境配置](https://hggq.github.io/paozhudocs/linux.html)  
  
#### 3.3 Windows 

支持vcpkg和xmake

xmake安装

在项目根目录 执行  
配置 `conf/server.conf` 替换 /Users/hzq/paozhu 为你的项目目录   
```
paozhu-main> xmake
paozhu-main> .\build\windows\x64\release\paozhu.exe
```

vcpkg安装

```
Invoke-WebRequest -OutFile vcpkg2023.zip https://github.com/microsoft/vcpkg/archive/refs/tags/2023.12.12.zip
            unzip vcpkg2023.zip
            cd vcpkg-2023.12.12
            bootstrap-vcpkg.bat
            vcpkg integrate install
            vcpkg integrate project
            vcpkg install
```

安装 ninja https://github.com/ninja-build/ninja/releases

编译项目 注意`CMAKE_TOOLCHAIN_FILE`指定vcpkg位置 

```
unzip asio.zip
cmake . -B ${{github.workspace}} -DENABLE_WIN_VCPKG=ON -DENABLE_VCPKG=ON -DCMAKE_TOOLCHAIN_FILE=vcpkg-2023.12.12/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE=Debug -G Ninja
cmake --build . --config Debug
```


### 4.安装方法

解压 `unzip asio.zip` 或 下载新版asio到项目根目录
```shell
|-vendor
|-CMakeLists.txt
|-asio
  |-asio
  |-asio.hpp
````
类似这样的目录结构

linux下注意目录权限

```shell
mkdir build
cd build
cmake ..
make
```

用于生产环境，可以编译为正式版，开-o3 优化同时编译为守护进程模式，方式如下加8个线程编译。

```shell
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```


### 5.hosts配置

- 查看`conf/server.conf`文件中`mainhost`值
- 替换 /Users/hzq/paozhu 为你的项目目录
- 打开host文件

```shell
sudo vim /etc/hosts
```

在hosts文件最后一行添加(如果是www.youdomain.com)

```text
127.0.0.1 www.youdomain.com
```

### 6.https测试

执行

```shell
sudo ./bin/paozhu 
```

打开浏览器，在地址栏输入`http://localhost` 或 `http://www.youdomain.com`

 

<div align="center">
<img src="https://hggq.github.io/paozhu/images/xmake_demo.jpg">
</div>

 

更多细节请查看`controller`目录下源码，包含`CRUD`示例


###  7.压力测试

使用 h2load 和 ab 测试, 你必须启用cmake编译选项`-DENABLE_BENCHMARK=ON`，即执行命令`cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_BENCHMARK=ON`，因为框架默认启用了流量限制措施。

```shell
 ab -n 120000 -c 20 http://www.xxx.com  
```

```shell
 h2load -n 12000 -c 100 -m 10 https://www.xxx.com  
```

```shell
 ./webbench -c 300 -t 59 http://www.xxx.com/cms/list  
```

###  8.1代码例子 Hello world

在项目 `controller/src` 目录里面 `testhello.cpp` 文件，建议所有业务代码入口放在 `controller/src`,因为自动提取注解到URL映射。

```c++
#include "orm.h"
#include "httppeer.h"
#include "testhello.h"
namespace http
{
//@urlpath(null,hello)
std::string testhello(std::shared_ptr<httppeer> peer)
{
  httppeer &client = peer->get_peer();
  client << " Hello world! 🧨 Paozhu c++ web framework ";
	
  auto users = orm::cms::User();
  try
  {
    users.where("name","admin").fetch_one();
    if (users.getUserid() > 0)
    {
      client<<"<p>found:"<<users.data.name<<"</p>";
      return "";
    }
  }
  catch (std::exception &e)
  {
    client << "<p>" << e.what() << "</p>";
    return "";
  }
  return "";
}

}// namespace http
```
浏览器打开 `http://localhost/hello`  

`//@urlpath(null,hello)` 为注解功能

###  8.2 代码例子 文章列表、搜索、分页

来源admin后台演示 controller/src/admin/articles.cpp

```c++
#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "articles.h"
#include "json_reflect_headers.h"
#include "array_to_tree.h"

//@urlpath(admin_islogin,admin/listarticle)
std::string admin_listarticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto topicm = orm::cms::Topic();
        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

        unsigned int topicid   = client.get["topicid"].to_int();
        unsigned int page      = client.get["page"].to_int();
        std::string searchword = client.get["searchword"].to_string();
        searchword             = mb_substr(searchword, 0, 15);
        client.val["topicid"]  = topicid;

        client.val["list"].set_array();
        obj_val temp;

        std::map<unsigned int, std::string> topickv;
        std::vector<unsigned int> topic_id_array;//articles under this topic and sub topics

        if (topicid > 0)
        {
            topic_id_array.push_back(topicid);
        }

        for (unsigned int i = 0; i < topicm.record.size(); i++)
        {
            temp["id"]       = topicm.record[i].topicid;
            temp["parentid"] = topicm.record[i].parentid;
            temp["value"]    = topicm.record[i].title;
            client.val["list"].push(temp);

            topickv[topicm.record[i].topicid] = topicm.record[i].title;
            if (topicid > 0)
            {
                for (unsigned int j = 0; j < topic_id_array.size(); j++)
                {
                    if (topicm.record[i].parentid == topic_id_array[j])
                    {
                        topic_id_array.push_back(topicm.record[i].topicid);
                        break;
                    }
                }
            }
        }

        auto artmodel = orm::cms::Article();
        artmodel.where("userid", client.session["userid"].to_int());
        if (topicid > 0)
        {
            std::string topicid_sql_str = array_to_sql(topic_id_array);
            if (topicid_sql_str.size() > 0)
            {
                artmodel.whereIn("topicid", topicid_sql_str);
            }
        }
        if (searchword.size() > 0)
        {
            artmodel.andsub().whereLike("title", str_addslash(searchword));
            artmodel.whereOrLike("content", str_addslash(searchword)).andsub();
            client.val["searchword"] = searchword;
        }
        auto [bar_min, bar_max, current_page, total_page] = artmodel.page(page, 10, 5);

        client.val["pageinfo"].set_object();
        client.val["pageinfo"]["min"]     = bar_min;
        client.val["pageinfo"]["max"]     = bar_max;
        client.val["pageinfo"]["current"] = current_page;
        client.val["pageinfo"]["total"]   = total_page;

        artmodel.select("aid,topicid,title,createtime,sortid,isopen,ishome").desc("aid").fetch();
        client.val["alist"].set_array();
        obj_val tempa;

        if (artmodel.size() > 0)
        {
            for (auto &item : artmodel)
            {
                tempa["title"]     = item.title;
                tempa["aid"]       = item.aid;
                tempa["date"]      = item.createtime.substr(0, 10);
                tempa["topicname"] = topickv[item.topicid];
                tempa["sortid"]    = item.sortid;
                tempa["isopen"]    = item.isopen;
                tempa["ishome"]    = item.ishome;
                client.val["alist"].push(tempa);
            }
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    peer->view("admin/listarticle");
    return "";
}

```

ORM 支持协程方式，目前仅支持MySQL，整个URL请求都走协程，注意注册函数也是协程函数  
来源框架自带演示文件 controller/src/techempower.cpp


```C++
//@urlpath(null,updates)
asio::awaitable<std::string> techempowerupdates(std::shared_ptr<httppeer> peer)
{
    peer->type("application/json; charset=UTF-8");
    peer->set_header("Date", get_gmttime());
    unsigned int get_num = peer->get["queries"].to_int();

    if (get_num == 0)
    {
        get_num = 1;
    }
    else if (get_num > 500)
    {
        get_num = 500;
    }
    auto myworld = orm::World();
    myworld.record.clear();
    myworld.record.reserve(get_num);
    for (unsigned int i = 0; i < get_num; i++)
    {
        myworld.wheresql.clear();
        myworld.where("id", rand_range(1, 10000));
        co_await myworld.async_fetch_append();
        if (myworld.effect() > 0)
        {
            unsigned int j                 = myworld.record.size() - 1;
            myworld.data.randomnumber      = rand_range(1, 10000);
            myworld.record[j].randomnumber = myworld.data.randomnumber;
            co_await myworld.async_update("randomnumber");
        }
    }
    peer->output = myworld.to_json();
    co_return "";
}
```

#### WebSocket Server 和 Client

实例文件在 `websockets/loopwebsockets.hpp` 服务器端演示

```C++
#include <iostream>
#include <memory>
#include <string_view>

#include "orm.h"
#include "websockets.h"
#include "terminal_color.h"

namespace http
{

class loopwebsockets : public websockets_api
{
  public:

    loopwebsockets(unsigned int m, unsigned int g) : websockets_api(8, m, g, 0) {}
    ~loopwebsockets() { std::cout << "~loopwebsockets" << std::endl; }

  public:
    void onopen() override
    { 
        isco=true;
        loop_num = 8; 
        std::cout << "onopen" << std::endl; 
    }

    asio::awaitable<void> async_onopen() override
    { 
        isco=true;
        loop_num = 8; 
        std::cout << "async_onopen" << std::endl; 
        co_return;
    }

    void onclose() override
    {
        isclose = true;
        std::cout << "onclose" << std::endl; 
    }

    asio::awaitable<void> async_onclose() override
    { 
        std::cout << "async_onclose" << std::endl; 
        co_return;
    }

    void onpong() override {}
    void run_loop() override
    {
        if (session_sock)
        {
            std::cout << "timeloop:" << std::endl;
            std::string aa = "test run_loop";
            std::string outhello;
            ws_parse->make_ws_text(aa, outhello);
            session_sock->send_writer(outhello);

            //   peer->send(aa);
            if (loop_num == 4)
            {
                loop_num = 0;
                return;
            }
            loop_num--;
        }
        else
        {
            isclose = true;
            loop_num = 0;
            std::cout << "session_sock is die!" << std::endl;
        }
    }
    asio::awaitable<void> async_run_loop() override
    {
        if (session_sock)
        {
            std::cout << "async async_run_loop" << std::endl;
            std::string aa = "test async_run_loop";
            std::string outhello;
            ws_parse->make_ws_text(aa, outhello);
            co_await session_sock->async_send_writer(outhello);
            //   peer->send(aa);
            if (loop_num == 4)
            {
                loop_num = 0;
                co_return;
            }
            loop_num--;
        }
        else
        {
            isclose = true;
            loop_num = 0;
            std::cout << "session_sock is die!" << std::endl;
        }
        co_return;
    }

    asio::awaitable<void> async_onmessage(websockets_data_list_t &&msg) override 
    {
        std::string outhello;
        ws_parse->make_ws_text(msg.value, outhello);
        co_await session_sock->async_send_writer(outhello);
        co_return;
    }
    void onmessage() override
    {
        std::unique_lock<std::mutex> lock(content_list_mutex);
        if(content_list.empty())
        {
            return;
        }
        auto msg = std::move(content_list.front());
        content_list.pop_front();
        lock.unlock();
        
        std::string outhello;
        ws_parse->make_ws_text(msg.value, outhello);
        session_sock->send_writer(outhello);
        return;
    }
 
};

}// namespace http
```

实例文件在 `controller/src/test_websocket_handle.cpp` 客户端演示  

```C++
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
    //end echo http client
    //Let the websocket client run alone in the background

    a->async_dur_time_loop_fun = [](std::shared_ptr<websocket_client> b)-> asio::awaitable<void> {
                            std::string send_content="websocket client loop";
                            std::string outdata;
                            b->make_ws_text(send_content,outdata);
                            unsigned int n = co_await b->async_write(outdata);
                            if(n == outdata.size())
                            {
                                
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
```  

#### Socket Server 和 Client

实例文件在 `controller/src/sockets/my_test_socket.hpp` Socket 服务器端演示  

```C++
#include <iostream>
#include <memory>
#include <string_view>

#include "orm.h"
#include "httppeer.h"
#include "http_socket.h"
#include "terminal_color.h"
 
namespace http
{

class my_test_socket : public socket_api
{
  public:

    my_test_socket(unsigned int m, unsigned int g) : socket_api(7, m, g, 0) {}
    ~my_test_socket() { DEBUG_LOG(" ~my_test_socket "); }

  public:
    void on_open() override { DEBUG_LOG(" onopen "); }
    void on_close() override { DEBUG_LOG(" onclose "); }
    asio::awaitable<void> async_on_message(const unsigned char *buffer, unsigned int readoffset, unsigned int readnum) override
    {
      for(; readoffset < readnum; readoffset++)
      {
          content.push_back(buffer[readoffset]);
      }
      co_await session_sock->co_send_writer(content);
      content.clear();
      co_return;
    }
    void run_loop() override
    {

    }
    asio::awaitable<void> async_run_loop() override
    {
      if(session_sock)
      {
        content="server socket loop send";
        co_await session_sock->co_send_writer(content);
        session_sock->time_limit.store(timeid());
      }
      
      co_return;
    }
};

}// namespace http

```  

实例文件在 `controller/src/test_socket_handle.cpp` Socket 客户端演示  

```C++
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
    //auto send header info "tcp [mytestsocket]0A0A", For normal connections, please use async_connect
    bool isok = co_await a->async_tcp_connect("http://127.0.0.1:80/mytestsocket",30);
    //bool isok = co_await a->async_connect("http://127.0.0.1:80/mytestsocket",30);
    // a->set_url("127.0.0.1/mytestsocket");
    // a->set_port(80);
    //bool isok = co_await a->async_connect();
    
    if(!isok)
    {
        client << " <hr> async_connect error.";
        co_return "";
    }

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


```  


### 9.相关教程

[Paozhu快速入门](https://hggq.github.io/paozhudocs/documentation.html)

 
### 10.路线图

* [ ] 1.增强HTTP特性  
* [ ] 2.Httpclient 改善
* [ ] 3.HTTP/2 客户端  
* [ ] 4.PostgresSQL、SQLite ORM 支持  

### 11.贡献👏👋

欢迎提issue互相交流，当然也欢迎大家踊跃PR

### 12.LICENSE

Paozhu遵循[MIT 开源协议](LICENSE)
