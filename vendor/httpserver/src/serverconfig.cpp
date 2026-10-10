#include <string>
#include <map>
#include <memory>
#include <climits>
#include <algorithm>
#include <cctype>
#include "serverconfig.h"
#include "server_localvar.h"
#include <cstring>
#include "httppeer.h"
#include "func.h"
#include "terminal_color.h"
#ifdef ENABLE_WEBPAY
#include "webpay_config.h"
#endif

namespace http
{
namespace fs = std::filesystem;

// usehtmlcache 的开关读法：空或 0 打头算关，其余非空值算开（写 "0" 想表达"关"才判得出关）
static bool htmlcache_switch_on(const std::string &val)
{
    return !val.empty() && val[0] != '0';
}
// usehtmlcachetime 的取值读法：跳过非数字，和 upload_max_size / siteid 那几处的数字扫描同形
static unsigned int htmlcache_seconds(const std::string &val)
{
    unsigned int temp = 0;
    for (size_t i = 0; i < val.size(); i++)
    {
        if (val[i] > 0x2F && val[i] < 0x3A)
        {
            temp = temp * 10 + (val[i] - '0');
        }
    }
    return temp;
}

// conf 里的钩子名（static_pre 的名字那一段、method_pre / method_after 名单）不规定写法，
// 带不带开头的 '/' 都行；注册表的键一律带 '/'，解析期补齐，让日志和 is_static_pre 认规范名。
// 只能用在注册名上：static_pre_lists 是 URL 前缀、rewrite_404_action / action_404_lists 是
// 拼进文件路径的片段，那些走这里会把功能改坏
static std::string conf_hook_name(const std::string &name)
{
    if (name.empty() || name[0] == '/')
        return name;
    return "/" + name;
}

serverconfig &getserversysconfig()
{
    static serverconfig instance;
    return instance;
}
std::map<std::string, std::map<std::string, std::string>> loadserversconfig(std::string filename)
{
    std::map<std::string, std::map<std::string, std::string>> sys_config;
    std::map<std::string, std::string> itemconfig;
    // FILE *f = fopen(filename.c_str(), "rb");
    std::unique_ptr<std::FILE, int (*)(FILE *)> f(fopen(filename.c_str(), "rb"), std::fclose);
    if (f == nullptr)
    {
        return sys_config;
    }
    fseek(f.get(), 0, SEEK_END);
    long fsize = ftell(f.get());
    fseek(f.get(), 0, SEEK_SET);

    if (fsize < 0)
    {
        return sys_config;
    }
    auto const size = static_cast<size_t>(fsize);

    std::string s, linestr, keyname, strval;
    s.resize(size);

    auto nread = fread(&s[0], 1, size, f.get());
    s.resize(nread);
    // fclose(f);

    bool readkey = false;
    bool isvalue = false;
    keyname      = "";

    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == ';')
        {
            i++;
            if (linestr.size() > 0)
            {
                itemconfig[linestr] = strval;
            }

            linestr.clear();
            strval.clear();
            isvalue = false;
            readkey = false;
            for (; i < s.size(); i++)
            {
                if (s[i] == 0x0A)
                {
                    break;
                }
            }
        }
        if (i < s.size() && s[i] == 0x0A)
        {
            readkey = false;

            if (linestr.size() > 0)
            {
                itemconfig[linestr] = strval;
            }
            linestr.clear();
            strval.clear();
            isvalue = false;
            continue;
        }
        if (i < s.size() && s[i] == '[')
        {
            if (keyname.size() > 0)
            {

                sys_config[keyname] = itemconfig;
                itemconfig.clear();
            }
            keyname.clear();
            readkey = true;
            continue;
        }
        if (s[i] == ']')
        {
            readkey = false;
            continue;
        }
        if (s[i] == 0x20)
        {

            continue;
        }
        if (s[i] == '\t')
        {

            continue;
        }
        if (s[i] == '"')
        {

            continue;
        }
        if (s[i] == '=')
        {
            isvalue = true;
            continue;
        }
        if (readkey)
        {
            keyname.push_back(s[i]);
        }
        else
        {
            if (isvalue)
            {
                strval.push_back(s[i]);
            }
            else
            {
                linestr.push_back(s[i]);
            }
        }
    }
    if (linestr.size() > 0)
    {
        itemconfig[linestr] = strval;
    }
    if (itemconfig.size() > 0)
    {

        sys_config[keyname] = itemconfig;
    }
    return sys_config;
}

namespace
{
// 解析 cors_domain 配置项，把配置串"消化"成请求期可直接哈希查表的白名单。
// 语法（loadserversconfig 读取阶段已剥离空格/Tab/引号，故分隔符只能是逗号）：
//   cors_domain 不写 / 写空 / 单个非 "*" 字符   未配置 → 默认拒绝：该站点不发任何 CORS 头
//   cors_domain = *                            显式放开全部（回显 "*"）
//   cors_domain = https://www.hggq.com         已写全 origin，原样入表（自定义端口请写全）
//   cors_domain = www.hggq.com, www.hggq.net   只写域名，自动补 http:// 与 https:// 两种 scheme
// 站点没写这一项时继承 [default] 解析好的白名单；站点写了就整表覆盖自己的，
// 包括写成空串——那等于显式本站不放开任何跨域，是站点退出继承的口子。
// 放开全部不是缺省值："什么都没配"时不发 ACAO，只有显式写 "*" 才放开全部来源；
// 要多站点同时放开就得每处显式写 "*"，加载期对"没配"的站点逐个告警。
// 每一项入表前统一归一：转小写、去尾斜杠、去掉与 scheme 匹配的默认端口（:80 / :443）；
// 归一后仍带路径的属于写错（Origin 里没有路径这一段），加载期告警但仍然入表。
void cors_domain_parse(const std::string &src, site_host_info_t &info)
{
    info.cors_domain_list.clear();
    info.cors_origin_allowed.clear();
    // is_cors 表示用户是否显式配置了 cors_domain（哪怕只写了一个 "*"）：
    //   未配置（空串）      → false，默认拒绝，加载期告警提示这一站点
    //   已配置（"*" 或域名） → true
    info.is_cors = (src.size() > 0);
    // 少于 2 个字符时唯一有意义的取值是单个 "*"：显式放开全部。
    // 其余（含空串）一律按未配置处理 = 拒绝，不兜底成放开
    if (src.size() < 2)
    {
        info.cors_allow_all = (src.size() == 1 && src[0] == '*');
        return;
    }
    info.cors_allow_all = false;

    std::size_t pos = 0;
    while (pos < src.size())
    {
        std::size_t comma = src.find(',', pos);
        if (comma == std::string::npos)
        {
            comma = src.size();
        }
        if (comma > pos)
        {
            std::string item = src.substr(pos, comma - pos);
            // origin 的 scheme 与 host 比较是大小写不敏感的，统一转小写入表
            std::transform(item.begin(), item.end(), item.begin(), [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            // 浏览器送出的 Origin 恒为 scheme://host[:port]：不带尾斜杠，默认端口也不写出来
            // （访问 http://host:80 时 Origin 是 http://host）。配置里多写了这些的归一到可比对的
            // 形状，否则这一条永远匹配不上；空格与引号 ini 读取器已经剥掉，这里不用 trim。
            while (!item.empty() && item.back() == '/')
            {
                item.pop_back();
            }
            if (std::string_view(item).starts_with("http://") && std::string_view(item).ends_with(":80"))
            {
                item.erase(item.size() - 3);
            }
            else if (std::string_view(item).starts_with("https://") && std::string_view(item).ends_with(":443"))
            {
                item.erase(item.size() - 4);
            }
            // 归一后仍带 '/' 的只可能是写了路径（scheme 分隔符除外），Origin 里没有路径这一段，
            // 这一条谁都匹配不上，配置期就报出来，别留一个静默的白名单空洞
            {
                std::size_t body = item.find("://");
                body             = (body == std::string::npos) ? 0 : body + 3;
                if (item.find('/', body) != std::string::npos)
                {
                    fprintf(stderr, "[CORS-WARN] cors_domain item '%s' is not an origin, it can never match\n", item.c_str());
                    fflush(stderr);
                }
            }
            if (item == "*")
            {
                info.cors_allow_all = true;
            }
            else if (item.find("://") != std::string::npos)
            {
                info.cors_origin_allowed.insert(item);
            }
            else
            {
                info.cors_origin_allowed.insert("http://" + item);
                info.cors_origin_allowed.insert("https://" + item);
            }
            info.cors_domain_list.emplace_back(std::move(item));
        }
        pos = comma + 1;
    }
}

// 解析 cors_allow_methods：逗号分隔，逐条转大写（Allow-Methods 按方法的大写规范出，
// 而校验 Access-Control-Request-Method 本身大小写不敏感），空 token 丢弃。
// 空格不用管：ini 读取器已按行剥掉。站点写了就是整表覆盖，写空等于"本站点预检不放行任何方法"。
void cors_methods_parse(const std::string &src, std::vector<std::string> &out)
{
    out.clear();
    std::size_t pos = 0;
    while (pos < src.size())
    {
        std::size_t comma = src.find(',', pos);
        if (comma == std::string::npos)
        {
            comma = src.size();
        }
        if (comma > pos)
        {
            std::string item = src.substr(pos, comma - pos);
            std::transform(item.begin(), item.end(), item.begin(), [](unsigned char c)
                           { return static_cast<char>(std::toupper(c)); });
            out.emplace_back(std::move(item));
        }
        pos = comma + 1;
    }
}
}// namespace

std::string site_host_info_t::cors_allow_origin(std::string_view request_origin) const
{
    // 只有显式写了 cors_domain = * 才会走到这一跳返回 "*"；未配置的站点 cors_allow_all 为 false，
    // 落到下面的查表分支，而它的白名单是空集 ⇒ 任何 Origin 都不放行（默认拒绝）
    if (cors_allow_all)
    {
        return "*";
    }
    if (request_origin.empty())
    {
        return "";
    }
    std::string origin;
    origin.resize(request_origin.size());
    std::transform(request_origin.begin(), request_origin.end(), origin.begin(), [](unsigned char c)
                   { return static_cast<char>(std::tolower(c)); });
    if (cors_origin_allowed.find(origin) == cors_origin_allowed.end())
    {
        return "";
    }
    // 命中：必须回显请求里的 Origin 原值——配置里往往只写了域名，
    // 与浏览器发来的完整 origin 不逐字节相等，回填配置值浏览器会判为不匹配。
    return std::string(request_origin);
}

std::tuple<unsigned int, std::string> serverconfig::gethost404(const std::string &host)
{
    std::string action_method;
    unsigned int rewritetype = 100;

    if (host_toint.find(host) != host_toint.end())
    {
        unsigned int temp_index = host_toint[host];
        if (temp_index < sitehostinfos.size())
        {
            rewritetype   = sitehostinfos[temp_index].rewrite404;
            action_method = sitehostinfos[temp_index].rewrite_404_action;
        }
        if (rewritetype != 100)
        {
            return std::make_tuple(rewritetype, action_method);
        }
    }

    if (map_value.find(host) != map_value.end())
    {
        if (map_value[host].find("rewrite_404") != map_value[host].end())
        {
            try
            {
                rewritetype = std::stoul(map_value[host]["rewrite_404"].c_str());
                if (map_value[host].find("rewrite_404_action") != map_value[host].end())
                {
                    action_method = map_value[host]["rewrite_404_action"];
                }
            }
            catch (const std::exception &e)
            {
                rewritetype = 0;
            }
        }
    }

    if (rewritetype == 100)
    {
        rewritetype = 0;
        if (map_value.find("default") != map_value.end())
        {
            if (map_value["default"].find("rewrite_404") != map_value["default"].end())
            {
                try
                {
                    rewritetype = std::stoul(map_value["default"]["rewrite_404"].c_str());
                    if (map_value["default"].find("rewrite_404_action") != map_value["default"].end())
                    {
                        action_method = map_value["default"]["rewrite_404_action"];
                    }
                }
                catch (const std::exception &e)
                {
                    rewritetype = 0;
                }
            }
        }
    }
    return std::make_tuple(rewritetype, action_method);
}
std::string serverconfig::getsitewwwpath(unsigned int host_index)
{
    if (host_index > 0 && host_index < sitehostinfos.size())
    {
        return sitehostinfos[host_index].wwwpath;
    }
    return sitehostinfos[0].wwwpath;
}
unsigned int serverconfig::get_hostindex(const std::string &host)
{
    auto iter = host_toint.find(host);
    if (iter != host_toint.end())
    {
        unsigned int host_index = iter->second;
        if (host_index > 0 && host_index < sitehostinfos.size())
        {
            return host_index;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}
std::string serverconfig::getsitepath(const std::string &host)
{
    clear_ctx = false;
    std::string sitepath;
    bool isnothost = true;

    if (host_toint.find(host) != host_toint.end())
    {
        unsigned int host_index = 0;
        host_index              = host_toint[host];
        if (host_index > 0 && host_index < sitehostinfos.size())
        {
            return sitehostinfos[host_index].wwwpath;
        }
        else
        {
            return sitehostinfos[0].wwwpath;
        }
    }

    if (map_value.find(host) != map_value.end())
    {

        if (map_value[host]["wwwpath"].empty())
        {
            sitepath = map_value["default"]["wwwpath"];
            if (sitepath.empty())
            {
                sitepath = "www/";
            }
            if (sitepath.back() != '/')
            {
                sitepath.push_back('/');
            }
        }
        else
        {
            sitepath = map_value[host]["wwwpath"];
        }
        isnothost = false;
    }
    else if (map_value.find("default") != map_value.end())
    {
        sitepath = map_value["default"]["wwwpath"];
        if (sitepath.empty())
        {
            sitepath = "www/";
        }
        isnothost = false;
    }
    else
    {
        sitepath = "www/";
    }

    if (sitepath.back() != '/')
    {
        sitepath.push_back('/');
    }

    if (isnothost)
    {
        // notfind host path
        if (host.size() > 3 && host[0] != 'w' && host[1] != 'w' && host[2] != 'w' && host[3] != '.')
        {
            std::string dddmod = "*.";
            bool isplithost    = true;
            std::string splithostpre;
            for (size_t i = 0; i < host.size(); i++)
            {
                if (isplithost && host[i] == '.')
                {
                    isplithost = false;
                    continue;
                }
                if (isplithost)
                {
                    splithostpre.push_back(host[i]);
                }
                else
                {
                    dddmod.push_back(host[i]);
                }
            }
            if (map_value.find(dddmod) != map_value.end())
            {
                if (map_value[dddmod]["aliaspath"].empty())
                {
                    sitepath.append(dddmod.substr(2));
                    if (sitepath.back() != '/')
                    {
                        sitepath.push_back('/');
                    }
                    sitepath.append(splithostpre);
                }
                else
                {
                    sitepath = map_value[dddmod]["aliaspath"];
                    if (sitepath.back() != '/')
                    {
                        sitepath.push_back('/');
                    }
                    sitepath.append(splithostpre);
                }
            }
            else
            {
                sitepath.append(host);
            }
        }
        else
        {
            sitepath.append(host);
        }
    }
    if (sitepath.back() != '/')
    {
        sitepath.push_back('/');
    }
    return sitepath;
}
void serverconfig::init_path()
{
    if (configfile.empty())
    {
        std::string currentpath;
        fs::path cpath = fs::current_path();

        currentpath = cpath.string();
        currentpath = currentpath + "/conf";

        if (fs::is_directory(cpath))
        {
            currentpath = currentpath + "/server.conf";
            cpath       = currentpath;
            if (fs::is_regular_file(cpath))
            {
                configfile = currentpath;
            }
        }
        if (configfile.empty())
        {
            currentpath = "/usr/local/etc/paozhu";
            cpath       = currentpath;
            if (fs::is_directory(cpath))
            {
                currentpath = currentpath + "/server.conf";
                cpath       = currentpath;
                if (fs::is_regular_file(cpath))
                {
                    configfile = currentpath;
                }
                else
                {
                    configfile.clear();
                }
            }
        }
        if (configfile.empty())
        {
            currentpath = "/etc/paozhu";
            currentpath = currentpath + "/conf";
            cpath       = currentpath;
            if (fs::is_directory(cpath))
            {

                currentpath = currentpath + "/server.conf";
                cpath       = currentpath;
                if (fs::is_regular_file(cpath))
                {
                    configfile = currentpath;
                }
                else
                {
                    configfile.clear();
                }
            }
        }
    }
    else
    {
        fs::path cpath = configfile;

        if (fs::is_directory(cpath))
        {
            configfile = configfile + "/conf/server.conf";
            cpath      = configfile;
            if (fs::is_regular_file(cpath))
            {
            }
            else
            {
                configfile.clear();
            }
        }
        if (fs::is_directory(cpath))
        {
            configfile = configfile + "/server.conf";
            cpath      = configfile;
            if (fs::is_regular_file(cpath))
            {
            }
            else
            {
                configfile.clear();
            }
        }
    }
    if (configfile.size() > 0)
    {
        std::size_t found = configfile.find_last_of("/");
        configpath        = configfile.substr(0, found);
        configpath.push_back('/');
    }
    server_loaclvar &static_server_var = get_server_global_var();
    static_server_var.config_path      = configpath;
    loadserverglobalconfig();
#ifdef ENABLE_WEBPAY
    // load acme.conf webpay.conf
    load_webpay_file();
#endif
}
#ifdef ENABLE_WEBPAY
// conf/webpay.conf -> get_webpay_config()
// 启动阶段单线程调用，重复调用只会重新解析，不会叠加重复段
bool serverconfig::load_webpay_file(const std::string &filename)
{
    std::string webpayfile = filename;
    if (webpayfile.empty())
    {
        if (configpath.empty())
        {
            return false;
        }
        webpayfile = configpath + "webpay.conf";
    }
    return get_webpay_config().load(webpayfile);
}
#endif// ENABLE_WEBPAY
bool serverconfig::loadserverglobalconfig()
{
    if (configfile.empty())
    {
        return false;
    }
    map_value = loadserversconfig(configfile);

    std::string current_wwwpath = map_value["default"]["wwwpath"];
    fs::path cpath              = current_wwwpath;
    if (!fs::is_directory(cpath))
    {
        cpath                              = fs::current_path();
        current_wwwpath                    = cpath.string();
        map_value["default"]["wwwpath"]    = current_wwwpath + "/www/default/";
        map_value["default"]["temppath"]   = current_wwwpath + "/temp/";
        map_value["default"]["logpath"]    = current_wwwpath + "/log/";
        map_value["default"]["serverpath"] = current_wwwpath + "/";

        if (map_value["default"]["mainhost"].size() > 0)
        {
            std::string temphost = map_value["default"]["mainhost"];

            if (map_value.find(temphost) != map_value.end())
            {
                if (map_value[temphost].find("wwwpath") != map_value[temphost].end())
                {
                    current_wwwpath     = map_value[temphost]["wwwpath"];
                    cpath               = current_wwwpath;
                    bool is_has_wwwpath = true;
                    if (current_wwwpath.size() > 0)
                    {
                        if (fs::is_directory(cpath))
                        {
                            is_has_wwwpath = false;
                        }
                    }
                    if (is_has_wwwpath)
                    {
                        cpath                          = fs::current_path();
                        current_wwwpath                = cpath.string();
                        map_value[temphost]["wwwpath"] = current_wwwpath + "/www/default/";
                    }
                }
            }
        }
    }

    if (map_value["default"]["index"].empty())
    {
        map_value["default"]["index"] = "index.html";
    }
    siteusehtmlchache     = htmlcache_switch_on(map_value["default"]["usehtmlcache"]);
    siteusehtmlchachetime = htmlcache_seconds(map_value["default"]["usehtmlcachetime"]);
    if (map_value["default"]["http2_enable"].size() > 0 && map_value["default"]["http2_enable"][0] == '1')
    {
        isallnothttp2 = true;
    }
    else
    {
        isallnothttp2 = false;
    }
    server_loaclvar &static_server_var = get_server_global_var();
    static_server_var.http2_enable     = isallnothttp2;
    struct site_host_info_t tempinfo_default;
    // html 缓存的开关和间隔落进站点表：site 0 直接用，其余站下面 tempinfo = tempinfo_default 时继承，
    // 自己段里写了 usehtmlcache / usehtmlcachetime 再覆盖。请求期只读站点这两项（httppeer.cpp 的 html_cache_expired）。
    tempinfo_default.is_usehtmlcache  = siteusehtmlchache;
    tempinfo_default.usehtmlcachetime = siteusehtmlchachetime;

    if (map_value["default"]["global_http2_enable"].size() > 0 && map_value["default"]["global_http2_enable"][0] == '1')
    {
        isallnothttp2 = true;
    }
    else
    {
        isallnothttp2 = false;
    }

    if (map_value["default"]["controlsopath"].size() > 0)
    {
        static_server_var.control_so_path = map_value["default"]["controlsopath"];
    }
    if (map_value["default"]["viewsopath"].size() > 0)
    {
        static_server_var.view_so_path = map_value["default"]["viewsopath"];
    }
    if (map_value["default"]["debug_enable"].size() > 0 && map_value["default"]["debug_enable"][0] == '1')
    {
        static_server_var.debug_enable = true;
    }
    else
    {
        static_server_var.debug_enable = false;
    }
    if (map_value["default"]["deamon_enable"].size() > 0 && map_value["default"]["deamon_enable"][0] == '1')
    {
        static_server_var.deamon_enable = true;
    }
    else
    {
        static_server_var.deamon_enable = false;
    }

    if (map_value["default"]["show_visitinfo"].size() > 0 && map_value["default"]["show_visitinfo"][0] == '1')
    {
        static_server_var.show_visitinfo = true;
    }
    else
    {
        static_server_var.show_visitinfo = false;
    }

    if (map_value["default"]["upload_max_size"].size() > 0)
    {
        unsigned int tempupmax = 0;
        for (size_t i = 0; i < map_value["default"]["upload_max_size"].size(); i++)
        {
            if (map_value["default"]["upload_max_size"][i] > 0x2F && map_value["default"]["upload_max_size"][i] < 0x3A)
            {
                tempupmax = tempupmax * 10 + (map_value["default"]["upload_max_size"][i] - '0');
            }
        }
        tempinfo_default.upload_max_size = tempupmax;
        tempinfo_default.is_limit_upload = false;
        if (tempupmax > 1024)
        {
            tempinfo_default.is_limit_upload = true;
        }
    }
    else
    {
        tempinfo_default.is_limit_upload = false;
        tempinfo_default.upload_max_size = 0;
    }

    //safe_domain(map_value["default"]["cors_domain"])
    tempinfo_default.cors_domain = map_value["default"]["cors_domain"];
    // 白名单在这里一次性解析完毕（未配置 → 默认拒绝，只有显式写 "*" 才放开全部），
    // 请求期只调用 cors_allow_origin() 查表
    cors_domain_parse(tempinfo_default.cors_domain, tempinfo_default);
    // 跨域时允许页面 JS 读取的响应头；留空则不发 Access-Control-Expose-Headers
    tempinfo_default.cors_expose_headers = map_value["default"]["cors_expose_headers"];
    // 跨域是否允许携带凭证（cookie / HTTP 认证）：1、T、t 开头为开，其余含未配置都是关
    {
        std::string tempcreds             = map_value["default"]["cors_credentials"];
        tempinfo_default.cors_credentials = (tempcreds.size() > 0 && (tempcreds[0] == '1' || tempcreds[0] == 'T' || tempcreds[0] == 't'));
    }

    // 预检允许的方法列表：不写就用 site_host_info_t 里的默认四项，写了整表覆盖（写空=一个方法都不放行）
    if (map_value["default"]["cors_allow_methods"].size() > 0)
    {
        cors_methods_parse(map_value["default"]["cors_allow_methods"], tempinfo_default.cors_allow_methods);
    }

    if (map_value["default"]["http_header_max_size"].size() > 0)
    {
        //     unsigned int tempupmax = 0;
        //     for (size_t i = 0; i < map_value["default"]["http_header_max_size"].size(); i++)
        //     {
        //         if (map_value["default"]["http_header_max_size"][i] > 0x2F && map_value["default"]["http_header_max_size"][i] < 0x3A)
        //         {
        //             tempupmax = tempupmax * 10 + (map_value["default"]["http_header_max_size"][i] - '0');
        //         }
        //     }

        //     if (tempupmax > 4096)
        //     {
        //         tempinfo.http_header_max_size = tempupmax;
        //     }
        // }
        // else
        // {
        //     tempinfo.http_header_max_size = 8192;
    }

    if (map_value["default"]["static_file_compress_cache"].size() > 0 && map_value["default"]["static_file_compress_cache"][0] == '1')
    {
        static_server_var.static_file_compress_cache = true;
    }
    else
    {
        static_server_var.static_file_compress_cache = false;
    }

    if (map_value["default"]["rate_limit_status"].size() > 0 && (map_value["default"]["rate_limit_status"][0] == '1' || map_value["default"]["rate_limit_status"][0] == 'T' || map_value["default"]["rate_limit_status"][0] == 't'))
    {
        static_server_var.rate_limit_status = true;
    }
    else
    {
        static_server_var.rate_limit_status = false;
    }

    if (map_value["default"]["session_type"].size() > 0)
    {
        switch (map_value["default"]["session_type"][0])
        {
        case '1':
            static_server_var.session_type = 1;
            break;
        default:
            static_server_var.session_type = 0;
        }
    }
    else
    {
        static_server_var.session_type = 0;
    }

    if (map_value["default"]["wwwpath"].size() == 0)
    {
        map_value["default"]["wwwpath"] = "/usr/local/var/www/";
    }

    static_server_var.map_value = map_value;
    static_server_var.www_path  = map_value["default"]["wwwpath"];

    static_server_var.temp_path = map_value["default"]["temppath"];
    static_server_var.log_path  = map_value["default"]["logpath"];
    mainhost                    = map_value["default"]["mainhost"];
    if (mainhost.size() > 3 && mainhost[0] == 'w' && mainhost[1] == 'w' && mainhost[2] == 'w' && mainhost[3] == '.')
    {
        secondhost.clear();
        if (mainhost.size() > 4)
        {
            secondhost.append(&mainhost[4], mainhost.size() - 4);
        }
    }
    else
    {
        if (mainhost.empty())
        {
            mainhost = "localhost";
        }
        else
        {
            secondhost = "www.";
            secondhost.append(mainhost);
        }
    }
    if (static_server_var.www_path.size() > 0 && static_server_var.www_path.back() != '/')
    {
        static_server_var.www_path.push_back('/');
    }
    if (static_server_var.temp_path.size() > 0 && static_server_var.temp_path.back() != '/')
    {
        static_server_var.temp_path.push_back('/');
    }
    if (static_server_var.log_path.size() > 0 && static_server_var.log_path.back() != '/')
    {
        static_server_var.log_path.push_back('/');
    }

    tempinfo_default.mainhost          = mainhost;
    tempinfo_default.wwwpath           = static_server_var.www_path;
    tempinfo_default.http2_enable      = static_server_var.http2_enable;
    tempinfo_default.document_index    = map_value["default"]["index"];
    tempinfo_default.is_show_directory = false;

    if (map_value["default"]["siteid"].size() > 0)
    {
        tempinfo_default.siteid = 0;
        for (size_t i = 0; i < map_value["default"]["siteid"].size(); i++)
        {
            if (map_value["default"]["siteid"][i] >= '0' && map_value["default"]["siteid"][i] <= '9')
            {
                tempinfo_default.siteid = tempinfo_default.siteid * 10 + (map_value["default"]["siteid"][i] - '0');
                continue;
            }
            else if (map_value["default"]["siteid"][i] == 0x20)
            {
                continue;
            }
            break;
        }
    }
    else
    {
        tempinfo_default.siteid = 0;
    }

    if (map_value["default"]["groupid"].size() > 0)
    {
        tempinfo_default.groupid = 0;
        for (size_t i = 0; i < map_value["default"]["groupid"].size(); i++)
        {
            if (map_value["default"]["groupid"][i] >= '0' && map_value["default"]["groupid"][i] <= '9')
            {
                tempinfo_default.groupid = tempinfo_default.groupid * 10 + (map_value["default"]["groupid"][i] - '0');
                continue;
            }
            else if (map_value["default"]["groupid"][i] == 0x20)
            {
                continue;
            }
            break;
        }
    }
    else
    {
        tempinfo_default.groupid = 0;
    }

    if (map_value["default"]["alias_domain"].size() > 2)
    {
        tempinfo_default.alias_domain.clear();
        for (size_t i = 0; i < map_value["default"]["alias_domain"].size(); i++)
        {
            if (map_value["default"]["alias_domain"][i] >= '0' && map_value["default"]["alias_domain"][i] <= '9')
            {
                tempinfo_default.alias_domain.push_back(map_value["default"]["alias_domain"][i]);
            }
            else if (map_value["default"]["alias_domain"][i] >= 'a' && map_value["default"]["alias_domain"][i] <= 'z')
            {
                tempinfo_default.alias_domain.push_back(map_value["default"]["alias_domain"][i]);
            }
            else if (map_value["default"]["alias_domain"][i] >= 'A' && map_value["default"]["alias_domain"][i] <= 'Z')
            {
                tempinfo_default.alias_domain.push_back(map_value["default"]["alias_domain"][i]);
            }
            else if (map_value["default"]["alias_domain"][i] == '.')
            {
                tempinfo_default.alias_domain.push_back(map_value["default"]["alias_domain"][i]);
            }
            else if (map_value["default"]["alias_domain"][i] == '_')
            {
                tempinfo_default.alias_domain.push_back(map_value["default"]["alias_domain"][i]);
            }
            else if (map_value["default"]["alias_domain"][i] == '-')
            {
                tempinfo_default.alias_domain.push_back(map_value["default"]["alias_domain"][i]);
            }
        }
    }

    if (map_value["default"]["directorylist"].size() > 0 && map_value["default"]["directorylist"][0] == '1')
    {
        tempinfo_default.is_show_directory = true;
    }

    if (map_value["default"]["directorylist"].size() > 0)
    {
        tempinfo_default.certificate_file = map_value["default"]["certificate_chain_file"];
    }
    else
    {
        tempinfo_default.certificate_file.clear();
    }

    if (map_value["default"]["certificate_chain_file"].size() > 0)
    {
        tempinfo_default.certificate_file = map_value["default"]["certificate_chain_file"];
    }
    else
    {
        tempinfo_default.certificate_file.clear();
    }

    if (map_value["default"]["private_key_file"].size() > 0)
    {
        tempinfo_default.privateKey_file = map_value["default"]["private_key_file"];
    }
    else
    {
        tempinfo_default.privateKey_file.clear();
    }

    if (map_value["default"]["rewrite_404"].size() > 0)
    {
        tempinfo_default.isrewrite = true;
        try
        {
            tempinfo_default.rewrite404 = std::stoul(map_value["default"]["rewrite_404"].c_str());
        }
        catch (const std::exception &e)
        {
            tempinfo_default.rewrite404 = 0;
        }
    }
    else
    {
        tempinfo_default.isrewrite  = false;
        tempinfo_default.rewrite404 = 0;
    }

    if (map_value["default"]["rewrite_404_action"].size() > 0)
    {
        std::string tempac, itemval;
        itemval        = map_value["default"]["rewrite_404_action"];
        unsigned int m = 0;
        for (; m < itemval.size(); m++)
        {
            if (itemval[m] == '|')
            {
                break;
            }
            tempac.push_back(itemval[m]);
        }
        tempinfo_default.rewrite_404_action = tempac;
        if (m < itemval.size() && itemval[m] == '|')
        {
            m++;
        }
        tempac.clear();
        for (; m < itemval.size(); m++)
        {
            if (itemval[m] == '|')
            {
                if (tempac.size() > 0)
                {
                    tempinfo_default.action_404_lists.push_back(tempac);
                }
                tempac.clear();
                continue;
            }
            tempac.push_back(itemval[m]);
        }
        if (tempac.size() > 0)
        {
            tempinfo_default.action_404_lists.push_back(tempac);
        }
    }
    else
    {
        tempinfo_default.rewrite_404_action.clear();
    }

    if (map_value["default"]["method_pre"].size() > 0)
    {
        std::string tempac, itemval;
        itemval        = map_value["default"]["method_pre"];
        unsigned int m = 0;
        tempac.clear();
        for (; m < itemval.size(); m++)
        {
            if (itemval[m] == '|')
            {
                if (tempac.size() > 0)
                {
                    tempinfo_default.action_pre_lists.push_back(conf_hook_name(tempac));
                }
                tempac.clear();
                continue;
            }
            tempac.push_back(itemval[m]);
        }
        if (tempac.size() > 0)
        {
            tempinfo_default.action_pre_lists.push_back(conf_hook_name(tempac));
        }
        if (tempinfo_default.action_pre_lists.size() > 0)
        {
            tempinfo_default.is_method_pre = true;
        }
    }
    else
    {
        tempinfo_default.action_pre_lists.clear();
    }

    if (map_value["default"]["method_after"].size() > 0)
    {
        std::string tempac, itemval;
        itemval        = map_value["default"]["method_after"];
        unsigned int m = 0;
        tempac.clear();
        for (; m < itemval.size(); m++)
        {
            if (itemval[m] == '|')
            {
                if (tempac.size() > 0)
                {
                    tempinfo_default.action_after_lists.push_back(conf_hook_name(tempac));
                }
                tempac.clear();
                continue;
            }
            tempac.push_back(itemval[m]);
        }
        if (tempac.size() > 0)
        {
            tempinfo_default.action_after_lists.push_back(conf_hook_name(tempac));
        }
        if (tempinfo_default.action_after_lists.size() > 0)
        {
            tempinfo_default.is_method_after = true;
        }
    }
    else
    {
        tempinfo_default.action_after_lists.clear();
    }

    if (map_value["default"]["acme_auto"].size() > 0)
    {
        std::string itemval;
        itemval = map_value["default"]["acme_auto"];

        if (itemval.size() > 0)
        {
            if (itemval[0] == '1' || itemval[0] == 'T')
            {
                tempinfo_default.is_acme = true;
            }
            else
            {
                tempinfo_default.is_acme = false;
            }
        }
        else
        {
            tempinfo_default.is_acme = false;
        }
    }
    else
    {
        tempinfo_default.is_acme = false;
    }

    if (map_value["default"]["rate_limit_new_wait_num"].size() > 0)
    {
        rate_limit_new_wait_num = 0;
        for (size_t i = 0; i < map_value["default"]["rate_limit_new_wait_num"].size(); i++)
        {
            if (map_value["default"]["rate_limit_new_wait_num"][i] >= '0' && map_value["default"]["rate_limit_new_wait_num"][i] <= '9')
            {
                rate_limit_new_wait_num = rate_limit_new_wait_num * 10 + (map_value["default"]["rate_limit_new_wait_num"][i] - '0');
                continue;
            }
            else if (map_value["default"]["rate_limit_new_wait_num"][i] == 0x20)
            {
                continue;
            }
            break;
        }

        if (rate_limit_new_wait_num < 60)
        {
            rate_limit_new_wait_num = 60;
        }
    }
    else
    {
        rate_limit_new_wait_num = 300;
    }

    if (map_value["default"]["rate_limit_accept_wait_num"].size() > 0)
    {
        rate_limit_accept_wait_num = 0;
        for (size_t i = 0; i < map_value["default"]["rate_limit_accept_wait_num"].size(); i++)
        {
            if (map_value["default"]["rate_limit_accept_wait_num"][i] >= '0' && map_value["default"]["rate_limit_accept_wait_num"][i] <= '9')
            {
                rate_limit_accept_wait_num = rate_limit_accept_wait_num * 10 + (map_value["default"]["rate_limit_accept_wait_num"][i] - '0');
                continue;
            }
            else if (map_value["default"]["rate_limit_accept_wait_num"][i] == 0x20)
            {
                continue;
            }
            break;
        }

        if (rate_limit_accept_wait_num < 120)
        {
            rate_limit_accept_wait_num = 120;
        }

        if ((rate_limit_new_wait_num + 100) >= rate_limit_accept_wait_num)
        {
            rate_limit_accept_wait_num = rate_limit_new_wait_num * 2;
        }
    }
    else
    {
        rate_limit_accept_wait_num = 600;
    }

    if (map_value["default"]["rate_limit_second_num1"].size() > 0)
    {
        rate_limit_second_num1 = 0;
        for (size_t i = 0; i < map_value["default"]["rate_limit_second_num1"].size(); i++)
        {
            if (map_value["default"]["rate_limit_second_num1"][i] >= '0' && map_value["default"]["rate_limit_second_num1"][i] <= '9')
            {
                rate_limit_second_num1 = rate_limit_second_num1 * 10 + (map_value["default"]["rate_limit_second_num1"][i] - '0');
                continue;
            }
            else if (map_value["default"]["rate_limit_second_num1"][i] == 0x20)
            {
                continue;
            }
            break;
        }

        if (rate_limit_second_num1 < 5)
        {
            rate_limit_second_num1 = 5;
        }

        if (rate_limit_second_num1 > 5000)
        {
            rate_limit_second_num1 = 5000;
        }
    }
    else
    {
        rate_limit_second_num1 = 20;
    }

    if (map_value["default"]["rate_limit_second_num2"].size() > 0)
    {
        rate_limit_second_num2 = 0;
        for (size_t i = 0; i < map_value["default"]["rate_limit_second_num2"].size(); i++)
        {
            if (map_value["default"]["rate_limit_second_num2"][i] >= '0' && map_value["default"]["rate_limit_second_num2"][i] <= '9')
            {
                rate_limit_second_num2 = rate_limit_second_num2 * 10 + (map_value["default"]["rate_limit_second_num2"][i] - '0');
                continue;
            }
            else if (map_value["default"]["rate_limit_second_num2"][i] == 0x20)
            {
                continue;
            }
            break;
        }

        if (rate_limit_second_num2 < 2)
        {
            rate_limit_second_num2 = 1;
        }

        if (rate_limit_second_num2 > 300)
        {
            rate_limit_second_num2 = 300;
        }
    }
    else
    {
        rate_limit_second_num2 = 5;
    }

    if (map_value["default"]["acme_every_day"].size() > 0)
    {
        acme_every_day_time = 0;
        for (size_t i = 0; i < map_value["default"]["acme_every_day"].size(); i++)
        {
            if (map_value["default"]["acme_every_day"][i] >= '0' && map_value["default"]["acme_every_day"][i] <= '9')
            {
                acme_every_day_time = acme_every_day_time * 10 + (map_value["default"]["acme_every_day"][i] - '0');
                continue;
            }
            break;
        }

        if (acme_every_day_time > 23)
        {
            acme_every_day_time = 23;
        }

        if (acme_every_day_time < 2)
        {
            acme_every_day_time = 2;
        }
    }
    else
    {
        acme_every_day_time = 7;
    }

    if (map_value["default"]["acme_every_num"].size() > 0)
    {
        acme_every_num = 0;
        for (size_t i = 0; i < map_value["default"]["acme_every_num"].size(); i++)
        {
            if (map_value["default"]["acme_every_num"][i] >= '0' && map_value["default"]["acme_every_num"][i] <= '9')
            {
                acme_every_num = acme_every_num * 10 + (map_value["default"]["acme_every_num"][i] - '0');
                continue;
            }
            break;
        }

        if (acme_every_num > 30)
        {
            acme_every_num = 30;
        }

        if (acme_every_num < 1)
        {
            acme_every_num = 1;
        }
    }
    else
    {
        acme_every_num = 5;
    }

    if (map_value["default"]["ocsp_intv_time"].size() > 0)
    {
        ocsp_interval_time = 0;
        for (size_t i = 0; i < map_value["default"]["ocsp_intv_time"].size(); i++)
        {
            if (map_value["default"]["ocsp_intv_time"][i] >= '0' && map_value["default"]["ocsp_intv_time"][i] <= '9')
            {
                ocsp_interval_time = ocsp_interval_time * 10 + (map_value["default"]["ocsp_intv_time"][i] - '0');
                continue;
            }
            break;
        }

        if (ocsp_interval_time > 86400)
        {
            ocsp_interval_time = 86400;
        }

        if (ocsp_interval_time < 500)
        {
            ocsp_interval_time = 500;
        }
    }
    else
    {
        ocsp_interval_time = 14400;
    }

    if (map_value["default"]["ip6_listen_enable"].size() > 0)
    {
        for (size_t j = 0; j < map_value["default"]["ip6_listen_enable"].size(); j++)
        {
            if (map_value["default"]["ip6_listen_enable"][j] == ' ')
            {
                continue;
            }
            else if (map_value["default"]["ip6_listen_enable"][j] == '1')
            {
                ip6_enable = true;
                break;
            }
            else if (map_value["default"]["ip6_listen_enable"][j] == 'T')
            {
                ip6_enable = true;
                break;
            }
            else if (map_value["default"]["ip6_listen_enable"][j] == 't')
            {
                ip6_enable = true;
                break;
            }
            else if (map_value["default"]["ip6_listen_enable"][j] == '0')
            {
                ip6_enable = false;
                break;
            }
            else if (map_value["default"]["ip6_listen_enable"][j] == 'F')
            {
                ip6_enable = false;
                break;
            }
            else if (map_value["default"]["ip6_listen_enable"][j] == 'f')
            {
                ip6_enable = false;
                break;
            }
            else
            {
                ip6_enable = true;
                break;
            }
        }
    }
    else
    {
        ip6_enable = false;
    }

    tempinfo_default.php_root_document = tempinfo_default.wwwpath;
    sitehostinfos.push_back(std::move(tempinfo_default));
    for (auto [first, second] : map_value)
    {
        if (first != "default")
        {
            if (first.size() > 0 && first[0] != '*')
            {
                struct site_host_info_t tempinfo;
                tempinfo          = tempinfo_default;
                tempinfo.mainhost = first;
                tempinfo.certificate_file.clear();
                tempinfo.privateKey_file.clear();
                tempinfo.alias_domain.clear();
                // CORS 白名单从 [default] 继承。上面把 tempinfo_default 移进了 sitehostinfos，
                // 容器成员被搬空（bool 成员是复制，所以 allow_all / is_cors
                // 反而带着默认站的值），因此这几项要显式从已入库的默认站点取回来。
                // 站点自己写了 cors_domain 时，下面的循环整表重解析覆盖。
                tempinfo.cors_domain         = sitehostinfos[0].cors_domain;
                tempinfo.cors_allow_all      = sitehostinfos[0].cors_allow_all;
                tempinfo.is_cors             = sitehostinfos[0].is_cors;
                tempinfo.cors_origin_allowed = sitehostinfos[0].cors_origin_allowed;
                tempinfo.cors_domain_list    = sitehostinfos[0].cors_domain_list;
                tempinfo.cors_credentials    = sitehostinfos[0].cors_credentials;
                tempinfo.cors_allow_methods  = sitehostinfos[0].cors_allow_methods;
                tempinfo.themes.clear();
                tempinfo.themes_url.clear();
                tempinfo.http2_enable = isallnothttp2;
                tempinfo.isuse_php    = false;

                for (auto [itemname, itemval] : second)
                {
                    if (itemname == "http2_enable")
                    {
                        if (itemval.size() > 0 && (itemval[0] == '1' || itemval[0] == 'T' || itemval[0] == 't'))
                        {
                            tempinfo.http2_enable = true;
                        }
                        else
                        {
                            tempinfo.http2_enable = false;
                        }
                    }
                    else if (itemname == "isuse_php")
                    {
                        if (itemval.size() > 0 && (itemval[0] == '1' || itemval[0] == 'T' || itemval[0] == 't'))
                        {
                            tempinfo.isuse_php = true;
                        }
                        else
                        {
                            tempinfo.isuse_php = false;
                        }
                    }
                    else if (itemname == "fastcgi_host")
                    {
                        tempinfo.fastcgi_host = itemval;
                    }
                    else if (itemname == "index")
                    {
                        tempinfo.document_index = itemval;
                    }
                    else if (itemname == "usehtmlcache")
                    {
                        // 没写这一行才跟着 [default]（上面 tempinfo = tempinfo_default 继承来的值），
                        // 写了就以此为准，所以本站可以写 usehtmlcache=0 单独关掉
                        tempinfo.is_usehtmlcache = htmlcache_switch_on(itemval);
                    }
                    else if (itemname == "usehtmlcachetime")
                    {
                        tempinfo.usehtmlcachetime = htmlcache_seconds(itemval);
                    }
                    else if (itemname == "fastcgi_port")
                    {
                        std::string tempac;
                        unsigned int m = 0;
                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] > 0x2F && itemval[m] < 0x3A)
                            {
                                tempac.push_back(itemval[m]);
                            }
                        }
                        tempinfo.fastcgi_port = itemval;
                    }
                    else if (itemname == "rewrite_php")
                    {
                        std::string tempac;
                        std::string tempab;
                        std::string tempaa;
                        unsigned int m      = 0;
                        bool is_prerootpath = true;
                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] == '|')
                            {
                                m++;
                                is_prerootpath = false;
                                break;
                            }
                            tempaa.push_back(itemval[m]);
                        }
                        if (is_prerootpath)
                        {
                            tempinfo.php_root_document = tempinfo.wwwpath;
                        }
                        else
                        {
                            if (tempaa.size() > 0 && tempaa.back() != '/')
                            {
                                tempaa.push_back('/');
                            }
                            tempinfo.php_root_document = tempaa;
                        }
                        tempaa.clear();

                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] == '|')
                            {
                                tempac.clear();
                                tempaa.clear();
                                unsigned int mm = 0;
                                for (; mm < tempab.size(); mm++)
                                {
                                    if (tempab[mm] == '/')
                                    {
                                        if (tempaa.size() > 0)
                                        {
                                            tempaa.push_back('/');
                                        }
                                        tempaa.append(tempac);
                                        tempac.clear();
                                        continue;
                                    }
                                    tempac.push_back(tempab[mm]);
                                }

                                struct stat sessfileinfo;
                                tempac = tempinfo.php_root_document + tempab;
                                memset(&sessfileinfo, 0, sizeof(sessfileinfo));
                                if (stat(tempac.c_str(), &sessfileinfo) == 0)
                                {
                                    if (sessfileinfo.st_mode & S_IFREG)
                                    {
                                        tempinfo.rewrite_php_lists.emplace_back(tempaa, tempab);
                                    }
                                }

                                tempab.clear();
                                continue;
                            }
                            if (itemval[m] == 0x20)
                            {
                                continue;
                            }
                            tempab.push_back(itemval[m]);
                        }

                        if (tempab.size() > 0)
                        {
                            unsigned int mm = 0;
                            tempac.clear();
                            tempaa.clear();
                            for (; mm < tempab.size(); mm++)
                            {
                                if (tempab[mm] == '/')
                                {
                                    if (tempaa.size() > 0)
                                    {
                                        tempaa.push_back('/');
                                    }
                                    tempaa.append(tempac);
                                    tempac.clear();
                                    continue;
                                }
                                tempac.push_back(tempab[mm]);
                            }

                            struct stat sessfileinfo;
                            tempac = tempinfo.php_root_document + tempab;
                            memset(&sessfileinfo, 0, sizeof(sessfileinfo));
                            if (stat(tempac.c_str(), &sessfileinfo) == 0)
                            {
                                if (sessfileinfo.st_mode & S_IFREG)
                                {
                                    tempinfo.rewrite_php_lists.emplace_back(tempaa, tempab);
                                }
                            }
                        }
                    }
                    else if (itemname == "wwwpath")
                    {
                        tempinfo.wwwpath = itemval;
                        if (tempinfo.wwwpath.size() == 0)
                        {
                            tempinfo.wwwpath = static_server_var.www_path;
                        }
                        if (tempinfo.wwwpath.size() > 0 && tempinfo.wwwpath.back() != '/')
                        {
                            tempinfo.wwwpath.push_back('/');
                        }
                    }
                    else if (itemname == "rewrite_404")
                    {

                        try
                        {
                            tempinfo.rewrite404 = std::stoul(itemval.c_str());
                        }
                        catch (const std::exception &e)
                        {
                            tempinfo.rewrite404 = 0;
                        }
                        if (tempinfo.rewrite404 > 0)
                        {
                            tempinfo.isrewrite = true;
                        }
                        else
                        {
                            tempinfo.isrewrite = false;
                        }
                    }
                    else if (itemname == "rewrite_404_action")
                    {
                        std::string tempac;
                        unsigned int m = 0;
                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] == '|')
                            {
                                break;
                            }
                            tempac.push_back(itemval[m]);
                        }
                        tempinfo.rewrite_404_action = tempac;
                        if (m < itemval.size() && itemval[m] == '|')
                        {
                            m++;
                        }
                        tempac.clear();
                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] == '|')
                            {
                                if (tempac.size() > 0)
                                {
                                    tempinfo.action_404_lists.push_back(tempac);
                                }
                                tempac.clear();
                                continue;
                            }
                            tempac.push_back(itemval[m]);
                        }
                        if (tempac.size() > 0)
                        {
                            tempinfo.action_404_lists.push_back(tempac);
                        }
                    }
                    else if (itemname == "certificate_chain_file")
                    {
                        tempinfo.certificate_file = itemval;
                    }
                    else if (itemname == "private_key_file")
                    {
                        tempinfo.privateKey_file = itemval;
                    }
                    else if (itemname == "static_pre")
                    {
                        std::string tempac;
                        unsigned int m = 0;
                        tempac.clear();
                        tempinfo.static_pre_method.clear();
                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] == '|')
                            {
                                tempinfo.static_pre_method = conf_hook_name(tempac);
                                m++;
                                break;
                            }
                            if (itemval[m] == 0x20 || itemval[m] == '\t')
                            {
                                continue;
                            }
                            tempac.push_back(itemval[m]);
                        }
                        if (tempinfo.static_pre_method.size() > 0)
                        {
                            tempinfo.is_static_pre = true;
                        }
                        else
                        {
                            if (tempac.size() > 0)
                            {
                                tempinfo.static_pre_method = conf_hook_name(tempac);
                                tempinfo.is_static_pre     = true;
                            }
                        }
                        // 钩子名的存在性由 router::validate_site_hooks() 在注册完成后校验：
                        // 这里注册表还是空的，解析期判断只会得到「全部不存在」

                        tempac.clear();
                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] == '|')
                            {
                                if (tempac.size() > 0)
                                {
                                    tempinfo.static_pre_lists.push_back(tempac);
                                }
                                tempac.clear();
                                continue;
                            }
                            if (itemval[m] == 0x20 || itemval[m] == '\t')
                            {
                                continue;
                            }
                            tempac.push_back(itemval[m]);
                        }
                        if (tempac.size() > 0)
                        {
                            tempinfo.static_pre_lists.push_back(tempac);
                        }
                    }
                    else if (itemname == "method_pre")
                    {
                        std::string tempac;
                        unsigned int m = 0;
                        tempac.clear();
                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] == '|')
                            {
                                if (tempac.size() > 0)
                                {
                                    tempinfo.action_pre_lists.push_back(conf_hook_name(tempac));
                                }
                                tempac.clear();
                                continue;
                            }
                            tempac.push_back(itemval[m]);
                        }
                        if (tempac.size() > 0)
                        {
                            tempinfo.action_pre_lists.push_back(conf_hook_name(tempac));
                        }

                        if (tempinfo.action_pre_lists.size() > 0)
                        {
                            tempinfo.is_method_pre = true;
                        }
                    }
                    else if (itemname == "method_after")
                    {
                        std::string tempac;
                        unsigned int m = 0;
                        tempac.clear();
                        for (; m < itemval.size(); m++)
                        {
                            if (itemval[m] == '|')
                            {
                                if (tempac.size() > 0)
                                {
                                    tempinfo.action_after_lists.push_back(conf_hook_name(tempac));
                                }
                                tempac.clear();
                                continue;
                            }
                            tempac.push_back(itemval[m]);
                        }
                        if (tempac.size() > 0)
                        {
                            tempinfo.action_after_lists.push_back(conf_hook_name(tempac));
                        }
                        if (tempinfo.action_after_lists.size() > 0)
                        {
                            tempinfo.is_method_after = true;
                        }
                    }
                    else if (itemname == "directorylist")
                    {
                        if (itemval.size() > 0 && (itemval[0] == '1' || itemval[0] == 'T' || itemval[0] == 't'))
                        {
                            tempinfo.is_show_directory = true;
                        }
                        else
                        {
                            tempinfo.is_show_directory = false;
                        }
                    }
                    else if (itemname == "upload_max_size")
                    {
                        tempinfo.is_limit_upload = false;
                        if (itemval.size() > 0)
                        {
                            unsigned int tempupmax = 0;
                            for (size_t i = 0; i < itemval.size(); i++)
                            {
                                if (itemval[i] > 0x2F && itemval[i] < 0x3A)
                                {
                                    tempupmax = tempupmax * 10 + (itemval[i] - '0');
                                }
                            }
                            tempinfo.upload_max_size = tempupmax;
                            if (tempupmax > 1024)
                            {
                                tempinfo.is_limit_upload = true;
                            }
                        }
                        else
                        {
                            tempinfo.upload_max_size = 0;
                        }
                    }
                    else if (itemname == "http_header_max_size")
                    {
                        // tempinfo.http_header_max_size = 8192;
                        // if (itemval.size() > 0)
                        // {
                        //     unsigned int tempupmax = 0;
                        //     for (size_t i = 0; i < itemval.size(); i++)
                        //     {
                        //         if (itemval[i] > 0x2F && itemval[i] < 0x3A)
                        //         {
                        //             tempupmax = tempupmax * 10 + (itemval[i] - '0');
                        //         }
                        //     }
                        //     if (tempupmax > 4096)
                        //     {
                        //         tempinfo.upload_max_size = tempupmax;
                        //     }
                        // }
                    }
                    else if (itemname == "siteid")
                    {
                        tempinfo.siteid = 0;
                        for (size_t i = 0; i < itemval.size(); i++)
                        {
                            if (itemval[i] >= '0' && itemval[i] <= '9')
                            {
                                tempinfo.siteid = tempinfo.siteid * 10 + (itemval[i] - '0');
                                continue;
                            }
                            else if (itemval[i] == 0x20)
                            {
                                continue;
                            }
                            break;
                        }
                    }
                    else if (itemname == "groupid")
                    {
                        tempinfo.groupid = 0;
                        for (size_t i = 0; i < itemval.size(); i++)
                        {
                            if (itemval[i] >= '0' && itemval[i] <= '9')
                            {
                                tempinfo.groupid = tempinfo.groupid * 10 + (itemval[i] - '0');
                                continue;
                            }
                            else if (itemval[i] == 0x20)
                            {
                                continue;
                            }
                            break;
                        }
                    }
                    else if (itemname == "is_close")
                    {
                        if (itemval.size() > 0 && (itemval[0] == '1' || itemval[0] == 'T' || itemval[0] == 't'))
                        {
                            tempinfo.is_close = true;
                        }
                        else
                        {
                            tempinfo.is_close = false;
                        }
                    }
                    else if (itemname == "alias_domain")
                    {
                        tempinfo.alias_domain.clear();
                        for (size_t i = 0; i < itemval.size(); i++)
                        {
                            if (itemval[i] >= '0' && itemval[i] <= '9')
                            {
                                tempinfo.alias_domain.push_back(itemval[i]);
                            }
                            else if (itemval[i] >= 'a' && itemval[i] <= 'z')
                            {
                                tempinfo.alias_domain.push_back(itemval[i]);
                            }
                            else if (itemval[i] >= 'A' && itemval[i] <= 'Z')
                            {
                                tempinfo.alias_domain.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '.')
                            {
                                tempinfo.alias_domain.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '_')
                            {
                                tempinfo.alias_domain.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '-')
                            {
                                tempinfo.alias_domain.push_back(itemval[i]);
                            }
                        }
                    }
                    else if (itemname == "cors_domain")
                    {
                        tempinfo.cors_domain = itemval;
                        // 逗号切分/补 scheme/'*' 判定全部在此完成（未配置或写空即默认拒绝），
                        // 请求期无需再碰配置串
                        cors_domain_parse(tempinfo.cors_domain, tempinfo);
                    }
                    else if (itemname == "cors_expose_headers")
                    {
                        // 跨域允许 JS 读取的响应头；不配置就沿用 default 段的取值
                        tempinfo.cors_expose_headers = itemval;
                    }
                    else if (itemname == "cors_credentials")
                    {
                        // 站点写了就以站点为准：0/空 是显式关闭，不继承 [default] 的开启
                        tempinfo.cors_credentials = (itemval.size() > 0 && (itemval[0] == '1' || itemval[0] == 'T' || itemval[0] == 't'));
                    }
                    else if (itemname == "cors_allow_methods")
                    {
                        // 站点写了就整表覆盖 [default] 继承来的列表；写空=本站预检不放行任何方法
                        cors_methods_parse(itemval, tempinfo.cors_allow_methods);
                    }
                    else if (itemname == "themes")
                    {
                        tempinfo.themes.clear();
                        for (size_t i = 0; i < itemval.size(); i++)
                        {
                            if (itemval[i] >= '0' && itemval[i] <= '9')
                            {
                                tempinfo.themes.push_back(itemval[i]);
                            }
                            else if (itemval[i] >= 'a' && itemval[i] <= 'z')
                            {
                                tempinfo.themes.push_back(itemval[i]);
                            }
                            else if (itemval[i] >= 'A' && itemval[i] <= 'Z')
                            {
                                tempinfo.themes.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '.')
                            {
                                tempinfo.themes.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '_')
                            {
                                tempinfo.themes.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '-')
                            {
                                tempinfo.themes.push_back(itemval[i]);
                            }
                        }
                    }
                    else if (itemname == "themes_url")
                    {
                        tempinfo.themes_url.clear();
                        for (size_t i = 0; i < itemval.size(); i++)
                        {
                            if (itemval[i] >= '0' && itemval[i] <= '9')
                            {
                                tempinfo.themes_url.push_back(itemval[i]);
                            }
                            else if (itemval[i] >= 'a' && itemval[i] <= 'z')
                            {
                                tempinfo.themes_url.push_back(itemval[i]);
                            }
                            else if (itemval[i] >= 'A' && itemval[i] <= 'Z')
                            {
                                tempinfo.themes_url.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '.')
                            {
                                tempinfo.themes_url.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '_')
                            {
                                tempinfo.themes_url.push_back(itemval[i]);
                            }
                            else if (itemval[i] == '-')
                            {
                                tempinfo.themes_url.push_back(itemval[i]);
                            }
                            else if (itemval[i] == ':')
                            {
                                tempinfo.themes_url.push_back(itemval[i]);
                            }
                            else if (itemval[i] == 0x2F)
                            {
                                tempinfo.themes_url.push_back(itemval[i]);
                            }
                        }
                    }
                    else if (itemname == "acme_auto")
                    {
                        if (itemval.size() > 0 && (itemval[0] == '1' || itemval[0] == 'T' || itemval[0] == 't'))
                        {
                            tempinfo.is_acme = true;
                        }
                        else
                        {
                            tempinfo.is_acme = false;
                        }
                    }
                }

                if (tempinfo.document_index.empty())
                {
                    tempinfo.document_index = "index.html";
                }
                sitehostinfos.push_back(std::move(tempinfo));
                if (sitehostinfos.size() > 0)
                {
                    unsigned int tempindex = static_cast<unsigned int>(sitehostinfos.size() - 1);
                    host_toint[first]      = tempindex;
                    // alias_domain 是注册函数的归属标识（不受真实域名影响），
                    // 不进 host_toint — 真实请求域名查 host_toint，注册 site 查 site_to_slot
                    // slot_id 由 router_init_sites() 统一分配，serverconfig 不管 slot
                }
            }
        }
    }

    // 既没写自己白名单、[default] 也没有可继承的白名单 → 该站点按"默认拒绝"处理，
    // 一个 CORS 头都不发。这里在加载完成后点名提示一次，别让运维以为是站点坏了：
    // 需要放开的站点必须显式写 cors_domain（白名单或 "*"）。
    // 告警留在加载期：请求热路径上逐次 fprintf+fflush 会刷满 stderr
    for (auto &site_item : sitehostinfos)
    {
        if (!site_item.is_cors)
        {
            fprintf(stderr, "[CORS-WARN] site host=%s not configured cors_domain, default deny all origin\n", site_item.mainhost.empty() ? "default" : site_item.mainhost.c_str());
            fflush(stderr);
        }
    }

    return true;
}
bool serverconfig::checkmaindomain(const char *servername)
{
    if (mainhost.empty())
    {
        return false;
    }
    if (str_casecmp(servername, mainhost))
    {
        return true;
    }
    if (str_casecmp(servername, secondhost))
    {
        return true;
    }

    return false;
}
std::string serverconfig::ssl_chain_file()
{
    return configpath + map_value["default"]["certificate_chain_file"];
}
std::string serverconfig::ssl_key_file()
{
    return configpath + map_value["default"]["private_key_file"];
}
std::string serverconfig::ssl_dh_file()
{
    return configpath + map_value["default"]["tmp_dh_file"];
}
std::string serverconfig::ssl_chain_crt_file()
{
    return configpath + map_value["default"]["chain_crt_file"];
}
unsigned char serverconfig::get_co_thread_num()
{
    unsigned char tempnum = 0;

    for (size_t i = 0; i < map_value["default"]["cothreadnum"].size(); i++)
    {
        if (map_value["default"]["cothreadnum"][i] > 0x2F && map_value["default"]["cothreadnum"][i] < 0x3A)
        {
            tempnum = tempnum * 10 + (map_value["default"]["cothreadnum"][i] - '0');
        }
    }
    if (tempnum > 0)
    {
        return tempnum;
    }
    else
    {
        return 4;
    }
}

unsigned int serverconfig::get_ssl_port()
{
    unsigned int tempnum = 0;

    for (size_t i = 0; i < map_value["default"]["httpsport"].size(); i++)
    {
        if (map_value["default"]["httpsport"][i] > 0x2F && map_value["default"]["httpsport"][i] < 0x3A)
        {
            tempnum = tempnum * 10 + (map_value["default"]["httpsport"][i] - '0');
        }
    }
    if (tempnum > 9)
    {
        return tempnum;
    }
    else
    {
        return 443;
    }
}
unsigned int serverconfig::get_port()
{
    unsigned int tempnum = 0;

    for (std::size_t i = 0; i < map_value["default"]["httpport"].size(); i++)
    {
        if (map_value["default"]["httpport"][i] > 0x2F && map_value["default"]["httpport"][i] < 0x3A)
        {
            tempnum = tempnum * 10 + (map_value["default"]["httpport"][i] - '0');
        }
    }
    if (tempnum > 9)
    {
        return tempnum;
    }
    else
    {
        return 80;
    }
}
SSL_CTX *serverconfig::getdefaultctx()
{
    SSL_CTX *ctx = NULL;
    for (auto i = g_ctxMap.begin(); i != g_ctxMap.end(); ++i)
    {
        ctx = i->second;
        break;
    }
    return ctx;
}
SSL_CTX *serverconfig::getctx(std::string filename)
{
    SSL_CTX *ctx = NULL;

    if (clear_ctx)
    {
        return ctx;
    }

    if (g_ctxMap.find(filename) != g_ctxMap.end())
    {
        ctx = g_ctxMap[filename];
    }
    else
    {
        std::string filepath = configpath;
        if (filepath.empty())
        {
            filepath = "conf/";
        }
        g_ctxMap[filename] = SSL_CTX_new(SSLv23_server_method());
        ctx                = g_ctxMap[filename];
        // filename.append(".pem");
        // boost::asio::error_code ec;
        std::string filepathurl;

        if (filepath.back() != '/')
        {
            filepath.push_back('/');
        }

        filepathurl.append(filepath);
        filepath.append(filename);
        filepath.append(".pem");
        DEBUG_LOG(" ssl file:%s", filepath.c_str());
        if (filename[0] == 'w' && filename[1] == 'w' && filename[2] == 'w' && filename[3] == '.')
        {
            if (::SSL_CTX_use_certificate_chain_file(ctx, filepath.c_str()) != 1)
            {
                // ec = boost::asio::error_code(
                //     static_cast<int>(::ERR_get_error()),
                //     boost::asio::error::get_ssl_category());
                filepath = filepathurl;
                filepath.append(&filename[4], filename.size() - 4);
                filepath.append(".pem");
                if (::SSL_CTX_use_certificate_chain_file(ctx, filepath.c_str()) != 1)
                {
                    // return ctx;
                    // isdomainsave=false;
                }
            }
        }
        else
        {

            if (::SSL_CTX_use_certificate_chain_file(ctx, filepath.c_str()) != 1)
            {
                // ec = boost::asio::error_code(
                //     static_cast<int>(::ERR_get_error()),
                //     boost::asio::error::get_ssl_category());

                filepathurl.append("www.");
                filepathurl.append(filename);
                filepath = filepathurl;
                filepath.append(".pem");
                DEBUG_LOG(" ssl file:%s", filepath.c_str());
                if (::SSL_CTX_use_certificate_chain_file(ctx, filepath.c_str()) != 1)
                {
                }
            }
        }
        filepath.erase(filepath.end() - 4, filepath.end());
        filepath.append(".key");
        if (::SSL_CTX_use_PrivateKey_file(ctx, filepath.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            // ec = boost::asio::error_code(
            //     static_cast<int>(::ERR_get_error()),
            //     boost::asio::error::get_ssl_category());
        }
        filepath.erase(filepath.end() - 4, filepath.end());
        filepath.append(".crt");
        if (::SSL_CTX_use_certificate_file(ctx, filepath.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            //ec = translate_error(::ERR_get_error());
            //ASIO_SYNC_OP_VOID_RETURN(ec);
        }

        if (!SSL_CTX_check_private_key(ctx))
        {
            //  ec = boost::asio::error_code(
            //     static_cast<int>(::ERR_get_error()),
            //     boost::asio::error::get_ssl_category());
        }

        // 在 per-domain SSL_CTX 上注册最小化 OCSP Stapling 回调
        // OpenSSL 3.x 要求 tlsext_status_cb 存在才会发送 OCSP response
        enable_ocsp_stapling(ctx);
    }
    return ctx;
}
void serverconfig::clearctx()
{
    clear_ctx = true;
    for (auto i = g_ctxMap.begin(); i != g_ctxMap.end(); ++i)
    {
        SSL_CTX_free(i->second);
    }
    g_ctxMap.clear();
    clear_ctx = false;
}

// ============================================================
//  OCSP Stapling 实现
// ============================================================

// OCSP stapling 回调：OpenSSL 在 TLS 握手构造 CertificateStatus 时调用。
// 负责从缓存中查找当前域名的 OCSP 响应并设置到 SSL 对象。
// 返回值使用 SSL_TLSEXT_ERR_* 系列常量（与 SNI 回调相同）：
//   SSL_TLSEXT_ERR_OK(0)    = 发送已设置的 OCSP response
//   SSL_TLSEXT_ERR_NOACK(3) = 不发送（缓存无数据或未请求）
// 注意: 返回 1(ALERT_WARNING) 或 2(ALERT_FATAL) 会导致 TLS alert 80
int serverconfig::ocsp_stapling_cb(SSL *ssl, void *arg)
{
    if (!ssl || !arg)
        return SSL_TLSEXT_ERR_NOACK;

    auto *self             = static_cast<serverconfig *>(arg);
    const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
    if (!servername || servername[0] == '\0')
        return SSL_TLSEXT_ERR_NOACK;

    auto resp = self->get_ocsp_staple(servername);
    if (resp.empty())
        return SSL_TLSEXT_ERR_NOACK;

    // OCSP 响应通常不超过几 KB，检查防溢出
    if (resp.size() > static_cast<size_t>(INT_MAX))
        return SSL_TLSEXT_ERR_NOACK;

    uint8_t *resp_copy = static_cast<uint8_t *>(OPENSSL_malloc(resp.size()));
    if (!resp_copy)
        return SSL_TLSEXT_ERR_NOACK;

    memcpy(resp_copy, resp.data(), resp.size());

    if (SSL_set_tlsext_status_ocsp_resp(ssl, resp_copy, static_cast<int>(resp.size())) != 1)
    {
        OPENSSL_free(resp_copy);
        return SSL_TLSEXT_ERR_NOACK;
    }

    return SSL_TLSEXT_ERR_OK;
}

void serverconfig::set_ocsp_staple(const std::string &domain, std::vector<uint8_t> ocsp_der)
{
    // 整段"取旧快照 → 克隆 → 改 → 换"都在锁内：两个写线程各自 snapshot 同一份旧 map 时，
    // 后一次 store 会吞掉前一次新增的域名（refresh_all_ocsp_staples 每轮 detach 一个新线程）
    std::lock_guard<std::mutex> lock(ocsp_cache_mutex_);
    auto old           = ocsp_cache_;
    auto new_map       = old ? std::make_shared<std::unordered_map<std::string, std::vector<uint8_t>>>(*old) : std::make_shared<std::unordered_map<std::string, std::vector<uint8_t>>>();
    (*new_map)[domain] = std::move(ocsp_der);
    ocsp_cache_        = std::shared_ptr<const std::unordered_map<std::string, std::vector<uint8_t>>>(std::move(new_map));
}

std::vector<uint8_t> serverconfig::get_ocsp_staple(const std::string &domain)
{
    std::shared_ptr<const std::unordered_map<std::string, std::vector<uint8_t>>> cache;
    {
        std::lock_guard<std::mutex> lock(ocsp_cache_mutex_);
        cache = ocsp_cache_;
    }
    if (cache)
    {
        auto it = cache->find(domain);
        if (it != cache->end())
            return it->second;
    }
    return {};
}

void serverconfig::set_ocsp_staple_to_ssl(SSL *ssl, const std::string &domain)
{
    if (!ssl)
        return;

    std::shared_ptr<const std::unordered_map<std::string, std::vector<uint8_t>>> cache;
    {
        std::lock_guard<std::mutex> lock(ocsp_cache_mutex_);
        cache = ocsp_cache_;
    }
    if (!cache)
        return;

    auto it = cache->find(domain);
    if (it == cache->end() || it->second.empty())
        return;

    // OCSP 响应通常不超过几 KB，检查防溢出
    if (it->second.size() > static_cast<size_t>(INT_MAX))
        return;

    // SSL_set_tlsext_status_ocsp_resp 接管缓冲区所有权（握手结束后自动 free）
    uint8_t *resp_copy = static_cast<uint8_t *>(OPENSSL_malloc(it->second.size()));
    if (!resp_copy)
        return;

    memcpy(resp_copy, it->second.data(), it->second.size());

    if (SSL_set_tlsext_status_ocsp_resp(ssl, resp_copy, static_cast<int>(it->second.size())) != 1)
    {
        OPENSSL_free(resp_copy);
    }
}

void serverconfig::enable_ocsp_stapling(SSL_CTX *ctx)
{
    if (!ctx)
        return;

    // 注册 OCSP stapling 回调，传入 this 指针作为参数
    SSL_CTX_set_tlsext_status_cb(ctx, ocsp_stapling_cb);
    SSL_CTX_set_tlsext_status_arg(ctx, this);
}

}// namespace http
