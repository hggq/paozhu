#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "httpclient.h"
#include "func.h"
#include "urlcode.h"
#include "alipay.h"

namespace pay
{

alipay::alipay() : is_sandbox_(false), product_code_("QUICK_MSECURITY_PAY") {}

alipay::alipay(std::map<std::string, std::string>&& params) : is_sandbox_(false), product_code_("QUICK_MSECURITY_PAY")
{
    auto it = params.find("app_id");
    if (it != params.end()) app_id_ = it->second;
    it = params.find("private_key");
    if (it != params.end()) private_key_ = it->second;
    it = params.find("public_key");
    if (it != params.end()) public_key_ = it->second;
    it = params.find("out_trade_no");
    if (it != params.end()) out_trade_no_ = it->second;
    it = params.find("subject");
    if (it != params.end()) subject_ = it->second;
    it = params.find("body");
    if (it != params.end()) body_ = it->second;
    it = params.find("total_amount");
    if (it != params.end()) total_amount_ = it->second;
    it = params.find("notify_url");
    if (it != params.end()) notify_url_ = it->second;
    it = params.find("return_url");
    if (it != params.end()) return_url_ = it->second;
    it = params.find("product_code");
    if (it != params.end()) product_code_ = it->second;
    it = params.find("auth_token");
    if (it != params.end()) auth_token_ = it->second;
    it = params.find("app_auth_token");
    if (it != params.end()) app_auth_token_ = it->second;
    it = params.find("sandbox");
    if (it != params.end() && (it->second == "1" || it->second == "true")) is_sandbox_ = true;
}

std::string alipay::generateNonceStr()
{
    static const char* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 61);
    std::string nonce;
    for (int i = 0; i < 32; ++i) {
        nonce += chars[dis(gen)];
    }
    return nonce;
}

std::string alipay::getTimestamp()
{
    std::time_t now = std::time(nullptr);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buffer;
}

std::string alipay::getApiUrl(const std::string& method)
{
    if (is_sandbox_) {
        return "https://openapi-sandbox.dl.alipaydev.com/gateway.do";
    }
    return "https://openapi.alipay.com/gateway.do";
}

std::string alipay::buildSignContent(const std::map<std::string, std::string>& params)
{
    std::string result;
    std::map<std::string, std::string> sorted_params(params.begin(), params.end());
    for (const auto& p : sorted_params) {
        if (p.second.empty() || p.first == "sign" || p.first == "sign_type") continue;
        if (!result.empty()) result += "&";
        result += p.first + "=" + p.second;
    }
    return result;
}

std::string alipay::rsaSign(const std::string& content)
{
    if (private_key_.empty()) return "";

    std::string key_str = private_key_;
    if (key_str.find("-----BEGIN") == std::string::npos) {
        key_str = "-----BEGIN PRIVATE KEY-----\n" + key_str + "\n-----END PRIVATE KEY-----";
    }

    BIO* bio = BIO_new_mem_buf(key_str.data(), key_str.size());
    if (!bio) return "";

    EVP_PKEY* pkey = nullptr;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
#else
    RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
    if (rsa) {
        pkey = EVP_PKEY_new();
        EVP_PKEY_assign_RSA(pkey, rsa);
    }
#endif
    BIO_free(bio);

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

    static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0, j = 0;
    unsigned char char_array_3[3], char_array_4[4];

    while (sig_len--) {
        char_array_3[i++] = sig[j++];
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (i = 0; i < 4; i++) result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) char_array_3[j] = 0;
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; j < i + 1; j++) result += base64_chars[char_array_4[j]];
        while (i++ < 3) result += '=';
    }

    return result;
}

bool alipay::rsaVerify(const std::string& content, const std::string& sign)
{
    if (public_key_.empty() || sign.empty()) return false;

    std::string key_str = public_key_;
    if (key_str.find("-----BEGIN") == std::string::npos) {
        key_str = "-----BEGIN PUBLIC KEY-----\n" + key_str + "\n-----END PUBLIC KEY-----";
    }

    BIO* bio = BIO_new_mem_buf(key_str.data(), key_str.size());
    if (!bio) return false;

    EVP_PKEY* pkey = nullptr;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
#else
    RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
    if (rsa) {
        pkey = EVP_PKEY_new();
        EVP_PKEY_assign_RSA(pkey, rsa);
    }
#endif
    BIO_free(bio);

    if (!pkey) return false;

    std::vector<unsigned char> decoded;
    int in_len = sign.size();
    int i = 0, j = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len--) {
        if (sign[j] == '=') break;
        unsigned char c = (unsigned char)sign[j++];
        if (c >= 'A' && c <= 'Z') c = c - 'A';
        else if (c >= 'a' && c <= 'z') c = c - 'a' + 26;
        else if (c >= '0' && c <= '9') c = c - '0' + 52;
        else if (c == '+') c = 62;
        else if (c == '/') c = 63;
        char_array_4[i++] = c;
        if (i == 4) {
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; i < 3; i++) decoded.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) char_array_4[j] = 0;
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (j = 0; j < i - 1; j++) decoded.push_back(char_array_3[j]);
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return false;
    }

    bool result = false;
    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) == 1) {
        if (EVP_DigestVerifyUpdate(ctx, content.data(), content.size()) == 1) {
            if (EVP_DigestVerifyFinal(ctx, decoded.data(), decoded.size()) == 1) {
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
    a->timeout(30);
    a->send();

    if (a->get_status() == 200) {
        response = a->get_body();
    } else {
        response = "{\"code\":\"-1\",\"msg\":\"HTTP Error: " + std::to_string(a->get_status()) + "\"}";
    }

    return response;
}

std::map<std::string, std::string> alipay::parseResponse(const std::string& response)
{
    std::map<std::string, std::string> result;
    size_t start = response.find('{');
    size_t end = response.rfind('}');
    
    if (start == std::string::npos || end == std::string::npos || start > end) {
        result["code"] = "-1";
        result["msg"] = "Invalid response format";
        return result;
    }

    std::string json_str = response.substr(start, end - start + 1);
    
    size_t pos = 0;
    while (pos < json_str.size()) {
        size_t key_start = json_str.find('"', pos);
        if (key_start == std::string::npos) break;
        key_start++;
        
        size_t key_end = json_str.find('"', key_start);
        if (key_end == std::string::npos) break;
        std::string key = json_str.substr(key_start, key_end - key_start);
        
        pos = key_end + 1;
        while (pos < json_str.size() && (json_str[pos] == ':' || json_str[pos] == ' ' || json_str[pos] == '\n')) pos++;
        
        std::string value;
        if (pos < json_str.size() && json_str[pos] == '"') {
            pos++;
            size_t value_end = pos;
            while (value_end < json_str.size()) {
                if (json_str[value_end] == '\\' && value_end + 1 < json_str.size()) {
                    value_end += 2;
                    continue;
                }
                if (json_str[value_end] == '"') break;
                value_end++;
            }
            value = json_str.substr(pos, value_end - pos);
            pos = value_end + 1;
        } else {
            size_t value_end = json_str.find_first_of(",}]", pos);
            if (value_end == std::string::npos) value_end = json_str.size();
            value = json_str.substr(pos, value_end - pos);
            while (!value.empty() && (value.back() == ' ' || value.back() == '\n')) value.pop_back();
            pos = value_end;
        }
        
        result[key] = value;
    }

    return result;
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
    params["nonce_str"] = generateNonceStr();

    if (!app_auth_token_.empty()) {
        params["app_auth_token"] = app_auth_token_;
    }

    std::string biz_content = "{";
    for (const auto& p : biz_content_map) {
        if (!biz_content.empty() && biz_content != "{") biz_content += ",";
        biz_content += "\"" + p.first + "\":\"" + http::json_escape(p.second) + "\"";
    }
    biz_content += "}";
    params["biz_content"] = biz_content;

    std::string sign_content = buildSignContent(params);
    std::string sign = rsaSign(sign_content);
    params["sign"] = sign;

    std::string request_data;
    for (const auto& p : params) {
        if (!request_data.empty()) request_data += "&";
        request_data += p.first + "=" + http::url_encode(p.second.data(), p.second.size());
    }

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
    if (!notify_url_.empty()) biz_content["notify_url"] = notify_url_;
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
    if (!notify_url_.empty()) biz_content["notify_url"] = notify_url_;

    std::string request_data = buildRequest("alipay.trade.precreate", biz_content);
    std::string url = getApiUrl("alipay.trade.precreate");
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
    if (!notify_url_.empty()) biz_content["notify_url"] = notify_url_;
    if (!return_url_.empty()) biz_content["return_url"] = return_url_;

    std::string request_data = buildRequest("alipay.trade.page.pay", biz_content);
    std::string url = getApiUrl("alipay.trade.page.pay");
    
    return url + "?" + request_data;
}

std::string alipay::queryTrade(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = trade_no;

    std::string request_data = buildRequest("alipay.trade.query", biz_content);
    std::string url = getApiUrl("alipay.trade.query");
    std::string response = postToUrl(url, request_data);

    return response;
}

std::string alipay::cancelTrade(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = trade_no;

    std::string request_data = buildRequest("alipay.trade.cancel", biz_content);
    std::string url = getApiUrl("alipay.trade.cancel");
    std::string response = postToUrl(url, request_data);

    return response;
}

std::string alipay::refundTrade(const std::string& refund_amount, const std::string& refund_reason, const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = trade_no;
    biz_content["refund_amount"] = refund_amount;
    if (!refund_reason.empty()) biz_content["refund_reason"] = refund_reason;

    std::string request_data = buildRequest("alipay.trade.refund", biz_content);
    std::string url = getApiUrl("alipay.trade.refund");
    std::string response = postToUrl(url, request_data);

    return response;
}

std::string alipay::queryRefund(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    
    std::map<std::string, std::string> biz_content;
    biz_content["out_trade_no"] = trade_no;

    std::string request_data = buildRequest("alipay.trade.fastpay.refund.query", biz_content);
    std::string url = getApiUrl("alipay.trade.fastpay.refund.query");
    std::string response = postToUrl(url, request_data);

    return response;
}

std::map<std::string, std::string> alipay::handleNotify(const std::string& postData)
{
    std::map<std::string, std::string> result;
    
    std::map<std::string, std::string> params;
    size_t pos = 0;
    
    while (pos < postData.size()) {
        size_t eq_pos = postData.find('=', pos);
        if (eq_pos == std::string::npos) break;
        
        std::string key = postData.substr(pos, eq_pos - pos);
        pos = eq_pos + 1;
        
        size_t amp_pos = postData.find('&', pos);
        std::string value;
        if (amp_pos != std::string::npos) {
            value = postData.substr(pos, amp_pos - pos);
            pos = amp_pos + 1;
        } else {
            value = postData.substr(pos);
            pos = postData.size();
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

    std::string sign = params["sign"];
    std::string sign_type = params["sign_type"];
    
    if (sign.empty()) {
        result["status_code"] = "1";
        result["error_msg"] = "sign is empty";
        return result;
    }

    std::string sign_content = buildSignContent(params);
    
    if (!rsaVerify(sign_content, sign)) {
        result["status_code"] = "2";
        result["error_msg"] = "sign verify failed";
        return result;
    }

    std::string trade_status = params["trade_status"];
    if (trade_status != "TRADE_SUCCESS" && trade_status != "TRADE_FINISHED") {
        result["status_code"] = "3";
        result["error_msg"] = "trade_status is not success";
        return result;
    }

    for (const auto& p : params) {
        result[p.first] = p.second;
    }
    result["status_code"] = "0";
    result["error_msg"] = "success";

    return result;
}

}