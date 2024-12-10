#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "category.h"

namespace http
{
//@urlpath(admin_islogin,admin/marcatalogue)
std::string admin_marcatalogue(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto cata = orm::cms::Catalogue();

    cata.where("userid", client.session["userid"].to_int());

    cata.fetch();

    auto treedata = cata.to_tree();
    cata.record.clear();
    cata.tree_torecord(treedata);

    client.val["list"].set_array();
    OBJ_ARRAY temp;

    for (unsigned int i = 0; i < cata.record.size(); i++)
    {
        temp["id"]       = cata.record[i].cid;
        temp["parentid"] = cata.record[i].parentid;
        temp["value"]    = cata.record[i].name;
        temp["level"]    = cata.record[i].level;

        client.val["list"].push(temp);
    }

    peer->view("admin/marcategory");
    return "";
}
//@urlpath(admin_islogin,admin/addcataloguepost)
std::string admin_addcataloguepost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto cata = orm::cms::Catalogue();
    cata.setName(client.post["topicname"].as_string());
    cata.setParentid(client.post["parentid"].to_int());
    cata.setUserid(client.session["userid"].to_int());
    cata.save();

    cata.where("userid", client.session["userid"].to_int());
    cata.fetch();

    auto treedata = cata.to_tree();
    cata.record.clear();
    cata.tree_torecord(treedata);

    client.val["list"].set_array();
    OBJ_ARRAY temp;
    std::vector<unsigned int> levelnum;
    for (unsigned int i = 0; i < cata.record.size(); i++)
    {
        temp["id"]       = cata.record[i].cid;
        temp["parentid"] = cata.record[i].parentid;
        temp["value"]    = cata.record[i].name;
        temp["level"]    = cata.record[i].level;

        client.val["list"].push(temp);
    }

    client.val["code"] = 0;
    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/editcataloguepost)
std::string admin_editcataloguepost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto cata = orm::cms::Catalogue();
    cata.setName(client.post["editname"].as_string());
    cata.setParentid(client.post["editparentid"].to_int());
    cata.where("userid", client.session["userid"].to_int()).where("cid", client.post["cid"].to_int());
    cata.update("parentid,name");
    client.val["code"] = 0;
    client.out_json();
    return "";
}

//@urlpath(admin_islogin,admin/deletecatalogue)
std::string admin_deletecatalogue(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    auto cata = orm::cms::Catalogue();

    unsigned int cid = client.get["id"].to_int();
    std::vector<unsigned int> del_id_array;

    if (cid > 0)
    {
        cata.select("cid,parentid").where("userid", client.session["userid"].to_int()).asc("parentid").fetch();
        del_id_array.push_back(cid);

        if (cata.record.size() > 0)
        {
            for (unsigned int i = 0; i < cata.record.size(); i++)
            {
                for (unsigned int j = 0; j < del_id_array.size(); j++)
                {
                    if (cata.record[i].parentid == del_id_array[j])
                    {
                        del_id_array.push_back(cata.record[i].cid);
                        break;
                    }
                }
            }
        }

        std::string dpid_sql_str = array_to_sql(del_id_array);
        if (dpid_sql_str.size() > 0)
        {
            cata.clear(true);
            cata.whereIn("cid", dpid_sql_str).remove();
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

    client.out_json();
    return "";
}

}// namespace http
