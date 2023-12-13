#include <iostream>
#include <ctime>
#include <array>
#include <string>
//#include <sys/time.h>
//#include <unistd.h>
#include <cstdlib>
#include <random>
#include "datetime.h"

namespace http
{

std::string get_gmttime(time_t inputtime)
{
    time_t curr_time;
    if (inputtime == 0)
    {
        curr_time = time((time_t *)NULL);
    }
    else
    {
        curr_time = inputtime;
    }
    tm *timeInfo;

    char timestr[30] = {};
    timeInfo         = gmtime(&curr_time);
    strftime(timestr, sizeof(timestr), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

    std::string temp(timestr);
    return temp;
}
std::string get_utctime(time_t inputtime)
{
    time_t curr_time;
    if (inputtime == 0)
    {
        curr_time = time((time_t *)NULL);
    }
    else
    {
        curr_time = inputtime;
    }
    tm *timeInfo;

    char timestr[30] = {};
    timeInfo         = gmtime(&curr_time);
    //std::size_t n=
    strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%SZ", timeInfo);
    std::string temp(timestr);
    return temp;
}
unsigned int rand_range(unsigned int a, unsigned int b)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> distrib(a, b);

    return distrib(gen);
}
std::string rand_string(unsigned int a, unsigned char isupper)
{
    const char strrand[]      = {"0123456789abcdefghijkmnpqrstuvwxyABCDEFGHJKLMNPQRSTUVWXY"};
    const char strrand_pure[] = {"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};
    std::string temp;

    switch (isupper)
    {
    case 0:
        srand((int)time(NULL));
        temp.resize(a);
        for (unsigned int i = 0; i < a; i++)
        {
            unsigned char ta = ((rand() % (56 - 0)) + 0);
            temp[i]          = strrand[ta];
        }
        break;
    case 1:
        srand((int)time(NULL));
        temp.resize(a);
        for (unsigned int i = 0; i < a; i++)
        {
            unsigned char ta = ((rand() % (36 - 0)) + 0);
            temp[i]          = strrand_pure[ta];
        }
        break;
    case 2:
        srand((int)time(NULL));
        temp.resize(a);
        for (unsigned int i = 0; i < a; i++)
        {
            unsigned char ta = ((rand() % (26 - 0)) + 0) + 10;
            temp[i]          = strrand_pure[ta];
        }
        break;
    case 3:
        srand((int)time(NULL));
        temp.resize(a);
        for (unsigned int i = 0; i < a; i++)
        {
            unsigned char ta = ((rand() % (26 - 0)) + 0) + 36;
            temp[i]          = strrand_pure[ta];
        }
        break;
    case 4:
        srand((int)time(NULL));
        temp.resize(a);
        for (unsigned int i = 0; i < a; i++)
        {
            unsigned char ta = ((rand() % (10 - 0)) + 0);
            temp[i]          = strrand[ta];
        }
        break;
    default:
        srand((int)time(NULL));
        temp.resize(a);
        for (unsigned int i = 0; i < a; i++)
        {
            unsigned char ta = ((rand() % (56 - 0)) + 0);
            temp[i]          = strrand[ta];
        }
        break;
    }

    return temp;
}

unsigned int timeid() { return time((time_t *)NULL); }

std::string get_date(const std::string &format, unsigned int inputtime)
{
    time_t curr_time;
    if (inputtime == 0)
    {
        curr_time = time((time_t *)NULL);
    }
    else
    {
        curr_time = inputtime;
    }
    tm *timeInfo;
    std::string temp(36, 0x00);
    timeInfo = localtime(&curr_time);
    strftime(temp.data(), temp.length(), format.c_str(), timeInfo);
    for (unsigned int i = 0; i < temp.size(); i++)
    {
        if (temp[i] == 0x00)
        {
            temp.resize(i);
            break;
        }
    }
    return temp;
}

// Sat, 02 Jan 2021 00:12:45 GMT to sec
unsigned int strgmttotime(const std::string &gmtstr)
{
    unsigned int temp = 0;
    tm timeInfo;
    unsigned char i = 0;

    for (; i < gmtstr.length(); i++)
    {
        if (gmtstr[i] != 0x20)
        {
            break;
        }
    }
    char tc[4] = {0x00};
    if (gmtstr[i + 3] == ',')
    {
        i += 5;
    }
    if (gmtstr[i] >= '0' && gmtstr[i] <= '9')
    {
        tc[0] = gmtstr[i] - '0';
    }
    i++;
    if (gmtstr[i] >= '0' && gmtstr[i] <= '9')
    {
        tc[1] = gmtstr[i] - '0';
    }
    i++;
    timeInfo.tm_mday = tc[0] * 10 + tc[1];
    if (gmtstr[i] != 0x20)
    {
        return temp;
    }
    i++;
    switch (gmtstr[i])
    {
    case 'J':
        if (gmtstr[i + 1] == 'a')
        {
            timeInfo.tm_mon = 0;
        }
        else if (gmtstr[i + 2] == 'n')
        {
            timeInfo.tm_mon = 5;
        }
        else
        {
            timeInfo.tm_mon = 6;
        }
        break;
    case 'F': timeInfo.tm_mon = 1; break;
    case 'S': timeInfo.tm_mon = 8; break;
    case 'M':
        if (gmtstr[i + 2] == 'r')
        {
            timeInfo.tm_mon = 2;
        }
        else
        {
            timeInfo.tm_mon = 4;
        }
        break;
    case 'A':
        if (gmtstr[i + 2] == 'r')
        {
            timeInfo.tm_mon = 3;
        }
        else
        {
            timeInfo.tm_mon = 7;
        }
        break;
    case 'O': timeInfo.tm_mon = 9; break;
    case 'N': timeInfo.tm_mon = 10; break;
    case 'D': timeInfo.tm_mon = 11; break;
    default: timeInfo.tm_mon = 0;
    }
    i += 4;
    if (i >= gmtstr.length())
    {
        return temp;
    }
    tc[0] = gmtstr[i] - '0';
    i++;
    tc[1] = gmtstr[i] - '0';
    i++;
    tc[2] = gmtstr[i] - '0';
    i++;
    tc[3] = gmtstr[i] - '0';
    i++;
    timeInfo.tm_year = tc[0] * 1000 + tc[1] * 100 + tc[2] * 10 + tc[3] - 1900;

    if (timeInfo.tm_year < 0)
    {
        timeInfo.tm_year = 70;
    }

    i++;

    if (gmtstr[i] >= '0' && gmtstr[i] <= '9')
    {
        tc[0] = gmtstr[i] - '0';
    }
    i++;
    if (gmtstr[i] >= '0' && gmtstr[i] <= '9')
    {
        tc[1] = gmtstr[i] - '0';
    }
    i++;

    i++;
    timeInfo.tm_hour = tc[0] * 10 + tc[1] + 1;

    if (gmtstr[i] >= '0' && gmtstr[i] <= '9')
    {
        tc[0] = gmtstr[i] - '0';
    }
    i++;
    if (gmtstr[i] >= '0' && gmtstr[i] <= '9')
    {
        tc[1] = gmtstr[i] - '0';
    }
    i++;
    i++;
    timeInfo.tm_min = tc[0] * 10 + tc[1];
    if (gmtstr[i] >= '0' && gmtstr[i] <= '9')
    {
        tc[0] = gmtstr[i] - '0';
    }
    i++;
    if (gmtstr[i] >= '0' && gmtstr[i] <= '9')
    {
        tc[1] = gmtstr[i] - '0';
    }
    i++;

    timeInfo.tm_sec = tc[0] * 10 + tc[1];

    i++;
    if (gmtstr[i] == 'G' && gmtstr[i + 1] == 'M' && gmtstr[i + 2] == 'T')
    {
        std::time_t rt = std::mktime(&timeInfo);
        std::tm *tm    = std::gmtime((const time_t *)&rt);
        std::time_t gt = std::mktime(tm);
        temp           = rt + (rt - gt);
    }
    else
    {
        temp = std::mktime(&timeInfo);
    }
    return temp;
}

// 2022-02-09 08:12:45 to sec
unsigned int strtotime(const std::string &str)
{
    unsigned int temp = 0;
    unsigned char i   = 0x00;
    tm datetime;
    char tc[4] = {0x00};

    for (; i < str.length(); i++)
    {
        if (str[i] != 0x20)
        {
            break;
        }
    }

    tc[0] = str[i] - '0';
    i++;
    tc[1] = str[i] - '0';
    i++;
    tc[2] = str[i] - '0';
    i++;
    tc[3] = str[i] - '0';
    i++;
    datetime.tm_year = tc[0] * 1000 + tc[1] * 100 + tc[2] * 10 + tc[3] - 1900;

    if ((unsigned char)str[i] > 0x7F)
    {
        i += 3;
    }
    else
    {
        i++;
    }
    if (str[i] >= '0' && str[i] <= '9')
    {
        tc[0] = str[i] - '0';
    }
    i++;
    if (str[i] >= '0' && str[i] <= '9')
    {
        tc[1] = str[i] - '0';
        i++;
        datetime.tm_mon = tc[0] * 10 + tc[1];
    }
    else
    {
        datetime.tm_mon = tc[0];
    }

    if (datetime.tm_mon > 0)
    {
        datetime.tm_mon = datetime.tm_mon - 1;
    }

    if ((unsigned char)str[i] > 0x7F)
    {
        i += 3;
    }
    else
    {
        i++;
    }

    if (str[i] >= '0' && str[i] <= '9')
    {
        tc[0] = str[i] - '0';
    }
    i++;
    if (str[i] >= '0' && str[i] <= '9')
    {
        tc[1] = str[i] - '0';
        i++;
        datetime.tm_mday = tc[0] * 10 + tc[1];
    }
    else
    {
        datetime.tm_mday = tc[0];
    }

    if (datetime.tm_mday == 0)
    {
        datetime.tm_mday = 1;
    }

    if ((unsigned char)str[i] > 0x7F)
    {
        i += 3;
    }
    else
    {
        i++;
    }

    if (i >= str.length())
    {
        datetime.tm_hour = 0;
        datetime.tm_min  = 0;
        datetime.tm_sec  = 0;
    }
    else
    {

        for (; i < str.length(); i++)
        {
            if (str[i] != 0x20)
            {
                break;
            }
        }

        if (str[i] >= '0' && str[i] <= '9')
        {
            tc[0] = str[i] - '0';
        }
        i++;

        if (str[i] >= '0' && str[i] <= '9')
        {
            tc[1] = str[i] - '0';
            i++;
            datetime.tm_hour = tc[0] * 10 + tc[1];
        }
        else
        {
            datetime.tm_hour = tc[0];
        }

        if ((unsigned char)str[i] > 0x7F)
        {
            i += 3;
        }
        else
        {
            i++;
        }

        if (i >= str.length())
        {

            datetime.tm_min = 0;
        }
        else
        {

            if (str[i] >= '0' && str[i] <= '9')
            {
                tc[0] = str[i] - '0';
            }
            i++;
            if (str[i] >= '0' && str[i] <= '9')
            {
                tc[1] = str[i] - '0';
                i++;
                datetime.tm_min = tc[0] * 10 + tc[1];
            }
            else
            {
                datetime.tm_min = tc[0];
            }

            if ((unsigned char)str[i] > 0x7F)
            {
                i += 3;
            }
            else
            {
                i++;
            }
        }

        if (i >= str.length())
        {

            datetime.tm_sec = 0;
        }
        else
        {

            if (str[i] >= '0' && str[i] <= '9')
            {
                tc[0] = str[i] - '0';
            }
            i++;
            if (str[i] >= '0' && str[i] <= '9')
            {
                tc[1] = str[i] - '0';
                i++;
                datetime.tm_sec = tc[0] * 10 + tc[1];
            }
            else
            {
                datetime.tm_sec = tc[0];
            }
        }
    }

    temp = mktime(&datetime);

    return temp;
}
std::string get_uuid()
{
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char *v     = "0123456789abcdef";
    const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

    std::string res;
    for (int i = 0; i < 16; i++)
    {
        if (dash[i])
            res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}
}// namespace http
