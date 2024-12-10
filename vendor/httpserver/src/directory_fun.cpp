/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file directory_fun.cpp
 *  @date 2022-10
 *
 *  directory index
 *
 *
 */
#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include <memory>
#include <cstdio>
#include <algorithm>
#include <filesystem>
#include <string_view>
#include "directory_fun.h"
#include "unicode.h"
#include "func.h"

namespace fs = std::filesystem;
namespace http
{
std::string file_time_tostring(fs::file_time_type const &ftime)
{
    time_t systimeid;
    tm *lsystemTime;
    unsigned char timeformat;
    std::ostringstream tempchache;

    auto elapse = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::file_time_type::clock::now().time_since_epoch() - std::chrono::system_clock::now().time_since_epoch()).count();
    systimeid = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count()-elapse;
    //systimeid =std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch() - std::chrono::system_clock::now().time_since_epoch()).count();
    lsystemTime = localtime(&systimeid);

    tempchache.str("");
    tempchache << (lsystemTime->tm_year + 1900);
    tempchache << "-";
    timeformat = lsystemTime->tm_mon + 1;
    if (timeformat < 10)
    {
        tempchache << "0" << std::to_string(timeformat);
    }
    else
    {
        tempchache << std::to_string(timeformat);
    }
    tempchache << "-";

    if (lsystemTime->tm_mday < 10)
    {
        tempchache << "0" << std::to_string(lsystemTime->tm_mday);
    }
    else
    {
        tempchache << std::to_string(lsystemTime->tm_mday);
    }

    tempchache << " ";

    if (lsystemTime->tm_hour < 10)
    {
        tempchache << "0" << std::to_string(lsystemTime->tm_hour);
    }
    else
    {
        tempchache << std::to_string(lsystemTime->tm_hour);
    }

    tempchache << ":";

    if (lsystemTime->tm_min < 10)
    {
        tempchache << "0" << std::to_string(lsystemTime->tm_min);
    }
    else
    {
        tempchache << std::to_string(lsystemTime->tm_min);
    }

    tempchache << ":";

    if (lsystemTime->tm_sec < 10)
    {
        tempchache << "0" << std::to_string(lsystemTime->tm_sec);
    }
    else
    {
        tempchache << std::to_string(lsystemTime->tm_sec);
    }
    return tempchache.str();
}

std::string displaydirectory(std::string localpath, std::string urlpath, std::string dir_sort, std::string configpath)
{
    std::string senddatastring;

    std::string httpquestpath, parentpath, templatea, templateb;
    if (urlpath[0] != '/')
    {
        httpquestpath.push_back('/');
    }
    httpquestpath.append(urlpath);
    if (httpquestpath.size() > 0 && httpquestpath.back() != '/')
    {
        httpquestpath.push_back('/');
    }
    if (configpath.size() > 0 && configpath.back() != '/')
    {
        configpath.push_back('/');
    }

    senddatastring.clear();
    parentpath = configpath + "directory.html";
    FILE *ff   = fopen(parentpath.c_str(), "rb");
    if (ff)
    {
        fseek(ff, 0, SEEK_END);
        size_t size = ftell(ff);
        fseek(ff, 0, SEEK_SET);
        senddatastring.resize(size);
        auto nread = fread(&senddatastring[0], 1, size, ff);
        senddatastring.resize(nread);
        size_t beginpos = senddatastring.find("<!---directorycontentlist---->");
        if (beginpos != std::string::npos)
        {
            templatea = senddatastring.substr(0, beginpos);
            templateb = senddatastring.substr(
                beginpos + sizeof("<!---directorycontentlist---->"));
        }
        senddatastring = templatea;
        fclose(ff);
    }
    parentpath.clear();
    int i = httpquestpath.size();
    if (i > 2)
    {
        i -= 2;
    }
    for (; i > 1; i--)
    {
        if (httpquestpath[i] == '/')
        {
            for (int j = 0; j < i; j++)
            {
                parentpath.push_back(httpquestpath[j]);
            }
            break;
        }
    }
    fs::path path(localpath);

    senddatastring.append("<ul class=\"directorycontent\">");
    senddatastring.append("<li class=\"rowhead\">");
    senddatastring.append("<span class=\"filename\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("\">");

    senddatastring.append("Filename");
    senddatastring.append("</a></span>");
    senddatastring.append("<span class=\"filesize\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("?sort=size\">Size");
    senddatastring.append("</a></span>");
    senddatastring.append("<span class=\"time\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("?sort=time\">Time");

    senddatastring.append("</a></span>");
    senddatastring.append("</li>");

    if (parentpath.size() > 0)
    {

        senddatastring.append("<li class=\"directoryrow\">");
        senddatastring.append("<span class=\"filename\"><a href=\"");
        senddatastring.append(parentpath);
        senddatastring.append("\">");
        senddatastring.append("..");
        senddatastring.append("</a></span>");
        senddatastring.append("<span class=\"filesize\">-");
        senddatastring.append("</span>");
        senddatastring.append("<span class=\"time\">-");

        senddatastring.append("</span>");
        senddatastring.append("</li>");
    }

    std::vector<directory_file_info_t> fileslists;
    if (fs::exists(path) && fs::is_directory(path))
    {

        for (const auto &entry : fs::directory_iterator(path))
        {
            directory_file_info_t pathfilelists;
            auto filename = entry.path().filename();
            if (fs::is_regular_file(entry.status()))
            {
                pathfilelists.filename = filename.string();
#ifndef _WIN32
                pathfilelists.ext = entry.path().extension();
#else
                pathfilelists.ext = entry.path().extension().string();
#endif

                long long filesize = fs::file_size(entry);
                pathfilelists.size = filesize;
                pathfilelists.time = file_time_tostring(entry.last_write_time());
            }
            else if (fs::is_directory(entry.status()))
            {

                pathfilelists.filename = filename.string();
                pathfilelists.size     = 0;
                pathfilelists.time     = "-";
            }
            fileslists.emplace_back(pathfilelists);
        }
    }
    if (str_casecmp(dir_sort, "size"))
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.size < b.size; });
    }
    else if (str_casecmp(dir_sort, "time"))
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.time < b.time; });
    }
    else
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.filename < b.filename; });
    }

    for (unsigned int i = 0; i < fileslists.size(); i++)
    {
        senddatastring.append("<li class=\"directoryrow\">");
        if (fileslists[i].time.size() < 3)
        {
            senddatastring.append("<div class=\"filename\"><i class=\"bi bi-folder");
            senddatastring.append("\"></i> <a href=\"");
        }
        else
        {
            senddatastring.append(
                "<div class=\"filename\"><i class=\"bi bi-filetype-");

            if (fileslists[i].ext.size() > 0 && fileslists[i].ext[0] == '.')
            {
                senddatastring.append(fileslists[i].ext.substr(1));
            }
            else
            {
                senddatastring.append(fileslists[i].ext);
            }
            senddatastring.append("\"></i> <a href=\"");
        }

        senddatastring.append(httpquestpath);
        senddatastring.append(fileslists[i].filename);

        senddatastring.append("\">");
        senddatastring.append(fileslists[i].filename);
        senddatastring.append("</a></div>");
        senddatastring.append("<div class=\"filesize\">");
        if (fileslists[i].time.size() < 3)
        {
            senddatastring.push_back('-');
        }
        else
        {
            senddatastring.append(std::to_string(fileslists[i].size));
        }
        senddatastring.append("</div>");
        senddatastring.append("<div class=\"time\">");
        senddatastring.append(fileslists[i].time);
        senddatastring.append("</div>");
        senddatastring.append("</li>");
    }

    senddatastring.append("</ul>");
    senddatastring.append(templateb);

    auto beginpos = senddatastring.find("<!---directoryurl---->");
    if (beginpos != std::string::npos)
    {
        templatea = senddatastring.substr(0, beginpos);
        templateb =
            senddatastring.substr(beginpos + sizeof("<!---directoryurl---->") - 1);

        senddatastring = templatea;
        senddatastring.append(urlpath);
        senddatastring.append(templateb);
    }

    return senddatastring;
}

std::string json_encode(const directory_file_info_t &json_reflectobj)
{

    std::stringstream _stream;
    _stream << "{";
    _stream << "\"filename\":\"" << http::utf8_to_jsonstring(json_reflectobj.filename) << "\"";
    _stream << ",";
    _stream << "\"size\":" << std::to_string(json_reflectobj.size);
    _stream << ",";
    _stream << "\"time\":\"" << http::utf8_to_jsonstring(json_reflectobj.time) << "\"";
    _stream << ",";
    _stream << "\"ext\":\"" << http::utf8_to_jsonstring(json_reflectobj.ext) << "\"";
    _stream << ",";
    _stream << "\"url\":\"" << http::utf8_to_jsonstring(json_reflectobj.url) << "\"";

    _stream << "}";

    return _stream.str();
}

std::string json_encode(const std::vector<directory_file_info_t> &json_reflectobj)
{
    std::stringstream _stream;
    _stream << "[";

    for (unsigned int i = 0; i < json_reflectobj.size(); i++)
    {
        if (i > 0)
        {
            _stream << ",";
        }
        _stream << json_encode(json_reflectobj[i]);
    }

    _stream << "]";

    return _stream.str();
}

std::vector<directory_file_info_t> displaydirectory_attachfilejson(std::string localpath, std::string urlpath, std::string realurlpath, std::string deleteurlpath, std::string dir_sort, std::string configpath, bool isroot)
{
    std::string senddatastring;

    std::string httpquestpath, parentpath, templatea, templateb;
    if (urlpath[0] != '/')
    {
        httpquestpath.push_back('/');
    }
    httpquestpath.append(urlpath);
    if (httpquestpath.size() > 0 && httpquestpath.back() != '/')
    {
        httpquestpath.push_back('/');
    }

    if (realurlpath.size() == 0)
    {
        realurlpath.push_back('/');
    }
    if (realurlpath.size() > 0 && realurlpath.back() != '/')
    {
        realurlpath.push_back('/');
    }

    if (deleteurlpath.size() > 0 && deleteurlpath.back() != '/')
    {
        deleteurlpath.push_back('/');
    }

    if (configpath.size() > 0 && configpath.back() != '/')
    {
        configpath.push_back('/');
    }

    parentpath.clear();
    int i = httpquestpath.size();
    if (i > 2)
    {
        i -= 2;
    }
    for (; i > 1; i--)
    {
        if (httpquestpath[i] == '/')
        {
            for (int j = 0; j < i; j++)
            {
                parentpath.push_back(httpquestpath[j]);
            }
            break;
        }
    }
    fs::path path(localpath);
    std::vector<directory_file_info_t> fileslists;
    directory_file_info_t one_fileinfo;
    if (parentpath.size() > 0 && isroot)
    {
        one_fileinfo.filename = "..";
        one_fileinfo.size     = 0;
        one_fileinfo.time     = "-";
        one_fileinfo.ext      = "-";
        one_fileinfo.url      = parentpath;
        fileslists.emplace_back(one_fileinfo);
    }

    if (fs::exists(path) && fs::is_directory(path))
    {

        for (const auto &entry : fs::directory_iterator(path))
        {
            directory_file_info_t pathfilelists;
            auto filename = entry.path().filename();
            if (fs::is_regular_file(entry.status()))
            {
                pathfilelists.filename = filename.string();
#ifndef _WIN32
                pathfilelists.ext = entry.path().extension();
#else
                pathfilelists.ext = entry.path().extension().string();
#endif

                long long filesize = fs::file_size(entry);
                pathfilelists.size = filesize;
                pathfilelists.time = file_time_tostring(entry.last_write_time());
            }
            else if (fs::is_directory(entry.status()))
            {

                pathfilelists.filename = filename.string();
                pathfilelists.size     = 0;
                pathfilelists.time     = "-";
            }
            fileslists.emplace_back(pathfilelists);
        }
    }

    if (str_casecmp(dir_sort, "size"))
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.size < b.size; });
    }
    else if (str_casecmp(dir_sort, "time"))
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.time < b.time; });
    }
    else
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.filename < b.filename; });
    }
    //begin 1
    for (unsigned int i = 1; i < fileslists.size(); i++)
    {
        fileslists[i].url = realurlpath;
    }

    return fileslists;
}

std::string displaydirectory_attachfile(std::string localpath, std::string urlpath, std::string realurlpath, std::string deleteurlpath, std::string dir_sort, std::string configpath, bool isroot)
{
    std::string senddatastring;

    std::string httpquestpath, parentpath, templatea, templateb;
    if (urlpath[0] != '/')
    {
        httpquestpath.push_back('/');
    }
    httpquestpath.append(urlpath);
    if (httpquestpath.size() > 0 && httpquestpath.back() != '/')
    {
        httpquestpath.push_back('/');
    }

    if (realurlpath.size() == 0)
    {
        realurlpath.push_back('/');
    }
    if (realurlpath.size() > 0 && realurlpath.back() != '/')
    {
        realurlpath.push_back('/');
    }

    if (deleteurlpath.size() > 0 && deleteurlpath.back() != '/')
    {
        deleteurlpath.push_back('/');
    }

    if (configpath.size() > 0 && configpath.back() != '/')
    {
        configpath.push_back('/');
    }

    senddatastring.clear();
    parentpath = configpath + "directory.html";
    FILE *ff   = fopen(parentpath.c_str(), "rb");
    if (ff)
    {
        fseek(ff, 0, SEEK_END);
        size_t size = ftell(ff);
        fseek(ff, 0, SEEK_SET);
        senddatastring.resize(size);
        auto nread = fread(&senddatastring[0], 1, size, ff);
        senddatastring.resize(nread);
        size_t beginpos = senddatastring.find("<!---directorycontentlist---->");
        if (beginpos != std::string::npos)
        {
            templatea = senddatastring.substr(0, beginpos);
            templateb = senddatastring.substr(
                beginpos + sizeof("<!---directorycontentlist---->"));
        }
        senddatastring = templatea;
        fclose(ff);
    }
    parentpath.clear();
    int i = httpquestpath.size();
    if (i > 2)
    {
        i -= 2;
    }
    for (; i > 1; i--)
    {
        if (httpquestpath[i] == '/')
        {
            for (int j = 0; j < i; j++)
            {
                parentpath.push_back(httpquestpath[j]);
            }
            break;
        }
    }
    fs::path path(localpath);

    senddatastring.append("<ul class=\"directorycontent\">");
    senddatastring.append("<li class=\"rowhead\">");
    senddatastring.append("<span class=\"filename\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("\">");

    senddatastring.append("Filename");
    senddatastring.append("</a></span>");
    senddatastring.append("<span class=\"filesize\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("?sort=size\">Size");
    senddatastring.append("</a></span>");
    senddatastring.append("<span class=\"time\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("?sort=time\">Time");

    senddatastring.append("</a></span>");

    senddatastring.append("<span class=\"editfile\">");

    senddatastring.append(" Edit ");

    senddatastring.append(" </span>");

    senddatastring.append("</li>");

    if (parentpath.size() > 0 && isroot)
    {

        senddatastring.append("<li class=\"directoryrowheader\">");
        senddatastring.append("<span class=\"filename\"><a href=\"");
        senddatastring.append(parentpath);
        senddatastring.append("\">");
        senddatastring.append("..");
        senddatastring.append("</a></span>");
        senddatastring.append("<span class=\"filesize\">-");
        senddatastring.append("</span>");
        senddatastring.append("<span class=\"time\">-");

        senddatastring.append("</span>");
        senddatastring.append("<span class=\"editfile\">-");

        senddatastring.append("</span>");
        senddatastring.append("</li>");
    }

    std::vector<directory_file_info_t> fileslists;
    if (fs::exists(path) && fs::is_directory(path))
    {

        for (const auto &entry : fs::directory_iterator(path))
        {
            directory_file_info_t pathfilelists;
            auto filename = entry.path().filename();
            if (fs::is_regular_file(entry.status()))
            {
                pathfilelists.filename = filename.string();
#ifndef _WIN32
                pathfilelists.ext = entry.path().extension();
#else
                pathfilelists.ext = entry.path().extension().string();
#endif

                long long filesize = fs::file_size(entry);
                pathfilelists.size = filesize;
                pathfilelists.time = file_time_tostring(entry.last_write_time());
            }
            else if (fs::is_directory(entry.status()))
            {

                pathfilelists.filename = filename.string();
                pathfilelists.size     = 0;
                pathfilelists.time     = "-";
            }
            fileslists.emplace_back(pathfilelists);
        }
    }
    if (str_casecmp(dir_sort, "size"))
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.size < b.size; });
    }
    else if (str_casecmp(dir_sort, "time"))
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.time < b.time; });
    }
    else
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.filename < b.filename; });
    }

    for (unsigned int i = 0; i < fileslists.size(); i++)
    {
        if (fileslists[i].time.size() < 3)
        {
            senddatastring.append("<li class=\"directoryrowfolder\">");
            senddatastring.append("<div class=\"filename\"><i class=\"bi bi-folder");
            senddatastring.append("\"></i> <a href=\"");
        }
        else
        {
            senddatastring.append("<li class=\"directoryrowfile\">");
            senddatastring.append(
                "<div class=\"filename\">");
            senddatastring.append("<a href=\"");
        }

        if (fileslists[i].size > 0)
        {
            senddatastring.append(realurlpath);
            senddatastring.append(fileslists[i].filename);
            senddatastring.append("\" target=_blank>");
        }
        else
        {
            senddatastring.append(httpquestpath);
            senddatastring.append(fileslists[i].filename);
            senddatastring.append("\">");
        }

        senddatastring.append(fileslists[i].filename);
        senddatastring.append("</a></div>");
        senddatastring.append("<div class=\"filesize\">");
        if (fileslists[i].time.size() < 3)
        {
            senddatastring.push_back('-');
        }
        else
        {
            senddatastring.append(std::to_string(fileslists[i].size));
        }
        senddatastring.append("</div>");
        senddatastring.append("<div class=\"time\">");
        senddatastring.append(fileslists[i].time);
        senddatastring.append("</div>");

        if (fileslists[i].size > 0)
        {
            senddatastring.append("<div class=\"editfile\"> <a href=\"");
            senddatastring.append(deleteurlpath);
            senddatastring.append("?file=");
            senddatastring.append(fileslists[i].filename);
            senddatastring.append("\" onclick=\"return confirm('Delete this image?')\">Delete</a></div>");
        }
        else
        {
            senddatastring.append("<div class=\"editfile\">-</div>");
        }

        senddatastring.append("</li>");
    }

    senddatastring.append("</ul>");
    senddatastring.append(templateb);

    auto beginpos = senddatastring.find("<!---directoryurl---->");
    if (beginpos != std::string::npos)
    {
        templatea = senddatastring.substr(0, beginpos);
        templateb =
            senddatastring.substr(beginpos + sizeof("<!---directoryurl---->") - 1);

        senddatastring = templatea;
        senddatastring.append(realurlpath);
        senddatastring.append(templateb);
    }

    return senddatastring;
}

std::string displaydirectory_attachimg(std::string localpath, std::string urlpath, std::string realurlpath, std::string deleteurlpath, std::string dir_sort, std::string configpath, bool isroot)
{
    std::string senddatastring;

    std::string httpquestpath, parentpath, templatea, templateb;
    if (urlpath[0] != '/')
    {
        httpquestpath.push_back('/');
    }
    httpquestpath.append(urlpath);
    if (httpquestpath.size() > 0 && httpquestpath.back() != '/')
    {
        httpquestpath.push_back('/');
    }

    if (realurlpath.size() == 0)
    {
        realurlpath.push_back('/');
    }
    if (realurlpath.size() > 0 && realurlpath.back() != '/')
    {
        realurlpath.push_back('/');
    }

    if (deleteurlpath.size() > 0 && deleteurlpath.back() != '/')
    {
        deleteurlpath.push_back('/');
    }

    if (configpath.size() > 0 && configpath.back() != '/')
    {
        configpath.push_back('/');
    }

    senddatastring.clear();
    parentpath = configpath + "directory.html";
    FILE *ff   = fopen(parentpath.c_str(), "rb");
    if (ff)
    {
        fseek(ff, 0, SEEK_END);
        size_t size = ftell(ff);
        fseek(ff, 0, SEEK_SET);
        senddatastring.resize(size);
        auto nread = fread(&senddatastring[0], 1, size, ff);
        senddatastring.resize(nread);
        size_t beginpos = senddatastring.find("<!---directorycontentlist---->");
        if (beginpos != std::string::npos)
        {
            templatea = senddatastring.substr(0, beginpos);
            templateb = senddatastring.substr(
                beginpos + sizeof("<!---directorycontentlist---->"));
        }
        senddatastring = templatea;
        fclose(ff);
    }
    parentpath.clear();
    int i = httpquestpath.size();
    if (i > 2)
    {
        i -= 2;
    }
    for (; i > 1; i--)
    {
        if (httpquestpath[i] == '/')
        {
            for (int j = 0; j < i; j++)
            {
                parentpath.push_back(httpquestpath[j]);
            }
            break;
        }
    }
    fs::path path(localpath);

    senddatastring.append("<ul class=\"directorycontent\">");
    senddatastring.append("<li class=\"rowhead\">");
    senddatastring.append("<span class=\"filename\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("\">");

    senddatastring.append("Filename");
    senddatastring.append("</a></span>");
    senddatastring.append("<span class=\"filesize\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("?sort=size\">Size");
    senddatastring.append("</a></span>");
    senddatastring.append("<span class=\"time\"><a href=\"");
    senddatastring.append(urlpath);
    senddatastring.append("?sort=time\">Time");

    senddatastring.append("</a></span>");

    senddatastring.append("<span class=\"editfile\">");

    senddatastring.append(" Edit ");

    senddatastring.append(" </span>");

    senddatastring.append("</li>");

    if (parentpath.size() > 0 && isroot)
    {

        senddatastring.append("<li class=\"directoryrowheader\">");
        senddatastring.append("<span class=\"filename\"><a href=\"");
        senddatastring.append(parentpath);
        senddatastring.append("\">");
        senddatastring.append("..");
        senddatastring.append("</a></span>");
        senddatastring.append("<span class=\"filesize\">-");
        senddatastring.append("</span>");
        senddatastring.append("<span class=\"time\">-");

        senddatastring.append("</span>");
        senddatastring.append("<span class=\"editfile\">-");

        senddatastring.append("</span>");
        senddatastring.append("</li>");
    }

    std::vector<directory_file_info_t> fileslists;
    if (fs::exists(path) && fs::is_directory(path))
    {

        for (const auto &entry : fs::directory_iterator(path))
        {
            directory_file_info_t pathfilelists;
            auto filename = entry.path().filename();
            if (fs::is_regular_file(entry.status()))
            {
                pathfilelists.filename = filename.string();
#ifndef _WIN32
                pathfilelists.ext = entry.path().extension();
#else
                pathfilelists.ext = entry.path().extension().string();
#endif

                long long filesize = fs::file_size(entry);
                pathfilelists.size = filesize;
                pathfilelists.time = file_time_tostring(entry.last_write_time());
            }
            else if (fs::is_directory(entry.status()))
            {

                pathfilelists.filename = filename.string();
                pathfilelists.size     = 0;
                pathfilelists.time     = "-";
            }
            fileslists.emplace_back(pathfilelists);
        }
    }
    if (str_casecmp(dir_sort, "size"))
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.size < b.size; });
    }
    else if (str_casecmp(dir_sort, "time"))
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.time < b.time; });
    }
    else
    {
        std::sort(fileslists.begin(), fileslists.end(), [](directory_file_info_t const &a, directory_file_info_t const &b)
                  { return a.filename < b.filename; });
    }

    for (unsigned int i = 0; i < fileslists.size(); i++)
    {

        if (fileslists[i].filename.size() > 0 && fileslists[i].filename[0] == '.')
        {
            continue;
        }
        if (fileslists[i].time.size() < 3 || (fileslists[i].filename.size() > 0 && fileslists[i].filename[0] == '.'))
        {
            senddatastring.append("<li class=\"directoryrowfolder\">");
            senddatastring.append("<div class=\"filename\"><i class=\"bi bi-folder");
            senddatastring.append("\"></i> <a href=\"");
        }
        else
        {
            senddatastring.append("<li class=\"directoryrowfile\">");
            senddatastring.append(
                "<div class=\"filename\">");
            senddatastring.append("<a href=\"");
        }

        if (fileslists[i].size > 0)
        {
            if (str_casecmp(fileslists[i].ext, ".png") || str_casecmp(fileslists[i].ext, ".jpg") || str_casecmp(fileslists[i].ext, ".gif") || str_casecmp(fileslists[i].ext, ".jpeg"))
            {
                senddatastring.append(realurlpath);
                senddatastring.append(fileslists[i].filename);
                senddatastring.append("\" target=_blank><img src=\"");
                senddatastring.append(realurlpath);
                senddatastring.append(fileslists[i].filename);
                senddatastring.append("\" width=\"60\" height=\"60\"/>");
            }
            else
            {
                senddatastring.append(httpquestpath);
                senddatastring.append(fileslists[i].filename);
                senddatastring.append("\">");
            }
        }
        else
        {
            senddatastring.append(httpquestpath);
            senddatastring.append(fileslists[i].filename);
            senddatastring.append("\">");
        }

        senddatastring.append(fileslists[i].filename);
        senddatastring.append("</a></div>");
        senddatastring.append("<div class=\"filesize\">");
        if (fileslists[i].time.size() < 3)
        {
            senddatastring.push_back('-');
        }
        else
        {
            senddatastring.append(std::to_string(fileslists[i].size));
        }
        senddatastring.append("</div>");
        senddatastring.append("<div class=\"time\">");
        senddatastring.append(fileslists[i].time);
        senddatastring.append("</div>");

        if (fileslists[i].size > 0)
        {
            senddatastring.append("<div class=\"editfile\"> <a href=\"");
            senddatastring.append(deleteurlpath);
            senddatastring.append("?file=");
            senddatastring.append(fileslists[i].filename);
            senddatastring.append("\" onclick=\"return confirm('Delete this image?')\">Delete</a></div>");
        }
        else
        {
            senddatastring.append("<div class=\"editfile\">-</div>");
        }

        senddatastring.append("</li>");
    }

    senddatastring.append("</ul>");
    senddatastring.append(templateb);

    auto beginpos = senddatastring.find("<!---directoryurl---->");
    if (beginpos != std::string::npos)
    {
        templatea = senddatastring.substr(0, beginpos);
        templateb =
            senddatastring.substr(beginpos + sizeof("<!---directoryurl---->") - 1);

        senddatastring = templatea;
        senddatastring.append(realurlpath);
        senddatastring.append(templateb);
    }

    return senddatastring;
}

}// namespace http