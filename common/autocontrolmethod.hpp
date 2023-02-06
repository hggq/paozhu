
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

#include "testmysqlinsert.h"
#include "testmodelfromjson.h"
#include "testormcache.h"
#include "testformpost.h"
#include "testpzcache.h"
#include "testcmake.h"
#include "testcommit.h"
#include "testsqltuple.h"


namespace http
{
  void _initauto_control_httpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
  {
    struct regmethold_t temp;

		temp.pre = nullptr;
		temp.regfun = testmysqlinsert;
		methodcallback.emplace("minsert",temp);
		temp.pre = nullptr;
		temp.regfun = testmysqlpagebar;
		methodcallback.emplace("mpagebar",temp);
		temp.pre = nullptr;
		temp.regfun = testmodelfromjson;
		methodcallback.emplace("mfromjson",temp);
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
		temp.regfun = testpzcache;
		methodcallback.emplace("testcache",temp);
		temp.pre = nullptr;
		temp.regfun = testshowcache;
		methodcallback.emplace("testshowcache",temp);
		temp.pre = nullptr;
		temp.regfun = testcmake;
		methodcallback.emplace("ccmake",temp);
		temp.pre = nullptr;
		temp.regfun = testcauto;
		methodcallback.emplace("ccauto",temp);
		temp.pre = nullptr;
		temp.regfun = testcommit;
		methodcallback.emplace("testcommit",temp);
		temp.pre = nullptr;
		temp.regfun = testsqltuple;
		methodcallback.emplace("mtuple",temp);


    }
}

#endif

    