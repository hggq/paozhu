#ifndef ORM_CMS_DEPARTMENT_H
#define ORM_CMS_DEPARTMENT_H
#include "mysqlorm.hpp"
#include "cms/include/departmentbase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
 *If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

namespace orm
{
namespace cms
{
class Department : public mysql_orm<Department, departmentbase>
{
  public:
    Department(std::string dbtag);
    Department();
};
}// namespace cms
};// namespace orm
#endif
