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
//g++ viewsrc/view/admin/listarticle.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string listarticle([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>文章管理</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n \n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">文章管理</h5>\n    <hr>\n    <div class=\"row p-3\">\n    <div class=\"col-auto\">\n    <label for=\"parentid\" class=\"col-form-label\">所属栏目</label>\n    </div>\n    <div class=\"col-5\">\n    \n        \n        <select class=\"form-select\" id=\"parentid\" name=\"parentid\">\n        </select>\n   \n    </div>\n    <div class=\"col-5\">\n    <div class=\"input-group\">\n    <input type=\"text\" class=\"form-control\" name=\"searchkeyword\" id=\"searchkeyword\" value=\"";
 echo<<obj["searchword"].to_string(); 
 			 echo<<"\" placeholder=\"\">\n      <button class=\"btn btn-outline-secondary\" type=\"button\" id=\"button-addon2\" onclick=\"searcharticle()\">搜索</button>\n    </div> \n    </div>  \n    </div>  \n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <table class=\"table table-bordered\">\n          <thead>\n            <tr>\n              <th scope=\"col\">#</th>\n              <th scope=\"col\">栏目</th>\n              <th scope=\"col\">标题</th>\n              <th scope=\"col\">日期</th>\n              <th scope=\"col\">排序</th>\n              <th scope=\"col\">显示</th>\n              <th scope=\"col\">首页</th>\n              <th scope=\"col\">操作</th>\n            </tr>\n          </thead>\n          <tbody>\n            ";
 for(auto &a:obj["alist"].as_array()){ 
 			 echo<<"            <tr id=\"article_";
 echo<<a["aid"].to_string(); 
 			 echo<<"\">\n              <td>";
 echo<<a["aid"].to_string(); 
 			 echo<<"</td>\n              <td>";
 echo<<a["topicname"].to_string(); 
 			 echo<<"</td>\n              <td>";
 echo<<a["title"].to_string(); 
 			 echo<<"</td>\n              <td>";
 echo<<a["date"].to_string(); 
 			 echo<<"</td>\n              <td>\n              <input type=\"text\" class=\"form-inline topicsort col-md-2\" id=\"title_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" name=\"title_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" value=\"";
 echo<<a["sortid"].to_string(); 
 			 echo<<"\" onchange=\"updatearticlesort(";
 echo<<a["aid"].to_string(); 
 			 echo<<",this.value)\">\n              </td>\n              <td><input class=\"form-check-input\" type=\"checkbox\" value=\"";
 echo<<a["aid"].to_string(); 
 			 echo<<"\" id=\"art_";
 echo<<a["aid"].to_string(); 
 			 echo<<"\" onclick=\"artupdateview(this)\" ";
 if(a["isopen"].to_int()==1){ 
 			 echo<<"checked";
 } 
 			 echo<<" >\n</td>\n              <td><input class=\"form-check-input\" type=\"checkbox\" value=\"";
 echo<<a["aid"].to_string(); 
 			 echo<<"\" id=\"ishome_";
 echo<<a["aid"].to_string(); 
 			 echo<<"\" onclick=\"artupdateishome(this)\" ";
 if(a["ishome"].to_int()==1){ 
 			 echo<<"checked";
 } 
 			 echo<<" >\n</td>\n              <td>\n              <a href=\"/admin/editarticle?id=";
 echo<<a["aid"].to_string(); 
 			 echo<<"\">编辑</a>｜\n                  <a href=\"/admin/deletearticle?id=";
 echo<<a["aid"].to_string(); 
 			 echo<<"\" onclick=\"return deletearticle(";
 echo<<a["aid"].to_string(); 
 			 echo<<")\">删除</a></td>\n            </tr>\n            ";
 } 
 			 echo<<"\n          </tbody>\n\n        </table>\n      </div>\n    </div>\n\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <nav aria-label=\"Page navigation example\">\n          <ul class=\"pagination\">\n          ";
 if(obj["pageinfo"]["min"].to_int()>1){ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/listarticle?topicid=";
echo<<obj["topicid"].to_int(); 
 			 echo<<"&page=";
echo<<obj["pageinfo"]["current"].to_int(); 
 			 echo<<"\">1</a></li>\n          ";
 } 
 			 echo<<"          ";
 if(obj["pageinfo"]["current"].to_int()>1){ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/listarticle?topicid=";
echo<<obj["topicid"].to_int(); 
 			 echo<<"&page=";
echo<<(obj["pageinfo"]["current"].to_int()-1); 
 			 echo<<"\">上一页</a></li>\n          ";
 } 
 			 echo<<"            \n            ";
 unsigned int maxpage=obj["pageinfo"]["max"].to_int(); 
 			 echo<<"            ";
 unsigned int currentpage=obj["pageinfo"]["current"].to_int(); 
 			 echo<<"            ";
 for(unsigned int i=obj["pageinfo"]["min"].to_int();i<=maxpage;i++){ 
 			 echo<<"            ";
 if(i==currentpage){ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link active\" href=\"/admin/listarticle?topicid=";
echo<<obj["topicid"].to_int(); 
 			 echo<<"&page=";
echo<<std::to_string(i); 
 			 echo<<"\">";
 echo<<std::to_string(i); 
 			 echo<<"</a></li>\n            ";
 }else{ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/listarticle?topicid=";
echo<<obj["topicid"].to_int(); 
 			 echo<<"&page=";
echo<<std::to_string(i); 
 			 echo<<"\">";
 echo<<std::to_string(i); 
 			 echo<<"</a></li>\n            ";
 } 
 			 echo<<"            ";
 } 
 			 echo<<"            ";
 if(obj["pageinfo"]["current"].to_int()<obj["pageinfo"]["total"].to_int()){ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/listarticle?topicid=";
echo<<obj["topicid"].to_int(); 
 			 echo<<"&page=";
echo<<(obj["pageinfo"]["current"].to_int()+1); 
 			 echo<<"\">下一页</a></li>\n            ";
 } 
 			 echo<<"            ";
 if(obj["pageinfo"]["total"].to_int()>obj["pageinfo"]["max"].to_int()){ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/listarticle?topicid=";
echo<<obj["topicid"].to_int(); 
 			 echo<<"&page=";
echo<<obj["pageinfo"]["total"].to_int(); 
 			 echo<<"\">";
echo<<obj["pageinfo"]["total"].to_string(); 
 			 echo<<"</a></li>\n            ";
 } 
 			 echo<<"          \n          </ul>\n        </nav>\n      </div>\n    </div>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n    <script lang=\"javascript\">\n      var topicid=";
echo<<obj["topicid"].to_int(); 
 			 echo<<";\n    var topic_json = ";
echo << obj["list"].to_json(); 
 			 echo<<" ;\n    $('#parentid').append($('<option>').val(\"0\").text(\"所有栏目\").attr(\"selected\", true));\n    for (var i = 0; i < topic_json.length; i++) \n    {\n      if(topic_json[i].id==topicid)\n      {\n        $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid).attr(\"selected\",\"selected\"));\n      }\n      else\n      {      \n        $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n      }\n    }\n    var treeselect =$(\"#parentid\").selectTree({ expandLevel: 1 ,\n        changeCallBack:function(data){\n                window.location.href=\"/admin/listarticle?topicid=\"+data[0].value;\n        }\n    });\n    \n    function deletearticle(id)\n    {\n      if(confirm('是否删除?'))\n      {          \n          $.getJSON(\"/admin/deletearticle?id=\"+id,function(result){\n                if(result.code==0)\n                {\n                \n                     $(\"#article_\"+id).remove();\n                }\n                \n          });\n\n      }\n      return false;\n    }\n    function updatearticlesort(id,sortid)\n    {\n       $.getJSON(\"/admin/updatearticlesort?id=\"+id+\"&sortid=\"+sortid,function(result){\n                               \n       });\n    }\n    function searcharticle()\n    {\n        if($(\"#searchkeyword\").val()<1)\n        {\n           alert(\"请输入关键词！\");\n        }\n        window.location.href=\"/admin/listarticle?topicid=\"+$(\"#parentid\").val()+\"&searchword=\"+$(\"#searchkeyword\").val();\n    }\n    function artupdateview(obj)\n    {\n      let isview=0;\n      if(obj.checked)\n        {\n           isview=1;\n        }\n        $.post(\"/admin/updatearticleview?id=\"+obj.value,{isview:isview},function(result){\n                               \n       });\n    }\n    function artupdateishome(obj)\n    {\n      let isview=0;\n      if(obj.checked)\n        {\n           isview=1;\n        }\n        $.post(\"/admin/updatearticleishome?id=\"+obj.value,{ishome:isview},function(result){\n                               \n       });\n    }\n    </script>\n</body>\n\n</html>";

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
    