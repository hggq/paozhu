#ifndef HTTP_CONTROL_HPP
#define HTTP_CONTROL_HPP
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>
#include <string_view>
#include <memory>
#include <map> 
#include <thread> 
#include <vector>
#include <ctime>
#include <mutex>
#include <array>
#include <condition_variable>
#include <sys/time.h>
#include <sys/stat.h>
#include <mysqlx/xdevapi.h>
#include <unistd.h>
#include <cstdlib>
#include "request.h"
#include "md5.h"
#include "httppeer.h"
#include "http_so_common_api.h" 


namespace http {
   unsigned int get_controlversion(){
       return 0x01000000;
   } 

mysqlx::RowResult domysqlexecute(std::string &sql,size_t dbhash){
        return    clientapi::get().api_mysqlselect(sql,dbhash);             
}
mysqlx::SqlResult domysqleditexecute(std::string &sql,size_t dbhash){
 return clientapi::get().api_mysqledit(sql,dbhash);       
}
bool domysqlcommit(std::list<std::string> &sql,size_t dbhash){
   return  clientapi::get().api_mysqlcommit(sql,dbhash);   
 }
//  std::string api_loadview(std::string path,http::OBJ_VALUE &b){
//    return  clientapi::get().api_loadview(path,b);   
//  }
#ifndef _CONTROL_DESTROY 
void _destroy(){
     
}
#endif


BOOST_DLL_ALIAS(http::clientapi::setclientapi, _setclientapi)
#define _SHOW(A) BOOST_DLL_ALIAS(http::A,A) 
#define _SHOWS(A,B) BOOST_DLL_ALIAS(http::A,B) 
}
#endif