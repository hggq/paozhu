#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "main.h"
#include "md5.h"
#include <filesystem>
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
    if (username.size() < 4 || username.size() > 20 || password.size() > 20 || password.size() < 4)
    {
        client.goto_url("/admin/login", 3, "用户名和密码必须在4个字符以上，20字符以下！");
        return "";
    }
    auto users = orm::cms::Sysuser();
    std::string md5string;

    try
    {
        md5string            = md5(password);
        unsigned int userid  = 0;
        unsigned int adminid = 0;
        users.where("name=", username).whereAnd("password=", md5string).fetch_one();
        // view orm create sql
        // client<<"<p>"<<users.sqlstring<<"</p>";
        auto loginfo = orm::cms::Logininfo();
        loginfo.setUsername(username);
        loginfo.setUserid(users.getAdminid());
        loginfo.setAddtime(get_date("%Y-%m-%d %X"));
        loginfo.setAddip(client.client_ip);

        if (users.getPassword() == md5string && password.size() > 4)
        {
            userid  = users.getCompanyid();
            adminid = users.getAdminid();

            auto stinfo = orm::cms::Siteinfo();
            stinfo.where("userid", userid).limit(1).fetch_one();
            // save session,other page get  int userid= client.session["userid"].to_int();

            auto superinfo = orm::cms::Superadmin();
            superinfo.select("basesitepath").where("adminid", stinfo.getAgentid()).limit(1).fetch_one();

            if (superinfo.data.basesitepath.size() > 0 && superinfo.data.basesitepath.back() != '/')
            {
                superinfo.data.basesitepath.push_back('/');
            }
            if (superinfo.data.basesitepath.size() == 0)
            {
                superinfo.data.basesitepath = client.get_sitepath();
                if (superinfo.data.basesitepath.size() > 0 && superinfo.data.basesitepath.back() != '/')
                {
                    superinfo.data.basesitepath.push_back('/');
                }
            }
            superinfo.data.basesitepath.append(stinfo.getSitepath());
            if (superinfo.data.basesitepath.size() > 0 && superinfo.data.basesitepath.back() != '/')
            {
                superinfo.data.basesitepath.push_back('/');
            }
            std::string hosturl = stinfo.getSitedomain();

            if (stinfo.data.sitedomain.size() > 5)
            {
                if (stinfo.data.sitedomain[4] != ':' && stinfo.data.sitedomain[5] != ':')
                {
                    if (client.is_ssl())
                    {
                        hosturl = "https://" + stinfo.data.sitedomain;
                    }
                    else
                    {
                        hosturl = "http://" + stinfo.data.sitedomain;
                    }
                }
            }

            if (hosturl.size() > 0 && hosturl.back() != '/')
            {
                hosturl.push_back('/');
            }
            if (hosturl.size() == 0)
            {
                hosturl = client.get_hosturl();
                if (hosturl.size() > 0 && hosturl.back() != '/')
                {
                    hosturl.push_back('/');
                }
            }
            client.session["adminid"]  = adminid;
            client.session["userid"]   = userid;
            client.session["hosturl"]  = hosturl;
            client.session["sitepath"] = superinfo.data.basesitepath;
            client.save_session();
            loginfo.setLoginstate("成功" + stinfo.getSitepath());
            loginfo.save();
            client.goto_url("/admin/main");
            return "";
        }
        else
        {
            loginfo.setLoginstate("失败");
            loginfo.save();
            client.goto_url("/admin/login", 3, "用户名或密码错误！");
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

    client.session["userid"]  = 0;
    client.session["adminid"] = 0;
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
        client.goto_url("/admin/login");
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

        client.val["info"]["sid"]      = stinfo.getSid();
        client.val["info"]["sitename"] = stinfo.getSitename();
        //client.val["info"]["sitedomain"] = stinfo.getSitedomain();
        client.val["info"]["metakeys"] = stinfo.getMetakeys();
        client.val["info"]["metadesc"] = stinfo.getMetadesc();
        //client.val["info"]["copyright"]  = html_encode(stinfo.data.copyright);//stinfo.getCopyright();
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
        stinfo.data.sitename = client.post["sitename"].to_string();
        //stinfo.data.sitedomain = client.post["sitedomain"].to_string();
        stinfo.data.metakeys = client.post["metakeys"].to_string();
        stinfo.data.metadesc = client.post["metadesc"].to_string();

        if (sid > 0)
        {
            //stinfo.setPK(sid);
            stinfo.where("userid", client.session["userid"].to_int()).whereAnd("sid",sid);

            int result_status = stinfo.update("sitename,metakeys,metadesc");
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
            stinfo.data.userid = client.session["userid"].to_int();
            // stinfo.data.isopen = 0;
            // stinfo.save();
            // sid = stinfo.getPK();
            // if (sid > 0)
            // {
            //     msg = "添加成功！";
            // }
            // else
            // {
            //     msg = "添加失败！";
            //}
            msg = "修改失败！没有该用户。";
        }
    }
    catch (...)
    {
        msg = "数据操作有误！";
    }

    client.goto_url("/admin/siteinfo", 3, msg);
    return "";
}

//@urlpath(admin_islogin,admin/footscript)
std::string admin_footscript(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto stinfo = orm::cms::Siteinfo();
        stinfo.where("userid", client.session["userid"].to_int()).fetch_one();
        client.val["info"].set_array();

        client.val["info"]["sid"]        = stinfo.getSid();
        client.val["info"]["footscript"] = html_encode(stinfo.data.footscript);//stinfo.getCopyright();
    }
    catch (...)
    {
    }

    peer->view("admin/footscript");
    return "";
}

//@urlpath(admin_islogin,admin/footscriptpost)
std::string admin_footscriptpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string msg;
    try
    {
        auto stinfo      = orm::cms::Siteinfo();
        unsigned int sid = 0;
        sid              = client.post["sid"].to_int();

        //stinfo.data.sid    = client.post["sid"].to_int();
        stinfo.data.footscript = client.post["footscript"].to_string();

        if (sid > 0)
        {
            stinfo.setPK(sid);
            stinfo.where("userid", client.session["userid"].to_int());

            int result_status = stinfo.update("footscript");
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
            msg = "添加失败！";
        }
    }
    catch (...)
    {
        msg = "数据操作有误！";
    }

    client.goto_url("/admin/footscript", 3, msg);
    return "";
}

//@urlpath(admin_islogin,admin/copyright)
std::string admin_copyright(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto stinfo = orm::cms::Siteinfo();
        stinfo.where("userid", client.session["userid"].to_int()).fetch_one();
        client.val["info"].set_array();

        client.val["info"]["sid"]       = stinfo.getSid();
        client.val["info"]["copyright"] = html_encode(stinfo.data.copyright);//stinfo.getCopyright();
    }
    catch (...)
    {
    }

    peer->view("admin/copyright");
    return "";
}

//@urlpath(admin_islogin,admin/copyrightpost)
std::string admin_copyrightpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string msg;
    try
    {
        auto stinfo      = orm::cms::Siteinfo();
        unsigned int sid = 0;
        sid              = client.post["sid"].to_int();

        //stinfo.data.sid    = client.post["sid"].to_int();
        stinfo.data.copyright = client.post["copyright"].to_string();

        if (sid > 0)
        {
            stinfo.setPK(sid);
            stinfo.where("userid", client.session["userid"].to_int());

            int result_status = stinfo.update("copyright");
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
            msg = "添加失败！";
        }
    }
    catch (...)
    {
        msg = "数据操作有误！";
    }

    client.goto_url("/admin/copyright", 3, msg);
    return "";
}

//@urlpath(admin_islogin,admin/sitelogo)
std::string admin_sitelogo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto stinfo = orm::cms::Siteinfo();
        stinfo.where("userid", client.session["userid"].to_int()).fetch_one();
        client.val["info"].set_array();

        client.val["info"]["sid"]      = stinfo.getSid();
        client.val["info"]["sitelogo"] = html_encode(stinfo.data.sitelogo);//stinfo.getCopyright();
    }
    catch (...)
    {
    }

    peer->view("admin/sitelogo");
    return "";
}

//@urlpath(admin_islogin,admin/sitelogopost)
std::string admin_sitelogopost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string msg;
    try
    {
        auto stinfo      = orm::cms::Siteinfo();
        unsigned int sid = 0;
        sid              = client.post["sid"].to_int();

        //stinfo.data.sid    = client.post["sid"].to_int();
        stinfo.data.sitelogo = client.post["sitelogo"].to_string();

        if (sid > 0)
        {
            stinfo.setPK(sid);
            stinfo.where("userid", client.session["userid"].to_int());

            int result_status = stinfo.update("sitelogo");
            if (result_status > 0)
            {
                msg = "修改成功！";
            }
            else
            {
                msg = "修改失败或没有修改！";
            }
            result_status = 0;
            result_status = client.post["uplogotoimages"].to_int();
            if (result_status == 1)
            {
                std::string sitepath = client.session["sitepath"].to_string();
                if (sitepath.size() > 0 && sitepath.back() != '/')
                {
                    sitepath.push_back('/');
                }
                if (sitepath.size() == 0)
                {
                    sitepath = client.get_sitepath();
                }
                std::string newlogositepath = sitepath + "images/logo";

                bool ispath    = false;
                bool isgoodurl = true;

                if (stinfo.data.sitelogo.size() > 5)
                {
                    if (stinfo.data.sitelogo[4] == ':' || stinfo.data.sitelogo[5] == ':')
                    {
                        if (stinfo.data.sitelogo.size() > 6)
                        {
                            for (unsigned int i = 7; i < stinfo.data.sitelogo.size(); i++)
                            {
                                if (stinfo.data.sitelogo[i] == '/')
                                {

                                    i++;
                                    for (; i < stinfo.data.sitelogo.size(); i++)
                                    {
                                        if (stinfo.data.sitelogo[i] == '.')
                                        {
                                            if (i > 0 && stinfo.data.sitelogo[i - 1] == '.')
                                            {
                                                sitepath.clear();
                                                break;
                                            }
                                            if (ispath)
                                            {
                                                isgoodurl = false;
                                                break;
                                            }
                                            ispath = true;
                                        }
                                        else
                                        {
                                            ispath = false;
                                        }
                                        sitepath.push_back(stinfo.data.sitelogo[i]);
                                    }

                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        unsigned int i = 0;
                        if (stinfo.data.sitelogo.size() > 0 && stinfo.data.sitelogo[0] == '/')
                        {
                            i = 1;
                        }
                        for (; i < stinfo.data.sitelogo.size(); i++)
                        {
                            if (stinfo.data.sitelogo[i] == '.')
                            {
                                if (i > 0 && stinfo.data.sitelogo[i - 1] == '.')
                                {
                                    sitepath.clear();
                                    break;
                                }
                                if (ispath)
                                {
                                    isgoodurl = false;
                                    break;
                                }
                                ispath = true;
                            }
                            else
                            {
                                ispath = false;
                            }
                            sitepath.push_back(stinfo.data.sitelogo[i]);
                        }
                    }
                }
                else
                {
                    sitepath.clear();
                }

                if (sitepath.size() > 0)
                {

                    if (isgoodurl)
                    {
                        bool isshowfile          = true;
                        unsigned int extfilesize = sitepath.size();
                        if (extfilesize > 4)
                        {
                            if (sitepath[extfilesize - 1] == 'g' && sitepath[extfilesize - 2] == 'n' &&
                                sitepath[extfilesize - 3] == 'p' && sitepath[extfilesize - 4] == '.')
                            {
                                newlogositepath.append(".png");
                                isshowfile = false;
                            }
                            if (isshowfile && sitepath[extfilesize - 1] == 'g' && sitepath[extfilesize - 2] == 'p' &&
                                sitepath[extfilesize - 3] == 'j' && sitepath[extfilesize - 4] == '.')
                            {
                                newlogositepath.append(".jpg");
                                isshowfile = false;
                            }
                            if (isshowfile && sitepath[extfilesize - 1] == 'g' && sitepath[extfilesize - 2] == 'v' &&
                                sitepath[extfilesize - 3] == 's' && sitepath[extfilesize - 4] == '.')
                            {
                                newlogositepath.append(".svg");
                                isshowfile = false;
                            }

                            if (!isshowfile)
                            {
                                try
                                {
                                    if (std::filesystem::exists(newlogositepath))
                                    {
                                        std::filesystem::remove(newlogositepath);
                                    }
                                    std::filesystem::copy_file(sitepath, newlogositepath);
                                    msg = "已经复制到新的LOGO！";
                                }
                                catch (fs::filesystem_error &e)
                                {
                                    msg = "复制文件失败！";
                                    msg.append(e.what());
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            msg = "添加失败！";
        }
    }
    catch (...)
    {
        msg = "数据操作有误！";
    }

    client.goto_url("/admin/sitelogo", 3, msg);
    return "";
}

//@urlpath(admin_islogin,admin/userinfo)
std::string admin_userinfo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto stuser = orm::cms::Sysuser();
        stuser.where("adminid", client.session["adminid"].to_int()).fetch_one();

        client.val["info"].set_array();

        client.val["info"]["adminid"] = stuser.getAdminid();
        client.val["info"]["level"]   = stuser.getLevel();
        client.val["info"]["name"]    = stuser.getName();

        auto loginfo = orm::cms::Logininfo();

        loginfo.where("userid", stuser.getAdminid()).desc("lgid").limit(10).fetch();

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
        auto stinfo = orm::cms::Sysuser();

        unsigned int sid = client.session["adminid"].to_int();
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
        stinfo.data.textword = new_password;
        stinfo.where("adminid", sid);
        int result_status = stinfo.update("password,textword");

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
