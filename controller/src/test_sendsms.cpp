#include <chrono>
#include <thread>
#include <tuple>
#include "httppeer.h"
#include "test_sendsms.h"
#ifdef ENABLE_SMS
#include "sms.h"
#endif
namespace http
{

std::string testsendsms(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<p>hello world!</p>";
#ifdef ENABLE_SMS
    sms::aliyun_sms_config().accessKeyId = "你的阿里云AccessKeyId";
    sms::aliyun_sms_config().accessKeySecret = "你的阿里云AccessKeySecret";

    sms::send_sms_info_t ssinfo;
    ssinfo.mobile = "13311112222";
    ssinfo.sign_name = "你的短信签名";
    ssinfo.template_code = "你的短信模板CODE";
    ssinfo.params.push_back({"name", "螺蛳粉"});
    ssinfo.params.push_back({"loginname", "msdfaassss@aaa.com"});
    ssinfo.params.push_back({"passwd", "msdfaa@sssddf"});
    ssinfo.params.push_back({"time", "不限时"});

    sms::send_sms sms_obj("aliyun");
    bool issend = sms_obj.send(ssinfo);
    client << "<p>发送结果: " << (issend ? "成功" : "失败") << "</p>";
    client << "<p>响应: " << ssinfo.respbody << "</p>";
#endif
    return "";
}

std::string testtencentsms(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<p>腾讯云短信测试</p>";
#ifdef ENABLE_SMS
    sms::tencent_sms_config().accessKeyId = "你的腾讯云SecretId";
    sms::tencent_sms_config().accessKeySecret = "你的腾讯云SecretKey";

    sms::send_sms_info_t ssinfo;
    ssinfo.mobile = "13311112222";
    ssinfo.sign_name = "你的短信签名";
    ssinfo.template_code = "你的短信模板ID";
    ssinfo.sms_sdk_app_id = "你的SDKAppID";
    ssinfo.region = "ap-guangzhou";
    ssinfo.params.push_back({"name", "螺蛳粉"});
    ssinfo.params.push_back({"time", "不限时"});

    sms::send_sms sms_obj("tencent");
    bool issend = sms_obj.send(ssinfo);
    client << "<p>发送结果: " << (issend ? "成功" : "失败") << "</p>";
    client << "<p>响应: " << ssinfo.respbody << "</p>";
#endif
    return "";
}

}
