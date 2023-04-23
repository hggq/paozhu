#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "department/department_type.h"
#include "unicode.h"

template<typename JSON_REF_OBJ_TEMP>
std::string json_encode(const JSON_REF_OBJ_TEMP &json_reflectobj){ return ""; }

template<typename JSON_REF_OBJ_TEMP>
std::string json_encode(const std::vector<JSON_REF_OBJ_TEMP> &json_reflectobj){ return ""; }

template<typename JSON_REF_OBJ_TEMP>
unsigned int json_decode(JSON_REF_OBJ_TEMP &json_reflectobj,const std::string &_json_data,unsigned int _offset){ return 0; }

template<typename JSON_REF_OBJ_TEMP>
unsigned int json_decode(std::vector<JSON_REF_OBJ_TEMP> &json_reflectobj,const std::string &_json_data,unsigned int _offset){ return 0; }


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
