#include <chrono>
#include <thread>
#include "httppeer.h"
#include "request.h"
#include "testhello.h"
namespace http
{
//@urlpath(null,testjson)
std::string testjson(std::shared_ptr<httppeer> peer)
{
    httppeer &client     = peer->get_peer();
    client.val["msg"]    = "insert 3 row";
    client.val["status"] = "ok,insert";
    client.val["num"]    = 1234;
    unsigned int isize= sizeof(obj_val);
    client.val["size"]    = isize;
    client.out_json();
    
    return "";
}

//@urlpath(null,test_requst)
std::string test_requst(std::shared_ptr<httppeer> peer)
{
    httppeer &client     = peer->get_peer();
    
    
    std::string a=R"(
    {
        "user": {
            "profile": {
            "tags": ["developer", "admin", "active"]
            }
        }
    }
    )";
    obj_val aa;
    aa.from_json(a);
    client<<"<pre>";
    client<< aa.to_json();
    client<<"</pre><hr><pre>";

    std::string b=R"(
    {
        "servers": [
            {
            "host": "192.168.1.10",
            "port": 8080
            }
        ]
        }
    )";
    obj_val bb;
    bb.from_json(b);
    client<<"</pre><hr><pre>";
    client<< bb.to_json();

    std::string c=R"(
        [
        [
            [1, 2, 3]
        ]
        ]
    )";
    obj_val cc;
    cc.from_json(c);
    client<<"</pre><hr><pre>";
    client<< cc.to_json();

    std::string d=R"(
{
  "metadata": {
    "version": "2.4.0-beta",
    "generated_at": "2026-06-12T15:27:00+08:00",
    "flags": [true, false, null, 0, "", [], {}]
  },
  "users": [
    {
      "id": 10086,
      "name": "张三 \u5f20\u4e09",
      "bio": "Line1\nLine2\tTabbed \"quoted\" \\backslash",
      "scores": [99.5, -0.001, 1.23e+4, 0.0],
      "address": {
        "city": "Beijing",
        "coordinates": {
          "lat": 39.9042,
          "lng": 116.4074,
          "altitude": null
        }
      }
    },
    {
      "id": -1,
      "name": "",
      "bio": null,
      "scores": [],
      "address": {
        "city": "\ud83c\udf89 Emoji City",
        "coordinates": {
          "lat": 0,
          "lng": 0,
          "altitude": -9999.99
        }
      }
    }
  ],
  "matrix": [
    [[1, 2], [3, 4]],
    [[5, 6], [7, 8]],
    []
  ],
  "escape_stress": "\b\f\r\n\t\/\\\"",
  "large_number": 9007199254740993,
  "empty_object": {},
  "empty_array": []
}
    )";
    obj_val dd;
    dd.from_json(d);
    client<<"</pre><hr><pre>";
    client<< dd.to_json();
    client<<"</pre>";
    return "";
}

}// namespace http