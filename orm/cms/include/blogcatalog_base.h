#ifndef ORM_CMS_BLOGCATALOGBASEMATA_H
#define ORM_CMS_BLOGCATALOGBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Mon, 13 Jul 2026 09:10:59 GMT
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

namespace blogcatalog_info
{
 
    enum class cols : unsigned char 
    {
		bid = 0,
		userid = 1,
		dateid = 2,
		articlenum = 3,
		languageid = 4,

    };

    struct meta
    {
		 unsigned  int  bid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  dateid = 0; ///*月份聚合*/
		 unsigned  int  articlenum = 0; ///*月份文章数量聚合*/
		 unsigned  int  languageid = 0; ///*语言版本*/
	};
  
    struct meta_tree
    {
		 unsigned  int  bid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  dateid = 0; ///*月份聚合*/
		 unsigned  int  articlenum = 0; ///*月份文章数量聚合*/
		 unsigned  int  languageid = 0; ///*语言版本*/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  bid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  dateid = 0; ///*月份聚合*/
		 unsigned  int  articlenum = 0; ///*月份文章数量聚合*/
		 unsigned  int  languageid = 0; ///*语言版本*/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::bid) { 
		 return m.bid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::dateid) { 
		 return m.dateid;
		} else if constexpr (Col == cols::articlenum) { 
		 return m.articlenum;
		} else if constexpr (Col == cols::languageid) { 
		 return m.languageid;
		
        } else {
            //static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using bid =  unsigned  int ;
		using userid =  unsigned  int ;
		using dateid =  unsigned  int ;
		using articlenum =  unsigned  int ;
		using languageid =  unsigned  int ;

    }

    
    #define ORM_CMS_BLOGCATALOG_EXPAND(x) x 
    
    #define ORM_CMS_BLOGCATALOG_META_FIELD_TYPE(col) \
         orm::cms::blogcatalog_info::type::col 
    
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBER(col) \
          ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_1(c1) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_1( c1)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c2))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c3))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c4))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c5))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c6))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c7))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c8))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c9))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c10))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c11))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c12))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c13))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c14))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c15))
         
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBER(c16))
         
    #define ORM_CMS_BLOGCATALOG_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_BLOGCATALOG_PROJ_MEMBERS(...) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_16, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_15, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_14, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_13, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_12, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_11, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_10, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_9, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_8, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_7, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_6, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_5, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_4, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_3, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_2, \
            ORM_CMS_BLOGCATALOG_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_BLOGCATALOG_COUNT(...) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_BLOGCATALOG_TO_JSON_1(c1) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_2(c1,c2) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_BLOGCATALOG_TO_JSON_BODY(...) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_BLOGCATALOG_TO_JSON_16,ORM_CMS_BLOGCATALOG_TO_JSON_15,ORM_CMS_BLOGCATALOG_TO_JSON_14,ORM_CMS_BLOGCATALOG_TO_JSON_13,ORM_CMS_BLOGCATALOG_TO_JSON_12,ORM_CMS_BLOGCATALOG_TO_JSON_11,ORM_CMS_BLOGCATALOG_TO_JSON_10,ORM_CMS_BLOGCATALOG_TO_JSON_9,ORM_CMS_BLOGCATALOG_TO_JSON_8,ORM_CMS_BLOGCATALOG_TO_JSON_7,ORM_CMS_BLOGCATALOG_TO_JSON_6,ORM_CMS_BLOGCATALOG_TO_JSON_5,ORM_CMS_BLOGCATALOG_TO_JSON_4,ORM_CMS_BLOGCATALOG_TO_JSON_3,ORM_CMS_BLOGCATALOG_TO_JSON_2,ORM_CMS_BLOGCATALOG_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_BLOGCATALOG_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_1(n1)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_1(n1)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_BLOGCATALOG_CAT(a, b) ORM_CMS_BLOGCATALOG_CAT_(a, b)
    #define ORM_CMS_BLOGCATALOG_CAT_(a, b) a##b

    #define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_BLOGCATALOG_CAT(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM(...) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_BLOGCATALOG_SET_VAL_1(c1) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_BLOGCATALOG_SET_VAL_2(c1,c2) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_1(c1)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_2(c1,c2)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_BLOGCATALOG_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_BLOGCATALOG_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_BLOGCATALOG_CAT(ORM_CMS_BLOGCATALOG_SET_VAL_, N)
    
    
    #define ORM_CMS_BLOGCATALOG_SET_VAL_FIELDS(...) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_BLOGCATALOG_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_FIELDS(ORM_CMS_BLOGCATALOG_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_BLOGCATALOG_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::blogcatalog_info { \
            struct StructName { \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_BLOGCATALOG_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::blogcatalog_info { \
            struct StructName { \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM(ORM_CMS_BLOGCATALOG_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_BLOGCATALOG_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::blogcatalog_info { \
            struct StructName { \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_BLOGCATALOG_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::blogcatalog_info { \
            struct StructName { \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_BLOGCATALOG_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::blogcatalog_info { \
            struct StructName { \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_TO_JSON_CUSTOM(ORM_CMS_BLOGCATALOG_UNWRAP CustomNames));  \
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
                        ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_BLOGCATALOG_EXPAND(ORM_CMS_BLOGCATALOG_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,5> col_names={"bid","userid","dateid","articlenum","languageid"};
	static constexpr std::array<unsigned char,5> col_types={3,3,3,3,3};
	static constexpr std::array<unsigned char,5> col_length={0,0,0,0,0};
	static constexpr std::array<unsigned char,5> col_decimals={0,0,0,0,0};

}

struct blogcatalog_base
{
      blogcatalog_info::meta data;
    std::vector<blogcatalog_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<blogcatalog_info::meta>::iterator begin(){     return record.begin(); }
std::vector<blogcatalog_info::meta>::iterator end(){     return record.end(); }
std::vector<blogcatalog_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<blogcatalog_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="blogcatalog";
static constexpr std::string_view org_tablename="blogcatalog";
static constexpr std::string_view modelname="Blogcatalog";

	  unsigned char findcolpos(const std::string &coln){
            if(coln.size()==0)
            {
                return 255;
            }
		    unsigned char  bi=coln[0];
         

	         if(bi<91&&bi>64){
				bi+=32;
			}
            switch(coln[0]){


         case 'a':
   	 return 3;
break;
case 'b':
   	 return 0;
break;
case 'd':
   	 return 2;
break;
case 'l':
   	 return 4;
break;
case 'u':
   	 return 1;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "bid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     blogcatalog_info::meta metatemp;    
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
        for(;j<blogcatalog_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<blogcatalog_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.bid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.bid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.dateid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.dateid);
}
if(data.articlenum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.articlenum);
}
if(data.languageid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.languageid);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string make_data_insert_sql(const blogcatalog_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<blogcatalog_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<blogcatalog_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.bid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.bid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.dateid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.dateid);
}
if(insert_data.articlenum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.articlenum);
}
if(insert_data.languageid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.languageid);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string make_vector_insert_sql(const std::vector<blogcatalog_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<blogcatalog_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<blogcatalog_info::col_names[j];
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


            	if(insert_data[i].bid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].bid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].dateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].dateid);
	}
	if(insert_data[i].articlenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].articlenum);
	}
	if(insert_data[i].languageid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].languageid);
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

        if(data.bid==0){
	tempsql<<"`bid`=0";
 }else{ 
	tempsql<<"`bid`="<<std::to_string(data.bid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.dateid==0){
	tempsql<<",`dateid`=0";
 }else{ 
	tempsql<<",`dateid`="<<std::to_string(data.dateid);
}
if(data.articlenum==0){
	tempsql<<",`articlenum`=0";
 }else{ 
	tempsql<<",`articlenum`="<<std::to_string(data.articlenum);
}
if(data.languageid==0){
	tempsql<<",`languageid`=0";
 }else{ 
	tempsql<<",`languageid`="<<std::to_string(data.languageid);
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
if(data.bid==0){
	tempsql<<"`bid`=0";
 }else{ 
	tempsql<<"`bid`="<<std::to_string(data.bid);
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
if(data.dateid==0){
	tempsql<<"`dateid`=0";
 }else{ 
	tempsql<<"`dateid`="<<std::to_string(data.dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.articlenum==0){
	tempsql<<"`articlenum`=0";
 }else{ 
	tempsql<<"`articlenum`="<<std::to_string(data.articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.languageid==0){
	tempsql<<"`languageid`=0";
 }else{ 
	tempsql<<"`languageid`="<<std::to_string(data.languageid);
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
        for (; j < blogcatalog_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << blogcatalog_info::col_names[j];
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
            	if(record[i].bid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].bid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].dateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].dateid);
	}
	if(record[i].articlenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].articlenum);
	}
	if(record[i].languageid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languageid);
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
        for (; j < blogcatalog_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << blogcatalog_info::col_names[j];
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
            	if(record[i].bid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].bid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].dateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].dateid);
	}
	if(record[i].articlenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].articlenum);
	}
	if(record[i].languageid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languageid);
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
            for(jj=0;jj<blogcatalog_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.bid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.bid));
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
if(data.dateid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.dateid));
}
 break;
 case 3:
if(data.articlenum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.articlenum));
}
 break;
 case 4:
if(data.languageid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.languageid));
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
            for(jj=0;jj<blogcatalog_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.bid==0){
	tempsql.insert({"bid","0"});
 }else{ 
	tempsql.insert({"bid",std::to_string(data.bid)});
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
if(data.dateid==0){
	tempsql.insert({"dateid","0"});
 }else{ 
	tempsql.insert({"dateid",std::to_string(data.dateid)});
}
 break;
 case 3:
if(data.articlenum==0){
	tempsql.insert({"articlenum","0"});
 }else{ 
	tempsql.insert({"articlenum",std::to_string(data.articlenum)});
}
 break;
 case 4:
if(data.languageid==0){
	tempsql.insert({"languageid","0"});
 }else{ 
	tempsql.insert({"languageid",std::to_string(data.languageid)});
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
if(data.bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(data.bid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.dateid==0){
	tempsql<<",\"dateid\":0";
 }else{ 
	tempsql<<",\"dateid\":"<<std::to_string(data.dateid);
}
if(data.articlenum==0){
	tempsql<<",\"articlenum\":0";
 }else{ 
	tempsql<<",\"articlenum\":"<<std::to_string(data.articlenum);
}
if(data.languageid==0){
	tempsql<<",\"languageid\":0";
 }else{ 
	tempsql<<",\"languageid\":"<<std::to_string(data.languageid);
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
            for(jj=0;jj<blogcatalog_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(data.bid);
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
if(data.dateid==0){
	tempsql<<"\"dateid\":0";
 }else{ 
	tempsql<<"\"dateid\":"<<std::to_string(data.dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.articlenum==0){
	tempsql<<"\"articlenum\":0";
 }else{ 
	tempsql<<"\"articlenum\":"<<std::to_string(data.articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.languageid==0){
	tempsql<<"\"languageid\":0";
 }else{ 
	tempsql<<"\"languageid\":"<<std::to_string(data.languageid);
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
        blogcatalog_info::meta metatemp; 
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
		  http::json_set_val(data.bid,set_value_name);
		 break;
		
		case 1:
		  http::json_set_val(data.userid,set_value_name);
		 break;
		
		case 2:
		  http::json_set_val(data.dateid,set_value_name);
		 break;
		
		case 3:
		  http::json_set_val(data.articlenum,set_value_name);
		 break;
		
		case 4:
		  http::json_set_val(data.languageid,set_value_name);
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
        for(jj=0;jj<blogcatalog_info::col_names.size();jj++){
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
if(record[n].bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(record[n].bid);
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
if(record[n].dateid==0){
	tempsql<<"\"dateid\":0";
 }else{ 
	tempsql<<"\"dateid\":"<<std::to_string(record[n].dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].articlenum==0){
	tempsql<<"\"articlenum\":0";
 }else{ 
	tempsql<<"\"articlenum\":"<<std::to_string(record[n].articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].languageid==0){
	tempsql<<"\"languageid\":0";
 }else{ 
	tempsql<<"\"languageid\":"<<std::to_string(record[n].languageid);
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
   
   std::string to_json(std::function<bool(std::string&,blogcatalog_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<blogcatalog_info::col_names.size();jj++){
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
if(record[n].bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(record[n].bid);
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
if(record[n].dateid==0){
	tempsql<<"\"dateid\":0";
 }else{ 
	tempsql<<"\"dateid\":"<<std::to_string(record[n].dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].articlenum==0){
	tempsql<<"\"articlenum\":0";
 }else{ 
	tempsql<<"\"articlenum\":"<<std::to_string(record[n].articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].languageid==0){
	tempsql<<"\"languageid\":0";
 }else{ 
	tempsql<<"\"languageid\":"<<std::to_string(record[n].languageid);
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
   long long getPK(){  return data.bid; } 
 void setPK(long long val){  data.bid=val;} 
 unsigned  int  getBid(){  return data.bid; } 
 void setBid( unsigned  int  val){  data.bid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getDateid(){  return data.dateid; } 
 void setDateid( unsigned  int  val){  data.dateid=val;} 

 unsigned  int  getArticlenum(){  return data.articlenum; } 
 void setArticlenum( unsigned  int  val){  data.articlenum=val;} 

 unsigned  int  getLanguageid(){  return data.languageid; } 
 void setLanguageid( unsigned  int  val){  data.languageid=val;} 

blogcatalog_info::meta getnewData(){
 	 struct blogcatalog_info::meta newdata;
	 return newdata; 
} 
blogcatalog_info::meta getData(){
 	 return data; 
} 
std::vector<blogcatalog_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<blogcatalog_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<blogcatalog_info::col_names.size();jj++){
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
if(tree_data[n].bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(tree_data[n].bid);
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
if(tree_data[n].dateid==0){
	tempsql<<"\"dateid\":0";
 }else{ 
	tempsql<<"\"dateid\":"<<std::to_string(tree_data[n].dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].articlenum==0){
	tempsql<<"\"articlenum\":0";
 }else{ 
	tempsql<<"\"articlenum\":"<<std::to_string(tree_data[n].articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].languageid==0){
	tempsql<<"\"languageid\":0";
 }else{ 
	tempsql<<"\"languageid\":"<<std::to_string(tree_data[n].languageid);
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
   
   std::string tree_tojson(const std::vector<blogcatalog_info::meta_tree> &tree_data,std::function<bool(std::string&,const blogcatalog_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<blogcatalog_info::col_names.size();jj++){
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
if(tree_data[n].bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(tree_data[n].bid);
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
if(tree_data[n].dateid==0){
	tempsql<<"\"dateid\":0";
 }else{ 
	tempsql<<"\"dateid\":"<<std::to_string(tree_data[n].dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].articlenum==0){
	tempsql<<"\"articlenum\":0";
 }else{ 
	tempsql<<"\"articlenum\":"<<std::to_string(tree_data[n].articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].languageid==0){
	tempsql<<"\"languageid\":0";
 }else{ 
	tempsql<<"\"languageid\":"<<std::to_string(tree_data[n].languageid);
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
   
    template<blogcatalog_info::cols KeyCol, blogcatalog_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>()));
        using ValType = decltype(blogcatalog_info::getField<ValCol>(std::declval<const blogcatalog_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(blogcatalog_info::getField<KeyCol>(iter), blogcatalog_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<blogcatalog_info::cols KeyCol, blogcatalog_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>())), 
            decltype(blogcatalog_info::getField<ValCol>(std::declval<const blogcatalog_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>())), 
                    decltype(blogcatalog_info::getField<ValCol>(std::declval<const blogcatalog_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>()));
        using ValType = decltype(blogcatalog_info::getField<ValCol>(std::declval<const blogcatalog_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(blogcatalog_info::getField<KeyCol>(iter), blogcatalog_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(blogcatalog_info::getField<KeyCol>(iter), blogcatalog_info::getField<ValCol>(iter))) {
                    result.emplace(blogcatalog_info::getField<KeyCol>(iter), blogcatalog_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<blogcatalog_info::cols KeyCol, blogcatalog_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>()));
        using ValType = decltype(blogcatalog_info::getField<ValCol>(std::declval<const blogcatalog_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(blogcatalog_info::getField<KeyCol>(iter), blogcatalog_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<blogcatalog_info::cols KeyCol, blogcatalog_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>())), 
            decltype(blogcatalog_info::getField<ValCol>(std::declval<const blogcatalog_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>())), 
                    decltype(blogcatalog_info::getField<ValCol>(std::declval<const blogcatalog_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>()));
        using ValType = decltype(blogcatalog_info::getField<ValCol>(std::declval<const blogcatalog_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(blogcatalog_info::getField<KeyCol>(iter), blogcatalog_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(blogcatalog_info::getField<KeyCol>(iter), blogcatalog_info::getField<ValCol>(iter))) {
                    result.emplace_back(blogcatalog_info::getField<KeyCol>(iter), blogcatalog_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<blogcatalog_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(blogcatalog_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<blogcatalog_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(blogcatalog_info::getField<KeyCol>(std::declval<const blogcatalog_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(blogcatalog_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(blogcatalog_info::getField<KeyCol>(iter))) {
                    result.emplace_back(blogcatalog_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<blogcatalog_info::cols Col>
        requires requires(std::ostream& os, decltype(blogcatalog_info::getField<Col>(std::declval<const blogcatalog_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << blogcatalog_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<blogcatalog_info::cols Col>
        requires requires(std::ostream& os, decltype(blogcatalog_info::getField<Col>(std::declval<const blogcatalog_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << blogcatalog_info::getField<Col>(iter); 
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
   