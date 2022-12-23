/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file templateparse.hpp
 *  @date 2022-04-05
 *
 *  html template to cpp file
 *  Use the <%c and %> tags to include segment of C + + source code
 *
 */

#ifndef PROJECT_CONTROLPARSE_HPP
#define PROJECT_CONTROLPARSE_HPP

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
    class controllerparse
    {

    public:
        std::map<std::string, time_t> contrasttime(std::string tagetpath)
        {
            std::map<std::string, time_t> temp;
            for (auto iter = fileslist.begin(); iter != fileslist.end(); iter++)
            {
                fs::path paths = iter->first;
                std::string extfile = paths.extension().string();

                if (::strcasecmp(extfile.c_str(), ".cpp") == 0)
                {
                    std::string tempfile;
                    tempfile.append(tagetpath);
                    paths.replace_extension(".so");
                    tempfile.append(paths.string());
                    auto citer = sofileslist.find(tempfile);

                    if (citer != sofileslist.end())
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
                        //  std::cout << lead << "[+] \033[1m\033[35m" << filename.string() << "\033[0m\n";
                        DisplayDirTree(entry, level + 1, t);
                        // std::cout << "\n";
                    }
                    else if (fs::is_regular_file(entry.status()))
                    {
                        // std::cout << lead << " [-]\033[31m"<< filename.string() ;
                        auto timeEntry = fs::last_write_time(entry);

                        time_t systimeid = std::chrono::duration_cast<std::chrono::seconds>(timeEntry.time_since_epoch()).count();
                        if (t)
                        {
                            std::string ext = entry.path().extension().string();
                            if (ext == ".cpp")
                            {
                                fileslist[entry.path().string()] = systimeid;
                            }
                        }
                        else
                        {
                            std::string ext = entry.path().extension().string();
                            if (ext == ".so")
                            {
                                sofileslist[entry.path().string()] = systimeid;
                            }
                        }

                        //    tm* lsystemTime = localtime(&systimeid);
                        //    std::cout << "\033[0m\t time: \033[32m" << (lsystemTime->tm_year +1900)<< "-" << (lsystemTime->tm_mon+1) << "-" << lsystemTime->tm_mday << " " << lsystemTime->tm_hour << ":" << lsystemTime->tm_min << ":" << lsystemTime->tm_sec << std::endl;
                        //    std::cout <<"\033[0m";
                    }
                    // else
                    //     std::cout << lead << " [?]" << filename << "\n";
                }
            }
        }
        void parsetoso(std::string source)
        {
            std::string readcontent;

            FILE *fp = fopen(source.c_str(), "rb");
            if (fp)
            {

                fseek(fp, 0, SEEK_END);
                int n = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                readcontent.resize(2048);
                if (n < 2048)
                {

                    auto nread = fread(&readcontent[0], 1, n, fp);
                    readcontent.resize(nread);
                }
                else
                {
                    auto nread = fread(&readcontent[0], 1, 2048, fp);
                    readcontent.resize(nread);
                }
                fclose(fp);
            }
            else
            {
                return;
            }
            std::string sofilename, compiler, cliapp, compilercontent;
            sofilename = "--compiler--begin--";

            auto beginpos = readcontent.find(sofilename);
            auto endpos = readcontent.find("--compiler--end--");

            if (beginpos != std::string::npos)
            {
                if (endpos != std::string::npos && endpos > beginpos)
                {
                    compilercontent = readcontent.substr(beginpos + sofilename.size(), endpos - beginpos - sofilename.size());
                }
            }
            compiler.clear();

            compiler.resize(compilercontent.size());

            for (unsigned int i = 0; i < compilercontent.size(); i++)
            {
                if (compilercontent[i] == '\n')
                {
                    continue;
                }
                if (compilercontent[i] == '\r')
                {
                    continue;
                }
                cliapp.push_back(compilercontent[i]);
            }

            compilercontent.clear();
            // 检查有没有httpclient.h
            // check controller file is include httpclient.h in
            endpos = readcontent.find("\"httpclient.h\"");
            if (endpos != std::string::npos)
            {
                endpos -= 1;
                for (int n = endpos; n > 0; n--)
                {
                    if (readcontent[n] == 0x20)
                    {
                        endpos--;
                        continue;
                    }
                    break;
                }
                // edulcni
                if (readcontent[endpos] == 'e' && readcontent[endpos - 1] == 'd' && readcontent[endpos - 2] == 'u' && readcontent[endpos - 3] == 'l' && readcontent[endpos - 4] == 'c' && readcontent[endpos - 5] == 'n' && readcontent[endpos - 6] == 'i')
                {
                    compilercontent.append(" vendor/httpserver/src/httpclient.cpp ");
                }
            }
            readcontent.clear();
            fp = fopen("orm/orm.h", "rb");
            if (fp)
            {

                fseek(fp, 0, SEEK_END);
                int n = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                readcontent.resize(n);
                auto nread = fread(&readcontent[0], 1, n, fp);
                fclose(fp);
                readcontent.resize(nread);
            }
            if (readcontent.size() > 10)
            {
                //bool iszhushi = true;
                bool isblank = false;

                for (unsigned int ij = 0; ij < readcontent.size(); ij++)
                {
                    if (isblank == false)
                    {
                        if (readcontent[ij] == 0x20 || readcontent[ij] == '\t')
                        {
                        }
                        else
                        {
                            isblank = true;
                        }
                    }

                    if (isblank && readcontent[ij] == '#' && readcontent[ij + 1] == 'i' && readcontent[ij + 2] == 'n' && readcontent[ij + 3] == 'c' && readcontent[ij + 4] == 'l' && readcontent[ij + 5] == 'u' && readcontent[ij + 6] == 'd' && readcontent[ij + 7] == 'e')
                    {
                        ij += 8;
                        for (; ij < readcontent.size(); ij++)
                        {
                            if (readcontent[ij] == 0x20)
                            {
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }
                        if (readcontent[ij] == '"')
                        {
                            ij++;
                            std::string cppincludefilename;
                            for (; ij < readcontent.size(); ij++)
                            {
                                if (readcontent[ij] == '"')
                                {
                                    break;
                                }
                                cppincludefilename.push_back(readcontent[ij]);
                            }
                            ij++;
                            if (cppincludefilename.back() == 'h')
                            {
                                cppincludefilename[cppincludefilename.size() - 1] = 'c';
                                cppincludefilename.append("pp");

                                std::string first, second;
                                bool isappend = true;
                                unsigned int j = 0;
                                for (; j < cppincludefilename.size(); j++)
                                {
                                    if (cppincludefilename[j] == '/')
                                    {
                                        isappend = false;
                                        j++;
                                        break;
                                    }
                                    first.push_back(cppincludefilename[j]);
                                }
                                if (isappend)
                                {
                                    compilercontent.append(" models/");
                                    compilercontent.append(first);
                                    compilercontent.append(" ");
                                }
                                else
                                {
                                    for (; j < cppincludefilename.size(); j++)
                                    {
                                        if (cppincludefilename[j] == '/')
                                        {

                                            second.clear();
                                            continue;
                                        }
                                        second.push_back(cppincludefilename[j]);
                                    }
                                    if (second.size() > 0)
                                    {
                                        compilercontent.append(" models/");
                                        compilercontent.append(first);
                                        compilercontent.append("/");
                                        compilercontent.append(second);
                                        compilercontent.append(" ");
                                    }
                                }
                            }
                        }
                    }
                    if (readcontent[ij] == 0x0A)
                    {
                        isblank = false;
                    }
                }
            }

            compiler = "g++ ";
            readcontent.clear();
            fp = fopen("conf/gcc.conf", "rb");
            if (fp)
            {
                fseek(fp, 0, SEEK_END);
                int n = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                readcontent.resize(n);
                auto nread = fread(&readcontent[0], 1, n, fp);
                fclose(fp);
                readcontent.resize(nread);
                compiler.clear();

                for (unsigned int i = 0; i < readcontent.size(); i++)
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

            compiler.append(source);
            compiler.append(" -o ");

            sofilename = source.substr(0, source.find(".cpp"));
            sofilename.append(".so");

            compiler.append(" module/");
            compiler.append(sofilename);

            std::cout << " \033[32m[+]\033[0m \033[1m\033[31m" << source << "\033[0m ->\033[32m module/" << sofilename << "\033[0m " << std::endl;
            readcontent.clear();
            fp = fopen("conf/controlbuild.conf", "rb");
            if (fp)
            {

                fseek(fp, 0, SEEK_END);
                int n = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                readcontent.resize(n);
                auto nread = fread(&readcontent[0], 1, n, fp);
                fclose(fp);
                readcontent.resize(nread);
                sofilename.clear();
                unsigned int i = 0;
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
                        sofilename.push_back(0x20);
                        continue;
                    }
                    sofilename.push_back(readcontent[i]);
                }
                readcontent = sofilename;
            }

            compiler.append(" ");
            if (readcontent.size() > 20)
            {
                compiler.append(readcontent);
            }
            else
            {
                compiler.append(" -shared -fPIC -std=c++20 -Iinclude/httpserver src/httpserver/request.cpp src/httpserver/unicode.cpp  -lssl -lcrypto -ldl -lz -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lboost_filesystem -lmysqlcppconn8 -Imodels -Imodels/include ");
            }

            compiler.append(compilercontent);
            compiler.append(cliapp);
            std::cout << compiler << std::endl;
            int ir=system(compiler.c_str());
            if(ir==0)
            {
                
            }
        }

    public:
        std::map<std::string, time_t> fileslist;
        std::map<std::string, time_t> sofileslist;
    };
}
#endif // PROJECT_TEMPLATEPARSE_HPP