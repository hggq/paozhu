English | [简体中文](README_CN.md)
<div align="center">
<img src="https://hggq.github.io/paozhu/images/logo.svg">
</div>

Paozhu(炮竹🧨) is a comprehensive and fast C++ web framework that integrates C++ ORM. The development speed is as
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
✅ 5. Support WebSocket server  
✅ 6. The framework comes with websocket push, which supports regular push to websocket client  
✅ 7. Support httpclient get or post  
✅ 8. The framework comes with an ORM. It uses the link pool method and currently supports MySQL  
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

### 2.Runtime environment

C++ standard request c++20   
asio mysql libmysqlclient-dev zlib brotli(br) [options](gd qrencode)



### 3.environment configuration

#### 3.1 MacOS

- system requirements

```
>= BigSur
```
- Install necessary dependencies
```bash
brew install asio
brew install mysql
brew install mysql-client
brew install zlib
brew install brotli

[options] brew install gd
[options] brew install qrencode
```

#### 3.2 Ubuntu && Fedora

- system requirements

```
Ubuntu>= 20.04 , test on Fedora38, need install asan（sudo yum install libasan）
```

```
sudo apt-get install cmake
sudo apt-get install -y brotli  libbrotli-dev zlib1g-dev
sudo apt-get install libgd-dev
sudo apt-get install qrencode libqrencode-dev
sudo apt-get install openssl libssl-dev
sudo apt-get install mysql-server
sudo apt-get install mysql-common
sudo apt-get install mysql-client
sudo apt-get install libmysqlclient-dev
```

**Notice：** Requires gcc11 support. For more installation details, please see 

- Environment configuration  
[paozhu linux environment configuration](https://github.com/hggq/paozhu/wiki/linux-%E7%8E%AF%E5%A2%83%E9%85%8D%E7%BD%AE)

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

Add to the last line of the hosts file (if it is www.869869.com)

```text
127.0.0.1 www.869869.com
```

### 6.https test

run

```shell
sudo ./bin/paozhu 
```

open the browser and enter `http://localhost` or `http://www.869869.com` in the address bar


<div align="center">
<img src="https://hggq.github.io/paozhu/images/home.png">
</div>

<div align="center">
<img src="https://hggq.github.io/paozhu/images/xmake_demo.jpg">
</div>

<div align="center">
<img src="https://hggq.github.io/paozhu/images/admin.png">
</div>


for more details, please see the source code under the `controller` directory, which includes `CRUD` examples.

###  7.Stress testing

Use h2load and ab testing

![ApacheBench test](https://hggq.github.io/paozhu/images/ab_stress_test.png "ApacheBench test")  

![h2load test](https://hggq.github.io/paozhu/images/h2load_stress_test.png "h2load test")  

![h2load test](https://hggq.github.io/paozhu/images/webbench_stress.png "webbench test")  

###  8.1 Sample Hello world

On `controller` directory ,testhello.cpp file

```c++
#include "orm.h"
#include "httppeer.h"
#include "testhello.h"
namespace http
{
//@urlpath(null,hello)
std::string testhello(std::shared_ptr<httppeer> peer)
{
  httppeer &client = peer->getpeer();
  client << " Hello world! 🧨 Paozhu c++ web framework ";
	
  auto users = orm::cms::User();
  try
  {
    users.where("name","admin").limit(1).fetch();
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
    httppeer &client = peer->getpeer();
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
        OBJ_ARRAY temp;

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

        client.val["pageinfo"].set_array();
        client.val["pageinfo"]["min"]     = bar_min;
        client.val["pageinfo"]["max"]     = bar_max;
        client.val["pageinfo"]["current"] = current_page;
        client.val["pageinfo"]["total"]   = total_page;

        artmodel.select("aid,topicid,title,createtime,sortid,isopen").desc("aid").fetch();
        client.val["alist"].set_array();
        OBJ_ARRAY tempa;

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


###  9.Related tutorial

1. [paozhu Principle](https://github.com/hggq/paozhu/wiki/paozhu-cpp-web-framework-%E6%A1%86%E6%9E%B6%E5%8E%9F%E7%90%86)

2. [paozhu hello world](https://github.com/hggq/paozhu/wiki/paozhu-%E6%A1%86%E6%9E%B6hello-world)

3. [paozhu ORM](https://github.com/hggq/paozhu/wiki/paozhu-%E6%A1%86%E6%9E%B6ORM%E5%85%A5%E9%97%A8)

4. [paozhu view](https://github.com/hggq/paozhu/wiki/paozhu-%E6%A1%86%E6%9E%B6view-%E8%A7%86%E5%9B%BE%E5%85%A5%E9%97%A8)

5. [paozhu CRUD](https://github.com/hggq/paozhu/wiki/paozhu-%E6%A1%86%E6%9E%B6-CRUD-%E6%95%99%E7%A8%8B)

### 10.Roadmap

* [ ] 1.Improved http features, Full testing HTTP/2, Resist hackers scanning websites
* [ ] 2.Use C++ parse JSX file , Server Side Rendering
* [ ] 3.Httpclient Improved,Support Protocol forwarding
* [ ] 4.Websocket client  
* [x] ~~5.Support windows~~  
* [ ] 6.Postgres,sqlite ORM
* [x] ~~7.Support php-fpm  backend~~  
* [ ] 8.Parsing MySQL Protocol for ORM


### 11.Contribute👏👋

welcome to raise issues for mutual communication, and of course, we also welcome your active PR.


### 12.LICENSE

Paozhu is provided under the [MIT License](LICENSE)
