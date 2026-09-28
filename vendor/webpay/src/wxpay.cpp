#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cctype>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <filesystem>
#include <atomic>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include "httpclient.h"
#include "server_localvar.h"
#include "base64.h"
#include "func.h"
#include "urlcode.h"
#include "wxpay.h"

namespace pay
{

// HTTP 头名字不区分大小写，找不到原样 Key 时再按小写比一遍
static std::string headerGet(const std::map<std::string, std::string>& headers, const std::string& key)
{
    auto it = headers.find(key);
    if (it != headers.end()) return it->second;

    std::string lower;
    for (char c : key) lower.push_back((char)std::tolower((unsigned char)c));

    for (const auto& h : headers)
    {
        std::string k;
        for (char c : h.first) k.push_back((char)std::tolower((unsigned char)c));
        if (k == lower) return h.second;
    }
    return "";
}

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
    // 单次 rd() 只有 32 位，喂不饱下面 32 个字符（≈190 位）的 nonce，且同种子完全可复现；
    // seed_seq 把 128 位喂进既有引擎 ⇒ 输出分布不变。不用 thread_local，也不引共享状态。
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    std::mt19937 gen(seed);
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
    // APIv3 只有正式域名（沙箱需单独接入，不再是 /sandboxnew）
    return "https://api.mch.weixin.qq.com";
}

// 密钥文件正文缓存：配置进内存的只有路径串，这里按路径把正文留住，一个进程只读一次盘。
// ⚠ 失效只有重启这一条：换过同一路径下的密钥文件，本进程继续用第一次成功解析的那份正文，
//   线上轮换密钥必须重启 http 服务器才生效。
// 与 alipay.cpp 那份同构，不抽公共头。
static std::shared_mutex g_wxkey_mtx;
static std::unordered_map<std::string, std::string> g_wxkey_text;

static std::string wx_slurp(const std::string &path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// true = 命中缓存，text 是正文；false = 未命中，调用方自己去读盘
static bool wx_keyfile_get(const std::string &path, std::string &text)
{
    const std::shared_lock lk(g_wxkey_mtx);
    auto it = g_wxkey_text.find(path);
    if (it == g_wxkey_text.end()) return false;
    text = it->second;
    return true;
}

// 只在"这一份正文确实解析出了密钥"之后调用：正在被替换的文件可能只读到半截，
// 那一刻的失败不该把这条通路钉住到重启为止，所以下一次照样重读。
static void wx_keyfile_put(const std::string &path, const std::string &text)
{
    const std::unique_lock lk(g_wxkey_mtx);
    g_wxkey_text.emplace(path, text);
}

static EVP_PKEY *wx_pkey_from_pem(const std::string &pem)
{
    if (pem.empty()) return nullptr;
    BIO *bio = BIO_new_mem_buf(pem.data(), pem.size());
    if (!bio) return nullptr;
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return pkey;
}

static X509 *wx_cert_from_pem(const std::string &pem)
{
    if (pem.empty()) return nullptr;
    BIO *bio = BIO_new_mem_buf(pem.data(), pem.size());
    if (!bio) return nullptr;
    X509 *cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return cert;
}

// 密钥值两态：含 "-----BEGIN" 的是 PEM 正文，否则当作文件路径。
// 路径形态先查缓存，未命中才读盘；解析成功才回写，于是"文件不存在""读到半截""读到垃圾"
// 都只是这一次签不出，不会把这条通路钉住。
std::string wxpay::rsaSign(const std::string& content)
{
    if (private_key_.empty()) return "";

    const std::string &key_str = private_key_;
    const bool is_file = (key_str.find("-----BEGIN") == std::string::npos);

    std::string key_text;
    bool cached = false;
    if (is_file)
    {
        cached = wx_keyfile_get(key_str, key_text);
        if (!cached)
        {
            key_text = wx_slurp(key_str);
            if (key_text.empty()) return "";
        }
    }
    else
    {
        key_text = key_str;
    }

    EVP_PKEY* pkey = wx_pkey_from_pem(key_text);
    if (!pkey) return "";
    if (is_file && !cached) wx_keyfile_put(key_str, key_text);

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

bool wxpay::rsaVerify(const std::string& content, const std::string& sign, const std::string& cert_pem)
{
    const std::string &key_str = cert_pem.empty() ? public_key_ : cert_pem;
    if (key_str.empty() || sign.empty()) return false;

    const bool is_file = (key_str.find("-----BEGIN") == std::string::npos);

    std::string key_text;
    bool cached = false;
    if (is_file)
    {
        cached = wx_keyfile_get(key_str, key_text);
        if (!cached)
        {
            key_text = wx_slurp(key_str);
            if (key_text.empty()) return false;
        }
    }
    else
    {
        key_text = key_str;
    }

    EVP_PKEY* pkey = nullptr;
    // 只认 X509 证书：读不出证书就这一次验签失败，不回写缓存
    X509* cert = wx_cert_from_pem(key_text);
    if (cert)
    {
        pkey = X509_get_pubkey(cert);
    }
    if (!pkey)
    {
        if (cert) X509_free(cert);
        return false;
    }
    if (is_file && !cached) wx_keyfile_put(key_str, key_text);

    // 框架的 base64_decode（base64.h）遇到不在字母表里的字符就停下，非法签名解出截断内容、验签自然失败。
    std::string decoded = http::base64_decode(sign.data(), (unsigned int)sign.size());

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        if (cert) X509_free(cert);
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

// 临时文件放哪：只用框架自己的临时目录。首选 server.conf 的 temppath（由 serverconfig 读进
// get_server_global_var().temp_path，保证以 '/' 结尾）；没加载服务器配置（CLI 等形态）或该目录
// 建不出来时退到 ./temp，最后才退到当前目录。
// 刻意不用 std::filesystem::temp_directory_path()：那是操作系统临时目录，权限模型不受本程序控制；
// 也不能把 /tmp 写成字面量 —— Windows 上没有那个目录，写失败只会静默退化成"握手不带私钥"，
// V3 那批双向 TLS 接口全部失败且页面上看不出是路径问题。
static std::filesystem::path wxpayTempDir()
{
    namespace fs = std::filesystem;
    std::error_code ec;
    const std::string &srv = http::get_server_global_var().temp_path;
    if (!srv.empty())
    {
        fs::path p(srv);
        fs::create_directories(p, ec);
        ec.clear();
        if (fs::is_directory(p, ec)) return p;
    }
    fs::path fallback("./temp");
    fs::create_directories(fallback, ec);
    ec.clear();
    if (fs::is_directory(fallback, ec)) return fallback;
    return fs::path(".");
}

// 随机文件名尾巴。不能用 std::filesystem::unique_path —— Apple 的 libc++ 压根没实现它
// （实测编译报 no member named 'unique_path'），而 <random> 三平台都有。
static std::string wxpayRandHex()
{
    static const char hexd[17] = "0123456789abcdef";
    // 直接从 OS 熵源取两个 32 位拼成 64 位，不经伪随机引擎：这里只要一次 16 个十六进制字符，
    // 用 mt19937 反而把 64 位尾巴压成"单次种子 + 它的第一手输出"（32 位熵，且同种子可复现）。
    std::random_device rd;
    const unsigned long long r = ((unsigned long long)rd() << 32) | (unsigned long long)rd();
    std::string s;
    s.reserve(16);
    for (int i = 0; i < 16; i++) s.push_back(hexd[(r >> (i * 4)) & 0xFULL]);
    return s;
}

// 微信支付 V3 接口要求 mTLS：把商户证书/私钥挂到 HTTPS 握手。
// 落盘只针对"配置里直接写了 PEM 正文"那一档；写的是文件路径时原样返回，不落盘。
static std::string wxpaySslFile(const std::string& v, std::vector<std::string>& tmpfiles)
{
    if (v.empty()) return "";
    if (v.find("-----BEGIN") == std::string::npos) return v;// 视为文件路径
    namespace fs = std::filesystem;
    // 自增必须 atomic：非原子版本在并发请求下是 UB，两个线程可能拿到同一个序号
    // ⇒ 互相覆盖对方刚写下的证书正文 ⇒ 握手用错证书，偶发失败且极难排查。
    static std::atomic<unsigned long long> seq{0};
    // atomic 只保住本进程内不撞名；多个 worker 共用同一个 temp 目录时还靠随机尾巴。
    const std::string path =
        (wxpayTempDir() / fs::path("wxpay_ssl_" + std::to_string(seq.fetch_add(1))
                                   + "_" + wxpayRandHex() + ".pem")).string();
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return "";
    // 0600 尽力而为：POSIX 上是真收紧，Windows 上这几个位映射不到 ACL —— 失败不算错误，
    // 只留在 pec 里。收窄暴露面主要靠目录本身（%TEMP% 每用户私有，而不是全局可写的 /tmp）。
    std::error_code pec;
    fs::permissions(path, fs::perms::owner_read | fs::perms::owner_write,
                    fs::perm_options::replace, pec);
    out << v;
    out.close();
    tmpfiles.push_back(path);
    return path;
}

// 落盘私钥的清理走 RAII：中途任何一次抛出（连接层异常、协程取消）都不能把私钥正文留在 temp 目录。
class wxpaySslTmp
{
public:
    wxpaySslTmp() = default;
    ~wxpaySslTmp()
    {
        for (const auto &f : files) std::remove(f.c_str());
    }
    wxpaySslTmp(const wxpaySslTmp &) = delete;
    wxpaySslTmp &operator=(const wxpaySslTmp &) = delete;
    std::vector<std::string> files;
};

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
    
    // mTLS：V3 接口必须携带商户证书与私钥完成双向 TLS 握手
    wxpaySslTmp tmpssl;
    std::string cert_path = wxpaySslFile(cert_file_, tmpssl.files);
    std::string key_path  = wxpaySslFile(private_key_, tmpssl.files);
    if (!cert_path.empty()) a->set_ssl_certificate_file(cert_path);
    if (!key_path.empty())  a->set_ssl_private_key_file(key_path);

    a->add_header("Authorization", auth);
    a->add_header("User-Agent", "wechatpay-cpp/1.0");
    a->add_header("Accept", "application/json");
    a->timeout(8);// 8s 是全渠道统一档：30s 只会把慢网关放大成一堆占死的线程
    a->send();

    std::string result;
    if (a->get_status() == 200) {
        result = a->get_body();
    } else {
        result = "{\"code\":\"-1\",\"msg\":\"HTTP Error: " + std::to_string(a->get_status()) + "\", \"body\":\"" + a->get_body() + "\"}";
    }
    return result;
}

namespace
{
// from_json 对非 JSON 抛 json_parse_error（派生自 std::exception），对尾部垃圾则停在第一个值之后
// 不管，所以这里连"根不是对象"一起收成布尔：读不到字段就等于没有这个应答。
bool parse_json_object(const std::string& text, http::obj_val& out)
{
    try
    {
        out.from_json(text);
    }
    catch (const std::exception&)
    {
        return false;
    }
    return out.is_object();
}
}

std::string wxpay::extract_prepay_id(const std::string& response)
{
    http::obj_val v;
    if (!parse_json_object(response, v)) return "";
    return v["prepay_id"].to_string();
}

std::string wxpay::aesGcmDecrypt(const std::string& ciphertext, const std::string& nonce, const std::string& associated_data)
{
    // 注意：这里必须是 APIv3 密钥（商户平台 → API安全 → APIv3 密钥，32 位），
    // 不是 V2 下单用的 API 密钥
    std::string v3key = getApiV3Key();
    if (v3key.size() != 32 || ciphertext.empty() || nonce.empty()) return "";
    
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
    
    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, (const unsigned char*)v3key.data(), (const unsigned char*)nonce.data()) != 1) {
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

std::string wxpay::build_native_body() const
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
    return body;
}

std::string wxpay::build_jsapi_body() const
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
    return body;
}

std::string wxpay::build_app_body() const
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
    return body;
}

std::string wxpay::build_h5_body() const
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
    return body;
}

// out_trade_no 传空表示用成员；同步版与协程版共用这一条构造口。
std::string wxpay::build_refund_body(const std::string& refund_amount, const std::string& refund_reason, const std::string& out_trade_no) const
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;

    std::string body = "{\"out_trade_no\":\"";
    body.append(http::json_escape(trade_no));
    body.append("\",\"out_refund_no\":\"");
    body.append(out_refund_no_);
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
    return body;
}

std::string wxpay::build_query_trade_path(const std::string& out_trade_no) const
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    // APIv3 查单必须带 mchid 查询参数，否则微信返回 400 PARAM_ERROR
    std::string path = "/v3/pay/transactions/out-trade-no/" + http::url_encode(trade_no.data(), trade_no.size());
    path.append("?mchid=");
    path.append(mch_id_);
    return path;
}

std::string wxpay::build_close_trade_path(const std::string& out_trade_no) const
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    return "/v3/pay/transactions/out-trade-no/" + http::url_encode(trade_no.data(), trade_no.size()) + "/close";
}

std::string wxpay::build_query_refund_path(const std::string& out_trade_no) const
{
    std::string trade_no = out_trade_no.empty() ? out_trade_no_ : out_trade_no;
    return "/v3/refund/domestic/refunds?out_trade_no=" + http::url_encode(trade_no.data(), trade_no.size());
}

std::string wxpay::createNative()
{
    return httpRequest("POST", "/v3/pay/transactions/native", build_native_body());
}

std::string wxpay::createJSAPI()
{
    std::string resp = httpRequest("POST", "/v3/pay/transactions/jsapi", build_jsapi_body());

    // 下单成功后还要再做一次签名，生成小程序 wx.requestPayment 需要的完整参数
    std::string prepay_id = extract_prepay_id(resp);
    if (prepay_id.empty())
    {
        // 下单失败时原样返回微信响应，便于排查
        return resp;
    }
    return buildPayParams(prepay_id);
}

std::string wxpay::buildPayParams(const std::string& prepay_id)
{
    // V3 调起支付签名串：appId\ntimeStamp\nnonceStr\npackage\n
    std::string timestamp = getTimestamp();
    std::string nonce_str = generateNonceStr();
    std::string package   = "prepay_id=" + prepay_id;

    std::string message = app_id_ + "\n";
    message += timestamp + "\n";
    message += nonce_str + "\n";
    message += package + "\n";

    std::string pay_sign = rsaSign(message);

    std::string out = "{\"appId\":\"";
    out.append(http::json_escape(app_id_));
    out.append("\",\"timeStamp\":\"");
    out.append(timestamp);
    out.append("\",\"nonceStr\":\"");
    out.append(nonce_str);
    out.append("\",\"package\":\"");
    out.append(package);
    out.append("\",\"signType\":\"RSA\",\"paySign\":\"");
    out.append(pay_sign);
    out.append("\",\"out_trade_no\":\"");
    out.append(http::json_escape(out_trade_no_));
    out.append("\",\"prepay_id\":\"");
    out.append(http::json_escape(prepay_id));
    out.append("\"}");
    return out;
}

std::string wxpay::createAPP()
{
    return httpRequest("POST", "/v3/pay/transactions/app", build_app_body());
}

std::string wxpay::createH5()
{
    return httpRequest("POST", "/v3/pay/transactions/h5", build_h5_body());
}

std::string wxpay::createMiniProgram()
{
    // 小程序支付和 JSAPI 是同一个接口，参数也一样（appid + openid），直接复用
    return createJSAPI();
}

std::string wxpay::queryTrade(const std::string& out_trade_no)
{
    return httpRequest("GET", build_query_trade_path(out_trade_no));
}

std::string wxpay::closeTrade(const std::string& out_trade_no)
{
    return httpRequest("POST", build_close_trade_path(out_trade_no), "{}");
}

std::string wxpay::refundTrade(const std::string& refund_amount, const std::string& refund_reason, const std::string& out_trade_no)
{
    // 幂等键必须显式给：空则不外发、SDK 不替调用点造兜底号（造号 ⇒ 同一笔每点一次退款都是新单号，
    // 重复退款拦不住）。错误应答形状与 httpRequest 非 200 时一致（带 code），调用方能读到。
    if (out_refund_no_.empty())
    {
        return "{\"code\":\"-1\",\"msg\":\"out_refund_no is empty\"}";
    }

    return httpRequest("POST", "/v3/refund/domestic/refunds", build_refund_body(refund_amount, refund_reason, out_trade_no));
}

std::string wxpay::queryRefund(const std::string& out_trade_no)
{
    return httpRequest("GET", build_query_refund_path(out_trade_no));
}

std::string wxpay::downloadCertificates()
{
    return httpRequest("GET", "/v3/certificates");
}

// 平台证书应答 → {证书序列号: {PEM, expire_time}}。按 data 数组里的对象逐个取 serial_no /
// encrypt_certificate / expire_time，与字段顺序无关；同时取 expire_time，避免为它单独发一趟下载。
std::map<std::string, wx_cert_t> wxpay::parsePlatformCerts(const std::string& response)
{
    std::map<std::string, wx_cert_t> certs;
    if (response.empty()) return certs;

    http::obj_val root;
    if (!parse_json_object(response, root)) return certs;
    http::obj_val &items = root["data"];
    if (!items.is_array()) return certs;

    for (http::obj_val &item : items.as_array())
    {
        if (!item.is_object()) continue;

        std::string serial = item["serial_no"].to_string();
        if (serial.empty()) continue;   // 不是证书对象（少了序列号的一律不收）

        http::obj_val &enc = item["encrypt_certificate"];
        if (!enc.is_object()) continue;
        std::string pem = aesGcmDecrypt(enc["ciphertext"].to_string(), enc["nonce"].to_string(),
                                        enc["associated_data"].to_string());
        if (pem.empty()) continue;   // 解不出来就这一条不收（APIv3 密钥不对也落在这里）

        wx_cert_t one;
        one.pem         = std::move(pem);
        one.expire_time = item["expire_time"].to_string();
        certs[serial]   = std::move(one);
    }
    return certs;
}

std::map<std::string, wx_cert_t> wxpay::downloadPlatformCerts()
{
    // {"data":[{"serial_no":"...","encrypt_certificate":{"ciphertext":"...","nonce":"...","associated_data":"..."},"expire_time":"..."}]}
    std::string resp = httpRequest("GET", "/v3/certificates");
    return parsePlatformCerts(resp);
}

bool wxpay::downloadAndSaveCert()
{
    std::map<std::string, wx_cert_t> certs = downloadPlatformCerts();
    for (const auto &c : certs)
    {
        platform_certs_[c.first] = c.second.pem;
        if (public_key_.empty()) public_key_ = c.second.pem;
    }
    return !certs.empty();
}

std::string wxpay::notifySuccessReply()
{
    return "{\"code\":\"SUCCESS\",\"message\":\"成功\"}";
}

std::string wxpay::notifyFailReply(const std::string& message)
{
    std::string out = "{\"code\":\"FAIL\",\"message\":\"";
    out.append(http::json_escape(message.empty() ? std::string("失败") : message));
    out.append("\"}");
    return out;
}

std::map<std::string, std::string> wxpay::handleNotify(const std::string& postData, const std::map<std::string, std::string>& headers)
{
    std::map<std::string, std::string> result;
    
    // HTTP 头名字不区分大小写，这里做一次兼容查找（不同框架拿到的头可能全是小写）
    std::string timestamp = headerGet(headers, "Wechatpay-Timestamp");
    std::string nonce     = headerGet(headers, "Wechatpay-Nonce");
    std::string signature = headerGet(headers, "Wechatpay-Signature");
    std::string serial_no = headerGet(headers, "Wechatpay-Serial");

    if (timestamp.empty() || nonce.empty() || signature.empty() || serial_no.empty()) {
        result["status_code"] = "1";
        result["error_msg"] = "missing required headers";
        return result;
    }
    
    std::string message = timestamp + "\n" + nonce + "\n" + postData + "\n";
    
    // 优先用与 Wechatpay-Serial 对应的平台证书验签，没有下载过就用 setPlatformCert 设置的那张
    std::string cert_pem;
    auto cit = platform_certs_.find(serial_no);
    if (cit != platform_certs_.end())
    {
        cert_pem = cit->second;
    }

    if (!rsaVerify(message, signature, cert_pem)) {
        result["status_code"] = "1";
        result["error_msg"] = "sign verify failed";
        return result;
    }
    
    // 回调报文：{"id":"..","resource":{"algorithm":"AEAD_AES_256_GCM","ciphertext":"..","nonce":"..","associated_data":".."}}
    // resource 必须按子对象读：扁平解析会把内外层键混到一起，字段一多就取错。
    // 缺 resource 要回失败：回 success 等于让微信停投一条解不出正文的通知。
    http::obj_val pkg;
    if (!parse_json_object(postData, pkg))
    {
        result["status_code"] = "3";
        result["error_msg"]   = "notify body is not valid json";
        return result;
    }
    http::obj_val &res = pkg["resource"];
    if (!res.is_object())
    {
        result["status_code"] = "3";
        result["error_msg"]   = "notify body has no resource object";
        return result;
    }

    std::string plaintext = aesGcmDecrypt(res["ciphertext"].to_string(), res["nonce"].to_string(),
                                          res["associated_data"].to_string());

    if (plaintext.empty()) {
        result["status_code"] = "2";
        result["error_msg"] = "decrypt failed";
        return result;
    }
    
    // 明文里的 amount / payer 是嵌套对象，只能按对象读；正文不是合法 JSON 就回失败等重投
    //（扁平读出来的是被截断的坏字段，拿它落账比回 FAIL 危险）
    http::obj_val v;
    if (!parse_json_object(plaintext, v))
    {
        result["status_code"] = "4";
        result["error_msg"]   = "decrypted body is not valid json";
        return result;
    }
    // 顶层字段（out_trade_no / trade_state / transaction_id / success_time / mchid / appid）
    result["out_trade_no"]  = v["out_trade_no"].to_string();
    result["trade_state"]   = v["trade_state"].to_string();
    result["transaction_id"]= v["transaction_id"].to_string();
    result["success_time"]  = v["success_time"].to_string();
    result["trade_state_desc"] = v["trade_state_desc"].to_string();
    result["trade_type"]    = v["trade_type"].to_string();
    result["mchid"]         = v["mchid"].to_string();
    result["appid"]         = v["appid"].to_string();
    result["attach"]        = v["attach"].to_string();
    result["bank_type"]     = v["bank_type"].to_string();
    result["openid"]        = v["payer"]["openid"].to_string();
    result["total"]         = std::to_string(v["amount"]["total"].to_int());
    result["currency"]      = v["amount"]["currency"].to_string();
    result["payer_total"]   = std::to_string(v["amount"]["payer_total"].to_int());
    result["status_code"] = "0";
    result["error_msg"] = "success";
    
    return result;
}

// ===== 协程镜像：以上同步方法的 awaitable 版本，内部改用 http::client::async_send() =====

asio::awaitable<std::string> wxpay::async_http_request(const std::string& method, const std::string& path, const std::string& body)
{
    std::string url = getApiUrl() + path;
    std::string auth = buildAuthorization(method, path, body);

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    if (method == "POST")
    {
        a->post(url);
        a->set_header("Content-Type", "application/json");
        a->set_body(body);
    }
    else
    {
        a->get(url);
    }

    // mTLS：V3 接口必须携带商户证书与私钥完成双向 TLS 握手
    wxpaySslTmp tmpssl;
    std::string cert_path = wxpaySslFile(cert_file_, tmpssl.files);
    std::string key_path  = wxpaySslFile(private_key_, tmpssl.files);
    if (!cert_path.empty()) a->set_ssl_certificate_file(cert_path);
    if (!key_path.empty())  a->set_ssl_private_key_file(key_path);

    a->add_header("Authorization", auth);
    a->add_header("User-Agent", "wechatpay-cpp/1.0");
    a->add_header("Accept", "application/json");
    a->timeout(8);
    co_await a->async_send();

    std::string result;
    if (a->get_status() == 200)
    {
        result = a->get_body();
    }
    else
    {
        result = "{\"code\":\"-1\",\"msg\":\"HTTP Error: " + std::to_string(a->get_status()) + "\", \"body\":\"" + a->get_body() + "\"}";
    }
    co_return result;
}

asio::awaitable<std::string> wxpay::async_create_native()
{
    co_return co_await async_http_request("POST", "/v3/pay/transactions/native", build_native_body());
}

asio::awaitable<std::string> wxpay::async_create_jsapi()
{
    std::string resp = co_await async_http_request("POST", "/v3/pay/transactions/jsapi", build_jsapi_body());
    std::string prepay_id = extract_prepay_id(resp);
    if (prepay_id.empty())
    {
        co_return resp;
    }
    co_return buildPayParams(prepay_id);
}

asio::awaitable<std::string> wxpay::async_create_app()
{
    co_return co_await async_http_request("POST", "/v3/pay/transactions/app", build_app_body());
}

asio::awaitable<std::string> wxpay::async_create_h5()
{
    co_return co_await async_http_request("POST", "/v3/pay/transactions/h5", build_h5_body());
}

asio::awaitable<std::string> wxpay::async_create_mini_program()
{
    // 小程序支付和 JSAPI 是同一个接口，直接复用
    co_return co_await async_create_jsapi();
}

asio::awaitable<std::string> wxpay::async_query_trade(const std::string& out_trade_no)
{
    co_return co_await async_http_request("GET", build_query_trade_path(out_trade_no));
}

asio::awaitable<std::string> wxpay::async_close_trade(const std::string& out_trade_no)
{
    co_return co_await async_http_request("POST", build_close_trade_path(out_trade_no), "{}");
}

asio::awaitable<std::string> wxpay::async_refund_trade(const std::string& refund_amount, const std::string& refund_reason, const std::string& out_trade_no)
{
    // 与同步版同一条闸门、同一个应答形状：幂等键漏 set 就报错，一趟都不外发。
    if (out_refund_no_.empty())
    {
        co_return "{\"code\":\"-1\",\"msg\":\"out_refund_no is empty\"}";
    }

    co_return co_await async_http_request("POST", "/v3/refund/domestic/refunds", build_refund_body(refund_amount, refund_reason, out_trade_no));
}

asio::awaitable<std::string> wxpay::async_query_refund(const std::string& out_trade_no)
{
    co_return co_await async_http_request("GET", build_query_refund_path(out_trade_no));
}

asio::awaitable<std::string> wxpay::async_download_certificates()
{
    co_return co_await async_http_request("GET", "/v3/certificates");
}

asio::awaitable<std::map<std::string, wx_cert_t>> wxpay::async_download_platform_certs()
{
    std::string resp = co_await async_http_request("GET", "/v3/certificates");
    co_return parsePlatformCerts(resp);
}

asio::awaitable<bool> wxpay::async_download_and_save_cert()
{
    std::map<std::string, wx_cert_t> certs = co_await async_download_platform_certs();
    for (const auto& c : certs)
    {
        platform_certs_[c.first] = c.second.pem;
        if (public_key_.empty()) public_key_ = c.second.pem;
    }
    co_return !certs.empty();
}

}