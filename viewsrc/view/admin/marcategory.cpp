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
//g++ viewsrc/view/admin/marcategory.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string marcategory([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>分类管理</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n  \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container\">\n  <h5>分类管理</h5>\n  <hr>\n  <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <table class=\"table table-bordered\">\n          <thead>\n            <tr>\n              <th scope=\"col\">#</th>\n              <th scope=\"col\">名称</th>\n              <th scope=\"col\">编辑</th>\n            </tr>\n          </thead>\n          <tbody>\n            ";
 for(auto &a:obj["list"].as_array()){ 
 			 echo<<"              <tr id=\"topicid_";
 echo<<a.second["id"].to_string(); 
 			 echo<<"\">\n                <td>\n                  ";
 echo<<a.second["id"].to_string(); 
 			 echo<<"                </td>\n                <td ";
 if(a.second["parentid"].to_int()>0){ 
 			 echo<<"style=\"padding-left:";
 echo<<(a.second["level"].to_int()*15+15); 
 			 echo<<"px\"";
 } 
 			 echo<<"  id=\"itemid_";
 echo<<a.second["id"].to_string(); 
 			 echo<<"\">\n                  ";
 echo<<a.second["value"].as_string(); 
 			 echo<<"                </td>\n                 \n                <td>\n                  <a href=\"javascript:void()\" data-cid=\"";
 echo<<a.second["id"].to_string(); 
 			 echo<<"\"  data-pid=\"";
 echo<<a.second["parentid"].to_string(); 
 			 echo<<"\" data-title=\"";
 echo<<a.second["value"].as_string(); 
 			 echo<<"\" data-bs-toggle=\"modal\" data-bs-target=\"#staticBackdrop\">编辑</a>｜\n                  <a href=\"/admin/deletecatalogue?id=";
 echo<<a.second["id"].to_string(); 
 			 echo<<"\" onclick=\"return deletecatalogue(";
 echo<<a.second["id"].to_string(); 
 			 echo<<")\">删除</a>\n                </td>\n              </tr>\n              ";
 } 
 			 echo<<"          </tbody>\n\n        </table>\n      </div>\n    </div>\n\n\n    <form class=\"row g-3 mt-1\" id=\"form\" method=\"post\">\n      \n      <div class=\"col-md-6\">\n        <label for=\"parentid\" class=\"form-label\">父类别</label>\n        <select class=\"form-select\" id=\"parentid\" name=\"parentid\">\n        </select>\n      </div>\n      <div class=\"col-md-6\">\n        <label for=\"topicname\" class=\"form-label\">名称</label>\n        <input type=\"text\" class=\"form-control\" id=\"topicname\" name=\"topicname\">\n      </div>\n      <div class=\"col-12\">\n        <button type=\"submit\" class=\"btn btn-primary\">提交</button>\n      </div>\n    </form>\n\n  </div>\n  \n  \n   <div class=\"modal fade\" id=\"staticBackdrop\" tabindex=\"-1\" aria-labelledby=\"exampleModalLabel\" aria-hidden=\"true\">\n  <div class=\"modal-dialog modal-lg modal-dialog-centered\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"exampleModalLabel\">编辑名称</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n      <form id=\"editcategory\" method=\"post\" enctype=\"application/x-www-form-urlencoded\">\n       <div class=\"col-md-6\">\n        <label for=\"editparentid\" class=\"form-label\">父类别</label>\n        <select class=\"form-select\" id=\"editparentid\" name=\"editparentid\">\n        </select>\n      </div>\n      <div class=\"col-md-6\">\n        <label for=\"editname\" class=\"form-label\">名称</label>\n        <input type=\"text\" class=\"form-control\" id=\"editname\" name=\"editname\">\n        <input type=\"hidden\" id=\"cid\" name=\"cid\" value=\"0\" />\n      </div>\n      </form>\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">Close</button>\n        <button type=\"button\" class=\"btn btn-primary\" onclick=\"typepost()\">Save</button>\n      </div>\n    </div>\n  </div>\n</div>\n  \n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n\n    var topic_json = ";
echo << obj["list"].to_json(); 
 			 echo<<" ;\n    $('#parentid').append($('<option>').val(\"0\").text(\"一级类别\").attr(\"selected\", true));\n    for (var i = 0; i < topic_json.length; i++) {\n          \n       $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n       $('#editparentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n   }\n\n    var treeselect = $(\"#parentid\").selectTree({ expandLevel: 1 });\n    var treeedit = $(\"#editparentid\").selectTree({ expandLevel: 1 });\n\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n\n      var data = $(this).serialize();\n\n      var obj = this;\n\n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/addcataloguepost\",\n        method: 'post',\n        data: data,\n        success: function (res) {\n          //添加成功，这里演示\n          console.log(res);\n          if (res.code == 0) {\n            topic_json = res.list;\n            var pid=$('#parentid').val();\n            $('#parentid').empty();\n\n            $('#parentid').append($('<option>').val(\"0\").text(\"一级类别\"));\n            for (var i = 0; i < topic_json.length; i++) {\n            if(topic_json[i].id==pid)\n              {\n                 $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid).attr(\"selected\", true));\n              }\n              else\n              {\n                  $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n        \n              }\n            }\n              treeselect.selectOption(pid);\n          }\n          $(\"#form\").trigger(\"reset\");\n        }\n      })\n    });\n\t\t$('#staticBackdrop').on('show.bs.modal', function (event) {\n\nvar button = $(event.relatedTarget) \n\nvar pid = button.data('pid');\nvar pname = button.data('title');\nvar cid = button.data('cid');\n\n$(\"#cid\").val(cid);\n$(\"#editname\").val(pname);\n$('#editparentid').empty();\n\n $('#editparentid').append($('<option>').val(\"0\").text(\"一级类别\"));\ntreeedit.clearSelect();\n\n    for (var i = 0; i < topic_json.length; i++) {\n      if(topic_json[i].id==pid)\n      {\n        $('#editparentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid).attr(\"selected\", true));\n      }\n      else\n      {      \n        $('#editparentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n      }\n   }\n   \n   treeedit.selectOption(pid);\n\n});\n\n   function typepost() {\n      var data = $(editcategory).serialize();\n\n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/editcataloguepost\",\n        method: 'post',\n        data: data,\n        success: function (res) {\n          //添加成功，这里演示\n          if (res.code == 0) {\n              $(\"#itemid_\"+$(\"#cid\").val()).html($(\"#editname\").val());\n          }\n          $('#staticBackdrop').modal('hide');\n        }\n      });\n    }\n    function deletecatalogue(id)\n    {\n      if(confirm('是否删除?'))\n      {          \n          $.getJSON(\"/admin/deletecatalogue?id=\"+id,function(result){\n                if(result.code==0)\n                {\n                \n                     $(\"#topicid_\"+id).remove();\n                }\n                \n          });\n\n      }\n      return false;\n    }\n  </script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    