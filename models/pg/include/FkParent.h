#ifndef ORM_PG_FKPARENT_H
#define ORM_PG_FKPARENT_H
#include "orm/pg/include/fk_parent_opsql.h" 
#include "orm/pg/include/fk_parent_base.h"

/* Sat, 12 Sep 2026 07:38:42 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace pg { 
		class FkParent : public fk_parent_opsql<FkParent,fk_parent_base>{
		 public:
		 FkParent(std::string dbtag_);
		 FkParent();
		};
	 }
 }
#endif
