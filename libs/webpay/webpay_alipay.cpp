#include <filesystem>
#include <string>
#include "webpay_alipay.h"

namespace http
{
namespace webpay
{

// 网关应答是 {"alipay_xxx_response":{"code":…}} 这种包了一层的形状，取字段要按包装键走。
// 取不到就回空串 —— 列表把它显示成"取消失败(应答里没有 code)"，比假装成功有用。
// from_json 遇到非 JSON（网关的 HTML 错误页）会抛 json_parse_error，这里接住当"取不到"。
std::string alipay_resp_field(const std::string &response, const std::string &wrapper,
                              const std::string &field)
{
    http::obj_val json;
    try
    {
        json.from_json(response);
    }
    catch (const std::exception &)
    {
        return "";
    }
    if (!json[wrapper].is_object()) return "";
    return json[wrapper][field].to_string();
}

#ifdef ENABLE_WEBPAY

bool alipay_fill(pay::alipay &apay, const std::string &tag, std::string &msg,
                 bool need_private_key)
{
    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mch = cfg.alipay(tag);

    const std::string appid     = mch.appid;
    const std::string notifyurl = mch.notifyurl;

    if (appid.empty() || mch.key_file.empty())
    {
        msg = "支付宝未配置：缺 appid 或应用私钥。配置来源 ";
        msg += cfg.is_load() ? cfg.file : "conf/webpay.conf（未加载）";
        msg += " 的 [";
        msg += mch.section.empty() ? std::string("alipay") : mch.section;// 点名实际取用的段，别让人去改没生效的那一段
        msg += "] 段";
        return false;
    }

    // 值写成目录时下面所有签名都会静默变成空 sign，所以在这里点名，别让人去查签名口径。
    const std::filesystem::path keys[2] = {std::filesystem::path(mch.key_file),
                                           std::filesystem::path(mch.public_key_file)};
    const char *knames[2]               = {"key_file", "public_key_file"};
    for (int i = 0; i < 2; i++)
    {
        std::error_code ec;
        if (std::filesystem::is_directory(keys[i], ec))
        {
            msg  = "支付宝配置不对：";
            msg += knames[i];
            msg += " 指向的是目录：";
            msg += keys[i].string();
            msg += " ；请填密钥本身（文件路径 / PEM 文本 / 裸 base64 一行），程序不会去目录里猜哪个是私钥";
            return false;
        }
    }

    apay.setAppId(appid);
    apay.setPrivateKey(mch.key_file);
    apay.setPublicKey(mch.public_key_file);
    apay.setNotifyUrl(notifyurl);
    apay.setReturnUrl(mch.returnurl);
    apay.setSandbox(mch.sandbox);

    // 闸门只认"私钥正文真的加载得出来"，不认"路径非空"：路径写得漂亮但盘上没有这种值，
    // 签出来是空 sign、请求照样发往网关。只卡私钥不卡公钥：公钥仅用于验签回调，
    // 存量部署留空是正常配置，卡它会断回调落账。
    if (need_private_key && !apay.privateKeyReadable())
    {
        msg = "支付宝配置不对：key_file 加载不出私钥（值是 [";
        msg += mch.key_file;
        msg += "]，三态都试过：PEM 文本 / PEM 文件 / 裸 base64 一行）。配置来源 ";
        msg += cfg.is_load() ? cfg.file : "conf/webpay.conf（未加载）";
        msg += " 的 [";
        msg += mch.section.empty() ? std::string("alipay") : mch.section;
        msg += "] 段。本次不会向支付宝发出任何请求 —— 空 sign 的报文发过去也只会被拒。";
        return false;
    }
    return true;
}

#endif// ENABLE_WEBPAY

}  // namespace webpay
}  // namespace http
