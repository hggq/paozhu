#pragma once

#include <cstddef>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <list>
#include "http_socket.h"

namespace http
{
    // SMTP 响应按 CRLF 分行：末行第 4 字节是空格（或整行只有 3 位码）才算应答结束，
    // 前面带 '-' 的行是续行。一次 recv 可能只给半行，也可能给好几条应答，
    // 所以从持久缓冲 buf 里取：取到一条就搬进 out，剩余字节留给下一次调用。
    // 返回 1 取到、0 还不够、-1 超过缓冲上限。
    inline int take_smtp_response(std::string &buf, std::string &out)
    {
        constexpr std::size_t kMaxSmtpResponse = 64 * 1024;
        std::size_t pos = 0;
        for (;;)
        {
            std::size_t eol = buf.find("\r\n", pos);
            if (eol == std::string::npos)
            {
                return buf.size() > kMaxSmtpResponse ? -1 : 0;
            }
            bool last = (eol - pos == 3) || (eol - pos >= 4 && buf[pos + 3] == ' ');
            if (last)
            {
                out.assign(buf, 0, eol);
                buf.erase(0, eol + 2);
                return 1;
            }
            pos = eol + 2;
        }
    }

    // 数据阶段以 '.' 开头的行要双写，否则会被服务器当成正文结束标记
    inline std::string smtp_dot_stuff(const std::string &body)
    {
        std::string out;
        out.reserve(body.size() + 16);
        bool line_start = true;
        for (char c : body)
        {
            if (line_start && c == '.')
            {
                out.push_back('.');
            }
            out.push_back(c);
            line_start = (c == '\n');
        }
        return out;
    }

    // 头字段值里不能出现 CR/LF，否则可以拼出额外的头甚至正文
    inline std::string smtp_header_value(std::string_view value)
    {
        std::string out;
        out.reserve(value.size());
        for (char c : value)
        {
            if (c != '\r' && c != '\n')
            {
                out.push_back(c);
            }
        }
        return out;
    }

    /**
     * @brief SMTP邮件发送类，支持SSL/TLS加密连接和附件发送
     * 
     * 实现SMTP协议的客户端，支持AUTH LOGIN认证方式，可发送纯文本或HTML邮件，
     * 支持添加附件。使用状态机模式管理SMTP会话流程。
     */
    class send_email
    {
    public:
        /**
         * @brief 默认构造函数
         */
        send_email()
        {
        }

        /**
         * @brief 带完整参数的构造函数
         * @param smpt_url SMTP服务器地址
         * @param account 邮箱账号
         * @param passwd 邮箱密码
         * @param p 端口号
         * @param isssl 是否使用SSL加密
         */
        send_email(std::string smpt_url, std::string account, std::string passwd, unsigned int p, bool isssl) : smpturl(smpt_url), username(account), password(passwd), port(p), isssl(isssl)
        {
        }

        /**
         * @brief 带基本参数的构造函数，默认端口25，不使用SSL
         * @param smpt_url SMTP服务器地址
         * @param account 邮箱账号
         * @param passwd 邮箱密码
         */
        send_email(std::string smpt_url, std::string account, std::string passwd) : smpturl(smpt_url), username(account), password(passwd)
        {
        }

        /**
         * @brief 执行邮件发送（根据isssl选择加密或明文连接）
         * @return true表示发送成功，false表示失败
         */
        bool send();

        /**
         * @brief 建立明文TCP连接发送邮件
         * @return true表示发送成功，false表示失败
         */
        bool senddata();

        /**
         * @brief 建立SSL加密连接发送邮件
         * @return true表示发送成功，false表示失败
         */
        bool sendssldata();

        /**
         * @brief 添加邮件附件
         * @param file_name 附件文件路径
         * @return true表示添加成功，false表示文件不存在
         */
        bool addattachments(std::string file_name);

        /**
         * @brief SMTP状态机循环处理函数，根据当前状态生成对应的SMTP命令
         * @return true表示处理成功
         */
        bool sendloop();

        /**
         * @brief 获取当前GMT时间，格式用于邮件Date头
         * @return GMT时间字符串，格式如 "Thu, 01 Jan 1970 00:00:00 GMT"
         */
        std::string getgmttime();

    public:
        std::string smpturl;                              ///< SMTP服务器地址
        std::string username;                             ///< 邮箱账号（用于认证和发件人）
        std::string password;                             ///< 邮箱密码
        unsigned int port = 25;                           ///< SMTP端口，默认25，SSL通常用465
        bool isssl = false;                               ///< 是否启用SSL加密
        bool ishtml = false;                              ///< 是否为HTML格式邮件
        bool ehlo_failed_ = false;                       ///< EHLO被拒绝(5xx)后回退HELO的标志

        std::string sendcommand;                          ///< 当前要发送的SMTP命令
        char data[2049];                                  ///< 接收服务器响应的缓冲区
        std::string smtp_readbuf_;                        ///< 读取缓冲：保存未凑成完整应答的剩余字节

        // 反复调用 read_chunk 直到凑齐一条完整应答；data 里是整条应答（截到 2048），
        // 返回值为长度，0 表示读失败。read_chunk(buf, cap) 返回读到的字节数，失败返回 <=0。
        template <class ReadFn>
        unsigned int read_smtp_response(ReadFn &&read_chunk)
        {
            for (;;)
            {
                std::string response;
                int st = take_smtp_response(smtp_readbuf_, response);
                if (st == 1)
                {
                    unsigned int len = response.size() > 2048 ? 2048 : static_cast<unsigned int>(response.size());
                    std::memset(data, 0x00, sizeof(data));
                    std::memcpy(data, response.data(), len);
                    return len;
                }
                if (st < 0)
                {
                    errormsg.append("smtp response too long!");
                    return 0;
                }
                char chunk[1024];
                int got = read_chunk(chunk, sizeof(chunk));
                if (got <= 0)
                {
                    return 0;
                }
                smtp_readbuf_.append(chunk, static_cast<std::size_t>(got));
            }
        }

        /**
         * @brief SMTP会话状态枚举，定义邮件发送流程的各个阶段
         */
        enum State
        {
            SEND_START,           ///< 会话开始，等待服务器欢迎信息
            SEND_EHLO,            ///< 发送EHLO/HELO命令
            SEND_AUTH_LOGIN,      ///< 发送AUTH LOGIN命令
            SEND_USER,            ///< 发送base64编码的用户名
            SEND_PASSWORD,        ///< 发送base64编码的密码
            SEND_MAIL_FROM,       ///< 发送MAIL FROM命令
            SEND_RCPT_TO,         ///< 发送RCPT TO命令
            SEND_DATA,            ///< 发送DATA命令
            SEND_EMAIL,           ///< 发送邮件内容
            SEND_END,             ///< 发送完成
            SEND_ERROR            ///< 发送失败
        };
        State state;                                      ///< 当前SMTP会话状态

        asio::error_code ec;                              ///< ASIO错误码
        std::string errormsg;                             ///< 错误信息描述

        std::string replyemail;                           ///< 回复邮箱地址
        std::string replyname;                            ///< 回复人名称
        std::string toemail;                              ///< 收件人邮箱地址
        std::string toname;                               ///< 收件人显示名称
        std::string subject;                              ///< 邮件主题
        std::string content;                              ///< 邮件正文内容
        std::string fromname;                             ///< 发件人显示名称（替代邮箱地址显示）

        std::vector<std::string> attachments;             ///< 附件文件路径列表

        /**
         * @brief 附件发送结构，存储附件的前置MIME头、文件名和后置数据
         */
        struct sendfile_t
        {
            std::string predata;   ///< 附件的MIME头信息（Content-Type等）
            std::string filename;  ///< 附件文件路径
            std::string afterdata; ///< 附件后的边界分隔符
        };
        std::vector<sendfile_t> sendfiles;                ///< 待发送的附件数据列表
        std::string boundarystsr;                         ///< MIME多部分邮件的边界标识
    };

}