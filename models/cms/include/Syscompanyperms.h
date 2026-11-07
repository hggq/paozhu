#ifndef ORM_CMS_SYSCOMPANYPERMS_H
#define ORM_CMS_SYSCOMPANYPERMS_H
#include "orm/cms/include/syscompanyperms_opsql.h" 
#include "orm/cms/include/syscompanyperms_base.h"

/* Sat, 07 Nov 2026 04:02:55 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Syscompanyperms : public syscompanyperms_opsql<Syscompanyperms,syscompanyperms_base>{
		 public:
		 Syscompanyperms(std::string dbtag_);
		 Syscompanyperms();
		};
	 }
 }
#endif
