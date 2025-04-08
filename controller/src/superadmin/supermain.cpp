#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "supermain.h"
#include "md5.h"
#include "request.h"
#include <filesystem>
namespace http
{

//@urlpath(null,superadmin/login)
std::string superadmin_login(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("superadmin/login");
    return "";
}
//@urlpath(null,superadmin/loginpost)
std::string superadmin_loginpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client     = peer->get_peer();
    std::string username = client.post["username"].to_string();
    std::string password = client.post["password"].to_string();
    if (username.size() < 4 || username.size() > 20 || password.size() > 20 || password.size() < 4)
    {
        client.goto_url("/superadmin/login", 3, "用户名和密码必须在4个字符以上，20字符以下！");
        return "";
    }
    auto users = orm::cms::Superadmin();
    std::string md5string;

    try
    {
        md5string           = md5(password);
        unsigned int userid = 0;
        users.where("name=", username).whereAnd("password=", md5string).fetch_one();
        // view orm create sql
        // client<<"<p>"<<users.sqlstring<<"</p>";
        auto loginfo = orm::cms::Logininfo();
        loginfo.setUsername(username);
        loginfo.setUserid(users.getAdminid());
        loginfo.setAddtime(get_date("%Y-%m-%d %X"));
        loginfo.setAddip(client.client_ip);
        loginfo.setLogtype(1);

        if (users.getPassword() == md5string && password.size() > 4)
        {
            userid = users.getAdminid();
            // save session,other page get  int userid= client.session["userid"].to_int();
            client.session["superid"] = userid;

            client.save_session();
            loginfo.setLoginstate("super成功");
            loginfo.save();
            client.goto_url("/superadmin/main");
            return "";
        }
        else
        {
            loginfo.setLoginstate("super失败");
            loginfo.save();
            client.goto_url("/superadmin/login", 3, "用户名或密码错误！");
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
//@urlpath(null,superadmin/logout)
std::string superadmin_logout(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.session["superid"] = 0;
    client.save_session();
    client.goto_url("/");
    return "";
}

//@urlpath(null,superadmin/islogin)
std::string superadmin_islogin(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    if (client.session["superid"].to_int() == 0)
    {
        return "superadmin/login";
    }
    return "ok";
}
//@urlpath(null,superadmin/isloginjson)
std::string superadmin_isloginjson(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    if (client.session["superid"].to_int() == 0)
    {
        client.val["code"] = 0;
        client.val["msg"]  = "please login";
        client.out_json();
        return "exit";
    }
    return "ok";
}
//@urlpath(superadmin_islogin,superadmin/main)
std::string superadmin_main(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (client.session["superid"].to_int() == 0)
    {
        client.goto_url("/superadmin/login");
        return "";
    }
    client.view("superadmin/main");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/editsiteinfo)
std::string superadmin_editsiteinfo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int sid = client.get["sid"].to_int();
    auto stinfo      = orm::cms::Siteinfo();
    stinfo.select("sid,sitename,sitedomain,companyname,linkname,linkmobile,isopen,sitepath,enddate").where("sid", sid).where("agentid", client.session["superid"].to_int()).fetch_one();

    client.val["info"].set_object();

    client.val["info"]["sid"]         = stinfo.data.sid;
    client.val["info"]["sitename"]    = stinfo.data.sitename;
    client.val["info"]["sitedomain"]  = stinfo.data.sitedomain;
    client.val["info"]["companyname"] = stinfo.data.companyname;
    client.val["info"]["linkname"]    = stinfo.data.linkname;
    client.val["info"]["linkmobile"]  = stinfo.data.linkmobile;
    client.val["info"]["sitepath"]    = stinfo.data.sitepath;
    client.val["info"]["isopen"]      = stinfo.data.isopen;
    client.val["info"]["expectdate"]  = get_date("%Y-%m-%d", stinfo.data.enddate);

    client.view("superadmin/editsiteinfo");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/editsiteinfopost)
std::string superadmin_editsiteinfopost(std::shared_ptr<httppeer> peer)
{
    namespace fs     = std::filesystem;
    httppeer &client = peer->get_peer();
    auto stinfo      = orm::cms::Siteinfo();
    unsigned int sid = client.post["sid"].to_int();

    stinfo.select("sitepath").where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).fetch_one();
    std::string oldsitepath = stinfo.data.sitepath;
    stinfo.clear();
    std::string sitepath = strip_domainname(client.post["sitepath"].to_string());

    if (sitepath.size() < 3 || sitepath.size() > 20)
    {
        client.goto_url("/superadmin/addsiteinfo", 3, "请输入正确的站点目录！");
        return "";
    }

    if (oldsitepath != sitepath)
    {

        auto users = orm::cms::Superadmin();
        users.where("adminid", client.session["superid"].to_int()).limit(1).fetch_one();

        std::string basepath = users.data.basesitepath;
        if (basepath.size() == 0)
        {
            basepath  = client.get_sitepath();
            auto ppos = sitepath.find_last_of("/");
            if (ppos != std::string::npos)
            {
                basepath.resize(ppos);
            }
        }
        if (basepath.size() == 0)
        {
            fs::path current_path = fs::current_path();
            basepath              = current_path.string();
            if (basepath.size() > 0 && basepath.back() != '/')
            {
                basepath += '/';
            }
            basepath.append("www/");
        }
        if (basepath.size() > 0 && basepath.back() != '/')
        {
            basepath += '/';
        }

        fs::path paths = basepath + sitepath;
        if (fs::exists(paths))
        {
            client.goto_url("/superadmin/addsiteinfo", 3, "站点目录已经存在！");
            return "";
        }

        if (fs::create_directories(paths))
        {
            fs::permissions(paths,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }
        else
        {
            client.goto_url("/superadmin/addsiteinfo", 3, "创建站点目录失败！");
            return "";
        }
    }
    std::string expdate = str_trim(client.post["expectdate"].to_string());
    if (!check_isodate(expdate))
    {
        expdate = get_date("%Y-%m-%d");
    }

    stinfo.data.sitedomain  = strip_domainname(client.post["sitedomain"].to_string());
    stinfo.data.sitename    = client.post["sitename"].to_string();
    stinfo.data.companyname = client.post["companyname"].to_string();
    stinfo.data.linkname    = client.post["linkname"].to_string();
    stinfo.data.linkmobile  = client.post["linkmobile"].to_string();
    stinfo.data.sitepath    = sitepath;
    stinfo.data.isopen      = client.post["isopen"].to_int() == 1 ? 1 : 0;
    // stinfo.data.agentid     = client.session["superid"].to_int();
    stinfo.data.enddate = strtotime(expdate);

    stinfo.where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).update("sitename, sitedomain, companyname, linkname, linkmobile, isopen,sitepath, enddate");
    client.goto_url("/superadmin/welcome");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/deletesiteinfo)
std::string superadmin_deletesiteinfo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int sid = client.get["sid"].to_int();
    auto stinfo      = orm::cms::Siteinfo();
    stinfo.select("sid,sitename,sitedomain,companyname,linkname,linkmobile,isopen,enddate").where("sid", sid).where("agentid", client.session["superid"].to_int()).fetch_one();

    client.val["info"].set_object();

    client.val["info"]["sid"]         = stinfo.data.sid;
    client.val["info"]["sitename"]    = stinfo.data.sitename;
    client.val["info"]["sitedomain"]  = stinfo.data.sitedomain;
    client.val["info"]["companyname"] = stinfo.data.companyname;
    client.val["info"]["linkname"]    = stinfo.data.linkname;
    client.val["info"]["linkmobile"]  = stinfo.data.linkmobile;

    client.view("superadmin/deletesiteinfo");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/deletesiteinfopost)
std::string superadmin_deletesiteinfopost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string msg;
    auto stinfo = orm::cms::Superadmin();

    unsigned int superid = client.session["superid"].to_int();
    stinfo.where("adminid", superid).fetch_one();

    unsigned int sid = client.post["sid"].to_int();

    std::string password = client.post["password"].to_string();
    std::string username = client.post["username"].to_string();

    std::string oldmd5str = md5(password);

    msg = "/superadmin/deletesiteinfo?sid=";
    msg.append(std::to_string(sid));
    if (oldmd5str != stinfo.data.password)
    {
        client.goto_url(msg, 3, "密码错误！");
        return "";
    }
    if (username != stinfo.data.name)
    {
        client.goto_url(msg, 3, "用户名错误！");
        return "";
    }

    auto clientinfo        = orm::cms::Siteinfo();
    clientinfo.data.isopen = 0;
    clientinfo.where("agentid", superid).whereAnd("sid", sid).fetch_one();

    if (clientinfo.data.sid == 0)
    {
        client.goto_url(msg, 3, "不存在用户！");
        return "";
    }

    auto artmodel = orm::cms::Article();
    artmodel.where("userid", sid).remove();

    auto cata = orm::cms::Catalogue();
    cata.where("userid", sid).remove();

    auto pro = orm::cms::Product();
    pro.where("userid", sid).remove();

    auto topicm = orm::cms::Topic();
    topicm.where("userid", sid).remove();

    auto users = orm::cms::Sysuser();
    users.where("companyid", sid).remove();

    clientinfo.clear();
    clientinfo.where("agentid", superid).whereAnd("sid", sid).remove();

    client.goto_url("/superadmin/welcome", 3, "删除成功！");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/addsiteinfo)
std::string superadmin_addsiteinfo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.view("superadmin/addsiteinfo");
    return "";
}
//@urlpath(superadmin_islogin,superadmin/addsiteinfopost)
std::string superadmin_addsiteinfopost(std::shared_ptr<httppeer> peer)
{
    namespace fs     = std::filesystem;
    httppeer &client = peer->get_peer();
    auto stinfo      = orm::cms::Siteinfo();

    std::string sitepath = strip_domainname(client.post["sitepath"].to_string());

    if (sitepath.size() < 3 || sitepath.size() > 20)
    {
        client.goto_url("/superadmin/addsiteinfo", 3, "请输入正确的站点目录！");
        return "";
    }
    auto users = orm::cms::Superadmin();
    users.where("adminid", client.session["superid"].to_int()).limit(1).fetch_one();

    std::string basepath = users.data.basesitepath;
    if (basepath.size() == 0)
    {
        basepath  = client.get_sitepath();
        auto ppos = sitepath.find_last_of("/");
        if (ppos != std::string::npos)
        {
            basepath.resize(ppos);
        }
    }
    if (basepath.size() == 0)
    {
        fs::path current_path = fs::current_path();
        basepath              = current_path.string();
        if (basepath.size() > 0 && basepath.back() != '/')
        {
            basepath += '/';
        }
        basepath.append("www/");
    }
    if (basepath.size() > 0 && basepath.back() != '/')
    {
        basepath += '/';
    }

    fs::path paths = basepath + sitepath;
    if (fs::exists(paths))
    {
        client.goto_url("/superadmin/addsiteinfo", 3, "站点目录已经存在！");
        return "";
    }

    if (fs::create_directories(paths))
    {
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }
    else
    {
        client.goto_url("/superadmin/addsiteinfo", 3, "创建站点目录失败！");
        return "";
    }

    std::string expdate = str_trim(client.post["expectdate"].to_string());
    if (!check_isodate(expdate))
    {
        expdate = get_date("%Y-%m-%d");
    }

    stinfo.data.sitedomain  = strip_domainname(client.post["sitedomain"].to_string());
    stinfo.data.sitename    = client.post["sitename"].to_string();
    stinfo.data.companyname = client.post["companyname"].to_string();
    stinfo.data.linkname    = client.post["linkname"].to_string();
    stinfo.data.linkmobile  = client.post["linkmobile"].to_string();
    stinfo.data.sitepath    = sitepath;
    stinfo.data.isopen      = client.post["isopen"].to_int() == 1 ? 1 : 0;
    stinfo.data.agentid     = client.session["superid"].to_int();

    stinfo.data.created_at = timeid();
    stinfo.data.enddate    = strtotime(expdate);

    stinfo.save();
    unsigned int sid = stinfo.getSid();

    stinfo.clear(false);
    stinfo.data.userid = stinfo.data.sid;
    stinfo.where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).update("userid");
    client.goto_url("/superadmin/welcome");
    return "";
}
//@urlpath(superadmin_islogin,superadmin/welcome)
std::string superadmin_welcome(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto stinfo         = orm::cms::Siteinfo();
        unsigned int total  = stinfo.select("sid,sitename,companyname,sitedomain,isopen,enddate").where("agentid", client.session["superid"].to_int()).count();
        client.val["total"] = total;
        stinfo.clear();
        stinfo.where("agentid", client.session["superid"].to_int()).desc("sid").fetch();

        client.val["infos"].set_array();
        obj_val temp;
        for (unsigned int i = 0; i < stinfo.record.size(); ++i)
        {
            temp["sid"]         = stinfo.record[i].sid;
            temp["sitename"]    = stinfo.record[i].sitename;
            temp["sitedomain"]  = stinfo.record[i].sitedomain;
            temp["companyname"] = stinfo.record[i].companyname;
            temp["isopen"]      = stinfo.record[i].isopen;
            temp["enddate"]     = stinfo.record[i].enddate;
            client.val["infos"].push(temp);
        }
    }
    catch (const std::exception &e)
    {
    }

    peer->view("superadmin/welcome");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/listuser)
std::string superadmin_listuser(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int sid = client.get["sid"].to_int();
    auto stinfo      = orm::cms::Siteinfo();

    stinfo.where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).fetch_one();

    if (stinfo.data.sid == 0)
    {
        client.goto_url("/superadmin/welcome", 3, "网站信息不存在！");
        return "";
    }

    client.val["info"].set_object();
    client.val["info"]["sitename"] = stinfo.data.sitename;
    client.val["info"]["sid"]      = stinfo.data.sid;

    auto uinfo = orm::cms::Sysuser();
    uinfo.where("companyid", sid).desc("adminid").fetch();
    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < uinfo.record.size(); ++i)
    {
        temp["nickname"] = uinfo.record[i].nickname;
        temp["name"]     = uinfo.record[i].name;
        temp["userid"]   = uinfo.record[i].adminid;
        temp["isopen"]   = uinfo.record[i].isopen;
        client.val["infos"].push(temp);
    }
    peer->view("superadmin/listuser");
    return "";
}
//@urlpath(superadmin_islogin,superadmin/edituser)
std::string superadmin_edituser(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->get_peer();
    unsigned int userid = client.get["userid"].to_int();

    auto uinfo = orm::cms::Sysuser();
    uinfo.where("adminid", userid).fetch_one();

    if (uinfo.data.adminid == 0)
    {
        client.goto_url("/superadmin/welcome", 3, "用户不存在！");
        return "";
    }
    unsigned int sid = uinfo.data.companyid;
    auto stinfo      = orm::cms::Siteinfo();
    stinfo.where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).fetch_one();
    if (stinfo.data.sid == 0)
    {
        client.goto_url("/superadmin/welcome", 3, "网站信息不存在！");
        return "";
    }

    client.val["info"].set_object();
    client.val["info"]["sitename"] = stinfo.data.sitename;
    client.val["info"]["sid"]      = stinfo.data.sid;

    client.val["info"]["nickname"]   = uinfo.data.nickname;
    client.val["info"]["username"]   = uinfo.data.name;
    client.val["info"]["userid"]     = uinfo.data.adminid;
    client.val["info"]["password"]   = uinfo.data.textword;
    client.val["info"]["email"]      = uinfo.data.email;
    client.val["info"]["mobile"]     = uinfo.data.mobile;
    client.val["info"]["isopen"]     = uinfo.data.isopen;
    client.val["info"]["expectdate"] = get_date("%Y-%m-%d", uinfo.data.enddate);

    peer->view("superadmin/edituser");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/adduser)
std::string superadmin_adduser(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int sid = client.get["sid"].to_int();
    auto stinfo      = orm::cms::Siteinfo();

    stinfo.where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).fetch_one();
    if (stinfo.data.userid == 0)
    {
        client.goto_url("/superadmin/welcome", 3, "没有站点信息！");
        return "";
    }

    client.val["info"].set_object();
    client.val["info"]["sitename"] = stinfo.data.sitename;
    client.val["info"]["sid"]      = stinfo.data.sid;
    client.val["info"]["userid"]   = stinfo.data.userid;
    peer->view("superadmin/adduser");
    return "";
}
//@urlpath(superadmin_islogin,superadmin/adduserpost)
std::string superadmin_adduserpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int sid = client.post["sid"].to_int();
    auto stinfo      = orm::cms::Siteinfo();

    stinfo.where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).fetch_one();
    if (stinfo.data.userid == 0)
    {
        client.goto_url("/superadmin/welcome", 3, "没有站点信息！");
        return "";
    }

    std::string expdate = str_trim(client.post["expectdate"].to_string());
    if (!check_isodate(expdate))
    {
        expdate = get_date("%Y-%m-%d");
    }

    auto uinfo = orm::cms::Sysuser();

    std::string username = client.post["username"].to_string();
    std::string password = client.post["password"].to_string();
    uinfo.where("name", username).fetch_one();
    std::string urlmsg = "/superadmin/adduser?sid=";
    urlmsg.append(std::to_string(sid));
    if (uinfo.data.adminid > 0)
    {
        client.goto_url(urlmsg, 3, "用户名已经存在！");
        return "";
    }
    if (username.size() > 26 || username.size() < 4)
    {
        client.goto_url(urlmsg, 3, "用户名必须在4到20之间！");
        return "";
    }
    if (password.size() > 20 || password.size() < 6)
    {
        client.goto_url(urlmsg, 3, "密码必须在4到20之间！");
        return "";
    }
    uinfo.clear();
    uinfo.data.name            = username;
    uinfo.data.password        = md5(password);
    uinfo.data.isopen          = client.post["isopen"].to_int() == 1 ? 1 : 0;
    uinfo.data.companyid       = sid;
    uinfo.data.textword        = password;
    uinfo.data.created_at      = timeid();
    uinfo.data.enddate         = strtotime(expdate);
    uinfo.data.nickname        = client.post["nickname"].to_string();
    uinfo.data.mobile          = client.post["mobile"].to_string();
    uinfo.data.email           = client.post["email"].to_string();
    auto [effect_num, last_id] = uinfo.save();
    if (effect_num == 0)
    {
        client.goto_url(urlmsg, 3, "保存出错！");
        return "";
    }
    urlmsg = "/superadmin/listuser?sid=";
    urlmsg.append(std::to_string(sid));
    client.goto_url(urlmsg, 3, "用户添加成功！");
    return "";
}
//@urlpath(superadmin_islogin,superadmin/edituserpost)
std::string superadmin_edituserpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->get_peer();
    unsigned int sid    = client.post["sid"].to_int();
    unsigned int userid = client.post["userid"].to_int();
    auto stinfo         = orm::cms::Siteinfo();

    stinfo.where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).fetch_one();
    if (stinfo.data.userid == 0)
    {
        client.goto_url("/superadmin/welcome", 3, "没有站点信息！");
        return "";
    }

    std::string expdate = str_trim(client.post["expectdate"].to_string());
    if (!check_isodate(expdate))
    {
        expdate = get_date("%Y-%m-%d");
    }
    std::string urlmsg = "/superadmin/edituser?sid=";
    urlmsg.append(std::to_string(sid));
    urlmsg.append("&userid=");
    urlmsg.append(std::to_string(userid));

    auto uinfo = orm::cms::Sysuser();
    uinfo.where("adminid", userid).whereAnd("companyid", sid).fetch_one();
    if (uinfo.data.adminid == 0)
    {
        client.goto_url(urlmsg, 3, "用户名不存在！");
        return "";
    }
    std::string oldname  = uinfo.data.name;
    std::string textword = uinfo.data.textword;

    std::string username = client.post["username"].to_string();
    std::string password = client.post["password"].to_string();

    std::string updatefield;
    uinfo.clear();
    if (oldname != username)
    {
        uinfo.where("name", username).fetch_one();

        if (uinfo.data.adminid > 0)
        {
            client.goto_url(urlmsg, 3, "用户名已经存在！");
            return "";
        }
        if (username.size() > 26 || username.size() < 4)
        {
            client.goto_url(urlmsg, 3, "用户名必须在4到20之间！");
            return "";
        }
        uinfo.clear();
        uinfo.data.name = username;
        updatefield.append("name");
    }
    if (textword != password)
    {
        if (password.size() > 20 || password.size() < 6)
        {
            client.goto_url(urlmsg, 3, "密码必须在4到20之间！");
            return "";
        }
        uinfo.data.password = md5(password);
        uinfo.data.textword = password;
        if (updatefield.size() > 0)
        {
            updatefield.append(",");
        }
        updatefield.append("password,textword");
    }
    if (updatefield.size() > 0)
    {
        updatefield.append(",");
    }
    updatefield.append("isopen,enddate,nickname,mobile,email");
    uinfo.data.isopen = client.post["isopen"].to_int() == 1 ? 1 : 0;

    uinfo.data.enddate  = strtotime(expdate);
    uinfo.data.nickname = client.post["nickname"].to_string();
    uinfo.data.mobile   = client.post["mobile"].to_string();
    uinfo.data.email    = client.post["email"].to_string();

    uinfo.where("adminid", userid).whereAnd("companyid", sid);
    if (uinfo.update(updatefield) == 0)
    {
        client.goto_url(urlmsg, 3, "没有修改内容或保存出错！");
        return "";
    }
    urlmsg = "/superadmin/listuser?sid=";
    urlmsg.append(std::to_string(sid));
    client.goto_url(urlmsg, 3, "用户修改成功！");

    return "";
}

//@urlpath(superadmin_islogin,superadmin/deleteuser)
std::string superadmin_deleteuser(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int userid = client.get["userid"].to_int();
    auto uinfo          = orm::cms::Sysuser();
    uinfo.where("adminid", userid).fetch_one();

    if (uinfo.data.adminid == 0)
    {
        client.goto_url("/superadmin/welcome", 3, "用户不存在！");
        return "";
    }
    unsigned int sid = uinfo.data.companyid;
    auto stinfo      = orm::cms::Siteinfo();
    stinfo.where("agentid", client.session["superid"].to_int()).whereAnd("sid", sid).fetch_one();
    if (stinfo.data.sid == 0)
    {
        client.goto_url("/superadmin/welcome", 3, "网站信息不存在！");
        return "";
    }
    uinfo.clear();
    uinfo.where("adminid", userid).remove();
    std::string urlmsg = "/superadmin/listuser?sid=";
    urlmsg.append(std::to_string(sid));
    client.goto_url(urlmsg, 3, "用户删除成功！");
    return "";
}
//@urlpath(superadmin_islogin,superadmin/userinfo)
std::string superadmin_userinfo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto stuser = orm::cms::Superadmin();
        stuser.where("adminid", client.session["superid"].to_int()).fetch_one();

        client.val["info"].set_object();

        client.val["info"]["userid"] = stuser.getAdminid();
        client.val["info"]["name"]   = stuser.getName();

        auto loginfo = orm::cms::Logininfo();

        loginfo.where("logtype", 1).where("adminid", stuser.getAdminid()).desc("lgid").limit(10).fetch();

        client.val["loginlist"].set_array();
        obj_val temp;
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

    peer->view("superadmin/userinfo");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/editpassword)
std::string superadmin_editpassword(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("superadmin/editpassword");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/editpwdpost)
std::string superadmin_editpwdpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string msg;

    try
    {
        auto stinfo = orm::cms::Superadmin();

        unsigned int sid = client.session["superid"].to_int();
        stinfo.where("adminid", sid).fetch_one();

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
        stinfo.where("adminid", sid);
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
