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

template<typename JSON_REF_OBJ_TEMP>
std::string json_encode([[maybe_unused]] const JSON_REF_OBJ_TEMP &json_reflectobj){ return ""; }

template<typename JSON_REF_OBJ_TEMP>
std::string json_encode([[maybe_unused]] const std::vector<JSON_REF_OBJ_TEMP> &json_reflectobj){ return ""; }

template<typename JSON_REF_OBJ_TEMP>
unsigned int json_decode([[maybe_unused]] JSON_REF_OBJ_TEMP &json_reflectobj,[[maybe_unused]] const std::string &_json_data,[[maybe_unused]] unsigned int _offset){ return 0; }

template<typename JSON_REF_OBJ_TEMP>
unsigned int json_decode([[maybe_unused]] std::vector<JSON_REF_OBJ_TEMP> &json_reflectobj,[[maybe_unused]] const std::string &_json_data,[[maybe_unused]] unsigned int _offset){ return 0; }


namespace psy
{


std::string json_encode(const topics_tree_outjson_t &json_reflectobj);


std::string json_encode(const std::vector<topics_tree_outjson_t> &json_reflectobj);

unsigned int json_decode(topics_tree_outjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<topics_tree_outjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);
}

namespace psy
{


std::string json_encode(const department_outjson_t &json_reflectobj);


std::string json_encode(const std::vector<department_outjson_t> &json_reflectobj);

unsigned int json_decode(department_outjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<department_outjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

std::string json_encode(const department_listoutjson_t &json_reflectobj);


std::string json_encode(const std::vector<department_listoutjson_t> &json_reflectobj);

unsigned int json_decode(department_listoutjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<department_listoutjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);
}

namespace http
{


std::string json_encode(const img_upload_list_t &json_reflectobj);


std::string json_encode(const std::vector<img_upload_list_t> &json_reflectobj);

unsigned int json_decode(img_upload_list_t &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<img_upload_list_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

std::string json_encode(const img_upload_outjson_t &json_reflectobj);


std::string json_encode(const std::vector<img_upload_outjson_t> &json_reflectobj);

unsigned int json_decode(img_upload_outjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<img_upload_outjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);
}
