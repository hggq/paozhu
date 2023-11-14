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
//g++ viewsrc/view/admin/addtopic.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string addtopic([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>添加栏目</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n  \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container\">\n    <form class=\"row g-3 mt-1\" id=\"form\" method=\"post\">\n      <h3>添加栏目</h3>\n      <div class=\"col-md-6\">\n        <label for=\"topicname\" class=\"form-label\">栏目名称</label>\n        <input type=\"text\" class=\"form-control\" id=\"topicname\" name=\"topicname\">\n      </div>\n      <div class=\"col-md-6\">\n        <label for=\"inputEmail4\" class=\"form-label\">父栏目</label>\n        <select class=\"form-select\" id=\"parentid\" name=\"parentid\">\n        </select>\n      </div>\n\n      <div class=\"col-md-4\">\n        <label for=\"topictype\" class=\"form-label\">栏目类型</label>\n\n        <select id=\"topictype\" class=\"form-select\" name=\"topictype\">\n          <option  value=\"0\">图文内容</option>\n          <option  value=\"1\">单页图文</option>\n          <option  value=\"2\">多页图文</option>\n          <option  value=\"3\">文档目录</option>\n          <option  value=\"4\">新闻列表</option>\n          <option  value=\"5\">新闻首页</option>\n          <option  value=\"6\">产品展示</option>\n          <option  value=\"7\">产品类别</option>\n          <option  value=\"8\">视频首页</option>\n          <option  value=\"9\">视频列表</option>\n          <option  value=\"10\">博客类型</option>\n          <option  value=\"11\">兴趣论坛</option>\n          <option  value=\"12\">报名表单</option>\n          <option  value=\"13\">数据查询</option>\n          <option  value=\"14\">跳转地址</option>\n          <option  value=\"15\">菜单下拉</option>\n          <option  value=\"16\">菜单弹出层</option>\n        </select>\n      </div>\n      <div class=\"col-md-2\">\n        <label for=\"topicstatus\" class=\"form-label\">状态</label>\n        <div class=\"form-check form-switch mt-2\">\n          <input class=\"form-check-input\" type=\"checkbox\" role=\"switch\" name=\"topicstatus\" id=\"topicstatus\" checked>\n          <label class=\"form-check-label\" for=\"topicstatus\">是否显示</label>\n        </div>\n\n      </div>\n\n      <div class=\"col-md-6\">\n        <label for=\"urlpath\" class=\"form-label\">栏目urlpath</label>\n        <input type=\"text\" class=\"form-control\" id=\"urlpath\" name=\"urlpath\">\n      </div>\n\n\n      <div class=\"col-12\">\n        <label for=\"memo\" class=\"form-label\">备注</label>\n        <input type=\"text\" class=\"form-control\" id=\"memo\" name=\"memo\" value=\"\">\n      </div>\n\n      <div class=\"col-12\">\n        <button type=\"submit\" class=\"btn btn-primary\">添加</button>\n      </div>\n    </form>\n\n  </div>\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n\n    var topic_json = ";
echo << obj["list"].to_json(); 
 			 echo<<" ;\n    $('#parentid').append($('<option>').val(\"0\").text(\"一级栏目\").attr(\"selected\", true));\n    for (var i = 0; i < topic_json.length; i++) {\n      if(topic_json[i].id==2)\n      {\n        $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid).attr(\"selected\",\"selected\"));\n      }\n      else\n      {      \n        $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n      }\n   }\n\n    var treeselect = $(\"#parentid\").selectTree({ expandLevel: 1 });\n\n\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n\n      var data = $(this).serialize();\n\n      var obj = this;\n\n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/addtopicpost\",\n        method: 'post',\n        data: data,\n        success: function (res) {\n          //添加成功，这里演示\n          console.log(res);\n          if (res.code == 0) {\n            topic_json = res.list;\n\n            $('#parentid').empty();\n\n            $('#parentid').append($('<option>').val(\"0\").text(\"一级栏目\").attr(\"selected\", true));\n            for (var i = 0; i < topic_json.length; i++) {\n              $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n            }\n            treeselect.clearSelect();\n            // treeselect =$(\"#parentid\").selectTree({ expandLevel: 1 });\n\n\n          }\n          $(\"#form\").trigger(\"reset\");\n        }\n      })\n    });\n\n  </script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    