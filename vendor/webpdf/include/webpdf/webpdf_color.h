#ifndef WEBPDF_COLOR_H
#define WEBPDF_COLOR_H

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

namespace pz
{

inline std::string web_color_lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// 解析 CSS/SVG 颜色：#rgb、#rgba、#rrggbb、#rrggbbaa、rgb(r,g,b) 和 Web 常用具名色。
// 失败返回 false，此时 r/g/b 不变。透明度目前没有下游消费，8 位十六进制只取 RGB。
inline bool parse_web_color(const std::string &value, int &r, int &g, int &b)
{
    std::size_t start = value.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return false;
    std::size_t end = value.find_last_not_of(" \t\n\r");
    std::string v   = value.substr(start, end - start + 1);

    if (v == "none" || v == "null")
        return false;

    if (v[0] == '#')
    {
        std::string hex = v.substr(1);
        if (hex.size() == 8)
        {
            hex = hex.substr(0, 6);
        }
        else if (hex.size() == 4)
        {
            std::string rgb;
            for (std::size_t i = 0; i < 3; i++)
            {
                rgb += hex[i];
                rgb += hex[i];
            }
            hex = rgb;
        }
        else if (hex.size() == 3)
        {
            std::string rgb;
            for (char c : hex)
            {
                rgb += c;
                rgb += c;
            }
            hex = rgb;
        }
        if (hex.size() != 6)
            return false;
        try
        {
            int tr = std::stoi(hex.substr(0, 2), nullptr, 16);
            int tg = std::stoi(hex.substr(2, 2), nullptr, 16);
            int tb = std::stoi(hex.substr(4, 2), nullptr, 16);
            r      = tr;
            g      = tg;
            b      = tb;
            return true;
        }
        catch (const std::exception &)
        {
            return false;
        }
    }

    if (v.size() > 4 && web_color_lower(v.substr(0, 4)) == "rgb(")
    {
        std::string inner = v.substr(4);
        std::size_t paren = inner.find(')');
        if (paren != std::string::npos)
            inner = inner.substr(0, paren);
        std::replace(inner.begin(), inner.end(), ',', ' ');
        std::istringstream iss(inner);
        int tr = 0, tg = 0, tb = 0;
        if (!(iss >> tr >> tg >> tb))
            return false;
        r = tr;
        g = tg;
        b = tb;
        return true;
    }

    std::string lower = web_color_lower(v);
    if (lower == "black")                          { r = 0;   g = 0;   b = 0;   }
    else if (lower == "white")                     { r = 255; g = 255; b = 255; }
    else if (lower == "red")                       { r = 255; g = 0;   b = 0;   }
    else if (lower == "green")                     { r = 0;   g = 128; b = 0;   }
    else if (lower == "blue")                      { r = 0;   g = 0;   b = 255; }
    else if (lower == "yellow")                    { r = 255; g = 255; b = 0;   }
    else if (lower == "cyan")                      { r = 0;   g = 255; b = 255; }
    else if (lower == "magenta")                   { r = 255; g = 0;   b = 255; }
    else if (lower == "gray" || lower == "grey")   { r = 128; g = 128; b = 128; }
    else if (lower == "orange")                    { r = 255; g = 165; b = 0;   }
    else if (lower == "purple")                    { r = 128; g = 0;   b = 128; }
    else if (lower == "brown")                     { r = 165; g = 42;  b = 42;  }
    else if (lower == "pink")                      { r = 255; g = 192; b = 203; }
    else if (lower == "navy")                      { r = 0;   g = 0;   b = 128; }
    else if (lower == "teal")                      { r = 0;   g = 128; b = 128; }
    else
        return false;
    return true;
}

}// namespace pz

#endif
