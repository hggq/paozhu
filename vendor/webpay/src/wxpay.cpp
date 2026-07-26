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
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include "httpclient.h"
#include "base64.h"
#include "func.h"
#include "urlcode.h"
#include "wxpay.h"

namespace pay
{

static std::string numberSanitize(const std::string& input)
{
    std::string out;
    for (char c : input) {
        if (c >= '0' && c <= '9') out.push_back(c);
    }
    return out.empty() ? "0" : out;
}

wxpay::wxpay() : is_sandbox_(false) {}

wxpay::wxpay(std::map<std::string, std::string>&& params) : is_sandbox_(false)
{
    auto it = params.find("app_id");
    if (it != params.end()) app_id_ = it->second;
    it = params.find("mch_id");
    if (it != params.end()) mch_id_ = it->second;
    it = params.find("private_key");
    if (it != params.end()) private_key_ = it->second;
    it = params.find("public_key");
    if (it != params.end()) public_key_ = it->second;
    it = params.find("api_key");
    if (it != params.end()) api_key_ = it->second;
    it = params.find("serial_no");
    if (it != params.end()) serial_no_ = it->second;
    it = params.find("out_trade_no");
    if (it != params.end()) out_trade_no_ = it->second;
    it = params.find("description");
    if (it != params.end()) description_ = it->second;
    it = params.find("body");
    if (it != params.end()) body_ = it->second;
    it = params.find("total_amount");
    if (it != params.end()) total_amount_ = it->second;
    it = params.find("notify_url");
    if (it != params.end()) notify_url_ = it->second;
    it = params.find("return_url");
    if (it != params.end()) return_url_ = it->second;
    it = params.find("openid");
    if (it != params.end()) openid_ = it->second;
    it = params.find("sandbox");
    if (it != params.end() && (it->second == "1" || it->second == "true")) is_sandbox_ = true;
}

std::string wxpay::generateNonceStr()
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

std::string wxpay::getTimestamp()
{
    return std::to_string(time(nullptr));
}

std::string wxpay::getApiUrl()
{
    if (is_sandbox_) {
        return "https://api.mch.weixin.qq.com/sandboxnew";
    }
    return "https://api.mch.weixin.qq.com";
}

std::string wxpay::rsaSign(const std::string& content)
{
    if (private_key_.empty()) return "";

    std::string key_str = private_key_;
    bool is_file = (key_str.find("-----BEGIN") == std::string::npos);
    
    EVP_PKEY* pkey = nullptr;
    
    if (is_file) {
        FILE* fp = fopen(key_str.c_str(), "rb");
        if (!fp) return "";
        pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
        fclose(fp);
    } else {
        BIO* bio = BIO_new_mem_buf(key_str.data(), key_str.size());
        if (!bio) return "";
        pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
    }

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

bool wxpay::rsaVerify(const std::string& content, const std::string& sign)
{
    if (public_key_.empty() || sign.empty()) return false;

    std::string key_str = public_key_;
    bool is_file = (key_str.find("-----BEGIN") == std::string::npos);
    
    EVP_PKEY* pkey = nullptr;
    X509* cert = nullptr;
    
    if (is_file) {
        FILE* fp = fopen(key_str.c_str(), "rb");
        if (!fp) return false;
        cert = PEM_read_X509(fp, NULL, NULL, NULL);
        if (cert) {
            pkey = X509_get_pubkey(cert);
        }
        fclose(fp);
    } else {
        BIO* bio = BIO_new_mem_buf(key_str.data(), key_str.size());
        if (!bio) return false;
        cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
        if (cert) {
            pkey = X509_get_pubkey(cert);
        }
        BIO_free(bio);
    }

    if (!pkey) {
        if (cert) X509_free(cert);
        return false;
    }

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
        if (cert) X509_free(cert);
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
    if (cert) X509_free(cert);
    return result;
}

std::string wxpay::buildAuthorization(const std::string& method, const std::string& path, const std::string& body)
{
    std::string timestamp = getTimestamp();
    std::string nonce_str = generateNonceStr();
    
    std::string message = method + "\n";
    message += path + "\n";
    message += timestamp + "\n";
    message += nonce_str + "\n";
    message += body + "\n";
    
    std::string sign = rsaSign(message);
    
    std::string auth;
    auth.append("WECHATPAY2-SHA256-RSA2048 mchid=\"");
    auth.append(mch_id_);
    auth.append("\",serial_no=\"");
    auth.append(serial_no_);
    auth.append("\",timestamp=\"");
    auth.append(timestamp);
    auth.append("\",nonce_str=\"");
    auth.append(nonce_str);
    auth.append("\",signature=\"");
    auth.append(sign);
    auth.push_back('\"');
    
    return auth;
}

std::string wxpay::httpRequest(const std::string& method, const std::string& path, const std::string& body)
{
    std::string url = getApiUrl() + path;
    std::string auth = buildAuthorization(method, path, body);
    
    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    
    if (method == "POST") {
        a->post(url);
        a->set_header("Content-Type", "application/json");
        a->set_body(body);
    } else {
        a->get(url);
    }
    
    a->add_header("Authorization", auth);
    a->add_header("User-Agent", "wechatpay-cpp/1.0");
    a->add_header("Accept", "application/json");
    a->timeout(30);
    a->send();
    
    if (a->get_status() == 200) {
        return a->get_body();
    } else {
        return "{\"code\":\"-1\",\"msg\":\"HTTP Error: " + std::to_string(a->get_status()) + "\", \"body\":\"" + a->get_body() + "\"}";
    }
}

std::map<std::string, std::string> wxpay::parseResponse(const std::string& response)
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

std::string wxpay::aesGcmDecrypt(const std::string& ciphertext, const std::string& nonce, const std::string& associated_data)
{
    if (api_key_.size() != 32 || ciphertext.empty() || nonce.empty()) return "";
    
    std::string detext = http::base64_decode(ciphertext.data(), ciphertext.size());
    if (detext.size() <= 16) return "";
    
    std::string authTag = detext.substr(detext.size() - 16);
    detext.resize(detext.size() - 16);
    
    std::string plaintext;
    plaintext.resize(detext.size());
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, (const unsigned char*)api_key_.data(), (const unsigned char*)nonce.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    if (!associated_data.empty()) {
        int out_len = 0;
        EVP_DecryptUpdate(ctx, nullptr, &out_len, (const unsigned char*)associated_data.data(), associated_data.size());
    }
    
    int out_len = 0;
    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext.data(), &out_len, (const unsigned char*)detext.data(), detext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, authTag.size(), (void*)authTag.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext.data() + out_len, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(out_len + final_len);
    
    return plaintext;
}

std::string wxpay::createNative()
{
    std::string body = "{\"mchid\":\"";
    body.append(http::json_escape(mch_id_));
    body.append("\",\"out_trade_no\":\"");
    body.append(http::json_escape(out_trade_no_));
    body.append("\",\"appid\":\"");
    body.append(http::json_escape(app_id_));
    body.append("\",\"description\":\"");
    body.append(http::json_escape(description_));
    body.append("\",\"notify_url\":\"");
    body.append(http::json_escape(notify_url_));
    body.append("\",\"amount\":{\"total\":");
    body.append(numberSanitize(total_amount_));
    body.append(",\"currency\":\"CNY\"}}");
    
    return httpRequest("POST", "/v3/pay/transactions/native", body);
}

std::string wxpay::createJSAPI()
{
    std::string body = "{\"mchid\":\"";
    body.append(http::json_escape(mch_id_));
    body.append("\",\"out_trade_no\":\"");
    body.append(http::json_escape(out_trade_no_));
    body.append("\",\"appid\":\"");
    body.append(http::json_escape(app_id_));
    body.append("\",\"description\":\"");
    body.append(http::json_escape(description_));
    body.append("\",\"notify_url\":\"");
    body.append(http::json_escape(notify_url_));
    body.append("\",\"amount\":{\"total\":");
    body.append(numberSanitize(total_amount_));
    body.append(",\"currency\":\"CNY\"},\"payer\":{\"openid\":\"");
    body.append(http::json_escape(openid_));
    body.append("\"}}");
    
    return httpRequest("POST", "/v3/pay/transactions/jsapi", body);
}

std::string wxpay::createAPP()
{
    std::string body = "{\"mchid\":\"";
    body.append(http::json_escape(mch_id_));
    body.append("\",\"out_trade_no\":\"");
    body.append(http::json_escape(out_trade_no_));
    body.append("\",\"appid\":\"");
    body.append(http::json_escape(app_id_));
    body.append("\",\"description\":\"");
    body.append(http::json_escape(description_));
    body.append("\",\"notify_url\":\"");
    body.append(http::json_escape(notify_url_));
    body.append("\",\"amount\":{\"total\":");
    body.append(numberSanitize(total_amount_));
    body.append(",\"currency\":\"CNY\"}}");
    
    return httpRequest("POST", "/v3/pay/transactions/app", body);
}

std::string wxpay::createH5()
{
    std::string body = "{\"mchid\":\"";
    body.append(http::json_escape(mch_id_));
    body.append("\",\"out_trade_no\":\"");
    body.append(http::json_escape(out_trade_no_));
    body.append("\",\"appid\":\"");
    body.append(http::json_escape(app_id_));
    body.append("\",\"description\":\"");
    body.append(http::json_escape(description_));
    body.append("\",\"notify_url\":\"");
    body.append(http::json_escape(notify_url_));
    body.append("\",\"amount\":{\"total\":");
    body.append(numberSanitize(total_amount_));
    body.append(",\"currency\":\"CNY\"},\"scene_info\":{\"payer_client_ip\":\"127.0.0.1\",\"h5_info\":{\"type\":\"Wap\",\"wap_url\":\"\",\"wap_name\":\"\"}}}");
    
    return httpRequest("POST", "/v3/pay/transactions/h5", body);
}

std::string wxpay::createMiniProgram()
{
    std::string body = "{\"mchid\":\"";
    body.append(http::json_escape(mch_id_));
    body.append("\",\"out_trade_no\":\"");
    body.append(http::json_escape(out_trade_no_));
    body.append("\",\"appid\":\"");
    body.append(http::json_escape(app_id_));
    body.append("\",\"description\":\"");
    body.append(http::json_escape(description_));
    body.append("\",\"notify_url\":\"");
    body.append(http::json_escape(notify_url_));
    body.append("\",\"amount\":{\"total\":");
    body.append(numberSanitize(total_amount_));
    body.append(",\"currency\":\"CNY\"},\"payer\":{\"openid\":\"");
    body.append(http::json_escape(openid_));
    body.append("\"}}");
    
    return httpRequest("POST", "/v3/pay/transactions/jsapi", body);
}

std::string wxpay::queryTrade(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    std::string path = "/v3/pay/transactions/out-trade-no/" + http::url_encode(trade_no.data(), trade_no.size());
    
    return httpRequest("GET", path);
}

std::string wxpay::closeTrade(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    std::string path = "/v3/pay/transactions/out-trade-no/" + http::url_encode(trade_no.data(), trade_no.size()) + "/close";
    
    return httpRequest("POST", path, "{}");
}

std::string wxpay::refundTrade(const std::string& refund_amount, const std::string& refund_reason, const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    
    std::string body = "{\"out_trade_no\":\"";
    body.append(http::json_escape(trade_no));
    body.append("\",\"out_refund_no\":\"");
    body.append("R" + generateNonceStr());
    body.append("\",\"amount\":{\"refund\":");
    body.append(numberSanitize(refund_amount));
    body.append(",\"total\":");
    body.append(numberSanitize(total_amount_));
    body.append(",\"currency\":\"CNY\"}");
    
    if (!refund_reason.empty()) {
        body.append(",\"reason\":\"");
        body.append(http::json_escape(refund_reason));
        body.append("\"");
    }
    
    body.append("}");
    
    return httpRequest("POST", "/v3/refund/domestic/refunds", body);
}

std::string wxpay::queryRefund(const std::string& out_trade_no)
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    std::string path = "/v3/refund/domestic/refunds?out_trade_no=" + http::url_encode(trade_no.data(), trade_no.size());
    
    return httpRequest("GET", path);
}

std::string wxpay::downloadCertificates()
{
    return httpRequest("GET", "/v3/certificates");
}

std::map<std::string, std::string> wxpay::handleNotify(const std::string& postData, const std::map<std::string, std::string>& headers)
{
    std::map<std::string, std::string> result;
    
    auto it_ts = headers.find("Wechatpay-Timestamp");
    auto it_nonce = headers.find("Wechatpay-Nonce");
    auto it_sig = headers.find("Wechatpay-Signature");
    auto it_serial = headers.find("Wechatpay-Serial");
    
    if (it_ts == headers.end() || it_nonce == headers.end() ||
        it_sig == headers.end() || it_serial == headers.end()) {
        result["status_code"] = "1";
        result["error_msg"] = "missing required headers";
        return result;
    }
    
    std::string timestamp = it_ts->second;
    std::string nonce = it_nonce->second;
    std::string signature = it_sig->second;
    std::string serial_no = it_serial->second;
    
    std::string message = timestamp + "\n" + nonce + "\n" + postData + "\n";
    
    if (!rsaVerify(message, signature)) {
        result["status_code"] = "1";
        result["error_msg"] = "sign verify failed";
        return result;
    }
    
    std::map<std::string, std::string> parsed = parseResponse(postData);
    
    std::string resource_type = parsed["resource_type"];
    if (resource_type != "encrypt-resource") {
        for (const auto& p : parsed) {
            result[p.first] = p.second;
        }
        result["status_code"] = "0";
        result["error_msg"] = "success";
        return result;
    }
    
    // parseResponse 是扁平解析，嵌套 resource 对象的键已提升到顶层，直接取用
    std::string ciphertext = parsed["ciphertext"];
    std::string nonce_val = parsed["nonce"];
    std::string associated_data = parsed["associated_data"];
    
    std::string plaintext = aesGcmDecrypt(ciphertext, nonce_val, associated_data);
    
    if (plaintext.empty()) {
        result["status_code"] = "2";
        result["error_msg"] = "decrypt failed";
        return result;
    }
    
    std::map<std::string, std::string> decrypted = parseResponse(plaintext);
    for (const auto& p : decrypted) {
        result[p.first] = p.second;
    }
    result["status_code"] = "0";
    result["error_msg"] = "success";
    
    return result;
}

}