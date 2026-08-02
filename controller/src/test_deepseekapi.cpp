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
//@urlpath(null,deepseek_api)
std::string test_deepseek_api(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<html><body>";

    const char *api_key = "sk-********************";//std::getenv("DEEPSEEK_API_KEY");
    if (!api_key || strlen(api_key) == 0)
    {
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
    sys_msg["role"]    = "system";
    sys_msg["content"] = "You are a helpful assistant.";
    messages.push(sys_msg);

    http::obj_val user_msg;
    user_msg["role"]    = "user";
    user_msg["content"] = "请介绍一下你自己。";
    messages.push(user_msg);

    request_body["messages"] = messages;
    request_body["stream"]   = false;

    http->set_body(request_body.to_json());
    client << "<div>";
    client << request_body.to_json();
    client << "</div>";
    client << "正在异步请求 DeepSeek API...<br>";
    http->send();// 异步发送

    if (http->get_status() == 200)
    {
        if (http->page.json.is_obj())
        {
            if (http->page.json.isset("choices") && http->page.json["choices"].is_array())
            {
                auto &choices = http->page.json["choices"];
                if (choices.size() > 0)
                {
                    auto &first = choices[0];
                    if (first.isset("message") && first["message"].is_obj())
                    {
                        std::string reply = first["message"]["content"].to_string();
                        client << "<br><b>DeepSeek 回复：</b><br><pre>" << reply << "</pre>";
                    }
                    else
                    {
                        client << "响应缺少 message 字段。";
                    }
                }
                else
                {
                    client << "choices 数组为空。";
                }
            }
            else
            {
                client << "响应缺少 choices 字段。";
            }
        }
        else
        {
            client << "响应不是有效的 JSON 对象。";
        }
    }
    else
    {
        client << "HTTP 错误 " << http->get_body() << "<br>";
        client << "HTTP 错误 " << http->get_status() << "<br>";
        client << "错误信息：" << http->error_msg;
    }

    client << "</body></html>";
    return "";
}

//@urlpath(null,deepseek_async)
asio::awaitable<std::string> test_deepseek_async(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<html><body>";

    const char *api_key = "sk-********************";//std::getenv("DEEPSEEK_API_KEY");
    if (!api_key || strlen(api_key) == 0)
    {
        client << "错误：请设置环境变量 DEEPSEEK_API_KEY";
        client << "</body></html>";
        co_return "";
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
    sys_msg["role"]    = "system";
    sys_msg["content"] = "You are a helpful assistant.";
    messages.push(sys_msg);

    http::obj_val user_msg;
    user_msg["role"]    = "user";
    user_msg["content"] = "请介绍一下你自己。";
    messages.push(user_msg);

    request_body["messages"] = messages;
    request_body["stream"]   = false;

    http->set_body(request_body.to_json());
    client << "<div>";
    client << request_body.to_json();
    client << "</div>";
    client << "正在异步请求 DeepSeek API...<br>";
    co_await http->async_send();// 异步发送

    if (http->get_status() == 200)
    {
        if (http->page.json.is_obj())
        {
            if (http->page.json.isset("choices") && http->page.json["choices"].is_array())
            {
                auto &choices = http->page.json["choices"];
                if (choices.size() > 0)
                {
                    auto &first = choices[0];
                    if (first.isset("message") && first["message"].is_obj())
                    {
                        std::string reply = first["message"]["content"].to_string();
                        client << "<br><b>DeepSeek 回复：</b><br><pre>" << reply << "</pre>";
                    }
                    else
                    {
                        client << "响应缺少 message 字段。";
                    }
                }
                else
                {
                    client << "choices 数组为空。";
                }
            }
            else
            {
                client << "响应缺少 choices 字段。";
            }
        }
        else
        {
            client << "响应不是有效的 JSON 对象。";
        }
    }
    else
    {
        client << "HTTP 错误 " << http->get_status() << "<br>";
        client << "错误信息：" << http->error_msg;
    }

    client << "</body></html>";
    co_return "";
}

//@urlpath(null,deepseek_sse)
asio::awaitable<std::string> test_deepseek_sse(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    const char *api_key = "sk-********************";
    if (!api_key || strlen(api_key) == 0)
    {
        co_return "";
    }

    // 1. 先发送 SSE 响应头
    co_await client.async_make_sse_header();

    // 2. 创建 httpclient 请求 DeepSeek
    auto http = std::make_shared<http::client>();

    http->post("https://api.deepseek.com/chat/completions");
    http->add_header("Authorization", std::string("Bearer ") + api_key);
    http->add_header("Content-Type", "application/json");

    http::obj_val request_body;
    request_body["model"] = "deepseek-v4-pro";

    http::obj_val messages;
    messages.set_array();
    http::obj_val sys_msg;
    sys_msg["role"]    = "system";
    sys_msg["content"] = "You are a helpful assistant.";
    messages.push(sys_msg);

    http::obj_val user_msg;
    user_msg["role"]    = "user";
    user_msg["content"] = "请介绍一下你自己。";
    messages.push(user_msg);

    request_body["messages"] = messages;
    request_body["stream"]   = true;

    http->set_body(request_body.to_json());

    // 3. 设置 SSE 回调：每收到一个事件就转发给客户端
    http->on_sse_event = [&client](std::string &respdata, std::shared_ptr<http::client>)
    {
        // respdata 格式为 "data: <content>"，去掉 "data: " 前缀（5 个字符）
        if (respdata.size() < 6)
            return;

        std::string data;
        unsigned int i = 5;
        for (; i < respdata.size(); i++)
        {
            if (respdata[i] == ' ' || respdata[i] == '\r' || respdata[i] == '\n')
            {
                continue;
            }
            data = respdata.substr(i);
            break;
        }

        if (data == "[DONE]")
        {
            client.make_sse_body("[DONE]");
            client.make_sse_end();
            return;
        }

        http::obj_val chunk;
        try
        {
            chunk.from_json(data);
        }
        catch (...)
        {
            client.make_sse_body("JSON 解析错误: " + data);
            return;
        }
        if (chunk.isset("choices") && chunk["choices"].is_array() && chunk["choices"].size() > 0)
        {
            auto &choice = chunk["choices"][0];
            if (choice.isset("delta") && choice["delta"].is_obj())
            {
                if (choice["delta"].isset("content"))
                {
                    std::string content = choice["delta"]["content"].to_string();
                    client.make_sse_body(content);
                }
            }
        }
    };

    // 4. 异步发送请求（会阻塞直到 SSE 流结束或出错）
    co_await http->async_send();

    // 5. 如果请求失败，发送错误事件
    if (http->get_status() != 200)
    {
        client.make_sse_body("HTTP Error from sse: " + std::to_string(http->get_status()) + ": " + http->error_msg);
        client.make_sse_body("[DONE]");
        client.make_sse_end();
    }

    co_return "";
}

//@urlpath(null,deepseek_chunk)
asio::awaitable<std::string> test_deepseek_chunk(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    const char *api_key = "sk-********************";
    if (!api_key || strlen(api_key) == 0)
    {
        co_return "";
    }

    // 1. 先发送 SSE 响应头
    client.type("text/html; charset=utf-8");
    co_await client.async_send_chunk_header();

    // 2. 创建 httpclient 请求 DeepSeek
    auto http = std::make_shared<http::client>();

    http->post("https://api.deepseek.com/chat/completions");
    http->add_header("Authorization", std::string("Bearer ") + api_key);
    http->add_header("Content-Type", "application/json");

    http::obj_val request_body;
    request_body["model"] = "deepseek-v4-pro";

    http::obj_val messages;
    messages.set_array();
    http::obj_val sys_msg;
    sys_msg["role"]    = "system";
    sys_msg["content"] = "You are a helpful assistant.";
    messages.push(sys_msg);

    http::obj_val user_msg;
    user_msg["role"]    = "user";
    user_msg["content"] = "请介绍一下你自己。";
    messages.push(user_msg);

    request_body["messages"] = messages;
    request_body["stream"]   = true;

    http->set_body(request_body.to_json());

    // 3. 设置 SSE 回调：每收到一个事件就转发给客户端
    http->async_on_sse_event = [&client](std::string &respdata, std::shared_ptr<http::client> httpclient_obj) -> asio::awaitable<void>
    {
        // respdata 格式为 "data: <content>"，去掉 "data: " 前缀（5 个字符）
        if (respdata.size() < 6)
        {
            co_await client.async_send_chunk_end();
            httpclient_obj->page.isend = true;
            co_return;
        }
        std::string data;
        unsigned int i = 5;
        for (; i < respdata.size(); i++)
        {
            if (respdata[i] == ' ' || respdata[i] == '\r' || respdata[i] == '\n')
            {
                continue;
            }
            data = respdata.substr(i);
            break;
        }

        if (data == "[DONE]")
        {
            co_await client.async_send_chunk_end();
            httpclient_obj->page.isend = true;
            co_return;
        }

        http::obj_val chunk;
        bool istry = false;
        try
        {
            chunk.from_json(data);
        }
        catch (...)
        {
            istry = true;
            co_return;
        }
        
        if(istry)
        {
            co_await client.async_send_chunk_body("JSON 解析错误: " + data);
            co_await client.async_send_chunk_end();
            httpclient_obj->page.isend = true;
            co_return;
        }
        

        if (chunk.isset("choices") && chunk["choices"].is_array() && chunk["choices"].size() > 0)
        {
            auto &choice = chunk["choices"][0];
            if (choice.isset("delta") && choice["delta"].is_obj())
            {
                if (choice["delta"].isset("content"))
                {
                    std::string content = choice["delta"]["content"].to_string();
                    if(content.size() > 0)
                    {
                        co_await client.async_send_chunk_body(content);
                    }
                }
            }
        }
        co_return;
    };

    // 4. 异步发送请求（会阻塞直到 SSE 流结束或出错）
    co_await http->async_send();

    // 5. 如果请求失败，发送错误事件
    if (http->get_status() != 200 && !http->page.isend)
    {
        co_await client.async_send_chunk_body("HTTP Error from sse: " + std::to_string(http->get_status()) + ": " + http->error_msg);
        co_await client.async_send_chunk_end();
    }

    co_return "";
}

}// namespace http