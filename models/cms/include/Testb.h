#ifndef ORM_CMS_TESTB_H
#define ORM_CMS_TESTB_H
#include "mysqlorm.hpp"
#include "cms/include/testbbase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
 *If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

namespace orm
{
namespace cms
{
class Testb : public mysql_orm<Testb, testbbase>
{
  public:
    Testb(std::string dbtag);
    Testb();
};
}// namespace cms
};// namespace orm
#endif
