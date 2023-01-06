/**
 *  @copyright 2023, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file autopickcontrolmethod.hpp
 *  @date 2023-01-05
 *
 *  controller cpp file auto pick method to reg file
 *
 *
 */

#ifndef PROJECT_AUTOPICKCONTROLMETHOD_HPP
#define PROJECT_AUTOPICKCONTROLMETHOD_HPP

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
#include <vector>
#include <cstring>
namespace http
{
    namespace fs = std::filesystem;
    struct reg_autoitem
    {
        std::string pre;
        std::string func;
        std::string urlpath;
    };
    class pickcontrol
    {
    public:
        std::vector<std::string> listpath(const std::string &methodpath)
        {
            fs::path tagetpath = methodpath;
            std::vector<std::string> temp;
            if (fs::exists(tagetpath) && fs::is_directory(tagetpath))
            {

                for (const auto &entry : fs::directory_iterator(tagetpath))
                {
                    auto filename = entry.path().filename();
                    if (fs::is_regular_file(entry.status()))
                    {
                        std::string ext = entry.path().extension().string();
                        if (ext == ".cpp")
                        {
                            ext.clear();
                            if (methodpath.back() == '/')
                            {

                                ext.append(methodpath);
                                ext.append(filename);
                            }
                            else
                            {
                                ext.append(methodpath);
                                ext.append("/");
                                ext.append(filename);
                            }

                            temp.emplace_back(ext);
                        }
                    }
                }
            }

            return temp;
        }
        std::vector<struct reg_autoitem> pickfile(const std::string &methodpathfile)
        {

            std::vector<struct reg_autoitem> temp;

            std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(std::fopen(methodpathfile.c_str(), "rb"), &std::fclose);

            if (!fp.get())
            {
                return temp;
            }

            std::string filecontent;

            fseek(fp.get(), 0, SEEK_END);
            unsigned long long file_size = ftell(fp.get());
            fseek(fp.get(), 0, SEEK_SET);

            filecontent.resize(file_size);
            file_size = fread(&filecontent[0], 1, file_size, fp.get());
            filecontent.resize(file_size);

            bool isbegin = true;
            for (unsigned int i = 0; i < filecontent.size(); i++)
            {
                if (isbegin == false)
                {
                    if (filecontent[i] == 0x0A)
                    {
                        isbegin = true;
                    }
                    continue;
                }

                if (filecontent[i] > 0x2F)
                {
                    isbegin = false;
                    continue;
                }

                if (filecontent[i] == '/' && filecontent[i + 1] == '/' && filecontent[i + 2] == '@' && filecontent[i + 3] == 'u' && filecontent[i + 4] == 'r' && filecontent[i + 5] == 'l' && filecontent[i + 6] == 'p')
                {
                    if (filecontent[i + 7] == 'a' && filecontent[i + 8] == 't' && filecontent[i + 9] == 'h' && filecontent[i + 10] == '(')
                    {
                        std::string prename;
                        std::string tempname;
                        unsigned int j = i + 11;
                        for (; j < filecontent.size(); j++)
                        {
                            if (filecontent[j] == ')' || filecontent[j] == 0x0A)
                            {
                                // j++;
                                break;
                            }
                            if (filecontent[j] == ',')
                            {
                                prename = tempname;
                                tempname.clear();
                                continue;
                            }
                            if (filecontent[j] == '"')
                            {
                                continue;
                            }
                            if (filecontent[j] == '\t')
                            {
                                continue;
                            }
                            if (filecontent[j] == 0x20)
                            {
                                continue;
                            }
                            tempname.push_back(filecontent[j]);
                        }

                        i = j;
                        std::string linestr;
                        if(tempname.empty())
                        {
                            isbegin = false;
                            continue;
                        }
                        for (; j < filecontent.size(); j++)
                        {
                            if (filecontent[j] == 0x0A)
                            {
                                j++;
                                break;
                            }
                        }
                        for (; j < filecontent.size(); j++)
                        {
                            if (filecontent[j] == 0x0A || filecontent[j] == '(')
                            {
                                // j++;
                                break;
                            }

                            if (filecontent[j] == 0x20 || filecontent[j] == '\t')
                            {
                                linestr.clear();
                                continue;
                            }
                            linestr.push_back(filecontent[j]);
                        }
                        if (linestr.size() < 2)
                        {
                            for (; j < filecontent.size(); j++)
                            {
                                if (filecontent[j] == 0x0A)
                                {
                                    j++;
                                    break;
                                }
                            }
                            for (; j < filecontent.size(); j++)
                            {
                                if (filecontent[j] == 0x0A || filecontent[j] == '(')
                                {
                                    // j++;
                                    break;
                                }

                                if (filecontent[j] == 0x20 || filecontent[j] == '\t')
                                {
                                    linestr.clear();
                                    continue;
                                }
                                linestr.push_back(filecontent[j]);
                            }
                        }
                        i = j;
                        isbegin = false;
                        struct reg_autoitem reg_temp;
                        reg_temp.pre = prename;
                        reg_temp.func = linestr;
                        reg_temp.urlpath = tempname;
                        temp.emplace_back(reg_temp);
                        continue;
                    }
                }
            }

            return temp;
        }
        void createhfile(const std::string &filename, const std::vector<struct reg_autoitem> &methodpathfile)
        {
            std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(std::fopen(filename.c_str(), "wb"), &std::fclose);

            if (!fp.get())
            {
                return;
            }

            std::string header_content = R"(
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
)";
            for (unsigned int i = 0; i < methodpathfile.size(); i++)
            {
                header_content.append("\tstd::string ");
                header_content.append(methodpathfile[i].func);
                header_content.append("(std::shared_ptr<httppeer> peer);\r\n");
            }
            header_content.append("}\r\n");

            fwrite(&header_content[0], 1, header_content.size(), fp.get());
        }
        // public:
        //     std::map<std::string, time_t> fileslist;
        //     std::map<std::string, time_t> sofileslist;
    };
}
#endif // PROJECT_AUTOPICKMEMTHOD_HPP