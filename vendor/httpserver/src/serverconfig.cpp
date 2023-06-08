#include <string>
#include <map>
#include <memory>
#include "serverconfig.h"
#include "server_localvar.h"
#include <cstring>

namespace http
{
    serverconfig &getserversysconfig()
    {
        static serverconfig instance;
        return instance;
    }
    std::map<std::string, std::map<std::string, std::string>> loadserversconfig(std::string filename)
    {
        std::map<std::string, std::map<std::string, std::string>> sys_config;
        std::map<std::string, std::string> itemconfig;
        // FILE *f = fopen(filename.c_str(), "rb");
        std::unique_ptr<std::FILE, decltype(&std::fclose)> f(fopen(filename.c_str(), "rb"), &std::fclose);
        if (f == nullptr)
        {
            return sys_config;
        }
        fseek(f.get(), 0, SEEK_END);
        auto const size = ftell(f.get());
        fseek(f.get(), 0, SEEK_SET);

        std::string s, linestr, keyname, strval;
        s.resize(size);

        auto nread = fread(&s[0], 1, size, f.get());
        s.resize(nread);
        // fclose(f);

        bool readkey = false;
        bool isvalue = false;
        keyname = "";
        std::string typeone;
        for (unsigned int i = 0; i < s.size(); i++)
        {
            if (s[i] == ';')
            {
                i++;
                if (linestr.size() > 0)
                {
                    itemconfig[linestr] = strval;
                }

                linestr.clear();
                strval.clear();
                isvalue = false;
                readkey = false;
                for (; i < s.size(); i++)
                {
                    if (s[i] == 0x0A)
                    {
                        break;
                    }
                }
            }
            if (s[i] == 0x0A)
            {
                readkey = false;

                if (linestr.size() > 0)
                {
                    itemconfig[linestr] = strval;
                }
                linestr.clear();
                strval.clear();
                isvalue = false;
                continue;
            }
            if (s[i] == '[')
            {
                if (keyname.size() > 0)
                {

                    sys_config[keyname] = itemconfig;
                    itemconfig.clear();
                }
                keyname.clear();
                readkey = true;
                continue;
            }
            if (s[i] == ']')
            {
                readkey = false;
                continue;
            }
            if (s[i] == 0x20)
            {

                continue;
            }
            if (s[i] == '\t')
            {

                continue;
            }
            if (s[i] == '"')
            {

                continue;
            }
            if (s[i] == '=')
            {
                isvalue = true;
                continue;
            }
            if (readkey)
            {
                keyname.push_back(s[i]);
            }
            else
            {
                if (isvalue)
                {
                    strval.push_back(s[i]);
                }
                else
                {
                    linestr.push_back(s[i]);
                }
            }
        }
        if (linestr.size() > 0)
        {
            itemconfig[linestr] = strval;
        }
        if (itemconfig.size() > 0)
        {

            sys_config[keyname] = itemconfig;
        }
        return sys_config;
    }
    
    std::tuple<unsigned int,std::string> serverconfig::gethost404(const std::string &host)
    {
        std::string action_method;
        unsigned int rewritetype=100;

        if (host_toint.find(host) != host_toint.end())
        {
            unsigned int temp_index=host_toint[host];
            if(temp_index<sitehostinfos.size())
            {
                rewritetype = sitehostinfos[temp_index].rewrite404;
                action_method = sitehostinfos[temp_index].rewrite_404_action;
            }
            if(rewritetype!=100)
            {
                return std::make_tuple(rewritetype,action_method);     
            }     
        }

        if (map_value.find(host) != map_value.end())
        {
            if (map_value[host].find("rewrite_404") != map_value[host].end())
            {
                try
                {
                    rewritetype = std::stoul(map_value[host]["rewrite_404"].c_str());
                    if (map_value[host].find("rewrite_404_action") != map_value[host].end())
                    {
                        action_method=map_value[host]["rewrite_404_action"];
                    }
                }
                catch (const std::exception &e)
                {
                    rewritetype = 0;
                }
            }
        }

        if(rewritetype==100)
        {
            rewritetype=0;
            if (map_value.find("default") != map_value.end())
            {
                if (map_value["default"].find("rewrite_404") != map_value["default"].end())
                {
                    try
                    {
                        rewritetype = std::stoul(map_value["default"]["rewrite_404"].c_str());
                        if (map_value["default"].find("rewrite_404_action") != map_value["default"].end())
                        {
                            action_method=map_value["default"]["rewrite_404_action"];
                        }
                    }
                    catch (const std::exception &e)
                    {
                        rewritetype = 0;
                    }
                }
            }
            
        }
        return std::make_tuple(rewritetype,action_method);        
    }
    std::string serverconfig::getsitewwwpath(unsigned int host_index)
    {       
        if(host_index>0&&host_index<sitehostinfos.size())
        {
            return sitehostinfos[host_index].wwwpath;
        }
        return sitehostinfos[0].wwwpath;
    }   
    std::string serverconfig::getsitepath(const std::string &host)
    {

        std::string sitepath;
        bool isnothost = true;
       
        if (host_toint.find(host) != host_toint.end())
        {
            unsigned int host_index=0;
            host_index=host_toint[host];
            if(host_index>0&&host_index<sitehostinfos.size())
            {
               return sitehostinfos[host_index].wwwpath;
            }
            else
            {
               return sitehostinfos[0].wwwpath;
            }
        }

        if (map_value.find(host) != map_value.end())
        {

            if (map_value[host]["wwwpath"].empty())
            {
                sitepath = map_value["default"]["wwwpath"];
                if (sitepath.empty())
                {
                    sitepath = "www/";
                }
                if (sitepath.back() != '/')
                {
                    sitepath.push_back('/');
                }
            }
            else
            {
                sitepath = map_value[host]["wwwpath"];
            }
            isnothost = false;
        }
        else if (map_value.find("default") != map_value.end())
        {
            sitepath = map_value["default"]["wwwpath"];
            if (sitepath.empty())
            {
                sitepath = "www/";
            }
            isnothost = false;
        }
        else
        {
            sitepath = "www/";
        }

        if (sitepath.back() != '/')
        {
            sitepath.push_back('/');
        }

        if (isnothost)
        {
            // notfind host path
            if (host.size() > 3 && host[0] != 'w' && host[1] != 'w' && host[2] != 'w' && host[3] != '.')
            {
                std::string dddmod = "*.";
                bool isplithost = true;
                std::string splithostpre;
                for (unsigned int i = 0; i < host.size(); i++)
                {
                    if (isplithost && host[i] == '.')
                    {
                        isplithost = false;
                        continue;
                    }
                    if (isplithost)
                    {
                        splithostpre.push_back(host[i]);
                    }
                    else
                    {
                        dddmod.push_back(host[i]);
                    }
                }
                if (map_value.find(dddmod) != map_value.end())
                {
                    if (map_value[dddmod]["aliaspath"].empty())
                    {
                        sitepath.append(dddmod.substr(2));
                        if (sitepath.back() != '/')
                        {
                            sitepath.push_back('/');
                        }
                        sitepath.append(splithostpre);
                    }
                    else
                    {
                        sitepath = map_value[dddmod]["aliaspath"];
                        if (sitepath.back() != '/')
                        {
                            sitepath.push_back('/');
                        }
                        sitepath.append(splithostpre);
                    }
                }
                else
                {
                    sitepath.append(host);
                }
            }
            else
            {
                sitepath.append(host);
            }
        }
        if (sitepath.back() != '/')
        {
            sitepath.push_back('/');
        }
        return sitepath;
    }
    void serverconfig::init_path()
    {
        if (configfile.empty())
        {
            std::string currentpath;
            fs::path cpath = fs::current_path();

            currentpath = cpath.string();
            currentpath = currentpath + "/conf";

            if (fs::is_directory(cpath))
            {
                currentpath = currentpath + "/server.conf";
                cpath = currentpath;
                if (fs::is_regular_file(cpath))
                {
                    configfile = currentpath;
                }
            }
            if (configfile.empty())
            {
                currentpath = "/usr/local/etc/paozhu";
                cpath = currentpath;
                if (fs::is_directory(cpath))
                {
                    currentpath = currentpath + "/server.conf";
                    cpath = currentpath;
                    if (fs::is_regular_file(cpath))
                    {
                        configfile = currentpath;
                    }
                    else
                    {
                        configfile.clear();
                    }
                }
            }
            if (configfile.empty())
            {
                currentpath = "/etc/paozhu";
                currentpath = currentpath + "/conf";
                cpath = currentpath;
                if (fs::is_directory(cpath))
                {

                    currentpath = currentpath + "/server.conf";
                    cpath = currentpath;
                    if (fs::is_regular_file(cpath))
                    {
                        configfile = currentpath;
                    }
                    else
                    {
                        configfile.clear();
                    }
                }
            }
        }
        else
        {
            fs::path cpath = configfile;

            if (fs::is_directory(cpath))
            {
                configfile = configfile + "/conf/server.conf";
                cpath = configfile;
                if (fs::is_regular_file(cpath))
                {
                }
                else
                {
                    configfile.clear();
                }
            }
            if (fs::is_directory(cpath))
            {
                configfile = configfile + "/server.conf";
                cpath = configfile;
                if (fs::is_regular_file(cpath))
                {
                }
                else
                {
                    configfile.clear();
                }
            }
        }
        if (configfile.size() > 0)
        {
            std::size_t found = configfile.find_last_of("/");
            configpath = configfile.substr(0, found);
            configpath.push_back('/');
        }
        server_loaclvar &static_server_var = get_server_global_var();
        static_server_var.config_path = configpath;
        loadserverglobalconfig();
    }
    bool serverconfig::loadserverglobalconfig()
    {
        if (configfile.empty())
        {
            return false;
        }
        map_value = loadserversconfig(configfile);

        std::string current_wwwpath = map_value["default"]["wwwpath"];
        fs::path cpath = current_wwwpath;
        if (!fs::is_directory(cpath))
        {
            cpath = fs::current_path();
            current_wwwpath = cpath.string();
            map_value["default"]["wwwpath"] = current_wwwpath + "/www/default/";
            map_value["default"]["temppath"] = current_wwwpath + "/temp/";
            map_value["default"]["logpath"] = current_wwwpath + "/log/";
            map_value["default"]["serverpath"] = current_wwwpath + "/";

            if (map_value["default"]["mainhost"].size() > 0)
            {
                std::string temphost = map_value["default"]["mainhost"];

                if (map_value.find(temphost) != map_value.end())
                {
                    if (map_value[temphost].find("wwwpath") != map_value[temphost].end())
                    {
                        current_wwwpath = map_value[temphost]["wwwpath"];
                        cpath = current_wwwpath;
                        bool is_has_wwwpath = true;
                        if (current_wwwpath.size() > 0)
                        {
                            if (fs::is_directory(cpath))
                            {
                                is_has_wwwpath = false;
                            }
                        }
                        if (is_has_wwwpath)
                        {
                            cpath = fs::current_path();
                            current_wwwpath = cpath.string();
                            map_value[temphost]["wwwpath"] = current_wwwpath + "/www/default/";
                        }
                    }
                }
            }
        }

        if (map_value["default"]["index"].empty())
        {
            map_value["default"]["index"] = "index.html";
        }
        if (map_value["default"]["usehtmlcache"].empty())
        {
            siteusehtmlchache = false;
        }
        else
        {
            siteusehtmlchache = true;
        }
        if (map_value["default"]["usehtmlcachetime"].empty())
        {
            siteusehtmlchachetime = 0;
        }
        else
        {
            siteusehtmlchachetime = 0;
            for (unsigned int i = 0; i < map_value["default"]["usehtmlcachetime"].size(); i++)
            {
                if (map_value["default"]["usehtmlcachetime"][i] > 0x2F && map_value["default"]["usehtmlcachetime"][i] < 0x3A)
                {
                    siteusehtmlchachetime = siteusehtmlchachetime * 10 + (map_value["default"]["usehtmlcachetime"][i] - '0');
                }
            }
        }
        if (map_value["default"]["http2_enable"].size() > 0 && map_value["default"]["http2_enable"][0] == '1')
        {
            isallnothttp2 = false;
        }
        else
        {
            isallnothttp2 = true;
        }
        server_loaclvar &static_server_var = get_server_global_var();
        static_server_var.http2_enable = isallnothttp2;

        if (map_value["default"]["controlsopath"].size() > 0)
        {
            static_server_var.control_so_path = map_value["default"]["controlsopath"];
        }
        if (map_value["default"]["viewsopath"].size() > 0)
        {
            static_server_var.view_so_path = map_value["default"]["viewsopath"];
        }
        if (map_value["default"]["debug_enable"].size() > 0 && map_value["default"]["debug_enable"][0] == '1')
        {
            static_server_var.debug_enable = true;
        }
        else
        {
            static_server_var.debug_enable = false;
        }
        if (map_value["default"]["deamon_enable"].size() > 0 && map_value["default"]["deamon_enable"][0] == '1')
        {
            static_server_var.deamon_enable = true;
        }
        else
        {
            static_server_var.deamon_enable = false;
        }

        if (map_value["default"]["show_visitinfo"].size() > 0 && map_value["default"]["show_visitinfo"][0] == '1')
        {
            static_server_var.show_visit_info = true;
        }
        else
        {
            static_server_var.show_visit_info = false;
        }

        if (map_value["default"]["static_file_compress_cache"].size() > 0 && map_value["default"]["static_file_compress_cache"][0] == '1')
        {
            static_server_var.static_file_compress_cache = true;
        }
        else
        {
            static_server_var.static_file_compress_cache = false;
        }

        if (map_value["default"]["session_type"].size() > 0)
        {
            switch (map_value["default"]["session_type"][0])
            {
            case '1':
                static_server_var.session_type = 1;
                break;
            default:
                static_server_var.session_type = 0;
            }
        }
        else
        {
            static_server_var.session_type = 0;
        }

        if (map_value["default"]["wwwpath"].size() == 0)
        {
            map_value["default"]["wwwpath"]="/usr/local/var/www/";
        }

        static_server_var.map_value = map_value;
        static_server_var.www_path = map_value["default"]["wwwpath"];
        
        static_server_var.temp_path = map_value["default"]["temppath"];
        static_server_var.log_path = map_value["default"]["logpath"];
        mainhost = map_value["default"]["mainhost"];
        if (mainhost.size() > 3 && mainhost[0] == 'w' && mainhost[1] == 'w' && mainhost[2] == 'w' && mainhost[3] == '.')
        {
            secondhost.clear();
            if (mainhost.size() > 4)
            {
                secondhost.append(&mainhost[4], mainhost.size() - 4);
            }
        }
        else
        {
            if (mainhost.empty())
            {
                mainhost = "localhost";
            }
            else
            {
                secondhost = "www.";
                secondhost.append(mainhost);
            }
        }
        if (static_server_var.www_path.size() > 0 && static_server_var.www_path.back() != '/')
        {
            static_server_var.www_path.push_back('/');
        }
        if (static_server_var.temp_path.size() > 0 && static_server_var.temp_path.back() != '/')
        {
            static_server_var.temp_path.push_back('/');
        }
        if (static_server_var.log_path.size() > 0 && static_server_var.log_path.back() != '/')
        {
            static_server_var.log_path.push_back('/');
        }
        struct site_host_info_t tempinfo;
        tempinfo.mainhost=mainhost;
        tempinfo.wwwpath=static_server_var.www_path;
        tempinfo.http2_enable=static_server_var.http2_enable;
        
        if (map_value["default"]["certificate_chain_file"].size() > 0)
        {
            tempinfo.certificate_file=map_value["default"]["certificate_chain_file"];
        }
        else
        {
            tempinfo.certificate_file.clear();;
        }

        if (map_value["default"]["private_key_file"].size() > 0)
        {
            tempinfo.privateKey_file=map_value["default"]["private_key_file"];
        }
        else
        {
            tempinfo.privateKey_file.clear();;
        }

        if (map_value["default"]["rewrite_404"].size() > 0)
        {
            tempinfo.isrewrite=true;
            try
            {
                tempinfo.rewrite404 = std::stoul(map_value["default"]["rewrite_404"].c_str());
            }
            catch (const std::exception &e)
            {
                tempinfo.rewrite404 = 0;
            }
        }
        else
        {
            tempinfo.isrewrite=false;
            tempinfo.rewrite404 = 0;
        }

        if (map_value["default"]["rewrite_404_action"].size() > 0)
        {
            tempinfo.rewrite_404_action=map_value["default"]["rewrite_404_action"];
        }
        else
        {
            tempinfo.rewrite_404_action.clear();
        }
        sitehostinfos.push_back(std::move(tempinfo));
        for(auto[first,second]:map_value)
        {
            if(first!="default")
            {
                 if(first.size()>0&&first[0]!='*')
                 {
                    struct site_host_info_t tempinfo;
                    tempinfo.mainhost=first;
                    for(auto[itemname,itemval]:second)
                    {
                        if(itemname=="http2_enable")
                        {
                            if(itemval.size()>0&&(itemval[0]=='1'||itemval[0]=='T'||itemval[0]=='t'))
                            {
                                tempinfo.http2_enable=true; 
                            }
                            else
                            {
                                tempinfo.http2_enable=false; 
                            }
                        }
                        else if(itemname=="wwwpath")
                        {
                            tempinfo.wwwpath=itemval;
                            if(tempinfo.wwwpath.size()==0)
                            {
                                tempinfo.wwwpath=static_server_var.www_path;
                            }
                            if (tempinfo.wwwpath.size() > 0 && tempinfo.wwwpath.back() != '/')
                            {
                                tempinfo.wwwpath.push_back('/');
                            }
                            
                        }
                        else if(itemname=="rewrite_404")
                        {
                            tempinfo.isrewrite=true;
                            try
                            {
                                tempinfo.rewrite404 = std::stoul(itemval.c_str());
                            }
                            catch (const std::exception &e)
                            {
                                tempinfo.rewrite404 = 0;
                            }
                        }
                        else if(itemname=="rewrite_404_action")
                        {
                            std::string tempac;
                            unsigned int m=0;
                            for(;m<itemval.size();m++)
                            {
                                if(itemval[m]=='|')
                                {
                                    break;
                                }
                                tempac.push_back(itemval[m]);
                            }
                            tempinfo.rewrite_404_action=tempac;
                            if(m<itemval.size()&&itemval[m]=='|')
                            {
                                m++;    
                            }
                            tempac.clear();
                            for(;m<itemval.size();m++)
                            {
                                if(itemval[m]=='|')
                                {
                                    if(tempac.size()>0)
                                    {
                                        tempinfo.action_404_lists.push_back(tempac);
                                    }
                                    tempac.clear();
                                    continue;
                                }
                                tempac.push_back(itemval[m]);
                            }
                            if(tempac.size()>0)
                            {
                                tempinfo.action_404_lists.push_back(tempac);
                            }
                        }
                        else if(itemname=="certificate_chain_file")
                        {
                            tempinfo.certificate_file=itemval;
                        }
                        else if(itemname=="private_key_file")
                        {
                            tempinfo.privateKey_file=itemval;
                        }
                    }
                    sitehostinfos.push_back(std::move(tempinfo));
                    if(sitehostinfos.size()>0)
                    {
                        unsigned int tempindex=sitehostinfos.size()-1;
                        host_toint[first]=tempindex;
                    }
                }   
            }
        }

        return true;
    }
    bool serverconfig::checkmaindomain(const char *servername)
    {
        if (mainhost.empty())
        {
            return false;
        }
        if (strcasecmp(servername, mainhost.c_str()) == 0)
        {
            return true;
        }
        if (strcasecmp(servername, secondhost.c_str()) == 0)
        {
            return true;
        }

        return false;
    }
    std::string serverconfig::ssl_chain_file()
    {
        return configpath + map_value["default"]["certificate_chain_file"];
    }
    std::string serverconfig::ssl_key_file()
    {
        return configpath + map_value["default"]["private_key_file"];
    }
    std::string serverconfig::ssl_dh_file()
    {
        return configpath + map_value["default"]["tmp_dh_file"];
    }

    unsigned char serverconfig::get_co_thread_num()
    {
        unsigned char tempnum = 0;

        for (unsigned int i = 0; i < map_value["default"]["cothreadnum"].size(); i++)
        {
            if (map_value["default"]["cothreadnum"][i] > 0x2F && map_value["default"]["cothreadnum"][i] < 0x3A)
            {
                tempnum = tempnum * 10 + (map_value["default"]["cothreadnum"][i] - '0');
            }
        }
        if (tempnum > 0)
        {
            return tempnum;
        }
        else
        {
            return 4;
        }
    }

    unsigned int serverconfig::get_ssl_port()
    {
        unsigned int tempnum = 0;

        for (unsigned int i = 0; i < map_value["default"]["httpsport"].size(); i++)
        {
            if (map_value["default"]["httpsport"][i] > 0x2F && map_value["default"]["httpsport"][i] < 0x3A)
            {
                tempnum = tempnum * 10 + (map_value["default"]["httpsport"][i] - '0');
            }
        }
        if (tempnum > 9)
        {
            return tempnum;
        }
        else
        {
            return 443;
        }
    }
    unsigned int serverconfig::get_port()
    {
        unsigned int tempnum = 0;

        for (std::size_t i = 0; i < map_value["default"]["httpport"].size(); i++)
        {
            if (map_value["default"]["httpport"][i] > 0x2F && map_value["default"]["httpport"][i] < 0x3A)
            {
                tempnum = tempnum * 10 + (map_value["default"]["httpport"][i] - '0');
            }
        }
        if (tempnum > 9)
        {
            return tempnum;
        }
        else
        {
            return 80;
        }
    }
    SSL_CTX *serverconfig::getdefaultctx()
    {
        SSL_CTX *ctx = NULL;
        for (auto i = g_ctxMap.begin(); i != g_ctxMap.end(); ++i)
        {
            ctx = i->second;
            break;
        }
        return ctx;
    }
    SSL_CTX *serverconfig::getctx(std::string filename)
    {
        SSL_CTX *ctx = NULL;
        std::string filepath = configpath;
        if (filepath.empty())
        {
            filepath = "conf/";
        }
        if (g_ctxMap.find(filename) != g_ctxMap.end())
        {
            ctx = g_ctxMap[filename];
        }
        else
        {
            g_ctxMap[filename] = SSL_CTX_new(SSLv23_server_method());
            ctx = g_ctxMap[filename];
            // filename.append(".pem");
            // boost::asio::error_code ec;
            std::string filepathurl;

            if (filepath.back() != '/')
            {
                filepath.push_back('/');
            }

            filepathurl.append(filepath);
            filepath.append(filename);
            filepath.append(".pem");
            if (filename[0] == 'w' && filename[1] == 'w' && filename[2] == 'w' && filename[3] == '.')
            {
                if (::SSL_CTX_use_certificate_file(ctx, filepath.c_str(), SSL_FILETYPE_PEM) != 1)
                {
                    // ec = boost::asio::error_code(
                    //     static_cast<int>(::ERR_get_error()),
                    //     boost::asio::error::get_ssl_category());
                    filepath = filepathurl;
                    filepath.append(&filename[4], filename.size() - 4);
                    filepath.append(".pem");
                    if (::SSL_CTX_use_certificate_file(ctx, filepath.c_str(), SSL_FILETYPE_PEM) != 1)
                    {
                        // return ctx;
                        // isdomainsave=false;
                    }
                }
            }
            else
            {

                if (::SSL_CTX_use_certificate_file(ctx, filepath.c_str(), SSL_FILETYPE_PEM) != 1)
                {
                    // ec = boost::asio::error_code(
                    //     static_cast<int>(::ERR_get_error()),
                    //     boost::asio::error::get_ssl_category());
                    filepathurl.append("www.");
                    filepathurl.append(filename);
                    filepath = filepathurl;
                    if (::SSL_CTX_use_certificate_file(ctx, filepath.c_str(), SSL_FILETYPE_PEM) != 1)
                    {
                    }
                }
            }
            filepath.erase(filepath.end() - 4, filepath.end());
            filepath.append(".key");
            if (::SSL_CTX_use_PrivateKey_file(ctx, filepath.c_str(), SSL_FILETYPE_PEM) != 1)
            {
                // ec = boost::asio::error_code(
                //     static_cast<int>(::ERR_get_error()),
                //     boost::asio::error::get_ssl_category());
            }

            if (!SSL_CTX_check_private_key(ctx))
            {
                //  ec = boost::asio::error_code(
                //     static_cast<int>(::ERR_get_error()),
                //     boost::asio::error::get_ssl_category());
            }
        }
        return ctx;
    }

}