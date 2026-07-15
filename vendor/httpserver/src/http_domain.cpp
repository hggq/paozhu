#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ocsp.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <filesystem>

#ifdef WIN32
#define stat _stat
#endif

#include "terminal_color.h"
#include "serverconfig.h"
#include "http2_define.h"
#include "http_domain.h"
#include "server_localvar.h"
#include "crypto_ptr.h"
#include "httpclient.h"
#include "datetime.h"
#include "func.h"
namespace http
{
namespace fs = std::filesystem;

std::vector<unsigned char> get_default_alpn()
{
    std::vector<unsigned char> res = {0x02, 'h', '2', 0x08, 'h', 't', 't', 'p', '/', '1', '.', '1'};// \x02h2\x08http/1.1"};
    //    std::vector<unsigned char>({});
    // auto p = std::begin(res);

    // p = std::copy_n(std::begin(PZ_HTTP2_H2_ALPN), PZ_HTTP2_H2_ALPN.size(), p);
    // p = std::copy_n(std::begin(PZ_HTTP2_H2_16_ALPN), PZ_HTTP2_H2_16_ALPN.size(), p);
    // p = std::copy_n(std::begin(PZ_HTTP2_H2_14_ALPN), PZ_HTTP2_H2_14_ALPN.size(), p);

    return res;
}

int next_proto_cb(SSL *s, const unsigned char **data, unsigned int *len, void *arg)
{
    //auto next_proto = static_cast<std::vector<unsigned char> *>(arg);
    auto next_proto = static_cast<unsigned char *>(arg);
    *data           = next_proto;
    if (next_proto[0] == 0x02)
    {
        *len = 12;
    }
    else
    {
        *len = 0;
    }
    //*data         = next_proto->data();
    //*len            = next_proto->size();
    if (true || s)
    {
    }
    return SSL_TLSEXT_ERR_OK;
}

int alpn_cb([[maybe_unused]] SSL *ssl,
            const unsigned char **out,
            unsigned char *outlen,
            const unsigned char *in,
            unsigned int inlen,
            void *arg)
{

    int ret = SSL_TLSEXT_ERR_NOACK;
    *out    = NULL;
    *outlen = 0;

    serverconfig &sysconfigpath = getserversysconfig();
    bool isusehttp2             = true;

    unsigned long long host_index = (unsigned long long)arg;
    if (host_index < sysconfigpath.sitehostinfos.size())
    {
        isusehttp2 = sysconfigpath.sitehostinfos[host_index].http2_enable;
    }

    host_index       = 0;
    unsigned char *p = (unsigned char *)in;
    while (inlen > 0 && in && (p - in) < inlen)
    {
        int len = p[0];
        p++;
        if (len == 2 && p[1] == '2')// && memcmp(p, "h2", 2) == 0)
        {
            host_index = host_index + 10;
        }
        else if (len == 8)//&& memcmp(p, "http/1.1", 8) == 0)
        {
            host_index = host_index + 1;
        }
        p = p + len;
    }

    unsigned int protos_len;
    const char *protos;
    unsigned char *tmp_out;

    if (isusehttp2 && host_index > 9)
    {
        protos     = HTTP2_H2_ALPN HTTP1_NPN;
        protos_len = sizeof(HTTP2_H2_ALPN HTTP1_NPN) - 1;
    }
    else if (host_index > 0)
    {
        protos     = HTTP1_NPN;
        protos_len = sizeof(HTTP1_NPN) - 1;
    }
    if (host_index > 0)
    {
        ret  = SSL_select_next_proto(&tmp_out, outlen, (const unsigned char *)protos, protos_len, in, inlen);
        *out = tmp_out;
    }

    ret = SSL_TLSEXT_ERR_OK;

    return ret;
}

std::string get_password() { return "sslpem123456"; }
int serverNameCallback(SSL *ssl, [[maybe_unused]] int *ad, [[maybe_unused]] void *arg)
{
    if (ssl == NULL)
        return SSL_TLSEXT_ERR_NOACK;

    const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
    if (servername == NULL)
    {
        //only ip
        return SSL_TLSEXT_ERR_ALERT_WARNING;
    }
    SSL_CTX *ctx                = NULL;
    serverconfig &sysconfigpath = getserversysconfig();

    if (sysconfigpath.checkmaindomain(servername))
    {
        // 主域名：不切换 SSL_CTX
        if (SSL_set_cipher_list(ssl, DEFAULT_CIPHER_LIST) == 0)
        {
            return SSL_TLSEXT_ERR_ALERT_FATAL;
        }
        sysconfigpath.set_ocsp_staple_to_ssl(ssl, servername);
        return SSL_TLSEXT_ERR_OK;
    }

    if (servername && strlen(servername) > 0)
    {
        ctx = sysconfigpath.getctx(servername);
    }

    if (ctx == NULL)
    {
        return SSL_TLSEXT_ERR_ALERT_FATAL;
    }

    // 非主域名：切换 SSL_CTX 以加载对应的证书链
    SSL_set_SSL_CTX(ssl, ctx);

    SSL_set_verify(ssl, SSL_CTX_get_verify_mode(ctx), SSL_CTX_get_verify_callback(ctx));

    SSL_set_verify_depth(ssl, SSL_CTX_get_verify_depth(ctx));
    SSL_clear_options(ssl, 0xFFFFFFFFL);
    SSL_set_options(ssl, SSL_CTX_get_options(ctx));

    if (SSL_set_cipher_list(ssl, DEFAULT_CIPHER_LIST) == 0)
    {
        return SSL_TLSEXT_ERR_ALERT_FATAL;
    }

    sysconfigpath.set_ocsp_staple_to_ssl(ssl, servername);

    unsigned long long temp_domain = sysconfigpath.get_hostindex(servername);

    SSL_CTX_set_alpn_select_cb(ctx, alpn_cb, (void *)temp_domain);
    return SSL_TLSEXT_ERR_OK;
}

cert_validity_t get_cert_validity(const std::string &cert_path)
{
    // 1. Open certificate file
    BIO_ptr bio(BIO_new_file(cert_path.c_str(), "r"));
    if (!bio)
    {
        throw std::runtime_error("Failed to open certificate file: " + cert_path);
    }

    // 2. Parse PEM-encoded X.509 certificate
    X509_ptr cert(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
    if (!cert)
    {
        unsigned long err = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        throw std::runtime_error("Failed to parse certificate: " + std::string(err_buf));
    }

    // 3. Extract ASN1_TIME pointers (non-owning, valid as long as cert lives)
    const ASN1_TIME *nb = X509_get0_notBefore(cert.get());
    const ASN1_TIME *na = X509_get0_notAfter(cert.get());
    if (!nb || !na)
    {
        throw std::runtime_error("Certificate missing validity fields");
    }

    // 4. Convert ASN1_TIME -> struct tm -> time_t
    struct tm tm_before{}, tm_after{};
    if (!ASN1_TIME_to_tm(nb, &tm_before))
    {
        throw std::runtime_error("Failed to convert notBefore to tm");
    }
    if (!ASN1_TIME_to_tm(na, &tm_after))
    {
        throw std::runtime_error("Failed to convert notAfter to tm");
    }

    // Use timegm() because ASN1_TIME is always UTC
    // time_t t_before = timegm(&tm_before);
    // time_t t_after  = timegm(&tm_after);

#ifdef WIN32
    time_t t_before = _mkgmtime(&tm_before);
    time_t t_after  = _mkgmtime(&tm_after);
#else
    time_t t_before = timegm(&tm_before);
    time_t t_after  = timegm(&tm_after);
#endif

    // 5. Calculate remaining days and expiration status
    time_t now          = time(nullptr);
    double seconds_left = difftime(t_after, now);
    double days_left    = seconds_left / 86400.0;

    unsigned int expired_day = days_left > 0 ? days_left : 0;

    return {
        /* .not_before     = */ t_before,
        /* .not_after      = */ t_after,
        /* .days_remaining = */ expired_day,
        /* .is_expired     = */ (now >= t_after)};
}

eab_credentials_t fetch_eab_by_email(std::string_view url, std::string_view email)
{
    //"Step 0: 通过邮箱申请 EAB 凭证..."
    //post https://api.zerossl.com/acme/eab-credentials-email
    std::string body = "email=";
    body.append(email);

    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->post(url);
    https->add_header("Content-Type", "application/x-www-form-urlencoded");
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->set_body(body);
    https->send();

    if (https->get_status() != 200)
        throw std::runtime_error("EAB 请求失败: HTTP " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());

    eab_credentials_t eab;
    if (https->page.isjson == 1)
    {
        if (https->page.json["success"].to_bool())
        {
            eab.eab_kid      = https->page.json["eab_kid"].to_string();
            eab.eab_hmac_key = https->page.json["eab_hmac_key"].to_string();
        }
        else
        {
            throw std::runtime_error("EAB 申请失败:\n" + https->get_body());
        }
    }
    else
    {
        throw std::runtime_error("EAB 请求失败: JSON " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());
    }

    if (eab.eab_kid.empty() || eab.eab_hmac_key.empty())
    {
        throw std::runtime_error("EAB 响应缺少字段:\n" + https->get_body());
    }
    return eab;
}

// ============================================================
//  OCSP Stapling: 刷新所有域名的 OCSP 响应
//  从证书的 AIA 扩展中提取 OCSP URL，获取 OCSP 响应并缓存
// ============================================================
void refresh_all_ocsp_staples()
{
    auto &cfg                          = getserversysconfig();
    server_loaclvar &static_server_var = get_server_global_var();
    std::string ocsp_debug;
    std::ostringstream oss;
    oss << " begin ocsp refresh task :" << get_date("%Y-%m-%d %X", timeid()) << "\n";
    ocsp_debug.append(oss.str());
    oss.str("");

    // 遍历所有配置的站点
    for (const auto &site : cfg.sitehostinfos)
    {
        if (site.is_close || site.mainhost.empty())
            continue;

        std::string domain_name = site.mainhost;
        oss << "[OCSP] domain: " << domain_name << "\n";
        // 构造证书文件路径（与 getctx() 保持一致的逻辑）
        std::string basepath = cfg.configpath;
        if (basepath.empty())
            basepath = "conf/";
        if (basepath.back() != '/')
            basepath.push_back('/');

        std::string cert_file = basepath + domain_name + ".pem";
        bool is_www           = (domain_name.size() > 4 && domain_name[0] == 'w' && domain_name[1] == 'w' && domain_name[2] == 'w' && domain_name[3] == '.');
        oss << "[OCSP] pem: " << cert_file << "\n";
        FILE *fp = fopen(cert_file.c_str(), "r");
        if (!fp)
        {
            // 回退策略：与 getctx() 一致
            if (is_www)
            {
                cert_file = basepath + domain_name.substr(4) + ".pem";
            }
            else
            {
                cert_file = basepath + "www." + domain_name + ".pem";
            }
            oss << "[OCSP] pem fallback: " << cert_file << "\n";
            fp = fopen(cert_file.c_str(), "r");
            if (!fp)
            {
                oss << "[OCSP] cert file not found, skipping\n";
                ocsp_debug.append(oss.str());
                oss.str("");
                continue;
            }
        }

        // 从 PEM 文件读取第一个证书（服务端证书）
        X509_ptr cert(PEM_read_X509(fp, nullptr, nullptr, nullptr));
        if (!cert)
        {
            fclose(fp);
            oss << "[OCSP] PEM_read_X509 failed\n";
            ocsp_debug.append(oss.str());
            oss.str("");
            continue;
        }

        const ASN1_TIME *na = X509_get0_notAfter(cert.get());
        if (!na)
        {
            fclose(fp);
            continue;
        }

        struct tm tm_after{};
        if (!ASN1_TIME_to_tm(na, &tm_after))
        {
            fclose(fp);
            continue;
        }

#ifdef WIN32
        time_t t_after = _mkgmtime(&tm_after);
#else
        time_t t_after = timegm(&tm_after);
#endif

        time_t now = time(nullptr);
        if ((now + 14400) > t_after)
        {
            // 已经过期不做 OCSP
            fclose(fp);
            oss << "[OCSP] cert expired or expiring soon, skipping\n";
            ocsp_debug.append(oss.str());
            oss.str("");
            continue;
        }
        oss << "[OCSP] cert expire time: " << t_after << "\n";
        // 从同一个 PEM 文件读取第二个证书（签发者/中间 CA 证书）
        X509_ptr issuer_cert(PEM_read_X509(fp, nullptr, nullptr, nullptr));
        fclose(fp);
        if (!issuer_cert)
        {
            oss << "[OCSP] issuer cert not found in PEM\n";
            ocsp_debug.append(oss.str());
            oss.str("");
            continue;
        }

        // 从证书的 AIA (Authority Information Access) 扩展中提取 OCSP URL
        STACK_OF(OPENSSL_STRING) *ocsp_urls = X509_get1_ocsp(cert.get());
        if (!ocsp_urls || sk_OPENSSL_STRING_num(ocsp_urls) == 0)
        {
            if (ocsp_urls)
                X509_email_free(ocsp_urls);
            oss << "[OCSP] no OCSP URL in cert AIA\n";
            ocsp_debug.append(oss.str());
            oss.str("");
            continue;
        }

        std::string ocsp_url = sk_OPENSSL_STRING_value(ocsp_urls, 0);
        X509_email_free(ocsp_urls);
        if (ocsp_url.empty())
            continue;
        oss << "[OCSP] ocsp_url: " << ocsp_url << "\n";

        // 缓存文件路径：temp/ocsp_<domain>.der
        // 路径净化：只允许字母数字点减号下划线
        std::string safe_domain;
        safe_domain.reserve(domain_name.size());
        for (char c : domain_name)
        {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_')
                safe_domain.push_back(c);
        }
        if (safe_domain.empty())
            continue;

        std::string cache_dir = dir_name(static_server_var.log_path);

        if (cache_dir.size() > 0 && cache_dir.back() != '/')
        {
            cache_dir.append("/");
        }
        cache_dir.append("temp");
        std::string cache_path = cache_dir + "/ocsp_" + safe_domain + ".der";
        std::string etag_path  = cache_dir + "/ocsp_" + safe_domain + ".etag";
        // 最大缓存文件大小 64KB
        constexpr size_t MAX_OCSP_FILE_SIZE = 65536;
        oss << "[OCSP] cache_path: " << cache_path << "\n";

        // 1. 先检查缓存文件是否存在且未过期
        bool cache_valid = false;
        // 保存的 ETag 值，用于条件请求
        std::string cached_etag;
        // 缓存 DER 数据（用于 304 时复用）
        std::vector<uint8_t> cached_der;
        {
            FILE *cf = fopen(cache_path.c_str(), "rb");
            if (cf)
            {
                unsigned char cbuf[4096];
                size_t cn;
                while ((cn = fread(cbuf, 1, sizeof(cbuf), cf)) > 0)
                {
                    if (cached_der.size() + cn > MAX_OCSP_FILE_SIZE)
                        break;
                    cached_der.insert(cached_der.end(), cbuf, cbuf + cn);
                }
                fclose(cf);

                if (!cached_der.empty())
                {
                    std::string cache_parse_log;
                    OCSPResult cached_result = ocsp_checker::parse_ocsp_response(cached_der, cert.get(), issuer_cert.get(), nullptr, &cache_parse_log);
                    oss << cache_parse_log;
                    if (cached_result.status == OCSPStatus::GOOD)
                    {
                        // 检查 next_update 是否过期（格式: YYYYMMDDHHMMSSZ）
                        bool is_fresh       = true;
                        long time_remaining = 0;
                        if (cached_result.next_update.size() >= 14)
                        {
                            const std::string &nu = cached_result.next_update;
                            oss << "[OCSP] cached next_update: " << nu << "\n";
                            struct tm tm_next{};
                            tm_next.tm_year = (nu[0] - '0') * 1000 + (nu[1] - '0') * 100 + (nu[2] - '0') * 10 + (nu[3] - '0') - 1900;
                            tm_next.tm_mon  = (nu[4] - '0') * 10 + (nu[5] - '0') - 1;
                            tm_next.tm_mday = (nu[6] - '0') * 10 + (nu[7] - '0');
                            tm_next.tm_hour = (nu[8] - '0') * 10 + (nu[9] - '0');
                            tm_next.tm_min  = (nu[10] - '0') * 10 + (nu[11] - '0');
                            tm_next.tm_sec  = (nu[12] - '0') * 10 + (nu[13] - '0');

#ifdef WIN32
                            time_t t_next = _mkgmtime(&tm_next);
#else
                            time_t t_next = timegm(&tm_next);
#endif

                            time_t now_t   = time(nullptr);
                            time_remaining = (long)(t_next - now_t);
                            // 还有3小时就要过期了，保险点 24小时，提前一天更新
                            if (now_t >= t_next + 86400)
                                is_fresh = false;
                        }
                        if (is_fresh)
                        {
                            cfg.set_ocsp_staple(domain_name, cached_der);
                            cache_valid = true;

                            // 读取已保存的 ETag
                            std::ifstream ef(etag_path);
                            if (ef)
                            {
                                std::getline(ef, cached_etag);
                                oss << "[OCSP] cached etag: " << cached_etag << " time_remaining=" << time_remaining << "s\n";
                            }

                            // 判断是否需要 ETag 条件请求：
                            // 缓存仍有效且 next_update 还有超过 24 小时 → 发 ETag 检查是否有更新
                            // 这样即使缓存没过期，也能及时发现证书被吊销
                            if (!cached_etag.empty() && time_remaining > 86400)
                            {
                                oss << "[OCSP] cache valid, sending conditional request with ETag\n";
                                // 跳过 continue，走 ETag 条件请求路径
                            }
                            else
                            {
                                oss << "[OCSP] cache valid, skipping refresh\n";
                                ocsp_debug.append(oss.str());
                                oss.str("");
                                continue;
                            }
                        }
                    }
                }
            }
        }
        if (cache_valid && cached_etag.empty())
        {
            ocsp_debug.append(oss.str());
            oss.str("");
            continue;
        }

        // 标记是否走 ETag 条件请求
        bool use_etag = cache_valid && !cached_etag.empty();
        ocsp_debug.append(oss.str());
        oss.str("");

        oss << "[OCSP] post request for " << domain_name << "\n";
        // 2. 缓存不可用，发送 OCSP POST 请求
        try
        {
            std::vector<uint8_t> ocsp_req = ocsp_checker::build_ocsp_request(cert.get(), issuer_cert.get());
            std::string ocsp_body(reinterpret_cast<const char *>(ocsp_req.data()), ocsp_req.size());
            oss << "[OCSP] ocsp_req built, size=" << ocsp_req.size() << "\n";

            auto http_client = std::make_shared<http::client>();
            http_client->post(ocsp_url);
            http_client->add_header("Content-Type", "application/ocsp-request");
            http_client->add_header("Connection", "close");
            http_client->add_header("User-Agent", "Paozhu ocsp Client/1.0");
            if (use_etag && !cached_etag.empty())
            {
                http_client->add_header("If-None-Match", cached_etag);
                oss << "[OCSP] sending with If-None-Match: " << cached_etag << "\n";
            }
            http_client->timeout(60);
            http_client->set_body(ocsp_body);
            oss << "[OCSP] sending POST to " << ocsp_url
                << " host=" << http_client->host
                << " port=" << http_client->port << "\n";
            http_client->send();
            unsigned int status = http_client->get_status();
            oss << "[OCSP] HTTP status=" << status << " iserror=" << (int)http_client->iserror;
            if (!http_client->error_msg.empty())
                oss << " error_msg=" << http_client->error_msg;
            oss << "\n";

            if (status == 0)
            {
                if (http_client->iswait_exit)
                    oss << "[OCSP] TIMEOUT: killed by timer (iswait_exit=true)\n";
                else
                    oss << "[OCSP] connection closed by remote, content_size=" << http_client->page.content.size() << "\n";
            }

            if (status != 200 && status != 304)
            {
                oss << "[OCSP] HTTP failed status=" << status << ", trying file fallback\n";
                // HTTP 失败时尝试从缓存文件加载（即使可能过期）
                FILE *fb = fopen(cache_path.c_str(), "rb");
                if (fb)
                {
                    std::vector<uint8_t> file_data;
                    unsigned char buf[4096];
                    size_t n;
                    while ((n = fread(buf, 1, sizeof(buf), fb)) > 0)
                    {
                        if (file_data.size() + n > MAX_OCSP_FILE_SIZE)
                            break;
                        file_data.insert(file_data.end(), buf, buf + n);
                    }
                    fclose(fb);
                    oss << "[OCSP] file fallback loaded size=" << file_data.size() << "\n";
                    if (!file_data.empty())
                    {
                        std::string fb_log;
                        OCSPResult file_result = ocsp_checker::parse_ocsp_response(file_data, cert.get(), issuer_cert.get(), nullptr, &fb_log);
                        oss << fb_log;
                        oss << "[OCSP] file fallback parse status=" << (int)file_result.status << " error=" << file_result.error_msg << "\n";
                        if (file_result.status == OCSPStatus::GOOD)
                        {
                            cfg.set_ocsp_staple(domain_name, file_data);
                            oss << "[OCSP] file fallback cached\n";
                        }
                    }
                }
                else
                {
                    oss << "[OCSP] no file fallback found at " << cache_path << "\n";
                }
                ocsp_debug.append(oss.str());
                oss.str("");
                continue;
            }

            // 304 Not Modified: 缓存仍有效，直接用缓存数据
            if (status == 304)
            {
                oss << "[OCSP] 304 Not Modified, using cached DER\n";
                if (!cached_der.empty())
                {
                    cfg.set_ocsp_staple(domain_name, cached_der);
                    oss << "[OCSP] staple refreshed from cache, size=" << cached_der.size() << "\n";
                }
                ocsp_debug.append(oss.str());
                oss.str("");
                continue;
            }

            std::string resp_body = http_client->get_body();
            oss << "[OCSP] response body size=" << resp_body.size() << "\n";
            if (resp_body.empty())
            {
                oss << "[OCSP] empty response body\n";
                ocsp_debug.append(oss.str());
                oss.str("");
                continue;
            }
            std::vector<uint8_t> response_data(resp_body.begin(), resp_body.end());

            std::string parse_log;
            OCSPResult result = ocsp_checker::parse_ocsp_response(response_data, cert.get(), issuer_cert.get(), &ocsp_req, &parse_log);
            oss << parse_log;
            oss << "[OCSP] parse result: status=" << (int)result.status << " error=" << result.error_msg << "\n";

            if (result.status == OCSPStatus::GOOD)
            {
                oss << "[OCSP] SUCCESS: staple cached, size=" << response_data.size() << "\n";
                cfg.set_ocsp_staple(domain_name, response_data);
                // 保存到 temp 目录供下次使用
                fs::create_directories(cache_dir);
                std::ofstream ofs(cache_path, std::ios::binary);
                if (ofs)
                {
                    ofs.write(reinterpret_cast<const char *>(response_data.data()),
                              static_cast<std::streamsize>(response_data.size()));
                    oss << "[OCSP] saved to file: " << cache_path << "\n";
                }
                // 从响应头提取 ETag 并保存
                auto it = http_client->page.header.find("ETag");
                if (it != http_client->page.header.end())
                {
                    std::ofstream ef(etag_path);
                    if (ef)
                    {
                        ef << it->second;
                        oss << "[OCSP] saved etag: " << it->second << " to " << etag_path << "\n";
                    }
                }
                else
                {
                    oss << "[OCSP] no ETag in response header\n";
                }
            }
            else if (result.status == OCSPStatus::REVOKED)
            {
                oss << "[OCSP] REVOKED: " << domain_name << " reason=" << result.reason << "\n";
            }
            else
            {
                oss << "[OCSP] CHECK FAILED: " << domain_name << " " << result.error_msg << "\n";
            }
        }
        catch (const std::exception &e)
        {
            oss << "[OCSP] EXCEPTION for " << domain_name << ": " << e.what() << "\n";
        }
        ocsp_debug.append(oss.str());
        oss.str("");
    }

    // 保存日志到 log 目录
    try
    {
        std::string log_dir = static_server_var.log_path;
        if (log_dir.size() > 0 && log_dir.back() != '/')
            log_dir.push_back('/');
        log_dir.append("ocsp_log");
        std::filesystem::create_directories(log_dir);
        log_dir.append("/ocsp");
        std::ofstream out_log(log_dir, std::ios::app);
        if (out_log.is_open())
        {
            out_log << ocsp_debug;
            out_log.close();
        }
    }
    catch (const std::exception &)
    {
    }
}

}// namespace http