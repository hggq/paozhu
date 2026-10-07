#ifndef __HTTP_REGHTTPMETHOD_PRE_HPP
#define __HTTP_REGHTTPMETHOD_PRE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#include "httppeer.h"
#include "testcookie.h"
#include "testlogin.h"
namespace http
{
inline void _inithttpmethodregto_pre()
{
    REG_SYNC_SYNC("", "addcookie",  testlogin, testaddcookie);
    REG_SYNC_SYNC("", "showcookie", testlogin, testshowcookie);
}

}
#endif
