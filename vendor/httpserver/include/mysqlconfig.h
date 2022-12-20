#pragma once
#ifndef FRAME_MYSQL_CONFIG_H
#define FRAME_MYSQL_CONFIG_H
#include <string>
#include <map>
#include "request.h"
#include "client_session.h"
// #include "Websockets_api.h"

namespace http
{

    struct mysqlconnect_t{
          std::string type;  
          std::string host;  
          std::string port;  
          std::string dbname;  
          std::string user;
          std::string password;
          std::string pretable;
          std::string maxpool;
          std::string spname;
          std::string unix_socket;
    };

    //class mysqlconfig;
    //extern std::string mysql_config_path;
    //mysqlconfig &getmysqlinfo();
    void mysqlconfig_init_link();

}
#endif