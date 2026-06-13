#ifndef ORM_CMS_TOPICSETBASEMATA_H
#define ORM_CMS_TOPICSETBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sat, 13 Jun 2026 16:09:21 GMT
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

namespace topicset_info
{
 
    enum class cols : unsigned char 
    {
		topicsetid = 0,
		userid = 1,
		topicid = 2,
		linktopicid = 3,
		linkrownum = 4,
		sidetype = 5,
		sidename = 6,
		txtcontent = 7,
		sort = 8,

    };

    struct meta
    {
		 unsigned  int  topicsetid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  topicid = 0; ///**/
		 unsigned  int  linktopicid = 0; ///**/
		 unsigned  char  linkrownum = 0; ///**/
		 unsigned  char  sidetype = 0; ///*showtype*/
		 std::string  sidename = ""; ///**/
		 std::string  txtcontent = ""; ///**/
		 unsigned  int  sort = 0; ///**/
	};
  
    struct meta_tree
    {
		 unsigned  int  topicsetid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  topicid = 0; ///**/
		 unsigned  int  linktopicid = 0; ///**/
		 unsigned  char  linkrownum = 0; ///**/
		 unsigned  char  sidetype = 0; ///*showtype*/
		 std::string  sidename = ""; ///**/
		 std::string  txtcontent = ""; ///**/
		 unsigned  int  sort = 0; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  topicsetid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  topicid = 0; ///**/
		 unsigned  int  linktopicid = 0; ///**/
		 unsigned  char  linkrownum = 0; ///**/
		 unsigned  char  sidetype = 0; ///*showtype*/
		 std::string  sidename = ""; ///**/
		 std::string  txtcontent = ""; ///**/
		 unsigned  int  sort = 0; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::topicsetid) { 
		 return m.topicsetid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::topicid) { 
		 return m.topicid;
		} else if constexpr (Col == cols::linktopicid) { 
		 return m.linktopicid;
		} else if constexpr (Col == cols::linkrownum) { 
		 return m.linkrownum;
		} else if constexpr (Col == cols::sidetype) { 
		 return m.sidetype;
		} else if constexpr (Col == cols::sidename) { 
		 return m.sidename;
		} else if constexpr (Col == cols::txtcontent) { 
		 return m.txtcontent;
		} else if constexpr (Col == cols::sort) { 
		 return m.sort;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using topicsetid =  unsigned  int ;
		using userid =  unsigned  int ;
		using topicid =  unsigned  int ;
		using linktopicid =  unsigned  int ;
		using linkrownum =  unsigned  char ;
		using sidetype =  unsigned  char ;
		using sidename =  std::string ;
		using txtcontent =  std::string ;
		using sort =  unsigned  int ;

    }

    
    #define ORM_CMS_TOPICSET_EXPAND(x) x 
    
    #define ORM_CMS_TOPICSET_META_FIELD_TYPE(col) \
         orm::cms::topicset_info::type::col 
    
    #define ORM_CMS_TOPICSET_PROJ_MEMBER(col) \
          ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_1(c1) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_1( c1)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c2))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c3))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c4))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c5))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c6))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c7))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c8))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c9))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c10))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c11))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c12))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c13))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c14))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c15))
         
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBER(c16))
         
    #define ORM_CMS_TOPICSET_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_TOPICSET_PROJ_MEMBERS(...) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_16, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_15, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_14, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_13, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_12, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_11, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_10, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_9, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_8, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_7, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_6, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_5, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_4, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_3, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_2, \
            ORM_CMS_TOPICSET_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_TOPICSET_COUNT(...) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_TOPICSET_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_TOPICSET_TO_JSON_1(c1) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_TOPICSET_TO_JSON_2(c1,c2) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_TOPICSET_TO_JSON_BODY(...) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_TOPICSET_TO_JSON_16,ORM_CMS_TOPICSET_TO_JSON_15,ORM_CMS_TOPICSET_TO_JSON_14,ORM_CMS_TOPICSET_TO_JSON_13,ORM_CMS_TOPICSET_TO_JSON_12,ORM_CMS_TOPICSET_TO_JSON_11,ORM_CMS_TOPICSET_TO_JSON_10,ORM_CMS_TOPICSET_TO_JSON_9,ORM_CMS_TOPICSET_TO_JSON_8,ORM_CMS_TOPICSET_TO_JSON_7,ORM_CMS_TOPICSET_TO_JSON_6,ORM_CMS_TOPICSET_TO_JSON_5,ORM_CMS_TOPICSET_TO_JSON_4,ORM_CMS_TOPICSET_TO_JSON_3,ORM_CMS_TOPICSET_TO_JSON_2,ORM_CMS_TOPICSET_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_TOPICSET_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_1(n1)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_1(n1)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_TOPICSET_CAT(a, b) ORM_CMS_TOPICSET_CAT_(a, b)
    #define ORM_CMS_TOPICSET_CAT_(a, b) a##b

    #define ORM_CMS_TOPICSET_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_TOPICSET_CAT(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_TOPICSET_TO_JSON_CUSTOM(...) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_TOPICSET_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length); \
        return; \
    }
    
    
    #define ORM_CMS_TOPICSET_SET_VAL_1(c1) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_TOPICSET_SET_VAL_2(c1,c2) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_1(c1)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_2(c1,c2)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_TOPICSET_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_TOPICSET_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_TOPICSET_CAT(ORM_CMS_TOPICSET_SET_VAL_, N)
    
    
    #define ORM_CMS_TOPICSET_SET_VAL_FIELDS(...) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_TOPICSET_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_FIELDS(ORM_CMS_TOPICSET_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_TOPICSET_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::topicset_info { \
            struct StructName { \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val([[maybe_unused]] const std::string& _orm_name, \
                        [[maybe_unused]] const unsigned char* _buf,[[maybe_unused]] size_t _length) { \
                        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_TOPICSET_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::topicset_info { \
            struct StructName { \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM(ORM_CMS_TOPICSET_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val([[maybe_unused]] const std::string& _orm_name, \
                        [[maybe_unused]] const unsigned char* _buf,[[maybe_unused]] size_t _length) { \
                        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_TOPICSET_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::topicset_info { \
            struct StructName { \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_BODY(__VA_ARGS__)); \
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
                void set_val([[maybe_unused]] const std::string& _orm_name, \
                        [[maybe_unused]] const unsigned char* _buf,[[maybe_unused]] size_t _length) { \
                        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_TOPICSET_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::topicset_info { \
            struct StructName { \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_BODY(__VA_ARGS__)); \
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
                void set_val([[maybe_unused]] const std::string& _orm_name, \
                        [[maybe_unused]] const unsigned char* _buf,[[maybe_unused]] size_t _length) { \
                        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_TOPICSET_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::topicset_info { \
            struct StructName { \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_TO_JSON_CUSTOM(ORM_CMS_TOPICSET_UNWRAP CustomNames));  \
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
                void set_val([[maybe_unused]] const std::string& _orm_name, \
                        [[maybe_unused]] const unsigned char* _buf,[[maybe_unused]] size_t _length) { \
                        ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_TOPICSET_EXPAND(ORM_CMS_TOPICSET_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,9> col_names={"topicsetid","userid","topicid","linktopicid","linkrownum","sidetype","sidename","txtcontent","sort"};
	static constexpr std::array<unsigned char,9> col_types={3,3,3,3,1,1,253,252,3};
	static constexpr std::array<unsigned char,9> col_length={0,0,0,0,0,0,80,0,0};
	static constexpr std::array<unsigned char,9> col_decimals={0,0,0,0,0,0,0,0,0};

}

struct topicset_base
{
      topicset_info::meta data;
    std::vector<topicset_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<topicset_info::meta>::iterator begin(){     return record.begin(); }
std::vector<topicset_info::meta>::iterator end(){     return record.end(); }
std::vector<topicset_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<topicset_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="topicset";
static constexpr std::string_view modelname="Topicset";

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


         case 'l':
 switch(coln.size()){  
case 10:
   	 return 4;
break;
case 11:
   	 return 3;
break;
 }
 break;
case 's':
 switch(coln.size()){  
case 4:
   	 return 8;
break;
case 8:
 if(coln.size()>4&&(coln[4]=='n'||coln[4]=='N')){ return 6; }
 if(coln.size()>4&&(coln[4]=='t'||coln[4]=='T')){ return 5; }
   	 break;
 }
 break;
case 't':
 switch(coln.size()){  
case 7:
   	 return 2;
break;
case 10:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 0; }
 if(colpospppc=='t'){ return 7; }
   	 break;
 }
 break;
case 'u':
   	 return 1;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "topicsetid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     topicset_info::meta metatemp;    
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
        for(;j<topicset_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<topicset_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.topicsetid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.topicsetid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.topicid);
}
if(data.linktopicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.linktopicid);
}
if(data.linkrownum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.linkrownum);
}
if(data.sidetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sidetype);
}
tempsql<<",'"<<stringaddslash(data.sidename)<<"'";
tempsql<<",'"<<stringaddslash(data.txtcontent)<<"'";
if(data.sort==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sort);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const topicset_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<topicset_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<topicset_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.topicsetid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.topicsetid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.topicid);
}
if(insert_data.linktopicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.linktopicid);
}
if(insert_data.linkrownum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.linkrownum);
}
if(insert_data.sidetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sidetype);
}
tempsql<<",'"<<stringaddslash(insert_data.sidename)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.txtcontent)<<"'";
if(insert_data.sort==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sort);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<topicset_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<topicset_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<topicset_info::col_names[j];
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


            	if(insert_data[i].topicsetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].topicsetid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].topicid);
	}
	if(insert_data[i].linktopicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].linktopicid);
	}
	if(insert_data[i].linkrownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].linkrownum);
	}
	if(insert_data[i].sidetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sidetype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].sidename)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].txtcontent)<<"'";
	if(insert_data[i].sort==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sort);
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

        if(data.topicsetid==0){
	tempsql<<"`topicsetid`=0";
 }else{ 
	tempsql<<"`topicsetid`="<<std::to_string(data.topicsetid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",`topicid`=0";
 }else{ 
	tempsql<<",`topicid`="<<std::to_string(data.topicid);
}
if(data.linktopicid==0){
	tempsql<<",`linktopicid`=0";
 }else{ 
	tempsql<<",`linktopicid`="<<std::to_string(data.linktopicid);
}
if(data.linkrownum==0){
	tempsql<<",`linkrownum`=0";
 }else{ 
	tempsql<<",`linkrownum`="<<std::to_string(data.linkrownum);
}
if(data.sidetype==0){
	tempsql<<",`sidetype`=0";
 }else{ 
	tempsql<<",`sidetype`="<<std::to_string(data.sidetype);
}
tempsql<<",`sidename`='"<<stringaddslash(data.sidename)<<"'";
tempsql<<",`txtcontent`='"<<stringaddslash(data.txtcontent)<<"'";
if(data.sort==0){
	tempsql<<",`sort`=0";
 }else{ 
	tempsql<<",`sort`="<<std::to_string(data.sort);
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
if(data.topicsetid==0){
	tempsql<<"`topicsetid`=0";
 }else{ 
	tempsql<<"`topicsetid`="<<std::to_string(data.topicsetid);
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
if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.linktopicid==0){
	tempsql<<"`linktopicid`=0";
 }else{ 
	tempsql<<"`linktopicid`="<<std::to_string(data.linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.linkrownum==0){
	tempsql<<"`linkrownum`=0";
 }else{ 
	tempsql<<"`linkrownum`="<<std::to_string(data.linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.sidetype==0){
	tempsql<<"`sidetype`=0";
 }else{ 
	tempsql<<"`sidetype`="<<std::to_string(data.sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sidename`='"<<stringaddslash(data.sidename)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`txtcontent`='"<<stringaddslash(data.txtcontent)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.sort==0){
	tempsql<<"`sort`=0";
 }else{ 
	tempsql<<"`sort`="<<std::to_string(data.sort);
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
        for (; j < topicset_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << topicset_info::col_names[j];
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
            	if(record[i].topicsetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].topicsetid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].linktopicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].linktopicid);
	}
	if(record[i].linkrownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].linkrownum);
	}
	if(record[i].sidetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sidetype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sidename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].txtcontent)<<"'";
	if(record[i].sort==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sort);
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
        for (; j < topicset_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << topicset_info::col_names[j];
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
            	if(record[i].topicsetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].topicsetid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].linktopicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].linktopicid);
	}
	if(record[i].linkrownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].linkrownum);
	}
	if(record[i].sidetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sidetype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sidename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].txtcontent)<<"'";
	if(record[i].sort==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sort);
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
            for(jj=0;jj<topicset_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.topicsetid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicsetid));
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
if(data.topicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicid));
}
 break;
 case 3:
if(data.linktopicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.linktopicid));
}
 break;
 case 4:
if(data.linkrownum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.linkrownum));
}
 break;
 case 5:
if(data.sidetype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sidetype));
}
 break;
 case 6:
	temparray.push_back(data.sidename);
 break;
 case 7:
	temparray.push_back(data.txtcontent);
 break;
 case 8:
if(data.sort==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sort));
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
            for(jj=0;jj<topicset_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.topicsetid==0){
	tempsql.insert({"topicsetid","0"});
 }else{ 
	tempsql.insert({"topicsetid",std::to_string(data.topicsetid)});
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
if(data.topicid==0){
	tempsql.insert({"topicid","0"});
 }else{ 
	tempsql.insert({"topicid",std::to_string(data.topicid)});
}
 break;
 case 3:
if(data.linktopicid==0){
	tempsql.insert({"linktopicid","0"});
 }else{ 
	tempsql.insert({"linktopicid",std::to_string(data.linktopicid)});
}
 break;
 case 4:
if(data.linkrownum==0){
	tempsql.insert({"linkrownum","0"});
 }else{ 
	tempsql.insert({"linkrownum",std::to_string(data.linkrownum)});
}
 break;
 case 5:
if(data.sidetype==0){
	tempsql.insert({"sidetype","0"});
 }else{ 
	tempsql.insert({"sidetype",std::to_string(data.sidetype)});
}
 break;
 case 6:
	tempsql.insert({"sidename",data.sidename});
 break;
 case 7:
	tempsql.insert({"txtcontent",data.txtcontent});
 break;
 case 8:
if(data.sort==0){
	tempsql.insert({"sort","0"});
 }else{ 
	tempsql.insert({"sort",std::to_string(data.sort)});
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
if(data.topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(data.topicsetid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",\"topicid\":0";
 }else{ 
	tempsql<<",\"topicid\":"<<std::to_string(data.topicid);
}
if(data.linktopicid==0){
	tempsql<<",\"linktopicid\":0";
 }else{ 
	tempsql<<",\"linktopicid\":"<<std::to_string(data.linktopicid);
}
if(data.linkrownum==0){
	tempsql<<",\"linkrownum\":0";
 }else{ 
	tempsql<<",\"linkrownum\":"<<std::to_string(data.linkrownum);
}
if(data.sidetype==0){
	tempsql<<",\"sidetype\":0";
 }else{ 
	tempsql<<",\"sidetype\":"<<std::to_string(data.sidetype);
}
tempsql<<",\"sidename\":\""<<http::utf8_to_jsonstring(data.sidename);
tempsql<<"\"";
tempsql<<",\"txtcontent\":\""<<http::utf8_to_jsonstring(data.txtcontent);
tempsql<<"\"";
if(data.sort==0){
	tempsql<<",\"sort\":0";
 }else{ 
	tempsql<<",\"sort\":"<<std::to_string(data.sort);
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
            for(jj=0;jj<topicset_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(data.topicsetid);
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
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.linktopicid==0){
	tempsql<<"\"linktopicid\":0";
 }else{ 
	tempsql<<"\"linktopicid\":"<<std::to_string(data.linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.linkrownum==0){
	tempsql<<"\"linkrownum\":0";
 }else{ 
	tempsql<<"\"linkrownum\":"<<std::to_string(data.linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.sidetype==0){
	tempsql<<"\"sidetype\":0";
 }else{ 
	tempsql<<"\"sidetype\":"<<std::to_string(data.sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sidename\":\""<<http::utf8_to_jsonstring(data.sidename)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"txtcontent\":\""<<http::utf8_to_jsonstring(data.txtcontent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.sort==0){
	tempsql<<"\"sort\":0";
 }else{ 
	tempsql<<"\"sort\":"<<std::to_string(data.sort);
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
        topicset_info::meta metatemp; 
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
			data.topicsetid=std::stoul(set_value_name);
		}catch (...) { 
			data.topicsetid=0;
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
			data.topicid=std::stoul(set_value_name);
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.linktopicid=std::stoul(set_value_name);
		}catch (...) { 
			data.linktopicid=0;
			 }
			break;
		case 4:
		 try{
			data.linkrownum=std::stoi(set_value_name);
		}catch (...) { 
			data.linkrownum=0;
			 }
			break;
		case 5:
		 try{
			data.sidetype=std::stoi(set_value_name);
		}catch (...) { 
			data.sidetype=0;
			 }
			break;
		case 6:
		 try{
			data.sidename.append(set_value_name);
		}catch (...) { 
			data.sidename.clear();
			 }
			break;
		case 7:
		 try{
			data.txtcontent.append(set_value_name);
		}catch (...) { 
			data.txtcontent.clear();
			 }
			break;
		case 8:
		 try{
			data.sort=std::stoul(set_value_name);
		}catch (...) { 
			data.sort=0;
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
			data.topicsetid=set_value_name;
		}catch (...) { 
			data.topicsetid=0;
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
			data.topicid=set_value_name;
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.linktopicid=set_value_name;
		}catch (...) { 
			data.linktopicid=0;
			 }
			break;
		case 4:
		 try{
			data.linkrownum=set_value_name;
		}catch (...) { 
			data.linkrownum=0;
			 }
			break;
		case 5:
		 try{
			data.sidetype=set_value_name;
		}catch (...) { 
			data.sidetype=0;
			 }
			break;
		case 6:
		 try{
			data.sidename=std::to_string(set_value_name);
		}catch (...) { 
			data.sidename.clear();
			 }
			break;
		case 7:
		 try{
			data.txtcontent=std::to_string(set_value_name);
		}catch (...) { 
			data.txtcontent.clear();
			 }
			break;
		case 8:
		 try{
			data.sort=set_value_name;
		}catch (...) { 
			data.sort=0;
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
			data.topicsetid=(unsigned int)set_value_name;
		}catch (...) { 
			data.topicsetid=0;
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
			data.topicid=(unsigned int)set_value_name;
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.linktopicid=(unsigned int)set_value_name;
		}catch (...) { 
			data.linktopicid=0;
			 }
			break;
		case 4:
		 try{
			data.linkrownum=(int)set_value_name;
		}catch (...) { 
			data.linkrownum=0;
			 }
			break;
		case 5:
		 try{
			data.sidetype=(int)set_value_name;
		}catch (...) { 
			data.sidetype=0;
			 }
			break;
		case 6:
		 try{
			data.sidename=std::to_string(set_value_name);
		}catch (...) { 
			data.sidename.clear();
			 }
			break;
		case 7:
		 try{
			data.txtcontent=std::to_string(set_value_name);
		}catch (...) { 
			data.txtcontent.clear();
			 }
			break;
		case 8:
		 try{
			data.sort=(unsigned int)set_value_name;
		}catch (...) { 
			data.sort=0;
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
        for(jj=0;jj<topicset_info::col_names.size();jj++){
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
if(record[n].topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(record[n].topicsetid);
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
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].linktopicid==0){
	tempsql<<"\"linktopicid\":0";
 }else{ 
	tempsql<<"\"linktopicid\":"<<std::to_string(record[n].linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].linkrownum==0){
	tempsql<<"\"linkrownum\":0";
 }else{ 
	tempsql<<"\"linkrownum\":"<<std::to_string(record[n].linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].sidetype==0){
	tempsql<<"\"sidetype\":0";
 }else{ 
	tempsql<<"\"sidetype\":"<<std::to_string(record[n].sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sidename\":\""<<http::utf8_to_jsonstring(record[n].sidename)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"txtcontent\":\""<<http::utf8_to_jsonstring(record[n].txtcontent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].sort==0){
	tempsql<<"\"sort\":0";
 }else{ 
	tempsql<<"\"sort\":"<<std::to_string(record[n].sort);
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
   
   std::string to_json(std::function<bool(std::string&,topicset_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<topicset_info::col_names.size();jj++){
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
if(record[n].topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(record[n].topicsetid);
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
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].linktopicid==0){
	tempsql<<"\"linktopicid\":0";
 }else{ 
	tempsql<<"\"linktopicid\":"<<std::to_string(record[n].linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].linkrownum==0){
	tempsql<<"\"linkrownum\":0";
 }else{ 
	tempsql<<"\"linkrownum\":"<<std::to_string(record[n].linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].sidetype==0){
	tempsql<<"\"sidetype\":0";
 }else{ 
	tempsql<<"\"sidetype\":"<<std::to_string(record[n].sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sidename\":\""<<http::utf8_to_jsonstring(record[n].sidename)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"txtcontent\":\""<<http::utf8_to_jsonstring(record[n].txtcontent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].sort==0){
	tempsql<<"\"sort\":0";
 }else{ 
	tempsql<<"\"sort\":"<<std::to_string(record[n].sort);
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
   long long getPK(){  return data.topicsetid; } 
 void setPK(long long val){  data.topicsetid=val;} 
 unsigned  int  getTopicsetid(){  return data.topicsetid; } 
 void setTopicsetid( unsigned  int  val){  data.topicsetid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getTopicid(){  return data.topicid; } 
 void setTopicid( unsigned  int  val){  data.topicid=val;} 

 unsigned  int  getLinktopicid(){  return data.linktopicid; } 
 void setLinktopicid( unsigned  int  val){  data.linktopicid=val;} 

 unsigned  char  getLinkrownum(){  return data.linkrownum; } 
 void setLinkrownum( unsigned  char  val){  data.linkrownum=val;} 

 unsigned  char  getSidetype(){  return data.sidetype; } 
 void setSidetype( unsigned  char  val){  data.sidetype=val;} 

 std::string  getSidename(){  return data.sidename; } 
 std::string & getRefSidename(){  return std::ref(data.sidename); } 
 void setSidename( std::string  &val){  data.sidename=val;} 
 void setSidename(std::string_view val){  data.sidename=val;} 

 std::string  getTxtcontent(){  return data.txtcontent; } 
 std::string & getRefTxtcontent(){  return std::ref(data.txtcontent); } 
 void setTxtcontent( std::string  &val){  data.txtcontent=val;} 
 void setTxtcontent(std::string_view val){  data.txtcontent=val;} 

 unsigned  int  getSort(){  return data.sort; } 
 void setSort( unsigned  int  val){  data.sort=val;} 

topicset_info::meta getnewData(){
 	 struct topicset_info::meta newdata;
	 return newdata; 
} 
topicset_info::meta getData(){
 	 return data; 
} 
std::vector<topicset_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<topicset_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<topicset_info::col_names.size();jj++){
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
if(tree_data[n].topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(tree_data[n].topicsetid);
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
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].linktopicid==0){
	tempsql<<"\"linktopicid\":0";
 }else{ 
	tempsql<<"\"linktopicid\":"<<std::to_string(tree_data[n].linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].linkrownum==0){
	tempsql<<"\"linkrownum\":0";
 }else{ 
	tempsql<<"\"linkrownum\":"<<std::to_string(tree_data[n].linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sidetype==0){
	tempsql<<"\"sidetype\":0";
 }else{ 
	tempsql<<"\"sidetype\":"<<std::to_string(tree_data[n].sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sidename\":\""<<http::utf8_to_jsonstring(tree_data[n].sidename)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"txtcontent\":\""<<http::utf8_to_jsonstring(tree_data[n].txtcontent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sort==0){
	tempsql<<"\"sort\":0";
 }else{ 
	tempsql<<"\"sort\":"<<std::to_string(tree_data[n].sort);
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
   
   std::string tree_tojson(const std::vector<topicset_info::meta_tree> &tree_data,std::function<bool(std::string&,const topicset_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<topicset_info::col_names.size();jj++){
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
if(tree_data[n].topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(tree_data[n].topicsetid);
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
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].linktopicid==0){
	tempsql<<"\"linktopicid\":0";
 }else{ 
	tempsql<<"\"linktopicid\":"<<std::to_string(tree_data[n].linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].linkrownum==0){
	tempsql<<"\"linkrownum\":0";
 }else{ 
	tempsql<<"\"linkrownum\":"<<std::to_string(tree_data[n].linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sidetype==0){
	tempsql<<"\"sidetype\":0";
 }else{ 
	tempsql<<"\"sidetype\":"<<std::to_string(tree_data[n].sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sidename\":\""<<http::utf8_to_jsonstring(tree_data[n].sidename)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"txtcontent\":\""<<http::utf8_to_jsonstring(tree_data[n].txtcontent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sort==0){
	tempsql<<"\"sort\":0";
 }else{ 
	tempsql<<"\"sort\":"<<std::to_string(tree_data[n].sort);
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
   
    template<topicset_info::cols KeyCol, topicset_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>()));
        using ValType = decltype(topicset_info::getField<ValCol>(std::declval<const topicset_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(topicset_info::getField<KeyCol>(iter), topicset_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<topicset_info::cols KeyCol, topicset_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>())), 
            decltype(topicset_info::getField<ValCol>(std::declval<const topicset_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>())), 
                    decltype(topicset_info::getField<ValCol>(std::declval<const topicset_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>()));
        using ValType = decltype(topicset_info::getField<ValCol>(std::declval<const topicset_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(topicset_info::getField<KeyCol>(iter), topicset_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(topicset_info::getField<KeyCol>(iter), topicset_info::getField<ValCol>(iter))) {
                    result.emplace(topicset_info::getField<KeyCol>(iter), topicset_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<topicset_info::cols KeyCol, topicset_info::cols ValCol> 
    auto get_vec_cols()
    {
        using KeyType = decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>()));
        using ValType = decltype(topicset_info::getField<ValCol>(std::declval<const topicset_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(topicset_info::getField<KeyCol>(iter), topicset_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<topicset_info::cols KeyCol, topicset_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>())), 
            decltype(topicset_info::getField<ValCol>(std::declval<const topicset_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>())), 
                    decltype(topicset_info::getField<ValCol>(std::declval<const topicset_info::meta&>()))>, bool>
    auto get_vec_cols(Callback&& callback)
    {
        using KeyType = decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>()));
        using ValType = decltype(topicset_info::getField<ValCol>(std::declval<const topicset_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(topicset_info::getField<KeyCol>(iter), topicset_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(topicset_info::getField<KeyCol>(iter), topicset_info::getField<ValCol>(iter))) {
                    result.emplace_back(topicset_info::getField<KeyCol>(iter), topicset_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<topicset_info::cols KeyCol>
    auto get_vec_col()
    {
        using KeyType = decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(topicset_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_col<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<topicset_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>()))>, bool>
    auto get_vec_col(Callback&& callback)
    {
        using KeyType = decltype(topicset_info::getField<KeyCol>(std::declval<const topicset_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(topicset_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(topicset_info::getField<KeyCol>(iter))) {
                    result.emplace_back(topicset_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<topicset_info::cols Col>
        requires requires(std::ostream& os, decltype(topicset_info::getField<Col>(std::declval<const topicset_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << topicset_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<topicset_info::cols Col>
        requires requires(std::ostream& os, decltype(topicset_info::getField<Col>(std::declval<const topicset_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << topicset_info::getField<Col>(iter); 
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
   