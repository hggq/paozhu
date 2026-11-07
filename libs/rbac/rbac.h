#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace http
{
namespace rbac
{
// 加载用户的全部有效角色 ID（含 sysuser.roleid 历史单角色）
// Load all effective role ids for a user, including the legacy sysuser.roleid.
bool load_user_roles(unsigned int adminid, unsigned int companyid, std::vector<unsigned int> &roleids);

// 加载角色集合下全部有效权限 ID，并与平台分配给该企业的集合取交集
// Load the permission ids granted to the given roles, narrowed to what the platform allocated
// to this tenant.
bool load_role_perms(const std::vector<unsigned int> &roleids, unsigned int companyid, std::vector<unsigned int> &permsids);

// 装载用户当前生效的全部授权（角色 → 权限 → 分配层 → 目录，6 条查询一次取完），
// 供 route_cache 做按用户的授权快照。值 = 该 urlpath 上所有生效权限行的 maskcode，
// 逐条保留不合并；路由条目的 maskcode 必须与其中某一条精确相等才放行。
// Load the user's effective grants in six statements: urlpath → the maskcode of every granted
// row on that path, kept separate. A route passes only when its maskcode equals one of them.
// An empty allocation set short-circuits the last query, so that path costs five.
// Returning false means a query failed and the caller must fail closed.
bool load_user_grants(unsigned int adminid,
                      unsigned int companyid,
                      std::unordered_map<std::string, std::vector<unsigned int>> &grants);

// 判断用户是否拥有某个 URL 路径的权限
// Check whether a user is granted the given url path.
bool user_has_url(unsigned int adminid, unsigned int companyid, const std::string &urlpath);

// 判断用户是否拥有某个权限码
// Check whether a user is granted the given permission code.
bool user_has_permscode(unsigned int adminid, unsigned int companyid, unsigned int permscode);

// 绑定用户与角色
// Bind one user to one role.
bool bind_user_role(unsigned int userid, unsigned int roleid, unsigned int companyid, unsigned int operid, std::string &error);

// 解绑用户与角色
// Unbind one user from one role.
bool unbind_user_role(unsigned int userid, unsigned int roleid, unsigned int companyid, std::string &error);

// 用给定权限集合替换角色的全部权限
// Replace all permissions of a role with the given set.
bool replace_role_perms(unsigned int roleid, unsigned int companyid, const std::vector<unsigned int> &permsids, unsigned int operid, std::string &error);

// 判断角色是否属于该租户（严格 userid == companyid，companyid 为 0 一律拒绝）
// Check whether a role belongs to the tenant; userid must equal companyid, and 0 is refused.
bool role_in_company(unsigned int roleid, unsigned int companyid);

// 一次取回平台分配给该企业的权限项 ID 集合；返回 false 表示查库出错（调用方按 fail-closed 处理）
// Load the permission ids the platform allocated to this tenant. Returning false means a query
// failed and the caller must fail closed; true with an empty vector means "allocated nothing".
bool load_company_perms(unsigned int companyid, std::vector<unsigned int> &permsids);

// 判断权限项是否「平台已分配给该企业、目录行属于该企业且启用中」——可分配集合的唯一口径
// Check the allocation row exists, the catalog row still belongs to this tenant, and is enabled;
// the assignable-set rule.
bool perms_assignable(unsigned int permsid, unsigned int companyid);
}// namespace rbac
}// namespace http
