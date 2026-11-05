#ifdef ENABLE_VIEWSO
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif
#include "request.h"
#include "cookie.h"
#include "pz_dll.h"
#include "loadviewso.h"
#include "viewso_param.h"
#include "http_so_common_api.h"
namespace http
{
std::mutex loadviewsomtx;
std::string httpempty([[maybe_unused]] struct view_param, [[maybe_unused]] obj_val &a) { return "<p>--.so image not found--</p>"; }

view_so_cachepath_t &get_viewsharedpathchache()
{
    static view_so_cachepath_t instance;
    return instance;
}
std::string &get_hostsopath()
{
    static std::string instance;
    return instance;
}
view_method_callback_t viewsomodulecreate(std::string module, std::string name)
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
            auto viewcall = lib->get_alias<std::string (*)(struct view_param, obj_val &)>(name);
            // Keep the library alive as long as the callback is cached: the function pointer is
            // invalid after dlclose.
            return [lib, viewcall](struct view_param vinfo, obj_val &obj) { return viewcall(vinfo, obj); };
        }
    }
    catch (std::exception &e)
    {
        // error log 捕获
        std::cout << e.what() << std::endl;
    }
    return httpempty;
}

view_method_callback_t loadviewso(std::string modulemethod)
{
    std::string hash;
    std::string path;

    hash.append(modulemethod);
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
    auto sharedpathchache = get_viewsharedpathchache();
    if (sharedpathchache.find(t) == sharedpathchache.end())
    {
        std::unique_lock<std::mutex> lock(loadviewsomtx);
        if (sharedpathchache.find(t) == sharedpathchache.end())
        {
            sharedpathchache[t] = viewsomodulecreate(path, "view");
        }
    }
    return sharedpathchache[t];
}
}// namespace http
#endif
