Paozhu c++ http web framework
Paozhu c++ http web 框架

介绍
---------------

Paozhu(炮竹) c++ web framework 框架是一个全面、快速web开发框架，集成ORM，开发速度跟脚本语言一样，框架集成了webserver,自己原生解析http1.1、http2协议，GET、POST全部解析出来，POST几种方式都做了区分，框架自带一个OBJ微型对象，可以存放char int string float等，框架自动解析URL、POST参数到微型对象里面，使用urlpath映射到函数挂载点，使用协程、线程池、数据库链接池。


特性
--------------

1. 支持多域名网站
2. 支持多域名ssl 服务端
3. 支持http1.1、http2协议
4. 支持websocket服务端，
5. 框架自带websocket推送，支持定时推送到webscoket客户端
6. 支持同步httpclient get post
7. 框架自带ORM,使用链接池方式，目前支持mysql 
8. 框架自带线程池
9. 框架使用asio自带的协程
10. 框架特色是I/O 使用协程池 运行使用线程池
11. 框架支持普通文件gzip
12. 框架解析URL和POST,解析结果类似PHP GET POST方式获取内容
13. 集成sendmail
14. 生成二维码(qrcode),需要gd、qrencode库
---------------------

