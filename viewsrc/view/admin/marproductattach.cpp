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
//g++ viewsrc/view/admin/marproductattach.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string marproductattach([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>产品管理</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n \n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">产品管理-附件查看</h5>\n    <hr>\n    <div class=\"row p-3\">\n      <div class=\"col-5\">\n        <div class=\"col-sm-10\">\n          <select class=\"form-select\" id=\"smallid\" name=\"smallid\"></select>\n        </div>\n      </div>\n      <div class=\"col-5\">\n        <div class=\"input-group\">\n        <input type=\"text\" class=\"form-control\" name=\"searchkeyword\" id=\"searchkeyword\" value=\"";
 echo<<obj["searchword"].to_string(); 
 			 echo<<"\" placeholder=\"\">\n          <button class=\"btn btn-outline-secondary\" type=\"button\" id=\"button-addon2\" onclick=\"searcharticle()\">搜索</button>\n        </div> \n      </div>  \n    </div>  \n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n            ";
 for(auto &a:obj["alist"].as_array()){ 
 			 echo<<"            <p class=\"proattach\" data-id=\"";
 echo<<a["pid"].to_string(); 
 			 echo<<"\"><strong>";
 echo<<a["cname"].to_string(); 
 			 echo<<" \t";
 echo<<a["name"].to_string(); 
 			 echo<<" </strong></p>\n            ";
 for(auto &b:a["profilelist"].as_array()){ 
              std::string attrname=b["name"].to_string();  
              if(attrname.size()>0)
              {
            
 			 echo<<"            <p style=\"color: #555555;text-decoration: underline;margin-left: 150px;\"\"><a href=\"";
 echo<<b["url"].to_string(); 
 			 echo<<"\" target=\"_blank\">";
 echo<<b["name"].as_string(); 
 			 echo<<" <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" fill=\"currentColor\" class=\"bi bi-cloud-download\" viewBox=\"0 0 16 16\">\n              <path d=\"M4.406 1.342A5.53 5.53 0 0 1 8 0c2.69 0 4.923 2 5.166 4.579C14.758 4.804 16 6.137 16 7.773 16 9.569 14.502 11 12.687 11H10a.5.5 0 0 1 0-1h2.688C13.979 10 15 8.988 15 7.773c0-1.216-1.02-2.228-2.313-2.228h-.5v-.5C12.188 2.825 10.328 1 8 1a4.53 4.53 0 0 0-2.941 1.1c-.757.652-1.153 1.438-1.153 2.055v.448l-.445.049C2.064 4.805 1 5.952 1 7.318 1 8.785 2.23 10 3.781 10H6a.5.5 0 0 1 0 1H3.781C1.708 11 0 9.366 0 7.318c0-1.763 1.266-3.223 2.942-3.593.143-.863.698-1.723 1.464-2.383\"/>\n              <path d=\"M7.646 15.854a.5.5 0 0 0 .708 0l3-3a.5.5 0 0 0-.708-.708L8.5 14.293V5.5a.5.5 0 0 0-1 0v8.793l-2.146-2.147a.5.5 0 0 0-.708.708z\"/>\n            </svg></a></p>\n            ";
 } } 
 			 echo<<" \n            ";
 } 
 			 echo<<"    \n \n      </div>\n    </div>\n\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <nav aria-label=\"Page navigation example\">\n          <ul class=\"pagination\">\n          ";
 if(obj["pageinfo"]["min"].to_int()>1){ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/marproductattach?categoryid=";
echo<<obj["categoryid"].to_int(); 
 			 echo<<"&page=";
echo<<obj["pageinfo"]["current"].to_int(); 
 			 echo<<"\">1</a></li>\n          ";
 } 
 			 echo<<"          ";
 if(obj["pageinfo"]["current"].to_int()>1){ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/marproductattach?categoryid=";
echo<<obj["categoryid"].to_int(); 
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
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link active\" href=\"/admin/marproductattach?categoryid=";
echo<<obj["categoryid"].to_int(); 
 			 echo<<"&page=";
echo<<std::to_string(i); 
 			 echo<<"\">";
 echo<<std::to_string(i); 
 			 echo<<"</a></li>\n            ";
 }else{ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/marproductattach?categoryid=";
echo<<obj["categoryid"].to_int(); 
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
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/marproductattach?categoryid=";
echo<<obj["categoryid"].to_int(); 
 			 echo<<"&page=";
echo<<(obj["pageinfo"]["current"].to_int()+1); 
 			 echo<<"\">下一页</a></li>\n            ";
 } 
 			 echo<<"            ";
 if(obj["pageinfo"]["total"].to_int()>obj["pageinfo"]["max"].to_int()){ 
 			 echo<<"            <li class=\"page-item\"><a class=\"page-link\" href=\"/admin/marproductattach?categoryid=";
echo<<obj["categoryid"].to_int(); 
 			 echo<<"&page=";
echo<<obj["pageinfo"]["total"].to_int(); 
 			 echo<<"\">";
echo<<obj["pageinfo"]["total"].to_string(); 
 			 echo<<"</a></li>\n            ";
 } 
 			 echo<<"          \n          </ul>\n        </nav>\n      </div>\n    </div>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n    <script lang=\"javascript\">\n \n    var categoryid=";
echo<<obj["categoryid"].to_int(); 
 			 echo<<";\n    var category_json = ";
echo << obj["categorylist"].to_json(); 
 			 echo<<" ;\n\n    $('#smallid').append($('<option>').val(\"0\").text(\"产品分类\").attr(\"selected\", true));\n    for (var i = 0; i < category_json.length; i++) \n    {\n      if(category_json[i].id==categoryid)\n      {\n        $('#smallid').append($('<option>').val(category_json[i].id).text(category_json[i].value).data(\"pid\", category_json[i].parentid).attr(\"selected\",\"selected\"));\n      }\n      else\n      {      \n        $('#smallid').append($('<option>').val(category_json[i].id).text(category_json[i].value).data(\"pid\", category_json[i].parentid));\n      }\n    }\n\n    var categoryselect =$(\"#smallid\").selectTree({ expandLevel: 1,\n        changeCallBack:function(data){\n          categoryid=data[0].value;\n        }\n    });\n\n \n    function searcharticle()\n    {\n        if($(\"#searchkeyword\").val()<1)\n        {\n           alert(\"请输入关键词！\");\n        }\n        window.location.href=\"/admin/marproductattach?categoryid=\"+categoryid+\"&searchword=\"+$(\"#searchkeyword\").val();\n    }\n \n\n    $(document).ready(function(e) {\n\n      $(\".proattach\").mouseover(function(e){\n              if($(this).children(\"a\").length==0)\n              {\n                $(this).append(\"<a href=\\\"/admin/editproduct?id=\"+$(this).data(\"id\")+\"\\\">编辑</a>\");\n              }\n        });\n\n        $(\".proattach\").mouseout(function(e){\n               $(this).find(\"a\").remove();\n        });\n\n    });\n\n    </script>\n</body>\n\n</html>";

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
    