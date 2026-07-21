#include <cstdarg>
#include <cstring>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include "base64.h"
#include "http_socket.h"
#include "async_send_email.h"
#include "func.h"
#include "client_context.h"

namespace http
{

/**
 * @brief 添加邮件附件
 * 
 * 检查附件文件是否存在，存在则添加到附件列表中
 * @param file_name 附件文件路径
 * @return true表示添加成功，false表示文件不存在
 */
bool async_send_email::addattachments(std::string file_name)
{
    FILE *ffile = fopen(file_name.c_str(), "rb");
    if (!ffile)
    {
        return false;
    }
    fclose(ffile);
    attachments.push_back(file_name);
    return true;
}

/**
 * @brief 获取当前GMT时间字符串
 * 
 * 生成符合RFC 2822标准的日期格式，用于邮件的Date头部字段
 * @return GMT时间字符串，格式如 "Thu, 01 Jan 1970 00:00:00 GMT"
 */
std::string async_send_email::getgmttime()
{
    time_t curr_time;
    curr_time = time((time_t *)NULL);
    tm *timeInfo;
    char timestr[30] = {'\0'};
    timeInfo = gmtime(&curr_time);
    strftime(timestr, sizeof(timestr), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    std::string temp(timestr);
    return temp;
}

/**
 * @brief SMTP状态机命令生成函数
 * 
 * 根据当前会话状态生成对应的SMTP协议命令，包括：
 * - SEND_EHLO: 发送HELO命令
 * - SEND_AUTH_LOGIN: 发送AUTH LOGIN命令
 * - SEND_USER: 发送base64编码的用户名
 * - SEND_PASSWORD: 发送base64编码的密码
 * - SEND_MAIL_FROM: 发送MAIL FROM命令
 * - SEND_RCPT_TO: 发送RCPT TO命令
 * - SEND_DATA: 发送DATA命令
 * - SEND_EMAIL: 构建并发送完整的邮件内容（含MIME头、正文和附件）
 * @return true表示处理成功
 */
bool async_send_email::sendloop()
{
    unsigned int n;
    switch (state)
    {
    case SEND_EHLO:
        sendcommand.append("HELO ");
        sendcommand.append(smpturl);
        sendcommand.append("\r\n");
        break;

    case SEND_AUTH_LOGIN:
        sendcommand.append("AUTH LOGIN\r\n");
        break;

    case SEND_USER:
        sendcommand.clear();
        sendcommand.append(base64_encode(username.data(), username.size()));
        sendcommand.append("\r\n");
        break;

    case SEND_PASSWORD:
        sendcommand.clear();
        sendcommand.append(base64_encode(password.data(), password.size()));
        sendcommand.append("\r\n");
        break;

    case SEND_MAIL_FROM:
        sendcommand.clear();
        sendcommand.append("MAIL FROM:<");
        sendcommand.append(username);
        sendcommand.append(">\r\n");
        break;

    case SEND_RCPT_TO:
        sendcommand.append("RCPT TO:<");
        sendcommand.append(toemail);
        sendcommand.append(">\r\n");
        break;

    case SEND_DATA:
        sendcommand.append("DATA\r\n");
        break;

    case SEND_EMAIL:
        if (attachments.size() > 0)
        {
            sendcommand.clear();
            sendcommand.append("MIME-Version: 1.0\r\n");
            sendcommand.append("From: =?UTF-8?B?");
            sendcommand.append(base64_encode(fromname.data(), fromname.size(), 0));
            sendcommand.append("?= <");
            sendcommand.append(username);
            sendcommand.append(">\r\n");

            sendcommand.append("To: =?UTF-8?B?");
            sendcommand.append(base64_encode(toname.data(), toname.size(), 0));
            sendcommand.append("?= <");
            sendcommand.append(toemail);
            sendcommand.append(">\r\n");

            sendcommand.append("Date: ");
            sendcommand.append(getgmttime());
            sendcommand.append("\r\n");

            sendcommand.append("Subject: ");
            sendcommand.append(subject);
            sendcommand.append("\r\n");

            sendcommand.append("Reply-To: <");
            sendcommand.append(replyemail);
            sendcommand.append(">\r\n");

            srand(time(0));
            boundarystsr.clear();
            std::stringstream ostr;
            unsigned int c = rand() * 10000000;
            ostr << c;
            c = rand() * 10000000;
            ostr << c;
            boundarystsr.append(ostr.str());

            sendcommand.append("Message-ID: <");
            sendcommand.append(boundarystsr);
            sendcommand.append(replyemail);
            sendcommand.append(">\r\n");

            boundarystsr.append("_paozhu_");

            sendcommand.append("Content-Type: multipart/mixed; boundary=\"");
            sendcommand.append(boundarystsr);
            sendcommand.append("\"\r\n\r\n--");
            sendcommand.append(boundarystsr);
            sendcommand.append("\r\n");
            sendcommand.append("Content-Type: text/html; charset=UTF-8\r\n\r\n");
            sendcommand.append(content);
            sendcommand.append("\r\n");

            n = 0;
            sendfiles.resize(attachments.size());
            for (; n < attachments.size(); n++)
            {
                FILE *ffile = fopen(attachments[n].c_str(), "rb");
                if (!ffile)
                {
                    n -= 1;
                    continue;
                }
                fclose(ffile);

                unsigned int k = 0;
                std::string filename_temp;
                for (k = attachments[n].size(); k > 0; k--)
                {
                    if (attachments[n][k] == '/' || attachments[n][k] == '\\')
                    {
                        if (k < attachments[n].size())
                        {
                            k += 1;
                        }
                        break;
                    }
                }
                filename_temp.append(attachments[n].substr(k));

                sendfiles[n].predata.append("\r\n--");
                sendfiles[n].predata.append(boundarystsr);
                sendfiles[n].predata.append("\r\nContent-Type: application/octet-stream;\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; filename=\"=?UTF-8?B?");
                sendfiles[n].predata.append(base64_encode(filename_temp.data(), filename_temp.size()));
                sendfiles[n].predata.append("?=\"\r\n\r\n");
                sendfiles[n].filename = attachments[n];
            }
            sendfiles.resize(n);
        }
        else
        {
            sendcommand.clear();
            sendcommand.append("MIME-Version: 1.0\r\n");
            sendcommand.append("Content-Type: text/html; charset=utf-8\r\n");
            sendcommand.append("From: =?UTF-8?B?");
            sendcommand.append(base64_encode(fromname.data(), fromname.size(), 0));
            sendcommand.append("?= <");
            sendcommand.append(username);
            sendcommand.append(">\r\n");

            sendcommand.append("To: =?UTF-8?B?");
            sendcommand.append(base64_encode(toname.data(), toname.size(), 0));
            sendcommand.append("?= <");
            sendcommand.append(toemail);
            sendcommand.append(">\r\n");

            sendcommand.append("Date: ");
            sendcommand.append(getgmttime());
            sendcommand.append("\r\n");

            sendcommand.append("Subject: ");
            sendcommand.append(subject);
            sendcommand.append("\r\n");

            sendcommand.append("Reply-To: <");
            sendcommand.append(replyemail);
            sendcommand.append(">\r\n");

            srand(time(0));
            boundarystsr.clear();
            std::stringstream ostr;
            unsigned int c = rand() * 10000000;
            ostr << c;

            sendcommand.append("Message-ID: <");
            sendcommand.append(ostr.str());
            sendcommand.append(replyemail);
            sendcommand.append(">\r\n\r\n");

            sendcommand.append(content);
            sendcommand.append("\r\n\r\n.\r\n");
        }
        break;

    default:
        break;
    }
    return true;
}

/**
 * @brief 协程版SSL加密连接发送邮件
 * 
 * 使用OpenSSL和ASIO协程建立SSL/TLS连接，通过状态机模式执行SMTP协议流程：
 * 连接 -> 握手 -> EHLO -> AUTH LOGIN -> 发送用户名/密码 -> MAIL FROM -> RCPT TO -> DATA -> 发送邮件内容 -> QUIT
 * @return asio::awaitable<bool> true表示发送成功，false表示失败
 */
asio::awaitable<bool> async_send_email::async_send()
{
    try
    {
        if (isssl)
        {
            ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
            sslsock = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(strand_, *ssl_context);
            ssl_context->set_default_verify_paths();

            asio::ip::tcp::resolver resolver(strand_);
            SSL_set_tlsext_host_name(sslsock->native_handle(), smpturl.c_str());

            constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
            auto endpoints = co_await resolver.async_resolve(smpturl.c_str(), std::to_string(port), asio::use_awaitable);

            for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
            {
                std::tie(ec) = co_await sslsock->lowest_layer().async_connect(*iter, tuple_awaitable);
                if (ec)
                {
                    continue;
                }
                break;
            }

            if (ec)
            {
                errormsg.append("ssl connect error!");
                co_return false;
            }

            ssl_context->set_verify_mode(asio::ssl::verify_peer);
            ssl_context->set_verify_callback(asio::ssl::host_name_verification(smpturl));

            std::tie(ec) = co_await sslsock->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
            if (ec)
            {
                errormsg.append("ssl handshake error!");
                co_return false;
            }

            state = SEND_START;
            unsigned int n;

            memset(data, 0x00, 2048);
            n = co_await sslsock->async_read_some(asio::buffer(data, 1024), asio::use_awaitable);

            if (state == SEND_START)
            {
                state = SEND_EHLO;
                while (state != SEND_END)
                {
                    sendcommand.clear();
                    switch (state)
                    {
                    case SEND_EHLO:
                    case SEND_AUTH_LOGIN:
                    case SEND_USER:
                    case SEND_PASSWORD:
                    case SEND_MAIL_FROM:
                    case SEND_RCPT_TO:
                    case SEND_DATA:
                    case SEND_EMAIL:
                        sendloop();
                        break;
                    default:
                        break;
                    }

                    if (sendcommand.size() > 0)
                    {
                        if (state == SEND_EMAIL)
                        {
                            if (sendfiles.size() == 0)
                            {
                                co_await sslsock->async_write_some(asio::buffer(sendcommand), asio::use_awaitable);
                            }
                            else
                            {
                                co_await asio::async_write(*sslsock, asio::buffer(sendcommand), asio::use_awaitable);
                                unsigned int sendsize = 0;
                                data[2048] = 0;
                                std::string filecontent;
                                std::string sendcontent;

                                for (unsigned int j = 0; j < sendfiles.size(); j++)
                                {
                                    if (sendfiles[j].predata.empty())
                                    {
                                        break;
                                    }
                                    co_await sslsock->async_write_some(asio::buffer(sendfiles[j].predata), asio::use_awaitable);

                                    FILE *ff = fopen(sendfiles[j].filename.c_str(), "rb");
                                    if (ff)
                                    {
                                        fseek(ff, 0, SEEK_END);
                                        unsigned int size = ftell(ff);
                                        fseek(ff, 0, SEEK_SET);
                                        sendsize = 0;
                                        filecontent.clear();
                                        filecontent.resize(size);
                                        unsigned int nread = fread(filecontent.data(), 1, size, ff);
                                        filecontent.resize(nread);
                                        filecontent = base64_encode(filecontent.data(), filecontent.size(), false);

                                        sendcontent.clear();
                                        for (; sendsize < filecontent.size(); sendsize++)
                                        {
                                            if (sendsize % 76 == 0 && sendsize > 0)
                                            {
                                                sendcontent.append("\r\n");
                                            }
                                            sendcontent.push_back(filecontent[sendsize]);
                                        }
                                        co_await asio::async_write(*sslsock, asio::buffer(sendcontent.data(), sendcontent.size()), asio::use_awaitable);
                                    }
                                    fclose(ff);
                                }

                                sendcommand.clear();
                                sendcommand.append("\r\n--");
                                sendcommand.append(boundarystsr);
                                sendcommand.append("--\r\n");
                                sendcommand.append("\r\n.\r\n");
                                co_await asio::async_write(*sslsock, asio::buffer(sendcommand), asio::use_awaitable);
                            }
                        }
                        else
                        {
                            co_await sslsock->async_write_some(asio::buffer(sendcommand), asio::use_awaitable);
                        }

                        memset(data, 0x00, 2048);
                        n = co_await sslsock->async_read_some(asio::buffer(data, 1024), asio::use_awaitable);
                    }

                    if (data[0] == '2' || data[0] == '3' || data[0] == '4' || data[0] == '5')
                    {
                        unsigned int temprenum = 0;
                        if (n > 2)
                        {
                            for (unsigned int i = 0; i < 3; i++)
                            {
                                if (data[i] <= '9' && data[i] >= '0')
                                {
                                    temprenum = temprenum * 10 + (data[i] - '0');
                                }
                            }
                        }

                        switch (state)
                        {
                        case SEND_EHLO:
                            state = SEND_AUTH_LOGIN;
                            break;
                        case SEND_AUTH_LOGIN:
                            if (temprenum == 334 || temprenum == 250)
                            {
                                state = SEND_USER;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_USER:
                            if (temprenum == 334 || temprenum == 250)
                            {
                                state = SEND_PASSWORD;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_PASSWORD:
                            if (temprenum == 334 || temprenum == 235 || temprenum == 250)
                            {
                                state = SEND_MAIL_FROM;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_MAIL_FROM:
                            if (temprenum == 250)
                            {
                                state = SEND_RCPT_TO;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_RCPT_TO:
                            if (temprenum == 250)
                            {
                                state = SEND_DATA;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_DATA:
                            if (temprenum == 354)
                            {
                                state = SEND_EMAIL;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_EMAIL:
                            if (temprenum == 250)
                            {
                                state = SEND_END;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        state = SEND_ERROR;
                        errormsg.clear();
                        for (unsigned int j = 0; j < 100; j++)
                        {
                            if (data[j] != 0x00)
                            {
                                errormsg.push_back(data[j]);
                            }
                        }
                    }

                    if (state == SEND_END || state == SEND_ERROR)
                    {
                        try
                        {
                            sendcommand.clear();
                            sendcommand.append("QUIT");
                            sendcommand.append("\r\n");
                            co_await sslsock->async_write_some(asio::buffer(sendcommand), asio::use_awaitable);
                        }
                        catch (std::exception &e)
                        {
                            errormsg.append(e.what());
                        }
                        break;
                    }
                }
            }

            if (state == SEND_END)
            {
                co_return true;
            }
            else
            {
                co_return false;
            }
        }
        else
        {
            sock = std::make_shared<asio::ip::tcp::socket>(strand_);
            asio::ip::tcp::resolver resolver(strand_);
            auto endpoints = co_await resolver.async_resolve(smpturl, std::to_string(port), asio::use_awaitable);

            constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
            for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
            {
                std::tie(ec) = co_await sock->async_connect(*iter, tuple_awaitable);
                if (ec)
                {
                    continue;
                }
                break;
            }

            if (ec)
            {
                errormsg.append("host connect error!");
                co_return false;
            }

            unsigned int n;
            state = SEND_START;
            memset(data, 0x00, 2048);
            n = co_await sock->async_read_some(asio::buffer(data, 1024), asio::use_awaitable);

            if (state == SEND_START)
            {
                state = SEND_EHLO;
                while (state != SEND_END)
                {
                    sendcommand.clear();
                    switch (state)
                    {
                    case SEND_EHLO:
                    case SEND_AUTH_LOGIN:
                    case SEND_USER:
                    case SEND_PASSWORD:
                    case SEND_MAIL_FROM:
                    case SEND_RCPT_TO:
                    case SEND_DATA:
                    case SEND_EMAIL:
                        sendloop();
                        break;
                    default:
                        break;
                    }

                    if (sendcommand.size() > 0)
                    {
                        if (state == SEND_EMAIL)
                        {
                            if (sendfiles.size() == 0)
                            {
                                co_await sock->async_write_some(asio::buffer(sendcommand), asio::use_awaitable);
                            }
                            else
                            {
                                co_await asio::async_write(*sock, asio::buffer(sendcommand), asio::use_awaitable);
                                unsigned int sendsize = 0;
                                data[2048] = 0;
                                std::string filecontent;
                                std::string sendcontent;

                                for (unsigned int j = 0; j < sendfiles.size(); j++)
                                {
                                    if (sendfiles[j].predata.empty())
                                    {
                                        break;
                                    }
                                    co_await sock->async_write_some(asio::buffer(sendfiles[j].predata), asio::use_awaitable);

                                    FILE *ff = fopen(sendfiles[j].filename.c_str(), "rb");
                                    if (ff)
                                    {
                                        fseek(ff, 0, SEEK_END);
                                        unsigned int size = ftell(ff);
                                        fseek(ff, 0, SEEK_SET);
                                        sendsize = 0;
                                        filecontent.clear();
                                        filecontent.resize(size);
                                        unsigned int nread = fread(filecontent.data(), 1, size, ff);
                                        filecontent.resize(nread);
                                        filecontent = base64_encode(filecontent.data(), filecontent.size(), false);

                                        sendcontent.clear();
                                        for (; sendsize < filecontent.size(); sendsize++)
                                        {
                                            if (sendsize % 76 == 0 && sendsize > 0)
                                            {
                                                sendcontent.append("\r\n");
                                            }
                                            sendcontent.push_back(filecontent[sendsize]);
                                        }
                                        co_await asio::async_write(*sock, asio::buffer(sendcontent.data(), sendcontent.size()), asio::use_awaitable);
                                    }
                                    fclose(ff);
                                }

                                sendcommand.clear();
                                sendcommand.append("\r\n--");
                                sendcommand.append(boundarystsr);
                                sendcommand.append("--\r\n");
                                sendcommand.append("\r\n.\r\n");
                                co_await asio::async_write(*sock, asio::buffer(sendcommand), asio::use_awaitable);
                            }
                        }
                        else
                        {
                            co_await sock->async_write_some(asio::buffer(sendcommand), asio::use_awaitable);
                        }

                        memset(data, 0x00, 2048);
                        n = co_await sock->async_read_some(asio::buffer(data, 1024), asio::use_awaitable);
                    }

                    if (data[0] == '2' || data[0] == '3' || data[0] == '4' || data[0] == '5')
                    {
                        unsigned int temprenum = 0;
                        if (n > 2)
                        {
                            for (unsigned int i = 0; i < 3; i++)
                            {
                                if (data[i] <= '9' && data[i] >= '0')
                                {
                                    temprenum = temprenum * 10 + (data[i] - '0');
                                }
                            }
                        }

                        switch (state)
                        {
                        case SEND_EHLO:
                            state = SEND_AUTH_LOGIN;
                            break;
                        case SEND_AUTH_LOGIN:
                            if (temprenum == 334 || temprenum == 235 || temprenum == 250)
                            {
                                state = SEND_USER;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_USER:
                            if (temprenum == 334 || temprenum == 235 || temprenum == 250)
                            {
                                state = SEND_PASSWORD;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_PASSWORD:
                            if (temprenum == 334 || temprenum == 235 || temprenum == 250)
                            {
                                state = SEND_MAIL_FROM;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_MAIL_FROM:
                            if (temprenum == 250)
                            {
                                state = SEND_RCPT_TO;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_RCPT_TO:
                            if (temprenum == 250)
                            {
                                state = SEND_DATA;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_DATA:
                            if (temprenum == 354)
                            {
                                state = SEND_EMAIL;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        case SEND_EMAIL:
                            if (temprenum == 250)
                            {
                                state = SEND_END;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[n]);
                                }
                                co_return false;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        state = SEND_ERROR;
                        errormsg.clear();
                        for (unsigned int j = 0; j < 100; j++)
                        {
                            if (data[j] != 0x00)
                            {
                                errormsg.push_back(data[j]);
                            }
                        }
                    }

                    if (state == SEND_END || state == SEND_ERROR)
                    {
                        try
                        {
                            sendcommand.clear();
                            sendcommand.append("QUIT");
                            sendcommand.append("\r\n");
                            co_await sock->async_write_some(asio::buffer(sendcommand), asio::use_awaitable);
                        }
                        catch (std::exception &e)
                        {
                            errormsg.append(e.what());
                        }
                        break;
                    }
                }
            }

            if (state == SEND_END)
            {
                co_return true;
            }
            else
            {
                co_return false;
            }
        }
    }
    catch (std::exception &e)
    {
        errormsg.append(e.what());
        co_return false;
    }
}

} // namespace http