#include <string.h>
#include <sstream>
#include <cstdio>
#include <string_view>
#include <algorithm>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include "md5.h"
#include "httpclient.h"
#include "func.h"
#include "unicode.h"
#include "sms.h"

namespace sms
{

aliyun_sms_config_t &aliyun_sms_config()
{
    static aliyun_sms_config_t instance;
    return instance;
}

tencent_sms_config_t &tencent_sms_config()
{
    static tencent_sms_config_t instance;
    return instance;
}

send_sms &get_send_sms_obj()
{
    static send_sms instance;
    return instance;
}

aliyun_sendsms::aliyun_sendsms() {}

bool aliyun_sendsms::send()
{
    http::client a;
    http::obj_val parameter;
    a.add_header("x-acs-version", "2017-05-25");
    a.add_header("x-acs-action", "SendSms");
    a.add_header("user-agent", "Paozhu server");
    std::string timestr = http::get_utctime();
    std::string noncestr = http::md5(timestr + http::rand_string(8));
    a.add_header("x-acs-date", timestr);
    a.add_header("x-acs-signature-nonce", noncestr);

    const char *str256 = "";
    unsigned char sSHA256[SHA256_DIGEST_LENGTH] = {0};
    SHA256((const unsigned char *)str256, 0, sSHA256);

    std::string sha256str = http::char2hex(&sSHA256[0], SHA256_DIGEST_LENGTH);
    a.add_header("x-acs-content-sha256", sha256str);
    a.add_header("accept", "application/json");

    std::map<std::string, std::string> headers_hash, query_hash;
    headers_hash["content-type"] = "application/json";
    headers_hash["host"] = "dysmsapi.aliyuncs.com";
    headers_hash["x-acs-version"] = "2017-05-25";
    headers_hash["x-acs-action"] = "SendSms";
    headers_hash["x-acs-date"] = timestr;
    headers_hash["x-acs-signature-nonce"] = noncestr;
    headers_hash["x-acs-content-sha256"] = sha256str;

    std::string signHeaders_hash_str, canonicalHeaderString, canonicalRequest, signatureAlgorithm;
    std::string accessKeyId = aliyun_sms_config().accessKeyId;
    std::string accessKeySecret = aliyun_sms_config().accessKeySecret;

    for (auto &[first, second] : headers_hash)
    {
        if (signHeaders_hash_str.size() > 0)
        {
            signHeaders_hash_str.push_back(';');
        }
        signHeaders_hash_str.append(first);

        canonicalHeaderString.append(first);
        canonicalHeaderString.push_back(':');
        canonicalHeaderString.append(second);
        canonicalHeaderString.push_back('\n');
    }

    signatureAlgorithm = "ACS3-HMAC-SHA256";
    canonicalRequest.append("POST\n");
    canonicalRequest.append("/\n");

    for (auto obj : TemplateParam)
    {
        parameter[obj.first] = obj.second;
    }

    query_hash["PhoneNumbers"] = PhoneNumbers;
    query_hash["SignName"] = SignName;
    query_hash["TemplateCode"] = TemplateCode;
    query_hash["TemplateParam"] = parameter.to_json();

    a.post_json("https://dysmsapi.aliyuncs.com/");
    a.build_query(query_hash);

    canonicalRequest.append(a.get_query());
    canonicalRequest.append("\n");
    canonicalRequest.append(canonicalHeaderString);
    canonicalRequest.append("\n");
    canonicalRequest.append(signHeaders_hash_str);
    canonicalRequest.append("\n");
    canonicalRequest.append(sha256str);

    SHA256((const unsigned char *)&canonicalRequest[0], canonicalRequest.size(), sSHA256);
    sha256str = http::char2hex(&sSHA256[0], SHA256_DIGEST_LENGTH);

    std::string hmac_str;
    hmac_str.append(signatureAlgorithm);
    hmac_str.push_back('\n');
    hmac_str.append(sha256str);

    unsigned char sSHA[EVP_MAX_MD_SIZE] = {0};
    unsigned int nSHALen = EVP_MAX_MD_SIZE;

    HMAC(EVP_sha256(), (const unsigned char *)&accessKeySecret[0], accessKeySecret.size(),
         (const unsigned char *)&hmac_str[0], hmac_str.size(), sSHA, &nSHALen);
    sha256str = http::char2hex(&sSHA[0], nSHALen);
    std::string Authorization;

    Authorization.append(signatureAlgorithm);
    Authorization.append(" Credential=");
    Authorization.append(accessKeyId);
    Authorization.append(",SignedHeaders=");
    Authorization.append(signHeaders_hash_str);
    Authorization.append(",Signature=");
    Authorization.append(sha256str);

    a.add_header("Authorization", Authorization);
    a.send();
    if (a.get_status() == 200)
    {
        resp_content = a.get_body();
        return true;
    }
    return false;
}

tencent_sendsms::tencent_sendsms() : host("sms.ap-guangzhou.tencentcloudapi.com") {}

std::string tencent_sendsms::get_parameter_json()
{
    std::stringstream _stream;
    _stream << "{\"PhoneNumberSet\":[\"" << PhoneNumberSet << "\"],\"SmsSdkAppId\":\"";
    _stream << SmsSdkAppId << "\",";
    _stream << "\"TemplateId\":\"" << TemplateId << "\",";
    _stream << "\"SignName\":\"" << http::utf8_to_jsonstring(SignName) << "\",";
    _stream << "\"TemplateParamSet\":[";

    for (unsigned int i = 0; i < TemplateParamSet.size(); i++)
    {
        if (i > 0)
        {
            _stream << ",";
        }
        _stream << "\"" << http::utf8_to_jsonstring(TemplateParamSet[i]) << "\"";
    }

    _stream << "]}";

    return _stream.str();
}

bool tencent_sendsms::send()
{
    http::client a;
    std::string SECRET_ID = accessKeyId.empty() ? tencent_sms_config().accessKeyId : accessKeyId;
    std::string SECRET_KEY = accessKeySecret.empty() ? tencent_sms_config().accessKeySecret : accessKeySecret;

    std::string service = "sms";
    std::string region = "ap-guangzhou";
    std::string action = "SendSms";
    std::string version = "2021-01-11";
    int64_t timestamp = time(NULL);
    std::string date = http::get_date("%Y-%m-%d", (unsigned int)timestamp);

    unsigned char sSHA[EVP_MAX_MD_SIZE] = {0};
    unsigned char sSHA256[SHA256_DIGEST_LENGTH] = {0};
    unsigned int nSHALen = EVP_MAX_MD_SIZE;

    std::string httpRequestMethod = "POST";
    std::string canonicalUri = "/";
    std::string canonicalQueryString = "";
    std::string lower = action;
    std::transform(action.begin(), action.end(), lower.begin(), ::tolower);

    std::string canonicalHeaders =
        std::string("content-type:application/json\n") + "host:" + host + "\n" + "x-tc-action:" + lower + "\n";
    std::string signedHeaders = "content-type;host;x-tc-action";
    std::string payload;

    payload = get_parameter_json();

    std::string hashedRequestPayload;
    SHA256((const unsigned char *)&payload[0], payload.size(), sSHA256);
    hashedRequestPayload = http::char2hex(sSHA256, SHA256_DIGEST_LENGTH);

    std::string canonicalRequest = httpRequestMethod + "\n" + canonicalUri + "\n" + canonicalQueryString + "\n" +
                                   canonicalHeaders + "\n" + signedHeaders + "\n" + hashedRequestPayload;

    std::string algorithm = "TC3-HMAC-SHA256";
    std::string credentialScope = date + "/" + service + "/" + "tc3_request";

    SHA256((const unsigned char *)&canonicalRequest[0], canonicalRequest.size(), sSHA256);
    std::string hashedCanonicalRequest = http::char2hex(sSHA256, SHA256_DIGEST_LENGTH);

    std::string stringToSign = algorithm + "\n" + std::to_string(timestamp) + "\n" + credentialScope + "\n" + hashedCanonicalRequest;

    std::string kKey = "TC3" + SECRET_KEY;
    static const char *tc3_request = "tc3_request";

    HMAC(EVP_sha256(), (const unsigned char *)&kKey[0], kKey.size(),
         (const unsigned char *)&date[0], date.size(), sSHA, &nSHALen);

    HMAC(EVP_sha256(), sSHA, nSHALen, (const unsigned char *)&service[0], service.size(), sSHA, &nSHALen);

    HMAC(EVP_sha256(), sSHA, nSHALen, (const unsigned char *)tc3_request, strlen(tc3_request), sSHA, &nSHALen);

    HMAC(EVP_sha256(), sSHA, nSHALen, (const unsigned char *)&stringToSign[0], stringToSign.size(), sSHA, &nSHALen);
    std::string signature = http::char2hex((const unsigned char *)sSHA, nSHALen);

    std::string authorization = algorithm + " " + "Credential=" + SECRET_ID + "/" + credentialScope + ", " +
                                "SignedHeaders=" + signedHeaders + ", " + "Signature=" + signature;

    a.post_json("https://" + host + "/");

    a.add_header("User-Agent", "Paozhu webclient");
    a.add_header("X-TC-Action", action);
    a.add_header("X-TC-RequestClient", "Paozhu sms plugin 1.0");
    a.add_header("X-TC-Timestamp", std::to_string(timestamp));
    a.add_header("X-TC-Version", version);
    a.add_header("X-TC-Region", region);
    a.add_header("Authorization", authorization);
    a.send();
    if (a.get_status() == 200)
    {
        resp_content = a.get_body();
        return true;
    }

    return false;
}

send_sms::send_sms(const std::string &service_type) : service(service_type) {}

void send_sms::set_service(const std::string &service_type)
{
    service = service_type;
}

bool send_sms::send(send_sms_info_t &ssinfo)
{
    if (http::str_casecmp(service, "aliyun"))
    {
        return aliyun_send(ssinfo);
    }
    else if (http::str_casecmp(service, "tencent"))
    {
        return tencent_send(ssinfo);
    }

    return false;
}

bool send_sms::aliyun_send(send_sms_info_t &ssinfo)
{
    http::client a;
    http::obj_val parameter;
    a.add_header("x-acs-version", "2017-05-25");
    a.add_header("x-acs-action", "SendSms");
    a.add_header("user-agent", "Paozhu server");
    std::string timestr = http::get_utctime();
    std::string noncestr = http::md5(timestr + http::rand_string(8));
    a.add_header("x-acs-date", timestr);
    a.add_header("x-acs-signature-nonce", noncestr);

    unsigned char sSHA256[SHA256_DIGEST_LENGTH] = {0};
    SHA256((const unsigned char *)"", 0, sSHA256);

    std::string sha256str = http::char2hex(&sSHA256[0], SHA256_DIGEST_LENGTH);
    a.add_header("x-acs-content-sha256", sha256str);
    a.add_header("accept", "application/json");

    std::map<std::string, std::string> headers_hash, query_hash;
    headers_hash["content-type"] = "application/json";
    headers_hash["host"] = "dysmsapi.aliyuncs.com";
    headers_hash["x-acs-version"] = "2017-05-25";
    headers_hash["x-acs-action"] = "SendSms";
    headers_hash["x-acs-date"] = timestr;
    headers_hash["x-acs-signature-nonce"] = noncestr;
    headers_hash["x-acs-content-sha256"] = sha256str;

    std::string signHeaders_hash_str, canonicalHeaderString, canonicalRequest, signatureAlgorithm;
    std::string accessKeyId = aliyun_sms_config().accessKeyId;
    std::string accessKeySecret = aliyun_sms_config().accessKeySecret;

    for (auto &[first, second] : headers_hash)
    {
        if (signHeaders_hash_str.size() > 0)
        {
            signHeaders_hash_str.push_back(';');
        }
        signHeaders_hash_str.append(first);

        canonicalHeaderString.append(first);
        canonicalHeaderString.push_back(':');
        canonicalHeaderString.append(second);
        canonicalHeaderString.push_back('\n');
    }

    signatureAlgorithm = "ACS3-HMAC-SHA256";
    canonicalRequest.append("POST\n");
    canonicalRequest.append("/\n");

    for (auto &obj : ssinfo.params)
    {
        parameter[obj.first] = obj.second;
    }

    query_hash["PhoneNumbers"] = ssinfo.mobile;
    query_hash["SignName"] = ssinfo.sign_name;
    query_hash["TemplateCode"] = ssinfo.template_code;
    query_hash["TemplateParam"] = parameter.to_json();

    a.post_json("https://dysmsapi.aliyuncs.com/");
    a.build_query(query_hash);

    canonicalRequest.append(a.get_query());
    canonicalRequest.append("\n");
    canonicalRequest.append(canonicalHeaderString);
    canonicalRequest.append("\n");
    canonicalRequest.append(signHeaders_hash_str);
    canonicalRequest.append("\n");
    canonicalRequest.append(sha256str);

    SHA256((const unsigned char *)&canonicalRequest[0], canonicalRequest.size(), sSHA256);
    sha256str = http::char2hex(&sSHA256[0], SHA256_DIGEST_LENGTH);

    std::string hmac_str;
    hmac_str.append(signatureAlgorithm);
    hmac_str.push_back('\n');
    hmac_str.append(sha256str);

    unsigned char sSHA[EVP_MAX_MD_SIZE] = {0};
    unsigned int nSHALen = EVP_MAX_MD_SIZE;

    HMAC(EVP_sha256(), (const unsigned char *)&accessKeySecret[0], accessKeySecret.size(),
         (const unsigned char *)&hmac_str[0], hmac_str.size(), sSHA, &nSHALen);
    sha256str = http::char2hex(&sSHA[0], nSHALen);
    std::string Authorization;

    Authorization.append(signatureAlgorithm);
    Authorization.append(" Credential=");
    Authorization.append(accessKeyId);
    Authorization.append(",SignedHeaders=");
    Authorization.append(signHeaders_hash_str);
    Authorization.append(",Signature=");
    Authorization.append(sha256str);

    a.add_header("Authorization", Authorization);
    a.send();
    if (a.get_status() == 200)
    {
        ssinfo.respbody = a.get_body();
        return true;
    }
    return false;
}

bool send_sms::tencent_send(send_sms_info_t &ssinfo)
{
    http::client a;
    std::string SECRET_ID = tencent_sms_config().accessKeyId;
    std::string SECRET_KEY = tencent_sms_config().accessKeySecret;

    std::string sms_service = "sms";
    std::string sms_region = ssinfo.region.empty() ? "ap-guangzhou" : ssinfo.region;
    std::string host = "sms." + sms_region + ".tencentcloudapi.com";
    std::string action = "SendSms";
    std::string version = "2021-01-11";
    int64_t timestamp = time(NULL);
    std::string date = http::get_date("%Y-%m-%d", (unsigned int)timestamp);

    unsigned char sSHA[EVP_MAX_MD_SIZE] = {0};
    unsigned char sSHA256[SHA256_DIGEST_LENGTH] = {0};
    unsigned int nSHALen = EVP_MAX_MD_SIZE;

    std::string httpRequestMethod = "POST";
    std::string canonicalUri = "/";
    std::string canonicalQueryString = "";
    std::string lower = action;
    std::transform(action.begin(), action.end(), lower.begin(), ::tolower);

    std::string canonicalHeaders =
        std::string("content-type:application/json\n") + "host:" + host + "\n" + "x-tc-action:" + lower + "\n";
    std::string signedHeaders = "content-type;host;x-tc-action";

    std::stringstream _stream;
    _stream << "{\"PhoneNumberSet\":[\"" << ssinfo.mobile << "\"],\"SmsSdkAppId\":\"";
    _stream << ssinfo.sms_sdk_app_id << "\",";
    _stream << "\"TemplateId\":\"" << ssinfo.template_code << "\",";
    _stream << "\"SignName\":\"" << http::utf8_to_jsonstring(ssinfo.sign_name) << "\",";
    _stream << "\"TemplateParamSet\":[";

    for (auto it = ssinfo.params.begin(); it != ssinfo.params.end(); ++it)
    {
        if (it != ssinfo.params.begin())
        {
            _stream << ",";
        }
        _stream << "\"" << http::utf8_to_jsonstring(it->second) << "\"";
    }

    _stream << "]}";
    std::string payload = _stream.str();

    std::string hashedRequestPayload;
    SHA256((const unsigned char *)&payload[0], payload.size(), sSHA256);
    hashedRequestPayload = http::char2hex(sSHA256, SHA256_DIGEST_LENGTH);

    std::string canonicalRequest = httpRequestMethod + "\n" + canonicalUri + "\n" + canonicalQueryString + "\n" +
                                   canonicalHeaders + "\n" + signedHeaders + "\n" + hashedRequestPayload;

    std::string algorithm = "TC3-HMAC-SHA256";
    std::string credentialScope = date + "/" + sms_service + "/" + "tc3_request";

    SHA256((const unsigned char *)&canonicalRequest[0], canonicalRequest.size(), sSHA256);
    std::string hashedCanonicalRequest = http::char2hex(sSHA256, SHA256_DIGEST_LENGTH);

    std::string stringToSign = algorithm + "\n" + std::to_string(timestamp) + "\n" + credentialScope + "\n" + hashedCanonicalRequest;

    std::string kKey = "TC3" + SECRET_KEY;
    static const char *tc3_request = "tc3_request";

    HMAC(EVP_sha256(), (const unsigned char *)&kKey[0], kKey.size(),
         (const unsigned char *)&date[0], date.size(), sSHA, &nSHALen);

    HMAC(EVP_sha256(), sSHA, nSHALen, (const unsigned char *)&sms_service[0], sms_service.size(), sSHA, &nSHALen);

    HMAC(EVP_sha256(), sSHA, nSHALen, (const unsigned char *)tc3_request, strlen(tc3_request), sSHA, &nSHALen);

    HMAC(EVP_sha256(), sSHA, nSHALen, (const unsigned char *)&stringToSign[0], stringToSign.size(), sSHA, &nSHALen);
    std::string signature = http::char2hex((const unsigned char *)sSHA, nSHALen);

    std::string authorization = algorithm + " " + "Credential=" + SECRET_ID + "/" + credentialScope + ", " +
                                "SignedHeaders=" + signedHeaders + ", " + "Signature=" + signature;

    a.post_json("https://" + host + "/");

    a.add_header("User-Agent", "Paozhu webclient");
    a.add_header("X-TC-Action", action);
    a.add_header("X-TC-RequestClient", "Paozhu sms plugin 1.0");
    a.add_header("X-TC-Timestamp", std::to_string(timestamp));
    a.add_header("X-TC-Version", version);
    a.add_header("X-TC-Region", sms_region);
    a.add_header("Authorization", authorization);
    a.send();
    if (a.get_status() == 200)
    {
        ssinfo.respbody = a.get_body();
        return true;
    }

    return false;
}

}
