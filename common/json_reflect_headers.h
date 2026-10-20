#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "types/topics_type.h"
#include "types/techempower_json.h"
#include "department/department_type.h"
#include "img/editorupload_type.h"
#include "types/json_ref_obj.h"
#include "unicode.h"

namespace psy
{


std::string json_encode(const topics_tree_outjson_t &json_reflectobj);


std::string json_encode(const std::vector<topics_tree_outjson_t> &json_reflectobj);

unsigned int json_decode(topics_tree_outjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<topics_tree_outjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);
}

namespace http
{


std::string json_encode(const techempower_outjson_t &json_reflectobj);


std::string json_encode(const std::vector<techempower_outjson_t> &json_reflectobj);

unsigned int json_decode(techempower_outjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<techempower_outjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);
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

namespace json
{


std::string json_encode(const json_ref_obj_recent_orders_t &json_reflectobj);


std::string json_encode(const std::vector<json_ref_obj_recent_orders_t> &json_reflectobj);

unsigned int json_decode(json_ref_obj_recent_orders_t &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<json_ref_obj_recent_orders_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

std::string json_encode(const json_ref_obj &json_reflectobj);


std::string json_encode(const std::vector<json_ref_obj> &json_reflectobj);

unsigned int json_decode(json_ref_obj &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);

unsigned int json_decode(std::vector<json_ref_obj> &json_reflectobj,const std::string &_json_data,unsigned int _offset=0);
}
