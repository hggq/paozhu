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

#include <map>
#include <typeinfo>
#include <memory>
#include <list>
#include <queue>
#include <condition_variable>
#include "mysql.h"
#include "mysqlpool.h"
namespace http
{

      std::unique_ptr<MYSQL, decltype(&mysql_close)> get_mysqleditexecute(size_t dbhash)
      {
            try
            {
                  std::map<std::size_t, mysqllinkpool> &myconn = get_mysqlpool();
                  auto iter = myconn.find(dbhash);
                  if (iter != myconn.end())
                  {
                        return std::move(iter->second.get_edit_connect());
                  }
                  throw " not find db link in pool!";
            }
            catch (const char *e)
            {
                  throw e;
            }
            std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(nullptr, &mysql_close);
            return conn;
      }
      std::unique_ptr<MYSQL, decltype(&mysql_close)> get_mysqlselectexecute(size_t dbhash)
      {
            try
            {
                  std::map<std::size_t, mysqllinkpool> &myconn = get_mysqlpool();
                  auto iter = myconn.find(dbhash);
                  if (iter != myconn.end())
                  {
                        return std::move(iter->second.get_select_connect());
                  }
                  throw " not find db link in pool!";
            }
            catch (const char *e)
            {
                  throw e;
            }
            std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(nullptr, &mysql_close);
            return conn;
      }

      bool back_mysql_connect(size_t dbhash, std::unique_ptr<MYSQL, decltype(&mysql_close)> conn)
      {
            
            try
            {
                  std::map<std::size_t, mysqllinkpool> &myconn = get_mysqlpool();
                  auto iter = myconn.find(dbhash);
                  if (iter != myconn.end())
                  {
                        iter->second.back_select_connect(std::move(conn));
                        return true;
                  }
                  return false;
            }
            catch (const std::exception &e)
            {
                  return false;
            }
      }

}

#endif