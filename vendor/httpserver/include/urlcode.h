#ifndef HTTP_URLDECODE_H
#define HTTP_URLDECODE_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <functional>
#include <map>
#include <utility>
#include <string>
#include <string_view>
#include <thread>
#include <chrono>

namespace http
{

std::string url_decode(const char *str, int len);
std::string url_decode_path(const char *str, int len);// 路径段解码：+ 保留字面量，不转空格

std::string url_encode(const char *str, int len);
std::string url_rawencode(const char *str, int len);
}// namespace http
#endif