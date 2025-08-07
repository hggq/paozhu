#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "topics.h"
#include "json_reflect_headers.h"
#include "array_to_tree.h"
#include "upload_images.h"
namespace http
{
//@urlpath(admin_islogin,admin/addtopic)
std::string admin_addtopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto topicm = orm::cms::Topic();
        // define out
        // psy::topics_tree_outjson_t a;

        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();

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
    }

    peer->view("admin/addtopic");
    return "";
}

//@urlpath(admin_islogin,admin/edittopic)
std::string admin_edittopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto topicm = orm::cms::Topic();
        // define out
        // psy::topics_tree_outjson_t a;

        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();
        client.val["list"].set_array();
        obj_val temp;
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
            topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicid", topicid).fetch_one();
            client.val["info"].set_object();
            client.val["info"]["topicid"]     = topicm.data.topicid;
            client.val["info"]["parentid"]    = topicm.data.parentid;
            client.val["info"]["topicname"]   = topicm.data.title;
            client.val["info"]["topictype"]   = topicm.data.cateid;
            client.val["info"]["topicstatus"] = topicm.data.isview == 1 ? true : false;
            client.val["info"]["isside"]      = topicm.data.isside == 1 ? true : false;
            client.val["info"]["urlpath"]     = topicm.data.urlpath;
            client.val["info"]["memo"]        = topicm.data.memo;
            client.val["info"]["sorttype"]    = topicm.data.sorttype;
            client.val["info"]["imgurl"]      = topicm.data.imgurl;

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
    httppeer &client = peer->get_peer();

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
        obj_val temp;
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
    httppeer &client = peer->get_peer();

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
    httppeer &client = peer->get_peer();
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

        unsigned char issidestatus = client.post["isside"].to_int();

        if (issidestatus == 1)
        {
            topicm.setIsside(1);
        }
        else
        {
            topicm.setIsside(0);
        }

        topicm.setMemo(client.post["memo"].as_string());
        topicm.setCateid(client.post["topictype"].to_int());
        topicm.setUrlpath(client.post["urlpath"].as_string());
        topicm.setParentid(client.post["parentid"].to_int());
        topicm.setSorttype(client.post["sorttype"].to_int());
        topicm.setUserid(client.session["userid"].to_int());
        topicm.setImgurl(client.post["icoimg"].as_string());

        topicm.save();
        unsigned int topicid = 0;
        topicid              = topicm.getPK();

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
        if (topicm.error_msg.size() > 0)
        {
            client.val["msg"] = topicm.error_msg;
        }

        topicm.clear(true);
        topicm.where("userid", client.session["userid"].to_int()).asc("parentid").fetch();
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
        client.val["code"]    = 1;
        client.val["topicid"] = 0;
    }
    client.out_json();
    return "";
}
//@urlpath(admin_isloginjson,admin/deletetopic)
std::string admin_deletetopic(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
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
                            break;
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
    httppeer &client = peer->get_peer();
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

        unsigned char issidestatus = client.post["isside"].to_int();

        if (issidestatus == 1)
        {
            topicm.setIsside(1);
        }
        else
        {
            topicm.setIsside(0);
        }

        unsigned int topicid_parentid = client.post["parentid"].to_int();
        unsigned int topicid          = client.post["topicid"].to_int();

        topicm.setMemo(client.post["memo"].as_string());
        topicm.setUrlpath(client.post["urlpath"].as_string());
        topicm.setCateid(client.post["topictype"].to_int());
        topicm.setSorttype(client.post["sorttype"].to_int());
        topicm.setImgurl(client.post["icoimg"].as_string());
        if (topicid_parentid != topicid)
        {
            // block set parentid as self
            topicm.setParentid(topicid_parentid);
        }

        topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicid", client.post["topicid"].to_int());
        if (topicid_parentid != topicid)
        {
            topicid_parentid = topicm.update("cateid,sorttype,title,isview,isside,memo,urlpath,imgurl,parentid");
        }
        else
        {
            topicid_parentid = topicm.update("cateid,sorttype,title,isview,isside,memo,urlpath,imgurl");
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
        // obj_val temp;
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
    httppeer &client      = peer->get_peer();
    std::string action_do = client.get["action"].to_string();

    auto img = std::make_unique<upload_images>(peer);
    // img->init();
    // img->set_host(peer->get_hosturl());
    std::string img_path = client.session["sitepath"].to_string();
    std::string hosturl  = client.session["hosturl"].to_string();
    img->set_host(hosturl);
    img->set_sitepath(img_path);

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
    httppeer &client = peer->get_peer();

    unsigned int imgnum = client.post["header_urlpath"].size();
    obj_val temp;
    obj_val headerimgobj;
    std::string tstr;

    headerimgobj.set_array();
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

//@urlpath(admin_isloginjson,admin/updatetopicview)
std::string admin_updatetopicview(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto topicm          = orm::cms::Topic();
        unsigned char isview = client.post["isview"].to_int() > 0 ? 1 : 0;

        topicm.setIsview(isview);
        topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicid", client.get["id"].to_int());
        client.val["code"] = topicm.update("isview");
        client.val["msg"]  = "ok";
    }
    catch (std::exception &e)
    {
        client.val["code"] = 0;
    }
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/updatetopicsideblocksort)
std::string admin_updatetopicsideblocksort(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    try
    {
        auto topicm          = orm::cms::Topicset();
        unsigned char sortid = client.get["sortid"].to_int();

        topicm.setSort(sortid);
        topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicsetid", client.get["id"].to_int());
        client.val["code"] = topicm.update("sort");
        client.val["msg"]  = "ok";
    }
    catch (std::exception &e)
    {
        client.val["code"] = 0;
    }
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/edittopicside)
std::string admin_edittopicside(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.val["list"].set_array();
    unsigned int topicid  = client.get["topicid"].to_int();
    client.val["topicid"] = topicid;

    try
    {
        auto topicsetm = orm::cms::Topicset();
        topicsetm.where("userid", client.session["userid"].to_int()).whereAnd("topicid", topicid).order("sort").fetch();

        obj_val temp;
        for (unsigned int i = 0; i < topicsetm.record.size(); i++)
        {
            temp["id"]       = topicsetm.record[i].topicsetid;
            temp["title"]    = topicsetm.record[i].sidename;
            temp["viewtype"] = topicsetm.record[i].sidetype;
            temp["sortid"] = topicsetm.record[i].sort;
            client.val["list"].push(temp);
        }
    }
    catch (std::exception &e)
    {
    }
    peer->view("admin/martopicside");
    return "";
}

//@urlpath(admin_isloginjson,admin/addtopicsidepick)
std::string admin_addtopicsidepick(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int topicid  = client.get["topicid"].to_int();
    client.val["topicid"] = topicid;

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
        obj_val temp;
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

    peer->view("admin/addtopicsidepick");
    return "";
}

//@urlpath(admin_isloginjson,admin/addtopicsidetext)
std::string admin_addtopicsidetext(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int topicid  = client.get["topicid"].to_int();
    client.val["topicid"] = topicid;

    peer->view("admin/addtopicsidetext");
    return "";
}

//@urlpath(admin_isloginjson,admin/addtopicsideblockpost)
std::string admin_addtopicsideblockpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client     = peer->get_peer();
    unsigned int topicid = client.post["topicid"].to_int();
    try
    {
        auto topicm = orm::cms::Topicset();
        topicm.setSidename(client.post["title"].as_string());
        topicm.setTxtcontent(client.post["content"].to_string());

        unsigned int typetemp = client.post["viewtype"].to_int();
        if (typetemp > 2)
        {
            typetemp = 0;
        }
        topicm.setSidetype(typetemp);

        typetemp = client.post["picktopicid"].to_int();
        topicm.setLinktopicid(typetemp);
        topicm.setTopicid(topicid);

        typetemp = client.post["rownum"].to_int();
        topicm.setLinkrownum(typetemp);

        topicm.setUserid(client.session["userid"].to_int());

        auto [effect_num, insert_last_id] = topicm.save();

        if (insert_last_id > 0)
        {
            client.val["code"]    = 0;
            client.val["topicid"] = topicid;
        }
        else
        {
            client.val["code"]    = 2;
            client.val["topicid"] = 0;
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

//@urlpath(admin_isloginjson,admin/deletetopicsideblock)
std::string admin_deletetopicsideblock(std::shared_ptr<httppeer> peer)
{
    httppeer &client        = peer->get_peer();
    unsigned int topicsetid = client.get["id"].to_int();
    try
    {
        auto topicm         = orm::cms::Topicset();
        unsigned int tempid = topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicsetid", topicsetid).remove();

        if (tempid > 0)
        {
            client.val["code"]    = 0;
            client.val["topicid"] = topicsetid;
        }
        else
        {
            client.val["code"]    = 2;
            client.val["topicid"] = 0;
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

//@urlpath(admin_isloginjson,admin/edittopicsideblocktext)
std::string admin_edittopicsideblocktext(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int topicsetid  = client.get["id"].to_int();
    client.val["topicsetid"] = topicsetid;

    try
    {
        auto topicm = orm::cms::Topicset();
        topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicsetid", topicsetid).fetch_one();

        client.val["info"].set_object();

        client.val["info"]["title"]      = topicm.data.sidename;
        client.val["info"]["content"]    = topicm.data.txtcontent;
        client.val["info"]["topicsetid"] = topicm.data.topicsetid;
        client.val["info"]["topicid"]    = topicm.data.topicid;
        client.val["topicid"]            = topicm.data.topicid;
    }
    catch (std::exception &e)
    {
    }
    peer->view("admin/edittopicsidetext");
    return "";
}

//@urlpath(admin_isloginjson,admin/edittopicsideblockpick)
std::string admin_edittopicsideblockpick(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int topicsetid  = client.get["id"].to_int();
    client.val["topicsetid"] = topicsetid;

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
        obj_val temp;
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

        auto topicsetm = orm::cms::Topicset();
        topicsetm.where("userid", client.session["userid"].to_int()).whereAnd("topicsetid", topicsetid).fetch_one();

        client.val["info"].set_object();

        client.val["info"]["title"]      = topicsetm.data.sidename;
        client.val["info"]["content"]    = topicsetm.data.txtcontent;
        client.val["info"]["topicsetid"] = topicsetm.data.topicsetid;
        client.val["info"]["topicid"]    = topicsetm.data.topicid;
        client.val["info"]["rownum"]    = topicsetm.data.linkrownum;
        client.val["info"]["picktopicid"]    = topicsetm.data.linktopicid;
        client.val["topicid"]            = topicsetm.data.topicid;
    }
    catch (std::exception &e)
    {
    }
    peer->view("admin/edittopicsidepick");
    return "";
}

//@urlpath(admin_isloginjson,admin/edittopicsideblockpost)
std::string admin_edittopicsideblockpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client        = peer->get_peer();
    unsigned int topicsetid = client.post["topicsetid"].to_int();
    try
    {
        auto topicm = orm::cms::Topicset();
        topicm.setSidename(client.post["title"].as_string());
        topicm.setTxtcontent(client.post["content"].as_string());

        unsigned int typetemp = client.post["viewtype"].to_int();
        if (typetemp > 2)
        {
            typetemp = 0;
        }
        // topicm.setSidetype(typetemp);

        typetemp = client.post["picktopicid"].to_int();
        topicm.setLinktopicid(typetemp);

        typetemp = client.post["rownum"].to_int();
        topicm.setLinkrownum(typetemp);


        topicm.where("userid", client.session["userid"].to_int()).whereAnd("topicsetid", topicsetid);

        unsigned int tempeffect = topicm.update("sidename,txtcontent,linkrownum,linktopicid");
        if (tempeffect > 0)
        {
            client.val["code"]    = 0;
            client.val["topicid"] = client.post["topicid"].to_int();
        }
        else
        {
            client.val["code"]    = 2;
            client.val["topicid"] = 0;
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

}// namespace http
