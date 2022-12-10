#ifndef HTTP_MYSQLPOOL_H
#define HTTP_MYSQLPOOL_H

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
  
  class mysqllinkpool
  {
  public:
    mysqllinkpool(std::string ms, std::string sl);

    void addselectsession(unsigned int num);

    void addeditsession(unsigned int num);

    mysqlx::RowResult sqlselectfetch(std::string &sql);

    mysqlx::SqlResult sqleditfetch(std::string &sql);
    bool sqleditcommit(std::list<std::string> &sql);

    mysqlx::SqlResult sqleditfetch(std::string &sql, long long &effectrow, std::function<std::unique_ptr<mysqlx::Session>(std::string &, mysqlx::SqlResult &, std::unique_ptr<mysqlx::Session>)> func);
    mysqlx::SqlResult sqleditfetch(std::string &sql, long long &effectrow, std::function<std::unique_ptr<mysqlx::Session>(std::string &, std::unique_ptr<mysqlx::Session>)> funcbeg, std::function<std::unique_ptr<mysqlx::Session>(std::string &, std::unique_ptr<mysqlx::Session>)> funcend);

    unsigned int getselectpoolnum();
    unsigned int geteditpoolnum();

    unsigned int clearpool();

    void initpool();

  public:
    size_t dbhash;
    unsigned editmax = 2, editmin = 1, selectmax = 2, selectmin = 1;
    unsigned int selectbusynum = 0, editbusynum = 0;
    std::queue<std::unique_ptr<mysqlx::Session>> sesseditpool;
    std::queue<std::unique_ptr<mysqlx::Session>> sessselecttpool;
    std::string connectedit;
    std::string connectselect;
  };
  std::map<size_t,mysqllinkpool> &get_mysqlpool();
}

#endif