#pragma once
#ifndef HTTP2_DEFINE_H
#define HTTP2_DEFINE_H

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <memory>

namespace http
{

#define HTTP2_H2_ALPN "\x02h2"
#define HTTP2_H2_16_ALPN "\x05h2-16"
#define HTTP2_H2_14_ALPN "\x05h2-14"
#define HTTP1_NPN "\x08http/1.1"
#define HTTP2_H1_1_ALPN "\x08http/1.1"
#define HTTP2_H1_1 "http/1.1"

extern std::string PZ_HTTP2_H2_ALPN;
extern std::string PZ_HTTP2_H2;

extern std::string PZ_HTTP2_H2_16_ALPN;
extern std::string PZ_HTTP2_H2_16;

extern std::string PZ_HTTP2_H2_14_ALPN;
extern std::string PZ_HTTP2_H2_14;

extern std::string PZ_HTTP2_H1_1_ALPN;
extern std::string PZ_HTTP2_H1_1;

extern void close_file(FILE *fp);

using FILE_AUTO = std::unique_ptr<std::FILE, int (*)(FILE *)>;
}// namespace http
#endif
