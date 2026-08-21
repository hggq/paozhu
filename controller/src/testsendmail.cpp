#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testsendmail.h"
#include "send_email.h"
#include "async_send_email.h"
namespace http
{
/**
 * @brief 同步方式发送邮件示例
 *
 * 使用 http::send_email 类以同步（阻塞）方式发送邮件。
 * 适用于对发送耗时不太敏感、并发量较低的场景。
 * 邮件内容默认为 HTML 格式（Content-Type: text/html）。
 *
 * 使用步骤：
 *   1. 配置 SMTP 服务器信息（地址、端口、是否 SSL、账号、密码）；
 *   2. 设置发件人、收件人、主题、正文等邮件参数；
 *   3. 可选：调用 addattachments() 添加附件；
 *   4. 调用 send() 触发发送，返回 true 表示成功，false 表示失败，
 *      失败原因保存在 errormsg 中。
 *
 * 注意：
 *   - 同步发送会阻塞当前线程直到整个 SMTP 会话结束；
 *   - 常用 SMTP 端口：明文 25/587，SSL 465；
 *   - 网易/腾讯等邮箱需要开启 SMTP 服务并填写“授权码”而非登录密码；
 *   - 若需要不阻塞当前线程的高并发发送，请改用协程版 async_send_email。
 */
//@urlpath(null,testsendmaildo)
std::string testsendmaildo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<p>hello world!</p>";
    client << "<p>";
    client << (unsigned int)client.post.size();
    client << "</p>";
    for (auto &[first, second] : client.post.as_object())
    {
        client << "<p>";
        client << client.post[first];
        client << "</p>";
    }

    send_email s;

    // 以下为发送前的参数配置，请替换为真实的值。
    s.isssl   = false; // ssl on off   ssl port 465，若开启 SSL 请将 port 设为 465
    s.smpturl = "smtp.163.com"; // SMTP 服务器地址
    s.port    = 25;             // SMTP 端口（明文 25/587，SSL 465）

    s.username = "XXX@163.com"; // 邮箱账号（发件人账号）
    s.password = "XXXXXXX";     // 邮箱密码（网易/腾讯需使用授权码）

    s.replyemail = "XXX@163.com";   // 回复邮箱地址（收件人点击“回复”时使用的地址）
    s.fromname   = "web site name"; // 发件人显示名称

    s.toemail = client.post["toemail"].to_string(); // 收件人邮箱地址（可从前端表单获取）
    s.toname  = client.post["toname"].to_string();  // 收件人显示名称

    s.subject = client.post["subject"].to_string();  // 邮件主题
    s.content = client.post["msgbody"].to_string();  // 邮件正文（默认为 HTML 内容）

    // 可选：添加附件（传入本地文件路径，文件必须存在）
    // s.addattachments("会议议程安排.docx");

    // 同步发送：此处会阻塞直到发送完成
    if (s.send())
    {
        client << "<p>send ok</p>";
    }
    else
    {
        client << "<p>";
        client << s.errormsg;
        client << "</p>";
        // std::cout<<s.errormsg<<std::endl;
    }

    return "";
}

//@urlpath(null,testcosendmaildo)
asio::awaitable<std::string> test_co_sendmaildo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<p>hello world!</p>";
    client << "<p>";
    client << (unsigned int)client.post.size();
    client << "</p>";
    for (auto &[first, second] : client.post.as_object())
    {
        client << "<p>";
        client << client.post[first];
        client << "</p>";
    }

    async_send_email s;

    // 以下为发送前的参数配置，请替换为真实的值。
    s.isssl   = false; // ssl on off   ssl port 465，若开启 SSL 请将 port 设为 465
    s.smpturl = "smtp.163.com"; // SMTP 服务器地址
    s.port    = 25;             // SMTP 端口（明文 25/587，SSL 465）

    s.username = "XXX@163.com"; // 邮箱账号（发件人账号）
    s.password = "XXXXXXX";     // 邮箱密码（网易/腾讯需使用授权码）

    s.replyemail = "XXX@163.com";   // 回复邮箱地址（收件人点击“回复”时使用的地址）
    s.fromname   = "web site name"; // 发件人显示名称

    s.toemail = client.post["toemail"].to_string(); // 收件人邮箱地址（可从前端表单获取）
    s.toname  = client.post["toname"].to_string();  // 收件人显示名称

    s.subject = client.post["subject"].to_string();  // 邮件主题
    s.content = client.post["msgbody"].to_string();  // 邮件正文（默认为 HTML 内容）

    // 可选：添加附件（传入本地文件路径，文件必须存在）
    // s.addattachments("会议议程安排.docx");

    // 同步发送：此处会阻塞直到发送完成
    if (co_await s.async_send())
    {
        client << "<p>send ok</p>";
    }
    else
    {
        client << "<p>";
        client << s.errormsg;
        client << "</p>";
        // std::cout<<s.errormsg<<std::endl;
    }

    co_return "";
}

}// namespace http