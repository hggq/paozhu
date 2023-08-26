#include "httppeer.h"
#include "func.h"
#include "testhttpclient.h"
namespace http
{
//@urlpath(null,testhttpclient1)
std::string testhttpclient_get_body(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "hello world!  test file_get_contents";
    std::string respbody;
    respbody = "https://www.kernel.org/category/about.html";
    respbody = file_get_contents(respbody);

    client << "<hr />";
    client << "<h1>https://www.kernel.org/category/about.html</h1>";
    client << "<hr />";
    client << respbody;
    return "";
}
//@urlpath(null,testhttpclient2)
std::string testhttpclient_get_file(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->getpeer();
    std::string content = "<p>hello world!  test testfileputcontent4 write content to local file</p>";
    client << content;
    std::string respbody = "https://fonts.googleapis.com/css?family=Source+Sans+Pro:300,400,400i,700&display=fallback";

    std::map<std::string, std::string> param;
    param["method"] = "GET";
    param["header-content"] =
        "Accept: "
        "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\nAccept-Language: "
        "zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\r\n";
    param["User-Agent"] = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:104.0) Gecko/20100101 Firefox/104.0";

    respbody = file_get_contents(respbody, param);

    client << "<hr />";
    client << "<h1>https://fonts.googleapis.com/css?family=Source+Sans+Pro:300,400,400i,700&display=fallback</h1>";
    client << "<hr />";
    client << "<pre>" << param["response-header"] << "</pre>";
    client << "<hr />";
    client << respbody;

    std::vector<std::string> urls;
    std::string murl;
    for (unsigned int i = 0; i < respbody.size(); i++)
    {
        if (respbody[i] == 'h' && respbody[i + 1] == 't' && respbody[i + 2] == 't' && respbody[i + 3] == 'p' &&
            respbody[i + 4] == 's' && respbody[i + 5] == ':')
        {
            murl.clear();
            for (; i < respbody.size(); i++)
            {
                if (respbody[i] == ')' || respbody[i] == '\"' || respbody[i] == 0x20 || respbody[i] == '\'')
                {
                    break;
                }
                murl.push_back(respbody[i]);
            }
            urls.emplace_back(murl);
        }
    }

    murl.clear();
    murl.append(client.get_sitepath());
    murl.append("fonts");
    std::filesystem::path paths = murl;
    if (!std::filesystem::exists(paths))
    {
        if (std::filesystem::create_directories(paths))
        {
            client << "<p>fs create dir succ: " << murl << " </p>";
        }
        std::filesystem::permissions(paths,
                                     std::filesystem::perms::owner_all | std::filesystem::perms::group_all |
                                         std::filesystem::perms::others_all,
                                     std::filesystem::perm_options::add);
    }

    for (unsigned int i = 0; i < urls.size(); i++)
    {
        respbody = file_get_contents(urls[i]);
        murl.clear();
        auto pinfo = filepath(urls[i]);
        murl.append(client.get_sitepath());
        murl.append("fonts/");
        murl.append(pinfo["basename"]);

        if (file_put_contents(murl, respbody))
        {
            client << "<p>success:" << murl << "</p>";
        }
        else
        {
            client << "<p>fail:" << murl << "</p>";
        }
    }
    return "";
}
}// namespace http