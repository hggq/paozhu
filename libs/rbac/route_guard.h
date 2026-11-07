#pragma once

#include <string>

namespace http
{
class httppeer;

namespace rbac
{
// 运行期 RBAC 判定入口：只有注解里带 maskcode（注册条目 mask_code != 0）的路由被 router 调用。
// 会话键在这里读，判定与缓存都在 route_cache，router 不认识任何业务会话键。
// 返回 0 = 放行；非 0 = 这条请求要写的状态码：
//   401 会话里没有 adminid（没登录）；
//   403 会话里的租户 id 是 0（登录了但没绑企业），或授权表里没有一条 maskcode 与该路由的掩码相等。
// Runtime RBAC decision entry: only routes whose annotation carries a maskcode (registered entry
// mask_code != 0) are dispatched here by the router, so the guard sits inside route dispatch
// rather than in front of it; unmasked routes never reach this function.
// The session keys are read here and only here: adminid identifies the login (sysuser.adminid),
// companyid identifies its tenant (sysuser.companyid). The router knows no business session keys;
// the decision and the grant cache live in route_cache.
// Fail-closed: a missing adminid yields 401 (not logged in); companyid == 0 (logged in but not
// bound to a company), a missing/empty urlpath, or a grant table without one maskcode exactly
// equal to this route's mask yields 403. A cache load failure is also 403, never a bypass.
// Return value: 0 = allowed; non-zero = the status code the request must write (401 or 403).
unsigned int check_masked_route(httppeer &client, const std::string &urlpath, unsigned int mask_code);

// 安装 router 钩子；只应调用一次，由应用入口在 run() 之前调用。
// Installs the router auth hook; call it exactly once from the application entry point, right
// before httpmy.run(). run() connects the database and then starts the accept loop, so the hook
// is already in place before the server accepts its first connection, and the database is
// available by the time the first masked request needs it. Installing only registers a callback
// and issues no query: if the permission tables are unreadable at startup, the first masked
// request fails closed (403) instead of blocking startup.
void install_route_guard();
}// namespace rbac
}// namespace http
