#ifndef FRAME_DATETIME_UTILITY
#define FRAME_DATETIME_UTILITY

#include <iostream>
#include <ctime>
#include <array>
#include <string>
#include <sys/time.h>
#include <unistd.h>

namespace http
{

std::string get_gmttime(time_t inputtime = 0);
unsigned long timeid();
unsigned long rand_range(unsigned int a, unsigned int b);
std::string rand_string(unsigned int a, unsigned char b = 0);
std::string dateid(const std::string &format, unsigned long inputtime = 0);

// Sat, 02 Jan 2021 00:12:45 GMT to sec
unsigned long strgmttotime(const std::string &gmtstr);

// 2022-02-09 08:12:45 to sec
unsigned long strtotime(const std::string &str);
std::string get_uuid();
} // namespace http
#endif