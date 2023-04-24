/*
 *  @author 黄自权 huangziquan
 *  @date 2023-01-06
 *  @dest controller method pick file
 */
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <vector>
#include <filesystem>

#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include "autopickcontrolmethod.hpp"
#include "md5.h"

namespace fs = std::filesystem;

std::string get_filename(const std::string &filename)
{
  std::string filename_name;
  int j = filename.size() - 1;
  if (j == -1)
    return "";
  for (; j >= 0; j--)
  {
    if (filename[j] == '.')
    {
      if (filename_name.size() == 2 && filename_name[0] == 'z' &&
          filename_name[1] == 'g')
      {
        continue;
      }
      filename_name.clear();
      continue;
    }
    if (filename[j] == '/')
    {
      j--;
      break;
    }
    filename_name.push_back(filename[j]);
  }
  std::reverse(filename_name.begin(), filename_name.end());
  return filename_name;
}

void make_all_directory(const std::string &path, const std::string &pathname)
{
  std::string path_temp = path;

  unsigned int path_size = pathname.size();

  for (unsigned int j = 0; j < path_size; j++)
  {
    if (pathname[j] == '/' && path_temp.size() > 0)
    {
      fs::path paths = path_temp;
      if (!fs::exists(paths))
      {
        fs::create_directories(paths);
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all |
                            fs::perms::others_read,
                        fs::perm_options::add);
      }
    }
    path_temp.push_back(pathname[j]);
  }
}

int main(int argc, char *argv[])
{

  std::string current_run_path    = "";
  std::string current_method_file = "";
  fs::path current_path;
  if (argc > 0)
  {
    current_run_path.append(argv[1]);
  }
  else
  {
    current_path = fs::current_path();
    current_run_path.append(current_path.string());
  }

  if (current_run_path.back() != '/')
  {
    current_run_path.push_back('/');
  }

  http::pickcontrol lp;
  std::vector<std::string> header_lists;
  std::vector<struct http::reg_autoitem> reg_method_lists;
  std::string src_path = current_run_path + "controller/src";

  fs::path vsrcpath = src_path;

  if (!fs::exists(vsrcpath))
  {
    std::cout << " controller directory not in current path " << std::endl;
    return 0;
  }

  fs::path cache_path = current_run_path + "cache";
  if (!fs::exists(cache_path))
  {
    fs::create_directories(cache_path);
    fs::permissions(cache_path,
                    fs::perms::owner_all | fs::perms::group_all |
                        fs::perms::others_read,
                    fs::perm_options::add);
  }
  // md5 hash => filetime

  auto plist = lp.listpath(src_path);

  std::string cache_file_path = current_run_path + "cache/method_notes.data";

  std::map<std::string, struct http::file_regitem> reginfo_list =
      lp.loadcacheinfo(cache_file_path);

  struct stat sessfileinfo;
  std::string md5hash;
  std::string hash_value;
  std::string mttime_value;
  struct http::file_regitem cahce_info;
  bool is_must_again = false;
  for (unsigned int i = 0; i < plist.size(); i++)
  {
    std::string filename = src_path; // get_filename(plist[i]);
    if (filename.back() != '/')
    {
      filename.push_back('/');
    }
    filename.append(plist[i]);
    filename.append(".cpp");
    auto method_item = lp.pickfile(filename);

    if (method_item.size() > 0)
    {
      md5hash.clear();
      mttime_value.clear();
      for (unsigned int j = 0; j < method_item.size(); j++)
      {
        md5hash.append(method_item[j].pre);
        md5hash.append(method_item[j].func);
        md5hash.append(method_item[j].urlpath);
      }
      hash_value = http::md5(md5hash);

      filename.clear();
      filename.append(current_run_path);
      filename.append("controller/include/");
      // create custom directory
      make_all_directory(filename, plist[i]);

      filename.append(plist[i]);

      std::string fileinh = filename + ".h";
      filename            = plist[i];

      memset(&sessfileinfo, 0, sizeof(sessfileinfo));
      if (stat(fileinh.c_str(), &sessfileinfo) == 0)
      {
        if (sessfileinfo.st_mode & S_IFREG)
        {
          mttime_value = std::to_string(sessfileinfo.st_mtime);
        }
      }
      cahce_info = reginfo_list[filename];

      if (cahce_info.filehash.size() > 0 && cahce_info.filehash == hash_value &&
          mttime_value.size() > 0 && mttime_value == cahce_info.filetime)
      {
      }
      else
      {
        lp.createhfile(fileinh, method_item);

        memset(&sessfileinfo, 0, sizeof(sessfileinfo));
        if (stat(fileinh.c_str(), &sessfileinfo) == 0)
        {
          if (sessfileinfo.st_mode & S_IFREG)
          {
            mttime_value = std::to_string(sessfileinfo.st_mtime);
          }
        }

        cahce_info.filename    = filename;
        cahce_info.filetime    = mttime_value;
        cahce_info.filehash    = hash_value;
        reginfo_list[filename] = cahce_info;
        is_must_again          = true;
      }

      for (unsigned int j = 0; j < method_item.size(); j++)
      {
        reg_method_lists.emplace_back(method_item[j]);
      }
      filename.append(".h\"\r\n");
      header_lists.emplace_back(filename);
    }
  }

  lp.savecacheinfo(cache_file_path, reginfo_list);

  current_method_file = current_run_path + "common/autocontrolmethod.hpp";

  std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(
      std::fopen(current_method_file.c_str(), "wb"), &std::fclose);

  if (!fp.get())
  {
    return 0;
  }

  std::string automethod_content;

  automethod_content = R"(
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

)";

  for (unsigned int j = 0; j < header_lists.size(); j++)
  {
    automethod_content.append("#include \"");
    automethod_content.append(header_lists[j]);
  }

  automethod_content.append(R"(

namespace http
{
  void _initauto_control_httpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
  {
    struct regmethold_t temp;

)");

  for (unsigned int j = 0; j < reg_method_lists.size(); j++)
  {
    if (strcasecmp(reg_method_lists[j].pre.c_str(), "null") == 0 ||
        strcasecmp(reg_method_lists[j].pre.c_str(), "nullptr") == 0)
    {
      automethod_content.append("\t\ttemp.pre = nullptr;\r\n");
    }
    else
    {
      automethod_content.append("\t\ttemp.pre = ");
      automethod_content.append(reg_method_lists[j].pre);
      automethod_content.append(";\r\n");
    }
    automethod_content.append("\t\ttemp.regfun = ");
    automethod_content.append(reg_method_lists[j].func);
    automethod_content.append(";\r\n");

    automethod_content.append("\t\tmethodcallback.emplace(\"");
    automethod_content.append(reg_method_lists[j].urlpath);
    automethod_content.append("\",temp);\r\n");
  }

  automethod_content.append(R"(

    }
}

#endif

    )");
  fwrite(&automethod_content[0], 1, automethod_content.size(), fp.get());
  if (is_must_again)
  {
    std::cout << "\033[1m\033[31m You must cmake and make again because the .h "
                 "header file has changed. \033[0m"
              << std::endl;
  }

  return 0;
}