#ifndef ORM_CMS_TESTBBASEMATA_H
#define ORM_CMS_TESTBBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Tue, 16 Jun 2026 15:26:26 GMT
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

namespace testb_info
{
 
    enum class cols : unsigned char 
    {
		tid = 0,
		score = 1,
		name = 2,
		pricenum = 3,
		orgprice = 4,
		subprice = 5,

    };

    struct meta
    {
		 int  tid = 0; ///**/
		 long long  score = 0; ///*分数[num*100]*/
		 std::string  name = ""; ///**/
		 double  pricenum = 0; ///**/
		 float  orgprice = 0; ///**/
		 double  subprice = 0; ///**/
	};
  
    struct meta_tree
    {
		 int  tid = 0; ///**/
		 long long  score = 0; ///*分数[num*100]*/
		 std::string  name = ""; ///**/
		 double  pricenum = 0; ///**/
		 float  orgprice = 0; ///**/
		 double  subprice = 0; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 int  tid = 0; ///**/
		 long long  score = 0; ///*分数[num*100]*/
		 std::string  name = ""; ///**/
		 double  pricenum = 0; ///**/
		 float  orgprice = 0; ///**/
		 double  subprice = 0; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::tid) { 
		 return m.tid;
		} else if constexpr (Col == cols::score) { 
		 return m.score;
		} else if constexpr (Col == cols::name) { 
		 return m.name;
		} else if constexpr (Col == cols::pricenum) { 
		 return m.pricenum;
		} else if constexpr (Col == cols::orgprice) { 
		 return m.orgprice;
		} else if constexpr (Col == cols::subprice) { 
		 return m.subprice;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using tid =  int ;
		using score =  long long ;
		using name =  std::string ;
		using pricenum =  double ;
		using orgprice =  float ;
		using subprice =  double ;

    }

    
    #define ORM_CMS_TESTB_EXPAND(x) x 
    
    #define ORM_CMS_TESTB_META_FIELD_TYPE(col) \
         orm::cms::testb_info::type::col 
    
    #define ORM_CMS_TESTB_PROJ_MEMBER(col) \
          ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_TESTB_PROJ_MEMBERS_1(c1) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_TESTB_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_1( c1)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c2))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c3))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c4))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c5))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c6))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c7))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c8))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c9))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c10))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c11))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c12))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c13))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c14))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c15))
         
    #define ORM_CMS_TESTB_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBER(c16))
         
    #define ORM_CMS_TESTB_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_TESTB_PROJ_MEMBERS(...) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_TESTB_PROJ_MEMBERS_16, \
            ORM_CMS_TESTB_PROJ_MEMBERS_15, \
            ORM_CMS_TESTB_PROJ_MEMBERS_14, \
            ORM_CMS_TESTB_PROJ_MEMBERS_13, \
            ORM_CMS_TESTB_PROJ_MEMBERS_12, \
            ORM_CMS_TESTB_PROJ_MEMBERS_11, \
            ORM_CMS_TESTB_PROJ_MEMBERS_10, \
            ORM_CMS_TESTB_PROJ_MEMBERS_9, \
            ORM_CMS_TESTB_PROJ_MEMBERS_8, \
            ORM_CMS_TESTB_PROJ_MEMBERS_7, \
            ORM_CMS_TESTB_PROJ_MEMBERS_6, \
            ORM_CMS_TESTB_PROJ_MEMBERS_5, \
            ORM_CMS_TESTB_PROJ_MEMBERS_4, \
            ORM_CMS_TESTB_PROJ_MEMBERS_3, \
            ORM_CMS_TESTB_PROJ_MEMBERS_2, \
            ORM_CMS_TESTB_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_TESTB_COUNT(...) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_TESTB_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_TESTB_TO_JSON_1(c1) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_TESTB_TO_JSON_2(c1,c2) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_TESTB_TO_JSON_BODY(...) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_TESTB_TO_JSON_16,ORM_CMS_TESTB_TO_JSON_15,ORM_CMS_TESTB_TO_JSON_14,ORM_CMS_TESTB_TO_JSON_13,ORM_CMS_TESTB_TO_JSON_12,ORM_CMS_TESTB_TO_JSON_11,ORM_CMS_TESTB_TO_JSON_10,ORM_CMS_TESTB_TO_JSON_9,ORM_CMS_TESTB_TO_JSON_8,ORM_CMS_TESTB_TO_JSON_7,ORM_CMS_TESTB_TO_JSON_6,ORM_CMS_TESTB_TO_JSON_5,ORM_CMS_TESTB_TO_JSON_4,ORM_CMS_TESTB_TO_JSON_3,ORM_CMS_TESTB_TO_JSON_2,ORM_CMS_TESTB_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_TESTB_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_TESTB_TO_JSON_CUSTOM_1(n1)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_1(n1)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_TESTB_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_TESTB_CAT(a, b) ORM_CMS_TESTB_CAT_(a, b)
    #define ORM_CMS_TESTB_CAT_(a, b) a##b

    #define ORM_CMS_TESTB_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_TESTB_CAT(ORM_CMS_TESTB_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_TESTB_TO_JSON_CUSTOM(...) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_TESTB_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_TESTB_SET_VAL_1(c1) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_TESTB_SET_VAL_2(c1,c2) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_1(c1)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_TESTB_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_2(c1,c2)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_TESTB_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_TESTB_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_TESTB_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_TESTB_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_TESTB_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_TESTB_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_TESTB_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_TESTB_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_TESTB_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_TESTB_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_TESTB_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_TESTB_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_TESTB_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_TESTB_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_TESTB_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_TESTB_CAT(ORM_CMS_TESTB_SET_VAL_, N)
    
    
    #define ORM_CMS_TESTB_SET_VAL_FIELDS(...) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_TESTB_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_FIELDS(ORM_CMS_TESTB_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_TESTB_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::testb_info { \
            struct StructName { \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_TESTB_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::testb_info { \
            struct StructName { \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM(ORM_CMS_TESTB_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_TESTB_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::testb_info { \
            struct StructName { \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_TESTB_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::testb_info { \
            struct StructName { \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_TESTB_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::testb_info { \
            struct StructName { \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_TO_JSON_CUSTOM(ORM_CMS_TESTB_UNWRAP CustomNames));  \
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
                        ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_TESTB_EXPAND(ORM_CMS_TESTB_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,6> col_names={"tid","score","name","pricenum","orgprice","subprice"};
	static constexpr std::array<unsigned char,6> col_types={3,8,253,5,4,246};
	static constexpr std::array<unsigned char,6> col_length={0,0,30,0,0,10};
	static constexpr std::array<unsigned char,6> col_decimals={0,0,0,0,0,2};

}

struct testb_base
{
      testb_info::meta data;
    std::vector<testb_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<testb_info::meta>::iterator begin(){     return record.begin(); }
std::vector<testb_info::meta>::iterator end(){     return record.end(); }
std::vector<testb_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<testb_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="testb";
static constexpr std::string_view org_tablename="testb";
static constexpr std::string_view modelname="Testb";

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


         case 'n':
   	 return 2;
break;
case 'o':
   	 return 4;
break;
case 'p':
   	 return 3;
break;
case 's':
 switch(coln.size()){  
case 5:
   	 return 1;
break;
case 8:
   	 return 5;
break;
 }
 break;
case 't':
   	 return 0;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "tid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     testb_info::meta metatemp;    
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
        for(;j<testb_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<testb_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.tid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.tid);
}
if(data.score==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.score);
}
tempsql<<",'"<<stringaddslash(data.name)<<"'";
if(data.pricenum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.pricenum);
}
if(data.orgprice==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.orgprice);
}
if(data.subprice==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.subprice);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const testb_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<testb_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<testb_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.tid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.tid);
}
if(insert_data.score==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.score);
}
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
if(insert_data.pricenum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.pricenum);
}
if(insert_data.orgprice==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.orgprice);
}
if(insert_data.subprice==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.subprice);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<testb_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<testb_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<testb_info::col_names[j];
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


            	if(insert_data[i].tid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].tid);
	}
	if(insert_data[i].score==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].score);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
	if(insert_data[i].pricenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].pricenum);
	}
	if(insert_data[i].orgprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].orgprice);
	}
	if(insert_data[i].subprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].subprice);
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

        if(data.tid==0){
	tempsql<<"`tid`=0";
 }else{ 
	tempsql<<"`tid`="<<std::to_string(data.tid);
}
if(data.score==0){
	tempsql<<",`score`=0";
 }else{ 
	tempsql<<",`score`="<<std::to_string(data.score);
}
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
if(data.pricenum==0){
	tempsql<<",`pricenum`=0";
 }else{ 
	tempsql<<",`pricenum`="<<std::to_string(data.pricenum);
}
if(data.orgprice==0){
	tempsql<<",`orgprice`=0";
 }else{ 
	tempsql<<",`orgprice`="<<std::to_string(data.orgprice);
}
if(data.subprice==0){
	tempsql<<",`subprice`=0";
 }else{ 
	tempsql<<",`subprice`="<<std::to_string(data.subprice);
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
if(data.tid==0){
	tempsql<<"`tid`=0";
 }else{ 
	tempsql<<"`tid`="<<std::to_string(data.tid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.score==0){
	tempsql<<"`score`=0";
 }else{ 
	tempsql<<"`score`="<<std::to_string(data.score);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.pricenum==0){
	tempsql<<"`pricenum`=0";
 }else{ 
	tempsql<<"`pricenum`="<<std::to_string(data.pricenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.orgprice==0){
	tempsql<<"`orgprice`=0";
 }else{ 
	tempsql<<"`orgprice`="<<std::to_string(data.orgprice);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.subprice==0){
	tempsql<<"`subprice`=0";
 }else{ 
	tempsql<<"`subprice`="<<std::to_string(data.subprice);
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
        for (; j < testb_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << testb_info::col_names[j];
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
            	if(record[i].tid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].tid);
	}
	if(record[i].score==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].score);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].pricenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].pricenum);
	}
	if(record[i].orgprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].orgprice);
	}
	if(record[i].subprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].subprice);
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
        for (; j < testb_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << testb_info::col_names[j];
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
            	if(record[i].tid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].tid);
	}
	if(record[i].score==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].score);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].pricenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].pricenum);
	}
	if(record[i].orgprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].orgprice);
	}
	if(record[i].subprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].subprice);
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
            for(jj=0;jj<testb_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.tid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.tid));
}
 break;
 case 1:
if(data.score==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.score));
}
 break;
 case 2:
	temparray.push_back(data.name);
 break;
 case 3:
if(data.pricenum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.pricenum));
}
 break;
 case 4:
if(data.orgprice==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.orgprice));
}
 break;
 case 5:
if(data.subprice==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.subprice));
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
            for(jj=0;jj<testb_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.tid==0){
	tempsql.insert({"tid","0"});
 }else{ 
	tempsql.insert({"tid",std::to_string(data.tid)});
}
 break;
 case 1:
if(data.score==0){
	tempsql.insert({"score","0"});
 }else{ 
	tempsql.insert({"score",std::to_string(data.score)});
}
 break;
 case 2:
	tempsql.insert({"name",data.name});
 break;
 case 3:
if(data.pricenum==0){
	tempsql.insert({"pricenum","0"});
 }else{ 
	tempsql.insert({"pricenum",std::to_string(data.pricenum)});
}
 break;
 case 4:
if(data.orgprice==0){
	tempsql.insert({"orgprice","0"});
 }else{ 
	tempsql.insert({"orgprice",std::to_string(data.orgprice)});
}
 break;
 case 5:
if(data.subprice==0){
	tempsql.insert({"subprice","0"});
 }else{ 
	tempsql.insert({"subprice",std::to_string(data.subprice)});
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
if(data.tid==0){
	tempsql<<"\"tid\":0";
 }else{ 
	tempsql<<"\"tid\":"<<std::to_string(data.tid);
}
if(data.score==0){
	tempsql<<",\"score\":0";
 }else{ 
	tempsql<<",\"score\":"<<std::to_string(data.score);
}
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
if(data.pricenum==0){
	tempsql<<",\"pricenum\":0";
 }else{ 
	tempsql<<",\"pricenum\":"<<std::to_string(data.pricenum);
}
if(data.orgprice==0){
	tempsql<<",\"orgprice\":0";
 }else{ 
	tempsql<<",\"orgprice\":"<<std::to_string(data.orgprice);
}
if(data.subprice==0){
	tempsql<<",\"subprice\":0";
 }else{ 
	tempsql<<",\"subprice\":"<<std::to_string(data.subprice);
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
            for(jj=0;jj<testb_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.tid==0){
	tempsql<<"\"tid\":0";
 }else{ 
	tempsql<<"\"tid\":"<<std::to_string(data.tid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.score==0){
	tempsql<<"\"score\":0";
 }else{ 
	tempsql<<"\"score\":"<<std::to_string(data.score);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.pricenum==0){
	tempsql<<"\"pricenum\":0";
 }else{ 
	tempsql<<"\"pricenum\":"<<std::to_string(data.pricenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.orgprice==0){
	tempsql<<"\"orgprice\":0";
 }else{ 
	tempsql<<"\"orgprice\":"<<std::to_string(data.orgprice);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.subprice==0){
	tempsql<<"\"subprice\":0";
 }else{ 
	tempsql<<"\"subprice\":"<<std::to_string(data.subprice);
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
        testb_info::meta metatemp; 
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
			data.tid=std::stoi(set_value_name);
		}catch (...) { 
			data.tid=0;
			 }
			break;
		case 1:
		 try{
			data.score=std::stoll(set_value_name);
		}catch (...) { 
			data.score=0;
			 }
			break;
		case 2:
		 try{
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.pricenum=std::stod(set_value_name);
		}catch (...) { 
			data.pricenum=0.0;
			 }
			break;
		case 4:
		 try{
			data.orgprice=std::stof(set_value_name);
		}catch (...) { 
			data.orgprice=0.0;
			 }
			break;
		case 5:
		 try{
			data.subprice=std::stof(set_value_name);
		}catch (...) { 
			data.subprice=0.0;
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
			data.tid=set_value_name;
		}catch (...) { 
			data.tid=0;
			 }
			break;
		case 1:
		 try{
			data.score=set_value_name;
		}catch (...) { 
			data.score=0;
			 }
			break;
		case 2:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.pricenum=(double)set_value_name;
		}catch (...) { 
			data.pricenum=0.0;
			 }
			break;
		case 4:
		 try{
			data.orgprice=set_value_name;
		}catch (...) { 
			data.orgprice=0.0;
			 }
			break;
		case 5:
		 try{
			data.subprice=(float)set_value_name;
		}catch (...) { 
			data.subprice=0.0;
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
			data.tid=(int)set_value_name;
		}catch (...) { 
			data.tid=0;
			 }
			break;
		case 1:
		 try{
			data.score=(long long)set_value_name;
		}catch (...) { 
			data.score=0;
			 }
			break;
		case 2:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.pricenum=set_value_name;
		}catch (...) { 
			data.pricenum=0.0;
			 }
			break;
		case 4:
		 try{
			data.orgprice=(float)set_value_name;
		}catch (...) { 
			data.orgprice=0.0;
			 }
			break;
		case 5:
		 try{
			data.subprice=(float)set_value_name;
		}catch (...) { 
			data.subprice=0.0;
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
        for(jj=0;jj<testb_info::col_names.size();jj++){
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
if(record[n].tid==0){
	tempsql<<"\"tid\":0";
 }else{ 
	tempsql<<"\"tid\":"<<std::to_string(record[n].tid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].score==0){
	tempsql<<"\"score\":0";
 }else{ 
	tempsql<<"\"score\":"<<std::to_string(record[n].score);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].pricenum==0){
	tempsql<<"\"pricenum\":0";
 }else{ 
	tempsql<<"\"pricenum\":"<<std::to_string(record[n].pricenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].orgprice==0){
	tempsql<<"\"orgprice\":0";
 }else{ 
	tempsql<<"\"orgprice\":"<<std::to_string(record[n].orgprice);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].subprice==0){
	tempsql<<"\"subprice\":0";
 }else{ 
	tempsql<<"\"subprice\":"<<std::to_string(record[n].subprice);
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
   
   std::string to_json(std::function<bool(std::string&,testb_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<testb_info::col_names.size();jj++){
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
if(record[n].tid==0){
	tempsql<<"\"tid\":0";
 }else{ 
	tempsql<<"\"tid\":"<<std::to_string(record[n].tid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].score==0){
	tempsql<<"\"score\":0";
 }else{ 
	tempsql<<"\"score\":"<<std::to_string(record[n].score);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].pricenum==0){
	tempsql<<"\"pricenum\":0";
 }else{ 
	tempsql<<"\"pricenum\":"<<std::to_string(record[n].pricenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].orgprice==0){
	tempsql<<"\"orgprice\":0";
 }else{ 
	tempsql<<"\"orgprice\":"<<std::to_string(record[n].orgprice);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].subprice==0){
	tempsql<<"\"subprice\":0";
 }else{ 
	tempsql<<"\"subprice\":"<<std::to_string(record[n].subprice);
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
   long long getPK(){  return data.tid; } 
 void setPK(long long val){  data.tid=val;} 
 int  getTid(){  return data.tid; } 
 void setTid( int  val){  data.tid=val;} 

 long long  getScore(){  return data.score; } 
 void setScore( long long  val){  data.score=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 double  getPricenum(){  return data.pricenum; } 
 void setPricenum( double  val){  data.pricenum=val;} 

 float  getOrgprice(){  return data.orgprice; } 
 void setOrgprice( float  val){  data.orgprice=val;} 

 double  getSubprice(){  return data.subprice; } 
 void setSubprice( double  val){  data.subprice=val;} 

testb_info::meta getnewData(){
 	 struct testb_info::meta newdata;
	 return newdata; 
} 
testb_info::meta getData(){
 	 return data; 
} 
std::vector<testb_info::meta> getRecord(){
 	 return record; 
} 

 double getScoreToNum()
{
	return (double)data.score/100;
}

 template<typename T>
 requires std::is_arithmetic_v<T>
 double getScoreToNum(T a)
{
	return (double)((long double)a/100);
}

 template<typename T>
 requires std::is_arithmetic_v<T>
 long long getNumToScore(T a)
{
	return std::round(a*100);
}

 template<typename T>
 requires std::is_arithmetic_v<T>
 void setScoreToNum(T a)
{
	data.score=std::round(a*100);
}

   std::string tree_tojson(const std::vector<testb_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<testb_info::col_names.size();jj++){
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
if(tree_data[n].tid==0){
	tempsql<<"\"tid\":0";
 }else{ 
	tempsql<<"\"tid\":"<<std::to_string(tree_data[n].tid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].score==0){
	tempsql<<"\"score\":0";
 }else{ 
	tempsql<<"\"score\":"<<std::to_string(tree_data[n].score);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].pricenum==0){
	tempsql<<"\"pricenum\":0";
 }else{ 
	tempsql<<"\"pricenum\":"<<std::to_string(tree_data[n].pricenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].orgprice==0){
	tempsql<<"\"orgprice\":0";
 }else{ 
	tempsql<<"\"orgprice\":"<<std::to_string(tree_data[n].orgprice);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].subprice==0){
	tempsql<<"\"subprice\":0";
 }else{ 
	tempsql<<"\"subprice\":"<<std::to_string(tree_data[n].subprice);
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
   
   std::string tree_tojson(const std::vector<testb_info::meta_tree> &tree_data,std::function<bool(std::string&,const testb_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<testb_info::col_names.size();jj++){
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
if(tree_data[n].tid==0){
	tempsql<<"\"tid\":0";
 }else{ 
	tempsql<<"\"tid\":"<<std::to_string(tree_data[n].tid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].score==0){
	tempsql<<"\"score\":0";
 }else{ 
	tempsql<<"\"score\":"<<std::to_string(tree_data[n].score);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].pricenum==0){
	tempsql<<"\"pricenum\":0";
 }else{ 
	tempsql<<"\"pricenum\":"<<std::to_string(tree_data[n].pricenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].orgprice==0){
	tempsql<<"\"orgprice\":0";
 }else{ 
	tempsql<<"\"orgprice\":"<<std::to_string(tree_data[n].orgprice);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].subprice==0){
	tempsql<<"\"subprice\":0";
 }else{ 
	tempsql<<"\"subprice\":"<<std::to_string(tree_data[n].subprice);
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
   
    template<testb_info::cols KeyCol, testb_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>()));
        using ValType = decltype(testb_info::getField<ValCol>(std::declval<const testb_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(testb_info::getField<KeyCol>(iter), testb_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<testb_info::cols KeyCol, testb_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>())), 
            decltype(testb_info::getField<ValCol>(std::declval<const testb_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>())), 
                    decltype(testb_info::getField<ValCol>(std::declval<const testb_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>()));
        using ValType = decltype(testb_info::getField<ValCol>(std::declval<const testb_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(testb_info::getField<KeyCol>(iter), testb_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(testb_info::getField<KeyCol>(iter), testb_info::getField<ValCol>(iter))) {
                    result.emplace(testb_info::getField<KeyCol>(iter), testb_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<testb_info::cols KeyCol, testb_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>()));
        using ValType = decltype(testb_info::getField<ValCol>(std::declval<const testb_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(testb_info::getField<KeyCol>(iter), testb_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<testb_info::cols KeyCol, testb_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>())), 
            decltype(testb_info::getField<ValCol>(std::declval<const testb_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>())), 
                    decltype(testb_info::getField<ValCol>(std::declval<const testb_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>()));
        using ValType = decltype(testb_info::getField<ValCol>(std::declval<const testb_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(testb_info::getField<KeyCol>(iter), testb_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(testb_info::getField<KeyCol>(iter), testb_info::getField<ValCol>(iter))) {
                    result.emplace_back(testb_info::getField<KeyCol>(iter), testb_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<testb_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(testb_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<testb_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(testb_info::getField<KeyCol>(std::declval<const testb_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(testb_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(testb_info::getField<KeyCol>(iter))) {
                    result.emplace_back(testb_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<testb_info::cols Col>
        requires requires(std::ostream& os, decltype(testb_info::getField<Col>(std::declval<const testb_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << testb_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<testb_info::cols Col>
        requires requires(std::ostream& os, decltype(testb_info::getField<Col>(std::declval<const testb_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << testb_info::getField<Col>(iter); 
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
   