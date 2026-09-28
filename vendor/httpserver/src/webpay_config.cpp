// conf/webpay.conf 里的商户配置。由 serverconfig::load_webpay_file() 启动时加载；
// 运行期只有证书页 save_value() 一条写回通道（save_mtx_ 串行 + atomic_write_file）。
// ENABLE_WEBPAY 关闭时整个编译单元为空。
#ifdef ENABLE_WEBPAY

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "webpay_config.h"

namespace http
{
namespace fs = std::filesystem;

// cert_file/key_file 多是相对路径，依次试四档，拼到哪个算哪个：webpay.conf 上一级（项目根）、
// webpay.conf 所在目录、进程当前目录、只取文件名去 webpay.conf 所在目录找（密钥与 conf 平级）。
// 含 "-----BEGIN" 的 PEM 正文和绝对路径直接返回；四档都拼不出来原样返回。
// 最后一档只在 conf 目录找、不去 cwd：线上 cwd 是 /tmp 人人可写，按文件名去 cwd 捡私钥等于
// 允许本机任何进程递一把私钥进来。
static std::string webpay_resolve_path(const std::string &value, const std::string &conffile)
{
    if (value.empty())
    {
        return value;
    }
    // 直接写死 PEM 内容的情况
    if (value.find("-----BEGIN") != std::string::npos)
    {
        return value;
    }
    fs::path rawvalue = fs::path(value);
    if (rawvalue.is_absolute())
    {
        return value;
    }

    std::error_code ec;
    std::vector<fs::path> bases;
    if (!conffile.empty())
    {
        fs::path confabs = fs::absolute(conffile, ec);
        if (!ec)
        {
            bases.push_back(confabs.parent_path().parent_path());// .../项目根
            bases.push_back(confabs.parent_path());              // .../项目根/conf
        }
    }
    bases.push_back(fs::current_path(ec));

    for (const auto &base : bases)
    {
        if (base.empty())
        {
            continue;
        }
        fs::path full = base / rawvalue;
        ec.clear();
        if (fs::exists(full, ec) && fs::is_regular_file(full, ec))
        {
            ec.clear();
            return fs::absolute(full, ec).lexically_normal().string();
        }
    }

    // 最后一档：只取文件名去 conf 目录找（值里本身没目录时是重复，跳过）。
    if (!conffile.empty())
    {
        fs::path confdir = fs::absolute(conffile, ec).parent_path();
        fs::path only    = rawvalue.filename();
        ec.clear();
        if (!confdir.empty() && !only.empty() && only != rawvalue)
        {
            fs::path full = confdir / only;
            if (fs::exists(full, ec) && fs::is_regular_file(full, ec))
            {
                ec.clear();
                return fs::absolute(full, ec).lexically_normal().string();
            }
        }
    }
    return value;
}

// 与 orm.conf 读 ssl/sslverify/debug 的 parse_bool 同一套可接受值（1/true/ON 都算真）。
static bool webpay_parse_bool(const std::string &v)
{
    return v == "1" || v == "true" || v == "True" || v == "TRUE" || v == "On" || v == "ON";
}

webpay_config_t &get_webpay_config()
{
   static  webpay_config_t instance;
   return instance;
}

bool webpay_config_t::load(const std::string &filename)
{
   loaded = false;
   file.clear();
   data.config.clear();

   if (filename.empty())
   {
      return false;
   }
   std::error_code ec;
   if (!fs::exists(filename, ec) || !fs::is_regular_file(filename, ec))
   {
      return false;
   }
   try
   {
      // parse_ini::parse_file 打不开文件会抛 runtime_error，这里拦住，不能让它把启动流程搞崩
      data.parse_file(filename);
   }
   catch (const std::exception &e)
   {
      std::cerr << "[webpay] load config fail: " << filename << " " << e.what() << std::endl;
      return false;
   }
   file   = filename;
   loaded = true;
   return true;
}

std::string webpay_config_t::get(const std::string &section, const std::string &name, const std::string &default_value) const
{
   if (!loaded)
   {
      return default_value;
   }
   // 用 try_* 而不是 operator[]，避免查不到时新建空段
   auto [pSec, sec_found] = data.config.try_section(section);
   if (!sec_found || pSec == nullptr)
   {
      return default_value;
   }
   auto [pVal, key_found] = pSec->try_find(name);
   if (!key_found || pVal == nullptr)
   {
      return default_value;
   }
   return *pVal;
}

// 段名规则 + 缺项回落：tag 空用裸段；tag 非空先取 "<tag>.<kind>"，段存在且关键凭据齐备才用它，
// 否则回落到裸段。实际取用的段名写进返回值的 section 字段。
webpay_merchant_t webpay_config_t::resolve(const std::string &kind, const std::string &tag) const
{
   if (!tag.empty())
   {
      std::string named = section_name(kind, tag);
      webpay_merchant_t tagged = get_merchant(named);
      if (tagged.found() && !webpay_key_missing(tagged, kind))
      {
         return tagged;
      }
      // 回落意味着钱进公共商户号，必须在源头打日志。只在真回落时打：配好的部署不会有这行。
      std::cout << "[webpay] 段 [" << named << "] " << (tagged.found() ? "关键凭据缺项" : "不存在")
                << " ⇒ 回落裸段 [" << kind << "]" << std::endl;
   }
   return get_merchant(kind);
}

bool webpay_config_t::has_section(const std::string &section) const
{
   if (!loaded || section.empty())
   {
      return false;
   }
   auto [pSec, sec_found] = data.config.try_section(section);
   return sec_found && pSec != nullptr;
}

// Host → tag，精确匹配。未命中返回空串 = 用裸段（存量口径）。
std::string webpay_config_t::host_tag(const std::string &host) const
{
   if (host.empty())
   {
      return "";
   }
   return get(WEBPAY_DOMAIN_SECTION, host);
}

webpay_merchant_t webpay_config_t::get_merchant(const std::string &section) const
{
   webpay_merchant_t mch;

   // 段不存在时每个 get() 都返回空串，和"段在但什么都没配"看起来一样；必须能分开——
   // 前者不许换段试。把段名本身记下来供 found() 判定。
   if (!has_section(section))
   {
      return mch;
   }
   mch.section = section;

   mch.appid        = get(section, "appid");
   mch.mch_id       = get(section, "mch_id");
   mch.apikey       = get(section, "apikey");
   mch.secret       = get(section, "secret");
   mch.notifyurl    = get(section, "notifyurl");
   mch.cert_file    = webpay_resolve_path(get(section, "cert_file"), file);
   mch.key_file     = webpay_resolve_path(get(section, "key_file"), file);
   mch.platform_cert_file = webpay_resolve_path(get(section, "platform_cert_file"), file);
   mch.v3_serial_no = get(section, "v3_serial_no");
   mch.api_v3_key   = get(section, "api_v3_key");

   // [alipay] 专用；微信段没有这几个键，get() 返回空串，sandbox 落到 false
   mch.public_key_file = webpay_resolve_path(get(section, "public_key_file"), file);
   mch.returnurl  = get(section, "returnurl");
   mch.content_aes_key = get(section, "api_aes");
   mch.sandbox    = webpay_parse_bool(get(section, "sandbox"));

   return mch;
}

// save_value() 已取 save_mtx_，这里不再加锁。
bool webpay_config_t::save_value_locked(const std::string &section, const std::string &name, const std::string &value, const std::string &comment)
{
   if (!loaded || file.empty())
   {
      return false;
   }
   if (section.empty() || name.empty() || value.empty())
   {
      return false;
   }
   // 同值直接成功不碰文件：写回是整份重写并轮换 .bak，同值重写会让备份堆同一份、
   // 还会搅乱 conf 的 mtime（看不出最后一次真改是什么时候）。
   if (get(section, name) == value)
   {
      return true;
   }
   return data.add_value(section, name, value, comment);
}

bool webpay_config_t::save_value(const std::string &section, const std::string &name, const std::string &value, const std::string &comment)
{
   std::lock_guard<std::mutex> lk(save_mtx_);
   return save_value_locked(section, name, value, comment);
}

std::vector<std::string> webpay_config_t::sections() const
{
   std::vector<std::string> out;
   for (const auto &sec : data.config)
   {
      // 跳过 [[数组]] 和 [domains]（Host→tag 映射，不是商户段）。
      if (!sec.is_array && !sec.name.empty() && sec.name != WEBPAY_DOMAIN_SECTION)
      {
         out.push_back(sec.name);
      }
   }
   return out;
}

}// namespace http
#endif // ENABLE_WEBPAY
