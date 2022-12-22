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
    void get_filename(const std::string &filename,std::string &filename_name,std::string &filename_ext)
    {

            int j = filename.size() - 1;
            if(j==-1) return;
            for (; j >= 0; j--)
            {
                if (filename[j] == '.')
                {
                    if(filename_name.size()==2&&filename_name[0]=='z'&&filename_name[1]=='g')
                    {
                        filename_name.push_back('.');
                        continue;
                    }
                    filename_ext=filename_name;
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
            int j = filename.size() - 1;
            if(j==-1) return "";
            for (; j >= 0; j--)
            {
                if (filename[j] == '.')
                {
                    if(filename_name.size()==2&&filename_name[0]=='z'&&filename_name[1]=='g')
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
        int j=0,n=msg.size();
        for (int i = 0; i < n; i++)
        {
            for(j=0;j<pattern.size();j++){
                if((i+j)>=n){
                    break;
                }
                if(pattern[j]!=msg[i+j]){
                    break;
                }
            }
            if(j==pattern.size()){
                temp.emplace_back(mp);
                mp.clear();
                i=i+j-1;
                continue;
            }
            mp.push_back(msg[i]);
        }
        if(mp.size()>0){
            temp.emplace_back(mp);
        }
        return temp;
    }
    int mb_strlen(std::string &str)
    {
        int length = 0;
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
                int n = strlength + begin;
                if (n > strlength)
                {
                    n = strlength;
                }
                if (n < 0)
                {
                    n = 0;
                }

                int offsetnum = 0;
                for (int pos = 0; pos < str.size(); pos++)
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
                int j = strlength + length;
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
                for (int pos = 0; pos < str.size(); pos++)
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
                for (int pos = 0; pos < str.size(); pos++)
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
                int n = begin;
                for (int pos = 0; pos < str.size(); pos++)
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
                int j = strlength + length;
                if (j < 0)
                {
                    j = 0;
                }
                if (begin > strlength)
                {
                    begin = strlength;
                }

                int n = j;
                j = begin;

                int offsetnum = 0;
                for (int pos = 0; pos < str.size(); pos++)
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
                int n = begin;
                int offsetnum = 0;
                for (int pos = 0; pos < str.size(); pos++)
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
    std::string file_get_contents(std::string str, std::map<std::string, std::string> &parabody)
    {
        std::string file_body;
        bool isurl = false;
        unsigned i = 0;
        if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == ':' && str[i + 5] == '/' && str[i + 6] == '/')
        {

            isurl = true;
        }
        else if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == 's' && str[i + 5] == ':' && str[i + 6] == '/' && str[i + 7] == '/')
        {
            isurl = true;
        }

        if (isurl)
        {
            http::client a;
            // http::OBJ_VALUE parameter;
            bool isaccept = false;
            bool isget = false;
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
                    isget = true;
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
                a.setheader("Accept", "text/html, application/xhtml+xml, application/json, application/xml;q=0.9, */*;q=0.8");
            }
            if (isget)
            {
                a.get(str);
            }
            else
            {
                a.post(str);
            }

            a.timeout(30);
            // a.data=parameter;
            a.send();
            parabody["state"]=std::to_string(a.getstate());
            parabody["response-header"]=a.getheader();
            parabody["content-length"]=std::to_string(a.getlength());
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
    std::string file_get_contents(std::string str)
    {
        std::string file_body;
        bool isurl = false;
        unsigned i = 0;
        if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == ':' && str[i + 5] == '/' && str[i + 6] == '/')
        {

            isurl = true;
        }
        else if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == 's' && str[i + 5] == ':' && str[i + 6] == '/' && str[i + 7] == '/')
        {
            isurl = true;
        }

        if (isurl)
        {
            http::client a;
            a.get(str);
            a.timeout(30);
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
    bool file_put_contents(std::string str,const char *body,unsigned int length, bool append)
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
        bool isext = false;
        bool isurl = false;
        int i = 0;
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
            if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == ':' && str[i + 5] == '/' && str[i + 6] == '/')
            {
                temp["scheme"] = "http";
                bool isip6 = false;
                bool isport = false;
                std::string temphost;
                std::string tempport;
                int j = i + 7;
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
                i = j;
                isurl = true;
            }
            else if (str[i] == 'h' && str[i + 1] == 't' && str[i + 2] == 't' && str[i + 3] == 'p' && str[i + 4] == 's' && str[i + 5] == ':' && str[i + 6] == '/' && str[i + 7] == '/')
            {
                temp["scheme"] = "https";
                bool isip6 = false;
                bool isport = false;
                std::string temphost;
                std::string tempport;
                int j = i + 8;
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
                i = j;
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
                    for (int j = i + 1; j < str.size(); j++)
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
                temp["basename"] = "";
                temp["extension"] = "";
                temp["filename"] = "";
                isext = false;

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
            for (int j = 0; j < vpath.size(); j++)
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
        int tlen = str.size();
        for (; tlen > 0; tlen--)
        {
            if (str[tlen - 1] == 0x20 || str[tlen - 1] == 0x09 || str[tlen - 1] == 0x0A || str[tlen - 1] == 0x0D)
            {
                continue;
            }
            break;
        }
        int i = 0;
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
        for (int i = 0; i < str.size(); i++)
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
        long long  temp = 0;
        int qi = 0;
        bool issub=false;
        for (; qi < str_length; qi++)
        {
            if (source[qi] != 0x20)
            {
                break;
            }    
        }
        if(source[qi]=='-')
        {
            issub=true;
            qi++;
        }
        for (; qi < str_length; qi++)
        {
            if (source[qi] < 0x3A && source[qi] > 0x2F)
            {
                temp = temp * 10 + (source[qi] - 0x30);
            }
        }
        if(issub)
        {
            temp=0-temp;
        }
        return temp;
    }
    std::string str2safepath(const char *source, unsigned int str_length)
    {
        std::string temp;
        for(int i=0;i<str_length;i++)
        {

            if((source[i]>0x2F&&source[i]<0x3A)||source[i]=='('||source[i]==')'||source[i]=='~'||source[i]=='_'||source[i]=='-'||(source[i]>0x40&&source[i]<0x5B)||(source[i]>0x60&&source[i]<0x7B))
            {
              temp.push_back(source[i]);
            }
        }
        return temp;
    }
    std::string str2safefile(const char *source, unsigned int str_length)
    {
        std::string temp;
        for(int i=0;i<str_length;i++)
        {

            if((source[i]>0x2F&&source[i]<0x3A)||source[i]=='.'||source[i]=='['||source[i]==']'||source[i]=='('||source[i]==')'||source[i]=='~'||source[i]=='_'||source[i]=='-'||(source[i]>0x40&&source[i]<0x5B)||(source[i]>0x60&&source[i]<0x7B))
            {
              temp.push_back(source[i]);
            }
        }
        return temp;
    }
    std::string str2safemethold(const char *source, unsigned int str_length)
    {
        std::string temp;
        for(int i=0;i<str_length;i++)
        {

            if((source[i]>0x2F&&source[i]<0x3A)||source[i]=='_'||(source[i]>0x40&&source[i]<0x5B)||(source[i]>0x60&&source[i]<0x7B))
            {
              temp.push_back(source[i]);
            }
        }
        return temp;
    }
}