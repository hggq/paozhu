// Copyright 2014 Renato Tegon Forti, Antony Polukhin.
// Copyright Antony Polukhin, 2015-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <boost/make_shared.hpp>
#include <memory>
// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION

//[plugcpp_my_plugin_aggregator
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS   
#include <string_view>

#include "orm.h"
#include "httppeer.h"
#include "websockets.h"

//g++ -shared -fPIC mywebsockets.cpp -o mywebsockets.so
namespace http {

class mywebsockets : public websockets_api {
    public:
//    unsigned int timeloop_num;
//    unsigned char state;
    unsigned int outcount=0;    
    mywebsockets(std::weak_ptr<httppeer> p) : websockets_api(4,0,p){}
    ~mywebsockets() {

        std::cout<<"~mywebsockets"<<std::endl;
    }
public:
    void onopen(){
        std::cout<<"onopen"<<std::endl;
    }
     void onclose(){
        std::cout<<"onopen"<<std::endl;
    }
     void pushloop() {
       std::shared_ptr<httppeer> peer=weak_peer.lock();
         if(peer){
                std::cout<<"timeloop:"<<std::endl;
                std::string aa="777888asdafa";
                std::string outhello;
                peer->ws->makeWSText(aa.data(), aa.length(), outhello);
                peer->send(outhello);    

             //   peer->send(aa);
             if(outcount==4){
                 timeloop_num=0;
                 outcount=0;
                 return;
             }
             outcount++;
         }else{
            std::cout<<"peer is die!"<<std::endl;
         }
    }
    
     void onfiles(std::string_view filename) {
        std::cout<<"--------onfiles:--------"<<filename<<std::endl;

    }
    void onmessage(std::string_view data) {
        std::cout<<std::this_thread::get_id()<<" onmessage:"<<data<<std::endl;
        std::shared_ptr<http::httppeer> peer=weak_peer.lock();
        if(peer){
                std::string outhello;
                peer->ws->makeWSText(data, outhello);
                peer->send(outhello);   
        }

    }
    static std::shared_ptr<mywebsockets> create(std::weak_ptr<http::httppeer> p) {
        return std::make_shared<mywebsockets>(p);
    }
};


BOOST_DLL_ALIAS(
    http::mywebsockets::create, // <-- this function is exported with...
    create_plugin                               // <-- ...this alias name
)

} // namespace my_namespace
//]



