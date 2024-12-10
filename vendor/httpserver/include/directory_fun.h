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

struct directory_file_info_t
{
    std::string filename;
    unsigned long long size = 0;
    std::string time;
    std::string ext;
    std::string url;
};

std::string json_encode(const directory_file_info_t &json_reflectobj);
std::string json_encode(const std::vector<directory_file_info_t> &json_reflectobj);

std::string file_time_tostring(fs::file_time_type const &ftime);
std::string displaydirectory(std::string localpath, std::string urlpath, std::string dir_sort, std::string tempfile);
std::string displaydirectory_attachfile(std::string localpath, std::string urlpath,std::string realurlpath,std::string deleteurlpath, std::string dir_sort, std::string tempfile,bool isroot);
std::string displaydirectory_attachimg(std::string localpath, std::string urlpath,std::string realurlpath,std::string deleteurlpath, std::string dir_sort, std::string tempfile,bool isroot);
std::vector<directory_file_info_t> displaydirectory_attachfilejson(std::string localpath, std::string urlpath, std::string realurlpath, std::string deleteurlpath, std::string dir_sort, std::string configpath, bool isroot);
}// namespace http
#endif
