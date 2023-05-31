#include<iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <map> 
#include <vector>
#include <ctime>
#include <array>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include "request.h"
#include "datetime.h"
#include "cookie.h"
#include "urlcode.h"
#include "loadviewso.h"
#include "viewso_param.h"
#include "http_so_common_api.h"
#include "viewsrc.h"
//g++ viewsrc/view/home/header.cpp
namespace http {

namespace view {
	namespace home{
 		 std::string header(const struct view_param &vinfo,http::OBJ_VALUE &obj)
			{
 
                     if(vinfo.viewsopath.empty()&&obj.is_array())
                     {

                     }
                     std::ostringstream echo;

        
 			 echo<<"<style>\n  ul {\n      list-style-type: none;\n      margin: 0;\n      padding: 0;\n      overflow: hidden;\n      background-color: #333;\n  }\n  \n  li {\n      float: left;\n      border-right:1px solid #bbb;\n  }\n  \n  li:last-child {\n      border-right: none;\n  }\n  \n  li a {\n      display: block;\n      color: white;\n      text-align: center;\n      padding: 14px 16px;\n      text-decoration: none;\n  }\n  \n  li a:hover:not(.active) {\n      background-color: #111;\n  }\n  \n  .active {\n      background-color: #4CAF50;\n  }\n  </style>\n<ul>\n  <li><a class=\"active\" href=\"/\">主页HOME</a></li>\n  <li><a href=\"#news\">新闻NEW</a></li>\n  <li><a href=\"#contact\">联系Contact</a></li>\n  <li style=\"float:right\"><a href=\"#about\">关于About</a></li>\n</ul>\n\n<h3>子页面 Sub page include_sub(\"home/header\")导航测试 Nav Test,test url get[\"aa\"] |";
 echo<<vinfo.get["aa"].to_string(); 
 			 echo<<"| </h3>\n     \n ";

                  return echo.str();
             }

         }
        }
       }
    