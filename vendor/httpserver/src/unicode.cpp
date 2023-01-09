#include <string>
#include <cstdlib>
#include <charconv>
#include <cstring>
#include <string_view>
#include <vector>
#include "unicode.h"


namespace http
{
    /*
     *  utf-16转utf8
     */
    std::string unicode_to_utf8(std::string &source)
    {
        //默认大端在前
        bool isbig = true;
        unsigned int pos = 0;
        unsigned short big_char; //大端高位
        unsigned short low_char; //低位
        unsigned int code;
        unsigned char c[5]={0x00};
        if (source.size() > 3)
        {
            c[0] = (unsigned char)source[0];
            c[1] = (unsigned char)source[1];
        }
        std::string obj;

        if (c[0] == 0xFF && c[1] == 0xFE)
        {
            isbig = false;
            pos = 2;
        }
        else if (c[0] == 0xFE && c[1] == 0xFF)
        {
            isbig = true;
            pos = 2;
        }
        for (; pos < source.size(); pos++)
        {
            //读取顺序
            if (isbig)
            {
                big_char = ((unsigned char)source[pos]) << 8 | (unsigned char)source[pos + 1];
            }
            else
            {
                big_char = ((unsigned char)source[pos + 1]) << 8 | (unsigned char)source[pos];
            }
            pos += 1;
            //从U+D800到U+DFFF的码位（代理区）
            //其中0xD800——0xDBFF是前导代理(lead surrogates).0xDC00——0xDFFF是后尾代理(trail surrogates)
            if (big_char < 0xD800 || big_char > 0xDFFF)
            {

                if (big_char < 0x80)
                {
                    c[0] = big_char & 0x7F;
                    obj.push_back(c[0]);
                }
                else if (big_char < 0x7FF)
                {
                    // c[2]='\0';
                    c[1] = (big_char & 0x3F) | 0x80;
                    c[0] = ((big_char >> 6) & 0x3F) | 0x80;
                    obj.push_back(c[0]);
                    obj.push_back(c[1]);
                }
                else
                {
                    // c[3]='\0';
                    c[2] = (big_char & 0x3F) | 0x80;
                    c[1] = ((big_char >> 6) & 0x3F) | 0x80;
                    c[0] = ((big_char >> 12) & 0x0F) | 0xE0;
                    obj.push_back(c[0]);
                    obj.push_back(c[1]);
                    obj.push_back(c[2]);
                }
            }
            else
            {
                if (isbig)
                {
                    low_char = ((unsigned char)source[pos + 1]) << 8 | (unsigned char)source[pos + 2];
                }
                else
                {
                    low_char = ((unsigned char)source[pos + 2]) << 8 | (unsigned char)source[pos + 1];
                }
                pos += 2;
                code = 0x10000 | ((big_char xor 0xD800) << 10) | (low_char xor 0xDC00);
                c[3] = (code & 0x3F) | 0x80;
                c[2] = ((code >> 6) & 0x3F) | 0x80;
                c[1] = ((code >> 12) & 0x3F) | 0x80;
                c[0] = ((code >> 18) & 0x07) | 0xF0;
                // c[4]='\0';
                obj.push_back(c[0]);
                obj.push_back(c[1]);
                obj.push_back(c[2]);
                obj.push_back(c[3]);
            }
        }
        return obj;
    }
    /*
     *  utf8转unicode
     */
    std::string utf8_to_unicode(std::string &source)
    {
        unsigned int pos = 0;
        unsigned short big_char; //大端高位
        unsigned short low_char; //低位
        unsigned int code;
        unsigned char c[5]={0x00};
        std::string obj;
        if (source.size() > 3)
        {

            c[0] = (unsigned char)source[0];
            c[1] = (unsigned char)source[1];
            c[2] = (unsigned char)source[2];
        }
        if (c[0] == 0xEF && c[1] == 0xBB && c[2] == 0xBF)
        {
            pos = 3;
        }
        for (; pos < source.size(); pos++)
        {
            c[0] = (unsigned char)source[pos];
            if (c[0] < 0x80)
            {
                obj.push_back(0x00);
                obj.push_back(c[0]);
            }
            else if (c[0] < 0xC0)
            {
                //原装ascii
                // obj.push_back(c[0]);
            }
            else if (c[0] >= 0xC0 && c[0] < 0xE0)
            {
                //两个utf8
                c[1] = (unsigned char)source[pos + 1];
                pos += 1;
                c[0] = c[0] & 0x1F;
                c[1] = c[1] & 0x3F;
                big_char = c[0] << 6 | c[1];
                //取两个字符
                c[1] = big_char & 0xFF;
                c[0] = (big_char & 0xFF00) >> 8;

                obj.push_back(c[0]);
                obj.push_back(c[1]);
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
                big_char = c[0] << 6 | c[1];
                big_char = big_char << 6 | c[2];

                //取两个字符
                c[1] = big_char & 0xFF;
                c[0] = (big_char & 0xFF00) >> 8;

                obj.push_back(c[0]);
                obj.push_back(c[1]);
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

                code = c[0] << 6 | c[1];
                code = code << 6 | c[2];
                code = code << 6 | c[3];

                big_char = ((code - 0x10000) >> 10) | 0xD800;
                low_char = ((code - 0x10000) & 0x3FF) | 0xDC00;
                //其中0xD800——0xDBFF是前导代理(lead surrogates).0xDC00——0xDFFF是后尾代理(trail surrogates)
                // code=(((((code - 0x10000) >>10) | 0xD800) << 16)  | (((code-0x10000) & 0x3FF) | 0xDC00));

                c[1] = big_char & 0xFF;
                c[0] = big_char >> 8 & 0xFF;

                c[3] = low_char & 0xFF;
                c[2] = low_char >> 8 & 0xFF;

                obj.push_back(c[0]);
                obj.push_back(c[1]);
                obj.push_back(c[2]);
                obj.push_back(c[3]);
            }
            else
            {
                // obj.push_back(c[0]);
            }
        }

        return obj;
    }
    /*
     *  utf8转unicodestring
     */
    std::string utf8_to_unicodestring(std::string &source)
    {
        unsigned int pos = 0;
        unsigned short big_char; //大端高位
        unsigned short low_char; //低位
        unsigned int code;
        unsigned char c[5]={0x00};
        unsigned char str[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        std::string obj;
        if (source.size() > 3)
        {

            c[0] = (unsigned char)source[0];
            c[1] = (unsigned char)source[1];
            c[2] = (unsigned char)source[2];
        }
        if (c[0] == 0xEF && c[1] == 0xBB && c[2] == 0xBF)
        {
            pos = 3;
        }
        for (; pos < source.size(); pos++)
        {
            c[0] = (unsigned char)source[pos];
            if (c[0] < 0x80)
            {

                obj.push_back('0');
                obj.push_back('0');
                c[3] = c[0] & 0x0F;
                c[2] = c[0] >> 4 & 0x0F;
                obj.push_back(str[c[2]]);
                obj.push_back(str[c[3]]);
            }
            else if (c[0] < 0xC0)
            {
                //原装ascii
                // obj.push_back(c[0]);
            }
            else if (c[0] >= 0xC0 && c[0] < 0xE0)
            {
                //两个utf8
                c[1] = (unsigned char)source[pos + 1];
                pos += 1;
                c[0] = c[0] & 0x1F;
                c[1] = c[1] & 0x3F;
                big_char = c[0] << 6 | c[1];
                //取两个字符
                // c[1]=big_char&0xFF;
                // c[0]=(big_char&0xFF00)>>8;

                // 1
                c[0] = big_char & 0xF000 >> 12;
                obj.push_back(str[c[0]]);
                // 2
                c[0] = big_char & 0xF00 >> 8;
                obj.push_back(str[c[0]]);

                // 3
                c[0] = big_char & 0xF0 >> 4;
                obj.push_back(str[c[0]]);

                // 4
                c[0] = big_char & 0x0F;
                obj.push_back(str[c[0]]);
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
                big_char = c[0] << 6 | c[1];
                big_char = big_char << 6 | c[2];

                //取两个字符
                //   c[1]=big_char&0xFF;
                //   c[0]=(big_char&0xFF00)>>8;

                c[0] = big_char & 0x0F;
                c[1] = big_char >> 4 & 0x0F;
                c[2] = big_char >> 8 & 0x0F;
                c[3] = big_char >> 12 & 0x0F;

                obj.push_back(str[c[3]]);
                obj.push_back(str[c[2]]);
                obj.push_back(str[c[1]]);
                obj.push_back(str[c[0]]);
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

                code = c[0] << 6 | c[1];
                code = code << 6 | c[2];
                code = code << 6 | c[3];

                big_char = ((code - 0x10000) >> 10) | 0xD800;
                low_char =((code - 0x10000) & 0x3FF )| 0xDC00;
                //其中0xD800——0xDBFF是前导代理(lead surrogates).0xDC00——0xDFFF是后尾代理(trail surrogates)
                // code=(((((code - 0x10000) >>10) | 0xD800) << 16)  | (((code-0x10000) & 0x3FF) | 0xDC00));

                c[0] = big_char & 0x0F;
                c[1] = big_char >> 4 & 0x0F;
                c[2] = big_char >> 8 & 0x0F;
                c[3] = big_char >> 12 & 0x0F;

                obj.push_back(str[c[3]]);
                obj.push_back(str[c[2]]);
                obj.push_back(str[c[1]]);
                obj.push_back(str[c[0]]);

                c[0] = low_char & 0x0F;
                c[1] = low_char >> 4 & 0x0F;
                c[2] = low_char >> 8 & 0x0F;
                c[3] = low_char >> 12 & 0x0F;

                obj.push_back(str[c[3]]);
                obj.push_back(str[c[2]]);
                obj.push_back(str[c[1]]);
                obj.push_back(str[c[0]]);
            }
            else
            {
                // obj.push_back(c[0]);
            }
        }

        return obj;
    }

    /*
     *  utf8转jsonstring
     */
    std::string utf8_to_jsonstring(std::string &source)
    {
        unsigned int pos = 0;
        unsigned short big_char; //大端高位
        unsigned short low_char; //低位
        unsigned int code;
        unsigned char c[5];
        unsigned char str[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        std::string obj;

        for (; pos < source.size(); pos++)
        {
            c[0] = (unsigned char)source[pos];
            if (c[0] < 0x80)
            {

                switch (c[0])
                {
                case 0x22: //"
                    obj.push_back(0x5c);
                    obj.push_back(0x22);
                    break;
                case 0x60: //''
                    obj.push_back(0x60);
                    break;
                case 0x27: //'\'
                    obj.push_back(0x27);
                    break;
                case 0x2f: //\/
                    obj.push_back(0x2f);
                    break;
                case 0x08: //\b
                    break;
                case 0x0c: //\f
                    break;
                case 0x0a: //\n
                    obj.push_back(0x5c);
                    obj.push_back('n');
                    break;
                case 0x0d: //\r
                    obj.push_back(0x5c);
                    obj.push_back('r');
                    break;
                case 0x09: //\t
                    obj.push_back(0x5c);
                    obj.push_back('t');
                    break;
                case 0x5c: //
                    obj.push_back(0x5c);
                    obj.push_back(0x5c);
                    break;
                default:
                    obj.push_back(c[0]);
                }
            }
            else if (c[0] < 0xC0)
            {
                //原装ascii
                // obj.push_back(c[0]);
            }
            else if (c[0] >= 0xC0 && c[0] < 0xE0)
            {
                //两个utf8
                c[1] = (unsigned char)source[pos + 1];
                pos += 1;
                c[0] = c[0] & 0x1F;
                c[1] = c[1] & 0x3F;
                big_char = c[0] << 6 | c[1];
                //取两个字符
                // c[1]=big_char&0xFF;
                // c[0]=(big_char&0xFF00)>>8;

                obj.push_back(0x5c);
                obj.push_back('u');
                // 1
                c[0] = big_char & 0xF000 >> 12;
                obj.push_back(str[c[0]]);
                // 2
                c[0] = big_char & 0xF00 >> 8;
                obj.push_back(str[c[0]]);

                // 3
                c[0] = big_char & 0xF0 >> 4;
                obj.push_back(str[c[0]]);

                // 4
                c[0] = big_char & 0x0F;
                obj.push_back(str[c[0]]);
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
                big_char = c[0] << 6 | c[1];
                big_char = big_char << 6 | c[2];

                //取两个字符
                //   c[1]=big_char&0xFF;
                //   c[0]=(big_char&0xFF00)>>8;

                c[0] = big_char & 0x0F;
                c[1] = big_char >> 4 & 0x0F;
                c[2] = big_char >> 8 & 0x0F;
                c[3] = big_char >> 12 & 0x0F;

                obj.push_back(0x5c);
                obj.push_back('u');

                obj.push_back(str[c[3]]);
                obj.push_back(str[c[2]]);
                obj.push_back(str[c[1]]);
                obj.push_back(str[c[0]]);
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

                code = c[0] << 6 | c[1];
                code = code << 6 | c[2];
                code = code << 6 | c[3];

                big_char = ((code - 0x10000) >> 10) | 0xD800;
                low_char = ((code - 0x10000) & 0x3FF) | 0xDC00;
                //其中0xD800——0xDBFF是前导代理(lead surrogates).0xDC00——0xDFFF是后尾代理(trail surrogates)
                // code=(((((code - 0x10000) >>10) | 0xD800) << 16)  | (((code-0x10000) & 0x3FF) | 0xDC00));

                c[0] = big_char & 0x0F;
                c[1] = big_char >> 4 & 0x0F;
                c[2] = big_char >> 8 & 0x0F;
                c[3] = big_char >> 12 & 0x0F;

                obj.push_back(0x5c);
                obj.push_back('u');

                obj.push_back(str[c[3]]);
                obj.push_back(str[c[2]]);
                obj.push_back(str[c[1]]);
                obj.push_back(str[c[0]]);

                c[0] = low_char & 0x0F;
                c[1] = low_char >> 4 & 0x0F;
                c[2] = low_char >> 8 & 0x0F;
                c[3] = low_char >> 12 & 0x0F;

                obj.push_back(0x5c);
                obj.push_back('u');

                obj.push_back(str[c[3]]);
                obj.push_back(str[c[2]]);
                obj.push_back(str[c[1]]);
                obj.push_back(str[c[0]]);
            }
            else
            {
                // obj.push_back(c[0]);
            }
        }

        return obj;
    }
    /*
     *  jsonstring 转 utf8
     */
    std::string jsonstring_to_utf8(std::string_view jsonstr,unsigned int &offset_length)
    {
        std::string str;
        unsigned int j = 0;

        if (jsonstr.length() > 20)
        {
            str.reserve(jsonstr.length());
        }
        if (jsonstr[j] == 0x22)
        {
            j++;
        }

        for (; j < jsonstr.length(); j++)
        {

            if (jsonstr[j] == 0x5c) //'\'
            {
                // 处理有斜杠情况 not slash
                switch (jsonstr[j + 1])
                {
                case 0x22: //"
                    str += 0x22;
                    j += 1;
                    break;
                case 0x5c: //'\'
                    str += 0x5c;
                    j += 1;
                    break;
                // case 0x2f: //\/
                case 0x62: //\b
                    str += 0x08;
                    j += 1;
                    break;
                case 0x66: //\f
                    str += 0x0c;
                    j += 1;
                    break;
                case 0x6e: //\n
                    str += 0x0a;
                    j += 1;
                    break;
                case 0x72: //\r
                    str += 0x0d;
                    j += 1;
                    break;
                case 0x74: //\t
                    str += 0x09;
                    j += 1;
                    break;
                case 'u':
                    // str+=json.substr(j,6);
                    {
                        unsigned char c[10];
                        unsigned char ch;
                        unsigned int temp;

                        // 检查是不是emoji两个转码符一共12个字符 this emoji char
                        if (jsonstr[j + 2] == 'd' && jsonstr[j + 6] == 0x5c && jsonstr[j + 7] == 'u' && jsonstr[j + 8] == 'd')
                        {

                            // 转换成utf16
                            for (int si = 2, cj = 0; si < 12; si++)
                            {

                                if (jsonstr[j + si] != 0x5c && jsonstr[j + si] != 'u')
                                {
                                    ch = jsonstr[j + si];
                                    //chartoint(ch, c[cj]);

                                    if (ch >= '0' && ch <= '9')
                                        c[cj]= (unsigned char)(ch - '0');
                                    if (ch >= 'a' && ch <= 'f')
                                        c[cj] = (unsigned char)(ch - 'a' + 10);
                                    if (ch >= 'A' && ch <= 'F')
                                        c[cj]= (unsigned char)(ch - 'A' + 10);

                                    cj++;
                                }
                            }

                            unsigned short ca = c[0] << 12 | c[1] << 8 | c[2] << 4 | c[3];
                            unsigned short cb = c[4] << 12 | c[5] << 8 | c[6] << 4 | c[7];
                            temp = 0x10000 | ((ca - 0xD800) << 10) | (cb - 0xDC00);
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
                                //chartoint(ch, c[cj]);
                                if (ch >= '0' && ch <= '9')
                                    c[cj]= (unsigned char)(ch - '0');
                                if (ch >= 'a' && ch <= 'f')
                                    c[cj] = (unsigned char)(ch - 'a' + 10);
                                if (ch >= 'A' && ch <= 'F')
                                    c[cj]= (unsigned char)(ch - 'A' + 10);

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
            }
        }
        offset_length=offset_length+j;
        return str;
    }
    /*
     *  unicode转unicodestring
     */
    std::string unicode_to_unicodestring(std::string &source)
    {
        unsigned int pos = 0;
        unsigned char c[3];
        unsigned char str[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        std::string obj;
        for (; pos < source.size(); pos++)
        {
            c[0] = (unsigned char)source[pos];
            c[2] = c[0] & 0x0F;
            c[1] = c[0] >> 4 & 0x0F;

            obj.push_back(str[c[1]]);
            obj.push_back(str[c[2]]);
        }
        return obj;
    }
    /*
     *  unicode转unicodestring
     */
    std::string utf8_to_string(std::string &source)
    {
        unsigned int pos = 0;
        unsigned char c[3];
        unsigned char str[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        std::string obj;
        for (; pos < source.size(); pos++)
        {
            c[0] = (unsigned char)source[pos];
            c[2] = c[0] & 0x0F;
            c[1] = c[0] >> 4 & 0x0F;

            obj.push_back(str[c[1]]);
            obj.push_back(str[c[2]]);
        }
        return obj;
    }
    /*
     *  long转hexstring
     */
    std::string to_hexstring(unsigned long long soucr)
    {
        int pos = 0;
        unsigned char c[18];
        unsigned char str[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        std::string obj;
        bool isbegin = false;

        for (; pos < 16; pos++)
        {
            c[pos] = str[soucr & 0x0F];
            soucr = soucr >> 4;
        }
        if(pos>0)
        {
            pos--;    
        }
        
        for (; pos >= 0; pos--)
        {
            if (c[pos] != '0')
            {
                isbegin = true;
            }
            if (isbegin)
            {
                obj.push_back(c[pos]);
            }
        }
        return obj;
    }
    /*
     * unicode 数字字符转utf8
     */

    std::string stringunicode_to_utf8(std::string &source)
    {
        unsigned int pos = 0;
        unsigned char c[2], zi;
        std::string obj;
        for (; pos < source.size(); pos++)
        {
            zi = (unsigned char)source[pos];
            pos += 1;
            if (zi > 0x60)
            {
                c[1] = zi - 'a' + 10;
            }
            else if (zi > 0x40)
            {
                c[1] = zi - 'A' + 10;
            }
            else
            {
                c[1] = zi - '0';
            }
            zi = (unsigned char)source[pos];

            if (zi > 0x60)
            {
                c[0] = zi - 'a' + 10;
            }
            else if (zi > 0x40)
            {
                c[0] = zi - 'A' + 10;
            }
            else
            {
                c[0] = zi - '0';
            }

            zi = c[1] << 4 | c[0];
            obj.push_back(zi);
        }
        return unicode_to_utf8(obj);
    }
}