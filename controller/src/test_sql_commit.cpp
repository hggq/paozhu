#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "test_sql_commit.h"
#include "orm_query.h"

namespace orm::cust
{
struct LocalusersqlStruct : orm::Base<LocalusersqlStruct>
{
    unsigned int adminid;
    std::string name;
    std::string nickname;
    unsigned int postid;
    ORM_NAMES(adminid, name, nickname, postid);
};

}// namespace orm::cust

namespace http
{
//@urlpath(null,test_sql_commit)
std::string test_sql_commit(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  standalone sqlquery ";

    try
    {
        // Result struct — use LocalusersqlStruct directly when only one row expected.
        orm::cust::LocalusersqlStruct loaduser;
        // Use db_conn directly (tag "cms" defined in orm.conf).
        auto ulink = std::make_unique<orm::db_conn>("cms");
        // SELECT fields must match the result struct fields.
        std::string sqlstring = "SELECT adminid,name,nickname,postid FROM ";
        sqlstring.append("sysuser");
        sqlstring.append(" where 1 limit 1");

        // Synchronous mode.
        ulink->query(sqlstring, loaduser);

        client << "sql:" << sqlstring << "<hr>";
        if (loaduser.adminid != 0)
        {
            client << "adminid:";
            client << loaduser.adminid;
            client << ", name:";
            client << loaduser.name;
            client << ", nickname:";
            client << loaduser.nickname;
            client << ", postid:";
            client << loaduser.postid;
            client << "<br />";
        }

        bool is_begin = ulink->begin_commit();
        orm::cust::LocalusersqlStruct commit_data;
        if (is_begin)
        {
            client << "begin_commit:OK<hr>";
            try
            {
                sqlstring = "update sysuser set nickname = 'nicename_a', postid = postid + 1 where adminid = 10000 ";
                ulink->edit_query(sqlstring);
                if (ulink->iserror)
                {
                    ulink->rollback();
                }
                else
                {
                    sqlstring = "INSERT INTO `sysuser` (`adminid`, `name`, `password`, `textword`, `isopen`, `level`, `companyid`, `dpid`, `jobid`, `roleid`, `postid`, `created_at`, `enddate`, `qrtemp`, `gender`, `nickname`, `realname`, `avatar`, `mobile`, `email`, `wxuuid`) VALUES (NULL, 'userroot', 'e10adc3949ba59abbe56e057f20f883e', '123456', '1', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', 'goodname', 'displayname', '', '', '', '');";
                    ulink->edit_query(sqlstring);
                    if (ulink->iserror)
                    {
                        ulink->rollback();
                    }
                    else
                    {
                        // SELECT over the same edit connection.
                        sqlstring = "SELECT adminid,name,nickname,postid FROM sysuser where adminid = 10000 limit 1 ";
                        ulink->edit_query(sqlstring, commit_data);
                        if (ulink->iserror)
                        {
                            ulink->rollback();
                        }
                        else
                        {
                            ulink->commit();
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                ulink->rollback();
                std::cerr << e.what() << '\n';
            }
        }

        client << "commit:" << ulink->error_msg << "<br />";
        if (commit_data.adminid != 0)
        {
            client << "adminid:";
            client << commit_data.adminid;
            client << ", name:";
            client << commit_data.name;
            client << ", nickname:";
            client << commit_data.nickname;
            client << ", postid:";
            client << commit_data.postid;
            client << "<br />";
        }

        is_begin = ulink->begin_commit();

        if (is_begin)
        {
            client << "begin_commit:OK<hr>";
            try
            {
                sqlstring = "update sysuser set nickname = 'nicename_b', postid = postid + 1 where adminid = 10000 ";
                ulink->edit_query(sqlstring);
                if (ulink->iserror)
                {
                    ulink->rollback();
                }
                else
                {
                    // INSERT with existing PK 10003 — must fail (triggers rollback path).
                    sqlstring = "INSERT INTO `sysuser` (`adminid`, `name`, `password`, `textword`, `isopen`, `level`, `companyid`, `dpid`, `jobid`, `roleid`, `postid`, `created_at`, `enddate`, `qrtemp`, `gender`, `nickname`, `realname`, `avatar`, `mobile`, `email`, `wxuuid`) VALUES (10003, 'userroot', 'e10adc3949ba59abbe56e057f20f883e', '123456', '1', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', 'goodname', 'displayname', '', '', '', '');";
                    ulink->edit_query(sqlstring);
                    if (ulink->iserror)
                    {
                        ulink->rollback();
                    }
                    else
                    {
                        // SELECT over the same edit connection.
                        sqlstring = "SELECT adminid,name,nickname,postid FROM sysuser where adminid = 10000 limit 1 ";
                        ulink->edit_query(sqlstring, commit_data);
                        if (ulink->iserror)
                        {
                            ulink->rollback();
                        }
                        else
                        {
                            ulink->commit();
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                ulink->rollback();
                std::cerr << e.what() << '\n';
            }
        }

        client << "commit:" << ulink->error_msg << "<br />";
        if (commit_data.adminid != 0)
        {
            client << "adminid:";
            client << commit_data.adminid;
            client << ", name:";
            client << commit_data.name;
            client << ", nickname:";
            client << commit_data.nickname;
            client << ", postid:";
            client << commit_data.postid;
            client << "<br />";
        }

        /*
            // Preferred approach: build SQL via ORM objects, then commit the transaction.

            auto user_m = orm::cms::Sysuser();
            user_m.data.nickname = "nicename_c";
            std::string newsqlstring = user_m.commit_insert(); // get INSERT SQL
            user_m.where("adminid",10005);
            newsqlstring = user_m.commit_update("nickname");   // get UPDATE SQL
            newsqlstring = user_m.commit_remove();             // get DELETE SQL

        */
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    return "";
}

//@urlpath(null,co_sql_commit)
asio::awaitable<std::string> test_co_sql_commit(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  async use async_insert_query ";

    try
    {
        // Single-row result carrier.
        orm::cust::LocalusersqlStruct commit_data;
        // Use db_conn directly (tag "cms" defined in orm.conf).
        auto ulink = std::make_unique<orm::db_conn>("cms");

        // Coroutine transaction: all async_* calls run on the same locked write
        // connection. Same shape as the sync version — just swap to co_await.
        bool is_begin = co_await ulink->async_begin_commit();
        if (is_begin)
        {
            client << "async_begin_commit:OK<hr>";
            bool need_rollback = false;
            try
            {
                // First, a plain edit inside the transaction (demo of mixed calls).
                std::string sqlstring = "update sysuser set nickname = 'nicename_a', postid = postid + 1 where adminid = 10000 ";
                co_await ulink->async_edit_query(sqlstring);
                if (ulink->iserror)
                {
                    co_await ulink->async_rollback();
                }
                else
                {
                    // INSERT and retrieve the generated PK: returns {affected_rows, last_id}.
                    // insert_query takes raw SQL only, no coupling with ORM objects; lastid
                    // must be carried by the caller.
                    // - MySQL/SQLite: auto-increment id comes from connection implicit state,
                    //   a bare INSERT is sufficient.
                    // - PostgreSQL: rawsql must include RETURNING <pk> (e.g. commit_insert_returning()),
                    //   otherwise lastid returns 0 and you need a separate round-trip.
                    sqlstring = "INSERT INTO `sysuser` (`adminid`, `name`, `password`, `textword`, `isopen`, `level`, `companyid`, `dpid`, `jobid`, `roleid`, `postid`, `created_at`, `enddate`, `qrtemp`, `gender`, `nickname`, `realname`, `avatar`, `mobile`, `email`, `wxuuid`) VALUES (NULL, 'userroot', 'e10adc3949ba59abbe56e057f20f883e', '123456', '1', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', 'goodname', 'displayname', '', '', '', '');";
                    // PostgreSQL has no implicit last_insert_id: append RETURNING to read the PK back.
                    if (ulink->db_type == orm::DB_TYPE::POSTGRESQL)
                    {
                        if (!sqlstring.empty() && sqlstring.back() == ';')
                            sqlstring.pop_back();
                        sqlstring.append(" RETURNING adminid");
                    }
                    auto [eff, pid] = co_await ulink->async_insert_query(sqlstring);
                    client << "async_insert_query effect_num:" << eff << ", last_id:" << pid << "<br />";
                    if (ulink->iserror)
                    {
                        co_await ulink->async_rollback();
                    }
                    else
                    {
                        // Use the retrieved pid for a follow-up SELECT (in real scenarios
                        // this is how child-table FK rows reference the parent auto-id).
                        sqlstring = "SELECT adminid,name,nickname,postid FROM sysuser where adminid = " + std::to_string(pid) + " limit 1 ";
                        co_await ulink->async_edit_query(sqlstring, commit_data);
                        if (ulink->iserror)
                        {
                            co_await ulink->async_rollback();
                        }
                        else
                        {
                            co_await ulink->async_commit();
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                need_rollback = true;
                std::cerr << e.what() << '\n';
            }
            if (need_rollback)
            {
                co_await ulink->async_rollback();
            }
        }

        client << "commit:" << ulink->error_msg << "<br />";
        if (commit_data.adminid != 0)
        {
            client << "adminid:" << commit_data.adminid
                   << ", name:" << commit_data.name
                   << ", nickname:" << commit_data.nickname
                   << ", postid:" << commit_data.postid << "<br />";
        }

        // Second branch: INSERT with a PK that already exists → unique-key violation,
        // transaction rolls back. Demonstrates error handling.
        is_begin = co_await ulink->async_begin_commit();
        if (is_begin)
        {
            client << "async_begin_commit(2):OK<hr>";
            bool need_rollback = false;
            try
            {
                std::string sqlstring = "INSERT INTO `sysuser` (`adminid`, `name`, `password`, `textword`, `isopen`, `level`, `companyid`, `dpid`, `jobid`, `roleid`, `postid`, `created_at`, `enddate`, `qrtemp`, `gender`, `nickname`, `realname`, `avatar`, `mobile`, `email`, `wxuuid`) VALUES (10003, 'userroot', 'e10adc3949ba59abbe56e057f20f883e', '123456', '1', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', 'goodname', 'displayname', '', '', '', '');";
                // On PG you must append RETURNING <pk>; using an ORM object (Sysuser_m.commit_insert())
                // is recommended — it generates the RETURNING clause automatically.
                if (ulink->db_type == orm::DB_TYPE::POSTGRESQL)
                {
                    if (!sqlstring.empty() && sqlstring.back() == ';')
                        sqlstring.pop_back();
                    sqlstring.append(" RETURNING adminid");
                }
                auto [eff2, pid2] = co_await ulink->async_insert_query(sqlstring);
                client << "async_insert_query(2) effect_num:" << eff2 << ", last_id:" << pid2 << "<br />";
                if (ulink->iserror)
                {
                    client << "got error, will async_rollback<br />";
                    co_await ulink->async_rollback();
                }
                else
                {
                    co_await ulink->async_commit();
                }
            }
            catch (const std::exception &e)
            {
                need_rollback = true;
                std::cerr << e.what() << '\n';
            }
            if (need_rollback)
            {
                co_await ulink->async_rollback();
            }
        }

        client << "commit(2):" << ulink->error_msg << "<br />";
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        co_return "";
    }
    co_return "";
}

//@urlpath(null,co_sql_orm)
asio::awaitable<std::string> test_co_sql_orm(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  ORM object style: commit_insert / commit_update / commit_remove<br>";

    try
    {
        auto ulink = std::make_unique<orm::db_conn>("cms");

        // Build SQL via ORM object (no hand-written SQL). Don't set adminid (=0) —
        // make_data_insert_sql will emit NULL so the database auto-assigns it.
        auto user_m            = orm::cms::Sysuser();
        user_m.data.name       = "userroot";
        user_m.data.password   = "e10adc3949ba59abbe56e057f20f883e";
        user_m.data.textword   = "123456";
        user_m.data.isopen     = 1;
        user_m.data.level      = 0;
        user_m.data.companyid  = 1;
        user_m.data.dpid       = 0;
        user_m.data.jobid      = 0;
        user_m.data.roleid     = 0;
        user_m.data.postid     = 0;
        user_m.data.created_at = 0;
        user_m.data.enddate    = 0;
        user_m.data.qrtemp     = 0;
        user_m.data.gender     = 0;
        user_m.data.nickname   = "goodname";
        user_m.data.realname   = "displayname";
        user_m.data.avatar     = "";
        user_m.data.mobile     = "";
        user_m.data.email      = "";
        user_m.data.wxuuid     = "";

        // 1) INSERT: commit_insert() builds SQL. PG gets RETURNING automatically;
        //    MySQL/SQLite emit a plain INSERT. db_conn::async_insert_query unifies
        //    the response as {affected_rows, last_id}.
        std::string newsqlstring = user_m.commit_insert();
        client << "commit_insert -&gt; " << newsqlstring << "<br />";
        auto [eff, pid] = co_await ulink->async_insert_query(newsqlstring);
        client << "async_insert_query effect_num:" << eff << ", last_id:" << pid << "<br />";
        if (ulink->iserror)
        {
            client << "insert error:" << ulink->error_msg << "<br />";
            co_return "";
        }

        // 2) UPDATE: set new value + where, commit_update() builds SQL.
        user_m.data.nickname = "nicename_b";
        user_m.where("adminid", orm::wq::eq, pid);
        newsqlstring = user_m.commit_update("nickname");
        client << "commit_update -&gt; " << newsqlstring << "<br />";
        unsigned int ueff = co_await ulink->async_edit_query(newsqlstring);
        client << "async_edit_query effect_num:" << ueff << "<br />";

        // 3) DELETE: commit_remove() builds SQL (reuses the where set above —
        // removes the row we just inserted so we don't pollute the database).
        newsqlstring = user_m.commit_remove();
        client << "commit_remove -&gt; " << newsqlstring << "<br />";
        unsigned int deff = co_await ulink->async_edit_query(newsqlstring);
        client << "async_edit_query effect_num:" << deff << "<br />";
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        co_return "";
    }
    co_return "";
}

}// namespace http
