#include "orm.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "datetime.h"
#include "func.h"
#include "httppeer.h"
#include "orm_query.h"
#include "rbac/rbacapi.h"
#include "rbac/rbac.h"
#include "rbac/role.h"
#include "rbac/route_cache.h"

namespace http
{
namespace
{
bool role_owned(unsigned int roleid, unsigned int companyid)
{
    orm::cms::Sysrole role;
    if (role.where("roleid", roleid).whereAnd("userid", companyid).fetch_one() == 0)
    {
        return false;
    }
    return true;
}

unsigned int uint_param(const obj_val &value, unsigned int default_value)
{
    if (value.to_string().empty())
    {
        return default_value;
    }
    return (unsigned int)value.to_int();
}
}// namespace

//@urlpath(admin_isloginjson,admin/rbac/role/list)
std::string rbac_role_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    orm::cms::Sysrole role;
    role.where("userid", user_companyid).asc("sortid").fetch();

    client.val["code"] = 0;
    client.val["msg"]  = "ok";
    client.val["data"].set_array();
    obj_val item;
    for (unsigned int i = 0; i < role.record.size(); i++)
    {
        item.clear();
        item["roleid"]    = role.record[i].roleid;
        item["name"]      = role.record[i].name;
        item["status"]    = role.record[i].status;
        item["rolecode"]  = role.record[i].rolecode;
        item["sortid"]    = role.record[i].sortid;
        item["rolevalue"] = role.record[i].rolevalue;
        client.val["data"].push(item);
    }
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/role/add)
std::string rbac_role_add(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);
    unsigned int operid         = rbacapi::adminid(client);
    // Fail closed: an account without a tenant id cannot own a role.
    if (user_companyid == 0)
    {
        rbacapi::forbid(client, "current account has no company id; the platform must bind one first");
        return "";
    }
    if (!rbacapi::check_csrf(client))
    {
        rbacapi::csrf_fail(client);
        return "";
    }

    std::string name = str_trim(client.post["name"].to_string());
    if (name.empty() || name.size() > 60)
    {
        rbacapi::fail(client, "name is required and must be 1-60 characters");
        return "";
    }

    orm::cms::Sysrole role;
    role.data.userid    = user_companyid;
    role.data.name      = name;
    role.data.status    = (unsigned char)uint_param(client.post["status"], 1);
    role.data.rolecode  = (unsigned long long)client.post["rolecode"].to_int();
    role.data.sortid    = (int)client.post["sortid"].to_int();
    role.data.rolevalue = uint_param(client.post["rolevalue"], 0);
    (void)operid;

    auto result = role.save();
    if (std::get<0>(result) == 0)
    {
        rbacapi::fail(client, role.error_msg.empty() ? "create role failed" : role.error_msg);
        return "";
    }

    client.val["code"]   = 0;
    client.val["msg"]    = "ok";
    client.val["roleid"] = std::get<1>(result);
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/role/edit)
std::string rbac_role_edit(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    if (!rbacapi::check_csrf(client))
    {
        rbacapi::csrf_fail(client);
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    unsigned int roleid = (unsigned int)client.post["roleid"].to_int();
    if (roleid == 0 || !role_owned(roleid, user_companyid))
    {
        rbacapi::fail(client, "role not found in this company");
        return "";
    }

    // Only non-empty POST fields are applied; omitted fields keep their stored values.
    orm::cms::Sysrole role;
    role.where("roleid", roleid).whereAnd("userid", user_companyid).fetch_one();
    if (!client.post["name"].to_string().empty())
    {
        std::string name = str_trim(client.post["name"].to_string());
        if (name.size() > 60)
        {
            rbacapi::fail(client, "name must be 1-60 characters");
            return "";
        }
        role.data.name = name;
    }
    if (!client.post["status"].to_string().empty())
    {
        role.data.status = (unsigned char)client.post["status"].to_int();
    }
    if (!client.post["rolecode"].to_string().empty())
    {
        role.data.rolecode = (unsigned long long)client.post["rolecode"].to_int();
    }
    if (!client.post["sortid"].to_string().empty())
    {
        role.data.sortid = (int)client.post["sortid"].to_int();
    }
    if (!client.post["rolevalue"].to_string().empty())
    {
        role.data.rolevalue = (unsigned int)client.post["rolevalue"].to_int();
    }

    role.update("name,status,rolecode,sortid,rolevalue");
    // update() 返回 0 混了语句失败、抛异常、值确实没变三种，所以只判旗标：
    // 把 0 行当失败会否掉今天能成功的原样提交。
    // update() answers 0 for a failure, a throw or unchanged values, so judge on the flags only.
    if (role.iserror || !role.error_msg.empty())
    {
        rbacapi::fail(client, "save role failed: " + (role.error_msg.empty() ? "unknown error" : role.error_msg));
        return "";
    }
    // 改名/停用都会改变生效集合，整表失效
    // Renaming or disabling changes what the snapshots mean, so drop them all.
    rbac::get_route_cache().invalidate_all();
    client.val["code"] = 0;
    client.val["msg"]  = "ok";
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/role/delete)
std::string rbac_role_delete(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    if (!rbacapi::check_csrf(client))
    {
        rbacapi::csrf_fail(client);
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    unsigned int roleid = (unsigned int)client.post["roleid"].to_int();
    if (roleid == 0 || !role_owned(roleid, user_companyid))
    {
        rbacapi::fail(client, "role not found in this company");
        return "";
    }

    // 三条 DELETE 一条事务：分开跑时后面失败、前面的删除已经生效，页面照样报 ok；
    // 残留在已删角色上的授权行还会被判定链按 sysuserrole 的 roleid 读到，继续放行。
    // The three deletes need one transaction: run separately, a later failure leaves earlier deletes
    // committed — and grants left behind a deleted role are still read by the decision chain.
    auto ulink = std::make_unique<orm::db_conn>("cms");
    if (!ulink->begin_commit())
    {
        rbacapi::fail(client, "failed to start a transaction; nothing was deleted: " + ulink->error_msg);
        return "";
    }

    bool write_failed = false;
    std::string dbmsg;
    {
        // 0 行被删是合法结果（角色可以本来就没有绑定或授权），而 remove() 的 catch 只写 error_msg
        // 不置 iserror，所以两个旗标都得看。
        // 0 rows deleted is legitimate, and remove()'s catch sets error_msg without iserror, so check both flags.
        orm::cms::Sysuserrole binding;
        binding.lock_conn(ulink->mysql_edit_conn);
        binding.where("role_id", roleid).whereAnd("company_id", user_companyid).remove();
        binding.unlock_conn();
        if (binding.iserror || !binding.error_msg.empty())
        {
            write_failed = true;
            dbmsg        = binding.error_msg;
        }
    }
    if (!write_failed)
    {
        orm::cms::Sysroleperms roleperms;
        roleperms.lock_conn(ulink->mysql_edit_conn);
        roleperms.where("roleid", roleid).whereAnd("userid", user_companyid).remove();
        roleperms.unlock_conn();
        if (roleperms.iserror || !roleperms.error_msg.empty())
        {
            write_failed = true;
            dbmsg        = roleperms.error_msg;
        }
    }
    if (!write_failed)
    {
        // 角色行上面 role_owned() 确认过存在，所以 0 行被删就是没删掉，与失败同等处理。
        // role_owned() confirmed this row exists, so 0 rows deleted means the delete did not take.
        orm::cms::Sysrole role;
        role.lock_conn(ulink->mysql_edit_conn);
        int role_count = role.where("roleid", roleid).whereAnd("userid", user_companyid).remove();
        role.unlock_conn();
        if (role_count == 0 || role.iserror || !role.error_msg.empty())
        {
            write_failed = true;
            dbmsg        = role.error_msg.empty() ? "the role row was not deleted (0 rows affected)" : role.error_msg;
        }
    }
    if (write_failed)
    {
        ulink->rollback();
        rbacapi::fail(client, "role delete failed, the whole cleanup was rolled back: " + (dbmsg.empty() ? "unknown error" : dbmsg));
        return "";
    }
    if (!ulink->commit())
    {
        std::string commitmsg = ulink->error_msg;
        ulink->rollback();
        rbacapi::fail(client, "commit failed, the whole cleanup was rolled back: " + (commitmsg.empty() ? "unknown error" : commitmsg));
        return "";
    }

    // 角色连同它的绑定与授权一起消失，整表失效
    // The role, its bindings and its grants all disappear, so drop every snapshot.
    rbac::get_route_cache().invalidate_all();

    client.val["code"] = 0;
    client.val["msg"]  = "ok";
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/roleperm/list)
std::string rbac_roleperm_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    unsigned int roleid = (unsigned int)client.get["roleid"].to_int();
    if (roleid == 0 || !role_owned(roleid, user_companyid))
    {
        rbacapi::fail(client, "role not found in this company");
        return "";
    }

    orm::cms::Sysroleperms roleperms;
    roleperms.where("roleid", roleid).whereAnd("userid", user_companyid).asc("sortid").fetch();

    client.val["code"] = 0;
    client.val["msg"]  = "ok";
    client.val["data"].set_array();
    obj_val item;
    for (unsigned int i = 0; i < roleperms.record.size(); i++)
    {
        item.clear();
        item["permsid"] = roleperms.record[i].permsid;
        item["sortid"]  = roleperms.record[i].sortid;
        client.val["data"].push(item);
    }
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/roleperm/bind)
std::string rbac_roleperm_bind(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    if (!rbacapi::check_csrf(client))
    {
        rbacapi::csrf_fail(client);
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);
    unsigned int operid         = rbacapi::adminid(client);

    unsigned int roleid = (unsigned int)client.post["roleid"].to_int();
    if (roleid == 0 || !role_owned(roleid, user_companyid))
    {
        rbacapi::fail(client, "role not found in this company");
        return "";
    }

    std::vector<unsigned int> permsids = rbacapi::uint_list(client.post["permsids"].to_string());

    // 天花板：只允许分配平台分配给本企业的权限项（syscompanyperms，且目录行启用中）；
    // replace_role_perms 内部同口径再校验一遍，绕开本页面直接调库函数也越不过去。
    // Ceiling: only rows the platform allocated to this company, still enabled in the catalog.
    // replace_role_perms enforces the same rule again underneath.
    if (!permsids.empty())
    {
        std::vector<unsigned int> alloc;
        if (!rbac::load_company_perms(user_companyid, alloc))
        {
            rbacapi::fail(client, "Assignable permission set unavailable.");
            return "";
        }
        std::vector<unsigned int> assignable_ids;
        if (!alloc.empty())
        {
            orm::cms::Sysperms assignable;
            assignable.select("permsid")
                .whereIn(orm::cms::sysperms_info::cols::permsid, alloc)
                .whereAnd("userid", user_companyid)
                .whereAnd("status", 1)
                .fetch();
            for (unsigned int i = 0; i < assignable.record.size(); i++)
            {
                assignable_ids.push_back(assignable.record[i].permsid);
            }
        }
        for (unsigned int i = 0; i < permsids.size(); i++)
        {
            if (std::find(assignable_ids.begin(), assignable_ids.end(), permsids[i]) == assignable_ids.end())
            {
                rbacapi::fail(client, "Permission is not assignable for this company.");
                return "";
            }
        }
    }

    std::string error;
    // 该接口用给定集合整体替换角色权限，重复调用结果一致
    // This endpoint replaces the whole permission set, so repeated calls are idempotent.
    auto replace_result = rbac::replace_role_perms(roleid, user_companyid, permsids, operid, error);
    if (replace_result == rbac::replace_role_perms_result::rejected)
    {
        rbacapi::fail(client, error.empty() ? "bind role permissions failed" : error);
        return "";
    }
    if (replace_result == rbac::replace_role_perms_result::rolled_back)
    {
        rbacapi::fail(client, error.empty() ? "Role permissions were rolled back; nothing was changed." : error);
        return "";
    }
    // 角色与权限的绑定变了，绑了该角色的账户生效集合都变了；整表失效，不漏人
    // Every account holding this role is affected, so invalidate the whole table.
    rbac::get_route_cache().invalidate_all();

    client.val["code"] = 0;
    client.val["msg"]  = "ok";
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/roleperm/unbind)
std::string rbac_roleperm_unbind(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    if (!rbacapi::check_csrf(client))
    {
        rbacapi::csrf_fail(client);
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    unsigned int roleid  = (unsigned int)client.post["roleid"].to_int();
    unsigned int permsid = (unsigned int)client.post["permsid"].to_int();
    if (roleid == 0 || permsid == 0 || !role_owned(roleid, user_companyid))
    {
        rbacapi::fail(client, "role not found in this company");
        return "";
    }

    // 单条 DELETE 自带原子性，不用事务，但失败得认出来：无条件报 ok 时授权行还在，
    // 缓存整表失效后下次判定按旧集合装载，看不出任何异常。
    // One DELETE is atomic, so no transaction — but a silent "ok" leaves the grant row in place
    // while the cache reload re-reads the set that was supposed to lose it.
    orm::cms::Sysroleperms roleperms;
    roleperms.where("roleid", roleid).whereAnd("permsid", permsid).whereAnd("userid", user_companyid).remove();
    // 0 行被删是合法结果（这条授权可能早已解绑），失败只由 iserror / error_msg 报。
    // Zero rows deleted is legitimate here (the grant may already be gone); only the flags fail.
    if (roleperms.iserror || !roleperms.error_msg.empty())
    {
        rbacapi::fail(client, "unbind role permission failed: " + (roleperms.error_msg.empty() ? "unknown error" : roleperms.error_msg));
        return "";
    }

    // 少了一条授权，绑了该角色的账户下次判定就该拒绝；整表失效，不漏人
    // One grant disappeared, so every account holding this role must re-read.
    rbac::get_route_cache().invalidate_all();

    client.val["code"] = 0;
    client.val["msg"]  = "ok";
    client.out_json();
    return "";
}
}// namespace http
