#ifndef HTTP_MYSQLPROXYFUN_H
#define HTTP_MYSQLPROXYFUN_H

#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include <set>
#include <string_view>
#include <thread>
#include <iostream>
#include <functional>
#include "mysql.h"
#include <map>
#include <typeinfo>

#include <memory>

#include <list>
#include <queue>

#include <condition_variable>

namespace http
{

std::unique_ptr<MYSQL, decltype(&mysql_close)>
get_mysqleditexecute(size_t dbhash);
std::unique_ptr<MYSQL, decltype(&mysql_close)>
get_mysqlselectexecute(size_t dbhash);
bool do_mysqlcommit(size_t dbhash);
bool back_mysql_connect(size_t dbhash,
                        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn);

} // namespace http

#endif