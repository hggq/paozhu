#ifndef HTTP_MYSQLMODEL_HPP
#define HTTP_MYSQLMODEL_HPP

#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include <set>
#include <string_view>
#include <thread>
#include "request.h"
#include "unicode.h"

#include <iostream>
#include <functional>
#include "mysql.h"
#include <typeinfo>
#include <memory>
#include <list>
#include <queue>
#include <condition_variable>
#include "mysqlproxyfun.h"

namespace orm
{
    // 通用操作 类 mysql 方法 在这里接上getSession(); 这里可以操作 data sql count page
    //  using namespace http;
    template <typename model, typename base>
    class mysqlclientDB : public base
    {
    public:
        mysqlclientDB(std::string tag) : dbtag(tag)
        {
            dbhash = hash_fn(dbtag);

            mod = &static_cast<model &>(*this);
        }
        mysqlclientDB() : dbtag(base::_rmstag)
        {

            dbhash = hash_fn(dbtag);

            mod = &static_cast<model &>(*this);
        }
        unsigned int count()
        {
            std::string countsql;
            countsql = "SELECT count(*) as total_countnum  FROM ";
            countsql.append(base::tablename);
            countsql.append(" WHERE ");
            if (wheresql.empty())
            {
                countsql.append(" 1 ");
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
            }

            try
            {

                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                MYSQL_RES *resultone = nullptr;

                int readnum = mysql_query(conn.get(), &countsql[0]);

                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    try
                    {
                        http::back_mysql_connect(dbhash, std::move(conn));
                    }
                    catch (...)
                    {
                    }
                    return 0;
                }

                resultone = mysql_store_result(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                }

                if (!resultone)
                {
                    return 0;
                }
                MYSQL_ROW row;
                readnum = 0;
                if ((row = mysql_fetch_row(resultone)))
                {
                    try
                    {
                        readnum = std::atoll(row[0]);
                    }
                    catch (...)
                    {
                        readnum = 0;
                    }
                }
                mysql_free_result(resultone);

                return readnum;
            }
            catch (...)
            {
                return 0;
            }

            return 0;
        }
        int updateCol(std::string colname, int num)
        {
            std::string countsql;
            countsql = "UPDATE ";
            countsql.append(base::tablename);
            countsql.append(" SET ");
            countsql.append(colname);
            if (num > 0)
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.append(" +");
                countsql.append(std::to_string(num));
            }
            else
            {
                countsql.append(" = ");
                countsql.append(colname);

                countsql.append(std::to_string(num));
            }
            countsql.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
            }

            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                long long readnum = mysql_query(conn.get(), &countsql[0]);
                readnum = mysql_affected_rows(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                   
                }
                return readnum;
            }
            catch (...)
            {
                return 0;
            }
        }

        model &select(std::string fieldname)
        {
            selectsql = fieldname;
            return *mod;
        }
        model &cache(unsigned int livetime)
        {
            //cachetime = livetime;
            return *mod;
        }
        model &where(std::string wq)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(wq);
            return *mod;
        }
        model &where(std::string wq, long long val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }
            wq.append(std::to_string(val));

            wheresql.append(wq);
            return *mod;
        }
        model &where(std::string wq, int val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }
            wq.append(std::to_string(val));

            wheresql.append(wq);
            return *mod;
        }
        model &where(std::string wq, unsigned int val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }
            wq.append(std::to_string(val));

            wheresql.append(wq);
            return *mod;
        }
        model &where(std::string wq, std::string bi, http::OBJ_VALUE &obj)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wq.append(bi);
            if (obj.is_string())
            {
                wq.push_back('\'');
                wq.append(obj.as_string());
                wq.push_back('\'');
            }
            else
            {

                wq.append(obj.to_string());
            }

            wheresql.append(wq);
            return *mod;
        }
        model &where(std::string wq, http::OBJ_VALUE &obj)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }

            if (obj.is_string())
            {
                wq.push_back('\'');
                wq.append(obj.as_string());
                wq.push_back('\'');
            }
            else
            {

                wq.append(obj.to_string());
            }

            wheresql.append(wq);
            return *mod;
        }
        model &where(std::string wq, std::string bi, unsigned long long val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wq.append(bi);
            wq.append(std::to_string(val));
            wheresql.append(wq);
            return *mod;
        }

        model &where(std::string wq, unsigned long long val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }

            wq.append(std::to_string(val));

            wheresql.append(wq);
            return *mod;
        }

        model &where(std::string wq, std::string bi, std::string val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wq.append(bi);
            wq.push_back('\'');

            wq.append(val);
            wq.push_back('\'');

            wheresql.append(wq);
            return *mod;
        }
        model &where(std::string wq, std::string val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }

            wq.push_back('\'');
            wq.append(val);
            wq.push_back('\'');

            wheresql.append(wq);
            return *mod;
        }
        model &whereLike(std::string wq, std::string val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wq.append(" like '");
            if (val[0] == '%' || val.back() == '%')
            {
                wq.append(val);
                wq.append("' ");
            }
            else
            {
                wq.push_back('%');
                wq.append(val);
                wq.append("%' ");
            }

            wheresql.append(wq);
            return *mod;
        }
        model &whereOrLike(std::string wq, std::string val)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" OR ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wq.append(" like '");
            if (val[0] == '%' || val.back() == '%')
            {
                wq.append(val);
                wq.append("' ");
            }
            else
            {
                wq.push_back('%');
                wq.append(val);
                wq.push_back('%');
            }

            wheresql.append(wq);
            return *mod;
        }
        model &whereAnd(std::string wq)
        {

            wheresql.append(" AND ");
            wheresql.append(wq);
            return *mod;
        }
        model &whereAnd(std::string wq, long long val)
        {

            wheresql.append(" AND ");

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }
            wq.append(std::to_string(val));

            wheresql.append(wq);
            return *mod;
        }
        model &whereAnd(std::string wq, std::string val)
        {

            wheresql.append(" AND ");

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }

            wq.push_back('\'');
            wq.append(val);
            wq.push_back('\'');

            wheresql.append(wq);

            return *mod;
        }
        model &whereOr(std::string wq)
        {

            wheresql.append(" OR ");
            wheresql.append(wq);
            return *mod;
        }
        model &whereOr(std::string wq, long long val)
        {

            wheresql.append(" OR ");

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }
            wq.append(std::to_string(val));

            wheresql.append(wq);
            return *mod;
        }
        model &whereOr(std::string wq, std::string val)
        {

            wheresql.append(" OR ");

            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wq.push_back('=');
            }

            wq.push_back('\'');
            wq.append(val);
            wq.push_back('\'');

            wheresql.append(wq);

            return *mod;
        }
        model &whereIn(std::string k)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            return *mod;
        }
        model &whereIn(std::string k, std::string &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            wheresql.append(a);
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::string a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            wheresql.append(a);
            wheresql.append(") ");
            return *mod;
        }
        model &whereNotIn(std::string k, std::string a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            wheresql.append(a);
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::list<std::string> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" IN (");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",\'");
                }
                else
                {
                    wheresql.append("\'");
                }
                wheresql.append(key);
                wheresql.append("\'");
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereNotIn(std::string k, std::list<std::string> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN (");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",\'");
                }
                else
                {
                    wheresql.append("\'");
                }
                wheresql.append(key);
                wheresql.append("\'");
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::vector<std::string> &a)
        {
            if (!wheresql.empty())
                wheresql.append(" AND ");
            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",\'");
                }
                else
                {
                    wheresql.append("\'");
                }
                wheresql.append(key);
                wheresql.append("\'");
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereNotIn(std::string k, std::vector<std::string> &a)
        {
            if (!wheresql.empty())
                wheresql.append(" AND ");
            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",\'");
                }
                else
                {
                    wheresql.append("\'");
                }
                wheresql.append(key);
                wheresql.append("\'");
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::list<unsigned int> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereNotIn(std::string k, std::list<unsigned int> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::vector<unsigned int> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereNotIn(std::string k, std::vector<unsigned int> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::list<int> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereNotIn(std::string k, std::list<int> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::vector<int> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereNotIn(std::string k, std::vector<int> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::list<unsigned long long> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::vector<unsigned long long> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }

        model &whereIn(std::string k, std::list<long> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &whereIn(std::string k, std::vector<long> &a)
        {
            if (wheresql.empty() || ishascontent)
            {
            }
            else
            {
                wheresql.append(" AND ");
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                wheresql.append(std::to_string(key));
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        model &order(std::string wq)
        {

            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            return *mod;
        }

        model &having(std::string wq)
        {

            groupsql.append(" HAVING by ");
            groupsql.append(wq);
            return *mod;
        }

        model &group(std::string wq)
        {

            groupsql.append(" GROUP BY ");
            groupsql.append(wq);
            return *mod;
        }

        model &orsub()
        {

            if (iskuohao == true)
            {
                iskuohao = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            else
            {
                wheresql.append(" OR (");
                iskuohao = true;
                ishascontent = false;
            }
            return *mod;
        }
        model &andsub()
        {

            if (iskuohao == true)
            {
                iskuohao = false;
                wheresql.append(" )");
                ishascontent = false;
            }
            else
            {
                wheresql.append(" AND (");
                iskuohao = true;
                ishascontent = false;
            }

            return *mod;
        }

        model &startTransaction()
        {
            iscommit = true;
            return *mod;
        }
        model &commit()
        {
            iscommit = false;

            return *mod;
        }
        model &limit(unsigned int num)
        {
            limitsql.append(" limit ");
            limitsql.append(std::to_string(num));
            return *mod;
        }
        model &limit(unsigned int num, unsigned int endnum)
        {
            limitsql.append(" limit ");
            limitsql.append(std::to_string(num));
            limitsql.push_back(',');
            limitsql.append(std::to_string(endnum));
            return *mod;
        }

        std::vector<std::map<std::string, std::string>> fetchObj()
        {
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(base::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }
            
            std::vector<std::map<std::string, std::string>> temprecord;
            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                MYSQL_RES *resultall = nullptr;

                int readnum = mysql_query(conn.get(), &sqlstring[0]);

                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    try
                    {
                        http::back_mysql_connect(dbhash, std::move(conn));
                    }
                    catch (...)
                    {
                    }
                    return temprecord;
                }

                resultall = mysql_store_result(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                }
                readnum = 0;

                int num_fields = mysql_num_fields(resultall);

                MYSQL_FIELD *fields;
                fields = mysql_fetch_fields(resultall);
                std::string type_temp;
                std::vector<std::string> table_fieldname;
                for (unsigned char index = 0; index < num_fields; index++)
                {
                    type_temp = std::string(fields[index].name);
                    std::transform(type_temp.begin(), type_temp.end(), type_temp.begin(), ::tolower);
                    table_fieldname.push_back(type_temp);
                }

                int j=0;
                MYSQL_ROW json_row;
                while ((json_row = mysql_fetch_row(resultall)))
                {
                    std::map<std::string, std::string> rowtemp;
                    for (unsigned char index = 0; index < num_fields; index++)
                    {
                        rowtemp.insert(table_fieldname[index],std::string(json_row[index]));
                    }
                    temprecord.push_back(std::move(rowtemp));
                    j++;
                }
                mysql_free_result(resultall);
                    
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                
            }

            return temprecord;

        }
        std::vector<std::vector<std::string>> fetchRow()
        {
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(base::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }
            
            std::vector<std::vector<std::string>> temprecord;
            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                MYSQL_RES *resultall = nullptr;

                int readnum = mysql_query(conn.get(), &sqlstring[0]);

                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    try
                    {
                        http::back_mysql_connect(dbhash, std::move(conn));
                    }
                    catch (...)
                    {
                    }
                    return temprecord;
                }

                resultall = mysql_store_result(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                }
                readnum = 0;

                int num_fields = mysql_num_fields(resultall);

                MYSQL_FIELD *fields;
                fields = mysql_fetch_fields(resultall);
                std::string type_temp;
                std::vector<std::string> table_fieldname;
                for (unsigned char index = 0; index < num_fields; index++)
                {
                    type_temp = std::string(fields[index].name);
                    std::transform(type_temp.begin(), type_temp.end(), type_temp.begin(), ::tolower);
                    table_fieldname.push_back(type_temp);
                }

                int j=0;
                MYSQL_ROW json_row;
                while ((json_row = mysql_fetch_row(resultall)))
                {
                    std::vector<std::string> rowtemp;
                    for (unsigned char index = 0; index < num_fields; index++)
                    {
                        rowtemp.push_back(std::string(json_row[index]));
                    }
                    temprecord.push_back(std::move(rowtemp));
                    j++;
                }
                mysql_free_result(resultall);
                temprecord.push_back(std::move(table_fieldname));             
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                
            }

            return temprecord;

        }
        model &fetch()
        {
            //unsigned long long sqlhashid = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(base::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }
            base::record.clear();

            // if (cachetime > 0)
            // {
            //     sqlhashid = std::hash<std::string>{}(sqlstring);
            //     if (getcacherecord(sqlhashid))
            //     {
            //         cachetime = 0;
            //         return *mod;
            //     }
            // }

            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                MYSQL_RES *resultall = nullptr;

                int readnum = mysql_query(conn.get(), &sqlstring[0]);

                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    try
                    {
                        http::back_mysql_connect(dbhash, std::move(conn));
                    }
                    catch (...)
                    {
                    }
                    return *mod;
                }

                resultall = mysql_store_result(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                }
                readnum = 0;

                int num_fields = mysql_num_fields(resultall);

                base::_keypos.clear();
                if (selectsql.empty())
                {
                    for (unsigned char index = 0; index < num_fields; index++)
                    {
                        base::_keypos.emplace_back(index);
                    }
                }
                else
                {
                    MYSQL_FIELD *fields;
                    fields = mysql_fetch_fields(resultall);
                    std::string type_temp;
                    for (unsigned char index = 0; index < num_fields; index++)
                    {
                        type_temp = std::string(fields[index].name);
                        base::_keypos.emplace_back(base::findcolpos(type_temp));
                    }
                }

                int j = 0;
                while ((base::_row = mysql_fetch_row(resultall)))
                {
                    if (j == 0)
                    {
                        base::_setColnamevalue();
                    }
                    else
                    {

                        base::_addnewrowvalue();
                    }
                    j++;
                }
                mysql_free_result(resultall);

                return *mod;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return *mod;
            }

            // if (cachetime > 0)
            // {
            //     if (sqlhashid == 0)
            //     {
            //         sqlhashid = std::hash<std::string>{}(sqlstring);
            //     }
            //     savecacherecord(sqlhashid);
            //     cachetime = 0;
            // }
            return *mod;
        }
        bool getcacherecord(unsigned long long cacheid)
        {

            return false;
        }
        bool savecacherecord(unsigned long long cacheid)
        {

            return true;
        }
        http::OBJ_VALUE fetchJson()
        {
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(base::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            http::OBJ_VALUE valuetemp;
            valuetemp.set_array();
            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                MYSQL_RES *resultall = nullptr;

                int readnum = mysql_query(conn.get(), &sqlstring[0]);

                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    try
                    {
                        http::back_mysql_connect(dbhash, std::move(conn));
                    }
                    catch (...)
                    {
                    }
                    return valuetemp;
                }

                resultall = mysql_store_result(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                }
                readnum = 0;

                int num_fields = mysql_num_fields(resultall);

                MYSQL_FIELD *fields;
                fields = mysql_fetch_fields(resultall);
                std::string type_temp;
                std::vector<std::string> table_fieldname;
                for (unsigned char index = 0; index < num_fields; index++)
                {
                    type_temp = std::string(fields[index].name);
                    std::transform(type_temp.begin(), type_temp.end(), type_temp.begin(), ::tolower);
                    table_fieldname.push_back(type_temp);
                }

                
                int j=0;
                MYSQL_ROW json_row;
                while ((json_row = mysql_fetch_row(resultall)))
                {
                    http::OBJ_VALUE rowtemp;
                    rowtemp.set_array();
                    for (unsigned char index = 0; index < num_fields; index++)
                    {
                        rowtemp[table_fieldname[index]]=std::string(json_row[index]);
                    }
                    valuetemp.push(j, std::move(rowtemp));
                    j++;
                }
                mysql_free_result(resultall);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                
            }
            return valuetemp;
        }
        model &getone(long long id)
        {

            sqlstring = "SELECT *  FROM ";

            sqlstring.append(base::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(base::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));
            base::record.clear();

            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                MYSQL_RES *resultone = nullptr;

                int readnum = mysql_query(conn.get(), &sqlstring[0]);

                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    try
                    {
                        http::back_mysql_connect(dbhash, std::move(conn));
                    }
                    catch (...)
                    {
                    }
                    return *mod;
                }

                resultone = mysql_store_result(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                }

                if (!resultone)
                {
                    mysql_free_result(resultone);
                    return *mod;
                }
                readnum = 0;

                readnum = mysql_num_fields(resultone);
                base::_keypos.clear();
                for (unsigned char index = 0; index < readnum; index++)
                {
                    base::_keypos.emplace_back(index);
                }

                if ((base::_row = mysql_fetch_row(resultone)))
                {
                    base::_setColnamevalue();
                }

                mysql_free_result(resultone);

                return *mod;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return *mod;
            }

            return *mod;
        }
        int update()
        {
            if (wheresql.empty())
            {
                return 0;
            }
            sqlstring = base::_makeupdatesql("");
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }
            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                long long readnum = mysql_query(conn.get(), &sqlstring[0]);
                readnum = mysql_affected_rows(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                   
                }
                return readnum;
            }
            catch (...)
            {
                return 0;
            }
           
        }
        int update(std::string fieldname)
        {
            if (wheresql.empty())
            {
                return 0;
            }
            sqlstring = base::_makeupdatesql(fieldname);
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }
            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                long long readnum = mysql_query(conn.get(), &sqlstring[0]);
                readnum = mysql_affected_rows(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                   
                }
                return readnum;
            }
            catch (...)
            {
                return 0;
            }

        }
        int remove()
        {
            sqlstring = "DELETE FROM  ";
            sqlstring.append(base::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }
            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                long long readnum = mysql_query(conn.get(), &sqlstring[0]);
                readnum = mysql_affected_rows(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                   
                }
                return readnum;
            }
            catch (...)
            {
                return 0;
            }

           
        }
        int remove(long long id)
        {
            sqlstring = "DELETE FROM  ";
            sqlstring.append(base::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(base::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));

            try
            {
                std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                long long readnum = mysql_query(conn.get(), &sqlstring[0]);
                readnum = mysql_affected_rows(conn.get());
                try
                {
                    http::back_mysql_connect(dbhash, std::move(conn));
                }
                catch (...)
                {
                   
                }
                return readnum;
            }
            catch (...)
            {
                return 0;
            }
 
        }

        int save()
        {
            if (base::getPK() > 0)
            {
                if (wheresql.empty())
                {
                    return 0;
                }
                sqlstring = base::_makeupdatesql("");
                sqlstring.append(" where ");
                if (wheresql.empty())
                {
                    return 0;
                }
                else
                {
                    sqlstring.append(wheresql);
                }
                if (!groupsql.empty())
                {
                    sqlstring.append(groupsql);
                }
                if (!ordersql.empty())
                {
                    sqlstring.append(ordersql);
                }
                if (!limitsql.empty())
                {
                    sqlstring.append(limitsql);
                }

                try
                {
                    std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                    long long readnum = mysql_query(conn.get(), &sqlstring[0]);

                    if (readnum != 0)
                    {
                        error_msg = std::string(mysql_error(conn.get()));
                        try
                        {
                            http::back_mysql_connect(dbhash, std::move(conn));
                        }
                        catch (...)
                        {
                        }
                        return 0;
                    }
                    readnum = mysql_affected_rows(conn.get());

                    try
                    {
                        http::back_mysql_connect(dbhash, std::move(conn));
                    }
                    catch (...)
                    {
                      
                    }
                    return readnum;
                }
                catch (...)
                {
                    return 0;
                }
            }
            else
            {

                try
                {
                    std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
                    long long readnum = mysql_query(conn.get(), &sqlstring[0]);

                    if (readnum != 0)
                    {
                        error_msg = std::string(mysql_error(conn.get()));
                        base::setPK(0);
                        try
                        {
                            http::back_mysql_connect(dbhash, std::move(conn));
                        }
                        catch (...)
                        {
                        }
                        return 0;
                    }
                    readnum = mysql_affected_rows(conn.get());
                    long long tempid = mysql_insert_id(conn.get());
                    base::setPK(tempid);
                    try
                    {
                        http::back_mysql_connect(dbhash, std::move(conn));
                    }
                    catch (...)
                    {
                    }
                    return readnum;
                }
                catch (const std::exception &e)
                {
                    return 0;
                }
            }
            return 0;
        }
        model &clear(bool both = true)
        {
            selectsql.clear();
            wheresql.clear();
            ordersql.clear();
            groupsql.clear();
            limitsql.clear();
            sqlstring.clear();
            iskuohao = false;
            ishascontent = false;
            if (both)
            {
                typename base::meta newdata;
                base::data = newdata;
            }
            return *mod;
        }

        model &setData(typename base::meta indata)
        {
            base::data = indata;
            return *mod;
        }
        model &get()
        {
             
            return *mod;
        }    
    public:
        std::string selectsql;
        std::string wheresql;
        std::string ordersql;
        std::string groupsql;
        std::string limitsql;
        std::string sqlstring;
        std::string dbtag;
        std::string error_msg;
        bool iskuohao = false;
        bool iscommit = false;
        bool ishascontent = false;
        size_t dbhash;
        model *mod;
        unsigned int offsetbegin = 0, offsetend = 0;

        std::hash<std::string> hash_fn;
        unsigned int cachetime = 0;
    };

}
#endif