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
// 同步发送的 SMTP 读超时：每次读最多等 30 秒。截止时刻记在 send_email 自己身上，
// 由 client_context 的常驻循环到点 shutdown 底层连接，把阻塞中的 poll/recv 唤醒；
// 到点判定是整秒 + 4 秒一拍，且要两拍（先标记再关闭），所以实际打断比 30 秒晚，
// 实测界值 36 秒。
constexpr unsigned int kSmtpReadTimeout = 30;

// 整个会话只登记一次；每次读之前用续期把截止时刻往后推。
// 对象必须是 shared_ptr 持有的，栈上的对象拿不到 shared_from_this()，
// 那种写法本次发信不带读超时，只打一条日志，不抛出去。
void register_smtp_timeout(send_email &mail)
{
    mail.iswait_exit = false;
    mail.timed_out_  = false;
    mail.set_timeout(kSmtpReadTimeout);
    try
    {
        client_context &temp_io_context = get_client_context_obj();
        temp_io_context.add_timeout_list(temp_io_context.smtp_timeout_lists, mail.shared_from_this());
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("smtp timeout register failed: %s", e.what());
    }
}

// 收摊时反过来：清掉截止时刻，再把本对象的节点从链表里摘掉。
// 只 disarm 不摘节点的话，这个空节点要等对象析构才走（timeout==0 判不出过期）。
// 栈对象那条路压根没登记过，shared_from_this() 抛异常正是"没有节点要摘"。
void unregister_smtp_timeout(send_email &mail)
{
    mail.disarm();
    try
    {
        client_context &temp_io_context = get_client_context_obj();
        temp_io_context.remove_timeout_list(temp_io_context.smtp_timeout_lists, mail.shared_from_this());
    }
    catch (const std::exception &e)
    {
        DEBUG_LOG("smtp timeout unregister failed: %s", e.what());
    }
}

// 会话结束时统一收摊：摘掉读超时、释放本对象持有的连接。
// 声明在 ssl_context 之后，于是先于它销毁——连接先断，上下文后释放。
class smtp_session_guard
{
  public:
    explicit smtp_session_guard(send_email &mail) : mail_(mail)
    {
    }
    ~smtp_session_guard()
    {
        unregister_smtp_timeout(mail_);
        std::lock_guard<std::mutex> lk(mail_.sock_mtx);
        mail_.sslsock.reset();
        mail_.sock.reset();
    }

  private:
    send_email &mail_;
};

void append_smtp_read_error(std::string &errormsg, const asio::error_code &ec, bool timed_out)
{
    if (timed_out)
    {
        errormsg.append("smtp read timeout!");
    }
    else
    {
        errormsg.append("smtp read error: ");
        errormsg.append(ec.message());
    }
}
}// namespace

/**
 * @brief 读超时到点时由 client_context 的循环调用
 *
 * 只 shutdown 不 close：连接还是本对象持有，等会话结束由 smtp_session_guard 释放。
 * close 会在业务线程还阻塞在 read 里的时候销毁 stream，而且会把 fd 号归还给系统，
 * 可能立刻被别的连接复用。
 * 这条在超时线程上跑，会话释放那条在业务线程上跑，所以句柄先在锁里拷成本地副本，
 * 之后的 shutdown 用副本，锁外进行。
 */
void send_email::close_connect()
{
    timed_out_.store(true);
    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> tls;
    std::shared_ptr<asio::ip::tcp::socket> plain;
    {
        std::lock_guard<std::mutex> lk(sock_mtx);
        tls   = sslsock;
        plain = sock;
    }
    asio::error_code ignored;
    if (tls)
    {
        tls->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    }
    else if (plain)
    {
        plain->shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    }
}

/**
 * @brief 添加邮件附件
 *
 * 检查附件文件是否存在，存在则添加到附件列表中
 * @param file_name 附件文件路径
 * @return true表示添加成功，false表示文件不存在
 */
bool send_email::addattachments(std::string file_name)
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
 * @brief 邮件发送入口函数（同步）
 *
 * 根据isssl标志选择SSL加密连接或明文TCP连接发送邮件。
 * 该方法为同步（阻塞）方式，调用后会阻塞当前线程直到整个 SMTP 会话结束，
 * 适用于对耗时不敏感、并发量较低的场景。
 *
 * 使用示例：
 * @code
 * auto mail = std::make_shared<http::send_email>("smtp.163.com", "user@163.com", "authcode", 25, false);
 * mail->fromname   = "网站名称";                 // 发件人显示名称
 * mail->replyemail = "user@163.com";             // 回复邮箱
 * mail->toemail    = "someone@example.com";      // 收件人邮箱
 * mail->toname     = "收件人";                    // 收件人显示名称
 * mail->subject    = "邮件主题";                 // 主题
 * mail->content    = "<h1>邮件正文</h1>";        // 正文（默认 HTML 格式）
 * mail->addattachments("/path/to/file.docx");    // 可选：添加附件
 * bool ok = mail->send();                        // 同步发送
 * if (!ok) { std::cout << mail->errormsg; }      // 失败原因
 * @endcode
 *
 * @note 必须用 std::make_shared 持有：读超时登记在 client_context 的常驻循环上，
 *       栈对象登记不进去，那次发信就没有读超时。
 *
 * @note 若需要不阻塞当前线程的高并发发送，请改用协程版 async_send_email。
 *
 * @return true表示发送成功，false表示失败，失败原因见 errormsg
 */
bool send_email::send()
{
    if (isssl)
    {
        return sendssldata();
    }
    else
    {
        return senddata();
    }
}

/**
 * @brief 建立SSL加密连接发送邮件
 *
 * 使用OpenSSL和ASIO建立SSL/TLS连接，通过状态机模式执行SMTP协议流程：
 * 连接 -> 握手 -> EHLO -> AUTH LOGIN -> 发送用户名/密码 -> MAIL FROM -> RCPT TO -> DATA -> 发送邮件内容 -> QUIT
 * @return true表示发送成功，false表示失败
 */
bool send_email::sendssldata()
{
    try
    {
        client_context &temp_io_context = get_client_context_obj();
        asio::ssl::context ssl_context(asio::ssl::context::sslv23);
        sslsock = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(temp_io_context.get_ctx(), ssl_context);
        asio::ssl::stream<asio::ip::tcp::socket> &socket = *sslsock;
        smtp_session_guard scope_guard(*this);

        ssl_context.set_default_verify_paths();
        asio::ip::tcp::resolver resolver(temp_io_context.get_ctx());
        auto endpoints = resolver.resolve(smpturl.c_str(), std::to_string(port));

        SSL_set_tlsext_host_name(socket.native_handle(), smpturl.c_str());
        asio::connect(socket.lowest_layer(), endpoints, ec);

        if (ec)
        {
            errormsg.append("ssl connect error!");
            return false;
        }

        register_smtp_timeout(*this);

        ssl_context.set_verify_mode(asio::ssl::verify_peer);
        ssl_context.set_verify_callback(asio::ssl::host_name_verification(smpturl));

        try
        {
            set_timeout(kSmtpReadTimeout);
            socket.handshake(asio::ssl::stream_base::client, ec);
        }
        catch (const std::exception &e)
        {
            errormsg.append(e.what());
        }

        if (ec)
        {
            if (is_read_timeout())
            {
                errormsg.append("smtp read timeout!");
            }
            else
            {
                errormsg.append("ssl handshake error!");
            }
            return false;
        }

        state = SEND_START;
        unsigned int n;

        n = read_smtp_response([&](char *buf, std::size_t cap)
        {
            set_timeout(kSmtpReadTimeout);
            std::size_t got = socket.read_some(asio::buffer(buf, cap), ec);
            if (ec)
            {
                append_smtp_read_error(errormsg, ec, is_read_timeout());
                return -1;
            }
            return static_cast<int>(got);
        });
        if (n == 0)
        {
            return false;
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
                            asio::write(socket, asio::buffer(sendcommand));
                        }
                        else
                        {
                            asio::write(socket, asio::buffer(sendcommand));
                            std::size_t sendsize = 0;
                            std::string filecontent;
                            std::string sendcontent;

                            for (size_t j = 0; j < sendfiles.size(); j++)
                            {
                                FILE *ff = fopen(sendfiles[j].filename.c_str(), "rb");
                                if (!ff)
                                {
                                    continue;
                                }
                                asio::write(socket, asio::buffer(sendfiles[j].predata));

                                if (fseek(ff, 0, SEEK_END) != 0)
                                {
                                    fclose(ff);
                                    errormsg.append("attachment seek failed: ");
                                    errormsg.append(sendfiles[j].filename);
                                    return false;
                                }
                                long long size = ftell(ff);
                                if (size < 0)
                                {
                                    fclose(ff);
                                    errormsg.append("attachment size query failed: ");
                                    errormsg.append(sendfiles[j].filename);
                                    return false;
                                }
                                if (size > kMaxAttachmentBytes)
                                {
                                    fclose(ff);
                                    errormsg.append("attachment too large (4GiB limit): ");
                                    errormsg.append(sendfiles[j].filename);
                                    return false;
                                }
                                if (fseek(ff, 0, SEEK_SET) != 0)
                                {
                                    fclose(ff);
                                    errormsg.append("attachment seek failed: ");
                                    errormsg.append(sendfiles[j].filename);
                                    return false;
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
                                    return false;
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
                                asio::write(socket, asio::buffer(sendcontent.data(), sendcontent.size()));
                                fclose(ff);
                            }

                            sendcommand.clear();
                            sendcommand.append("\r\n--");
                            sendcommand.append(boundarystsr);
                            sendcommand.append("--\r\n");
                            sendcommand.append("\r\n.\r\n");
                            asio::write(socket, asio::buffer(sendcommand));
                        }
                    }
                    else
                    {
                        socket.write_some(asio::buffer(sendcommand));
                    }

                    n = read_smtp_response([&](char *buf, std::size_t cap)
                    {
                        set_timeout(kSmtpReadTimeout);
                        std::size_t got = socket.read_some(asio::buffer(buf, cap), ec);
                        if (ec)
                        {
                            append_smtp_read_error(errormsg, ec, is_read_timeout());
                            return -1;
                        }
                        return static_cast<int>(got);
                    });
                    if (n == 0)
                    {
                        return false;
                    }
                }

                if (data[0] == '2' || data[0] == '3' || data[0] == '4' || data[0] == '5')
                {
                    unsigned int temprenum = 0;
                    if (n > 2)
                    {
                        for (size_t i = 0; i < 3; i++)
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
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_AUTH_LOGIN:
                        if (temprenum == 334 || temprenum == 250)
                        {
                            state = SEND_USER;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_USER:
                        if (temprenum == 334 || temprenum == 250)
                        {
                            state = SEND_PASSWORD;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_PASSWORD:
                        if (temprenum == 334 || temprenum == 235 || temprenum == 250)
                        {
                            state = SEND_MAIL_FROM;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_MAIL_FROM:
                        if (temprenum == 250)
                        {
                            state = SEND_RCPT_TO;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_RCPT_TO:
                        if (temprenum == 250)
                        {
                            state = SEND_DATA;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_DATA:
                        if (temprenum == 354)
                        {
                            state = SEND_EMAIL;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_EMAIL:
                        if (temprenum == 250)
                        {
                            state = SEND_END;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
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
                    for (size_t j = 0; j < n; j++)
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
                        socket.write_some(asio::buffer(sendcommand));
                        break;
                    }
                    catch (std::exception &e)
                    {
                        errormsg.append(e.what());
                        break;
                    }
                }
            }
        }

        if (state == SEND_END)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (std::exception &e)
    {
        errormsg.append(e.what());
        return false;
    }
}

/**
 * @brief 建立明文TCP连接发送邮件
 *
 * 使用ASIO建立明文TCP连接，通过状态机模式执行SMTP协议流程：
 * 连接 -> EHLO -> AUTH LOGIN -> 发送用户名/密码 -> MAIL FROM -> RCPT TO -> DATA -> 发送邮件内容 -> QUIT
 * @return true表示发送成功，false表示失败
 */
bool send_email::senddata()
{
    try
    {
        client_context &temp_io_context = get_client_context_obj();
        sock = std::make_shared<asio::ip::tcp::socket>(temp_io_context.get_ctx());
        asio::ip::tcp::socket &socket = *sock;
        smtp_session_guard scope_guard(*this);
        asio::ip::tcp::resolver resolver(temp_io_context.get_ctx());
        auto endpoints = resolver.resolve(smpturl, std::to_string(port));
        asio::connect(socket, endpoints, ec);

        if (ec)
        {
            errormsg.append("host connect error!");
            return false;
        }

        register_smtp_timeout(*this);

        unsigned int n;
        state = SEND_START;
        n = read_smtp_response([&](char *buf, std::size_t cap)
        {
            set_timeout(kSmtpReadTimeout);
            std::size_t got = socket.read_some(asio::buffer(buf, cap), ec);
            if (ec)
            {
                append_smtp_read_error(errormsg, ec, is_read_timeout());
                return -1;
            }
            return static_cast<int>(got);
        });
        if (n == 0)
        {
            return false;
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
                            asio::write(socket, asio::buffer(sendcommand));
                        }
                        else
                        {
                            asio::write(socket, asio::buffer(sendcommand));
                            std::size_t sendsize = 0;
                            std::string filecontent;
                            std::string sendcontent;

                            for (size_t j = 0; j < sendfiles.size(); j++)
                            {
                                FILE *ff = fopen(sendfiles[j].filename.c_str(), "rb");
                                if (!ff)
                                {
                                    continue;
                                }
                                asio::write(socket, asio::buffer(sendfiles[j].predata));

                                if (fseek(ff, 0, SEEK_END) != 0)
                                {
                                    fclose(ff);
                                    errormsg.append("attachment seek failed: ");
                                    errormsg.append(sendfiles[j].filename);
                                    return false;
                                }
                                long long size = ftell(ff);
                                if (size < 0)
                                {
                                    fclose(ff);
                                    errormsg.append("attachment size query failed: ");
                                    errormsg.append(sendfiles[j].filename);
                                    return false;
                                }
                                if (size > kMaxAttachmentBytes)
                                {
                                    fclose(ff);
                                    errormsg.append("attachment too large (4GiB limit): ");
                                    errormsg.append(sendfiles[j].filename);
                                    return false;
                                }
                                if (fseek(ff, 0, SEEK_SET) != 0)
                                {
                                    fclose(ff);
                                    errormsg.append("attachment seek failed: ");
                                    errormsg.append(sendfiles[j].filename);
                                    return false;
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
                                    return false;
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
                                asio::write(socket, asio::buffer(sendcontent.data(), sendcontent.size()));
                                fclose(ff);
                            }

                            sendcommand.clear();
                            sendcommand.append("\r\n--");
                            sendcommand.append(boundarystsr);
                            sendcommand.append("--\r\n");
                            sendcommand.append("\r\n.\r\n");
                            asio::write(socket, asio::buffer(sendcommand));
                        }
                    }
                    else
                    {
                        socket.write_some(asio::buffer(sendcommand));
                    }

                    n = read_smtp_response([&](char *buf, std::size_t cap)
                    {
                        set_timeout(kSmtpReadTimeout);
                        std::size_t got = socket.read_some(asio::buffer(buf, cap), ec);
                        if (ec)
                        {
                            append_smtp_read_error(errormsg, ec, is_read_timeout());
                            return -1;
                        }
                        return static_cast<int>(got);
                    });
                    if (n == 0)
                    {
                        return false;
                    }
                }

                if (data[0] == '2' || data[0] == '3' || data[0] == '4' || data[0] == '5')
                {
                    unsigned int temprenum = 0;
                    if (n > 2)
                    {
                        for (size_t i = 0; i < 3; i++)
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
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_AUTH_LOGIN:
                        if (temprenum == 334 || temprenum == 235 || temprenum == 250)
                        {
                            state = SEND_USER;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_USER:
                        if (temprenum == 334 || temprenum == 235 || temprenum == 250)
                        {
                            state = SEND_PASSWORD;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_PASSWORD:
                        if (temprenum == 334 || temprenum == 235 || temprenum == 250)
                        {
                            state = SEND_MAIL_FROM;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_MAIL_FROM:
                        if (temprenum == 250)
                        {
                            state = SEND_RCPT_TO;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_RCPT_TO:
                        if (temprenum == 250)
                        {
                            state = SEND_DATA;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_DATA:
                        if (temprenum == 354)
                        {
                            state = SEND_EMAIL;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
                        }
                        break;
                    case SEND_EMAIL:
                        if (temprenum == 250)
                        {
                            state = SEND_END;
                        }
                        else
                        {
                            for (size_t i = 0; i < n; i++)
                            {
                                errormsg.push_back(data[i]);
                            }
                            return false;
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
                    for (size_t j = 0; j < n; j++)
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
                        socket.write_some(asio::buffer(sendcommand));
                        break;
                    }
                    catch (std::exception &e)
                    {
                        errormsg.append(e.what());
                        break;
                    }
                }
            }
        }

        if (state == SEND_END)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (std::exception &e)
    {
        errormsg.append(e.what());
        return false;
    }
}

/**
 * @brief 获取当前GMT时间字符串
 *
 * 生成符合RFC 2822标准的日期格式，用于邮件的Date头部字段
 * @return GMT时间字符串，格式如 "Thu, 01 Jan 1970 00:00:00 GMT"
 */
std::string send_email::getgmttime()
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
bool send_email::sendloop()
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

}// namespace http