English | [简体中文](README_CN.md)
<div align="center">
<img src="https://hggq.github.io/paozhu/images/logo.svg">
</div>

**Paozhu** (炮竹 🧨) is a full-stack rapid development framework for the Web. Our slogan is _"Born for rapid C++ web development."_ The framework integrates a C++ ORM, delivering development speed comparable to scripting languages. It ships with a built-in web server that natively parses HTTP/1.1, HTTP/2, JSON, WebSocket protocols, and handles both GET and POST requests — with distinct handling for different POST content types. A lightweight built-in `obj_val` micro-object can store data of type `char`, `int`, `string`, `float`, and more. URL and POST parameters are automatically parsed into this micro-object, and URL path mapping is used to mount handler functions. Coroutines, thread pools, and database connection pools are all built in.

**QQ Group:** 668296235

Join community developers to discuss the joy of C++ web development.

### 1. Features 🔥🔥🔥🔥🔥

✅ 1. JSON encoding/decoding with zero third-party dependencies, fully compliant with JSON standards  
✅ 2. Multi-domain website support, with SaaS tenancy and theme customization  
✅ 3. Multi-domain SSL server  
✅ 4. HTTP/1.1 and HTTP/2 protocol support  
✅ 5. WebSocket server and client with full-duplex communication  
✅ 6. Built-in WebSocket push, supporting periodic pushes to connected clients  
✅ 7. HTTP client GET/POST requests in both synchronous and asynchronous modes  
✅ 8. Built-in ORM supporting MySQL, PostgreSQL, SQLite, and MariaDB, with coroutine and synchronous modes  
✅ 9. Two separate thread pools: a framework pool and a user-program pool  
✅ 10. Built on Asio coroutines  
✅ 11. I/O operations run across both coroutine pools and thread pools  
✅ 12. Gzip and Brotli compression for output files  
✅ 13. URL-based POST file uploads, accessed via `client.get[]`, `client.post[]`, and `client.files[]`  
✅ 14. Integrated SendMail library with synchronous and asynchronous modes  
✅ 15. QR code, CAPTCHA, and SVG chart generation  
✅ 16. Plugin module with Hot Module Replacement (HMR)  
✅ 17. Cache module support; ORM query results can be stored in cache  
✅ 18. Annotations in controller `.cpp` files are automatically extracted for URL mapping  
✅ 19. Struct/class annotation for JSON serialization via `json_encode` / `json_decode`  
✅ 20. Complete admin backend demo available at `/admin/main`  
✅ 21. PHP-FPM FastCGI support, replacing Apache to help PHP developers transition to C++  
✅ 22. Built-in WeChat Mini Program payment  
✅ 23. Socket server and client with SSL support — ideal for IoT development  
✅ 24. RPC server and client using HTTP-annotated URL functions  
✅ 25. Two-stage rate limiting based on a 60-second moving average  
✅ 26. ACME protocol integration for automatic SSL certificate renewal and OCSP stapling  
✅ 27. Integrated ZIP archive packaging and extraction  
✅ 28. Excel (`.xlsx`) read/write, and Word (`.docx`) ↔ HTML conversion  
✅ 29. Built-in image generation and processing (JPG, PNG)  
✅ 30. Built-in HTML-to-PDF module (`webpdf`)


### 2. Runtime environment

- **C++ standard:** C++20
- **Dependencies:** Asio, OpenSSL, Zlib, Brotli
- **Supported platforms:** Linux, macOS, Windows


### 3. Environment setup

#### 3.1 macOS

- **System requirements:** Sequoia or later

- Install dependencies:
```bash
brew install asio
brew install cmake
brew install openssl
brew install zlib
brew install brotli
```

#### 3.2 Ubuntu & Fedora

- **System requirements:** Ubuntu ≥ 22.04; tested on Fedora 38 (install ASAN via `sudo yum install libasan`)

```bash
sudo apt-get install cmake
sudo apt-get install -y brotli libbrotli-dev zlib1g-dev
sudo apt-get install openssl libssl-dev
sudo apt-get install mysql-server
```

- Full environment guides:
  - [macOS environment setup](https://hggq.github.io/paozhudocs/macos_en.html)
  - [Ubuntu environment setup](https://hggq.github.io/paozhudocs/ubuntu_en.html)
  - [RockyLinux / AlmaLinux environment setup](https://hggq.github.io/paozhudocs/linux_en.html)

#### 3.3 Windows

Supports installation via **vcpkg** or **xmake**.

- **xmake installation —** run from the project root:
  1. Edit `conf/server.conf` and replace `/Users/hzq/paozhu` with your project path
  2. Build:

```bat
paozhu-main> xmake
paozhu-main> .\build\windows\x64\release\paozhu.exe
```

- **vcpkg installation:**

```powershell
Invoke-WebRequest -OutFile vcpkg2023.zip https://github.com/microsoft/vcpkg/archive/refs/tags/2023.12.12.zip
unzip vcpkg2023.zip
cd vcpkg-2023.12.12
bootstrap-vcpkg.bat
vcpkg integrate install
vcpkg integrate project
vcpkg install
```

Install Ninja from https://github.com/ninja-build/ninja/releases, then build: set `CMAKE_TOOLCHAIN_FILE` to your vcpkg installation path.

```bat
unzip asio.zip
cmake . -B ${{github.workspace}} -DENABLE_WIN_VCPKG=ON -DENABLE_VCPKG=ON ^
    -DCMAKE_TOOLCHAIN_FILE=vcpkg-2023.12.12/scripts/buildsystems/vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE=Debug -G Ninja
cmake --build . --config Debug
```


### 4. Installation

Unzip `asio.zip` into the project root, or download the latest Asio release and place it there. Your directory should look like:

```
|-vendor
|-CMakeLists.txt
|-asio
  |-asio
  |-asio.hpp
```

Then build:

```bash
mkdir build
cd build
cmake ..
make
```

For production, build as a daemon (Release mode):

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```


### 5. Hosts configuration

1. Open `conf/server.conf` and note the `mainhost` value
2. Replace any hardcoded `/Users/hzq/paozhu` paths with your own project path
3. Edit your hosts file:

```bash
sudo vim /etc/hosts
```

Append the following line (using your actual domain, e.g., `www.yourdomain.com`):

```text
127.0.0.1 www.yourdomain.com
```


### 6. HTTPS test

Run:

```bash
sudo ./bin/paozhu
```

Open a browser and navigate to `http://localhost` or `http://www.yourdomain.com`.


<div align="center">
<img src="https://hggq.github.io/paozhu/images/xmake_demo.jpg">
</div>


For more examples, see the source code in the `controller` directory, which includes full CRUD operations.


### 7. Stress testing

When using `h2load` or `ab`, enable `-DENABLE_BENCHMARK=ON` during CMake configuration:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_BENCHMARK=ON
```

The framework has rate limiting enabled by default, which must be disabled for accurate benchmarking.

```bash
ab -n 120000 -c 20 http://www.xxx.com
```

```bash
h2load -n 12000 -c 100 -m 10 https://www.xxx.com
```

```bash
./webbench -c 300 -t 59 http://www.xxx.com/cms/list
```


### 8.1 Hello World example

Example file: `controller/src/testhello.cpp`. Place all business logic entries under `controller/src` — annotations are auto-extracted for URL mapping, and the ORM auto-generates C++ entity code from the database, so no manual coding is needed for model definitions.

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
  client << " Hello world! 🧨 Paozhu C++ web framework ";

  auto users = orm::cms::User();
  try
  {
    users.where("name","admin").fetch_one();
    if (users.getUserid() > 0)
    {
      // users.getName() or users.data.name
      client << "<p>found:" << users.data.name << "</p>";
    }
  }
  catch (std::exception &e)
  {
    client << "<p>" << e.what() << "</p>";
  }
  return "";
}

}// namespace http
```

Open a browser and navigate to `http://localhost/hello`.

`//@urlpath(null,hello)` is an annotation directive — `testhello.h` is auto-generated, so you don't need to create it manually.


### 8.2 Article list, search, and pagination example

From the admin demo: `controller/src/admin/articles.cpp`

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
        // articles under this topic and its sub-topics
        std::vector<unsigned int> topic_id_array;

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

            // save topic name to topickv map
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
            // sql: and topicid in(xxx,xxx,xxx)
            std::string topicid_sql_str = array_to_sql(topic_id_array);
            if (topicid_sql_str.size() > 0)
            {
                artmodel.whereIn("topicid", topicid_sql_str);
            }
        }
        if (searchword.size() > 0)
        {
            // sql: and (title like '%searchword%' or content like '%searchword%')
            artmodel.andsub().whereLike("title", str_addslash(searchword));
            artmodel.whereOrLike("content", str_addslash(searchword)).endsub();
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
    // client.val is rendered to HTML
    peer->view("admin/listarticle");
    return "";
}
```

**C++ ORM with coroutines** — the entire URL request is processed within a single coroutine function. From the admin demo: `controller/src/techempower.cpp`

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
        // Note: using co_await myworld.async_fetch() will NOT append additional data
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

#### WebSocket server and client

Server-side example: `websockets/loopwebsockets.hpp`

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

Client-side example: `controller/src/test_websocket_handle.cpp`

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

#### Socket server and client

Server-side example: `controller/src/sockets/my_test_socket.hpp`

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

Client-side example: `controller/src/test_socket_handle.cpp`

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



### 9. Documentation

- [Paozhu Documentation](https://hggq.github.io/paozhudocs/)


### 10. Roadmap

* [ ] Improved HTTP features  
* [ ] Enhanced HTTP client  
* [ ] HTTP/2 client  


### 11. Contributing 👏👋

Feel free to open issues for discussions, and pull requests are always welcome!


### 12. License

Paozhu is licensed under the [MIT License](LICENSE).
