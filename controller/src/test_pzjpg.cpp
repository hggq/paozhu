#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_pzpng.h"
#include "func.h"
#include <memory>
#include <string>
#ifdef ENABLE_IMAGE
#include "pzjpg.h"
#include "ttffont.h"
#endif// ENABLE_IMAGE
namespace http
{
//@urlpath(null,test_pzjpg)
std::string test_pzjpg(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_IMAGE
    server_loaclvar &static_server_var = get_server_global_var();

    if (static_server_var.config_path.size() < 5)
    {
        client << "<p> static_server_var.config_path empty </p>";
        return "";
    }

    std::string file_conf = dir_name(static_server_var.config_path);

    if (file_conf.size() > 0 && file_conf.back() != '/')
    {
        file_conf.push_back('/');
    }
    file_conf.append("docs/");

    image::jpg img;
    img.create(800, 600);
    img.fillColor({255, 255, 255});

    img.drawLine(100, 100, 700, 500, {255, 0, 0}, 5);
    img.drawLine(50, 300, 750, 300, {0, 0, 255}, 10);
    img.drawLine(400, 50, 400, 550, {0, 255, 0}, 8);
    client << "<p>开始</p>";
    img.save(file_conf + "line_thickness_test.jpg");
    client << "<p>保存完成</p>";

#else
    client << "<p>Please: cmake .. -DENABLE_IMAGE=ON </p>";
#endif// ENABLE_IMAGE

    return "";
}

//@urlpath(null,test_outjpg)
std::string test_outjpg(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_IMAGE
    server_loaclvar &static_server_var = get_server_global_var();

    if (static_server_var.log_path.size() < 5)
    {
        client << "<p> static_server_var.log_path empty </p>";
        return "";
    }

    std::string file_conf = dir_name(static_server_var.log_path);

    if (file_conf.size() > 0 && file_conf.back() != '/')
    {
        file_conf.push_back('/');
    }
    file_conf.append("docs/");

    image::jpg img;
    img.create(800, 600);
    img.fillColor({255, 255, 255});

    img.drawLine(100, 100, 700, 500, {255, 0, 0}, 5);
    img.drawLine(50, 300, 750, 300, {0, 0, 255}, 10);
    img.drawLine(400, 50, 400, 550, {0, 255, 0}, 8);
    // client << "<p>开始</p>";
    // img.save(file_conf+"line_thickness_test.jpg");
    // client << "<p>保存完成</p>";

    image::ttffont font;
    if (font.load(file_conf + "font/AlibabaPuHuiTi-Light.ttf"))
    {
        const std::string text = "AbcXyz8核鑫03炮竹";
        font.drawTextOutline(img, 20, 70, 56.0f, text, {30, 30, 200});
        font.drawText(img, 20, 160, 56.0f, text, {30, 130, 30});
    }

    client.type("image/jpg");
    auto vec      = img.imshow();
    client.output = std::string(reinterpret_cast<const char *>(vec.data()), vec.size());

#else
    client << "<p>Please: cmake .. -DENABLE_IMAGE=ON </p>";
#endif// ENABLE_IMAGE

    return "";
}

//@urlpath(null,test_showjpg)
std::string test_showjpg(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_IMAGE
    server_loaclvar &static_server_var = get_server_global_var();

    if (static_server_var.config_path.size() < 5)
    {
        client << "<p> static_server_var.config_path empty </p>";
        return "";
    }

    std::string file_conf = dir_name(static_server_var.config_path);

    if (file_conf.size() > 0 && file_conf.back() != '/')
    {
        file_conf.push_back('/');
    }
    client << "<p>config_path:"<< static_server_var.config_path <<" </p>";
    client << "<p>file_conf:"<< file_conf <<" </p>";
    
    std::string srcfile = file_conf + "docs/images/2388_445.jpg";
    std::string newfile = file_conf + "docs/images/2388_445_new.jpg";
    
    client << "<p>srcfile:"<< srcfile <<" </p>";
    client << "<p>newfile:"<< newfile <<" </p>";
    
    image::jpg img;
    bool isok = img.read(srcfile);
    if(isok)
    {
        client << "<p>read file OK</p>";
        client << "<p>size:"<< img.width <<"x"<< img.height <<" </p>";
        client << "<p>pixels:"<< img.pixels.size() <<" </p>";
        bool saveok = img.save(newfile);
        if(saveok)
        {
            client << "<p>save new file OK:"<< newfile <<" </p>";
        }
        else
        {
            client << "<p>save new file FAIL:"<< newfile <<" </p>";
        }
    }
    else
    {
        client << "<p>read file error:"<< srcfile <<" </p>";
    }

#else
    client << "<p>Please: cmake .. -DENABLE_IMAGE=ON </p>";
#endif// ENABLE_IMAGE

    return "";
}

}// namespace http