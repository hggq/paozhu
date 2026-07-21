#pragma once

#include <functional>
#include <string>
#include <vector>
#include <list>
#include <memory>
#include "http_socket.h"
#include "client_context.h"

namespace http
{

    /**
     * @brief 协程版SMTP邮件发送类，支持SSL/TLS加密连接和附件发送
     * 
     * 实现基于ASIO协程的SMTP协议客户端，支持AUTH LOGIN认证方式，
     * 可发送纯文本或HTML邮件，支持添加附件。使用状态机模式管理SMTP会话流程。
     */
    class async_send_email : public std::enable_shared_from_this<async_send_email>
    {
    public:
        /**
         * @brief 默认构造函数
         */
        async_send_email() : strand_(asio::make_strand(*(get_client_context_obj().ioc)))
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
        async_send_email(std::string smpt_url, std::string account, std::string passwd, unsigned int p, bool isssl)
            : smpturl(smpt_url), username(account), password(passwd), port(p), isssl(isssl),
              strand_(asio::make_strand(*(get_client_context_obj().ioc)))
        {
        }

        /**
         * @brief 带基本参数的构造函数，默认端口25，不使用SSL
         * @param smpt_url SMTP服务器地址
         * @param account 邮箱账号
         * @param passwd 邮箱密码
         */
        async_send_email(std::string smpt_url, std::string account, std::string passwd)
            : smpturl(smpt_url), username(account), password(passwd),
              strand_(asio::make_strand(*(get_client_context_obj().ioc)))
        {
        }

        /**
         * @brief 协程版邮件发送入口函数
         * @return asio::awaitable<bool> true表示发送成功，false表示失败
         */
        asio::awaitable<bool> async_send();

        /**
         * @brief 添加邮件附件
         * @param file_name 附件文件路径
         * @return true表示添加成功，false表示文件不存在
         */
        bool addattachments(std::string file_name);

        /**
         * @brief SMTP状态机命令生成函数
         * @return true表示处理成功
         */
        bool sendloop();

        /**
         * @brief 获取当前GMT时间，格式用于邮件Date头
         * @return GMT时间字符串
         */
        std::string getgmttime();

    public:
        std::string smpturl;                              ///< SMTP服务器地址
        std::string username;                             ///< 邮箱账号（用于认证和发件人）
        std::string password;                             ///< 邮箱密码
        unsigned int port = 25;                           ///< SMTP端口，默认25，SSL通常用465
        bool isssl = false;                               ///< 是否启用SSL加密
        bool ishtml = false;                              ///< 是否为HTML格式邮件

        std::string sendcommand;                          ///< 当前要发送的SMTP命令
        char data[2049];                                  ///< 接收服务器响应的缓冲区

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

        asio::strand<asio::io_context::executor_type> strand_;  ///< ASIO strand用于线程安全

        std::shared_ptr<asio::ip::tcp::socket> sock;                          ///< 明文TCP socket
        std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsock;    ///< SSL socket
        std::shared_ptr<asio::ssl::context> ssl_context;                      ///< SSL上下文
    };

}