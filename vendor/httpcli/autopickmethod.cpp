/*
 *  @author 黄自权 huangziquan
 *  @date 2023-01-06
 *  @dest controller method pick file
 */
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <filesystem>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#define stat _stat
#endif

#include "autopickcontrolmethod.hpp"
#include "md5.h"

namespace fs = std::filesystem;
bool stringcasecmp(std::string_view str1, std::string_view str2)
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
std::string get_filename(const std::string &filename)
{
    std::string filename_name;
    int j = filename.size() - 1;
    if (j == -1)
        return "";
    for (; j >= 0; j--)
    {
        if (filename[j] == '.')
        {
            if (filename_name.size() == 2 && filename_name[0] == 'z' && filename_name[1] == 'g')
            {
                continue;
            }
            filename_name.clear();
            continue;
        }
        if (filename[j] == '/')
        {
            j--;
            break;
        }
        filename_name.push_back(filename[j]);
    }
    std::reverse(filename_name.begin(), filename_name.end());
    return filename_name;
}

void make_all_directory(const std::string &path, const std::string &pathname)
{
    std::string path_temp = path;

    unsigned int path_size = pathname.size();

    for (unsigned int j = 0; j < path_size; j++)
    {
        if (pathname[j] == '/' && path_temp.size() > 0)
        {
            fs::path paths = path_temp;
            if (!fs::exists(paths))
            {
                fs::create_directories(paths);
                fs::permissions(paths,
                                fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                                fs::perm_options::add);
            }
        }
        path_temp.push_back(pathname[j]);
    }
}

int main(int argc, char *argv[])
{

    std::string current_run_path    = "";
    std::string current_method_file = "";
    fs::path current_path;
    if (argc > 1)
    {
        current_run_path.append(argv[1]);
    }
    else
    {
        current_path = fs::current_path();
        current_run_path.append(current_path.string());
    }

    if (current_run_path.back() != '/')
    {
        current_run_path.push_back('/');
    }

    http::pickcontrol lp;
    std::vector<std::string> header_lists;
    std::vector<struct http::reg_autoitem> reg_method_lists;
    std::vector<struct http::reg_autoitem> domain_method_lists;

    std::vector<struct http::reg_autoitem> co_reg_method_lists;
    std::vector<struct http::reg_autoitem> co_domain_method_lists;

    // 域目录 → 该域里声明过的函数名集合。
    // pre 的查找顺序是「同域 → 全局」：
    //   同域（本域目录里声明出来的名字）→ 加本域的命名空间前缀（shop::pre）；
    //   其余一律按全局原样引用（pre），全局文件里的 pre 就是这么接上的；
    //   跨域引用（别的域目录里的 pre）不在这里兜 —— 落回全局写法，编不过就编不过，
    //   由业务代码自己看到那个 undeclared identifier，比生成器悄悄猜一个前缀好定位。
    std::map<std::string, std::set<std::string>> domain_funcs;
    // 命名空间 → 第一个占用它的域目录，用来喊出"两个域目录撞进同一个命名空间"
    std::map<std::string, std::string> ns_owner;
    // 同一对"命名空间 + 后到的域"只喊一次（一个域目录下可能有多个文件，循环会重复进来）
    std::map<std::string, bool> ns_warned;
    // 全仓索引：注解函数名 → 是否协程。第一遍扫描时填，第二遍用它定 pre 的槽位
    std::map<std::string, bool> sig_is_co;
    // 本轮还有注解的文件（缓存键 = 去扩展名的相对路径），用来清理缓存里的残留条目
    std::set<std::string> seen_files;

    std::string src_path = current_run_path + "controller/src";

    fs::path vsrcpath = src_path;

    if (!fs::exists(vsrcpath))
    {
        std::cout << " controller directory not in current path " << std::endl;
        return 0;
    }

    fs::path cache_path = current_run_path + "cache";
    if (!fs::exists(cache_path))
    {
        fs::create_directories(cache_path);
        fs::permissions(cache_path,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }
    // md5 hash => filetime

    auto plist = lp.listpath(src_path);

    std::string cache_file_path = current_run_path + "cache/method_notes.data";

    std::map<std::string, struct http::file_regitem> reginfo_list = lp.loadcacheinfo(cache_file_path);

    struct stat sessfileinfo;
    std::string md5hash;
    std::string hash_value;
    std::string mttime_value;
    std::string domain_value;
    struct http::file_regitem cahce_info;
    bool is_must_again = false;
    for (unsigned int i = 0; i < plist.size(); i++)
    {
        std::string filename;// = src_path;// get_filename(plist[i]);

        bool is_file_domain = http::split_domain_path(plist[i], domain_value);

        // 命名空间推导是"取第一个 '.' 之前的子串"，所以 a.example.com 与 a.other.com 会撞成
        // 同一个命名空间 a。撞车不会在生成期报错，只会在两个域目录出现同名函数时炸在编译期，
        // 所以这里提前喊一次（按"命名空间 + 后到的域"记一对，同一个组合只喊一次）
        if (is_file_domain)
        {
            std::string nsname = http::domain_namespace(domain_value);
            auto owner         = ns_owner.emplace(nsname, domain_value);
            if (!owner.second && owner.first->second != domain_value)
            {
                std::string pairkey = nsname + "|" + domain_value;
                if (ns_warned.emplace(pairkey, true).second)
                {
                    std::fprintf(stderr,
                                 "[ROUTE-WARN] domain dirs '%s' and '%s' both map to namespace '%s' "
                                 "(namespace = the part before the first '.'); same-named handlers "
                                 "in them will collide at compile time\n",
                                 owner.first->second.c_str(), domain_value.c_str(), nsname.c_str());
                }
            }
        }

        filename = src_path;
        if (filename.back() != '/')
        {
            filename.push_back('/');
        }
        filename.append(plist[i]);
        filename.append(".cpp");
        auto method_item = lp.pickfile(filename);

        if (method_item.size() > 0)
        {
            seen_files.insert(plist[i]);
            md5hash.clear();
            mttime_value.clear();
            for (unsigned int j = 0; j < method_item.size(); j++)
            {
                md5hash.append(method_item[j].pre);
                md5hash.append(method_item[j].func);
                md5hash.append(method_item[j].urlpath);
                // 返回类型必须进 hash：.h 里声明的就是 "<func_type> <func>(...)"，
                // 只把 sync 改成 coro（注解文本一字不动）时，漏了它 .h 就不会重建，
                // 于是 .h 还声明着 std::string，而生成表已经把它放进 co 槽位 —— 编译期才炸。
                md5hash.append(method_item[j].func_type);
                // 第一遍：把本文件注解函数的返回类型登记进全仓索引。
                // pre 常常定义在别的文件里（admin_islogin 这类共用前置），只在本文件里嗅探
                // 签名会漏，漏了就默认按 sync 处理。先全扫一遍，第二遍再按这个索引定夺。
                sig_is_co.emplace(method_item[j].func, method_item[j].is_co);
            }
            hash_value = http::md5(md5hash);

            filename.clear();
            filename.append(current_run_path);
            filename.append("controller/include/");
            // create custom directory
            make_all_directory(filename, plist[i]);

            filename.append(plist[i]);

            std::string fileinh = filename + ".h";
            filename            = plist[i];

            memset(&sessfileinfo, 0, sizeof(sessfileinfo));
            if (stat(fileinh.c_str(), &sessfileinfo) == 0)
            {
                if (sessfileinfo.st_mode & S_IFREG)
                {
                    mttime_value = std::to_string(sessfileinfo.st_mtime);
                }
            }
            cahce_info = reginfo_list[filename];

            if (cahce_info.filehash.size() > 0 && cahce_info.filehash == hash_value && mttime_value.size() > 0 &&
                mttime_value == cahce_info.filetime)
            {
            }
            else
            {

                lp.createhfile(fileinh, method_item, domain_value, is_file_domain);

                memset(&sessfileinfo, 0, sizeof(sessfileinfo));
                if (stat(fileinh.c_str(), &sessfileinfo) == 0)
                {
                    if (sessfileinfo.st_mode & S_IFREG)
                    {
                        mttime_value = std::to_string(sessfileinfo.st_mtime);
                    }
                }

                cahce_info.filename    = filename;
                cahce_info.filetime    = mttime_value;
                cahce_info.filehash    = hash_value;
                reginfo_list[filename] = cahce_info;
                is_must_again          = true;
            }

            for (unsigned int j = 0; j < method_item.size(); j++)
            {
                if (is_file_domain)
                {
                    // 本域声明出来的名字：给 pre 的「同域优先」判断留底（见 gen_item 的 qualify_pre）
                    domain_funcs[domain_value].insert(method_item[j].func);
                    if (method_item[j].is_co)
                    {
                        method_item[j].domain = domain_value;
                        co_domain_method_lists.emplace_back(method_item[j]);
                    }
                    else
                    {
                        method_item[j].domain = domain_value;
                        domain_method_lists.emplace_back(method_item[j]);
                    }
                }
                else
                {
                    if (method_item[j].is_co)
                    {
                        co_reg_method_lists.emplace_back(method_item[j]);
                    }
                    else
                    {
                        reg_method_lists.emplace_back(method_item[j]);
                    }
                }
            }
            filename.append(".h\"\r\n");
            header_lists.emplace_back(filename);
        }
    }

    // 缓存只留这一轮还有注解的文件：删除（或注解被全部摘掉）的 controller 文件如果不清，
    // cache/method_notes.data 会一直变大；更麻烦的是那条过期记录会在同名文件回来时被当成
    // "没变过"而跳过 .h 重建。
    for (auto it = reginfo_list.begin(); it != reginfo_list.end();)
    {
        if (seen_files.count(it->first) == 0)
        {
            it = reginfo_list.erase(it);
        }
        else
        {
            ++it;
        }
    }

    lp.savecacheinfo(cache_file_path, reginfo_list);

    // — 第二遍：pre 的返回类型按全仓索引补齐 —
    // pickfile 只在本文件里嗅探 pre 的签名，跨文件的（pre 定义在另一个 controller 里）
    // 一律落空、被当成 sync。这里用第一遍建好的索引定夺：
    //   索引里有 → 按索引给 pre_type / pre_is_co，协程 pre 就能正确落进 co_pre 槽位；
    //   索引里也没有 → 说明这个 pre 既不在本文件、也不是任何注解函数（可能是个没注解的
    //                  普通函数），生成器无从判断，按 sync 假设并当场喊出来，免得
    //                  "协程 pre 被塞进 pre_sync 槽位"变成一个看不懂的编译错误。
    auto resolve_pre_type = [&sig_is_co](std::vector<struct http::reg_autoitem> &lists) {
        for (auto &item : lists)
        {
            if (item.pre.empty() || stringcasecmp(item.pre, "null") || stringcasecmp(item.pre, "nullptr"))
            {
                continue;
            }
            if (!item.pre_type.empty()) continue;   // 本文件里已经判定了
            // 带 "::" 的写法取最后一段去查（vip::vipauth → vipauth）
            std::string lookup = item.pre;
            auto sep           = lookup.rfind("::");
            if (sep != std::string::npos) lookup = lookup.substr(sep + 2);

            auto it = sig_is_co.find(lookup);
            if (it == sig_is_co.end())
            {
                std::fprintf(stderr,
                             "[PRE-WARN] %s: handler %s pre '%s' signature not found in any controller "
                             "file, assumed sync; if it is a coroutine the generated reg_raw will not compile\n",
                             item.filename.c_str(), item.func.c_str(), item.pre.c_str());
                continue;
            }
            item.pre_is_co = it->second;
            item.pre_type  = it->second ? "asio::awaitable<std::string>" : "std::string";
        }
    };
    resolve_pre_type(reg_method_lists);
    resolve_pre_type(co_reg_method_lists);
    resolve_pre_type(domain_method_lists);
    resolve_pre_type(co_domain_method_lists);

    current_method_file = current_run_path + "common/autocontrolmethod.hpp";

    std::string automethod_content;

    automethod_content = R"(
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

)";
    //all .h files
    for (unsigned int j = 0; j < header_lists.size(); j++)
    {
        automethod_content.append("#include \"");
        automethod_content.append(header_lists[j]);
    }
    automethod_content.append("\nnamespace http\n{\n");

    // — v6 router: generate ONE function _initauto_all_httputils() —
    // mid_param=true 表示「参数段后面又跟了固定段」（如 user/:id/edit）。这种写法注册出来的
    // 名字是拼掉参数段后的 "/user/edit"，而查表用的前缀表是按真实 URL 段拼的
    // （"/user/7/edit"），两边永远对不上 ⇒ 运行期必定 404。生成期就把它喊出来（见 gen_item）
    auto parse_urlpath = [](const std::string &raw, std::string &out_path,
                             std::vector<std::string> &out_names, bool &mid_param) {
        out_names.clear();
        mid_param  = false;
        bool has_param  = false;
        bool seen_param = false;  // 已经吐出过至少一个参数段
        unsigned int n = (raw.size() > 0 && raw[0] == '/') ? 1 : 0;
        bool prev_fixed = true;
        std::string seg, fixed_path;
        for (; n < raw.size(); n++) {
            if (raw[n] == '"' || raw[n] == '\\' || raw[n] == '\'') continue;
            if (raw[n] == ':') { has_param = true; prev_fixed = false; continue; }
            else if (raw[n] == '/') {
                if (prev_fixed) {
                    if (seen_param) mid_param = true;
                    if (!fixed_path.empty()) fixed_path.push_back('/');
                    fixed_path.append(seg);
                }
                else {
                    seen_param = true;
                }
                out_names.push_back(prev_fixed ? "" : seg);
                seg.clear(); prev_fixed = true;
                continue;
            }
            seg.push_back(raw[n]);
        }
        if (!seg.empty()) out_names.push_back(prev_fixed ? "" : seg);
        if (prev_fixed) {
            if (seen_param) mid_param = true;
            if (!fixed_path.empty()) fixed_path.push_back('/');
            fixed_path.append(seg);
        }
        out_path = has_param ? fixed_path : raw;
        // 二次规范化：确保解析后路径以 '/' 开头 — double-check: ensure parsed path starts with '/'
        if (!out_path.empty() && out_path.front() != '/')
        {
            out_path.insert(out_path.begin(), '/');
        }
        if (!has_param) out_names.clear();
    };

    automethod_content.append(
"\n    void _initauto_all_httputils()\n    {\n"
    );

    // 本轮生成出来的注册名，用来和手写注册表（common/reghttpmethod*.hpp）比对撞名
    std::set<std::string> generated_paths;

    auto gen_item = [&](const std::string &site, const http::reg_autoitem &item) {
        std::string parsed_path;
        std::vector<std::string> urlpath_names;
        bool mid_param = false;
        parse_urlpath(item.urlpath, parsed_path, urlpath_names, mid_param);

        if (mid_param)
        {
            // 路由照样注册（行为不变），只在生成期喊一声：这条注解运行期打不到
            std::fprintf(stderr,
                         "[ROUTE-WARN] %s: handler %s urlpath '%s' has a fixed segment after ':param', "
                         "it registers as '%s' and can never match (params must be trailing), route will 404\n",
                         item.filename.c_str(), item.func.c_str(), item.urlpath.c_str(), parsed_path.c_str());
        }

        // handler 一定在自己那条注册所在的域里，所以照旧按 item.domain 加前缀
        auto qualify = [&](const std::string &name) -> std::string {
            if (name.empty() || stringcasecmp(name, "null") || stringcasecmp(name, "nullptr"))
                return "nullptr";
            if (!item.domain.empty())
                return http::domain_namespace(item.domain) + "::" + name;
            return name;
        };

        // pre 的写法三种，按这个顺序判：
        //   1. 带 "::" —— 业务已经把限定写全了（vip::vipauth、shop::shopauth），原样输出，
        //      生成器不再插手；
        //   2. 不带 "::" 且本域目录里声明过这个名字 —— 自动补本域前缀（loginis → shop::loginis）；
        //   3. 不带 "::" 且本域目录里没有 —— 按全局处理，原样输出裸名。
        //      注意别写成 http::xxx：本文件生成的 _initauto_all_httputils() 本身就在
        //      namespace http 里，全局函数裸名即可查到，加 http:: 既多余也容易让人误以为
        //      还有一层外层命名空间。
        //      跨域引用就落在这条上（别的域目录里的 pre 不会被自动加前缀）：编不过就编不过，
        //      让业务代码自己看到那个 undeclared identifier，比生成器悄悄猜一个别域前缀好定位；
        //      真要跨域就按第 1 条把 "::" 写全。
        auto qualify_pre = [&](const std::string &name) -> std::string {
            if (name.empty() || stringcasecmp(name, "null") || stringcasecmp(name, "nullptr"))
                return "nullptr";
            if (name.find("::") != std::string::npos) return name;   // 1. 写全了 → 原样
            if (!item.domain.empty())
            {
                auto it = domain_funcs.find(item.domain);
                if (it != domain_funcs.end() && it->second.count(name) > 0)
                {
                    return http::domain_namespace(item.domain) + "::" + name;   // 2. 本域
                }
            }
            return name;   // 3. 全局
        };

        std::string q_pre = qualify_pre(item.pre);
        std::string q_reg = qualify(item.func);

        // 根据 pre_is_co / is_co 决定四个参数的组合
        std::string pre_sync = "nullptr", co_pre = "nullptr";
        if (!item.pre.empty() && !stringcasecmp(item.pre, "null") && !stringcasecmp(item.pre, "nullptr")) {
            if (item.pre_is_co) co_pre = q_pre;
            else                pre_sync = q_pre;
        }

        std::string reg_sync = "nullptr", co_reg = "nullptr";
        if (item.is_co)  co_reg = q_reg;
        else             reg_sync = q_reg;

        generated_paths.insert(parsed_path);

        automethod_content += "        reg_raw(\"" + site + "\", \"" + parsed_path + "\", "
                            + pre_sync + ", " + co_pre + ", "
                            + reg_sync + ", " + co_reg + ");\r\n";

        if (!urlpath_names.empty()) {
            automethod_content += "        reg_urlpath(\"" + site + "\", \""
                                + parsed_path + "\", {";
            for (unsigned int k = 0; k < urlpath_names.size(); k++) {
                if (k) automethod_content += ", ";
                automethod_content += "\"" + urlpath_names[k] + "\"";
            }
            automethod_content += "});\r\n";
        }
    };

    for (auto const &item : reg_method_lists)       gen_item("", item);
    for (auto const &item : co_reg_method_lists)    gen_item("", item);
    for (auto const &item : domain_method_lists)    gen_item(item.domain, item);
    for (auto const &item : co_domain_method_lists) gen_item(item.domain, item);

    // — 手写注册表撞名提示 —
    // common/reghttpmethod.hpp / _pre.hpp 里的手写注册跑在 _initauto_all_httputils() 之后
    // （server.cpp:8912-8913），撞名时 reg_add 保留首次、也就是生成表，手写那条整条失效。
    // 编译期完全看不出来，请求期才发现"我明明注册了却不生效"，所以在生成期把撞名的喊出来。
    auto warn_hand_reg_dup = [&generated_paths](const std::string &file) {
        std::string content;
        std::FILE *f = std::fopen(file.c_str(), "rb");
        if (f == nullptr) return;
        std::fseek(f, 0, SEEK_END);
        long n = std::ftell(f);
        std::fseek(f, 0, SEEK_SET);
        if (n > 0)
        {
            content.resize(n);
            long nread = (long)std::fread(&content[0], 1, n, f);
            content.resize(nread < 0 ? 0 : (size_t)nread);
        }
        std::fclose(f);

        // 取 "REG_*(" / "reg_raw(" 之后的第 2 个带引号实参 —— 第 1 个是 site，第 2 个是 path
        size_t pos = 0;
        for (;;)
        {
            size_t macro = content.find("REG_", pos);
            size_t raw   = content.find("reg_raw(", pos);
            size_t at    = std::min(macro, raw);
            if (at == std::string::npos) break;
            pos = at + 4;

            // 返回结束位置，下一段从这里接着找 —— 否则会落到两个实参之间的 ", " 上
            auto quoted = [&](size_t from, std::string &out, size_t &endp) -> bool {
                size_t a = content.find('"', from);
                if (a == std::string::npos) return false;
                size_t b = content.find('"', a + 1);
                if (b == std::string::npos) return false;
                out  = content.substr(a + 1, b - a - 1);
                endp = b + 1;
                return true;
            };
            std::string site, path;
            size_t endp = 0;
            if (!quoted(at, site, endp)) continue;
            if (!quoted(endp, path, endp)) continue;

            // 手写表里 path 可能不带前导 '/'，两边都剥掉再比
            std::string key = path.empty() ? path : (path[0] == '/' ? path.substr(1) : path);
            for (auto const &gp : generated_paths)
            {
                std::string gkey = gp.empty() ? gp : (gp[0] == '/' ? gp.substr(1) : gp);
                if (!key.empty() && key == gkey)
                {
                    std::fprintf(stderr,
                                 "[ROUTE-WARN] %s: hand-written '%s' also generated from an @urlpath "
                                 "annotation; reg_add keeps the first, so this hand-written entry is dead\n",
                                 file.c_str(), path.c_str());
                    break;
                }
            }
        }
    };
    warn_hand_reg_dup(current_run_path + "common/reghttpmethod.hpp");
    warn_hand_reg_dup(current_run_path + "common/reghttpmethod_pre.hpp");

    automethod_content +=
"\n    }\n\n}\n\n#endif\n";

    http::write_file_if_changed(current_method_file, automethod_content);
    return 0;
}
