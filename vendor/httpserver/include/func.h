#pragma once
#ifndef _PROJECT_FUNC_H
#define _PROJECT_FUNC_H

#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <memory>
#include <sstream>
#include <string_view>

namespace http
{
void get_filename(const std::string &filename, std::string &filename_name, std::string &filename_ext);
std::string get_filename(const std::string &filename);
std::vector<std::string> mb_split(const std::string, std::string &);
std::string html_encode(std::string &);
std::string strip_html(const std::string &);
std::string strip_annot(const std::string &);
std::string mb_trim(std::string &);
std::string mb_substr(std::string &, int, int length = 0);
int mb_strlen(std::string &);
std::map<std::string, std::string> filepath(std::string &);
struct stat filestat(std::string &);

std::string file_get_contents(std::string, std::map<std::string, std::string> &);
std::string file_get_contents(std::string);
bool file_put_contents(std::string, std::string &, bool append = false);
bool file_put_contents(std::string, const char *, unsigned int, bool append = false);

long long str2int(const char *source, unsigned int str_length);
std::string str2safepath(const char *source, unsigned int str_length);
std::string str2safefile(const char *source, unsigned int str_length);
std::string str2safemethold(const char *source, unsigned int str_length);

template <typename _Tp>
    requires std::is_integral_v<_Tp> || std::is_floating_point_v<_Tp>

std::string array_to_sql(const std::vector<_Tp> &a, char b = ',')
{
    std::stringstream _stream;
    for (unsigned int j = 0; j < a.size(); j++)
    {
        if (j > 0)
        {
            _stream << b;
        }
        _stream << a[j];
    }
    return _stream.str();
}

std::string array_to_sql(const std::vector<std::string>, char b = ',', bool isquote = false);

std::string str_addslash(const std::string &content);
std::string json_addslash(const std::string &content);
} // namespace http
#endif