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
//g++ viewsrc/view/admin/edittopic.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string edittopic([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>编辑栏目</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container\">\n    <form class=\"row g-3 mt-1\" id=\"form\" method=\"post\">\n      <h3>编辑栏目</h3>\n      <div class=\"col-md-6\">\n        <label for=\"topicname\" class=\"form-label\">栏目名称</label>\n        <input type=\"text\" class=\"form-control\" id=\"topicname\" name=\"topicname\" value=\"";
echo<<obj["info"]["topicname"].to_string(); 
 			 echo<<"\">\n      </div>\n      <div class=\"col-md-6\">\n        <label for=\"inputEmail4\" class=\"form-label\">父栏目</label>\n        <select class=\"form-select\" id=\"parentid\" name=\"parentid\">\n        </select>\n      </div>\n\n      <div class=\"col-md-4\">\n        <label for=\"topictype\" class=\"form-label\">栏目类型</label>\n\n        <select id=\"topictype\" class=\"form-select\" name=\"topictype\">\n          <option  value=\"0\">图文内容</option>\n          <option  value=\"1\">单页图文</option>\n          <option  value=\"2\">多页图文</option>\n          <option  value=\"3\">单页倒叙</option>\n          <option  value=\"4\">文档目录</option>\n          <option  value=\"5\">新闻列表</option>\n          <option  value=\"6\">新闻首页</option>\n          <option  value=\"7\">产品展示</option>\n          <option  value=\"8\">产品类别</option>\n          <option  value=\"9\">商品列表</option>\n          <option  value=\"10\">商品展示</option>\n          \n          <option  value=\"11\">视频首页</option>\n          <option  value=\"12\">视频列表</option>\n          <option  value=\"13\">博客类型</option>\n          <option  value=\"14\">兴趣论坛</option>\n          <option  value=\"15\">报名表单</option>\n          <option  value=\"16\">数据查询</option>\n          <option  value=\"17\">跳转地址</option>\n          <option  value=\"18\">菜单下拉</option>\n          <option  value=\"19\">菜单弹出层</option>\n          <option  value=\"20\">会员注册</option>\n          <option  value=\"21\">会员登录</option>\n          <option  value=\"22\">会员信息</option>\n          <option  value=\"23\">购物车</option>\n          <option  value=\"24\">品牌分类</option>\n          <option  value=\"31\">文章首页图片1</option>\n          <option  value=\"32\">文章首页图片2</option>\n          <option  value=\"33\">文章单页图片3</option>\n          <option  value=\"34\">文章列表无日期1</option>\n          <option  value=\"35\">文章单页图片5</option>\n        </select>\n      </div>\n      <div class=\"col-md-2\">\n        <label for=\"topicstatus\" class=\"form-label\">状态</label>\n        <div class=\"form-check form-switch mt-2\">\n          <input class=\"form-check-input\" type=\"checkbox\" role=\"switch\" name=\"topicstatus\" id=\"topicstatus\" ";
if(obj["info"]["topicstatus"].to_bool()){ 
 			 echo<<"checked";
} 
 			 echo<<">\n          <label class=\"form-check-label\" for=\"topicstatus\">是否显示</label>\n        </div>\n\n      </div>\n\n      <div class=\"col-md-6\">\n        <label for=\"urlpath\" class=\"form-label\">栏目urlpath</label>\n        <input type=\"text\" class=\"form-control\" id=\"urlpath\" name=\"urlpath\" value=\"";
echo<<obj["info"]["urlpath"].to_string(); 
 			 echo<<"\">\n      </div>\n      \n      <div class=\"col-md-12\">\n        <button type=\"button\" class=\"btn btn-outline-info\" data-bs-toggle=\"modal\" data-bs-target=\"#staticBackdrop\">\n          头图设置\n        </button>\n      </div>   \n      <div class=\"col-md-6\">\n        <label for=\"isside\" class=\"form-label\">是否有边栏</label>\n        <div class=\"form-check form-switch mt-2\">\n          <input class=\"form-check-input\" type=\"checkbox\" role=\"switch\" name=\"isside\" id=\"isside\" value=\"1\" ";
if(obj["info"]["isside"].to_bool()){ 
 			 echo<<"checked";
} 
 			 echo<<">\n          <label class=\"form-check-label\" for=\"isside\">是否边栏</label>\n        </div>\n      </div>\n      <div class=\"col-md-6\">\n        <label for=\"sorttype\" class=\"form-label\">栏目内容排序</label>\n        <select class=\"form-select\" id=\"sorttype\" name=\"sorttype\">\n            <option  value=\"0\" ";
if(obj["info"]["sorttype"].to_int()==0){ 
 			 echo<<"selected";
} 
 			 echo<<">最新在前面</option>\n            <option  value=\"1\" ";
if(obj["info"]["sorttype"].to_int()==1){ 
 			 echo<<"selected";
} 
 			 echo<<">最新在后面</option>\n            <option  value=\"2\" ";
if(obj["info"]["sorttype"].to_int()==2){ 
 			 echo<<"selected";
} 
 			 echo<<">排序值最小在前</option>\n            <option  value=\"3\" ";
if(obj["info"]["sorttype"].to_int()==3){ 
 			 echo<<"selected";
} 
 			 echo<<">排序值最小在后</option>\n        </select>\n      </div>\n\n      <div class=\"col-12\">\n        <label for=\"icoimg\" class=\"col-sm-2 col-form-label text-sm-end\">栏目图标</label>\n        <div class=\"col-sm-6\">\n          <div class=\"input-group mb-3\">\n          <input type=\"text\" class=\"form-control\" id=\"icoimg\" name=\"icoimg\" value=\"";
echo<<obj["info"]["imgurl"].to_string(); 
 			 echo<<"\" placeholder=\"可以留空，列表使用封面图或头图使用\">\n          <input class=\"form-control\" type=\"file\" style=\"width:100px\" id=\"relate-icoimg\" accept=\".png,.jpg,.jpeg\">\n          </div>\n        </div>\n        <div class=\"col-sm-4\">\n          <div class=\"progress\" id=\"progress-bar-show\" style=\"display:none\" role=\"progressbar\" aria-valuenow=\"0\" aria-valuemin=\"0\" aria-valuemax=\"100\">\n              <div class=\"progress-bar\" id=\"progress-bar\" style=\"width: 0%\">0</div>\n            </div>\n          <div id=\"progress-bar-result\"></div>  \n        </div>\n      </div> \n\n      <div class=\"col-12\">\n        <label for=\"memo\" class=\"form-label\">备注</label>\n        <input type=\"text\" class=\"form-control\" id=\"memo\" name=\"memo\" value=\"";
echo<<obj["info"]["memo"].to_string(); 
 			 echo<<"\">\n      </div>\n\n      <div class=\"col-12\">\n        <button type=\"submit\" class=\"btn btn-primary\">编辑</button>\n        <input type=\"hidden\" id=\"topicid\" name=\"topicid\" value=\"";
echo<<obj["info"]["topicid"].to_string(); 
 			 echo<<"\" />\n        <a href=\"/admin/martopic\" class=\"btn btn-link rounded-pill px-3\">返回</a>\n      </div>\n    </form>\n\n  </div>\n  \n <div class=\"modal fade\" id=\"staticBackdrop\" tabindex=\"-1\" aria-labelledby=\"exampleModalLabel\" aria-hidden=\"true\">\n  <div class=\"modal-dialog modal-lg modal-dialog-centered\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"exampleModalLabel\">栏目头图</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n      <form id=\"headerimg\" method=\"post\" enctype=\"application/x-www-form-urlencoded\">\n      ";
 for(auto &a:obj["infotopimgs"].as_array()){ 
 			 echo<<"      <div class=\"row g-3 mt-1\">\n            <div class=\"col-md-2\">\n            <input type=\"text\" class=\"form-control\"  name=\"header_title[]\" placeholder=\"头图标题\" value=\"";
echo<<a["header_title"].to_string(); 
 			 echo<<"\">\n          </div>\n          <div class=\"col-md-2\">\n            <input type=\"text\" class=\"form-control\"  name=\"header_content[]\"  placeholder=\"头图说明\" value=\"";
echo<<a["header_content"].to_string(); 
 			 echo<<"\">\n          </div>\n          <div class=\"col-md-6\">\n            <div class=\"input-group row\">\n              <input type=\"file\" class=\"form-control uploadfile col-md-4\"  accept=\".jpg,.png,.jpeg\"  aria-describedby=\"inputGroupFileAddon04\" aria-label=\"Upload\">\n              <input type=\"text\" class=\"form-control col-md-8\"  name=\"header_urlpath[]\"  placeholder=\"图片地址\" value=\"";
echo<<a["header_urlpath"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"col-md-2\">\n             <button type=\"button\" class=\"btn btn-outline-info\" onclick=\"additem(this)\">+</button>\n             <button type=\"button\" class=\"btn btn-outline-danger\" onclick=\"removeitem(this)\">-</button>\n          </div>\n      </div>\n       ";
 } 
 			 echo<<" \n       ";
 if(obj["infotopimgs"].size()==0){ 
 			 echo<<"      <div class=\"row g-3 mt-1\">\n            <div class=\"col-md-2\">\n            <input type=\"text\" class=\"form-control\"  name=\"header_title[]\" placeholder=\"头图标题\" value=\"\">\n          </div>\n          <div class=\"col-md-2\">\n            <input type=\"text\" class=\"form-control\"  name=\"header_content[]\"  placeholder=\"头图说明\" value=\"\">\n          </div>\n          <div class=\"col-md-6\">\n            <div class=\"input-group row\">\n              <input type=\"file\" class=\"form-control uploadfile col-md-4\"  accept=\".jpg,.png,.jpeg\"  aria-describedby=\"inputGroupFileAddon04\" aria-label=\"Upload\">\n              <input type=\"text\" class=\"form-control col-md-8\"  name=\"header_urlpath[]\"  placeholder=\"图片地址\" value=\"\">\n            </div>\n          </div>\n          <div class=\"col-md-2\">\n             <button type=\"button\" class=\"btn btn-outline-info\" onclick=\"additem(this)\">+</button>\n             <button type=\"button\" class=\"btn btn-outline-danger\" onclick=\"removeitem(this)\">-</button>\n          </div>\n      </div>\n       ";
 } 
 			 echo<<" \n       \n       </form>\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">Close</button>\n        <button type=\"button\" class=\"btn btn-primary\" onclick=\"typepost()\">Save</button>\n      </div>\n    </div>\n  </div>\n</div>\n \n<div class=\"modal fade\" id=\"myfiletobigModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"staticBackdropLabel\" aria-hidden=\"true\">\n  <div class=\"modal-dialog\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"staticBackdropLabel\">提示</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n         文件太大了，请传4M以下的文件！\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">关闭</button>\n         \n      </div>\n    </div>\n  </div>\n</div> \n\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n    var topicid=";
echo<<obj["info"]["parentid"].to_int(); 
 			 echo<<";\n    var topic_json = ";
echo << obj["list"].to_json(); 
 			 echo<<" ;\n    $('#parentid').append($('<option>').val(\"0\").text(\"一级栏目\").attr(\"selected\", true));\n    for (var i = 0; i < topic_json.length; i++) \n    {\n      if(topic_json[i].id==topicid)\n      {\n        $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid).attr(\"selected\",\"selected\"));\n      }\n      else\n      {      \n        $('#parentid').append($('<option>').val(topic_json[i].id).text(topic_json[i].value).data(\"pid\", topic_json[i].parentid));\n      }\n    }\n\n    var treeselect =$(\"#parentid\").selectTree({ expandLevel: 1 });\n     function gotourl()\n    {\n        window.location.href='/admin/martopic';\n    }\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n      var data = $(this).serialize();\n\n      var obj=this;  \n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/edittopicpost?topicid=";
echo<<obj["info"]["topicid"].to_string(); 
 			 echo<<"\",\n        method: 'post',\n        contentType:\"application/x-www-form-urlencoded\",\n        data: data,\n        dataType: \"json\"\n        }).done(function(data) {\n          if(data.code==0)\n          {\n             setTimeout(\"gotourl()\",500 );  \n          }\n        });\n    });\n    function additem(obj)\n    {\n        jQuery(obj).parent().parent().after(jQuery(obj).parent().parent().clone()); \n        upfile_event();\n    }\n    function removeitem(obj)\n    {\n        if(jQuery(obj).parent().parent().parent().children().length>1)\n        {\n           jQuery(obj).parent().parent().remove();      \n        }\n        \n    }\n    \n   \tfunction typepost(){\n\t\t$.ajax({\n        url:\"/admin/topicimgtextupload?topicid=";
echo<<obj["info"]["topicid"].to_string(); 
 			 echo<<"\",\n        type:\"POST\",\n        processData: false,\n        contentType:\"application/x-www-form-urlencoded\",\n        data: $(\"#headerimg\").serialize(),\n        dataType: \"json\",\n        success:function(data){\n                 $(\"#staticBackdrop\").modal('hide');\n        }\n      });\n\t} \n  \n  function upfile_event()\n    {\n\n      $(\".uploadfile\").change(function(){\n              var fd=new FormData();\n              if($(this).get(0).files[0].size>15728640){\n                alert(\"附件不能大于15M，附件为zip、rar、jpg格式！\");\n                return false;\n              }\n              fd.append(\"upfile\",$(this).get(0).files[0]);\n              var selfobj=this;\n              $.ajax({\n                url:\"/admin/topicfileupload?action=uploadimage\",\n                type:\"POST\",\n                processData: false,\n                contentType:false,\n                data: fd,\n                dataType: \"json\",\n                success:function(data){\n                    if(data.state==\"SUCCESS\"){\n                      $(selfobj).next().val(data.url);\n                    }\n                    else\n                    {\n                        alert(data.state);\n                    }\n                    //console.log(data.status);\n                }\n              });\n            });\n    }\n\n    $(document).ready(function(e) {\n            $(\"#topictype\").val(";
echo<<obj["info"]["topictype"].to_int(); 
 			 echo<<");\n            \n             $(\".uploadfile\").change(function(){\n              var fd=new FormData();\n              if($(this).get(0).files[0].size>15728640){\n                alert(\"附件不能大于15M，附件为zip、rar、jpg格式！\");\n                return false;\n              }\n              fd.append(\"upfile\",$(this).get(0).files[0]);\n              var selfobj=this;\n              $.ajax({\n                url:\"/admin/topicfileupload?action=uploadimage\",\n                type:\"POST\",\n                processData: false,\n                contentType:false,\n                data: fd,\n                dataType: \"json\",\n                success:function(data){\n                    if(data.state==\"SUCCESS\"){\n                      $(selfobj).next().val(data.url);\n                    }\n                    else\n                    {\n                        alert(data.state);\n                    }\n                    //console.log(data.status);\n                }\n              });\n            });\n\n            $(\"#relate-icoimg\").change(function(){\n            var fd=new FormData();\n            if($(this).get(0).files[0].size>4596640){\n                $('#myfiletobigModal').modal('show');\n                return false;\n            }\n            fd.append(\"upfile\",$(this).get(0).files[0]);\n            $(\"#progress-bar-show\").show();\n            $(\"#progress-bar-result\").hide();\n            $.ajax({\n                url:\"/imageapi/upload?action=uploadimage\",\n                type:\"POST\",\n                processData: false,\n                contentType:false,\n                data: fd,\n                dataType: \"json\",\n                xhr: function() {  // custom xhr\n                    myXhr = $.ajaxSettings.xhr();\n                    if(myXhr.upload){ // check if upload property exists\n                        myXhr.upload.addEventListener('progress',function(evt){\n                            var percentComplete = Math.round(evt.loaded*100 / evt.total);\n                            $( \"#progress-bar\").css(\"width\",percentComplete*2);\n                            $( \"#progress-bar\").html(percentComplete+'%');\n                        }, false); // for handling the progress of the upload\n                    }\n                    return myXhr;\n                },\n                success:function(data){\n                    if(data.state==\"SUCCESS\"){\n                        $(\"#progress-bar-show\").hide();\n                        $(\"#progress-bar-result\").show();\n                        $(\"#icoimg\").val(data.url);\n\t\t\t\t\t\t$(\"#progress-bar-result\").html(\"<a href='\"+data.url+\"' target=_blank><img height=\\\"50\\\" width=\\\"50\\\"  src=\\\"\"+data.url+\"\\\"/></a>\");\n                       $('#myfilesuccessModal').modal('show');\n                    }\n                     \n                }\n            });\n        });\n    });\n  </script>\n</body>\n\n</html>";

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
    