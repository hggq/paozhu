#include "orm.h"
#include <chrono>
#include <thread>
#include "md5.h"
#include "func.h"
#include "httppeer.h"
#include "request.h"
#include "testcrud.h"
namespace http
{

//@urlpath(null,cms/login)
std::string articlelogin(std::shared_ptr<httppeer> peer)
{
    // step1  show login page
    // 显示登录页面
    peer->view("login/login");
    return "";
}

//@urlpath(null,cms/islogin)
std::string articleislogin(std::shared_ptr<httppeer> peer)
{
    //判断是否已经登录了，主要是修改内容时候需要权限
    //Please check if you're logged in, since permissions are needed for modifying content.
    httppeer &client = peer->get_peer();

    if (client.session["userid"].to_int() == 0)
    {
        return "cms/login";
    }
    return "ok";
}

//@urlpath(null,cms/loginpost)
std::string articleloginpost(std::shared_ptr<httppeer> peer)
{
    // step2
    httppeer &client     = peer->get_peer();
    //注意必须处理用户输入的字符串，预防攻击
    //Be sure to handle user-input strings to prevent attacks
    std::string username = client.post["username"].to_escape(); 
    std::string password = client.post["password"].to_string();

    auto users = orm::cms::Sysuser();
    std::string md5string;

    try
    {
        md5string = md5(password);
        users.where("name", username).whereAnd("password", md5string).fetch_one();
        // view orm create sql
        // client<<"<p>"<<users.sqlstring<<"</p>";
        if (users.getAdminid() > 0)
        {
            // save session,other page get  int userid= client.session["userid"].to_int();
            // 保存已经登录信息，供其它页面使用，会输出一个sessionid到浏览器
            client.session["userid"] = users.getCompanyid();
            client.save_session();
            client.goto_url("/cms/list");
            return "";
        }
        else
        {
            client.goto_url("/cms/login", 3, "用户名或密码错误！");
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

//@urlpath(null,cms/list)
std::string articlelist(std::shared_ptr<httppeer> peer)
{
    // step3 content list
    try
    {
        httppeer &client = peer->get_peer();
        int userid       = client.session["userid"].to_int();
        if (userid == 0)
        {
            return "";
        }
        auto articles = orm::cms::Article();
        int page      = client.get["page"].to_int();
        if (page < 0)
        {
            page = 0;
        }

        articles.where("isopen",1);
        auto [bar_min, bar_max, current_page, total_page] = articles.page(page, 10, 5);

        client.val["pageinfo"].set_object();
        client.val["pageinfo"]["min"]     = bar_min;
        client.val["pageinfo"]["max"]     = bar_max;
        client.val["pageinfo"]["current"] = current_page;
        client.val["pageinfo"]["total"]   = total_page;

        //自动分页 automatic pagination
        articles.order(" aid desc ").fetch();
        // 也可以直接返回OBJ_VALUE 对象; 不过正常业务会要处理下结果集
        // You can also return the OBJ_VALUE object directly; but normal business process will need to process the result
        // set
        if (articles.error_msg.size() > 0)
        {
            client.val["msg"] = client.val["msg"].to_string() + articles.error_msg;
        }
        client.val["list"].set_array();
        if (articles.size() > 0)
        {
            obj_val item;
            for (auto &bb : articles)
            {
                item["aid"]        = bb.aid;
                item["title"]      = bb.title;
                item["createtime"] = bb.createtime;
                item["summary"]    = bb.summary;
                // client<<"<p><a href=\"/cms/show?id="<<bb.aid<<"\">"<<bb.title<<"</a>  "<<bb.createtime<<" </p>";
                client.val["list"].push(std::move(item));
            }
        }
        peer->view("cms/list");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return "";
}

//@urlpath(null,cms/show)
std::string articleshow(std::shared_ptr<httppeer> peer)
{
    // step4
    httppeer &client = peer->get_peer();
    auto articles    = orm::cms::Article();
    int aid          = client.get["id"].to_int();

    articles.where("isopen",1).where(" aid", aid).fetch_one();

    client.val["title"]   = articles.getTitle();
    client.val["content"] = articles.getContent();

    peer->view("cms/show");
    return "";
}
//@urlpath(articleislogin,cms/edit)
std::string articleedit(std::shared_ptr<httppeer> peer)
{
    // same the show content
    httppeer &client = peer->get_peer();
    auto articles    = orm::cms::Article();
    int aid          = client.get["id"].to_int();

    articles.where("isopen",1).where("aid", aid).fetch_one();

    client.val["title"]   = articles.getTitle();
    client.val["content"] = html_encode(articles.getRefContent());
    client.val["aid"]     = articles.getAid();
    peer->view("cms/edit");
    return "";
}

//@urlpath(articleislogin,cms/editpost)
std::string articleeditpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->get_peer();
    std::string title   = client.post["title"].to_string();
    std::string content = client.post["content"].to_string();
    unsigned int aid    = client.post["aid"].to_int();

    auto articles = orm::cms::Article();
    // articles.where("isopen=1").where(" aid=",aid).limit(1).fetch();
    // articles.data.aid=aid;
    // articles.data.title=title;
    // articles.setAid(aid);
    articles.setTitle(title);
    // articles.setTitle("直接标题");
    articles.setContent(content);

    articles.where("aid", aid);
    int effectnum = 0;
    try
    {
        effectnum = articles.update("title,content");
    }
    catch (std::exception &e)
    {
        client << "<p>" << articles.sqlstring << "</p>";
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    if (effectnum > 0)
    {

        client.goto_url("/cms/list", 3, "内容已经更新");
        return "";
    }
    else
    {
        client.goto_url("/cms/list", 3, "更新出错(error)");
        return "";
    }

    return "";
}

//@urlpath(articleislogin,cms/add)
std::string articleadd(std::shared_ptr<httppeer> peer)
{
    // 添加一个 articleislogin 登录拦截
    // Add an articleislogin login interceptor
    //httppeer &client = peer->get_peer();
    peer->view("cms/add");
    return "";
}

//@urlpath(articleislogin,cms/addpost)
std::string articleaddpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->get_peer();
    std::string title   = client.post["title"].to_string();
    std::string content = client.post["content"].to_string();
    unsigned int aid    = 0;

    auto articles = orm::cms::Article();

    // articles.data.aid=aid;
    // articles.data.title=title;
    // articles.setAid(aid);
    articles.setIsopen(1);
    articles.setCreatetime(get_date("%Y-%m-%d %X"));// Y-m-d H:i:s
    articles.setAddtime(timeid());                  // unix timestamp
    articles.setAddip(client.client_ip);            // client ip
    articles.setTitle(title);
    // articles.setTitle("直接标题");
    articles.setContent(content);

    int effectnum = 0;
    try
    {
        auto[effectnum,last_id] = articles.save();
        aid       = articles.getAid();
        client << "<p>新(new)id " << aid << " 或 新(new)id " << effectnum << "</p>";
    }
    catch (std::exception &e)
    {
        client << "<p>" << articles.sqlstring << "</p>";
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    if (effectnum > 0)
    {

        client.goto_url("/cms/list", 3, "内容已经添加");
        return "";
    }
    else
    {
        client.goto_url("/cms/list", 3, "添加出错(error)");
        return "";
    }

    return "";
}

//@urlpath(articleislogin,cms/delete)
std::string articledelete(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int aid = client.get["id"].to_int();

    auto articles = orm::cms::Article();
    //  可以先查询是否存在或有权限之类
    // articles.where("isopen",1).where("aid",aid).limit(1).fetch();

    int effectnum = 0;
    try
    {
        effectnum = articles.remove(aid);
    }
    catch (std::exception &e)
    {
        client << "<p>" << articles.sqlstring << "</p>";
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    if (effectnum > 0)
    {

        client.goto_url("/cms/list", 3, "内容已经删除");
        return "";
    }
    else
    {
        client.goto_url("/cms/list", 3, "删除出错(error)");
        return "";
    }

    return "";
}
}//namespace http