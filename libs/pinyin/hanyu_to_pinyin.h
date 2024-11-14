#ifndef LIBS_PINYIN_HANYU_TO_PINYIN_H
#define LIBS_PINYIN_HANYU_TO_PINYIN_H
/*
*name 汉语转拼音
*date 2024-11-03
*author 黄自权 
*data sources https://github.com/mozillazg/pinyin-data
*/
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <vector>


namespace http
{
    struct pinyin_t{
        unsigned int code=0;
        char value[8]={0};
    };

    class hanyu_to_pinyin
    {
    public:
        hanyu_to_pinyin()
        {

        }
        bool is_loaded();
        bool init_data(const std::string &filename="");
        bool read_data(const std::string &filename);
        void clear_data();
        std::string get_pinyin(const std::string &source,bool isup=false);
        void dichotomy_search(std::string &result,unsigned int code,bool is_up_char,unsigned int a, unsigned int b);
        
    public:
    std::vector<pinyin_t> pinyin_data;
    };
    hanyu_to_pinyin &get_hanyu_to_pinyin();
}
#endif