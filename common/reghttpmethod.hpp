#ifndef __HTTP_REGHTTPMETHOD_HPP
#define __HTTP_REGHTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif// defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h"
#include "testlogin.h"
#include "testmysql.h"
#include "testview.h"
#include "testcrud.h"
#include "testhello.h"
#include "testjson.h"
#include "testmodel.h"
namespace http
{
void _inithttpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
{
    struct regmethold_t temp;
    temp.pre = nullptr;

    temp.regfun = testjson;
    methodcallback.emplace("testjson", temp);

    temp.regfun = testmodel;
    methodcallback.emplace("testmodel", temp);

    temp.regfun = testmodelsmartptr;
    methodcallback.emplace("testmodelsmartptr", temp);

    temp.regfun = testlogin;
    methodcallback.emplace("testlogin", temp);
    temp.regfun = testshowlogin;
    methodcallback.emplace("showlogin", temp);
    temp.regfun = testloginpost;
    methodcallback.emplace("loginpost", temp);

    temp.regfun = testloginview;
    methodcallback.emplace("testview", temp);

    // testmysqlconnect
    temp.regfun = testmysqlconnect;
    methodcallback.emplace("testmysql", temp);

    temp.regfun = [](std::shared_ptr<httppeer> peer) -> std::string
    {
        peer->output = "Hello, World!";
        return "";
    };
    methodcallback.emplace("helloworld", temp);
}

}// namespace http
#endif