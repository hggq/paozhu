#ifndef ORM_CMS_TOPIC_H
#define ORM_CMS_TOPIC_H
#include "topic_mysql.h" 
#include "cms/include/topic_base.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Topic : public topic_mysql<Topic,topic_base>{
		 public:
		 Topic(std::string dbtag);
		 Topic();
		};
	 }
 }
#endif
