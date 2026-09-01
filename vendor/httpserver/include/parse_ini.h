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

    void clear() { data.clear(); }

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
                if (item.comment.empty())
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
                        oss << "; " << item.comment << '\n';
                    }
                }
            }
            else
            {
                if (item.comment.empty())
                {
                    oss << item.name << " = " << item.value << '\n';
                }
                else
                {
                    if (!item.comment.empty() && (item.comment[0] == ';' || item.comment[0] == '#'))
                        oss << item.name << " = " << item.value << " " << item.comment << '\n';
                    else
                        oss << item.name << " = " << item.value << " ; " << item.comment << '\n';
                }
            }
        }
        return oss.str();
    }

    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
};

struct ini_section_value_t
{
    std::string name;
    ini_item_t value;
    bool is_array   = false; // 如果是 true 表示是数组 [[xxx]] 这种两个中括号

    ini_section_value_t() = default;
    ini_section_value_t(std::string n, ini_item_t v, bool a)
        : name(std::move(n)), value(std::move(v)), is_array(a) {}

    // ---------- 转发 ini_item_t 接口，调用点可直接把本结构当作键值列表使用 ----------
    auto begin() noexcept { return value.begin(); }
    auto end() noexcept { return value.end(); }
    auto begin() const noexcept { return value.begin(); }
    auto end() const noexcept { return value.end(); }
    auto cbegin() const noexcept { return value.cbegin(); }
    auto cend() const noexcept { return value.cend(); }

    size_t size() const { return value.size(); }
    bool empty() const { return value.empty(); }
    void clear() { value.clear(); }

    std::string &operator[](const std::string &key) { return value[key]; }
    const std::string &operator[](const std::string &key) const { return value[key]; }

    ini_key_value_t &operator[](size_t idx) { return value[idx]; }
    const ini_key_value_t &operator[](size_t idx) const { return value[idx]; }

    std::string &find(const std::string &key) { return value.find(key); }
    const std::string &find(const std::string &key) const { return value.find(key); }

    std::pair<std::string *, bool> try_find(const std::string &key) { return value.try_find(key); }
    std::pair<const std::string *, bool> try_find(const std::string &key) const { return value.try_find(key); }

    bool contains(const std::string &key) const { return value.contains(key); }
    size_t erase(const std::string &key) { return value.erase(key); }

    void push_back(const ini_key_value_t &kv) { value.push_back(kv); }
    void push_back(ini_key_value_t &&kv) { value.push_back(std::move(kv)); }

    std::string out_put() const { return value.out_put(); }
};

struct ini_parse_t
{
    std::vector<ini_section_value_t> config;

    static constexpr size_t npos = std::string::npos;

    // ---------- operator[]：按名取第一个匹配元素，不存在则新建普通段 [name] ----------
    ini_section_value_t &operator[](const std::string &key)
    {
        for (auto &item : config)
            if (item.name == key)
                return item;
        config.emplace_back(key, ini_item_t{}, false);
        return config.back();
    }

    const ini_section_value_t &operator[](const std::string &key) const
    {
        for (const auto &item : config)
            if (item.name == key)
                return item;
        throw std::out_of_range("Key not found");
    }

    // ---------- 下标访问 ----------
    ini_section_value_t &operator[](size_t idx) { return config[idx]; }
    const ini_section_value_t &operator[](size_t idx) const { return config[idx]; }

    ini_section_value_t &at(size_t idx) { return config.at(idx); }
    const ini_section_value_t &at(size_t idx) const { return config.at(idx); }

    void clear() { config.clear(); }

    // ---------- 名字定位：本结构允许同名段（[[xxx]] 数组），以下方法区分第几个 ----------
    size_t index_of(const std::string &key) const
    {
        for (size_t i = 0; i < config.size(); ++i)
            if (config[i].name == key)
                return i;
        return npos;
    }

    std::vector<size_t> indices_of(const std::string &key) const
    {
        std::vector<size_t> out;
        for (size_t i = 0; i < config.size(); ++i)
            if (config[i].name == key)
                out.push_back(i);
        return out;
    }

    size_t count_of(const std::string &key) const
    {
        size_t n = 0;
        for (const auto &item : config)
            if (item.name == key)
                ++n;
        return n;
    }

    // ---------- contains（已有的，但也同时匹配 is_array 标志） ----------
    bool contains(const std::string &key, bool is_array) const
    {
        for (const auto &item : config)
            if (item.name == key && item.is_array == is_array)
                return true;
        return false;
    }

    // 第 nth 个同名段，越界抛异常
    ini_section_value_t &section(const std::string &key, size_t nth = 0)
    {
        for (auto &item : config)
        {
            if (item.name != key)
                continue;
            if (nth-- == 0)
                return item;
        }
        throw std::out_of_range("Key not found");
    }

    const ini_section_value_t &section(const std::string &key, size_t nth = 0) const
    {
        for (const auto &item : config)
        {
            if (item.name != key)
                continue;
            if (nth-- == 0)
                return item;
        }
        throw std::out_of_range("Key not found");
    }

    std::pair<ini_section_value_t *, bool> try_section(const std::string &key, size_t nth = 0)
    {
        for (auto &item : config)
        {
            if (item.name != key)
                continue;
            if (nth-- == 0)
                return {&item, true};
        }
        return {nullptr, false};
    }

    std::pair<const ini_section_value_t *, bool> try_section(const std::string &key, size_t nth = 0) const
    {
        for (const auto &item : config)
        {
            if (item.name != key)
                continue;
            if (nth-- == 0)
                return {&item, true};
        }
        return {nullptr, false};
    }

    // ---------- 新建段 ----------
    // 追加一个 [[key]] 数组元素，始终新建，不合并
    ini_section_value_t &append_section(const std::string &key)
    {
        config.emplace_back(key, ini_item_t{}, true);
        return config.back();
    }

    // is_array=true 始终新建数组元素；false 时查找第一个同名且 is_array==false 的普通段（没有则新建）
    // 关键：与 operator[] 不同，这里严格区分 is_array 类型，
    // 避免 [mixed] 被错误合并到已存在的 [[mixed]] 数组段中
    ini_section_value_t &get_or_add(const std::string &key, bool is_array)
    {
        if (is_array)
        {
            // 数组段：每次新建，不合并
            return append_section(key);
        }
        // 普通段：只匹配 is_array==false 的同名段；找不到则新建普通段
        for (auto &item : config)
            if (item.name == key && !item.is_array)
                return item;
        config.emplace_back(key, ini_item_t{}, false);
        return config.back();
    }

    // ---------- push_back ----------
    void push_back(const std::string &key, const ini_item_t &value, bool is_array = false)
    {
        config.push_back(ini_section_value_t{key, value, is_array});
    }

    void push_back(const std::string &key, ini_item_t &&value, bool is_array = false)
    {
        config.push_back(ini_section_value_t{key, std::move(value), is_array});
    }

    void push_back(const ini_section_value_t &kv) { config.push_back(kv); }
    void push_back(ini_section_value_t &&kv) { config.push_back(std::move(kv)); }

    // ---------- find（返回引用，未找到抛异常）按名取第一个匹配段 ----------
    ini_item_t &find(const std::string &key)
    {
        for (auto &item : config)
            if (item.name == key)
                return item.value;
        throw std::out_of_range("Key not found");
    }
    const ini_item_t &find(const std::string &key) const
    {
        for (const auto &item : config)
            if (item.name == key)
                return item.value;
        throw std::out_of_range("Key not found");
    }

    // 保留 try_find（返回 pair<指针,bool>）供选择
    std::pair<ini_item_t *, bool> try_find(const std::string &key)
    {
        for (auto &item : config)
            if (item.name == key)
                return {&item.value, true};
        return {nullptr, false};
    }
    std::pair<const ini_item_t *, bool> try_find(const std::string &key) const
    {
        for (const auto &item : config)
            if (item.name == key)
                return {&item.value, true};
        return {nullptr, false};
    }

    // ---------- erase：按名删除，erase 删第一个匹配，erase_all 删全部同名 ----------
    size_t erase(const std::string &key)
    {
        for (auto it = config.begin(); it != config.end(); ++it)
        {
            if (it->name == key)
            {
                config.erase(it);
                return 1;
            }
        }
        return 0;
    }

    size_t erase_all(const std::string &key)
    {
        size_t before = config.size();
        config.erase(std::remove_if(config.begin(), config.end(), [&key](const ini_section_value_t &item)
                                    { return item.name == key; }),
                     config.end());
        return before - config.size();
    }

    // ---------- contains ----------
    bool contains(const std::string &key) const
    {
        for (const auto &item : config)
            if (item.name == key)
                return true;
        return false;
    }

    // ---------- 迭代器 ----------
    auto begin() noexcept { return config.begin(); }
    auto end() noexcept { return config.end(); }
    auto begin() const noexcept { return config.begin(); }
    auto end() const noexcept { return config.end(); }
    auto cbegin() const noexcept { return config.cbegin(); }
    auto cend() const noexcept { return config.cend(); }

    // ---------- 辅助 ----------
    std::string out_put() const
    {
        std::ostringstream oss;
        size_t n = config.size();
        for (size_t i = 0; i < n; ++i)
        {
            if (i > 0)
                oss << '\n';

            const auto &sec = config[i];
            oss << (sec.is_array ? "[[" : "[") << sec.name << (sec.is_array ? "]]" : "]") << '\n';

            // 直接委托 ini_item_t::out_put()，避免复制粘贴
            oss << sec.value.out_put();
        }
        return oss.str();
    }

    size_t size() const { return config.size(); }
    bool empty() const { return config.empty(); }
};

class parse_ini
{
  private:
    //原子写入文件（临时文件 + rename，保留 .bak 备份）
    bool atomic_write_file(const std::string &filename, const std::vector<std::string> &lines);

    //解析节标题行（要求传入已 trim 的行），识别 [section] 与 [[section]]
    //返回 true 表示这是一行节标题，name 为段名，is_array 标识是否双中括号
    bool section_header(const std::string &trimmed, std::string &name, bool &is_array);

  public:
    parse_ini();
    parse_ini(const std::string &filename);
    //trim 行前后空白
    std::string line_trim(const std::string &s);
    //解析文件
    void parse_file(const std::string &filename);

    //返回某name 值（同名段取第一个匹配元素）
    std::string get_value(const std::string &section, const std::string &name, const std::string &default_value = "");
    //返回某段所有内容（同名段取第一个匹配元素）
    ini_item_t get_section(const std::string &section);
    //替换掉filename_文件中 某个值，不用config内容，重新加载找出,name 然后换掉 值，保留原文注释
    bool save_value(const std::string &section, const std::string &name, const std::string &default_value = "", const std::string &comment = "");

    // ---------- 添加或更新某个值到文件中 ----------
    bool add_value(const std::string &section, const std::string &name, const std::string &default_value = "", const std::string &comment = "");

    // ---------- 删除文件中的某个值（保留注释和格式） ----------
    bool delete_value(const std::string &section, const std::string &name);
    bool delete_section(const std::string &section);
    //删除文件中全部同名段（[[xxx]] 数组的所有元素）
    bool delete_all_sections(const std::string &section);
    //保存config到文件
    bool save_file(const std::string &filename);
    unsigned int fix_file(const std::string &filename);

  public:
    ini_parse_t config;
    std::string filename_;//文件名
};
}// namespace http
#endif