#ifndef LIBS_TYPES_TOPICS_TYPE_H
#define LIBS_TYPES_TOPICS_TYPE_H
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sstream>
#include "unicode.h"

namespace psy
{
//@reflect json to_json from_json
struct topics_tree_outjson_t
{
    unsigned int id       = 0;
    unsigned int parentid = 0;
    unsigned int cateid   = 0;
    std::string value;
    std::string urlpath;
    bool isview = false;
    std::vector<topics_tree_outjson_t> children;

    bool _is_use        = false;
    unsigned int _level = 0;
};

} // namespace psy
#endif
