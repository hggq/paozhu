#pragma once

#include <string>
#include <vector>

namespace http
{
namespace rbac
{

struct route_entry
{
    std::string site;   // "*" = 全局 slot 0；其它为 alias_domain/mainhost
    std::string urlpath;// 注册键，已带前导 '/'
    unsigned int mask_code  = 0;
    unsigned char rate_time = 0;
    unsigned char rate_num  = 0;
    unsigned char pre_kind  = 0;// 1=sync 2=coro 0=none
    unsigned char reg_kind  = 0;// 1=sync 2=coro 0=none
    std::string title;          // 注解 :title(名称) 里的名称，没写为空串
};

// 当前请求 host_index 对应的注册 slot（越界回退到全局 slot 0）。
unsigned int current_slot_for_host(unsigned int host_index);

// slot 对应的站点名；slot 0 返回 "*"。
std::string site_name_for_slot(unsigned int slot);

// 只收集「全局 slot 0 + 当前域名 slot」的路由，供注册函数列表登记。
std::vector<route_entry> collect_routes(unsigned int current_slot);

// 全部站点的路由，只用于页面只读展示，不允许登记。
std::vector<route_entry> list_all_routes();

}// namespace rbac
}// namespace http
