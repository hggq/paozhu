#include <algorithm>
#include <asio.hpp>
// #include <asio/ssl.hpp>
// #include <asio/co_spawn.hpp>
// #include <asio/detached.hpp>
// #include <asio/io_context.hpp>
// #include <asio/ip/tcp.hpp>
// #include <asio/signal_set.hpp>
// #include <asio/write.hpp>
#include <cstdio>
#include <list>
#include <map>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <memory>
#include <cstdio>
#include <sstream>
#include <filesystem>
#include <sys/fcntl.h>
#include "datetime.h"
#include "urlcode.h"

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include "httpclient.h"
#include "func.h"

namespace http
{
namespace fs = std::filesystem;
bool str_compare(const std::string &str1, std::string &str2, unsigned int length)
{
    for (unsigned int i = 0; i < length; i++)
    {
        if (i < str1.size() && i < str2.size())
        {
            if (str1[i] == str2[i])
            {
                continue;
            }
            else
            {
                return false;
            }
        }
        if (i == str1.size())
        {
            return true;
        }
        if (i == str2.size())
        {
            return true;
        }
        return false;
    }
    return true;
}
void get_filename(const std::string &filename, std::string &filename_name, std::string &filename_ext)
{

    int j = filename.size() - 1;
    if (j == -1)
        return;
    for (; j >= 0; j--)
    {
        if (filename[j] == '.')
        {
            if (filename_name.size() == 2 && filename_name[0] == 'z' && filename_name[1] == 'g')
            {
                filename_name.push_back('.');
                continue;
            }
            filename_ext = filename_name;
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
    std::reverse(filename_ext.begin(), filename_ext.end());
}
std::string get_filename(const std::string &filename)
{
    std::string filename_name;
    if (filename.empty())
    {
        return "";
    }
    int j = filename.size() - 1;
    if (j == -1)
        return "";
    for (; j >= 0; j--)
    {
        if (filename[j] == '.')
        {
            if (filename_name.size() == 2 && filename_name[0] == 'z' && filename_name[1] == 'g')
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
std::vector<std::string> mb_split(std::string pattern, std::string &msg)
{
    std::vector<std::string> temp;

    std::string mp;
    unsigned int j = 0, n = msg.size();
    for (unsigned int i = 0; i < n; i++)
    {
        for (j = 0; j < pattern.size(); j++)
        {
            if ((i + j) >= n)
            {
                break;
            }
            if (pattern[j] != msg[i + j])
            {
                break;
            }
        }
        if (j == pattern.size())
        {
            temp.emplace_back(mp);
            mp.clear();
            i = i + j - 1;
            continue;
        }
        mp.push_back(msg[i]);
    }
    if (mp.size() > 0)
    {
        temp.emplace_back(mp);
    }
    return temp;
}
int mb_strlen(std::string &str)
{
    int length             = 0;
    unsigned long long pos = 0;
    unsigned char c;
    for (; pos < str.size(); pos++)
    {
        c = (unsigned char)str[pos];
        if (c < 0x80)
        {
            length++;
        }
        else if (c < 0xC0)
        {
            length++;
        }
        else if (c >= 0xC0 && c < 0xE0)
        {
            pos += 1;
            length++;
        }
        else if (c >= 0xE0 && c < 0xF0)
        {
            pos += 2;
            length++;
        }
        else if (c >= 0xF0 && c < 0xF8)
        {
            pos += 3;
            length++;
        }
        else
        {
            length++;
        }
    }
    return length;
}
std::string mb_substr(std::string &str, int begin, int length)
{
    std::string temp;
    int strlength = 0;
    // int spacenum = 0;
    unsigned char c;
    if (begin < 0)
    {
        if (length == 0)
        {
            strlength = mb_strlen(str);
            int n     = strlength + begin;
            if (n > strlength)
            {
                n = strlength;
            }
            if (n < 0)
            {
                n = 0;
            }

            int offsetnum = 0;
            for (unsigned int pos = 0; pos < str.size(); pos++)
            {
                c = (unsigned char)str[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                        temp.push_back(str[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
            }

            return temp;
        }
        else if (length < 0)
        {
            strlength = mb_strlen(str);
            int j     = strlength + length;
            if (j > strlength)
            {
                j = strlength;
            }
            if (j < 0)
            {
                j = 0;
            }
            int n = strlength + begin;
            if (n > strlength)
            {
                n = strlength;
            }
            if (n < 0)
            {
                n = 0;
            }

            if (n >= j)
            {
                return temp;
            }
            int offsetnum = 0;
            for (unsigned int pos = 0; pos < str.size(); pos++)
            {
                c = (unsigned char)str[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                        temp.push_back(str[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                if (offsetnum >= j)
                {
                    break;
                }
            }
            return temp;
        }
        else
        {
            strlength = mb_strlen(str);

            int n = strlength + begin;
            if (n > strlength)
            {
                n = strlength;
            }
            if (n < 0)
            {
                n = 0;
            }

            int j = n + length;
            if (j > strlength)
            {
                j = strlength;
            }
            if (j < 0)
            {
                j = 0;
            }
            if (n >= j)
            {
                return temp;
            }

            int offsetnum = 0;
            for (unsigned int pos = 0; pos < str.size(); pos++)
            {
                c = (unsigned char)str[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                        temp.push_back(str[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                if (offsetnum >= j)
                {
                    break;
                }
            }

            return temp;
        }
    }
    else
    {
        if (length == 0)
        {

            int offsetnum = 0;
            int n         = begin;
            for (unsigned int pos = 0; pos < str.size(); pos++)
            {
                c = (unsigned char)str[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                        temp.push_back(str[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
            }

            return temp;
        }
        else if (length < 0)
        {
            strlength = mb_strlen(str);
            int j     = strlength + length;
            if (j < 0)
            {
                j = 0;
            }
            if (begin > strlength)
            {
                begin = strlength;
            }

            int n = j;
            j     = begin;

            int offsetnum = 0;
            for (unsigned int pos = 0; pos < str.size(); pos++)
            {
                c = (unsigned char)str[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                        temp.push_back(str[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                if (offsetnum >= j)
                {
                    break;
                }
            }

            return temp;
        }
        else
        {
            strlength = mb_strlen(str);

            if (begin > strlength)
            {
                begin = strlength;
            }
            int j = begin + length;
            if (j > strlength)
            {
                j = strlength;
            }
            int n         = begin;
            int offsetnum = 0;
            for (unsigned int pos = 0; pos < str.size(); pos++)
            {
                c = (unsigned char)str[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                        temp.push_back(str[pos + 1]);
                        temp.push_back(str[pos + 2]);
                        temp.push_back(str[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(str[pos]);
                    }
                    offsetnum++;
                }
                if (offsetnum >= j)
                {
                    break;
                }
            }
            return temp;
        }
    }

    return temp;
}
std::string file_get_contents(std::string str, std::map<std::string, std::string> &parabody, unsigned int timeoutnum)
{
    std::string file_body;
    bool isurl = false;
    unsigned i = 0;
    if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == ':' &&
        str[i + 5] == '/' && str[i + 6] == '/')
    {

        isurl = true;
    }
    else if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == 's' &&
             str[i + 5] == ':' && str[i + 6] == '/' && str[i + 7] == '/')
    {
        isurl = true;
    }

    if (isurl)
    {
        http::client a;
        // http::OBJ_VALUE parameter;
        bool isaccept = false;
        bool isget    = true;
        for (auto [hkey, vvalue] : parabody)
        {
            if (hkey == "Content-Type")
            {
                a.setheader("Content-Type", vvalue);
            }
            else if (hkey == "Post-Type")
            {
                a.posttype(vvalue);
            }
            else if (hkey == "User-Agent")
            {
                a.setheader("User-Agent", vvalue);
            }
            else if (hkey == "Accept")
            {
                a.setheader("Accept", vvalue);
                isaccept = true;
            }
            else if (hkey == "method")
            {
                if (vvalue == "POST")
                {
                    isget = false;
                }
            }
            else if (hkey == "header-content")
            {
                a.addheader(vvalue);
            }
            else
            {
                a.data[hkey] = vvalue;
            }
        }

        if (!isaccept)
        {
            a.setheader("Accept",
                        "text/html, application/xhtml+xml, application/json, application/xml;q=0.9, */*;q=0.8");
        }
        if (isget)
        {
            a.get(str);
        }
        else
        {
            a.post(str);
        }
        if (timeoutnum > 1)
        {
            a.timeout(timeoutnum);
        }

        // a.data=parameter;
        a.send();
        parabody["state"]           = std::to_string(a.getstate());
        parabody["response-header"] = a.getheader();
        parabody["content-length"]  = std::to_string(a.getlength());
        if (a.getstate() == 200)
        {
            if (a.getlength() < 33554432)
            {
                return a.getbody();
            }
            else
            {
                return a.gettempfile();
            }
        }
        else
        {
            return a.getbody();
        }
    }
    else
    {

        FILE *ffp = fopen(str.c_str(), "rb");
        if (!ffp)
        {
            return file_body;
        }
        fseek(ffp, 0, SEEK_END);
        unsigned int nsize = ftell(ffp);
        fseek(ffp, 0, SEEK_SET);

        file_body.resize(nsize);

        unsigned int nread = fread(&file_body[0], 1, nsize, ffp);
        file_body.resize(nread);
        fclose(ffp);
    }

    return file_body;
}
std::string file_get_contents(std::string str, unsigned int timeoutnum)
{
    std::string file_body;
    bool isurl = false;
    unsigned i = 0;
    if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == ':' &&
        str[i + 5] == '/' && str[i + 6] == '/')
    {

        isurl = true;
    }
    else if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == 's' &&
             str[i + 5] == ':' && str[i + 6] == '/' && str[i + 7] == '/')
    {
        isurl = true;
    }

    if (isurl)
    {
        http::client a;
        a.get(str);
        if (timeoutnum > 1)
        {
            a.timeout(timeoutnum);
        }

        a.send();
        if (a.getstate() == 200)
        {
            if (a.getlength() < 33554432)
            {
                return a.getbody();
            }
            else
            {
                return a.gettempfile();
            }
        }
        else
        {
            return a.getheader();
        }
    }
    else
    {

        FILE *ffp = fopen(str.c_str(), "rb");
        if (!ffp)
        {
            return file_body;
        }
        fseek(ffp, 0, SEEK_END);
        unsigned int nsize = ftell(ffp);
        fseek(ffp, 0, SEEK_SET);

        file_body.resize(nsize);

        unsigned int nread = fread(&file_body[0], 1, nsize, ffp);
        file_body.resize(nread);
        fclose(ffp);
    }

    return file_body;
}
bool file_put_contents(std::string str, std::string &body, bool append)
{
    bool issuccess = false;
    FILE *ffp;
    if (append)
    {
        ffp = fopen(str.c_str(), "ab");
    }
    else
    {
        ffp = fopen(str.c_str(), "wb");
    }

    if (!ffp)
    {
        return issuccess;
    }

    auto nsize = fwrite(&body[0], body.size(), 1, ffp);
    fclose(ffp);
    if (nsize > 0)
    {
        issuccess = true;
    }
    return issuccess;
}
bool file_put_contents(std::string str, const char *body, unsigned int length, bool append)
{
    bool issuccess = false;
    FILE *ffp;
    if (append)
    {
        ffp = fopen(str.c_str(), "ab");
    }
    else
    {
        ffp = fopen(str.c_str(), "wb");
    }

    if (!ffp)
    {
        return issuccess;
    }

    auto nsize = fwrite(body, length, 1, ffp);
    fclose(ffp);
    if (nsize > 0)
    {
        issuccess = true;
    }
    return issuccess;
}
struct stat filestat(std::string &file_name)
{
    struct stat finfo;
    if (stat(file_name.c_str(), &finfo) == 0)
    {
    }
    return finfo;
}
std::map<std::string, std::string> filepath(std::string &str)
{
    std::map<std::string, std::string> temp;
    std::vector<std::string> vpath;
    bool isext     = false;
    bool isurl     = false;
    unsigned int i = 0;
    for (; i < str.size(); i++)
    {
        if (str[i] == 0x20 || str[i] == 0x09 || str[i] == 0x0D || str[i] == 0x0A)
        {
            continue;
        }
        break;
    }
    if (str[i] == 'h')
    {
        if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == ':' &&
            str[i + 5] == '/' && str[i + 6] == '/')
        {
            temp["scheme"] = "http";
            bool isip6     = false;
            bool isport    = false;
            std::string temphost;
            std::string tempport;
            unsigned int j = i + 7;
            for (; j < str.size(); j++)
            {
                if (str[j] == '/')
                {
                    break;
                }
                if (str[j] == '[')
                {
                    isip6 = true;
                    continue;
                }
                if (str[j] == ']')
                {
                    isip6 = false;
                    continue;
                }
                if (isip6)
                {
                    temphost.push_back(str[j]);
                }
                else
                {
                    if (str[j] == ':')
                    {
                        isport = true;
                        continue;
                    }
                    if (isport)
                    {
                        tempport.push_back(str[j]);
                    }
                    else
                    {
                        temphost.push_back(str[j]);
                    }
                }
            }
            temp["host"] = temphost;
            if (tempport.size() > 0)
            {
                temp["port"] = tempport;
            }
            i     = j;
            isurl = true;
        }
        else if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == 's' &&
                 str[i + 5] == ':' && str[i + 6] == '/' && str[i + 7] == '/')
        {
            temp["scheme"] = "https";
            bool isip6     = false;
            bool isport    = false;
            std::string temphost;
            std::string tempport;
            unsigned int j = i + 8;
            for (; j < str.size(); j++)
            {
                if (str[j] == '/')
                {
                    break;
                }
                if (str[j] == '[')
                {
                    isip6 = true;
                    continue;
                }
                if (str[j] == ']')
                {
                    isip6 = false;
                    continue;
                }
                if (isip6)
                {
                    temphost.push_back(str[j]);
                }
                else
                {
                    if (str[j] == ':')
                    {
                        isport = true;
                        continue;
                    }
                    if (isport)
                    {
                        tempport.push_back(str[j]);
                    }
                    else
                    {
                        temphost.push_back(str[j]);
                    }
                }
            }
            temp["host"] = temphost;
            if (tempport.size() > 0)
            {
                temp["port"] = tempport;
            }
            i     = j;
            isurl = true;
        }
    }

    if (str[i] == '/')
    {
        temp["dirname"] = "/";
    }
    for (; i < str.size(); i++)
    {
        if (isurl)
        {
            if (str[i] == '#')
            {
                break;
            }
            if (str[i] == '?')
            {
                for (unsigned int j = i + 1; j < str.size(); j++)
                {
                    if (str[j] == '#')
                    {
                        break;
                    }
                    temp["querystring"].push_back(str[j]);
                }
                break;
            }
        }
        if (str[i] == '/')
        {
            if (temp["basename"].size() > 0)
            {
                if (temp["basename"].size() == 1 && temp["basename"][0] == '.')
                {
                }
                else if (temp["basename"].size() == 2 && temp["basename"] == "..")
                {

                    if (vpath.size() > 0)
                    {
                        vpath.pop_back();
                    }
                }
                else
                {
                    vpath.push_back(temp["basename"]);
                }
            }
            temp["basename"]  = "";
            temp["extension"] = "";
            temp["filename"]  = "";
            isext             = false;

            continue;
        }
        temp["basename"].push_back(str[i]);
        if (str[i] == '.')
        {
            if (isext)
            {
                temp["filename"].push_back('.');
                temp["filename"].append(temp["extension"]);
                temp["extension"] = "";
            }
            isext = true;
            continue;
        }

        if (isext)
        {
            temp["extension"].push_back(str[i]);
        }
        else
        {
            temp["filename"].push_back(str[i]);
        }
    }
    if (vpath.size() > 0)
    {
        for (unsigned int j = 0; j < vpath.size(); j++)
        {
            if (j > 0)
            {
                temp["dirname"].push_back('/');
            }
            temp["dirname"].append(vpath[j]);
        }
    }
    return temp;
}
std::string mb_trim(std::string &str)
{
    std::string temp;
    unsigned int tlen = str.size();
    for (; tlen > 0; tlen--)
    {
        if (str[tlen - 1] == 0x20 || str[tlen - 1] == 0x09 || str[tlen - 1] == 0x0A || str[tlen - 1] == 0x0D)
        {
            continue;
        }
        break;
    }
    unsigned int i = 0;
    for (; i < tlen; i++)
    {
        if (str[i] == 0x20 || str[i] == 0x09 || str[i] == 0x0A || str[i] == 0x0D)
        {
            continue;
        }
        break;
    }
    for (; i < tlen; i++)
    {
        temp.push_back(str[i]);
    }
    return temp;
}
std::string html_encode(std::string &str)
{
    std::string temp;
    for (unsigned int i = 0; i < str.size(); i++)
    {
        if (str[i] == '&')
        {
            temp.append("&amp;");
            continue;
        }
        else if (str[i] == 0x22)
        {
            // double quote
            temp.append("&quot;");
            continue;
        }
        else if (str[i] == 0x27)
        {
            // single quote &#039; ie not support &apos;
            temp.append("&apos;");
            continue;
        }
        else if (str[i] == 0x3C)
        {
            //<
            temp.append("&lt;");
            continue;
        }
        else if (str[i] == 0x3E)
        {
            //>
            temp.append("&gt;");
            continue;
        }
        else
        {
            temp.push_back(str[i]);
        }
    }
    return temp;
}
long long str2int(const char *source, unsigned int str_length)
{
    long long temp  = 0;
    unsigned int qi = 0;
    bool issub      = false;
    for (; qi < str_length; qi++)
    {
        if (source[qi] != 0x20)
        {
            break;
        }
    }
    if (source[qi] == '-')
    {
        issub = true;
        qi++;
    }
    for (; qi < str_length; qi++)
    {
        if (source[qi] < 0x3A && source[qi] > 0x2F)
        {
            temp = temp * 10 + (source[qi] - 0x30);
        }
    }
    if (issub)
    {
        temp = 0 - temp;
    }
    return temp;
}
std::string str2safepath(const char *source, unsigned int str_length)
{
    std::string temp;
    for (unsigned int i = 0; i < str_length; i++)
    {

        if ((source[i] > 0x2F && source[i] < 0x3A) || source[i] == '(' || source[i] == ')' || source[i] == '~' ||
            source[i] == '_' || source[i] == '-' || (source[i] > 0x40 && source[i] < 0x5B) ||
            (source[i] > 0x60 && source[i] < 0x7B))
        {
            temp.push_back(source[i]);
        }
    }
    return temp;
}
std::string str2safefile(const char *source, unsigned int str_length)
{
    std::string temp;
    for (unsigned int i = 0; i < str_length; i++)
    {

        if ((source[i] > 0x2F && source[i] < 0x3A) || source[i] == '.' || source[i] == '[' || source[i] == ']' ||
            source[i] == '(' || source[i] == ')' || source[i] == '~' || source[i] == '_' || source[i] == '-' ||
            (source[i] > 0x40 && source[i] < 0x5B) || (source[i] > 0x60 && source[i] < 0x7B))
        {
            temp.push_back(source[i]);
        }
    }
    return temp;
}
std::string str2safemethold(const char *source, unsigned int str_length)
{
    std::string temp;
    for (unsigned int i = 0; i < str_length; i++)
    {

        if ((source[i] > 0x2F && source[i] < 0x3A) || source[i] == '_' || (source[i] > 0x40 && source[i] < 0x5B) ||
            (source[i] > 0x60 && source[i] < 0x7B))
        {
            temp.push_back(source[i]);
        }
    }
    return temp;
}
std::string array_to_sql(const std::vector<std::string> a, char b, bool isquote)
{
    std::string temp;
    for (unsigned int j = 0; j < a.size(); j++)
    {
        if (j > 0)
        {
            temp.push_back(b);
        }
        if (isquote)
        {
            temp.append(a[j]);
        }
        else
        {
            temp.push_back('"');
            temp.append(str_addslash(a[j]));
            temp.push_back('"');
        }
    }
    return temp;
}
inline std::string str_addslash(const std::string &content)
{
    std::string temp;
    for (unsigned int i = 0; i < content.size(); i++)
    {
        if (content[i] == '\'')
        {
            temp.append("\\'");
            continue;
        }
        else if (content[i] == '"')
        {
            temp.append("\\\"");
            continue;
        }
        temp.push_back(content[i]);
    }
    return temp;
}
inline std::string json_addslash(const std::string &content)
{
    std::string temp;
    for (unsigned int i = 0; i < content.size(); i++)
    {
        if (content[i] == '"')
        {
            temp.append("\\\"");
            continue;
        }
        temp.push_back(content[i]);
    }
    return temp;
}

std::string strip_html(const std::string &content)
{
    std::string temp;
    std::string tempstr;
    unsigned int tag_count = 0;

    for (unsigned int i = 0; i < content.size(); i++)
    {
        if (content[i] == '<')
        {
            if (content[i + 1] > 0x20 && content[i + 1] < 0x7F)
            {
                i++;
                tempstr.clear();
                for (; i < content.size(); i++)
                {
                    if (content[i] == '>' || content[i] == ' ' || content[i] == '\t' || content[i] == 0x0D ||
                        content[i] == 0x0A)
                    {
                        break;
                    }
                    tempstr.push_back(content[i]);
                }

                if (strcasecmp(tempstr.c_str(), "style") == 0)
                {
                    for (; i < content.size(); i++)
                    {
                        if (content[i] == '>')
                        {
                            break;
                        }
                        if (content[i] == '"')
                        {
                            i++;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == '"' && content[i - 1] != 0x5C)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                        else if (content[i] == 0x27)
                        {
                            i++;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == 0x27 && content[i - 1] != 0x5C)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                    }
                    for (; i < content.size(); i++)
                    {
                        if (content[i] == '<' && content[i + 1] == 0x2F)
                        {
                            i += 2;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == '>' || content[i] == ' ' || content[i] == '\t' ||
                                    content[i] == 0x0D || content[i] == 0x0A)
                                {
                                    continue;
                                }
                                break;
                            }
                            tempstr.clear();
                            for (; i < content.size(); i++)
                            {
                                if ((content[i] > 0x40 && content[i] < 0x5B) ||
                                    (content[i] > 0x60 && content[i] < 0x7B))
                                {
                                    tempstr.push_back(content[i]);
                                    continue;
                                }
                                break;
                            }
                            if (strcasecmp(tempstr.c_str(), "style") == 0)
                            {
                                for (; i < content.size(); i++)
                                {
                                    if (content[i] == ' ' || content[i] == '\t')
                                    {
                                        continue;
                                    }
                                    break;
                                }
                                break;
                            }
                        }
                        if (content[i] == '"')
                        {
                            i++;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == '"' && content[i - 1] != 0x5C)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                        else if (content[i] == 0x27)
                        {
                            i++;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == 0x27 && content[i - 1] != 0x5C)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                    }
                    continue;
                }
                else if (strcasecmp(tempstr.c_str(), "script") == 0)
                {
                    for (; i < content.size(); i++)
                    {
                        if (content[i] == '>')
                        {
                            i++;
                            break;
                        }
                        if (content[i] == '"')
                        {
                            i++;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == '"' && content[i - 1] != 0x5C)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                        else if (content[i] == 0x27)
                        {
                            i++;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == 0x27 && content[i - 1] != 0x5C)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                    }

                    for (; i < content.size(); i++)
                    {
                        if (content[i] == '<' && content[i + 1] == 0x2F)
                        {
                            i += 2;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == '>' || content[i] == ' ' || content[i] == '\t' ||
                                    content[i] == 0x0D || content[i] == 0x0A)
                                {
                                    continue;
                                }
                                break;
                            }

                            tempstr.clear();
                            for (; i < content.size(); i++)
                            {
                                if ((content[i] > 0x40 && content[i] < 0x5B) ||
                                    (content[i] > 0x60 && content[i] < 0x7B))
                                {
                                    tempstr.push_back(content[i]);
                                    continue;
                                }
                                break;
                            }

                            if (strcasecmp(tempstr.c_str(), "script") == 0)
                            {
                                for (; i < content.size(); i++)
                                {
                                    if (content[i] == ' ')
                                    {
                                        continue;
                                    }
                                    break;
                                }

                                break;
                            }
                        }

                        if (content[i] == '"')
                        {
                            i++;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == '"' && content[i - 1] != 0x5C)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                        else if (content[i] == 0x27)
                        {
                            i++;
                            for (; i < content.size(); i++)
                            {
                                if (content[i] == 0x27 && content[i - 1] != 0x5C)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                    }
                    continue;
                }

                tag_count = 0;
                // nornal tag
                for (; i < content.size(); i++)
                {
                    if (content[i] == '>')
                    {
                        if (tag_count == 0)
                        {
                            break;
                        }
                        else
                        {
                            tag_count -= 1;
                            continue;
                        }
                    }
                    if (content[i] == '"')
                    {
                        i++;
                        for (; i < content.size(); i++)
                        {
                            if (content[i] == '"' && content[i - 1] != 0x5C)
                            {
                                break;
                            }
                        }
                        continue;
                    }
                    if (content[i] == 0x27)
                    {
                        i++;
                        for (; i < content.size(); i++)
                        {
                            if (content[i] == 0x27 && content[i - 1] != 0x5C)
                            {
                                break;
                            }
                        }
                        continue;
                    }
                    if (content[i] == '<')
                    {
                        // may be script tag in tag
                        tag_count += 1;
                        continue;
                    }
                }
                continue;
            }
            else
            {
                continue;
            }
        }
        temp.push_back(content[i]);
    }

    return temp;
}
std::string strip_annot(const std::string &content)
{
    std::string temp;
    for (unsigned int i = 0; i < content.size(); i++)
    {
        if (content[i] == '/' && ((i + 1) < content.size()) && content[i + 1] == '/')
        {

            i += 2;
            for (; i < content.size(); i++)
            {
                if (content[i] == 0x0A)
                {
                    if (i < content.size() && content[i] == 0x0D)
                    {
                        i++;
                    }
                    break;
                }
            }
            continue;
        }
        else if (content[i] == '/' && ((i + 1) < content.size()) && content[i + 1] == '*')
        {
            i += 2;
            for (; i < content.size(); i++)
            {
                if (content[i] == '*')
                {
                    if ((i + 1) < content.size() && content[i + 1] == '/')
                    {
                        i += 1;
                        break;
                    }
                }
            }
            continue;
        }
        else if (content[i] == '"')
        {
            temp.push_back(content[i]);
            i++;
            for (; i < content.size(); i++)
            {
                temp.push_back(content[i]);
                if (content[i] == '"' && content[i - 1] != 0x5C)
                {
                    break;
                }
            }
            continue;
        }
        else if (content[i] == 0x27)
        {
            temp.push_back(content[i]);
            i++;
            for (; i < content.size(); i++)
            {
                temp.push_back(content[i]);
                if (content[i] == 0x27 && content[i - 1] != 0x5C)
                {
                    break;
                }
            }
            continue;
        }

        temp.push_back(content[i]);
    }
    return temp;
}

void get_directory_all_file(std::map<unsigned long long, std::string> &listobj,
                            const std::string &file_path,
                            const std::string &url_path,
                            const std::string &extfile)
{
    std::filesystem::path tagetpath = file_path;
    std::string extname;
    std::string filename;
    std::string urlname = url_path;
    std::string temp_str;
    if (urlname.size() > 0 && urlname.back() != '/')
    {
        urlname.push_back('/');
    }
    if (std::filesystem::exists(tagetpath) && std::filesystem::is_directory(tagetpath))
    {
        for (const auto &entry : std::filesystem::directory_iterator(tagetpath))
        {
            auto filename = entry.path().filename().string();
            if (std::filesystem::is_regular_file(entry.status()))
            {
                extname = entry.path().extension().string();
                if (extfile.size() == 1 && extfile[0] == '*')
                {
                    unsigned long long time_last_edit = 0;
                    time_last_edit =
                        std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch())
                            .count();
                    temp_str                = urlname + filename;
                    listobj[time_last_edit] = temp_str;
                }
                else if (filename.size() > 0 && filename[0] != '.')
                {
                    if (extname.size() > 0 && extfile.find(extname) != std::string::npos)
                    {
                        unsigned long long time_last_edit = 0;
                        time_last_edit =
                            std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch())
                                .count();
                        temp_str                = urlname + filename;
                        listobj[time_last_edit] = temp_str;
                    }
                }
            }
            else if (std::filesystem::is_directory(entry.status()))
            {
                std::string tempurlpath  = urlname + filename + "/";
                std::string tempfilepath = file_path + filename + "/";
                get_directory_all_file(listobj, tempfilepath, tempurlpath, extfile);
            }
        }
    }
}

std::string numstr_to_sql(const char *source, unsigned int str_length, char b)
{
    std::string tempt;
    std::string tempstr;

    for (unsigned int i = 0; i < str_length; i++)
    {
        if (source[i] == '-' || (source[i] > 0x2F && source[i] < 0x3A))
        {
            tempstr.push_back(source[i]);
        }
        else if (source[i] == b)
        {
            if (tempstr.size() > 0)
            {
                if (tempt.size() > 0)
                {
                    tempt.push_back(b);
                }
                tempt.append(tempstr);
            }
            tempstr.clear();
        }
    }
    if (tempstr.size() > 0)
    {
        if (tempt.size() > 0)
        {
            tempt.push_back(b);
        }
        tempt.append(tempstr);
    }
    return tempt;
}

}// namespace http