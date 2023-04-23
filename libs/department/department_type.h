#ifndef LIBS_DEPARTMENT_TYPE_H
#define LIBS_DEPARTMENT_TYPE_H
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sstream>
#include "unicode.h"

namespace psy
{ 
//@reflect json to_json from_json
struct department_outjson_t
{
  unsigned int id          = 0;
  unsigned int key         = 0;
  unsigned int value       = 0;
  unsigned int parentid    = 0;
  unsigned int bianzhi_num = 0;
  bool isopen              = true;
  bool _is_use             = false;
  std::string title;
  std::string desc;
  std::vector<department_outjson_t> children;
  std::string to_json(std::string aa="",int offset=0);
  unsigned int from_json()
  {
        //**aaaaa
        /*
        aaaa [][}]
        */
        char b='}';
        if(b=='{')
        {
            std::cout<<"assss}sss}{ssss}sss}"<<std::endl;
        }
        return 0;
  }
};

//@reflect json to_json from_json
struct department_listoutjson_t
{
  unsigned int code=0;
  struct department_tables
  {
    std::vector<department_outjson_t> list;
    unsigned int total=0;
  } data;
  std::vector<std::vector<std::string>> names;
};

} // namespace psy
#endif
