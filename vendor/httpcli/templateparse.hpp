/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file templateparse.hpp
 *  @date 2022-03-31
 *
 *  html template to cpp file
 *  Use the <%c and %> tags to include segment of C + + source code
 *
 */

#ifndef PROJECT_TEMPLATEPARSE_HPP
#define PROJECT_TEMPLATEPARSE_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <array>
#include <sys/time.h>
#include <sys/stat.h>
#include <map>
#include <strings.h>
#include <cstring>

namespace http
{
    namespace fs = std::filesystem;
    class viewtemplate
    {
    public:
        viewtemplate(std::string filename) : filename(std::move(filename))
        {
        }
        viewtemplate()
        {
        }
        std::map<std::string, time_t> contrasttime(std::string tagetpath)
        {
            std::map<std::string, time_t> temp;
            for (auto iter = fileslist.begin(); iter != fileslist.end(); iter++)
            {
                fs::path paths = iter->first;
                std::string extfile = paths.extension().string();

                if (strcasecmp(extfile.c_str(), ".html") == 0)
                {
                    std::string tempfile;
                    tempfile.append(tagetpath);
                    paths.replace_extension(".cpp");
                    tempfile.append(paths.string());
                    auto citer = cachefileslist.find(tempfile);

                    if (citer != cachefileslist.end())
                    {
                        time_t t = citer->second;

                        if (citer->second < iter->second)
                        {
                            temp[iter->first] = iter->second;
                        }
                    }
                    else
                    {
                        temp[iter->first] = iter->second;
                    }
                }
            }
            return temp;
        }
        void DisplayDirTree(const fs::path &pathToShow, int level, bool t)
        {
            if (fs::exists(pathToShow) && fs::is_directory(pathToShow))
            {
                auto lead = std::string(level * 3, ' ');
                for (const auto &entry : fs::directory_iterator(pathToShow))
                {
                    auto filename = entry.path().filename();
                    if (fs::is_directory(entry.status()))
                    {

                        DisplayDirTree(entry, level + 1, t);
                    }
                    else if (fs::is_regular_file(entry.status()))
                    {

                        auto timeEntry = fs::last_write_time(entry);

                        time_t systimeid = std::chrono::duration_cast<std::chrono::seconds>(timeEntry.time_since_epoch()).count();
                        if (t)
                        {
                            fileslist[entry.path().string()] = systimeid;
                        }
                        else
                        {
                            cachefileslist[entry.path().string()] = systimeid;
                        }
                    }
                }
            }
        }

        void reset()
        {
            filename.clear();
            savefilename.clear();
            viewcontent.clear();
            value.clear();
            parsefile.clear();
            custrominclude.clear();
            codein = true;

            fileslist.clear();
            cachefileslist.clear();
            compiler.clear();
            sofilename.clear();
        }
        bool parseto(std::string file, std::string dest)
        {
            filename = std::move(file);
            if (filename.empty())
            {
                return false;
            }

            doparsefile();
            savefilename = dest;
            templatefiletocpp();

            return true;
        }
        bool parse(std::string file)
        {
            filename = std::move(file);
            if (filename.empty())
            {
                return false;
            }

            doparsefile();

            return true;
        }
        bool parse()
        {
            if (filename.empty())
            {
                return false;
            }

            doparsefile();

            return true;
        }
        void doparsefile()
        {

            FILE *fp = fopen(filename.c_str(), "rb");
            if (fp)
            {
                fseek(fp, 0, SEEK_END);
                int n = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                viewcontent.resize(n);
                auto nread = fread(&viewcontent[0], 1, n, fp);
                fclose(fp);
            }
            else
            {
                return;
            }
            for (int i = 0; i < viewcontent.size(); i++)
            {

                if (codein && viewcontent[i] == '<' && viewcontent[i + 1] == '%' && viewcontent[i + 2] == 'c')
                {
                    if (viewcontent[i] == '+')
                    {
                        i++;
                    }
                    if (viewcontent[i] == '+')
                    {
                        i++;
                    }
                    parsefile.append("\r\n echo<<\"");
                    parsefile.append(value);
                    parsefile.append("\";\r\n");
                    codein = false;
                    i += 2;
                    value.clear();

                    continue;
                }
                if (viewcontent[i] == '%' && viewcontent[i + 1] == '>' && !codein)
                {
                    parsefile.append(value);
                    codein = true;
                    i += 1;
                    value.clear();
                    if (viewcontent[i + 1] == 0x0A)
                    {
                        i++;
                    }
                    continue;
                }
                if (codein)
                {
                    if (viewcontent[i] == '"')
                    {
                        value.push_back(0x5C);
                    }
                    if (viewcontent[i] == 0x5C)
                    {
                        value.push_back(0x5C);
                    }

                    if (viewcontent[i] == 0x0D)
                    {
                        value.push_back(0x5C);
                        value.push_back('r');
                    }
                    else if (viewcontent[i] == 0x0A)
                    {
                        value.push_back(0x5C);
                        value.push_back('n');
                    }
                    else if (viewcontent[i] == 0x09)
                    {
                        value.push_back(0x5C);
                        value.push_back('t');
                    }
                    else
                    {
                        value.push_back(viewcontent[i]);
                    }
                }
                else
                {
                    // include_sub("aa/bbb",obj);
                    if (viewcontent[i] == 'i' && viewcontent[i + 1] == 'n' && viewcontent[i + 2] == 'c')
                    {

                        if (viewcontent[i + 3] == 'l' && viewcontent[i + 4] == 'u' && viewcontent[i + 5] == 'd')
                        {

                            if (viewcontent[i + 6] == 'e' && viewcontent[i + 7] == '_' && viewcontent[i + 8] == 's')
                            {

                                if (viewcontent[i + 9] == 'u' && viewcontent[i + 10] == 'b' && viewcontent[i + 11] == '(')
                                {

                                    // echo<<render["view"]("show/after")(obj);
                                    int jj = i + 11;
                                    int nn = 00;

                                    if (viewcontent[jj] == '(')
                                    {
                                        jj++;
                                    }
                                    if (viewcontent[jj] == 0x20)
                                    {
                                        jj++;
                                    }
                                    if (viewcontent[jj] == 0x22)
                                    {
                                        jj++;
                                    }
                                    value.append(" try{ echo<<loadviewso(vinfo.viewsopath+\"");
                                    std::string tempmodelname;
                                    for (; jj < viewcontent.size(); jj++)
                                    {
                                        if (viewcontent[jj] == 0x22)
                                        {
                                            jj++;
                                            break;
                                        }
                                        nn++;
                                        if (nn > 72)
                                        {
                                            value.append("\")(vinfo,obj);  }catch (std::exception& e)   {          echo<< \"--- include_sub ");
                                            value.append(tempmodelname);
                                            value.append(" error ---\";  }");
                                            nn = 0;
                                            break;
                                        }
                                        value.push_back(viewcontent[jj]);
                                        tempmodelname.push_back(viewcontent[jj]);
                                    }
                                    if (nn > 0)
                                    {
                                        // value.append("\")(obj); ");
                                        value.append("\")(vinfo,");
                                        if (viewcontent[jj] == ',')
                                        {
                                            jj++;
                                        }

                                        for (; jj < viewcontent.size(); jj++)
                                        {
                                            if (viewcontent[jj] == 0x3B)
                                            {
                                                value.append(";");
                                                jj++;

                                                break;
                                            }

                                            value.push_back(viewcontent[jj]);
                                        }
                                        i = jj;
                                        value.append(" }catch (std::exception& e)   {          echo<< \"");
                                        value.append(tempmodelname);
                                        value.append(" error ---\";  }");
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                    else if (viewcontent[i] == '#' && viewcontent[i + 1] == 'i' && viewcontent[i + 2] == 'n' && viewcontent[i + 3] == 'c')
                    {

                        if (viewcontent[i + 4] == 'l' && viewcontent[i + 5] == 'u' && viewcontent[i + 6] == 'd' && viewcontent[i + 7] == 'e')
                        {
                            int jj = i + 8;
                            for (; jj < viewcontent.size(); jj++)
                            {
                                if (viewcontent[jj] != 0x20)
                                {
                                    break;
                                }
                            }
                            custrominclude.append("\n#include ");
                            for (; jj < viewcontent.size(); jj++)
                            {
                                if (viewcontent[jj] == 0x20)
                                {
                                    break;
                                }

                                custrominclude.push_back(viewcontent[jj]);
                            }
                            custrominclude.append("\n");
                            i = jj;
                            continue;
                        }
                    }

                    value.push_back(viewcontent[i]);
                }
            }
            if (value.size() > 0)
            {
                parsefile.append("\r\n echo<<\"");
                parsefile.append(value);
                parsefile.append("\";\r\n");
            }
        }

        void templatefiletocpp(std::string savefile = "")
        {

            if (savefilename.size() == 0)
            {
                if (savefile.size() == 0)
                {
                    return;
                }
                savefilename = savefile;
            }

            fs::path paths = savefilename;
            fs::path mulu = paths.remove_filename();
            fs::create_directories(mulu);
            fs::permissions(mulu,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);

            FILE *f = fopen(savefilename.c_str(), "wb");

            std::string headtxt = R"(#include<iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <boost/dll/alias.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>


#include <map> 
#include <vector>
#include <ctime>
#include <array>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include "request.h"
#include "datetime.h"
#include "cookie.h"
#include "urlcode.h"
#include "loadviewso.h"
#include "viewso_param.h"
#include "http_so_common_api.h"
)";
            fwrite(&headtxt[0], 1, headtxt.size(), f);

            if (custrominclude.size() > 0)
            {
                fwrite(&custrominclude[0], 1, custrominclude.size(), f);
            }
            value.clear();
            value.append("module/");
            value.append(filename);
            sofilename = value.substr(0, value.find(".html"));
            sofilename.append(".so");

            headtxt = "//g++ ";
            headtxt.append(savefilename);
            headtxt.append(" -o ");
            headtxt.append(sofilename);
            headtxt.append(" -shared -fPIC -std=c++20 -Ivendor/httpserver/include vendor/httpserver/src/request.cpp   -lssl -lcrypto -ldl -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lboost_filesystem\n");
            fwrite(&headtxt[0], 1, headtxt.size(), f);
            headtxt = R"(
  namespace http {
       
        std::string view(struct view_param vinfo,http::OBJ_VALUE &obj){
            std::ostringstream echo;

        )";
            std::string endtxt = R"(
            return echo.str();
        }

            BOOST_DLL_ALIAS(http::view, view)
           // BOOST_DLL_ALIAS(http::clientapi::setclientapi, _setclientapi)
        }
    )";
            fwrite(&headtxt[0], 1, headtxt.size(), f);
            fwrite(&parsefile[0], 1, parsefile.size(), f);
            fwrite(&endtxt[0], 1, endtxt.size(), f);
            fclose(f);

            fs::permissions(savefilename,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read | fs::perms::others_write,
                            fs::perm_options::add);

            paths = value;
            mulu = paths.remove_filename();
            fs::create_directories(mulu);
            fs::permissions(mulu,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);

            std::string readcontent, tempcontent;
            compiler = "g++ ";
            FILE *fp = fopen("conf/gcc.conf", "rb");
            if (fp)
            {
                fseek(fp, 0, SEEK_END);
                int n = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                readcontent.resize(n);
                auto nread = fread(&readcontent[0], 1, n, fp);
                fclose(fp);

                compiler.clear();

                for (int i = 0; i < readcontent.size(); i++)
                {
                    if (readcontent[i] == 0x0D)
                    {
                        break;
                    }
                    if (readcontent[i] == 0x0A)
                    {
                        break;
                    }
                    compiler.push_back(readcontent[i]);
                }
                compiler.push_back(0x20);
            }

            compiler.append(savefilename);
            compiler.append(" -o ");
            compiler.append(sofilename);

            readcontent.clear();
            fp = fopen("conf/viewbuild.conf", "rb");
            if (fp)
            {

                fseek(fp, 0, SEEK_END);
                int n = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                readcontent.resize(n);
                auto nread = fread(&readcontent[0], 1, n, fp);
                fclose(fp);
                tempcontent.clear();
                int i = 0;
                for (; i < readcontent.size(); i++)
                {
                    if (readcontent[i] == 0x20 || readcontent[i] == '\t')
                    {
                        continue;
                    }
                    break;
                }
                if (readcontent[i] == '#')
                {
                    for (; i < readcontent.size(); i++)
                    {
                        if (readcontent[i] == 0x0A)
                        {
                            break;
                        }
                    }
                }
                for (; i < readcontent.size(); i++)
                {
                    if (readcontent[i] == 0x20 || readcontent[i] == '\t')
                    {
                        continue;
                    }
                    break;
                }
                if (readcontent[i] == '#')
                {
                    for (; i < readcontent.size(); i++)
                    {
                        if (readcontent[i] == 0x0A)
                        {
                            break;
                        }
                        if (readcontent[i] == 0x0D)
                        {
                            break;
                        }
                    }
                }
                for (; i < readcontent.size(); i++)
                {
                    if (readcontent[i] == 0x0A)
                    {
                        continue;
                    }
                    if (readcontent[i] == 0x0D)
                    {
                        continue;
                    }
                    if (readcontent[i] == '\t')
                    {
                        tempcontent.push_back(0x20);
                        continue;
                    }
                    tempcontent.push_back(readcontent[i]);
                }
                readcontent = tempcontent;
            }

            compiler.append(" ");
            if (readcontent.size() > 20)
            {
                compiler.append(readcontent);
            }
            else
            {
                compiler.append(" -shared -fPIC -std=c++20 -Ivendor/httpserver/include vendor/httpserver/src/request.cpp   -lssl -lcrypto -ldl -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lboost_filesystem vendor/httpserver/src/httpsocommonapi.cpp -lmysqlcppconn8 ");
            }

            system(compiler.c_str());
        }

    public:
        std::map<std::string, time_t> fileslist;
        std::map<std::string, time_t> cachefileslist;
        std::string compiler;
        std::string sofilename;
        bool isso = true;

    private:
        std::string filename;
        std::string savefilename;
        std::string viewcontent;
        std::string value;
        std::string parsefile;
        std::string custrominclude;
        bool codein = true;
    };
}
#endif // PROJECT_TEMPLATEPARSE_HPP