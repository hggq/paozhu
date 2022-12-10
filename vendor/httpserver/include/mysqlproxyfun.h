#ifndef HTTP_MYSQLPROXYFUN_H
#define HTTP_MYSQLPROXYFUN_H

#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include <set>
#include <string_view>
#include <thread>
// #include "http.hpp"
#include "request.h"
#include <iostream>
#include <functional>
#include <mysqlx/xdevapi.h>
#include <map>
#include <typeinfo>

#include <memory>

#include <list>
#include <queue>

#include <condition_variable>

namespace http
{

      mysqlx::SqlResult domysqleditexecute(std::string &sql, size_t dbhash);
      mysqlx::RowResult domysqlexecute(std::string &sql, size_t dbhash);
      bool domysqlcommit(std::list<std::string> &sql, size_t dbhash);

}

#endif