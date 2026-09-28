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
bool str_cmp_pre(std::string_view str1, std::string_view str2, unsigned int length = 0);
bool str_casecmp_pre(std::string_view str1, std::string_view str2, unsigned int length = 0);
bool str_cmp_last(std::string_view str1, std::string_view str2, unsigned int length = 0);
bool str_casecmp_last(std::string_view str1, std::string_view str2, unsigned int length = 0);
std::string mb_reverse(std::string_view str);
void get_filename(const std::string &filename, std::string &filename_name, std::string &filename_ext);
std::string get_filename(const std::string &filename);
std::string get_fileext(const std::string &filename);
std::vector<std::string> mb_split(std::string_view, std::string_view);
std::string html_encode(std::string_view);
std::string strip_html(std::string_view);
std::string strip_annot(std::string_view);
std::string str_trim(std::string_view);
std::string str_nl2br(std::string_view);
std::string_view str_trim_view(std::string_view);
std::string mb_substr(std::string_view, int, int length = 0);
bool tidy_domainname(std::string &filename);
bool is_domainname(std::string_view filename);
std::string strip_domainname(std::string_view filename);
bool check_isodate(std::string_view filename);
unsigned int mb_strlen(std::string_view);
std::map<std::string, std::string> filepath(std::string &);
struct stat filestat(std::string &);

unsigned long long str2uint(std::string_view);
unsigned long long str2uint(const char *source, unsigned int str_length);
long long str2int(std::string_view);
long long str2int(const char *source, unsigned int str_length);
// H1 修复：长度类头部（Content-Length 等）的严格解析。
// 仅接受纯数字（不接受空格/逗号/负号/十六进制/科学计数），并做位数与溢出判断，
// 避免 str2int 那种"跳过所有非数字后顺序累加"带来的走私面。
bool str2uint64_strict(std::string_view source, unsigned long long &out, unsigned int max_digits = 20);
std::string char2hex(const unsigned char *source, unsigned int str_length, unsigned char sp = 0);
std::string str2hex(std::string_view source, bool is_space = false);
std::string hex2str(std::string_view source, unsigned char sp = 0);
std::string str2safepath(const char *source, unsigned int str_length);
std::string str2safefile(const char *source, unsigned int str_length);
std::string str2safemethold(const char *source, unsigned int str_length);
std::string get_safepath(std::string_view str1);
std::string numstr_to_sql(const char *source, unsigned int str_length, char b = ',');
std::string dir_name(std::string_view name);
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
std::vector<_Tp> numstr_to_vector(std::string_view source, char b = ',')
{
    std::vector<_Tp> tempt;
    std::string tempstr;

    for (unsigned int i = 0; i < source.size(); i++)
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

std::string array_to_sql(const std::vector<std::string>, char b = ',', bool isquote = true);

std::string str_addslash(const std::string &content);
std::string json_addslash(const std::string &content);
std::string json_escape(const std::string &content);

void get_directory_all_file(std::map<unsigned long long, std::string> &listobj,
                            const std::string &file_path,
                            const std::string &url_path,
                            const std::string &extfile);

double money_get_num(long long a);
long long money_put_num(double a);

long long num_put_money(long long a);
long long num_get_money(long long a);

// IP 是否为本机回环或内网单播（127/8、10/8、192.168/16、172.16-31/12、IPv6 fc00::/7 与 ::1）。
// 用于证书下载页等敏感路由的来源 IP 闸门。严格解析，带端口/空格/主机名一律拒。
bool ip_is_local(const std::string &ip);

// ---- 请求落盘临时文件的生命周期 ----
// 命名前缀由「写入方」和「清理方」共用，二者必须保持一致：
//   写入方：http_parse.cpp / http2_parse.cpp（raw body 与 multipart 上传件）、rpc_parse.cpp（tempraw）
//   清理方：httpserver::httpwatch_clear_temp_files()（按 mtime + 前缀白名单删文件）
// 加前缀的目的是让清理只命中框架自己的临时文件，绝不误删 temp_path 下的业务文件。
inline constexpr std::string_view HTTP_TEMP_RAW_PREFIX    = "pzraw_"; // 请求体整体落盘（rawcontent/tempraw）
inline constexpr std::string_view HTTP_TEMP_UPLOAD_PREFIX = "pzup_";  // multipart 上传件落盘

// 生成落盘临时文件名（仅文件名，不含目录）。二者均无扩展名。
std::string make_http_temp_raw_name();
std::string make_http_temp_upload_name(unsigned long long content_length);
// 是否为框架生成的落盘临时文件名（严格匹配前缀 + 后续字符集）。
bool is_http_temp_filename(std::string_view name);

}// namespace http
#endif
