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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#define stat _stat
#endif

namespace http
{
bool str_cmp(std::string_view str1, std::string_view str2);
bool str_casecmp(std::string_view str1, std::string_view str2);
bool str_compare(std::string_view str1, std::string_view str2, unsigned int length);
bool str_casecompare(std::string_view str1, std::string_view str2, unsigned int length);
bool str_lastcmp(std::string_view str1, std::string_view str2, unsigned int length);
bool str_caselastcmp(std::string_view str1, std::string_view str2, unsigned int length);
void get_filename(const std::string &filename, std::string &filename_name, std::string &filename_ext);
std::string get_filename(const std::string &filename);
std::vector<std::string> mb_split(std::string_view, std::string_view);
std::string html_encode(std::string_view);
std::string strip_html(std::string_view);
std::string strip_annot(std::string_view);
std::string str_trim(std::string_view);
std::string_view str_trim_view(std::string_view);
std::string mb_substr(std::string_view, int, int length = 0);
unsigned int mb_strlen(std::string_view);
std::map<std::string, std::string> filepath(std::string &);
struct stat filestat(std::string &);

long long str2int(const char *source, unsigned int str_length);
std::string char2str(const unsigned char *source, unsigned int str_length);
std::string str2safepath(const char *source, unsigned int str_length);
std::string str2safefile(const char *source, unsigned int str_length);
std::string str2safemethold(const char *source, unsigned int str_length);

std::string numstr_to_sql(const char *source, unsigned int str_length, char b = ',');

template <typename _Tp>
std::string str_join(const _Tp &source, char b = 0x00)
{
    std::stringstream _stream;
    unsigned int j = 0;
    for (typename _Tp::const_iterator iter = source.begin(); iter != source.end(); iter++)
    {
        if (j > 0 && b != 0x00)
        {
            _stream << b;
        }
        _stream << *iter;
        j++;
    }
    return _stream.str();
}

template <typename _Tp>
    requires std::is_integral_v<_Tp>
std::vector<_Tp> numstr_to_vector(const char *source, unsigned int str_length, char b = ',')
{
    std::vector<_Tp> tempt;
    std::string tempstr;

    for (unsigned int i = 0; i < str_length; i++)
    {
        if (source[i] == '-' || source[i] == '.' || (source[i] > 0x2F && source[i] < 0x3A))
        {
            tempstr.push_back(source[i]);
        }
        else if (source[i] == b)
        {
            if (tempstr.size() > 0)
            {
                try
                {
                    tempt.push_back(std::stoi(tempstr.c_str()));
                }
                catch (...)
                {
                }
            }
            tempstr.clear();
        }
    }
    if (tempstr.size() > 0)
    {
        tempt.push_back(std::stoi(tempstr.c_str()));
    }
    return tempt;
}

template <typename _Tp>
    requires std::is_floating_point_v<_Tp>
std::vector<_Tp> numstr_to_vector(const char *source, unsigned int str_length, char b = ',')
{
    std::vector<_Tp> tempt;
    std::string tempstr;

    for (unsigned int i = 0; i < str_length; i++)
    {
        if (source[i] == '-' || source[i] == '.' || (source[i] > 0x2F && source[i] < 0x3A))
        {
            tempstr.push_back(source[i]);
        }
        else if (source[i] == b)
        {
            if (tempstr.size() > 0)
            {
                try
                {
                    tempt.push_back(std::stof(tempstr.c_str()));
                }
                catch (...)
                {
                }
            }
            tempstr.clear();
        }
    }
    if (tempstr.size() > 0)
    {
        tempt.push_back(std::stof(tempstr.c_str()));
    }
    return tempt;
}

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

void get_directory_all_file(std::map<unsigned long long, std::string> &listobj,
                            const std::string &file_path,
                            const std::string &url_path,
                            const std::string &extfile);

}// namespace http
#endif