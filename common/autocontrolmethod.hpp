
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

#include "testmodelfromjson.h"
#include "testcmake.h"


namespace http
{
  void _initauto_control_httpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
  {
    struct regmethold_t temp;

		temp.pre = nullptr;
		temp.regfun = testmodelfromjson;
		methodcallback.emplace("mfromjson",temp);
		temp.pre = nullptr;
		temp.regfun = testcmake;
		methodcallback.emplace("ccmake",temp);
		temp.pre = nullptr;
		temp.regfun = testcauto;
		methodcallback.emplace("ccauto",temp);


    }
}

#endif

    