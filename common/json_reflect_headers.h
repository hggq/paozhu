#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "types/topics_type.h"
#include "department/department_type.h"
#include "img/editorupload_type.h"
#include "img/upload_images.h"
#include "unicode.h"

template <typename JSON_REF_OBJ_TEMP> std::string json_encode(const JSON_REF_OBJ_TEMP &json_reflectobj)
{
    if (sizeof(json_reflectobj))
    {
    }
    return "";
}

template <typename JSON_REF_OBJ_TEMP> std::string json_encode(const std::vector<JSON_REF_OBJ_TEMP> &json_reflectobj)
{
    if (json_reflectobj.size() > 0)
    {
    }
    return "";
}

template <typename JSON_REF_OBJ_TEMP>
unsigned int json_decode(JSON_REF_OBJ_TEMP &json_reflectobj, const std::string &_json_data, unsigned int _offset)
{
    if (_offset < _json_data.size() && sizeof(json_reflectobj))
    {
    }
    return _json_data.size();
}

template <typename JSON_REF_OBJ_TEMP>
unsigned int
json_decode(std::vector<JSON_REF_OBJ_TEMP> &json_reflectobj, const std::string &_json_data, unsigned int _offset)
{
    if (_offset < _json_data.size() && json_reflectobj.size() > 0)
    {
    }
    return _json_data.size();
}

namespace psy
{

std::string json_encode(const topics_tree_outjson_t &json_reflectobj);

std::string json_encode(const std::vector<topics_tree_outjson_t> &json_reflectobj);

unsigned int
json_decode(topics_tree_outjson_t &json_reflectobj, const std::string &_json_data, unsigned int _offset = 0);

unsigned int json_decode(std::vector<topics_tree_outjson_t> &json_reflectobj,
                         const std::string &_json_data,
                         unsigned int _offset = 0);
}// namespace psy

namespace psy
{

std::string json_encode(const department_outjson_t &json_reflectobj);

std::string json_encode(const std::vector<department_outjson_t> &json_reflectobj);

unsigned int
json_decode(department_outjson_t &json_reflectobj, const std::string &_json_data, unsigned int _offset = 0);

unsigned int json_decode(std::vector<department_outjson_t> &json_reflectobj,
                         const std::string &_json_data,
                         unsigned int _offset = 0);

std::string json_encode(const department_listoutjson_t &json_reflectobj);

std::string json_encode(const std::vector<department_listoutjson_t> &json_reflectobj);

unsigned int
json_decode(department_listoutjson_t &json_reflectobj, const std::string &_json_data, unsigned int _offset = 0);

unsigned int json_decode(std::vector<department_listoutjson_t> &json_reflectobj,
                         const std::string &_json_data,
                         unsigned int _offset = 0);
}// namespace psy

namespace http
{

std::string json_encode(const img_upload_list_t &json_reflectobj);

std::string json_encode(const std::vector<img_upload_list_t> &json_reflectobj);

unsigned int json_decode(img_upload_list_t &json_reflectobj, const std::string &_json_data, unsigned int _offset = 0);

unsigned int
json_decode(std::vector<img_upload_list_t> &json_reflectobj, const std::string &_json_data, unsigned int _offset = 0);

std::string json_encode(const img_upload_outjson_t &json_reflectobj);

std::string json_encode(const std::vector<img_upload_outjson_t> &json_reflectobj);

unsigned int
json_decode(img_upload_outjson_t &json_reflectobj, const std::string &_json_data, unsigned int _offset = 0);

unsigned int json_decode(std::vector<img_upload_outjson_t> &json_reflectobj,
                         const std::string &_json_data,
                         unsigned int _offset = 0);
}// namespace http
