
#include <iostream>
#include <string>
#include <mysqlx/xdevapi.h>
#include <boost/dll/alias.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>
#include "request.h"
#include "serverconfig.h"
#include "loadmodule.h"
#include "httppeer.h"
#include "http_so_common_api.h"
namespace http
{
    std::mutex loadcontrolmtx,loadviewmtx,moudulecachemethod;
    std::string httpempty(OBJ_VALUE &a)
    {
        return "<p>--.so image not found--</p>";
    }
    std::string httpso(std::shared_ptr<httppeer> peer)
    {   
        httppeer &client = peer->getpeer();
        client<<"<p>-- controller not found--</p>";
        return "";
    }
    so_cachepath_t &get_sharedpathchache()
    {
        static so_cachepath_t instance;
        return instance;
    }
    std::map<std::size_t,www_method_call> &get_controlpathchache()
    {
        static std::map<std::size_t,www_method_call>  instance;
        return instance;
    }
    std::map<std::size_t,std::vector<std::string>> &get_sharedmethodchache()
    {
        static std::map<std::size_t,std::vector<std::string>> instance;
        return instance;
    }
    method_callback_t viewmodulecreate(std::string module, std::string name)
    {
        boost::dll::fs::path shared_library_path = module;
        try
        {
            if (boost::dll::fs::exists(shared_library_path))
            {

                boost::dll::shared_library lib(shared_library_path);
                if(lib.has("_setclientapi"))  
                {
                    boost::function<http::clientapi*(http::clientapi*)> setclientapicall= boost::dll::import_alias<http::clientapi*(http::clientapi*)>(shared_library_path, "_setclientapi"); 
                        http::clientapi* pn =http::clientapi::instance();
                        setclientapicall(pn);
                }
                if (lib.has(name))
                {
                    return std::move(boost::dll::import_alias<std::string(OBJ_VALUE &)>(shared_library_path, name));
                }
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
        size_t t = std::hash<std::string>{}(hash);
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
    www_method_call controlmodulecreate(std::string module, std::string name, size_t tt)
    {
        auto sharedmethodchache=get_sharedmethodchache();
        auto controlpathchache = get_controlpathchache();
        boost::dll::fs::path shared_library_path = module;
        size_t t = std::hash<std::string>{}(module);
        auto iter = sharedmethodchache.find(t);
        bool hasmethod = true;
        if (iter != sharedmethodchache.end())
        {
            for (int i = 0; i < iter->second.size(); i++)
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
       
         try {
    
                    if(boost::dll::fs::exists(shared_library_path)){  
                            boost::dll::shared_library lib(shared_library_path); 
                            if(lib.has("_setclientapi"))  
                            {
                              boost::function<http::clientapi*(http::clientapi*)> setclientapicall= boost::dll::import_alias<http::clientapi*(http::clientapi*)>(shared_library_path, "_setclientapi"); 
                                    http::clientapi* pn =http::clientapi::instance();
                                    setclientapicall(pn);
                            }

                             if(lib.has(name))  
                            {
                               controlpathchache[tt]=std::move(boost::dll::import_alias<std::string(std::shared_ptr<http::httppeer> peer)>(shared_library_path, name ));
                               return controlpathchache[tt]; 
                            }else if(lib.has("_init404")){
                                controlpathchache[tt]=std::move(boost::dll::import_alias<std::string(std::shared_ptr<http::httppeer> peer)>(shared_library_path, "_init404" ));
                               return controlpathchache[tt]; 
                            }
                                   
                            
                    }

                    
            }catch (std::exception& e)  
            {  
                //error log 捕获
                std::cout << e.what() << std::endl; 
            }  
        return httpso;
    }
    www_method_call loadcontrol(const std::string& modulemethod,const std::string& hashname)
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
        size_t t = std::hash<std::string>{}(path);
        auto sharedpathchache = get_sharedpathchache();
        auto citer = sharedpathchache.find(t);
        if (citer != sharedpathchache.end())
        {
            try
            {
                citer->second.clear();
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
        size_t t = std::hash<std::string>{}(path);
        auto sharedmethodchache=get_sharedmethodchache();
        auto iter = sharedmethodchache.find(t);
        auto controlpathchache = get_controlpathchache();
        try
        {
            if (iter != sharedmethodchache.end())
            {
                //清除所有方法
                std::string hash;
                for (int i = 0; i < iter->second.size(); i++)
                {
                    hash.clear();
                    hash.append(module);
                    hash.push_back('/');
                    hash.append(iter->second[i]);

                    hash.append(path);
                    size_t t = std::hash<std::string>{}(hash);
                    auto citer = controlpathchache.find(t);
                    if (citer != controlpathchache.end())
                    {
                        citer->second.clear();
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
                citer->second.clear();
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

}