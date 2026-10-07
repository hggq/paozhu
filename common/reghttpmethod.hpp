#ifndef __HTTP_REGHTTPMETHOD_HPP
#define __HTTP_REGHTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

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
inline void _inithttpmethodregto()
{
    REG_SYNC_SYNC("", "/testjson",    nullptr, testjson);
    REG_SYNC_SYNC("", "/testmodel",   nullptr, testmodel);
    REG_SYNC_SYNC("", "/testmodelsmartptr", nullptr, testmodelsmartptr);
    REG_SYNC_SYNC("", "/testlogin",   nullptr, testlogin);
    REG_SYNC_SYNC("", "/showlogin",   nullptr, testshowlogin);
    REG_SYNC_SYNC("", "/loginpost",   nullptr, testloginpost);
    REG_SYNC_SYNC("", "/testview",    nullptr, testloginview);
    REG_SYNC_SYNC("", "/testmysql",   nullptr, testmysqlconnect);
    REG_SYNC_SYNC("", "/helloworld",  nullptr,
        [](std::shared_ptr<httppeer> peer) -> std::string {
            peer->output = "Hello, World!";
            return "";
        });
}

}
#endif
