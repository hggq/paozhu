// parse_ini.cpp
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstdio>
#include <filesystem>
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
        // 跳过空行
        if (trimmed.empty())
        {
            continue;
        }
        // 注释行：存储到当前 section 的 comment 字段
        if (trimmed.front() == ';' || trimmed.front() == '#')
        {
            if (!current_section.empty())
            {
                std::string comment_text = trimmed.substr(1);
                auto start               = std::find_if_not(comment_text.begin(), comment_text.end(), [](unsigned char c)
                                              { return std::isspace(c); });
                if (start != comment_text.end())
                    comment_text = std::string(start, comment_text.end());
                else
                    comment_text.clear();
                config[current_section].push_back(ini_key_value_t{"", "", comment_text});
            }
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
                std::string comment_temp;

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
                    // 跳过闭合引号，继续扫描注释
                    ++j;
                    for (; j < value.size(); j++)
                    {
                        if (value[j] == '#' || value[j] == ';')
                        {
                            comment_temp = value.substr(j);
                            break;
                        }
                    }
                }
                else
                {
                    // 查找注释分隔符位置
                    size_t comment_pos = std::string::npos;
                    for (size_t k = 0; k < value.size(); k++)
                    {
                        if (value[k] == '#' || value[k] == ';')
                        {
                            comment_pos = k;
                            break;
                        }
                    }

                    if (comment_pos != std::string::npos)
                    {
                        // 去除注释前的空格作为值
                        size_t end_pos = comment_pos;
                        while (end_pos > 0 && value[end_pos - 1] == ' ')
                            end_pos--;
                        value_temp = value.substr(0, end_pos);

                        // 提取注释（包含 ; 或 # 前缀）
                        comment_temp = value.substr(comment_pos);
                    }
                    else
                    {
                        value_temp = value;
                        // 去除尾部空格
                        while (!value_temp.empty() && value_temp.back() == ' ')
                            value_temp.pop_back();
                    }
                }

                if (!key.empty())
                {
                    bool found = false;
                    for (auto &item : config[current_section].data)
                    {
                        if (item.name == key)
                        {
                            item.value   = value_temp;
                            item.comment = comment_temp;
                            found        = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        config[current_section].push_back(ini_key_value_t{key, value_temp, comment_temp});
                    }
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
        auto [pVal, found] = itSec->second.try_find(name);
        if (found)
        {
            return *pVal;
        }
    }
    return default_value;
}

// ---------- 获取整个节 ----------
ini_item_t parse_ini::get_section(const std::string &section)
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

    std::filesystem::permissions(tmp_file,
                                 std::filesystem::perms::owner_all |
                                     std::filesystem::perms::group_read | std::filesystem::perms::group_write |
                                     std::filesystem::perms::others_read,
                                 std::filesystem::perm_options::replace);

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
bool parse_ini::save_value(const std::string &section, const std::string &name, const std::string &new_value, const std::string &comment)
{
    if (filename_.empty())
        return false;

    // 防止 INI 注入：值和注释中不能包含换行符
    if (new_value.find('\n') != std::string::npos || new_value.find('\r') != std::string::npos)
        return false;
    if (comment.find('\n') != std::string::npos || comment.find('\r') != std::string::npos)
        return false;

    // 规范化注释：空字符串表示不修改注释；否则确保以 ; 或 # 开头
    std::string normalized_comment;
    if (!comment.empty())
    {
        if (comment[0] == ';' || comment[0] == '#')
            normalized_comment = comment;
        else
            normalized_comment = "; " + comment;
    }

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
    std::string old_comment;// 保存原文件中的注释

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

                    // 提取等号后的原始内容
                    std::string sub_str = line.substr(origEq + 1);

                    // 4. 构建新行：等号前部分 + 空格 + 新值
                    line = prefix + " " + new_value;

                    if (!normalized_comment.empty())
                    {
                        // 用新注释替换原注释
                        line += " " + normalized_comment;
                    }
                    else
                    {
                        // 保留原注释的逻辑
                        size_t j = 0;

                        // 跳过等号后的空格和制表符
                        for (; j < sub_str.size(); j++)
                            if (sub_str[j] != '\t' && sub_str[j] != ' ')
                                break;

                        // 情况 A：值被双引号包裹
                        if (j < sub_str.size() && sub_str[j] == '"')
                        {
                            j++;
                            for (; j < sub_str.size(); j++)
                            {
                                if (sub_str[j] == '"')
                                {
                                    size_t backslash_count = 0;
                                    size_t k               = j;
                                    while (k > 0 && sub_str[k - 1] == '\\')
                                    {
                                        backslash_count++;
                                        k--;
                                    }
                                    if (backslash_count % 2 == 1)
                                        continue;
                                    j++;
                                    break;
                                }
                            }

                            size_t comment_start = std::string::npos;
                            for (; j < sub_str.size(); j++)
                            {
                                if (sub_str[j] == '#' || sub_str[j] == ';')
                                {
                                    comment_start = j;
                                    while (comment_start > 0 && sub_str[comment_start - 1] == ' ')
                                        comment_start--;
                                    if (comment_start == j)
                                        line.push_back(' ');
                                    break;
                                }
                            }
                            if (comment_start != std::string::npos)
                            {
                                line.append(sub_str.substr(comment_start));
                                old_comment = sub_str.substr(comment_start);
                            }
                        }
                        else
                        {
                            for (; j < sub_str.size(); j++)
                            {
                                if (sub_str[j] == ' ')
                                    break;
                            }
                            for (; j < sub_str.size(); j++)
                            {
                                if (sub_str[j] != ' ')
                                    break;
                            }

                            if (j < sub_str.size() && (sub_str[j] == '#' || sub_str[j] == ';'))
                            {
                                if (j > 0 && sub_str[j - 1] == ' ')
                                    j--;
                                else
                                    line.append(" ");
                                line.append(sub_str.substr(j));
                                old_comment = sub_str.substr(j);
                            }
                            else
                            {
                                size_t comment_start = std::string::npos;
                                for (; j < sub_str.size(); j++)
                                {
                                    if (sub_str[j] == '#' || sub_str[j] == ';')
                                    {
                                        comment_start = j;
                                        while (comment_start > 0 && sub_str[comment_start - 1] == ' ')
                                            comment_start--;
                                        if (comment_start == j)
                                            line.push_back(' ');
                                        break;
                                    }
                                }
                                if (comment_start != std::string::npos)
                                {
                                    line.append(sub_str.substr(comment_start));
                                    old_comment = sub_str.substr(comment_start);
                                }
                            }
                        }
                    }

                    found = true;
                }
            }
        }
    }

    if (!found)
        return false;// 未找到键

    // 5. 原子写入：临时文件 + rename，保留 .bak 备份
    if (!atomic_write_file(filename_, lines))
        return false;

    // 6. 同步内存中的 config
    auto itSec = config.find(section);
    if (itSec != config.end())
    {
        for (auto &item : itSec->second.data)
        {
            if (item.name == name)
            {
                item.value = new_value;
                if (!normalized_comment.empty())
                    item.comment = normalized_comment;
                else if (!old_comment.empty())
                    item.comment = old_comment;
                break;
            }
        }
    }
    else
    {
        ini_key_value_t kv;
        kv.name    = name;
        kv.value   = new_value;
        kv.comment = !normalized_comment.empty() ? normalized_comment : old_comment;
        config[section].push_back(kv);
    }

    return true;
}

// ---------- 添加或更新某个值到文件中 ----------
bool parse_ini::add_value(const std::string &section, const std::string &name, const std::string &new_value, const std::string &comment)
{
    if (filename_.empty())
        return false;

    // 防止 INI 注入：节名、键名、值和注释中不能包含换行符
    if (section.find('\n') != std::string::npos || section.find('\r') != std::string::npos)
        return false;
    if (name.find('\n') != std::string::npos || name.find('\r') != std::string::npos)
        return false;
    if (new_value.find('\n') != std::string::npos || new_value.find('\r') != std::string::npos)
        return false;
    if (comment.find('\n') != std::string::npos || comment.find('\r') != std::string::npos)
        return false;

    // 节名和键名不能为空，键名不能包含 '='
    if (section.empty() || name.empty() || name.find('=') != std::string::npos)
        return false;

    // 规范化注释
    std::string normalized_comment;
    if (!comment.empty())
    {
        if (comment[0] == ';' || comment[0] == '#')
            normalized_comment = comment;
        else
            normalized_comment = "; " + comment;
    }

    // 如果键已存在，直接更新
    auto itSec = config.find(section);
    if (itSec != config.end() && itSec->second.contains(name))
    {
        return save_value(section, name, new_value, comment);
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
                        if (j > (i + 1))
                        {
                            //保留 下一个节的标题 前一行，可能是 下一个节的标题的注释
                            t = line_trim(lines[j - 1]);
                            if (t.empty() || t.front() == ';' || t.front() == '#')
                            {
                                next_sec = next_sec - 1;
                            }
                        }
                        break;
                    }
                }
                insert_pos = next_sec;// 插入到下一个节之前
                break;
            }
        }
    }

    // 构建新行（包含注释）
    std::string new_line = name + " = " + new_value;
    if (!normalized_comment.empty())
        new_line += " " + normalized_comment;

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
    ini_key_value_t kv;
    kv.name    = name;
    kv.value   = new_value;
    kv.comment = normalized_comment;
    config[section].push_back(kv);
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

// ---------- 删除文件中的某个section（注释也删除） ----------
bool parse_ini::delete_section(const std::string &section)
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

    // 2. 定位目标节的起始和结束区间（左闭右开）
    size_t start_idx = lines.size();// 未找到时使用最大索引
    size_t end_idx   = lines.size();

    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::string trimmed = line_trim(lines[i]);

        // 跳过空行和注释行（它们不可能是节标题，但不会影响索引记录）
        if (trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#')
            continue;

        // 检测节标题 [section]
        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            std::string cur_section = trimmed.substr(1, trimmed.size() - 2);
            if (cur_section == section)
            {
                start_idx = i;// 节标题行本身的索引
                // 向后查找下一个节标题（或文件末尾）
                for (size_t j = i + 1; j < lines.size(); ++j)
                {
                    std::string t2 = line_trim(lines[j]);
                    if (!t2.empty() && t2.front() == '[' && t2.back() == ']')
                    {
                        end_idx = j;// 下一个节的标题行索引
                        if (j > (start_idx + 1))
                        {
                            //保留 下一个节的标题 前一行，可能是 下一个节的标题的注释
                            t2 = line_trim(lines[j - 1]);
                            if (!t2.empty() && (t2.front() == ';' || t2.front() == '#'))
                            {
                                end_idx = end_idx - 1;
                            }
                        }
                        break;
                    }
                }
                if (end_idx == lines.size())// 未找到下一个节，则到文件末尾
                    end_idx = lines.size();
                break;// 找到目标节，退出外层循环
            }
        }
    }

    if (start_idx == lines.size())
        return false;// 未找到该节

    // 3. 删除区间 [start_idx, end_idx) —— 包含节标题及其后所有行，直到下一个节前
    lines.erase(lines.begin() + start_idx, lines.begin() + end_idx);

    // 4. 原子写入（使用您已有的 atomic_write_file）
    if (!atomic_write_file(filename_, lines))
        return false;

    // 5. 同步内存中的 config
    config.erase(section);

    return true;
}

// ---------- 将内存中的配置保存到文件 ----------
bool parse_ini::save_file(const std::string &filename)
{
    // 构建行列表
    std::vector<std::string> lines;
    for (const auto &secPair : config)
    {
        lines.push_back("[" + secPair.first + "]");
        for (const auto &kv : secPair.second)
        {
            if (kv.name.empty())
            {
                // 纯注释行
                if (!kv.comment.empty())
                    lines.push_back("; " + kv.comment);
                else
                    lines.push_back("");
            }
            else
            {
                std::string line = kv.name + " = " + kv.value;
                if (!kv.comment.empty())
                {
                    if (kv.comment[0] == ';' || kv.comment[0] == '#')
                        line += " " + kv.comment;
                    else
                        line += " ; " + kv.comment;
                }
                lines.push_back(line);
            }
        }
        lines.push_back("");// 节之间空行
    }

    // 原子写入：临时文件 + rename，保留 .bak 备份
    if (!atomic_write_file(filename, lines))
        return false;

    filename_ = filename;
    return true;
}

// ---------- 规范化ini文件，不删内容，只是格式化 ----------
//section 除了第一个，每个前面有一个空行，= 号两边都有一个空格，value后面注释都有一个空格隔开, ; #一行注释必须顶尽开头
unsigned int parse_ini::fix_file(const std::string &filename)
{
    if (filename.empty())
        return false;

    // 1. 读取文件并去除每行首尾空白
    std::ifstream inFile(filename);
    if (!inFile.is_open())
        return false;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line))
        lines.push_back(line_trim(line));// 去除首尾空白
    inFile.close();

    // 2. 删除多余空行：只保留每个 [section] 前一个空行（第一个 section 前无空行）
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i].empty())
        {
            // 若下一行是 section，且当前空行不是第一个 section 前的空行，则保留
            if (i + 1 < lines.size() && !lines[i + 1].empty() && lines[i + 1][0] == '[')
            {
                //下一行是section，非常标准
                continue;
            }
            else if (i + 1 < lines.size() && lines[i + 1].empty())
            {
                // i后一行
                lines.erase(lines.begin() + i + 1);
                --i;
            }
        }
    }

    // 3. 计算所有 key 的最大长度（仅针对包含 '=' 的有效行，忽略空行、section、注释）
    size_t max_key_len = 0;
    for (const auto &l : lines)
    {
        if (l.empty() || l[0] == '[' || l[0] == ';' || l[0] == '#')
            continue;

        unsigned int kk = 0;
        for (; kk < l.size(); kk++)
        {
            if (l[kk] == '"' || l[kk] == ';' || l[kk] == '#')
            {
                //废行，不统计，因为没有=号
                kk = 0;
                break;
            }
            else if (l[kk] == '=')
            {
                //回退到字符
                while (kk > 0 && l[kk - 1] == ' ')
                {
                    kk--;
                }
                break;
            }
        }

        if (max_key_len < kk)
        {
            max_key_len = kk;
        }
    }

    // 4. 重构每一行
    std::vector<std::string> new_lines;
    for (const auto &l : lines)
    {
        // 空行、section、注释行直接保留原样（不进行对齐处理）
        if (l.empty() || l[0] == '[' || l[0] == ';' || l[0] == '#')
        {
            if (l.size() > 0 && l[0] == '[')
            {
                if (!new_lines.empty() && !new_lines.back().empty())
                {
                    new_lines.push_back("");
                }
            }
            new_lines.push_back(l);
            continue;
        }

        // 处理普通 key=value 行
        size_t eq_pos    = std::string::npos;
        bool is_continue = false;
        for (unsigned int kk = 0; kk < l.size(); kk++)
        {
            if (l[kk] == '"' || l[kk] == ';' || l[kk] == '#')
            {
                new_lines.push_back(l);
                is_continue = true;
                break;
            }
            else if (l[kk] == '=')
            {
                eq_pos = kk;
                break;
            }
        }
        if (is_continue)
        {
            continue;
        }

        if (eq_pos == std::string::npos)
        {
            // 没有等号的行（可能是格式错误），原样保留
            new_lines.push_back(l);
            continue;
        }

        // 提取 key（去除尾部空格）
        std::string key = l.substr(0, eq_pos);
        while (!key.empty() && key.back() == ' ')
            key.pop_back();

        // 提取等号后的部分，分离 value 和注释
        std::string rest = l.substr(eq_pos + 1);
        // 去除 rest 前导空格
        while (!rest.empty() && rest.front() == ' ')
            rest.erase(rest.begin());

        std::string value, comment;
        //分离出值和注释
        // " 引号开头，里面可能有; # 之类
        if (rest.size() > 0 && rest[0] == '"')
        {
            unsigned int j = 1;
            for (; j < rest.size(); j++)
            {
                if (rest[j] == '"')
                {
                    // 统计前面的连续反斜杠数量，奇数表示转义引号
                    size_t backslash_count = 0;
                    size_t k               = j;
                    while (k > 0 && rest[k - 1] == '\\')
                    {
                        backslash_count++;
                        k--;
                    }
                    if (backslash_count % 2 == 1)
                    {
                        value.push_back(rest[j]);
                        continue;// 转义引号，跳过
                    }
                    break;
                }
                value.push_back(rest[j]);
            }
            for (; j < rest.size(); j++)
            {
                if (rest[j] == ';' || rest[j] == '#')
                {
                    comment = rest.substr(j);
                    break;
                }
            }
        }
        else
        {
            size_t comment_pos = rest.find_first_of(";#");
            if (comment_pos != std::string::npos)
            {
                value = rest.substr(0, comment_pos);
                while (!value.empty() && value.back() == ' ')
                    value.pop_back();
                comment = rest.substr(comment_pos);
                // 确保注释前有一个空格（如果原注释前已有空格则保留，否则添加）
            }
            else
            {
                value = rest;
                while (!value.empty() && value.back() == ' ')
                    value.pop_back();
                comment.clear();
            }
        }

        // 重构：key 左对齐补空格，然后 " = "，然后 value
        std::string new_line = key;
        if (key.size() < max_key_len)
            new_line.append(max_key_len - key.size(), ' ');
        new_line += " = " + value;

        // 添加注释（确保前面有一个空格）
        if (!comment.empty())
        {
            if (comment.front() != ' ')
                new_line += " ";
            new_line += comment;
        }

        new_lines.push_back(new_line);
    }

    // 5. 原子写入（临时文件 + rename，保留 .bak）
    if (!atomic_write_file(filename, new_lines))
        return false;

    return true;
}

}// namespace http