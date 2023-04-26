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
#include <string_view>

namespace http
{
void get_filename(const std::string &filename, std::string &filename_name, std::string &filename_ext);
std::string get_filename(const std::string &filename);
std::vector<std::string> mb_split(const std::string, std::string &);
std::string html_encode(std::string &);
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

} // namespace http
#endif