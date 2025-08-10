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
//@urlpath(admin_islogin,admin/addarticle)
std::string admin_addarticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto topicm = orm::cms::Topic();

        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

        unsigned int topicid = client.get["topicid"].to_int();
        //unsigned int page     = client.get["page"].to_int();
        client.val["topicid"] = topicid;

        client.val["list"].set_array();
        obj_val temp;

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

//@urlpath(admin_islogin,admin/addarticlepost)
std::string admin_addarticlepost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
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
        artmodel.data.texturl       = client.post["texturl"].to_string();
        artmodel.data.relatecontent = client.post["relatecontent"].to_string();
        artmodel.data.userid        = client.session["userid"].to_int();
        artmodel.data.isopen        = 1;
        artmodel.data.showtype      = 0;
        artmodel.data.createtime    = get_date("%Y-%m-%d %X");
        artmodel.data.addtime       = timeid();
        artmodel.data.addip         = client.client_ip;

        msg.clear();
        for (unsigned int i = 0; i < artmodel.data.relatecontent.size(); i++)
        {
            if (artmodel.data.relatecontent[i] >= '0' && artmodel.data.relatecontent[i] <= '9')
            {
                msg.push_back(artmodel.data.relatecontent[i]);
            }
            else if (artmodel.data.relatecontent[i] == ',')
            {
                if (msg.size() > 0)
                {
                    msg.push_back(',');
                }
            }
        }
        artmodel.data.relatecontent = msg;
        
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

//@urlpath(admin_islogin,admin/editarticle)
std::string admin_editarticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto topicm = orm::cms::Topic();

        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

        unsigned int aid = client.get["id"].to_int();

        auto artmodel = orm::cms::Article();

        artmodel.where("userid", client.session["userid"].to_int()).whereAnd("aid", aid).fetch_one();

        unsigned int topicid = artmodel.getTopicid();

        client.val["topicid"] = topicid;

        client.val["list"].set_array();
        obj_val temp;

        for (unsigned int i = 0; i < topicm.record.size(); i++)
        {

            temp["id"]       = topicm.record[i].topicid;
            temp["parentid"] = topicm.record[i].parentid;
            temp["value"]    = topicm.record[i].title;
            client.val["list"].push(temp);
        }
        client.val["info"].set_object();

        client.val["info"]["aid"]           = artmodel.getAid();
        client.val["info"]["title"]         = artmodel.getTitle();
        client.val["info"]["author"]        = artmodel.getAuthor();
        client.val["info"]["fromsource"]    = artmodel.getFromsource();
        client.val["info"]["icoimg"]        = artmodel.getIcoimg();
        client.val["info"]["relatecontent"] = artmodel.getRelatecontent();
        client.val["info"]["texturl"]       = artmodel.getTexturl();
        client.val["info"]["keywords"]      = artmodel.getKeywords();
        client.val["info"]["content"]       = html_encode(artmodel.getRefContent());
        client.val["info"]["summary"]       = html_encode(artmodel.getRefSummary());
        client.val["info"]["showtype"]      = artmodel.getShowtype();
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
        client.val["msg"]  = e.what();
    }
    peer->view("admin/editarticle");
    return "";
}

//@urlpath(admin_islogin,admin/editarticlepost)
std::string admin_editarticlepost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
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
        artmodel.data.texturl       = client.post["texturl"].to_string();
        artmodel.data.relatecontent = client.post["relatecontent"].to_string();
        artmodel.data.showtype      = client.post["showtype"].to_int();

        msg.clear();
        for (unsigned int i = 0; i < artmodel.data.relatecontent.size(); i++)
        {
            if (artmodel.data.relatecontent[i] >= '0' && artmodel.data.relatecontent[i] <= '9')
            {
                msg.push_back(artmodel.data.relatecontent[i]);
            }
            else if (artmodel.data.relatecontent[i] == ',')
            {
                if (msg.size() > 0)
                {
                    msg.push_back(',');
                }
            }
        }
        artmodel.data.relatecontent = msg;

        artmodel.where("userid", client.session["userid"].to_int()).whereAnd("aid", aid).limit(1);
        int result_status =
            artmodel.update("topicid,title,author,fromsource,icoimg,keywords,summary,showtype,content,texturl,relatecontent");

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

//@urlpath(admin_isloginjson,admin/deletearticle)
std::string admin_deletearticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        unsigned int aid = client.get["id"].to_int();

        auto artmodel       = orm::cms::Article();
        unsigned int tempid = artmodel.where("userid", client.session["userid"].to_int()).whereAnd("aid", aid).remove();
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

//@urlpath(admin_isloginjson,admin/gettoparticle)
std::string admin_gettoparticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto topicm = orm::cms::Topic();

        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

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
                        break;
                    }
                }
            }
        }

        auto artmodel = orm::cms::Article();

        artmodel.where("userid", client.session["userid"].to_int());
        if (topicid > 0)
        {
            std::string topicid_sql_str = array_to_sql(topic_id_array);
            if (topicid_sql_str.size() > 0)
            {
                artmodel.whereIn("topicid", topicid_sql_str);
            }
        }
        auto [bar_min, bar_max, current_page, total_page] = artmodel.page(page, 10, 5);

        client.val["pageinfo"].set_object();
        client.val["pageinfo"]["min"]     = bar_min;
        client.val["pageinfo"]["max"]     = bar_max;
        client.val["pageinfo"]["current"] = current_page;
        client.val["pageinfo"]["total"]   = total_page;

        artmodel.desc("aid").fetch();

        client.val["alist"].set_array();
        obj_val tempa;

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

//@urlpath(admin_isloginjson,admin/updatearticlesort)
std::string admin_updatearticlesort(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto artmodel = orm::cms::Article();
        artmodel.setSortid(client.get["sortid"].to_int());
        artmodel.where("userid", client.session["userid"].to_int()).whereAnd("aid", client.get["id"].to_int());
        artmodel.update("sortid");
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
//@urlpath(admin_isloginjson,admin/updatearticleview)
std::string admin_updatearticleview(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto artmodel        = orm::cms::Article();
        unsigned char isview = client.post["isview"].to_int() > 0 ? 1 : 0;

        artmodel.setIsopen(isview);
        artmodel.where("userid", client.session["userid"].to_int()).whereAnd("aid", client.get["id"].to_int());
        client.val["code"] = artmodel.update("isopen");
        client.val["msg"]  = "ok";
    }
    catch (std::exception &e)
    {
        client.val["code"] = 0;
    }
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/updatearticleishome)
std::string admin_updatearticleishome(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto artmodel        = orm::cms::Article();
        unsigned char ishome = client.post["ishome"].to_int() > 0 ? 1 : 0;

        artmodel.setIshome(ishome);
        artmodel.where("userid", client.session["userid"].to_int()).whereAnd("aid", client.get["id"].to_int());
        client.val["code"] = artmodel.update("ishome");
        client.val["msg"]  = "ok";
    }
    catch (std::exception &e)
    {
        client.val["code"] = 0;
    }
    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/listarticle)
std::string admin_listarticle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    try
    {
        auto topicm = orm::cms::Topic();
        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

        unsigned int topicid   = client.get["topicid"].to_int();
        unsigned int page      = client.get["page"].to_int();
        std::string searchword = client.get["searchword"].to_string();
        searchword             = mb_substr(searchword, 0, 15);
        client.val["topicid"]  = topicid;

        client.val["list"].set_array();
        obj_val temp;

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
                        break;
                    }
                }
            }
        }

        auto artmodel = orm::cms::Article();
        artmodel.where("userid", client.session["userid"].to_int());
        if (topicid > 0)
        {
            std::string topicid_sql_str = array_to_sql(topic_id_array);
            if (topicid_sql_str.size() > 0)
            {
                artmodel.whereIn("topicid", topicid_sql_str);
            }
        }
        if (searchword.size() > 0)
        {
            artmodel.andsub().whereLike("title", str_addslash(searchword));
            artmodel.whereOrLike("content", str_addslash(searchword)).andsub();
            client.val["searchword"] = searchword;
        }
        auto [bar_min, bar_max, current_page, total_page] = artmodel.page(page, 10, 5);

        client.val["pageinfo"].set_object();
        client.val["pageinfo"]["min"]     = bar_min;
        client.val["pageinfo"]["max"]     = bar_max;
        client.val["pageinfo"]["current"] = current_page;
        client.val["pageinfo"]["total"]   = total_page;

        artmodel.select("aid,topicid,title,createtime,sortid,isopen,ishome").desc("aid").fetch();
        client.val["alist"].set_array();
        obj_val tempa;

        if (artmodel.size() > 0)
        {
            for (auto &item : artmodel)
            {
                tempa["title"]     = item.title;
                tempa["aid"]       = item.aid;
                tempa["date"]      = item.createtime.substr(0, 10);
                tempa["topicname"] = topickv[item.topicid];
                tempa["sortid"]    = item.sortid;
                tempa["isopen"]    = item.isopen;
                tempa["ishome"]    = item.ishome;
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
