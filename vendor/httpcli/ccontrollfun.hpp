
#pragma once

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <filesystem>
#include "templateparse.hpp"
#include "controllerparse.hpp"
namespace fs = std::filesystem;
// g++ testcontrollerparse.cpp -std=c++20 -Iinclude

int controlcli()
{

    http::controllerparse ctp;
    fs::path current_path = fs::current_path();

    std::cout << "\033[36m 🍄 current path:\033[0m \033[1m\033[35m" << current_path.string() << "\033[0m" << std::endl;
    std::string controlpath = "controller/";
    std::string controlrunpath = "module/controller/";
    fs::path vpath = controlpath;

    if (!fs::exists(vpath))
    {
        std::cout << " controller directory not in current path " << std::endl;
    }

    fs::path paths = controlrunpath;
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    ctp.DisplayDirTree(controlpath, 0, true);
    ctp.DisplayDirTree(controlrunpath, 0, false);

    std::map<std::string, time_t> mustfile = ctp.contrasttime("module/");
    int i = 1;
    // for(auto it=ctp.fileslist.begin();it!=ctp.fileslist.end();it++){
    //         std::cout <<i<<" \033[32m[+]\033[0m \033[1m\033[31m" << it->first << "\033[0m ";
    //         tm* lsystemTime = localtime(&it->second);
    //                    std::cout << "\033[0m\t time: \033[32m" << (lsystemTime->tm_year +1900)<< "-" << (lsystemTime->tm_mon+1) << "-" << lsystemTime->tm_mday << " " << lsystemTime->tm_hour << ":" << lsystemTime->tm_min << ":" << lsystemTime->tm_sec <<"\033[0m"<< std::endl;
    //          i++;
    // }

    i = 1;
    std::string compiler, sofilename;
    for (auto it = mustfile.begin(); it != mustfile.end(); it++)
    {
        std::cout << i << " \033[32m[+]\033[0m \033[1m\033[31m" << it->first << "\033[0m ";
        tm *lsystemTime = localtime(&it->second);
        std::cout << "\033[0m\t time: \033[32m" << (lsystemTime->tm_year + 1900) << "-" << (lsystemTime->tm_mon + 1) << "-" << lsystemTime->tm_mday << " " << lsystemTime->tm_hour << ":" << lsystemTime->tm_min << ":" << lsystemTime->tm_sec << "\033[0m" << std::endl;
        // ctp.parsetoso(it->first,"aaaa");
        i++;
    }

    std::string commond;
    if (mustfile.size())
    {

        while (i > 0)
        {

            std::cout << " please input number select cpp file.\r\n a update all , example: 1 3 4 5 enter key, q or x to exit,r reload " << std::endl;
            std::cout << " input number:";
            commond.clear();
            std::cin >> commond;

            if (commond == "x" || commond == "q")
            {
                break;
            }
            if (commond == "r" || commond == "R")
            {
                ctp.DisplayDirTree(controlpath, 0, true);
                ctp.DisplayDirTree(controlrunpath, 0, false);
                mustfile = ctp.contrasttime("module/");
                i = 1;
                for (auto it = mustfile.begin(); it != mustfile.end(); it++)
                {
                    std::cout << i << " \033[32m[+]\033[0m \033[1m\033[31m" << it->first << "\033[0m ";
                    tm *lsystemTime = localtime(&it->second);
                    std::cout << "\033[0m\t time: \033[32m" << (lsystemTime->tm_year + 1900) << "-" << (lsystemTime->tm_mon + 1) << "-" << lsystemTime->tm_mday << " " << lsystemTime->tm_hour << ":" << lsystemTime->tm_min << ":" << lsystemTime->tm_sec << "\033[0m" << std::endl;
                    i++;
                }
                if (i == 1)
                {
                    i = 0;
                }
                continue;
            }

            if (commond.size() > 0)
            {
                if (commond[0] == 'a' || commond[0] == 'A')
                {

                    int k = 1;
                    for (auto it = mustfile.begin(); it != mustfile.end(); it++)
                    {

                        ctp.parsetoso(it->first);

                        k++;
                    }

                    i = 0;
                    break;
                }
                char ss[10];
                std::list<int> saveu;
                int jj = 0;
                for (int nn = 0; nn < commond.size(); nn++)
                {
                    if (commond[nn] == 0x20)
                    {
                        jj = 0;
                        if (ss[0] == 0x00)
                        {
                            continue;
                        }

                        int offset = 0;
                        for (int m = 0; m < 4; m++)
                        {
                            if (ss[m] == 0x00)
                            {
                                break;
                            }
                            offset = offset * 10 + (ss[m] - '0');
                        }
                        if (offset > 0)
                        {

                            saveu.push_back(offset);
                        }
                        ss[0] = 0x00;
                        ss[1] = 0x00;
                        ss[2] = 0x00;
                        ss[3] = 0x00;
                        jj = 0;
                        continue;
                    }
                    if (commond[nn] >= '0' && commond[nn] <= '9')
                    {
                        ss[jj] = commond[nn];
                        jj++;
                        if (jj > 4)
                        {
                            jj = 0;
                        }
                    }
                }

                if (jj > 0)
                {
                    int offset = 0;
                    for (int m = 0; m < 4; m++)
                    {
                        if (ss[m] == 0x00)
                        {
                            break;
                        }

                        offset = offset * 10 + (ss[m] - '0');
                    }
                    if (offset > 0)
                    {

                        saveu.push_back(offset);
                    }
                }
                if (saveu.size() > 0)
                {
                    saveu.sort();
                    int k = 1;
                    for (auto it = mustfile.begin(); it != mustfile.end(); it++)
                    {

                        if (k == saveu.front())
                        {

                            saveu.pop_front();

                            ctp.parsetoso(it->first);

                            i -= 1;
                        }
                        k++;
                    }
                    if (i == 1)
                    {
                        i = 0;
                    }
                }
            }

            // endwhile
        }
    }
    return 0;
}