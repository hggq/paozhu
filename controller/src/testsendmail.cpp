#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testsendmail.h"
#include "send_email.h"
namespace http
{
//@urlpath(null,testsendmaildo)
std::string testsendmaildo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "<p>hello world!</p>";
    client << "<p>";
    client << (unsigned int)client.post.size();
    client << "</p>";
    for (auto [first, second] : client.post.as_array())
    {
        client << "<p>";
        client << client.post[first];
        client << "</p>";
    }

    send_email s;
    s.isssl   = false;// ssl on off   ssl port 465
    s.smpturl = "smtp.163.com";
    s.port    = 25;// port

    s.username = "XXX@163.com";// account
    s.password = "XXXXXXX";    // passworld

    s.replyemail = "XXX@163.com";  // show reply email
    s.fromname   = "web site name";// send of name

    s.toemail = client.post["toemail"].to_string();// send to email
    s.toname  = client.post["toname"].to_string(); //   name of person to send to

    s.subject = client.post["subject"].to_string();//  email title
    s.content = client.post["msgbody"].to_string();//  email content,default html content

    // s.addattachments("会议议程安排.docx");

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

}// namespace http