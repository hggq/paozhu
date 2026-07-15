// ocsp_check.cpp
// OCSP 请求构造与响应解析 — 纯 OpenSSL，无 ASIO 依赖
#include "ocsp_check.h"
#include "crypto_ptr.h"
#include <sstream>
#include <ctime>

namespace http
{

// ============================================================
//  构造 OCSP 请求（DER 编码）
// ============================================================
std::vector<uint8_t> ocsp_checker::build_ocsp_request(X509 *cert, X509 *issuer_cert)
{
    // 1. 根据证书和颁发者生成 OCSP_CERTID
    OCSP_CERTID_ptr cid(OCSP_cert_to_id(nullptr, cert, issuer_cert));
    if (!cid)
        throw std::runtime_error("OCSP_cert_to_id failed");

    // 2. 创建 OCSP 请求
    OCSP_REQUEST_ptr req(OCSP_REQUEST_new());
    if (!req)
        throw std::runtime_error("OCSP_REQUEST_new failed");

    // 3. 添加证书 ID（所有权转移给 req）
    if (!OCSP_request_add0_id(req.get(), cid.release()))
        throw std::runtime_error("OCSP_request_add0_id failed");

    // 4. 添加 nonce 扩展（防重放攻击）
    //    使用 OpenSSL 标准 API OCSP_request_add1_nonce，确保 DER 编码
    //    与 openssl 命令行生成的请求完全一致（多一层 OCTET STRING 包装）
    //    val=nullptr 时 OpenSSL 自动生成随机 nonce，len=-1 表示默认长度
    if (OCSP_request_add1_nonce(req.get(), nullptr, -1) != 1)
        throw std::runtime_error("OCSP_request_add1_nonce failed");

    // 5. 序列化为 DER 字节流
    BIO_ptr bio(BIO_new(BIO_s_mem()));
    if (!bio)
        throw std::runtime_error("BIO_new failed");

    if (i2d_OCSP_REQUEST_bio(bio.get(), req.get()) == 0)
        throw std::runtime_error("i2d_OCSP_REQUEST_bio failed");

    BUF_MEM *buf = nullptr;
    BIO_get_mem_ptr(bio.get(), &buf);
    return {buf->data, buf->data + buf->length};
}

// ============================================================
//  解析 OCSP 响应
// ============================================================
OCSPResult ocsp_checker::parse_ocsp_response(const std::vector<uint8_t> &raw,
                                             X509 *cert,
                                             X509 *issuer_cert,
                                             const std::vector<uint8_t> *ocsp_req_der,
                                             std::string *debug_log)
{
    OCSPResult result;
    result.status = OCSPStatus::ERROR;
    std::ostringstream oss;

    oss << "[OCSP_PARSE] ENTER, raw size=" << raw.size() << "\n";

    // 1. 从 DER 字节流解析 OCSP 响应
    const unsigned char *p = raw.data();
    OCSP_RESPONSE_ptr resp(
        d2i_OCSP_RESPONSE(nullptr, &p, static_cast<long>(raw.size())));
    if (!resp)
    {
        result.error_msg = "d2i_OCSP_RESPONSE: failed to parse DER response";
        oss << "[OCSP_PARSE] FAIL: d2i_OCSP_RESPONSE failed\n";
        if (debug_log)
            debug_log->append(oss.str());
        return result;
    }
    oss << "[OCSP_PARSE] d2i_OCSP_RESPONSE OK\n";

    // 2. 获取 OCSP 响应状态码
    int resp_status = OCSP_response_status(resp.get());
    oss << "[OCSP_PARSE] resp_status=" << resp_status << "\n";
    if (resp_status != OCSP_RESPONSE_STATUS_SUCCESSFUL)
    {
        result.error_msg = "OCSP response status error: " + std::to_string(resp_status);
        oss << "[OCSP_PARSE] FAIL: resp_status != SUCCESSFUL\n";
        if (debug_log)
            debug_log->append(oss.str());
        return result;
    }

    // 3. 提取 Basic OCSP Response
    OCSP_BASICRESP_ptr basic(OCSP_response_get1_basic(resp.get()));
    if (!basic)
    {
        result.error_msg = "OCSP_response_get1_basic failed";
        oss << "[OCSP_PARSE] FAIL: OCSP_response_get1_basic failed\n";
        if (debug_log)
            debug_log->append(oss.str());
        return result;
    }
    oss << "[OCSP_PARSE] OCSP_response_get1_basic OK\n";

    // 4. OCSP_basic_verify 签名验证
    //    使用 issuer_cert 的独立副本（X509_dup）进行验证。
    //    OCSP_basic_verify 内部会修改证书的 ex_flags 等状态，
    //    若直接使用原始 issuer_cert，后续 OCSP_cert_to_id 会因
    //    X509_get0_pubkey_bitstr 返回异常而 SEGV。
    //    副本由 RAII 管理，验证完毕后自动释放，不影响原始证书。
    oss << "[OCSP_PARSE] OCSP_basic_verify start...\n";
    {
        X509_ptr issuer_copy(X509_dup(issuer_cert));
        if (!issuer_copy)
        {
            result.error_msg = "X509_dup(issuer_cert) failed for OCSP verify";
            oss << "[OCSP_PARSE] FAIL: X509_dup failed\n";
            if (debug_log)
                debug_log->append(oss.str());
            return result;
        }

        X509_STACK_ptr verify_chain(sk_X509_new_null());
        if (!verify_chain)
        {
            result.error_msg = "sk_X509_new_null failed";
            oss << "[OCSP_PARSE] FAIL: sk_X509_new_null failed\n";
            if (debug_log)
                debug_log->append(oss.str());
            return result;
        }
        if (sk_X509_push(verify_chain.get(), issuer_copy.get()) != 1)
        {
            result.error_msg = "sk_X509_push failed";
            oss << "[OCSP_PARSE] FAIL: sk_X509_push failed\n";
            if (debug_log)
                debug_log->append(oss.str());
            return result;
        }

        X509_STORE_ptr store(X509_STORE_new());
        if (!store)
        {
            result.error_msg = "X509_STORE_new failed";
            oss << "[OCSP_PARSE] FAIL: X509_STORE_new failed\n";
            if (debug_log)
                debug_log->append(oss.str());
            return result;
        }
        if (X509_STORE_add_cert(store.get(), issuer_cert) != 1)
        {
            result.error_msg = "X509_STORE_add_cert(issuer) failed";
            oss << "[OCSP_PARSE] FAIL: X509_STORE_add_cert failed\n";
            if (debug_log)
                debug_log->append(oss.str());
            return result;
        }

        int verify_ret = OCSP_basic_verify(basic.get(), verify_chain.get(), store.get(), OCSP_TRUSTOTHER);
        oss << "[OCSP_PARSE] OCSP_basic_verify ret=" << verify_ret << "\n";
        if (verify_ret != 1)
        {
            unsigned long err = ERR_get_error();
            char err_buf[256] = {0};
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            result.error_msg = std::string("OCSP_basic_verify failed: ") +
                               (err_buf[0] ? err_buf : "verify_ret=" + std::to_string(verify_ret));
            oss << "[OCSP_PARSE] FAIL: OCSP_basic_verify: " << result.error_msg << "\n";
            if (debug_log)
                debug_log->append(oss.str());
            return result;
        }
    }
    oss << "[OCSP_PARSE] OCSP_basic_verify OK\n";

    // 4b. nonce 验证（防重放攻击）
    //    如果提供了原始请求 DER，从中反序列化出 OCSP_REQUEST，
    //    用 OCSP_check_nonce 验证请求与响应中的 nonce 一致。
    //    OCSP_check_nonce 返回: 1=匹配, 0=双方都无nonce, -1=单方有nonce或不匹配
    //    很多OCSP响应者(如Sectigo经Cloudflare代理)不回传nonce，返回-1是正常现象
    //    仅当双方都有nonce但不一致时才视为安全问题，但OCSP_check_nonce无法区分
    //    "响应无nonce"和"nonce不匹配"，所以-1降级为警告而非失败
    if (ocsp_req_der && !ocsp_req_der->empty())
    {
        oss << "[OCSP_PARSE] nonce check, req_der size=" << ocsp_req_der->size() << "\n";
        const unsigned char *req_p = ocsp_req_der->data();
        OCSP_REQUEST_ptr req_copy(
            d2i_OCSP_REQUEST(nullptr, &req_p, static_cast<long>(ocsp_req_der->size())));
        if (req_copy)
        {
            int nonce_ret = OCSP_check_nonce(req_copy.get(), basic.get());
            oss << "[OCSP_PARSE] OCSP_check_nonce ret=" << nonce_ret << "\n";
            if (nonce_ret == 1)
            {
                oss << "[OCSP_PARSE] nonce check OK\n";
            }
            else if (nonce_ret == 0)
            {
                oss << "[OCSP_PARSE] WARN: no nonce in both request and response\n";
            }
            else// nonce_ret == -1
            {
                oss << "[OCSP_PARSE] WARN: nonce missing in response or mismatch (ret=-1), accepting anyway\n";
            }
        }
        else
        {
            oss << "[OCSP_PARSE] WARN: d2i_OCSP_REQUEST from req_der failed, skip nonce check\n";
        }
    }
    else
    {
        oss << "[OCSP_PARSE] no req_der provided, skip nonce check\n";
    }

    // 5. 使用 OCSP_cert_to_id 构造 CERTID 并查找证书状态
    if (!cert || !issuer_cert)
    {
        result.error_msg = "cert or issuer_cert is null in parse_ocsp_response";
        return result;
    }

    OCSP_CERTID_ptr cid(OCSP_cert_to_id(nullptr, cert, issuer_cert));
    if (!cid)
    {
        result.error_msg = "OCSP_cert_to_id failed in parse";
        oss << "[OCSP_PARSE] FAIL: OCSP_cert_to_id failed\n";
        if (debug_log)
            debug_log->append(oss.str());
        return result;
    }
    oss << "[OCSP_PARSE] OCSP_cert_to_id OK\n";

    int status                        = V_OCSP_CERTSTATUS_UNKNOWN;
    int reason                        = -1;
    ASN1_GENERALIZEDTIME *revoke_time = nullptr;
    ASN1_GENERALIZEDTIME *this_update = nullptr;
    ASN1_GENERALIZEDTIME *next_update = nullptr;

    if (OCSP_resp_find_status(basic.get(), cid.get(), &status, &reason, &revoke_time, &this_update, &next_update) != 1)
    {
        result.error_msg = "OCSP_resp_find_status: certificate not found";
        oss << "[OCSP_PARSE] FAIL: OCSP_resp_find_status not found\n";
        if (debug_log)
            debug_log->append(oss.str());
        return result;
    }
    oss << "[OCSP_PARSE] OCSP_resp_find_status OK, status=" << status << " reason=" << reason << "\n";

    // 6. 转换结果
    switch (status)
    {
    case V_OCSP_CERTSTATUS_GOOD:
        result.status = OCSPStatus::GOOD;
        oss << "[OCSP_PARSE] cert status=GOOD\n";
        break;
    case V_OCSP_CERTSTATUS_REVOKED:
        result.status = OCSPStatus::REVOKED;
        result.reason = reason;
        oss << "[OCSP_PARSE] cert status=REVOKED, reason=" << reason << "\n";
        break;
    default:
        result.status = OCSPStatus::UNKNOWN;
        oss << "[OCSP_PARSE] cert status=UNKNOWN\n";
        break;
    }

    // 6b. 时间有效性检查
    //    this_update 必须 <= 当前时间，next_update 必须 >= 当前时间
    //    防止重放过期或未来时间无效的响应
    if (this_update)
    {
        struct tm tm_this{};
        if (ASN1_TIME_to_tm(this_update, &tm_this) == 1)
        {
#ifdef WIN32
            time_t t_this = _mkgmtime(&tm_this);
#else
            time_t t_this = timegm(&tm_this);
#endif
            oss << "[OCSP_PARSE] this_update t=" << t_this << " now=" << time(nullptr) << "\n";
            if (time(nullptr) < t_this)
            {
                result.error_msg = "OCSP thisUpdate is in the future";
                result.status    = OCSPStatus::ERROR;
                oss << "[OCSP_PARSE] FAIL: thisUpdate is in the future\n";
                if (debug_log)
                    debug_log->append(oss.str());
                return result;
            }
        }
    }
    if (next_update)
    {
        struct tm tm_next{};
        if (ASN1_TIME_to_tm(next_update, &tm_next) == 1)
        {
#ifdef WIN32
            time_t t_next = _mkgmtime(&tm_next);
#else
            time_t t_next = timegm(&tm_next);
#endif
            oss << "[OCSP_PARSE] next_update t=" << t_next << " now=" << time(nullptr) << "\n";
            if (time(nullptr) >= t_next)
            {
                result.error_msg = "OCSP nextUpdate has expired";
                result.status    = OCSPStatus::ERROR;
                oss << "[OCSP_PARSE] FAIL: nextUpdate has expired\n";
                if (debug_log)
                    debug_log->append(oss.str());
                return result;
            }
        }
    }

    // 7. 提取时间字段（格式: YYYYMMDDHHMMSSZ）
    if (this_update)
        result.this_update.assign(
            reinterpret_cast<const char *>(this_update->data),
            this_update->length);
    if (next_update)
        result.next_update.assign(
            reinterpret_cast<const char *>(next_update->data),
            next_update->length);

    oss << "[OCSP_PARSE] SUCCESS, this_update=" << result.this_update
        << " next_update=" << result.next_update << "\n";
    if (debug_log)
        debug_log->append(oss.str());
    return result;
}

}// namespace http