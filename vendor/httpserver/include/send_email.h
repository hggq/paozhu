#pragma once

#include <functional>
#include <string>
#include <vector>
#include <list>
#include "base64.h"
#include "http_socket.h"

namespace http
{

    class send_email
    {
    public:
        send_email()
        {
        }
        send_email(std::string smpt_url, std::string account, std::string passwd, unsigned int p, bool isssl) : smpturl(smpt_url), username(account), password(passwd), port(p), isssl(isssl)
        {
        }
        send_email(std::string smpt_url, std::string account, std::string passwd) : smpturl(smpt_url), username(account), password(passwd)
        {
        }
        bool send(std::string reply_email, std::string from_name, std::string to_email, std::string to_name, std::string tosubject, std::string body);
        bool send();
        bool senddata();
        bool sendssldata();
        bool addattachments(std::string file_name);
        bool sendloop();
        std::string getgmttime();

    public:
        ////////////////////
        std::string smpturl;
        std::string username;
        std::string password;
        unsigned int port;
        bool isssl;
        bool ishtml;

        std::string sendcommand;
        char data[2049];

        enum State
        {
            SEND_START,
            SEND_EHLO,
            SEND_AUTH_LOGIN,
            SEND_USER,
            SEND_PASSWORD,
            SEND_MAIL_FROM,
            SEND_RCPT_TO,
            SEND_DATA,
            SEND_EMAIL,
            SEND_END,
            SEND_ERROR
        };
        State state;
        asio::error_code ec;
        std::string errormsg;
        /////////////////////////////////
        std::string replyemail;
        std::string toemail;
        std::string toname;
        std::string subject;
        std::string content;
        std::string fromname; // show formemail name alias 代替显示邮件地址名称

        std::vector<std::string> attachments; // 附件
        struct sendfile_t
        {
            std::string predata;
            std::string filename;
            std::string afterdata;
        };
        std::vector<sendfile_t> sendfiles;
        std::string boundarystsr;
    };

}