#pragma once
#ifndef HTTP_INIFILE_CONFIG
#define HTTP_INIFILE_CONFIG

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>

namespace http
{
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
    std::unordered_map<std::string, std::string> get_section(const std::string &section);
    //替换掉filename_文件中 某个值，不用config内容，重新加载找出,name 然后换掉 值，保留原文注释
    bool save_value(const std::string &section, const std::string &name, const std::string &default_value = "");

    // ---------- 添加或更新某个值到文件中 ----------
    bool add_value(const std::string &section, const std::string &name, const std::string &default_value = "");

    // ---------- 删除文件中的某个值（保留注释和格式） ----------
    bool delete_value(const std::string &section, const std::string &name);
    //保存config到文件
    bool save_file(const std::string &filename);

  public:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> config;
    std::string filename_;//文件名
};
}// namespace http
#endif