#ifndef BOOST_DLL_MY_WEBSOCKET_API_HPP
#define BOOST_DLL_MY_WEBSOCKET_API_HPP

//[plugapi
#include <string>
#include <string_view>
#include <memory>
#include <mutex>
#include "client_session.h"
#include "websockets_parse.h"

namespace http
{
struct websockets_api_data_t
{
    std::string name;
    std::string value;
}; 
struct websockets_data_list_t
{
    bool isfile;
    std::string value;
}; 
class websockets_api: public std::enable_shared_from_this<websockets_api>
{
  public:
    websockets_api(unsigned char t,unsigned int m, unsigned int g, unsigned char s) :state(s), durtime(t), myid(m), groupid(g){}
    virtual void onopen()                    = 0;
    virtual asio::awaitable<void> async_onopen() = 0;
    virtual void onmessage() = 0;
    virtual asio::awaitable<void> async_onmessage() = 0;
    virtual void onfiles()   = 0;
    virtual asio::awaitable<void> async_onfiles() = 0;
    virtual void run_loop()                  = 0;
    virtual asio::awaitable<void> async_run_loop()  = 0;
    //  virtual void timeloop(clientpeer*) = 0;
    virtual void onclose() = 0;
    //  virtual void onping() const = 0;
    virtual void onpong() = 0;
    virtual ~websockets_api() {}
    void push(websockets_data_list_t&& item) 
    {
        std::unique_lock<std::mutex> lock(mtx_list_lock);
        content_list.push_back(std::move(item));
    }
  public:
    bool isclose = false;
    bool iserror = false;
    bool isfile = false;
    bool isco = false;
    bool isloopco = false;
    unsigned char state;
    unsigned int durtime=8;
    unsigned int loop_num=1;
    unsigned int myid=0;
    unsigned int groupid=0;
    unsigned int siteid=0;
    std::string url;
    std::string host;
    std::list<websockets_data_list_t> content_list;
    std::vector<websockets_api_data_t> header;
    std::shared_ptr<client_session> session_sock = nullptr;
    std::shared_ptr<websocketparse> ws_parse = nullptr;
    std::mutex mtx_list_lock;
};
}// namespace http
#endif