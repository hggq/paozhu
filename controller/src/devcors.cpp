#include <chrono>
#include <thread>
#include "httppeer.h"
#include "devcors.h"
namespace http
{
// CORS 全部由框架处理，业务侧不再手写：
//   普通请求 → 解析期 cors_origin_process() 按站点 cors_domain 白名单设 Allow-Origin / Expose-Headers
//   OPTIONS  → httpserver::send_cors_domain() 直接应答预检，不会分发到控制器
// 原 cors_domain("*") 与 OPTIONS 分支（现在已不可达）随之删除，本接口只回演示数据
//@urlpath(null,api/dev/hostcors)
std::string api_dev_hostcors(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.val["code"]       = 0;
    client.val["host"]       = client.get_header("host");
    client.val["host_index"] = (int)client.host_index;
    client.out_json();
    return "";
}

}// namespace http