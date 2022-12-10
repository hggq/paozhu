#ifndef __DIRECTORY_FUN_H__
#define __DIRECTORY_FUN_H__
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <memory>
#include <cstdio>
#include <sys/fcntl.h>

#include <filesystem>
#include <string>
#include <string_view>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif
namespace fs = std::filesystem;
namespace http
{
    std::string file_time_tostring(fs::file_time_type const &ftime);
    std::string displaydirectory(std::string localpath, std::string urlpath, std::string dir_sort, std::string tempfile);
}
#endif
