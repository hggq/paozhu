#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <sys/stat.h>

#include "request.h"
#include "serverconfig.h"
#include "pz_dll.h"
#include "loadmodule.h"
#include "httppeer.h"
#if defined(ENABLE_VIEWSO) || defined(ENABLE_MODELSO)
#include "http_so_common_api.h"
#endif
namespace http
{
#ifdef ENABLE_VIEWSO
std::mutex loadviewmtx;
std::string httpempty([[maybe_unused]] obj_val &a) { return "<p>--.so image not found--</p>"; }
so_cachepath_t &get_sharedpathchache()
{
    static so_cachepath_t instance;
    return instance;
}
method_callback_t viewmodulecreate(std::string module, std::string name)
{
    try
    {
        auto lib = std::make_shared<pz::dll::library>(module);
        if (lib->has("_setclientapi"))
        {
            auto setclientapicall = lib->get_alias<http::clientapi *(*)(http::clientapi *)>("_setclientapi");
            http::clientapi *pn  = http::clientapi::instance();
            setclientapicall(pn);
        }
        if (lib->has(name))
        {
            auto viewcall = lib->get_alias<std::string (*)(obj_val &)>(name);
            // Keep the library alive as long as the callback is cached: the function pointer is
            // invalid after dlclose.
            return [lib, viewcall](obj_val &obj) { return viewcall(obj); };
        }
    }
    catch (std::exception &e)
    {
        // error log 捕获
        std::cout << e.what() << std::endl;
    }
    return httpempty;
}

method_callback_t loadview(std::string modulemethod)
{
    std::string hash;
    std::string path;
    serverconfig &sysconfigpath = getserversysconfig();

    path = sysconfigpath.map_value["default"]["viewsopath"];

    if (path.empty())
    {
        path.append("module/view/");
    }
    else
    {
        if (path.back() != '/')
        {
            path.push_back('/');
        }
    }

    hash.append(path);

    if (modulemethod[0] == '/')
    {

        hash.append(modulemethod.substr(1, modulemethod.size() - 1));
    }
    else
    {
        hash.append(modulemethod);
    }
    hash.append(".so");

    struct stat modso;
    if (stat(hash.c_str(), &modso) == 0)
    {
        if (modso.st_mode & S_IFREG)
        {
        }
        else
        {
            hash.clear();
        }
    }
    else
    {
        hash.clear();
    }
    if (hash.empty())
    {
        return httpempty;
    }
    path = hash;
    hash.clear();
    hash.append(modulemethod);
    hash.append(path);
    size_t t              = std::hash<std::string>{}(hash);
    auto sharedpathchache = get_sharedpathchache();
    if (sharedpathchache.find(t) == sharedpathchache.end())
    {
        std::unique_lock<std::mutex> lock(loadviewmtx);
        if (sharedpathchache.find(t) == sharedpathchache.end())
        {
            sharedpathchache[t] = viewmodulecreate(path, "view");
        }
    }
    return sharedpathchache[t];
}

void viewmoduleclear(std::string module, std::string method)
{
    std::string path;
    serverconfig &sysconfigpath = getserversysconfig();

    std::string siteviewpath = sysconfigpath.map_value["default"]["viewsopath"];
    path.append(module);
    path.append("/");
    path.append(method);
    path.append(siteviewpath);

    if (siteviewpath.back() != '/')
    {
        path.push_back('/');
    }
    path.append(module);
    path.append("/");
    path.append(method);

    path.append(".so");
    size_t t              = std::hash<std::string>{}(path);
    auto sharedpathchache = get_sharedpathchache();
    auto citer            = sharedpathchache.find(t);
    if (citer != sharedpathchache.end())
    {
        try
        {
            citer->second = nullptr;
            std::unique_lock<std::mutex> lock(loadviewmtx);
            sharedpathchache.erase(citer);
        }
        catch (std::exception &e)
        {
            // error log 捕获
            std::cout << e.what() << std::endl;
        }
    }
}
#endif // ENABLE_VIEWSO

#ifdef ENABLE_MODELSO
std::mutex loadcontrolmtx, moudulecachemethod;
std::string httpso(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<p>-- controller not found--</p>";
    return "";
}
std::map<std::size_t, www_method_call> &get_controlpathchache()
{
    static std::map<std::size_t, www_method_call> instance;
    return instance;
}
std::map<std::size_t, std::vector<std::string>> &get_sharedmethodchache()
{
    static std::map<std::size_t, std::vector<std::string>> instance;
    return instance;
}
www_method_call controlmodulecreate(std::string module, std::string name, size_t tt)
{
    auto sharedmethodchache = get_sharedmethodchache();
    auto controlpathchache = get_controlpathchache();
    size_t t                = std::hash<std::string>{}(module);
    auto iter               = sharedmethodchache.find(t);
    bool hasmethod          = true;
    if (iter != sharedmethodchache.end())
    {
        for (size_t i = 0; i < iter->second.size(); i++)
        {
            if (iter->second[i] == name)
            {
                hasmethod = false;
                break;
            }
        }
    }
    if (hasmethod)
    {
        sharedmethodchache[t].emplace_back(name);
    }

    try
    {
        auto lib = std::make_shared<pz::dll::library>(module);
        if (lib->has("_setclientapi"))
        {
            auto setclientapicall = lib->get_alias<http::clientapi *(*)(http::clientapi *)>("_setclientapi");
            http::clientapi *pn  = http::clientapi::instance();
            setclientapicall(pn);
        }

        if (lib->has(name))
        {
            auto methodcall = lib->get_alias<std::string (*)(std::shared_ptr<http::httppeer>)>(name);
            // Keep the library alive as long as the callback is cached.
            controlpathchache[tt] = [lib, methodcall](std::shared_ptr<http::httppeer> peer) {
                return methodcall(peer);
            };
            return controlpathchache[tt];
        }
        else if (lib->has("_init404"))
        {
            auto init404 = lib->get_alias<std::string (*)(std::shared_ptr<http::httppeer>)>("_init404");
            controlpathchache[tt] = [lib, init404](std::shared_ptr<http::httppeer> peer) {
                return init404(peer);
            };
            return controlpathchache[tt];
        }
    }
    catch (std::exception &e)
    {
        // error log 捕获
        std::cout << e.what() << std::endl;
    }
    return httpso;
}
www_method_call loadcontrol(const std::string &modulemethod, const std::string &hashname)
{

    size_t t = std::hash<std::string>{}(modulemethod);

    auto controlpathchache = get_controlpathchache();
    if (controlpathchache.find(t) == controlpathchache.end())
    {
        std::unique_lock<std::mutex> lock(loadcontrolmtx);
        if (controlpathchache.find(t) == controlpathchache.end())
        {
            controlpathchache[t] = controlmodulecreate(modulemethod, hashname, t);
        }
    }

    return controlpathchache[t];
}

void controlmoduleclear(std::string module, std::string method)
{
    serverconfig &sysconfigpath = getserversysconfig();

    std::string sitecontrolpath = sysconfigpath.map_value["default"]["controlsopath"];
    std::string path(sitecontrolpath);
    if (path.back() != '/')
    {
        path.push_back('/');
    }
    path.append(module);

    if (!path.ends_with(".so"))
    {
        path.append(".so");
    }
    size_t t                = std::hash<std::string>{}(path);
    auto sharedmethodchache = get_sharedmethodchache();
    auto iter               = sharedmethodchache.find(t);
    auto controlpathchache  = get_controlpathchache();
    try
    {
        if (iter != sharedmethodchache.end())
        {
            // 清除所有方法
            std::string hash;
            for (size_t i = 0; i < iter->second.size(); i++)
            {
                hash.clear();
                hash.append(module);
                hash.push_back('/');
                hash.append(iter->second[i]);

                hash.append(path);
                size_t hash_t = std::hash<std::string>{}(hash);
                auto citer    = controlpathchache.find(hash_t);
                if (citer != controlpathchache.end())
                {
                    citer->second = nullptr;
                    std::unique_lock<std::mutex> lock(loadcontrolmtx);
                    controlpathchache.erase(citer);
                }
            }
            std::unique_lock<std::mutex> lock(moudulecachemethod);
            sharedmethodchache.erase(iter);
        }
    }
    catch (std::exception &e)
    {
        // error log 捕获
        std::cout << e.what() << std::endl;
    }
    path.clear();
    path.append(module);
    path.append("/");
    path.append(method);
    path.append(sitecontrolpath);
    if (path.back() != '/')
    {
        path.push_back('/');
    }
    path.append(module);
    if (!path.ends_with(".so"))
    {
        path.append(".so");
    }
    t = std::hash<std::string>{}(path);

    auto citer = controlpathchache.find(t);
    if (citer != controlpathchache.end())
    {
        try
        {
            citer->second = nullptr;
            std::unique_lock<std::mutex> lock(loadcontrolmtx);
            controlpathchache.erase(citer);
        }
        catch (std::exception &e)
        {
            // error log 捕获
            std::cout << e.what() << std::endl;
        }
    }
}
#endif // ENABLE_MODELSO

}// namespace http
