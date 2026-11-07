#include "route_catalog.h"

#include <unordered_set>

#include "httppeer.h"
#include "serverconfig.h"

namespace http
{
namespace rbac
{

namespace
{

route_entry make_entry(unsigned int slot, const std::string &path, unsigned int idx)
{
    route_entry e;
    e.site    = site_name_for_slot(slot);
    e.urlpath = path;
    if (idx < _handlers.size())
    {
        auto const &h = _handlers[idx];
        e.mask_code   = h.mask_code;
        e.rate_time   = h.rate_time;
        e.rate_num    = h.rate_num;
        e.pre_kind    = static_cast<unsigned char>(h.pre_kind);
        e.reg_kind    = static_cast<unsigned char>(h.reg_kind);
        e.title       = h.title;
    }
    return e;
}

void collect_slot(unsigned int slot, std::vector<route_entry> &out, std::unordered_set<std::string> *seen)
{
    if (slot >= _slots.size())
    {
        return;
    }
    for (auto const &kv : _slots[slot].path_map)
    {
        if (seen != nullptr && !seen->insert(kv.first).second)
        {
            continue;
        }
        out.push_back(make_entry(slot, kv.first, kv.second));
    }
}

}// namespace

unsigned int current_slot_for_host(unsigned int host_index)
{
    auto &cfg = getserversysconfig();
    if (host_index < cfg.sitehostinfos.size())
    {
        return cfg.sitehostinfos[host_index].slot_id;
    }
    return 0;
}

std::string site_name_for_slot(unsigned int slot)
{
    if (slot == 0)
    {
        return "*";
    }
    for (auto const &kv : _site_to_slot)
    {
        if (kv.first.empty() || kv.first == "*")
        {
            continue;
        }
        if (kv.second == slot)
        {
            return kv.first;
        }
    }
    return "*";
}

std::vector<route_entry> collect_routes(unsigned int current_slot)
{
    // 先收站点 slot，再收全局 slot：同一 urlpath 只保留站点这一条，保证列表、勾选值和保存
    // 命中是同一行（站点优先）。
    // Collect the site slot first, then the global slot: a urlpath keeps only the site entry, so
    // the list row, the checkbox value and the save lookup all resolve to the same entry.
    std::vector<route_entry> out;
    std::unordered_set<std::string> seen;
    if (current_slot != 0)
    {
        collect_slot(current_slot, out, &seen);
    }
    collect_slot(0, out, &seen);
    return out;
}

std::vector<route_entry> list_all_routes()
{
    std::vector<route_entry> out;
    for (unsigned int slot = 0; slot < _slots.size(); ++slot)
    {
        collect_slot(slot, out, nullptr);
    }
    return out;
}

}// namespace rbac
}// namespace http
