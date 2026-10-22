#include <cstdarg>
#include <cstring>
#include <stdexcept>
#include <string>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include "base64.h"
#include "http_socket.h"
#include "send_email.h"
#include "async_send_email.h"
#include "session_id.h"
#include "func.h"
#include "terminal_color.h"
#include "client_context.h"

namespace http
{

// 附件上限：base64_encode 的长度参数是 unsigned int，4GiB 及以上装不下，
// 超限直接拒绝发送，避免长度截断后发出损坏内容。
constexpr long long kMaxAttachmentBytes = 4LL * 1024 * 1024 * 1024 - 1;

namespace
{
// 协程发送的 SMTP 读超时：每次等服务器应答最多 30 秒。截止时刻记在
// async_send_email 自己身上，由 client_context 的常驻循环到点 shutdown 底层连接，
// 挂起的读会带着错误完成，让整次发送失败而不是永久等待。
// 到点判定是整秒 + 4 秒一拍，且要两拍（先标记再关闭），所以实际打断比 30 秒晚，
// 实测界值 36 秒。
constexpr unsigned int kSmtpReadTimeout = 30;

// 整个会话只登记一次；每次读之前续期。对象必须是 shared_ptr 持有的，
// 栈对象拿不到 shared_from_this()，那次发信就没有读超时（只记日志，不抛）。
void register_async_smtp_timeout(async_send_email &mail)
{
    mail.iswait_exit = false;
    mail.timed_out_  = false;
    mail.set_timeout(kSmtpReadTimeout);
    try
    {
        client_context &temp_io_context = get_client_context_obj();
        temp_io_context.add_timeout_list(temp_io_context.async_smtp_timeout_lists, mail.shared_from_this());
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("smtp timeout register failed: %s", e.what());
    }
}

// 收摊时反过来：清掉截止时刻，再把本对象的节点从链表里摘掉。
// 只 disarm 不摘节点的话，这个空节点要等对象析构才走（timeout==0 判不出过期）。
void unregister_async_smtp_timeout(async_send_email &mail)
{
    mail.disarm();
    try
    {
        client_context &temp_io_context = get_client_context_obj();
        temp_io_context.remove_timeout_list(temp_io_context.async_smtp_timeout_lists, mail.shared_from_this());
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("smtp timeout unregister failed: %s", e.what());
    }
}

// 会话结束时摘掉读超时并释放本对象持有的连接（协程帧销毁时执行）。
// 这个守卫在 strand_ 上随协程帧销毁，到点关闭那条也排到了 strand_ 上，
// 两边由 strand 串行，所以这里不需要同步版那把句柄锁。
class async_smtp_session_guard
{
  public:
    explicit async_smtp_session_guard(async_send_email &mail) : mail_(mail)
    {
    }
    ~async_smtp_session_guard()
    {
        unregister_async_smtp_timeout(mail_);
        mail_.sslsock.reset();
        mail_.sock.reset();
    }

  private:
    async_send_email &mail_;
};

// 反复读 socket 直到凑齐一条完整 SMTP 应答。应答写进 out（最多 out_size-1 字节），
// 返回长度；0 表示失败（应答超长）。多读出来的字节留在 readbuf 里给下一次调用。
template <class Stream>
asio::awaitable<unsigned int> read_smtp_response_async(Stream &stream, std::string &readbuf,
                                                       char *out, std::size_t out_size,
                                                       async_send_email &mail,
                                                       std::string &errormsg)
{
    for (;;)
    {
        std::string response;
        int st = take_smtp_response(readbuf, response);
        if (st == 1)
        {
            std::size_t cap = out_size > 0 ? out_size - 1 : 0;
            unsigned int len = response.size() > cap ? static_cast<unsigned int>(cap) : static_cast<unsigned int>(response.size());
            std::memset(out, 0x00, out_size);
            std::memcpy(out, response.data(), len);
            co_return len;
        }
        if (st < 0)
        {
            errormsg.append("smtp response too long!");
            co_return 0;
        }
        char chunk[1024];
        mail.set_timeout(kSmtpReadTimeout);
        std::size_t got = co_await stream.async_read_some(asio::buffer(chunk, sizeof(chunk)), asio::use_awaitable);
        readbuf.append(chunk, got);
    }
}
}// namespace

/**
 * @brief 读超时到点时由 client_context 的循环调用
 *
 * 只 shutdown 不 close：连接还是本对象持有，协程帧销毁时由守卫释放；
 * 在这里 close 会把还在用的对象拆掉。
 * 在途的异步读挂在自己的 strand_ 上，从别的线程直接碰 socket 是 asio 禁止的，
 * 所以非 strand 线程只把关闭排进 strand_ 队列（排在已入队的读写之后），
 * 真正动手的还是这条连接自己的线程。
 */
void async_send_email::close_connect()
{
    timed_out_.store(true);
    if (!strand_.running_in_this_thread())
    {
        // weak_from_this：析构期 shared_from_this() 会抛。锁不到说明已经没有共享
        // 所有权能续命，而扫描线程手里那份是刚摘的快照，就地收才是安全的。
        auto self = weak_from_this().lock();
        if (self)
        {
            asio::post(strand_, [self]()
                       { self->close_connect(); });
            return;
        }
    }
    asio::error_code ignored;
    if (sslsock)
    {
        sslsock->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    }
    else if (sock)
    {
        sock->shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    }
}

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
    tm timeInfo{};
    char timestr[30] = {'\0'};
#if defined(_WIN32)
    gmtime_s(&timeInfo, &curr_time);
#else
    gmtime_r(&curr_time, &timeInfo);
#endif
    strftime(timestr, sizeof(timestr), "%a, %d %b %Y %H:%M:%S GMT", &timeInfo);
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
        if (ehlo_failed_)
        {
            sendcommand.append("HELO ");
        }
        else
        {
            sendcommand.append("EHLO ");
        }
        sendcommand.append(smtp_header_value(smpturl));
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
        sendcommand.append(smtp_header_value(username));
        sendcommand.append(">\r\n");
        break;

    case SEND_RCPT_TO:
        sendcommand.append("RCPT TO:<");
        sendcommand.append(smtp_header_value(toemail));
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
            sendcommand.append(smtp_header_value(username));
            sendcommand.append(">\r\n");

            sendcommand.append("To: =?UTF-8?B?");
            sendcommand.append(base64_encode(toname.data(), toname.size(), 0));
            sendcommand.append("?= <");
            sendcommand.append(smtp_header_value(toemail));
            sendcommand.append(">\r\n");

            sendcommand.append("Date: ");
            sendcommand.append(getgmttime());
            sendcommand.append("\r\n");

            sendcommand.append("Subject: ");
            sendcommand.append(smtp_header_value(subject));
            sendcommand.append("\r\n");

            sendcommand.append("Reply-To: <");
            sendcommand.append(smtp_header_value(replyemail));
            sendcommand.append(">\r\n");

            boundarystsr = get_rand_session_id();

            sendcommand.append("Message-ID: <");
            sendcommand.append(boundarystsr);
            sendcommand.append("@");
            sendcommand.append(smtp_header_value(smpturl));
            sendcommand.append(">\r\n");

            boundarystsr.append("_paozhu_");

            sendcommand.append("Content-Type: multipart/mixed; boundary=\"");
            sendcommand.append(boundarystsr);
            sendcommand.append("\"\r\n\r\n--");
            sendcommand.append(boundarystsr);
            sendcommand.append("\r\n");
            sendcommand.append("Content-Type: text/html; charset=UTF-8\r\n\r\n");
            sendcommand.append(smtp_dot_stuff(content));
            sendcommand.append("\r\n");

            n = 0;
            sendfiles.clear();
            for (; n < attachments.size(); n++)
            {
                FILE *ffile = fopen(attachments[n].c_str(), "rb");
                if (!ffile)
                {
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

                sendfile_t sf;
                sf.predata.append("\r\n--");
                sf.predata.append(boundarystsr);
                sf.predata.append("\r\nContent-Type: application/octet-stream;\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; filename=\"=?UTF-8?B?");
                sf.predata.append(base64_encode(filename_temp.data(), filename_temp.size()));
                sf.predata.append("?=\"\r\n\r\n");
                sf.filename = attachments[n];
                sendfiles.push_back(std::move(sf));
            }
        }
        else
        {
            sendcommand.clear();
            sendcommand.append("MIME-Version: 1.0\r\n");
            sendcommand.append("Content-Type: text/html; charset=utf-8\r\n");
            sendcommand.append("From: =?UTF-8?B?");
            sendcommand.append(base64_encode(fromname.data(), fromname.size(), 0));
            sendcommand.append("?= <");
            sendcommand.append(smtp_header_value(username));
            sendcommand.append(">\r\n");

            sendcommand.append("To: =?UTF-8?B?");
            sendcommand.append(base64_encode(toname.data(), toname.size(), 0));
            sendcommand.append("?= <");
            sendcommand.append(smtp_header_value(toemail));
            sendcommand.append(">\r\n");

            sendcommand.append("Date: ");
            sendcommand.append(getgmttime());
            sendcommand.append("\r\n");

            sendcommand.append("Subject: ");
            sendcommand.append(smtp_header_value(subject));
            sendcommand.append("\r\n");

            sendcommand.append("Reply-To: <");
            sendcommand.append(smtp_header_value(replyemail));
            sendcommand.append(">\r\n");

            boundarystsr = get_rand_session_id();

            sendcommand.append("Message-ID: <");
            sendcommand.append(boundarystsr);
            sendcommand.append("@");
            sendcommand.append(smtp_header_value(smpturl));
            sendcommand.append(">\r\n\r\n");

            sendcommand.append(smtp_dot_stuff(content));
            sendcommand.append("\r\n\r\n.\r\n");
        }
        break;

    default:
        break;
    }
    return true;
}

/**
 * @brief 协程版发送邮件（异步，不阻塞当前线程）
 * 
 * 使用OpenSSL和ASIO协程建立SSL/TLS连接，通过状态机模式执行SMTP协议流程：
 * 连接 -> 握手 -> EHLO -> AUTH LOGIN -> 发送用户名/密码 -> MAIL FROM -> RCPT TO -> DATA -> 发送邮件内容 -> QUIT
 *
 * 使用示例（在协程环境中调用）：
 * @code
 * auto mail = std::make_shared<http::async_send_email>(
 *     "smtp.163.com", "user@163.com", "authcode", 25, true);   // 第5个参数 isssl=true
 * mail->fromname   = "网站名称";
 * mail->replyemail = "user@163.com";
 * mail->toemail    = "someone@example.com";
 * mail->toname     = "收件人";
 * mail->subject    = "邮件主题";
 * mail->content    = "<h1>邮件正文</h1>";
 * mail->addattachments("/path/to/file.docx");  // 可选：添加附件
 * bool ok = co_await mail->async_send();       // 协程挂起，不阻塞当前线程
 * if (!ok) { std::cout << mail->errormsg; }    // 失败原因
 * @endcode
 *
 * @note 必须用 std::make_shared 持有：读超时登记在 client_context 的常驻循环上，
 *       栈对象登记不进去，那次发信就没有读超时。
 *
 * @note 该方法必须返回 awaitable<bool>，需要在 asio 协程（co_await 上下文）中使用；
 *       相比同步版 send_email::send()，它不会阻塞当前执行线程，适合高并发场景。
 *
 * @return asio::awaitable<bool> true表示发送成功，false表示失败，失败原因见 errormsg
 */
asio::awaitable<bool> async_send_email::async_send()
{
    async_smtp_session_guard scope_guard(*this);
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
                    ++iter;
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

            register_async_smtp_timeout(*this);
            std::tie(ec) = co_await sslsock->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
            if (ec)
            {
                errormsg.append(is_read_timeout() ? "smtp read timeout!" : "ssl handshake error!");
                co_return false;
            }

            state = SEND_START;
            unsigned int n;

            n = co_await read_smtp_response_async(*sslsock, smtp_readbuf_, data, sizeof(data), *this, errormsg);
            if (n == 0)
            {
                co_return false;
            }

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
                                co_await asio::async_write(*sslsock, asio::buffer(sendcommand), asio::use_awaitable);
                            }
                            else
                            {
                                co_await asio::async_write(*sslsock, asio::buffer(sendcommand), asio::use_awaitable);
                                std::size_t sendsize = 0;
                                std::string filecontent;
                                std::string sendcontent;

                                for (unsigned int j = 0; j < sendfiles.size(); j++)
                                {
                                    FILE *ff = fopen(sendfiles[j].filename.c_str(), "rb");
                                    if (!ff)
                                    {
                                        continue;
                                    }
                                    co_await asio::async_write(*sslsock, asio::buffer(sendfiles[j].predata), asio::use_awaitable);

                                    if (fseek(ff, 0, SEEK_END) != 0)
                                    {
                                        fclose(ff);
                                        errormsg.append("attachment seek failed: ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    long long size = ftell(ff);
                                    if (size < 0)
                                    {
                                        fclose(ff);
                                        errormsg.append("attachment size query failed: ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    if (size > kMaxAttachmentBytes)
                                    {
                                        fclose(ff);
                                        errormsg.append("attachment too large (4GiB limit): ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    if (fseek(ff, 0, SEEK_SET) != 0)
                                    {
                                        fclose(ff);
                                        errormsg.append("attachment seek failed: ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    sendsize = 0;
                                    filecontent.clear();
                                    filecontent.resize(static_cast<std::size_t>(size));
                                    std::size_t nread = fread(filecontent.data(), 1, static_cast<std::size_t>(size), ff);
                                    if (nread != static_cast<std::size_t>(size))
                                    {
                                        bool read_error = ferror(ff) != 0;
                                        fclose(ff);
                                        errormsg.append(read_error ? "attachment read error: " : "attachment truncated: ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    filecontent = base64_encode(filecontent.data(), static_cast<unsigned int>(filecontent.size()), false);

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

                        n = co_await read_smtp_response_async(*sslsock, smtp_readbuf_, data, sizeof(data), *this, errormsg);
                        if (n == 0)
                        {
                            co_return false;
                        }
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
                            if (temprenum == 250)
                            {
                                state = SEND_AUTH_LOGIN;
                            }
                            else if (!ehlo_failed_)
                            {
                                ehlo_failed_ = true;
                                state        = SEND_EHLO;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[i]);
                                }
                                co_return false;
                            }
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                        for (unsigned int j = 0; j < n; j++)
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
                    ++iter;
                    continue;
                }
                break;
            }

            if (ec)
            {
                errormsg.append("host connect error!");
                co_return false;
            }

            register_async_smtp_timeout(*this);

            unsigned int n;
            state = SEND_START;
            n = co_await read_smtp_response_async(*sock, smtp_readbuf_, data, sizeof(data), *this, errormsg);
            if (n == 0)
            {
                co_return false;
            }

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
                                co_await asio::async_write(*sock, asio::buffer(sendcommand), asio::use_awaitable);
                            }
                            else
                            {
                                co_await asio::async_write(*sock, asio::buffer(sendcommand), asio::use_awaitable);
                                std::size_t sendsize = 0;
                                std::string filecontent;
                                std::string sendcontent;

                                for (unsigned int j = 0; j < sendfiles.size(); j++)
                                {
                                    FILE *ff = fopen(sendfiles[j].filename.c_str(), "rb");
                                    if (!ff)
                                    {
                                        continue;
                                    }
                                    co_await asio::async_write(*sock, asio::buffer(sendfiles[j].predata), asio::use_awaitable);

                                    if (fseek(ff, 0, SEEK_END) != 0)
                                    {
                                        fclose(ff);
                                        errormsg.append("attachment seek failed: ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    long long size = ftell(ff);
                                    if (size < 0)
                                    {
                                        fclose(ff);
                                        errormsg.append("attachment size query failed: ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    if (size > kMaxAttachmentBytes)
                                    {
                                        fclose(ff);
                                        errormsg.append("attachment too large (4GiB limit): ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    if (fseek(ff, 0, SEEK_SET) != 0)
                                    {
                                        fclose(ff);
                                        errormsg.append("attachment seek failed: ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    sendsize = 0;
                                    filecontent.clear();
                                    filecontent.resize(static_cast<std::size_t>(size));
                                    std::size_t nread = fread(filecontent.data(), 1, static_cast<std::size_t>(size), ff);
                                    if (nread != static_cast<std::size_t>(size))
                                    {
                                        bool read_error = ferror(ff) != 0;
                                        fclose(ff);
                                        errormsg.append(read_error ? "attachment read error: " : "attachment truncated: ");
                                        errormsg.append(sendfiles[j].filename);
                                        co_return false;
                                    }
                                    filecontent = base64_encode(filecontent.data(), static_cast<unsigned int>(filecontent.size()), false);

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

                        n = co_await read_smtp_response_async(*sock, smtp_readbuf_, data, sizeof(data), *this, errormsg);
                        if (n == 0)
                        {
                            co_return false;
                        }
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
                            if (temprenum == 250)
                            {
                                state = SEND_AUTH_LOGIN;
                            }
                            else if (!ehlo_failed_)
                            {
                                ehlo_failed_ = true;
                                state        = SEND_EHLO;
                            }
                            else
                            {
                                for (unsigned int i = 0; i < n; i++)
                                {
                                    errormsg.push_back(data[i]);
                                }
                                co_return false;
                            }
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                                    errormsg.push_back(data[i]);
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
                        for (unsigned int j = 0; j < n; j++)
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
        if (is_read_timeout())
        {
            errormsg.append("smtp read timeout!");
        }
        else
        {
            errormsg.append(e.what());
        }
        co_return false;
    }
}

} // namespace http