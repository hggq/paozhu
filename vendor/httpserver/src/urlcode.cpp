#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <functional>
#include <map>
#include <utility>
#include <string>
#include <string_view>
#include <thread>
#include <chrono>
#include "urlcode.h"
namespace http
{

    std::string url_decode(const char *str, int len)
    {
        std::string url;
        const char *p = str;
        unsigned char word[2];
        char zifu;
        for (int i = 0; i < len; i++)
        {
            if (p[i] == '%' && p[i + 1] != '%')
            {
                word[0] = p[i + 1];
                word[1] = p[i + 2];
                if (word[0] >= 'A' && word[0] <= 'Z')
                {
                    word[0] = word[0] - 'A' + 10;
                }
                else if (word[0] >= 'a' && word[0] <= 'z')
                {
                    word[0] = word[0] - 'a' + 10;
                }
                else if (word[0] >= '0' && word[0] <= '9')
                {
                    word[0] = word[0] - '0';
                }
                else
                {
                    word[0] = 0x00;
                }

                if (word[1] >= 'A' && word[1] <= 'Z')
                {
                    word[1] = word[1] - 'A' + 10;
                }
                else if (word[1] >= 'a' && word[1] <= 'z')
                {
                    word[1] = word[1] - 'a' + 10;
                }
                else if (word[1] >= '0' && word[1] <= '9')
                {
                    word[1] = word[1] - '0';
                }
                else
                {
                    word[1] = 0x20;
                }
                zifu = word[0] * 16 + word[1];

                url.push_back(zifu);
                i += 2;
            }
            else if (p[i] == '+')
            {
                url.push_back(0x20);
            }
            else
            {
                url.push_back(p[i]);
            }
        }

        return url;
    }

    std::string url_encode(const char *str, int len)
    {
        std::string url;
        const char *p = str;
        unsigned char word;
        if (len > 0)
            url.reserve(len);
        for (int i = 0; i < len; i++)
        {
            if ((p[i] >= 'A' && p[i] <= 'Z') || (p[i] >= 'a' && p[i] <= 'z') || (p[i] >= '0' && p[i] <= '9') ||
                (p[i] == '-') ||
                (p[i] == '_') ||
                (p[i] == '.') ||
                (p[i] == '%') ||
                (p[i] == '~'))
            {
                url.push_back(p[i]);
            }
            else if (p[i] == ' ')
            {
                url.push_back('+');
            }
            else
            {

                url.push_back('%');

                word = (unsigned char)p[i] >> 4;
                word = word > 9 ? word + 55 : word + 48;
                url.push_back(word);

                word = (unsigned char)p[i] % 16;
                word = word > 9 ? word + 55 : word + 48;
                url.push_back(word);
            }
        }

        return url;
    }

    std::string url_rawencode(const char *str, int len)
    {
        std::string url;
        const char *p = str;
        unsigned char word;
        if (len > 0)
            url.reserve(len);
        for (int i = 0; i < len; i++)
        {
            if ((p[i] >= 'A' && p[i] <= 'Z') || (p[i] >= 'a' && p[i] <= 'z') || (p[i] >= '0' && p[i] <= '9') ||
                (p[i] == '-') ||
                (p[i] == '_') ||
                (p[i] == '/') ||
                (p[i] == '&') ||
                (p[i] == '=') ||
                (p[i] == '?') ||
                (p[i] == '#') ||
                (p[i] == '.') ||
                (p[i] == '~'))
            {
                url.push_back(p[i]);
            }
            else if (p[i] == ' ')
            {
                url.push_back('+');
            }
            else
            {

                url.push_back('%');

                word = (unsigned char)p[i] >> 4;
                word = word > 9 ? word + 55 : word + 48;
                url.push_back(word);

                word = (unsigned char)p[i] % 16;
                word = word > 9 ? word + 55 : word + 48;
                url.push_back(word);
            }
        }

        return url;
    }
}
