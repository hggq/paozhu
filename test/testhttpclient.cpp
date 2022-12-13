#include <string.h>
#include <sstream>

#include <cstdio>
#include <string_view>
#include "zlib.h"
#include "gzip.h"
#include "urlcode.h"
#include "request.h"
#include "httpclient.h"

/*
g++ test/testhttpclient.cpp -std=c++20  -Ivendor/httpserver/include vendor/httpserver/src/request.cpp vendor/httpserver/src/httpclient.cpp -lssl -lcrypto -ldl -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lz vendor/httpserver/src/gzip.cpp vendor/httpserver/src/urlcode.cpp vendor/httpserver/src/cookie.cpp vendor/httpserver/src/md5.cpp vendor/httpserver/src/datetime.cpp vendor/httpserver/src/mime.cpp

*/
int main()
{

  http::client a;
  http::OBJ_VALUE parameter;
  parameter["name"] = "老师短发";
  std::cout << "asio version:" << ASIO_VERSION << std::endl;
  std::cout << "name:" << parameter["name"].size() << std::endl;

  parameter["num"] = 345;

  a.getjson("http://www.869869.com/testpost.html", parameter);
  a.timeout(10);
  a.send();
  if (a.state.code == 200)
  {
    std::cout << "get ok" << std::endl;
  }
  a.save("./temp/");
  std::cout << "========" << a.state.code << " " << a.state.codemessage << std::endl;
  std::cout << "==============content==============\r\n"
            << a.state.content << std::endl;

  for (auto &bb : a.state.cookie)
  {
    std::cout << bb.first << ":" << bb.second << std::endl;
  }
}