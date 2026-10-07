/**
 *  @copyright 2023, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file autopickcontrolmethod.hpp
 *  @date 2023-01-05
 *
 *  controller cpp file auto pick method to reg file
 *
 *
 */

#ifndef PROJECT_AUTOPICKCONTROLMETHOD_HPP
#define PROJECT_AUTOPICKCONTROLMETHOD_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <array>
#include <map>
#include <vector>
#include <cstring>
#include <string_view>
namespace http
{
namespace fs = std::filesystem;
struct reg_autoitem
{
    std::string pre;
    std::string pre_type;    // pre 函数返回类型："std::string" 或 "asio::awaitable<std::string>"
    std::string func;
    std::string urlpath;
    std::string domain;
    std::string func_type;   // handler 返回类型
    std::string filename;
    bool is_co = false;      // handler 是否协程（兼容旧逻辑）
    bool pre_is_co = false;  // pre 是否协程
};
struct file_regitem
{
    std::string filename;
    std::string filetime;
    std::string filehash;
};

// make 只看 mtime：paozhu_pre 每次构建都跑，无条件重写会把生成文件的依赖者一起拖脏，
// 所以内容没变就不落盘。与视图生成器里的同名成员同义，两处各自独立（不在同一个 TU）。
inline bool write_file_if_changed(const std::string &path, const std::string &content)
{
    std::string oldcontent;
    std::FILE *rf = std::fopen(path.c_str(), "rb");
    if (rf)
    {
        std::fseek(rf, 0, SEEK_END);
        long n = std::ftell(rf);
        std::fseek(rf, 0, SEEK_SET);
        if (n > 0)
        {
            oldcontent.resize(n);
            auto nread = std::fread(&oldcontent[0], 1, n, rf);
            oldcontent.resize(nread);
        }
        std::fclose(rf);
        if (oldcontent == content)
        {
            return false;
        }
    }
    std::FILE *wf = std::fopen(path.c_str(), "wb");
    if (wf == nullptr)
    {
        std::cout << " [!] cannot write file: " << path << std::endl;
        return false;
    }
    if (!content.empty())
    {
        std::fwrite(&content[0], 1, content.size(), wf);
    }
    std::fclose(wf);
    return true;
}

// 函数签名里 '(' 之前的 token 序列去掉限定词就是返回类型。旧规则只认第一个长度 >5 的 token，
// 于是 inline / static 会被当成类型，而 int 这类短类型整个丢掉
inline std::string trim_type_tokens(const std::string &raw)
{
    static const char *quals[] = {"inline",   "static",    "constexpr", "consteval",
                                  "constinit", "virtual",  "explicit",  "extern",
                                  "mutable",   "const",    "friend"};
    std::string out, token;
    auto flush = [&]() {
        if (token.empty())
        {
            return;
        }
        bool is_qual = false;
        for (auto q : quals)
        {
            if (token == q)
            {
                is_qual = true;
                break;
            }
        }
        if (!is_qual)
        {
            if (!out.empty()) out.push_back(' ');
            out.append(token);
        }
        token.clear();
    };
    for (char c : raw)
    {
        if (c == ' ' || c == '\t' || c == 0x0D || c == 0x0A)
        {
            flush();
        }
        else
        {
            token.push_back(c);
        }
    }
    flush();
    return out;
}

// — 站点推导的唯一规则：路径第一段含 '.' 才算域名目录（saas.com/x 是，plain/site.com/x 不是） —
// 必须出现 '/'：顶层文件名自己含点（a.b.cpp）不是站点目录，它压根没有目录段
// 返回 false 时 site 一定为空，调用方可以只看返回值，也可以直接读 site
inline bool split_domain_path(const std::string &relpath, std::string &site)
{
    size_t slash = relpath.find('/');
    if (slash == std::string::npos)
    {
        site.clear();
        return false;
    }
    std::string first_seg = relpath.substr(0, slash);
    if (first_seg.find('.') == std::string::npos)
    {
        site.clear();
        return false;
    }
    site = first_seg;
    return true;
}
// — 命名空间 = 站点第一个 '.' 之前的部分；头文件的包裹与 reg 行的限定符必须同源 —
inline std::string domain_namespace(const std::string &site)
{
    size_t dot = site.find('.');
    return dot == std::string::npos ? site : site.substr(0, dot);
}

// 逐字符不区分大小写地比较两个字节
inline bool ci_equal(char a, char b)
{
    if (a == b) return true;
    if (a >= 'A' && a <= 'Z') a = static_cast<char>(a + 32);
    if (b >= 'A' && b <= 'Z') b = static_cast<char>(b + 32);
    return a == b;
}

// 与一个 C 字面量做整串比较（长度不等即不等）。
// 不用 POSIX 的 strcasecmp：MSVC 下没有它，这个头文件又是 Windows 也要编的。
inline bool str_case_equal(const std::string &a, const char *b)
{
    if (b == nullptr) return false;
    size_t n = std::strlen(b);
    if (a.size() != n) return false;
    for (size_t i = 0; i < n; i++)
    {
        if (!ci_equal(a[i], b[i])) return false;
    }
    return true;
}

// 在 text 的 pos 处按字面量做前缀匹配，只比 litlen 个字节。
// 不要写成 str_casecmp_pre(&text[pos], "..."): 那样会先 strlen 从 pos 扫到结尾，
// 行首扫描每前进一个字符都可能触发一次，大文件上是 O(n²)。
inline bool match_at(const std::string &text, size_t pos, const char *lit, size_t litlen)
{
    if (lit == nullptr || pos >= text.size() || text.size() - pos < litlen) return false;
    for (size_t k = 0; k < litlen; k++)
    {
        if (!ci_equal(text[pos + k], lit[k])) return false;
    }
    return true;
}

bool str_casecmp_pre(std::string_view str1, std::string_view str2, unsigned int length)
{
    if (length == 0)
    {
        length = str2.size();
    }

    if (str1.size() < str2.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < length; i++)
    {
        if (i < str2.size())
        {
            if (str1[i] == str2[i])
            {
                continue;
            }
            else
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
        else
        {
            return true;
        }
    }
    return true;
}

class pickcontrol
{
  public:
    void savecacheinfo(const std::string &methodpathfile, const std::map<std::string, struct file_regitem> &info_list)
    {

        std::unique_ptr<std::FILE, int (*)(std::FILE *)> f(fopen(methodpathfile.c_str(), "wb"), std::fclose);
        if (f == nullptr)
        {
            return;
        }
        std::string c_content;

        for (const auto &[first, second] : info_list)
        {
            c_content.append(second.filename);
            c_content.push_back(',');
            c_content.append(second.filetime);
            c_content.push_back(',');
            c_content.append(second.filehash);
            c_content.push_back(0x0A);
        }
        fwrite(&c_content[0], 1, c_content.size(), f.get());
    }
    std::map<std::string, struct file_regitem> loadcacheinfo(const std::string &methodpathfile)
    {
        std::map<std::string, struct file_regitem> reginfo_temp;
        struct file_regitem str_temp;
        std::string c_content;
        std::unique_ptr<std::FILE, int (*)(std::FILE *)> f(fopen(methodpathfile.c_str(), "rb"), std::fclose);
        if (f == nullptr)
        {
            return reginfo_temp;
        }
        fseek(f.get(), 0, SEEK_END);
        unsigned int file_size = ftell(f.get());
        fseek(f.get(), 0, SEEK_SET);
        c_content.resize(file_size);
        file_size = fread(&c_content[0], 1, file_size, f.get());
        c_content.resize(file_size);

        std::string tempstr;
        unsigned char foffset = 0;
        for (unsigned int i = 0; i < file_size; i++)
        {
            if (c_content[i] == 0x0A)
            {
                str_temp.filehash               = tempstr;
                reginfo_temp[str_temp.filename] = str_temp;

                str_temp.filename.clear();
                str_temp.filetime.clear();
                str_temp.filehash.clear();
                foffset = 0;
                tempstr.clear();
                continue;
            }
            if (c_content[i] == ',')
            {
                if (foffset == 0)
                {
                    str_temp.filename = tempstr;
                    tempstr.clear();
                    foffset = 1;
                }
                else if (foffset == 1)
                {
                    str_temp.filetime = tempstr;
                    tempstr.clear();
                    foffset = 2;
                }
                else
                {
                    str_temp.filehash = tempstr;
                    tempstr.clear();
                    foffset = 0;
                }
                continue;
            }
            if (c_content[i] == 0x20 || c_content[i] == '\t')
            {
                continue;
            }
            tempstr.push_back(c_content[i]);
        }
        if (tempstr.size() > 0)
        {
            if (foffset == 0)
            {
                str_temp.filename = tempstr;
                tempstr.clear();
                foffset = 1;
            }
            else if (foffset == 1)
            {
                str_temp.filetime = tempstr;
                tempstr.clear();
                foffset = 2;
            }
            else
            {
                str_temp.filehash = tempstr;
                tempstr.clear();
                foffset = 0;
            }
            reginfo_temp[str_temp.filename] = str_temp;
        }
        return reginfo_temp;
    }
    std::vector<std::string> listpath(const std::string &methodpath)
    {
        fs::path tagetpath = methodpath;
        std::vector<std::string> temp;
        if (fs::exists(tagetpath) && fs::is_directory(tagetpath))
        {

            for (const auto &entry : fs::directory_iterator(tagetpath))
            {
                auto filename = entry.path().filename().string();
                if (fs::is_regular_file(entry.status()))
                {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".cpp")
                    {
                        temp.emplace_back(filename.substr(0, filename.size() - 4));
                    }
                }
                else if (fs::is_directory(entry.status()))
                {
                    std::string ext;
                    if (methodpath.back() == '/')
                    {

                        ext.append(methodpath);
                        ext.append(filename);
                    }
                    else
                    {
                        ext.append(methodpath);
                        ext.append("/");
                        ext.append(filename);
                    }
                    std::vector<std::string> directory_temp = listpath(ext);
                    if (directory_temp.size() > 0)
                    {
                        for (unsigned int j = 0; j < directory_temp.size(); j++)
                        {
                            ext.clear();
                            ext.append(filename);
                            ext.append("/");
                            ext.append(directory_temp[j]);
                            temp.emplace_back(ext);
                        }
                    }
                }
            }
        }
        // readdir 顺序在 APFS 与 ext4/xfs 上不同，而「同名注册谁先入选」（首次保留）和路由表
        // 的 idx 都跟着这个顺序走 ⇒ 每层都排一次，顶层排完整体就是字典序
        std::sort(temp.begin(), temp.end());

        return temp;
    }
    std::vector<struct reg_autoitem> pickfile(const std::string &methodpathfile)
    {

        std::vector<struct reg_autoitem> temp;

        std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(methodpathfile.c_str(), "rb"), std::fclose);

        if (!fp.get())
        {
            return temp;
        }

        std::string filecontent;

        fseek(fp.get(), 0, SEEK_END);
        unsigned long long file_size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);

        filecontent.resize(file_size);
        file_size = fread(&filecontent[0], 1, file_size, fp.get());
        filecontent.resize(file_size);

        bool isbegin = true;
        // 本行在匹配之前已消费的 '/' 个数：注解的注释头只可能是 "//"，第三个 '/' 说明这行
        // 本身就是注释掉的一条注解（// //@urlpath(...)），不能当注册用
        unsigned char line_slash = 0;
        for (unsigned int i = 0; i < filecontent.size(); i++)
        {
            if (isbegin == false)
            {
                if (filecontent[i] == 0x0A)
                {
                    isbegin    = true;
                    line_slash = 0;
                }
                continue;
            }

            if (filecontent[i] == 0x2F)
            {
                if (++line_slash > 2)
                {
                    isbegin = false;
                    continue;
                }
            }
            else if (filecontent[i] != 0x20 && filecontent[i] != '\t'
                     && filecontent[i] != 0x0A && filecontent[i] != 0x0D)
            {
                // 行首到注解之间只允许空白：'*'（块注释/文档行）、'"' 等一律离开行首状态
                isbegin = false;
                continue;
            }
            bool is_match            = false;
            unsigned int offset_temp = 0;
            if (match_at(filecontent, i, "//@urlpath(", 11))
            {
                is_match    = true;
                offset_temp = 11;
            }
            else if (match_at(filecontent, i, "// @urlpath(", 12))
            {
                is_match    = true;
                offset_temp = 12;
            }

            if (is_match)
            {
                std::string prename;
                std::string tempname;
                unsigned int j = i + offset_temp;
                for (; j < filecontent.size(); j++)
                {
                    if (filecontent[j] == ')' || filecontent[j] == 0x0A)
                    {
                        // j++;
                        break;
                    }
                    if (filecontent[j] == ',')
                    {
                        prename = tempname;
                        tempname.clear();
                        continue;
                    }
                    if (filecontent[j] == '"')
                    {
                        continue;
                    }
                    if (filecontent[j] == '\t')
                    {
                        continue;
                    }
                    if (filecontent[j] == 0x20)
                    {
                        continue;
                    }
                    if (filecontent[j] == 0x0d)
                    {
                        continue;
                    }
                    tempname.push_back(filecontent[j]);
                }

                i = j;
                std::string linestr, func_pre_type;
                if (tempname.empty())
                {
                    isbegin = false;
                    continue;
                }
                //process remaining
                for (; j < filecontent.size(); j++)
                {
                    if (filecontent[j] == 0x0A)
                    {
                        j++;
                        break;
                    }
                }
                for (; j < filecontent.size(); j++)
                {
                    if (filecontent[j] == 0x0A || filecontent[j] == '(')
                    {
                        if (linestr.size() < 2)
                        {
                            linestr.clear();
                            if (filecontent[j] == 0x0A)
                            {
                                continue;
                            }

                            for (; j < filecontent.size(); j++)
                            {
                                if (filecontent[j] == 0x0A)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                        if (filecontent[j] == '(')
                        {
                            break;
                        }
                        // func type maybe alone line
                        func_pre_type = linestr;
                        linestr.clear();
                        continue;
                    }
                    else if (filecontent[j] == '/')
                    {
                        // is ship annotation
                        linestr.clear();
                        unsigned int nnn = j + 1;
                        if (nnn < filecontent.size() && filecontent[nnn] == '*')
                        {
                            j += 2;
                            for (; j < filecontent.size(); j++)
                            {
                                if (filecontent[j] == '*')
                                {
                                    nnn = j + 1;
                                    if (nnn < filecontent.size() && filecontent[nnn] == '/')
                                    {
                                        j++;
                                        break;
                                    }
                                }
                            }
                            continue;
                        }
                        else
                        {
                            for (; j < filecontent.size(); j++)
                            {
                                if (filecontent[j] == 0x0A)
                                {
                                    break;
                                }
                            }
                        }
                        continue;
                    }

                    if (filecontent[j] == 0x20 || filecontent[j] == '\t')
                    {
                        unsigned int jj = j + 1;
                        for (; jj < filecontent.size(); jj++)
                        {
                            if (filecontent[jj] == 0x20 || filecontent[jj] == '\t')
                            {
                                j++;
                                continue;
                            }
                            break;
                        }
                        if (jj < filecontent.size() && filecontent[jj] == '(')
                        {
                            break;
                        }
                        if (jj < filecontent.size() && filecontent[jj] == 0x0A)
                        {
                            continue;
                        }

                        if (!linestr.empty())
                        {
                            if (!func_pre_type.empty()) func_pre_type.push_back(' ');
                            func_pre_type.append(linestr);
                        }
                        linestr.clear();
                        continue;
                    }
                    if (filecontent[j] == 0x0d)
                    {
                        continue;
                    }
                    linestr.push_back(filecontent[j]);
                }

                if (linestr.size() < 2)
                {
                    //again process
                    for (; j < filecontent.size(); j++)
                    {
                        if (filecontent[j] == 0x0A)
                        {
                            j++;
                            break;
                        }
                    }
                    for (; j < filecontent.size(); j++)
                    {
                        if (filecontent[j] == 0x0A || filecontent[j] == '(')
                        {
                            // j++;
                            break;
                        }

                        if (filecontent[j] == 0x20 || filecontent[j] == '\t')
                        {
                            //cut space
                            unsigned int jj = j + 1;
                            for (; jj < filecontent.size(); jj++)
                            {
                                if (filecontent[jj] == 0x20 || filecontent[jj] == '\t')
                                {
                                    j++;
                                    continue;
                                }
                                break;
                            }
                            if (jj < filecontent.size() && filecontent[jj] == '(')
                            {
                                break;
                            }
                            if (!linestr.empty())
                            {
                                if (!func_pre_type.empty()) func_pre_type.push_back(' ');
                                func_pre_type.append(linestr);
                            }
                            linestr.clear();
                            continue;
                        }
                        linestr.push_back(filecontent[j]);
                    }
                }

                i       = j;
                isbegin = false;
                struct reg_autoitem reg_temp;
                reg_temp.pre     = prename;
                reg_temp.urlpath = tempname;
                // 规范化：确保非空 urlpath 以 '/' 开头 — normalize: ensure non-empty urlpath starts with '/'
                if (!reg_temp.urlpath.empty() && reg_temp.urlpath.front() != '/')
                {
                    reg_temp.urlpath.insert(reg_temp.urlpath.begin(), '/');
                }
                //process left right space
                prename.clear();
                for (unsigned int iii = 0; iii < linestr.size(); iii++)
                {
                    if (linestr[iii] == ' ' || linestr[iii] == '\t' || linestr[iii] == 0x0D || linestr[iii] == 0x0A)
                    {
                        continue;
                    }
                    prename.push_back(linestr[iii]);
                }
                reg_temp.func = prename;

                reg_temp.func_type = trim_type_tokens(func_pre_type);
                if (reg_temp.func_type != "std::string" && reg_temp.func_type != "asio::awaitable<std::string>")
                {
                    // 这种签名既写不进 controller/include，也塞不进 reg_raw 的两个参数槽；
                    // 照旧注册只会让生成文件报 undeclared identifier，这里丢掉并当场喊出来
                    std::fprintf(stderr,
                                 "[PRE-WARN] %s: handler %s return type '%s' is not a controller signature, route not registered\n",
                                 methodpathfile.c_str(), reg_temp.func.c_str(), reg_temp.func_type.c_str());
                    continue;
                }
                reg_temp.is_co = (reg_temp.func_type == "asio::awaitable<std::string>");

                // — 也找 pre 函数的返回类型 —
                auto &pname = reg_temp.pre;
                // str_case_equal 而不是 strcasecmp：后者是 POSIX，MSVC 下没有
                bool pre_null = str_case_equal(pname, "null") || str_case_equal(pname, "nullptr")
                                || pname.empty();
                if (!pre_null)
                {
                    auto is_ident_char = [](char c) -> bool {
                        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
                               || c == '_' || c == '$' || c == ':' || c == '.';
                    };
                    // 注释行里的签名长得和声明一模一样，只能看这一行的起头是不是 '/' 或 '*'
                    auto is_comment_head = [](const std::string &text) -> bool {
                        size_t k = 0;
                        while (k < text.size() && (text[k] == ' ' || text[k] == '\t' || text[k] == 0x0D))
                        {
                            ++k;
                        }
                        return k < text.size() && (text[k] == '/' || text[k] == '*');
                    };
                    // 在 filecontent 里搜 "返回类型  prefunc(.*httppeer"
                    size_t pos = filecontent.find(pname);
                    while (pos != std::string::npos)
                    {
                        // 前一位是标识符字符就说明命中的是别的名字（wrapper_case_pre 这类后缀撞车）
                        if (pos > 0 && is_ident_char(filecontent[pos - 1]))
                        {
                            pos = filecontent.find(pname, pos + 1);
                            continue;
                        }
                        size_t line_start = filecontent.rfind('\n', pos) + 1;
                        size_t line_end   = filecontent.find('\n', pos);
                        if (line_end == std::string::npos) line_end = filecontent.size();
                        std::string line = filecontent.substr(line_start, line_end - line_start);
                        // 包含 "httppeer" 才是函数声明
                        if (is_comment_head(line) || line.find("httppeer") == std::string::npos
                            || line.find(pname + "(") == std::string::npos)
                        {
                            pos = filecontent.find(pname, pos + 1);
                            continue;
                        }
                        // 取 '(' 之前、去掉函数名本身的那一段作为返回类型
                        std::string pre_ret;
                        size_t lp = line.find('(');
                        if (lp != std::string::npos)
                        {
                            std::string head = line.substr(0, lp);
                            size_t cut       = head.find_last_of(" \t*&");
                            std::string tail = (cut == std::string::npos) ? head : head.substr(cut + 1);
                            bool   is_name   = (tail == pname);
                            if (!is_name && !pname.empty() && tail.size() > pname.size())
                            {
                                is_name = (tail.compare(tail.size() - pname.size(), pname.size(), pname) == 0);
                            }
                            if (is_name)
                            {
                                head = (cut == std::string::npos) ? std::string() : head.substr(0, cut);
                            }
                            pre_ret = trim_type_tokens(head);
                        }
                        // 返回类型单独占一行的写法：本行只剩函数名，回到上一行找类型
                        if (pre_ret.empty() && line_start > 1)
                        {
                            size_t prev_end   = line_start - 1;
                            size_t prev_start = filecontent.rfind('\n', prev_end - 1) + 1;
                            std::string prev  = filecontent.substr(prev_start, prev_end - prev_start);
                            if (!is_comment_head(prev) && prev.find(';') == std::string::npos
                                && prev.find('{') == std::string::npos && prev.find("httppeer") == std::string::npos)
                            {
                                pre_ret = trim_type_tokens(prev);
                            }
                        }
                        if (!pre_ret.empty())
                        {
                            reg_temp.pre_type  = pre_ret;
                            reg_temp.pre_is_co = (pre_ret == "asio::awaitable<std::string>");
                            break;
                        }
                        pos = filecontent.find(pname, pos + 1);
                    }
                }

                reg_temp.filename = methodpathfile;
                // site 与命名空间不在这里猜路径，由调用方按 split_domain_path 那一条规则填
                temp.emplace_back(reg_temp);
                continue;
            }
        }

        return temp;
    }
    void createhfile(const std::string &filename, const std::vector<struct reg_autoitem> &methodpathfile, const std::string &domain_value, bool isdomain)
    {
        std::string namespace_name;
        if (isdomain)
        {
            namespace_name = domain_namespace(domain_value);
        }

        std::string header_content = R"(
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
)";
        //domain pre
        if (namespace_name.size() > 0)
        {
            header_content.append(" namespace ");
            header_content.append(namespace_name);
            header_content.append(" { \n");
        }

        for (unsigned int i = 0; i < methodpathfile.size(); i++)
        {
            if (methodpathfile[i].func_type == "std::string")
            {
                header_content.append("\tstd::string ");
                header_content.append(methodpathfile[i].func);
                header_content.append("(std::shared_ptr<httppeer> peer);\r\n");
            }
            else if (methodpathfile[i].func_type == "asio::awaitable<std::string>")
            {
                header_content.append("\tasio::awaitable<std::string> ");
                header_content.append(methodpathfile[i].func);
                header_content.append("(std::shared_ptr<httppeer> peer);\r\n");
            }
        }

        if (namespace_name.size() > 0)
        {
            header_content.append(" }\n");
        }

        header_content.append("}\r\n");

        write_file_if_changed(filename, header_content);
    }
    // public:
    //     std::map<std::string, time_t> fileslist;
    //     std::map<std::string, time_t> sofileslist;
};
}// namespace http
#endif// PROJECT_AUTOPICKMEMTHOD_HPP
