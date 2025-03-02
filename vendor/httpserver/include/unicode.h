#ifndef PROJECT_UNICODE_HPP
#define PROJECT_UNICODE_HPP

#include <string>
#include <cstdlib>
#include <charconv>
#include <cstring>
#include <string_view>
#include <vector>

namespace http
{

    /*
    *   消除空格
    */
    unsigned int json_string_trim(std::string_view _json_data,unsigned int _offset);
    /*
     *  utf-16转utf8
     */
    std::string unicode_to_utf8(std::string &source);
    /*
     *  utf8转unicode
     */
    std::string utf8_to_unicode(std::string &source);
    /*
     *  utf8转unicodestring
     */
    std::string utf8_to_unicodestring(std::string &source);
    /*
     *  utf8转jsonstring
     */
    //std::string utf8_to_jsonstring(std::string &source);
    std::string utf8_to_jsonstring(const std::string &source);
    /*
     *  jsonstring转utf8
     */
    std::string jsonstring_to_utf8(const char * jsonstr,unsigned int str_length,unsigned int &offset_length);
    /*
     *  unicode转unicodestring
     */
    std::string unicode_to_unicodestring(std::string &source);
    /*
     *  unicode转unicodestring
     */
    std::string utf8_to_string(std::string &source);
    /*
     *  long转hexstring
     */
    std::string to_hexstring(unsigned long long soucr);
    /*
     * unicode 数字字符转utf8
     */

    std::string stringunicode_to_utf8(std::string &source);
}
#endif