// parse_ini.cpp
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstdio>
#include "parse_ini.h"

namespace http
{
// ---------- 构造函数 ----------
parse_ini::parse_ini() : filename_() {}

parse_ini::parse_ini(const std::string &filename) : filename_(filename)
{
    parse_file(filename);
}

// ---------- 行去除空白 ----------
std::string parse_ini::line_trim(const std::string &s)
{
    auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char c)
                                  { return std::isspace(c); });
    auto end   = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c)
                                { return std::isspace(c); })
                   .base();
    return (start < end) ? std::string(start, end) : std::string();
}

// ---------- 解析文件 ----------
void parse_ini::parse_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    config.clear();
    std::string line, current_section;

    while (std::getline(file, line))
    {
        std::string trimmed = line_trim(line);
        // 跳过空行和注释（; 或 #）
        if (trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#')
        {
            continue;
        }

        // 节标题 [section]
        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            current_section = trimmed.substr(1, trimmed.size() - 2);
            if (current_section.empty())
            {
                continue;// 跳过空节名 []
            }
            // 确保该节存在
            if (config.find(current_section) == config.end())
            {
                config[current_section] = {};
            }
            continue;
        }

        // 键值对
        if (!current_section.empty())
        {
            auto eqPos = trimmed.find('=');
            if (eqPos != std::string::npos)
            {
                std::string key   = line_trim(trimmed.substr(0, eqPos));
                std::string value = line_trim(trimmed.substr(eqPos + 1));

                std::string value_temp;
                if (value.size() > 0 && value[0] == '"')
                {
                    unsigned int j = 1;
                    for (; j < value.size(); j++)
                    {
                        if (value[j] == '"')
                        {
                            // 统计前面的连续反斜杠数量，奇数表示转义引号
                            size_t backslash_count = 0;
                            size_t k               = j;
                            while (k > 0 && value[k - 1] == '\\')
                            {
                                backslash_count++;
                                k--;
                            }
                            if (backslash_count % 2 == 1)
                            {
                                value_temp.push_back(value[j]);
                                continue;// 转义引号，跳过
                            }
                            break;
                        }
                        value_temp.push_back(value[j]);
                    }
                }
                else
                {
                    unsigned int j = 0;
                    for (; j < value.size(); j++)
                    {
                        if (value[j] == '#' || value[j] == ';')
                        {
                            // 去除分割之前的空格
                            while (j > 0 && value[j - 1] == ' ')
                            {
                                j--;
                            }
                            break;
                        }
                    }

                    if (j > 0)
                    {
                        value_temp = value.substr(0, j);
                    }
                }

                if (!key.empty())
                {
                    config[current_section][key] = value_temp;
                }
            }
        }
        // 其他行（如没有节的信息）忽略
    }
    filename_ = filename;
}

// ---------- 获取值 ----------
std::string parse_ini::get_value(const std::string &section, const std::string &name, const std::string &default_value)
{
    auto itSec = config.find(section);
    if (itSec != config.end())
    {
        auto itKey = itSec->second.find(name);
        if (itKey != itSec->second.end())
        {
            return itKey->second;
        }
    }
    return default_value;
}

// ---------- 获取整个节 ----------
std::unordered_map<std::string, std::string> parse_ini::get_section(const std::string &section)
{
    auto itSec = config.find(section);
    if (itSec != config.end())
    {
        return itSec->second;
    }
    return {};
}

// ---------- 原子写入文件（临时文件 + rename，保留 .bak 备份） ----------
bool parse_ini::atomic_write_file(const std::string &filename, const std::vector<std::string> &lines)
{
    std::string tmp_file = filename + ".tmp";
    std::string bak_file = filename + ".bak";

    // 1. 写入临时文件
    std::ofstream outFile(tmp_file, std::ios::trunc);
    if (!outFile.is_open())
        return false;
    for (const auto &l : lines)
        outFile << l << '\n';
    outFile.flush();
    if (!outFile.good())
    {
        outFile.close();
        std::remove(tmp_file.c_str());
        return false;
    }
    outFile.close();

    // 2. 备份原文件（先删除旧备份，兼容 Windows rename 行为）
    std::remove(bak_file.c_str());
    std::rename(filename.c_str(), bak_file.c_str());
    // 原文件可能不存在，rename 失败可忽略

    // 3. 将临时文件重命名为目标文件
    if (std::rename(tmp_file.c_str(), filename.c_str()) != 0)
    {
        // 写入失败，从备份恢复原文件
        std::rename(bak_file.c_str(), filename.c_str());
        std::remove(tmp_file.c_str());
        return false;
    }

    return true;
}

// ---------- 修改文件中的某个值（保留注释和格式） ----------
bool parse_ini::save_value(const std::string &section, const std::string &name, const std::string &new_value)
{
    if (filename_.empty())
        return false;

    // 防止 INI 注入：值中不能包含换行符
    if (new_value.find('\n') != std::string::npos || new_value.find('\r') != std::string::npos)
        return false;

    // 1. 读取整个文件到内存
    std::ifstream inFile(filename_);
    if (!inFile.is_open())
        return false;

    std::vector<std::string> lines;
    std::string line_one;
    while (std::getline(inFile, line_one))
        lines.push_back(line_one);
    inFile.close();

    // 2. 遍历每一行，查找目标节和键
    std::string current_section;
    bool found = false;

    for (auto &line : lines)
    {
        std::string trimmed = line_trim(line);

        // 忽略空行和注释行（但保留它们，后面写回时不会丢失）
        if (trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#')
            continue;

        // 检测节标题
        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            current_section = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }

        // 在目标节内，且该行包含 '='
        if (current_section == section && trimmed.find('=') != std::string::npos)
        {
            auto eqPos      = trimmed.find('=');
            std::string key = line_trim(trimmed.substr(0, eqPos));
            if (key == name)
            {
                // 3. 定位原始行中第一个 '=' 的位置
                size_t origEq = line.find('=');
                if (origEq != std::string::npos)
                {
                    // 保留等号之前的所有字符（包括缩进和等号本身）
                    std::string prefix = line.substr(0, origEq + 1);// 包含 '='

                    // 提取等号后的原始内容（包括值、空格、引号、注释等）
                    std::string sub_str = line.substr(origEq + 1);

                    // 4. 构建新行：等号前部分 + 空格 + 新值
                    line = prefix + " " + new_value;

                    // 5. 试图从 sub_str 中提取行尾注释和引号结构，并追加到新行后面
                    size_t j = 0;

                    // 跳过等号后的空格和制表符
                    for (; j < sub_str.size(); j++)
                        if (sub_str[j] != '\t' && sub_str[j] != ' ')
                            break;

                    // 情况 A：值被双引号包裹（例如 value = "old"  # comment）
                    if (j < sub_str.size() && sub_str[j] == '"')
                    {
                        // 跳过开引号，从下一个字符开始查找匹配的闭合引号
                        j++;
                        for (; j < sub_str.size(); j++)
                        {
                            if (sub_str[j] == '"')
                            {
                                // 统计前面的连续反斜杠数量，奇数表示转义引号
                                size_t backslash_count = 0;
                                size_t k               = j;
                                while (k > 0 && sub_str[k - 1] == '\\')
                                {
                                    backslash_count++;
                                    k--;
                                }
                                if (backslash_count % 2 == 1)
                                    continue;// 转义引号，跳过
                                j++;         // 指向闭合引号的下一个字符
                                break;
                            }
                        }

                        // 继续移动，直到遇到注释符 # 或 ;（或字符串结尾）
                        size_t comment_start = std::string::npos;
                        for (; j < sub_str.size(); j++)
                        {
                            if (sub_str[j] == '#' || sub_str[j] == ';')
                            {
                                // 回退到注释符前的空格，保留格式
                                comment_start = j;
                                while (comment_start > 0 && sub_str[comment_start - 1] == ' ')
                                {
                                    comment_start--;
                                }
                                if(comment_start == j)
                                {
                                    line.push_back(' ');
                                }
                                break;
                            }
                        }

                        // 如果找到了注释，则追加到新行
                        if (comment_start != std::string::npos)
                            line.append(sub_str.substr(comment_start));
                    }
                    else
                    {
                        // 情况 B：值没有引号（直接值，可能带注释）
                        // j 已指向值的首字符，直接扫描注释符
                        // 跳过等号后的空格（已经跳过部分，但可能还有）
                        // 例子 aa = this comment # comment
                        for (; j < sub_str.size(); j++)
                        {
                            if (sub_str[j] == ' ')
                            {
                                break;
                            }
                        }

                        // 然后跳过连续空格，直到遇到第一个非空格字符
                        // 因为不使用 双引号 包裹值，直接确认有没有 # ;
                        for (; j < sub_str.size(); j++)
                        {
                            if (sub_str[j] != ' ')
                            {
                                break;
                            }
                        }

                        // 如果此时是注释符，直接追加
                        if (j < sub_str.size() && (sub_str[j] == '#' || sub_str[j] == ';'))
                        {
                            //回退一个空格
                            if (j > 0 && sub_str[j - 1] == ' ')
                            {
                                j--;
                            }
                            else
                            {
                                line.append(" ");
                            }
                            line.append(sub_str.substr(j));
                        }
                        else
                        {
                            // 否则，找到第一个注释符并追加（保留注释前的空格）
                            size_t comment_start = std::string::npos;
                            for (; j < sub_str.size(); j++)
                            {
                                if (sub_str[j] == '#' || sub_str[j] == ';')
                                {
                                    // 回退到注释符前的空格，保留格式
                                    comment_start = j;
                                    while (comment_start > 0 && sub_str[comment_start - 1] == ' ')
                                    {
                                        comment_start--;
                                    }
                                    if(comment_start == j)
                                    {
                                        line.push_back(' ');
                                    }
                                    break;
                                }
                            }

                            if (comment_start != std::string::npos)
                                line.append(sub_str.substr(comment_start));
                        }
                    }

                    found = true;// 标记已修改
                }
            }
        }
    }

    if (!found)
        return false;// 未找到键

    // 6. 原子写入：临时文件 + rename，保留 .bak 备份
    if (!atomic_write_file(filename_, lines))
        return false;

    // 7. 同步内存中的 config
    auto itSec = config.find(section);
    if (itSec != config.end())
        itSec->second[name] = new_value;
    else
        config[section][name] = new_value;

    return true;
}

// ---------- 添加或更新某个值到文件中 ----------
bool parse_ini::add_value(const std::string &section, const std::string &name, const std::string &new_value)
{
    if (filename_.empty())
        return false;

    // 防止 INI 注入：节名、键名、值中不能包含换行符
    if (section.find('\n') != std::string::npos || section.find('\r') != std::string::npos)
        return false;
    if (name.find('\n') != std::string::npos || name.find('\r') != std::string::npos)
        return false;
    if (new_value.find('\n') != std::string::npos || new_value.find('\r') != std::string::npos)
        return false;
    // 节名和键名不能为空，键名不能包含 '='
    if (section.empty() || name.empty() || name.find('=') != std::string::npos)
        return false;

    // 如果键已存在，直接更新
    auto itSec = config.find(section);
    if (itSec != config.end() && itSec->second.find(name) != itSec->second.end())
    {
        return save_value(section, name, new_value);
    }

    // 读取文件
    std::ifstream inFile(filename_);
    if (!inFile.is_open())
        return false;
    std::vector<std::string> lines;
    std::string line_one;
    while (std::getline(inFile, line_one))
        lines.push_back(line_one);
    inFile.close();

    // 查找目标节，并计算插入位置
    std::string current_section;
    bool section_found = false;
    size_t insert_pos  = lines.size();// 默认文件末尾

    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::string trimmed = line_trim(lines[i]);
        if (trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#')
            continue;

        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            current_section = trimmed.substr(1, trimmed.size() - 2);
            if (current_section == section)
            {
                section_found = true;
                // 从当前节之后开始找下一个节
                size_t next_sec = lines.size();
                for (size_t j = i + 1; j < lines.size(); ++j)
                {
                    std::string t = line_trim(lines[j]);
                    if (!t.empty() && t.front() == '[' && t.back() == ']')
                    {
                        next_sec = j;
                        break;
                    }
                }
                insert_pos = next_sec;// 插入到下一个节之前
                break;
            }
        }
    }

    std::string new_line = name + " = " + new_value;

    if (!section_found)
    {
        // 节不存在：在末尾追加空行 + [section] + 键值
        if (!lines.empty() && !lines.back().empty())
            lines.push_back("");
        lines.push_back("[" + section + "]");
        lines.push_back(new_line);
    }
    else
    {
        // 节存在：在 insert_pos 处插入新行
        lines.insert(lines.begin() + insert_pos, new_line);
    }

    // 原子写入：临时文件 + rename，保留 .bak 备份
    if (!atomic_write_file(filename_, lines))
        return false;

    // 更新内存
    config[section][name] = new_value;
    return true;
}

// ---------- 删除文件中的某个值（保留注释和格式） ----------
bool parse_ini::delete_value(const std::string &section, const std::string &name)
{
    if (filename_.empty())
        return false;

    // 1. 读取整个文件到内存
    std::ifstream inFile(filename_);
    if (!inFile.is_open())
        return false;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line))
        lines.push_back(line);
    inFile.close();

    // 2. 遍历行，找到目标节和键，记录需要删除的行索引
    std::string current_section;
    bool found          = false;
    size_t delete_index = 0;// 记录要删除的行索引

    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::string trimmed = line_trim(lines[i]);

        // 跳过空行和注释行（但只用于判断，不跳过这些行本身）
        if (trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#')
            continue;

        // 检测节标题
        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            current_section = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }

        // 在目标节内，且该行包含 '='
        if (current_section == section && trimmed.find('=') != std::string::npos)
        {
            auto eqPos      = trimmed.find('=');
            std::string key = line_trim(trimmed.substr(0, eqPos));
            if (key == name)
            {
                delete_index = i;// 记录要删除的行索引
                found        = true;
                break;// 找到后即可退出循环
            }
        }
    }

    if (!found)
        return false;// 未找到键

    // 3. 从 vector 中移除该行
    lines.erase(lines.begin() + delete_index);

    // 4. 原子写入：临时文件 + rename，保留 .bak 备份
    if (!atomic_write_file(filename_, lines))
        return false;

    // 5. 同步内存中的 config（删除该键）
    auto itSec = config.find(section);
    if (itSec != config.end())
    {
        itSec->second.erase(name);// 删除键
    }

    return true;
}

// ---------- 将内存中的配置保存到文件（无注释，纯格式） ----------
bool parse_ini::save_file(const std::string &filename)
{
    // 构建行列表
    std::vector<std::string> lines;
    for (const auto &secPair : config)
    {
        lines.push_back("[" + secPair.first + "]");
        for (const auto &kv : secPair.second)
        {
            lines.push_back(kv.first + " = " + kv.second);
        }
        lines.push_back("");// 节之间空行
    }

    // 原子写入：临时文件 + rename，保留 .bak 备份
    if (!atomic_write_file(filename, lines))
        return false;

    filename_ = filename;
    return true;
}
}// namespace http