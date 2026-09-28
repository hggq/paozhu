#include <sstream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <ctime>
#include <cstdio>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <iostream>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "httpclient.h"
#include "base64.h"
#include "func.h"
#include "urlcode.h"
#include "alipay.h"

namespace pay
{

alipay::alipay() : product_code_("QUICK_MSECURITY_PAY"), is_sandbox_(false) {}

std::string alipay::getTimestamp()
{
    // 支付宝只接受北京时间并校验时间窗，服务器时区/夏令时不能参与；
    // 且 std::localtime 返回静态缓冲，本函数在业务线程里跑，不能用。
    std::time_t now = std::time(nullptr) + 8 * 3600;
    std::tm tmv;
#ifdef _WIN32
    gmtime_s(&tmv, &now);
#else
    gmtime_r(&now, &tmv);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tmv);
    return buffer;
}

std::string alipay::getApiUrl()
{
    if (is_sandbox_) {
        return "https://openapi-sandbox.dl.alipaydev.com/gateway.do";
    }
    return "https://openapi.alipay.com/gateway.do";
}

std::string alipay::buildSignContent(const std::map<std::string, std::string>& params, bool exclude_sign_type)
{
    std::string result;
    // std::map 本身就按 key 有序，直接遍历拼接，不要再深拷贝一份来排序
    for (const auto& p : params) {
        if (p.second.empty() || p.first == "sign") continue;
        if (exclude_sign_type && p.first == "sign_type") continue;
        if (!result.empty()) result += "&";
        result += p.first + "=" + p.second;
    }
    return result;
}

// 密钥值支持三态，按"先当路径、后当正文"的固定次序判：
//   1) 含 "-----BEGIN" 的 PEM 文本（已带 armor）
//   2) 指向一个存在的常规文件（PEM 或裸 base64 都行）
//   3) 裸 base64 正文，直接写在配置里
// 只做"补 armor 行"这一件事，base64 解码交给 OpenSSL 的 PEM 层。于是"非法 base64"与
// "路径不存在"落到同一个出口（读不出密钥 -> 空签名），不需要再为值加字符集或长度校验。
// 私钥在 PKCS#8 解析失败后会再按 PKCS#1 试一次，兼容两种常见格式。
static std::string alipay_slurp(const std::string &path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// 密钥文件正文缓存：配置进内存的只有路径串，这里按路径把正文留住，一个进程只读一次盘。
// ⚠ 失效只有重启这一条：换过同一路径下的密钥文件，本进程继续用第一次成功解析的那份正文，
//   线上轮换密钥必须重启 http 服务器才生效。
static std::shared_mutex g_keyfile_mtx;
static std::unordered_map<std::string, std::string> g_keyfile_text;

// true = 命中缓存，text 是正文；false = 未命中，调用方自己去读盘
static bool alipay_keyfile_get(const std::string &path, std::string &text)
{
    const std::shared_lock lk(g_keyfile_mtx);
    auto it = g_keyfile_text.find(path);
    if (it == g_keyfile_text.end()) return false;
    text = it->second;
    return true;
}

// 只在"这一份正文确实解析出了密钥"之后调用：正在被替换的文件可能只读到半截，
// 那一刻的失败不该把这条通路钉住到重启为止，所以下一次照样重读。
static void alipay_keyfile_put(const std::string &path, const std::string &text)
{
    const std::unique_lock lk(g_keyfile_mtx);
    g_keyfile_text.emplace(path, text);
}

static std::string alipay_armour_pem(const std::string &raw, const char *label)
{
    std::string b64;
    b64.reserve(raw.size());
    for (const char c : raw)
    {
        if (c == '\r' || c == '\n' || c == ' ' || c == '\t') continue;
        b64.push_back(c);
    }
    if (b64.empty()) return "";

    std::string out = "-----BEGIN ";
    out += label;
    out += "-----\n";
    for (std::size_t i = 0; i < b64.size(); i += 64)
    {
        out += b64.substr(i, 64);
        out += '\n';
    }
    out += "-----END ";
    out += label;
    out += "-----\n";
    return out;
}

static EVP_PKEY *alipay_pkey_from_pem(const std::string &pem, bool is_private)
{
    if (pem.empty()) return nullptr;

    BIO *bio = BIO_new_mem_buf(pem.data(), pem.size());
    if (!bio) return nullptr;
    EVP_PKEY *pkey = nullptr;
    if (is_private) pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    else            pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return pkey;
}

static EVP_PKEY *alipay_pkey_from_bare_base64(const std::string &b64, bool is_private)
{
    if (!is_private)
    {
        // 公钥只认 X.509 SubjectPublicKeyInfo（后台与工具给的形态），PKCS#1 公钥不试。
        return alipay_pkey_from_pem(alipay_armour_pem(b64, "PUBLIC KEY"), false);
    }
    EVP_PKEY *pkey = alipay_pkey_from_pem(alipay_armour_pem(b64, "PRIVATE KEY"), true);
    if (!pkey)
    {
        pkey = alipay_pkey_from_pem(alipay_armour_pem(b64, "RSA PRIVATE KEY"), true);
    }
    return pkey;
}

static EVP_PKEY *alipay_read_key(const std::string &value, bool is_private)
{
    if (value.empty()) return nullptr;

    if (value.find("-----BEGIN") != std::string::npos)
    {
        return alipay_pkey_from_pem(value, is_private);
    }

    std::string text;
    bool        cached = alipay_keyfile_get(value, text);
    if (!cached)
    {
        std::error_code ec;
        if (!std::filesystem::is_regular_file(value, ec))
        {
            // 第三态：裸 base64 正文直接写在配置里。这一档本来就不碰盘，没有正文可留，
            // 所以它不进缓存 —— 缓存只管"值是文件路径"那一档。
            return alipay_pkey_from_bare_base64(value, is_private);
        }
        text = alipay_slurp(value);
        if (text.empty()) return nullptr;
    }

    EVP_PKEY *pkey = (text.find("-----BEGIN") != std::string::npos)
                       ? alipay_pkey_from_pem(text, is_private)
                       : alipay_pkey_from_bare_base64(text, is_private);
    if (pkey && !cached) alipay_keyfile_put(value, text);
    return pkey;
}

bool alipay::privateKeyReadable() const
{
    EVP_PKEY* pkey = alipay_read_key(private_key_, true);
    if (!pkey) return false;
    EVP_PKEY_free(pkey);
    return true;
}

bool alipay::publicKeyReadable() const
{
    EVP_PKEY* pkey = alipay_read_key(public_key_, false);
    if (!pkey) return false;
    EVP_PKEY_free(pkey);
    return true;
}

std::string alipay::rsaSign(const std::string& content)
{
    EVP_PKEY* pkey = alipay_read_key(private_key_, true);

    if (!pkey) return "";

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return "";
    }

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return "";
    }

    if (EVP_DigestSignUpdate(ctx, content.data(), content.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return "";
    }

    size_t sig_len = 0;
    if (EVP_DigestSignFinal(ctx, nullptr, &sig_len) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return "";
    }

    std::vector<unsigned char> sig(sig_len);
    if (EVP_DigestSignFinal(ctx, sig.data(), &sig_len) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return "";
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    // 长度取 sig.size()：sig_len 是上面 EVP_DigestSignFinal 的入出参，不能当编码器输入。
    return http::base64_encode((const char*)sig.data(), (unsigned int)sig.size());
}

bool alipay::rsaVerify(const std::string& content, const std::string& sign)
{
    if (sign.empty()) return false;

    EVP_PKEY* pkey = alipay_read_key(public_key_, false);

    if (!pkey) return false;

    // 框架的 base64_decode（base64.h）遇到不在字母表里的字符就停下，非法签名解出截断内容、验签自然失败。
    std::string decoded = http::base64_decode(sign.data(), (unsigned int)sign.size());

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return false;
    }

    bool result = false;
    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) == 1) {
        if (EVP_DigestVerifyUpdate(ctx, content.data(), content.size()) == 1) {
            if (EVP_DigestVerifyFinal(ctx, (const unsigned char*)decoded.data(), decoded.size()) == 1) {
                result = true;
            }
        }
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return result;
}

std::string alipay::postToUrl(const std::string& url, const std::string& data)
{
    std::string response;
    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->post(url);
    a->set_header("Content-Type", "application/x-www-form-urlencoded");
    a->set_body(data);
    a->timeout(8);// 8s 是全渠道统一档：30s 只会把慢网关放大成一堆占死的线程
    a->send();

    if (a->get_status() == 200) {
        response = a->get_body();
    } else {
        response = "{\"code\":\"-1\",\"msg\":\"HTTP Error: " + std::to_string(a->get_status()) + "\"}";
    }

    return response;
}

// 与 postToUrl() 同形，只把 send() 换成 co_await async_send()：报文逐字节不变，等的时候不占线程。
asio::awaitable<std::string> alipay::async_post_to_url(const std::string& url, const std::string& data)
{
    std::string response;
    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->post(url);
    a->set_header("Content-Type", "application/x-www-form-urlencoded");
    a->set_body(data);
    a->timeout(8);
    co_await a->async_send();

    if (a->get_status() == 200) {
        response = a->get_body();
    } else {
        response = "{\"code\":\"-1\",\"msg\":\"HTTP Error: " + std::to_string(a->get_status()) + "\"}";
    }

    co_return response;
}

std::string alipay::buildRequest(const std::string& method, const std::map<std::string, std::string>& biz_content_map)
{
    std::map<std::string, std::string> params;
    params["app_id"] = app_id_;
    params["method"] = method;
    params["charset"] = "UTF-8";
    params["sign_type"] = "RSA2";
    params["timestamp"] = getTimestamp();
    params["version"] = "1.0";

    if (!app_auth_token_.empty()) {
        params["app_auth_token"] = app_auth_token_;
    }

    // 这两个是公共请求参数（顶层），不是 biz_content 里的业务参数。塞在 biz_content 里网关会静默
    // 忽略：页面支付付完后收银台的 qrPaySuccGotoURL 是空的，同步跳转不发生。
    if (!notify_url_.empty()) {
        params["notify_url"] = notify_url_;
    }
    if (!return_url_.empty()) {
        params["return_url"] = return_url_;
    }

    std::string biz_content = "{";
    for (const auto& p : biz_content_map) {
        if (!biz_content.empty() && biz_content != "{") biz_content += ",";
        biz_content += "\"" + p.first + "\":\"" + http::json_escape(p.second) + "\"";
    }
    biz_content += "}";
    params["biz_content"] = biz_content;

    // 请求签名口径：只有 sign 不参与，sign_type 要参与
    std::string sign_content = buildSignContent(params, false);
    std::string sign = rsaSign(sign_content);
    params["sign"] = sign;

    std::string request_data;
    for (const auto& p : params) {
        if (!request_data.empty()) request_data += "&";
        request_data += p.first + "=" + http::url_encode(p.second.data(), p.second.size());
    }

    last_sign_content_ = sign_content;
    last_request_body_ = request_data;

    return request_data;
}

std::string alipay::createTrade()
{
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = out_trade_no_;
    biz_content["subject"] = subject_;
    biz_content["body"] = body_;
    biz_content["total_amount"] = total_amount_;
    biz_content["product_code"] = product_code_;
    if (!auth_token_.empty()) biz_content["auth_token"] = auth_token_;

    std::string request_data = buildRequest("alipay.trade.app.pay", biz_content);
    return request_data;
}

std::string alipay::createTradeQRCode()
{
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = out_trade_no_;
    biz_content["subject"] = subject_;
    biz_content["total_amount"] = total_amount_;

    std::string request_data = buildRequest("alipay.trade.precreate", biz_content);
    std::string url = getApiUrl();
    std::string response = postToUrl(url, request_data);

    return response;
}

std::string alipay::createTradePage()
{
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = out_trade_no_;
    biz_content["subject"] = subject_;
    biz_content["body"] = body_;
    biz_content["total_amount"] = total_amount_;
    biz_content["product_code"] = "FAST_INSTANT_TRADE_PAY";
    // 空则连键都不发：整串 biz_content 参与签名，多一个键就换一份签名，
    // 未开启该模式的调用点必须保持不带这个键的报文。
    if (!qr_pay_mode_.empty()) biz_content["qr_pay_mode"] = qr_pay_mode_;
    if (!qrcode_width_.empty()) biz_content["qrcode_width"] = qrcode_width_;

    std::string request_data = buildRequest("alipay.trade.page.pay", biz_content);
    std::string url = getApiUrl();
    
    return url + "?" + request_data;
}

std::string alipay::queryTrade(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = trade_no;

    std::string request_data = buildRequest("alipay.trade.query", biz_content);
    std::string url = getApiUrl();
    std::string response = postToUrl(url, request_data);

    return response;
}

std::string alipay::cancelTrade(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = trade_no;

    std::string request_data = buildRequest("alipay.trade.cancel", biz_content);
    std::string url = getApiUrl();
    std::string response = postToUrl(url, request_data);

    return response;
}

std::string alipay::refundTrade(const std::string& refund_amount, const std::string& refund_reason, const std::string& out_trade_no)
{
    // 幂等键必须显式给：空则不外发。应答形状照网关的错误报文（wrapper 里带 code），
    // 调用方那句 code=="10000" 天然读到失败。
    if (out_request_no_.empty())
    {
        return "{\"alipay_trade_refund_response\":{\"code\":\"40004\",\"msg\":\"Business Failed\","
               "\"sub_code\":\"isv.out-request-no-empty\",\"sub_msg\":\"out_request_no is empty\"}}";
    }

    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = trade_no;
    biz_content["refund_amount"] = refund_amount;
    if (!refund_reason.empty()) biz_content["refund_reason"] = refund_reason;
    // 上面的闸门已保证非空，这个键必须发：省略出去会退化成"按 out_trade_no 全额退"。
    biz_content["out_request_no"] = out_request_no_;

    std::string request_data = buildRequest("alipay.trade.refund", biz_content);
    std::string url = getApiUrl();
    std::string response = postToUrl(url, request_data);

    return response;
}

// ↓ 协程版孪生：报文组装与同步版一致，只把网关那一跳换成 async_post_to_url()。
//   只有 query 有协程调用点，其余四个同步方法没有孪生。
asio::awaitable<std::string> alipay::async_query_trade(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;

    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = trade_no;

    std::string request_data = buildRequest("alipay.trade.query", biz_content);
    std::string url = getApiUrl();
    co_return co_await async_post_to_url(url, request_data);
}

// 回调报文（异步通知的 body、同步回跳的 querystring）都是 urlencoded，且值要**解码后**才参与验签。
// 这个解析是手写的：只按 '=' 和 '&' 切，没有"没有 '=' 的残段"这种分支。
static std::map<std::string, std::string> alipay_parse_form(const std::string& body)
{
    std::map<std::string, std::string> params;
    size_t pos = 0;

    while (pos < body.size()) {
        size_t eq_pos = body.find('=', pos);
        if (eq_pos == std::string::npos) break;

        std::string key = body.substr(pos, eq_pos - pos);
        pos = eq_pos + 1;

        size_t amp_pos = body.find('&', pos);
        std::string value;
        if (amp_pos != std::string::npos) {
            value = body.substr(pos, amp_pos - pos);
            pos = amp_pos + 1;
        } else {
            value = body.substr(pos);
            pos = body.size();
        }

        std::string decoded_value;
        for (size_t i = 0; i < value.size();) {
            if (value[i] == '%' && i + 2 < value.size()) {
                char hex[3] = {value[i + 1], value[i + 2], '\0'};
                decoded_value += (char)std::strtol(hex, nullptr, 16);
                i += 3;
            } else if (value[i] == '+') {
                decoded_value += ' ';
                i++;
            } else {
                decoded_value += value[i];
                i++;
            }
        }

        params[key] = decoded_value;
    }

    return params;
}

// 两种回调的前半段是同一件事：验签（口径与请求签名不同，sign 和 sign_type 都不参与）。
// 状态不为 0 时也把已解析的参数原样带回去，红行才不会是"不知道哪一单"。
std::map<std::string, std::string> alipay::verifySignedParams(std::map<std::string, std::string> params)
{
    std::string sign = params["sign"];

    if (sign.empty()) {
        params["status_code"] = "1";
        params["error_msg"] = "sign is empty";
        return params;
    }

    if (!rsaVerify(buildSignContent(params, true), sign)) {
        params["status_code"] = "2";
        params["error_msg"] = "sign verify failed";
        return params;
    }

    params["status_code"] = "0";
    params["error_msg"] = "success";
    return params;
}

std::map<std::string, std::string> alipay::handleNotify(const std::string& postData)
{
    std::map<std::string, std::string> result = verifySignedParams(alipay_parse_form(postData));
    if (result["status_code"] != "0") return result;

    std::string trade_status = result["trade_status"];
    if (trade_status != "TRADE_SUCCESS" && trade_status != "TRADE_FINISHED") {
        result["status_code"] = "3";
        result["error_msg"] = "trade_status is not success";
    }
    return result;
}

std::map<std::string, std::string> alipay::handleReturn(const std::string& queryString)
{
    return verifySignedParams(alipay_parse_form(queryString));
}

}