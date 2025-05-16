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
//This file create by paozhu Fri, 16 May 2025 06:02:27 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string addhomepic([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>图片轮播</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container\">\n    <form class=\"g-3 mt-1\" id=\"form\" method=\"post\">\n      <h3>图片轮播</h3>\n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"title\" class=\"form-label\">模块名称</label>\n          <input type=\"text\" class=\"form-control\" id=\"title\" name=\"title\" value=\"";
echo<<obj["info"]["title"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n    \n      <div class=\"row\">\n        ";
 for(auto &a:obj["infotopimgs"].as_array()){ 
 			 echo<<"          <div class=\"row g-3 mt-1\">\n                <div class=\"col-md-2\">\n                <input type=\"text\" class=\"form-control\"  name=\"header_title[]\" placeholder=\"头图标题\" value=\"";
echo<<a["header_title"].to_string(); 
 			 echo<<"\">\n              </div>\n              <div class=\"col-md-2\">\n                <input type=\"text\" class=\"form-control\"  name=\"header_content[]\"  placeholder=\"头图说明\" value=\"";
echo<<a["header_content"].to_string(); 
 			 echo<<"\">\n              </div>\n              <div class=\"col-md-6\">\n                <div class=\"input-group row\">\n                  <input type=\"file\" class=\"form-control uploadfile col-md-4\"  accept=\".jpg,.png,.jpeg\"  aria-describedby=\"inputGroupFileAddon04\" aria-label=\"Upload\">\n                  <input type=\"text\" class=\"form-control col-md-8\"  name=\"header_urlpath[]\"  placeholder=\"图片地址\" value=\"";
echo<<a["header_urlpath"].to_string(); 
 			 echo<<"\">\n                </div>\n              </div>\n              <div class=\"col-md-2\">\n                 <button type=\"button\" class=\"btn btn-outline-info\" onclick=\"additem(this)\">+</button>\n                 <button type=\"button\" class=\"btn btn-outline-danger\" onclick=\"removeitem(this)\">-</button>\n              </div>\n          </div>\n           ";
 } 
 			 echo<<" \n           ";
 if(obj["infotopimgs"].size()==0){ 
 			 echo<<"          <div class=\"row g-3 mt-1\">\n                <div class=\"col-md-2\">\n                <input type=\"text\" class=\"form-control\"  name=\"header_title[]\" placeholder=\"头图标题\" value=\"\">\n              </div>\n              <div class=\"col-md-2\">\n                <input type=\"text\" class=\"form-control\"  name=\"header_content[]\"  placeholder=\"头图说明\" value=\"\">\n              </div>\n              <div class=\"col-md-6\">\n                <div class=\"input-group row\">\n                  <input type=\"file\" class=\"form-control uploadfile col-md-4\"  accept=\".jpg,.png,.jpeg\"  aria-describedby=\"inputGroupFileAddon04\" aria-label=\"Upload\">\n                  <input type=\"text\" class=\"form-control col-md-8\"  name=\"header_urlpath[]\"  placeholder=\"图片地址\" value=\"\">\n                </div>\n              </div>\n              <div class=\"col-md-2\">\n                 <button type=\"button\" class=\"btn btn-outline-info\" onclick=\"additem(this)\">+</button>\n                 <button type=\"button\" class=\"btn btn-outline-danger\" onclick=\"removeitem(this)\">-</button>\n              </div>\n          </div>\n           ";
 } 
 			 echo<<" \n      </div>  \n       \n      <div class=\"row\" style=\"margin-top: 20px;\">\n        <div class=\"col-md-6\">\n        <br />\n        <input type=\"hidden\" name=\"hbid\" value=\"";
echo<<obj["info"]["hbid"].to_string(); 
 			 echo<<"\">\n        <input type=\"hidden\" name=\"viewtype\" value=\"1\">\n        <button type=\"submit\" class=\"btn btn-primary\">添加</button>\n        <a href=\"/admin/marhome\" class=\"btn btn-link rounded-pill px-3\">返回</a>\n        </div>\n      </div>\n    </form>\n\n  </div>\n  \n \n \n<div class=\"modal fade\" id=\"myfiletobigModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"staticBackdropLabel\" aria-hidden=\"true\">\n  <div class=\"modal-dialog\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"staticBackdropLabel\">提示</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n         文件太大了，请传4M以下的文件！\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">关闭</button>\n         \n      </div>\n    </div>\n  </div>\n</div> \n\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n\n     function gotourl()\n    {\n        window.location.href='/admin/marhome';\n    }\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n      var data = $(this).serialize();\n\n      var obj=this;  \n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/addhomeblockpost\",\n        method: 'post',\n        contentType:\"application/x-www-form-urlencoded\",\n        data: data,\n        dataType: \"json\"\n        }).done(function(data) {\n          if(data.code==0)\n          {\n             setTimeout(\"gotourl()\",500 );  \n          }\n        });\n    });\n    function additem(obj)\n    {\n        jQuery(obj).parent().parent().after(jQuery(obj).parent().parent().clone()); \n        upfile_event();\n    }\n    function removeitem(obj)\n    {\n        if(jQuery(obj).parent().parent().parent().children().length>1)\n        {\n           jQuery(obj).parent().parent().remove();      \n        }\n        \n    }\n    \n  \n  function upfile_event()\n    {\n\n      $(\".uploadfile\").change(function(){\n              var fd=new FormData();\n              if($(this).get(0).files[0].size>15728640){\n                alert(\"附件不能大于15M，附件为zip、rar、jpg格式！\");\n                return false;\n              }\n              fd.append(\"upfile\",$(this).get(0).files[0]);\n              var selfobj=this;\n              $.ajax({\n                url:\"/admin/topicfileupload?action=uploadimage\",\n                type:\"POST\",\n                processData: false,\n                contentType:false,\n                data: fd,\n                dataType: \"json\",\n                success:function(data){\n                    if(data.state==\"SUCCESS\"){\n                      $(selfobj).next().val(data.url);\n                    }\n                    else\n                    {\n                        alert(data.state);\n                    }\n                    //console.log(data.status);\n                }\n              });\n            });\n    }\n\n    $(document).ready(function(e) {\n         \n            \n             $(\".uploadfile\").change(function(){\n              var fd=new FormData();\n              if($(this).get(0).files[0].size>15728640){\n                alert(\"附件不能大于15M，附件为zip、rar、jpg格式！\");\n                return false;\n              }\n              fd.append(\"upfile\",$(this).get(0).files[0]);\n              var selfobj=this;\n              $.ajax({\n                url:\"/admin/topicfileupload?action=uploadimage\",\n                type:\"POST\",\n                processData: false,\n                contentType:false,\n                data: fd,\n                dataType: \"json\",\n                success:function(data){\n                    if(data.state==\"SUCCESS\"){\n                      $(selfobj).next().val(data.url);\n                    }\n                    else\n                    {\n                        alert(data.state);\n                    }\n                    //console.log(data.status);\n                }\n              });\n            });\n\n \n    });\n  </script>\n</body>\n\n</html>";

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
    