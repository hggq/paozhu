#include <algorithm>
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
#include <string_view>
#include "file_get_content.h"
#include "httpclient.h"
#include "func.h"
namespace http
{
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
        std::shared_ptr<client> a = std::make_shared<client>();
        // http::OBJ_VALUE parameter;
        bool isaccept = false;
        bool isget    = true;
        for (auto [hkey, vvalue] : parabody)
        {
            if (hkey == "Content-Type")
            {
                a->setheader("Content-Type", vvalue);
            }
            else if (hkey == "Post-Type")
            {
                a->posttype(vvalue);
            }
            else if (hkey == "User-Agent")
            {
                a->setheader("User-Agent", vvalue);
            }
            else if (hkey == "Accept")
            {
                a->setheader("Accept", vvalue);
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
                a->addheader(vvalue);
            }
            else
            {
                a->data[hkey] = vvalue;
            }
        }

        if (!isaccept)
        {
            a->setheader("Accept",
                         "text/html, application/xhtml+xml, application/json, application/xml;q=0.9, */*;q=0.8");
        }
        if (isget)
        {
            a->get(str);
        }
        else
        {
            a->post(str);
        }

        if (timeoutnum > 1)
        {
            a->timeout(timeoutnum);
        }
        // a->data=parameter;
        a->send();
        parabody["state"]           = std::to_string(a->getStatus());
        parabody["response-header"] = a->getHeader();
        parabody["content-length"]  = std::to_string(a->getLength());
        if (a->getStatus() == 200)
        {
            if (a->getLength() < 33554432)
            {
                return a->getBody();
            }
            else
            {
                return a->getTempfile();
            }
        }
        else
        {
            return a->getBody();
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
        std::shared_ptr<client> a = std::make_shared<client>();
        a->get(str);
        if (timeoutnum > 1)
        {
            a->timeout(timeoutnum);
        }
        a->send();
        if (a->getStatus() == 200)
        {
            if (a->getLength() < 33554432)
            {
                return a->getBody();
            }
            else
            {
                return a->getTempfile();
            }
        }
        else
        {
            return a->getHeader();
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
}