#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testjsonreflect.h"
#include "json_reflect_headers.h"
#include "array_to_tree.h"
namespace http
{
//@urlpath(null,testjsonreflect)
std::string testjsonreflect(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    // client << " json reflect 🧨 Paozhu c++ web framework ";

    psy::department_outjson_t deps_json_one;
    std::vector<psy::department_outjson_t> depsjsonlist;

    deps_json_one.id          = 38;
    deps_json_one.key         = 38;
    deps_json_one.value       = 38;
    deps_json_one.parentid    = 0;
    deps_json_one.isopen      = true;
    deps_json_one.title       = "First Item 第一条";
    deps_json_one.desc        = "First Item 第一条备注";
    deps_json_one.bianzhi_num = 12;

    depsjsonlist.push_back(deps_json_one);

    deps_json_one.id          = 48;
    deps_json_one.key         = 48;
    deps_json_one.value       = 48;
    deps_json_one.parentid    = 0;
    deps_json_one.isopen      = true;
    deps_json_one.title       = "Second Item 第二条";
    deps_json_one.desc        = "Second memo 第二条备注";
    deps_json_one.bianzhi_num = 16;

    depsjsonlist.push_back(deps_json_one);

    deps_json_one.id          = 52;
    deps_json_one.key         = 52;
    deps_json_one.value       = 52;
    deps_json_one.parentid    = 48;
    deps_json_one.isopen      = true;
    deps_json_one.title       = "Sub Second Item 第二条";
    deps_json_one.desc        = "Sub Second memo 第二条备注";
    deps_json_one.bianzhi_num = 16;

    depsjsonlist.push_back(deps_json_one);

    deps_json_one.id          = 58;
    deps_json_one.key         = 58;
    deps_json_one.value       = 58;
    deps_json_one.parentid    = 0;
    deps_json_one.isopen      = true;
    deps_json_one.title       = "Three Item 第二条01";
    deps_json_one.desc        = "Three memo 第二条01备注";
    deps_json_one.bianzhi_num = 22;

    depsjsonlist.push_back(deps_json_one);

    psy::department_listoutjson_t depout_data;

    array_to_tree<psy::department_outjson_t>(depout_data.data.list, depsjsonlist);

    depout_data.code = 0;
    // depout_data.data.list=depsjsonlist; //has array_to_tree
    depout_data.data.total = 0;

    std::string jsondep_str = psy::json_encode(depout_data);

    psy::department_listoutjson_t new_depout_data;
    psy::json_decode(new_depout_data, jsondep_str);
    std::cout << "data.list:" << depout_data.data.list.size() << std::endl;
    std::cout << "data.list:" << new_depout_data.data.list.size() << std::endl;

    std::string jsondep_str_temp = psy::json_encode(new_depout_data);

    client << jsondep_str_temp;
    client.out_json();
    return "";
}

} // namespace http