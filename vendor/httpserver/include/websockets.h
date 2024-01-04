#ifndef BOOST_DLL_MY_WEBSOCKET_API_HPP
#define BOOST_DLL_MY_WEBSOCKET_API_HPP

//[plugapi
#include <string>
#include <string_view>
#include <memory>
#include "httppeer.h"
namespace http
{
class httppeer;
class websockets_api
{
  public:
    websockets_api(unsigned int t, unsigned char s, std::weak_ptr<httppeer> p) : timeloop_num(t), state(s), weak_peer(p) {}
    virtual void onopen()                    = 0;
    virtual void onmessage(std::string_view) = 0;
    virtual void onfiles(std::string_view)   = 0;
    virtual void pushloop()                  = 0;
    //  virtual void timeloop(clientpeer*) = 0;
    virtual void onclose() = 0;
    //  virtual void onping() const = 0;
    virtual void onpong() = 0;
    virtual ~websockets_api() {}
    unsigned int timeloop_num;
    unsigned char state;
    std::weak_ptr<httppeer> weak_peer;
};
}// namespace http
#endif