/*
 * paozhu micro obj rebuild
 * author Huang ziquan (黄自权)
 * date 2025-03-21
 */
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <termios.h>
#include <utility>
#include <vector>
#include <cmath>
#include "request.h"

namespace http
{
unsigned int obj_val::mb_strlen()
{
    if (_val_type != obj_type::STRING)
    {
        return 0;
    }
    unsigned int temp_length = 0;
    unsigned int pos         = 0;

    if (length < 8)
    {
        for (; pos < length; pos++)
        {
            unsigned char c = (unsigned char)name[pos];
            if (c < 0x80)
            {
                temp_length++;
            }
            else if (c < 0xC0)
            {
                temp_length++;
            }
            else if (c >= 0xC0 && c < 0xE0)
            {
                pos += 1;
                temp_length++;
            }
            else if (c >= 0xE0 && c < 0xF0)
            {
                pos += 2;
                temp_length++;
            }
            else if (c >= 0xF0 && c < 0xF8)
            {
                pos += 3;
                temp_length++;
            }
            else
            {
                temp_length++;
            }
        }
    }
    else
    {
        for (; pos < length; pos++)
        {
            unsigned char c = (unsigned char)str[pos];
            if (c < 0x80)
            {
                temp_length++;
            }
            else if (c < 0xC0)
            {
                temp_length++;
            }
            else if (c >= 0xC0 && c < 0xE0)
            {
                pos += 1;
                temp_length++;
            }
            else if (c >= 0xE0 && c < 0xF0)
            {
                pos += 2;
                temp_length++;
            }
            else if (c >= 0xF0 && c < 0xF8)
            {
                pos += 3;
                temp_length++;
            }
            else
            {
                temp_length++;
            }
        }
    }

    return temp_length;
}
std::string obj_val::mb_substr(int begin_pos, int cut_size)
{

    if (length < 8)
    {
        return mb_substr_name(begin_pos, cut_size);
    }

    std::string temp;
    if (_val_type != obj_type::STRING)
    {
        return temp;
    }

    int str_length = 0;

    if (cut_size > 16)
    {
        temp.reserve(cut_size);
    }
    else if (cut_size == 0)
    {
        if (begin_pos > 0)
        {
            str_length = length - begin_pos;
            if (str_length > 0 && str_length > 16)
            {
                temp.reserve(str_length);
            }
        }
        else
        {
            str_length = length + begin_pos;
            if (str_length > 0 && str_length > 16)
            {
                temp.reserve(str_length);
            }
        }
    }

    str_length = mb_strlen();

    // int spacenum = 0;
    unsigned char c;
    if (begin_pos < 0)
    {
        if (cut_size == 0)
        {
            int n = str_length + begin_pos;
            if (n > str_length)
            {
                n = str_length;
            }
            if (n < 0)
            {
                n = 0;
            }

            int offsetnum = 0;
            for (unsigned int pos = 0; pos < length; pos++)
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
        else if (cut_size < 0)
        {
            int j = str_length + cut_size;
            if (j > str_length)
            {
                j = str_length;
            }
            if (j < 0)
            {
                j = 0;
            }
            int n = str_length + begin_pos;
            if (n > str_length)
            {
                n = str_length;
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
            for (unsigned int pos = 0; pos < length; pos++)
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
            int n = str_length + begin_pos;
            if (n > str_length)
            {
                n = str_length;
            }
            if (n < 0)
            {
                n = 0;
            }

            int j = n + cut_size;
            if (j > str_length)
            {
                j = str_length;
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
            for (unsigned int pos = 0; pos < length; pos++)
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
        if (cut_size == 0)
        {
            int offsetnum = 0;
            int n         = begin_pos;
            for (unsigned int pos = 0; pos < length; pos++)
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
        else if (cut_size < 0)
        {
            int j = str_length + cut_size;
            if (j < 0)
            {
                j = 0;
            }
            if (begin_pos > str_length)
            {
                begin_pos = str_length;
            }

            int n = j;
            j     = begin_pos;

            int offsetnum = 0;
            for (unsigned int pos = 0; pos < length; pos++)
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
            if (begin_pos > str_length)
            {
                begin_pos = str_length;
            }
            int j = begin_pos + cut_size;
            if (j > str_length)
            {
                j = str_length;
            }
            int n         = begin_pos;
            int offsetnum = 0;
            for (unsigned int pos = 0; pos < length; pos++)
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

std::string obj_val::mb_substr_name(int begin_pos, int cut_size)
{
    std::string temp;
    if (_val_type != obj_type::STRING)
    {
        return temp;
    }
    if (length > 7)
    {
        return temp;
    }
    int str_length = 0;
    str_length     = mb_strlen();

    // int spacenum = 0;
    unsigned char c;
    if (begin_pos < 0)
    {
        if (cut_size == 0)
        {
            int n = str_length + begin_pos;
            if (n > str_length)
            {
                n = str_length;
            }
            if (n < 0)
            {
                n = 0;
            }

            int offsetnum = 0;
            for (unsigned int pos = 0; pos < length; pos++)
            {
                c = (unsigned char)name[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                        temp.push_back(name[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
            }

            return temp;
        }
        else if (cut_size < 0)
        {
            int j = str_length + cut_size;
            if (j > str_length)
            {
                j = str_length;
            }
            if (j < 0)
            {
                j = 0;
            }
            int n = str_length + begin_pos;
            if (n > str_length)
            {
                n = str_length;
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
            for (unsigned int pos = 0; pos < length; pos++)
            {
                c = (unsigned char)name[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                        temp.push_back(name[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
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
            int n = str_length + begin_pos;
            if (n > str_length)
            {
                n = str_length;
            }
            if (n < 0)
            {
                n = 0;
            }

            int j = n + cut_size;
            if (j > str_length)
            {
                j = str_length;
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
            for (unsigned int pos = 0; pos < length; pos++)
            {
                c = (unsigned char)name[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                        temp.push_back(name[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
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
        if (cut_size == 0)
        {
            int offsetnum = 0;
            int n         = begin_pos;
            for (unsigned int pos = 0; pos < length; pos++)
            {
                c = (unsigned char)name[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                        temp.push_back(name[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
            }

            return temp;
        }
        else if (cut_size < 0)
        {
            int j = str_length + cut_size;
            if (j < 0)
            {
                j = 0;
            }
            if (begin_pos > str_length)
            {
                begin_pos = str_length;
            }

            int n = j;
            j     = begin_pos;

            int offsetnum = 0;
            for (unsigned int pos = 0; pos < length; pos++)
            {
                c = (unsigned char)name[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                        temp.push_back(name[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
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
            if (begin_pos > str_length)
            {
                begin_pos = str_length;
            }
            int j = begin_pos + cut_size;
            if (j > str_length)
            {
                j = str_length;
            }
            int n         = begin_pos;
            int offsetnum = 0;
            for (unsigned int pos = 0; pos < length; pos++)
            {
                c = (unsigned char)name[pos];
                if (c < 0x80)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c < 0xC0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                    }
                    offsetnum++;
                }
                else if (c >= 0xC0 && c < 0xE0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                    }
                    pos += 1;
                    offsetnum++;
                }
                else if (c >= 0xE0 && c < 0xF0)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                    }
                    pos += 2;
                    offsetnum++;
                }
                else if (c >= 0xF0 && c < 0xF8)
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
                        temp.push_back(name[pos + 1]);
                        temp.push_back(name[pos + 2]);
                        temp.push_back(name[pos + 3]);
                    }
                    pos += 3;
                    offsetnum++;
                }
                else
                {
                    if (offsetnum >= n)
                    {
                        temp.push_back(name[pos]);
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

obj_type obj_val::get_type() const
{
    return _val_type;
}
void obj_val::set_type(obj_type _type)
{
    _val_type = _type;
}
const char *obj_val::c_str() const
{
    if (_val_type == obj_type::STRING)
    {
        if (length < 8)
        {
            return name;
        }
        else
        {
            return str;
        }
    }
    return nullptr;
}
char *obj_val::str_data()
{
    if (_val_type == obj_type::STRING)
    {
        if (length < 8)
        {
            return name;
        }
        else
        {
            return str;
        }
    }
    return nullptr;
}

std::string_view obj_val::str_view()
{
    if (_val_type == obj_type::STRING)
    {
        if (length < 8)
        {
            return std::string_view(name, length);
        }
        else
        {
            return std::string_view(str, length);
        }
    }
    return std::string_view(name, 0);
}

std::string_view obj_val::str_view(std::string_view default_val)
{
    if (_val_type == obj_type::STRING)
    {
        if (length < 8)
        {
            return std::string_view(name, length);
        }
        else
        {
            return std::string_view(str, length);
        }
    }
    return default_val;
}

std::string_view obj_val::str_view(int a, int b)
{
    if (_val_type != obj_type::STRING)
    {
        return std::string_view(name, 0);
    }
    int aa = length, bb = length;

    if (a < 0)
    {
        aa = aa + a;
        if (aa < 0)
        {
            aa = 0;
        }
    }
    else
    {
        if (a < length)
        {
            aa = a;
        }
    }

    if (b < 0)
    {
        bb = bb + b;
        if (bb < 0)
        {
            bb = 0;
        }
    }
    else
    {
        bb = aa + b;
        if (bb >= length)
        {
            bb = length;
        }
    }

    bb = bb - aa;
    if (bb < 0)
    {
        bb = 0;
    }

    if (aa >= bb)
    {
        std::string_view(name, 0);
    }

    if (length < 8)
    {
        return std::string_view(name + aa, bb);
    }
    else
    {
        return std::string_view(str + aa, bb);
    }
}

bool obj_val::reserve(unsigned int resize)
{
    if (_val_type == obj_type::STRING)
    {
        if (resize > 0xFFFFFF)
        {
            return false;
        }

        if (number >= resize)
        {
            return false;
        }

        if (resize < 8)
        {
            return false;
        }

        if (resize <= length)
        {
            return false;
        }

        resize = resize - resize % 8 + 8;
        if (length < 8)
        {
            char *tempstr = (char *)std::malloc(resize);
            std::memcpy(tempstr, name, length);
            tempstr[length] = 0x00;
            str             = tempstr;
            number          = resize;
        }
        else
        {
            char *tempstr = (char *)std::malloc(resize);
            std::memcpy(tempstr, str, length);
            free(str);
            tempstr[length] = 0x00;
            str             = tempstr;
            number          = resize;
        }
        return true;
    }
    else if (_val_type == obj_type::OBJECT)
    {
        if (resize > obj->_data.size())
        {
            obj->_data.reserve(resize);
            return true;
        }
    }
    else if (_val_type == obj_type::ARRAY)
    {
        if (resize > array_val->_data.size())
        {
            array_val->_data.reserve(resize);
            return true;
        }
    }
    return false;
}

std::string obj_val::substr(int a, int b)
{
    std::string temp;
    if (_val_type != obj_type::STRING)
    {
        return temp;
    }
    int aa = 0, bb = 0;
    if (a < 0)
    {
        aa = length + a;
    }
    else
    {
        if (a < length)
        {
            aa = a;
        }
        else
        {
            aa = length - 1;
            if (aa < 0)
            {
                aa = 0;
            }
        }
    }
    if (b < 0)
    {
        bb = length + b;
    }
    else
    {
        if (b < length)
        {
            bb = b;
        }
        else
        {
            bb = length;
        }
    }

    if (aa < 0)
    {
        aa = 0;
    }
    if (bb < 0)
    {
        bb = 0;
    }
    if (bb > length)
    {
        bb = length;
    }
    for (; aa < bb; aa++)
    {
        if (length < 8)
        {
            temp.push_back(name[aa]);
        }
        else
        {
            temp.push_back(str[aa]);
        }
    }
    return temp;
}
std::string obj_val::substr(int a)
{
    std::string temp;
    if (_val_type != obj_type::STRING)
    {
        return temp;
    }

    int aa = 0;
    if (a < 0)
    {
        aa = length + a;
        if (aa < 0)
        {
            aa = 0;
        }
    }
    else
    {
        if (a < length)
        {
            aa = a;
        }
        else
        {
            aa = length;
        }
    }

    if (length - a > 32)
    {
        temp.reserve(length - a);
    }

    for (; aa < length; aa++)
    {
        if (length < 8)
        {
            temp.push_back(name[aa]);
        }
        else
        {
            temp.push_back(str[aa]);
        }
    }
    return temp;
}
void obj_val::append(const std::string &v)
{
    if (v.size() > 0xFFFFFF)
    {
        return;
    }
    if (_val_type == obj_type::NIL)
    {
        _val_type = obj_type::STRING;
    }
    if (_val_type != obj_type::STRING)
    {
        return;
    }

    if ((length + v.size()) < 8)
    {
        unsigned int llength = length + v.size();
        for (unsigned int j = length, jj = 0; j < llength; j++, jj++)
        {
            name[j] = v[jj];
        }
        name[llength] = 0x00;
        length        = length + v.size();
    }
    else
    {
        if (length < 8)
        {
            unsigned int jlenth = length + v.size() * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            jlenth = v.size();
            if ((jlenth + length) >= 0xFFFFFF)
            {
                if (length < 0xFFFFFF)
                {
                    jlenth = 0xFFFFFF - length;
                }
                else
                {
                    jlenth = 0;
                }
            }
            char *tempstr = (char *)std::malloc(number);
            for (unsigned int jj = 0; jj < length; jj++)
            {
                tempstr[jj] = name[jj];
            }

            std::memcpy(&tempstr[length], v.data(), jlenth);
            length          = jlenth + length;
            tempstr[length] = 0x00;
            str             = tempstr;
        }
        else
        {
            if (number <= (length + v.size()))
            {
                unsigned int jlenth = length + v.size() * 2;
                jlenth              = jlenth - jlenth % 8 + 8;
                if (jlenth >= 0xFFFFFF)
                {
                    number = 0xFFFFFF;
                }
                else
                {
                    number = jlenth;
                }

                jlenth = v.size();
                if ((jlenth + length) >= 0xFFFFFF)
                {
                    if (length < 0xFFFFFF)
                    {
                        jlenth = 0xFFFFFF - length;
                    }
                    else
                    {
                        jlenth = 0;
                    }
                }

                char *tempstr = (char *)std::malloc(number);
                std::memcpy(tempstr, str, length);
                std::memcpy(&tempstr[length], v.data(), jlenth);
                length          = jlenth + length;
                tempstr[length] = 0x00;
                free(str);
                str = tempstr;
            }
            else
            {
                std::memcpy(&str[length], v.data(), v.size());
                length      = v.size() + length;
                str[length] = 0x00;
            }
        }
    }
}

void obj_val::append(const char *_str, unsigned int str_length)
{
    if (str_length > 0xFFFFFF)
    {
        return;
    }
    if (_val_type == obj_type::NIL)
    {
        _val_type = obj_type::STRING;
    }
    if (_val_type != obj_type::STRING)
    {
        return;
    }

    if ((length + str_length) < 8)
    {
        unsigned int llength = length + str_length;
        for (unsigned int j = length, jj = 0; j < llength; j++, jj++)
        {
            name[j] = _str[jj];
        }
        name[llength] = 0x00;
        length        = length + str_length;
    }
    else
    {
        if (length < 8)
        {
            unsigned int jlenth = (length + str_length) * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            jlenth = str_length;
            if ((jlenth + length) >= 0xFFFFFF)
            {
                if (length < 0xFFFFFF)
                {
                    jlenth = 0xFFFFFF - length;
                }
                else
                {
                    jlenth = 0;
                }
            }
            char *tempstr = (char *)std::malloc(number);
            for (unsigned int jj = 0; jj < length; jj++)
            {
                tempstr[jj] = name[jj];
            }

            std::memcpy(&tempstr[length], _str, jlenth);
            length          = str_length + length;
            tempstr[length] = 0x00;
            str             = tempstr;
        }
        else
        {
            if (number <= (length + str_length))
            {
                unsigned int jlenth = (length + str_length) * 2;
                jlenth              = jlenth - jlenth % 8 + 8;
                if (jlenth >= 0xFFFFFF)
                {
                    number = 0xFFFFFF;
                }
                else
                {
                    number = jlenth;
                }

                jlenth = str_length;
                if ((jlenth + length) >= 0xFFFFFF)
                {
                    if (length < 0xFFFFFF)
                    {
                        jlenth = 0xFFFFFF - length;
                    }
                    else
                    {
                        jlenth = 0;
                    }
                }

                char *tempstr = (char *)std::malloc(number);
                std::memcpy(tempstr, str, length);
                std::memcpy(&tempstr[length], _str, jlenth);
                length          = jlenth + length;
                tempstr[length] = 0x00;
                free(str);
                str = tempstr;
            }
            else
            {
                std::memcpy(&str[length], _str, str_length);
                length      = str_length + length;
                str[length] = 0x00;
            }
        }
    }
}

unsigned int obj_val::size()
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data.size();
    }
    else if (_val_type == obj_type::OBJECT)
    {
        return obj->_data.size();
    }
    else if (_val_type == obj_type::STRING)
    {
        return length;
    }
    return 0;
}

obj_val &obj_val::operator[](unsigned int index)
{
    if (_val_type == obj_type::ARRAY)
    {
        if (index < array_val->_data.size())
        {
            return array_val->_data[index];
        }
    }
    else if (_val_type == obj_type::OBJECT)
    {
        if (index < obj->_data.size())
        {
            return obj->_data[index].second;
        }
        std::string key = std::to_string(index);
        for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
        {
            if (iter->first == key)
            {
                return iter->second;
            }
            iter++;
        }
        obj->_data.emplace_back(key, nullptr);
        obj->_data.back().second.set_type(obj_type::NIL);
        return obj->_data.back().second;
    }
    throw "Out of range operator[] index";
}

obj_val &obj_val::operator[](const std::string &key)
{
    if (_val_type == obj_type::ARRAY)
    {
        unsigned int i_pos = 0xFFFFFFFF;
        try
        {
            i_pos = std::stoul(key);
        }
        catch (const std::invalid_argument &e)
        {
            i_pos = 0xFFFFFFFF;
        }
        catch (const std::out_of_range &e)
        {
            i_pos = 0xFFFFFFFF;
        }
        if (i_pos != 0xFFFFFFFF)
        {
            if (i_pos < array_val->_data.size())
            {
                return array_val->_data[i_pos];
            }
        }
        throw "Out of range operator[] &key";
    }

    if (_val_type != obj_type::OBJECT)
    {
        clear();
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }

    for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
    {
        if (iter->first == key)
        {
            return iter->second;
        }
        iter++;
    }
    obj->_data.emplace_back(key, nullptr);
    obj->_data.back().second.set_type(obj_type::NIL);
    return obj->_data.back().second;
}
obj_val &obj_val::operator[](std::string &&key)
{
    if (_val_type == obj_type::ARRAY)
    {
        if (array_val->_data.size() > 0)
        {
            unsigned int i_pos = 0xFFFFFFFF;
            try
            {
                i_pos = std::stoul(key);
            }
            catch (const std::invalid_argument &e)
            {
                i_pos = 0xFFFFFFFF;
            }
            catch (const std::out_of_range &e)
            {
                i_pos = 0xFFFFFFFF;
            }
            if (i_pos != 0xFFFFFFFF)
            {
                if (i_pos < array_val->_data.size())
                {
                    return array_val->_data[i_pos];
                }
            }
            throw "Out of range operator &&key";
        }
    }

    if (_val_type != obj_type::OBJECT)
    {
        clear();
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }

    for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
    {
        if (iter->first == key)
        {
            return iter->second;
        }
        iter++;
    }
    obj->_data.emplace_back(std::move(key), nullptr);
    obj->_data.back().second.set_type(obj_type::NIL);
    return obj->_data.back().second;
}

void obj_val::clear()
{
    if (_val_type == obj_type::OBJECT)
    {
        if (obj != nullptr)
        {
            obj->_data.clear();
            delete obj;
            obj = nullptr;
        }
    }
    else if (_val_type == obj_type::ARRAY)
    {
        if (array_val != nullptr)
        {
            array_val->_data.clear();
            delete array_val;
            array_val = nullptr;
        }
    }
    else if (_val_type == obj_type::STRING)
    {
        if (length > 7 && str != nullptr)
        {
            free(str);
            str = nullptr;
        }
        else
        {
            std::memset(name, 0x00, 8);
        }
    }
    _val_type = obj_type::NIL;
}

obj_val &obj_val::operator=(long long i)
{
    clear();
    lval      = i;
    _val_type = obj_type::LONG;
    return *this;
}
obj_val &obj_val::operator=(unsigned long long i)
{
    clear();
    uval      = i;
    _val_type = obj_type::ULONG;
    return *this;
}
obj_val &obj_val::operator=(int i)
{
    clear();
    ival      = i;
    _val_type = obj_type::INT;
    return *this;
}
obj_val &obj_val::operator=(short i)
{
    clear();
    ival      = i;
    _val_type = obj_type::INT;
    return *this;
}
obj_val &obj_val::operator=(unsigned short i)
{
    clear();
    uival     = i;
    _val_type = obj_type::UINT;
    return *this;
}
obj_val &obj_val::operator=(unsigned int i)
{
    clear();
    uival     = i;
    _val_type = obj_type::UINT;
    return *this;
}
obj_val &obj_val::operator=(long i)
{
    clear();
    ival      = i;
    _val_type = obj_type::INT;
    return *this;
}
obj_val &obj_val::operator=(unsigned long i)
{
    clear();
    uival     = i;
    _val_type = obj_type::UINT;
    return *this;
}
obj_val &obj_val::operator=(float i)
{
    clear();
    fval      = i;
    _val_type = obj_type::FLOAT;
    return *this;
}
obj_val &obj_val::operator=(double i)
{
    clear();
    dval      = i;
    _val_type = obj_type::DOUBLE;
    return *this;
}
obj_val &obj_val::operator=(long double i)
{
    clear();
    dval      = i;
    _val_type = obj_type::DOUBLE;
    return *this;
}

obj_val &obj_val::operator=(bool i)
{
    clear();
    isbool    = i;
    _val_type = obj_type::BOOL;
    return *this;
}
obj_val &obj_val::operator=(std::string_view v)
{
    if (v.size() >= 0xFFFFFF)
    {
        return *this;
    }
    if (_val_type == obj_type::STRING)
    {
        if (v.size() < 8)
        {
            clear();
            memcpy(name, v.data(), v.size());
            name[v.size()] = 0x00;
            length         = v.size();
            number         = 8;
            _val_type      = obj_type::STRING;
            return *this;
        }

        if (number > v.size())
        {
            memcpy(str, v.data(), v.size());
            str[v.size()] = 0x00;
            length        = v.size();
            _val_type     = obj_type::STRING;
            return *this;
        }

        if (number <= v.size())
        {
            clear();
            length              = 0;
            unsigned int jlenth = v.size() * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            str = (char *)std::malloc(number);
        }
        memcpy(str, v.data(), v.size());
        str[v.size()] = 0x00;
        length        = v.size();
        _val_type     = obj_type::STRING;
    }
    else
    {
        clear();
        if (v.size() < 8)
        {
            memcpy(name, v.data(), v.size());
            name[v.size()] = 0x00;
            number         = 8;
            length         = v.size();
        }
        else
        {
            length              = v.size();
            unsigned int jlenth = v.size() * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            str = (char *)std::malloc(number);
            memcpy(str, v.data(), v.size());
            str[v.size()] = 0x00;
        }
        _val_type = obj_type::STRING;
    }
    return *this;
}
obj_val &obj_val::operator=(const char *v)
{
    unsigned int str_length = strlen(v);
    if (str_length >= 0xFFFFFF)
    {
        return *this;
    }
    if (_val_type == obj_type::STRING)
    {
        if (str_length < 8)
        {
            clear();
            memcpy(name, v, str_length);
            name[str_length] = 0x00;
            length           = str_length;
            number           = 8;
            _val_type        = obj_type::STRING;
            return *this;
        }

        if (number > str_length)
        {
            memcpy(str, v, str_length);
            str[str_length] = 0x00;
            length          = str_length;
            _val_type       = obj_type::STRING;
            return *this;
        }

        if (number <= str_length)
        {
            clear();
            length              = 0;
            unsigned int jlenth = str_length * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            str = (char *)std::malloc(number);
        }
        memcpy(str, v, str_length);
        str[str_length] = 0x00;
        length          = str_length;
        _val_type       = obj_type::STRING;
    }
    else
    {
        clear();
        if (str_length < 8)
        {
            memcpy(name, v, str_length);
            name[str_length] = 0x00;
            number           = 8;
            length           = str_length;
        }
        else
        {
            length              = str_length;
            unsigned int jlenth = str_length * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            str = (char *)std::malloc(number);
            memcpy(str, v, str_length);
            str[str_length] = 0x00;
        }
        _val_type = obj_type::STRING;
    }
    return *this;
}
obj_val &obj_val::operator=(std::string &&v)
{
    if (v.size() >= 0xFFFFFF)
    {
        return *this;
    }
    if (_val_type == obj_type::STRING)
    {
        if (v.size() < 8)
        {
            clear();
            memcpy(name, v.data(), v.size());
            name[v.size()] = 0x00;
            length         = v.size();
            number         = 8;
            _val_type      = obj_type::STRING;
            return *this;
        }

        if (number > v.size())
        {
            memcpy(str, v.data(), v.size());
            str[v.size()] = 0x00;
            length        = v.size();
            _val_type     = obj_type::STRING;
            return *this;
        }

        if (number <= v.size())
        {
            clear();
            length              = 0;
            unsigned int jlenth = v.size() * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            str = (char *)std::malloc(number);
        }
        memcpy(str, v.data(), v.size());
        str[v.size()] = 0x00;
        length        = v.size();
        _val_type     = obj_type::STRING;
    }
    else
    {
        clear();
        if (v.size() < 8)
        {
            memcpy(name, v.data(), v.size());
            name[v.size()] = 0x00;
            number         = 8;
            length         = v.size();
        }
        else
        {
            length              = v.size();
            unsigned int jlenth = v.size() * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            str = (char *)std::malloc(number);
            memcpy(str, v.data(), v.size());
            str[v.size()] = 0x00;
        }
        _val_type = obj_type::STRING;
    }
    return *this;
}
obj_val &obj_val::operator=(const std::string &v)
{
    if (v.size() >= 0xFFFFFF)
    {
        return *this;
    }
    if (_val_type == obj_type::STRING)
    {
        if (v.size() < 8)
        {
            clear();
            memcpy(name, v.data(), v.size());
            name[v.size()] = 0x00;
            length         = v.size();
            number         = 8;
            _val_type      = obj_type::STRING;
            return *this;
        }

        if (number > 7 && number > v.size())
        {
            memcpy(str, v.data(), v.size());
            str[v.size()] = 0x00;
            length        = v.size();
            _val_type     = obj_type::STRING;
            return *this;
        }

        clear();

        length              = 0;
        unsigned int jlenth = v.size() * 2;
        jlenth              = jlenth - jlenth % 8 + 8;
        if (jlenth >= 0xFFFFFF)
        {
            number = 0xFFFFFF;
        }
        else
        {
            number = jlenth;
        }
        str = (char *)std::malloc(number);

        memcpy(str, v.data(), v.size());
        str[v.size()] = 0x00;
        length        = v.size();
        _val_type     = obj_type::STRING;
    }
    else
    {
        clear();
        if (v.size() < 8)
        {
            memcpy(name, v.data(), v.size());
            name[v.size()] = 0x00;
            number         = 8;
            length         = v.size();
        }
        else
        {
            length              = v.size();
            unsigned int jlenth = v.size() * 2;
            jlenth              = jlenth - jlenth % 8 + 8;
            if (jlenth >= 0xFFFFFF)
            {
                number = 0xFFFFFF;
            }
            else
            {
                number = jlenth;
            }
            str = (char *)std::malloc(number);
            memcpy(str, v.data(), v.size());
            str[v.size()] = 0x00;
        }
        _val_type = obj_type::STRING;
    }
    return *this;
}
obj_val &obj_val::operator=(const obj_val &v)
{
    if (this == &v)
    {
        return *this;
    }
    //add kv
    if (_val_type == obj_type::OBJECT)
    {
        if (v.get_type() == obj_type::OBJECT)
        {
            for (auto iter = v.obj->_data.begin(); iter != v.obj->_data.end();)
            {
                //search key
                auto iter2 = obj->_data.begin();
                for (; iter2 != obj->_data.end();)
                {
                    if (iter->first == iter2->first)
                    {
                        iter2->second = iter->second;
                        break;
                    }
                    iter2++;
                }

                //not in
                if (iter2 == obj->_data.end())
                {
                    obj->_data.push_back(*iter);
                }

                iter++;
            }
            return *this;
        }
    }
    else if (_val_type == obj_type::ARRAY)
    {
        array_val->_data.push_back(v);
        return *this;
    }

    {
        clear();
        switch (v.get_type())
        {
        case obj_type::INT:
            ival      = v.ival;
            _val_type = obj_type::INT;
            break;
        case obj_type::UINT:
            uival     = v.uival;
            _val_type = obj_type::UINT;
            break;
        case obj_type::ULONG:
            uval      = v.uval;
            _val_type = obj_type::ULONG;
            break;
        case obj_type::LONG:
            lval      = v.lval;
            _val_type = obj_type::LONG;
            break;
        case obj_type::BOOL:
            isbool    = v.isbool;
            _val_type = obj_type::BOOL;
            break;
        case obj_type::FLOAT:
            fval      = v.fval;
            _val_type = obj_type::FLOAT;
            break;
        case obj_type::DOUBLE:
            dval      = v.dval;
            _val_type = obj_type::DOUBLE;
            break;
        case obj_type::NIL:

            _val_type = obj_type::NIL;
            break;

        case obj_type::STRING:
            if (v.length < 8)
            {
                std::memcpy(name, v.name, v.length);
                name[v.length] = 0x00;
                length         = v.length;
                number         = 8;
            }
            else
            {
                number = v.length + 1;
                number = number - number % 8 + 8;
                str    = (char *)std::malloc(number);
                std::memcpy(str, v.str, v.length);
                str[v.length] = 0x00;
                length        = v.length;
            }
            _val_type = obj_type::STRING;
            break;
        case obj_type::OBJECT:
            _val_type = obj_type::OBJECT;
            obj       = new obj_t;
            obj->_data.reserve(v.obj->_data.size());
            for (auto iter = v.obj->_data.begin(); iter != v.obj->_data.end();)
            {
                obj->_data.push_back(*iter);
                iter++;
            }
            break;
        case obj_type::ARRAY:
            _val_type = obj_type::ARRAY;
            array_val = new obj_array;
            array_val->_data.reserve(v.array_val->_data.size());
            for (auto iter = v.array_val->_data.begin(); iter != v.array_val->_data.end();)
            {
                array_val->_data.push_back(*iter);
                iter++;
            }
            break;
        default:
            _val_type = obj_type::NIL;
        }
    }

    return *this;
}
obj_val &obj_val::operator=(obj_val &&v)
{
    if (this == &v)
    {
        return *this;
    }

    if (_val_type == obj_type::OBJECT)
    {
        if (v.get_type() == obj_type::OBJECT)
        {
            for (auto iter = v.obj->_data.begin(); iter != v.obj->_data.end();)
            {
                //search key
                auto iter2 = obj->_data.begin();
                for (; iter2 != obj->_data.end();)
                {
                    if (iter->first == iter2->first)
                    {
                        iter2->second = iter->second;
                        break;
                    }
                    iter2++;
                }

                //not in
                if (iter2 == obj->_data.end())
                {
                    obj->_data.push_back(std::move(*iter));
                }
                iter++;
            }
            v.obj->_data.clear();
            v.obj = nullptr;
            v.set_type(obj_type::NIL);
            return *this;
        }
    }
    else if (_val_type == obj_type::ARRAY)
    {
        array_val->_data.push_back(std::move(v));
        return *this;
    }

    {
        clear();
        switch (v.get_type())
        {
        case obj_type::INT:
            ival      = v.ival;
            _val_type = obj_type::INT;
            break;
        case obj_type::UINT:
            uival     = v.uival;
            _val_type = obj_type::UINT;
            break;
        case obj_type::ULONG:
            uval      = v.uval;
            _val_type = obj_type::ULONG;
            break;
        case obj_type::LONG:
            lval      = v.lval;
            _val_type = obj_type::LONG;
            break;
        case obj_type::BOOL:
            isbool    = v.isbool;
            _val_type = obj_type::BOOL;
            break;
        case obj_type::FLOAT:
            fval      = v.fval;
            _val_type = obj_type::FLOAT;
            break;
        case obj_type::DOUBLE:
            dval      = v.dval;
            _val_type = obj_type::DOUBLE;
            break;
        case obj_type::NIL:

            _val_type = obj_type::NIL;
            break;

        case obj_type::STRING:
            if (v.length < 8)
            {
                std::memcpy(name, v.name, v.length);
                name[v.length] = 0x00;
                length         = v.length;
                number         = 8;
            }
            else
            {
                str   = v.str;
                v.str = nullptr;
                v.set_type(obj_type::NIL);
                length = v.length;
                number = v.number;
            }
            _val_type = obj_type::STRING;
            break;
        case obj_type::OBJECT:
            obj   = v.obj;
            v.obj = nullptr;
            v.set_type(obj_type::NIL);
            _val_type = obj_type::OBJECT;
            break;
        case obj_type::ARRAY:
            _val_type   = obj_type::ARRAY;
            array_val   = v.array_val;
            v.array_val = nullptr;
            v.set_type(obj_type::NIL);
            break;
        default:
            _val_type = obj_type::NIL;
        }
    }

    return *this;
}

obj_val &obj_val::push(const obj_val &v)
{
    if (this == &v)
    {
        return *this;
    }
    if (_val_type == obj_type::ARRAY)
    {
        array_val->_data.emplace_back(v);
    }
    else if (_val_type == obj_type::NIL)
    {
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
        array_val->_data.emplace_back(v);
    }
    return *this;
}
obj_val &obj_val::push(obj_val &&v)
{
    if (this == &v)
    {
        return *this;
    }

    if (_val_type == obj_type::ARRAY)
    {
        array_val->_data.emplace_back(std::move(v));
    }
    else if (_val_type == obj_type::NIL)
    {
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
        array_val->_data.emplace_back(std::move(v));
    }
    return *this;
}

obj_val &obj_val::push(const std::string &key, const obj_val &v)
{
    if (this == &v)
    {
        return *this;
    }
    if (_val_type == obj_type::OBJECT)
    {
        obj->_data.emplace_back(key, v);
    }
    else if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
        obj->_data.emplace_back(key, v);
    }
    return *this;
}

obj_val &obj_val::push(const std::string &key, obj_val &&v)
{
    if (this == &v)
    {
        return *this;
    }
    if (_val_type == obj_type::OBJECT)
    {
        obj->_data.emplace_back(key, std::move(v));
    }
    else if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
        obj->_data.emplace_back(key, std::move(v));
    }
    return *this;
}

obj_val::obj_val(const obj_val &v)
{
    switch (v.get_type())
    {
    case obj_type::NIL:
        _val_type = obj_type::NIL;
        break;
    case obj_type::BOOL:
        _val_type = obj_type::BOOL;
        isbool    = v.isbool;
        break;
    case obj_type::STRING:
        if (v.length > 0xFFFFFE)
        {
            _val_type = obj_type::NIL;
            break;
        }
        _val_type = obj_type::STRING;
        if (v.length < 8)
        {
            std::memcpy(name, v.name, v.length);
            name[v.length] = 0x00;
            length         = v.length;
            number         = 8;
        }
        else
        {
            number = v.length + 1;
            number = number - number % 8 + 8;
            str    = (char *)std::malloc(number);
            std::memcpy(str, v.str, v.length);
            str[v.length] = 0x00;
            length        = v.length;
        }
        break;
    case obj_type::INT:
        _val_type = obj_type::INT;
        ival      = v.ival;
        break;
    case obj_type::UINT:
        _val_type = obj_type::UINT;
        uival     = v.uival;
        break;
    case obj_type::LONG:
        _val_type = obj_type::LONG;
        lval      = v.lval;
        break;
    case obj_type::ULONG:
        _val_type = obj_type::ULONG;
        uval      = v.uval;
        break;
    case obj_type::FLOAT:
        _val_type = obj_type::FLOAT;
        fval      = v.fval;
        break;
    case obj_type::DOUBLE:
        _val_type = obj_type::DOUBLE;
        dval      = v.dval;
        break;
    case obj_type::OBJECT:
        _val_type = obj_type::OBJECT;
        obj       = new obj_t;
        obj->_data.reserve(v.obj->_data.size());
        for (auto iter = v.obj->_data.begin(); iter != v.obj->_data.end();)
        {
            obj->_data.push_back(*iter);
            iter++;
        }
        break;
    case obj_type::ARRAY:
        _val_type = obj_type::ARRAY;
        array_val = new obj_array;
        // array_val->key = v.array_val->key;
        array_val->_data.reserve(v.array_val->_data.size());
        for (auto iter = v.array_val->_data.begin(); iter != v.array_val->_data.end();)
        {
            array_val->_data.push_back(*iter);
            iter++;
        }
        break;
    default:
        _val_type = obj_type::NIL;
        /*10-14*/;
    }
}
obj_val::obj_val(obj_val &&v)
{
    switch (v.get_type())
    {
    case obj_type::NIL:
        _val_type = obj_type::NIL;
        break;
    case obj_type::BOOL:
        _val_type = obj_type::BOOL;
        isbool    = v.isbool;
        break;
    case obj_type::STRING:
        if (v.length > 0xFFFFFE)
        {
            _val_type = obj_type::NIL;
            break;
        }
        if (v.length < 8)
        {
            std::memcpy(name, v.name, v.length);
            name[v.length] = 0x00;
            length         = v.length;
            number         = 8;
        }
        else
        {
            str    = v.str;
            number = v.number;
            v.str  = nullptr;
            v.set_type(obj_type::NIL);
            length = v.length;
        }
        _val_type = obj_type::STRING;
        break;
    case obj_type::INT:
        _val_type = obj_type::INT;
        ival      = v.ival;
        break;
    case obj_type::UINT:
        _val_type = obj_type::UINT;
        uival     = v.uival;
        break;
    case obj_type::LONG:
        _val_type = obj_type::LONG;
        lval      = v.lval;
        break;
    case obj_type::ULONG:
        _val_type = obj_type::ULONG;
        uval      = v.uval;
        break;
    case obj_type::FLOAT:
        _val_type = obj_type::FLOAT;
        fval      = v.fval;
        break;
    case obj_type::DOUBLE:
        _val_type = obj_type::DOUBLE;
        dval      = v.dval;
        break;
    case obj_type::OBJECT:
        _val_type = obj_type::OBJECT;
        obj       = v.obj;
        v.obj     = nullptr;
        v.set_type(obj_type::NIL);
        break;
    case obj_type::ARRAY:
        _val_type   = obj_type::ARRAY;
        array_val   = v.array_val;
        v.array_val = nullptr;
        v.set_type(obj_type::NIL);
        break;

    default:
        _val_type = obj_type::NIL;
        /*10-14*/;
    }
}
std::string obj_val::to_string()
{

    if (_val_type == obj_type::OBJECT)
    {
        return "{}";
    }
    else if (_val_type == obj_type::ARRAY)
    {
        return "[]";
    }
    else if (_val_type == obj_type::STRING)
    {
        if (length < 8)
        {
            std::string a_temp;
            for (unsigned int j = 0; j < length; j++)
            {
                a_temp.push_back(name[j]);
            }
            return a_temp;
        }
        return std::string(str, length);
    }
    else if (_val_type == obj_type::BOOL)
    {
        if (isbool)
        {
            return "true";
        }
        else
        {
            return "false";
        }
    }
    else if (_val_type == obj_type::INT)
    {
        return std::to_string(ival);
    }
    else if (_val_type == obj_type::UINT)
    {
        return std::to_string(uival);
    }
    else if (_val_type == obj_type::FLOAT)
    {
        return std::to_string(fval);
    }
    else if (_val_type == obj_type::DOUBLE)
    {
        return std::to_string(dval);
    }
    else if (_val_type == obj_type::LONG)
    {
        return std::to_string(lval);
    }
    else if (_val_type == obj_type::ULONG)
    {
        return std::to_string(uval);
    }
    else if (_val_type == obj_type::NIL)
    {
        return "";
    }
    return "";
}
obj_val::~obj_val()
{
    if (_val_type == obj_type::OBJECT)
    {
        obj->_data.clear();
        delete obj;
    }
    else if (_val_type == obj_type::ARRAY)
    {
        array_val->_data.clear();
        delete array_val;
    }
    else if (_val_type == obj_type::STRING)
    {
        if (length > 7 && str != nullptr)
        {
            free(str);
        }
    }
}
obj_val::obj_val() : _val_type(obj_type::NIL)
{
}
obj_val::obj_val(std::nullptr_t i) : _val_type(obj_type::NIL)
{
    if (i == nullptr)
    {
        str = nullptr;
    }
}

obj_val::obj_val(bool i) : _val_type(obj_type::BOOL)
{
    isbool = i;
}

obj_val::obj_val(long long i) : _val_type(obj_type::LONG)
{
    lval = i;
}
obj_val::obj_val(unsigned long long i) : _val_type(obj_type::ULONG)
{
    uval = i;
}
obj_val::obj_val(int i) : _val_type(obj_type::INT)
{
    ival = i;
}
obj_val::obj_val(long i) : _val_type(obj_type::LONG)
{
    lval = i;
}
obj_val::obj_val(unsigned int i) : _val_type(obj_type::UINT)
{
    uival = i;
}
obj_val::obj_val(unsigned long i) : _val_type(obj_type::ULONG)
{
    uval = i;
}
obj_val::obj_val(unsigned short i) : _val_type(obj_type::UINT)
{
    uival = i;
}
obj_val::obj_val(short i) : _val_type(obj_type::INT)
{
    ival = i;
}
obj_val::obj_val(float i) : _val_type(obj_type::FLOAT)
{
    fval = i;
}
obj_val::obj_val(double i) : _val_type(obj_type::DOUBLE)
{
    dval = i;
}
obj_val::obj_val(long double i) : _val_type(obj_type::DOUBLE)
{
    dval = i;
}
obj_val::obj_val(const char *_str) : _val_type(obj_type::STRING)
{
    unsigned int str_length = strlen(_str);
    if (str_length < 8)
    {
        std::memcpy(name, _str, str_length);
        name[str_length] = 0x00;
        length           = str_length;
        number           = 8;
    }
    else
    {
        number = str_length;
        number = number - number % 8 + 8;
        if (number > 0xFFFFFF)
        {
            number = 0xFFFFFF;
        }
        length = str_length;
        if (length >= 0xFFFFFF)
        {
            length = 0xFFFFFF - 1;
        }

        str = (char *)std::malloc(number);
        std::memcpy(str, _str, length);
        str[length] = 0x00;
    }
}
obj_val::obj_val(const char *_str, unsigned int str_length) : _val_type(obj_type::STRING)
{
    if (str_length < 8)
    {
        std::memcpy(name, _str, str_length);
        name[str_length] = 0x00;
        length           = str_length;
        number           = 8;
    }
    else
    {
        number = str_length;
        number = number - number % 8 + 8;
        if (number > 0xFFFFFF)
        {
            number = 0xFFFFFF;
        }
        length = str_length;
        if (length >= 0xFFFFFF)
        {
            length = 0xFFFFFF - 1;
        }

        str = (char *)std::malloc(number);
        std::memcpy(str, _str, length);
        str[length] = 0x00;
    }
}
obj_val::obj_val(std::string_view _str) : _val_type(obj_type::STRING)
{
    if (_str.size() < 8)
    {
        std::memcpy(name, _str.data(), _str.size());
        name[_str.size()] = 0x00;
        length            = _str.size();
        number            = 8;
    }
    else
    {
        number = _str.size();
        number = number - number % 8 + 8;
        if (number > 0xFFFFFF)
        {
            number = 0xFFFFFF;
        }
        length = _str.size();
        if (length >= 0xFFFFFF)
        {
            length = 0xFFFFFF - 1;
        }
        str = (char *)std::malloc(number);
        std::memcpy(str, _str.data(), length);
        str[length] = 0x00;
    }
}

obj_val::obj_val(const std::string &_str) : _val_type(obj_type::STRING)
{
    if (_str.size() < 8)
    {
        std::memcpy(name, _str.data(), _str.size());
        name[_str.size()] = 0x00;
        length            = _str.size();
        number            = 8;
    }
    else
    {
        number = _str.size();
        number = number - number % 8 + 8;
        if (number > 0xFFFFFF)
        {
            number = 0xFFFFFF;
        }
        length = _str.size();
        if (length >= 0xFFFFFF)
        {
            length = 0xFFFFFF - 1;
        }
        str = (char *)std::malloc(number);
        std::memcpy(str, _str.data(), length);
        str[length] = 0x00;
    }
}
obj_val::obj_val(std::string &&_str) : _val_type(obj_type::STRING)
{
    if (_str.size() < 8)
    {
        std::memcpy(name, _str.data(), _str.size());
        name[_str.size()] = 0x00;
        length            = _str.size();
        number            = 8;
    }
    else
    {
        number = _str.size();
        number = number - number % 8 + 8;
        if (number > 0xFFFFFF)
        {
            number = 0xFFFFFF;
        }
        length = _str.size();
        if (length >= 0xFFFFFF)
        {
            length = 0xFFFFFF - 1;
        }
        str = (char *)std::malloc(number);
        std::memcpy(str, _str.data(), length);
        str[length] = 0x00;
    }
}

obj_val::obj_val(const obj_t &v) : _val_type(obj_type::OBJECT)
{
    obj = new obj_t;
    for (auto iter = v._data.begin(); iter != v._data.end();)
    {
        obj->_data.push_back(*iter);
        iter++;
    }
}
obj_val::obj_val(obj_t &&v) : _val_type(obj_type::OBJECT)
{
    obj        = new obj_t;
    obj->_data = std::move(v._data);
}

obj_val::obj_val(const obj_array &v) : _val_type(obj_type::ARRAY)
{
    array_val = new obj_array;
    array_val->_data.reserve(v._data.size());
    for (auto iter = v._data.begin(); iter != v._data.end();)
    {
        array_val->_data.push_back(*iter);
        iter++;
    }
}
obj_val::obj_val(obj_array &&v) : _val_type(obj_type::ARRAY)
{
    array_val        = new obj_array;
    array_val->_data = std::move(v._data);
}

float obj_val::str_to_float()
{
    if (_val_type != obj_type::STRING)
    {
        return 0;
    }
    try
    {
        if (length < 8)
        {
            name[7] = 0x00;
            return std::stof(name);
        }
        else
        {
            return std::stof(str);
        }
    }
    catch (const std::invalid_argument &e)
    {
        return 0;
    }
    catch (const std::out_of_range &e)
    {
        return 0;
    }
    return 0;
}

double obj_val::str_to_double()
{
    if (_val_type != obj_type::STRING)
    {
        return 0;
    }
    try
    {
        if (length < 8)
        {
            name[7] = 0x00;
            return std::stod(name);
        }
        else
        {
            return std::stod(str);
        }
    }
    catch (const std::invalid_argument &e)
    {
        return 0;
    }
    catch (const std::out_of_range &e)
    {
        return 0;
    }
    return 0;
}
int obj_val::str_to_int()
{
    if (_val_type != obj_type::STRING)
    {
        return 0;
    }
    try
    {
        if (length < 8)
        {
            name[7] = 0x00;
            return std::stoi(name);
        }
        else
        {
            return std::stoi(str);
        }
    }
    catch (const std::invalid_argument &e)
    {
        return 0;
    }
    catch (const std::out_of_range &e)
    {
        return 0;
    }
    return 0;
}
long long obj_val::str_to_long()
{
    if (_val_type != obj_type::STRING)
    {
        return 0;
    }
    try
    {
        if (length < 8)
        {
            name[7] = 0x00;
            return std::stoll(name);
        }
        else
        {
            return std::stoll(str);
        }
    }
    catch (const std::invalid_argument &e)
    {
        return 0;
    }
    catch (const std::out_of_range &e)
    {
        return 0;
    }
    return 0;
}

unsigned int obj_val::str_to_uint()
{
    if (_val_type != obj_type::STRING)
    {
        return 0;
    }
    try
    {
        if (length < 8)
        {
            name[7] = 0x00;
            return std::stoul(name);
        }
        else
        {
            return std::stoul(str);
        }
    }
    catch (const std::invalid_argument &e)
    {
        return 0;
    }
    catch (const std::out_of_range &e)
    {
        return 0;
    }
    return 0;
}
unsigned long long obj_val::str_to_ulong()
{
    if (_val_type != obj_type::STRING)
    {
        return 0;
    }
    try
    {
        if (length < 8)
        {
            name[7] = 0x00;
            return std::stoull(name);
        }
        else
        {
            return std::stoull(str);
        }
    }
    catch (const std::invalid_argument &e)
    {
        return 0;
    }
    catch (const std::out_of_range &e)
    {
        return 0;
    }
    return 0;
}

bool obj_val::to_bool()
{
    switch (_val_type)
    {
    case obj_type::NIL:
        return false;
        break;
    case obj_type::BOOL:
        return isbool;
        break;
    case obj_type::STRING:
        if (length == 0)
        {
            return false;
        }
        if (length < 8 && length > 4)
        {
            if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
            {
                return false;
            }
            else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
            {
                return false;
            }
            else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
            {
                return false;
            }
            return true;
        }
        else if (length == 1)
        {
            if (name[0] == '0' || name[0] == 0x20)
            {
                return false;
            }
        }

        if (length > 0)
        {
            return true;
        }
        return false;
        break;
    case obj_type::INT:
        if (ival > 0)
        {
            return true;
        }
        return false;
        break;
    case obj_type::UINT:
        if (uival > 0)
        {
            return true;
        }
        return false;
        break;
    case obj_type::LONG:
        if (lval > 0)
        {
            return true;
        }
        return false;
        break;
    case obj_type::ULONG:
        if (uval > 0)
        {
            return true;
        }
        return false;
        break;
    case obj_type::FLOAT:
        if (fval > 0)
        {
            return true;
        }
        return false;
        break;
    case obj_type::DOUBLE:
        if (dval > 0)
        {
            return true;
        }
        return false;
        break;
    case obj_type::OBJECT:
        if (obj->_data.size() > 0)
        {
            return true;
        }
        return false;
        break;
    default:
        return false;
    }
    return false;
}

long long obj_val::to_int()
{
    switch (_val_type)
    {
    case obj_type::NIL:
        return 0;
        break;
    case obj_type::BOOL:
        return isbool ? 1 : 0;
        break;
    case obj_type::STRING:
        if (length == 0)
        {
            return 0;
        }
        if (length < 8 && length > 4)
        {
            if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
            {
                return 0;
            }
            else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
            {
                return 0;
            }
            else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
            {
                return 0;
            }
        }
        else if (length == 1)
        {
            if (name[0] == '0' || name[0] == 0x20)
            {
                return 0;
            }
        }

        {
            return str_to_long();
        }
        break;
    case obj_type::INT:
        return ival;
        break;
    case obj_type::UINT:
        return uival;
        break;
    case obj_type::LONG:

        return lval;
        break;
    case obj_type::ULONG:
        return uval;
        break;
    case obj_type::FLOAT:
        return fval;
        break;
    case obj_type::DOUBLE:
        return dval;
        break;
    case obj_type::OBJECT:
        return 0;
        break;
    default:
        return 0;
    }
    return 0;
}

double obj_val::to_float()
{
    switch (_val_type)
    {
    case obj_type::NIL:
        return 0.0;
        break;
    case obj_type::BOOL:
        return isbool ? 1 : 0;
        break;
    case obj_type::STRING:
        if (length == 0)
        {
            return 0.0;
        }
        if (length < 8 && length > 4)
        {
            if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
            {
                return 0.0;
            }
            else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
            {
                return 0.0;
            }
            else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
            {
                return 0.0;
            }

            return str_to_double();
        }
        else if (length == 1)
        {
            if (name[0] == '0' || name[0] == 0x20)
            {
                return 0.0;
            }
        }

        {
            return str_to_double();
        }
        break;
    case obj_type::INT:
        return ival;
        break;
    case obj_type::UINT:
        return uival;
        break;
    case obj_type::LONG:
        return lval;
        break;
    case obj_type::ULONG:
        return uval;
        break;
    case obj_type::FLOAT:
        return fval;
        break;
    case obj_type::DOUBLE:
        return dval;
        break;
    case obj_type::OBJECT:
        return 0.0;
        break;
    default:
        return 0.0;
    }
    return 0.0;
}

obj_val &obj_val::set_array()
{
    if (_val_type != obj_type::ARRAY)
    {
        clear();
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
    }
    return *this;
}
obj_val &obj_val::set_obj()
{
    if (_val_type != obj_type::OBJECT)
    {
        clear();
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }
    return *this;
}
obj_val &obj_val::set_object()
{
    if (_val_type != obj_type::OBJECT)
    {
        clear();
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }
    return *this;
}
obj_val &obj_val::set_int()
{
    if (_val_type != obj_type::INT)
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            ival = 0;
            break;
        case obj_type::BOOL:
            ival = isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                ival = 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    ival = 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    ival = 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    ival = 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    ival = 0;
                }
            }

            {
                ival = str_to_int();
            }
            clear();
            number = 0;
            length = 0;
            break;
        case obj_type::INT:
            break;
        case obj_type::UINT:
            ival = uival;
            break;
        case obj_type::LONG:

            ival = lval;
            break;
        case obj_type::ULONG:
            ival = uval;
            break;
        case obj_type::FLOAT:
            ival = fval;
            break;
        case obj_type::DOUBLE:
            ival = dval;
            break;
        case obj_type::OBJECT:
            clear();
            ival = 0;
            break;
        case obj_type::ARRAY:
            clear();
            ival = 0;
            break;
        default:
            ival = 0;
        }
        _val_type = obj_type::INT;
    }
    return *this;
}
obj_val &obj_val::set_long()
{
    if (_val_type != obj_type::LONG)
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            lval = 0;
            break;
        case obj_type::BOOL:
            lval = isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                lval = 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    lval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    lval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    lval = 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    lval = 0;
                }
            }

            {
                lval = str_to_long();
            }
            clear();
            number = 0;
            length = 0;
            break;
        case obj_type::INT:
            lval = ival;
            break;
        case obj_type::UINT:
            lval = uival;
            break;
        case obj_type::LONG:
            lval = 0;
            break;
        case obj_type::ULONG:
            lval = uval;
            break;
        case obj_type::FLOAT:
            lval = fval;
            break;
        case obj_type::DOUBLE:
            lval = dval;
            break;
        case obj_type::OBJECT:
            clear();
            lval = 0;
            break;
        case obj_type::ARRAY:
            clear();
            lval = 0;
            break;
        default:
            lval = 0;
        }
        _val_type = obj_type::LONG;
    }
    return *this;
}
obj_val &obj_val::set_uint()
{
    if (_val_type != obj_type::UINT)
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            uival = 0;
            break;
        case obj_type::BOOL:
            uival = isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                uival = 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    uival = 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    uival = 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    uival = 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    uival = 0;
                }
            }

            {
                uival = str_to_uint();
            }
            clear();
            number = 0;
            length = 0;
            break;
        case obj_type::INT:
            uival = ival;
            break;
        case obj_type::UINT:
            uival = 0;
            break;
        case obj_type::LONG:
            uival = 0;
            break;
        case obj_type::ULONG:
            uival = uval;
            break;
        case obj_type::FLOAT:
            uival = fval;
            break;
        case obj_type::DOUBLE:
            uival = dval;
            break;
        case obj_type::OBJECT:
            clear();
            uival = 0;
            break;
        case obj_type::ARRAY:
            clear();
            uival = 0;
            break;
        default:
            uival = 0;
        }
        _val_type = obj_type::UINT;
    }
    return *this;
}
obj_val &obj_val::set_ulong()
{
    if (_val_type != obj_type::ULONG)
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            uval = 0;
            break;
        case obj_type::BOOL:
            uval = isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                uval = 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    uval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    uval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    uval = 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    uval = 0;
                }
            }

            {
                uval = str_to_ulong();
            }
            clear();
            number = 0;
            length = 0;
            break;
        case obj_type::INT:
            uval = ival;
            break;
        case obj_type::UINT:
            uval = 0;
            break;
        case obj_type::LONG:
            uval = lval;
            break;
        case obj_type::ULONG:
            uval = 0;
            break;
        case obj_type::FLOAT:
            uval = fval;
            break;
        case obj_type::DOUBLE:
            uval = dval;
            break;
        case obj_type::OBJECT:
            clear();
            uval = 0;
            break;
        case obj_type::ARRAY:
            clear();
            uval = 0;
            break;
        default:
            uval = 0;
        }
        _val_type = obj_type::ULONG;
    }
    return *this;
}
obj_val &obj_val::set_number()
{
    if (_val_type != obj_type::LONG)
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            lval = 0;
            break;
        case obj_type::BOOL:
            lval = isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                lval = 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    lval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    lval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    lval = 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    lval = 0;
                }
            }

            {
                lval = str_to_long();
            }
            clear();
            number = 0;
            length = 0;
            break;
        case obj_type::INT:
            lval = ival;
            break;
        case obj_type::UINT:
            lval = uival;
            break;
        case obj_type::LONG:
            lval = 0;
            break;
        case obj_type::ULONG:
            lval = uval;
            break;
        case obj_type::FLOAT:
            lval = fval;
            break;
        case obj_type::DOUBLE:
            lval = dval;
            break;
        case obj_type::OBJECT:
            clear();
            lval = 0;
            break;
        case obj_type::ARRAY:
            clear();
            lval = 0;
            break;
        default:
            lval = 0;
        }
        _val_type = obj_type::LONG;
    }
    return *this;
}
obj_val &obj_val::set_float()
{
    if (_val_type != obj_type::FLOAT)
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            fval = 0;
            break;
        case obj_type::BOOL:
            fval = isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                fval = 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    fval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    fval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    fval = 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    fval = 0;
                }
            }

            {
                fval = str_to_float();
            }
            clear();
            number = 0;
            length = 0;
            break;
        case obj_type::INT:
            fval = ival;
            break;
        case obj_type::UINT:
            fval = uival;
            break;
        case obj_type::LONG:
            fval = lval;
            break;
        case obj_type::ULONG:
            fval = uval;
            break;
        case obj_type::FLOAT:
            fval = 0;
            break;
        case obj_type::DOUBLE:
            fval = dval;
            break;
        case obj_type::OBJECT:
            clear();
            fval = 0;
            break;
        case obj_type::ARRAY:
            clear();
            fval = 0;
            break;
        default:
            fval = 0;
        }
        _val_type = obj_type::FLOAT;
    }
    return *this;
}
obj_val &obj_val::set_double()
{
    if (_val_type != obj_type::DOUBLE)
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            dval = 0;
            break;
        case obj_type::BOOL:
            dval = isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                dval = 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    dval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    dval = 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    dval = 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    dval = 0;
                }
            }

            {
                dval = str_to_double();
            }
            clear();
            number = 0;
            length = 0;
            break;
        case obj_type::INT:
            dval = ival;
            break;
        case obj_type::UINT:
            dval = uival;
            break;
        case obj_type::LONG:
            dval = lval;
            break;
        case obj_type::ULONG:
            dval = uval;
            break;
        case obj_type::FLOAT:
            dval = fval;
            break;
        case obj_type::DOUBLE:
            dval = 0;
            break;
        case obj_type::OBJECT:
            clear();
            dval = 0;
            break;
        case obj_type::ARRAY:
            clear();
            dval = 0;
            break;
        default:
            dval = 0;
        }
        _val_type = obj_type::DOUBLE;
    }
    return *this;
}
obj_val &obj_val::set_string()
{
    if (_val_type != obj_type::STRING)
    {
        clear();
        number    = 8;
        length    = 0;
        _val_type = obj_type::STRING;
    }
    return *this;
}
obj_val &obj_val::set_bool()
{
    if (_val_type != obj_type::BOOL)
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            isbool = false;
            break;
        case obj_type::BOOL:

            break;
        case obj_type::STRING:
            if (length == 0)
            {
                isbool = false;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    isbool = false;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    isbool = false;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    isbool = false;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    isbool = false;
                }
            }

            {
                isbool = true;
            }
            clear();
            number = 0;
            length = 0;
            break;
        case obj_type::INT:
            isbool = ival == 0 ? false : true;
            break;
        case obj_type::UINT:
            isbool = uival == 0 ? false : true;
            break;
        case obj_type::LONG:
            isbool = lval == 0 ? false : true;
            break;
        case obj_type::ULONG:

            isbool = uval == 0 ? false : true;
            break;
        case obj_type::FLOAT:
            if (fabs(fval) < 1e-6)
            {
                isbool = false;
            }
            else
            {
                isbool = true;
            }
            break;
        case obj_type::DOUBLE:
            if (fabs(dval) < 1e-15)
            {
                isbool = false;
            }
            else
            {
                isbool = true;
            }
            break;
        case obj_type::OBJECT:
            clear();
            isbool = true;
            break;
        case obj_type::ARRAY:
            clear();
            isbool = true;
            break;
        default:
            isbool = false;
        }
        _val_type = obj_type::BOOL;
    }
    return *this;
}
obj_val &obj_val::set_null()
{
    clear();
    _val_type = obj_type::NIL;
    return *this;
}

obj_val &obj_val::set_link_info()
{
    if (_val_type != obj_type::link_info)
    {
        clear();
        uival     = 0;
        num       = 0;
        _val_type = obj_type::link_info;
    }
    return *this;
}

obj_val &obj_val::set_card_info()
{
    if (_val_type != obj_type::card_info)
    {
        clear();
        total_val = 0.0;
        item_val  = 0.0;
        _val_type = obj_type::card_info;
    }
    return *this;
}
obj_val &obj_val::set_list_info()
{
    if (_val_type != obj_type::list_info)
    {
        clear();
        mid       = 0;
        isview    = false;
        ishome    = false;
        isjin     = false;
        istop     = false;
        _val_type = obj_type::list_info;
    }
    return *this;
}

obj_val &obj_val::set_node_info()
{
    if (_val_type != obj_type::node_info)
    {
        clear();
        node_id   = 0;
        node_num  = 0;
        _val_type = obj_type::node_info;
    }
    return *this;
}
obj_val &obj_val::set_item_info()
{
    if (_val_type != obj_type::item_info)
    {
        clear();
        item_id   = 0;
        tag[0]    = 0x00;
        tag[1]    = 0x00;
        tag[2]    = 0x00;
        tag[3]    = 0x00;
        _val_type = obj_type::item_info;
    }
    return *this;
}

obj_val &obj_val::operator+(const std::string &v)
{
    if (_val_type == obj_type::STRING)
    {
        if ((length + v.size()) < 8)
        {
            unsigned int llength = length + v.size();
            for (unsigned int j = length, jj = 0; j < llength; j++, jj++)
            {
                name[j] = v[jj];
            }
            name[llength] = 0x00;
            number        = 8;
        }
        else
        {
            if (length < 8)
            {
                number = length + v.size() * 2;
                number = number - number % 8 + 8;
                if (number > 0xFFFFFF)
                {
                    number = 0xFFFFFF;
                }
                unsigned int temp_len = length;
                length                = length + v.size();
                if (length > 0xFFFFFF)
                {
                    length = 0xFFFFFF;
                }
                char *temp = (char *)std::malloc(number);
                std::memcpy(temp, name, temp_len);
                std::memcpy(&temp[temp_len], v.data(), (length - temp_len));
                str         = temp;
                str[length] = 0x00;
            }
            else
            {
                if (number <= (length + v.size()))
                {
                    unsigned int jlenth = length + v.size() * 2;
                    jlenth              = jlenth - jlenth % 8 + 8;
                    if (jlenth >= 0xFFFFFF)
                    {
                        number = 0xFFFFFF;
                    }
                    else
                    {
                        number = jlenth;
                    }

                    jlenth = length;
                    length = length + v.size();
                    if (length > 0xFFFFFF)
                    {
                        length = 0xFFFFFF;
                    }

                    char *temp = (char *)std::malloc(number);
                    std::memcpy(temp, str, jlenth);
                    std::memcpy(&temp[jlenth], v.data(), length - jlenth);
                    str[length] = 0x00;
                    free(str);
                    str = temp;
                }
                else
                {
                    std::memcpy(&str[length], v.data(), v.size());
                    length      = v.size() + length;
                    str[length] = 0x00;
                }
            }
        }
    }
    return *this;
}

obj_val &obj_val::operator+(long long i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::LONG;
        lval      = i;
        break;
    case obj_type::BOOL:
        isbool = i == 0 ? false : true;
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival + i;
        break;
    case obj_type::UINT:
        uival = uival + i;
        break;
    case obj_type::LONG:
        lval = lval + i;
        break;
    case obj_type::ULONG:
        uval = uval + i;
        break;
    case obj_type::FLOAT:
        fval = fval + i;
        break;
    case obj_type::DOUBLE:
        dval = dval + i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator+(unsigned long long i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::ULONG;
        uval      = i;
        break;
    case obj_type::BOOL:
        isbool = i == 0 ? false : true;
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival + i;
        break;
    case obj_type::UINT:
        uival = uival + i;
        break;
    case obj_type::LONG:
        lval = lval + i;
        break;
    case obj_type::ULONG:
        uval = uval + i;
        break;
    case obj_type::FLOAT:
        fval = fval + i;
        break;
    case obj_type::DOUBLE:
        dval = dval + i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator+(int i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::INT;
        ival      = i;
        break;
    case obj_type::BOOL:
        isbool = i == 0 ? false : true;
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival + i;
        break;
    case obj_type::UINT:
        uival = uival + i;
        break;
    case obj_type::LONG:
        lval = lval + i;
        break;
    case obj_type::ULONG:
        uval = uval + i;
        break;
    case obj_type::FLOAT:
        fval = fval + i;
        break;
    case obj_type::DOUBLE:
        dval = dval + i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator+(unsigned int i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::UINT;
        uival     = i;
        break;
    case obj_type::BOOL:
        isbool = i == 0 ? false : true;
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival + i;
        break;
    case obj_type::UINT:
        uival = uival + i;
        break;
    case obj_type::LONG:
        lval = lval + i;
        break;
    case obj_type::ULONG:
        uval = uval + i;
        break;
    case obj_type::FLOAT:
        fval = fval + i;
        break;
    case obj_type::DOUBLE:
        dval = dval + i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator+(long i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::INT;
        ival      = i;
        break;
    case obj_type::BOOL:
        isbool = i == 0 ? false : true;
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival + i;
        break;
    case obj_type::UINT:
        uival = uival + i;
        break;
    case obj_type::LONG:
        lval = lval + i;
        break;
    case obj_type::ULONG:
        uval = uval + i;
        break;
    case obj_type::FLOAT:
        fval = fval + i;
        break;
    case obj_type::DOUBLE:
        dval = dval + i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator+(unsigned long i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::UINT;
        uival     = i;
        break;
    case obj_type::BOOL:
        isbool = i == 0 ? false : true;
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival + i;
        break;
    case obj_type::UINT:
        uival = uival + i;
        break;
    case obj_type::LONG:
        lval = lval + i;
        break;
    case obj_type::ULONG:
        uval = uval + i;
        break;
    case obj_type::FLOAT:
        fval = fval + i;
        break;
    case obj_type::DOUBLE:
        dval = dval + i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator+(float i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::FLOAT;
        fval      = i;
        break;
    case obj_type::BOOL:
        if (fabs(i) < 1e-6)
        {
            isbool = false;
        }
        else
        {
            isbool = true;
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival + i;
        break;
    case obj_type::UINT:
        uival = uival + i;
        break;
    case obj_type::LONG:
        lval = lval + i;
        break;
    case obj_type::ULONG:
        uval = uval + i;
        break;
    case obj_type::FLOAT:
        fval = fval + i;
        break;
    case obj_type::DOUBLE:
        dval = dval + i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator+(double i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::DOUBLE;
        dval      = i;
        break;
    case obj_type::BOOL:
        if (fabs(i) < 1e-15)
        {
            isbool = false;
        }
        else
        {
            isbool = true;
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival + i;
        break;
    case obj_type::UINT:
        uival = uival + i;
        break;
    case obj_type::LONG:
        lval = lval + i;
        break;
    case obj_type::ULONG:
        uval = uval + i;
        break;
    case obj_type::FLOAT:
        fval = fval + i;
        break;
    case obj_type::DOUBLE:
        dval = dval + i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}

obj_val &obj_val::operator-(long long i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::LONG;
        lval      = -i;
        break;
    case obj_type::BOOL:
        if (i != 0)
        {
            if (isbool)
            {
                isbool = false;
            }
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival - i;
        break;
    case obj_type::UINT:
        uival = uival - i;
        break;
    case obj_type::LONG:
        lval = lval - i;
        break;
    case obj_type::ULONG:
        uval = uval - i;
        break;
    case obj_type::FLOAT:
        fval = fval - i;
        break;
    case obj_type::DOUBLE:
        dval = dval - i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator-(unsigned long long i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::ULONG;
        uval      = i;
        break;
    case obj_type::BOOL:
        if (i != 0)
        {
            if (isbool)
            {
                isbool = false;
            }
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival - i;
        break;
    case obj_type::UINT:
        uival = uival - i;
        break;
    case obj_type::LONG:
        lval = lval - i;
        break;
    case obj_type::ULONG:
        uval = uval - i;
        break;
    case obj_type::FLOAT:
        fval = fval - i;
        break;
    case obj_type::DOUBLE:
        dval = dval - i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator-(int i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::INT;
        ival      = i;
        break;
    case obj_type::BOOL:
        if (i != 0)
        {
            if (isbool)
            {
                isbool = false;
            }
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival - i;
        break;
    case obj_type::UINT:
        uival = uival - i;
        break;
    case obj_type::LONG:
        lval = lval - i;
        break;
    case obj_type::ULONG:
        uval = uval - i;
        break;
    case obj_type::FLOAT:
        fval = fval - i;
        break;
    case obj_type::DOUBLE:
        dval = dval - i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator-(unsigned int i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::UINT;
        uival     = i;
        break;
    case obj_type::BOOL:
        if (i != 0)
        {
            if (isbool)
            {
                isbool = false;
            }
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival - i;
        break;
    case obj_type::UINT:
        uival = uival - i;
        break;
    case obj_type::LONG:
        lval = lval - i;
        break;
    case obj_type::ULONG:
        uval = uval - i;
        break;
    case obj_type::FLOAT:
        fval = fval - i;
        break;
    case obj_type::DOUBLE:
        dval = dval - i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator-(long i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::INT;
        ival      = i;
        break;
    case obj_type::BOOL:
        if (i != 0)
        {
            if (isbool)
            {
                isbool = false;
            }
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival - i;
        break;
    case obj_type::UINT:
        uival = uival - i;
        break;
    case obj_type::LONG:
        lval = lval - i;
        break;
    case obj_type::ULONG:
        uval = uval - i;
        break;
    case obj_type::FLOAT:
        fval = fval - i;
        break;
    case obj_type::DOUBLE:
        dval = dval - i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator-(unsigned long i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::UINT;
        uival     = i;
        break;
    case obj_type::BOOL:
        if (i != 0)
        {
            if (isbool)
            {
                isbool = false;
            }
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival - i;
        break;
    case obj_type::UINT:
        uival = uival - i;
        break;
    case obj_type::LONG:
        lval = lval - i;
        break;
    case obj_type::ULONG:
        uval = uval - i;
        break;
    case obj_type::FLOAT:
        fval = fval - i;
        break;
    case obj_type::DOUBLE:
        dval = dval - i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator-(float i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::FLOAT;
        fval      = i;
        break;
    case obj_type::BOOL:
        if (fabs(i) < 1e-6)
        {
            //isbool = false;
        }
        else
        {
            if (isbool)
            {
                isbool = false;
            }
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival - i;
        break;
    case obj_type::UINT:
        uival = uival - i;
        break;
    case obj_type::LONG:
        lval = lval - i;
        break;
    case obj_type::ULONG:
        uval = uval - i;
        break;
    case obj_type::FLOAT:
        fval = fval - i;
        break;
    case obj_type::DOUBLE:
        dval = dval - i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}
obj_val &obj_val::operator-(double i)
{
    switch (_val_type)
    {
    case obj_type::NIL:
        _val_type = obj_type::DOUBLE;
        dval      = i;
        break;
    case obj_type::BOOL:
        if (fabs(i) < 1e-15)
        {
            //isbool = false;
        }
        else
        {
            if (isbool)
            {
                isbool = false;
            }
        }
        break;
    case obj_type::STRING:
        break;
    case obj_type::INT:
        ival = ival - i;
        break;
    case obj_type::UINT:
        uival = uival - i;
        break;
    case obj_type::LONG:
        lval = lval - i;
        break;
    case obj_type::ULONG:
        uval = uval - i;
        break;
    case obj_type::FLOAT:
        fval = fval - i;
        break;
    case obj_type::DOUBLE:
        dval = dval - i;
        break;
    case obj_type::OBJECT:
        break;
    case obj_type::ARRAY:
        break;
    default:
        break;
    }
    return *this;
}

bool obj_val::operator==(const obj_val &v)
{
    if (this == &v)
    {
        return true;
    }

    switch (_val_type)
    {
    case obj_type::NIL:
        if (v.get_type() == obj_type::NIL)
        {
            return true;
        }
        return false;
        break;
    case obj_type::BOOL:
        if (v.get_type() == obj_type::BOOL)
        {
            return isbool == v.isbool;
        }
        return false;
        break;
    case obj_type::STRING:
        if (v.get_type() == obj_type::STRING)
        {
            if (v.length != length)
            {
                return false;
            }

            if (length < 8)
            {
                for (unsigned int jj = 0; jj < length; jj++)
                {
                    if (name[jj] != v.name[jj])
                    {
                        return false;
                    }
                }
                return true;
            }
            else
            {
                if (str == nullptr)
                {
                    return false;
                }
                if (v.str == nullptr)
                {
                    return false;
                }

                for (unsigned int jj = 0; jj < length; jj++)
                {
                    if (str[jj] != v.str[jj])
                    {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
        break;
    case obj_type::INT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                return ival == v.ival;
                break;
            case obj_type::UINT:
                if (ival < 0)
                {
                    return false;
                }
                {
                    unsigned long long t_i = abs(ival);
                    return t_i == v.uival;
                }

                break;
            case obj_type::LONG:
                return ival == v.lval;
                break;
            case obj_type::ULONG:
                if (ival < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(ival);
                    return t_i == v.uval;
                }
                break;
            case obj_type::FLOAT:
                if (fabs(ival - v.fval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(ival - v.dval) < 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::UINT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (v.ival < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.ival);
                    return uival == t_i;
                }

                break;
            case obj_type::UINT:
                return uival == v.uival;
                break;
            case obj_type::LONG:
                if (v.lval < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.lval);
                    return uival == t_i;
                }

                break;
            case obj_type::ULONG:
                return uival == v.uval;
                break;
            case obj_type::FLOAT:
                if (fabs(uival - v.fval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(uival - v.dval) < 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::LONG:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                return lval == v.ival;
                break;
            case obj_type::UINT:
                if (lval < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(lval);
                    return t_i == v.uival;
                }

                break;
            case obj_type::LONG:
                return lval == v.lval;
                break;
            case obj_type::ULONG:
                if (lval < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(lval);
                    return t_i == v.uval;
                }

                break;
            case obj_type::FLOAT:
                if (fabs(lval - v.fval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(lval - v.dval) < 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::ULONG:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (v.ival < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(v.ival);
                    return uval == t_i;
                }

                break;
            case obj_type::UINT:
                return uval == v.uival;
                break;
            case obj_type::LONG:
                if (v.lval < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(v.lval);
                    return uval == t_i;
                }

                break;
            case obj_type::ULONG:
                return uval == v.uval;
                break;
            case obj_type::FLOAT:
                if (fabs(uval - v.fval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(uval - v.dval) < 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::FLOAT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (fabs(fval - v.ival) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::UINT:
                if (fabs(fval - v.uival) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::LONG:
                if (fabs(fval - v.lval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::ULONG:
                if (fabs(fval - v.uval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::FLOAT:
                if (fabs(fval - v.fval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(fval - v.dval) < 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::DOUBLE:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (fabs(dval - v.ival) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::UINT:
                if (fabs(dval - v.uival) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::LONG:
                if (fabs(dval - v.lval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::ULONG:
                if (fabs(dval - v.uval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::FLOAT:
                if (fabs(dval - v.fval) < 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(dval - v.dval) < 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::OBJECT:
        if (v.get_type() == obj_type::OBJECT)
        {
            if (obj->_data.size() != v.obj->_data.size())
            {
                return false;
            }
            for (auto iter1 = obj->_data.begin(), iter2 = v.obj->_data.begin(); iter1 != obj->_data.end() && iter2 != v.obj->_data.end();)
            {
                if (iter1->first != iter2->first || iter1->second != iter2->second)
                {
                    return false;
                }
            }
            return true;
        }
        return false;
        break;
    case obj_type::ARRAY:
        if (v.get_type() == obj_type::ARRAY)
        {
            if (array_val->_data.size() != v.array_val->_data.size())
            {
                return false;
            }
            for (auto iter1 = array_val->_data.begin(), iter2 = v.array_val->_data.begin(); iter1 != array_val->_data.end() && iter2 != v.array_val->_data.end();)
            {
                if (*iter1 != *iter2)
                {
                    return false;
                }
            }
            return true;
        }
        return false;
        break;
    default:
        break;
    }
    return false;
}

bool obj_val::operator!=(const obj_val &v)
{
    if (this == &v)
    {
        return false;
    }

    switch (_val_type)
    {
    case obj_type::NIL:
        if (v.get_type() == obj_type::NIL)
        {
            return false;
        }
        return true;
        break;
    case obj_type::BOOL:
        if (v.get_type() == obj_type::BOOL)
        {
            return isbool != v.isbool;
        }
        return true;
        break;
    case obj_type::STRING:
        if (v.get_type() == obj_type::STRING)
        {
            if (v.length != length)
            {
                return true;
            }

            if (length < 8)
            {
                for (unsigned int jj = 0; jj < length; jj++)
                {
                    if (name[jj] != v.name[jj])
                    {
                        return true;
                    }
                }
                return false;
            }
            else
            {
                if (str == nullptr)
                {
                    return true;
                }
                if (v.str == nullptr)
                {
                    return true;
                }

                for (unsigned int jj = 0; jj < length; jj++)
                {
                    if (str[jj] != v.str[jj])
                    {
                        return true;
                    }
                }
                return false;
            }
        }
        return true;
        break;
    case obj_type::INT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return true;
                break;
            case obj_type::BOOL:
                return true;
                break;
            case obj_type::STRING:
                return true;
                break;
            case obj_type::INT:
                return ival != v.ival;
                break;
            case obj_type::UINT:
                if (ival < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(ival);
                    return t_i != v.uival;
                }

                break;
            case obj_type::LONG:
                return ival != v.lval;
                break;
            case obj_type::ULONG:
                if (ival < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(ival);
                    return t_i != v.uval;
                }

                break;
            case obj_type::FLOAT:
                if (fabs(ival - v.fval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::DOUBLE:
                if (fabs(ival - v.dval) < 1e-15)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::OBJECT:
                return true;
                break;
            case obj_type::ARRAY:
                return true;
                break;
            default:
                return true;
            }
        }
        return true;
        break;
    case obj_type::UINT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return true;
                break;
            case obj_type::BOOL:
                return true;
                break;
            case obj_type::STRING:
                return true;
                break;
            case obj_type::INT:
                if (v.ival < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.ival);
                    return uival != t_i;
                }

                break;
            case obj_type::UINT:
                return uival != v.uival;
                break;
            case obj_type::LONG:
                if (v.lval < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.lval);
                    return uival != t_i;
                }

                break;
            case obj_type::ULONG:

                return uival != v.uval;
                break;
            case obj_type::FLOAT:
                if (fabs(uival - v.fval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::DOUBLE:
                if (fabs(uival - v.dval) < 1e-15)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::OBJECT:
                return true;
                break;
            case obj_type::ARRAY:
                return true;
                break;
            default:
                return true;
            }
        }
        return true;
        break;
    case obj_type::LONG:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return true;
                break;
            case obj_type::BOOL:
                return true;
                break;
            case obj_type::STRING:
                return true;
                break;
            case obj_type::INT:
                return lval != v.ival;
                break;
            case obj_type::UINT:
                if (lval < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(lval);
                    return t_i != v.uival;
                }

                break;
            case obj_type::LONG:
                return lval != v.lval;
                break;
            case obj_type::ULONG:
                if (lval < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(lval);
                    return t_i != v.uval;
                }

                break;
            case obj_type::FLOAT:
                if (fabs(lval - v.fval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::DOUBLE:
                if (fabs(lval - v.dval) < 1e-15)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::OBJECT:
                return true;
                break;
            case obj_type::ARRAY:
                return true;
                break;
            default:
                return true;
            }
        }
        return true;
        break;
    case obj_type::ULONG:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return true;
                break;
            case obj_type::BOOL:
                return true;
                break;
            case obj_type::STRING:
                return true;
                break;
            case obj_type::INT:
                if (v.ival < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.ival);
                    return uval != t_i;
                }

                break;
            case obj_type::UINT:
                return uval != v.uival;
                break;
            case obj_type::LONG:
                if (v.lval < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.lval);
                    return uval != t_i;
                }

                break;
            case obj_type::ULONG:
                return uval != v.uval;
                break;
            case obj_type::FLOAT:
                if (fabs(uval - v.fval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::DOUBLE:
                if (fabs(uval - v.dval) < 1e-15)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::OBJECT:
                return true;
                break;
            case obj_type::ARRAY:
                return true;
                break;
            default:
                return true;
            }
        }
        return true;
        break;
    case obj_type::FLOAT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return true;
                break;
            case obj_type::BOOL:
                return true;
                break;
            case obj_type::STRING:
                return true;
                break;
            case obj_type::INT:
                if (fabs(fval - v.ival) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::UINT:
                if (fabs(fval - v.uival) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::LONG:
                if (fabs(fval - v.lval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::ULONG:
                if (fabs(fval - v.uval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::FLOAT:
                if (fabs(fval - v.fval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::DOUBLE:
                if (fabs(fval - v.dval) < 1e-15)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::OBJECT:
                return true;
                break;
            case obj_type::ARRAY:
                return true;
                break;
            default:
                return true;
            }
        }
        return true;
        break;
    case obj_type::DOUBLE:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return true;
                break;
            case obj_type::BOOL:
                return true;
                break;
            case obj_type::STRING:
                return true;
                break;
            case obj_type::INT:
                if (fabs(dval - v.ival) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::UINT:
                if (fabs(dval - v.uival) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::LONG:
                if (fabs(dval - v.lval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::ULONG:
                if (fabs(dval - v.uval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::FLOAT:
                if (fabs(dval - v.fval) < 1e-6)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::DOUBLE:
                if (fabs(dval - v.dval) < 1e-15)
                {
                    return false;
                }
                return true;
                break;
            case obj_type::OBJECT:
                return true;
                break;
            case obj_type::ARRAY:
                return true;
                break;
            default:
                return true;
            }
        }
        return true;
        break;
    case obj_type::OBJECT:
        if (v.get_type() == obj_type::OBJECT)
        {
            if (obj->_data.size() != v.obj->_data.size())
            {
                return true;
            }
            for (auto iter1 = obj->_data.begin(), iter2 = v.obj->_data.begin(); iter1 != obj->_data.end() && iter2 != v.obj->_data.end();)
            {
                if (iter1->first == iter2->first && iter1->second == iter2->second)
                {
                    ++iter1;
                    ++iter2;
                    continue;
                }
                return true;
            }
            return false;
        }
        return true;
        break;
    case obj_type::ARRAY:
        if (v.get_type() == obj_type::ARRAY)
        {
            if (array_val->_data.size() != v.array_val->_data.size())
            {
                return true;
            }
            for (auto iter1 = array_val->_data.begin(), iter2 = v.array_val->_data.begin(); iter1 != array_val->_data.end() && iter2 != v.array_val->_data.end();)
            {
                if (*iter1 == *iter2)
                {
                    ++iter1;
                    ++iter2;
                    continue;
                }
                return true;
            }
            return false;
        }
        return true;
        break;
    default:
        break;
    }
    return true;
}

bool obj_val::operator>(const obj_val &v)
{
    if (this == &v)
    {
        return false;
    }

    switch (_val_type)
    {
    case obj_type::NIL:
        return false;
        break;
    case obj_type::BOOL:
        if (isbool == true && v.isbool == false)
        {
            return true;
        }
        return false;
        break;
    case obj_type::STRING:
        if (v.get_type() == obj_type::STRING)
        {
            if (v.length != length)
            {
                return false;
            }

            if (length < 8)
            {
                for (unsigned int jj = 0; jj < length; jj++)
                {
                    if (name[jj] <= v.name[jj])
                    {
                        return false;
                    }
                }
                return true;
            }
            else
            {
                if (str == nullptr)
                {
                    return false;
                }
                if (v.str == nullptr)
                {
                    return false;
                }

                for (unsigned int jj = 0; jj < length; jj++)
                {

                    if (str[jj] <= v.str[jj])
                    {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
        break;
    case obj_type::INT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                return ival > v.ival;
                break;
            case obj_type::UINT:
                if (ival < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(ival);
                    return t_i > v.uival;
                }

                break;
            case obj_type::LONG:
                return ival > v.lval;
                break;
            case obj_type::ULONG:
                if (ival < 0)
                {
                    return false;
                }
                {
                    unsigned long long t_i = abs(ival);
                    return t_i > v.uval;
                }

                break;
            case obj_type::FLOAT:
                if (ival > v.fval && fabs(ival - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (ival > v.dval && fabs(ival - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::UINT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (v.ival < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(v.ival);
                    return uival > t_i;
                }

                break;
            case obj_type::UINT:
                return uival > v.uival;
                break;
            case obj_type::LONG:
                if (v.lval < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(v.lval);
                    return uival > t_i;
                }

                break;
            case obj_type::ULONG:
                return uival > v.uval;
                break;
            case obj_type::FLOAT:
                if (uival > v.fval && fabs(uival - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (uival > v.dval && fabs(uival - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::LONG:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                return lval >= v.ival;
                break;
            case obj_type::UINT:
                if (lval < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(lval);
                    return t_i >= v.uival;
                }

                break;
            case obj_type::LONG:
                return lval >= v.lval;
                break;
            case obj_type::ULONG:
                if (lval < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(lval);
                    return t_i >= v.uval;
                }

                break;
            case obj_type::FLOAT:
                if (lval > v.fval && fabs(lval - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (lval > v.dval && fabs(lval - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::ULONG:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (v.ival < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(v.ival);
                    return uval >= t_i;
                }

                break;
            case obj_type::UINT:
                return uval >= v.uival;
                break;
            case obj_type::LONG:
                if (v.lval < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(v.lval);
                    return uval >= t_i;
                }

                break;
            case obj_type::ULONG:
                return uval >= v.uval;
                break;
            case obj_type::FLOAT:
                if (uval > v.fval && fabs(uval - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (uval > v.dval && fabs(uval - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::FLOAT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (fval > v.ival && fabs(fval - v.ival) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::UINT:
                if (fval > v.uival && fabs(fval - v.uival) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::LONG:
                if (fval > v.lval && fabs(fval - v.lval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::ULONG:
                if (fval > v.uval && fabs(fval - v.uval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::FLOAT:
                if (fval > v.fval && fabs(fval - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fval > v.dval && fabs(fval - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::DOUBLE:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (dval > v.ival && fabs(dval - v.ival) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::UINT:
                if (dval - v.uival && fabs(dval - v.uival) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::LONG:
                if (dval > v.lval && fabs(dval - v.lval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::ULONG:
                if (dval > v.uval && fabs(dval - v.uval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::FLOAT:
                if (dval > v.fval && fabs(dval - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (dval > v.dval && fabs(dval - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::OBJECT:

        return false;
        break;
    case obj_type::ARRAY:

        return false;
        break;
    default:
        break;
    }
    return false;
}

bool obj_val::operator>=(const obj_val &v)
{
    if (this == &v)
    {
        return true;
    }

    switch (_val_type)
    {
    case obj_type::NIL:
        if (v.get_type() == obj_type::NIL)
        {
            return true;
        }
        return false;
        break;
    case obj_type::BOOL:
        if (v.get_type() == obj_type::BOOL)
        {
            return isbool == v.isbool;
        }
        return false;
        break;
    case obj_type::STRING:
        if (v.get_type() == obj_type::STRING)
        {
            if (v.length != length)
            {
                return false;
            }

            if (length < 8)
            {
                for (unsigned int jj = 0; jj < length; jj++)
                {
                    if (name[jj] < v.name[jj])
                    {
                        return false;
                    }
                }
                return true;
            }
            else
            {
                if (str == nullptr)
                {
                    return false;
                }
                if (v.str == nullptr)
                {
                    return false;
                }

                for (unsigned int jj = 0; jj < length; jj++)
                {

                    if (str[jj] < v.str[jj])
                    {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
        break;
    case obj_type::INT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                return ival >= v.ival;
                break;
            case obj_type::UINT:
                if (ival < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(ival);
                    return t_i >= v.uival;
                }

                break;
            case obj_type::LONG:
                return ival >= v.lval;
                break;
            case obj_type::ULONG:
                if (ival < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(ival);
                    return t_i >= v.uval;
                }

                break;
            case obj_type::FLOAT:
                if (fabs(ival - v.fval) < 1e-6)
                {
                    return true;
                }
                else if (ival > v.fval && fabs(ival - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(ival - v.dval) < 1e-15)
                {
                    return true;
                }
                else if (ival > v.dval && fabs(ival - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::UINT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (v.ival < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.ival);
                    return uival >= t_i;
                }

                break;
            case obj_type::UINT:
                return uival >= v.uival;
                break;
            case obj_type::LONG:
                if (v.lval < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.lval);
                    return uival >= t_i;
                }

                break;
            case obj_type::ULONG:
                return uival >= v.uval;
                break;
            case obj_type::FLOAT:
                if (fabs(uival - v.fval) < 1e-6)
                {
                    return true;
                }
                else if (uival > v.fval && fabs(uival - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(uival - v.dval) < 1e-15)
                {
                    return true;
                }
                else if (uival > v.dval && fabs(uival - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::LONG:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                return lval >= v.ival;
                break;
            case obj_type::UINT:
                if (lval < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(lval);
                    return t_i >= v.uival;
                }

                break;
            case obj_type::LONG:
                return lval >= v.lval;
                break;
            case obj_type::ULONG:
                if (lval < 0)
                {
                    return false;
                }

                {
                    unsigned long long t_i = abs(lval);
                    return t_i >= v.uval;
                }

                break;
            case obj_type::FLOAT:
                if (fabs(lval - v.fval) < 1e-6)
                {
                    return true;
                }
                else if (lval > v.fval && fabs(lval - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(lval - v.dval) < 1e-15)
                {
                    return true;
                }
                else if (lval > v.dval && fabs(lval - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::ULONG:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (v.ival < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.ival);
                    return uval >= t_i;
                }

                break;
            case obj_type::UINT:
                return uval >= v.uival;
                break;
            case obj_type::LONG:
                if (v.lval < 0)
                {
                    return true;
                }

                {
                    unsigned long long t_i = abs(v.lval);
                    return uval >= t_i;
                }

                break;
            case obj_type::ULONG:
                return uval >= v.uval;
                break;
            case obj_type::FLOAT:
                if (fabs(uval - v.fval) < 1e-6)
                {
                    return true;
                }
                else if (uval > v.fval && fabs(uval - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(uval - v.dval) < 1e-15)
                {
                    return true;
                }
                else if (uval > v.dval && fabs(uval - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::FLOAT:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (fabs(fval - v.ival) < 1e-6)
                {
                    return true;
                }
                else if (fval > v.ival && fabs(fval - v.ival) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::UINT:
                if (fabs(fval - v.uival) < 1e-6)
                {
                    return true;
                }
                else if (fval > v.uival && fabs(fval - v.uival) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::LONG:
                if (fabs(fval - v.lval) < 1e-6)
                {
                    return true;
                }
                else if (fval > v.lval && fabs(fval - v.lval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::ULONG:
                if (fabs(fval - v.uval) < 1e-6)
                {
                    return true;
                }
                else if (fval > v.uval && fabs(fval - v.uval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::FLOAT:
                if (fabs(fval - v.fval) < 1e-6)
                {
                    return true;
                }
                else if (fval > v.fval && fabs(fval - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(fval - v.dval) < 1e-15)
                {
                    return true;
                }
                else if (fval > v.dval && fabs(fval - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::DOUBLE:
        if (v.is_number())
        {
            switch (v.get_type())
            {
            case obj_type::NIL:
                return false;
                break;
            case obj_type::BOOL:
                return false;
                break;
            case obj_type::STRING:
                return false;
                break;
            case obj_type::INT:
                if (fabs(dval - v.ival) < 1e-6)
                {
                    return true;
                }
                else if (dval > v.ival && fabs(dval - v.ival) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::UINT:
                if (fabs(dval - v.uival) < 1e-6)
                {
                    return true;
                }
                else if (dval > v.uival && fabs(dval - v.uival) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::LONG:
                if (fabs(dval - v.lval) < 1e-6)
                {
                    return true;
                }
                else if (dval > v.lval && fabs(dval - v.lval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::ULONG:
                if (fabs(dval - v.uval) < 1e-6)
                {
                    return true;
                }
                else if (dval > v.uval && fabs(dval - v.uval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::FLOAT:
                if (fabs(dval - v.fval) < 1e-6)
                {
                    return true;
                }
                else if (dval > v.fval && fabs(dval - v.fval) > 1e-6)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::DOUBLE:
                if (fabs(dval - v.dval) < 1e-15)
                {
                    return true;
                }
                else if (dval > v.dval && fabs(dval - v.dval) > 1e-15)
                {
                    return true;
                }
                return false;
                break;
            case obj_type::OBJECT:
                return false;
                break;
            case obj_type::ARRAY:
                return false;
                break;
            default:
                return false;
            }
        }
        return false;
        break;
    case obj_type::OBJECT:

        return false;
        break;
    case obj_type::ARRAY:

        return false;
        break;
    default:
        break;
    }
    return false;
}

obj_val &obj_val::find(const obj_val &v)
{
    if (_val_type == obj_type::ARRAY)
    {
        for (unsigned int jj = 0; jj < array_val->_data.size(); jj++)
        {
            if (array_val->_data[jj] == v)
            {
                return array_val->_data[jj];
            }
        }
        throw "Not found in this array";
    }
    else if (_val_type == obj_type::OBJECT)
    {
        for (unsigned int jj = 0; jj < obj->_data.size(); jj++)
        {
            if (obj->_data[jj].second == v)
            {
                return obj->_data[jj].second;
            }
        }
        throw "Not found in this object";
    }
    throw "This var not is object or array";
}
obj_val &obj_val::find(const std::string &v)
{
    if (_val_type == obj_type::OBJECT)
    {
        for (unsigned int jj = 0; jj < obj->_data.size(); jj++)
        {
            if (obj->_data[jj].first == v)
            {
                return obj->_data[jj].second;
            }
        }
        throw "Not found in this object";
    }

    throw "This var not is object find";
}

bool obj_val::unset(const std::string &v)
{
    if (_val_type == obj_type::OBJECT)
    {
        for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
        {
            if (iter->first == v)
            {
                iter = obj->_data.erase(iter);
                return true;
            }
            ++iter;
        }
    }
    return false;
}

bool obj_val::erase(const std::string &v)
{
    if (_val_type == obj_type::OBJECT)
    {
        for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
        {
            if (iter->first == v)
            {
                iter = obj->_data.erase(iter);
                return true;
            }
            ++iter;
        }
    }
    return false;
}

bool obj_val::erase(const obj_val &key)
{
    if (_val_type == obj_type::OBJECT)
    {
        for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
        {
            if (iter->second == key)
            {
                iter = obj->_data.erase(iter);
                return true;
            }
            ++iter;
        }
    }
    else if (_val_type == obj_type::ARRAY)
    {
        for (auto iter = array_val->_data.begin(); iter != array_val->_data.end();)
        {
            if ((*iter) == key)
            {
                iter = array_val->_data.erase(iter);
                return true;
            }
            ++iter;
        }
    }
    return false;
}

obj_val::obj_val(const std::initializer_list<std::string> nsl)
{
    _val_type = obj_type::ARRAY;
    array_val = new obj_array;
    array_val->_data.reserve(nsl.size());
    for (auto &v : nsl)
    {
        array_val->_data.emplace_back(v);
    }
}

obj_val::obj_val(const std::initializer_list<double> nsl)
{
    _val_type = obj_type::ARRAY;
    array_val = new obj_array;
    array_val->_data.reserve(nsl.size());
    for (auto &v : nsl)
    {
        array_val->_data.emplace_back(v);
    }
}

obj_val::obj_val(const std::initializer_list<std::pair<std::string, double>> nsl)
{
    _val_type = obj_type::OBJECT;
    obj       = new obj_t;

    obj->_data.reserve(nsl.size());
    for (auto &v : nsl)
    {
        obj->_data.emplace_back(v.first, v.second);
    }
}

//    void obj_val::set_keyname(const std::string &key)
//     {
//         if(_val_type == obj_type::ARRAY)
//         {
//             array_val->key=key;
//         }
//     }
//     std::string obj_val::get_keyname()
//     {
//         if(_val_type == obj_type::ARRAY)
//         {
//             return array_val->key;
//         }
//         return "";
//     }

std::vector<obj_val>::iterator obj_val::begin()
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data.begin();
    }
    throw "This not array";
}
std::vector<obj_val>::iterator obj_val::end()
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data.end();
    }
    throw "This not array";
}
std::vector<obj_val>::const_iterator obj_val::cbegin() const
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data.cbegin();
    }
    throw "This not array";
}
std::vector<obj_val>::const_iterator obj_val::cend() const
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data.cend();
    }
    throw "This not array";
}

//obj
std::vector<std::pair<std::string, obj_val>>::iterator obj_val::obj_begin()
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data.begin();
    }
    throw "This not array obj_begin";
}
std::vector<std::pair<std::string, obj_val>>::iterator obj_val::obj_end()
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data.end();
    }
    throw "This not array obj_end";
}
std::vector<std::pair<std::string, obj_val>>::const_iterator obj_val::obj_cbegin() const
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data.cbegin();
    }
    throw "This not array obj_cbegin";
}
std::vector<std::pair<std::string, obj_val>>::const_iterator obj_val::obj_cend() const
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data.cend();
    }
    throw "This not array obj_cend";
}

std::pair<std::string, obj_val> obj_val::get_obj_val(unsigned int index)
{
    if (_val_type == obj_type::OBJECT)
    {
        if (index < obj->_data.size())
        {
            return obj->_data[index];
        }
        throw "Out of range get_obj_val";
    }
    throw "This not object get_obj_val";
}

obj_val obj_val::get_array_val(unsigned int index)
{
    if (_val_type == obj_type::ARRAY)
    {
        if (index < array_val->_data.size())
        {
            return array_val->_data[index];
        }
        throw "Out of range get_array_val";
    }
    throw "This not array get_array_val";
}

std::pair<std::string, obj_val> &obj_val::ref_obj_val(unsigned int index)
{
    if (_val_type == obj_type::OBJECT)
    {
        if (index < obj->_data.size())
        {
            return obj->_data[index];
        }
        throw "Out of range ref_obj_val";
    }
    throw "This not object ref_obj_val";
}

obj_val &obj_val::ref_array_val(unsigned int index)
{
    if (_val_type == obj_type::ARRAY)
    {
        if (index < array_val->_data.size())
        {
            return array_val->_data[index];
        }
        throw "Out of range ref_array_val ref_array_val";
    }
    throw "This not array ref_array_val";
}

std::pair<std::string, obj_val> &obj_val::cref_obj_val(unsigned int index) const
{
    if (_val_type == obj_type::OBJECT)
    {
        if (index < obj->_data.size())
        {
            return obj->_data[index];
        }
        throw "Out of range cref_obj_val";
    }
    throw "This not object cref_obj_val";
}

obj_val &obj_val::cref_array_val(unsigned int index) const
{
    if (_val_type == obj_type::ARRAY)
    {
        if (index < array_val->_data.size())
        {
            return array_val->_data[index];
        }
        throw "Out of range cref_array_val";
    }
    throw "This not array cref_array_val";
}

std::vector<std::pair<std::string, obj_val>> obj_val::get_obj()
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data;
    }
    else if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
        return obj->_data;
    }
    throw "This not object get_obj";
}

std::vector<obj_val> obj_val::get_array()
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data;
    }
    else if (_val_type == obj_type::NIL)
    {
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
        return array_val->_data;
    }
    throw "This not array get_array";
}

std::vector<std::pair<std::string, obj_val>> &obj_val::ref_obj()
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data;
    }
    else if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
        return obj->_data;
    }
    throw "This not object ref_obj";
}

std::vector<obj_val> &obj_val::ref_array()
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data;
    }
    else if (_val_type == obj_type::NIL)
    {
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
        return array_val->_data;
    }
    throw "This not array ref_array";
}

std::vector<std::pair<std::string, obj_val>> &obj_val::cref_obj() const
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data;
    }
    throw "This not object cref_obj";
}

std::vector<obj_val> &obj_val::cref_array() const
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data;
    }
    throw "This not array cref_array";
}

bool obj_val::isset(const std::string &key)
{
    if (_val_type == obj_type::OBJECT)
    {
        for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
        {
            if (iter->first == key)
            {
                return true;
            }
            ++iter;
        }
    }
    return false;
}

//json begin
void obj_val::from_json(const std::string &json_str)
{
    unsigned int offset = 0;
    if (json_str.length() > 0xFFFFFF)
    {
        return;
    }
    // 分出数组或对象 消除前面空格
    for (unsigned int i = 0; i < json_str.length(); i++)
    {
        if (json_str[i] == 0x7b || json_str[i] == 0x5b)
        {
            offset = i;
            break;
        }
    }

    if (json_str[offset] == 0x7b)
    {
        // 对象情况
        if (_val_type != obj_type::OBJECT)
        {
            clear();
        }
        _val_type = obj_type::OBJECT;
        obj       = new obj_t;
        JSON_OBJ(json_str, *obj, offset);
    }
    else if (json_str[offset] == 0x5b)
    {
        // 数组情况
        if (_val_type != obj_type::ARRAY)
        {
            clear();
        }
        _val_type = obj_type::ARRAY;
        array_val = new obj_array;
        JSON_ARRAY(json_str, *array_val, offset);
    }
}

std::string obj_val::JSON_STRING(const std::string &jsonstr, unsigned int &offset)
{
    unsigned int j = 0;
    std::string str;
    j = jsonstr.length();
    if (j > 200)
    {
        str.reserve(1024);
    }

    j = 0;
    if (jsonstr[offset] == 0x22)
    {
        offset++;
    }
    for (j = offset; j < jsonstr.length(); j++)
    {

        if (jsonstr[j] == 0x5c)//'\'
        {
            // 处理有斜杠情况 not slash
            switch (jsonstr[j + 1])
            {
            case 0x22://"
                str += 0x22;
                j += 1;
                break;
            case 0x5c://'\'
                str += 0x5c;
                j += 1;
                break;
            // case 0x2f: //\/
            case 0x62://\b
                str += 0x08;
                j += 1;
                break;
            case 0x66://\f
                str += 0x0c;
                j += 1;
                break;
            case 0x6e://\n
                str += 0x0a;
                j += 1;
                break;
            case 0x72://\r
                str += 0x0d;
                j += 1;
                break;
            case 0x74://\t
                str += 0x09;
                j += 1;
                break;
            case 'u':
                // str+=json.substr(j,6);
                {
                    unsigned char c[10] = {0x00};
                    unsigned char ch;
                    unsigned int temp;
                    auto chartoint = [](unsigned char &ch, unsigned char &c)
                    {
                        if (ch >= '0' && ch <= '9')
                            c = (unsigned char)(ch - '0');
                        if (ch >= 'a' && ch <= 'f')
                            c = (unsigned char)(ch - 'a' + 10);
                        if (ch >= 'A' && ch <= 'F')
                            c = (unsigned char)(ch - 'A' + 10);
                    };

                    // 检查是不是emoji两个转码符一共12个字符 this emoji char
                    if (jsonstr[j + 2] == 'd' && jsonstr[j + 6] == 0x5c && jsonstr[j + 7] == 'u' &&
                        jsonstr[j + 8] == 'd')
                    {

                        // 转换成utf16
                        for (int si = 2, cj = 0; si < 12; si++)
                        {

                            if (jsonstr[j + si] != 0x5c && jsonstr[j + si] != 'u')
                            {
                                ch = jsonstr[j + si];
                                chartoint(ch, c[cj]);
                                cj++;
                            }
                        }

                        unsigned short ca = c[0] << 12 | c[1] << 8 | c[2] << 4 | c[3];
                        unsigned short cb = c[4] << 12 | c[5] << 8 | c[6] << 4 | c[7];
                        temp              = 0x10000 | ((ca - 0xD800) << 10) | (cb - 0xDC00);
                        // Unicode码再转换成utf-8
                        c[3] = (temp & 0x3F) | 0x80;
                        c[2] = ((temp >> 6) & 0x3F) | 0x80;
                        c[1] = ((temp >> 12) & 0x3F) | 0x80;
                        c[0] = ((temp >> 18) & 0x07) | 0xF0;
                        c[4] = '\0';
                        // emoji:c
                        // 再把utf-8组装到字符串上
                        str += c[0];
                        str += c[1];
                        str += c[2];
                        str += c[3];
                        j += 11;
                    }
                    else
                    {
                        // 只是Unicode码情况 4 char
                        for (int si = 2, cj = 0; si < 6; si++)
                        {
                            ch = jsonstr[j + si];
                            chartoint(ch, c[cj]);
                            cj++;
                        }
                        temp = c[0] << 4 | c[1];
                        temp = temp << 8 | c[2] << 4 | c[3];
                        c[3] = '\0';
                        c[2] = (temp & 0x3F) | 0x80;
                        c[1] = ((temp >> 6) & 0x3F) | 0x80;
                        c[0] = ((temp >> 12) & 0x0F) | 0xE0;

                        str += c[0];
                        str += c[1];
                        str += c[2];
                        j += 5;
                    }
                }
                break;
            default:
                str += jsonstr[j];
                str += jsonstr[j + 1];
                j += 1;
            }
        }
        else if (jsonstr[j] == 0x22)
        {
            //" 如果是单独的双引号表示值结束 if quotation marks this end
            break;
        }
        else
        {
            str += jsonstr[j];
            if (str.length() > 3145728)
            {
                offset = jsonstr.length();
                return str;
            }
        }
    }
    offset = j;
    return str;
}

bool obj_val::string_casecmp(std::string_view str1, std::string_view str2)
{
    if (str1.size() != str2.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < str1.size(); i++)
    {
        if (str1[i] != str2[i])
        {
            if (str1[i] < 91 && str1[i] > 64)
            {
                if ((str1[i] + 32) == str2[i])
                {
                    continue;
                }
            }
            else if (str2[i] < 91 && str2[i] > 64)
            {
                if (str1[i] == (str2[i] + 32))
                {
                    continue;
                }
            }
            return false;
        }
    }
    return true;
}

std::string obj_val::JSON_VALUE(const std::string &jsonstr, unsigned int &offset)
{
    unsigned int j = 0;
    std::string temp;
    if (jsonstr[offset] == 0x3a)
    {//:
        offset++;
    }
    j = offset;
    for (j = offset; j < jsonstr.length(); j++)
    {
        if (jsonstr[j] == 0x0D || jsonstr[j] == 0x0A || jsonstr[j] == 0x20 || jsonstr[j] == 0x09)
        {
            continue;
        }
        break;
    }
    offset = j;

    for (j = offset; j < jsonstr.length(); j++)
    {
        // } ],
        if (jsonstr[j] == 0x7d || jsonstr[j] == 0x5d || jsonstr[j] == 0x2c || jsonstr[j] == 0x22)
        {
            // 因为没有分隔所以要退一步 is no separation, so to step back
            j--;
            break;
        }
        else if (jsonstr[j] != 0x0D && jsonstr[j] != 0x0A && jsonstr[j] != 0x20 && jsonstr[j] != 0x09)
        {
            temp.push_back(jsonstr[j]);
            if (temp.size() > 36)
            {
                offset = jsonstr.length();
                return temp;
            }
        }
    }
    offset = j;
    return temp;
}

int obj_val::JSON_OBJ(const std::string &jsonstr, obj_t &level_obj, unsigned int &offset)
{
    unsigned int i = 0;
    std::string key, value;
    if (jsonstr[offset] == 0x7b)
    {
        offset++;
    }
    for (i = offset; i < jsonstr.length(); i++)
    {
        if (jsonstr[i] == 0x20 || jsonstr[i] == 0x0D || jsonstr[i] == 0x0A || jsonstr[i] == 0x09)
        {
            // 如果是空格 space
            continue;
        }

        if (jsonstr[i] == 0x7b)
        {
            // OBJ_ARRAY newobj;
            level_obj._data.emplace_back(key, 0);
            level_obj._data.back().second.set_obj();

            i = JSON_OBJ(jsonstr, *(level_obj._data.back().second.obj), i);
        }
        else if (jsonstr[i] == 0x7d)
        {
            // 结束本层对象 end obj
            break;
        }
        else if (jsonstr[i] == 0x5b)
        {
            //obj_array 数组
            if (key.size() > 255)
            {
                i      = jsonstr.length();
                offset = i;
                return i;
            }
            level_obj._data.emplace_back(key, 0);
            level_obj._data.back().second.set_array();

            i = JSON_ARRAY(jsonstr, *(level_obj._data.back().second.array_val), i);
        }
        else if (jsonstr[i] == 0x22)
        {
            // 遇到字符
            key = JSON_STRING(jsonstr, i);
        }
        else if (jsonstr[i] == 0x2c)
        {
            // 逗号，表示结速一个key,value
            key.clear();
            value.clear();
        }
        else if (jsonstr[i] == 0x3A)
        {
            // 冒号，表示结速一个key
            // 下一个字符如果不是[{"说明是数字或true或null
            // A JSON value can be an string,array,object,number,true,false,or null.
            // 如果是引号说明了没有下一层
            value.clear();
            if (key.size() > 255)
            {
                i      = jsonstr.length();
                offset = i;
                return i;
            }
            // 消除空格
            // jsonstr[j]!=0x0D&&jsonstr[j]!=0x0A&&jsonstr[j]!=0x20&&jsonstr[j]!=0x09
            while (jsonstr[i + 1] == 0x20 || jsonstr[i + 1] == 0x0D || jsonstr[i + 1] == 0x0A || jsonstr[i + 1] == 0x09)
            {
                ++i;
            }

            if (jsonstr[i + 1] == 0x22)
            {
                // 字符
                i++;
                value = JSON_STRING(jsonstr, i);
                level_obj._data.emplace_back(key, value);
            }
            else if (jsonstr[i + 1] == 0x5b)
            {
                // OBJ_ARRAY newobj;
                i++;
                level_obj._data.emplace_back(key, nullptr);
                level_obj._data.back().second.set_array();
                i = JSON_ARRAY(jsonstr, *(level_obj._data.back().second.array_val), i);
            }
            else if (jsonstr[i + 1] == 0x7b)
            {
                // 进入对象
                i++;
                //obj_t;
                level_obj._data.emplace_back(key, 0);
                level_obj._data.back().second.set_obj();
                i = JSON_OBJ(jsonstr, *(level_obj._data.back().second.obj), i);
            }
            else
            {
                // number,true,false,or null
                value = JSON_VALUE(jsonstr, i);
                if (string_casecmp(value, "TRUE"))
                {
                    level_obj._data.emplace_back(key, true);
                }
                else if (string_casecmp(value, "FALSE"))
                {
                    level_obj._data.emplace_back(key, false);
                }
                else if (string_casecmp(value, "NULL"))
                {
                    level_obj._data.emplace_back(key, 0);
                    level_obj._data.back().second.set_null();
                }
                else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos ||
                         value.find('E') != std::string::npos)
                {
                    double tc;
                    try
                    {
                        tc = std::stold(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0.0;
                    }
                    level_obj._data.emplace_back(key, tc);
                }
                else
                {
                    long long tc;
                    try
                    {
                        tc = std::atoll(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0;
                    }
                    level_obj._data.emplace_back(key, tc);
                }
            }
            key.clear();
            value.clear();
        }
    }
    offset = i;
    return i;
}

int obj_val::JSON_ARRAY(const std::string &jsonstr, obj_array &level_obj, unsigned int &offset)
{
    unsigned int i = 0;
    std::string key, value;
    if (jsonstr[offset] == 0x5b)
    {
        offset++;
    }
    for (i = offset; i < jsonstr.length(); i++)
    {
        if (jsonstr[i] == 0x20 || jsonstr[i] == 0x0D || jsonstr[i] == 0x0A || jsonstr[i] == 0x09)
        {
            // 如果是空格 space
            continue;
        }

        if (jsonstr[i] == 0x5b)
        {
            // 数组
            // OBJ_ARRAY newobj;
            level_obj._data.emplace_back(0);
            level_obj._data.back().set_array();

            i = JSON_ARRAY(jsonstr, *(level_obj._data.back().array_val), i);
            value.clear();
            key.clear();
        }
        else if (jsonstr[i] == 0x5d)
        {
            // 结束数组
            if (!value.empty())
            {
                if (string_casecmp(value, "TRUE"))
                {
                    level_obj._data.emplace_back(true);
                }
                else if (string_casecmp(value, "FALSE"))
                {
                    level_obj._data.emplace_back(false);
                }
                else if (string_casecmp(value, "NULL"))
                {
                    level_obj._data.emplace_back(0);
                    level_obj._data.back().set_null();
                }
                else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos ||
                         value.find('E') != std::string::npos)
                {
                    double tc;
                    try
                    {
                        tc = std::stold(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0.0;
                    }

                    level_obj._data.emplace_back(tc);
                }
                else
                {
                    long long tc;
                    try
                    {
                        tc = std::atoll(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0;
                    }
                    level_obj._data.emplace_back(tc);
                }
            }
            value.clear();
            key.clear();
            break;
        }
        else if (jsonstr[i] == 0x7b)
        {
            // 进入对象
            level_obj._data.emplace_back(value);
            level_obj._data.back().set_obj();
            i = JSON_OBJ(jsonstr, *(level_obj._data.back().obj), i);
            key.clear();
            value.clear();
        }
        else if (jsonstr[i] == 0x22)
        {
            // 遇到字符,里面肯定没有下一层了
            value = JSON_STRING(jsonstr, i);
            level_obj._data.emplace_back(value);
            value.clear();
        }
        else if (jsonstr[i] == 0x2c)
        {
            // 逗号，表示结速一个key,value
            // 冒号，表示结速一个key
            // 如果是接着一个引号，说明也是字符，没有下一层
            // A JSON value can be an string,array,object,number,true,false,or null.
            // value.clear();
            // 消除空格

            if (!value.empty())
            {
                if (string_casecmp(value, "TRUE"))
                {
                    level_obj._data.emplace_back(true);
                }
                else if (string_casecmp(value, "FALSE"))
                {
                    level_obj._data.emplace_back(false);
                }
                else if (string_casecmp(value, "NULL"))
                {
                    level_obj._data.emplace_back(0);
                    level_obj._data.back().set_null();
                }
                else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos ||
                         value.find('E') != std::string::npos)
                {
                    double tc;
                    try
                    {
                        tc = std::stold(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0.0;
                    }
                    level_obj._data.emplace_back(tc);
                }
                else
                {
                    long long tc;
                    try
                    {
                        tc = std::atoll(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0;
                    }
                    level_obj._data.emplace_back(tc);
                }
            }
            value.clear();
            key.clear();
        }
        else
        {
            value += jsonstr[i];
        }
    }
    return i;
}

// utf8转为asii 包括emoji转为ascii表示
std::string obj_val::JSON_UTF8_TO_ASCII(const char *text, unsigned int text_length)
{
    std::string ptext;
    unsigned char c, t = 0;
    unsigned char str[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    ptext.reserve(text_length * 2 + 4);

    for (unsigned int i = 0; i < text_length; i++)
    {
        c = *(text + i);
        if (c < 0x80)
        {
            switch (c)
            {
            case 0x22://"
                ptext.push_back(0x5c);
                ptext.push_back(0x22);
                break;
            case 0x60://''
                ptext.push_back(0x60);
                break;
            case 0x27://'\'
                ptext.push_back(0x27);
                break;
            case 0x2f://\/
                ptext.push_back('/');
                break;
            case 0x08://\b
                break;
            case 0x0c://\f
                break;
            case 0x0a://\n
                ptext.push_back(0x5c);
                ptext.push_back('n');
                break;
            case 0x0d://\r
                ptext.push_back(0x5c);
                ptext.push_back('r');
                break;
            case 0x09://\t
                ptext.push_back(0x5c);
                ptext.push_back('t');
                break;
            case 0x5c://
                ptext.push_back(0x5c);
                ptext.push_back(0x5c);
                break;
            default:
                ptext.push_back(c);
            }
        }
        else if (c >= 0x80 && c < 0xC0)
        {
            ptext.push_back(c);
        }
        else if (c >= 0xC0 && c < 0xE0)
        {
            unsigned char b1, b2, cp[2], temp;
            b1 = c;
            b2 = *(text + i + 1);
            if ((b2 & 0xE0) != 0x80)
            {
                ptext.push_back(c);
            }
            else
            {
                ptext.push_back(0x5c);
                ptext.push_back(0x75);

                temp = (b1 >> 2) & 0x07;
                // numtohex(temp,cp);

                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext.push_back(cp[0]);
                ptext.push_back(cp[1]);

                temp = (b1 << 6) + (b2 & 0x3F);
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext.push_back(cp[0]);
                ptext.push_back(cp[1]);

                i++;
            }
        }
        else if (c >= 0xE0 && c < 0xF0)
        {
            unsigned char b1, b2, b3, cp[2], temp;
            b1 = c;
            b2 = *(text + i + 1);
            b3 = *(text + i + 2);
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80))
            {
                ptext.push_back(c);
            }
            else
            {
                ptext.push_back(0x5c);
                ptext.push_back(0x75);

                temp = (b1 << 4) + ((b2 >> 2) & 0x0F);
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext.push_back(cp[0]);
                ptext.push_back(cp[1]);

                temp = (b2 << 6) + (b3 & 0x3F);
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext.push_back(cp[0]);
                ptext.push_back(cp[1]);
                i += 2;
            }
        }
        else if (c >= 0xF0 && c < 0xF8)
        {
            unsigned char b1, b2, b3, b4, cp[2], temp, *pOutput;
            unsigned int out, untext;

            b1 = c;
            b2 = *(text + i + 1);
            b3 = *(text + i + 2);
            b4 = *(text + i + 3);

            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80))
            {
                ptext.push_back(c);
            }
            else
            {
                ptext.push_back(0x5c);
                ptext.push_back(0x75);
                // utf8转为unicode
                pOutput        = (unsigned char *)&out;
                *pOutput       = (b3 << 6) + (b4 & 0x3F);
                *(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
                *(pOutput + 2) = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
                untext         = (((((out - 0x10000) >> 10) | 0xD800) << 16) | (((out - 0x10000) & 0x3FF) | 0xDC00));

                pOutput = (unsigned char *)&untext;
                // 高端字节在前
                temp = pOutput[3];
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext.push_back(cp[0]);
                ptext.push_back(cp[1]);

                temp = pOutput[2];
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext.push_back(cp[0]);
                ptext.push_back(cp[1]);
                // 加上\u
                ptext.push_back(0x5c);
                ptext.push_back(0x75);

                // 处理低两个字节
                temp = pOutput[1];
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext.push_back(cp[0]);
                ptext.push_back(cp[1]);

                temp = pOutput[0];

                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext.push_back(cp[0]);
                ptext.push_back(cp[1]);

                i += 3;
            }
        }
        else if (c >= 0xF8 && c < 0xFC)
        {
            ptext.push_back(c);
        }
        else
        {
            ptext.push_back(c);
        };
    }
    return ptext;
}

std::string obj_val::to_json()
{
    std::stringstream os;
    if (_val_type == obj_type::OBJECT)
    {
        os << "{";
        for (unsigned int i = 0; i < obj->_data.size(); i++)
        {
            if (i > 0)
            {
                os << ",";
            }
            os << "\"";
            os << obj->_data[i].first;
            os << "\":";
            os << obj->_data[i].second.to_json();
        }
        os << "}";
    }
    else if (_val_type == obj_type::ARRAY)
    {
        // if(array_val->key.size()>0)
        // {
        //     os << array_val->key;
        //     os << ":";
        // }
        os << "[";
        for (unsigned int i = 0; i < array_val->_data.size(); i++)
        {
            if (i > 0)
            {
                os << ",";
            }
            os << array_val->_data[i].to_json();
        }
        os << "]";
    }
    else if (_val_type == obj_type::STRING)
    {
        if (length < 8)
        {
            os << '"' << JSON_UTF8_TO_ASCII(name, length) << '"';
        }
        else
        {
            os << '"' << JSON_UTF8_TO_ASCII(str, length) << '"';
        }
    }
    else if (_val_type == obj_type::BOOL)
    {
        if (isbool)
        {
            os << "true";
        }
        else
        {
            os << "false";
        }
    }
    else if (_val_type == obj_type::INT)
    {
        os << std::to_string(ival);
    }
    else if (_val_type == obj_type::UINT)
    {
        os << std::to_string(uival);
    }
    else if (_val_type == obj_type::FLOAT)
    {
        std::stringstream stream;
        std::string temp;

        stream.precision(15);
        // stream << std::fixed;
        stream << fval;

        temp           = stream.str();
        unsigned int i = 0;
        for (; i < temp.length(); i++)
        {
            if (temp[i] == '.')
            {
                break;
            }
        }
        if (i == temp.length())
        {
            temp.push_back('.');
            temp.push_back('0');
        }
        os << temp;
    }
    else if (_val_type == obj_type::DOUBLE)
    {
        std::stringstream stream;
        std::string temp;

        stream.precision(17);
        // stream << std::fixed;
        stream << dval;

        temp           = stream.str();
        unsigned int i = 0;
        for (; i < temp.length(); i++)
        {
            if (temp[i] == '.')
            {
                break;
            }
        }
        if (i == temp.length())
        {
            temp.push_back('.');
            temp.push_back('0');
        }
        os << temp;
    }
    else if (_val_type == obj_type::LONG)
    {
        os << std::to_string(lval);
    }
    else if (_val_type == obj_type::ULONG)
    {
        os << std::to_string(uval);
    }
    else if (_val_type == obj_type::NIL)
    {
        os << "null";
    }

    return os.str();
}

std::vector<std::pair<std::string, obj_val>> &obj_val::as_object()
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data;
    }
    else if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
        return obj->_data;
    }
    else if (_val_type == obj_type::ARRAY)
    {
        if (array_val->_data.size() == 0)
        {
            clear();
            obj       = new obj_t;
            _val_type = obj_type::OBJECT;
            return obj->_data;
        }
    }
    throw "This not object for as_object";
}
std::vector<obj_val> &obj_val::as_array()
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data;
    }
    else if (_val_type == obj_type::NIL)
    {
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
        return array_val->_data;
    }
    else if (_val_type == obj_type::OBJECT)
    {
        if (obj->_data.size() == 0)
        {
            clear();
            array_val = new obj_array;
            _val_type = obj_type::ARRAY;
            return array_val->_data;
        }
    }
    throw "This not array for as_array";
}
std::string obj_val::as_string()
{
    if (_val_type == obj_type::STRING)
    {
        if (length < 8)
        {
            std::string a_temp;
            for (unsigned int j = 0; j < length; j++)
            {
                a_temp.push_back(name[j]);
            }
            return a_temp;
        }
        return std::string(str, length);
    }
    return "";
}

std::string obj_val::as_string(std::string_view default_val)
{
    if (_val_type == obj_type::STRING)
    {
        if (length == 0)
        {
            return std::string(default_val);
        }
        if (length < 8)
        {
            std::string a_temp;
            for (unsigned int j = 0; j < length; j++)
            {
                a_temp.push_back(name[j]);
            }
            return a_temp;
        }
        return std::string(str, length);
    }
    return std::string(default_val);
}

std::string obj_val::get_string(std::string_view key, std::string_view default_val)
{
    if (_val_type == obj_type::OBJECT)
    {
        for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
        {
            if (iter->first == key)
            {
                return iter->second.as_string(default_val);
            }
            iter++;
        }
    }
    return std::string(default_val);
}

std::string_view obj_val::get_str_view(std::string_view key, std::string_view default_val)
{
    if (_val_type == obj_type::OBJECT)
    {
        for (auto iter = obj->_data.begin(); iter != obj->_data.end();)
        {
            if (iter->first == key)
            {
                return iter->second.str_view(default_val);
            }
            iter++;
        }
    }
    return default_val;
}

std::map<unsigned int, std::vector<unsigned int>> obj_val::get_obj_key_index()
{
    std::map<unsigned int, std::vector<unsigned int>> temp;
    if (_val_type == obj_type::OBJECT)
    {
        for (unsigned int i = 0; i < obj->_data.size(); i++)
        {
            unsigned int c = 0;
            if (obj->_data[i].first.size() > 0)
            {
                c = obj->_data[i].first[0];
            }

            if (c < 0x80)
            {
                temp[c].emplace_back(i);
            }
            else if (c < 0xC0)
            {
                temp[c].emplace_back(i);
            }
            else if (c >= 0xC0 && c < 0xE0)
            {
                c = ((c << 8)) + (obj->_data[i].first[1] & 0xFF);
                temp[c].emplace_back(i);
            }
            else if (c >= 0xE0 && c < 0xF0)
            {
                c = ((c << 8)) + (obj->_data[i].first[1] & 0xFF);
                c = ((c << 8)) + (obj->_data[i].first[2] & 0xFF);
                temp[c].emplace_back(i);
            }
            else if (c >= 0xF0 && c < 0xF8)
            {
                c = ((c << 8)) + (obj->_data[i].first[1] & 0xFF);
                c = ((c << 8)) + (obj->_data[i].first[2] & 0xFF);
                c = ((c << 8)) + (obj->_data[i].first[3] & 0xFF);
                temp[c].emplace_back(i);
            }
            else
            {
                temp[c].emplace_back(i);
            }
        }
    }
    return temp;
}

obj_val &obj_val::get_obj_val_index(std::string_view key, const std::map<unsigned int, std::vector<unsigned int>> &index_array)
{
    if (_val_type != obj_type::OBJECT)
    {
        clear();
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }

    unsigned int c = 0;
    if (key.size() > 0)
    {
        c = key[0];
    }

    if (c >= 0xC0 && c < 0xE0)
    {
        c = (c << 8) + (key[1] & 0xFF);
    }
    else if (c >= 0xE0 && c < 0xF0)
    {
        c = (c << 8) + (key[1] & 0xFF);
        c = (c << 8) + (key[2] & 0xFF);
    }
    else if (c >= 0xF0 && c < 0xF8)
    {
        c = (c << 8) + (key[1] & 0xFF);
        c = (c << 8) + (key[2] & 0xFF);
        c = (c << 8) + (key[3] & 0xFF);
    }

    auto iter = index_array.find(c);
    if (iter != index_array.end())
    {
        for (unsigned int i = 0; i < iter->second.size(); i++)
        {
            c = iter->second[i];
            if (c < obj->_data.size())
            {
                if (key == obj->_data[c].first)
                {
                    return obj->_data[c].second;
                }
            }
        }
    }
    obj->_data.emplace_back(key, nullptr);
    obj->_data.back().second.set_type(obj_type::NIL);
    return obj->_data.back().second;
}

void obj_val::zip(const std::vector<std::string> &key, const std::vector<int> &val)
{
    if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }

    if (_val_type == obj_type::OBJECT)
    {
        for (unsigned int i = 0; i < val.size(); ++i)
        {
            if (i < key.size())
            {
                obj->_data.emplace_back(key[i], val[i]);
            }
        }
    }

    if (_val_type == obj_type::ARRAY)
    {
        for (unsigned int i = 0; i < val.size(); ++i)
        {
            if (i < key.size())
            {
                obj_val temp;
                temp.set_object();
                temp.push(key[i], val[i]);
                array_val->_data.emplace_back(std::move(temp));
            }
        }
    }
}

void obj_val::zip(const std::vector<std::string> &key, const std::vector<float> &val)
{
    if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }

    if (_val_type == obj_type::OBJECT)
    {
        for (unsigned int i = 0; i < val.size(); ++i)
        {
            if (i < key.size())
            {
                obj->_data.emplace_back(key[i], val[i]);
            }
        }
    }

    if (_val_type == obj_type::ARRAY)
    {
        for (unsigned int i = 0; i < val.size(); ++i)
        {
            if (i < key.size())
            {
                obj_val temp;
                temp.set_object();
                temp.push(key[i], val[i]);
                array_val->_data.emplace_back(std::move(temp));
            }
        }
    }
}

void obj_val::zip(const std::vector<std::string> &key, const std::vector<std::string> &val)
{
    if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }

    if (_val_type == obj_type::OBJECT)
    {
        for (unsigned int i = 0; i < val.size(); ++i)
        {
            if (i < key.size())
            {
                obj->_data.emplace_back(key[i], val[i]);
            }
        }
    }

    if (_val_type == obj_type::ARRAY)
    {
        for (unsigned int i = 0; i < val.size(); ++i)
        {
            if (i < key.size())
            {
                obj_val temp;
                temp.set_object();
                temp.push(key[i], val[i]);
                array_val->_data.emplace_back(std::move(temp));
            }
        }
    }
}

void obj_val::zip(const std::vector<std::string> &key, const obj_val &val)
{
    if (val.is_array() || val.is_object())
    {
    }
    else
    {
        return;
    }

    if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }

    if (_val_type == obj_type::OBJECT)
    {
        if (val.is_object())
        {
            unsigned int i = 0;
            for (auto &[a, b] : val.obj->_data)
            {
                if (i < key.size())
                {
                    obj->_data.emplace_back(key[i], b);
                }
                i++;
            }
        }
        else if (val.is_array())
        {
            unsigned int i = 0;
            for (auto &a : val.array_val->_data)
            {
                if (i < key.size())
                {
                    obj->_data.emplace_back(key[i], a);
                }
                i++;
            }
        }
    }

    if (_val_type == obj_type::ARRAY)
    {
        if (val.is_object())
        {
            unsigned int i = 0;
            for (auto &[a, b] : val.obj->_data)
            {
                if (i < key.size())
                {
                    obj_val temp;
                    temp.set_object();
                    temp.push(key[i], b);
                    array_val->_data.emplace_back(std::move(temp));
                }
                i++;
            }
        }
        else if (val.is_array())
        {
            unsigned int i = 0;
            for (auto &a : val.array_val->_data)
            {
                if (i < key.size())
                {
                    obj_val temp;
                    temp.set_object();
                    temp.push(key[i], a);
                    array_val->_data.emplace_back(std::move(temp));
                }
                i++;
            }
        }
    }
}

void obj_val::zip(const obj_val &key, const obj_val &val)
{
    if (val.is_array() || val.is_object())
    {
    }
    else
    {
        return;
    }

    if (key.is_array() || key.is_object())
    {
    }
    else
    {
        return;
    }

    if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
    }

    if (_val_type == obj_type::OBJECT)
    {
        if (val.is_object())
        {
            unsigned int i = 0;
            for (auto &[a, b] : val.obj->_data)
            {
                if (key.is_array())
                {
                    if (i < key.array_val->_data.size())
                    {
                        obj->_data.emplace_back(key.array_val->_data[i].to_string(), b);
                    }
                }
                else if (key.is_object())
                {
                    if (i < key.obj->_data.size())
                    {
                        obj->_data.emplace_back(key.obj->_data[i].first, b);
                    }
                }
                i++;
            }
        }
        else if (val.is_array())
        {
            unsigned int i = 0;
            for (auto &a : val.array_val->_data)
            {
                if (key.is_array())
                {
                    if (i < key.array_val->_data.size())
                    {
                        obj->_data.emplace_back(key.array_val->_data[i].to_string(), a);
                    }
                }
                else if (key.is_object())
                {
                    if (i < key.obj->_data.size())
                    {
                        obj->_data.emplace_back(key.obj->_data[i].first, a);
                    }
                }
                i++;
            }
        }
    }

    if (_val_type == obj_type::ARRAY)
    {
        if (val.is_object())
        {
            unsigned int i = 0;
            for (auto &[a, b] : val.obj->_data)
            {
                if (key.is_array())
                {
                    if (i < key.array_val->_data.size())
                    {
                        obj_val temp;
                        temp.set_object();
                        temp.push(key.array_val->_data[i].to_string(), b);
                        array_val->_data.emplace_back(std::move(temp));
                    }
                }
                else if (key.is_object())
                {
                    if (i < key.obj->_data.size())
                    {
                        obj_val temp;
                        temp.set_object();
                        temp.push(key.array_val->_data[i].to_string(), b);
                        array_val->_data.emplace_back(std::move(temp));
                    }
                }
                i++;
            }
        }
        else if (val.is_array())
        {
            unsigned int i = 0;
            for (auto &a : val.array_val->_data)
            {
                if (key.is_array())
                {
                    if (i < key.array_val->_data.size())
                    {
                        obj_val temp;
                        temp.set_object();
                        temp.push(key.array_val->_data[i].to_string(), a);
                        array_val->_data.emplace_back(std::move(temp));
                    }
                }
                else if (key.is_object())
                {
                    if (i < key.obj->_data.size())
                    {
                        obj_val temp;
                        temp.set_object();
                        temp.push(key.array_val->_data[i].to_string(), a);
                        array_val->_data.emplace_back(std::move(temp));
                    }
                }
                i++;
            }
        }
    }
}

obj_val obj_val::multi_sort(std::string_view key, unsigned char order)
{
    obj_val temp_obj;
    temp_obj.set_array();

    if (_val_type != obj_type::ARRAY)
    {
        return temp_obj;
    }
    temp_obj.set_array();
    temp_obj.reserve(array_val->_data.size());

    std::list<unsigned int> temp_sort;

    unsigned int pos_num  = 0xFFFFFFFF;
    unsigned int obj_size = 0;
    try
    {
        for (auto iter = array_val->_data.begin(); iter != array_val->_data.end();)
        {
            if (iter->is_object())
            {
                for (auto iter_obj = iter->obj_begin(); iter_obj != iter->obj_end();)
                {
                    if (iter_obj->first == key)
                    {
                        pos_num = obj_size;
                        break;
                    }
                    obj_size++;
                    iter_obj++;
                }
                obj_size = iter->size();
            }
            iter++;
            break;
        }
        if (pos_num == 0xFFFFFFFF)
        {
            return temp_obj;
        }

        unsigned int temp_ppos = 0;
        temp_sort.push_back(temp_ppos);
        temp_ppos = 1;
        auto iter = array_val->_data.begin();
        if (iter == array_val->_data.end())
        {
            return temp_obj;
        }
        iter++;

        for (; iter != array_val->_data.end();)
        {
            if (iter->is_object())
            {
                unsigned int temp_ppos2                = 0;
                std::pair<std::string, obj_val> temp_c = iter->ref_obj_val(pos_num);
                auto list_iter                         = temp_sort.begin();
                for (; list_iter != temp_sort.end();)
                {
                    temp_ppos2 = *list_iter;

                    if (temp_ppos2 == temp_ppos)
                    {
                        break;
                    }
                    if (temp_ppos2 < array_val->_data.size())
                    {
                        std::pair<std::string, obj_val> temp_d = array_val->_data[temp_ppos2].ref_obj_val(pos_num);

                        if (temp_c.second >= temp_d.second)
                        {
                        }
                        else
                        {
                            temp_sort.insert(list_iter, temp_ppos);
                            break;
                        }
                    }
                    list_iter++;
                }
                if (list_iter == temp_sort.end())
                {
                    temp_sort.insert(temp_sort.end(), temp_ppos);
                }
            }
            iter++;
            temp_ppos++;
        }

        if (order == 0)
        {
            auto list_iter = temp_sort.begin();
            for (; list_iter != temp_sort.end();)
            {
                unsigned int temp_ppos2 = *list_iter;
                if (temp_ppos2 < array_val->_data.size())
                {
                    temp_obj.push(array_val->_data[temp_ppos2]);
                }
                list_iter++;
            }
        }
        else
        {
            auto list_iter = temp_sort.rbegin();
            for (; list_iter != temp_sort.rend();)
            {
                unsigned int temp_ppos2 = *list_iter;
                if (temp_ppos2 < array_val->_data.size())
                {
                    temp_obj.push(array_val->_data[temp_ppos2]);
                }
                list_iter++;
            }
        }
    }
    catch (const char *e)
    {
        return temp_obj;
    }
    return temp_obj;
}

obj_val obj_val::multi_sort(std::string_view key, unsigned char order, std::string_view key2, unsigned char order2)
{
    obj_val temp_obj;
    temp_obj.set_array();

    if (_val_type != obj_type::ARRAY)
    {
        return temp_obj;
    }
    temp_obj.set_array();
    temp_obj.reserve(array_val->_data.size());

    std::list<unsigned int> temp_sort;

    unsigned int pos_num  = 0xFFFFFFFF;
    unsigned int pos2_num = 0xFFFFFFFF;
    unsigned int obj_size = 0;
    try
    {
        for (auto iter = array_val->_data.begin(); iter != array_val->_data.end();)
        {
            if (iter->is_object())
            {
                for (auto iter_obj = iter->obj_begin(); iter_obj != iter->obj_end();)
                {
                    if (iter_obj->first == key)
                    {
                        pos_num = obj_size;
                    }
                    else if (iter_obj->first == key2)
                    {
                        pos2_num = obj_size;
                    }

                    if (pos_num != 0xFFFFFFFF && pos2_num != 0xFFFFFFFF)
                    {
                        break;
                    }
                    obj_size++;
                    iter_obj++;
                }
                obj_size = iter->size();
            }
            iter++;
            break;
        }
        if (pos_num == 0xFFFFFFFF || pos2_num == 0xFFFFFFFF)
        {
            return temp_obj;
        }

        unsigned int temp_ppos = 0;
        temp_sort.push_back(temp_ppos);
        temp_ppos = 1;
        auto iter = array_val->_data.begin();
        if (iter == array_val->_data.end())
        {
            return temp_obj;
        }
        iter++;

        for (; iter != array_val->_data.end();)
        {
            if (iter->is_object())
            {

                unsigned int temp_ppos2                = 0;
                std::pair<std::string, obj_val> temp_c = iter->ref_obj_val(pos_num);
                auto list_iter                         = temp_sort.begin();
                for (; list_iter != temp_sort.end();)
                {
                    temp_ppos2 = *list_iter;

                    if (temp_ppos2 == temp_ppos)
                    {
                        break;
                    }
                    if (temp_ppos2 < array_val->_data.size())
                    {

                        std::pair<std::string, obj_val> temp_d = array_val->_data[temp_ppos2].ref_obj_val(pos_num);

                        if (temp_c.second >= temp_d.second)
                        {
                        }
                        else
                        {
                            temp_sort.insert(list_iter, temp_ppos);
                            break;
                        }
                    }
                    list_iter++;
                }
                if (list_iter == temp_sort.end())
                {
                    temp_sort.insert(temp_sort.end(), temp_ppos);
                }
            }
            iter++;
            temp_ppos++;
        }

        if (order == 0)
        {
            auto list_iter = temp_sort.begin();
            std::list<unsigned int> temp2_sort;
            for (; list_iter != temp_sort.end();)
            {
                auto list_iter2 = list_iter;
                list_iter2++;
                unsigned int temp_ppos2 = *list_iter;
                temp2_sort.clear();
                temp2_sort.emplace_back(temp_ppos2);
                std::pair<std::string, obj_val> temp_c = array_val->_data[temp_ppos2].ref_obj_val(pos_num);
                for (; list_iter2 != temp_sort.end();)
                {
                    unsigned int temp_ppos3                = *list_iter2;
                    std::pair<std::string, obj_val> temp_d = array_val->_data[temp_ppos3].ref_obj_val(pos_num);

                    if (temp_c.second == temp_d.second)
                    {
                        //has eq member
                        auto list2_iter                         = temp2_sort.begin();
                        std::pair<std::string, obj_val> temp_dd = array_val->_data[temp_ppos3].ref_obj_val(pos2_num);
                        for (; list2_iter != temp2_sort.end();)
                        {
                            unsigned int list2_pos                  = *list2_iter;
                            std::pair<std::string, obj_val> temp_cc = array_val->_data[list2_pos].ref_obj_val(pos2_num);
                            if (temp_dd.second > temp_cc.second)
                            {
                                temp2_sort.insert(list2_iter, temp_ppos3);
                                break;
                            }
                            list2_iter++;
                        }
                        if (list2_iter == temp2_sort.end())
                        {
                            temp2_sort.insert(temp2_sort.end(), temp_ppos3);
                        }

                        list_iter2++;
                        continue;
                    }
                    break;
                }

                //is has eq member
                if (temp2_sort.size() > 1)
                {
                    if (order2 == 0)
                    {
                        auto list2_iter = temp2_sort.begin();
                        for (; list2_iter != temp2_sort.end();)
                        {
                            unsigned int list2_pos = *list2_iter;
                            temp_obj.push(array_val->_data[list2_pos]);
                            list2_iter++;
                        }
                    }
                    else
                    {
                        auto list2_iter = temp2_sort.cbegin();
                        for (; list2_iter != temp2_sort.cend();)
                        {
                            unsigned int list2_pos = *list2_iter;
                            temp_obj.push(array_val->_data[list2_pos]);
                            list2_iter++;
                        }
                    }
                    list_iter = list_iter2;
                }
                else
                {
                    if (temp_ppos2 < array_val->_data.size())
                    {
                        temp_obj.push(array_val->_data[temp_ppos2]);
                    }
                    list_iter++;
                }
            }
        }
        else
        {
            auto list_iter = temp_sort.rbegin();
            std::list<unsigned int> temp2_sort;

            for (; list_iter != temp_sort.rend();)
            {
                auto list_iter2 = list_iter;
                list_iter2++;
                unsigned int temp_ppos2 = *list_iter;
                temp2_sort.clear();
                temp2_sort.emplace_back(temp_ppos2);
                std::pair<std::string, obj_val> temp_c = array_val->_data[temp_ppos2].ref_obj_val(pos_num);
                for (; list_iter2 != temp_sort.rend();)
                {
                    unsigned int temp_ppos3                = *list_iter2;
                    std::pair<std::string, obj_val> temp_d = array_val->_data[temp_ppos3].ref_obj_val(pos_num);
                    if (temp_c.second == temp_d.second)
                    {
                        //has eq member
                        auto list2_iter                         = temp2_sort.begin();
                        std::pair<std::string, obj_val> temp_dd = array_val->_data[temp_ppos3].ref_obj_val(pos2_num);
                        for (; list2_iter != temp2_sort.end();)
                        {
                            unsigned int list2_pos                  = *list2_iter;
                            std::pair<std::string, obj_val> temp_cc = array_val->_data[list2_pos].ref_obj_val(pos2_num);
                            if (temp_dd.second > temp_cc.second)
                            {
                                temp2_sort.insert(list2_iter, temp_ppos3);
                                break;
                            }
                            list2_iter++;
                        }
                        if (list2_iter == temp2_sort.end())
                        {
                            temp2_sort.insert(temp2_sort.end(), temp_ppos3);
                        }

                        list_iter2++;
                        continue;
                    }
                    break;
                }

                //is has eq member
                if (temp2_sort.size() > 1)
                {
                    if (order2 == 0)
                    {
                        auto list2_iter = temp2_sort.begin();
                        for (; list2_iter != temp2_sort.end();)
                        {
                            unsigned int list2_pos = *list2_iter;
                            temp_obj.push(array_val->_data[list2_pos]);
                            list2_iter++;
                        }
                    }
                    else
                    {
                        auto list2_iter = temp2_sort.cbegin();
                        for (; list2_iter != temp2_sort.cend();)
                        {
                            unsigned int list2_pos = *list2_iter;
                            temp_obj.push(array_val->_data[list2_pos]);
                            list2_iter++;
                        }
                    }
                    list_iter = list_iter2;
                }
                else
                {
                    if (temp_ppos2 < array_val->_data.size())
                    {
                        temp_obj.push(array_val->_data[temp_ppos2]);
                    }
                    list_iter++;
                }
            }
        }
    }
    catch (const char *e)
    {
        return temp_obj;
    }
    return temp_obj;
}

}// namespace http
std::ostream &operator<<(std::ostream &os, http::obj_val &val)
{
    if (val.get_type() == http::obj_type::OBJECT)
    {
        os << "{";
        for (unsigned int i = 0; i < val.obj->_data.size(); i++)
        {
            if (i > 0)
            {
                os << ",";
            }
            os << "\"";
            os << val.obj->_data[i].first;
            os << "\":";
            os << val.obj->_data[i].second.to_json();
        }
        os << "}";
    }
    else if (val.get_type() == http::obj_type::ARRAY)
    {
        // if(val.array_val->key.size()>0)
        // {
        //     os << val.array_val->key;
        //     os << ":";
        // }
        os << "[";
        for (unsigned int i = 0; i < val.array_val->_data.size(); i++)
        {
            if (i > 0)
            {
                os << ",";
            }
            os << val.array_val->_data[i].to_json();
        }
        os << "]";
    }
    else if (val.get_type() == http::obj_type::STRING)
    {
        if (val.length < 8)
        {
            os << '"' << val.JSON_UTF8_TO_ASCII(val.name, val.length) << '"';
        }
        else
        {
            os << '"' << val.JSON_UTF8_TO_ASCII(val.str, val.length) << '"';
        }
    }
    else if (val.get_type() == http::obj_type::BOOL)
    {
        if (val.isbool)
        {
            os << "true";
        }
        else
        {
            os << "false";
        }
    }
    else if (val.get_type() == http::obj_type::INT)
    {
        os << std::to_string(val.ival);
    }
    else if (val.get_type() == http::obj_type::UINT)
    {
        os << std::to_string(val.uival);
    }
    else if (val.get_type() == http::obj_type::FLOAT)
    {
        std::stringstream stream;
        std::string temp;

        stream.precision(15);
        // stream << std::fixed;
        stream << val.fval;

        temp           = stream.str();
        unsigned int i = 0;
        for (; i < temp.length(); i++)
        {
            if (temp[i] == '.')
            {
                break;
            }
        }
        if (i == temp.length())
        {
            temp.push_back('.');
            temp.push_back('0');
        }
        os << temp;
    }
    else if (val.get_type() == http::obj_type::DOUBLE)
    {
        std::stringstream stream;
        std::string temp;

        stream.precision(17);
        // stream << std::fixed;
        stream << val.dval;

        temp           = stream.str();
        unsigned int i = 0;
        for (; i < temp.length(); i++)
        {
            if (temp[i] == '.')
            {
                break;
            }
        }
        if (i == temp.length())
        {
            temp.push_back('.');
            temp.push_back('0');
        }
        os << temp;
    }
    else if (val.get_type() == http::obj_type::LONG)
    {
        os << std::to_string(val.lval);
    }
    else if (val.get_type() == http::obj_type::ULONG)
    {
        os << std::to_string(val.uval);
    }
    else if (val.get_type() == http::obj_type::NIL)
    {
        os << "null";
    }

    return os;
}
