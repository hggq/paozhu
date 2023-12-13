#ifndef __DIRECTORY_FUN_H__
#define __DIRECTORY_FUN_H__
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <memory>
#include <cstdio>

#include <filesystem>
#include <string>
#include <string_view>

namespace fs = std::filesystem;
namespace http
{
std::string file_time_tostring(fs::file_time_type const &ftime);
std::string displaydirectory(std::string localpath, std::string urlpath, std::string dir_sort, std::string tempfile);
}// namespace http
#endif
