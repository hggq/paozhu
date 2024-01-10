#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "main.h"
#include "md5.h"
namespace http
{

//@urlpath(null,admin/login)
std::string admin_login(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("admin/login");
    return "";
}
//@urlpath(null,admin/loginpost)
std::string admin_loginpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client     = peer->get_peer();
    std::string username = client.post["username"].to_string();
    std::string password = client.post["password"].to_string();

    auto users = orm::cms::User();
    std::string md5string;

    try
    {
        md5string = md5(password);
        users.where("name=", username).whereAnd("password=", md5string).fetch_one();
        // view orm create sql
        // client<<"<p>"<<users.sqlstring<<"</p>";
        auto loginfo = orm::cms::Logininfo();
        loginfo.setUserid(users.getUserid());
        loginfo.setAddtime(get_date("%Y-%m-%d %X"));
        loginfo.setAddip(client.client_ip);

        if (users.getPassword() == md5string && password.size() > 4)
        {
            // save session,other page get  int userid= client.session["userid"].to_int();
            client.session["userid"] = users.getUserid();
            client.save_session();
            loginfo.setLoginstate("成功");
            loginfo.save();
            client.goto_url("/admin/main");
            return "";
        }
        else
        {
            loginfo.setLoginstate("失败");
            loginfo.save();
            client.goto_url("admin/login", 3, "用户名或密码错误！");
            return "";
        }
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    return "";
}
//@urlpath(null,admin/logout)
std::string admin_logout(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.session["userid"] = 0;
    client.save_session();
    client.goto_url("/");
    return "";
}

//@urlpath(null,admin/islogin)
std::string admin_islogin(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    if (client.session["userid"].to_int() == 0)
    {
        return "admin/login";
    }
    return "ok";
}
//@urlpath(null,admin/isloginjson)
std::string admin_isloginjson(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    if (client.session["userid"].to_int() == 0)
    {
        client.val["code"] = 0;
        client.val["msg"]  = "please login";
        client.out_json();
        return "exit";
    }
    return "ok";
}
//@urlpath(admin_islogin,admin/main)
std::string admin_main(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (client.session["userid"].to_int() == 0)
    {
        client.goto_url("admin/login");
        return "";
    }
    client.view("admin/main");
    return "";
}
//@urlpath(admin_islogin,admin/welcome)
std::string admin_welcome(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto artmodel       = orm::cms::Article();
        unsigned int total  = artmodel.where("userid", client.session["userid"].to_int()).count();
        client.val["total"] = total;

        unsigned long long lastweek_time = timeid() - 604800;//3600*24*7

        artmodel.clear(true);
        total = artmodel.where("userid", client.session["userid"].to_int()).where("addtime", '>', lastweek_time).count();

        client.val["lastweektotal"] = total;
    }
    catch (const std::exception &e)
    {
    }

    peer->view("admin/welcome");
    return "";
}

//@urlpath(admin_islogin,admin/siteinfo)
std::string admin_siteinfo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto stinfo = orm::cms::Siteinfo();
        stinfo.where("userid", client.session["userid"].to_int()).fetch_one();

        client.val["info"].set_array();

        client.val["info"]["sid"]        = stinfo.getSid();
        client.val["info"]["sitename"]   = stinfo.getSitename();
        client.val["info"]["sitedomain"] = stinfo.getSitedomain();
        client.val["info"]["metakeys"]   = stinfo.getMetakeys();
        client.val["info"]["metadesc"]   = stinfo.getMetadesc();
        client.val["info"]["copyright"]  = stinfo.getCopyright();
    }
    catch (...)
    {
    }

    peer->view("admin/siteinfo");
    return "";
}

//@urlpath(admin_islogin,admin/siteinfopost)
std::string admin_siteinfopost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string msg;
    try
    {
        auto stinfo      = orm::cms::Siteinfo();
        unsigned int sid = 0;
        sid              = client.post["sid"].to_int();

        //stinfo.data.sid    = client.post["sid"].to_int();
        stinfo.data.sitename   = client.post["sitename"].to_string();
        stinfo.data.sitedomain = client.post["sitedomain"].to_string();
        stinfo.data.metakeys   = client.post["metakeys"].to_string();
        stinfo.data.metadesc   = client.post["metadesc"].to_string();
        stinfo.data.copyright  = client.post["copyright"].to_string();

        if (sid > 0)
        {
            stinfo.setPK(sid);
            stinfo.where("userid", client.session["userid"].to_int());

            int result_status = stinfo.update("sitename,sitedomain,metakeys,metadesc,copyright");
            if (result_status > 0)
            {
                msg = "修改成功！";
            }
            else
            {
                msg = "修改失败或没有修改！";
            }
        }
        else
        {
            stinfo.data.userid = 0;
            stinfo.save();
            sid = stinfo.getPK();
            if (sid > 0)
            {
                msg = "添加成功！";
            }
            else
            {
                msg = "添加失败！";
            }
        }
    }
    catch (...)
    {
        msg = "数据操作有误！";
    }

    client.goto_url("/admin/siteinfo", 3, msg);
    return "";
}

//@urlpath(admin_islogin,admin/userinfo)
std::string admin_userinfo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto stuser = orm::cms::User();
        stuser.where("userid", 1).fetch_one();

        client.val["info"].set_array();

        client.val["info"]["userid"] = stuser.getUserid();
        client.val["info"]["level"]  = stuser.getLevel();
        client.val["info"]["name"]   = stuser.getName();

        auto loginfo = orm::cms::Logininfo();

        loginfo.where("userid", stuser.getUserid()).desc("lgid").limit(10).fetch();

        client.val["loginlist"].set_array();
        OBJ_ARRAY temp;
        for (unsigned int i = 0; i < loginfo.record.size(); i++)
        {

            temp["id"]         = i + 1;
            temp["addtime"]    = loginfo.record[i].addtime;
            temp["addip"]      = loginfo.record[i].addip;
            temp["addregion"]  = loginfo.record[i].addregion;
            temp["loginstate"] = loginfo.record[i].loginstate;
            client.val["loginlist"].push(temp);
        }
    }
    catch (const std::exception &e)
    {
    }

    peer->view("admin/userinfo");
    return "";
}

//@urlpath(admin_islogin,admin/editpassword)
std::string admin_editpassword(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("admin/editpassword");
    return "";
}

//@urlpath(admin_islogin,admin/editpwdpost)
std::string admin_editpwdpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string msg;

    try
    {
        auto stinfo = orm::cms::User();

        unsigned int sid = 1;
        stinfo.where("userid", sid).fetch_one();

        std::string old_password = client.post["oldpassword"].to_string();
        std::string new_password;
        std::string confirm_password;

        std::string oldmd5str = md5(old_password);

        if (oldmd5str != stinfo.data.password)
        {
            client.val["msg"]    = "旧密码不同！";
            client.val["status"] = 1;
            client.out_json();
            return "";
        }

        new_password     = client.post["newpassword"].to_string();
        confirm_password = client.post["newpasswordconfirm"].to_string();

        if (new_password.size() == 0)
        {
            client.val["status"] = 3;
            client.val["msg"]    = "密码不能为空！";
            client.out_json();
            return "";
        }
        if (confirm_password != new_password && new_password.size() > 0)
        {
            client.val["status"] = 2;
            client.val["msg"]    = "前后密码不同！";
            client.out_json();
            return "";
        }

        stinfo.clear(true);
        stinfo.data.password = md5(new_password);
        stinfo.where("userid", sid);
        int result_status = stinfo.update("password");

        if (result_status > 0)
        {
            msg                  = "修改成功！";
            client.val["status"] = 0;
        }
        else
        {
            msg                  = "修改失败或没有修改！";
            client.val["status"] = 20;
        }
    }
    catch (...)
    {
        msg                  = "数据操作有误！";
        client.val["status"] = 10;
    }
    client.val["msg"] = msg;
    client.out_json();
    return "";
}

}//namespace http
