#ifndef __STR_FUN_H__
#define __STR_FUN_H__

#include <iostream>
#include <string>
namespace http
{
    long long str2int(const char *source, unsigned int str_length);
    std::string str2safepath(const char *source, unsigned int str_length);
    std::string str2safefile(const char *source, unsigned int str_length);
     std::string str2safemethold(const char *source, unsigned int str_length);
}
#endif
