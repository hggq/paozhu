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
//g++ viewsrc/view/admin/marbrand.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string marbrand([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>分类管理</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n  \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container\">\n  <h5>分类管理</h5>\n  <hr>\n  <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <table class=\"table table-bordered\">\n          <thead>\n            <tr>\n              <th scope=\"col\">#</th>\n              <th scope=\"col\">分类</th>\n              <th scope=\"col\">名称</th>\n              <th scope=\"col\">Logo</th>\n              <th scope=\"col\">排序</th>\n              <th scope=\"col\">编辑</th>\n            </tr>\n          </thead>\n          <tbody>\n            ";
 for(auto &a:obj["list"].as_array()){ 
 			 echo<<"              <tr id=\"topicid_";
 echo<<a["id"].to_string(); 
 			 echo<<"\">\n                <td>\n                  ";
 echo<<a["id"].to_string(); 
 			 echo<<"                </td>\n                <td>\n                  ";
 echo<<a["cname"].as_string(); 
 			 echo<<"                </td>\n                <td>\n                  ";
 echo<<a["name"].as_string(); 
 			 echo<<"                </td>\n                \n                <td>\n                  <img src=\"";
 echo<<a["logo"].as_string(); 
 			 echo<<"\" width=\"60\" height=\"60\" />  <a href=\"";
 echo<<a["logo"].as_string(); 
 			 echo<<"\" target=_blank>View</a>\n                </td>\n                <td>\n                  <input type=\"text\" class=\"form-inline topicsort col-md-2\" id=\"title_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" name=\"title_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" value=\"";
 echo<<a["sortid"].to_string(); 
 			 echo<<"\" onchange=\"updatebrandsort(";
 echo<<a["id"].to_string(); 
 			 echo<<",this.value)\">\n                </td>\n                <td>\n                  <a href=\"/admin/marbrand?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\">编辑</a>｜\n                  <a href=\"/admin/deletebrand?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\" onclick=\"return confirm('确定删除吗？')\">删除</a>\n                </td>\n              </tr>\n              ";
 } 
 			 echo<<"          </tbody>\n\n        </table>\n      </div>\n    </div>\n\n\n    <form class=\"g-3 mt-1\" id=\"form\" method=\"post\" action=\"/admin/brandaddpost\" >\n        <div class=\"row p-3\">\n      <div class=\"col-md-6\">\n        <label for=\"topicid\" class=\"form-label\">栏目</label>\n        <select class=\"form-select\" id=\"topicid\" name=\"topicid\">\n        <option value=\"0\">无分类</option>\n        ";
 for(auto &a:obj["topiclist"].as_array()){ 
 			 echo<<"        <option value=\"";
 echo<<a["id"].to_string(); 
 			 echo<<"\" ";
 if(obj["topicid"].to_int()==a["id"].to_int()){ 
 			 echo<<"selected";
 } 
 			 echo<<">";
 echo<<a["name"].to_string(); 
 			 echo<<"</option>\n        ";
 } 
 			 echo<<"        </select>\n      </div>\n      <div class=\"col-md-6\">\n        <label for=\"topicname\" class=\"form-label\">名称</label>\n        <input type=\"text\" class=\"form-control\" id=\"topicname\" name=\"topicname\" value=\"";
echo<<obj["info"]["title"].to_string(); 
 			 echo<<"\">\n      </div>\n      \n      </div>\n      \n      <div class=\"row p-3\">\n            <label for=\"icoimg\" class=\"col-md-2 col-form-label text-sm-end\">Logo</label>\n            <div class=\"col-md-6\">\n              <div class=\"input-group mb-3\">\n              <input type=\"text\" class=\"form-control\" id=\"icoimg\" name=\"icoimg\" value=\"";
echo<<obj["info"]["logo"].to_string(); 
 			 echo<<"\" placeholder=\"品牌标志\">\n              <input class=\"form-control\" type=\"file\" style=\"width:100px\" id=\"relate-icoimg\" accept=\".png,.jpg,.jpeg\">\n              </div>\n            </div>\n            <div class=\"col-md-4\">\n              <div class=\"progress\" id=\"progress-bar-show\" style=\"display:none\" role=\"progressbar\" aria-valuenow=\"0\" aria-valuemin=\"0\" aria-valuemax=\"100\">\n                  <div class=\"progress-bar\" id=\"progress-bar\" style=\"width: 0%\">0</div>\n                </div>\n              <div id=\"progress-bar-result\"></div>  \n            </div>\n      </div> \n\n                <div class=\"row p-3\">\n            <label for=\"editor\" class=\"col-md-2 col-form-label text-sm-end\">品牌介绍</label>\n            <div class=\"col-md-10\">\n              <textarea class=\"form-control\" id=\"editor\" rows=\"4\" name=\"content\"\n                placeholder=\"\">";
echo<<obj["info"]["detailcontent"].to_string(); 
 			 echo<<"</textarea>\n            </div>\n          </div>\n\n          <div class=\"row p-3\">\n            <label for=\"introduce\" class=\"col-md-2 col-form-label text-sm-end\">自定义内容（首页）</label>\n            <div class=\"col-md-10\">\n              <textarea class=\"form-control\" id=\"introduce\" rows=\"4\" name=\"introduce\"\n                placeholder=\"\">";
echo<<obj["info"]["introduce"].to_string(); 
 			 echo<<"</textarea>\n            </div>\n          </div>\n          \n      <div class=\"col-12\">\n        <input type=\"hidden\" id=\"brandid\" name=\"brandid\" value=\"";
echo<<obj["info"]["brandid"].to_string(); 
 			 echo<<"\" />\n        <button type=\"submit\" class=\"btn btn-primary\">提交</button>\n      </div>\n    </form>\n\n  </div>\n  \n<div class=\"modal fade\" id=\"myfiletobigModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"staticBackdropLabel\" aria-hidden=\"true\">\n  <div class=\"modal-dialog\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"staticBackdropLabel\">提示</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n         文件太大了，请传4M以下的文件！\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">关闭</button>\n         \n      </div>\n    </div>\n  </div>\n</div>  \n\n<div class=\"modal fade\" id=\"myfilesuccessModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"myfilesuccess\" aria-hidden=\"true\">\n  <div class=\"modal-dialog\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"myfilesuccess\">提示</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n         上传成功！\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">确定</button>\n         \n      </div>\n    </div>\n  </div>\n</div>\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script src=\"/nicedit/nicEdit.js\" type=\"text/javascript\"></script>\n  <script lang=\"javascript\">\n\n   bkLib.onDomLoaded(function() {\n\t  new nicEditor({fullPanel : true,iconsPath : '/nicedit/nicEditorIcons.gif'}).panelInstance('editor');\n    new nicEditor({fullPanel : true,iconsPath : '/nicedit/nicEditorIcons.gif'}).panelInstance('introduce');\n    });\n    \n    function updatebrandsort(id,sortid)\n    {\n       $.getJSON(\"/admin/updatebrandsort?id=\"+id+\"&sortid=\"+sortid,function(result){\n                               \n       });\n    }\n \n\t    $(document).ready(function(e) {\n        $(\"#relate-icoimg\").change(function(){\n            var fd=new FormData();\n            if($(this).get(0).files[0].size>4596640){\n                $('#myfiletobigModal').modal('show');\n                return false;\n            }\n            fd.append(\"upfile\",$(this).get(0).files[0]);\n            $(\"#progress-bar-show\").show();\n            $(\"#progress-bar-result\").hide();\n            $.ajax({\n                url:\"/imageapi/upload?action=uploadimage\",\n                type:\"POST\",\n                processData: false,\n                contentType:false,\n                data: fd,\n                dataType: \"json\",\n                xhr: function() {  // custom xhr\n                    myXhr = $.ajaxSettings.xhr();\n                    if(myXhr.upload){ // check if upload property exists\n                        myXhr.upload.addEventListener('progress',function(evt){\n                            var percentComplete = Math.round(evt.loaded*100 / evt.total);\n                            $( \"#progress-bar\").css(\"width\",percentComplete*2);\n                            $( \"#progress-bar\").html(percentComplete+'%');\n                        }, false); // for handling the progress of the upload\n                    }\n                    return myXhr;\n                },\n                success:function(data){\n                    if(data.state==\"SUCCESS\"){\n                      $(\"#progress-bar-show\").hide();\n                      $(\"#progress-bar-result\").show();\n                      $(\"#icoimg\").val(data.url);\n                      $(\"#progress-bar-result\").html(\"<a href='\"+data.url+\"' target=_blank><img height=\\\"50\\\" width=\\\"50\\\"  src=\\\"\"+data.url+\"\\\"/></a>\");\n                      $('#myfilesuccessModal').modal('show');\n                    }\n                     \n                }\n            });\n        });\n    });\t  \n\t\n \n  </script>\n</body>\n\n</html>";

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
    