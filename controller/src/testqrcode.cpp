#include <chrono>
#include <thread>
#include "httppeer.h"
#include <filesystem>
#ifdef ENABLE_IMAGE
#include "qrcode.h"
#include "pzpng.h"
#endif

#include "testqrcode.h"
namespace http
{
//@urlpath(null,testqrcode)
std::string testqrcode(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string atxt = client.post["qrcontent"].to_string();
    if (atxt.size() == 0)
    {
        atxt = "qrsting hello world !";
    }
    client << "<p>" << atxt << "</p>";

#ifdef ENABLE_IMAGE
    namespace fs = std::filesystem;
    qr::qrcode q;
    q.text(atxt, qr::Ecc::M, 1);
    image::png img;
    unsigned char scale = 10;
    img.create(q.width()*scale + 40, q.height()*scale + 40, 6, 8);
    img.fillColor({255, 255, 255, 255});
    img.qrdata(q.data,q.width(),q.height(),scale, 20 ,20);
    std::string wwwpath;
    wwwpath.append(client.get_sitepath());
    wwwpath.append("/upload");

    fs::path paths  = wwwpath;
    bool is_success = fs::create_directories(paths);
    if (is_success)
    {
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    wwwpath.append("/test.png");
    // std::string wwwurl = client.get_hosturl();
    img.save(wwwpath);

    client << "文字      : " << atxt << '\n';
    client << "版本      : " << q.version << '\n';
    client << "黑点大小  : " << q.scale << '\n';
    client << "尺寸      : " << q.width() << " x " << q.height() << '\n';
    client << "q.data 长度: " << q.data.size() << " (= width*height)\n\n";
    //client.output = img.imshow();
    client << "<img src=\"" << "/upload/test.png" << "?token=123456\">";

#endif
    client << "<form method=\"post\" action=\"/testqrcode\"><p><textarea name=\"qrcontent\"></textarea></p><p><input type=\"submit\" name=\"submit2\" value=\"Submit\"></p></form>";
    return "";
}

}// namespace http