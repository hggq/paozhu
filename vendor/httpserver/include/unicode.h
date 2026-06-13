#ifndef PROJECT_UNICODE_HPP
#define PROJECT_UNICODE_HPP

#include <string>
#include <cstdlib>
#include <charconv>
#include <cstring>
#include <string_view>
#include <vector>
#include <utility>
#include <concepts>
#include <iostream>
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
    std::string utf8_to_json_string(const std::string &source);
    
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
    bool str_colname_casecmp(std::string_view str1, std::string_view str2);

    template<typename T>
    std::string to_json_value(const T& val) 
    {
        using U = std::decay_t<T>;
        if constexpr (std::is_arithmetic_v<U>) {
            if constexpr (std::is_same_v<U, bool>) {
                return val ? "true" : "false";
            } else {
                return std::to_string(val);
            }
        } else if constexpr (std::is_convertible_v<U, std::string_view>) {
            //std::string s = std::string(val);
            return utf8_to_json_string(val);
        } else {
            return "";
        }
    }

        // 整数赋值重载
    template <typename T>
    inline auto try_set_val(T& field, const unsigned char* buf, size_t length) 
        -> std::enable_if_t<std::is_integral_v<T>> 
    {
        T _tmp{};
        auto [ptr, ec] = std::from_chars(
            reinterpret_cast<const char*>(buf),
            reinterpret_cast<const char*>(buf) + length,
            _tmp);
        if (ec == std::errc{}) {
            field = _tmp;
        }
        else
        {
            field = 0;
        }
    }

    // 浮点赋值重载
    template <typename T>
    inline auto try_set_val(T& field, const unsigned char* buf, size_t length) 
        -> std::enable_if_t<std::is_floating_point_v<T>> 
    {
        T _tmp{};
        auto [ptr, ec] = std::from_chars(
            reinterpret_cast<const char*>(buf),
            reinterpret_cast<const char*>(buf) + length,
            _tmp);
        if (ec == std::errc{}) {
            field = _tmp;
        }
        else
        {
            field = 0;
        }
    }

    // 字符串赋值重载
    template <typename T>
    inline auto try_set_val(T& field, const unsigned char* buf, size_t length) 
        -> std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string>> 
    {
        field = std::string(reinterpret_cast<const char*>(buf), length);
    }

    // 字符串赋值重载
    template <typename T>
    inline auto try_set_val(T& field, const unsigned char* buf, size_t length) 
        -> std::enable_if_t<std::is_same_v<std::decay_t<T>, bool>> 
    {
        if(length > 0 && length < 4)
        {
            if(buf[0] != '0')
            {
                field = true;
            }
            else
            {
                field = false;
            }
        }
        else if(length > 2)
        {
            if(buf[0] == 'F' || buf[0] == 'f')
            {
                field = false;
            }
            else
            {
                field = true;
            }
        }
    }
}
#endif