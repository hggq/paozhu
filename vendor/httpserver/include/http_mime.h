#ifndef __HTTP_MIME_H
#define __HTTP_MIME_H

#include <string>
#include <map>
#include <set>
#include <vector>

namespace http
{
    extern std::map<std::string, std::string> mime_map;
    extern std::set<std::string> mime_compress;
}
#endif