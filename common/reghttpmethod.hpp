#ifndef __HTTP_REGHTTPMETHOD_HPP
#define __HTTP_REGHTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h"
#include "testlogin.h"
#include "testmysql.h"
#include "testview.h"
#include "testcurd.h"
#include "testhello.h"
#include "testjson.h"
#include "testmodel.h"
namespace http
{
  void _inithttpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
  {
    struct regmethold_t temp;
    temp.pre = nullptr;
    temp.regfun = testhello;
    methodcallback.emplace("hello", temp);

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

    //testmysqlconnect
    temp.regfun = testmysqlconnect;
    methodcallback.emplace("testmysql", temp); 

    temp.regfun = articlelogin;
    methodcallback.emplace("cms/login", temp); 

    temp.regfun = articleloginpost;
    methodcallback.emplace("cms/loginpost", temp); 

    temp.regfun = articlelist;
    methodcallback.emplace("cms/list", temp); 

    temp.regfun = articleshow;
    methodcallback.emplace("cms/show", temp); 

    temp.regfun = articleedit;
    methodcallback.emplace("cms/edit", temp); 

    temp.regfun = articleeditpost;
    methodcallback.emplace("cms/editpost", temp); 

    temp.regfun = articledelete;
    methodcallback.emplace("cms/delete", temp); 

    temp.regfun = articleadd;
    methodcallback.emplace("cms/add", temp); 

    temp.regfun = articleaddpost;
    methodcallback.emplace("cms/addpost", temp); 


    temp.regfun = [](std::shared_ptr<httppeer> peer) -> std::string
    { std::string aa="hello world!"; return aa; };
    methodcallback.emplace("hello", temp);
  }

}
#endif