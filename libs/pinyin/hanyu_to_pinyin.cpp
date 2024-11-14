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
#include "hanyu_to_pinyin.h"
#include "serverconfig.h"

namespace http
{ 
    hanyu_to_pinyin &get_hanyu_to_pinyin()
    {
        static hanyu_to_pinyin instance;
        return instance;
    }
    bool hanyu_to_pinyin::init_data(const std::string &filename)
    {
        if(filename.size()>0)
        {
            return read_data(filename);
        }
        else
        {
            serverconfig &sysconfigpath = getserversysconfig();
            std::string htmlcontent     = sysconfigpath.configpath;
            htmlcontent.append("pinyin.txt");
            return read_data(htmlcontent);
        }
    }
    bool hanyu_to_pinyin::is_loaded()
    {
        if(pinyin_data.size()>0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    void hanyu_to_pinyin::clear_data()
    {
        pinyin_data.clear();
    }
    bool hanyu_to_pinyin::read_data(const std::string &filename)
    {
        std::string htmlcontent;
        std::unique_ptr<std::FILE, int (*)(FILE *)> fp(std::fopen(filename.c_str(), "rb"), std::fclose);
        if(fp.get())
        {
            htmlcontent.clear();
            fseek(fp.get(), 0, SEEK_END);
            unsigned long long file_size = ftell(fp.get());
            fseek(fp.get(), 0, SEEK_SET);
            htmlcontent.resize(file_size);
            file_size = fread(&htmlcontent[0], 1, file_size, fp.get());
            htmlcontent.resize(file_size);

            pinyin_t temp_data;
            std::string lines_data;
            std::string py_temp_data;
            for(unsigned int i=0;i< htmlcontent.size();i++)
            {
                if(htmlcontent[i]==0x0A||htmlcontent[i]==0x0D)
                {
                    unsigned int code_value=0;
                    unsigned int j=0;
                    for(;j<lines_data.size();j++)
                    {
                        if(lines_data[j]==':')
                        {
                            j++;
                            break;
                        }  
                        if(lines_data[j]>='0'&&lines_data[j]<='9')
                        {
                            code_value = code_value * 10 + (lines_data[j]-'0');
                        }
                    }
                    if(code_value > 0)
                    {
                        py_temp_data.clear();
                        for(;j<lines_data.size();j++)
                        {
                            py_temp_data.push_back(lines_data[j]);
                        }
                        if(py_temp_data.size()>0)
                        {
                            temp_data.code = code_value;
                            for(unsigned int n=0;n<8;n++)
                            {
                                if(n<py_temp_data.size())
                                {
                                    temp_data.value[n]=py_temp_data[n];
                                }
                                else
                                {
                                    temp_data.value[n]=0x00;
                                }
                            }
                            temp_data.value[7]=0x00;
                            pinyin_data.push_back(temp_data);
                        }
                    }

                    lines_data.clear();
                    i++;
                    if(i<htmlcontent.size() &&htmlcontent[i]!=0x0D)
                    {
                        i--;
                    }
                    continue;
                }

                lines_data.push_back(htmlcontent[i]);
            }
            if(pinyin_data.size()==0)
            {
                return false;
            }
            return true;
        }
        return false;
    }
    std::string hanyu_to_pinyin::get_pinyin(const std::string &source,bool isup)
    {
        unsigned int pos=0;
        unsigned int code=0;
        unsigned char c[4];
        
        std::string result;
        result.clear();
 
        for (; pos < source.size(); pos++)
        {
            c[0] = (unsigned char)source[pos];
            if (c[0] < 0x80)
            {
                result.push_back(c[0]);
            }
            else if (c[0] >= 0xC0 && c[0] < 0xE0)
            {
                //两个utf8
                c[1] = (unsigned char)source[pos + 1];
                pos += 1;
                c[0]     = c[0] & 0x1F;
                c[1]     = c[1] & 0x3F;
                code = 0;
                code = c[0] << 6 | c[1];

                dichotomy_search(result,code,isup,0,pinyin_data.size());
            }
            else if (c[0] >= 0xE0 && c[0] < 0xF0)
            {
                            //三个utf8
                c[1] = (unsigned char)source[pos + 1];
                c[2] = (unsigned char)source[pos + 2];
                pos += 2;

                c[0] = c[0] & 0x0F;
                c[1] = c[1] & 0x3F;
                c[2] = c[2] & 0x3F;

                //取两个字符
                code = 0;
                code = c[0] << 6 | c[1];
                code = code << 6 | c[2];

                dichotomy_search(result,code,isup,0,pinyin_data.size());
            }
            else if (c[0] >= 0xF0 && c[0] < 0xF8)
            {
                //四个utf8
                c[1] = (unsigned char)source[pos + 1];
                c[2] = (unsigned char)source[pos + 2];
                c[3] = (unsigned char)source[pos + 3];
                pos += 3;

                c[0] = c[0] & 0x0F;
                c[1] = c[1] & 0x3F;
                c[2] = c[2] & 0x3F;
                c[3] = c[3] & 0x3F;

                code = 0;
                code = c[0] << 6 | c[1];
                code = code << 6 | c[2];
                code = code << 6 | c[3];

                dichotomy_search(result,code,isup,0,pinyin_data.size());
            }
        }
        return result;
    } 
    void hanyu_to_pinyin::dichotomy_search(std::string &result,unsigned int code,bool is_up_char, unsigned int a, unsigned int b)
    {
        if(pinyin_data.size()==0)
        {
            return;
        }
        unsigned int temp_num = a + (b - a) / 2;
        if(pinyin_data.size()==temp_num)
        {
            return;
        }
        if (code == pinyin_data[temp_num].code)
        {
            if(is_up_char)
            {
                a=result.size();
            }
            result.append(pinyin_data[temp_num].value);
            if(is_up_char)
            {
                if(result.size()>a)
                {
                    result[a]=result[a]-32;
                }
            }
            return;
        }
 
        if (code < pinyin_data[temp_num].code)
        {
            if(temp_num>0)
            {
                dichotomy_search(result,code,is_up_char,a, temp_num - 1);
            }
        }
        else
        {
            dichotomy_search(result,code,is_up_char,temp_num + 1, b);
        }
    }
}