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
#include <mysqlx/xdevapi.h>
#include <typeinfo>
#include <memory>
#include <list>
#include <queue>
#include <condition_variable>
#include "mysqlproxyfun.h"

namespace orm
{
    //通用操作 类 mysql 方法 在这里接上getSession(); 这里可以操作 data sql count page
    // using namespace http;
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

            mysqlx::RowResult ress = http::domysqlexecute(countsql, dbhash);
            if (ress.count() > 0)
            {
                mysqlx::Row one = ress.fetchOne();
                return one[0];
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
            mysqlx::SqlResult ress = http::domysqleditexecute(countsql, dbhash);
            return ress.getAffectedItemsCount();
        }

        model &select(std::string fieldname)
        {
            selectsql = fieldname;
            return *mod;
        }
        model &cache(unsigned int livetime)
        {
            cachetime = livetime;
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

        std::list<mysqlx::Row> fetchAll()
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
            base::record.clear();
            res = http::domysqlexecute(sqlstring, dbhash);
            return res.fetchAll();
        }
        mysqlx::RowResult fetchResult()
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
            base::record.clear();
            return http::domysqlexecute(sqlstring, dbhash);
        }
        model &fetch()
        {
            unsigned long long sqlhashid = 0;
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

            if (cachetime > 0)
            {
                sqlhashid = std::hash<std::string>{}(sqlstring);
                if (getcacherecord(sqlhashid))
                {
                    cachetime = 0;
                    return *mod;
                }
            }
            res = http::domysqlexecute(sqlstring, dbhash);
            const mysqlx::Columns &columns = res.getColumns();
            base::_keypos.clear();
            if (selectsql.empty())
            {
                for (unsigned char index = 0; index < res.getColumnCount(); index++)
                {
                    base::_keypos.emplace_back(index);
                }
            }
            else
            {
                for (unsigned char index = 0; index < res.getColumnCount(); index++)
                {
                    base::_keypos.emplace_back(base::findcolpos(columns[index].getColumnName()));
                }
            }

            int j = 0;
            while ((base::_row = res.fetchOne()))
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
            if (cachetime > 0)
            {
                if (sqlhashid == 0)
                {
                    sqlhashid = std::hash<std::string>{}(sqlstring);
                }
                savecacherecord(sqlhashid);
                cachetime = 0;
            }
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
        http::OBJ_VALUE fetchOBJ()
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
            base::record.clear();
            res = http::domysqlexecute(sqlstring, dbhash);
            const mysqlx::Columns &columns = res.getColumns();
            base::_keypos.clear();
            http::OBJ_VALUE valuetemp;
            std::vector<std::string> colnametemp;
            for (unsigned char index = 0; index < res.getColumnCount(); index++)
            {
                colnametemp.emplace_back(columns[index].getColumnName());
            }

            int j = 0;
            valuetemp.set_array();
            std::string tempraw;
            while ((base::_row = res.fetchOne()))
            {
                http::OBJ_VALUE rowtemp;

                for (unsigned char i = 0; i < res.getColumnCount(); i++)
                {
                    switch (base::_row[i].getType())
                    {
                    case mysqlx::Value::Type::UINT64:
                        rowtemp[colnametemp[i]] = (uint64_t)base::_row[i];
                        break;
                    case mysqlx::Value::Type::INT64:
                        rowtemp[colnametemp[i]] = (int64_t)base::_row[i];
                        break;
                    case mysqlx::Value::Type::FLOAT:
                        rowtemp[colnametemp[i]] = (double)base::_row[i];
                        break;
                    case mysqlx::Value::Type::DOUBLE:
                        rowtemp[colnametemp[i]] = (double)base::_row[i];
                        break;
                    case mysqlx::Value::Type::STRING:
                        rowtemp[colnametemp[i]] = (std::string)base::_row[i];
                        break;
                    case mysqlx::Value::Type::BOOL:
                        rowtemp[colnametemp[i]] = (bool)base::_row[i];
                        break;
                    case mysqlx::Value::Type::RAW:
                        tempraw.clear();
                        for (const mysqlx::byte *ptr = base::_row[i].getRawBytes().begin(); ptr < base::_row[i].getRawBytes().end(); ++ptr)
                        {
                            tempraw.push_back(*ptr);
                        }

                        {
                            unsigned int tdtime = base::findcolpos(colnametemp[i]);
                            if (tdtime < 200)
                            {
                                if (base::colnamestype[tdtime] == 60 && tempraw.size() > 5)
                                {
                                    std::ostringstream datetime;
                                    tdtime = (unsigned char)tempraw[0] - 128 + (unsigned char)tempraw[1] * 128;
                                    datetime << std::to_string(tdtime);
                                    if ((unsigned char)tempraw[2] < 10)
                                    {
                                        datetime << "-0" << std::to_string(tempraw[2]);
                                    }
                                    else
                                    {
                                        datetime << "-" << std::to_string(tempraw[2]);
                                    }
                                    if ((unsigned char)tempraw[3] < 10)
                                    {
                                        datetime << "-0" << std::to_string(tempraw[3]);
                                    }
                                    else
                                    {
                                        datetime << "-" << std::to_string(tempraw[3]);
                                    }

                                    if ((unsigned char)tempraw[4] < 10)
                                    {
                                        datetime << " 0" << std::to_string(tempraw[4]);
                                    }
                                    else
                                    {
                                        datetime << " " << std::to_string(tempraw[4]);
                                    }
                                    if ((unsigned char)tempraw[5] < 10)
                                    {
                                        datetime << ":0" << std::to_string(tempraw[5]);
                                    }
                                    else
                                    {
                                        datetime << ":" << std::to_string(tempraw[5]);
                                    }
                                    if ((unsigned char)tempraw[6] < 10)
                                    {
                                        datetime << ":0" << std::to_string(tempraw[6]);
                                    }
                                    else
                                    {
                                        datetime << ":" << std::to_string(tempraw[6]);
                                    }
                                    rowtemp[colnametemp[i]] = datetime.str();
                                }
                                else if (base::colnamestype[tdtime] == 61 && tempraw.size() > 3)
                                {
                                    std::ostringstream datetime;
                                    tdtime = (unsigned char)tempraw[0] - 128 + (unsigned char)tempraw[1] * 128;
                                    datetime << std::to_string(tdtime);
                                    if ((unsigned char)tempraw[2] < 10)
                                    {
                                        datetime << "-0" << std::to_string(tempraw[2]);
                                    }
                                    else
                                    {
                                        datetime << "-" << std::to_string(tempraw[2]);
                                    }
                                    if ((unsigned char)tempraw[3] < 10)
                                    {
                                        datetime << "-0" << std::to_string(tempraw[3]);
                                    }
                                    else
                                    {
                                        datetime << "-" << std::to_string(tempraw[3]);
                                    }

                                    rowtemp[colnametemp[i]] = datetime.str();
                                }
                                else
                                {
                                    rowtemp[colnametemp[i]] = tempraw;
                                }
                            }
                        }

                        break;
                    default:
                        rowtemp[colnametemp[i]] = "";
                    }
                }
                valuetemp.push(j, std::move(rowtemp));
                j++;
            }

            return std::move(valuetemp);
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
            res = http::domysqlexecute(sqlstring, dbhash);

            const mysqlx::Columns &columns = res.getColumns();
            base::_keypos.clear();
            for (unsigned char index = 0; index < res.getColumnCount(); index++)
            {
                base::_keypos.emplace_back(index);
            }

            int j = 0;
            while ((base::_row = res.fetchOne()))
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

            ses = http::domysqleditexecute(sqlstring, dbhash);
            return ses.getAffectedItemsCount();
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

            ses = http::domysqleditexecute(sqlstring, dbhash);
            return ses.getAffectedItemsCount();
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
            ses = http::domysqleditexecute(sqlstring, dbhash);
            return ses.getAffectedItemsCount();
        }
        int remove(long long id)
        {
            sqlstring = "DELETE FROM  ";
            sqlstring.append(base::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(base::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));

            ses = http::domysqleditexecute(sqlstring, dbhash);
            return ses.getAffectedItemsCount();
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
                ses = http::domysqleditexecute(sqlstring, dbhash);
                return ses.getAffectedItemsCount();
            }
            else
            {
                sqlstring = base::_makeinsertsql();
                ses = http::domysqleditexecute(sqlstring, dbhash);
                    base::setPK(ses.getAutoIncrementValue());
                    return ses.getAutoIncrementValue();    
                // if (iscommit)
                // {

                // }
                // else
                // {
                    
                // }
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

        mysqlx::RowResult dofetch(std::string sql)
        {
            return http::domysqlexecute(sql, dbhash);
        }
        mysqlx::SqlResult doexecute(std::string sql)
        {
            return http::domysqleditexecute(sql, dbhash);
        }

    public:
        std::string selectsql;
        std::string wheresql;
        std::string ordersql;
        std::string groupsql;
        std::string limitsql;
        std::string sqlstring;
        std::string dbtag;
        bool iskuohao = false;
        bool iscommit = false;
        bool ishascontent = false;
        size_t dbhash;
        model *mod;
        unsigned int offsetbegin = 0, offsetend = 0;

        std::hash<std::string> hash_fn;
        mysqlx::RowResult res;
        mysqlx::SqlResult ses;

        unsigned int cachetime = 0;
    };

}
#endif