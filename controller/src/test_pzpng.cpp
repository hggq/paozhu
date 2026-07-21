#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_pzpng.h"
#include "func.h"
#include <memory>
#include <string>
#ifdef ENABLE_IMAGE
#include "pzpng.h"
#endif// ENABLE_IMAGE
namespace http
{
//@urlpath(null,test_pzpng)
std::string test_pzpng(std::shared_ptr<httppeer> peer)
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

    image::png img;
    img.create(800, 600, 6, 8);
    img.fillColor({255, 255, 255, 255});

    // 设置模式：速度优先 + 显示进度
    img.setSpeedMode(false);   // true: 快速, false: 平衡（默认）
 
    img.drawLine(100, 100, 700, 500, {255, 0, 0, 255}, 5);
    img.drawLine(50, 300, 750, 300, {0, 0, 255, 255}, 10);
    img.drawLine(400, 50, 400, 550, {0, 255, 0, 255}, 8);
    client << "<p>开始</p>";
    img.save(file_conf+"line_thickness_test.png");
    client << "<p>保存完成</p>";


// // 逐像素（自行保证越界）
// img.at(2, 1) = {9, 8, 7, 6};
// uint8_t r = img.at(0, 0).r;

// // 整行高效遍历
// if (Color *row = img.rowPtr(y)) {
// for (uint32_t x = 0; x < img.info.width; ++x) row[x].a = 128;
// }

#else
    client << "<p>Please: cmake .. -DENABLE_IMAGE=ON </p>";
#endif// ENABLE_IMAGE

    return "";
}

//@urlpath(null,test_outpng)
std::string test_outpng(std::shared_ptr<httppeer> peer)
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

    image::png img;
    img.create(800, 600, 6, 8);
    img.fillColor({255, 255, 255, 255});

    // 设置模式：速度优先 + 显示进度
    img.setSpeedMode(false);   // true: 快速, false: 平衡（默认）
 
    img.drawLine(100, 100, 700, 500, {255, 0, 0, 255}, 5);
    img.drawLine(50, 300, 750, 300, {0, 0, 255, 255}, 10);
    img.drawLine(400, 50, 400, 550, {0, 255, 0, 255}, 8);

    
    // client << "<p>开始</p>";
    // img.save(file_conf+"line_thickness_test.png");
    // client << "<p>保存完成</p>";
    client.type("image/png");
    client.output = img.imshow();
// // 逐像素（自行保证越界）
// img.at(2, 1) = {9, 8, 7, 6};
// uint8_t r = img.at(0, 0).r;

// // 整行高效遍历
// if (Color *row = img.rowPtr(y)) {
// for (uint32_t x = 0; x < img.info.width; ++x) row[x].a = 128;
// }

#else
    client << "<p>Please: cmake .. -DENABLE_IMAGE=ON </p>";
#endif// ENABLE_IMAGE

    return "";
}

}// namespace http