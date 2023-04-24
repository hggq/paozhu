/*
 *  @author 黄自权 huangziquan
 *  @date 2023-04-24
 *  @dest struct json reflect file
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

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include "typestatement.hpp"

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

int jsoncli()
{
  namespace fs = std::filesystem;
  typejsonfiles tjf;
  fs::path current_path        = fs::current_path();
  std::string current_run_path = current_path.string();

  if (current_run_path.back() != '/')
  {
    current_run_path.push_back('/');
  }
  std::string filepath = current_run_path + "libs";
  auto filist          = tjf.readallfile(filepath);

  std::string jsonreflect_headers_content;
  std::vector<std::string> filelists;
  unsigned int ifindex = 1;
  for (unsigned int i = 0; i < filist.size(); i++)
  {
    std::string tempfile = filepath + "/" + filist[i] + ".h";
    auto fa              = tjf.pickreflectfile(tempfile);
    // 取出所有 对象 命名空间 kv 方式 留作 生成json_encode json_decode使用
    std::string tempcppfile = filepath + "/" + filist[i] + "_jsonreflect.cpp";
    std::string headerincludefile = "#include \"" + filist[i] + ".h\"";
    filelists.push_back(headerincludefile);
    if (fa.size() > 0)
    {
      std::cout << ifindex << " " << tempcppfile << std::endl;
      ifindex += 1;
      tjf.createhfile(tempcppfile, fa, headerincludefile);
      jsonreflect_headers_content += tjf.get_jsonreflect_header(fa);
    }
    else
    {
      struct stat sessfileinfo;
      memset(&sessfileinfo, 0, sizeof(sessfileinfo));
      if (stat(tempcppfile.c_str(), &sessfileinfo) == 0)
      {
        if (sessfileinfo.st_mode & S_IFREG)
        {
          remove(tempcppfile.c_str());
        }
      }
    }
  }

  /////////////////
  fs::path include_jsontype_path = current_run_path + "include/types";
  filist.clear();
  filepath = include_jsontype_path.string();
  if (fs::exists(include_jsontype_path))
  {
    fs::path src_jsontype_path = current_run_path + "src/types";
    if (!fs::exists(src_jsontype_path))
    {
      std::string path_temp  = current_run_path;
      std::string pathname   = "src/types/";
      unsigned int path_size = pathname.size();
      for (unsigned int j = 0; j < path_size; j++)
      {
        if (pathname[j] == '/' && j > 0 && path_temp.size() > 0)
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
    filist = tjf.readallfile(include_jsontype_path);
    for (unsigned int i = 0; i < filist.size(); i++)
    {

      std::string tempfile = filepath + "/" + filist[i] + ".h";

      auto fa = tjf.pickreflectfile(tempfile);
      // 取出所有 对象 命名空间 kv 方式 留作 生成json_encode json_decode使用
      tempfile = src_jsontype_path.string();
      if (tempfile.back() != '/')
      {
        tempfile.push_back('/');
      }
      make_all_directory(tempfile, filist[i]);
      std::string tempcppfile = tempfile + filist[i] + "_jsonreflect.cpp";

      std::string headerincludefile = "#include \"" + filist[i] + ".h\"";
      filelists.push_back(headerincludefile);
      if (fa.size() > 0)
      {
        std::cout << ifindex << " " << tempcppfile << std::endl;
        ifindex += 1;
        tjf.createhfile(tempcppfile, fa, headerincludefile);
        jsonreflect_headers_content += tjf.get_jsonreflect_header(fa);
      }
      else
      {
        struct stat sessfileinfo;
        memset(&sessfileinfo, 0, sizeof(sessfileinfo));
        if (stat(tempcppfile.c_str(), &sessfileinfo) == 0)
        {
          if (sessfileinfo.st_mode & S_IFREG)
          {
            remove(tempcppfile.c_str());
          }
        }
      }
    }
  }
  std::string json_header_filename =
      current_run_path + "common/json_reflect_headers.h";
  tjf.createhjson_reflectfile(json_header_filename, jsonreflect_headers_content,
                              filelists);
  return 0;
}