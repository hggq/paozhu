#ifndef LIBS_SMS_TYPES_SEND_SMS_TYPE_H
#define LIBS_SMS_TYPES_SEND_SMS_TYPE_H
#include <string>
#include <list>
#include <tuple>
#include <map>
#include <vector>

namespace sms
{

struct send_sms_info_t
{
    std::string mobile;
    std::string sendid;
    std::string sign_name;
    std::string template_code;
    std::string sms_sdk_app_id;
    std::string region;
    std::list<std::pair<std::string, std::string>> params;
    std::string respbody;
};

struct aliyun_sms_config_t
{
    std::string host;
    std::string accessKeyId;
    std::string accessKeySecret;
};

struct tencent_sms_config_t
{
    std::string host;
    std::string accessKeyId;
    std::string accessKeySecret;
};

class aliyun_sendsms
{
public:
    aliyun_sendsms();
    bool send();

public:
    std::string PhoneNumbers;
    std::string SignName;
    std::string TemplateCode;
    std::map<std::string, std::string> TemplateParam;

    std::string resp_content;
    std::string RequestId;
};

class tencent_sendsms
{
public:
    tencent_sendsms();
    bool send();
    std::string get_parameter_json();

public:
    std::string PhoneNumberSet;
    std::string SmsSdkAppId;
    std::string TemplateId;
    std::string SignName;
    std::vector<std::string> TemplateParamSet;

    std::string resp_content;
    std::string RequestId;

    std::string host;
    std::string accessKeyId;
    std::string accessKeySecret;
};

class send_sms
{
public:
    send_sms(const std::string &service_type = "aliyun");
    bool aliyun_send(send_sms_info_t &ssinfo);
    bool tencent_send(send_sms_info_t &ssinfo);
    void set_service(const std::string &);
    bool send(send_sms_info_t &ssinfo);

public:
    std::string service;
};

aliyun_sms_config_t &aliyun_sms_config();
tencent_sms_config_t &tencent_sms_config();
send_sms &get_send_sms_obj();

}

#endif
