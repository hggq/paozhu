#include<iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <map> 
#include <vector>
#include <ctime>
#include <array>
#include <sys/stat.h>
#include <cstdlib>
#include "request.h"
#include "datetime.h"
#include "cookie.h"
#include "urlcode.h"
#include "loadviewso.h"
#include "viewso_param.h"
#include "http_so_common_api.h"
#include "viewsrc.h"
//This file create by paozhu Wed, 21 May 2025 02:24:02 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string martopic([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>栏目管理</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>  \n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">栏目管理</h5>\n    <hr>\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <table class=\"table table-bordered\">\n          <thead>\n            <tr>\n              <th scope=\"col\">#</th>\n              <th scope=\"col\">名称</th>\n              <th scope=\"col\">url</th>\n              <th scope=\"col\">类型</th>\n              <th scope=\"col\">排序</th>\n              <th scope=\"col\">显示</th>\n              <th scope=\"col\">编辑</th>\n            </tr>\n          </thead>\n          <tbody>\n            ";
 for(auto &a:obj["list"].as_array()){ 
 			 echo<<"              <tr id=\"topicid_";
 echo<<a["id"].to_string(); 
 			 echo<<"\">\n                <td>\n                  ";
 echo<<a["id"].to_string(); 
 			 echo<<"                </td>\n                <td ";
 if(a["parentid"].to_int()>0){ 
 			 echo<<"style=\"padding-left:";
 echo<<(a["level"].to_int()*15+15); 
 			 echo<<"px\"";
 } 
 			 echo<<">\n                  ";
 echo<<a["value"].as_string(); 
 			 echo<<"                </td>\n                <td>\n                  ";
 echo<<a["urlpath"].as_string(); 
 			 echo<<"                </td>\n                <td>\n                  ";
 
                      switch(a["cateid"].to_int())
                      {
                            case 0:
                                 echo<<"图文内容";
                                 break;
                            case 1:
                                 echo<<"单页图文";
                                 break;
                            case 2:
                                 echo<<"多页图文";
                                 break;
                            case 3:
                                 echo<<"单页倒叙";
                                 break;   
                            case 4:
                                 echo<<"文档目录";
                                 break;
                            case 5:
                                 echo<<"新闻列表";
                                 break;       
                            case 7:
                                 echo<<"产品展示";
                                 break;     
                            case 9:
                                  echo<<"产品栏目";
                                  break;  
                            case 10:
                                  echo<<"产品列表";
                                  break;       
                            case 13:
                                 echo<<"博客类型";
                                 break;   
                            case 24:
                                 echo<<"品牌分类";
                                 break;  
                            case 31:
                                 echo<<"文章首页图片1";
                                 break;  
                            case 32:
                                 echo<<"文章首页图片2";
                                 break;      
                            case 33:
                                 echo<<"文章单页图片3";
                                 break;   
                            case 34:
                                 echo<<"文章列表无日期1";
                                 break;  
                            case 35:
                                 echo<<"文章单页图片4";            
                            case 100:
                                 echo<<"网站首页";                               
                                 break;                
                      }
                  
 			 echo<<"                </td>\n                <td>\n                  <input type=\"text\" class=\"form-inline topicsort col-md-3\" id=\"title_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" name=\"title_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" value=\"";
 echo<<a["sortid"].to_string(); 
 			 echo<<"\" onchange=\"updatetopic(";
 echo<<a["id"].to_string(); 
 			 echo<<",this.value)\">\n                </td>\n                <td><input class=\"form-check-input\" type=\"checkbox\" value=\"";
 echo<<a["id"].to_string(); 
 			 echo<<"\" id=\"art_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" onclick=\"artupdateview(this)\" ";
 if(a["isview"].to_int()==1){ 
 			 echo<<"checked";
 } 
 			 echo<<" >\n                </td>\n                <td>\n                  <a href=\"/admin/edittopic?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\">编辑</a>｜\n                  <a href=\"/admin/deletetopic?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\" onclick=\"return deletetopic(";
 echo<<a["id"].to_string(); 
 			 echo<<")\">删除</a>\n                </td>\n              </tr>\n              ";
 } 
 			 echo<<"          </tbody>\n\n        </table>\n      </div>\n    </div>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  \n  <script lang=\"javascript\">\n  \n    function deletetopic(id)\n    {\n      if(confirm('是否删除?'))\n      {          \n          $.getJSON(\"/admin/deletetopic?id=\"+id,function(result){\n                if(result.code==0)\n                {\n                \n                     $(\"#topicid_\"+id).remove();\n                }\n                \n          });\n\n      }\n      return false;\n    }\n      function updatetopic(id,sortid)\n    {\n       $.getJSON(\"/admin/updatetopicsort?id=\"+id+\"&sortid=\"+sortid,function(result){\n                               \n       });\n    }\n    function artupdateview(obj)\n    {\n      let isview=0;\n      if(obj.checked)\n        {\n           isview=1;\n        }\n        $.post(\"/admin/updatetopicview?id=\"+obj.value,{isview:isview},function(result){\n                               \n       });\n    }\n  </script>\n</body>\n\n</html>\n";

                    }
                    catch(const char *e)
                    {
                        echo << e;
                        return echo.str();
                    }
                  return echo.str();
             }

         }
        }
       }
    