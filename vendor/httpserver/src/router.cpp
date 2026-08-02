#include <map>
#include <memory>
#include <string>
#include <vector>

#include "router.h"
#include "httppeer.h"
#include "serverconfig.h"
#include "func.h"
#include "debug_log.h"

namespace http
{

router &get_router()
{
    static router instance;
    return instance;
}

std::map<std::string, std::map<std::string, regmethold_t>>::iterator router::find_domain_methods(std::shared_ptr<httppeer> &peer)
{
    serverconfig &sysconfigpath = getserversysconfig();
    if (sysconfigpath.sitehostinfos[peer->host_index].alias_domain.size() > 0)
    {
        return _domain_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].alias_domain);
    }
    return _domain_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].mainhost);
}

std::string router::resolve(std::shared_ptr<httppeer> peer)
{
    serverconfig &sysconfigpath = getserversysconfig();
    std::string regmethold_path;

    std::map<std::string, std::map<std::string, regmethold_t>>::iterator domain_method_map_iter          = find_domain_methods(peer);
    std::map<std::string, std::map<std::string, std::vector<std::string>>>::iterator domain_url_map_iter = _domain_regurlpath_table.end();
    if (sysconfigpath.sitehostinfos[peer->host_index].alias_domain.size() > 0)
    {
        domain_url_map_iter = _domain_regurlpath_table.find(sysconfigpath.sitehostinfos[peer->host_index].alias_domain);
    }
    else
    {
        domain_url_map_iter = _domain_regurlpath_table.find(sysconfigpath.sitehostinfos[peer->host_index].mainhost);
    }

    if (peer->pathinfos.size() > 0)
    {
        unsigned int pathinfos_size = peer->pathinfos.size();
        for (unsigned int i = pathinfos_size; i > 0; i--)
        {
            regmethold_path.clear();
            for (unsigned int j = 0; j < i; j++)
            {
                if (j > 0)
                {
                    regmethold_path.push_back('/');
                }
                regmethold_path.append(get_filename(peer->pathinfos[j]));
            }

            if (domain_method_map_iter != _domain_regmethod_table.end())
            {
                if (domain_method_map_iter->second.contains(regmethold_path))
                {
                    if (pathinfos_size != i)
                    {
                        if (domain_url_map_iter != _domain_regurlpath_table.end() && domain_url_map_iter->second.contains(regmethold_path))
                        {
                            for (unsigned int m = i; m < pathinfos_size; m++)
                            {
                                if (domain_url_map_iter->second[regmethold_path].size() > m)
                                {
                                    if (domain_url_map_iter->second[regmethold_path][m].size() > 0)
                                    {
                                        peer->get[domain_url_map_iter->second[regmethold_path][m]] = peer->pathinfos[m];
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }

            if (_http_regmethod_table.contains(regmethold_path))
            {
                if (pathinfos_size != i)
                {
                    if (_http_regurlpath_table.contains(regmethold_path))
                    {
                        for (unsigned int m = i; m < pathinfos_size; m++)
                        {
                            if (_http_regurlpath_table[regmethold_path].size() > m)
                            {
                                if (_http_regurlpath_table[regmethold_path][m].size() > 0)
                                {
                                    peer->get[_http_regurlpath_table[regmethold_path][m]] = peer->pathinfos[m];
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                regmethold_path.clear();
            }
        }
    }

    if (regmethold_path.empty())
    {
        if (peer->pathinfos.size() > 0)
        {
            if (domain_method_map_iter != _domain_regmethod_table.end())
            {
                if (domain_method_map_iter->second.contains("404"))
                {
                    regmethold_path = "404";
                }
            }
        }
        if (regmethold_path.empty())
        {
            regmethold_path = "home";
        }
    }
    return regmethold_path;
}

std::string router::run_chain(std::map<std::string, regmethold_t> &method_table,
                              std::map<std::string, regmethold_t>::iterator method_iter,
                              std::string regmethold_path,
                              std::shared_ptr<httppeer> &peer)
{
    std::string sitecontent;
    for (int i = 0; i < 30; i++)
    {
        if (method_iter->second.pre != nullptr)
        {
            sitecontent = method_iter->second.pre(peer);
            if (sitecontent.size() == 2 && str_casecmp(sitecontent, "ok"))
            {
                sitecontent = method_iter->second.regfun(peer);
                if (sitecontent.size() == 1 && sitecontent[0] == 'T')
                {
                    auto method_loop_iter = method_table.find("frametasks_timeloop");
                    if (method_loop_iter != method_table.end())
                    {
                        sitecontent = method_loop_iter->second.regfun(peer);
                    }
                    sitecontent.clear();
                }
            }
            else
            {
                if (sitecontent.empty())
                {
                    return sitecontent;
                }
                if (str_casecmp(regmethold_path, sitecontent))
                {
                    sitecontent = method_iter->second.regfun(peer);
                }
                else
                {
                    peer->push_flow(regmethold_path);// record not execute method
                    auto method_loop_iter = method_table.find(sitecontent);
                    if (method_loop_iter != method_table.end())
                    {
                        sitecontent = method_loop_iter->second.regfun(peer);
                    }
                    else
                    {
                        // redirect target is not in this table, hand it back to the caller
                        return sitecontent;
                    }
                }
            }
        }
        else
        {
            sitecontent = method_iter->second.regfun(peer);
        }

        if (sitecontent.empty())
        {
            return sitecontent;
        }
        if (sitecontent.size() == 4 && str_casecmp(sitecontent, "exit"))
        {
            sitecontent.clear();
            return sitecontent;
        }

        method_iter = method_table.find(sitecontent);
        if (method_iter == method_table.end())
        {
            return sitecontent;
        }
        regmethold_path = sitecontent;
    }
    return sitecontent;
}

void router::dispatch(std::shared_ptr<httppeer> peer)
{
    serverconfig &sysconfigpath = getserversysconfig();
    std::string regmethold_path = resolve(peer);
    std::string sitecontent;

    DEBUG_LOG("http_regmethod pre in");
    if (sysconfigpath.sitehostinfos[peer->host_index].is_method_pre)
    {
        if (sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists.size() > 0 && sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists.size() < 16)
        {
            for (unsigned char jk = 0; jk < sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists.size(); jk++)
            {
                if (sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists[jk].size() > 0 && _http_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists[jk]) != _http_regmethod_table.end())
                {
                    sitecontent = _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].action_pre_lists[jk]].regfun(peer);
                    if (sitecontent.size() == 4 && str_casecmp(sitecontent, "exit"))
                    {
                        return;
                    }
                }
            }
        }
    }

    sitecontent.clear();
    unsigned char method_type = 0;

    std::map<std::string, std::map<std::string, regmethold_t>>::iterator domain_method_map_iter = find_domain_methods(peer);
    if (domain_method_map_iter != _domain_regmethod_table.end())
    {
        auto method_iter = domain_method_map_iter->second.find(regmethold_path);
        if (method_iter != domain_method_map_iter->second.end())
        {
            DEBUG_LOG("http domain regmethod main in");
            method_type = 10;
            sitecontent = run_chain(domain_method_map_iter->second, method_iter, regmethold_path, peer);
        }
        if (!sitecontent.empty())
        {
            regmethold_path = sitecontent;
            method_type     = 1;
        }
    }

    if (method_type > 9)
    {
    }
    else if (auto method_iter = _http_regmethod_table.find(regmethold_path); method_iter != _http_regmethod_table.end())
    {
        DEBUG_LOG("http_regmethod main in");
        run_chain(_http_regmethod_table, method_iter, regmethold_path, peer);
    }
    else if (method_type > 0)
    {
    }
    else
    {
        make_404_content(peer);
    }
    DEBUG_LOG("action method after");
    if (sysconfigpath.sitehostinfos[peer->host_index].is_method_after)
    {
        if (sysconfigpath.sitehostinfos[peer->host_index].action_after_lists.size() > 0 && sysconfigpath.sitehostinfos[peer->host_index].action_after_lists.size() < 16)
        {
            for (unsigned char jk = 0; jk < sysconfigpath.sitehostinfos[peer->host_index].action_after_lists.size(); jk++)
            {
                if (sysconfigpath.sitehostinfos[peer->host_index].action_after_lists[jk].size() > 0 && _http_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].action_after_lists[jk]) != _http_regmethod_table.end())
                {
                    _http_regmethod_table[sysconfigpath.sitehostinfos[peer->host_index].action_after_lists[jk]].regfun(peer);
                }
            }
        }
    }
}

asio::awaitable<co_route_result> router::co_dispatch(std::shared_ptr<httppeer> peer)
{
    DEBUG_LOG("---  in %s co_dispatch --------", peer->sendfilename.c_str());
    co_route_result route_ret;
    route_ret.matched   = false;
    serverconfig &sysconfigpath = getserversysconfig();

    if (sysconfigpath.sitehostinfos[peer->host_index].alias_domain.size() > 0)
    {
        auto iter1 = _co_domain_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].alias_domain);
        if (iter1 != _co_domain_regmethod_table.end())
        {
            auto iter2 = iter1->second.find(peer->sendfilename);
            if (iter2 != iter1->second.end())
            {
                std::string respcontent;
                route_ret.matched   = true;
                route_ret.need_sync = false;

                if (iter2->second.pre != nullptr)
                {
                    respcontent = co_await iter2->second.pre(peer);
                    if (respcontent.size() == 2 && str_casecmp(respcontent, "ok"))
                    {
                        respcontent = co_await iter2->second.regfun(peer);
                    }
                }
                else
                {
                    respcontent = co_await iter2->second.regfun(peer);
                }
            }
        }
    }
    else
    {
        auto iter1 = _co_domain_regmethod_table.find(sysconfigpath.sitehostinfos[peer->host_index].mainhost);
        if (iter1 != _co_domain_regmethod_table.end())
        {
            auto iter2 = iter1->second.find(peer->sendfilename);
            if (iter2 != iter1->second.end())
            {
                std::string respcontent;
                route_ret.matched   = true;
                route_ret.need_sync = false;
                if (iter2->second.pre != nullptr)
                {
                    respcontent = co_await iter2->second.pre(peer);
                    if (respcontent.size() == 2 && str_casecmp(respcontent, "ok"))
                    {
                        respcontent = co_await iter2->second.regfun(peer);
                    }
                }
                else
                {
                    respcontent = co_await iter2->second.regfun(peer);
                }
            }
        }
    }
    if(route_ret.matched)
    {
        co_return route_ret;
    }

    auto co_iter = _co_http_regmethod_table.find(peer->sendfilename);
    if (co_iter != _co_http_regmethod_table.end())
    {
        route_ret.matched   = true;
        route_ret.need_sync = false;
        std::string respcontent;
        if (co_iter->second.pre != nullptr)
        {
            respcontent = co_await co_iter->second.pre(peer);
            if (respcontent.size() == 2 && str_casecmp(respcontent, "ok"))
            {
                respcontent = co_await co_iter->second.regfun(peer);
            }
        }
        else
        {
            respcontent = co_await co_iter->second.regfun(peer);
        }

        for (unsigned int co_loop_num = 0; co_loop_num < 30; ++co_loop_num)
        {
            if (respcontent.size() > 0)
            {
                co_iter = _co_http_regmethod_table.find(respcontent);
                if (co_iter != _co_http_regmethod_table.end())
                {
                    respcontent = co_await co_iter->second.regfun(peer);
                }
                else
                {
                    if (_http_regmethod_table.contains(respcontent))
                    {
                        // the chain redirected to a sync method, fall back to the thread pool
                        route_ret.need_sync = true;
                    }
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    co_return route_ret;
}

}// namespace http
