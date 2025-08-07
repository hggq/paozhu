
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

#include "testhttpclient.h"
#include "testmysqlinsert.h"
#include "testrestfulpath.h"
#include "testdownloadauth.h"
#include "testmodelfromjson.h"
#include "techempower.h"
#include "teststrip_html.h"
#include "testsoft_remove.h"
#include "teststr_join.h"
#include "test_co_handle.h"
#include "testrand.h"
#include "testormcache.h"
#include "teststr_trim.h"
#include "testsendmail.h"
#include "superadmin/supermain.h"
#include "admin/articles.h"
#include "admin/marproduct.h"
#include "admin/topics.h"
#include "admin/marbrand.h"
#include "admin/attachfile.h"
#include "admin/category.h"
#include "admin/marhome.h"
#include "admin/main.h"
#include "testformpost.h"
#include "teststr2int.h"
#include "testfield_num.h"
#include "testqrcode.h"
#include "imageapi.h"
#include "testpzcache.h"
#include "testhello.h"
#include "testsitepath.h"
#include "testto_tree.h"
#include "testcmake.h"
#include "testjsonreflect.h"
#include "testaddclienttask.h"
#include "testormclient.h"
#include "testmarkdown.h"
#include "testcommit.h"
#include "apicrudtest.h"
#include "testpinyin.h"
#include "testmoney_num.h"
#include "testipsearch.h"
#include "testcowaitclient.h"
#include "testcors.h"
#include "testsqltuple.h"
#include "testsiteid.h"
#include "testsessionid.h"
#include "devcors.h"
 
namespace http
{ 
     
    void _initauto_control_httpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
    {
        struct regmethold_t temp;

		temp.pre = nullptr;
		temp.regfun = testhttpclient_get_body;
		methodcallback.emplace("testhttpclient1",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_get_timebody;
		methodcallback.emplace("testhttpclient3",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_get_file;
		methodcallback.emplace("testhttpclient2",temp);
		temp.pre = nullptr;
		temp.regfun = testmysqlinsert;
		methodcallback.emplace("minsert",temp);
		temp.pre = nullptr;
		temp.regfun = testmysqlpagebar;
		methodcallback.emplace("mpagebar",temp);
		temp.pre = nullptr;
		temp.regfun = testrestfulpath;
		methodcallback.emplace("user/info",temp);
		temp.pre = nullptr;
		temp.regfun = testrestfulprofilepath;
		methodcallback.emplace("user/profile",temp);
		temp.pre = nullptr;
		temp.regfun = downloadfileauthmethod;
		methodcallback.emplace("downloadfileauth",temp);
		temp.pre = nullptr;
		temp.regfun = testmodelfromjson;
		methodcallback.emplace("mfromjson",temp);
		temp.pre = nullptr;
		temp.regfun = teststrip_html;
		methodcallback.emplace("teststrip_html",temp);
		temp.pre = nullptr;
		temp.regfun = testsoftremove;
		methodcallback.emplace("testsoftremove",temp);
		temp.pre = nullptr;
		temp.regfun = teststrjoin;
		methodcallback.emplace("teststr_join",temp);
		temp.pre = nullptr;
		temp.regfun = testrand;
		methodcallback.emplace("testrand",temp);
		temp.pre = nullptr;
		temp.regfun = testormcache;
		methodcallback.emplace("testormcache",temp);
		temp.pre = nullptr;
		temp.regfun = testormcacheb;
		methodcallback.emplace("testormcacheb",temp);
		temp.pre = nullptr;
		temp.regfun = testormcachec;
		methodcallback.emplace("testormcachec",temp);
		temp.pre = nullptr;
		temp.regfun = teststrtrim;
		methodcallback.emplace("teststr_trim",temp);
		temp.pre = nullptr;
		temp.regfun = testsendmaildo;
		methodcallback.emplace("testsendmaildo",temp);
		temp.pre = nullptr;
		temp.regfun = superadmin_login;
		methodcallback.emplace("superadmin/login",temp);
		temp.pre = nullptr;
		temp.regfun = superadmin_loginpost;
		methodcallback.emplace("superadmin/loginpost",temp);
		temp.pre = nullptr;
		temp.regfun = superadmin_logout;
		methodcallback.emplace("superadmin/logout",temp);
		temp.pre = nullptr;
		temp.regfun = superadmin_islogin;
		methodcallback.emplace("superadmin/islogin",temp);
		temp.pre = nullptr;
		temp.regfun = superadmin_isloginjson;
		methodcallback.emplace("superadmin/isloginjson",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_main;
		methodcallback.emplace("superadmin/main",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_editsiteinfo;
		methodcallback.emplace("superadmin/editsiteinfo",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_editsiteinfopost;
		methodcallback.emplace("superadmin/editsiteinfopost",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_deletesiteinfo;
		methodcallback.emplace("superadmin/deletesiteinfo",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_deletesiteinfopost;
		methodcallback.emplace("superadmin/deletesiteinfopost",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_addsiteinfo;
		methodcallback.emplace("superadmin/addsiteinfo",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_addsiteinfopost;
		methodcallback.emplace("superadmin/addsiteinfopost",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_welcome;
		methodcallback.emplace("superadmin/welcome",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_listuser;
		methodcallback.emplace("superadmin/listuser",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_edituser;
		methodcallback.emplace("superadmin/edituser",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_adduser;
		methodcallback.emplace("superadmin/adduser",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_adduserpost;
		methodcallback.emplace("superadmin/adduserpost",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_edituserpost;
		methodcallback.emplace("superadmin/edituserpost",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_deleteuser;
		methodcallback.emplace("superadmin/deleteuser",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_userinfo;
		methodcallback.emplace("superadmin/userinfo",temp);
		temp.pre = superadmin_islogin;
		temp.regfun = superadmin_editpassword;
		methodcallback.emplace("superadmin/editpassword",temp);
		temp.pre = superadmin_isloginjson;
		temp.regfun = superadmin_editpwdpost;
		methodcallback.emplace("superadmin/editpwdpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addarticle;
		methodcallback.emplace("admin/addarticle",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addarticlepost;
		methodcallback.emplace("admin/addarticlepost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_editarticle;
		methodcallback.emplace("admin/editarticle",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_editarticlepost;
		methodcallback.emplace("admin/editarticlepost",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_deletearticle;
		methodcallback.emplace("admin/deletearticle",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_gettoparticle;
		methodcallback.emplace("admin/gettoparticle",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_updatearticlesort;
		methodcallback.emplace("admin/updatearticlesort",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_updatearticleview;
		methodcallback.emplace("admin/updatearticleview",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_updatearticleishome;
		methodcallback.emplace("admin/updatearticleishome",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_listarticle;
		methodcallback.emplace("admin/listarticle",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addproduct;
		methodcallback.emplace("admin/addproduct",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addproductpost;
		methodcallback.emplace("admin/addproductpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_editproductpost;
		methodcallback.emplace("admin/editproductpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_getcategorytopproduct;
		methodcallback.emplace("admin/getcategorytopproduct",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_editproduct;
		methodcallback.emplace("admin/editproduct",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_listproduct;
		methodcallback.emplace("admin/listproduct",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_marproductattach;
		methodcallback.emplace("admin/marproductattach",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_deleteproduct;
		methodcallback.emplace("admin/deleteproduct",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_deleteproductajax;
		methodcallback.emplace("admin/deleteproductajax",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_updateproducthome;
		methodcallback.emplace("admin/updateproducthome",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_updateproductstore;
		methodcallback.emplace("admin/updateproductstore",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_updateproductview;
		methodcallback.emplace("admin/updateproductview",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_updateproductsort;
		methodcallback.emplace("admin/updateproductsort",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addtopic;
		methodcallback.emplace("admin/addtopic",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_edittopic;
		methodcallback.emplace("admin/edittopic",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_martopic;
		methodcallback.emplace("admin/martopic",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_updatetopicsort;
		methodcallback.emplace("admin/updatetopicsort",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_addtopicpost;
		methodcallback.emplace("admin/addtopicpost",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_deletetopic;
		methodcallback.emplace("admin/deletetopic",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_edittopicpost;
		methodcallback.emplace("admin/edittopicpost",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_topicfileupload;
		methodcallback.emplace("admin/topicfileupload",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_topicimgtextupload;
		methodcallback.emplace("admin/topicimgtextupload",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_updatetopicview;
		methodcallback.emplace("admin/updatetopicview",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_updatetopicsideblocksort;
		methodcallback.emplace("admin/updatetopicsideblocksort",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_edittopicside;
		methodcallback.emplace("admin/edittopicside",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_addtopicsidepick;
		methodcallback.emplace("admin/addtopicsidepick",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_addtopicsidetext;
		methodcallback.emplace("admin/addtopicsidetext",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_addtopicsideblockpost;
		methodcallback.emplace("admin/addtopicsideblockpost",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_deletetopicsideblock;
		methodcallback.emplace("admin/deletetopicsideblock",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_edittopicsideblocktext;
		methodcallback.emplace("admin/edittopicsideblocktext",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_edittopicsideblockpick;
		methodcallback.emplace("admin/edittopicsideblockpick",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_edittopicsideblockpost;
		methodcallback.emplace("admin/edittopicsideblockpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_marbrand;
		methodcallback.emplace("admin/marbrand",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_brandaddpost;
		methodcallback.emplace("admin/brandaddpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_deletebrand;
		methodcallback.emplace("admin/deletebrand",temp);
		temp.pre = admin_isloginjson;
		temp.regfun = admin_updatebrandsort;
		methodcallback.emplace("admin/updatebrandsort",temp);
		temp.pre = admin_islogin;
		temp.regfun = attachfile_gateway;
		methodcallback.emplace("attachfile/gateway",temp);
		temp.pre = admin_islogin;
		temp.regfun = attachimg_gateway;
		methodcallback.emplace("attachimg/gateway",temp);
		temp.pre = admin_islogin;
		temp.regfun = attachfile_delete;
		methodcallback.emplace("attachfile/delete",temp);
		temp.pre = admin_islogin;
		temp.regfun = attachimg_delete;
		methodcallback.emplace("attachimg/delete",temp);
		temp.pre = admin_islogin;
		temp.regfun = attachfile_upload;
		methodcallback.emplace("attachfile/upload",temp);
		temp.pre = admin_islogin;
		temp.regfun = attachfile_uploadpost;
		methodcallback.emplace("attachfile/uploadpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_attachselectfiles;
		methodcallback.emplace("admin/attachselectfiles",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_marcatalogue;
		methodcallback.emplace("admin/marcatalogue",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addcataloguepost;
		methodcallback.emplace("admin/addcataloguepost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_editcataloguepost;
		methodcallback.emplace("admin/editcataloguepost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_deletecatalogue;
		methodcallback.emplace("admin/deletecatalogue",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_marhome;
		methodcallback.emplace("admin/marhome",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_edithomeblockpic;
		methodcallback.emplace("admin/edithomeblockpic",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_updatehomeblocksort;
		methodcallback.emplace("admin/updatehomeblocksort",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addhomepic;
		methodcallback.emplace("admin/addhomepic",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addhometext;
		methodcallback.emplace("admin/addhometext",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addhometopic;
		methodcallback.emplace("admin/addhometopic",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addhomecontent;
		methodcallback.emplace("admin/addhomecontent",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_edithomeblockcontent;
		methodcallback.emplace("admin/edithomeblockcontent",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_edithomeblocktopic;
		methodcallback.emplace("admin/edithomeblocktopic",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_edithomeblockpost;
		methodcallback.emplace("admin/edithomeblockpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addhomeblockpost;
		methodcallback.emplace("admin/addhomeblockpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_addhomehot;
		methodcallback.emplace("admin/addhomehot",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_edithomeblockhot;
		methodcallback.emplace("admin/edithomeblockhot",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_edithomeblocktext;
		methodcallback.emplace("admin/edithomeblocktext",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_deletehomeblock;
		methodcallback.emplace("admin/deletehomeblock",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_homedesign;
		methodcallback.emplace("admin/homedesign",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_edithomeblockmulit;
		methodcallback.emplace("admin/edithomeblockmulit",temp);
		temp.pre = nullptr;
		temp.regfun = admin_login;
		methodcallback.emplace("admin/login",temp);
		temp.pre = nullptr;
		temp.regfun = admin_loginpost;
		methodcallback.emplace("admin/loginpost",temp);
		temp.pre = nullptr;
		temp.regfun = admin_logout;
		methodcallback.emplace("admin/logout",temp);
		temp.pre = nullptr;
		temp.regfun = admin_islogin;
		methodcallback.emplace("admin/islogin",temp);
		temp.pre = nullptr;
		temp.regfun = admin_isloginjson;
		methodcallback.emplace("admin/isloginjson",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_main;
		methodcallback.emplace("admin/main",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_welcome;
		methodcallback.emplace("admin/welcome",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_siteinfo;
		methodcallback.emplace("admin/siteinfo",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_siteinfopost;
		methodcallback.emplace("admin/siteinfopost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_footscript;
		methodcallback.emplace("admin/footscript",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_footscriptpost;
		methodcallback.emplace("admin/footscriptpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_copyright;
		methodcallback.emplace("admin/copyright",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_copyrightpost;
		methodcallback.emplace("admin/copyrightpost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_sitelogo;
		methodcallback.emplace("admin/sitelogo",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_sitelogopost;
		methodcallback.emplace("admin/sitelogopost",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_userinfo;
		methodcallback.emplace("admin/userinfo",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_editpassword;
		methodcallback.emplace("admin/editpassword",temp);
		temp.pre = admin_islogin;
		temp.regfun = admin_editpwdpost;
		methodcallback.emplace("admin/editpwdpost",temp);
		temp.pre = nullptr;
		temp.regfun = testurlencoded;
		methodcallback.emplace("tformpost",temp);
		temp.pre = nullptr;
		temp.regfun = testformmultipart;
		methodcallback.emplace("tfilepost",temp);
		temp.pre = nullptr;
		temp.regfun = testformjsonpost;
		methodcallback.emplace("tjsonpost",temp);
		temp.pre = nullptr;
		temp.regfun = testformxmlpost;
		methodcallback.emplace("txmlupload",temp);
		temp.pre = nullptr;
		temp.regfun = testuploadpostfile;
		methodcallback.emplace("addpostfile",temp);
		temp.pre = nullptr;
		temp.regfun = teststr2int;
		methodcallback.emplace("teststr2int",temp);
		temp.pre = nullptr;
		temp.regfun = testfieldnum;
		methodcallback.emplace("testfieldnum",temp);
		temp.pre = nullptr;
		temp.regfun = testqrcode;
		methodcallback.emplace("testqrcode",temp);
		temp.pre = nullptr;
		temp.regfun = imageapi_gateway;
		methodcallback.emplace("imageapi/gateway",temp);
		temp.pre = nullptr;
		temp.regfun = imageapi_upload;
		methodcallback.emplace("imageapi/upload",temp);
		temp.pre = nullptr;
		temp.regfun = testpzcache;
		methodcallback.emplace("testcache",temp);
		temp.pre = nullptr;
		temp.regfun = testshowcache;
		methodcallback.emplace("testshowcache",temp);
		temp.pre = nullptr;
		temp.regfun = testhello;
		methodcallback.emplace("hello",temp);
		temp.pre = nullptr;
		temp.regfun = testhellobusy;
		methodcallback.emplace("hellobusy",temp);
		temp.pre = nullptr;
		temp.regfun = testsitepath;
		methodcallback.emplace("testsitepath",temp);
		temp.pre = nullptr;
		temp.regfun = testtotree;
		methodcallback.emplace("testtotree",temp);
		temp.pre = nullptr;
		temp.regfun = testcmake;
		methodcallback.emplace("ccmake",temp);
		temp.pre = nullptr;
		temp.regfun = testcauto;
		methodcallback.emplace("ccauto",temp);
		temp.pre = nullptr;
		temp.regfun = testjsonreflect;
		methodcallback.emplace("testjsonreflect",temp);
		temp.pre = nullptr;
		temp.regfun = testaddclienttaskpre;
		methodcallback.emplace("testnotaddclienttaskpre",temp);
		temp.pre = testaddclienttaskpre;
		temp.regfun = testaddclienttask;
		methodcallback.emplace("testaddclienttask",temp);
		temp.pre = nullptr;
		temp.regfun = testexecuteclienttask;
		methodcallback.emplace("executeclienttask",temp);
		temp.pre = nullptr;
		temp.regfun = testormclient;
		methodcallback.emplace("testormclient",temp);
		temp.pre = nullptr;
		temp.regfun = testmarkdown2html;
		methodcallback.emplace("testmarkdown2html",temp);
		temp.pre = nullptr;
		temp.regfun = testcommit;
		methodcallback.emplace("testcommit",temp);
		temp.pre = nullptr;
		temp.regfun = pxapidepartmentsaddpost;
		methodcallback.emplace("api/departments/addpost",temp);
		temp.pre = nullptr;
		temp.regfun = pxapidepartmentseditpost;
		methodcallback.emplace("api/departments/editpost",temp);
		temp.pre = nullptr;
		temp.regfun = pxapidepartmentslist;
		methodcallback.emplace("api/departments/list",temp);
		temp.pre = nullptr;
		temp.regfun = pxapipagesdepartlist;
		methodcallback.emplace("api/departments/deletedep",temp);
		temp.pre = nullptr;
		temp.regfun = testpinyin;
		methodcallback.emplace("testpinyin",temp);
		temp.pre = nullptr;
		temp.regfun = testpinyin_loaded;
		methodcallback.emplace("testpinyin_loaded",temp);
		temp.pre = nullptr;
		temp.regfun = testtestmoneynum;
		methodcallback.emplace("testtestmoneynum",temp);
		temp.pre = nullptr;
		temp.regfun = testipsearch;
		methodcallback.emplace("testipsearch",temp);
		temp.pre = nullptr;
		temp.regfun = testipsearchget;
		methodcallback.emplace("ipsearchget",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_cowait_php;
		methodcallback.emplace("testcowaitclient4",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_cowait_body;
		methodcallback.emplace("testcowaitclient1",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_cowait_post;
		methodcallback.emplace("testcowaitclient5",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_cowait_urls;
		methodcallback.emplace("testcowaitclient2",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_cowait_spawn;
		methodcallback.emplace("testcowaitclient3",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_get_range;
		methodcallback.emplace("testclientgetrange",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_downfilelist;
		methodcallback.emplace("downfilelist",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient_getdownfile;
		methodcallback.emplace("downfilecontent",temp);
		temp.pre = nullptr;
		temp.regfun = testcors;
		methodcallback.emplace("api/user/message",temp);
		temp.pre = nullptr;
		temp.regfun = testcorssimple;
		methodcallback.emplace("api/user/info",temp);
		temp.pre = nullptr;
		temp.regfun = testsqltuple;
		methodcallback.emplace("mtuple",temp);
		temp.pre = nullptr;
		temp.regfun = testsiteid;
		methodcallback.emplace("testsiteid",temp);
		temp.pre = nullptr;
		temp.regfun = testsetsession;
		methodcallback.emplace("testsetsession",temp);
		temp.pre = nullptr;
		temp.regfun = testshowsession;
		methodcallback.emplace("testshowsession",temp);
		temp.pre = nullptr;
		temp.regfun = api_dev_hostcors;
		methodcallback.emplace("api/dev/hostcors",temp);


    }
    
    void _initauto_co_control_httpmethodregto(std::map<std::string, regmethold_co_t> &methodcallback)
    {
        struct regmethold_co_t temp;
		temp.pre = nullptr;
		temp.regfun = techempowerplaintext;
		methodcallback.emplace("plaintext",temp);
		temp.pre = nullptr;
		temp.regfun = techempowerjson;
		methodcallback.emplace("json",temp);
		temp.pre = nullptr;
		temp.regfun = techempowerdb;
		methodcallback.emplace("db",temp);
		temp.pre = nullptr;
		temp.regfun = techempowerqueries;
		methodcallback.emplace("queries",temp);
		temp.pre = nullptr;
		temp.regfun = techempowerfortunes;
		methodcallback.emplace("fortunes",temp);
		temp.pre = nullptr;
		temp.regfun = techempowerupdates;
		methodcallback.emplace("updates",temp);
		temp.pre = nullptr;
		temp.regfun = techempowercached_queries;
		methodcallback.emplace("cached-queries",temp);
		temp.pre = nullptr;
		temp.regfun = techempowercached_db;
		methodcallback.emplace("cached-db",temp);
		temp.pre = nullptr;
		temp.regfun = test_co_handle;
		methodcallback.emplace("testcohandle",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient21_cowait_body;
		methodcallback.emplace("testcowaitclient21",temp);
		temp.pre = nullptr;
		temp.regfun = testhttpclient22_cowait_body;
		methodcallback.emplace("testcowaitclient22",temp);


    }
    
    void _initauto_domain_httpmethodregto(std::map<std::string, std::map<std::string, regmethold_t>> &domain_methodcallback)
    {
        struct regmethold_t temp;
        std::map<std::string, regmethold_t> methodcallback;
        std::map<std::string, std::map<std::string, regmethold_t>>::iterator domain_iterator;
    

    }
    
    void _initauto_co_domain_httpmethodregto(std::map<std::string, std::map<std::string, regmethold_co_t>> &domain_methodcallback)
    {
        struct regmethold_co_t temp;
        std::map<std::string, regmethold_co_t> methodcallback;
        std::map<std::string, std::map<std::string, regmethold_co_t>>::iterator domain_iterator;
    

    }
        
}    
#endif

    