#ifndef __WEB_PAY_CONFIG_H__
#define __WEB_PAY_CONFIG_H__

// 只有开启 ENABLE_WEBPAY 才提供微信/支付宝商户配置能力。
// 关闭时本头文件内容为空，调用方用 #ifdef ENABLE_WEBPAY 决定是否包含/使用。
#ifdef ENABLE_WEBPAY

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "parse_ini.h"

namespace http
{
// conf/webpay.conf 里存 Host→tag 映射的保留段名。不是商户段，sections() 跳过它。
constexpr const char *WEBPAY_DOMAIN_SECTION = "domains";

// conf/webpay.conf 由 serverconfig::load_webpay_file() 在启动阶段加载。
// 写通道只有 save_value()（save_mtx_ 串行 + atomic_write_file 整份重写），读路径不加锁。
// api_v3_key 不走这条通道：只能手改 conf + 重启，不要为它加写回接口。

// 一个段（如 [wxpayv2] [alipay]）对应一个支付商户。
struct webpay_merchant_t
{
    // 实际命中的段名（调用方据此确认这一单走的哪家商户）。空串 = 没取到段。
    std::string section;
    std::string appid;
    std::string mch_id;
    std::string apikey;   // V2 API 密钥
    std::string secret;   // 小程序/公众号 AppSecret（jscode2session、获取 access_token 用）
    std::string cert_file;// 商户证书
    std::string key_file; // 商户私钥
    std::string notifyurl;
    std::string v3_serial_no;      // V3 商户 API 证书序列号（不是平台证书序列号）
    std::string api_v3_key;        // V3 API 密钥，32 位
    std::string platform_cert_file;// V3 微信支付平台证书，回调验签用

    // 以下 [alipay] 段专用，微信段忽略。复用 appid=app_id, key_file=应用私钥, notifyurl=notify_url。
    std::string public_key_file;// 支付宝公钥，路径 / PEM 文本 / 裸 base64，回调验签用
    std::string returnurl;      // 页面支付同步跳转地址
    // 后台"接口内容加密方式"的 Base64 密钥（解码 16 字节 = AES-128）。
    // AES/CBC/PKCS5Padding + IV 全 0 + 密文纯 Base64。
    std::string content_aes_key;
    bool sandbox = false;// true 走沙箱网关

    bool empty() const { return appid.empty() && mch_id.empty(); }
    // 取到了段吗。没有段就没有商户配置，调用方必须据此拒绝下单/拒绝落账。
    bool found() const { return !section.empty(); }
};

struct webpay_config_t
{
    parse_ini data;
    std::string file;// 实际加载的 webpay.conf 路径
    bool loaded = false;

    mutable std::mutex save_mtx_;// save_value() 串行

    // 加载 conf/webpay.conf。文件不存在返回 false，不抛也不创建。
    bool load(const std::string &filename);
    bool is_load() const { return loaded; }

    // 取 [section] name 的值，找不到返回 default_value；只读，不创建空段。
    std::string get(const std::string &section, const std::string &name, const std::string &default_value = "") const;

    // 段存在吗（含 [domains] 这类非商户段）；只读，不创建空段。
    bool has_section(const std::string &section) const;

    // 取整段商户配置，相对路径规整成绝对路径。
    webpay_merchant_t get_merchant(const std::string &section) const;

    // ===== 多域名：一个域名一套商户 =====
    // 段名规则 [<tag>.<kind>]：tag 空用裸段；tag 非空先取 "<tag>.<kind>"，段不存在或关键凭据
    // 缺项（webpay_key_missing）才回落到裸段。命中还是回落，看返回值的 section 字段。
    // 业务路由选商户只走带 tag 的 resolve()，不要自己拼段名。
    static std::string section_name(const std::string &kind, const std::string &tag)
    {
        return tag.empty() ? kind : tag + "." + kind;
    }
    // Host -> tag，精确匹配（http_parse 层已做小写 + 剥端口）。未命中返回空串。
    std::string host_tag(const std::string &host) const;

    webpay_merchant_t wxpayv2() const { return get_merchant("wxpayv2"); }
    webpay_merchant_t wxpayv3() const { return get_merchant("wxpayv3"); }
    webpay_merchant_t wxpayv3_web() const { return get_merchant("wxpayv3_web"); }
    webpay_merchant_t alipay() const { return get_merchant("alipay"); }

    // 带 tag 的解析版（resolve 内部处理段名拼接 + 缺项回落）。
    webpay_merchant_t wxpayv2(const std::string &tag) const { return resolve("wxpayv2", tag); }
    webpay_merchant_t wxpayv3(const std::string &tag) const { return resolve("wxpayv3", tag); }
    webpay_merchant_t wxpayv3_web(const std::string &tag) const { return resolve("wxpayv3_web", tag); }
    webpay_merchant_t alipay(const std::string &tag) const { return resolve("alipay", tag); }

  private:
    // 段名规则 + 缺项回落的唯一实现（所有带 tag 的访问器都走它）。
    webpay_merchant_t resolve(const std::string &kind, const std::string &tag) const;
    // save_value() 内部复用，持 save_mtx_ 时调用。
    bool save_value_locked(const std::string &section, const std::string &name, const std::string &value, const std::string &comment);

  public:
    // 把任意字段写回 conf/webpay.conf（保留注释）；load 成功过才允许写，value 为空不写。
    bool save_value(const std::string &section, const std::string &name, const std::string &value, const std::string &comment = "");

    // 列出所有普通商户段名（跳过 [[数组]] 和 [domains]），供配置页做段切换导航。
    std::vector<std::string> sections() const;
};

webpay_config_t &get_webpay_config();

// 商户配置字段占位符判定（空串/典型模板占位串）。调用方据此拒绝向网关发请求。
inline bool webpay_is_placeholder(const std::string &v)
{
    if (v.empty())
        return true;
    if (v.find("你的") != std::string::npos)
        return true;
    if (v.find("商户id") != std::string::npos)
        return true;
    if (v.find("小程序id") != std::string::npos)
        return true;
    if (v.find("apiv2_key") != std::string::npos)
        return true;
    if (v.find("xxx") != std::string::npos || v.find("XXX") != std::string::npos)
        return true;
    if (v.find("yourdomain") != std::string::npos || v.find("myweixin") != std::string::npos)
        return true;
    return false;
}

// 关键凭据缺项判定：resolve() 在段存在但字段不全时据此回落到裸段。
// V3 要 appid/mch_id/key_file/cert_file/v3_serial_no；V2 要 appid/mch_id/apikey；
// 支付宝要 appid/key_file/public_key_file。未知 kind 保守放行。
inline bool webpay_key_missing(const webpay_merchant_t &m, const std::string &kind)
{
    if (kind == "wxpayv2")
        return webpay_is_placeholder(m.appid) || webpay_is_placeholder(m.mch_id) || webpay_is_placeholder(m.apikey);
    if (kind == "wxpayv3" || kind == "wxpayv3_web")
        return webpay_is_placeholder(m.appid) || webpay_is_placeholder(m.mch_id) || webpay_is_placeholder(m.key_file) ||
               webpay_is_placeholder(m.cert_file) || webpay_is_placeholder(m.v3_serial_no);
    if (kind == "alipay")
        return webpay_is_placeholder(m.appid) || webpay_is_placeholder(m.key_file) || webpay_is_placeholder(m.public_key_file);
    return false;
}

// 元字符串 → 分（正数，最多两位小数）。纯字符串切分，不走 stof/stod 避免浮点误差。
// 溢出饱和到 UINT_MAX 而不是回绕：回绕会把"金额巨大"变成一个小而看起来合法的数；饱和之后
// 调用点都判不过（can_refund 拒回、回调金额比对不上 ⇒ 不落账）。
// 刻意不返回 0：0 在 resolve_refund_fen() 里是"按订单金额全额退"。
inline unsigned int yuan_to_fen(const std::string &yuan)
{
    constexpr unsigned long long kMaxFen = 0xFFFFFFFFULL;
    // integer 用 64 位存，累加时钳到 kAccCap：这一钳只防 64 位自身溢出，
    // "装不进 unsigned int 分"由末尾那一次钳统一处理，所以两种溢出都饱和到同一个值。
    constexpr unsigned long long kAccCap = kMaxFen / 10;
    unsigned long long integer           = 0;
    unsigned int frac                    = 0;
    std::size_t i                        = 0;
    for (; i < yuan.size() && yuan[i] != '.'; i++)
        if (yuan[i] >= '0' && yuan[i] <= '9')
        {
            if (integer > kAccCap)
            {
                integer = kAccCap;
                continue;
            }
            integer = integer * 10 + (unsigned long long)(yuan[i] - '0');
        }
    if (integer > kAccCap)
        integer = kAccCap;
    if (i < yuan.size() && yuan[i] == '.')
    {
        unsigned int digits = 0;
        for (i++; i < yuan.size() && digits < 2; i++)
            if (yuan[i] >= '0' && yuan[i] <= '9')
            {
                frac = frac * 10 + (unsigned int)(yuan[i] - '0');
                digits++;
            }
        if (digits == 1)
            frac *= 10;// 一位小数是十分位，不是百分位
    }
    const unsigned long long fen = integer * 100 + frac;
    return (unsigned int)(fen > kMaxFen ? kMaxFen : fen);
}

}//namespace http
#endif// ENABLE_WEBPAY
#endif
