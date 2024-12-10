#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "marproduct.h"
#include "json_reflect_headers.h"
#include "array_to_tree.h"
#include "upload_images.h"
#include "directory_fun.h"
namespace http
{
//@urlpath(admin_islogin,admin/addproduct)
std::string admin_addproduct(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto cata = orm::cms::Catalogue();
    cata.where("userid", client.session["userid"].to_int());

    cata.fetch();

    auto treedata = cata.to_tree();
    cata.record.clear();
    cata.tree_torecord(treedata);

    client.val["categorylist"].set_array();
    OBJ_ARRAY temp;
    std::vector<unsigned int> levelnum;
    for (unsigned int i = 0; i < cata.record.size(); i++)
    {
        temp["id"]       = cata.record[i].cid;
        temp["parentid"] = cata.record[i].parentid;
        temp["value"]    = cata.record[i].name;
        temp["level"]    = cata.record[i].level;

        client.val["categorylist"].push(temp);
    }

    auto topicm = orm::cms::Topic();

    topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

    unsigned int topicid = client.get["topicid"].to_int();
    //unsigned int page     = client.get["page"].to_int();
    client.val["topicid"] = topicid;

    client.val["topiclist"].set_array();
    OBJ_ARRAY topictemp;

    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {

        topictemp["id"]       = topicm.record[i].topicid;
        topictemp["parentid"] = topicm.record[i].parentid;
        topictemp["value"]    = topicm.record[i].title;
        client.val["topiclist"].push(topictemp);
    }

    auto brand = orm::cms::Brand();
    brand.where("userid", client.session["userid"].to_int());
    brand.fetch();

    client.val["brandlist"].set_array();
    OBJ_ARRAY btemp;
    for (unsigned int i = 0; i < brand.record.size(); i++)
    {
        btemp["id"]   = brand.record[i].brandid;
        btemp["name"] = brand.record[i].title;

        client.val["brandlist"].push(btemp);
    }

    peer->view("admin/addproduct");
    return "";
}

//@urlpath(admin_islogin,admin/addproductpost)
std::string admin_addproductpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto pro           = orm::cms::Product();
    pro.data.topicid   = client.post["topicid"].to_int();
    pro.data.name      = client.post["title"].to_string();
    pro.data.sntype    = client.post["sntype"].to_string();
    pro.data.introduce = client.post["introduce"].to_string();
    pro.data.brandid   = client.post["brandid"].to_int();
    pro.data.keywords  = client.post["keywords"].to_string();
    pro.data.bigimg    = client.post["icoimg"].to_string();

    pro.data.smallid = client.post["smallid"].to_int();

    pro.data.listimg     = client.post["header_urlpath"].to_json();
    pro.data.maincontent = client.post["content"].to_string();
    pro.data.samepro     = client.post["relatecontent"].to_string();
    pro.data.adddate     = get_date("%Y-%m-%d %X");
    pro.data.showtype    = 0;
    OBJ_VALUE urltemp_array;
    OBJ_ARRAY objtemp;

    urltemp_array.set_array();
    if (client.post["attach_urlpath"].is_array())
    {
        auto tempurls = client.post["attach_urlpath"].as_array();

        unsigned int nu = tempurls.size();
        for (unsigned int j = 0; j < nu; j++)
        {
            objtemp["url"]  = "";
            objtemp["name"] = "";
            if (tempurls.isset(j))
            {
                objtemp["url"] = tempurls[j];
            }
            if (client.post["attach_name"].isset(j))
            {
                objtemp["name"] = client.post["attach_name"][j];
            }
            urltemp_array.push(objtemp);
        }
    }
    pro.data.attatchfiles = urltemp_array.to_json();

    pro.setUserid(client.session["userid"].to_int());

    pro.save();

    unsigned int pid = pro.data.pid;
    std::string msg  = "Add product error!";
    if (pid > 0)
    {
        msg = "Add product success.";
    }

    client.goto_url("/admin/addproduct", 1, msg);
    return "";
}

//@urlpath(admin_islogin,admin/editproductpost)
std::string admin_editproductpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client   = peer->get_peer();
    unsigned int pid   = client.post["pid"].to_int();
    auto pro           = orm::cms::Product();
    pro.data.topicid   = client.post["topicid"].to_int();
    pro.data.name      = client.post["title"].to_string();
    pro.data.sntype    = client.post["sntype"].to_string();
    pro.data.introduce = client.post["introduce"].to_string();
    pro.data.brandid   = client.post["brandid"].to_int();
    pro.data.keywords  = client.post["keywords"].to_string();
    pro.data.bigimg    = client.post["icoimg"].to_string();

    pro.data.smallid  = client.post["smallid"].to_int();
    pro.data.showtype = client.post["showtype"].to_int();

    pro.data.listimg     = client.post["header_urlpath"].to_json();
    pro.data.maincontent = client.post["content"].to_string();
    pro.data.samepro     = client.post["relatecontent"].to_string();
    pro.data.editdate    = get_date("%Y-%m-%d %X");

    OBJ_VALUE urltemp_array;
    OBJ_ARRAY objtemp;

    urltemp_array.set_array();
    if (client.post["attach_urlpath"].is_array())
    {
        auto tempurls = client.post["attach_urlpath"].as_array();

        unsigned int nu = tempurls.size();
        for (unsigned int j = 0; j < nu; j++)
        {
            objtemp["url"]  = "";
            objtemp["name"] = "";
            if (tempurls.isset(j))
            {
                objtemp["url"] = tempurls[j];
            }
            if (client.post["attach_name"].isset(j))
            {
                objtemp["name"] = client.post["attach_name"][j];
            }
            urltemp_array.push(objtemp);
        }
    }
    pro.data.attatchfiles = urltemp_array.to_json();
    pro.where("userid", client.session["userid"].to_int()).where("pid", pid);
    pro.update("topicid,name,showtype,sntype,introduce,brandid,keywords,bigimg,smallid,listimg,maincontent,samepro,editdate,attatchfiles");

    std::string msg = "Edit product error!";
    if (pro.effect() > 0)
    {
        msg = "Edit product success.";
    }

    client.goto_url("/admin/listproduct", 1, msg);
    return "";
}

//@urlpath(admin_islogin,admin/getcategorytopproduct)
std::string admin_getcategorytopproduct(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto cata = orm::cms::Catalogue();

    unsigned int smallid = client.get["id"].to_int();

    cata.where("userid", client.session["userid"].to_int()).where("cid", smallid).fetch_one();

    unsigned int tempsmallid = cata.data.parentid;

    cata.clear();
    cata.where("userid", client.session["userid"].to_int()).where("parentid", tempsmallid).fetch();

    std::vector<unsigned int> smallid_id_array;
    smallid_id_array.push_back(smallid);
    if (cata.size() > 0)
    {
        for (auto &item : cata)
        {
            if (smallid != item.cid)
            {
                smallid_id_array.push_back(item.cid);
            }
        }
    }
    auto pro = orm::cms::Product();

    pro.where("userid", client.session["userid"].to_int()).whereIn("smallid", smallid_id_array).fetch();

    unsigned int page                                 = client.get["page"].to_int();
    auto [bar_min, bar_max, current_page, total_page] = pro.page(page, 10, 5);

    client.val["pageinfo"].set_array();
    client.val["pageinfo"]["min"]     = bar_min;
    client.val["pageinfo"]["max"]     = bar_max;
    client.val["pageinfo"]["current"] = current_page;
    client.val["pageinfo"]["total"]   = total_page;

    pro.desc("pid").fetch();
    client.val["alist"].set_array();

    OBJ_ARRAY tempa;

    if (pro.size() > 0)
    {
        for (auto &item : pro)
        {
            tempa["title"] = item.name;
            tempa["aid"]   = item.pid;

            client.val["alist"].push(tempa);
        }
    }
    client.val["code"] = 0;

    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/editproduct)
std::string admin_editproduct(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int pid = client.get["id"].to_int();

    auto pro = orm::cms::Product();
    pro.where("userid", client.session["userid"].to_int()).where("pid", pid).fetch_one();

    client.val["info"]["pid"]     = pro.data.pid;
    client.val["info"]["topicid"] = pro.data.topicid;
    client.val["info"]["smallid"] = pro.data.smallid;
    client.val["info"]["brandid"] = pro.data.brandid;
    client.val["info"]["sntype"]  = pro.data.sntype;
    client.val["info"]["name"]    = pro.data.name;

    client.val["info"]["keywords"]  = html_encode(pro.data.keywords);
    client.val["info"]["introduce"] = html_encode(pro.data.introduce);

    client.val["info"]["bigimg"]      = pro.data.bigimg;
    client.val["info"]["maincontent"] = html_encode(pro.data.maincontent);
    client.val["info"]["showtype"]    = pro.data.showtype;
    client.val["info"]["samepro"]     = pro.data.samepro;

    client.val["proimglist"].from_json(pro.data.listimg);
    client.val["profilelist"].from_json(pro.data.attatchfiles);

    client.val["topicid"]    = pro.data.topicid;
    client.val["categoryid"] = pro.data.smallid;

    auto cata = orm::cms::Catalogue();
    cata.where("userid", client.session["userid"].to_int());

    cata.fetch();

    auto treedata = cata.to_tree();
    cata.record.clear();
    cata.tree_torecord(treedata);

    client.val["categorylist"].set_array();
    OBJ_ARRAY temp;
    std::vector<unsigned int> levelnum;
    for (unsigned int i = 0; i < cata.record.size(); i++)
    {
        temp["id"]       = cata.record[i].cid;
        temp["parentid"] = cata.record[i].parentid;
        temp["value"]    = cata.record[i].name;
        temp["level"]    = cata.record[i].level;

        client.val["categorylist"].push(temp);
    }

    auto topicm = orm::cms::Topic();

    topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();
    client.val["topiclist"].set_array();
    OBJ_ARRAY topictemp;

    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {

        topictemp["id"]       = topicm.record[i].topicid;
        topictemp["parentid"] = topicm.record[i].parentid;
        topictemp["value"]    = topicm.record[i].title;
        client.val["topiclist"].push(topictemp);
    }

    auto brand = orm::cms::Brand();
    brand.where("userid", client.session["userid"].to_int());
    brand.fetch();

    client.val["brandlist"].set_array();
    OBJ_ARRAY btemp;
    for (unsigned int i = 0; i < brand.record.size(); i++)
    {
        btemp["id"]   = brand.record[i].brandid;
        btemp["name"] = brand.record[i].title;

        client.val["brandlist"].push(btemp);
    }

    peer->view("admin/editproduct");
    return "";
}

//@urlpath(admin_islogin,admin/listproduct)
std::string admin_listproduct(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int categoryid = client.get["categoryid"].to_int();

    auto cata = orm::cms::Catalogue();
    cata.where("userid", client.session["userid"].to_int());

    cata.fetch();

    auto treedata = cata.to_tree();
    cata.record.clear();
    cata.tree_torecord(treedata);

    client.val["categoryid"] = categoryid;
    client.val["categorylist"].set_array();
    OBJ_ARRAY temp;

    std::map<unsigned int, std::string> categorykv;
    for (unsigned int i = 0; i < cata.record.size(); i++)
    {
        temp["id"]       = cata.record[i].cid;
        temp["parentid"] = cata.record[i].parentid;
        temp["value"]    = cata.record[i].name;
        temp["level"]    = cata.record[i].level;

        client.val["categorylist"].push(temp);
        categorykv.insert({cata.record[i].cid, cata.record[i].name});
    }

    auto pro = orm::cms::Product();
    pro.where("userid", client.session["userid"].to_int());

    unsigned int page      = client.get["page"].to_int();
    std::string searchword = client.get["searchword"].to_string();
    searchword             = mb_substr(searchword, 0, 15);

    if (searchword.size() > 0)
    {
        pro.andsub().whereLike("name", str_addslash(searchword));
        pro.whereOrLike("maincontent", str_addslash(searchword)).andsub();
        client.val["searchword"] = searchword;
    }
    if (categoryid > 0)
    {
        pro.where("smallid", categoryid);
    }
    auto [bar_min, bar_max, current_page, total_page] = pro.page(page, 10, 5);

    client.val["pageinfo"].set_array();
    client.val["pageinfo"]["min"]     = bar_min;
    client.val["pageinfo"]["max"]     = bar_max;
    client.val["pageinfo"]["current"] = current_page;
    client.val["pageinfo"]["total"]   = total_page;

    pro.select("pid,smallid,name,bigimg,isview,isstore,ishome,sortid").desc("pid").fetch();
    client.val["alist"].set_array();
    OBJ_ARRAY tempa;

    if (pro.size() > 0)
    {
        for (auto &item : pro)
        {
            tempa["name"]    = item.name;
            tempa["pid"]     = item.pid;
            tempa["cname"]   = categorykv[item.smallid];
            tempa["isview"]  = item.isview;
            tempa["isstore"] = item.isstore;
            tempa["ishome"]  = item.ishome;
            tempa["bigimg"]  = item.bigimg;
            tempa["sortid"]  = item.sortid;
            client.val["alist"].push(tempa);
        }
    }
    peer->view("admin/listproduct");
    return "";
}

//@urlpath(admin_islogin,admin/marproductattach)
std::string admin_marproductattach(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int categoryid = client.get["categoryid"].to_int();

    auto cata = orm::cms::Catalogue();
    cata.where("userid", client.session["userid"].to_int());

    cata.fetch();

    auto treedata = cata.to_tree();
    cata.record.clear();
    cata.tree_torecord(treedata);

    client.val["categoryid"] = categoryid;
    client.val["categorylist"].set_array();
    OBJ_ARRAY temp;

    std::map<unsigned int, std::string> categorykv;
    for (unsigned int i = 0; i < cata.record.size(); i++)
    {
        temp["id"]       = cata.record[i].cid;
        temp["parentid"] = cata.record[i].parentid;
        temp["value"]    = cata.record[i].name;
        temp["level"]    = cata.record[i].level;

        client.val["categorylist"].push(temp);
        categorykv.insert({cata.record[i].cid, cata.record[i].name});
    }

    auto pro = orm::cms::Product();
    pro.where("userid", client.session["userid"].to_int());

    unsigned int page      = client.get["page"].to_int();
    std::string searchword = client.get["searchword"].to_string();
    searchword             = mb_substr(searchword, 0, 15);

    if (searchword.size() > 0)
    {
        pro.andsub().whereLike("name", str_addslash(searchword));
        pro.whereOrLike("maincontent", str_addslash(searchword)).andsub();
        client.val["searchword"] = searchword;
    }
    if (categoryid > 0)
    {
        pro.where("smallid", categoryid);
    }
    auto [bar_min, bar_max, current_page, total_page] = pro.page(page, 10, 5);

    client.val["pageinfo"].set_array();
    client.val["pageinfo"]["min"]     = bar_min;
    client.val["pageinfo"]["max"]     = bar_max;
    client.val["pageinfo"]["current"] = current_page;
    client.val["pageinfo"]["total"]   = total_page;

    pro.select("pid,smallid,name,isview,isstore,ishome,sortid,attatchfiles").desc("pid").fetch();
    client.val["alist"].set_array();
    OBJ_ARRAY tempa;

    if (pro.size() > 0)
    {
        for (auto &item : pro)
        {
            tempa["name"]  = item.name;
            tempa["pid"]   = item.pid;
            tempa["cname"] = categorykv[item.smallid];
            tempa["profilelist"].clear();
            tempa["profilelist"].from_json(item.attatchfiles);

            client.val["alist"].push(tempa);
        }
    }
    peer->view("admin/marproductattach");
    return "";
}

//@urlpath(admin_islogin,admin/deleteproduct)
std::string admin_deleteproduct(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int pid = client.get["id"].to_int();

    auto pro = orm::cms::Product();
    pro.where("userid", client.session["userid"].to_int()).where("pid", pid).remove();

    std::string msg = "Delete product error!";
    if (pro.effect() > 0)
    {
        msg = "Delete product success.";
    }
    client.goto_url("/admin/listproduct", 1, msg);
    return "";
}

//@urlpath(admin_islogin,admin/deleteproductajax)
std::string admin_deleteproductajax(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int pid = client.get["id"].to_int();

    auto pro = orm::cms::Product();
    pro.where("userid", client.session["userid"].to_int()).where("pid", pid).remove();

    std::string msg = "{\"code\":1}";
    if (pro.effect() > 0)
    {
        msg = "{\"code\":0}";
    }
    client << msg;
    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/updateproducthome)
std::string admin_updateproducthome(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int pid = client.get["id"].to_int();

    auto pro        = orm::cms::Product();
    pro.data.ishome = client.post["ishome"].to_int() > 0 ? 1 : 0;

    pro.where("userid", client.session["userid"].to_int()).where("pid", pid).update("ishome");

    std::string msg = "{\"code\":1}";
    if (pro.effect() > 0)
    {
        msg = "{\"code\":0}";
    }
    client << msg;
    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/updateproductstore)
std::string admin_updateproductstore(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int pid = client.get["id"].to_int();

    auto pro         = orm::cms::Product();
    pro.data.isstore = client.post["isstore"].to_int() > 0 ? 1 : 0;
    pro.where("userid", client.session["userid"].to_int()).where("pid", pid).update("isstore");

    std::string msg = "{\"code\":1}";
    if (pro.effect() > 0)
    {
        msg = "{\"code\":0}";
    }
    client << msg;
    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/updateproductview)
std::string admin_updateproductview(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int pid = client.get["id"].to_int();

    auto pro        = orm::cms::Product();
    pro.data.isview = client.post["isview"].to_int() > 0 ? 1 : 0;
    pro.where("userid", client.session["userid"].to_int()).where("pid", pid).update("isview");

    std::string msg = "{\"code\":1}";
    if (pro.effect() > 0)
    {
        msg = "{\"code\":0}";
    }
    client << msg;
    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/updateproductsort)
std::string admin_updateproductsort(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int pid = client.get["id"].to_int();

    auto pro        = orm::cms::Product();
    pro.data.sortid = client.get["sortid"].to_int();
    pro.where("userid", client.session["userid"].to_int()).where("pid", pid).update("sortid");

    std::string msg = "{\"code\":1}";
    if (pro.effect() > 0)
    {
        msg = "{\"code\":0}";
    }
    client << msg;
    client.out_json();
    return "";
}

}// namespace http
