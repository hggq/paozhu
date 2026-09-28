#pragma once

#include <string>
#include <cstddef>

// 填充 len 字节的密码学安全随机数据。
// 失败时抛 std::runtime_error。
namespace http
{
void fill_secure_random(unsigned char *buf, size_t len);

// 生成 128 位密码学安全的 session id，返回 32 个十六进制字符。
// 失败时抛 std::runtime_error。
std::string get_rand_session_id();
std::string get_rand_32char();
}// namespace http