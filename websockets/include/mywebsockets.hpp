#include <iostream>
#include <memory>

#ifdef ENABLE_BOOST
#include <boost/make_shared.hpp>

// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION

//[plugcpp_my_plugin_aggregator
#include <boost/dll/alias.hpp>// for BOOST_DLL_ALIAS
#endif

#include <string_view>

#include "orm.h"
#include "httppeer.h"
#include "websockets.h"
#include "terminal_color.h"
// g++ -shared -fPIC mywebsockets.cpp -o mywebsockets.so
namespace http
{

class mywebsockets : public websockets_api
{
  public:
    //    unsigned int timeloop_num;
    //    unsigned char state;
    unsigned int outcount = 0;
    mywebsockets(std::weak_ptr<httppeer> p) : websockets_api(4, 0, p) {}
    ~mywebsockets() { DEBUG_LOG(" ~mywebsockets "); }

  public:
    void onopen() { DEBUG_LOG(" onopen "); }
    void onclose() { DEBUG_LOG(" onclose "); }
    void pushloop()
    {
        std::shared_ptr<httppeer> peer = weak_peer.lock();
        if (peer)
        {
            DEBUG_LOG(" timeloop ");
            std::string aa = "777888asdafa";
            std::string outhello;
            peer->ws->makeWSText(aa.data(), aa.length(), outhello);
            peer->send(outhello);

            //   peer->send(aa);
            if (outcount == 4)
            {
                timeloop_num = 0;
                outcount     = 0;
                return;
            }
            outcount++;
        }
        else
        {
            DEBUG_LOG(" peer is die! ");
        }
    }

    void onfiles(std::string_view filename) { DEBUG_LOG("onfiles"); }
    void onmessage(std::string_view data)
    {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        oss << " onmessage:" << data << std::endl;
        std::string temp = oss.str();
        DEBUG_LOG("%s", temp.c_str());
        std::shared_ptr<http::httppeer> peer = weak_peer.lock();
        if (peer)
        {
            std::string outhello;
            peer->ws->makeWSText(data, outhello);
            peer->send(outhello);
        }
    }
    static std::shared_ptr<mywebsockets> create(std::weak_ptr<http::httppeer> p)
    {
        return std::make_shared<mywebsockets>(p);
    }
};

#ifdef ENABLE_BOOST
BOOST_DLL_ALIAS(http::mywebsockets::create,// <-- this function is exported with...
                create_plugin              // <-- ...this alias name
)
#endif
}// namespace http
//]
