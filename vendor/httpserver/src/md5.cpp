
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include "md5.h"

namespace http
{
MD5::MD5()
{
    temp_state[0] = 0x67452301;
    temp_state[1] = 0xefcdab89;
    temp_state[2] = 0x98badcfe;
    temp_state[3] = 0x10325476;
    strlength     = 0;
}
// F函数
unsigned int MD5::F(unsigned int b, unsigned int c, unsigned int d) { return (b & c) | ((~b) & d); }
// G函数
unsigned int MD5::G(unsigned int b, unsigned int c, unsigned int d) { return (b & d) | (c & (~d)); }
// H函数
unsigned int MD5::H(unsigned int b, unsigned int c, unsigned int d) { return b ^ c ^ d; }
// I函数
unsigned int MD5::I(unsigned int b, unsigned int c, unsigned int d) { return c ^ (b | (~d)); }
// 移位操作函数
unsigned int MD5::shift(unsigned int a, unsigned int n) { return (a << n) | (a >> (32 - n)); }
// 编码函数
std::string MD5::encode(const std::string &src)
{

    temp_state[0] = 0x67452301;
    temp_state[1] = 0xefcdab89;
    temp_state[2] = 0x98badcfe;
    temp_state[3] = 0x10325476;

    strlength                     = 0;
    std::vector<unsigned int> rec = padding(&src[0], src.size());
    for (unsigned int i = 0; i < strlength / 16; i++)
    {
        unsigned int num[16];
        for (int j = 0; j < 16; j++)
        {
            num[j] = rec[i * 16 + j];
        }
        iterateFunc(num);// iterateFunc(num, 16);
    }

    out_format();
    return out_value;
}
std::string MD5::encode(const char *src, unsigned int src_size)
{

    temp_state[0] = 0x67452301;
    temp_state[1] = 0xefcdab89;
    temp_state[2] = 0x98badcfe;
    temp_state[3] = 0x10325476;

    strlength                     = 0;
    std::vector<unsigned int> rec = padding(src, src_size);
    for (unsigned int i = 0; i < strlength / 16; i++)
    {
        unsigned int num[16];
        for (int j = 0; j < 16; j++)
        {
            num[j] = rec[i * 16 + j];
        }
        iterateFunc(num);
    }
    out_format();
    return out_value;
}
// 循环压缩
void MD5::iterateFunc(unsigned int *X)//, unsigned int size
{
    unsigned int a = temp_state[0], b = temp_state[1], c = temp_state[2], d = temp_state[3], rec = 0, g, k;

    for (int i = 0; i < 64; i++)
    {
        if (i < 16)
        {
            // F迭代
            g = F(b, c, d);
            k = i;
        }
        else if (i < 32)
        {
            // G迭代
            g = G(b, c, d);
            k = (1 + 5 * i) % 16;
        }
        else if (i < 48)
        {
            // H迭代
            g = H(b, c, d);
            k = (5 + 3 * i) % 16;
        }
        else
        {
            // I迭代
            g = I(b, c, d);
            k = (7 * i) % 16;
        }
        rec = d;
        d   = c;
        c   = b;
        b   = b + shift(a + g + X[k] + T[i], s[i]);
        a   = rec;
    }

    temp_state[0] += a;
    temp_state[1] += b;
    temp_state[2] += c;
    temp_state[3] += d;
}
// 填充字符串
std::vector<unsigned int> MD5::padding(const char *src, unsigned int src_size)
{
    // 以512位,64个字节为一组
    unsigned int num = ((src_size + 8) / 64) + 1;
    std::vector<unsigned int> rec(num * 16);
    strlength = num * 16;
    for (unsigned int i = 0; i < src_size; i++)
    {
        // 一个unsigned int对应4个字节，保存4个字符信息
        rec[i >> 2] |= (int)(src[i]) << ((i % 4) * 8);
    }
    // 补充1000...000
    rec[src_size >> 2] |= (0x80 << ((src_size % 4) * 8));
    // 填充原文长度
    rec[rec.size() - 2] = (src_size << 3);
    return rec;
}
// 整理输出
void MD5::out_format()
{

    out_value.reserve(32);
    unsigned int base = 1 << 8;
    char aa, bb;
    char str16[] = "0123456789abcdef";
    for (int n = 0; n < 4; n++)
    {

        for (int i = 0; i < 4; i++)
        {
            unsigned int b = (temp_state[n] >> (i * 8)) % base & 0xff;
            aa             = str16[b % 16];
            b /= 16;
            bb = str16[b % 16];

            out_value.push_back(bb);
            out_value.push_back(aa);
        }
    }
}

std::string md5(const std::string &str)
{
    MD5 md;
    return md.encode(str);
}
std::string md5(const char *str, unsigned int str_size)
{
    MD5 md;
    return md.encode(str, str_size);
}
}// namespace http