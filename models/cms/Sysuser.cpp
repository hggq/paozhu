#include "mysqlorm.hpp"
#include "cms/include/sysuserbase.h"
#include "cms/include/Sysuser.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
 *If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

namespace orm
{
namespace cms
{

Sysuser::Sysuser(std::string dbtag) : mysql_orm(dbtag) {}
Sysuser::Sysuser() : mysql_orm() {}

std::vector<orm::cms::articlebase::meta> Sysuser::gettoparticle(int userid)
{
    auto art = orm::cms::Article();
    userid   = this->data.companyid;
    art.where("userid", userid).order(" aid desc ").limit(5).fetch();
    //art.where("userid",userid).whereOr("userid",0).order(" aid desc ").limit(5).fetch();
    std::cout << art.sqlstring;
    return art.record;
}
}// namespace cms

}// namespace orm
