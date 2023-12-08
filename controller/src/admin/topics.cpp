#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "topics.h"
#include "json_reflect_headers.h"
#include "array_to_tree.h"
namespace http
{
//@urlpath(admin_islogin,admin/addtopic)
std::string admin_addtopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();

    try
    {
        auto topicm = orm::cms::Topic();
        // define out
        // psy::topics_tree_outjson_t a;

        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

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
    }

    peer->view("admin/addtopic");
    return "";
}

//@urlpath(admin_islogin,admin/edittopic)
std::string admin_edittopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();

    try
    {
        auto topicm = orm::cms::Topic();
        // define out
        // psy::topics_tree_outjson_t a;

        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

        client.val["list"].set_array();
        OBJ_ARRAY temp;
        for (unsigned int i = 0; i < topicm.record.size(); i++)
        {
            temp["id"]       = topicm.record[i].topicid;
            temp["parentid"] = topicm.record[i].parentid;
            temp["value"]    = topicm.record[i].title;
            client.val["list"].push(temp);
        }

        topicm.clear(true);
        unsigned int topicid = client.get["id"].to_int();
        if (topicid > 0)
        {
            topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicid", topicid).limit(1).fetch();
            client.val["info"].set_array();
            client.val["info"]["topicid"]     = topicm.data.topicid;
            client.val["info"]["parentid"]    = topicm.data.parentid;
            client.val["info"]["topicname"]   = topicm.data.title;
            client.val["info"]["topictype"]   = topicm.data.cateid;
            client.val["info"]["topicstatus"] = topicm.data.isview == 1 ? true : false;
            client.val["info"]["urlpath"]     = topicm.data.urlpath;
            client.val["info"]["memo"]        = topicm.data.memo;

            client.val["infotopimgs"].set_array();
            client.val["infotopimgs"].from_json(topicm.data.topimg);
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }

    peer->view("admin/edittopic");
    return "";
}

//@urlpath(admin_islogin,admin/martopic)
std::string admin_martopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();

    try
    {
        auto topicm = orm::cms::Topic();
        // define out
        psy::topics_tree_outjson_t topicone;
        std::vector<psy::topics_tree_outjson_t> topiclists;

        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

        for (unsigned int i = 0; i < topicm.record.size(); i++)
        {
            topicone.id       = topicm.record[i].topicid;
            topicone.parentid = topicm.record[i].parentid;
            topicone.value    = topicm.record[i].title;
            topicone.cateid   = topicm.record[i].cateid;
            topicone.sortid   = topicm.record[i].sortid;
            topicone.urlpath  = topicm.record[i].urlpath;
            topicone.isview   = topicm.record[i].isview > 0 ? true : false;

            topiclists.push_back(topicone);
        }
        std::vector<psy::topics_tree_outjson_t> topiclists_data, list_data;
        array_to_tree<psy::topics_tree_outjson_t>(topiclists_data, topiclists);
        tree_to_array<psy::topics_tree_outjson_t>(list_data, topiclists_data);

        client.val["list"].set_array();
        OBJ_ARRAY temp;
        for (unsigned int i = 0; i < list_data.size(); i++)
        {
            temp["id"]       = list_data[i].id;
            temp["parentid"] = list_data[i].parentid;
            temp["value"]    = list_data[i].value;
            temp["cateid"]   = list_data[i].cateid;
            temp["urlpath"]  = list_data[i].urlpath;
            temp["isview"]   = list_data[i].isview;
            temp["level"]    = list_data[i]._level;
            temp["sortid"]   = list_data[i].sortid;
            client.val["list"].push(temp);
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }

    peer->view("admin/martopic");
    return "";
}

//@urlpath(admin_isloginjson,admin/updatetopicsort)
std::string admin_updatetopicsort(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();

    try
    {
        auto topicm = orm::cms::Topic();
        topicm.setSortid(client.get["sortid"].to_int());
        topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicid", client.get["id"].to_int());
        topicm.update("sortid");
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
//@urlpath(admin_isloginjson,admin/addtopicpost)
std::string admin_addtopicpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    try
    {
        auto topicm = orm::cms::Topic();
        topicm.setTitle(client.post["topicname"].as_string());

        std::string tempstatus = client.post["topicstatus"].to_string();

        if (tempstatus[0] == 'O' || tempstatus[0] == 'o' || tempstatus[0] == '1')
        {
            topicm.setIsview(true);
        }
        else
        {
            topicm.setIsview(false);
        }

        topicm.setMemo(client.post["memo"].as_string());
        topicm.setCateid(client.post["topictype"].to_int());
        topicm.setUrlpath(client.post["urlpath"].as_string());
        topicm.setParentid(client.post["parentid"].to_int());

        topicm.setUserid(0);

        topicm.save();
        unsigned int topicid = 0;
        topicid              = topicm.getPK();
        if (topicm.error_msg.size() > 0)
        {
            std::cout << topicm.error_msg << std::endl;
        }
        if (topicid > 0)
        {
            client.val["code"]    = 0;
            client.val["topicid"] = topicid;
        }
        else
        {
            client.val["code"]    = 2;
            client.val["topicid"] = 0;
        }

        topicm.clear(true);
        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();
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
        client.val["code"]    = 1;
        client.val["topicid"] = 0;
    }
    client.out_json();
    return "";
}
//@urlpath(admin_isloginjson,admin/deletetopic)
std::string admin_deletetopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    try
    {
        auto topicm          = orm::cms::Topic();
        unsigned int topicid = client.get["id"].to_int();
        std::vector<unsigned int> del_id_array;

        if (topicid > 0)
        {
            topicm.select("topicid,parentid").where("userid", client.session["userid"].to_int()).asc("parentid").fetch();
            del_id_array.push_back(topicid);

            if (topicm.record.size() > 0)
            {
                for (unsigned int i = 0; i < topicm.record.size(); i++)
                {
                    for (unsigned int j = 0; j < del_id_array.size(); j++)
                    {
                        if (topicm.record[i].parentid == del_id_array[j])
                        {
                            del_id_array.push_back(topicm.record[i].topicid);
                        }
                    }
                }
            }

            std::string dpid_sql_str = array_to_sql(del_id_array);
            if (dpid_sql_str.size() > 0)
            {
                topicm.clear(true);
                topicm.whereIn("topicid", dpid_sql_str).remove();
                client.val["code"] = 0;
            }
            else
            {
                client.val["code"] = 2;
            }
        }
        else
        {
            client.val["code"] = 3;
        }
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    client.out_json();
    return "";
}
//@urlpath(admin_isloginjson,admin/edittopicpost)
std::string admin_edittopicpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    try
    {
        auto topicm = orm::cms::Topic();
        topicm.setTitle(client.post["topicname"].as_string());

        std::string tempstatus = client.post["topicstatus"].to_string();

        if (tempstatus[0] == 'O' || tempstatus[0] == 'o' || tempstatus[0] == '1')
        {
            topicm.setIsview(true);
        }
        else
        {
            topicm.setIsview(false);
        }
        unsigned int topicid_parentid = client.post["parentid"].to_int();
        unsigned int topicid          = client.post["topicid"].to_int();

        topicm.setMemo(client.post["memo"].as_string());
        topicm.setUrlpath(client.post["urlpath"].as_string());
        topicm.setCateid(client.post["topictype"].to_int());
        if (topicid_parentid != topicid)
        {
            // block set parentid as self
            topicm.setParentid(topicid_parentid);
        }

        topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicid", client.post["topicid"].to_int());
        if (topicid_parentid != topicid)
        {
            topicid_parentid = topicm.update("cateid,title,isview,memo,urlpath,parentid");
        }
        else
        {
            topicid_parentid = topicm.update("cateid,title,isview,memo,urlpath");
        }

        if (topicm.error_msg.size() > 0)
        {
            client.val["msg"] = topicm.error_msg;
        }
        if (topicid_parentid > 0)
        {
            client.val["code"]    = 0;
            client.val["topicid"] = topicid;
        }
        else
        {
            client.val["code"]    = 2;
            client.val["topicid"] = 0;
        }

        // topicm.clear(true);
        // topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();
        // client.val["list"].set_array();
        // OBJ_ARRAY temp;
        // for (unsigned int i = 0; i < topicm.record.size(); i++)
        // {

        //     temp["id"]       = topicm.record[i].topicid;
        //     temp["parentid"] = topicm.record[i].parentid;
        //     temp["value"]    = topicm.record[i].title;
        //     client.val["list"].push(temp);
        // }
    }
    catch (std::exception &e)
    {
        client.val["code"]    = 1;
        client.val["topicid"] = 0;
    }
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/topicfileupload)
std::string admin_topicfileupload(std::shared_ptr<httppeer> peer)
{
    httppeer &client      = peer->getpeer();
    std::string action_do = client.get["action"].to_string();

    auto img = std::make_unique<upload_images>(peer);
    img->init();

    if (action_do == "uploadimage")
    {
        bool issuccess = img->upload_img("upfile");
        if (issuccess)
        {
            client << img->upload_info.to_json();
        }
        else
        {
            client << "{\"state\":\"\",\"msg\":\"" << img->upload_info.state << "upload error!\"}";
        }

        client.out_json();
    }

    return "";
}

//@urlpath(admin_isloginjson,admin/topicimgtextupload)
std::string admin_topicimgtextupload(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();

    unsigned int imgnum = client.post["header_urlpath"].size();
    OBJ_ARRAY temp;
    OBJ_ARRAY headerimgobj;
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
    imgnum = 0;
    imgnum = client.get["topicid"].to_int();
    tstr   = headerimgobj.to_json();
    try
    {
        auto topicm = orm::cms::Topic();
        topicm.setTopimg(tstr);
        topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicid", imgnum);
        topicm.update("topimg");
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
}// namespace http
