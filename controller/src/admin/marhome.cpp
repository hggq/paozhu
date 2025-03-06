#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "marhome.h"

namespace http
{
//@urlpath(admin_islogin,admin/marhome)
std::string admin_marhome(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto homeblock = orm::cms::Homeblock();
    auto tempinfo  = homeblock.data;

    homeblock.where("userid", client.session["userid"].to_int()).asc("sortid");
    homeblock.fetch();

    client.val["list"].set_array();

    OBJ_ARRAY temp;
    for (unsigned int i = 0; i < homeblock.record.size(); i++)
    {
        temp["id"]       = homeblock.record[i].hbid;
        temp["title"]    = homeblock.record[i].title;
        temp["sortid"]   = homeblock.record[i].sortid;
        temp["viewtype"] = homeblock.record[i].viewtype;

        client.val["list"].push(temp);
    }

    peer->view("admin/marhome");
    return "";
}

//@urlpath(admin_islogin,admin/edithomeblockpic)
std::string admin_edithomeblockpic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto homeblock   = orm::cms::Homeblock();
    unsigned int id  = client.get["id"].to_int();

    homeblock.where("userid", client.session["userid"].to_int()).whereAnd("hbid", id);
    homeblock.fetch_one();

    if (homeblock.data.hbid > 0)
    {
        client.val["info"]["title"] = homeblock.data.title;
        client.val["info"]["hbid"]  = homeblock.data.hbid;

        client.val["infotopimgs"].set_array();

        if (homeblock.data.jsonconfig.size() > 0)
        {
            client.val["infotopimgs"].from_json(homeblock.data.jsonconfig);
        }
    }

    peer->view("admin/edithomepic");
    return "";
}

//@urlpath(admin_islogin,admin/updatehomeblocksort)
std::string admin_updatehomeblocksort(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto homeblock = orm::cms::Homeblock();
        homeblock.setSortid(client.get["sortid"].to_int());
        homeblock.where("userid", client.session["userid"].to_int()).whereAnd("hbid", client.get["id"].to_int());
        homeblock.update("sortid");
        client.val["code"] = 0;
        client.val["msg"]  = "ok";
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    client.out_json();

    return "";
}

//@urlpath(admin_islogin,admin/addhomepic)
std::string admin_addhomepic(std::shared_ptr<httppeer> peer)
{
    peer->view("admin/addhomepic");
    return "";
}

//@urlpath(admin_islogin,admin/addhometext)
std::string admin_addhometext(std::shared_ptr<httppeer> peer)
{
    peer->view("admin/addhometext");
    return "";
}
//@urlpath(admin_islogin,admin/addhometopic)
std::string admin_addhometopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto topicm      = orm::cms::Topic();
    // define out
    // psy::topics_tree_outjson_t a;

    topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

    client.val["list"].set_array();
    OBJ_ARRAY temp;
    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {

        temp["topicid"] = topicm.record[i].topicid;
        temp["title"]   = topicm.record[i].title;
        client.val["list"].push(temp);
    }
    peer->view("admin/addhometopic");
    return "";
}

//@urlpath(admin_islogin,admin/addhomecontent)
std::string admin_addhomecontent(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto topicm      = orm::cms::Topic();
    // define out
    // psy::topics_tree_outjson_t a;

    topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

    client.val["list"].set_array();
    OBJ_ARRAY temp;
    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {

        temp["topicid"] = topicm.record[i].topicid;
        temp["title"]   = topicm.record[i].title;
        client.val["list"].push(temp);
    }
    peer->view("admin/addhomecontent");
    return "";
}

//@urlpath(admin_islogin,admin/edithomeblockcontent)
std::string admin_edithomeblockcontent(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto topicm      = orm::cms::Topic();
    auto homeblock   = orm::cms::Homeblock();

    unsigned int id = client.get["id"].to_int();

    homeblock.where("userid", client.session["userid"].to_int()).whereAnd("hbid", id);
    homeblock.fetch_one();

    client.val["info"]["title"]   = homeblock.data.title;
    client.val["info"]["hbid"]    = homeblock.data.hbid;
    client.val["info"]["rownum"]  = homeblock.data.rownum;
    client.val["info"]["content"] = homeblock.data.content;

    auto topicArray = numstr_to_vector<unsigned int>(homeblock.data.content);

    topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

    client.val["list"].set_array();
    OBJ_ARRAY temp;
    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {

        temp["topicid"] = topicm.record[i].topicid;
        temp["title"]   = topicm.record[i].title;
        temp["checked"] = "";
        if (topicArray.size() > 0)
        {
            std::cout << " topicArray " << topicArray.size() << std::endl;
            for (unsigned int j = 0; j < topicArray.size(); j++)
            {
                std::cout << " topicid " << topicm.record[i].topicid << " | " << topicArray[j] << std::endl;
                if (topicm.record[i].topicid == topicArray[j])
                {
                    temp["checked"] = "checked=\"checked\"";
                    break;
                }
            }
        }

        client.val["list"].push(temp);
    }

    peer->view("admin/edithomecontent");
    return "";
}

//@urlpath(admin_islogin,admin/edithomeblocktopic)
std::string admin_edithomeblocktopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto topicm      = orm::cms::Topic();
    auto homeblock   = orm::cms::Homeblock();

    unsigned int id = client.get["id"].to_int();

    homeblock.where("userid", client.session["userid"].to_int()).whereAnd("hbid", id);
    homeblock.fetch_one();

    client.val["info"]["title"]   = homeblock.data.title;
    client.val["info"]["hbid"]    = homeblock.data.hbid;
    client.val["info"]["content"] = homeblock.data.content;

    auto topicArray = numstr_to_vector<unsigned int>(homeblock.data.content);

    topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

    client.val["list"].set_array();
    OBJ_ARRAY temp;
    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {

        temp["topicid"] = topicm.record[i].topicid;
        temp["title"]   = topicm.record[i].title;
        temp["checked"] = "";
        if (topicArray.size() > 0)
        {
            std::cout << " topicArray " << topicArray.size() << std::endl;
            for (unsigned int j = 0; j < topicArray.size(); j++)
            {
                std::cout << " topicid " << topicm.record[i].topicid << " | " << topicArray[j] << std::endl;
                if (topicm.record[i].topicid == topicArray[j])
                {
                    temp["checked"] = "checked=\"checked\"";
                    break;
                }
            }
        }

        client.val["list"].push(temp);
    }

    peer->view("admin/edithometopic");
    return "";
}

//@urlpath(admin_islogin,admin/edithomeblockpost)
std::string admin_edithomeblockpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int imgnum = client.post["header_urlpath"].size();
    OBJ_ARRAY temp;
    OBJ_ARRAY headerimgobj;
    std::string title = client.post["title"].to_string();
    std::string tstr;
    unsigned int id = client.post["hbid"].to_int();
    //For safety reasons, do not directly client.post.to_json();
    for (unsigned int i = 0; i < imgnum; i++)
    {
        tstr = client.post["header_urlpath"][i].to_string();
        if (tstr.length() > 0)
        {
            temp["header_urlpath"] = tstr;
            temp["header_content"] = client.post["header_content"][i].to_string();
            temp["header_title"]   = client.post["header_title"][i].to_string();
            headerimgobj.push(temp);
        }
    }
    auto homeblock = orm::cms::Homeblock();

    homeblock.data.title      = title;
    homeblock.data.jsonconfig = headerimgobj.to_json();
    homeblock.data.viewtype   = client.post["viewtype"].to_int();
    homeblock.data.gettype    = client.post["gettype"].to_int();
    homeblock.data.rownum     = client.post["rownum"].to_int();
    homeblock.data.content    = client.post["content"].to_string();
    homeblock.where("userid", client.session["userid"].to_int()).whereAnd("hbid", id);
    imgnum = homeblock.update("title,jsonconfig,viewtype,gettype,rownum,content");

    if (imgnum > 0)
    {
        client.val["code"] = 0;
        client.val["msg"]  = "ok";
    }
    else
    {
        client.val["code"] = 0;
        client.val["msg"]  = "fail";
    }

    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/addhomeblockpost)
std::string admin_addhomeblockpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int imgnum = client.post["header_urlpath"].size();
    OBJ_ARRAY temp;
    OBJ_ARRAY headerimgobj;
    std::string title = client.post["title"].to_string();
    std::string tstr;
    //For safety reasons, do not directly client.post.to_json();
    for (unsigned int i = 0; i < imgnum; i++)
    {
        tstr = client.post["header_urlpath"][i].to_string();
        if (tstr.length() > 0)
        {
            temp["header_urlpath"] = tstr;
            temp["header_content"] = client.post["header_content"][i].to_string();
            temp["header_title"]   = client.post["header_title"][i].to_string();
            headerimgobj.push(temp);
        }
    }
    auto homeblock          = orm::cms::Homeblock();
    unsigned int tempsortid = homeblock.where("userid", client.session["userid"].to_int()).count();
    homeblock.clear();

    homeblock.data.title      = title;
    homeblock.data.jsonconfig = headerimgobj.to_json();
    homeblock.data.userid     = client.session["userid"].to_int();
    homeblock.data.viewtype   = client.post["viewtype"].to_int();
    homeblock.data.sortid     = tempsortid;
    homeblock.data.gettype    = client.post["gettype"].to_int();
    homeblock.data.rownum     = client.post["rownum"].to_int();
    homeblock.data.content    = client.post["content"].to_string();

    imgnum = homeblock.save();

    if (imgnum > 0)
    {
        client.val["code"] = 0;
        client.val["msg"]  = "ok";
    }
    else
    {
        client.val["code"] = 0;
        client.val["msg"]  = "fail";
    }

    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/addhomehot)
std::string admin_addhomehot(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto topicm      = orm::cms::Topic();
    // define out
    // psy::topics_tree_outjson_t a;

    topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

    client.val["list"].set_array();
    OBJ_ARRAY temp;
    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {

        temp["topicid"] = topicm.record[i].topicid;
        temp["title"]   = topicm.record[i].title;
        client.val["list"].push(temp);
    }
    peer->view("admin/addhomehot");
    return "";
}

//@urlpath(admin_islogin,admin/edithomeblockhot)
std::string admin_edithomeblockhot(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto topicm      = orm::cms::Topic();
    auto homeblock   = orm::cms::Homeblock();

    unsigned int id = client.get["id"].to_int();

    homeblock.where("userid", client.session["userid"].to_int()).whereAnd("hbid", id);
    homeblock.fetch_one();

    client.val["info"]["title"]   = homeblock.data.title;
    client.val["info"]["hbid"]    = homeblock.data.hbid;
    client.val["info"]["rownum"]  = homeblock.data.rownum;
    client.val["info"]["content"] = homeblock.data.content;

    auto topicArray = numstr_to_vector<unsigned int>(homeblock.data.content);

    topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

    client.val["list"].set_array();
    OBJ_ARRAY temp;
    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {

        temp["topicid"] = topicm.record[i].topicid;
        temp["title"]   = topicm.record[i].title;
        temp["checked"] = "";
        if (topicArray.size() > 0)
        {
            std::cout << " topicArray " << topicArray.size() << std::endl;
            for (unsigned int j = 0; j < topicArray.size(); j++)
            {
                std::cout << " topicid " << topicm.record[i].topicid << " | " << topicArray[j] << std::endl;
                if (topicm.record[i].topicid == topicArray[j])
                {
                    temp["checked"] = "checked=\"checked\"";
                    break;
                }
            }
        }

        client.val["list"].push(temp);
    }

    peer->view("admin/edithomehot");
    return "";
}

//@urlpath(admin_islogin,admin/edithomeblocktext)
std::string admin_edithomeblocktext(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto homeblock   = orm::cms::Homeblock();

    unsigned int id = client.get["id"].to_int();

    homeblock.where("userid", client.session["userid"].to_int()).whereAnd("hbid", id);
    homeblock.fetch_one();

    client.val["info"]["title"]   = homeblock.data.title;
    client.val["info"]["hbid"]    = homeblock.data.hbid;
    client.val["info"]["rownum"]  = homeblock.data.rownum;
    client.val["info"]["content"] = html_encode(homeblock.data.content);

    peer->view("admin/edithometext");
    return "";
}

//@urlpath(admin_islogin,admin/deletehomeblock)
std::string admin_deletehomeblock(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto homeblock   = orm::cms::Homeblock();

    unsigned int id = client.get["id"].to_int();

    homeblock.where("userid", client.session["userid"].to_int()).whereAnd("hbid", id);
    id = homeblock.remove();
    if (id > 0)
    {
        client.val["code"] = 0;
        client.val["msg"]  = "ok";
    }
    else
    {
        client.val["code"] = 1;
        client.val["msg"]  = "fail";
    }
    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/homedesign)
std::string admin_homedesign(std::shared_ptr<httppeer> peer)
{
    peer->view("admin/homedesign");
    return "";
}
//@urlpath(admin_islogin,admin/edithomeblockmulit)
std::string admin_edithomeblockmulit(std::shared_ptr<httppeer> peer)
{
    peer->view("admin/edithomeblockmulit");
    return "";
}

}// namespace http
