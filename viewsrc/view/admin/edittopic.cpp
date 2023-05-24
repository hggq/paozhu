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
//g++ viewsrc/view/admin/edittopic.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string edittopic(const struct view_param &vinfo,http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>编辑栏目</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container\">\n    <form class=\"row g-3 mt-1\" id=\"form\" method=\"post\">\n      <h3>编辑栏目</h3>\n      <div class=\"col-md-6\">\n        <label for=\"topicname\" class=\"form-label\">栏目名称</label>\n        <input type=\"text\" class=\"form-control\" id=\"topicname\" name=\"topicname\" value=\"";
echo<<obj["info"]["topicname"].to_string(); 
 			 echo<<"\">\n      </div>\n      <div class=\"col-md-6\">\n        <label for=\"inputEmail4\" class=\"form-label\">父栏目</label>\n        <select class=\"form-select\" id=\"parentid\" name=\"parentid\">\n        </select>\n      </div>\n\n      <div class=\"col-md-4\">\n        <label for=\"topictype\" class=\"form-label\">栏目类型</label>\n\n        <select id=\"topictype\" class=\"form-select\" name=\"topictype\">\n          <option selected value=\"0\">图文内容</option>\n          <option   value=\"1\">文档目录</option>\n        </select>\n      </div>\n      <div class=\"col-md-2\">\n        <label for=\"topicstatus\" class=\"form-label\">状态</label>\n        <div class=\"form-check form-switch mt-2\">\n          <input class=\"form-check-input\" type=\"checkbox\" role=\"switch\" name=\"topicstatus\" id=\"topicstatus\" ";
if(obj["info"]["topicstatus"].as_bool()){ 
 			 echo<<"checked";
} 
 			 echo<<">\n          <label class=\"form-check-label\" for=\"topicstatus\">是否显示</label>\n        </div>\n\n      </div>\n\n      <div class=\"col-md-6\">\n        <label for=\"urlpath\" class=\"form-label\">栏目urlpath</label>\n        <input type=\"text\" class=\"form-control\" id=\"urlpath\" name=\"urlpath\" value=\"";
echo<<obj["info"]["urlpath"].to_string(); 
 			 echo<<"\">\n      </div>\n\n\n      <div class=\"col-12\">\n        <label for=\"memo\" class=\"form-label\">备注</label>\n        <input type=\"text\" class=\"form-control\" id=\"memo\" name=\"memo\" value=\"";
echo<<obj["info"]["memo"].to_string(); 
 			 echo<<"\">\n      </div>\n\n      <div class=\"col-12\">\n        <button type=\"submit\" class=\"btn btn-primary\">编辑</button>\n        <input type=\"hidden\" id=\"topicid\" name=\"topicid\" value=\"";
echo<<obj["info"]["topicid"].to_string(); 
 			 echo<<"\" />\n        <a href=\"/admin/martopic\" class=\"btn btn-link rounded-pill px-3\">返回</a>\n      </div>\n    </form>\n\n  </div>\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n    var topicid=";
echo<<obj["info"]["parentid"].to_int(); 
 			 echo<<";\n    var topic_json = ";
echo << obj["list"].to_json(); 
 			 echo<<" ;\n    $('#parentid').append($('<option>').val(\"0\").text(\"一级栏目\").attr(\"selected\", true));\n    for (var i = 0; i < topic_json.length; i++) \n    {\n      if(topic_json[i].id==topicid)\n      {\n        $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid).attr(\"selected\",\"selected\"));\n      }\n      else\n      {      \n        $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n      }\n    }\n\n    var treeselect =$(\"#parentid\").selectTree({ expandLevel: 1 });\n\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n      var data = $(this).serialize();\n\n      var obj=this;  \n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/edittopicpost\",\n        method: 'post',\n        data: data,\n        success: function (res) {\n          //添加成功，这里演示\n          console.log(res);\n          if(res.code==0)\n          {\n                window.location.href='/admin/martopic';\n          }\n        }\n      })\n    });\n\n  </script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    