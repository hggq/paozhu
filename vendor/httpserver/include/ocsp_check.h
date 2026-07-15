#pragma once
#ifndef HTTP_OCSP_CHECK_H
#define HTTP_OCSP_CHECK_H

#include <string>
#include <vector>
#include <cstdint>

#include <openssl/ocsp.h>
#include <openssl/x509.h>

namespace http
{

// OCSP 响应状态枚举
enum class OCSPStatus
{
    GOOD,
    REVOKED,
    UNKNOWN,
    OCSP_ERROR
};

// OCSP 证书吊销检查结果
struct OCSPResult
{
    OCSPStatus status = OCSPStatus::OCSP_ERROR;
    std::string error_msg;
    int reason = -1;        // 吊销原因（仅 REVOKED 时有效）
    std::string this_update;// OCSP 响应的 thisUpdate
    std::string next_update;// OCSP 响应的 nextUpdate
};

// ============================================================
//  OCSP 请求构造与响应解析（纯 OpenSSL，无 ASIO 依赖）
// ============================================================
class ocsp_checker
{
  public:
    // 构造 OCSP 请求（DER 格式）
    static std::vector<uint8_t> build_ocsp_request(X509 *cert, X509 *issuer_cert);

    // 解析 OCSP 响应（DER 格式）
    // ocsp_req_der: 原始请求 DER（可选，用于 nonce 验证）
    // debug_log: 调试日志输出（可选，传入则收集解析过程信息）
    static OCSPResult parse_ocsp_response(const std::vector<uint8_t> &raw,
                                          X509 *cert,
                                          X509 *issuer_cert,
                                          const std::vector<uint8_t> *ocsp_req_der = nullptr,
                                          std::string *debug_log = nullptr);
};

}// namespace http

#endif