#ifndef __FILE_GET_CONTENT_H__
#define __FILE_GET_CONTENT_H__

#include <iostream>
#include <string>
namespace http
{
std::string file_get_contents(std::string, std::map<std::string, std::string> &, unsigned int timeoutnum = 0);
std::string file_get_contents(std::string, unsigned int timeoutnum = 0);
bool file_put_contents(std::string, std::string &, bool append = false);
bool file_put_contents(std::string, const char *, unsigned int, bool append = false);
}// namespace http
#endif
