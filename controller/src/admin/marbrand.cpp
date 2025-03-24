#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "marbrand.h"

namespace http
{
//@urlpath(admin_islogin,admin/marbrand)
std::string admin_marbrand(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto brand    = orm::cms::Brand();
    auto tempinfo = brand.data;

    brand.where("userid", client.session["userid"].to_int());
    brand.fetch();

    client.val["list"].set_array();
    obj_val btemp;
    unsigned int editid  = client.get["id"].to_int();
    unsigned int topicid = 0;

    auto topicm = orm::cms::Topic();
    std::map<unsigned int, std::string> topickv;
    topickv.emplace(0, "无类别");
    //cateid 24 brand type
    topicm.where("userid", client.session["userid"].to_int()).where("cateid", 24).asc("parentid").fetch();

    client.val["topiclist"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < topicm.record.size(); i++)
    {
        temp["id"]   = topicm.record[i].topicid;
        temp["name"] = topicm.record[i].title;
        topickv.emplace(topicm.record[i].topicid, topicm.record[i].title);
        client.val["topiclist"].push(temp);
    }

    for (unsigned int i = 0; i < brand.record.size(); i++)
    {
        btemp["id"]     = brand.record[i].brandid;
        btemp["logo"]   = brand.record[i].logo;
        btemp["name"]   = brand.record[i].title;
        btemp["sortid"] = brand.record[i].sortid;
        auto iter       = topickv.find(brand.record[i].topicid);
        if (iter != topickv.end())
        {
            btemp["cname"] = iter->second;
        }
        else
        {
            btemp["cname"] = "无类别";
        }

        client.val["list"].push(btemp);
        if (editid == brand.record[i].brandid)
        {
            tempinfo.brandid       = brand.record[i].brandid;
            tempinfo.topicid       = brand.record[i].topicid;
            tempinfo.logo          = brand.record[i].logo;
            tempinfo.title         = brand.record[i].title;
            tempinfo.introduce     = brand.record[i].introduce;
            tempinfo.detailcontent = brand.record[i].detailcontent;

            topicid = brand.record[i].topicid;
        }
    }

    client.val["info"]["topicid"]       = tempinfo.topicid;
    client.val["info"]["brandid"]       = tempinfo.brandid;
    client.val["info"]["title"]         = tempinfo.title;
    client.val["info"]["logo"]          = tempinfo.logo;
    client.val["info"]["detailcontent"] = html_encode(tempinfo.detailcontent);
    client.val["info"]["introduce"]     = html_encode(tempinfo.introduce);

    client.val["editid"]  = editid;
    client.val["topicid"] = topicid;
    peer->view("admin/marbrand");
    return "";
}

//@urlpath(admin_islogin,admin/brandaddpost)
std::string admin_brandaddpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto brand = orm::cms::Brand();

    unsigned int brandid = client.post["brandid"].to_int();

    if (brandid == 0)
    {
        brand.setTopicid(client.post["topicid"].to_int());
        brand.setTitle(client.post["topicname"].to_string());
        brand.setIntroduce(client.post["introduce"].to_string());
        brand.setDetailcontent(client.post["content"].to_string());
        brand.setLogo(client.post["icoimg"].to_string());
        brand.setUserid(client.session["userid"].to_int());
        brand.save();
    }
    else
    {
        brand.setTopicid(client.post["topicid"].to_int());
        brand.setTitle(client.post["topicname"].to_string());
        brand.setIntroduce(client.post["introduce"].to_string());
        brand.setDetailcontent(client.post["content"].to_string());
        brand.setLogo(client.post["icoimg"].to_string());

        brand.where("userid", client.session["userid"].to_int()).where("brandid", brandid);
        brand.update("topicid,title,logo,introduce,detailcontent");
    }

    client.goto_url("/admin/marbrand", 1);
    return "";
}
//@urlpath(admin_islogin,admin/deletebrand)
std::string admin_deletebrand(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto brand = orm::cms::Brand();

    unsigned int brandid = client.get["id"].to_int();

    brand.where("userid", client.session["userid"].to_int()).where("brandid", brandid);
    brand.remove();

    client.goto_url("/admin/marbrand", 1, "Delete ...");
    return "";
}
//@urlpath(admin_isloginjson,admin/updatebrandsort)
std::string admin_updatebrandsort(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto brand = orm::cms::Brand();
        brand.setSortid(client.get["sortid"].to_int());
        brand.where("userid", client.session["userid"].to_int()).whereAnd("brandid", client.get["id"].to_int());
        auto isok = brand.update("sortid");

        client.val["code"] = 0;
        if (isok > 0)
        {
            client.val["msg"] = "ok";
        }
        else
        {
            client.val["msg"] = "error!";
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    client.out_json();
    return "";
}
}// namespace http
