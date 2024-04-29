#ifndef LIBS_IMG_EDITORUPLOAD_TYPE_H
#define LIBS_IMG_EDITORUPLOAD_TYPE_H
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sstream>
#include "unicode.h"

namespace http
{

//@reflect json to_json from_json
struct img_upload_list_t
{
    std::string url;
    unsigned long long mtime = 0;
};

//@reflect json to_json from_json
struct img_upload_outjson_t
{
    std::string state;
    std::vector<img_upload_list_t> list;
    unsigned int total = 0;
    unsigned int start = 0;
};

} //namespace http
#endif
