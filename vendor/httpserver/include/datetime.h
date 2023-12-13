#ifndef FRAME_DATETIME_UTILITY
#define FRAME_DATETIME_UTILITY

#include <iostream>
#include <ctime>
#include <array>
#include <string>
#ifndef _MSC_VER
#include <sys/time.h>
#include <unistd.h>
#endif
namespace http
{

std::string get_gmttime(time_t inputtime = 0);
std::string get_utctime(time_t inputtime = 0);
unsigned int timeid();
unsigned int rand_range(unsigned int a, unsigned int b);
std::string rand_string(unsigned int a, unsigned char b = 0);
std::string get_date(const std::string &format, unsigned int inputtime = 0);

// Sat, 02 Jan 2021 00:12:45 GMT to sec
unsigned int strgmttotime(const std::string &gmtstr);

// 2022-02-09 08:12:45 to sec
unsigned int strtotime(const std::string &str);
std::string get_uuid();
}// namespace http
#endif