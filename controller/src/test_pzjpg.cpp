#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_pzpng.h"
#include "func.h"
#include <memory>
#include <string>
#ifdef ENABLE_IMAGE
#include "pzjpg.h"
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
    // server_loaclvar &static_server_var = get_server_global_var();

    // if (static_server_var.config_path.size() < 5)
    // {
    //     client << "<p> static_server_var.config_path empty </p>";
    //     return "";
    // }

    // std::string file_conf = dir_name(static_server_var.config_path);

    // if (file_conf.size() > 0 && file_conf.back() != '/')
    // {
    //     file_conf.push_back('/');
    // }
    // file_conf.append("docs/");

    image::jpg img;
    img.create(800, 600);
    img.fillColor({255, 255, 255});

    img.drawLine(100, 100, 700, 500, {255, 0, 0}, 5);
    img.drawLine(50, 300, 750, 300, {0, 0, 255}, 10);
    img.drawLine(400, 50, 400, 550, {0, 255, 0}, 8);
    // client << "<p>开始</p>";
    // img.save(file_conf+"line_thickness_test.jpg");
    // client << "<p>保存完成</p>";
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
    file_conf.append("docs/images/2388_445.jpg");

    image::jpg img;
    bool isok = img.read(file_conf);
    if(!isok)
    {
        client.type("image/jpg");
        auto vec      = img.imshow();
        client.output = std::string(reinterpret_cast<const char *>(vec.data()), vec.size());
    }
    client << "<p>read file error:"<< file_conf <<" </p>";

#else
    client << "<p>Please: cmake .. -DENABLE_IMAGE=ON </p>";
#endif// ENABLE_IMAGE

    return "";
}

}// namespace http