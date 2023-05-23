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

#ifndef PROJECT_TEMPLATEPARSEFILE_HPP
#define PROJECT_TEMPLATEPARSEFILE_HPP

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
class viewtemplatefile
{
  public:
    viewtemplatefile(std::string filename) : filename(std::move(filename)) {}
    viewtemplatefile() {}
    std::map<std::string, time_t> contrasttime(std::string tagetpath)
    {
        std::map<std::string, time_t> temp;
        for (auto iter = fileslist.begin(); iter != fileslist.end(); iter++)
        {
            fs::path paths      = iter->first;
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

                    time_t systimeid =
                        std::chrono::duration_cast<std::chrono::seconds>(timeEntry.time_since_epoch()).count();
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
    void savecpptoh_file()
    {
        std::string includetemp;
        std::map<std::string, std::vector<std::string>> includelist;
        std::string regitem;
        std::string filename_methold, regtemp;
        std::string filename_namespace;
        for (auto iter = fileslist.begin(); iter != fileslist.end(); iter++)
        {
            fs::path paths       = iter->first;
            std::string extfile  = paths.extension().string();
            std::string filename = iter->first;
            if (strcasecmp(extfile.c_str(), ".html") == 0)
            {

                filename_methold.clear();
                filename_namespace.clear();

                int j = filename.size() - 1;
                for (; j > 0; j--)
                {
                    if (filename[j] == '.')
                    {
                        filename_methold.clear();
                        continue;
                    }
                    if (filename[j] == '/')
                    {
                        j--;
                        break;
                    }
                    filename_methold.push_back(filename[j]);
                }
                std::reverse(filename_methold.begin(), filename_methold.end());

                for (; j > 0; j--)
                {
                    if (filename[j] == '.')
                    {
                        continue;
                    }
                    if (filename[j] == '/')
                    {

                        break;
                    }
                    filename_namespace.push_back(filename[j]);
                }
                std::reverse(filename_namespace.begin(), filename_namespace.end());

                regtemp = filename_namespace;
                regtemp.append("/");
                regtemp.append(filename_methold);

                // filename_namespace.append("::");
                // filename_namespace.append(filename_methold);

                filename = "\n\tstd::string ";
                filename.append(filename_methold);
                filename.append("(const struct view_param &vinfo,http::OBJ_VALUE &obj);");
                // includetemp.append(filename);
                includelist[filename_namespace].push_back(filename);

                filename = "\n\t_viewmetholdreg.emplace(\"";
                filename.append(regtemp);
                filename.append("\",http::view::");

                filename_namespace.append("::");
                filename_namespace.append(filename_methold);

                filename.append(filename_namespace);
                filename.append(");");
                regitem.append(filename);
            }
        }

        std::string includefilename = sofilename + "viewsrc.h";
        std::string regfilefilename = sofilename + "regviewmethod.hpp";

        FILE *f = fopen(regfilefilename.c_str(), "wb");
        std::string content;

        content = R"(#ifndef __HTTP_REG_VIEW_METHOD_HPP
#define __HTTP_REG_VIEW_METHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include<string>
#include<map>
#include<functional>
#include "request.h"
#include "viewso_param.h"
#include "viewmethold_reg.h"
#include "viewsrc.h"

namespace http
{
  void _initview_method_regto(VIEW_REG  &_viewmetholdreg)
  {
            )";
        content.append("\t //create time: ");
        content.append(viewgetgmtdatetime(0));
        content.append("\n");
        content.append(regitem);
        content.append("\n");
        content.append("\n\t} \n}\n#endif");

        fwrite(&content[0], 1, content.size(), f);
        fclose(f);

        f = fopen(includefilename.c_str(), "wb");
        content.clear();
        content = "#pragma once\n#include<string>\n#include<map>\n#include<functional>\n#include "
                  "\"request.h\"\n#include \"viewso_param.h\"\n";
        content = R"(#ifndef __HTTP_VIEWSRC_ALL_METHOD_H
#define __HTTP_VIEWSRC_ALL_METHOD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include<string>
#include<map>
#include<functional>
#include "request.h"
#include "viewso_param.h"

)";
        content.append("namespace http { \n");
        content.append("namespace view { \n");
        for (auto [first, second] : includelist)
        {
            content.append("\nnamespace ");
            content.append(first);
            content.append("{ \n");

            for (unsigned int i = 0; i < second.size(); i++)
            {
                content.append(second[i]);
            }
            content.append("\n}\n");
        }

        content.append(includetemp);
        content.append("\n");
        content.append("\n}\n");
        content.append("\n}\n#endif");

        fwrite(&content[0], 1, content.size(), f);
        fclose(f);
    }
    std::string viewgetgmtdatetime(time_t inputtime = 0)
    {
        time_t curr_time;
        if (inputtime == 0)
        {
            curr_time = time((time_t *)NULL);
        }
        else
        {
            curr_time = inputtime;
        }
        tm *timeInfo;

        char timestr[30] = {'\0'};
        timeInfo         = gmtime(&curr_time);
        strftime(timestr, sizeof(timestr), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

        std::string temp(timestr);
        return temp;
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
        doparse_hcppfile();
        savefilename = dest;
        templatefiletoframe_hcpp();
        return true;
    }
    bool parse(std::string file)
    {
        filename = std::move(file);
        if (filename.empty())
        {
            return false;
        }
        doparse_hcppfile();

        return true;
    }
    bool parse()
    {
        if (filename.empty())
        {
            return false;
        }

        doparse_hcppfile();

        return true;
    }

    void doparse_hcppfile()
    {

        FILE *fp = fopen(filename.c_str(), "rb");
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            int n = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            viewcontent.resize(n);
            auto nread = fread(&viewcontent[0], 1, n, fp);
            viewcontent.resize(nread);
            fclose(fp);
        }
        else
        {
            return;
        }

        std::string filename_methold;
        std::string filename_namespace;

        int j = filename.size() - 1;
        for (; j > 0; j--)
        {
            if (filename[j] == '.')
            {
                filename_methold.clear();
                continue;
            }
            if (filename[j] == '/')
            {
                j--;
                break;
            }
            filename_methold.push_back(filename[j]);
        }
        std::reverse(filename_methold.begin(), filename_methold.end());

        for (; j > 0; j--)
        {
            if (filename[j] == '.')
            {
                continue;
            }
            if (filename[j] == '/')
            {

                break;
            }
            filename_namespace.push_back(filename[j]);
        }
        std::reverse(filename_namespace.begin(), filename_namespace.end());
        filename_namespace.append("::");
        filename_namespace.append(filename_methold);
        for (unsigned int i = 0; i < viewcontent.size(); i++)
        {

            if (codein && viewcontent[i] == '<' && viewcontent[i + 1] == '%' && viewcontent[i + 2] == 'c')
            {
                i += 3;
                if (viewcontent[i] == '+')
                {
                    i++;
                }
                if (viewcontent[i] == '+')
                {
                    i++;
                }
                parsefile.append("\r\n \t\t\t echo<<\"");
                parsefile.append(value);
                parsefile.append("\";\r\n");
                codein = false;
                i -= 1;
                value.clear();

                continue;
            }
            else if (codein && viewcontent[i] == '<' && viewcontent[i + 1] == '%' && viewcontent[i + 2] == ' ')
            {
                i += 3;
                if (viewcontent[i] == 'c' && viewcontent[i + 1] == ' ')
                {
                    i += 2;
                }

                parsefile.append("\r\n \t\t\t echo<<\"");
                parsefile.append(value);
                parsefile.append("\";\r\n");
                codein = false;
                i -= 1;
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
                                unsigned int jj = i + 11;
                                int nn          = 00;

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
                                value.append(" \t\t\ttry{ echo<<http::view::");
                                // value.append(filename_namespace);
                                // value.append("(vinfo.viewsopath+\"");

                                std::string tempmodelname;
                                for (; jj < viewcontent.size(); jj++)
                                {
                                    if (viewcontent[jj] == 0x22)
                                    {
                                        jj++;
                                        break;
                                    }
                                    if (viewcontent[jj] == '/')
                                    {
                                        value.append("::");
                                        tempmodelname.push_back(viewcontent[jj]);
                                        continue;
                                    }
                                    nn++;
                                    if (nn > 72)
                                    {
                                        value.append("(vinfo,obj);  }catch (std::exception& e)   {          echo<< "
                                                     "\"--- include_sub ");
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
                                    value.append("(vinfo,");
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
                                    value.append(" }catch (std::exception& e)   {          echo<< \"--- include_sub ");
                                    value.append(tempmodelname);
                                    value.append(" error ---\";  }");
                                    continue;
                                }
                            }
                        }
                    }
                }
                else if (viewcontent[i] == '#' && viewcontent[i + 1] == 'i' && viewcontent[i + 2] == 'n' &&
                         viewcontent[i + 3] == 'c')
                {

                    if (viewcontent[i + 4] == 'l' && viewcontent[i + 5] == 'u' && viewcontent[i + 6] == 'd' &&
                        viewcontent[i + 7] == 'e')
                    {
                        unsigned int jj = i + 8;
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
            parsefile.append("\r\n \t\t\t echo<<\"");
            parsefile.append(value);
            parsefile.append("\";\r\n");
        }
    }

    void templatefiletoframe_hcpp(std::string savefile = "")
    {

        if (savefilename.size() == 0)
        {
            if (savefile.size() == 0)
            {
                return;
            }
            savefilename = savefile;
        }

        std::string filename_methold;
        std::string filename_namespace;

        int j = savefilename.size() - 1;
        for (; j > 0; j--)
        {
            if (savefilename[j] == '.')
            {
                filename_methold.clear();
                continue;
            }
            if (savefilename[j] == '/')
            {
                j--;
                break;
            }
            filename_methold.push_back(savefilename[j]);
        }

        std::reverse(filename_methold.begin(), filename_methold.end());
        for (; j > 0; j--)
        {
            if (savefilename[j] == '.')
            {
                continue;
            }
            if (savefilename[j] == '/')
            {

                break;
            }
            filename_namespace.push_back(savefilename[j]);
        }

        std::reverse(filename_namespace.begin(), filename_namespace.end());
        fs::path paths = savefilename;
        fs::path mulu  = paths.remove_filename();
        fs::create_directories(mulu);
        fs::permissions(mulu,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);

        FILE *f = fopen(savefilename.c_str(), "wb");

        std::string headtxt = R"(#include<iostream>
#include <cstdio>
#include <string>
#include <sstream>
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
#include "viewsrc.h"
)";
        fwrite(&headtxt[0], 1, headtxt.size(), f);

        if (custrominclude.size() > 0)
        {
            fwrite(&custrominclude[0], 1, custrominclude.size(), f);
        }

        headtxt = "//g++ ";
        headtxt.append(savefilename);

        headtxt.append("\nnamespace http {\n");
        headtxt.append("\nnamespace view {\n");
        headtxt.append("\tnamespace ");
        headtxt.append(filename_namespace);
        headtxt.append("{\n \t\t std::string ");
        headtxt.append(filename_methold);
        headtxt.append("(const struct view_param &vinfo,http::OBJ_VALUE &obj)\n\t\t\t{\n ");

        fwrite(&headtxt[0], 1, headtxt.size(), f);

        headtxt            = R"(
                     std::ostringstream echo;

        )";
        std::string endtxt = R"(
                  return echo.str();
             }

         }
        }
       }
    )";
        fwrite(&headtxt[0], 1, headtxt.size(), f);
        fwrite(&parsefile[0], 1, parsefile.size(), f);
        fwrite(&endtxt[0], 1, endtxt.size(), f);
        fclose(f);

        fs::permissions(savefilename,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read | fs::perms::others_write,
                        fs::perm_options::add);
    }

  public:
    std::map<std::string, time_t> fileslist;
    std::map<std::string, time_t> cachefileslist;
    std::string compiler;
    std::string sofilename;

  private:
    std::string filename;
    std::string savefilename;
    std::string viewcontent;
    std::string value;
    std::string parsefile;
    std::string custrominclude;
    bool codein = true;
};
} //namespace http
#endif // PROJECT_TEMPLATEPARSE_HPP