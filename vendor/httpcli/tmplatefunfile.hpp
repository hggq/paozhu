
#pragma once
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>

#include "templateparsefile.hpp"
int viewfilecli()
{
    namespace fs = std::filesystem;
    http::viewtemplatefile tp;
    fs::path current_path = fs::current_path();

    std::cout << "\033[36m 🍄 current path:\033[0m \033[1m\033[35m" << current_path.string() << "\033[0m" << std::endl;
    std::string viewpath = "view/";
    std::string viewcache = "viewsrc/";
    tp.sofilename = "viewsrc/include/";
    fs::path vpath = viewpath;

    if (!fs::exists(vpath))
    {
        std::cout << " view directory not in current path " << std::endl;
    }

    fs::path paths ;

    paths = viewcache;
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }
    paths = tp.sofilename;
    if (!fs::exists(paths))
    {
        fs::create_directories(paths);
        fs::permissions(paths, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }
    tp.DisplayDirTree("view", 0, true);
    tp.DisplayDirTree("viewsrc", 0, false);
    std::map<std::string, time_t> mustfile = tp.contrasttime("viewsrc/");
    tp.savecpptoh_file();
    int i = 1;
    for (auto it = mustfile.begin(); it != mustfile.end(); it++)
    {
        std::cout << i << " \033[32m[+]\033[0m \033[1m\033[31m" << it->first << "\033[0m ";
        tm *lsystemTime = localtime(&it->second);
        std::cout << "\033[0m\t time: \033[32m" << (lsystemTime->tm_year + 1900) << "-" << (lsystemTime->tm_mon + 1) << "-" << lsystemTime->tm_mday << " " << lsystemTime->tm_hour << ":" << lsystemTime->tm_min << ":" << lsystemTime->tm_sec << "\033[0m" << std::endl;
        i++;
    }
    if (mustfile.size() > 0)
    {

        std::string commond;
        while (i > 0)
        {
            std::cout << " please input number to parse to cpp file.\r\n a update all , example: 1 3 4 5 enter key, q or x to exit,r reload " << std::endl;
            std::cout << " input number:";
            commond.clear();
            std::cin >> commond;

            if (commond == "x" || commond == "q")
            {
                break;
            }
            if (commond == "r" || commond == "R")
            {
                tp.DisplayDirTree("view", 0, true);
                tp.DisplayDirTree("viewsrc", 0, false);
                mustfile = tp.contrasttime("viewsrc/");
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

                        std::string tempfile = "viewsrc/";
                        tempfile.append(it->first);
                        fs::path tmpath = tempfile;
                        tmpath.replace_extension(".cpp");
                        tp.reset();
                        tp.parseto(it->first, tmpath.string());

                        std::cout << k << " \033[32m[+]\033[0m \033[1m\033[31m" << it->first << "\033[0m ->" << tmpath.string()  << std::endl;

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
                            std::string tempfile = "viewsrc/";
                            tempfile.append(it->first);
                            fs::path tmpath = tempfile;
                            tmpath.replace_extension(".cpp");
                            tp.reset();
                            tp.parseto(it->first, tmpath.string());

                            std::cout << k << " \033[32m[+]\033[0m \033[1m\033[31m" << it->first << "\033[0m ->" << tmpath.string() << std::endl;
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
        }
    }

    return 0;
}