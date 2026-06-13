#ifndef ORM_CMS_XMEETBASEMATA_H
#define ORM_CMS_XMEETBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sat, 13 Jun 2026 05:09:34 GMT
***/
#include <iostream>
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

namespace xmeet_info
{
 
    enum class cols : unsigned char 
    {
		xmeetid = 0,
		userid = 1,
		xpjid = 2,
		xtaskid = 3,
		adminid = 4,
		title = 5,
		zhuchi = 6,
		jilu = 7,
		meetnotice = 8,
		meetfiles = 9,
		address = 10,
		meettime = 11,
		regdate = 12,
		expecttime = 13,
		endtime = 14,
		presents = 15,
		content = 16,
		postresult = 17,
		postfiles = 18,
		jiluphoto = 19,

    };

    struct meta
    {
		 unsigned  int  xmeetid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  xpjid = 0; ///**/
		 unsigned  int  xtaskid = 0; ///**/
		 unsigned  int  adminid = 0; ///**/
		 std::string  title = ""; ///**/
		 std::string  zhuchi = ""; ///**/
		 std::string  jilu = ""; ///**/
		 std::string  meetnotice = ""; ///**/
		 std::string  meetfiles = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  meettime = ""; ///**/
		 unsigned  int  regdate = 0; ///**/
		 unsigned  int  expecttime = 0; ///**/
		 unsigned  int  endtime = 0; ///**/
		 std::string  presents = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  postresult = ""; ///**/
		 std::string  postfiles = ""; ///**/
		 std::string  jiluphoto = ""; ///**/
	};
  
    struct meta_tree
    {
		 unsigned  int  xmeetid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  xpjid = 0; ///**/
		 unsigned  int  xtaskid = 0; ///**/
		 unsigned  int  adminid = 0; ///**/
		 std::string  title = ""; ///**/
		 std::string  zhuchi = ""; ///**/
		 std::string  jilu = ""; ///**/
		 std::string  meetnotice = ""; ///**/
		 std::string  meetfiles = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  meettime = ""; ///**/
		 unsigned  int  regdate = 0; ///**/
		 unsigned  int  expecttime = 0; ///**/
		 unsigned  int  endtime = 0; ///**/
		 std::string  presents = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  postresult = ""; ///**/
		 std::string  postfiles = ""; ///**/
		 std::string  jiluphoto = ""; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  xmeetid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  xpjid = 0; ///**/
		 unsigned  int  xtaskid = 0; ///**/
		 unsigned  int  adminid = 0; ///**/
		 std::string  title = ""; ///**/
		 std::string  zhuchi = ""; ///**/
		 std::string  jilu = ""; ///**/
		 std::string  meetnotice = ""; ///**/
		 std::string  meetfiles = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  meettime = ""; ///**/
		 unsigned  int  regdate = 0; ///**/
		 unsigned  int  expecttime = 0; ///**/
		 unsigned  int  endtime = 0; ///**/
		 std::string  presents = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  postresult = ""; ///**/
		 std::string  postfiles = ""; ///**/
		 std::string  jiluphoto = ""; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::xmeetid) { 
		 return m.xmeetid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::xpjid) { 
		 return m.xpjid;
		} else if constexpr (Col == cols::xtaskid) { 
		 return m.xtaskid;
		} else if constexpr (Col == cols::adminid) { 
		 return m.adminid;
		} else if constexpr (Col == cols::title) { 
		 return m.title;
		} else if constexpr (Col == cols::zhuchi) { 
		 return m.zhuchi;
		} else if constexpr (Col == cols::jilu) { 
		 return m.jilu;
		} else if constexpr (Col == cols::meetnotice) { 
		 return m.meetnotice;
		} else if constexpr (Col == cols::meetfiles) { 
		 return m.meetfiles;
		} else if constexpr (Col == cols::address) { 
		 return m.address;
		} else if constexpr (Col == cols::meettime) { 
		 return m.meettime;
		} else if constexpr (Col == cols::regdate) { 
		 return m.regdate;
		} else if constexpr (Col == cols::expecttime) { 
		 return m.expecttime;
		} else if constexpr (Col == cols::endtime) { 
		 return m.endtime;
		} else if constexpr (Col == cols::presents) { 
		 return m.presents;
		} else if constexpr (Col == cols::content) { 
		 return m.content;
		} else if constexpr (Col == cols::postresult) { 
		 return m.postresult;
		} else if constexpr (Col == cols::postfiles) { 
		 return m.postfiles;
		} else if constexpr (Col == cols::jiluphoto) { 
		 return m.jiluphoto;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using xmeetid = decltype(std::declval<const meta>().xmeetid);
		using userid = decltype(std::declval<const meta>().userid);
		using xpjid = decltype(std::declval<const meta>().xpjid);
		using xtaskid = decltype(std::declval<const meta>().xtaskid);
		using adminid = decltype(std::declval<const meta>().adminid);
		using title = decltype(std::declval<const meta>().title);
		using zhuchi = decltype(std::declval<const meta>().zhuchi);
		using jilu = decltype(std::declval<const meta>().jilu);
		using meetnotice = decltype(std::declval<const meta>().meetnotice);
		using meetfiles = decltype(std::declval<const meta>().meetfiles);
		using address = decltype(std::declval<const meta>().address);
		using meettime = decltype(std::declval<const meta>().meettime);
		using regdate = decltype(std::declval<const meta>().regdate);
		using expecttime = decltype(std::declval<const meta>().expecttime);
		using endtime = decltype(std::declval<const meta>().endtime);
		using presents = decltype(std::declval<const meta>().presents);
		using content = decltype(std::declval<const meta>().content);
		using postresult = decltype(std::declval<const meta>().postresult);
		using postfiles = decltype(std::declval<const meta>().postfiles);
		using jiluphoto = decltype(std::declval<const meta>().jiluphoto);

    }

    
    #define ORM_CMS_XMEET_META_FIELD_TYPE(col) \
        decltype(orm::cms::xmeet_info::getField<orm::cms::xmeet_info::cols::col>(std::declval<const orm::cms::xmeet_info::meta&>()))

    #define ORM_CMS_XMEET_PROJ_MEMBER(col) \
          ORM_CMS_XMEET_META_FIELD_TYPE(col) col;
                 
    #define ORM_CMS_XMEET_PROJ_MEMBERS_1(c1) \
        ORM_CMS_XMEET_PROJ_MEMBER(c1) 
     
    #define ORM_CMS_XMEET_PROJ_MEMBERS_2( c1, c2) \
        ORM_CMS_XMEET_PROJ_MEMBERS_1( c1) ORM_CMS_XMEET_PROJ_MEMBER(c2)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_3( c1, c2, c3) \
        ORM_CMS_XMEET_PROJ_MEMBERS_2( c1, c2) ORM_CMS_XMEET_PROJ_MEMBER(c3)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_4( c1, c2, c3, c4) \
        ORM_CMS_XMEET_PROJ_MEMBERS_3( c1, c2, c3) ORM_CMS_XMEET_PROJ_MEMBER(c4)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
        ORM_CMS_XMEET_PROJ_MEMBERS_4( c1, c2, c3, c4) ORM_CMS_XMEET_PROJ_MEMBER(c5)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
        ORM_CMS_XMEET_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) ORM_CMS_XMEET_PROJ_MEMBER(c6)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
        ORM_CMS_XMEET_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) ORM_CMS_XMEET_PROJ_MEMBER(c7)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
        ORM_CMS_XMEET_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) ORM_CMS_XMEET_PROJ_MEMBER(c8)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
        ORM_CMS_XMEET_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) ORM_CMS_XMEET_PROJ_MEMBER(c9)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
        ORM_CMS_XMEET_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) ORM_CMS_XMEET_PROJ_MEMBER(c10)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
        ORM_CMS_XMEET_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) ORM_CMS_XMEET_PROJ_MEMBER(c11)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
        ORM_CMS_XMEET_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) ORM_CMS_XMEET_PROJ_MEMBER(c12)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
        ORM_CMS_XMEET_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) ORM_CMS_XMEET_PROJ_MEMBER(c13)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
        ORM_CMS_XMEET_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) ORM_CMS_XMEET_PROJ_MEMBER(c14)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
        ORM_CMS_XMEET_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) ORM_CMS_XMEET_PROJ_MEMBER(c15)
         
    #define ORM_CMS_XMEET_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
        ORM_CMS_XMEET_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) ORM_CMS_XMEET_PROJ_MEMBER(c16)
         
    #define ORM_CMS_XMEET_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_XMEET_PROJ_MEMBERS(...) \
        ORM_CMS_XMEET_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_XMEET_PROJ_MEMBERS_16, \
            ORM_CMS_XMEET_PROJ_MEMBERS_15, \
            ORM_CMS_XMEET_PROJ_MEMBERS_14, \
            ORM_CMS_XMEET_PROJ_MEMBERS_13, \
            ORM_CMS_XMEET_PROJ_MEMBERS_12, \
            ORM_CMS_XMEET_PROJ_MEMBERS_11, \
            ORM_CMS_XMEET_PROJ_MEMBERS_10, \
            ORM_CMS_XMEET_PROJ_MEMBERS_9, \
            ORM_CMS_XMEET_PROJ_MEMBERS_8, \
            ORM_CMS_XMEET_PROJ_MEMBERS_7, \
            ORM_CMS_XMEET_PROJ_MEMBERS_6, \
            ORM_CMS_XMEET_PROJ_MEMBERS_5, \
            ORM_CMS_XMEET_PROJ_MEMBERS_4, \
            ORM_CMS_XMEET_PROJ_MEMBERS_3, \
            ORM_CMS_XMEET_PROJ_MEMBERS_2, \
            ORM_CMS_XMEET_PROJ_MEMBERS_1, \
        )(__VA_ARGS__)

    
    #define ORM_CMS_XMEET_COUNT(...) \
        ORM_CMS_XMEET_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
    
    
    #define ORM_CMS_XMEET_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_XMEET_TO_JSON_1(c1) \
         ORM_CMS_XMEET_TO_JSON_ITEM(c1)
        
    #define ORM_CMS_XMEET_TO_JSON_2(c1,c2) \
         ORM_CMS_XMEET_TO_JSON_1(c1); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c2) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_XMEET_TO_JSON_2(c1,c2); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c3) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_XMEET_TO_JSON_3(c1,c2,c3); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c4) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_XMEET_TO_JSON_4(c1,c2,c3,c4); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c5) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_XMEET_TO_JSON_5(c1,c2,c3,c4,c5); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c6) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_XMEET_TO_JSON_6(c1,c2,c3,c4,c5,c6); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c7) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_XMEET_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c8) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_XMEET_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c9) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_XMEET_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c10) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_XMEET_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c11) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_XMEET_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c12) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_XMEET_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c13) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_XMEET_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c14) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_XMEET_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c15) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_XMEET_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15); \
            oss << ','; \
            ORM_CMS_XMEET_TO_JSON_ITEM(c16) 
        
        
    #define ORM_CMS_XMEET_TO_JSON_BODY(...) \
        ORM_CMS_XMEET_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_XMEET_TO_JSON_16,ORM_CMS_XMEET_TO_JSON_15,ORM_CMS_XMEET_TO_JSON_14,ORM_CMS_XMEET_TO_JSON_13,ORM_CMS_XMEET_TO_JSON_12,ORM_CMS_XMEET_TO_JSON_11,ORM_CMS_XMEET_TO_JSON_10,ORM_CMS_XMEET_TO_JSON_9,ORM_CMS_XMEET_TO_JSON_8,ORM_CMS_XMEET_TO_JSON_7,ORM_CMS_XMEET_TO_JSON_6,ORM_CMS_XMEET_TO_JSON_5,ORM_CMS_XMEET_TO_JSON_4,ORM_CMS_XMEET_TO_JSON_3,ORM_CMS_XMEET_TO_JSON_2,ORM_CMS_XMEET_TO_JSON_1 \
         )(__VA_ARGS__)
         
          
    #define ORM_CMS_XMEET_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_XMEET_TO_JSON_CUSTOM_1(n1) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n1)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_2(n1,n2) ORM_CMS_XMEET_TO_JSON_CUSTOM_1(n1) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n2)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_3(n1,n2,n3) ORM_CMS_XMEET_TO_JSON_CUSTOM_2(n1,n2) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n3)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_4(n1,n2,n3,n4) ORM_CMS_XMEET_TO_JSON_CUSTOM_3(n1,n2,n3) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n4)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5) ORM_CMS_XMEET_TO_JSON_CUSTOM_4(n1,n2,n3,n4) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n5)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6) ORM_CMS_XMEET_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n6)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7) ORM_CMS_XMEET_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n7)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8) ORM_CMS_XMEET_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n8)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9) ORM_CMS_XMEET_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n9)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10) ORM_CMS_XMEET_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n10)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11) ORM_CMS_XMEET_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n11)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12) ORM_CMS_XMEET_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n12)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13) ORM_CMS_XMEET_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n13)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14) ORM_CMS_XMEET_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n14)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15) ORM_CMS_XMEET_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n15)

#define ORM_CMS_XMEET_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16) ORM_CMS_XMEET_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15) ORM_CMS_XMEET_TO_JSON_CUSTOM_ITEM(n16)


    #define ORM_CMS_XMEET_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_XMEET_TO_JSON_CUSTOM_##N

    #define ORM_CMS_XMEET_TO_JSON_CUSTOM(...) \
        ORM_CMS_XMEET_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__)


    #define ORM_CMS_XMEET_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::xmeet_info { \
            struct StructName { \
                ORM_CMS_XMEET_PROJ_MEMBERS(__VA_ARGS__) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XMEET_TO_JSON_BODY(__VA_ARGS__); \
                oss << '}'; \
                return oss.str(); \
            } \
            }; \
       }
        
    
    #define ORM_CMS_XMEET_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::xmeet_info { \
            struct StructName { \
                ORM_CMS_XMEET_PROJ_MEMBERS(__VA_ARGS__) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XMEET_TO_JSON_BODY(__VA_ARGS__); \
                ORM_CMS_XMEET_TO_JSON_CUSTOM(ORM_CMS_XMEET_UNWRAP CustomNames)  \
                oss << '}'; \
                return oss.str(); \
            } \
            }; \
       }
        
    
    #define ORM_CMS_XMEET_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::xmeet_info { \
            struct StructName { \
                ORM_CMS_XMEET_PROJ_MEMBERS(__VA_ARGS__) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XMEET_TO_JSON_BODY(__VA_ARGS__); \
                oss << ",\"children\":["; \
                for(unsigned int i=0;i< children.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << children[i].to_json(); \
                }\
                oss << ']'; \
                oss << '}'; \
                return oss.str(); \
                }\
            }; \
       }
        
    
    #define ORM_CMS_XMEET_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::xmeet_info { \
            struct StructName { \
                ORM_CMS_XMEET_PROJ_MEMBERS(__VA_ARGS__) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XMEET_TO_JSON_BODY(__VA_ARGS__); \
                oss << ",\"children\":["; \
                for(unsigned int i=0;i< children.size(); i++){ \
                    if(i>0) oss << ','; \
                    oss << children[i]->to_json(); \
                }\
                oss << ']'; \
                oss << '}'; \
                return oss.str(); \
                }\
            }; \
       }
        
    static constexpr std::array<std::string_view,20> col_names={"xmeetid","userid","xpjid","xtaskid","adminid","title","zhuchi","jilu","meetnotice","meetfiles","address","meettime","regdate","expecttime","endtime","presents","content","postresult","postfiles","jiluphoto"};
	static constexpr std::array<unsigned char,20> col_types={3,3,3,3,3,253,253,253,252,253,253,253,3,3,3,253,252,252,252,253};
	static constexpr std::array<unsigned char,20> col_length={0,0,0,0,0,120,120,120,0,0,120,120,0,0,0,0,0,0,0,0};
	static constexpr std::array<unsigned char,20> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct xmeet_base
{
      xmeet_info::meta data;
    std::vector<xmeet_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<xmeet_info::meta>::iterator begin(){     return record.begin(); }
std::vector<xmeet_info::meta>::iterator end(){     return record.end(); }
std::vector<xmeet_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<xmeet_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="xmeet";
static constexpr std::string_view modelname="Xmeet";

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
 if(colpospppc=='d'){ return 4; }
 if(colpospppc=='s'){ return 10; }
   	 break;
 }
 break;
case 'c':
   	 return 16;
break;
case 'e':
 switch(coln.size()){  
case 7:
   	 return 14;
break;
case 10:
   	 return 13;
break;
 }
 break;
case 'j':
 switch(coln.size()){  
case 4:
   	 return 7;
break;
case 9:
   	 return 19;
break;
 }
 break;
case 'm':
 switch(coln.size()){  
case 8:
   	 return 11;
break;
case 9:
   	 return 9;
break;
case 10:
   	 return 8;
break;
 }
 break;
case 'p':
 switch(coln.size()){  
case 8:
   	 return 15;
break;
case 9:
   	 return 18;
break;
case 10:
   	 return 17;
break;
 }
 break;
case 'r':
   	 return 12;
break;
case 't':
   	 return 5;
break;
case 'u':
   	 return 1;
break;
case 'x':
 switch(coln.size()){  
case 5:
   	 return 2;
break;
case 7:
 if(coln.size()>1&&(coln[1]=='m'||coln[1]=='M')){ return 0; }
 if(coln.size()>1&&(coln[1]=='t'||coln[1]=='T')){ return 3; }
   	 break;
 }
 break;
case 'z':
   	 return 6;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "xmeetid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     xmeet_info::meta metatemp;    
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

   std::string _makeinsertsql(){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<xmeet_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xmeet_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.xmeetid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.xmeetid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.xpjid);
}
if(data.xtaskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.xtaskid);
}
if(data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.adminid);
}
tempsql<<",'"<<stringaddslash(data.title)<<"'";
tempsql<<",'"<<stringaddslash(data.zhuchi)<<"'";
tempsql<<",'"<<stringaddslash(data.jilu)<<"'";
tempsql<<",'"<<stringaddslash(data.meetnotice)<<"'";
tempsql<<",'"<<stringaddslash(data.meetfiles)<<"'";
tempsql<<",'"<<stringaddslash(data.address)<<"'";
tempsql<<",'"<<stringaddslash(data.meettime)<<"'";
if(data.regdate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.regdate);
}
if(data.expecttime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.expecttime);
}
if(data.endtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.endtime);
}
tempsql<<",'"<<stringaddslash(data.presents)<<"'";
tempsql<<",'"<<stringaddslash(data.content)<<"'";
tempsql<<",'"<<stringaddslash(data.postresult)<<"'";
tempsql<<",'"<<stringaddslash(data.postfiles)<<"'";
tempsql<<",'"<<stringaddslash(data.jiluphoto)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const xmeet_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<xmeet_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xmeet_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.xmeetid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.xmeetid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.xpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.xpjid);
}
if(insert_data.xtaskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.xtaskid);
}
if(insert_data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.adminid);
}
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.zhuchi)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.jilu)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.meetnotice)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.meetfiles)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.address)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.meettime)<<"'";
if(insert_data.regdate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.regdate);
}
if(insert_data.expecttime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.expecttime);
}
if(insert_data.endtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.endtime);
}
tempsql<<",'"<<stringaddslash(insert_data.presents)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.content)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.postresult)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.postfiles)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.jiluphoto)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<xmeet_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<xmeet_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xmeet_info::col_names[j];
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


            	if(insert_data[i].xmeetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].xmeetid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].xpjid);
	}
	if(insert_data[i].xtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].xtaskid);
	}
	if(insert_data[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].adminid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].zhuchi)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].jilu)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].meetnotice)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].meetfiles)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].address)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].meettime)<<"'";
	if(insert_data[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].regdate);
	}
	if(insert_data[i].expecttime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].expecttime);
	}
	if(insert_data[i].endtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].endtime);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].presents)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].content)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].postresult)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].postfiles)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].jiluphoto)<<"'";
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

        if(data.xmeetid==0){
	tempsql<<"`xmeetid`=0";
 }else{ 
	tempsql<<"`xmeetid`="<<std::to_string(data.xmeetid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",`xpjid`=0";
 }else{ 
	tempsql<<",`xpjid`="<<std::to_string(data.xpjid);
}
if(data.xtaskid==0){
	tempsql<<",`xtaskid`=0";
 }else{ 
	tempsql<<",`xtaskid`="<<std::to_string(data.xtaskid);
}
if(data.adminid==0){
	tempsql<<",`adminid`=0";
 }else{ 
	tempsql<<",`adminid`="<<std::to_string(data.adminid);
}
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
tempsql<<",`zhuchi`='"<<stringaddslash(data.zhuchi)<<"'";
tempsql<<",`jilu`='"<<stringaddslash(data.jilu)<<"'";
tempsql<<",`meetnotice`='"<<stringaddslash(data.meetnotice)<<"'";
tempsql<<",`meetfiles`='"<<stringaddslash(data.meetfiles)<<"'";
tempsql<<",`address`='"<<stringaddslash(data.address)<<"'";
tempsql<<",`meettime`='"<<stringaddslash(data.meettime)<<"'";
if(data.regdate==0){
	tempsql<<",`regdate`=0";
 }else{ 
	tempsql<<",`regdate`="<<std::to_string(data.regdate);
}
if(data.expecttime==0){
	tempsql<<",`expecttime`=0";
 }else{ 
	tempsql<<",`expecttime`="<<std::to_string(data.expecttime);
}
if(data.endtime==0){
	tempsql<<",`endtime`=0";
 }else{ 
	tempsql<<",`endtime`="<<std::to_string(data.endtime);
}
tempsql<<",`presents`='"<<stringaddslash(data.presents)<<"'";
tempsql<<",`content`='"<<stringaddslash(data.content)<<"'";
tempsql<<",`postresult`='"<<stringaddslash(data.postresult)<<"'";
tempsql<<",`postfiles`='"<<stringaddslash(data.postfiles)<<"'";
tempsql<<",`jiluphoto`='"<<stringaddslash(data.jiluphoto)<<"'";
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
if(data.xmeetid==0){
	tempsql<<"`xmeetid`=0";
 }else{ 
	tempsql<<"`xmeetid`="<<std::to_string(data.xmeetid);
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
if(data.xpjid==0){
	tempsql<<"`xpjid`=0";
 }else{ 
	tempsql<<"`xpjid`="<<std::to_string(data.xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.xtaskid==0){
	tempsql<<"`xtaskid`=0";
 }else{ 
	tempsql<<"`xtaskid`="<<std::to_string(data.xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`zhuchi`='"<<stringaddslash(data.zhuchi)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`jilu`='"<<stringaddslash(data.jilu)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`meetnotice`='"<<stringaddslash(data.meetnotice)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`meetfiles`='"<<stringaddslash(data.meetfiles)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`address`='"<<stringaddslash(data.address)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`meettime`='"<<stringaddslash(data.meettime)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.regdate==0){
	tempsql<<"`regdate`=0";
 }else{ 
	tempsql<<"`regdate`="<<std::to_string(data.regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.expecttime==0){
	tempsql<<"`expecttime`=0";
 }else{ 
	tempsql<<"`expecttime`="<<std::to_string(data.expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.endtime==0){
	tempsql<<"`endtime`=0";
 }else{ 
	tempsql<<"`endtime`="<<std::to_string(data.endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`presents`='"<<stringaddslash(data.presents)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`content`='"<<stringaddslash(data.content)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`postresult`='"<<stringaddslash(data.postresult)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`postfiles`='"<<stringaddslash(data.postfiles)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`jiluphoto`='"<<stringaddslash(data.jiluphoto)<<"'";
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
        for (; j < xmeet_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << xmeet_info::col_names[j];
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
            	if(record[i].xmeetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].xmeetid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xpjid);
	}
	if(record[i].xtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xtaskid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].zhuchi)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jilu)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meetnotice)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meetfiles)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meettime)<<"'";
	if(record[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].regdate);
	}
	if(record[i].expecttime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expecttime);
	}
	if(record[i].endtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].endtime);
	}
	tempsql<<",'"<<stringaddslash(record[i].presents)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].postresult)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].postfiles)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jiluphoto)<<"'";
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
        for (; j < xmeet_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << xmeet_info::col_names[j];
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
            	if(record[i].xmeetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].xmeetid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xpjid);
	}
	if(record[i].xtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xtaskid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].zhuchi)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jilu)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meetnotice)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meetfiles)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meettime)<<"'";
	if(record[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].regdate);
	}
	if(record[i].expecttime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expecttime);
	}
	if(record[i].endtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].endtime);
	}
	tempsql<<",'"<<stringaddslash(record[i].presents)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].postresult)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].postfiles)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jiluphoto)<<"'";
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
            for(jj=0;jj<xmeet_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.xmeetid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xmeetid));
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
if(data.xpjid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xpjid));
}
 break;
 case 3:
if(data.xtaskid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xtaskid));
}
 break;
 case 4:
if(data.adminid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.adminid));
}
 break;
 case 5:
	temparray.push_back(data.title);
 break;
 case 6:
	temparray.push_back(data.zhuchi);
 break;
 case 7:
	temparray.push_back(data.jilu);
 break;
 case 8:
	temparray.push_back(data.meetnotice);
 break;
 case 9:
	temparray.push_back(data.meetfiles);
 break;
 case 10:
	temparray.push_back(data.address);
 break;
 case 11:
	temparray.push_back(data.meettime);
 break;
 case 12:
if(data.regdate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.regdate));
}
 break;
 case 13:
if(data.expecttime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expecttime));
}
 break;
 case 14:
if(data.endtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.endtime));
}
 break;
 case 15:
	temparray.push_back(data.presents);
 break;
 case 16:
	temparray.push_back(data.content);
 break;
 case 17:
	temparray.push_back(data.postresult);
 break;
 case 18:
	temparray.push_back(data.postfiles);
 break;
 case 19:
	temparray.push_back(data.jiluphoto);
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
            for(jj=0;jj<xmeet_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.xmeetid==0){
	tempsql.insert({"xmeetid","0"});
 }else{ 
	tempsql.insert({"xmeetid",std::to_string(data.xmeetid)});
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
if(data.xpjid==0){
	tempsql.insert({"xpjid","0"});
 }else{ 
	tempsql.insert({"xpjid",std::to_string(data.xpjid)});
}
 break;
 case 3:
if(data.xtaskid==0){
	tempsql.insert({"xtaskid","0"});
 }else{ 
	tempsql.insert({"xtaskid",std::to_string(data.xtaskid)});
}
 break;
 case 4:
if(data.adminid==0){
	tempsql.insert({"adminid","0"});
 }else{ 
	tempsql.insert({"adminid",std::to_string(data.adminid)});
}
 break;
 case 5:
	tempsql.insert({"title",data.title});
 break;
 case 6:
	tempsql.insert({"zhuchi",data.zhuchi});
 break;
 case 7:
	tempsql.insert({"jilu",data.jilu});
 break;
 case 8:
	tempsql.insert({"meetnotice",data.meetnotice});
 break;
 case 9:
	tempsql.insert({"meetfiles",data.meetfiles});
 break;
 case 10:
	tempsql.insert({"address",data.address});
 break;
 case 11:
	tempsql.insert({"meettime",data.meettime});
 break;
 case 12:
if(data.regdate==0){
	tempsql.insert({"regdate","0"});
 }else{ 
	tempsql.insert({"regdate",std::to_string(data.regdate)});
}
 break;
 case 13:
if(data.expecttime==0){
	tempsql.insert({"expecttime","0"});
 }else{ 
	tempsql.insert({"expecttime",std::to_string(data.expecttime)});
}
 break;
 case 14:
if(data.endtime==0){
	tempsql.insert({"endtime","0"});
 }else{ 
	tempsql.insert({"endtime",std::to_string(data.endtime)});
}
 break;
 case 15:
	tempsql.insert({"presents",data.presents});
 break;
 case 16:
	tempsql.insert({"content",data.content});
 break;
 case 17:
	tempsql.insert({"postresult",data.postresult});
 break;
 case 18:
	tempsql.insert({"postfiles",data.postfiles});
 break;
 case 19:
	tempsql.insert({"jiluphoto",data.jiluphoto});
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
if(data.xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(data.xmeetid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",\"xpjid\":0";
 }else{ 
	tempsql<<",\"xpjid\":"<<std::to_string(data.xpjid);
}
if(data.xtaskid==0){
	tempsql<<",\"xtaskid\":0";
 }else{ 
	tempsql<<",\"xtaskid\":"<<std::to_string(data.xtaskid);
}
if(data.adminid==0){
	tempsql<<",\"adminid\":0";
 }else{ 
	tempsql<<",\"adminid\":"<<std::to_string(data.adminid);
}
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"zhuchi\":\""<<http::utf8_to_jsonstring(data.zhuchi);
tempsql<<"\"";
tempsql<<",\"jilu\":\""<<http::utf8_to_jsonstring(data.jilu);
tempsql<<"\"";
tempsql<<",\"meetnotice\":\""<<http::utf8_to_jsonstring(data.meetnotice);
tempsql<<"\"";
tempsql<<",\"meetfiles\":\""<<http::utf8_to_jsonstring(data.meetfiles);
tempsql<<"\"";
tempsql<<",\"address\":\""<<http::utf8_to_jsonstring(data.address);
tempsql<<"\"";
tempsql<<",\"meettime\":\""<<http::utf8_to_jsonstring(data.meettime);
tempsql<<"\"";
if(data.regdate==0){
	tempsql<<",\"regdate\":0";
 }else{ 
	tempsql<<",\"regdate\":"<<std::to_string(data.regdate);
}
if(data.expecttime==0){
	tempsql<<",\"expecttime\":0";
 }else{ 
	tempsql<<",\"expecttime\":"<<std::to_string(data.expecttime);
}
if(data.endtime==0){
	tempsql<<",\"endtime\":0";
 }else{ 
	tempsql<<",\"endtime\":"<<std::to_string(data.endtime);
}
tempsql<<",\"presents\":\""<<http::utf8_to_jsonstring(data.presents);
tempsql<<"\"";
tempsql<<",\"content\":\""<<http::utf8_to_jsonstring(data.content);
tempsql<<"\"";
tempsql<<",\"postresult\":\""<<http::utf8_to_jsonstring(data.postresult);
tempsql<<"\"";
tempsql<<",\"postfiles\":\""<<http::utf8_to_jsonstring(data.postfiles);
tempsql<<"\"";
tempsql<<",\"jiluphoto\":\""<<http::utf8_to_jsonstring(data.jiluphoto);
tempsql<<"\"";
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
            for(jj=0;jj<xmeet_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(data.xmeetid);
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
if(data.xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(data.xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(data.xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zhuchi\":\""<<http::utf8_to_jsonstring(data.zhuchi)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jilu\":\""<<http::utf8_to_jsonstring(data.jilu)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetnotice\":\""<<http::utf8_to_jsonstring(data.meetnotice)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetfiles\":\""<<http::utf8_to_jsonstring(data.meetfiles)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(data.address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meettime\":\""<<http::utf8_to_jsonstring(data.meettime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(data.regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.expecttime==0){
	tempsql<<"\"expecttime\":0";
 }else{ 
	tempsql<<"\"expecttime\":"<<std::to_string(data.expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.endtime==0){
	tempsql<<"\"endtime\":0";
 }else{ 
	tempsql<<"\"endtime\":"<<std::to_string(data.endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"presents\":\""<<http::utf8_to_jsonstring(data.presents)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postresult\":\""<<http::utf8_to_jsonstring(data.postresult)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postfiles\":\""<<http::utf8_to_jsonstring(data.postfiles)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jiluphoto\":\""<<http::utf8_to_jsonstring(data.jiluphoto)<<"\"";
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
        xmeet_info::meta metatemp; 
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
			data.xmeetid=std::stoul(set_value_name);
		}catch (...) { 
			data.xmeetid=0;
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
			data.xpjid=std::stoul(set_value_name);
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.xtaskid=std::stoul(set_value_name);
		}catch (...) { 
			data.xtaskid=0;
			 }
			break;
		case 4:
		 try{
			data.adminid=std::stoul(set_value_name);
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 5:
		 try{
			data.title.append(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.zhuchi.append(set_value_name);
		}catch (...) { 
			data.zhuchi.clear();
			 }
			break;
		case 7:
		 try{
			data.jilu.append(set_value_name);
		}catch (...) { 
			data.jilu.clear();
			 }
			break;
		case 8:
		 try{
			data.meetnotice.append(set_value_name);
		}catch (...) { 
			data.meetnotice.clear();
			 }
			break;
		case 9:
		 try{
			data.meetfiles.append(set_value_name);
		}catch (...) { 
			data.meetfiles.clear();
			 }
			break;
		case 10:
		 try{
			data.address.append(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 11:
		 try{
			data.meettime.append(set_value_name);
		}catch (...) { 
			data.meettime.clear();
			 }
			break;
		case 12:
		 try{
			data.regdate=std::stoul(set_value_name);
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 13:
		 try{
			data.expecttime=std::stoul(set_value_name);
		}catch (...) { 
			data.expecttime=0;
			 }
			break;
		case 14:
		 try{
			data.endtime=std::stoul(set_value_name);
		}catch (...) { 
			data.endtime=0;
			 }
			break;
		case 15:
		 try{
			data.presents.append(set_value_name);
		}catch (...) { 
			data.presents.clear();
			 }
			break;
		case 16:
		 try{
			data.content.append(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 17:
		 try{
			data.postresult.append(set_value_name);
		}catch (...) { 
			data.postresult.clear();
			 }
			break;
		case 18:
		 try{
			data.postfiles.append(set_value_name);
		}catch (...) { 
			data.postfiles.clear();
			 }
			break;
		case 19:
		 try{
			data.jiluphoto.append(set_value_name);
		}catch (...) { 
			data.jiluphoto.clear();
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
			data.xmeetid=set_value_name;
		}catch (...) { 
			data.xmeetid=0;
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
			data.xpjid=set_value_name;
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.xtaskid=set_value_name;
		}catch (...) { 
			data.xtaskid=0;
			 }
			break;
		case 4:
		 try{
			data.adminid=set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 5:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.zhuchi=std::to_string(set_value_name);
		}catch (...) { 
			data.zhuchi.clear();
			 }
			break;
		case 7:
		 try{
			data.jilu=std::to_string(set_value_name);
		}catch (...) { 
			data.jilu.clear();
			 }
			break;
		case 8:
		 try{
			data.meetnotice=std::to_string(set_value_name);
		}catch (...) { 
			data.meetnotice.clear();
			 }
			break;
		case 9:
		 try{
			data.meetfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.meetfiles.clear();
			 }
			break;
		case 10:
		 try{
			data.address=std::to_string(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 11:
		 try{
			data.meettime=std::to_string(set_value_name);
		}catch (...) { 
			data.meettime.clear();
			 }
			break;
		case 12:
		 try{
			data.regdate=set_value_name;
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 13:
		 try{
			data.expecttime=set_value_name;
		}catch (...) { 
			data.expecttime=0;
			 }
			break;
		case 14:
		 try{
			data.endtime=set_value_name;
		}catch (...) { 
			data.endtime=0;
			 }
			break;
		case 15:
		 try{
			data.presents=std::to_string(set_value_name);
		}catch (...) { 
			data.presents.clear();
			 }
			break;
		case 16:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 17:
		 try{
			data.postresult=std::to_string(set_value_name);
		}catch (...) { 
			data.postresult.clear();
			 }
			break;
		case 18:
		 try{
			data.postfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.postfiles.clear();
			 }
			break;
		case 19:
		 try{
			data.jiluphoto=std::to_string(set_value_name);
		}catch (...) { 
			data.jiluphoto.clear();
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
			data.xmeetid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xmeetid=0;
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
			data.xpjid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.xtaskid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xtaskid=0;
			 }
			break;
		case 4:
		 try{
			data.adminid=(unsigned int)set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 5:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.zhuchi=std::to_string(set_value_name);
		}catch (...) { 
			data.zhuchi.clear();
			 }
			break;
		case 7:
		 try{
			data.jilu=std::to_string(set_value_name);
		}catch (...) { 
			data.jilu.clear();
			 }
			break;
		case 8:
		 try{
			data.meetnotice=std::to_string(set_value_name);
		}catch (...) { 
			data.meetnotice.clear();
			 }
			break;
		case 9:
		 try{
			data.meetfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.meetfiles.clear();
			 }
			break;
		case 10:
		 try{
			data.address=std::to_string(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 11:
		 try{
			data.meettime=std::to_string(set_value_name);
		}catch (...) { 
			data.meettime.clear();
			 }
			break;
		case 12:
		 try{
			data.regdate=(unsigned int)set_value_name;
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 13:
		 try{
			data.expecttime=(unsigned int)set_value_name;
		}catch (...) { 
			data.expecttime=0;
			 }
			break;
		case 14:
		 try{
			data.endtime=(unsigned int)set_value_name;
		}catch (...) { 
			data.endtime=0;
			 }
			break;
		case 15:
		 try{
			data.presents=std::to_string(set_value_name);
		}catch (...) { 
			data.presents.clear();
			 }
			break;
		case 16:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 17:
		 try{
			data.postresult=std::to_string(set_value_name);
		}catch (...) { 
			data.postresult.clear();
			 }
			break;
		case 18:
		 try{
			data.postfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.postfiles.clear();
			 }
			break;
		case 19:
		 try{
			data.jiluphoto=std::to_string(set_value_name);
		}catch (...) { 
			data.jiluphoto.clear();
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
        for(jj=0;jj<xmeet_info::col_names.size();jj++){
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
if(record[n].xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(record[n].xmeetid);
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
if(record[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(record[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(record[n].xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zhuchi\":\""<<http::utf8_to_jsonstring(record[n].zhuchi)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jilu\":\""<<http::utf8_to_jsonstring(record[n].jilu)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetnotice\":\""<<http::utf8_to_jsonstring(record[n].meetnotice)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetfiles\":\""<<http::utf8_to_jsonstring(record[n].meetfiles)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meettime\":\""<<http::utf8_to_jsonstring(record[n].meettime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(record[n].regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].expecttime==0){
	tempsql<<"\"expecttime\":0";
 }else{ 
	tempsql<<"\"expecttime\":"<<std::to_string(record[n].expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].endtime==0){
	tempsql<<"\"endtime\":0";
 }else{ 
	tempsql<<"\"endtime\":"<<std::to_string(record[n].endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"presents\":\""<<http::utf8_to_jsonstring(record[n].presents)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postresult\":\""<<http::utf8_to_jsonstring(record[n].postresult)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postfiles\":\""<<http::utf8_to_jsonstring(record[n].postfiles)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jiluphoto\":\""<<http::utf8_to_jsonstring(record[n].jiluphoto)<<"\"";
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
   
   std::string to_json(std::function<bool(std::string&,xmeet_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<xmeet_info::col_names.size();jj++){
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
if(record[n].xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(record[n].xmeetid);
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
if(record[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(record[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(record[n].xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zhuchi\":\""<<http::utf8_to_jsonstring(record[n].zhuchi)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jilu\":\""<<http::utf8_to_jsonstring(record[n].jilu)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetnotice\":\""<<http::utf8_to_jsonstring(record[n].meetnotice)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetfiles\":\""<<http::utf8_to_jsonstring(record[n].meetfiles)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meettime\":\""<<http::utf8_to_jsonstring(record[n].meettime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(record[n].regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].expecttime==0){
	tempsql<<"\"expecttime\":0";
 }else{ 
	tempsql<<"\"expecttime\":"<<std::to_string(record[n].expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].endtime==0){
	tempsql<<"\"endtime\":0";
 }else{ 
	tempsql<<"\"endtime\":"<<std::to_string(record[n].endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"presents\":\""<<http::utf8_to_jsonstring(record[n].presents)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postresult\":\""<<http::utf8_to_jsonstring(record[n].postresult)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postfiles\":\""<<http::utf8_to_jsonstring(record[n].postfiles)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jiluphoto\":\""<<http::utf8_to_jsonstring(record[n].jiluphoto)<<"\"";
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
   long long getPK(){  return data.xmeetid; } 
 void setPK(long long val){  data.xmeetid=val;} 
 unsigned  int  getXmeetid(){  return data.xmeetid; } 
 void setXmeetid( unsigned  int  val){  data.xmeetid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getXpjid(){  return data.xpjid; } 
 void setXpjid( unsigned  int  val){  data.xpjid=val;} 

 unsigned  int  getXtaskid(){  return data.xtaskid; } 
 void setXtaskid( unsigned  int  val){  data.xtaskid=val;} 

 unsigned  int  getAdminid(){  return data.adminid; } 
 void setAdminid( unsigned  int  val){  data.adminid=val;} 

 std::string  getTitle(){  return data.title; } 
 std::string & getRefTitle(){  return std::ref(data.title); } 
 void setTitle( std::string  &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

 std::string  getZhuchi(){  return data.zhuchi; } 
 std::string & getRefZhuchi(){  return std::ref(data.zhuchi); } 
 void setZhuchi( std::string  &val){  data.zhuchi=val;} 
 void setZhuchi(std::string_view val){  data.zhuchi=val;} 

 std::string  getJilu(){  return data.jilu; } 
 std::string & getRefJilu(){  return std::ref(data.jilu); } 
 void setJilu( std::string  &val){  data.jilu=val;} 
 void setJilu(std::string_view val){  data.jilu=val;} 

 std::string  getMeetnotice(){  return data.meetnotice; } 
 std::string & getRefMeetnotice(){  return std::ref(data.meetnotice); } 
 void setMeetnotice( std::string  &val){  data.meetnotice=val;} 
 void setMeetnotice(std::string_view val){  data.meetnotice=val;} 

 std::string  getMeetfiles(){  return data.meetfiles; } 
 std::string & getRefMeetfiles(){  return std::ref(data.meetfiles); } 
 void setMeetfiles( std::string  &val){  data.meetfiles=val;} 
 void setMeetfiles(std::string_view val){  data.meetfiles=val;} 

 std::string  getAddress(){  return data.address; } 
 std::string & getRefAddress(){  return std::ref(data.address); } 
 void setAddress( std::string  &val){  data.address=val;} 
 void setAddress(std::string_view val){  data.address=val;} 

 std::string  getMeettime(){  return data.meettime; } 
 std::string & getRefMeettime(){  return std::ref(data.meettime); } 
 void setMeettime( std::string  &val){  data.meettime=val;} 
 void setMeettime(std::string_view val){  data.meettime=val;} 

 unsigned  int  getRegdate(){  return data.regdate; } 
 void setRegdate( unsigned  int  val){  data.regdate=val;} 

 unsigned  int  getExpecttime(){  return data.expecttime; } 
 void setExpecttime( unsigned  int  val){  data.expecttime=val;} 

 unsigned  int  getEndtime(){  return data.endtime; } 
 void setEndtime( unsigned  int  val){  data.endtime=val;} 

 std::string  getPresents(){  return data.presents; } 
 std::string & getRefPresents(){  return std::ref(data.presents); } 
 void setPresents( std::string  &val){  data.presents=val;} 
 void setPresents(std::string_view val){  data.presents=val;} 

 std::string  getContent(){  return data.content; } 
 std::string & getRefContent(){  return std::ref(data.content); } 
 void setContent( std::string  &val){  data.content=val;} 
 void setContent(std::string_view val){  data.content=val;} 

 std::string  getPostresult(){  return data.postresult; } 
 std::string & getRefPostresult(){  return std::ref(data.postresult); } 
 void setPostresult( std::string  &val){  data.postresult=val;} 
 void setPostresult(std::string_view val){  data.postresult=val;} 

 std::string  getPostfiles(){  return data.postfiles; } 
 std::string & getRefPostfiles(){  return std::ref(data.postfiles); } 
 void setPostfiles( std::string  &val){  data.postfiles=val;} 
 void setPostfiles(std::string_view val){  data.postfiles=val;} 

 std::string  getJiluphoto(){  return data.jiluphoto; } 
 std::string & getRefJiluphoto(){  return std::ref(data.jiluphoto); } 
 void setJiluphoto( std::string  &val){  data.jiluphoto=val;} 
 void setJiluphoto(std::string_view val){  data.jiluphoto=val;} 

xmeet_info::meta getnewData(){
 	 struct xmeet_info::meta newdata;
	 return newdata; 
} 
xmeet_info::meta getData(){
 	 return data; 
} 
std::vector<xmeet_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<xmeet_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<xmeet_info::col_names.size();jj++){
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
if(tree_data[n].xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(tree_data[n].xmeetid);
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
if(tree_data[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(tree_data[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(tree_data[n].xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(tree_data[n].adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zhuchi\":\""<<http::utf8_to_jsonstring(tree_data[n].zhuchi)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jilu\":\""<<http::utf8_to_jsonstring(tree_data[n].jilu)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetnotice\":\""<<http::utf8_to_jsonstring(tree_data[n].meetnotice)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetfiles\":\""<<http::utf8_to_jsonstring(tree_data[n].meetfiles)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(tree_data[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meettime\":\""<<http::utf8_to_jsonstring(tree_data[n].meettime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(tree_data[n].regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expecttime==0){
	tempsql<<"\"expecttime\":0";
 }else{ 
	tempsql<<"\"expecttime\":"<<std::to_string(tree_data[n].expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].endtime==0){
	tempsql<<"\"endtime\":0";
 }else{ 
	tempsql<<"\"endtime\":"<<std::to_string(tree_data[n].endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"presents\":\""<<http::utf8_to_jsonstring(tree_data[n].presents)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postresult\":\""<<http::utf8_to_jsonstring(tree_data[n].postresult)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postfiles\":\""<<http::utf8_to_jsonstring(tree_data[n].postfiles)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jiluphoto\":\""<<http::utf8_to_jsonstring(tree_data[n].jiluphoto)<<"\"";
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
   
   std::string tree_tojson(const std::vector<xmeet_info::meta_tree> &tree_data,std::function<bool(std::string&,const xmeet_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<xmeet_info::col_names.size();jj++){
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
if(tree_data[n].xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(tree_data[n].xmeetid);
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
if(tree_data[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(tree_data[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(tree_data[n].xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(tree_data[n].adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zhuchi\":\""<<http::utf8_to_jsonstring(tree_data[n].zhuchi)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jilu\":\""<<http::utf8_to_jsonstring(tree_data[n].jilu)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetnotice\":\""<<http::utf8_to_jsonstring(tree_data[n].meetnotice)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetfiles\":\""<<http::utf8_to_jsonstring(tree_data[n].meetfiles)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(tree_data[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meettime\":\""<<http::utf8_to_jsonstring(tree_data[n].meettime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(tree_data[n].regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expecttime==0){
	tempsql<<"\"expecttime\":0";
 }else{ 
	tempsql<<"\"expecttime\":"<<std::to_string(tree_data[n].expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].endtime==0){
	tempsql<<"\"endtime\":0";
 }else{ 
	tempsql<<"\"endtime\":"<<std::to_string(tree_data[n].endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"presents\":\""<<http::utf8_to_jsonstring(tree_data[n].presents)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postresult\":\""<<http::utf8_to_jsonstring(tree_data[n].postresult)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postfiles\":\""<<http::utf8_to_jsonstring(tree_data[n].postfiles)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jiluphoto\":\""<<http::utf8_to_jsonstring(tree_data[n].jiluphoto)<<"\"";
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
   
    template<xmeet_info::cols KeyCol, xmeet_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>()));
        using ValType = decltype(xmeet_info::getField<ValCol>(std::declval<const xmeet_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(xmeet_info::getField<KeyCol>(iter), xmeet_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xmeet_info::cols KeyCol, xmeet_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>())), 
            decltype(xmeet_info::getField<ValCol>(std::declval<const xmeet_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>())), 
                    decltype(xmeet_info::getField<ValCol>(std::declval<const xmeet_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>()));
        using ValType = decltype(xmeet_info::getField<ValCol>(std::declval<const xmeet_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(xmeet_info::getField<KeyCol>(iter), xmeet_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xmeet_info::getField<KeyCol>(iter), xmeet_info::getField<ValCol>(iter))) {
                    result.emplace(xmeet_info::getField<KeyCol>(iter), xmeet_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xmeet_info::cols KeyCol, xmeet_info::cols ValCol> 
    auto get_vec_cols()
    {
        using KeyType = decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>()));
        using ValType = decltype(xmeet_info::getField<ValCol>(std::declval<const xmeet_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(xmeet_info::getField<KeyCol>(iter), xmeet_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xmeet_info::cols KeyCol, xmeet_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>())), 
            decltype(xmeet_info::getField<ValCol>(std::declval<const xmeet_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>())), 
                    decltype(xmeet_info::getField<ValCol>(std::declval<const xmeet_info::meta&>()))>, bool>
    auto get_vec_cols(Callback&& callback)
    {
        using KeyType = decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>()));
        using ValType = decltype(xmeet_info::getField<ValCol>(std::declval<const xmeet_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(xmeet_info::getField<KeyCol>(iter), xmeet_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xmeet_info::getField<KeyCol>(iter), xmeet_info::getField<ValCol>(iter))) {
                    result.emplace_back(xmeet_info::getField<KeyCol>(iter), xmeet_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xmeet_info::cols KeyCol>
    auto get_vec_col()
    {
        using KeyType = decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(xmeet_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_col<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xmeet_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>()))>, bool>
    auto get_vec_col(Callback&& callback)
    {
        using KeyType = decltype(xmeet_info::getField<KeyCol>(std::declval<const xmeet_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(xmeet_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xmeet_info::getField<KeyCol>(iter))) {
                    result.emplace_back(xmeet_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xmeet_info::cols Col>
        requires requires(std::ostream& os, decltype(xmeet_info::getField<Col>(std::declval<const xmeet_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << xmeet_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<xmeet_info::cols Col>
        requires requires(std::ostream& os, decltype(xmeet_info::getField<Col>(std::declval<const xmeet_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << xmeet_info::getField<Col>(iter); 
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
   