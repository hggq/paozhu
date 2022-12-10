
#include <iostream>
#include <string>
#include <mysqlx/xdevapi.h>
#include "mysqlproxyfun.h"
#include "mysqlconfig.h"
#include "mysqlpool.h"
namespace http
{
    mysqlx::SqlResult domysqleditexecute(std::string &sql, size_t dbhash)
    {
        std::map<size_t, mysqllinkpool> &mysqldbpoolglobal = get_mysqlpool();
        auto iter = mysqldbpoolglobal.find(dbhash);
         mysqlx::SqlResult res;
        if (iter != mysqldbpoolglobal.end())
        {
             res = iter->second.sqleditfetch(sql);

            return std::move(res);
        }
        else
        {
            return res;
        }
    }
    mysqlx::RowResult domysqlexecute(std::string &sql, size_t dbhash)
    {
        std::map<size_t, mysqllinkpool> &mysqldbpoolglobal = get_mysqlpool();
        auto iter = mysqldbpoolglobal.find(dbhash);
        mysqlx::RowResult res;
        if (iter != mysqldbpoolglobal.end())
        {
            res = iter->second.sqlselectfetch(sql);

            return std::move(res);
        }
        else
        {
            return res;
        }
    }

    bool domysqlcommit(std::list<std::string> &sql, size_t dbhash)
    {
        std::map<size_t, mysqllinkpool> &mysqldbpoolglobal = get_mysqlpool();
        auto iter = mysqldbpoolglobal.find(dbhash);
        if (iter != mysqldbpoolglobal.end())
        {
            return iter->second.sqleditcommit(sql);
        }
        else
        {
            return false;
        }
    }
}