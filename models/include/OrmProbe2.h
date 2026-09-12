#ifndef ORM_DEFAULT_ORMPROBE2_H
#define ORM_DEFAULT_ORMPROBE2_H
#include "orm/include/orm_probe2_opsql.h" 
#include "orm/include/orm_probe2_base.h"

/* Sat, 12 Sep 2026 07:42:43 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
		class OrmProbe2 : public orm_probe2_opsql<OrmProbe2,orm_probe2_base>{
		 public:
		 OrmProbe2(std::string dbtag_);
		 OrmProbe2();
		};
}
#endif
