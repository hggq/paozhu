#include "httppeer.h"

namespace http
{
// 注册条目上的掩码怎么落到请求上：注解里冒号后面的数字（//@urlpath(前置过滤器,URL路径):数字）
// 存进这条注册，路由在调用 pre 之前把它写进 client.mask_code，pre 就能按它做判断。
// 直接访问 /maskpre/check 时，这条注册自己没写冒号，所以读到 0，pre 会拒绝。
//@urlpath(null,maskpre/check)
std::string test_mask_pre(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "pre 读到 mask_code=" << client.mask_code << "<br />";
    if (client.mask_code == 0)
    {
        client << "掩码为 0，pre 拦下这次请求";
        return "exit";
    }
    client << "掩码非 0，pre 放行";
    return "ok";
}

//@urlpath(test_mask_pre,maskpre):123456
std::string test_maskcode(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "handler 读到 mask_code=" << client.mask_code;
    return "";
}

}// namespace http
