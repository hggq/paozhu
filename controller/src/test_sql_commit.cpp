#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "test_sql_commit.h"
#include "mysql_query.h"

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

}

namespace http
{
//@urlpath(null,test_sql_commit)
std::string test_sql_commit(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  alone use sqlquery ";

    try
    {
        //返回结果结构体，如果只有一条可以使用 orm::cust::LocalusersqlStruct loaduser 单独一个
        // Return the result struct. If there is only one, you can use orm::cust::LocalusersqlStruct loaduser alone
        orm::cust::LocalusersqlStruct loaduser;
        //独立使用数据库 db conn  Use database db conn independently
        //cms为orm.conf配置文件标签 cms is the tag for the orm.conf configuration file
        auto  ulink= std::make_unique<orm::db_conn>("cms"); 
        //select 字段要和返回结果结构体字段对得上
        //The "select" field must correspond to the field of the returned result struct
        std::string sqlstring="SELECT adminid,name,nickname,postid FROM ";
        sqlstring.append("sysuser");
        sqlstring.append(" where 1 limit 1");

        //同步模式 synchronous mode
        ulink->query(sqlstring,loaduser);
 
        client << "sql:"<<sqlstring<<"<hr>";
        if (loaduser.adminid != 0)
        {
            client << "adminid:";
            client << loaduser.adminid ;
            client << ", name:";
            client << loaduser.name ;
            client << ", nickname:";
            client << loaduser.nickname ;
            client << ", postid:";
            client << loaduser.postid ;
            client << "<br />";
        }

       bool is_begin =  ulink->begin_commit();
       orm::cust::LocalusersqlStruct commit_data; 
       if(is_begin)
       {
          try
          {
            sqlstring="update sysuser set nickname = '好听名字', postid = postid + 1 where adminid = 10000 ";
            ulink->edit_query(sqlstring);
            if(ulink->iserror)
            {
                ulink->rollback();
            }
            else
            {
                sqlstring="INSERT INTO `sysuser` (`adminid`, `name`, `password`, `textword`, `isopen`, `level`, `companyid`, `dpid`, `jobid`, `roleid`, `postid`, `created_at`, `enddate`, `qrtemp`, `gender`, `nickname`, `realname`, `avatar`, `mobile`, `email`, `wxuuid`) VALUES (NULL, 'userroot', 'e10adc3949ba59abbe56e057f20f883e', '123456', '1', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '好名', '住宿名', '', '', '', '');";
                ulink->edit_query(sqlstring);
                if(ulink->iserror)
                {
                    ulink->rollback();
                }
                else
                {
                    //select go edit conn link
                    sqlstring="SELECT adminid,name,nickname,postid FROM sysuser where adminid = 10000 limit 1 ";
                    ulink->edit_query(sqlstring, commit_data);
                    if(ulink->iserror)
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
          catch(const std::exception& e)
          {
            ulink->rollback();
            std::cerr << e.what() << '\n';
          }
       } 

       client << "commit:" << ulink->error_msg <<"<br />";
        if (commit_data.adminid != 0)
        {
            client << "adminid:";
            client << commit_data.adminid ;
            client << ", name:";
            client << commit_data.name ;
            client << ", nickname:";
            client << commit_data.nickname ;
            client << ", postid:";
            client << commit_data.postid ;
            client << "<br />";
        }

        is_begin =  ulink->begin_commit();

        if(is_begin)
       {
          try
          {
            sqlstring="update sysuser set nickname = '新命名字', postid = postid + 1 where adminid = 10000 ";
            ulink->edit_query(sqlstring);
            if(ulink->iserror)
            {
                ulink->rollback();
            }
            else
            {
                // insert has 10003, must to error; 
                sqlstring="INSERT INTO `sysuser` (`adminid`, `name`, `password`, `textword`, `isopen`, `level`, `companyid`, `dpid`, `jobid`, `roleid`, `postid`, `created_at`, `enddate`, `qrtemp`, `gender`, `nickname`, `realname`, `avatar`, `mobile`, `email`, `wxuuid`) VALUES (10003, 'userroot', 'e10adc3949ba59abbe56e057f20f883e', '123456', '1', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '好名', '住宿名', '', '', '', '');";
                ulink->edit_query(sqlstring);
                if(ulink->iserror)
                {
                    ulink->rollback();
                }
                else
                {
                    //select go edit conn link
                    sqlstring="SELECT adminid,name,nickname,postid FROM sysuser where adminid = 10000 limit 1 ";
                    ulink->edit_query(sqlstring, commit_data);
                    if(ulink->iserror)
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
          catch(const std::exception& e)
          {
            ulink->rollback();
            std::cerr << e.what() << '\n';
          }
       } 

        client << "commit:" << ulink->error_msg <<"<br />";
        if (commit_data.adminid != 0)
        {
            client << "adminid:";
            client << commit_data.adminid ;
            client << ", name:";
            client << commit_data.name ;
            client << ", nickname:";
            client << commit_data.nickname ;
            client << ", postid:";
            client << commit_data.postid ;
            client << "<br />";
        }
 
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    return "";
}

 
}// namespace http