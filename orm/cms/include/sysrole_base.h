#ifndef ORM_CMS_SYSROLEBASEMATA_H
#define ORM_CMS_SYSROLEBASEMATA_H
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

namespace sysrole_info
{
 
    enum class cols : unsigned char 
    {
		roleid = 0,
		userid = 1,
		name = 2,
		status = 3,
		rolecode = 4,
		sortid = 5,
		rolevalue = 6,

    };

    struct meta
    {
		 unsigned  int  roleid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 std::string  name = ""; ///**/
		 unsigned  char  status = 0; ///**/
		 unsigned  long long  rolecode = 0; ///**/
		 int  sortid = 0; ///**/
		 unsigned  int  rolevalue = 0; ///**/
	};
  
    struct meta_tree
    {
		 unsigned  int  roleid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 std::string  name = ""; ///**/
		 unsigned  char  status = 0; ///**/
		 unsigned  long long  rolecode = 0; ///**/
		 int  sortid = 0; ///**/
		 unsigned  int  rolevalue = 0; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  roleid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 std::string  name = ""; ///**/
		 unsigned  char  status = 0; ///**/
		 unsigned  long long  rolecode = 0; ///**/
		 int  sortid = 0; ///**/
		 unsigned  int  rolevalue = 0; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::roleid) { 
		 return m.roleid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::name) { 
		 return m.name;
		} else if constexpr (Col == cols::status) { 
		 return m.status;
		} else if constexpr (Col == cols::rolecode) { 
		 return m.rolecode;
		} else if constexpr (Col == cols::sortid) { 
		 return m.sortid;
		} else if constexpr (Col == cols::rolevalue) { 
		 return m.rolevalue;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using roleid = decltype(std::declval<const meta>().roleid);
		using userid = decltype(std::declval<const meta>().userid);
		using name = decltype(std::declval<const meta>().name);
		using status = decltype(std::declval<const meta>().status);
		using rolecode = decltype(std::declval<const meta>().rolecode);
		using sortid = decltype(std::declval<const meta>().sortid);
		using rolevalue = decltype(std::declval<const meta>().rolevalue);

    }

    
    #define ORM_CMS_SYSROLE_META_FIELD_TYPE(col) \
        decltype(orm::cms::sysrole_info::getField<orm::cms::sysrole_info::cols::col>(std::declval<const orm::cms::sysrole_info::meta&>()))

    #define ORM_CMS_SYSROLE_PROJ_MEMBER(col) \
          ORM_CMS_SYSROLE_META_FIELD_TYPE(col) col;
                 
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_1(c1) \
        ORM_CMS_SYSROLE_PROJ_MEMBER(c1) 
     
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_2( c1, c2) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_1( c1) ORM_CMS_SYSROLE_PROJ_MEMBER(c2)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_3( c1, c2, c3) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_2( c1, c2) ORM_CMS_SYSROLE_PROJ_MEMBER(c3)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_4( c1, c2, c3, c4) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_3( c1, c2, c3) ORM_CMS_SYSROLE_PROJ_MEMBER(c4)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_4( c1, c2, c3, c4) ORM_CMS_SYSROLE_PROJ_MEMBER(c5)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) ORM_CMS_SYSROLE_PROJ_MEMBER(c6)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) ORM_CMS_SYSROLE_PROJ_MEMBER(c7)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) ORM_CMS_SYSROLE_PROJ_MEMBER(c8)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) ORM_CMS_SYSROLE_PROJ_MEMBER(c9)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) ORM_CMS_SYSROLE_PROJ_MEMBER(c10)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) ORM_CMS_SYSROLE_PROJ_MEMBER(c11)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) ORM_CMS_SYSROLE_PROJ_MEMBER(c12)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) ORM_CMS_SYSROLE_PROJ_MEMBER(c13)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) ORM_CMS_SYSROLE_PROJ_MEMBER(c14)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) ORM_CMS_SYSROLE_PROJ_MEMBER(c15)
         
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
        ORM_CMS_SYSROLE_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) ORM_CMS_SYSROLE_PROJ_MEMBER(c16)
         
    #define ORM_CMS_SYSROLE_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_SYSROLE_PROJ_MEMBERS(...) \
        ORM_CMS_SYSROLE_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_16, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_15, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_14, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_13, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_12, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_11, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_10, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_9, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_8, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_7, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_6, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_5, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_4, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_3, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_2, \
            ORM_CMS_SYSROLE_PROJ_MEMBERS_1, \
        )(__VA_ARGS__)

    
    #define ORM_CMS_SYSROLE_COUNT(...) \
        ORM_CMS_SYSROLE_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
    
    
    #define ORM_CMS_SYSROLE_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_SYSROLE_TO_JSON_1(c1) \
         ORM_CMS_SYSROLE_TO_JSON_ITEM(c1)
        
    #define ORM_CMS_SYSROLE_TO_JSON_2(c1,c2) \
         ORM_CMS_SYSROLE_TO_JSON_1(c1); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c2) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_SYSROLE_TO_JSON_2(c1,c2); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c3) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_SYSROLE_TO_JSON_3(c1,c2,c3); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c4) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_SYSROLE_TO_JSON_4(c1,c2,c3,c4); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c5) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_SYSROLE_TO_JSON_5(c1,c2,c3,c4,c5); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c6) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_SYSROLE_TO_JSON_6(c1,c2,c3,c4,c5,c6); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c7) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_SYSROLE_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c8) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_SYSROLE_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c9) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_SYSROLE_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c10) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_SYSROLE_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c11) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_SYSROLE_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c12) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_SYSROLE_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c13) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_SYSROLE_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c14) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_SYSROLE_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c15) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_SYSROLE_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15); \
            oss << ','; \
            ORM_CMS_SYSROLE_TO_JSON_ITEM(c16) 
        
        
    #define ORM_CMS_SYSROLE_TO_JSON_BODY(...) \
        ORM_CMS_SYSROLE_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_SYSROLE_TO_JSON_16,ORM_CMS_SYSROLE_TO_JSON_15,ORM_CMS_SYSROLE_TO_JSON_14,ORM_CMS_SYSROLE_TO_JSON_13,ORM_CMS_SYSROLE_TO_JSON_12,ORM_CMS_SYSROLE_TO_JSON_11,ORM_CMS_SYSROLE_TO_JSON_10,ORM_CMS_SYSROLE_TO_JSON_9,ORM_CMS_SYSROLE_TO_JSON_8,ORM_CMS_SYSROLE_TO_JSON_7,ORM_CMS_SYSROLE_TO_JSON_6,ORM_CMS_SYSROLE_TO_JSON_5,ORM_CMS_SYSROLE_TO_JSON_4,ORM_CMS_SYSROLE_TO_JSON_3,ORM_CMS_SYSROLE_TO_JSON_2,ORM_CMS_SYSROLE_TO_JSON_1 \
         )(__VA_ARGS__)
         
          
    #define ORM_CMS_SYSROLE_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_1(n1) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n1)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_2(n1,n2) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_1(n1) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n2)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_3(n1,n2,n3) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_2(n1,n2) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n3)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_4(n1,n2,n3,n4) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_3(n1,n2,n3) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n4)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_4(n1,n2,n3,n4) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n5)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n6)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n7)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n8)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n9)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n10)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n11)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n12)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n13)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n14)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n15)

#define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15) ORM_CMS_SYSROLE_TO_JSON_CUSTOM_ITEM(n16)


    #define ORM_CMS_SYSROLE_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_SYSROLE_TO_JSON_CUSTOM_##N

    #define ORM_CMS_SYSROLE_TO_JSON_CUSTOM(...) \
        ORM_CMS_SYSROLE_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__)


    #define ORM_CMS_SYSROLE_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::sysrole_info { \
            struct StructName { \
                ORM_CMS_SYSROLE_PROJ_MEMBERS(__VA_ARGS__) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SYSROLE_TO_JSON_BODY(__VA_ARGS__); \
                oss << '}'; \
                return oss.str(); \
            } \
            }; \
       }
        
    
    #define ORM_CMS_SYSROLE_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::sysrole_info { \
            struct StructName { \
                ORM_CMS_SYSROLE_PROJ_MEMBERS(__VA_ARGS__) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SYSROLE_TO_JSON_BODY(__VA_ARGS__); \
                ORM_CMS_SYSROLE_TO_JSON_CUSTOM(ORM_CMS_SYSROLE_UNWRAP CustomNames)  \
                oss << '}'; \
                return oss.str(); \
            } \
            }; \
       }
        
    
    #define ORM_CMS_SYSROLE_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::sysrole_info { \
            struct StructName { \
                ORM_CMS_SYSROLE_PROJ_MEMBERS(__VA_ARGS__) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SYSROLE_TO_JSON_BODY(__VA_ARGS__); \
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
        
    
    #define ORM_CMS_SYSROLE_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::sysrole_info { \
            struct StructName { \
                ORM_CMS_SYSROLE_PROJ_MEMBERS(__VA_ARGS__) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SYSROLE_TO_JSON_BODY(__VA_ARGS__); \
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
        
    static constexpr std::array<std::string_view,7> col_names={"roleid","userid","name","status","rolecode","sortid","rolevalue"};
	static constexpr std::array<unsigned char,7> col_types={3,3,253,1,8,3,3};
	static constexpr std::array<unsigned char,7> col_length={0,0,60,0,0,0,0};
	static constexpr std::array<unsigned char,7> col_decimals={0,0,0,0,0,0,0};

}

struct sysrole_base
{
      sysrole_info::meta data;
    std::vector<sysrole_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<sysrole_info::meta>::iterator begin(){     return record.begin(); }
std::vector<sysrole_info::meta>::iterator end(){     return record.end(); }
std::vector<sysrole_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<sysrole_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="sysrole";
static constexpr std::string_view modelname="Sysrole";

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


         case 'n':
   	 return 2;
break;
case 'r':
 switch(coln.size()){  
case 6:
   	 return 0;
break;
case 8:
   	 return 4;
break;
case 9:
   	 return 6;
break;
 }
 break;
case 's':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 5; }
 if(colpospppc=='s'){ return 3; }
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
       return "roleid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     sysrole_info::meta metatemp;    
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
        for(;j<sysrole_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<sysrole_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.roleid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.roleid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
tempsql<<",'"<<stringaddslash(data.name)<<"'";
if(data.status==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.status);
}
if(data.rolecode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.rolecode);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
if(data.rolevalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.rolevalue);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const sysrole_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<sysrole_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<sysrole_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.roleid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.roleid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
if(insert_data.status==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.status);
}
if(insert_data.rolecode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.rolecode);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
if(insert_data.rolevalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.rolevalue);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<sysrole_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<sysrole_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<sysrole_info::col_names[j];
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


            	if(insert_data[i].roleid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].roleid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
	if(insert_data[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].status);
	}
	if(insert_data[i].rolecode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].rolecode);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
	if(insert_data[i].rolevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].rolevalue);
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

        if(data.roleid==0){
	tempsql<<"`roleid`=0";
 }else{ 
	tempsql<<"`roleid`="<<std::to_string(data.roleid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
if(data.status==0){
	tempsql<<",`status`=0";
 }else{ 
	tempsql<<",`status`="<<std::to_string(data.status);
}
if(data.rolecode==0){
	tempsql<<",`rolecode`=0";
 }else{ 
	tempsql<<",`rolecode`="<<std::to_string(data.rolecode);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
if(data.rolevalue==0){
	tempsql<<",`rolevalue`=0";
 }else{ 
	tempsql<<",`rolevalue`="<<std::to_string(data.rolevalue);
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
if(data.roleid==0){
	tempsql<<"`roleid`=0";
 }else{ 
	tempsql<<"`roleid`="<<std::to_string(data.roleid);
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
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.status==0){
	tempsql<<"`status`=0";
 }else{ 
	tempsql<<"`status`="<<std::to_string(data.status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.rolecode==0){
	tempsql<<"`rolecode`=0";
 }else{ 
	tempsql<<"`rolecode`="<<std::to_string(data.rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.rolevalue==0){
	tempsql<<"`rolevalue`=0";
 }else{ 
	tempsql<<"`rolevalue`="<<std::to_string(data.rolevalue);
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
        for (; j < sysrole_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << sysrole_info::col_names[j];
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
            	if(record[i].roleid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].roleid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].status);
	}
	if(record[i].rolecode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rolecode);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	if(record[i].rolevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rolevalue);
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
        for (; j < sysrole_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << sysrole_info::col_names[j];
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
            	if(record[i].roleid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].roleid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].status);
	}
	if(record[i].rolecode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rolecode);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	if(record[i].rolevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rolevalue);
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
            for(jj=0;jj<sysrole_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.roleid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.roleid));
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
	temparray.push_back(data.name);
 break;
 case 3:
if(data.status==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.status));
}
 break;
 case 4:
if(data.rolecode==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.rolecode));
}
 break;
 case 5:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 6:
if(data.rolevalue==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.rolevalue));
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
            for(jj=0;jj<sysrole_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.roleid==0){
	tempsql.insert({"roleid","0"});
 }else{ 
	tempsql.insert({"roleid",std::to_string(data.roleid)});
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
	tempsql.insert({"name",data.name});
 break;
 case 3:
if(data.status==0){
	tempsql.insert({"status","0"});
 }else{ 
	tempsql.insert({"status",std::to_string(data.status)});
}
 break;
 case 4:
if(data.rolecode==0){
	tempsql.insert({"rolecode","0"});
 }else{ 
	tempsql.insert({"rolecode",std::to_string(data.rolecode)});
}
 break;
 case 5:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 6:
if(data.rolevalue==0){
	tempsql.insert({"rolevalue","0"});
 }else{ 
	tempsql.insert({"rolevalue",std::to_string(data.rolevalue)});
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
if(data.roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(data.roleid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
if(data.status==0){
	tempsql<<",\"status\":0";
 }else{ 
	tempsql<<",\"status\":"<<std::to_string(data.status);
}
if(data.rolecode==0){
	tempsql<<",\"rolecode\":0";
 }else{ 
	tempsql<<",\"rolecode\":"<<std::to_string(data.rolecode);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
if(data.rolevalue==0){
	tempsql<<",\"rolevalue\":0";
 }else{ 
	tempsql<<",\"rolevalue\":"<<std::to_string(data.rolevalue);
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
            for(jj=0;jj<sysrole_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(data.roleid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(data.status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.rolecode==0){
	tempsql<<"\"rolecode\":0";
 }else{ 
	tempsql<<"\"rolecode\":"<<std::to_string(data.rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.rolevalue==0){
	tempsql<<"\"rolevalue\":0";
 }else{ 
	tempsql<<"\"rolevalue\":"<<std::to_string(data.rolevalue);
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
        sysrole_info::meta metatemp; 
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
			data.roleid=std::stoul(set_value_name);
		}catch (...) { 
			data.roleid=0;
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
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.status=std::stoi(set_value_name);
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 4:
		 try{
			data.rolecode=std::stoull(set_value_name);
		}catch (...) { 
			data.rolecode=0;
			 }
			break;
		case 5:
		 try{
			data.sortid=std::stoi(set_value_name);
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 6:
		 try{
			data.rolevalue=std::stoul(set_value_name);
		}catch (...) { 
			data.rolevalue=0;
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
			data.roleid=set_value_name;
		}catch (...) { 
			data.roleid=0;
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
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.status=set_value_name;
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 4:
		 try{
			data.rolecode=set_value_name;
		}catch (...) { 
			data.rolecode=0;
			 }
			break;
		case 5:
		 try{
			data.sortid=set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 6:
		 try{
			data.rolevalue=set_value_name;
		}catch (...) { 
			data.rolevalue=0;
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
			data.roleid=(unsigned int)set_value_name;
		}catch (...) { 
			data.roleid=0;
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
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.status=(int)set_value_name;
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 4:
		 try{
			data.rolecode=(unsigned long long)set_value_name;
		}catch (...) { 
			data.rolecode=0;
			 }
			break;
		case 5:
		 try{
			data.sortid=(int)set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 6:
		 try{
			data.rolevalue=(unsigned int)set_value_name;
		}catch (...) { 
			data.rolevalue=0;
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
        for(jj=0;jj<sysrole_info::col_names.size();jj++){
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
if(record[n].roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(record[n].roleid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(record[n].status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].rolecode==0){
	tempsql<<"\"rolecode\":0";
 }else{ 
	tempsql<<"\"rolecode\":"<<std::to_string(record[n].rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].rolevalue==0){
	tempsql<<"\"rolevalue\":0";
 }else{ 
	tempsql<<"\"rolevalue\":"<<std::to_string(record[n].rolevalue);
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
   
   std::string to_json(std::function<bool(std::string&,sysrole_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<sysrole_info::col_names.size();jj++){
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
if(record[n].roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(record[n].roleid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(record[n].status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].rolecode==0){
	tempsql<<"\"rolecode\":0";
 }else{ 
	tempsql<<"\"rolecode\":"<<std::to_string(record[n].rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].rolevalue==0){
	tempsql<<"\"rolevalue\":0";
 }else{ 
	tempsql<<"\"rolevalue\":"<<std::to_string(record[n].rolevalue);
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
   long long getPK(){  return data.roleid; } 
 void setPK(long long val){  data.roleid=val;} 
 unsigned  int  getRoleid(){  return data.roleid; } 
 void setRoleid( unsigned  int  val){  data.roleid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 unsigned  char  getStatus(){  return data.status; } 
 void setStatus( unsigned  char  val){  data.status=val;} 

 unsigned  long long  getRolecode(){  return data.rolecode; } 
 void setRolecode( unsigned  long long  val){  data.rolecode=val;} 

 int  getSortid(){  return data.sortid; } 
 void setSortid( int  val){  data.sortid=val;} 

 unsigned  int  getRolevalue(){  return data.rolevalue; } 
 void setRolevalue( unsigned  int  val){  data.rolevalue=val;} 

sysrole_info::meta getnewData(){
 	 struct sysrole_info::meta newdata;
	 return newdata; 
} 
sysrole_info::meta getData(){
 	 return data; 
} 
std::vector<sysrole_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<sysrole_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<sysrole_info::col_names.size();jj++){
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
if(tree_data[n].roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(tree_data[n].roleid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(tree_data[n].status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].rolecode==0){
	tempsql<<"\"rolecode\":0";
 }else{ 
	tempsql<<"\"rolecode\":"<<std::to_string(tree_data[n].rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].rolevalue==0){
	tempsql<<"\"rolevalue\":0";
 }else{ 
	tempsql<<"\"rolevalue\":"<<std::to_string(tree_data[n].rolevalue);
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
   
   std::string tree_tojson(const std::vector<sysrole_info::meta_tree> &tree_data,std::function<bool(std::string&,const sysrole_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<sysrole_info::col_names.size();jj++){
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
if(tree_data[n].roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(tree_data[n].roleid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(tree_data[n].status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].rolecode==0){
	tempsql<<"\"rolecode\":0";
 }else{ 
	tempsql<<"\"rolecode\":"<<std::to_string(tree_data[n].rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].rolevalue==0){
	tempsql<<"\"rolevalue\":0";
 }else{ 
	tempsql<<"\"rolevalue\":"<<std::to_string(tree_data[n].rolevalue);
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
   
    template<sysrole_info::cols KeyCol, sysrole_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>()));
        using ValType = decltype(sysrole_info::getField<ValCol>(std::declval<const sysrole_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(sysrole_info::getField<KeyCol>(iter), sysrole_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<sysrole_info::cols KeyCol, sysrole_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>())), 
            decltype(sysrole_info::getField<ValCol>(std::declval<const sysrole_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>())), 
                    decltype(sysrole_info::getField<ValCol>(std::declval<const sysrole_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>()));
        using ValType = decltype(sysrole_info::getField<ValCol>(std::declval<const sysrole_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(sysrole_info::getField<KeyCol>(iter), sysrole_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(sysrole_info::getField<KeyCol>(iter), sysrole_info::getField<ValCol>(iter))) {
                    result.emplace(sysrole_info::getField<KeyCol>(iter), sysrole_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<sysrole_info::cols KeyCol, sysrole_info::cols ValCol> 
    auto get_vec_cols()
    {
        using KeyType = decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>()));
        using ValType = decltype(sysrole_info::getField<ValCol>(std::declval<const sysrole_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(sysrole_info::getField<KeyCol>(iter), sysrole_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<sysrole_info::cols KeyCol, sysrole_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>())), 
            decltype(sysrole_info::getField<ValCol>(std::declval<const sysrole_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>())), 
                    decltype(sysrole_info::getField<ValCol>(std::declval<const sysrole_info::meta&>()))>, bool>
    auto get_vec_cols(Callback&& callback)
    {
        using KeyType = decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>()));
        using ValType = decltype(sysrole_info::getField<ValCol>(std::declval<const sysrole_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(sysrole_info::getField<KeyCol>(iter), sysrole_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(sysrole_info::getField<KeyCol>(iter), sysrole_info::getField<ValCol>(iter))) {
                    result.emplace_back(sysrole_info::getField<KeyCol>(iter), sysrole_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<sysrole_info::cols KeyCol>
    auto get_vec_col()
    {
        using KeyType = decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(sysrole_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_col<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<sysrole_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>()))>, bool>
    auto get_vec_col(Callback&& callback)
    {
        using KeyType = decltype(sysrole_info::getField<KeyCol>(std::declval<const sysrole_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(sysrole_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(sysrole_info::getField<KeyCol>(iter))) {
                    result.emplace_back(sysrole_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<sysrole_info::cols Col>
        requires requires(std::ostream& os, decltype(sysrole_info::getField<Col>(std::declval<const sysrole_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << sysrole_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<sysrole_info::cols Col>
        requires requires(std::ostream& os, decltype(sysrole_info::getField<Col>(std::declval<const sysrole_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << sysrole_info::getField<Col>(iter); 
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
   