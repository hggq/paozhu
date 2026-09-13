#ifndef ORM_DEFAULT_FK_PARENTBASEMATA_H
#define ORM_DEFAULT_FK_PARENTBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sat, 12 Sep 2026 16:10:51 GMT
***/
#include <iostream>
#include <charconv>
#include <cstdio>
#include <sstream>
#include <array>
#include <map>
#include <string_view>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <ctime>
#include <array>
#include <concepts>
#include <utility>
#include <bit>
#include <algorithm>
#include <bitset>
#include "unicode.h"

namespace orm { 
   
    
namespace fk_parent_info
{
 
    static constexpr std::size_t col_count = 2;
    enum class cols : unsigned char 
    {
		id = 0,
		name = 1,

    };

    struct meta
    {
		 int  id = 0; ///**/
		 std::string  name = ""; ///**/
	};
  
    struct meta_tree
    {
		 int  id = 0; ///**/
		 std::string  name = ""; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 int  id = 0; ///**/
		 std::string  name = ""; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::id) { 
		 return m.id;
		} else if constexpr (Col == cols::name) { 
		 return m.name;
		
        } else {
            //static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using id =  int ;
		using name =  std::string ;

    }

    
    #define ORM_FK_PARENT_EXPAND(x) x 
    
    #define ORM_FK_PARENT_META_FIELD_TYPE(col) \
         orm::fk_parent_info::type::col 
    
    #define ORM_FK_PARENT_PROJ_MEMBER(col) \
          ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_FK_PARENT_PROJ_MEMBERS_1(c1) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c1)) 
     
    #define ORM_FK_PARENT_PROJ_MEMBERS_2( c1, c2) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_1( c1)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c2))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_2( c1, c2)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c3))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_3( c1, c2, c3)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c4))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c5))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c6))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c7))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c8))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c9))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c10))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c11))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c12))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c13))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c14))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c15))
         
    #define ORM_FK_PARENT_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBER(c16))
         
    #define ORM_FK_PARENT_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_FK_PARENT_PROJ_MEMBERS(...) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_GET_MACRO(__VA_ARGS__, \
            ORM_FK_PARENT_PROJ_MEMBERS_16, \
            ORM_FK_PARENT_PROJ_MEMBERS_15, \
            ORM_FK_PARENT_PROJ_MEMBERS_14, \
            ORM_FK_PARENT_PROJ_MEMBERS_13, \
            ORM_FK_PARENT_PROJ_MEMBERS_12, \
            ORM_FK_PARENT_PROJ_MEMBERS_11, \
            ORM_FK_PARENT_PROJ_MEMBERS_10, \
            ORM_FK_PARENT_PROJ_MEMBERS_9, \
            ORM_FK_PARENT_PROJ_MEMBERS_8, \
            ORM_FK_PARENT_PROJ_MEMBERS_7, \
            ORM_FK_PARENT_PROJ_MEMBERS_6, \
            ORM_FK_PARENT_PROJ_MEMBERS_5, \
            ORM_FK_PARENT_PROJ_MEMBERS_4, \
            ORM_FK_PARENT_PROJ_MEMBERS_3, \
            ORM_FK_PARENT_PROJ_MEMBERS_2, \
            ORM_FK_PARENT_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_FK_PARENT_COUNT(...) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_FK_PARENT_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_FK_PARENT_TO_JSON_1(c1) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c1))
        
    #define ORM_FK_PARENT_TO_JSON_2(c1,c2) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_3(c1,c2,c3) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_4(c1,c2,c3,c4) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_FK_PARENT_TO_JSON_BODY(...) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_GET_MACRO(__VA_ARGS__, \
            ORM_FK_PARENT_TO_JSON_16,ORM_FK_PARENT_TO_JSON_15,ORM_FK_PARENT_TO_JSON_14,ORM_FK_PARENT_TO_JSON_13,ORM_FK_PARENT_TO_JSON_12,ORM_FK_PARENT_TO_JSON_11,ORM_FK_PARENT_TO_JSON_10,ORM_FK_PARENT_TO_JSON_9,ORM_FK_PARENT_TO_JSON_8,ORM_FK_PARENT_TO_JSON_7,ORM_FK_PARENT_TO_JSON_6,ORM_FK_PARENT_TO_JSON_5,ORM_FK_PARENT_TO_JSON_4,ORM_FK_PARENT_TO_JSON_3,ORM_FK_PARENT_TO_JSON_2,ORM_FK_PARENT_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_FK_PARENT_UNWRAP(...) __VA_ARGS__  

    #define ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_FK_PARENT_TO_JSON_CUSTOM_1(n1)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_2(n1,n2)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_1(n1)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_2(n1,n2)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_FK_PARENT_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_FK_PARENT_CAT(a, b) ORM_FK_PARENT_CAT_(a, b)
    #define ORM_FK_PARENT_CAT_(a, b) a##b

    #define ORM_FK_PARENT_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_FK_PARENT_CAT(ORM_FK_PARENT_TO_JSON_CUSTOM_, N)

    

    #define ORM_FK_PARENT_TO_JSON_CUSTOM(...) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_FK_PARENT_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_FK_PARENT_SET_VAL_1(c1) \
        ORM_FK_PARENT_SET_VAL_FIELD(c1)
    
    
    #define ORM_FK_PARENT_SET_VAL_2(c1,c2) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_1(c1)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c2)
        
        
    #define ORM_FK_PARENT_SET_VAL_3(c1,c2,c3) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_2(c1,c2)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c3)
        
        
    #define ORM_FK_PARENT_SET_VAL_4(c1,c2,c3,c4) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_3(c1,c2,c3)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c4)
        
        
    #define ORM_FK_PARENT_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c5)
        
        
    #define ORM_FK_PARENT_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c6)
        
        
    #define ORM_FK_PARENT_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c7)
        
        
    #define ORM_FK_PARENT_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c8)
        
        
    #define ORM_FK_PARENT_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c9)
        
        
    #define ORM_FK_PARENT_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c10)
        
        
    #define ORM_FK_PARENT_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c11)
        
        
    #define ORM_FK_PARENT_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c12)
        
        
    #define ORM_FK_PARENT_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c13)
        
        
    #define ORM_FK_PARENT_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c14)
        
        
    #define ORM_FK_PARENT_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c15)
        
        
    #define ORM_FK_PARENT_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_FK_PARENT_SET_VAL_FIELD(c16)
        
        
    #define ORM_FK_PARENT_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_FK_PARENT_CAT(ORM_FK_PARENT_SET_VAL_, N)
    
    
    #define ORM_FK_PARENT_SET_VAL_FIELDS(...) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_FK_PARENT_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_FIELDS(ORM_FK_PARENT_UNWRAP __VA_ARGS__))
    
    
    #define ORM_FK_PARENT_DEFINE_STRUCT(StructName, ...) \
        namespace orm::fk_parent_info { \
            struct StructName { \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_FK_PARENT_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::fk_parent_info { \
            struct StructName { \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM(ORM_FK_PARENT_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_FK_PARENT_TREE_STRUCT(StructName, ...) \
        namespace orm::fk_parent_info { \
            struct StructName { \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_FK_PARENT_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::fk_parent_info { \
            struct StructName { \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_FK_PARENT_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::fk_parent_info { \
            struct StructName { \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_TO_JSON_CUSTOM(ORM_FK_PARENT_UNWRAP CustomNames));  \
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
                        ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_FK_PARENT_EXPAND(ORM_FK_PARENT_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,2> col_names={"id","name"};
	static constexpr std::array<unsigned char,2> col_types={3,253};
	static constexpr std::array<unsigned short,2> col_length={0,100};
	static constexpr std::array<unsigned char,2> col_decimals={0,0};
	static constexpr std::array<bool,2> col_null={false,true};
	static constexpr std::array<bool,2> col_indexed={true,false};
	static constexpr std::string_view auto_pk_name ="id";
	static constexpr int auto_pk_index = 0;

}

struct fk_parent_base
{
    using cols = fk_parent_info::cols;
      fk_parent_info::meta data;
    std::vector<fk_parent_info::meta> record;
static constexpr std::string_view _rmstag="default";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<fk_parent_info::meta>::iterator begin(){     return record.begin(); }
std::vector<fk_parent_info::meta>::iterator end(){     return record.end(); }
std::vector<fk_parent_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<fk_parent_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="fk_parent";
static constexpr std::string_view org_tablename="fk_parent";
static constexpr std::string_view modelname="FkParent";
	static constexpr std::array<bool,2> col_need_quote={false,true};

            std::bitset<2> dirty_bits;
            void clear_dirty() noexcept {
                dirty_bits.reset();
            }

            void set_dirty(std::size_t idx) noexcept {
                if(idx < 2)
                dirty_bits.set(idx);
            }

            [[nodiscard]] std::vector<unsigned char> get_dirty_indices() const noexcept {
                std::vector<unsigned char> result;
                for (std::size_t i = 0; i < dirty_bits.size(); ++i) {
                    if (dirty_bits.test(i)) {
                        result.push_back(static_cast<unsigned char>(i));
                    }
                }
                return result;
            }

            [[nodiscard]] std::vector<std::string_view> get_dirty_names() const
            {
                std::vector<std::string_view> result;
                result.reserve(dirty_bits.size()); // 预分配
                for (size_t i = 0; i < dirty_bits.size(); ++i) {
                    if (dirty_bits.test(i)) {
                        result.push_back(fk_parent_info::col_names[i]);
                    }
                }
                return result;
            }

            [[nodiscard]] std::string get_dirty_names_str(std::string_view sep = ",") const
            {
                auto names = get_dirty_names();

                if (names.empty()) {
                    return {};
                }
                
                std::size_t total_len = 0;
                for (const auto& name : names) {
                    total_len += name.size();
                }
                total_len += sep.size() * (names.size() - 1);

                std::string result;
                result.reserve(total_len);

                bool first = true;
                for (const auto& name : names) {
                    if (!first) {
                        result.append(sep);
                    }
                    result.append(name);
                    first = false;
                }
                return result;
            }
    
	  [[nodiscard]] static constexpr unsigned char findcolpos(std::string_view coln) noexcept {
            if(coln.size()==0)
            {
                return 255;
            }
		    unsigned char  bi= static_cast<unsigned char>(coln[0]);
         

	         if(bi<91&&bi>64){
	         bi+=32;
	         }
	         switch(bi){

         case 'i':
   	 return 0;
break;
case 'n':
   	 return 1;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "id";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     fk_parent_info::meta metatemp;    
            data = metatemp; 
      }
      
      std::string soft_remove_sql([[maybe_unused]] const std::string &fieldsql){
          std::string temp;
     
         return temp;
     }
     

  inline  std::string stringaddslash(std::string_view content){
        std::string temp;
        temp.reserve(content.size());
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
  inline  std::string jsonaddslash(std::string_view content){
        std::string temp;
        temp.reserve(content.size());
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
        for(;j<fk_parent_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                   // tempsql<<"`";
                }
                tempsql<<fk_parent_info::col_names[j];
        }
        if(j>0){
            //tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.id==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.id);
}
tempsql<<",'"<<stringaddslash(data.name)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string make_data_insert_sql(const fk_parent_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<fk_parent_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                    //tempsql<<"`";
                }
                tempsql<<fk_parent_info::col_names[j];
        }
        if(j>0){
           // tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.id==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.id);
}
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string make_vector_insert_sql(const std::vector<fk_parent_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<fk_parent_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                   // tempsql<<"`";
                }
                tempsql<<fk_parent_info::col_names[j];
        }
        if(j>0){
           //tempsql<<"`";
        }
        tempsql<<") VALUES ";

        for(unsigned int i=0;i<insert_data.size();i++)
        {
            if(i>0)
            {
                tempsql<<",";	
            }
            tempsql<<"(";

            	if(insert_data[i].id==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].id);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
		tempsql<<")";
	 } 

     
       return tempsql.str();
   } 
       
    std::string make_update_sql(std::string_view field){
        std::ostringstream tempsql;
        tempsql<<"UPDATE ";
        tempsql<<tablename;
        tempsql<<" SET ";

        bool isall=false;
        if(field.empty()){
            isall=true;
        }
        if(isall){

        if(data.id==0){
	tempsql<<"id=0";
 }else{ 
	tempsql<<"id="<<std::to_string(data.id);
}
tempsql<<",name='"<<stringaddslash(data.name)<<"'";
 }else{ 

     
  unsigned int jj=0;
                  std::string keyname;
                  std::vector<unsigned char> keypos;
                  for(;jj<field.size();jj++){
                        if(field[jj]==','){
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
                        if(field[jj]==0x20){

                             continue;   
                        }
                        keyname.push_back(field[jj]);

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
if(data.id==0){
	tempsql<<"id=0";
 }else{ 
	tempsql<<"id="<<std::to_string(data.id);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"name='"<<stringaddslash(data.name)<<"'";
 break;

     
                  default:
                                ;
                     }
                 }   

            }        

        return tempsql.str();
   } 
   
   std::string make_update_dirty_sql()
   {
    std::ostringstream tempsql;
    tempsql << "UPDATE " << tablename << " SET ";

    constexpr std::size_t total = fk_parent_info::col_names.size();

    bool first = true;
    for (std::size_t idx = 0; idx < total; ++idx) {
        if (dirty_bits.test(idx)) {
            if (idx < total) {
                if (!first) tempsql << ",";
                switch (idx) {
                    case 0:
                        if(data.id==0){
                            tempsql<<"id=0";
                        }else{ 
                            tempsql<<"id="<<std::to_string(data.id);
                        }
                        break;
                    case 1:
                        tempsql<<"name='"<<stringaddslash(data.name)<<"'";
                        break;
                }
                first = false;
            }
        }
    }
    if (first) return "";
    return tempsql.str();
   } 

    std::string make_record_replace_sql()
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
            tempsql << "REPLACE INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < fk_parent_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << ",";
            }
            else
            {
                tempsql << "";
            }
            tempsql << fk_parent_info::col_names[j];
        }
        if (j > 0)
        {
            tempsql << "";
        }
        tempsql << ") VALUES ";

        for (unsigned int i = 0; i < record.size(); i++)
        {
            if (i > 0)
            {
                tempsql << ",\n";
            }
            tempsql << "(";
            	if(record[i].id==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].id);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<")";
  }
 
 return tempsql.str();
}

    std::string make_record_into_sql(std::string_view field)
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
        tempsql << "INSERT INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < fk_parent_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << ",";
            }
            else
            {
                tempsql << "";
            }
            tempsql << fk_parent_info::col_names[j];
        }
        if (j > 0)
        {
            tempsql << "";
        }
        tempsql << ") VALUES ";

        for (unsigned int i = 0; i < record.size(); i++)
        {
            if (i > 0)
            {
                tempsql << ",\n";
            }
            tempsql << "(";
            	if(record[i].id==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].id);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<")";
	 }
	 tempsql<<" as new ON DUPLICATE KEY UPDATE ";

     
        std::string keyname;
        unsigned char jj=0;
        j=0;
        if(field.size()>0){
        for(;jj<field.size();jj++){
            if(field[jj]==','){
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
            if(field[jj]==0x20){
                continue;   
            }
            keyname.push_back(field[jj]);

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

   std::vector<std::string> data_toarray(std::string_view field=""){
        std::vector<std::string> temparray;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(field.size()>1){
            for(;jj<field.size();jj++){
                if(field[jj]==','){
                    if(findcolpos(keyname)<255)
                    {
                        keypos.emplace_back(findcolpos(keyname)); 
                    }
                    keyname.clear();
                    continue;   
                }
                if(field[jj]==0x20){

                    continue;   
                }
                keyname.push_back(field[jj]);

            }  
            if(keyname.size()>0){
                if(findcolpos(keyname)<255)
                {
                    keypos.emplace_back(findcolpos(keyname)); 
                }
                keyname.clear();
            }
        }else{
            for(jj=0;jj<fk_parent_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.id==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.id));
}
 break;
 case 1:
	temparray.push_back(data.name);
 break;

                             default:
                                ;
                     }
                 }   
   
     return temparray;             
   }   
   
   std::map<std::string,std::string> data_tomap(std::string_view field=""){
       std::map<std::string,std::string> tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(field.size()>1){
        for(;jj<field.size();jj++){
            if(field[jj]==','){
                if(findcolpos(keyname)<255)
                {
                    keypos.emplace_back(findcolpos(keyname)); 
                }
                keyname.clear();
                continue;   
            }
            if(field[jj]==0x20){

                continue;   
            }
            keyname.push_back(field[jj]);

        }  
        if(keyname.size()>0){
            if(findcolpos(keyname)<255)
            {
                keypos.emplace_back(findcolpos(keyname)); 
            }
            keyname.clear();
        }
        }else{
            for(jj=0;jj<fk_parent_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.id==0){
	tempsql.insert({"id","0"});
 }else{ 
	tempsql.insert({"id",std::to_string(data.id)});
}
 break;
 case 1:
	tempsql.insert({"name",data.name});
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
if(data.id==0){
	tempsql<<"\"id\":0";
 }else{ 
	tempsql<<"\"id\":"<<std::to_string(data.id);
}
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
tempsql<<"}";

     
     return tempsql.str();             
   }   
   
   std::string data_tojson(std::string field){
        std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(field.size()>0){
        for(;jj<field.size();jj++){
            if(field[jj]==','){
                if(findcolpos(keyname)<255)
                {
                    keypos.emplace_back(findcolpos(keyname)); 
                }
                keyname.clear();
                continue;   
            }
            if(field[jj]==0x20){

                continue;   
            }
            keyname.push_back(field[jj]);

        }  
        if(keyname.size()>0){
            if(findcolpos(keyname)<255)
            {
                keypos.emplace_back(findcolpos(keyname)); 
            }
            keyname.clear();
        }
        }else{
            for(jj=0;jj<fk_parent_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.id==0){
	tempsql<<"\"id\":0";
 }else{ 
	tempsql<<"\"id\":"<<std::to_string(data.id);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
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
        fk_parent_info::meta metatemp; 
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
                                    bool json_value_skipped=false;
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
                                        if(json_content[json_offset]=='{'||json_content[json_offset]==']')
                                        {
                                            json_value_skipped=true;
                                        }
                                        else
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
                                    if(!json_value_skipped)
                                    {
                                        set_val(json_key_name,json_value_name);
                                    }
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
                                bool json_value_skipped=false;
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
                                    if(json_content[json_offset]=='{'||json_content[json_offset]==']')
                                    {
                                        json_value_skipped=true;
                                    }
                                    else
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
                                if(!json_value_skipped)
                                {
                                    set_val(json_key_name,json_value_name);
                                }
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
		  http::json_set_val(data.id,set_value_name);
		 break;
		
		case 1:
		  http::json_set_val(data.name,set_value_name);
		 break;
		
		default:
		 { }
			


        }
   } 
    
   std::string to_json(std::string_view field=""){
    std::ostringstream tempsql;
    std::string keyname;
    unsigned char jj=0;
    std::vector<unsigned char> keypos;
    if(field.size()>0){
        for(;jj<field.size();jj++){
            if(field[jj]==','){
                if(findcolpos(keyname)<255)
                {
                    keypos.emplace_back(findcolpos(keyname)); 
                }
                keyname.clear();
                continue;   
            }
            if(field[jj]==0x20){

                continue;   
            }
            keyname.push_back(field[jj]);

        }  
        if(keyname.size()>0){
            if(findcolpos(keyname)<255)
            {
                keypos.emplace_back(findcolpos(keyname)); 
            }
            keyname.clear();
        }
    }else{
        for(jj=0;jj<fk_parent_info::col_names.size();jj++){
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
if(record[n].id==0){
	tempsql<<"\"id\":0";
 }else{ 
	tempsql<<"\"id\":"<<std::to_string(record[n].id);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
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
   
   std::string to_json(std::function<bool(std::string&,fk_parent_info::meta&)> func,std::string_view field=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(field.size()>0){
            for(;jj<field.size();jj++){
                if(field[jj]==','){
                    if(findcolpos(keyname)<255)
                    {
                        keypos.emplace_back(findcolpos(keyname)); 
                    }
                    keyname.clear();
                    continue;   
                }
                if(field[jj]==0x20){

                    continue;   
                }
                keyname.push_back(field[jj]);

            }  
            if(keyname.size()>0){
                if(findcolpos(keyname)<255)
                {
                    keypos.emplace_back(findcolpos(keyname)); 
                }
                keyname.clear();
            }
        }else{
            for(jj=0;jj<fk_parent_info::col_names.size();jj++){
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
if(record[n].id==0){
	tempsql<<"\"id\":0";
 }else{ 
	tempsql<<"\"id\":"<<std::to_string(record[n].id);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
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
   long long getPK(){  return data.id; } 
 void setPK(long long val){  data.id=val;} 
 int  getId(){  return data.id; } 
 void setId( int  val){  data.id=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;
		 set_dirty(1);  }
 void setName(std::string_view val){  data.name=val;
		 set_dirty(1);  }

fk_parent_info::meta getnewData(){
 	 struct fk_parent_info::meta newdata;
	 return newdata; 
} 
fk_parent_info::meta getData(){
 	 return data; 
} 
std::vector<fk_parent_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<fk_parent_info::meta_tree> &tree_data, std::string_view field=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(field.size()>0){
            for(;jj<field.size();jj++){
                if(field[jj]==','){
                    if(findcolpos(keyname)<255)
                    {
                        keypos.emplace_back(findcolpos(keyname)); 
                    }
                    keyname.clear();
                    continue;   
                }
                if(field[jj]==0x20){

                    continue;   
                }
                keyname.push_back(field[jj]);

            }  
            if(keyname.size()>0){
                if(findcolpos(keyname)<255)
                {
                            keypos.emplace_back(findcolpos(keyname)); 
                }
                            keyname.clear();
            }
        }else{
            for(jj=0;jj<fk_parent_info::col_names.size();jj++){
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
if(tree_data[n].id==0){
	tempsql<<"\"id\":0";
 }else{ 
	tempsql<<"\"id\":"<<std::to_string(tree_data[n].id);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;

                             default:
                                ;
                     }
                 }

        tempsql<<",\"children\":";
         tempsql<<tree_tojson(tree_data[n].children, field);     
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   
   std::string tree_tojson(const std::vector<fk_parent_info::meta_tree> &tree_data,std::function<bool(std::string&,const fk_parent_info::meta_tree&)> func,std::string_view field=""){
       std::ostringstream tempsql;
        std::string keyname;
        unsigned char jj=0;
        std::vector<unsigned char> keypos;
        if(field.size()>0){
            for(;jj<field.size();jj++){
                if(field[jj]==','){
                    if(findcolpos(keyname)<255)
                    {
                        keypos.emplace_back(findcolpos(keyname)); 
                    }
                    keyname.clear();
                    continue;   
                }
                if(field[jj]==0x20){

                    continue;   
                }
                keyname.push_back(field[jj]);

            }  
            if(keyname.size()>0){
                if(findcolpos(keyname)<255)
                {
                            keypos.emplace_back(findcolpos(keyname)); 
                }
                            keyname.clear();
            }
        }else{
            for(jj=0;jj<fk_parent_info::col_names.size();jj++){
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
if(tree_data[n].id==0){
	tempsql<<"\"id\":0";
 }else{ 
	tempsql<<"\"id\":"<<std::to_string(tree_data[n].id);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;

                             default:
                                ;
                     }
                 }   
         tempsql<<",\"children\":";
         tempsql<<tree_tojson(tree_data[n].children,func,field);     
      tempsql<<"}";  
            }
      tempsql<<"]";
     return tempsql.str();             
   }   
   
    template<fk_parent_info::cols KeyCol, fk_parent_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>()));
        using ValType = decltype(fk_parent_info::getField<ValCol>(std::declval<const fk_parent_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(fk_parent_info::getField<KeyCol>(iter), fk_parent_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<fk_parent_info::cols KeyCol, fk_parent_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>())), 
            decltype(fk_parent_info::getField<ValCol>(std::declval<const fk_parent_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>())), 
                    decltype(fk_parent_info::getField<ValCol>(std::declval<const fk_parent_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>()));
        using ValType = decltype(fk_parent_info::getField<ValCol>(std::declval<const fk_parent_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(fk_parent_info::getField<KeyCol>(iter), fk_parent_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(fk_parent_info::getField<KeyCol>(iter), fk_parent_info::getField<ValCol>(iter))) {
                    result.emplace(fk_parent_info::getField<KeyCol>(iter), fk_parent_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<fk_parent_info::cols KeyCol, fk_parent_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>()));
        using ValType = decltype(fk_parent_info::getField<ValCol>(std::declval<const fk_parent_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(fk_parent_info::getField<KeyCol>(iter), fk_parent_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<fk_parent_info::cols KeyCol, fk_parent_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>())), 
            decltype(fk_parent_info::getField<ValCol>(std::declval<const fk_parent_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>())), 
                    decltype(fk_parent_info::getField<ValCol>(std::declval<const fk_parent_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>()));
        using ValType = decltype(fk_parent_info::getField<ValCol>(std::declval<const fk_parent_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(fk_parent_info::getField<KeyCol>(iter), fk_parent_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(fk_parent_info::getField<KeyCol>(iter), fk_parent_info::getField<ValCol>(iter))) {
                    result.emplace_back(fk_parent_info::getField<KeyCol>(iter), fk_parent_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<fk_parent_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(fk_parent_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<fk_parent_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(fk_parent_info::getField<KeyCol>(std::declval<const fk_parent_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(fk_parent_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(fk_parent_info::getField<KeyCol>(iter))) {
                    result.emplace_back(fk_parent_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<fk_parent_info::cols Col>
        requires requires(std::ostream& os, decltype(fk_parent_info::getField<Col>(std::declval<const fk_parent_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << fk_parent_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<fk_parent_info::cols Col>
        requires requires(std::ostream& os, decltype(fk_parent_info::getField<Col>(std::declval<const fk_parent_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << fk_parent_info::getField<Col>(iter); 
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
#endif
   