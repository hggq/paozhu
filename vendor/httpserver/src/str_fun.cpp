
#include <iostream>
#include <string>
#include "str_fun.h"

namespace http
{
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