#ifndef LIBS_IMG_UPLOAD_IMAGES_H
#define LIBS_IMG_UPLOAD_IMAGES_H
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sstream>
#include "serverconfig.h"
#include "unicode.h"
#include "httppeer.h"
#include "request.h"
#include "json_reflect_headers.h"
namespace http
{

class upload_images
{
  public:
    upload_images(std::shared_ptr<httppeer> peer) : client(peer->get_peer()) {};
    bool init();
    bool get_config();
    bool upload_img(const std::string &);
    unsigned int list_img();
    unsigned int list_file();
    bool upload_video(const std::string &);
    bool upload_base64(const std::string &);
    bool upload_upfile(const std::string &);
    void set_host(const std::string &);
    void set_sitepath(const std::string &);

  public:
    std::string upload_path;
    std::string url_host;
    std::string url_path;
    std::string jsoncontent;
    std::string configpath;
    std::string sitepath;
    httppeer &client;
    obj_val config_json;
    struct
    {
        std::string state;
        std::string url;
        std::string title;
        std::string original;
        std::string type;
        unsigned long long size;
        std::string to_json()
        {
            std::stringstream _stream;
            _stream << "{\"state\":\"";
            _stream << state;
            _stream << "\",\"url\":\"";
            _stream << url;
            _stream << "\",\"title\":\"";
            _stream << title;
            _stream << "\",\"original\":\"";
            _stream << original;
            _stream << "\",\"type\":\"";
            _stream << type;
            _stream << "\",\"size\":";
            _stream << std::to_string(size);
            _stream << "}";
            return _stream.str();
        }
    } upload_info;
    img_upload_outjson_t imglist;
};

}//namespace http
#endif
