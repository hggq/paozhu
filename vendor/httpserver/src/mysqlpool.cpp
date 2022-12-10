#include <mutex>
#include <string>
#include <map>
#include <set>
#include <string_view>
#include <thread>
#include <iostream>
#include <functional>
#include <mysqlx/xdevapi.h>
#include <map>
#include <typeinfo>
#include <memory>
#include <list>
#include <queue>
#include "request.h"
#include "mysqlpool.h"

#include <condition_variable>

namespace http
{

    std::mutex sessedit_mutex, sessselect_mutex;
    std::map<size_t,mysqllinkpool> &get_mysqlpool()
    {
        static std::map<size_t,mysqllinkpool>  instance;
        return instance;
    }
    mysqllinkpool::mysqllinkpool(std::string ms, std::string sl) : connectedit(ms), connectselect(sl)
    {
    }
    void mysqllinkpool::addselectsession(unsigned int num)
    {
        for (unsigned int i = 0; i < num; i++)
        {
            sessselecttpool.push(std::move(std::make_unique<mysqlx::Session>(connectselect)));
        }
    }
    void mysqllinkpool::addeditsession(unsigned int num)
    {
        for (unsigned int i = 0; i < num; i++)
        {
            sesseditpool.push(std::move(std::make_unique<mysqlx::Session>(connectedit)));
        }
    }

    mysqlx::RowResult mysqllinkpool::sqlselectfetch(std::string &sql)
    {

        std::unique_lock<std::mutex> lock(sessselect_mutex);
        if (sessselecttpool.empty())
        {
            sessselecttpool.push(std::move(std::make_unique<mysqlx::Session>(connectselect)));
        }
        std::unique_ptr<mysqlx::Session> sess(std::move(sessselecttpool.front()));
        sessselecttpool.pop();
        selectbusynum++;
        lock.unlock();

        mysqlx::RowResult temp = sess->sql(sql).execute();
        std::unique_lock<std::mutex> lck(sessselect_mutex);
        sessselecttpool.push(std::move(sess));
        selectbusynum--;
        lck.unlock();
        return std::move(temp);
    }

    mysqlx::SqlResult mysqllinkpool::sqleditfetch(std::string &sql)
    {
        // dbreadpool[0].sql()
        std::unique_lock<std::mutex> lock(sessedit_mutex);
        if (sesseditpool.empty())
        {
            sesseditpool.push(std::move(std::make_unique<mysqlx::Session>(connectedit)));
        }
        std::unique_ptr<mysqlx::Session> sess(std::move(sesseditpool.front()));
        sesseditpool.pop();
        editbusynum++;
        lock.unlock();

        mysqlx::SqlResult temp = sess->sql(sql).execute();

        std::unique_lock<std::mutex> lck(sessedit_mutex);
        sesseditpool.push(std::move(sess));
        editbusynum--;
        lck.unlock();
        return std::move(temp);
    }
    bool mysqllinkpool::sqleditcommit(std::list<std::string> &sql)
    {
        // dbreadpool[0].sql()
        std::unique_lock<std::mutex> lock(sessedit_mutex);
        if (sesseditpool.empty())
        {
            sesseditpool.push(std::move(std::make_unique<mysqlx::Session>(connectedit)));
        }
        std::unique_ptr<mysqlx::Session> sess(std::move(sesseditpool.front()));
        sesseditpool.pop();
        editbusynum++;
        lock.unlock();
        sess->startTransaction();
        try
        {

            for (; !sql.empty();)
            {
                sess->sql(sql.front()).execute();
                sql.pop_front();
            }

            sess->commit();
            std::unique_lock<std::mutex> lck(sessedit_mutex);
            sesseditpool.push(std::move(sess));
            editbusynum--;
            lck.unlock();
            return true;
        }
        catch (const mysqlx::Error &err)
        {

            sess->rollback();
            std::unique_lock<std::mutex> lck(sessedit_mutex);
            sesseditpool.push(std::move(sess));
            editbusynum--;
            lck.unlock();

            return false;
        }
    }
    mysqlx::SqlResult mysqllinkpool::sqleditfetch(std::string &sql, long long &effectrow, std::function<std::unique_ptr<mysqlx::Session>(std::string &, mysqlx::SqlResult &, std::unique_ptr<mysqlx::Session>)> func)
    {

        std::unique_lock<std::mutex> lock(sessedit_mutex);
        if (sesseditpool.empty())
        {
            sesseditpool.push(std::move(std::make_unique<mysqlx::Session>(connectedit)));
        }
        std::unique_ptr<mysqlx::Session> sess(std::move(sesseditpool.front()));
        sesseditpool.pop();
        editbusynum++;
        lock.unlock();
        mysqlx::SqlResult temp;
        sess = func(sql, temp, std::move(sess));

        std::unique_lock<std::mutex> lck(sessedit_mutex);
        sesseditpool.push(std::move(sess));
        editbusynum--;
        lck.unlock();

        return std::move(temp);
    }
    mysqlx::SqlResult mysqllinkpool::sqleditfetch(std::string &sql, long long &effectrow, std::function<std::unique_ptr<mysqlx::Session>(std::string &, std::unique_ptr<mysqlx::Session>)> funcbeg, std::function<std::unique_ptr<mysqlx::Session>(std::string &, std::unique_ptr<mysqlx::Session>)> funcend)
    {
        std::unique_lock<std::mutex> lock(sessedit_mutex);
        if (sesseditpool.empty())
        {
            sesseditpool.push(std::move(std::make_unique<mysqlx::Session>(connectedit)));
        }
        std::unique_ptr<mysqlx::Session> sess(std::move(sesseditpool.front()));
        sesseditpool.pop();
        editbusynum++;
        lock.unlock();

        sess = funcbeg(sql, std::move(sess));
        mysqlx::SqlResult temp = sess->sql(sql).execute();
        ;
        sess = funcend(sql, std::move(sess));

        std::unique_lock<std::mutex> lck(sessedit_mutex);
        sesseditpool.push(std::move(sess));
        editbusynum--;
        lck.unlock();

        return std::move(temp);
    }

    unsigned int mysqllinkpool::getselectpoolnum()
    {
        std::unique_lock<std::mutex> lock(sessselect_mutex);
        return sessselecttpool.size();
    }
    unsigned int mysqllinkpool::geteditpoolnum()
    {
        std::unique_lock<std::mutex> lock(sessedit_mutex);
        return sesseditpool.size();
    }

    unsigned int mysqllinkpool::clearpool()
    {
        int i = 0;
        if (selectbusynum == 0)
        {

            std::unique_lock<std::mutex> lock(sessselect_mutex);
            for (; !sessselecttpool.empty();)
            {
                std::unique_ptr<mysqlx::Session> sess(std::move(sessselecttpool.front()));
                sessselecttpool.pop();
                sess->close();
                sess.reset(nullptr);

                i++;
            }
        }
        if (editbusynum == 0)
        {

            std::unique_lock<std::mutex> lock(sessedit_mutex);
            for (; !sesseditpool.empty();)
            {
                std::unique_ptr<mysqlx::Session> sess(std::move(sesseditpool.front()));
                sesseditpool.pop();
                sess->close();
                sess.reset(nullptr);

                i++;
            }
        }
        return i;
    }

    void mysqllinkpool::initpool()
    {
        addselectsession(2);
        addeditsession(1);
    }

}
