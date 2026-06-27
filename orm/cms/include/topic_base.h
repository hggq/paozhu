#ifndef ORM_CMS_TOPICBASEMATA_H
#define ORM_CMS_TOPICBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sat, 27 Jun 2026 11:17:55 GMT
***/
#include <iostream>
#include <charconv>
#include <cstdio>
#include <sstream>
#include <array>
#include <map> 
#include <string_view> 
#include <string> 
#include <vector>
#include <ctime>
#include <array>
#include <concepts>
#include <utility>
#include "unicode.h"

namespace orm { 
   
     namespace cms { 

namespace topic_info
{
 
    enum class cols : unsigned char 
    {
		topicid = 0,
		userid = 1,
		parentid = 2,
		cateid = 3,
		sorttype = 4,
		languagetype = 5,
		isview = 6,
		isside = 7,
		sortid = 8,
		title = 9,
		twotitle = 10,
		memo = 11,
		templatename = 12,
		url = 13,
		urlpath = 14,
		imgurl = 15,
		topimg = 16,
		accesscode = 17,

    };

    struct meta
    {
		 unsigned  int  topicid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  parentid = 0; ///**/
		 unsigned  int  cateid = 0; ///*类型*/
		 unsigned  char  sorttype = 0; ///*内容排序*/
		 unsigned  int  languagetype = 0; ///*语言类型*/
		 unsigned  char  isview = 0; ///*是否显示*/
		 unsigned  char  isside = 1; ///*是否边栏*/
		 unsigned  int  sortid = 0; ///*排序id*/
		 std::string  title = ""; ///*标题*/
		 std::string  twotitle = ""; ///*第二标题*/
		 std::string  memo = ""; ///*备注*/
		 std::string  templatename = ""; ///*模板名称*/
		 std::string  url = ""; ///*网址*/
		 std::string  urlpath = ""; ///*路径*/
		 std::string  imgurl = ""; ///*图片地址*/
		 std::string  topimg = ""; ///*头图*/
		 unsigned  int  accesscode = 0; ///*权限代码*/
	};
  
    struct meta_tree
    {
		 unsigned  int  topicid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  parentid = 0; ///**/
		 unsigned  int  cateid = 0; ///*类型*/
		 unsigned  char  sorttype = 0; ///*内容排序*/
		 unsigned  int  languagetype = 0; ///*语言类型*/
		 unsigned  char  isview = 0; ///*是否显示*/
		 unsigned  char  isside = 1; ///*是否边栏*/
		 unsigned  int  sortid = 0; ///*排序id*/
		 std::string  title = ""; ///*标题*/
		 std::string  twotitle = ""; ///*第二标题*/
		 std::string  memo = ""; ///*备注*/
		 std::string  templatename = ""; ///*模板名称*/
		 std::string  url = ""; ///*网址*/
		 std::string  urlpath = ""; ///*路径*/
		 std::string  imgurl = ""; ///*图片地址*/
		 std::string  topimg = ""; ///*头图*/
		 unsigned  int  accesscode = 0; ///*权限代码*/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  topicid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  parentid = 0; ///**/
		 unsigned  int  cateid = 0; ///*类型*/
		 unsigned  char  sorttype = 0; ///*内容排序*/
		 unsigned  int  languagetype = 0; ///*语言类型*/
		 unsigned  char  isview = 0; ///*是否显示*/
		 unsigned  char  isside = 1; ///*是否边栏*/
		 unsigned  int  sortid = 0; ///*排序id*/
		 std::string  title = ""; ///*标题*/
		 std::string  twotitle = ""; ///*第二标题*/
		 std::string  memo = ""; ///*备注*/
		 std::string  templatename = ""; ///*模板名称*/
		 std::string  url = ""; ///*网址*/
		 std::string  urlpath = ""; ///*路径*/
		 std::string  imgurl = ""; ///*图片地址*/
		 std::string  topimg = ""; ///*头图*/
		 unsigned  int  accesscode = 0; ///*权限代码*/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::topicid) { 
		 return m.topicid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::parentid) { 
		 return m.parentid;
		} else if constexpr (Col == cols::cateid) { 
		 return m.cateid;
		} else if constexpr (Col == cols::sorttype) { 
		 return m.sorttype;
		} else if constexpr (Col == cols::languagetype) { 
		 return m.languagetype;
		} else if constexpr (Col == cols::isview) { 
		 return m.isview;
		} else if constexpr (Col == cols::isside) { 
		 return m.isside;
		} else if constexpr (Col == cols::sortid) { 
		 return m.sortid;
		} else if constexpr (Col == cols::title) { 
		 return m.title;
		} else if constexpr (Col == cols::twotitle) { 
		 return m.twotitle;
		} else if constexpr (Col == cols::memo) { 
		 return m.memo;
		} else if constexpr (Col == cols::templatename) { 
		 return m.templatename;
		} else if constexpr (Col == cols::url) { 
		 return m.url;
		} else if constexpr (Col == cols::urlpath) { 
		 return m.urlpath;
		} else if constexpr (Col == cols::imgurl) { 
		 return m.imgurl;
		} else if constexpr (Col == cols::topimg) { 
		 return m.topimg;
		} else if constexpr (Col == cols::accesscode) { 
		 return m.accesscode;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using topicid =  unsigned  int ;
		using userid =  unsigned  int ;
		using parentid =  unsigned  int ;
		using cateid =  unsigned  int ;
		using sorttype =  unsigned  char ;
		using languagetype =  unsigned  int ;
		using isview =  unsigned  char ;
		using isside =  unsigned  char ;
		using sortid =  unsigned  int ;
		using title =  std::string ;
		using twotitle =  std::string ;
		using memo =  std::string ;
		using templatename =  std::string ;
		using url =  std::string ;
		using urlpath =  std::string ;
		using imgurl =  std::string ;
		using topimg =  std::string ;
		using accesscode =  unsigned  int ;

    }

    
    #define ORM_CMS_TOPIC_EXPAND(x) x 
    
    #define ORM_CMS_TOPIC_META_FIELD_TYPE(col) \
         orm::cms::topic_info::type::col 
    
    #define ORM_CMS_TOPIC_PROJ_MEMBER(col) \
          ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_1(c1) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_1( c1)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c2))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c3))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c4))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c5))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c6))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c7))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c8))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c9))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c10))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c11))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c12))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c13))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c14))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c15))
         
    #define ORM_CMS_TOPIC_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBER(c16))
         
    #define ORM_CMS_TOPIC_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_TOPIC_PROJ_MEMBERS(...) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_16, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_15, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_14, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_13, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_12, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_11, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_10, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_9, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_8, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_7, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_6, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_5, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_4, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_3, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_2, \
            ORM_CMS_TOPIC_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_TOPIC_COUNT(...) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_TOPIC_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_TOPIC_TO_JSON_1(c1) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_TOPIC_TO_JSON_2(c1,c2) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_TOPIC_TO_JSON_BODY(...) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_TOPIC_TO_JSON_16,ORM_CMS_TOPIC_TO_JSON_15,ORM_CMS_TOPIC_TO_JSON_14,ORM_CMS_TOPIC_TO_JSON_13,ORM_CMS_TOPIC_TO_JSON_12,ORM_CMS_TOPIC_TO_JSON_11,ORM_CMS_TOPIC_TO_JSON_10,ORM_CMS_TOPIC_TO_JSON_9,ORM_CMS_TOPIC_TO_JSON_8,ORM_CMS_TOPIC_TO_JSON_7,ORM_CMS_TOPIC_TO_JSON_6,ORM_CMS_TOPIC_TO_JSON_5,ORM_CMS_TOPIC_TO_JSON_4,ORM_CMS_TOPIC_TO_JSON_3,ORM_CMS_TOPIC_TO_JSON_2,ORM_CMS_TOPIC_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_TOPIC_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_TOPIC_TO_JSON_CUSTOM_1(n1)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_1(n1)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_TOPIC_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_TOPIC_CAT(a, b) ORM_CMS_TOPIC_CAT_(a, b)
    #define ORM_CMS_TOPIC_CAT_(a, b) a##b

    #define ORM_CMS_TOPIC_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_TOPIC_CAT(ORM_CMS_TOPIC_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_TOPIC_TO_JSON_CUSTOM(...) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_TOPIC_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_TOPIC_SET_VAL_1(c1) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_TOPIC_SET_VAL_2(c1,c2) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_1(c1)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_2(c1,c2)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_TOPIC_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_TOPIC_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_TOPIC_CAT(ORM_CMS_TOPIC_SET_VAL_, N)
    
    
    #define ORM_CMS_TOPIC_SET_VAL_FIELDS(...) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_TOPIC_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_FIELDS(ORM_CMS_TOPIC_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_TOPIC_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::topic_info { \
            struct StructName { \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_FIELDS(__VA_ARGS__)) \
            } \
            }; \
            std::string to_json(const std::vector<StructName> &vec_){\
            std::ostringstream oss; \
                oss << '['; \
                for(unsigned int i=0; i<vec_.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << vec_[i].to_json(); \
                }\
                oss << ']'; \
                return oss.str(); }\
       }
        
    
    #define ORM_CMS_TOPIC_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::topic_info { \
            struct StructName { \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM(ORM_CMS_TOPIC_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
            } \
            }; \
            std::string to_json(const std::vector<StructName> &vec_){\
            std::ostringstream oss; \
                oss << '['; \
                for(unsigned int i=0; i<vec_.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << vec_[i].to_json(); \
                }\
                oss << ']'; \
                return oss.str(); }\
       }
        
    
    #define ORM_CMS_TOPIC_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::topic_info { \
            struct StructName { \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_BODY(__VA_ARGS__)); \
                oss << ",\"children\":["; \
                for(unsigned int i=0;i< children.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << children[i].to_json(); \
                }\
                oss << ']'; \
                oss << '}'; \
                return oss.str(); \
                }\
                \
                void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_FIELDS(__VA_ARGS__)) \
                } \
            }; \
            std::string to_json(const std::vector<StructName> &vec_){\
            std::ostringstream oss; \
                oss << '['; \
                for(unsigned int i=0; i<vec_.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << vec_[i].to_json(); \
                }\
                oss << ']'; \
                return oss.str(); }\
       }
        
    
    #define ORM_CMS_TOPIC_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::topic_info { \
            struct StructName { \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_BODY(__VA_ARGS__)); \
                oss << ",\"children\":["; \
                for(unsigned int i=0;i< children.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << children[i]->to_json(); \
                }\
                oss << ']'; \
                oss << '}'; \
                return oss.str(); \
                }\
                \
                void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_FIELDS(__VA_ARGS__)) \
                } \
            }; \
            std::string to_json(const std::vector<StructName> &vec_){\
            std::ostringstream oss; \
                oss << '['; \
                for(unsigned int i=0; i<vec_.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << vec_[i].to_json(); \
                }\
                oss << ']'; \
                return oss.str(); }\
       }
        
    
    #define ORM_CMS_TOPIC_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::topic_info { \
            struct StructName { \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_TO_JSON_CUSTOM(ORM_CMS_TOPIC_UNWRAP CustomNames));  \
                oss << ",\"children\":["; \
                for(unsigned int i=0;i< children.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << children[i]->to_json(); \
                }\
                oss << ']'; \
                oss << '}'; \
                return oss.str(); \
                }\
                \
                void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_TOPIC_EXPAND(ORM_CMS_TOPIC_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
                } \
            }; \
            std::string to_json(const std::vector<StructName> &vec_){\
            std::ostringstream oss; \
                oss << '['; \
                for(unsigned int i=0; i<vec_.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << vec_[i].to_json(); \
                }\
                oss << ']'; \
                return oss.str(); }\
       }
        
    static constexpr std::array<std::string_view,18> col_names={"topicid","userid","parentid","cateid","sorttype","languagetype","isview","isside","sortid","title","twotitle","memo","templatename","url","urlpath","imgurl","topimg","accesscode"};
	static constexpr std::array<unsigned char,18> col_types={3,3,3,3,1,3,1,1,3,253,253,253,253,253,253,253,252,3};
	static constexpr std::array<unsigned char,18> col_length={0,0,0,0,0,0,0,0,0,120,120,254,254,255,60,254,0,0};
	static constexpr std::array<unsigned char,18> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct topic_base
{
      topic_info::meta data;
    std::vector<topic_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<topic_info::meta>::iterator begin(){     return record.begin(); }
std::vector<topic_info::meta>::iterator end(){     return record.end(); }
std::vector<topic_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<topic_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="topic";
static constexpr std::string_view org_tablename="topic";
static constexpr std::string_view modelname="Topic";

	  unsigned char findcolpos(const std::string &coln){
            if(coln.size()==0)
            {
                return 255;
            }
		    unsigned char  bi=coln[0];
         char colpospppc;

	         if(bi<91&&bi>64){
				bi+=32;
			}
            switch(coln[0]){


         case 'a':
   	 return 17;
break;
case 'c':
   	 return 3;
break;
case 'i':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 7; }
 if(colpospppc=='l'){ return 15; }
 if(colpospppc=='w'){ return 6; }
   	 break;
 }
 break;
case 'l':
   	 return 5;
break;
case 'm':
   	 return 11;
break;
case 'p':
   	 return 2;
break;
case 's':
 switch(coln.size()){  
case 6:
   	 return 8;
break;
case 8:
   	 return 4;
break;
 }
 break;
case 't':
 switch(coln.size()){  
case 5:
   	 return 9;
break;
case 6:
   	 return 16;
break;
case 7:
   	 return 0;
break;
case 8:
   	 return 10;
break;
case 12:
   	 return 12;
break;
 }
 break;
case 'u':
 switch(coln.size()){  
case 3:
   	 return 13;
break;
case 6:
   	 return 1;
break;
case 7:
   	 return 14;
break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "topicid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     topic_info::meta metatemp;    
            data = metatemp; 
      }
      
      std::string soft_remove_sql([[maybe_unused]] const std::string &fieldsql){
          std::string temp;
     
         return temp;
     }
     

  inline  std::string stringaddslash(const std::string &content){
        std::string temp;
        for(unsigned int i=0;i<content.size();i++){
            if(content[i]=='\''){
                temp.append("\\'");
                continue;
            }else if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }else if(content[i]=='\\'){
                temp.append("\\\\");
                continue;
            }
            temp.push_back(content[i]);
        }
        return temp;
   }  
  inline  std::string jsonaddslash(const std::string &content){
        std::string temp;
        for(unsigned int i=0;i<content.size();i++){
            if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }
            else if(content[i]=='\\'){
                temp.append("\\\\");
                continue;
            }
            temp.push_back(content[i]);
        }
        return temp;
   }  

   std::string make_data_insert_sql(){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<topic_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<topic_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.topicid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.topicid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.parentid);
}
if(data.cateid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.cateid);
}
if(data.sorttype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sorttype);
}
if(data.languagetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.languagetype);
}
if(data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isview);
}
if(data.isside==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isside);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
tempsql<<",'"<<stringaddslash(data.title)<<"'";
tempsql<<",'"<<stringaddslash(data.twotitle)<<"'";
tempsql<<",'"<<stringaddslash(data.memo)<<"'";
tempsql<<",'"<<stringaddslash(data.templatename)<<"'";
tempsql<<",'"<<stringaddslash(data.url)<<"'";
tempsql<<",'"<<stringaddslash(data.urlpath)<<"'";
tempsql<<",'"<<stringaddslash(data.imgurl)<<"'";
tempsql<<",'"<<stringaddslash(data.topimg)<<"'";
if(data.accesscode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.accesscode);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string make_data_insert_sql(const topic_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<topic_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<topic_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.topicid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.topicid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.parentid);
}
if(insert_data.cateid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.cateid);
}
if(insert_data.sorttype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sorttype);
}
if(insert_data.languagetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.languagetype);
}
if(insert_data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isview);
}
if(insert_data.isside==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isside);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.twotitle)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.memo)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.templatename)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.url)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.urlpath)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.imgurl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.topimg)<<"'";
if(insert_data.accesscode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.accesscode);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string make_vector_insert_sql(const std::vector<topic_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<topic_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<topic_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES ";

        for(unsigned int i=0;i<insert_data.size();i++)
        {
            if(i>0)
            {
                tempsql<<",";	
            }
            tempsql<<"(";


            	if(insert_data[i].topicid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].topicid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].parentid);
	}
	if(insert_data[i].cateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].cateid);
	}
	if(insert_data[i].sorttype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sorttype);
	}
	if(insert_data[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].languagetype);
	}
	if(insert_data[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isview);
	}
	if(insert_data[i].isside==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isside);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].twotitle)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].memo)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].templatename)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].url)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].urlpath)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].imgurl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].topimg)<<"'";
	if(insert_data[i].accesscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].accesscode);
	}
		tempsql<<")";
	 } 

     
       return tempsql.str();
   } 
       
    std::string make_update_sql(std::string_view fileld){
        std::ostringstream tempsql;
        tempsql<<"UPDATE ";
        tempsql<<tablename;
        tempsql<<" SET ";

        bool isall=false;
        if(fileld.empty()){
            isall=true;
        }
        if(isall){

        if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.parentid==0){
	tempsql<<",`parentid`=0";
 }else{ 
	tempsql<<",`parentid`="<<std::to_string(data.parentid);
}
if(data.cateid==0){
	tempsql<<",`cateid`=0";
 }else{ 
	tempsql<<",`cateid`="<<std::to_string(data.cateid);
}
if(data.sorttype==0){
	tempsql<<",`sorttype`=0";
 }else{ 
	tempsql<<",`sorttype`="<<std::to_string(data.sorttype);
}
if(data.languagetype==0){
	tempsql<<",`languagetype`=0";
 }else{ 
	tempsql<<",`languagetype`="<<std::to_string(data.languagetype);
}
if(data.isview==0){
	tempsql<<",`isview`=0";
 }else{ 
	tempsql<<",`isview`="<<std::to_string(data.isview);
}
if(data.isside==0){
	tempsql<<",`isside`=0";
 }else{ 
	tempsql<<",`isside`="<<std::to_string(data.isside);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
tempsql<<",`twotitle`='"<<stringaddslash(data.twotitle)<<"'";
tempsql<<",`memo`='"<<stringaddslash(data.memo)<<"'";
tempsql<<",`templatename`='"<<stringaddslash(data.templatename)<<"'";
tempsql<<",`url`='"<<stringaddslash(data.url)<<"'";
tempsql<<",`urlpath`='"<<stringaddslash(data.urlpath)<<"'";
tempsql<<",`imgurl`='"<<stringaddslash(data.imgurl)<<"'";
tempsql<<",`topimg`='"<<stringaddslash(data.topimg)<<"'";
if(data.accesscode==0){
	tempsql<<",`accesscode`=0";
 }else{ 
	tempsql<<",`accesscode`="<<std::to_string(data.accesscode);
}
 }else{ 

     
  unsigned int jj=0;
                  std::string keyname;
                  std::vector<unsigned char> keypos;
                  for(;jj<fileld.size();jj++){
                        if(fileld[jj]==','){
                                unsigned char bpos_i=findcolpos(keyname);
                               keypos.emplace_back(bpos_i); 
#ifdef DEBUG
                    if (bpos_i == 255)
                    {
                        std::cout << "\033[1m\033[31m-----------\n"
                                  << keyname << " not in " << tablename << " table Field.\n-----------\033[0m"
                                  << std::endl;
                    }
#endif                               
                               keyname.clear();
                             continue;   
                        }
                        if(fileld[jj]==0x20){

                             continue;   
                        }
                        keyname.push_back(fileld[jj]);

                  }  
                 if(keyname.size()>0){
                                unsigned char bpos_i=findcolpos(keyname);
 #ifdef DEBUG
                    if (bpos_i == 255)
                    {
                        std::cout << "\033[1m\033[31m-----------\n"
                                  << keyname << " not in " << tablename << " table Field.\n-----------\033[0m"
                                  << std::endl;
                    }
#endif                                       
                                keypos.emplace_back(bpos_i); 
                                keyname.clear();
                 }
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){

         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"`userid`=0";
 }else{ 
	tempsql<<"`userid`="<<std::to_string(data.userid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.parentid==0){
	tempsql<<"`parentid`=0";
 }else{ 
	tempsql<<"`parentid`="<<std::to_string(data.parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.cateid==0){
	tempsql<<"`cateid`=0";
 }else{ 
	tempsql<<"`cateid`="<<std::to_string(data.cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.sorttype==0){
	tempsql<<"`sorttype`=0";
 }else{ 
	tempsql<<"`sorttype`="<<std::to_string(data.sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.languagetype==0){
	tempsql<<"`languagetype`=0";
 }else{ 
	tempsql<<"`languagetype`="<<std::to_string(data.languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isview==0){
	tempsql<<"`isview`=0";
 }else{ 
	tempsql<<"`isview`="<<std::to_string(data.isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.isside==0){
	tempsql<<"`isside`=0";
 }else{ 
	tempsql<<"`isside`="<<std::to_string(data.isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`twotitle`='"<<stringaddslash(data.twotitle)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`memo`='"<<stringaddslash(data.memo)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`templatename`='"<<stringaddslash(data.templatename)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`url`='"<<stringaddslash(data.url)<<"'";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`urlpath`='"<<stringaddslash(data.urlpath)<<"'";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`imgurl`='"<<stringaddslash(data.imgurl)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`topimg`='"<<stringaddslash(data.topimg)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(data.accesscode==0){
	tempsql<<"`accesscode`=0";
 }else{ 
	tempsql<<"`accesscode`="<<std::to_string(data.accesscode);
}
 break;

     
                  default:
                                ;
                     }
                 }   

            }        

        return tempsql.str();
   } 
   
    std::string make_record_replace_sql()
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
        tempsql << "REPLACE INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < topic_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << topic_info::col_names[j];
        }
        if (j > 0)
        {
            tempsql << "`";
        }
        tempsql << ") VALUES ";

        for (unsigned int i = 0; i < record.size(); i++)
        {
            if (i > 0)
            {
                tempsql << ",\n";
            }
            tempsql << "(";
            	if(record[i].topicid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].topicid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
	}
	if(record[i].cateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].cateid);
	}
	if(record[i].sorttype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sorttype);
	}
	if(record[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languagetype);
	}
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].isside==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isside);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].twotitle)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].memo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].templatename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].url)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].urlpath)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].imgurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].topimg)<<"'";
	if(record[i].accesscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].accesscode);
	}
	tempsql<<")";

 }
 return tempsql.str();
}

    std::string make_record_into_sql(std::string_view fileld)
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
        tempsql << "INSERT INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < topic_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << topic_info::col_names[j];
        }
        if (j > 0)
        {
            tempsql << "`";
        }
        tempsql << ") VALUES ";

        for (unsigned int i = 0; i < record.size(); i++)
        {
            if (i > 0)
            {
                tempsql << ",\n";
            }
            tempsql << "(";
            	if(record[i].topicid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].topicid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
	}
	if(record[i].cateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].cateid);
	}
	if(record[i].sorttype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sorttype);
	}
	if(record[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languagetype);
	}
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].isside==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isside);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].twotitle)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].memo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].templatename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].url)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].urlpath)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].imgurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].topimg)<<"'";
	if(record[i].accesscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].accesscode);
	}
	tempsql<<")";
	 }
	 tempsql<<" as new ON DUPLICATE KEY UPDATE ";

     
        std::string keyname;
        unsigned char jj=0;
        j=0;
        if(fileld.size()>0){
        for(;jj<fileld.size();jj++){
            if(fileld[jj]==','){
                if(findcolpos(keyname)<255)
                {
                    if(j>0)
                    {
                        tempsql<<",";
                    }
                    tempsql<<keyname;
                    tempsql<<"=new.";
                    tempsql<<keyname;
                }
                continue;   
            }
            if(fileld[jj]==0x20){
                continue;   
            }
            keyname.push_back(fileld[jj]);

        }  
        if(keyname.size()>0){
            if(findcolpos(keyname)<255)
            {
                if(j>0)
                {
                    tempsql<<",";
                }
                tempsql<<keyname;
                tempsql<<"=new.";
                tempsql<<keyname;
                
            }
        }

    } 
 
 return tempsql.str();
}

   std::vector<std::string> data_toarray(std::string_view fileld=""){
        std::vector<std::string> temparray;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>1){
            for(;jj<fileld.size();jj++){
                if(fileld[jj]==','){
                    keypos.emplace_back(findcolpos(keyname)); 
                    keyname.clear();
                    continue;   
                }
                if(fileld[jj]==0x20){

                    continue;   
                }
                keyname.push_back(fileld[jj]);

            }  
            if(keyname.size()>0){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
            }
        }else{
            for(jj=0;jj<topic_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.topicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicid));
}
 break;
 case 1:
if(data.userid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.userid));
}
 break;
 case 2:
if(data.parentid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.parentid));
}
 break;
 case 3:
if(data.cateid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.cateid));
}
 break;
 case 4:
if(data.sorttype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sorttype));
}
 break;
 case 5:
if(data.languagetype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.languagetype));
}
 break;
 case 6:
if(data.isview==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isview));
}
 break;
 case 7:
if(data.isside==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isside));
}
 break;
 case 8:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 9:
	temparray.push_back(data.title);
 break;
 case 10:
	temparray.push_back(data.twotitle);
 break;
 case 11:
	temparray.push_back(data.memo);
 break;
 case 12:
	temparray.push_back(data.templatename);
 break;
 case 13:
	temparray.push_back(data.url);
 break;
 case 14:
	temparray.push_back(data.urlpath);
 break;
 case 15:
	temparray.push_back(data.imgurl);
 break;
 case 16:
	temparray.push_back(data.topimg);
 break;
 case 17:
if(data.accesscode==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.accesscode));
}
 break;

                             default:
                                ;
                     }
                 }   
   
     return temparray;             
   }   
   
   std::map<std::string,std::string> data_tomap(std::string_view fileld=""){
       std::map<std::string,std::string> tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>1){
        for(;jj<fileld.size();jj++){
            if(fileld[jj]==','){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
                continue;   
            }
            if(fileld[jj]==0x20){

                continue;   
            }
            keyname.push_back(fileld[jj]);

        }  
        if(keyname.size()>0){
            keypos.emplace_back(findcolpos(keyname)); 
            keyname.clear();
        }
        }else{
            for(jj=0;jj<topic_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.topicid==0){
	tempsql.insert({"topicid","0"});
 }else{ 
	tempsql.insert({"topicid",std::to_string(data.topicid)});
}
 break;
 case 1:
if(data.userid==0){
	tempsql.insert({"userid","0"});
 }else{ 
	tempsql.insert({"userid",std::to_string(data.userid)});
}
 break;
 case 2:
if(data.parentid==0){
	tempsql.insert({"parentid","0"});
 }else{ 
	tempsql.insert({"parentid",std::to_string(data.parentid)});
}
 break;
 case 3:
if(data.cateid==0){
	tempsql.insert({"cateid","0"});
 }else{ 
	tempsql.insert({"cateid",std::to_string(data.cateid)});
}
 break;
 case 4:
if(data.sorttype==0){
	tempsql.insert({"sorttype","0"});
 }else{ 
	tempsql.insert({"sorttype",std::to_string(data.sorttype)});
}
 break;
 case 5:
if(data.languagetype==0){
	tempsql.insert({"languagetype","0"});
 }else{ 
	tempsql.insert({"languagetype",std::to_string(data.languagetype)});
}
 break;
 case 6:
if(data.isview==0){
	tempsql.insert({"isview","0"});
 }else{ 
	tempsql.insert({"isview",std::to_string(data.isview)});
}
 break;
 case 7:
if(data.isside==0){
	tempsql.insert({"isside","0"});
 }else{ 
	tempsql.insert({"isside",std::to_string(data.isside)});
}
 break;
 case 8:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 9:
	tempsql.insert({"title",data.title});
 break;
 case 10:
	tempsql.insert({"twotitle",data.twotitle});
 break;
 case 11:
	tempsql.insert({"memo",data.memo});
 break;
 case 12:
	tempsql.insert({"templatename",data.templatename});
 break;
 case 13:
	tempsql.insert({"url",data.url});
 break;
 case 14:
	tempsql.insert({"urlpath",data.urlpath});
 break;
 case 15:
	tempsql.insert({"imgurl",data.imgurl});
 break;
 case 16:
	tempsql.insert({"topimg",data.topimg});
 break;
 case 17:
if(data.accesscode==0){
	tempsql.insert({"accesscode","0"});
 }else{ 
	tempsql.insert({"accesscode",std::to_string(data.accesscode)});
}
 break;

                             default:
                                ;
                     }
                 }   
    
     return tempsql;             
   }   
   
   std::string data_tojson(){
       std::ostringstream tempsql;

        tempsql<<"{";
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.parentid==0){
	tempsql<<",\"parentid\":0";
 }else{ 
	tempsql<<",\"parentid\":"<<std::to_string(data.parentid);
}
if(data.cateid==0){
	tempsql<<",\"cateid\":0";
 }else{ 
	tempsql<<",\"cateid\":"<<std::to_string(data.cateid);
}
if(data.sorttype==0){
	tempsql<<",\"sorttype\":0";
 }else{ 
	tempsql<<",\"sorttype\":"<<std::to_string(data.sorttype);
}
if(data.languagetype==0){
	tempsql<<",\"languagetype\":0";
 }else{ 
	tempsql<<",\"languagetype\":"<<std::to_string(data.languagetype);
}
if(data.isview==0){
	tempsql<<",\"isview\":0";
 }else{ 
	tempsql<<",\"isview\":"<<std::to_string(data.isview);
}
if(data.isside==0){
	tempsql<<",\"isside\":0";
 }else{ 
	tempsql<<",\"isside\":"<<std::to_string(data.isside);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"twotitle\":\""<<http::utf8_to_jsonstring(data.twotitle);
tempsql<<"\"";
tempsql<<",\"memo\":\""<<http::utf8_to_jsonstring(data.memo);
tempsql<<"\"";
tempsql<<",\"templatename\":\""<<http::utf8_to_jsonstring(data.templatename);
tempsql<<"\"";
tempsql<<",\"url\":\""<<http::utf8_to_jsonstring(data.url);
tempsql<<"\"";
tempsql<<",\"urlpath\":\""<<http::utf8_to_jsonstring(data.urlpath);
tempsql<<"\"";
tempsql<<",\"imgurl\":\""<<http::utf8_to_jsonstring(data.imgurl);
tempsql<<"\"";
tempsql<<",\"topimg\":\""<<http::utf8_to_jsonstring(data.topimg);
tempsql<<"\"";
if(data.accesscode==0){
	tempsql<<",\"accesscode\":0";
 }else{ 
	tempsql<<",\"accesscode\":"<<std::to_string(data.accesscode);
}
tempsql<<"}";

     
     return tempsql.str();             
   }   
   
   std::string data_tojson(std::string fileld){
        std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>0){
        for(;jj<fileld.size();jj++){
            if(fileld[jj]==','){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
                continue;   
            }
            if(fileld[jj]==0x20){

                continue;   
            }
            keyname.push_back(fileld[jj]);

        }  
        if(keyname.size()>0){
            keypos.emplace_back(findcolpos(keyname)); 
            keyname.clear();
        }
        }else{
            for(jj=0;jj<topic_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(data.userid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(data.parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(data.cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(data.sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(data.languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(data.isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(data.isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(data.twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(data.memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(data.templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(data.url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(data.urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(data.imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(data.topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(data.accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(data.accesscode);
}
 break;

                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
     return tempsql.str();             
   }   
   
    void from_json(const std::string &json_content)
   {
        record.clear();
        topic_info::meta metatemp; 
        data = metatemp;
        unsigned int json_offset=0;
        bool isarray=false;
        for(;json_offset<json_content.size();json_offset++)
        {
            if(json_content[json_offset]=='{')
            {
                break;
            }
            if(json_content[json_offset]=='[')
            {
                isarray=true;
                break;
            }
        }
        if(isarray)
        {
            json_offset+=1; 
            std::string json_key_name,json_value_name; 
            for(;json_offset<json_content.size();json_offset++)
            {
                if(json_content[json_offset]!='{')
                {
                    continue;
                }
                if(record.size()>0)
                {
                    data = metatemp;
                }
                json_offset++;
                if(json_offset >= json_content.size())
                {
                    break;
                }
                for(;json_offset<json_content.size();json_offset++)
                {
    
                            if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                            {
                                continue;
                            }
                            else
                            {
                                if(json_content[json_offset]==0x22)
                                {
                                    unsigned int temp_offset=json_offset;
                                    json_key_name=http::jsonstring_to_utf8(&json_content[json_offset],json_content.size()-json_offset,temp_offset);
                                    json_offset=temp_offset;
                                    if(json_content[json_offset]==0x22)
                                    {
                                        json_offset+=1;
                                    }
                                    for(;json_offset<json_content.size();json_offset++)
                                    {
                                    
                                        if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                        {
                                            continue;
                                        }
                                        break;
                                    }       
                                    if(json_offset < json_content.size() && json_content[json_offset]!=':')
                                    {
                                        break;
                                    }
                                    json_offset+=1;
                                    for(;json_offset<json_content.size();json_offset++)
                                    {
                                        if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                        {
                                            continue;
                                        }
                                        break;
                                    } 
                                    
                                    if(json_offset>=json_content.size())
                                    {
                                        break;
                                    }
                                    json_value_name.clear();
                                    if(json_content[json_offset]==0x22)
                                    {
                                        
                                        temp_offset=json_offset;
                                        json_value_name=http::jsonstring_to_utf8(&json_content[json_offset],json_content.size()-json_offset,temp_offset);
                                        json_offset=temp_offset;
                                        if(json_content[json_offset]==0x22)
                                        {
                                            json_offset+=1;
                                        }
                                    }
                                    else
                                    {
                                        if(json_content[json_offset]!='{'&&json_content[json_offset]!=']')
                                        {
                                            for(;json_offset<json_content.size();json_offset++)
                                            {
                                                if(json_content[json_offset]==0x5D||json_content[json_offset]==0x7D||json_content[json_offset]==0x22||json_content[json_offset]==0x2C||json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                                {
                                                    if(json_content[json_offset]==0x7D)
                                                    {
                                                        json_offset-=1;
                                                    } 
                                                    break;
                                                }
                                                json_value_name.push_back(json_content[json_offset]);
                                            }   
                                        }
                                    }
                                    //////////////////////////
                                    set_val(json_key_name,json_value_name);
                                    continue;
                                }
                                else
                                {
                                    break;
                                }
                            }
    
                }
                record.emplace_back(data);

            }
        }
        else
        {
           if(json_content[json_offset]=='{')
            {
                json_offset+=1; 
                std::string json_key_name,json_value_name; 
                 
                for(;json_offset<json_content.size();json_offset++)
                {
                        if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                        {
                            continue;
                        }
                        else
                        {
                            if(json_content[json_offset]==0x22)
                            {
                                 unsigned int temp_offset=json_offset;
                                 json_key_name=http::jsonstring_to_utf8(&json_content[json_offset],json_content.size()-json_offset,temp_offset);
                                 json_offset=temp_offset;
                                 if(json_content[json_offset]==0x22)
                                 {
                                    json_offset+=1;
                                 }
                                for(;json_offset<json_content.size();json_offset++)
                                {
                                    if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                    {
                                        continue;
                                    }
                                    break;
                                }       
                                if(json_content[json_offset]!=':')
                                {
                                    break;
                                }
                                json_offset+=1;
                                for(;json_offset<json_content.size();json_offset++)
                                {
                                    if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                    {
                                        continue;
                                    }
                                    break;
                                } 
                                
                                if(json_offset >= json_content.size())
                                {
                                    break;
                                }
                                json_value_name.clear();
                                if(json_content[json_offset]==0x22)
                                {
                                    temp_offset=json_offset;
                                    json_value_name=http::jsonstring_to_utf8(&json_content[json_offset],json_content.size()-json_offset,temp_offset);
                                    json_offset=temp_offset;
                                    if(json_content[json_offset]==0x22)
                                    {
                                        json_offset+=1;
                                    }
                                }
                                else
                                {
                                    if(json_content[json_offset]!='{'&&json_content[json_offset]!=']')
                                    {
                                        for(;json_offset<json_content.size();json_offset++)
                                        {
                                            if(json_content[json_offset]==0x5D||json_content[json_offset]==0x7D||json_content[json_offset]==0x22||json_content[json_offset]==0x2C||json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                            {
                                               if(json_content[json_offset]==0x7D)
                                               {
                                                   json_offset-=1;
                                               } 
                                               break;
                                            }
                                            json_value_name.push_back(json_content[json_offset]);
                                        }   
                                    }
                                }
                                //////////////////////////
                                set_val(json_key_name,json_value_name);
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }
 
                }
                if(isarray)
                {
                    record.emplace_back(data);
                }
            }
        }
    }
    
    void set_val(const std::string& set_key_name,const std::string& set_value_name)
    {
        switch(findcolpos(set_key_name))
        {
    
		case 0:
		  http::json_set_val(data.topicid,set_value_name);
		 break;
		
		case 1:
		  http::json_set_val(data.userid,set_value_name);
		 break;
		
		case 2:
		  http::json_set_val(data.parentid,set_value_name);
		 break;
		
		case 3:
		  http::json_set_val(data.cateid,set_value_name);
		 break;
		
		case 4:
		  http::json_set_val(data.sorttype,set_value_name);
		 break;
		
		case 5:
		  http::json_set_val(data.languagetype,set_value_name);
		 break;
		
		case 6:
		  http::json_set_val(data.isview,set_value_name);
		 break;
		
		case 7:
		  http::json_set_val(data.isside,set_value_name);
		 break;
		
		case 8:
		  http::json_set_val(data.sortid,set_value_name);
		 break;
		
		case 9:
		  http::json_set_val(data.title,set_value_name);
		 break;
		
		case 10:
		  http::json_set_val(data.twotitle,set_value_name);
		 break;
		
		case 11:
		  http::json_set_val(data.memo,set_value_name);
		 break;
		
		case 12:
		  http::json_set_val(data.templatename,set_value_name);
		 break;
		
		case 13:
		  http::json_set_val(data.url,set_value_name);
		 break;
		
		case 14:
		  http::json_set_val(data.urlpath,set_value_name);
		 break;
		
		case 15:
		  http::json_set_val(data.imgurl,set_value_name);
		 break;
		
		case 16:
		  http::json_set_val(data.topimg,set_value_name);
		 break;
		
		case 17:
		  http::json_set_val(data.accesscode,set_value_name);
		 break;
		
		default:
		 { }
			


        }
   } 
    
   std::string to_json(std::string_view fileld=""){
    std::ostringstream tempsql;
    std::string keyname;
    unsigned char jj=0;
    std::vector<unsigned char> keypos;
    if(fileld.size()>0){
        for(;jj<fileld.size();jj++){
            if(fileld[jj]==','){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
                continue;   
            }
            if(fileld[jj]==0x20){

                continue;   
            }
            keyname.push_back(fileld[jj]);

        }  
        if(keyname.size()>0){
            keypos.emplace_back(findcolpos(keyname)); 
            keyname.clear();
        }
    }else{
        for(jj=0;jj<topic_info::col_names.size();jj++){
            keypos.emplace_back(jj); 
        }
    }
    tempsql<<"[";
    for(size_t n=0;n<record.size();n++){
        if(n>0){
            tempsql<<",{";
        }else{
            tempsql<<"{";
        }  
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(record[n].cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(record[n].sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(record[n].languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(record[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(record[n].isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(record[n].twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(record[n].memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(record[n].templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(record[n].url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(record[n].urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(record[n].imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(record[n].topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(record[n].accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(record[n].accesscode);
}
 break;

                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   
   std::string to_json(std::function<bool(std::string&,topic_info::meta&)> func,std::string_view fileld=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>0){
            for(;jj<fileld.size();jj++){
                if(fileld[jj]==','){
                    keypos.emplace_back(findcolpos(keyname)); 
                    keyname.clear();
                    continue;   
                }
                if(fileld[jj]==0x20){

                    continue;   
                }
                keyname.push_back(fileld[jj]);

            }  
            if(keyname.size()>0){
                keypos.emplace_back(findcolpos(keyname)); 
                keyname.clear();
            }
        }else{
            for(jj=0;jj<topic_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"[";
        for(size_t n=0;n<record.size();n++){
            keyname.clear();
            if(func(keyname,record[n])){ 
                if(n>0){
                    tempsql<<",{";
                }else{
                    tempsql<<"{";
                } 
                tempsql<<keyname;
            }else{
            continue;
            } 
        
        for(jj=0;jj<keypos.size();jj++){
            
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(record[n].cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(record[n].sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(record[n].languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(record[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(record[n].isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(record[n].twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(record[n].memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(record[n].templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(record[n].url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(record[n].urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(record[n].imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(record[n].topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(record[n].accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(record[n].accesscode);
}
 break;

                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   long long getPK(){  return data.topicid; } 
 void setPK(long long val){  data.topicid=val;} 
 unsigned  int  getTopicid(){  return data.topicid; } 
 void setTopicid( unsigned  int  val){  data.topicid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getParentid(){  return data.parentid; } 
 void setParentid( unsigned  int  val){  data.parentid=val;} 

 unsigned  int  getCateid(){  return data.cateid; } 
 void setCateid( unsigned  int  val){  data.cateid=val;} 

 unsigned  char  getSorttype(){  return data.sorttype; } 
 void setSorttype( unsigned  char  val){  data.sorttype=val;} 

 unsigned  int  getLanguagetype(){  return data.languagetype; } 
 void setLanguagetype( unsigned  int  val){  data.languagetype=val;} 

 unsigned  char  getIsview(){  return data.isview; } 
 void setIsview( unsigned  char  val){  data.isview=val;} 

 unsigned  char  getIsside(){  return data.isside; } 
 void setIsside( unsigned  char  val){  data.isside=val;} 

 unsigned  int  getSortid(){  return data.sortid; } 
 void setSortid( unsigned  int  val){  data.sortid=val;} 

 std::string  getTitle(){  return data.title; } 
 std::string & getRefTitle(){  return std::ref(data.title); } 
 void setTitle( std::string  &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

 std::string  getTwotitle(){  return data.twotitle; } 
 std::string & getRefTwotitle(){  return std::ref(data.twotitle); } 
 void setTwotitle( std::string  &val){  data.twotitle=val;} 
 void setTwotitle(std::string_view val){  data.twotitle=val;} 

 std::string  getMemo(){  return data.memo; } 
 std::string & getRefMemo(){  return std::ref(data.memo); } 
 void setMemo( std::string  &val){  data.memo=val;} 
 void setMemo(std::string_view val){  data.memo=val;} 

 std::string  getTemplatename(){  return data.templatename; } 
 std::string & getRefTemplatename(){  return std::ref(data.templatename); } 
 void setTemplatename( std::string  &val){  data.templatename=val;} 
 void setTemplatename(std::string_view val){  data.templatename=val;} 

 std::string  getUrl(){  return data.url; } 
 std::string & getRefUrl(){  return std::ref(data.url); } 
 void setUrl( std::string  &val){  data.url=val;} 
 void setUrl(std::string_view val){  data.url=val;} 

 std::string  getUrlpath(){  return data.urlpath; } 
 std::string & getRefUrlpath(){  return std::ref(data.urlpath); } 
 void setUrlpath( std::string  &val){  data.urlpath=val;} 
 void setUrlpath(std::string_view val){  data.urlpath=val;} 

 std::string  getImgurl(){  return data.imgurl; } 
 std::string & getRefImgurl(){  return std::ref(data.imgurl); } 
 void setImgurl( std::string  &val){  data.imgurl=val;} 
 void setImgurl(std::string_view val){  data.imgurl=val;} 

 std::string  getTopimg(){  return data.topimg; } 
 std::string & getRefTopimg(){  return std::ref(data.topimg); } 
 void setTopimg( std::string  &val){  data.topimg=val;} 
 void setTopimg(std::string_view val){  data.topimg=val;} 

 unsigned  int  getAccesscode(){  return data.accesscode; } 
 void setAccesscode( unsigned  int  val){  data.accesscode=val;} 

topic_info::meta getnewData(){
 	 struct topic_info::meta newdata;
	 return newdata; 
} 
topic_info::meta getData(){
 	 return data; 
} 
std::vector<topic_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<topic_info::meta_tree> &tree_data, std::string_view fileld=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>0){
            for(;jj<fileld.size();jj++){
                if(fileld[jj]==','){
                    keypos.emplace_back(findcolpos(keyname)); 
                    keyname.clear();
                    continue;   
                }
                if(fileld[jj]==0x20){

                    continue;   
                }
                keyname.push_back(fileld[jj]);

            }  
            if(keyname.size()>0){
                            keypos.emplace_back(findcolpos(keyname)); 
                            keyname.clear();
            }
        }else{
            for(jj=0;jj<topic_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"[";
        for(size_t n=0;n<tree_data.size();n++){
            if(n>0){
                tempsql<<",{";
            }else{
                tempsql<<"{";
            }  
        
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(tree_data[n].userid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(tree_data[n].cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(tree_data[n].sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(tree_data[n].languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(tree_data[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(tree_data[n].isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(tree_data[n].twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(tree_data[n].memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(tree_data[n].templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(tree_data[n].url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(tree_data[n].urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(tree_data[n].imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(tree_data[n].topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(tree_data[n].accesscode);
}
 break;

                             default:
                                ;
                     }
                 }

        tempsql<<",\"children\":";
         tempsql<<tree_tojson(tree_data[n].children, fileld);     
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   
   std::string tree_tojson(const std::vector<topic_info::meta_tree> &tree_data,std::function<bool(std::string&,const topic_info::meta_tree&)> func,std::string_view fileld=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(fileld.size()>0){
            for(;jj<fileld.size();jj++){
                if(fileld[jj]==','){
                    keypos.emplace_back(findcolpos(keyname)); 
                    keyname.clear();
                    continue;   
                }
                if(fileld[jj]==0x20){

                    continue;   
                }
                keyname.push_back(fileld[jj]);

            }  
            if(keyname.size()>0){
                            keypos.emplace_back(findcolpos(keyname)); 
                            keyname.clear();
            }
        }else{
            for(jj=0;jj<topic_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    tempsql<<"[";
    for(size_t n=0;n<tree_data.size();n++){
        keyname.clear();
        if(func(keyname,tree_data[n])){ 
                if(n>0){
                    tempsql<<",{";
                }else{
                    tempsql<<"{";
                } 
                tempsql<<keyname;
        }else{
        continue;
        } 
        
        for(jj=0;jj<keypos.size();jj++){
            
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(tree_data[n].userid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(tree_data[n].cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(tree_data[n].sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(tree_data[n].languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(tree_data[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(tree_data[n].isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(tree_data[n].twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(tree_data[n].memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(tree_data[n].templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(tree_data[n].url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(tree_data[n].urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(tree_data[n].imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(tree_data[n].topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(tree_data[n].accesscode);
}
 break;

                             default:
                                ;
                     }
                 }   
         tempsql<<",\"children\":";
         tempsql<<tree_tojson(tree_data[n].children,func,fileld);     
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   
    template<topic_info::cols KeyCol, topic_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>()));
        using ValType = decltype(topic_info::getField<ValCol>(std::declval<const topic_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(topic_info::getField<KeyCol>(iter), topic_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<topic_info::cols KeyCol, topic_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>())), 
            decltype(topic_info::getField<ValCol>(std::declval<const topic_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>())), 
                    decltype(topic_info::getField<ValCol>(std::declval<const topic_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>()));
        using ValType = decltype(topic_info::getField<ValCol>(std::declval<const topic_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(topic_info::getField<KeyCol>(iter), topic_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(topic_info::getField<KeyCol>(iter), topic_info::getField<ValCol>(iter))) {
                    result.emplace(topic_info::getField<KeyCol>(iter), topic_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<topic_info::cols KeyCol, topic_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>()));
        using ValType = decltype(topic_info::getField<ValCol>(std::declval<const topic_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(topic_info::getField<KeyCol>(iter), topic_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<topic_info::cols KeyCol, topic_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>())), 
            decltype(topic_info::getField<ValCol>(std::declval<const topic_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>())), 
                    decltype(topic_info::getField<ValCol>(std::declval<const topic_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>()));
        using ValType = decltype(topic_info::getField<ValCol>(std::declval<const topic_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(topic_info::getField<KeyCol>(iter), topic_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(topic_info::getField<KeyCol>(iter), topic_info::getField<ValCol>(iter))) {
                    result.emplace_back(topic_info::getField<KeyCol>(iter), topic_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<topic_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(topic_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<topic_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(topic_info::getField<KeyCol>(std::declval<const topic_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(topic_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(topic_info::getField<KeyCol>(iter))) {
                    result.emplace_back(topic_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<topic_info::cols Col>
        requires requires(std::ostream& os, decltype(topic_info::getField<Col>(std::declval<const topic_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << topic_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<topic_info::cols Col>
        requires requires(std::ostream& os, decltype(topic_info::getField<Col>(std::declval<const topic_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << topic_info::getField<Col>(iter); 
            oss << ",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
  };
    
} 
}
#endif
   