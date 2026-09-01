#ifndef ORM_CMS_TOPICSET_H
#define ORM_CMS_TOPICSET_H
#include "orm/cms/include/topicset_opsql.h" 
#include "orm/cms/include/topicset_base.h"

/* Tue, 01 Sep 2026 03:21:26 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Topicset : public topicset_opsql<Topicset,topicset_base>{
		 public:
		 Topicset(std::string dbtag_);
		 Topicset();
		};
	 }
 }
#endif
