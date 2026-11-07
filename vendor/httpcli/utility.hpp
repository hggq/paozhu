
#ifndef PROJECT_UTILITY_HPP
#define PROJECT_UTILITY_HPP
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

bool stringcasecmp(std::string_view str1, std::string_view str2)
{
    if (str1.size() != str2.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < str1.size(); i++)
    {
        if (str1[i] != str2[i])
        {
            if (str1[i] < 91 && str1[i] > 64)
            {
                if ((str1[i] + 32) == str2[i])
                {
                    continue;
                }
            }
            else if (str2[i] < 91 && str2[i] > 64)
            {
                if (str1[i] == (str2[i] + 32))
                {
                    continue;
                }
            }
            return false;
        }
    }
    return true;
}

// 生成物按 mtime 判脏（viewtemplate::contrasttime / viewtemplatefile::contrasttime），所以内容逐字节
// 相同就不重写，免得把依赖它的编译单元一起拖脏。但跳过重写时要把它的时间戳抬到源文件那一刻：
// 源文件被编辑器重新保存而内容未变时，产物会永远比源旧，每次运行都判成待生成、又什么都不写。
// 只抬不降——force 重新生成时源往往比产物更旧，降下去反而让下一次又判脏。
inline bool write_file_if_changed(const std::string &path, const std::string &content,
                                  const std::string &srcpath = "")
{
    std::string oldcontent;
    FILE *rf = fopen(path.c_str(), "rb");
    if (rf)
    {
        fseek(rf, 0, SEEK_END);
        long n = ftell(rf);
        fseek(rf, 0, SEEK_SET);
        if (n > 0)
        {
            oldcontent.resize(n);
            auto nread = fread(&oldcontent[0], 1, n, rf);
            oldcontent.resize(nread);
        }
        fclose(rf);
        if (oldcontent == content)
        {
            if (!srcpath.empty())
            {
                std::error_code ec;
                auto srctime = std::filesystem::last_write_time(srcpath, ec);
                if (!ec)
                {
                    auto desttime = std::filesystem::last_write_time(path, ec);
                    if (!ec && desttime < srctime)
                    {
                        std::filesystem::last_write_time(path, srctime, ec);
                    }
                }
            }
            return false;
        }
    }
    FILE *wf = fopen(path.c_str(), "wb");
    if (wf == nullptr)
    {
        std::cout << " [!] cannot write file: " << path << std::endl;
        return false;
    }
    if (!content.empty())
    {
        fwrite(&content[0], 1, content.size(), wf);
    }
    fclose(wf);
    return true;
}
#endif