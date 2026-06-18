#ifndef ORM_CMS_XTALKBASEMATA_H
#define ORM_CMS_XTALKBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Thu, 18 Jun 2026 12:31:02 GMT
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

namespace xtalk_info
{
 
    enum class cols : unsigned char 
    {
		talkid = 0,
		userid = 1,
		adminid = 2,
		taskid = 3,
		projectid = 4,
		content = 5,
		isdelete = 6,
		addtime = 7,
		update_at = 8,
		islock = 9,
		replyid = 10,

    };

    struct meta
    {
		 unsigned  int  talkid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  adminid = 0; ///**/
		 unsigned  int  taskid = 0; ///**/
		 unsigned  int  projectid = 0; ///**/
		 std::string  content = ""; ///**/
		 unsigned  char  isdelete = 0; ///**/
		 unsigned  int  addtime = 0; ///**/
		 unsigned  int  update_at = 0; ///**/
		 unsigned  char  islock = 0; ///**/
		 unsigned  int  replyid = 0; ///**/
	};
  
    struct meta_tree
    {
		 unsigned  int  talkid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  adminid = 0; ///**/
		 unsigned  int  taskid = 0; ///**/
		 unsigned  int  projectid = 0; ///**/
		 std::string  content = ""; ///**/
		 unsigned  char  isdelete = 0; ///**/
		 unsigned  int  addtime = 0; ///**/
		 unsigned  int  update_at = 0; ///**/
		 unsigned  char  islock = 0; ///**/
		 unsigned  int  replyid = 0; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  talkid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  adminid = 0; ///**/
		 unsigned  int  taskid = 0; ///**/
		 unsigned  int  projectid = 0; ///**/
		 std::string  content = ""; ///**/
		 unsigned  char  isdelete = 0; ///**/
		 unsigned  int  addtime = 0; ///**/
		 unsigned  int  update_at = 0; ///**/
		 unsigned  char  islock = 0; ///**/
		 unsigned  int  replyid = 0; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::talkid) { 
		 return m.talkid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::adminid) { 
		 return m.adminid;
		} else if constexpr (Col == cols::taskid) { 
		 return m.taskid;
		} else if constexpr (Col == cols::projectid) { 
		 return m.projectid;
		} else if constexpr (Col == cols::content) { 
		 return m.content;
		} else if constexpr (Col == cols::isdelete) { 
		 return m.isdelete;
		} else if constexpr (Col == cols::addtime) { 
		 return m.addtime;
		} else if constexpr (Col == cols::update_at) { 
		 return m.update_at;
		} else if constexpr (Col == cols::islock) { 
		 return m.islock;
		} else if constexpr (Col == cols::replyid) { 
		 return m.replyid;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using talkid =  unsigned  int ;
		using userid =  unsigned  int ;
		using adminid =  unsigned  int ;
		using taskid =  unsigned  int ;
		using projectid =  unsigned  int ;
		using content =  std::string ;
		using isdelete =  unsigned  char ;
		using addtime =  unsigned  int ;
		using update_at =  unsigned  int ;
		using islock =  unsigned  char ;
		using replyid =  unsigned  int ;

    }

    
    #define ORM_CMS_XTALK_EXPAND(x) x 
    
    #define ORM_CMS_XTALK_META_FIELD_TYPE(col) \
         orm::cms::xtalk_info::type::col 
    
    #define ORM_CMS_XTALK_PROJ_MEMBER(col) \
          ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_XTALK_PROJ_MEMBERS_1(c1) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_XTALK_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_1( c1)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c2))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c3))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c4))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c5))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c6))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c7))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c8))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c9))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c10))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c11))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c12))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c13))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c14))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c15))
         
    #define ORM_CMS_XTALK_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBER(c16))
         
    #define ORM_CMS_XTALK_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_XTALK_PROJ_MEMBERS(...) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_XTALK_PROJ_MEMBERS_16, \
            ORM_CMS_XTALK_PROJ_MEMBERS_15, \
            ORM_CMS_XTALK_PROJ_MEMBERS_14, \
            ORM_CMS_XTALK_PROJ_MEMBERS_13, \
            ORM_CMS_XTALK_PROJ_MEMBERS_12, \
            ORM_CMS_XTALK_PROJ_MEMBERS_11, \
            ORM_CMS_XTALK_PROJ_MEMBERS_10, \
            ORM_CMS_XTALK_PROJ_MEMBERS_9, \
            ORM_CMS_XTALK_PROJ_MEMBERS_8, \
            ORM_CMS_XTALK_PROJ_MEMBERS_7, \
            ORM_CMS_XTALK_PROJ_MEMBERS_6, \
            ORM_CMS_XTALK_PROJ_MEMBERS_5, \
            ORM_CMS_XTALK_PROJ_MEMBERS_4, \
            ORM_CMS_XTALK_PROJ_MEMBERS_3, \
            ORM_CMS_XTALK_PROJ_MEMBERS_2, \
            ORM_CMS_XTALK_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_XTALK_COUNT(...) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_XTALK_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_XTALK_TO_JSON_1(c1) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_XTALK_TO_JSON_2(c1,c2) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_XTALK_TO_JSON_BODY(...) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_XTALK_TO_JSON_16,ORM_CMS_XTALK_TO_JSON_15,ORM_CMS_XTALK_TO_JSON_14,ORM_CMS_XTALK_TO_JSON_13,ORM_CMS_XTALK_TO_JSON_12,ORM_CMS_XTALK_TO_JSON_11,ORM_CMS_XTALK_TO_JSON_10,ORM_CMS_XTALK_TO_JSON_9,ORM_CMS_XTALK_TO_JSON_8,ORM_CMS_XTALK_TO_JSON_7,ORM_CMS_XTALK_TO_JSON_6,ORM_CMS_XTALK_TO_JSON_5,ORM_CMS_XTALK_TO_JSON_4,ORM_CMS_XTALK_TO_JSON_3,ORM_CMS_XTALK_TO_JSON_2,ORM_CMS_XTALK_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_XTALK_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_XTALK_TO_JSON_CUSTOM_1(n1)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_1(n1)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_XTALK_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_XTALK_CAT(a, b) ORM_CMS_XTALK_CAT_(a, b)
    #define ORM_CMS_XTALK_CAT_(a, b) a##b

    #define ORM_CMS_XTALK_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_XTALK_CAT(ORM_CMS_XTALK_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_XTALK_TO_JSON_CUSTOM(...) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_XTALK_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_XTALK_SET_VAL_1(c1) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_XTALK_SET_VAL_2(c1,c2) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_1(c1)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_XTALK_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_2(c1,c2)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_XTALK_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_XTALK_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_XTALK_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_XTALK_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_XTALK_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_XTALK_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_XTALK_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_XTALK_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_XTALK_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_XTALK_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_XTALK_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_XTALK_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_XTALK_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_XTALK_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_XTALK_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_XTALK_CAT(ORM_CMS_XTALK_SET_VAL_, N)
    
    
    #define ORM_CMS_XTALK_SET_VAL_FIELDS(...) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_XTALK_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_FIELDS(ORM_CMS_XTALK_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_XTALK_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::xtalk_info { \
            struct StructName { \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_XTALK_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::xtalk_info { \
            struct StructName { \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM(ORM_CMS_XTALK_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_XTALK_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::xtalk_info { \
            struct StructName { \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_XTALK_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::xtalk_info { \
            struct StructName { \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_XTALK_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::xtalk_info { \
            struct StructName { \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_TO_JSON_CUSTOM(ORM_CMS_XTALK_UNWRAP CustomNames));  \
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
                        ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_XTALK_EXPAND(ORM_CMS_XTALK_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,11> col_names={"talkid","userid","adminid","taskid","projectid","content","isdelete","addtime","update_at","islock","replyid"};
	static constexpr std::array<unsigned char,11> col_types={3,3,3,3,3,252,1,3,3,1,3};
	static constexpr std::array<unsigned char,11> col_length={0,0,0,0,0,0,0,0,0,0,0};
	static constexpr std::array<unsigned char,11> col_decimals={0,0,0,0,0,0,0,0,0,0,0};

}

struct xtalk_base
{
      xtalk_info::meta data;
    std::vector<xtalk_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<xtalk_info::meta>::iterator begin(){     return record.begin(); }
std::vector<xtalk_info::meta>::iterator end(){     return record.end(); }
std::vector<xtalk_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<xtalk_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="xtalk";
static constexpr std::string_view org_tablename="xtalk";
static constexpr std::string_view modelname="Xtalk";

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
 switch(coln.size()){  
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 2; }
 if(colpospppc=='e'){ return 7; }
   	 break;
 }
 break;
case 'c':
   	 return 5;
break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 9;
break;
case 8:
   	 return 6;
break;
 }
 break;
case 'p':
   	 return 4;
break;
case 'r':
   	 return 10;
break;
case 't':
 switch(coln.size()){  
case 6:
 if(coln.size()>2&&(coln[2]=='l'||coln[2]=='L')){ return 0; }
 if(coln.size()>2&&(coln[2]=='s'||coln[2]=='S')){ return 3; }
   	 break;
 }
 break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 1;
break;
case 9:
   	 return 8;
break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "talkid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     xtalk_info::meta metatemp;    
            data = metatemp; 
      }
      
      std::string soft_remove_sql([[maybe_unused]] const std::string &fieldsql){
          std::string temp;
     
         if(fieldsql.size()<2)
         {
            temp="UPDATE `";
            temp.append(tablename);
            temp.push_back('`');
            temp.append(" SET ");
         }
         else
         {
            temp=_makeupdatesql(fieldsql);
            if(temp.size()>2)
            {
                if(temp.back()==0x20&&temp[temp.size()-2]=='T')
                {

                }
                else
                {
                    temp.push_back(',');
                }
            }
            
         }   
         
      	temp.push_back('`');
		temp.append("isdelete");
		temp.push_back('`');
		temp.append("=1 ");
		 if(fieldsql.size()>0){ data.isdelete=1; }
	
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

   std::string _makeinsertsql(){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<xtalk_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xtalk_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.talkid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.talkid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.adminid);
}
if(data.taskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.taskid);
}
if(data.projectid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.projectid);
}
tempsql<<",'"<<stringaddslash(data.content)<<"'";
if(data.isdelete==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isdelete);
}
if(data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.addtime);
}
if(data.update_at==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.update_at);
}
if(data.islock==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.islock);
}
if(data.replyid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.replyid);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const xtalk_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<xtalk_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xtalk_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.talkid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.talkid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.adminid);
}
if(insert_data.taskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.taskid);
}
if(insert_data.projectid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.projectid);
}
tempsql<<",'"<<stringaddslash(insert_data.content)<<"'";
if(insert_data.isdelete==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isdelete);
}
if(insert_data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.addtime);
}
if(insert_data.update_at==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.update_at);
}
if(insert_data.islock==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.islock);
}
if(insert_data.replyid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.replyid);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<xtalk_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<xtalk_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xtalk_info::col_names[j];
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


            	if(insert_data[i].talkid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].talkid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].adminid);
	}
	if(insert_data[i].taskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].taskid);
	}
	if(insert_data[i].projectid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].projectid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].content)<<"'";
	if(insert_data[i].isdelete==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isdelete);
	}
	if(insert_data[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].addtime);
	}
	if(insert_data[i].update_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].update_at);
	}
	if(insert_data[i].islock==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].islock);
	}
	if(insert_data[i].replyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].replyid);
	}
		tempsql<<")";
	 } 

     
       return tempsql.str();
   } 
       
    std::string _makeupdatesql(std::string_view fileld){
        std::ostringstream tempsql;
        tempsql<<"UPDATE ";
        tempsql<<tablename;
        tempsql<<" SET ";

        bool isall=false;
        if(fileld.empty()){
            isall=true;
        }
        if(isall){

        if(data.talkid==0){
	tempsql<<"`talkid`=0";
 }else{ 
	tempsql<<"`talkid`="<<std::to_string(data.talkid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.adminid==0){
	tempsql<<",`adminid`=0";
 }else{ 
	tempsql<<",`adminid`="<<std::to_string(data.adminid);
}
if(data.taskid==0){
	tempsql<<",`taskid`=0";
 }else{ 
	tempsql<<",`taskid`="<<std::to_string(data.taskid);
}
if(data.projectid==0){
	tempsql<<",`projectid`=0";
 }else{ 
	tempsql<<",`projectid`="<<std::to_string(data.projectid);
}
tempsql<<",`content`='"<<stringaddslash(data.content)<<"'";
if(data.isdelete==0){
	tempsql<<",`isdelete`=0";
 }else{ 
	tempsql<<",`isdelete`="<<std::to_string(data.isdelete);
}
if(data.addtime==0){
	tempsql<<",`addtime`=0";
 }else{ 
	tempsql<<",`addtime`="<<std::to_string(data.addtime);
}
if(data.update_at==0){
	tempsql<<",`update_at`=0";
 }else{ 
	tempsql<<",`update_at`="<<std::to_string(data.update_at);
}
if(data.islock==0){
	tempsql<<",`islock`=0";
 }else{ 
	tempsql<<",`islock`="<<std::to_string(data.islock);
}
if(data.replyid==0){
	tempsql<<",`replyid`=0";
 }else{ 
	tempsql<<",`replyid`="<<std::to_string(data.replyid);
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
if(data.talkid==0){
	tempsql<<"`talkid`=0";
 }else{ 
	tempsql<<"`talkid`="<<std::to_string(data.talkid);
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
if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.taskid==0){
	tempsql<<"`taskid`=0";
 }else{ 
	tempsql<<"`taskid`="<<std::to_string(data.taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.projectid==0){
	tempsql<<"`projectid`=0";
 }else{ 
	tempsql<<"`projectid`="<<std::to_string(data.projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`content`='"<<stringaddslash(data.content)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isdelete==0){
	tempsql<<"`isdelete`=0";
 }else{ 
	tempsql<<"`isdelete`="<<std::to_string(data.isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"`addtime`=0";
 }else{ 
	tempsql<<"`addtime`="<<std::to_string(data.addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.update_at==0){
	tempsql<<"`update_at`=0";
 }else{ 
	tempsql<<"`update_at`="<<std::to_string(data.update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.islock==0){
	tempsql<<"`islock`=0";
 }else{ 
	tempsql<<"`islock`="<<std::to_string(data.islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.replyid==0){
	tempsql<<"`replyid`=0";
 }else{ 
	tempsql<<"`replyid`="<<std::to_string(data.replyid);
}
 break;

     
                  default:
                                ;
                     }
                 }   

            }        

        return tempsql.str();
   } 
   
    std::string _make_replace_into_sql()
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
        tempsql << "REPLACE INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < xtalk_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << xtalk_info::col_names[j];
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
            	if(record[i].talkid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].talkid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	if(record[i].taskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].taskid);
	}
	if(record[i].projectid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].projectid);
	}
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	if(record[i].isdelete==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isdelete);
	}
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].update_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].update_at);
	}
	if(record[i].islock==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].islock);
	}
	if(record[i].replyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].replyid);
	}
	tempsql<<")";

 }
 return tempsql.str();
}

    std::string _make_insert_into_sql(std::string_view fileld)
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
        tempsql << "INSERT INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < xtalk_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << xtalk_info::col_names[j];
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
            	if(record[i].talkid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].talkid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	if(record[i].taskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].taskid);
	}
	if(record[i].projectid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].projectid);
	}
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	if(record[i].isdelete==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isdelete);
	}
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].update_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].update_at);
	}
	if(record[i].islock==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].islock);
	}
	if(record[i].replyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].replyid);
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
            for(jj=0;jj<xtalk_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.talkid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.talkid));
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
if(data.adminid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.adminid));
}
 break;
 case 3:
if(data.taskid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.taskid));
}
 break;
 case 4:
if(data.projectid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.projectid));
}
 break;
 case 5:
	temparray.push_back(data.content);
 break;
 case 6:
if(data.isdelete==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isdelete));
}
 break;
 case 7:
if(data.addtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.addtime));
}
 break;
 case 8:
if(data.update_at==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.update_at));
}
 break;
 case 9:
if(data.islock==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.islock));
}
 break;
 case 10:
if(data.replyid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.replyid));
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
            for(jj=0;jj<xtalk_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.talkid==0){
	tempsql.insert({"talkid","0"});
 }else{ 
	tempsql.insert({"talkid",std::to_string(data.talkid)});
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
if(data.adminid==0){
	tempsql.insert({"adminid","0"});
 }else{ 
	tempsql.insert({"adminid",std::to_string(data.adminid)});
}
 break;
 case 3:
if(data.taskid==0){
	tempsql.insert({"taskid","0"});
 }else{ 
	tempsql.insert({"taskid",std::to_string(data.taskid)});
}
 break;
 case 4:
if(data.projectid==0){
	tempsql.insert({"projectid","0"});
 }else{ 
	tempsql.insert({"projectid",std::to_string(data.projectid)});
}
 break;
 case 5:
	tempsql.insert({"content",data.content});
 break;
 case 6:
if(data.isdelete==0){
	tempsql.insert({"isdelete","0"});
 }else{ 
	tempsql.insert({"isdelete",std::to_string(data.isdelete)});
}
 break;
 case 7:
if(data.addtime==0){
	tempsql.insert({"addtime","0"});
 }else{ 
	tempsql.insert({"addtime",std::to_string(data.addtime)});
}
 break;
 case 8:
if(data.update_at==0){
	tempsql.insert({"update_at","0"});
 }else{ 
	tempsql.insert({"update_at",std::to_string(data.update_at)});
}
 break;
 case 9:
if(data.islock==0){
	tempsql.insert({"islock","0"});
 }else{ 
	tempsql.insert({"islock",std::to_string(data.islock)});
}
 break;
 case 10:
if(data.replyid==0){
	tempsql.insert({"replyid","0"});
 }else{ 
	tempsql.insert({"replyid",std::to_string(data.replyid)});
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
if(data.talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(data.talkid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.adminid==0){
	tempsql<<",\"adminid\":0";
 }else{ 
	tempsql<<",\"adminid\":"<<std::to_string(data.adminid);
}
if(data.taskid==0){
	tempsql<<",\"taskid\":0";
 }else{ 
	tempsql<<",\"taskid\":"<<std::to_string(data.taskid);
}
if(data.projectid==0){
	tempsql<<",\"projectid\":0";
 }else{ 
	tempsql<<",\"projectid\":"<<std::to_string(data.projectid);
}
tempsql<<",\"content\":\""<<http::utf8_to_jsonstring(data.content);
tempsql<<"\"";
if(data.isdelete==0){
	tempsql<<",\"isdelete\":0";
 }else{ 
	tempsql<<",\"isdelete\":"<<std::to_string(data.isdelete);
}
if(data.addtime==0){
	tempsql<<",\"addtime\":0";
 }else{ 
	tempsql<<",\"addtime\":"<<std::to_string(data.addtime);
}
if(data.update_at==0){
	tempsql<<",\"update_at\":0";
 }else{ 
	tempsql<<",\"update_at\":"<<std::to_string(data.update_at);
}
if(data.islock==0){
	tempsql<<",\"islock\":0";
 }else{ 
	tempsql<<",\"islock\":"<<std::to_string(data.islock);
}
if(data.replyid==0){
	tempsql<<",\"replyid\":0";
 }else{ 
	tempsql<<",\"replyid\":"<<std::to_string(data.replyid);
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
            for(jj=0;jj<xtalk_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(data.talkid);
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
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.taskid==0){
	tempsql<<"\"taskid\":0";
 }else{ 
	tempsql<<"\"taskid\":"<<std::to_string(data.taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.projectid==0){
	tempsql<<"\"projectid\":0";
 }else{ 
	tempsql<<"\"projectid\":"<<std::to_string(data.projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isdelete==0){
	tempsql<<"\"isdelete\":0";
 }else{ 
	tempsql<<"\"isdelete\":"<<std::to_string(data.isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(data.addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.update_at==0){
	tempsql<<"\"update_at\":0";
 }else{ 
	tempsql<<"\"update_at\":"<<std::to_string(data.update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.islock==0){
	tempsql<<"\"islock\":0";
 }else{ 
	tempsql<<"\"islock\":"<<std::to_string(data.islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.replyid==0){
	tempsql<<"\"replyid\":0";
 }else{ 
	tempsql<<"\"replyid\":"<<std::to_string(data.replyid);
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
        xtalk_info::meta metatemp; 
        data=metatemp;
        unsigned int json_offset=0;
        bool isarray=false;
        //std::vector<std::string> list_content;
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
                for(;json_offset<json_content.size();json_offset++)
                {
                    if(json_content[json_offset]=='{')
                    {
                        json_offset+=1;
                        break;
                    }
                }
                if(record.size()>0)
                {
                    data=metatemp;
                }
                if(json_offset>=json_content.size())
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
                                    if(json_content[json_offset]!=':')
                                    {
                                        break;
                                    }
                                    for(;json_offset<json_content.size();json_offset++)
                                    {
                                        if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                        {
                                            continue;
                                        }
                                        break;
                                    } 
                                    json_offset+=1;
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
                
                json_offset+=1;
            }
            if(record.size()>1)
            {
                data=record[0];
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
                                for(;json_offset<json_content.size();json_offset++)
                                {
                                    if(json_content[json_offset]==0x20||json_content[json_offset]==0x0A||json_content[json_offset]==0x0D||json_content[json_offset]=='\t')
                                    {
                                        continue;
                                    }
                                    break;
                                } 
                                json_offset+=1;
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
   }   
    
    void set_val(const std::string& set_key_name,const std::string& set_value_name)
    {
        switch(findcolpos(set_key_name))
        {
    		case 0:
		 try{
			data.talkid=std::stoul(set_value_name);
		}catch (...) { 
			data.talkid=0;
			 }
			break;
		case 1:
		 try{
			data.userid=std::stoul(set_value_name);
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 2:
		 try{
			data.adminid=std::stoul(set_value_name);
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 3:
		 try{
			data.taskid=std::stoul(set_value_name);
		}catch (...) { 
			data.taskid=0;
			 }
			break;
		case 4:
		 try{
			data.projectid=std::stoul(set_value_name);
		}catch (...) { 
			data.projectid=0;
			 }
			break;
		case 5:
		 try{
			data.content.append(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 6:
		 try{
			data.isdelete=std::stoi(set_value_name);
		}catch (...) { 
			data.isdelete=0;
			 }
			break;
		case 7:
		 try{
			data.addtime=std::stoul(set_value_name);
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 8:
		 try{
			data.update_at=std::stoul(set_value_name);
		}catch (...) { 
			data.update_at=0;
			 }
			break;
		case 9:
		 try{
			data.islock=std::stoi(set_value_name);
		}catch (...) { 
			data.islock=0;
			 }
			break;
		case 10:
		 try{
			data.replyid=std::stoul(set_value_name);
		}catch (...) { 
			data.replyid=0;
			 }
			break;
	default:
		 { }
			


        }
   } 
    
    void set_val(const std::string& set_key_name,const long long set_value_name)
    {
        switch(findcolpos(set_key_name))
        {
    		case 0:
		 try{
			data.talkid=set_value_name;
		}catch (...) { 
			data.talkid=0;
			 }
			break;
		case 1:
		 try{
			data.userid=set_value_name;
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 2:
		 try{
			data.adminid=set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 3:
		 try{
			data.taskid=set_value_name;
		}catch (...) { 
			data.taskid=0;
			 }
			break;
		case 4:
		 try{
			data.projectid=set_value_name;
		}catch (...) { 
			data.projectid=0;
			 }
			break;
		case 5:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 6:
		 try{
			data.isdelete=set_value_name;
		}catch (...) { 
			data.isdelete=0;
			 }
			break;
		case 7:
		 try{
			data.addtime=set_value_name;
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 8:
		 try{
			data.update_at=set_value_name;
		}catch (...) { 
			data.update_at=0;
			 }
			break;
		case 9:
		 try{
			data.islock=set_value_name;
		}catch (...) { 
			data.islock=0;
			 }
			break;
		case 10:
		 try{
			data.replyid=set_value_name;
		}catch (...) { 
			data.replyid=0;
			 }
			break;
	default:
		 { }
			


        }
   } 
    
    void set_val(const std::string& set_key_name,const double set_value_name)
    {
        switch(findcolpos(set_key_name))
        {
    		case 0:
		 try{
			data.talkid=(unsigned int)set_value_name;
		}catch (...) { 
			data.talkid=0;
			 }
			break;
		case 1:
		 try{
			data.userid=(unsigned int)set_value_name;
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 2:
		 try{
			data.adminid=(unsigned int)set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 3:
		 try{
			data.taskid=(unsigned int)set_value_name;
		}catch (...) { 
			data.taskid=0;
			 }
			break;
		case 4:
		 try{
			data.projectid=(unsigned int)set_value_name;
		}catch (...) { 
			data.projectid=0;
			 }
			break;
		case 5:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 6:
		 try{
			data.isdelete=(int)set_value_name;
		}catch (...) { 
			data.isdelete=0;
			 }
			break;
		case 7:
		 try{
			data.addtime=(unsigned int)set_value_name;
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 8:
		 try{
			data.update_at=(unsigned int)set_value_name;
		}catch (...) { 
			data.update_at=0;
			 }
			break;
		case 9:
		 try{
			data.islock=(int)set_value_name;
		}catch (...) { 
			data.islock=0;
			 }
			break;
		case 10:
		 try{
			data.replyid=(unsigned int)set_value_name;
		}catch (...) { 
			data.replyid=0;
			 }
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
        for(jj=0;jj<xtalk_info::col_names.size();jj++){
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
if(record[n].talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(record[n].talkid);
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
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].taskid==0){
	tempsql<<"\"taskid\":0";
 }else{ 
	tempsql<<"\"taskid\":"<<std::to_string(record[n].taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].projectid==0){
	tempsql<<"\"projectid\":0";
 }else{ 
	tempsql<<"\"projectid\":"<<std::to_string(record[n].projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isdelete==0){
	tempsql<<"\"isdelete\":0";
 }else{ 
	tempsql<<"\"isdelete\":"<<std::to_string(record[n].isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].update_at==0){
	tempsql<<"\"update_at\":0";
 }else{ 
	tempsql<<"\"update_at\":"<<std::to_string(record[n].update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].islock==0){
	tempsql<<"\"islock\":0";
 }else{ 
	tempsql<<"\"islock\":"<<std::to_string(record[n].islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].replyid==0){
	tempsql<<"\"replyid\":0";
 }else{ 
	tempsql<<"\"replyid\":"<<std::to_string(record[n].replyid);
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
   
   std::string to_json(std::function<bool(std::string&,xtalk_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<xtalk_info::col_names.size();jj++){
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
if(record[n].talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(record[n].talkid);
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
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].taskid==0){
	tempsql<<"\"taskid\":0";
 }else{ 
	tempsql<<"\"taskid\":"<<std::to_string(record[n].taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].projectid==0){
	tempsql<<"\"projectid\":0";
 }else{ 
	tempsql<<"\"projectid\":"<<std::to_string(record[n].projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isdelete==0){
	tempsql<<"\"isdelete\":0";
 }else{ 
	tempsql<<"\"isdelete\":"<<std::to_string(record[n].isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].update_at==0){
	tempsql<<"\"update_at\":0";
 }else{ 
	tempsql<<"\"update_at\":"<<std::to_string(record[n].update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].islock==0){
	tempsql<<"\"islock\":0";
 }else{ 
	tempsql<<"\"islock\":"<<std::to_string(record[n].islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].replyid==0){
	tempsql<<"\"replyid\":0";
 }else{ 
	tempsql<<"\"replyid\":"<<std::to_string(record[n].replyid);
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
   long long getPK(){  return data.talkid; } 
 void setPK(long long val){  data.talkid=val;} 
 unsigned  int  getTalkid(){  return data.talkid; } 
 void setTalkid( unsigned  int  val){  data.talkid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getAdminid(){  return data.adminid; } 
 void setAdminid( unsigned  int  val){  data.adminid=val;} 

 unsigned  int  getTaskid(){  return data.taskid; } 
 void setTaskid( unsigned  int  val){  data.taskid=val;} 

 unsigned  int  getProjectid(){  return data.projectid; } 
 void setProjectid( unsigned  int  val){  data.projectid=val;} 

 std::string  getContent(){  return data.content; } 
 std::string & getRefContent(){  return std::ref(data.content); } 
 void setContent( std::string  &val){  data.content=val;} 
 void setContent(std::string_view val){  data.content=val;} 

 unsigned  char  getIsdelete(){  return data.isdelete; } 
 void setIsdelete( unsigned  char  val){  data.isdelete=val;} 

 unsigned  int  getAddtime(){  return data.addtime; } 
 void setAddtime( unsigned  int  val){  data.addtime=val;} 

 unsigned  int  getUpdateAt(){  return data.update_at; } 
 void setUpdateAt( unsigned  int  val){  data.update_at=val;} 

 unsigned  char  getIslock(){  return data.islock; } 
 void setIslock( unsigned  char  val){  data.islock=val;} 

 unsigned  int  getReplyid(){  return data.replyid; } 
 void setReplyid( unsigned  int  val){  data.replyid=val;} 

xtalk_info::meta getnewData(){
 	 struct xtalk_info::meta newdata;
	 return newdata; 
} 
xtalk_info::meta getData(){
 	 return data; 
} 
std::vector<xtalk_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<xtalk_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<xtalk_info::col_names.size();jj++){
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
if(tree_data[n].talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(tree_data[n].talkid);
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
if(tree_data[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(tree_data[n].adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].taskid==0){
	tempsql<<"\"taskid\":0";
 }else{ 
	tempsql<<"\"taskid\":"<<std::to_string(tree_data[n].taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].projectid==0){
	tempsql<<"\"projectid\":0";
 }else{ 
	tempsql<<"\"projectid\":"<<std::to_string(tree_data[n].projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isdelete==0){
	tempsql<<"\"isdelete\":0";
 }else{ 
	tempsql<<"\"isdelete\":"<<std::to_string(tree_data[n].isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(tree_data[n].addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].update_at==0){
	tempsql<<"\"update_at\":0";
 }else{ 
	tempsql<<"\"update_at\":"<<std::to_string(tree_data[n].update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].islock==0){
	tempsql<<"\"islock\":0";
 }else{ 
	tempsql<<"\"islock\":"<<std::to_string(tree_data[n].islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].replyid==0){
	tempsql<<"\"replyid\":0";
 }else{ 
	tempsql<<"\"replyid\":"<<std::to_string(tree_data[n].replyid);
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
   
   std::string tree_tojson(const std::vector<xtalk_info::meta_tree> &tree_data,std::function<bool(std::string&,const xtalk_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<xtalk_info::col_names.size();jj++){
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
if(tree_data[n].talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(tree_data[n].talkid);
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
if(tree_data[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(tree_data[n].adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].taskid==0){
	tempsql<<"\"taskid\":0";
 }else{ 
	tempsql<<"\"taskid\":"<<std::to_string(tree_data[n].taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].projectid==0){
	tempsql<<"\"projectid\":0";
 }else{ 
	tempsql<<"\"projectid\":"<<std::to_string(tree_data[n].projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isdelete==0){
	tempsql<<"\"isdelete\":0";
 }else{ 
	tempsql<<"\"isdelete\":"<<std::to_string(tree_data[n].isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(tree_data[n].addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].update_at==0){
	tempsql<<"\"update_at\":0";
 }else{ 
	tempsql<<"\"update_at\":"<<std::to_string(tree_data[n].update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].islock==0){
	tempsql<<"\"islock\":0";
 }else{ 
	tempsql<<"\"islock\":"<<std::to_string(tree_data[n].islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].replyid==0){
	tempsql<<"\"replyid\":0";
 }else{ 
	tempsql<<"\"replyid\":"<<std::to_string(tree_data[n].replyid);
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
   
    template<xtalk_info::cols KeyCol, xtalk_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>()));
        using ValType = decltype(xtalk_info::getField<ValCol>(std::declval<const xtalk_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(xtalk_info::getField<KeyCol>(iter), xtalk_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xtalk_info::cols KeyCol, xtalk_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>())), 
            decltype(xtalk_info::getField<ValCol>(std::declval<const xtalk_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>())), 
                    decltype(xtalk_info::getField<ValCol>(std::declval<const xtalk_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>()));
        using ValType = decltype(xtalk_info::getField<ValCol>(std::declval<const xtalk_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(xtalk_info::getField<KeyCol>(iter), xtalk_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xtalk_info::getField<KeyCol>(iter), xtalk_info::getField<ValCol>(iter))) {
                    result.emplace(xtalk_info::getField<KeyCol>(iter), xtalk_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xtalk_info::cols KeyCol, xtalk_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>()));
        using ValType = decltype(xtalk_info::getField<ValCol>(std::declval<const xtalk_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(xtalk_info::getField<KeyCol>(iter), xtalk_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xtalk_info::cols KeyCol, xtalk_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>())), 
            decltype(xtalk_info::getField<ValCol>(std::declval<const xtalk_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>())), 
                    decltype(xtalk_info::getField<ValCol>(std::declval<const xtalk_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>()));
        using ValType = decltype(xtalk_info::getField<ValCol>(std::declval<const xtalk_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(xtalk_info::getField<KeyCol>(iter), xtalk_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xtalk_info::getField<KeyCol>(iter), xtalk_info::getField<ValCol>(iter))) {
                    result.emplace_back(xtalk_info::getField<KeyCol>(iter), xtalk_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xtalk_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(xtalk_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xtalk_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(xtalk_info::getField<KeyCol>(std::declval<const xtalk_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(xtalk_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xtalk_info::getField<KeyCol>(iter))) {
                    result.emplace_back(xtalk_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xtalk_info::cols Col>
        requires requires(std::ostream& os, decltype(xtalk_info::getField<Col>(std::declval<const xtalk_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << xtalk_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<xtalk_info::cols Col>
        requires requires(std::ostream& os, decltype(xtalk_info::getField<Col>(std::declval<const xtalk_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << xtalk_info::getField<Col>(iter); 
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
   