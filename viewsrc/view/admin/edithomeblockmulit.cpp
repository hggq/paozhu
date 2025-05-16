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
 		 std::string edithomeblockmulit([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>显示栏目</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container\">\n    <form class=\"g-3 mt-1\" id=\"form\" method=\"post\">\n      <h3>显示内容块</h3>\n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"title\" class=\"form-label\">模块名称</label>\n          <input type=\"text\" class=\"form-control\" id=\"title\" name=\"title\" value=\"";
echo<<obj["info"]["title"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n      <div class=\"row\">\n        <div class=\"col-md-4\">\n          <label for=\"title\" class=\"form-label\">显示列数</label>\n          <input type=\"text\" class=\"form-control\" id=\"rownum\" name=\"rownum\" value=\"";
echo<<obj["info"]["rownum"].to_string(); 
 			 echo<<"\">\n        </div>\n        <div class=\"col-md-4\">\n          <label for=\"title\" class=\"form-label\">宽度</label>\n          <input type=\"text\" class=\"form-control\" id=\"width\" name=\"width\" value=\"";
echo<<obj["info"]["width"].to_string(); 
 			 echo<<"\">\n        </div>\n        <div class=\"col-md-4\">\n          <label for=\"title\" class=\"form-label\">高度</label>\n          <input type=\"text\" class=\"form-control\" id=\"height\" name=\"height\" value=\"";
echo<<obj["info"]["height"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n\n      <div class=\"row\">\n        <input type=\"hidden\" id=\"hbid\" name=\"hbid\" value=\"";
echo<<obj["info"]["hbid"].to_string(); 
 			 echo<<"\" />\n        <button type=\"submit\" class=\"btn btn-primary\">保存</button>\n      </div>\n\n      <div class=\"row\">\n        ";
 for(auto &a:obj["list"].as_array()){ 
 			 echo<<"          <div class=\"row g-3 mt-1\">\n                <div class=\"col-md-2\">\n                  ";
echo<<a["title"].to_string(); 
 			 echo<<"              </div>\n              <div class=\"col-md-2\">\n                <a href=\"/admin/marbrand?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\">编辑</a>｜<a href=\"/admin/deletebrand?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\" onclick=\"return confirm('确定删除吗？')\">删除</a>\n             </div>\n          </div>\n           ";
 } 
 			 echo<<" \n \n      </div>  \n       \n    </form>\n\n\n    <form class=\"g-3 mt-1\" id=\"form\" method=\"post\" action=\"/admin/brandaddpost\" >\n      <div class=\"row p-3\">\n    <div class=\"col-md-6\">\n      <label for=\"newtitle\" class=\"form-label\">标题</label>\n      <input type=\"text\" class=\"form-control\" id=\"newtitle\" name=\"newtitle\" value=\"";
echo<<obj["info"]["title"].to_string(); 
 			 echo<<"\">\n    </div>\n    \n    </div>\n  \n              <div class=\"row p-3\">\n          <label for=\"editor\" class=\"col-md-2 col-form-label text-sm-end\">自定义内容</label>\n          <div class=\"col-md-10\">\n            <textarea class=\"form-control\" id=\"editor\" rows=\"4\" name=\"content\"\n              placeholder=\"\">";
echo<<obj["info"]["detailcontent"].to_string(); 
 			 echo<<"</textarea>\n          </div>\n        </div>\n\n        \n    <div class=\"col-12\">\n      <input type=\"hidden\" id=\"hbid\" name=\"hbid\" value=\"";
echo<<obj["info"]["hbid"].to_string(); 
 			 echo<<"\" />\n      <button type=\"submit\" class=\"btn btn-primary\">提交</button>\n    </div>\n  </form>\n\n\n  </div>\n  \n \n \n  <div class=\"modal fade\" id=\"myfiletobigModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"staticBackdropLabel\" aria-hidden=\"true\">\n    <div class=\"modal-dialog\">\n      <div class=\"modal-content\">\n        <div class=\"modal-header\">\n          <h1 class=\"modal-title fs-5\" id=\"staticBackdropLabel\">提示</h1>\n          <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n        </div>\n        <div class=\"modal-body\">\n           文件太大了，请传4M以下的文件！\n        </div>\n        <div class=\"modal-footer\">\n          <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">关闭</button>\n           \n        </div>\n      </div>\n    </div>\n  </div>  \n  \n  <div class=\"modal fade\" id=\"myfilesuccessModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"myfilesuccess\" aria-hidden=\"true\">\n    <div class=\"modal-dialog\">\n      <div class=\"modal-content\">\n        <div class=\"modal-header\">\n          <h1 class=\"modal-title fs-5\" id=\"myfilesuccess\">提示</h1>\n          <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n        </div>\n        <div class=\"modal-body\">\n           上传成功！\n        </div>\n        <div class=\"modal-footer\">\n          <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">确定</button>\n           \n        </div>\n      </div>\n    </div>\n  </div>\n\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n   bkLib.onDomLoaded(function() {\n\t  new nicEditor({fullPanel : true,iconsPath : '/nicedit/nicEditorIcons.gif'}).panelInstance('editor');\n    });\n\n     function gotourl()\n    {\n        window.location.href='/admin/marhome';\n    }\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n      var data = $(this).serialize();\n\n      var obj=this;  \n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/edithomeblockpost\",\n        method: 'post',\n        contentType:\"application/x-www-form-urlencoded\",\n        data: data,\n        dataType: \"json\"\n        }).done(function(data) {\n          if(data.code==0)\n          {\n             setTimeout(\"gotourl()\",500 );  \n          }\n        });\n    });\n    \n    \n\n    $(document).ready(function(e) {\n        $(\"#relate-icoimg\").change(function(){\n            var fd=new FormData();\n            if($(this).get(0).files[0].size>4596640){\n                $('#myfiletobigModal').modal('show');\n                return false;\n            }\n            fd.append(\"upfile\",$(this).get(0).files[0]);\n            $(\"#progress-bar-show\").show();\n            $(\"#progress-bar-result\").hide();\n            $.ajax({\n                url:\"/imageapi/upload?action=uploadimage\",\n                type:\"POST\",\n                processData: false,\n                contentType:false,\n                data: fd,\n                dataType: \"json\",\n                xhr: function() {  // custom xhr\n                    myXhr = $.ajaxSettings.xhr();\n                    if(myXhr.upload){ // check if upload property exists\n                        myXhr.upload.addEventListener('progress',function(evt){\n                            var percentComplete = Math.round(evt.loaded*100 / evt.total);\n                            $( \"#progress-bar\").css(\"width\",percentComplete*2);\n                            $( \"#progress-bar\").html(percentComplete+'%');\n                        }, false); // for handling the progress of the upload\n                    }\n                    return myXhr;\n                },\n                success:function(data){\n                    if(data.state==\"SUCCESS\"){\n                      $(\"#progress-bar-show\").hide();\n                      $(\"#progress-bar-result\").show();\n                      $(\"#icoimg\").val(data.url);\n                      $(\"#progress-bar-result\").html(\"<a href='\"+data.url+\"' target=_blank><img height=\\\"50\\\" width=\\\"50\\\"  src=\\\"\"+data.url+\"\\\"/></a>\");\n                      $('#myfilesuccessModal').modal('show');\n                    }\n                     \n                }\n            });\n        });\n    });\t  \n  </script>\n</body>\n\n</html>";

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
    