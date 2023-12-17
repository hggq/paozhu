
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
#include "teststrip_html.h"
#include "testrand.h"
#include "testormcache.h"
#include "testsendmail.h"
#include "admin/articles.h"
#include "admin/topics.h"
#include "admin/main.h"
#include "testformpost.h"
#include "testqrcode.h"
#include "imageapi.h"
#include "testpzcache.h"
#include "testhello.h"
#include "testcmake.h"
#include "testjsonreflect.h"
#include "testaddclienttask.h"
#include "testormclient.h"
#include "testcommit.h"
#include "apicrudtest.h"
#include "testipsearch.h"
#include "testcowaitclient.h"
#include "testcors.h"
#include "testsqltuple.h"
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
		temp.regfun = testsendmaildo;
		methodcallback.emplace("testsendmaildo",temp);
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
		temp.pre = admin_islogin;
		temp.regfun = admin_listarticle;
		methodcallback.emplace("admin/listarticle",temp);
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
		temp.regfun = testcors;
		methodcallback.emplace("api/user/message",temp);
		temp.pre = nullptr;
		temp.regfun = testcorssimple;
		methodcallback.emplace("api/user/info",temp);
		temp.pre = nullptr;
		temp.regfun = testsqltuple;
		methodcallback.emplace("mtuple",temp);
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
}

#endif

    