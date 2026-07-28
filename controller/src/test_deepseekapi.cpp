#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "httpclient.h"
#include "func.h"
#include <memory>
#include <string>
#include "test_deepseekapi.h"


namespace http
{
//@urlpath(null,deepseek_async)
std::string test_deepseek_async(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<html><body>";

    const char* api_key = "sk-********************";//std::getenv("DEEPSEEK_API_KEY");
    if (!api_key || strlen(api_key) == 0) {
        client << "错误：请设置环境变量 DEEPSEEK_API_KEY";
        client << "</body></html>";
        return "";
    }

    auto http = std::make_shared<http::client>();

    http->post("https://api.deepseek.com/chat/completions");
    http->add_header("Authorization", std::string("Bearer ") + api_key);
    http->add_header("Content-Type", "application/json");

    http::obj_val request_body;
    request_body["model"] = "deepseek-v4-pro";

    http::obj_val messages;
    messages.set_array();
    http::obj_val sys_msg;
    sys_msg["role"] = "system";
    sys_msg["content"] = "You are a helpful assistant.";
    messages.push(sys_msg);

    http::obj_val user_msg;
    user_msg["role"] = "user";
    user_msg["content"] = "请介绍一下你自己。";
    messages.push(user_msg);

    request_body["messages"] = messages;
    request_body["stream"] = false;

    http->set_body(request_body.to_json());
    client <<"<div>";
    client << request_body.to_json();
    client <<"</div>";
    client << "正在异步请求 DeepSeek API...<br>";
    http->send();   // 异步发送

    if (http->get_status() == 200) {
        if (http->page.json.is_obj()) {
            if (http->page.json.isset("choices") && http->page.json["choices"].is_array()) {
                auto &choices = http->page.json["choices"];
                if (choices.size() > 0) {
                    auto &first = choices[0];
                    if (first.isset("message") && first["message"].is_obj()) {
                        std::string reply = first["message"]["content"].to_string();
                        client << "<br><b>DeepSeek 回复：</b><br><pre>" << reply << "</pre>";
                    } else {
                        client << "响应缺少 message 字段。";
                    }
                } else {
                    client << "choices 数组为空。";
                }
            } else {
                client << "响应缺少 choices 字段。";
            }
        } else {
            client << "响应不是有效的 JSON 对象。";
        }
    } else {
        client << "HTTP 错误 " << http->get_status() << "<br>";
        client << "错误信息：" << http->error_msg;
    }

    client << "</body></html>";
    return "";
}

}// namespace http