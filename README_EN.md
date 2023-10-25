English | [简体中文](README_CN.md)
<div align="center">
<img src="https://www.paozhu.org/images/logo.svg">
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
asio mysql libmysqlclient-dev zlib brotli(br) [options](boost gd qrencode)



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

[options] brew install boost
[options] brew install gd
[options] brew install qrencode
```

#### 3.2 Ubuntu && Fedora

- system requirements

```
Ubuntu>= 20.04 , test on Fedora38, need install asan（sudo yum install libasan）
```
**Notice：** Requires gcc11 support. For more installation details, please see 

- Environment configuration  
[paozhu linux environment configuration](https://github.com/hggq/paozhu/wiki/linux-%E7%8E%AF%E5%A2%83%E9%85%8D%E7%BD%AE)

### 4.Install

Download asio new version to project root part 
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
<img src="https://www.paozhu.org/images/home.png">
</div>


<div align="center">
<img src="https://www.paozhu.org/images/admin.png">
</div>


for more details, please see the source code under the `controller` directory, which includes `CRUD` examples.

###  7.Stress testing

Use h2load and ab testing

![ApacheBench test](https://www.paozhu.org/images/ab_stress_test.png "ApacheBench test")  

![h2load test](https://www.paozhu.org/images/h2load_stress_test.png "h2load test")  


###  8.Sample Hello world

On `controller` directory ,testhello.cpp file

```c++
#include "httppeer.h"
#include "testhello.h"
namespace http
{
//@urlpath(null,hello)
std::string testhello(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << " Hello world! 🧨 Paozhu c++ web framework ";

    return "";
}

}// namespace http
```
Open the browser and enter `http://localhost/hello`  

`//@urlpath(null,hello)` is annotation  


###  9.Related tutorial

1. [paozhu Principle](https://github.com/hggq/paozhu/wiki/paozhu-cpp-web-framework-%E6%A1%86%E6%9E%B6%E5%8E%9F%E7%90%86)

2. [paozhu hello world](https://github.com/hggq/paozhu/wiki/paozhu-%E6%A1%86%E6%9E%B6hello-world)

3. [paozhu ORM](https://github.com/hggq/paozhu/wiki/paozhu-%E6%A1%86%E6%9E%B6ORM%E5%85%A5%E9%97%A8)

4. [paozhu view](https://github.com/hggq/paozhu/wiki/paozhu-%E6%A1%86%E6%9E%B6view-%E8%A7%86%E5%9B%BE%E5%85%A5%E9%97%A8)

5. [paozhu CRUD](https://github.com/hggq/paozhu/wiki/paozhu-%E6%A1%86%E6%9E%B6-CRUD-%E6%95%99%E7%A8%8B)

### 10.Roadmap

* [x] 1.Improved http features, Full testing HTTP/2, Resist hackers scanning websites
* [x] 2.Use C++ parse JSX file , Server Side Rendering
* [x] 3.Httpclient Improved,Support Protocol forwarding
* [x] 4.Websocket client  
* [x] 5.Support windows  
* [x] 6.Postgres,sqlite ORM
* [x] 7.Support php-fpm  backend 
* [x] 8.Parsing MySQL Protocol for ORM


### 11.Contribute👏👋

welcome to raise issues for mutual communication, and of course, we also welcome your active PR.


### 12.LICENSE

Paozhu is provided under the [MIT License](LICENSE)
