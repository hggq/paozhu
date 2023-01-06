/*
 *  @author 黄自权 huangziquan
 *  @date 2023-01-06
 *  @dest controller method pick file
 */
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <filesystem>
#include "autopickcontrolmethod.hpp"

namespace fs = std::filesystem;
// macos  g++ vendor/httpcli/autopickmethod.cpp -o clihttp -std=c++20 -Ivendor/httpserver/include -Ivendor/httpcli/include
// ubuntu20.04  g++ vendor/httpcli/autopickmethod.cpp -o clihttp -std=c++20 -Ivendor/httpserver/include -Ivendor/httpcli/include

std::string get_filename(const std::string &filename)
{
    std::string filename_name;
    int j = filename.size() - 1;
    if (j == -1)
        return "";
    for (; j >= 0; j--)
    {
        if (filename[j] == '.')
        {
            if (filename_name.size() == 2 && filename_name[0] == 'z' && filename_name[1] == 'g')
            {
                continue;
            }
            filename_name.clear();
            continue;
        }
        if (filename[j] == '/')
        {
            j--;
            break;
        }
        filename_name.push_back(filename[j]);
    }
    std::reverse(filename_name.begin(), filename_name.end());
    return filename_name;
}

int main(int argc, char *argv[])
{

    std::string current_run_path = "";
    std::string current_method_file = "";
    fs::path current_path;
    if (argc > 0)
    {
        current_run_path.append(argv[1]);
    }
    else
    {
        current_path = fs::current_path();
        current_run_path.append(current_path.string());
    }

    if (current_run_path.back() != '/')
    {
        current_run_path.push_back('/');
    }

    http::pickcontrol lp;
    std::vector<std::string> header_lists;
    std::vector<struct http::reg_autoitem> reg_method_lists;
    auto plist = lp.listpath(current_run_path + "controller/src");

    for (unsigned int i = 0; i < plist.size(); i++)
    {
        std::string filename = get_filename(plist[i]);
        auto method_item = lp.pickfile(plist[i]);

        if (method_item.size() > 0)
        {

            std::string fileinh = current_run_path + "controller/include/" + filename + ".h";

            lp.createhfile(fileinh, method_item);

            for (unsigned int j = 0; j < method_item.size(); j++)
            {
                reg_method_lists.emplace_back(method_item[j]);
            }
            filename.append(".h\"\r\n");
            header_lists.emplace_back(filename);
        }
    }
    current_method_file = current_run_path + "common/autocontrolmethod.hpp";

    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(std::fopen(current_method_file.c_str(), "wb"), &std::fclose);

    if (!fp.get())
    {
        return 0;
    }

    std::string automethod_content;

    automethod_content = R"(
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

)";

    for (unsigned int j = 0; j < header_lists.size(); j++)
    {
        automethod_content.append("#include \"");
        automethod_content.append(header_lists[j]);
    }

    automethod_content.append(R"(

namespace http
{
  void _initauto_control_httpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
  {
    struct regmethold_t temp;

)");

    for (unsigned int j = 0; j < reg_method_lists.size(); j++)
    {
        if (strcasecmp(reg_method_lists[j].pre.c_str(), "null") == 0 || strcasecmp(reg_method_lists[j].pre.c_str(), "nullptr") == 0)
        {
            automethod_content.append("\t\ttemp.pre = nullptr;\r\n");
        }
        else
        {
            automethod_content.append("\t\ttemp.pre = ");
            automethod_content.append(reg_method_lists[j].pre);
            automethod_content.append(";\r\n");
        }
        automethod_content.append("\t\ttemp.regfun = ");
        automethod_content.append(reg_method_lists[j].func);
        automethod_content.append(";\r\n");

        automethod_content.append("\t\tmethodcallback.emplace(\"");
        automethod_content.append(reg_method_lists[j].urlpath);
        automethod_content.append("\",temp);\r\n");
    }

    automethod_content.append(R"(

    }
}

#endif

    )");
    fwrite(&automethod_content[0], 1, automethod_content.size(), fp.get());

    return 0;
}