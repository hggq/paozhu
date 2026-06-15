English | [简体中文](README_CN.md)
<div align="center">
<img src="https://hggq.github.io/paozhu/images/logo.svg">
</div>

Paozhu(炮竹🧨) is a comprehensive and fast C++ web framework, Our slogan is "Born for rapid development of C++ Web", that integrates C++ ORM. The development speed is as
fast as script languages. The framework integrates a webserver, which can natively parse HTTP/1.1 HTTP/2 JSON WebSocket protocols
and GET and POST requests. The framework also distinguishes between different methods of POST requests. The framework
comes with a built-in OBJ micro-object that can store data such as char, int, string, float, etc. The framework
automatically parses URL and POST parameters into the micro-object, and uses URL path mapping to function mounting
points. It also uses coroutines, thread pools, and database connection pools.

QQ GROUP 668296235
 
Discussing the joy of C++ web development with community developers.

### 1.Features🔥🔥🔥🔥🔥

✅ 1. Support json decode encode, not use thirdparty, support json standard  
✅ 2. Support multiple domain name websites  
✅ 3. Support multiple domain name ssl server  
✅ 4. Support http/1.1 and http/2 protocols  
✅ 5. Supports WebSocket server and client, with duplex sending and receiving  
✅ 6. The framework comes with WebSocket push, which supports regular push to WebSocket client  
✅ 7. Support httpclient get or post  
✅ 8. Built in ORM, currently only supports MySQL, providing coroutine and connection pooling functionality  
✅ 9. Framework has two thread pool,framework thread pool,user program runing thread pool  
✅10. The framework uses asio Coroutines  
✅11. The framework features that I/O uses the coroutine pool to run and thread pool to run  
✅12. The framework supports compress gzip br out files  
✅13. URL POST Upload Files. use client.get[] client.post[] client.files[] get    
✅14. Integrated sendmail  
✅15. Generating QR code requires gd and qrencode libraries  
✅16. Plugin module,Hot Module Replacement  
✅17. The framework supports cache Module，ORM supports Result save to cache  
✅18. The controller directory cpp files annotation auto pick to url mapping  
✅19. Struct or class annotation to JSON Object, json_encode json_decode function  
✅20. Support full demo admin backend, visit url /admin/main  
✅21. Support PHP-FPM fastcgi running, which can replace Apache, allowing PHP programmers to smoothly transition to C++development mode  
✅22. Built-in WeChat Mini Program payment function  
✅23. Supports Socket server and client, supports Socket SSL connection  
✅24. Supports RPC server and client, uses HTTP registered URL functions  
✅25. Traffic limiting, with a two-stage limit based on a 60-second moving average of traffic  


### 2.Runtime environment

C++ standard request c++20   
asio openssl zlib brotli [options](gd qrencode)

Supports Linux MacOS Windows

### 3.Environment configuration

#### 3.1 MacOS

- system requirements

```
>= BigSur
```
- Install necessary dependencies
```bash
brew install asio
brew install cmake
brew install openssl
brew install zlib
brew install brotli

[options] brew install gd
[options] brew install qrencode
```

#### 3.2 Ubuntu && Fedora

- system requirements

```
Ubuntu>= 22.04 , test on Fedora38, need install asan（sudo yum install libasan）
```

```
sudo apt-get install cmake
sudo apt-get install -y brotli  libbrotli-dev zlib1g-dev
sudo apt-get install libgd-dev
sudo apt-get install qrencode libqrencode-dev
sudo apt-get install openssl libssl-dev
sudo apt-get install mysql-server
 
```

- Environment configuration  
 [MacOS environment configuration](https://hggq.github.io/paozhudocs/macos_en.html)  
 [Ubuntu environment configuration](https://hggq.github.io/paozhudocs/ubuntu_en.html)  
 [RockyLinux AlmaLinux environment configuration](https://hggq.github.io/paozhudocs/linux_en.html)  

#### 3.3 Windows

Supports install methods vcpkg and xmake

- xmake install
Execute at the root directory of the project  
Config `conf/server.conf` replace /Users/hzq/paozhu path to your project path  
```
paozhu-main> xmake
paozhu-main> .\build\windows\x64\release\paozhu.exe
```

- vcpkg install

```
Invoke-WebRequest -OutFile vcpkg2023.zip https://github.com/microsoft/vcpkg/archive/refs/tags/2023.12.12.zip
            unzip vcpkg2023.zip
            cd vcpkg-2023.12.12
            bootstrap-vcpkg.bat
            vcpkg integrate install
            vcpkg integrate project
            vcpkg install
```

Install ninja https://github.com/ninja-build/ninja/releases

Compile project  `CMAKE_TOOLCHAIN_FILE` is vcpkg install localtion path

```
unzip asio.zip
cmake . -B ${{github.workspace}} -DENABLE_WIN_VCPKG=ON -DENABLE_VCPKG=ON -DCMAKE_TOOLCHAIN_FILE=vcpkg-2023.12.12/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE=Debug -G Ninja
cmake --build . --config Debug
```



### 4.Install

Project root `unzip asio.zip` or Download asio new version to project root part 
```shell
|-vendor
|-CMakeLists.txt
|-asio
  |-asio
  |-asio.hpp
````
Directory like this


```shell
mkdir build
cd build
cmake ..
make
```

Use in production environment,Compile as daemon 
```shell
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```


### 5.hosts configuration

- view`conf/server.conf`file and get`mainhost`value
- replace /Users/hzq/paozhu path to your project path
- open host file

```shell
sudo vim /etc/hosts
```

Add to the last line of the hosts file (if it is www.youdomain.com)

```text
127.0.0.1 www.youdomain.com
```

### 6.https test

run

```shell
sudo ./bin/paozhu 
```

open the browser and enter `http://localhost` or `http://www.youdomain.com` in the address bar


<div align="center">
<img src="https://hggq.github.io/paozhu/images/xmake_demo.jpg">
</div>

 


for more details, please see the source code under the `controller` directory, which includes `CRUD` examples.

###  7.Stress testing

Use h2load and ab testing, you must enable the cmake compilation option `-DENABLE_BENCHMARK=ON`, `cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_BENCHMARK=ON`, Because the framework has enabled traffic limiting measures by default.  

```shell
 ab -n 120000 -c 20 http://www.xxx.com  
```

```shell
 h2load -n 12000 -c 100 -m 10 https://www.xxx.com  
```

```shell
 ./webbench -c 300 -t 59 http://www.xxx.com/cms/list  
```

###  8.1 Sample Hello world

In the `controller/src` directory of the project `testhello.cpp` file, It is recommended to place all business code entries in `controller/src`, as annotations will be automatically extracted for URL mapping.

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
Open the browser and enter `http://localhost/hello`  

`//@urlpath(null,hello)` is annotation  


###  8.2 Sample Article list,search,pagebar

From admin demo file controller/src/admin/articles.cpp

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

C++ ORM Coroutines,Only supports MySQL,URL request is completed throughout the entire coroutine function  
From admin demo file controller/src/techempower.cpp

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

#### WebSocket Server and Client

The example file is demonstrated on the server side in `websockets/loopwebsockets.hpp`  

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

The example file is demonstrated in the client at `controller/src/test_websocket_handle.cpp`  

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

#### Socket Server and Client

The instance file is demonstrated on the server side of the Socket in `controller/src/sockets/my_test_socket.hpp`  

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

The instance file is in the `controller/src/test_socket_handle.cpp` for the Socket client demonstration

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



###  9.Related tutorial

- [paozhu docs](https://hggq.github.io/paozhudocs/documentation_en.html)


### 10.Roadmap

* [ ] 1.Improved HTTP features  
* [ ] 2.HttpClient Improved
* [ ] 3.HTTP/2 client  
* [ ] 4.PostgresSQL,SQLite ORM  



### 11.Contribute👏👋

welcome to raise issues for mutual communication, and of course, we also welcome your active PR.


### 12.LICENSE

Paozhu is provided under the [MIT License](LICENSE)
