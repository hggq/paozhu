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
#include "datetime.h"
#include <stdexcept>
#include <iostream>
#include <functional>
#include <tuple>
#include "mysql.h"
#include <typeinfo>
#include <memory>
#include <list>
#include <queue>
#include <cmath>
#include <algorithm>
#include <condition_variable>
#include <sstream>
#include "mysqlpool.h"
namespace orm
{
// mysql 操作数据中间件
// page
// using namespace http;
typedef std::vector<std::pair<std::string, std::function<void(long long, long long)>>> commit_lists_callback;

template <typename BASE_T>
std::map<std::size_t, BASE_T> &get_static_model_cache()
{
    static std::map<std::size_t, BASE_T> instance;
    return instance;
}

template <typename BASE_MODEL>
class model_meta_cache
{
  private:
    model_meta_cache() {};
    ~model_meta_cache() {};
    model_meta_cache(const model_meta_cache &);
    model_meta_cache &operator=(const model_meta_cache &);

  public:
    struct data_cache_t
    {
        std::vector<BASE_MODEL> data;
        unsigned int exptime = 0;
    };

  public:
    void save(std::size_t hashid, BASE_MODEL &data_list, int expnum = 0, bool cover_data = false)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        struct data_cache_t temp;
        temp.data.push_back(data_list);
        if (expnum != 0)
        {
            temp.exptime = http::timeid() + expnum;
        }
        else
        {
            temp.exptime = 0;
        }
        std::unique_lock<std::mutex> lock(editlock);
        auto [_, success] = obj.insert({hashid, temp});
        if (!success)
        {
            if (cover_data)
            {
                obj[hashid] = temp;
            }
            else
            {
                obj[hashid].exptime = temp.exptime;
            }
        }
    }
    void save(std::size_t hashid, std::vector<BASE_MODEL> &data_list, int expnum = 0, bool cover_data = false)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        struct data_cache_t temp;
        temp.data = data_list;
        if (expnum != 0)
        {
            temp.exptime = http::timeid() + expnum;
        }
        else
        {
            temp.exptime = 0;
        }
        std::unique_lock<std::mutex> lock(editlock);
        auto [_, success] = obj.insert({hashid, temp});
        if (!success)
        {
            if (cover_data)
            {
                obj[hashid] = temp;
            }
            else
            {
                obj[hashid].exptime = temp.exptime;
            }
        }
    }
    bool remove(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            obj.erase(iter++);
            return true;
        }
        return false;
    }
    void remove_exptime()
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid();
        std::unique_lock<std::mutex> lock(editlock);
        for (auto iter = obj.begin(); iter != obj.end();)
        {
            if (iter->second.exptime == 0)
            {
                continue;
            }
            if (iter->second.exptime < nowtime)
            {
                obj.erase(iter++);
            }
        }
    }
    void clear()
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        std::unique_lock<std::mutex> lock(editlock);
        obj.clear();
    }
    int check(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime == 0)
            {
                return 0;
            }
            return ((int)(iter->second.exptime - nowtime));
        }
        return -1;
    }

    int update(std::size_t hashid, int exptime = 0)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid() + exptime;
        if (exptime == 0)
        {
            nowtime = 0;
        }
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime == 0)
            {
                iter->second.exptime = nowtime;
                return 0;
            }
            iter->second.exptime = nowtime;
            return 1;
        }
        return -1;
    }
    std::vector<BASE_MODEL> get(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime == 0)
            {
                return iter->second.data;
            }

            if (iter->second.exptime >= nowtime)
            {
                return iter->second.data;
            }
            else
            {
                obj.erase(iter++);
            }
        }
        lock.unlock();
        std::vector<BASE_MODEL> temp;
        return temp;
    }
    BASE_MODEL get_obj(std::size_t hashid)
    {
        std::map<std::size_t, data_cache_t> &obj = get_static_model_cache<data_cache_t>();
        unsigned int nowtime                     = http::timeid();
        std::unique_lock<std::mutex> lock(editlock);
        auto iter = obj.find(hashid);
        if (iter != obj.end())
        {
            if (iter->second.exptime == 0)
            {
                if (iter->second.data.size() > 0)
                {
                    return iter->second.data[0];
                }
            }

            if (iter->second.exptime >= nowtime)
            {
                if (iter->second.data.size() > 0)
                {
                    return iter->second.data[0];
                }
            }
            else
            {
                obj.erase(iter++);
            }
        }
        lock.unlock();
        BASE_MODEL temp;
        return temp;
    }
    static model_meta_cache &getinstance()
    {
        static model_meta_cache instance;
        return instance;
    }

  public:
    std::mutex editlock;
};

class get_orm_client
{
  public:
    get_orm_client(const std::string &tag)
    {
        dbhash                                                              = std::hash<std::string>{}(tag);
        db_type                                                             = 0;// 0 Mysql 1 MariaDB 2 PostgreSQL 3 SQLite
        std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &myconn = http::get_mysqlpool();
        auto iter                                                           = myconn.find(dbhash);
        if (iter == myconn.end())
        {
            error_msg = "not find orm link tag in pool";
            iserror   = true;
        }
        else
        {
            linkconn = iter->second;
        }
    }
    bool switchDB(const std::string &tag)
    {
        dbhash                                                              = std::hash<std::string>{}(tag);
        db_type                                                             = 0;// 0 Mysql 1 MariaDB 2 PostgreSQL 3 SQLite
        std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &myconn = http::get_mysqlpool();
        auto iter                                                           = myconn.find(dbhash);
        if (iter == myconn.end())
        {
            error_msg = "not find orm link tag in pool";
            iserror   = true;
            return false;
        }
        else
        {
            linkconn = iter->second;
        }
        return true;
    }
    std::tuple<std::vector<std::string>, std::vector<std::vector<std::string>>> query(const std::string &rawsql,
                                                                                      bool isedit = false)
    {

        std::vector<std::vector<std::string>> temprecord;
        std::vector<std::string> table_fieldname;
        if (iserror)
        {
            return std::make_tuple(table_fieldname, temprecord);
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        if (isedit)
        {
            try
            {
                conn = linkconn->get_edit_connect();
            }
            catch (const char *e)
            {
                iserror   = true;
                error_msg = std::string(e);
                return std::make_tuple(table_fieldname, temprecord);
            }
        }
        else
        {
            try
            {
                conn = linkconn->get_select_connect();
            }
            catch (const char *e)
            {
                iserror   = true;
                error_msg = std::string(e);
                return std::make_tuple(table_fieldname, temprecord);
            }
        }
        try
        {
            MYSQL_RES *resultall = nullptr;
            //mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &rawsql[0], rawsql.size());

            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                try
                {
                    if (isedit)
                    {
                        linkconn->back_edit_connect(std::move(conn));
                    }
                    else
                    {
                        linkconn->back_select_connect(std::move(conn));
                    }
                }
                catch (...)
                {
                }
                return std::make_tuple(table_fieldname, temprecord);
            }

            resultall = mysql_store_result(conn.get());

            readnum = 0;

            int num_fields = mysql_num_fields(resultall);

            MYSQL_FIELD *fields;
            fields = mysql_fetch_fields(resultall);
            std::string type_temp;

            for (unsigned char index = 0; index < num_fields; index++)
            {
                type_temp = std::string(fields[index].name);
                std::transform(type_temp.begin(), type_temp.end(), type_temp.begin(), ::tolower);
                table_fieldname.push_back(type_temp);
            }

            // int j = 0;
            MYSQL_ROW json_row;
            while ((json_row = mysql_fetch_row(resultall)))
            {
                std::vector<std::string> rowtemp;
                for (unsigned char index = 0; index < num_fields; index++)
                {
                    if (json_row[index] != NULL)
                    {
                        rowtemp.push_back(std::string(json_row[index]));
                    }
                    else
                    {
                        rowtemp.push_back("");
                    }
                }
                temprecord.push_back(std::move(rowtemp));
                // j++;
            }
            mysql_free_result(resultall);
            try
            {
                if (isedit)
                {
                    linkconn->back_edit_connect(std::move(conn));
                }
                else
                {
                    linkconn->back_select_connect(std::move(conn));
                }
            }
            catch (...)
            {
            }
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            iserror   = true;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            iserror   = true;
        }
        catch (...)
        {
        }
        return std::make_tuple(table_fieldname, temprecord);
    }
    long long query_edit(const std::string &rawsql, bool isinsert = false)
    {
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        if (iserror)
        {
            return 0;
        }
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            iserror   = true;
            return 0;
        }
        try
        {
            //mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &rawsql[0], rawsql.size());
            readnum           = mysql_affected_rows(conn.get());
            if (isinsert)
            {
                readnum = mysql_insert_id(conn.get());
            }
            linkconn->back_select_connect(std::move(conn));
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            iserror   = true;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            iserror   = true;
        }
        catch (...)
        {
        }
        return 0;
    }
    void error_clear()
    {
        error_msg.clear();
        iserror = false;
    }
    bool is_error()
    {
        return iserror;
    }

  public:
    std::size_t dbhash;
    std::string error_msg;
    unsigned char db_type;
    bool iserror = false;
    std::shared_ptr<http::mysqllinkpool> linkconn;
};

template <typename model, typename base>
class mysqlclientDB : public base
{
  public:
    mysqlclientDB(const std::string &tag) : dbtag(tag)
    {
        // dbhash = hash_fn(dbtag);
        dbhash                                                              = std::hash<std::string>{}(dbtag);
        mod                                                                 = &static_cast<model &>(*this);
        std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &myconn = http::get_mysqlpool();
        auto iter                                                           = myconn.find(dbhash);
        if (iter == myconn.end())
        {
            error_msg = "not find orm link tag in pool";
            iserror   = true;
        }
        else
        {
            linkconn = iter->second;
        }
    }
    mysqlclientDB() : dbtag(base::_rmstag)
    {

        dbhash                                                              = std::hash<std::string>{}(dbtag);
        mod                                                                 = &static_cast<model &>(*this);
        std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &myconn = http::get_mysqlpool();
        auto iter                                                           = myconn.find(dbhash);
        if (iter == myconn.end())
        {
            error_msg = "not find orm link tag in pool";
            iserror   = true;
        }
        else
        {
            linkconn = iter->second;
        }
    }
    model &switchDB(const std::string &temptag)
    {
        dbhash                                                              = std::hash<std::string>{}(temptag);
        std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &myconn = http::get_mysqlpool();
        auto iter                                                           = myconn.find(dbhash);
        if (iter == myconn.end())
        {
            error_msg = "not find orm link tag in pool";
            iserror   = true;
        }
        else
        {
            linkconn = iter->second;
        }
    }
    model &set_table(const std::string &table_name)
    {
        if (original_tablename.empty())
        {
            original_tablename = base::tablename;
        }
        if (table_name.size() > 0)
        {
            base::tablename = table_name;
        }
        return *mod;
    }
    model &reset_table()
    {
        if (original_tablename.empty())
        {
            return *mod;
        }
        base::tablename = original_tablename;
        return *mod;
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

        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_select_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }

        try
        {
            MYSQL_RES *resultone = nullptr;
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &countsql[0], countsql.size());

            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                linkconn->back_select_connect(std::move(conn));
                return 0;
            }

            resultone = mysql_store_result(conn.get());
            linkconn->back_select_connect(std::move(conn));

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
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }

        return 0;
    }
    std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>
    page(unsigned int page, unsigned int per_page = 10, unsigned int list_num = 5)
    {
        unsigned int total_page = count();
        if (per_page == 0)
        {
            per_page = 10;
        }
        if (list_num < 1)
        {
            list_num = 1;
        }
        total_page = std::ceil((float)total_page / per_page);

        if (total_page < 1)
        {
            total_page = 1;
        }
        if (page > total_page)
        {
            page = total_page;
        }
        if (page < 1)
        {
            page = 1;
        }
        unsigned int mid_num  = std::floor(list_num / 2);
        unsigned int last_num = list_num - 1;

        int temp_num = page - mid_num;

        unsigned int minpage = temp_num < 1 ? 1 : temp_num;
        unsigned int maxpage = minpage + last_num;

        if (maxpage > total_page)
        {
            maxpage  = total_page;
            temp_num = (maxpage - last_num);
            if (temp_num < 1)
            {
                minpage = 1;
            }
            else
            {
                minpage = temp_num;
            }
        }
        limit((page - 1) * per_page, per_page);
        return std::make_tuple(minpage, maxpage, page, total_page);
    }
    int update_col(std::string colname, int num)
    {
        effect_num = 0;
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
            if (base::getPK() > 0)
            {
                std::ostringstream tempsql;
                tempsql << " ";
                tempsql << base::getPKname();
                tempsql << " = '";
                tempsql << base::getPK();
                tempsql << "' ";
                countsql.append(tempsql.str());
            }
            else
            {
                return 0;
            }
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
        if (iscommit)
        {
            iscommit = false;
            return 0;
        }

        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &countsql[0], countsql.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return 0;
            }
            readnum    = mysql_affected_rows(conn.get());
            effect_num = readnum;
            linkconn->back_edit_connect(std::move(conn));

            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }
    }

    model &select(const std::string &fieldname)
    {
        if (selectsql.size() > 0)
        {
            selectsql.push_back(',');
        }
        selectsql.append(fieldname);
        return *mod;
    }

    model &where(const std::string &wq)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }

        wheresql.append(wq);
        return *mod;
    }
    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &where(const std::string &wq, _SQL_Value val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        char bi = wq.back();
        if (bi == '=' || bi == '>' || bi == '<')
        {
        }
        else
        {
            wheresql.push_back('=');
        }

        std::stringstream _stream;
        _stream << val;
        wheresql.append(_stream.str());
        return *mod;
    }

    model &where(const std::string &wq, char bi, http::OBJ_VALUE &obj)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        wheresql.push_back(bi);
        if (obj.is_string())
        {
            wheresql.push_back('\'');
            wheresql.append(obj.as_string());
            wheresql.push_back('\'');
        }
        else
        {

            wheresql.append(obj.to_string());
        }
        return *mod;
    }
    model &where(const std::string &wq, http::OBJ_VALUE &obj)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        char bi = wq.back();
        if (bi == '=' || bi == '>' || bi == '<')
        {
        }
        else
        {
            wheresql.push_back('=');
        }

        if (obj.is_string())
        {
            wheresql.push_back('\'');
            wheresql.append(obj.as_string());
            wheresql.push_back('\'');
        }
        else
        {

            wheresql.append(obj.to_string());
        }
        return *mod;
    }
    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &where(const std::string &wq, char bi, _SQL_Value val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        wheresql.push_back(bi);
        std::stringstream _stream;
        _stream << val;
        wheresql.append(_stream.str());
        return *mod;
    }

    model &where(const std::string &wq, char bi, const std::string &val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        wheresql.push_back(bi);
        wheresql.push_back('\'');

        wheresql.append(val);
        wheresql.push_back('\'');
        return *mod;
    }
    model &where(const std::string &wq, const std::string &val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        char bi = wq.back();
        if (bi == '=' || bi == '>' || bi == '<')
        {
        }
        else
        {
            wheresql.push_back('=');
        }

        wheresql.push_back('\'');
        wheresql.append(val);
        wheresql.push_back('\'');
        return *mod;
    }

    model &between(const std::string &wq, const std::string &a, const std::string &b)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" (");
        wheresql.append(wq);
        wheresql.append(" BETWEEN '");
        std::stringstream _stream;
        _stream << a;
        _stream << "' AND '";
        _stream << b;
        _stream << "' ) ";
        wheresql.append(_stream.str());
        return *mod;
    }
    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &between(const std::string &wq, _SQL_Value a, _SQL_Value b)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" (");
        wheresql.append(wq);
        wheresql.append(" BETWEEN ");
        std::stringstream _stream;
        _stream << a;
        _stream << " AND ";
        _stream << b;
        _stream << " ) ";
        wheresql.append(_stream.str());
        return *mod;
    }
    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &orBetween(const std::string &wq, _SQL_Value a, _SQL_Value b)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" (");
        wheresql.append(wq);
        wheresql.append(" BETWEEN ");
        std::stringstream _stream;
        _stream << a;
        _stream << " AND ";
        _stream << b;
        _stream << " ) ";
        wheresql.append(_stream.str());
        return *mod;
    }
    model &whereLike(const std::string &wq, const std::string &val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        wheresql.append(" like '");
        if (val.size() > 0 && (val[0] == '%' || val.back() == '%'))
        {
            wheresql.append(val);
            wheresql.append("' ");
        }
        else
        {
            wheresql.push_back('%');
            wheresql.append(val);
            wheresql.append("%' ");
        }
        return *mod;
    }
    model &whereLikeLeft(const std::string &wq, const std::string &val)
    {

        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        wheresql.append(" like '");
        wheresql.push_back('%');
        wheresql.append(val);
        wheresql.append("' ");
        return *mod;
    }
    model &whereLikeRight(const std::string &wq, const std::string &val)
    {

        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        wheresql.append(" like '");
        wheresql.append(val);
        wheresql.append("%' ");
        return *mod;
    }
    model &whereOrLike(const std::string &wq, const std::string &val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        wheresql.append(" like '");
        if (val[0] == '%' || val.back() == '%')
        {
            wheresql.append(val);
            wheresql.append("' ");
        }
        else
        {
            wheresql.push_back('%');
            wheresql.append(val);
            wheresql.append("%' ");
        }
        return *mod;
    }
    model &whereAnd(const std::string &wq)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        return *mod;
    }
    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &whereAnd(const std::string &wq, _SQL_Value val)
    {

        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        char bi = wq.back();
        if (bi == '=' || bi == '>' || bi == '<')
        {
        }
        else
        {
            wheresql.push_back('=');
        }
        std::stringstream _stream;
        _stream << val;
        wheresql.append(_stream.str());
        return *mod;
    }
    model &whereAnd(const std::string &wq, const std::string &val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        char bi = wq.back();
        if (bi == '=' || bi == '>' || bi == '<')
        {
        }
        else
        {
            wheresql.push_back('=');
        }

        wheresql.push_back('\'');
        wheresql.append(val);
        wheresql.push_back('\'');

        return *mod;
    }
    model &whereOr(const std::string &wq)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        return *mod;
    }
    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &whereOr(const std::string &wq, _SQL_Value val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        char bi = wq.back();
        if (bi == '=' || bi == '>' || bi == '<')
        {
        }
        else
        {
            wheresql.push_back('=');
        }
        std::stringstream _stream;
        _stream << val;
        wheresql.append(_stream.str());
        return *mod;
    }
    model &whereOr(const std::string &wq, const std::string &val)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(wq);
        char bi = wq.back();
        if (bi == '=' || bi == '>' || bi == '<')
        {
        }
        else
        {
            wheresql.push_back('=');
        }

        wheresql.push_back('\'');
        wheresql.append(val);
        wheresql.push_back('\'');
        return *mod;
    }
    model &whereIn(const std::string &k)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(k);
        return *mod;
    }
    model &whereIn(const std::string &k, const std::string &a)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }

        wheresql.append(k);
        wheresql.append(" IN(");
        wheresql.append(a);
        wheresql.append(") ");
        return *mod;
    }

    model &whereIn(const std::string &k, const std::vector<std::string> &a)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
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
    model &whereNotIn(const std::string &k, const std::vector<std::string> &a)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
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
    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &whereIn(const std::string &k, const std::list<_SQL_Value> &a)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }

        wheresql.append(k);
        wheresql.append(" in(");
        int i = 0;
        std::stringstream _stream;
        for (auto &key : a)
        {
            if (i > 0)
            {
                wheresql.append(",");
            }
            _stream << key;
            wheresql.append(_stream.str());
            i++;
            _stream.str("");
        }
        wheresql.append(") ");
        return *mod;
    }

    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &whereIn(const std::string &k, const std::vector<_SQL_Value> &a)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }

        wheresql.append(k);
        wheresql.append(" IN(");
        int i = 0;
        std::stringstream _stream;
        for (auto &key : a)
        {
            if (i > 0)
            {
                wheresql.append(",");
            }
            _stream << key;
            wheresql.append(_stream.str());
            i++;
            _stream.str("");
        }
        wheresql.append(") ");
        return *mod;
    }
    template <typename _SQL_Value>
        requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
    model &whereNotIn(const std::string &k, const std::vector<_SQL_Value> &a)
    {
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }

        wheresql.append(k);
        wheresql.append(" NOT IN(");
        int i = 0;
        std::stringstream _stream;
        for (auto &key : a)
        {
            if (i > 0)
            {
                wheresql.append(",");
            }
            _stream << key;
            wheresql.append(_stream.str());
            i++;
            _stream.str("");
        }
        wheresql.append(") ");
        return *mod;
    }

    model &order(const std::string &wq)
    {
        ordersql.append(" ORDER by ");
        ordersql.append(wq);
        return *mod;
    }
    model &asc(const std::string &wq)
    {

        ordersql.append(" ORDER by ");
        ordersql.append(wq);
        ordersql.append(" ASC ");
        return *mod;
    }

    model &desc(const std::string &wq)
    {

        ordersql.append(" ORDER by ");
        ordersql.append(wq);
        ordersql.append(" DESC ");
        return *mod;
    }

    model &having(const std::string &wq)
    {

        groupsql.append(" HAVING by ");
        groupsql.append(wq);
        return *mod;
    }

    model &group(const std::string &wq)
    {

        groupsql.append(" GROUP BY ");
        groupsql.append(wq);
        return *mod;
    }

    model &orsub()
    {

        if (iskuohao == true)
        {
            iskuohao     = false;
            ishascontent = false;
            wheresql.append(" )");
        }
        else
        {
            wheresql.append(" OR (");
            iskuohao     = true;
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
            iskuohao     = true;
            ishascontent = false;
        }

        return *mod;
    }

    model &endsub()
    {
        if (iskuohao == true)
        {
            iskuohao     = false;
            ishascontent = false;
            wheresql.append(" )");
        }
        return *mod;
    }

    model &limit(unsigned int num)
    {
        limitsql.clear();
        limitsql.append(" limit ");
        limitsql.append(std::to_string(num));
        return *mod;
    }
    model &limit(unsigned int num, unsigned int endnum)
    {
        limitsql.clear();
        limitsql.append(" limit ");
        limitsql.append(std::to_string(num));
        limitsql.push_back(',');
        limitsql.append(std::to_string(endnum));
        return *mod;
    }

    std::vector<std::map<std::string, std::string>> fetch_obj()
    {
        effect_num = 0;
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

        if (iserror)
        {
            return temprecord;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_select_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return temprecord;
        }
        try
        {
            MYSQL_RES *resultall = nullptr;
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());

            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                try
                {
                    linkconn->back_select_connect(std::move(conn));
                }
                catch (...)
                {
                }
                return temprecord;
            }

            resultall = mysql_store_result(conn.get());
            try
            {
                linkconn->back_select_connect(std::move(conn));
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

            //int j = 0;
            MYSQL_ROW json_row;

            while ((json_row = mysql_fetch_row(resultall)))
            {
                std::map<std::string, std::string> rowtemp;
                for (unsigned char index = 0; index < num_fields; index++)
                {
                    if (json_row[index] != NULL)
                    {
                        rowtemp.insert(table_fieldname[index], std::string(json_row[index]));
                    }
                    else
                    {
                        rowtemp.insert(table_fieldname[index], std::string(""));
                    }
                }
                temprecord.push_back(std::move(rowtemp));
                effect_num++;
            }
            mysql_free_result(resultall);
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return temprecord;
        }
        catch (...)
        {
        }
        return temprecord;
    }
    std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>
    fetch_row()
    {
        effect_num = 0;
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
        std::vector<std::string> table_fieldname;
        std::map<std::string, unsigned int> table_fieldmap;

        if (iscache)
        {
            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);

            model_meta_cache<std::vector<std::string>> &temp_cache =
                model_meta_cache<std::vector<std::string>>::getinstance();
            temprecord = temp_cache.get(sqlhashid);
            if (temprecord.size() > 0)
            {
                iscache                                    = false;
                model_meta_cache<std::string> &table_cache = model_meta_cache<std::string>::getinstance();
                table_fieldname                            = table_cache.get(sqlhashid);

                model_meta_cache<std::map<std::string, unsigned int>> &tablemap_cache =
                    model_meta_cache<std::map<std::string, unsigned int>>::getinstance();
                table_fieldmap = tablemap_cache.get_obj(sqlhashid);

                return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }
        }

        if (iserror)
        {
            return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_select_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }
        try
        {
            MYSQL_RES *resultall = nullptr;
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());

            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }

            resultall = mysql_store_result(conn.get());
            try
            {
                linkconn->back_select_connect(std::move(conn));
            }
            catch (...)
            {
            }
            readnum = 0;

            int num_fields = mysql_num_fields(resultall);

            MYSQL_FIELD *fields;
            fields = mysql_fetch_fields(resultall);
            std::string type_temp;

            for (unsigned char index = 0; index < num_fields; index++)
            {
                type_temp = std::string(fields[index].name);
                std::transform(type_temp.begin(), type_temp.end(), type_temp.begin(), ::tolower);
                table_fieldname.push_back(type_temp);
                table_fieldmap.insert({type_temp, index});
            }

            //int j = 0;
            MYSQL_ROW json_row;

            while ((json_row = mysql_fetch_row(resultall)))
            {
                std::vector<std::string> rowtemp;
                for (unsigned char index = 0; index < num_fields; index++)
                {
                    if (json_row[index] != NULL)
                    {
                        rowtemp.push_back(std::string(json_row[index]));
                    }
                    else
                    {
                        rowtemp.push_back("");
                    }
                }
                temprecord.push_back(std::move(rowtemp));
                effect_num++;
            }
            mysql_free_result(resultall);

            if (iscache)
            {
                if (exptime > 0)
                {
                    if (temprecord.size() > 0)
                    {
                        std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);

                        model_meta_cache<std::vector<std::string>> &temp_cache =
                            model_meta_cache<std::vector<std::string>>::getinstance();
                        temp_cache.save(sqlhashid, temprecord, exptime);

                        exptime += 1;
                        model_meta_cache<std::string> &table_cache = model_meta_cache<std::string>::getinstance();
                        table_cache.save(sqlhashid, table_fieldname, exptime);

                        model_meta_cache<std::map<std::string, unsigned int>> &tablemap_cache =
                            model_meta_cache<std::map<std::string, unsigned int>>::getinstance();
                        tablemap_cache.save(sqlhashid, table_fieldmap, exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
        }
        catch (...)
        {
        }
        return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
    }
    model &fetch()
    {
        effect_num = 0;
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

        if (iscache)
        {
            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
            if (get_cacherecord(sqlhashid))
            {
                iscache = false;
                return *mod;
            }
        }

        base::record_reset();
        if (iserror)
        {
            return *mod;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_select_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return *mod;
        }

        try
        {
            MYSQL_RES *resultall    = nullptr;
            unsigned int num_fields = 0;
            error_msg.clear();
            for (unsigned int i = 0; i < 2; i++)
            {
                num_fields = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
                if (num_fields == 2013)
                {
                    if (error_msg.size() > 0)
                    {
                        return *mod;
                    }
                    error_msg = std::string(mysql_error(conn.get()));
                    mysql_close(conn.get());
                    conn.reset();
                    try
                    {
                        conn = linkconn->add_select_connect();
                    }
                    catch (const char *e)
                    {
                        error_msg = std::string(e);
                        return *mod;
                    }
                    continue;
                }
                else if (num_fields != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    mysql_close(conn.get());
                    conn.reset();
                    return *mod;
                }
                break;
            }

            resultall = mysql_store_result(conn.get());
            linkconn->back_select_connect(std::move(conn));

            num_fields = mysql_num_fields(resultall);

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

            while ((base::_row = mysql_fetch_row(resultall)))
            {
                base::_addnewrowvalue();
                effect_num++;
            }
            mysql_free_result(resultall);
            if (iscache)
            {
                if (exptime > 0)
                {
                    save_cache(exptime);
                    exptime = 0;
                    iscache = false;
                }
            }
            return *mod;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return *mod;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return *mod;
        }
        catch (...)
        {
        }
        return *mod;
    }
    model &fetch_append()
    {
        effect_num = 0;
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

        if (iscache)
        {
            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
            if (get_cacherecord(sqlhashid))
            {
                iscache = false;
                return *mod;
            }
        }

        if (iserror)
        {
            return *mod;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_select_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return *mod;
        }

        try
        {
            MYSQL_RES *resultall    = nullptr;
            unsigned int num_fields = 0;
            error_msg.clear();
            for (unsigned int i = 0; i < 2; i++)
            {
                num_fields = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
                if (num_fields == 2013)
                {
                    if (error_msg.size() > 0)
                    {
                        return *mod;
                    }
                    error_msg = std::string(mysql_error(conn.get()));
                    mysql_close(conn.get());
                    conn.reset();
                    try
                    {
                        conn = linkconn->add_select_connect();
                    }
                    catch (const char *e)
                    {
                        error_msg = std::string(e);
                        return *mod;
                    }
                    continue;
                }
                else if (num_fields != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    mysql_close(conn.get());
                    conn.reset();
                    return *mod;
                }
                break;
            }

            resultall = mysql_store_result(conn.get());
            linkconn->back_select_connect(std::move(conn));

            num_fields = mysql_num_fields(resultall);

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

            while ((base::_row = mysql_fetch_row(resultall)))
            {
                base::_addnewrowvalue();
                effect_num++;
            }
            mysql_free_result(resultall);
            if (iscache)
            {
                if (exptime > 0)
                {
                    save_cache(exptime);
                    exptime = 0;
                    iscache = false;
                }
            }
            return *mod;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return *mod;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return *mod;
        }
        catch (...)
        {
        }
        return *mod;
    }
    model &fetch_one(bool isappend = false)
    {
        effect_num = 0;
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

        sqlstring.append(" limit 1");

        if (iscache)
        {
            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
            if (get_cacherecord(sqlhashid))
            {
                iscache = false;
                return *mod;
            }
        }

        base::data_reset();
        if (iserror)
        {
            return *mod;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_select_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return *mod;
        }

        try
        {
            MYSQL_RES *resultall    = nullptr;
            unsigned int num_fields = 0;
            error_msg.clear();
            for (unsigned int i = 0; i < 2; i++)
            {
                num_fields = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
                if (num_fields == 2013)
                {
                    if (error_msg.size() > 0)
                    {
                        return *mod;
                    }
                    error_msg = std::string(mysql_error(conn.get()));
                    mysql_close(conn.get());
                    conn.reset();
                    try
                    {
                        conn = linkconn->add_select_connect();
                    }
                    catch (const char *e)
                    {
                        error_msg = std::string(e);
                        return *mod;
                    }
                    continue;
                }
                else if (num_fields != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    mysql_close(conn.get());
                    conn.reset();
                    return *mod;
                }
                break;
            }

            resultall = mysql_store_result(conn.get());
            linkconn->back_select_connect(std::move(conn));

            num_fields = mysql_num_fields(resultall);

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

            if ((base::_row = mysql_fetch_row(resultall)))
            {
                base::_setColnamevalue();
                if (isappend)
                {
                    base::record.emplace_back(base::data);
                }
                effect_num++;
            }

            mysql_free_result(resultall);
            if (iscache)
            {
                if (exptime > 0)
                {
                    save_cache(exptime);
                    exptime = 0;
                    iscache = false;
                }
            }
            return *mod;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return *mod;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return *mod;
        }
        catch (...)
        {
        }
        return *mod;
    }
    model &use_cache(int cache_time = 0)
    {
        iscache = true;
        exptime = cache_time;
        return *mod;
    }
    bool isuse_cache(bool iscachedate = false)
    {
        if (iscachedate)
        {
            return exptime == 0 && iscache == false;
        }
        return iscache;
    }
    void set_cache_state(bool isrestatus = false) { iscache = isrestatus; }
    void remove_exptime_cache()
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        temp_cache.remove_exptime();
    }
    void clear_cache()
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        temp_cache.clear();
    }
    bool remove_cache()
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        std::size_t sqlhashid                             = std::hash<std::string>{}(sqlstring);
        return temp_cache.remove(sqlhashid);
    }
    bool remove_cache(std::size_t cache_key_name)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        return temp_cache.remove(cache_key_name);
    }
    int check_cache(std::size_t cache_key_name)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        return temp_cache.check(cache_key_name);
    }
    std::vector<typename base::meta> get_cache_data(std::size_t cache_key_name)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        auto cache_data                                   = temp_cache.get(cache_key_name);
        return cache_data;
    }
    typename base::meta get_cache_obj(std::size_t cache_key_name)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        auto cache_data                                   = temp_cache.get_obj(cache_key_name);
        return cache_data;
    }
    model &get_cache(std::size_t cache_key_name)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        base::record                                      = temp_cache.get(cache_key_name);
        if (base::record.size() == 0)
        {
            base::record_reset();
        }
        else
        {
            base::data = base::record[0];
        }
        return *mod;
    }
    int update_cache(int exp_time = 0)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        std::size_t sqlhashid                             = std::hash<std::string>{}(sqlstring);
        return temp_cache.update(sqlhashid, exp_time);
    }
    int update_cache(std::size_t cache_key_name, int exp_time)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        return temp_cache.update(cache_key_name, exp_time);
    }
    bool save_cache(int exp_time = 0)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        std::size_t sqlhashid                             = std::hash<std::string>{}(sqlstring);
        temp_cache.save(sqlhashid, base::record, exp_time);
        return true;
    }

    bool save_cache(std::size_t cache_key_name, typename base::meta &cache_data, int exp_time = 0)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        temp_cache.save(cache_key_name, cache_data, exp_time);
        return true;
    }

    bool save_cache(std::size_t cache_key_name, std::vector<typename base::meta> &cache_data, int exp_time = 0)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        temp_cache.save(cache_key_name, cache_data, exp_time);
        return true;
    }
    bool get_cacherecord(std::size_t cache_key_name)
    {
        model_meta_cache<typename base::meta> &temp_cache = model_meta_cache<typename base::meta>::getinstance();
        base::record                                      = temp_cache.get(cache_key_name);
        if (base::record.size() == 0)
        {
            return false;
        }
        else
        {
            base::data = base::record[0];
            return true;
        }
    }
    http::OBJ_VALUE fetch_json()
    {
        effect_num = 0;
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

        //std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqlselectexecute(dbhash);
        // std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &myconn = http::get_mysqlpool();
        // auto iter                                                           = myconn.find(dbhash);
        // if (iter == myconn.end())
        // {
        //     error_msg = "not find orm link tag in pool";
        //     return valuetemp;
        // }
        //std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = linkconn->get_select_connect();
        if (iserror)
        {
            return valuetemp;
        }
        MYSQL_RES *resultall = nullptr;
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_select_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return valuetemp;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());

            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return valuetemp;
            }

            resultall = mysql_store_result(conn.get());
            linkconn->back_select_connect(std::move(conn));

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

            MYSQL_ROW json_row;
            while ((json_row = mysql_fetch_row(resultall)))
            {
                http::OBJ_VALUE rowtemp;
                rowtemp.set_array();
                for (unsigned char index = 0; index < num_fields; index++)
                {
                    if (json_row[index] != NULL)
                    {
                        rowtemp[table_fieldname[index]] = std::string(json_row[index]);
                    }
                    else
                    {
                        rowtemp[table_fieldname[index]] = "";
                    }
                }
                valuetemp.push(effect_num, std::move(rowtemp));
                effect_num++;
            }
            mysql_free_result(resultall);
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
        }
        catch (...)
        {
        }
        return valuetemp;
    }
    model &get_one(long long id)
    {
        effect_num = 0;
        sqlstring  = "SELECT *  FROM ";

        sqlstring.append(base::tablename);
        sqlstring.append(" WHERE ");

        sqlstring.append(base::getPKname());
        sqlstring.append("=");
        sqlstring.append(std::to_string(id));
        if (iscache)
        {
            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
            if (get_cacherecord(sqlhashid))
            {
                iscache = false;
                return *mod;
            }
        }

        base::data_reset();

        if (iserror)
        {
            return *mod;
        }
        MYSQL_RES *resultone = nullptr;
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_select_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return *mod;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return *mod;
            }

            resultone = mysql_store_result(conn.get());

            linkconn->back_select_connect(std::move(conn));

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
                effect_num++;
            }

            mysql_free_result(resultone);
            if (iscache)
            {
                if (exptime > 0)
                {
                    save_cache(exptime);
                    exptime = 0;
                    iscache = false;
                }
            }
            return *mod;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return *mod;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return *mod;
        }
        catch (...)
        {
        }
        return *mod;
    }
    int update()
    {
        effect_num = 0;
        if (wheresql.empty())
        {
            if (base::getPK() > 0)
            {
                std::ostringstream tempsql;
                tempsql << " ";
                tempsql << base::getPKname();
                tempsql << " = '";
                tempsql << base::getPK();
                tempsql << "' ";
                wheresql = tempsql.str();
            }
            else
            {
                return 0;
            }
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

        if (iscommit)
        {
            iscommit = false;
            return 0;
        }

        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            readnum           = mysql_affected_rows(conn.get());
            effect_num        = readnum;
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }
    }
    int update(const std::string &fieldname)
    {
        effect_num = 0;
        if (wheresql.empty())
        {
            if (base::getPK() > 0)
            {
                std::ostringstream tempsql;
                tempsql << " ";
                tempsql << base::getPKname();
                tempsql << " = '";
                tempsql << base::getPK();
                tempsql << "' ";
                wheresql = tempsql.str();
            }
            else
            {
                return 0;
            }
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

        if (iscommit)
        {
            iscommit = false;
            return 0;
        }

        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return 0;
            }
            readnum    = mysql_affected_rows(conn.get());
            effect_num = readnum;
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            throw std::runtime_error(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }
    }
    int update_batch(const std::string &fieldname)
    {
        effect_num = 0;
        if (base::record.size() == 0)
        {
            return 0;
        }
        if (fieldname.size() > 0)
        {
            sqlstring = base::_make_insert_into_sql(fieldname);
        }
        else
        {
            sqlstring = base::_make_replace_into_sql();
        }

        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return 0;
            }
            readnum    = mysql_affected_rows(conn.get());
            effect_num = readnum;
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            throw std::runtime_error(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }
    }
    int remove()
    {
        effect_num = 0;
        if (wheresql.empty())
        {
            if (base::getPK() > 0)
            {
                std::ostringstream tempsql;
                tempsql << " ";
                tempsql << base::getPKname();
                tempsql << " = '";
                tempsql << base::getPK();
                tempsql << "' ";
                wheresql = tempsql.str();
            }
            else
            {
                return 0;
            }
        }

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

        if (iscommit)
        {
            iscommit = false;
            return 0;
        }

        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }

        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return 0;
            }
            readnum    = mysql_affected_rows(conn.get());
            effect_num = readnum;
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            throw std::runtime_error(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }
    }
    int remove(long long id)
    {
        effect_num = 0;
        sqlstring  = "DELETE FROM  ";
        sqlstring.append(base::tablename);
        sqlstring.append(" WHERE ");

        sqlstring.append(base::getPKname());
        sqlstring.append("=");
        sqlstring.append(std::to_string(id));

        if (iscommit)
        {
            iscommit = false;
            return 0;
        }

        if (iserror)
        {
            return 0;
        }
        //mysql_ping(conn.get());
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        try
        {
            mysql_ping(conn.get());
            error_msg.clear();
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                base::setPK(0);
                mysql_close(conn.get());
                conn.reset();
                return 0;
            }
            readnum    = mysql_affected_rows(conn.get());
            effect_num = readnum;
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }
    }
    int soft_remove(const std::string &fieldsql)
    {
        effect_num = 0;
        if (wheresql.empty())
        {
            if (base::getPK() > 0)
            {
                std::ostringstream tempsql;
                tempsql << " ";
                tempsql << base::getPKname();
                tempsql << " = '";
                tempsql << base::getPK();
                tempsql << "' ";
                wheresql = tempsql.str();
            }
            else
            {
                return 0;
            }
        }

        sqlstring = base::soft_remove_sql(fieldsql);
        if (sqlstring.empty())
        {
            error_msg = "soft delete field empty.";
            return 0;
        }
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

        if (iscommit)
        {
            iscommit = false;
            return 0;
        }

        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return 0;
            }
            readnum    = mysql_affected_rows(conn.get());
            effect_num = readnum;
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            throw std::runtime_error(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }
    }
    int soft_remove()
    {
        effect_num = 0;
        if (wheresql.empty())
        {
            if (base::getPK() > 0)
            {
                std::ostringstream tempsql;
                effect_num = 1;
                tempsql << " ";
                tempsql << base::getPKname();
                tempsql << " = '";
                tempsql << base::getPK();
                tempsql << "' ";
                wheresql = tempsql.str();
            }
            else
            {
                return 0;
            }
        }
        if (effect_num == 1)
        {
            sqlstring = base::soft_remove_sql(" ");
        }
        else
        {
            sqlstring = base::soft_remove_sql("");
        }
        effect_num = 0;
        if (sqlstring.empty())
        {
            error_msg = "soft delete field empty.";
            return 0;
        }
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

        if (iscommit)
        {
            iscommit = false;
            return 0;
        }

        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return 0;
            }
            readnum    = mysql_affected_rows(conn.get());
            effect_num = readnum;
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            return 0;
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            throw std::runtime_error(e);
            return 0;
        }
        catch (...)
        {
            return 0;
        }
    }
    std::tuple<long long, long long> insert(typename base::meta &insert_data, bool isclear = true)
    {
        effect_num = 0;
        sqlstring  = base::_makerecordinsertsql(insert_data);
        if (iscommit)
        {
            iscommit = false;
            return std::make_tuple(0, 0);
        }

        if (iserror)
        {
            return std::make_tuple(0, 0);
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return std::make_tuple(0, 0);
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (isclear)
            {
                sqlstring.clear();
            }
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                base::setPK(0);
                mysql_close(conn.get());
                conn.reset();
                return std::make_tuple(readnum, 0);
            }
            readnum            = mysql_affected_rows(conn.get());
            effect_num         = readnum;
            long long insertid = mysql_insert_id(conn.get());
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return std::make_tuple(readnum, insertid);
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
        }
        catch (...)
        {
        }
        return std::make_tuple(0, 0);
    }
    std::tuple<long long, long long> insert(std::vector<typename base::meta> &insert_data, bool isclear = true)
    {
        effect_num = 0;
        sqlstring  = base::_makerecordinsertsql(insert_data);
        if (iscommit)
        {
            iscommit = false;
            return std::make_tuple(0, 0);
        }

        if (iserror)
        {
            return std::make_tuple(0, 0);
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return std::make_tuple(0, 0);
        }
        try
        {
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (isclear)
            {
                sqlstring.clear();
            }
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                base::setPK(0);
                mysql_close(conn.get());
                conn.reset();
                return std::make_tuple(readnum, 0);
            }
            readnum            = mysql_affected_rows(conn.get());
            effect_num         = readnum;
            long long insertid = mysql_insert_id(conn.get());
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return std::make_tuple(readnum, insertid);
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
        }
        catch (...)
        {
        }
        return std::make_tuple(0, 0);
    }
    std::tuple<long long, long long> insert()
    {
        effect_num = 0;
        sqlstring  = base::_makeinsertsql();
        if (iscommit)
        {
            iscommit = false;
            return std::make_tuple(0, 0);
        }

        if (iserror)
        {
            return std::make_tuple(0, 0);
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return std::make_tuple(0, 0);
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                base::setPK(0);
                mysql_close(conn.get());
                conn.reset();
                return std::make_tuple(readnum, 0);
            }
            readnum            = mysql_affected_rows(conn.get());
            effect_num         = readnum;
            long long insertid = mysql_insert_id(conn.get());
            base::setPK(insertid);
            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            return std::make_tuple(readnum, insertid);
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
        }
        catch (...)
        {
        }
        return std::make_tuple(0, 0);
    }

    int save(bool isrealnew = false)
    {
        effect_num = 0;
        if (base::getPK() > 0 && isrealnew == false)
        {
            if (wheresql.empty())
            {
                std::ostringstream tempsql;
                tempsql << " ";
                tempsql << base::getPKname();
                tempsql << " = '";
                tempsql << base::getPK();
                tempsql << "' ";
                wheresql = tempsql.str();
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
            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
            std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
            try
            {
                conn = linkconn->get_edit_connect();
            }
            catch (const char *e)
            {
                error_msg = std::string(e);
                return 0;
            }
            try
            {
                mysql_ping(conn.get());
                long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    mysql_close(conn.get());
                    conn.reset();
                    return 0;
                }
                readnum    = mysql_affected_rows(conn.get());
                effect_num = readnum;
                linkconn->back_edit_connect(std::move(conn));
                return readnum;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const char *e)
            {
                error_msg = std::string(e);
                return 0;
            }
            catch (...)
            {
                return 0;
            }
        }
        else
        {
            sqlstring = base::_makeinsertsql();
            if (iscommit)
            {
                iscommit = false;
                return 0;
            }

            if (iserror)
            {
                return 0;
            }
            std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
            try
            {
                conn = linkconn->get_edit_connect();
            }
            catch (const char *e)
            {
                error_msg = std::string(e);
                return 0;
            }
            try
            {
                mysql_ping(conn.get());
                long long readnum = mysql_real_query(conn.get(), &sqlstring[0], sqlstring.size());
                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    base::setPK(0);
                    mysql_close(conn.get());
                    conn.reset();
                    return 0;
                }
                readnum          = mysql_affected_rows(conn.get());
                effect_num       = readnum;
                long long tempid = mysql_insert_id(conn.get());
                base::setPK(tempid);
                linkconn->back_edit_connect(std::move(conn));
                return readnum;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const char *e)
            {
                error_msg = std::string(e);
                return 0;
            }
            catch (...)
            {
            }
        }
        return 0;
    }
    std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>
    query(const std::string &rawsql, bool isedit = false)
    {

        std::vector<std::vector<std::string>> temprecord;
        std::vector<std::string> table_fieldname;
        std::map<std::string, unsigned int> table_fieldmap;

        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        // std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &myconn = http::get_mysqlpool();
        // auto iter                                                           = myconn.find(dbhash);
        // if (iter == myconn.end())
        // {
        //     error_msg = "not find orm link tag in pool";
        //     return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        // }
        //std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = linkconn->get_edit_connect();
        if (iserror)
        {
            return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }
        if (isedit)
        {
            try
            {
                conn = linkconn->get_edit_connect();
            }
            catch (const char *e)
            {
                error_msg = std::string(e);
                return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }
        }
        else
        {
            try
            {
                conn = linkconn->get_select_connect();
            }
            catch (const char *e)
            {
                error_msg = std::string(e);
                return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }
        }
        try
        {
            MYSQL_RES *resultall = nullptr;
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &rawsql[0], rawsql.size());

            if (readnum != 0)
            {
                error_msg = std::string(mysql_error(conn.get()));
                mysql_close(conn.get());
                conn.reset();
                return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }

            resultall = mysql_store_result(conn.get());

            readnum = 0;

            int num_fields = mysql_num_fields(resultall);

            MYSQL_FIELD *fields;
            fields = mysql_fetch_fields(resultall);
            std::string type_temp;

            for (unsigned char index = 0; index < num_fields; index++)
            {
                type_temp = std::string(fields[index].name);
                std::transform(type_temp.begin(), type_temp.end(), type_temp.begin(), ::tolower);
                table_fieldname.push_back(type_temp);
                table_fieldmap.insert({type_temp, index});
            }

            //int j = 0;
            MYSQL_ROW json_row;
            while ((json_row = mysql_fetch_row(resultall)))
            {
                std::vector<std::string> rowtemp;
                for (unsigned char index = 0; index < num_fields; index++)
                {
                    if (json_row[index] != NULL)
                    {
                        rowtemp.push_back(std::string(json_row[index]));
                    }
                    else
                    {
                        rowtemp.push_back("");
                    }
                }
                temprecord.push_back(std::move(rowtemp));
                //j++;
            }
            mysql_free_result(resultall);
            try
            {
                if (isedit)
                {
                    linkconn->back_edit_connect(std::move(conn));
                }
                else
                {
                    linkconn->back_select_connect(std::move(conn));
                }
            }
            catch (...)
            {
            }
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
        }
        catch (...)
        {
        }
        return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
    }
    long long edit_query(const std::string &rawsql, bool isinsert = false)
    {
        if (iserror)
        {
            return 0;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return 0;
        }
        try
        {
            mysql_ping(conn.get());
            long long readnum = mysql_real_query(conn.get(), &rawsql[0], rawsql.size());
            readnum           = mysql_affected_rows(conn.get());
            if (isinsert)
            {
                readnum = mysql_insert_id(conn.get());
            }
            linkconn->back_edit_connect(std::move(conn));
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
        }
        catch (...)
        {
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
        error_msg.clear();
        iskuohao     = false;
        ishascontent = false;
        iscommit     = false;
        iscache      = false;
        effect_num   = 0;
        if (both)
        {
            base::record_reset();
            base::data_reset();
        }
        return *mod;
    }

    model &set_data(typename base::meta indata)
    {
        base::data = indata;
        return *mod;
    }
    model &get() { return *mod; }
    std::string get_query() { return sqlstring; }
    model &open_commit()
    {
        iscommit = true;
        return *mod;
    }
    model &close_commit()
    {
        iscommit = false;
        return *mod;
    }
    int submit_commit(const commit_lists_callback &commit_sqllist)
    {
        iscommit = false;

        int turn_state = 0;
        //std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = http::get_mysqleditexecute(dbhash);
        // std::map<std::size_t, std::shared_ptr<http::mysqllinkpool>> &myconn = http::get_mysqlpool();
        // auto iter                                                           = myconn.find(dbhash);
        // if (iter == myconn.end())
        // {
        //     error_msg = "not find orm link tag in pool";
        //     return -1;
        // }
        //std::unique_ptr<MYSQL, decltype(&mysql_close)> conn = linkconn->get_edit_connect();
        if (iserror)
        {
            return -1;
        }
        std::unique_ptr<MYSQL, decltype(&mysql_close)> conn(NULL, &mysql_close);
        try
        {
            conn = linkconn->get_edit_connect();
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
            return -1;
        }
        try
        {
            mysql_autocommit(conn.get(), turn_state);
            long long readnum = 0;
            std::vector<long long> insert_lastid;
            for (unsigned int j = 0; j < commit_sqllist.size(); j++)
            {
                readnum = mysql_real_query(conn.get(), &commit_sqllist[j].first[0], commit_sqllist[j].first.size());
                if (readnum != 0)
                {
                    error_msg = std::string(mysql_error(conn.get()));
                    mysql_rollback(conn.get());

                    error_msg.append(" Commit error, raw sql: ");
                    error_msg.append(commit_sqllist[j].first);

                    break;
                }
                if (commit_sqllist[j].first.size() > 2 && commit_sqllist[j].first[0] == 'I' &&
                    commit_sqllist[j].first[1] == 'N')
                {
                    long long tempid = mysql_insert_id(conn.get());
                    insert_lastid.push_back(tempid);
                }
                else
                {
                    insert_lastid.push_back(0);
                }
            }

            if (readnum == 0)
            {
                mysql_commit(conn.get());
            }
            turn_state = 1;
            mysql_autocommit(conn.get(), turn_state);

            try
            {
                linkconn->back_edit_connect(std::move(conn));
            }
            catch (...)
            {
            }
            if (readnum == 0)
            {
                for (unsigned int j = 0; j < commit_sqllist.size(); j++)
                {
                    if (j < insert_lastid.size())
                    {
                        if (insert_lastid[j] > 0 && commit_sqllist[j].second != nullptr)
                        {
                            commit_sqllist[j].second(0, insert_lastid[j]);
                        }
                    }
                }
            }
            return readnum;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }
        catch (const char *e)
        {
            error_msg = std::string(e);
        }
        catch (...)
        {
        }

        return -1;
    }
    unsigned int effect()
    {
        return effect_num;
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
    std::string original_tablename;

    // std::list<std::string> commit_sqllist;
    bool iskuohao           = false;
    bool iscommit           = false;
    bool ishascontent       = false;
    bool iscache            = false;
    bool iserror            = false;
    int exptime             = 0;
    unsigned int effect_num = 0;
    std::size_t dbhash;
    model *mod;

    std::shared_ptr<http::mysqllinkpool> linkconn;
};

}// namespace orm
#endif