#pragma once
#ifndef HTTP_INIFILE_CONFIG
#define HTTP_INIFILE_CONFIG

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <utility>

namespace http
{

struct ini_key_value_t
{
    std::string name;
    std::string value;
    std::string comment;
};

struct ini_item_t
{

    std::vector<ini_key_value_t> data;
    // ---------- operator[] ----------
    std::string &operator[](const std::string &key)
    {
        for (auto &item : data)
            if (item.name == key)
                return item.value;
        data.push_back({key, ""});
        return data.back().value;
    }

    const std::string &operator[](const std::string &key) const
    {
        for (const auto &item : data)
            if (item.name == key)
                return item.value;
        throw std::out_of_range("Key not found");
    }

    // ---------- 下标访问 ----------
    ini_key_value_t &operator[](size_t idx) { return data[idx]; }
    const ini_key_value_t &operator[](size_t idx) const { return data[idx]; }

    ini_key_value_t &at(size_t idx) { return data.at(idx); }
    const ini_key_value_t &at(size_t idx) const { return data.at(idx); }

    // ---------- push_back ----------
    void push_back(const std::string &key, const std::string &value)
    {
        data.push_back({key, value});
    }
    
    void push_back(const std::string &key, const std::string &value, const std::string &comment)
    {
        data.push_back({key, value, comment});
    }

    void push_back(const ini_key_value_t &kv) { data.push_back(kv); }
    void push_back(ini_key_value_t &&kv) { data.push_back(std::move(kv)); }
    void push_back(const std::pair<std::string, std::string> &kv)
    {
        data.push_back({kv.first, kv.second});
    }
    void push_back(std::pair<std::string, std::string> &&kv)
    {
        data.push_back({std::move(kv.first), std::move(kv.second)});
    }

    // ---------- find（返回引用，未找到返回 抛异常，和[]） ----------
    std::string &find(const std::string &key)
    {
        for (auto &item : data)
            if (item.name == key)
                return item.value;
        throw std::out_of_range("Key not found");
    }
    const std::string &find(const std::string &key) const
    {
        for (const auto &item : data)
            if (item.name == key)
                return item.value;
        throw std::out_of_range("Key not found");
    }

    // 保留 try_find（返回 pair<指针,bool>）供选择
    std::pair<std::string *, bool> try_find(const std::string &key)
    {
        for (auto &item : data)
            if (item.name == key)
                return {&item.value, true};
        return {nullptr, false};
    }
    std::pair<const std::string *, bool> try_find(const std::string &key) const
    {
        for (const auto &item : data)
            if (item.name == key)
                return {&item.value, true};
        return {nullptr, false};
    }

    //重复键值 分成一个数组 比如 orm.conf 那样
    std::vector<std::vector<ini_key_value_t>> splits()
    {
        std::vector<std::vector<ini_key_value_t>> out_data;
        if (data.empty())
            return out_data;

        // 记录本组已经出现过的键
        std::unordered_set<std::string> used_keys;
        std::vector<ini_key_value_t> current_group;

        for (const auto &item : data)
        {
            if (used_keys.find(item.name) != used_keys.end())
            {
                // 键重复 → 结束当前组，存入结果
                if (!current_group.empty())
                {
                    out_data.push_back(std::move(current_group));
                    current_group.clear();
                    used_keys.clear();
                }
                // 将当前元素加入新组
                current_group.push_back(item);
                used_keys.insert(item.name);
            }
            else
            {
                // 新键，加入当前组
                current_group.push_back(item);
                used_keys.insert(item.name);
            }
        }

        // 存入最后一组
        if (!current_group.empty())
        {
            out_data.push_back(std::move(current_group));
        }

        return out_data;
    }

    // ---------- erase ----------
    size_t erase(const std::string &key)
    {
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            if (it->name == key)
            {
                data.erase(it);
                return 1;
            }
        }
        return 0;
    }

    // ---------- contains ----------
    bool contains(const std::string &key) const
    {
        for (const auto &item : data)
            if (item.name == key)
                return true;
        return false;
    }

    // ---------- 迭代器 ----------
    auto begin() noexcept { return data.begin(); }
    auto end() noexcept { return data.end(); }
    auto begin() const noexcept { return data.begin(); }
    auto end() const noexcept { return data.end(); }
    auto cbegin() const noexcept { return data.cbegin(); }
    auto cend() const noexcept { return data.cend(); }

    // ---------- 辅助 ----------
    std::string out_put() const
    {
        std::ostringstream oss;
        for (const auto &item : data)
        {
            if (item.name.empty())
            {
                if(item.comment.empty())
                {
                    oss << '\n';
                }
                else
                {
                    if (!item.comment.empty() && (item.comment[0] == ';' || item.comment[0] == '#'))
                    {
                        oss << item.comment << '\n';
                    }
                    else
                    {
                        oss << "; "<< item.comment << '\n';
                    }
                }
            }
            else
            {
                if(item.comment.empty())
                {
                    oss << item.name << " = " << item.value << '\n';
                }
                else
                {
                    if (!item.comment.empty() && (item.comment[0] == ';' || item.comment[0] == '#'))
                        oss << item.name << " = " << item.value << " " << item.comment << '\n';
                    else
                        oss << item.name << " = " << item.value << " ; "<< item.comment << '\n';
                }
            }
        }
        return oss.str();
    }

    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
};

class parse_ini
{
  private:
    //原子写入文件（临时文件 + rename，保留 .bak 备份）
    bool atomic_write_file(const std::string &filename, const std::vector<std::string> &lines);

  public:
    parse_ini();
    parse_ini(const std::string &filename);
    //trim 行前后空白
    std::string line_trim(const std::string &s);
    //解析文件
    void parse_file(const std::string &filename);

    //返回某name 值
    std::string get_value(const std::string &section, const std::string &name, const std::string &default_value = "");
    //返回某段所有内容
    ini_item_t get_section(const std::string &section);
    //替换掉filename_文件中 某个值，不用config内容，重新加载找出,name 然后换掉 值，保留原文注释
    bool save_value(const std::string &section, const std::string &name, const std::string &default_value = "", const std::string &comment = "");
    

    // ---------- 添加或更新某个值到文件中 ----------
    bool add_value(const std::string &section, const std::string &name, const std::string &default_value = "", const std::string &comment = "");

    // ---------- 删除文件中的某个值（保留注释和格式） ----------
    bool delete_value(const std::string &section, const std::string &name);
    bool delete_section(const std::string &section);
    //保存config到文件
    bool save_file(const std::string &filename);
    unsigned int fix_file(const std::string &filename);

  public:
    std::unordered_map<std::string, ini_item_t> config;
    std::string filename_;//文件名
};
}// namespace http
#endif