#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "articles.h"
#include "json_reflect_headers.h"
#include "array_to_tree.h"

namespace http
{
//@urlpath(null,admin/addarticle)
std::string admin_addarticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    try
    {
        auto topicm = orm::cms::Topic();

        topicm.where("userid", 0).asc("parentid").fetch();

        unsigned int topicid = client.get["topicid"].to_int();
        //unsigned int page     = client.get["page"].to_int();
        client.val["topicid"] = topicid;

        client.val["list"].set_array();
        OBJ_ARRAY temp;

        for (unsigned int i = 0; i < topicm.record.size(); i++)
        {

            temp["id"]       = topicm.record[i].topicid;
            temp["parentid"] = topicm.record[i].parentid;
            temp["value"]    = topicm.record[i].title;
            client.val["list"].push(temp);
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
        client.val["msg"]  = e.what();
    }
    peer->view("admin/addarticle");
    return "";
}

//@urlpath(null,admin/addarticlepost)
std::string admin_addarticlepost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    std::string msg;
    try
    {
        auto artmodel = orm::cms::Article();

        artmodel.data.topicid       = client.post["topicid"].to_int();
        artmodel.data.title         = client.post["title"].to_string();
        artmodel.data.author        = client.post["author"].to_string();
        artmodel.data.fromsource    = client.post["fromsource"].to_string();
        artmodel.data.icoimg        = client.post["icoimg"].to_string();
        artmodel.data.keywords      = client.post["keywords"].to_string();
        artmodel.data.summary       = client.post["summary"].to_string();
        artmodel.data.content       = client.post["content"].to_string();
        artmodel.data.relatecontent = client.post["relatecontent"].to_string();
        artmodel.data.userid        = 0;
        artmodel.data.isopen        = 1;
        artmodel.data.createtime    = get_date("%Y-%m-%d %X");
        artmodel.data.addtime       = timeid();
        artmodel.data.addip         = client.client_ip;

        artmodel.save();

        unsigned int aid = 0;
        aid              = artmodel.getPK();
        if (aid > 0)
        {
            msg = "插入成功！";
        }
        else
        {
            msg = "插入失败！";
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
        client.val["msg"]  = e.what();
        msg                = "插入异常！";
    }
    client.goto_url("/admin/addarticle", 3, msg);
    return "";
}

//@urlpath(null,admin/editarticle)
std::string admin_editarticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    try
    {
        auto topicm = orm::cms::Topic();

        topicm.where("userid", 0).asc("parentid").fetch();

        unsigned int aid = client.get["id"].to_int();

        auto artmodel = orm::cms::Article();

        artmodel.where("userid", 0).whereAnd("aid", aid).limit(1).fetch();

        unsigned int topicid = artmodel.getTopicid();

        client.val["topicid"] = topicid;

        client.val["list"].set_array();
        OBJ_ARRAY temp;

        for (unsigned int i = 0; i < topicm.record.size(); i++)
        {

            temp["id"]       = topicm.record[i].topicid;
            temp["parentid"] = topicm.record[i].parentid;
            temp["value"]    = topicm.record[i].title;
            client.val["list"].push(temp);
        }
        client.val["info"].set_array();

        client.val["info"]["aid"]           = artmodel.getAid();
        client.val["info"]["title"]         = artmodel.getTitle();
        client.val["info"]["author"]        = artmodel.getAuthor();
        client.val["info"]["fromsource"]    = artmodel.getFromsource();
        client.val["info"]["icoimg"]        = artmodel.getIcoimg();
        client.val["info"]["relatecontent"] = artmodel.getRelatecontent();
        client.val["info"]["content"]       = html_encode(artmodel.getRefContent());
        client.val["info"]["summary"]       = html_encode(artmodel.getRefSummary());
        client.val["info"]["keywords"]      = html_encode(artmodel.getRefKeywords());
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
        client.val["msg"]  = e.what();
    }
    peer->view("admin/editarticle");
    return "";
}

//@urlpath(null,admin/editarticlepost)
std::string admin_editarticlepost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    std::string msg;
    try
    {
        auto artmodel    = orm::cms::Article();
        unsigned int aid = 0;
        aid              = client.post["aid"].to_int();

        artmodel.data.topicid       = client.post["topicid"].to_int();
        artmodel.data.title         = client.post["title"].to_string();
        artmodel.data.author        = client.post["author"].to_string();
        artmodel.data.fromsource    = client.post["fromsource"].to_string();
        artmodel.data.icoimg        = client.post["icoimg"].to_string();
        artmodel.data.keywords      = client.post["keywords"].to_string();
        artmodel.data.summary       = client.post["summary"].to_string();
        artmodel.data.content       = client.post["content"].to_string();
        artmodel.data.relatecontent = client.post["relatecontent"].to_string();

        artmodel.where("userid", 0).whereAnd("aid", aid).limit(1);
        int result_status =
            artmodel.update("topicid,title,author,fromsource,icoimg,keywords,summary,content,relatecontent");

        if (result_status > 0)
        {
            msg = "修改成功！";
        }
        else
        {
            msg = "修改失败！";
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
        client.val["msg"]  = e.what();
        msg                = "修改异常！";
    }
    client.goto_url("/admin/listarticle", 3, msg);
    return "";
}

//@urlpath(null,admin/deletearticle)
std::string admin_deletearticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();

    try
    {
        unsigned int aid = client.get["id"].to_int();

        auto artmodel       = orm::cms::Article();
        unsigned int tempid = artmodel.where("userid", 0).whereAnd("aid", aid).remove();
        if (tempid > 0)
        {
            client.val["code"] = 0;
        }
        else
        {
            client.val["code"] = 1;
            client.val["msg"]  = artmodel.error_msg;
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
        client.val["msg"]  = e.what();
    }
    client.out_json();
    return "";
}

//@urlpath(null,admin/gettoparticle)
std::string admin_gettoparticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();

    try
    {
        auto topicm = orm::cms::Topic();

        topicm.where("userid", 0).asc("parentid").fetch();

        unsigned int topicid  = client.get["topicid"].to_int();
        unsigned int page     = client.get["page"].to_int();
        client.val["topicid"] = topicid;

        std::vector<unsigned int> topic_id_array;//articles under this topic and sub topics

        if (topicid > 0)
        {
            topic_id_array.push_back(topicid);
        }

        for (unsigned int i = 0; i < topicm.record.size(); i++)
        {

            if (topicid > 0)
            {
                for (unsigned int j = 0; j < topic_id_array.size(); j++)
                {
                    if (topicm.record[i].parentid == topic_id_array[j])
                    {
                        topic_id_array.push_back(topicm.record[i].topicid);
                    }
                }
            }
        }

        auto artmodel = orm::cms::Article();

        artmodel.where("userid", 0);
        if (topicid > 0)
        {
            std::string topicid_sql_str = array_to_sql(topic_id_array);
            if (topicid_sql_str.size() > 0)
            {
                artmodel.whereIn("topicid", topicid_sql_str);
            }
        }
        auto [bar_min, bar_max, current_page, total_page] = artmodel.page(page, 10, 5);

        client.val["pageinfo"].set_array();
        client.val["pageinfo"]["min"]     = bar_min;
        client.val["pageinfo"]["max"]     = bar_max;
        client.val["pageinfo"]["current"] = current_page;
        client.val["pageinfo"]["total"]   = total_page;

        artmodel.desc("aid").fetch();

        client.val["alist"].set_array();
        OBJ_ARRAY tempa;

        if (artmodel.size() > 0)
        {
            for (auto &item : artmodel)
            {
                tempa["title"] = item.title;
                tempa["aid"]   = item.aid;

                client.val["alist"].push(tempa);
            }
        }
        client.val["code"] = 0;
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    client.out_json();
    return "";
}

//@urlpath(null,admin/listarticle)
std::string admin_listarticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();

    try
    {
        auto topicm = orm::cms::Topic();

        topicm.where("userid", 0).asc("parentid").fetch();

        unsigned int topicid  = client.get["topicid"].to_int();
        unsigned int page     = client.get["page"].to_int();
        client.val["topicid"] = topicid;

        client.val["list"].set_array();
        OBJ_ARRAY temp;

        std::map<unsigned int, std::string> topickv;
        std::vector<unsigned int> topic_id_array;//articles under this topic and sub topics

        if (topicid > 0)
        {
            topic_id_array.push_back(topicid);
        }

        for (unsigned int i = 0; i < topicm.record.size(); i++)
        {

            temp["id"]       = topicm.record[i].topicid;
            temp["parentid"] = topicm.record[i].parentid;
            temp["value"]    = topicm.record[i].title;
            client.val["list"].push(temp);

            topickv[topicm.record[i].topicid] = topicm.record[i].title;

            if (topicid > 0)
            {
                for (unsigned int j = 0; j < topic_id_array.size(); j++)
                {
                    if (topicm.record[i].parentid == topic_id_array[j])
                    {
                        topic_id_array.push_back(topicm.record[i].topicid);
                    }
                }
            }
        }

        auto artmodel = orm::cms::Article();

        artmodel.where("userid", 0);
        if (topicid > 0)
        {
            std::string topicid_sql_str = array_to_sql(topic_id_array);
            if (topicid_sql_str.size() > 0)
            {
                artmodel.whereIn("topicid", topicid_sql_str);
            }
        }
        auto [bar_min, bar_max, current_page, total_page] = artmodel.page(page, 10, 5);

        client.val["pageinfo"].set_array();
        client.val["pageinfo"]["min"]     = bar_min;
        client.val["pageinfo"]["max"]     = bar_max;
        client.val["pageinfo"]["current"] = current_page;
        client.val["pageinfo"]["total"]   = total_page;

        artmodel.desc("aid").fetch();

        client.val["alist"].set_array();
        OBJ_ARRAY tempa;

        if (artmodel.size() > 0)
        {
            for (auto &item : artmodel)
            {
                tempa["title"]     = item.title;
                tempa["aid"]       = item.aid;
                tempa["date"]      = item.createtime.substr(0, 10);
                tempa["topicname"] = topickv[item.topicid];

                client.val["alist"].push(tempa);
            }
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    peer->view("admin/listarticle");
    return "";
}

}//namespace http