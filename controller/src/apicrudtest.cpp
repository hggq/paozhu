#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "json_reflect_headers.h"
#include "array_to_tree.h"
#include "apicrudtest.h"

namespace http
{
//@urlpath(null,api/departments/addpost)
std::string pxapidepartmentsaddpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client.cors_domain("*");
    if (client.method == 3)
    {
        client.cors_method("origin, x-requested-with");
        return "";
    }
    try
    {
        auto deps = orm::cms::Department();

        deps.setParentid(client.json["parentid"].to_int());
        deps.setName(client.json["title"].to_string());

        deps.setBianzhi_num(client.json["bianzhi_num"].to_int());
        deps.setMemo(client.json["desc"].to_string());
        deps.setIsopen(client.json["isopen"].to_int());

        unsigned int lastdepid = deps.save();
        lastdepid              = deps.getPK();
        if (deps.error_msg.size() > 0)
        {
            std::cout << deps.error_msg << std::endl;
        }

        client.val["code"] = 0;
        client.val["data"].set_array();
        client.val["data"]["value"]       = lastdepid;
        client.val["data"]["title"]       = deps.getName();
        client.val["data"]["key"]         = lastdepid;
        client.val["data"]["parentid"]    = deps.getParentid();
        client.val["data"]["desc"]        = deps.getMemo();
        client.val["data"]["bianzhi_num"] = deps.getBianzhi_num();
        client.val["data"]["isopen"]      = deps.getIsopen() == 0 ? false : true;
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    client.out_json();
    return "";
}

//@urlpath(null,api/departments/editpost)
std::string pxapidepartmentseditpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client.cors_domain("*");
    if (client.method == 3)
    {
        client.cors_method("origin, x-requested-with");
        return "";
    }
    try
    {
        auto deps = orm::cms::Department();

        deps.setParentid(client.json["parentid"].to_int());
        deps.setName(client.json["title"].to_string());

        deps.setBianzhi_num(client.json["bianzhi_num"].to_int());
        deps.setMemo(client.json["desc"].to_string());
        deps.setIsopen(client.json["isopen"].to_int());

        unsigned int lastdepid = client.json["value"].to_int();
        if (lastdepid > 0)
        {
            deps.setPK(lastdepid);
            deps.update("parentid,name,bianzhi_num,memo,isopen");
        }
        else
        {
            lastdepid = deps.save();
        }

        lastdepid = deps.getPK();
        if (deps.error_msg.size() > 0)
        {
            std::cout << deps.error_msg << std::endl;
        }

        client.val["code"] = 0;
        client.val["data"].set_array();
        client.val["data"]["value"]       = lastdepid;
        client.val["data"]["title"]       = deps.getName();
        client.val["data"]["key"]         = lastdepid;
        client.val["data"]["parentid"]    = deps.getParentid();
        client.val["data"]["desc"]        = deps.getMemo();
        client.val["data"]["bianzhi_num"] = deps.getBianzhi_num();
        client.val["data"]["isopen"]      = deps.getIsopen() == 0 ? false : true;
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    client.out_json();
    return "";
}

//@urlpath(null,api/departments/list)
std::string pxapidepartmentslist(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client.cors_domain("*");
    if (client.method == 3)
    {
        client.cors_method("origin, x-requested-with");
        return "";
    }
    try
    {
        auto deps = orm::cms::Department();
        deps.where("userid", 0).order("parentid asc").fetch();

        psy::department_outjson_t deps_json_one;
        std::vector<psy::department_outjson_t> depsjsonlist;

        for (unsigned int i = 0; i < deps.record.size(); i++)
        {
            deps_json_one.id          = deps.record[i].dpid;
            deps_json_one.key         = deps.record[i].dpid;
            deps_json_one.value       = deps.record[i].dpid;
            deps_json_one.parentid    = deps.record[i].parentid;
            deps_json_one.isopen      = deps.record[i].isopen == 1 ? true : false;
            deps_json_one.title       = deps.record[i].name;
            deps_json_one.desc        = deps.record[i].memo;
            deps_json_one.bianzhi_num = deps.record[i].bianzhi_num;

            depsjsonlist.push_back(deps_json_one);
        }
        psy::department_listoutjson_t depout_data;

        array_to_tree<psy::department_outjson_t>(depout_data.data.list, depsjsonlist);
        depout_data.code       = 0;
        depout_data.data.total = 0;

        client << psy::json_encode(depout_data);
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }

    client.out_json();
    return "";
}

//@urlpath(null,api/departments/deletedep)
std::string pxapipagesdepartlist(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client.cors_domain("*");
    if (client.method == 3)
    {
        client.cors_method("origin, x-requested-with");
        return "";
    }

    try
    {
        auto deps         = orm::cms::Department();
        unsigned int dpid = client.get["deleteid"].to_int();
        std::vector<unsigned int> del_id_array;

        if (dpid > 0)
        {
            deps.select("dpid,parentid,name").where("userid", 0).asc("parentid").fetch();
            del_id_array.push_back(dpid);
            if (deps.record.size() > 0)
            {
                for (unsigned int i = 0; i < deps.record.size(); i++)
                {
                    for (unsigned int j = 0; j < del_id_array.size(); j++)
                    {
                        if (deps.record[i].parentid == del_id_array[j])
                        {
                            del_id_array.push_back(deps.record[i].dpid);
                            break;
                        }
                    }
                }
            }

            std::string dpid_sql_str = array_to_sql(del_id_array);
            if (dpid_sql_str.size() > 0)
            {
                deps.clear(true);
                deps.whereIn("dpid", dpid_sql_str).remove();
            }
        }

        deps.clear(true);
        deps.where("userid", 0).order("parentid asc").fetch();

        psy::department_outjson_t deps_json_one;
        std::vector<psy::department_outjson_t> depsjsonlist;

        for (unsigned int i = 0; i < deps.record.size(); i++)
        {
            deps_json_one.id          = deps.record[i].dpid;
            deps_json_one.key         = deps.record[i].dpid;
            deps_json_one.value       = deps.record[i].dpid;
            deps_json_one.parentid    = deps.record[i].parentid;
            deps_json_one.isopen      = deps.record[i].isopen == 1 ? true : false;
            deps_json_one.title       = deps.record[i].name;
            deps_json_one.desc        = deps.record[i].memo;
            deps_json_one.bianzhi_num = deps.record[i].bianzhi_num;

            depsjsonlist.push_back(deps_json_one);
        }

        psy::department_listoutjson_t depout_data;
        array_to_tree<psy::department_outjson_t>(depout_data.data.list, depsjsonlist);

        depout_data.code       = 0;
        depout_data.data.total = 0;
        client << psy::json_encode(depout_data);
    }
    catch (std::exception &e)
    {
        client.val["code"] = 1;
    }
    client.out_json();
    return "";
}
}// namespace http