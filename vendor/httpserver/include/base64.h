#ifndef _PROJECT_BASE64_H__
#define _PROJECT_BASE64_H__

#include <iostream>
#include <string>

// static const std::string base64_chars =
// "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
// "abcdefghijklmnopqrstuvwxyz"
// "0123456789+/";
namespace http
{
static const char base64_chars[2][65] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
                                         "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"};
static const char base64Index[]       = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, 62, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1};

// static inline bool is_base64(const char c);

std::string base64_encode(const char *bytes_to_encode, unsigned int in_len, unsigned char isurl = 0);

std::string base64_decode(const char *encoded_string, unsigned int in_len);
}// namespace http
#endif
