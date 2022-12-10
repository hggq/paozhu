paozhu c++ http web framework

paozhu c++ http web 框架

介绍
---------------

paozhu(炮竹🧨) c++ web framework 框架是一个全面、快速web开发框架，集成ORM，开发速度跟脚本语言一样，框架集成了webserver,自己原生解析http1.1、http2协议，GET、POST全部解析出来，POST几种方式都做了区分，框架自带一个OBJ微型对象，可以存放char int string float等，框架自动解析URL、POST参数到微型对象里面，使用urlpath映射到函数挂载点，使用协程、线程池、数据库链接池。

可以加QQ群 668296235


特性 Features
--------------

1. support json decode encode，not use thirdparty,support json standard
2. Support multiple domain name websites
3. Support multiple domain name ssl server
4. Support http1.1 and http2 protocols
5. Support websocket server
6. The framework comes with websocket push, which supports regular push to websocket client
7. Support synchronization of httpclient get post
8. The framework comes with an ORM. It uses the link pool method and currently supports MySQL
9. Framework has two thread pool,framework thread pool,user program runing thread pool
10. The framework uses asio Coroutines
11. The framework features that I/O uses the coroutine pool to run and thread pool to run
12. The framework supports compress gzip br out files 
13. The framework parses the URL and POST. The parsing result is similar to the PHP GET POST method to obtain the content
14. Integrated sendmail
15. Generating QR code requires gd and qrencode libraries

1. 自带json decode encode 不用第三方库，标准json支持 通过 Benchmark
2. 支持多域名网站
3. 支持多域名ssl 服务端
4. 支持http1.1、http2协议
5. 支持websocket服务端，
6. 框架自带websocket推送，支持定时推送到webscoket客户端
7. 支持同步httpclient get post
8. 框架自带ORM,使用链接池方式，目前支持mysql 
9. 框架自带线程池，和用户代码运行的线程池
10. 框架使用asio自带的协程
11. 框架特色是I/O 使用协程池 运行使用线程池
12. 框架支持普通文件gzip br
13. 框架解析URL和POST,解析结果类似PHP GET POST方式获取内容
14. 集成sendmail
15. 生成二维码(qrcode),需要gd、qrencode库

---------------------


Runtime environment
--------------------------------------

C++ standard request c++20

boost asio mysql gd MySQL Connector/C++ qrencode 
zlib brotli(br)


mac os request >=big sur  
---------

brew install this soft Package

1. brew install boost
2. brew install asio
3. brew install mysql-connector-c++
4. brew install gd
5. brew install qrencode
6. brew install zlib
7. brew install brotli


ubuntu 20.04
-------------------

need gcc11 install see  ubuntu install gcc.txt


ubuntu 22.04
-------------------

install see ubuntu install gcc.txt



Install 
--------------------

mkdir temp

mkdir log

mkdir build

cd build

cmake ..

make



edit conf/server.conf file
replace your current path to

sudo vim /etc/hosts

add one line

127.0.0.1 www.869869.com 

test https

runing 

sudo ./bin/paozhu 

open browser http://localhost

![正确运行效果](https://github.com/hggq/paozhu/blob/main/www/default/firstrun.jpg)


More see controller Directory include CURD example



Roadmap
-----------------

1. Improved http features
2. websocket client
3. postgres,sqlite orm


Contribute
-----------------

Welcome to join！ You are Raise an issue Or submit a Pull Request。


LICENSE
-----------------

MIT License
