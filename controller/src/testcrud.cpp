#include "orm.h"
#include <chrono>
#include <thread>
#include "md5.h"
#include "func.h"
#include "httppeer.h"
#include "testcrud.h"
namespace http
{

std::string articlelogin(std::shared_ptr<httppeer> peer)
{
    // step1  show login page
    peer->view("login/login");
    return "";
}
std::string articleloginpost(std::shared_ptr<httppeer> peer)
{
    // step2
    // get login/login post field
    httppeer &client     = peer->getpeer();
    std::string username = client.post["username"].to_string();
    std::string password = client.post["password"].to_string();

    auto users = orm::cms::User();
    std::string md5string;

    try
    {
        md5string = md5(password);
        users.where("name=", username).whereAnd("password=", md5string).limit(1).fetch();
        // view orm create sql
        // client<<"<p>"<<users.sqlstring<<"</p>";
        if (users.getUserid() > 0)
        {
            // save session,other page get  int userid= client.session["userid"].to_int();
            client.session["userid"] = users.getUserid();
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
std::string articlelist(std::shared_ptr<httppeer> peer)
{
    // step3 content list
    httppeer &client = peer->getpeer();
    int userid       = client.session["userid"].to_int();
    if (userid == 0)
    {
        // client.goto_url("/cms/login");
        client.val["msg"] = "<a href=\"/cms/login\">Please login </a>";
    }

    auto articles = orm::cms::Article();

    int page = client.get["page"].to_int();
    if (page < 0)
    {
        page = 0;
    }
    page = page * 20;
    articles.where("isopen=1").order(" aid desc ").limit(page, 20).fetch();
    // 也可以直接返回OBJ_VALUE 对象; 不过正常业务会要处理下结果集
    // You can also return the OBJ_VALUE object directly; but normal business process will need to process the result
    // set
    client.val["list"].set_array();
    if (articles.size() > 0)
    {
        for (auto &bb : articles)
        {

            OBJ_ARRAY item;
            item["aid"]        = bb.aid;
            item["title"]      = bb.title;
            item["createtime"] = bb.createtime;
            item["summary"]    = bb.summary;
            // client<<"<p><a href=\"/cms/show?id="<<bb.aid<<"\">"<<bb.title<<"</a>  "<<bb.createtime<<" </p>";
            client.val["list"].push(std::move(item));
        }
    }

    peer->view("cms/list");
    return "";
}
std::string articleshow(std::shared_ptr<httppeer> peer)
{
    // step4
    httppeer &client = peer->getpeer();
    auto articles    = orm::cms::Article();
    int aid          = client.get["id"].to_int();

    articles.where("isopen=1").where(" aid=", aid).limit(1).fetch();

    client.val["title"]   = articles.getTitle();
    client.val["content"] = articles.getContent();

    peer->view("cms/show");
    return "";
}
std::string articleedit(std::shared_ptr<httppeer> peer)
{
    // same the show content
    httppeer &client = peer->getpeer();
    auto articles    = orm::cms::Article();
    int aid          = client.get["id"].to_int();

    articles.where("isopen=1").where(" aid=", aid).limit(1).fetch();

    client.val["title"]   = articles.getTitle();
    client.val["content"] = html_encode(articles.getRefContent());
    client.val["aid"]     = articles.getAid();
    peer->view("cms/edit");
    return "";
}

std::string articleeditpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->getpeer();
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

    articles.where(" aid=", aid);
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

std::string articleadd(std::shared_ptr<httppeer> peer)
{
    //httppeer &client = peer->getpeer();
    peer->view("cms/add");
    return "";
}
std::string articleaddpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->getpeer();
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
        effectnum = articles.save();
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
std::string articledelete(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    unsigned int aid = client.get["id"].to_int();

    auto articles = orm::cms::Article();
    //  可以先查询是否存在或有权限之类
    // articles.where("isopen=1").where(" aid=",aid).limit(1).fetch();

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