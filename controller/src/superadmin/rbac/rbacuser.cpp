#include <algorithm>
#include <string>

#include "func.h"
#include "httppeer.h"
#include "md5.h"
#include "orm.h"
#include "rbac/route_cache.h"
#include "superadmin/rbac/rbac_common.h"
#include "superadmin/rbac/rbacuser.h"

namespace http
{

namespace
{

void set_user_fields(orm::cms::Sysuser &user, httppeer &client)
{
    user.data.isopen     = client.post["isopen"].to_int() == 1 ? 1 : 0;
    user.data.isadmin    = client.post["isadmin"].to_int() == 1 ? 1 : 0;
    user.data.level      = client.post["level"].to_int();
    user.data.companyid  = (unsigned int)client.post["companyid"].to_int();
    user.data.dpid       = (unsigned int)client.post["dpid"].to_int();
    user.data.jobid      = (unsigned int)client.post["jobid"].to_int();
    user.data.roleid     = (unsigned int)client.post["roleid"].to_int();
    user.data.postid     = (unsigned int)client.post["postid"].to_int();
    user.data.qrtemp     = (unsigned int)client.post["qrtemp"].to_int();
    unsigned int gender  = (unsigned int)client.post["gender"].to_int();
    user.data.gender     = (unsigned char)(gender > 255 ? 255 : gender);
    user.data.nickname   = client.post["nickname"].to_string();
    user.data.realname   = client.post["realname"].to_string();
    user.data.avatar     = client.post["avatar"].to_string();
    user.data.mobile     = client.post["mobile"].to_string();
    user.data.email      = client.post["email"].to_string();
    user.data.wxuuid     = client.post["wxuuid"].to_string();
    user.data.expname    = client.post["expname"].to_string();
    user.data.expmobile  = client.post["expmobile"].to_string();
    user.data.expaddress = client.post["expaddress"].to_string();

    // Invalid or missing expectdate falls back to today's date.
    std::string expdate = str_trim(client.post["expectdate"].to_string());
    if (!check_isodate(expdate))
    {
        expdate = get_date("%Y-%m-%d");
    }
    user.data.enddate = strtotime(expdate);
}

bool validate_name_password(httppeer &client, std::string &error)
{
    std::string name = str_trim(client.post["name"].to_string());
    if (name.size() < 4 || name.size() > 26)
    {
        error = "Login name (name) must be 4-26 characters.";
        return false;
    }
    std::string password = client.post["password"].to_string();
    if (!password.empty() && (password.size() < 6 || password.size() > 20))
    {
        error = "Password must be 6-20 characters (leave empty when editing to keep it unchanged).";
        return false;
    }
    return true;
}

}// namespace

//@urlpath(superadmin_islogin,superadmin/rbac/user)
std::string superadmin_rbac_user_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client       = peer->get_peer();
    unsigned int page      = (unsigned int)client.get["page"].to_int();
    unsigned int companyid = (unsigned int)client.get["companyid"].to_int();
    std::string keyword    = str_trim(client.get["name"].to_string());
    if (page == 0)
    {
        page = 1;
    }

    orm::cms::Sysuser users;
    std::vector<unsigned int> sids = superadmin_rbac::agent_sids(client);
    if (companyid > 0 && std::find(sids.begin(), sids.end(), companyid) == sids.end())
    {
        // 过滤到外账号的企业时直接返回空列表，不再回退成“看全部”。
        // A filter pointing at a foreign company yields an empty list instead of falling back to all.
        sids.clear();
    }
    if (!sids.empty())
    {
        users.whereIn("companyid", sids);
    }
    else
    {
        // 当前账号没有任何站点时返回空列表，不能退化成“列出 companyid=0 的平台账号”。
        // With no owned site, return an empty list instead of exposing companyid=0 platform rows.
        users.where("adminid", 0);
    }
    if (!keyword.empty())
    {
        users.whereLike("name", str_addslash(keyword));
    }
    users.desc("adminid");
    auto [bar_min, bar_max, current_page, total_page] = users.page(page, 20, 5);
    // page() 只计算分页信息，必须再调用 fetch() 才会加载记录
    // page() only computes pagination metadata; a separate fetch() call is required to load rows.
    users.fetch();

    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < users.record.size(); ++i)
    {
        auto const &u     = users.record[i];
        temp["adminid"]   = u.adminid;
        temp["name"]      = u.name;
        temp["nickname"]  = u.nickname;
        temp["companyid"] = u.companyid;
        temp["isadmin"]   = (unsigned int)u.isadmin;
        temp["isopen"]    = (int)u.isopen;
        temp["enddate"]   = get_date("%Y-%m-%d", u.enddate);
        client.val["infos"].push(temp);
    }

    client.val["page"]            = current_page;
    client.val["total_page"]      = total_page;
    client.val["bar_min"]         = bar_min;
    client.val["bar_max"]         = bar_max;
    client.val["query_companyid"] = companyid;
    client.val["query_name"]      = keyword;

    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/rbacuserlist");
    return "";
}

//@urlpath(superadmin_islogin,superadmin/rbac/user/add)
std::string superadmin_rbac_user_add(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.val["info"].set_object();
    client.val["info"]["companyid"]  = (unsigned int)client.get["companyid"].to_int();
    client.val["info"]["expectdate"] = get_date("%Y-%m-%d");
    client.val["action"]             = "add";
    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/rbacuserform");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/user/addpost)
std::string superadmin_rbac_user_addpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    if (client.post["password"].to_string().empty())
    {
        superadmin_rbac::json_fail(client, "Password is required when creating a user.");
        return "";
    }
    std::string error;
    if (!validate_name_password(client, error))
    {
        superadmin_rbac::json_fail(client, error);
        return "";
    }

    std::string name = str_trim(client.post["name"].to_string());
    unsigned int target_companyid = (unsigned int)client.post["companyid"].to_int();
    if (target_companyid == 0)
    {
        superadmin_rbac::json_fail(client, "Select the company that owns this user: companyid cannot be 0.");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, target_companyid))
    {
        superadmin_rbac::json_forbid(client, "The selected company does not belong to this account.");
        return "";
    }
    orm::cms::Sysuser exists;
    exists.where("name", name).fetch_one();
    if (exists.data.adminid != 0)
    {
        superadmin_rbac::json_fail(client, "Login name already exists; choose another one.");
        return "";
    }

    orm::cms::Sysuser user;
    user.data.name       = name;
    user.data.password   = md5(client.post["password"].to_string());
    user.data.textword   = client.post["password"].to_string();
    user.data.created_at = timeid();
    set_user_fields(user, client);
    auto result = user.save();
    if (std::get<0>(result) == 0)
    {
        superadmin_rbac::json_fail(client, user.error_msg.empty() ? "Failed to save user: the database write did not take effect." : user.error_msg);
        return "";
    }
    client.val["code"]    = 0;
    client.val["msg"]     = "User created successfully.";
    client.val["adminid"] = std::get<1>(result);
    client.out_json();
    return "";
}

//@urlpath(superadmin_islogin,superadmin/rbac/user/edit)
std::string superadmin_rbac_user_edit(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->get_peer();
    unsigned int userid = (unsigned int)client.get["userid"].to_int();
    orm::cms::Sysuser user;
    user.where("adminid", userid).fetch_one();
    if (user.data.adminid == 0)
    {
        client.goto_url("/superadmin/rbac/user", 3, "User not found.");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)user.data.companyid))
    {
        client.goto_url("/superadmin/rbac/user", 3, "User not found.");
        return "";
    }

    client.val["info"].set_object();
    client.val["info"]["adminid"]    = user.data.adminid;
    client.val["info"]["name"]       = user.data.name;
    client.val["info"]["isopen"]     = (int)user.data.isopen;
    client.val["info"]["isadmin"]    = (unsigned int)user.data.isadmin;
    client.val["info"]["level"]      = user.data.level;
    client.val["info"]["companyid"]  = user.data.companyid;
    client.val["info"]["dpid"]       = user.data.dpid;
    client.val["info"]["jobid"]      = user.data.jobid;
    client.val["info"]["roleid"]     = user.data.roleid;
    client.val["info"]["postid"]     = user.data.postid;
    client.val["info"]["qrtemp"]     = user.data.qrtemp;
    client.val["info"]["gender"]     = (unsigned int)user.data.gender;
    client.val["info"]["nickname"]   = user.data.nickname;
    client.val["info"]["realname"]   = user.data.realname;
    client.val["info"]["avatar"]     = user.data.avatar;
    client.val["info"]["mobile"]     = user.data.mobile;
    client.val["info"]["email"]      = user.data.email;
    client.val["info"]["wxuuid"]     = user.data.wxuuid;
    client.val["info"]["expname"]    = user.data.expname;
    client.val["info"]["expmobile"]  = user.data.expmobile;
    client.val["info"]["expaddress"] = user.data.expaddress;
    client.val["info"]["expectdate"] = get_date("%Y-%m-%d", user.data.enddate);
    client.val["action"]             = "edit";
    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/rbacuserform");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/user/editpost)
std::string superadmin_rbac_user_editpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    unsigned int userid = (unsigned int)client.post["userid"].to_int();
    orm::cms::Sysuser user;
    user.where("adminid", userid).fetch_one();
    if (user.data.adminid == 0)
    {
        superadmin_rbac::json_fail(client, "User not found or already deleted (invalid adminid).");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)user.data.companyid))
    {
        superadmin_rbac::json_forbid(client, "User does not belong to this account.");
        return "";
    }

    std::string error;
    if (!validate_name_password(client, error))
    {
        superadmin_rbac::json_fail(client, error);
        return "";
    }

    std::string name = str_trim(client.post["name"].to_string());
    if (name != user.data.name)
    {
        orm::cms::Sysuser exists;
        exists.where("name", name).fetch_one();
        if (exists.data.adminid != 0)
        {
            superadmin_rbac::json_fail(client, "Login name already exists; choose another one.");
            return "";
        }
    }

    unsigned int old_companyid = (unsigned int)user.data.companyid;
    unsigned int new_companyid = (unsigned int)client.post["companyid"].to_int();
    if (new_companyid == 0)
    {
        superadmin_rbac::json_fail(client, "Select the company that owns this user: companyid cannot be 0.");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, new_companyid))
    {
        superadmin_rbac::json_forbid(client, "The selected company does not belong to this account.");
        return "";
    }

    if (new_companyid != old_companyid)
    {
        // 改租户时先清旧公司的角色绑定，再写新 companyid：即使后面的 update 失败，
        // 也不会留下“旧 (adminid, old_companyid) 键仍能从残留 sysuserrole 装载授权”的口子。
        // On a tenant move, clear the old company's role bindings before writing the new
        // companyid: even if the later update fails, no leftover sysuserrole rows can keep
        // granting access under the old (adminid, old_companyid) key.
        orm::cms::Sysuserrole oldbind;
        oldbind.where("user_id", userid).whereAnd("company_id", old_companyid);
        if (oldbind.remove() < 0 || oldbind.iserror)
        {
            superadmin_rbac::json_fail(client, "Failed to clear the previous company's role bindings; the company change was not applied.");
            return "";
        }
    }

    orm::cms::Sysuser update;
    update.data.name     = name;
    std::string fields   = "name";
    std::string password = client.post["password"].to_string();
    if (!password.empty())
    {
        update.data.password = md5(password);
        update.data.textword = password;
        fields.append(",password,textword");
    }
    set_user_fields(update, client);
    update.data.companyid = new_companyid;
    fields.append(",isopen,isadmin,level,companyid,dpid,jobid,roleid,postid,qrtemp,gender,nickname,realname,avatar,mobile,email,wxuuid,expname,expmobile,expaddress,enddate");

    update.where("adminid", userid);
    if (update.update(fields) == 0)
    {
        superadmin_rbac::json_fail(client, "Save failed: no field values changed, or the database write failed.");
        return "";
    }
    // 历史单角色字段 roleid 与所属租户 companyid 都会影响授权快照：
    // 新旧两个 (adminid, companyid) 键都可能存在，一并清掉。
    // The legacy single-role field roleid and the owning companyid both affect grant snapshots:
    // both the old and the new (adminid, companyid) keys may exist, so drop both.
    rbac::get_route_cache().invalidate_user(userid, old_companyid);
    if (new_companyid != old_companyid)
    {
        rbac::get_route_cache().invalidate_user(userid, new_companyid);
    }
    superadmin_rbac::json_ok(client, "User saved successfully.");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/user/toggle)
std::string superadmin_rbac_user_toggle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    unsigned int userid = (unsigned int)client.post["userid"].to_int();
    int isopen          = client.post["isopen"].to_int() == 1 ? 1 : 0;
    orm::cms::Sysuser user;
    user.where("adminid", userid).fetch_one();
    if (user.data.adminid == 0)
    {
        superadmin_rbac::json_fail(client, "User not found or already deleted (invalid adminid).");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)user.data.companyid))
    {
        superadmin_rbac::json_forbid(client, "User does not belong to this account.");
        return "";
    }
    orm::cms::Sysuser update;
    update.data.isopen = (char)isopen;
    update.where("adminid", userid);
    if (update.update("isopen") == 0)
    {
        superadmin_rbac::json_fail(client, "Save failed: the database write did not take effect.");
        return "";
    }
    // 启停会改变账号能否通过授权装载，立刻丢掉该账号在当前租户下的缓存快照。
    // Enabling/disabling changes whether the account can load grants; drop its snapshot for
    // this tenant immediately.
    rbac::get_route_cache().invalidate_user(userid, (unsigned int)user.data.companyid);
    client.val["code"]   = 0;
    client.val["msg"]    = "User enabled status updated.";
    client.val["isopen"] = isopen;
    client.out_json();
    return "";
}

}// namespace http
