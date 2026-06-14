#ifndef ORM_CMS_DEPARTMENTBASEMATA_H
#define ORM_CMS_DEPARTMENTBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sun, 14 Jun 2026 12:13:59 GMT
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

namespace department_info
{
 
    enum class cols : unsigned char 
    {
		dpid = 0,
		userid = 1,
		parentid = 2,
		name = 3,
		depart_code = 4,
		bianzhi_num = 5,
		real_num = 6,
		quan_weight = 7,
		isopen = 8,
		memo = 9,
		created_time = 10,
		created_user = 11,
		updated_time = 12,
		updated_user = 13,
		isvirtual = 14,
		linkdpid = 15,

    };

    struct meta
    {
		 unsigned  int  dpid = 0; ///**/
		 unsigned  int  userid = 0; ///*用户id*/
		 unsigned  int  parentid = 0; ///*父id*/
		 std::string  name = ""; ///*部门名称*/
		 std::string  depart_code = ""; ///*部门代号*/
		 int  bianzhi_num = 0; ///*编制人数*/
		 int  real_num = 0; ///*实际人数*/
		 int  quan_weight = 0; ///*部门权重*/
		 char  isopen = 0; ///*是否开放*/
		 std::string  memo = ""; ///*备注*/
		 unsigned  int  created_time = 0; ///*添加时间*/
		 unsigned  int  created_user = 0; ///**/
		 unsigned  int  updated_time = 0; ///*修改时间*/
		 unsigned  int  updated_user = 0; ///**/
		 char  isvirtual = 0; ///*是否虚拟部门*/
		 std::string  linkdpid = ""; ///*关联部门*/
	};
  
    struct meta_tree
    {
		 unsigned  int  dpid = 0; ///**/
		 unsigned  int  userid = 0; ///*用户id*/
		 unsigned  int  parentid = 0; ///*父id*/
		 std::string  name = ""; ///*部门名称*/
		 std::string  depart_code = ""; ///*部门代号*/
		 int  bianzhi_num = 0; ///*编制人数*/
		 int  real_num = 0; ///*实际人数*/
		 int  quan_weight = 0; ///*部门权重*/
		 char  isopen = 0; ///*是否开放*/
		 std::string  memo = ""; ///*备注*/
		 unsigned  int  created_time = 0; ///*添加时间*/
		 unsigned  int  created_user = 0; ///**/
		 unsigned  int  updated_time = 0; ///*修改时间*/
		 unsigned  int  updated_user = 0; ///**/
		 char  isvirtual = 0; ///*是否虚拟部门*/
		 std::string  linkdpid = ""; ///*关联部门*/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  dpid = 0; ///**/
		 unsigned  int  userid = 0; ///*用户id*/
		 unsigned  int  parentid = 0; ///*父id*/
		 std::string  name = ""; ///*部门名称*/
		 std::string  depart_code = ""; ///*部门代号*/
		 int  bianzhi_num = 0; ///*编制人数*/
		 int  real_num = 0; ///*实际人数*/
		 int  quan_weight = 0; ///*部门权重*/
		 char  isopen = 0; ///*是否开放*/
		 std::string  memo = ""; ///*备注*/
		 unsigned  int  created_time = 0; ///*添加时间*/
		 unsigned  int  created_user = 0; ///**/
		 unsigned  int  updated_time = 0; ///*修改时间*/
		 unsigned  int  updated_user = 0; ///**/
		 char  isvirtual = 0; ///*是否虚拟部门*/
		 std::string  linkdpid = ""; ///*关联部门*/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::dpid) { 
		 return m.dpid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::parentid) { 
		 return m.parentid;
		} else if constexpr (Col == cols::name) { 
		 return m.name;
		} else if constexpr (Col == cols::depart_code) { 
		 return m.depart_code;
		} else if constexpr (Col == cols::bianzhi_num) { 
		 return m.bianzhi_num;
		} else if constexpr (Col == cols::real_num) { 
		 return m.real_num;
		} else if constexpr (Col == cols::quan_weight) { 
		 return m.quan_weight;
		} else if constexpr (Col == cols::isopen) { 
		 return m.isopen;
		} else if constexpr (Col == cols::memo) { 
		 return m.memo;
		} else if constexpr (Col == cols::created_time) { 
		 return m.created_time;
		} else if constexpr (Col == cols::created_user) { 
		 return m.created_user;
		} else if constexpr (Col == cols::updated_time) { 
		 return m.updated_time;
		} else if constexpr (Col == cols::updated_user) { 
		 return m.updated_user;
		} else if constexpr (Col == cols::isvirtual) { 
		 return m.isvirtual;
		} else if constexpr (Col == cols::linkdpid) { 
		 return m.linkdpid;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using dpid =  unsigned  int ;
		using userid =  unsigned  int ;
		using parentid =  unsigned  int ;
		using name =  std::string ;
		using depart_code =  std::string ;
		using bianzhi_num =  int ;
		using real_num =  int ;
		using quan_weight =  int ;
		using isopen =  char ;
		using memo =  std::string ;
		using created_time =  unsigned  int ;
		using created_user =  unsigned  int ;
		using updated_time =  unsigned  int ;
		using updated_user =  unsigned  int ;
		using isvirtual =  char ;
		using linkdpid =  std::string ;

    }

    
    #define ORM_CMS_DEPARTMENT_EXPAND(x) x 
    
    #define ORM_CMS_DEPARTMENT_META_FIELD_TYPE(col) \
         orm::cms::department_info::type::col 
    
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBER(col) \
          ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_1(c1) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_1( c1)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c2))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c3))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c4))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c5))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c6))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c7))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c8))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c9))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c10))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c11))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c12))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c13))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c14))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c15))
         
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBER(c16))
         
    #define ORM_CMS_DEPARTMENT_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_DEPARTMENT_PROJ_MEMBERS(...) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_16, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_15, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_14, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_13, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_12, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_11, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_10, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_9, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_8, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_7, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_6, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_5, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_4, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_3, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_2, \
            ORM_CMS_DEPARTMENT_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_DEPARTMENT_COUNT(...) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_DEPARTMENT_TO_JSON_1(c1) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_2(c1,c2) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_DEPARTMENT_TO_JSON_BODY(...) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_DEPARTMENT_TO_JSON_16,ORM_CMS_DEPARTMENT_TO_JSON_15,ORM_CMS_DEPARTMENT_TO_JSON_14,ORM_CMS_DEPARTMENT_TO_JSON_13,ORM_CMS_DEPARTMENT_TO_JSON_12,ORM_CMS_DEPARTMENT_TO_JSON_11,ORM_CMS_DEPARTMENT_TO_JSON_10,ORM_CMS_DEPARTMENT_TO_JSON_9,ORM_CMS_DEPARTMENT_TO_JSON_8,ORM_CMS_DEPARTMENT_TO_JSON_7,ORM_CMS_DEPARTMENT_TO_JSON_6,ORM_CMS_DEPARTMENT_TO_JSON_5,ORM_CMS_DEPARTMENT_TO_JSON_4,ORM_CMS_DEPARTMENT_TO_JSON_3,ORM_CMS_DEPARTMENT_TO_JSON_2,ORM_CMS_DEPARTMENT_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_DEPARTMENT_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_1(n1)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_1(n1)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_DEPARTMENT_CAT(a, b) ORM_CMS_DEPARTMENT_CAT_(a, b)
    #define ORM_CMS_DEPARTMENT_CAT_(a, b) a##b

    #define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_DEPARTMENT_CAT(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM(...) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_DEPARTMENT_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_DEPARTMENT_SET_VAL_1(c1) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_DEPARTMENT_SET_VAL_2(c1,c2) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_1(c1)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_2(c1,c2)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_DEPARTMENT_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_DEPARTMENT_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_DEPARTMENT_CAT(ORM_CMS_DEPARTMENT_SET_VAL_, N)
    
    
    #define ORM_CMS_DEPARTMENT_SET_VAL_FIELDS(...) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_DEPARTMENT_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_FIELDS(ORM_CMS_DEPARTMENT_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_DEPARTMENT_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::department_info { \
            struct StructName { \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_DEPARTMENT_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::department_info { \
            struct StructName { \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM(ORM_CMS_DEPARTMENT_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_DEPARTMENT_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::department_info { \
            struct StructName { \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_DEPARTMENT_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::department_info { \
            struct StructName { \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_DEPARTMENT_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::department_info { \
            struct StructName { \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_TO_JSON_CUSTOM(ORM_CMS_DEPARTMENT_UNWRAP CustomNames));  \
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
                        ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_DEPARTMENT_EXPAND(ORM_CMS_DEPARTMENT_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,16> col_names={"dpid","userid","parentid","name","depart_code","bianzhi_num","real_num","quan_weight","isopen","memo","created_time","created_user","updated_time","updated_user","isvirtual","linkdpid"};
	static constexpr std::array<unsigned char,16> col_types={3,3,3,253,253,3,3,3,1,252,3,3,3,3,1,253};
	static constexpr std::array<unsigned char,16> col_length={0,0,0,40,20,0,0,0,0,0,0,0,0,0,0,0};
	static constexpr std::array<unsigned char,16> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct department_base
{
      department_info::meta data;
    std::vector<department_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<department_info::meta>::iterator begin(){     return record.begin(); }
std::vector<department_info::meta>::iterator end(){     return record.end(); }
std::vector<department_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<department_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="department";
static constexpr std::string_view org_tablename="department";
static constexpr std::string_view modelname="Department";

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


         case 'b':
   	 return 5;
break;
case 'c':
 switch(coln.size()){  
case 12:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 10; }
 if(colpospppc=='r'){ return 11; }
   	 break;
 }
 break;
case 'd':
 switch(coln.size()){  
case 4:
   	 return 0;
break;
case 11:
   	 return 4;
break;
 }
 break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 8;
break;
case 9:
   	 return 14;
break;
 }
 break;
case 'l':
   	 return 15;
break;
case 'm':
   	 return 9;
break;
case 'n':
   	 return 3;
break;
case 'p':
   	 return 2;
break;
case 'q':
   	 return 7;
break;
case 'r':
   	 return 6;
break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 1;
break;
case 12:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 12; }
 if(colpospppc=='r'){ return 13; }
   	 break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "dpid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     department_info::meta metatemp;    
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
        for(;j<department_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<department_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.dpid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.dpid);
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
tempsql<<",'"<<stringaddslash(data.name)<<"'";
tempsql<<",'"<<stringaddslash(data.depart_code)<<"'";
if(data.bianzhi_num==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.bianzhi_num);
}
if(data.real_num==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.real_num);
}
if(data.quan_weight==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.quan_weight);
}
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
tempsql<<",'"<<stringaddslash(data.memo)<<"'";
if(data.created_time==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.created_time);
}
if(data.created_user==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.created_user);
}
if(data.updated_time==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.updated_time);
}
if(data.updated_user==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.updated_user);
}
if(data.isvirtual==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isvirtual);
}
tempsql<<",'"<<stringaddslash(data.linkdpid)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const department_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<department_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<department_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.dpid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.dpid);
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
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.depart_code)<<"'";
if(insert_data.bianzhi_num==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.bianzhi_num);
}
if(insert_data.real_num==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.real_num);
}
if(insert_data.quan_weight==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.quan_weight);
}
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
}
tempsql<<",'"<<stringaddslash(insert_data.memo)<<"'";
if(insert_data.created_time==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.created_time);
}
if(insert_data.created_user==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.created_user);
}
if(insert_data.updated_time==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.updated_time);
}
if(insert_data.updated_user==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.updated_user);
}
if(insert_data.isvirtual==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isvirtual);
}
tempsql<<",'"<<stringaddslash(insert_data.linkdpid)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<department_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<department_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<department_info::col_names[j];
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


            	if(insert_data[i].dpid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].dpid);
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
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].depart_code)<<"'";
	if(insert_data[i].bianzhi_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].bianzhi_num);
	}
	if(insert_data[i].real_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].real_num);
	}
	if(insert_data[i].quan_weight==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].quan_weight);
	}
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].memo)<<"'";
	if(insert_data[i].created_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].created_time);
	}
	if(insert_data[i].created_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].created_user);
	}
	if(insert_data[i].updated_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].updated_time);
	}
	if(insert_data[i].updated_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].updated_user);
	}
	if(insert_data[i].isvirtual==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isvirtual);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].linkdpid)<<"'";
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

        if(data.dpid==0){
	tempsql<<"`dpid`=0";
 }else{ 
	tempsql<<"`dpid`="<<std::to_string(data.dpid);
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
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
tempsql<<",`depart_code`='"<<stringaddslash(data.depart_code)<<"'";
if(data.bianzhi_num==0){
	tempsql<<",`bianzhi_num`=0";
 }else{ 
	tempsql<<",`bianzhi_num`="<<std::to_string(data.bianzhi_num);
}
if(data.real_num==0){
	tempsql<<",`real_num`=0";
 }else{ 
	tempsql<<",`real_num`="<<std::to_string(data.real_num);
}
if(data.quan_weight==0){
	tempsql<<",`quan_weight`=0";
 }else{ 
	tempsql<<",`quan_weight`="<<std::to_string(data.quan_weight);
}
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
tempsql<<",`memo`='"<<stringaddslash(data.memo)<<"'";
if(data.created_time==0){
	tempsql<<",`created_time`=0";
 }else{ 
	tempsql<<",`created_time`="<<std::to_string(data.created_time);
}
if(data.created_user==0){
	tempsql<<",`created_user`=0";
 }else{ 
	tempsql<<",`created_user`="<<std::to_string(data.created_user);
}
if(data.updated_time==0){
	tempsql<<",`updated_time`=0";
 }else{ 
	tempsql<<",`updated_time`="<<std::to_string(data.updated_time);
}
if(data.updated_user==0){
	tempsql<<",`updated_user`=0";
 }else{ 
	tempsql<<",`updated_user`="<<std::to_string(data.updated_user);
}
if(data.isvirtual==0){
	tempsql<<",`isvirtual`=0";
 }else{ 
	tempsql<<",`isvirtual`="<<std::to_string(data.isvirtual);
}
tempsql<<",`linkdpid`='"<<stringaddslash(data.linkdpid)<<"'";
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
if(data.dpid==0){
	tempsql<<"`dpid`=0";
 }else{ 
	tempsql<<"`dpid`="<<std::to_string(data.dpid);
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
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`depart_code`='"<<stringaddslash(data.depart_code)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.bianzhi_num==0){
	tempsql<<"`bianzhi_num`=0";
 }else{ 
	tempsql<<"`bianzhi_num`="<<std::to_string(data.bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.real_num==0){
	tempsql<<"`real_num`=0";
 }else{ 
	tempsql<<"`real_num`="<<std::to_string(data.real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.quan_weight==0){
	tempsql<<"`quan_weight`=0";
 }else{ 
	tempsql<<"`quan_weight`="<<std::to_string(data.quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`memo`='"<<stringaddslash(data.memo)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.created_time==0){
	tempsql<<"`created_time`=0";
 }else{ 
	tempsql<<"`created_time`="<<std::to_string(data.created_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.created_user==0){
	tempsql<<"`created_user`=0";
 }else{ 
	tempsql<<"`created_user`="<<std::to_string(data.created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.updated_time==0){
	tempsql<<"`updated_time`=0";
 }else{ 
	tempsql<<"`updated_time`="<<std::to_string(data.updated_time);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.updated_user==0){
	tempsql<<"`updated_user`=0";
 }else{ 
	tempsql<<"`updated_user`="<<std::to_string(data.updated_user);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.isvirtual==0){
	tempsql<<"`isvirtual`=0";
 }else{ 
	tempsql<<"`isvirtual`="<<std::to_string(data.isvirtual);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`linkdpid`='"<<stringaddslash(data.linkdpid)<<"'";
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
        for (; j < department_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << department_info::col_names[j];
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
            	if(record[i].dpid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].dpid);
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
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].depart_code)<<"'";
	if(record[i].bianzhi_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].bianzhi_num);
	}
	if(record[i].real_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].real_num);
	}
	if(record[i].quan_weight==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].quan_weight);
	}
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	tempsql<<",'"<<stringaddslash(record[i].memo)<<"'";
	if(record[i].created_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_time);
	}
	if(record[i].created_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_user);
	}
	if(record[i].updated_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updated_time);
	}
	if(record[i].updated_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updated_user);
	}
	if(record[i].isvirtual==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isvirtual);
	}
	tempsql<<",'"<<stringaddslash(record[i].linkdpid)<<"'";
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
        for (; j < department_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << department_info::col_names[j];
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
            	if(record[i].dpid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].dpid);
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
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].depart_code)<<"'";
	if(record[i].bianzhi_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].bianzhi_num);
	}
	if(record[i].real_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].real_num);
	}
	if(record[i].quan_weight==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].quan_weight);
	}
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	tempsql<<",'"<<stringaddslash(record[i].memo)<<"'";
	if(record[i].created_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_time);
	}
	if(record[i].created_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_user);
	}
	if(record[i].updated_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updated_time);
	}
	if(record[i].updated_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updated_user);
	}
	if(record[i].isvirtual==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isvirtual);
	}
	tempsql<<",'"<<stringaddslash(record[i].linkdpid)<<"'";
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
            for(jj=0;jj<department_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.dpid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.dpid));
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
	temparray.push_back(data.name);
 break;
 case 4:
	temparray.push_back(data.depart_code);
 break;
 case 5:
if(data.bianzhi_num==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.bianzhi_num));
}
 break;
 case 6:
if(data.real_num==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.real_num));
}
 break;
 case 7:
if(data.quan_weight==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.quan_weight));
}
 break;
 case 8:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 9:
	temparray.push_back(data.memo);
 break;
 case 10:
if(data.created_time==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.created_time));
}
 break;
 case 11:
if(data.created_user==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.created_user));
}
 break;
 case 12:
if(data.updated_time==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.updated_time));
}
 break;
 case 13:
if(data.updated_user==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.updated_user));
}
 break;
 case 14:
if(data.isvirtual==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isvirtual));
}
 break;
 case 15:
	temparray.push_back(data.linkdpid);
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
            for(jj=0;jj<department_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.dpid==0){
	tempsql.insert({"dpid","0"});
 }else{ 
	tempsql.insert({"dpid",std::to_string(data.dpid)});
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
	tempsql.insert({"name",data.name});
 break;
 case 4:
	tempsql.insert({"depart_code",data.depart_code});
 break;
 case 5:
if(data.bianzhi_num==0){
	tempsql.insert({"bianzhi_num","0"});
 }else{ 
	tempsql.insert({"bianzhi_num",std::to_string(data.bianzhi_num)});
}
 break;
 case 6:
if(data.real_num==0){
	tempsql.insert({"real_num","0"});
 }else{ 
	tempsql.insert({"real_num",std::to_string(data.real_num)});
}
 break;
 case 7:
if(data.quan_weight==0){
	tempsql.insert({"quan_weight","0"});
 }else{ 
	tempsql.insert({"quan_weight",std::to_string(data.quan_weight)});
}
 break;
 case 8:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 9:
	tempsql.insert({"memo",data.memo});
 break;
 case 10:
if(data.created_time==0){
	tempsql.insert({"created_time","0"});
 }else{ 
	tempsql.insert({"created_time",std::to_string(data.created_time)});
}
 break;
 case 11:
if(data.created_user==0){
	tempsql.insert({"created_user","0"});
 }else{ 
	tempsql.insert({"created_user",std::to_string(data.created_user)});
}
 break;
 case 12:
if(data.updated_time==0){
	tempsql.insert({"updated_time","0"});
 }else{ 
	tempsql.insert({"updated_time",std::to_string(data.updated_time)});
}
 break;
 case 13:
if(data.updated_user==0){
	tempsql.insert({"updated_user","0"});
 }else{ 
	tempsql.insert({"updated_user",std::to_string(data.updated_user)});
}
 break;
 case 14:
if(data.isvirtual==0){
	tempsql.insert({"isvirtual","0"});
 }else{ 
	tempsql.insert({"isvirtual",std::to_string(data.isvirtual)});
}
 break;
 case 15:
	tempsql.insert({"linkdpid",data.linkdpid});
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
if(data.dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(data.dpid);
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
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
tempsql<<",\"depart_code\":\""<<http::utf8_to_jsonstring(data.depart_code);
tempsql<<"\"";
if(data.bianzhi_num==0){
	tempsql<<",\"bianzhi_num\":0";
 }else{ 
	tempsql<<",\"bianzhi_num\":"<<std::to_string(data.bianzhi_num);
}
if(data.real_num==0){
	tempsql<<",\"real_num\":0";
 }else{ 
	tempsql<<",\"real_num\":"<<std::to_string(data.real_num);
}
if(data.quan_weight==0){
	tempsql<<",\"quan_weight\":0";
 }else{ 
	tempsql<<",\"quan_weight\":"<<std::to_string(data.quan_weight);
}
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
tempsql<<",\"memo\":\""<<http::utf8_to_jsonstring(data.memo);
tempsql<<"\"";
if(data.created_time==0){
	tempsql<<",\"created_time\":0";
 }else{ 
	tempsql<<",\"created_time\":"<<std::to_string(data.created_time);
}
if(data.created_user==0){
	tempsql<<",\"created_user\":0";
 }else{ 
	tempsql<<",\"created_user\":"<<std::to_string(data.created_user);
}
if(data.updated_time==0){
	tempsql<<",\"updated_time\":0";
 }else{ 
	tempsql<<",\"updated_time\":"<<std::to_string(data.updated_time);
}
if(data.updated_user==0){
	tempsql<<",\"updated_user\":0";
 }else{ 
	tempsql<<",\"updated_user\":"<<std::to_string(data.updated_user);
}
if(data.isvirtual==0){
	tempsql<<",\"isvirtual\":0";
 }else{ 
	tempsql<<",\"isvirtual\":"<<std::to_string(data.isvirtual);
}
tempsql<<",\"linkdpid\":\""<<http::utf8_to_jsonstring(data.linkdpid);
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
            for(jj=0;jj<department_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(data.dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(data.depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(data.bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(data.real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(data.quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(data.memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(data.created_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(data.created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(data.updated_time);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(data.updated_user);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(data.isvirtual);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(data.linkdpid)<<"\"";
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
        department_info::meta metatemp; 
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
			data.dpid=std::stoul(set_value_name);
		}catch (...) { 
			data.dpid=0;
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
			data.parentid=std::stoul(set_value_name);
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 3:
		 try{
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 4:
		 try{
			data.depart_code.append(set_value_name);
		}catch (...) { 
			data.depart_code.clear();
			 }
			break;
		case 5:
		 try{
			data.bianzhi_num=std::stoi(set_value_name);
		}catch (...) { 
			data.bianzhi_num=0;
			 }
			break;
		case 6:
		 try{
			data.real_num=std::stoi(set_value_name);
		}catch (...) { 
			data.real_num=0;
			 }
			break;
		case 7:
		 try{
			data.quan_weight=std::stoi(set_value_name);
		}catch (...) { 
			data.quan_weight=0;
			 }
			break;
		case 8:
		 try{
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 9:
		 try{
			data.memo.append(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 10:
		 try{
			data.created_time=std::stoul(set_value_name);
		}catch (...) { 
			data.created_time=0;
			 }
			break;
		case 11:
		 try{
			data.created_user=std::stoul(set_value_name);
		}catch (...) { 
			data.created_user=0;
			 }
			break;
		case 12:
		 try{
			data.updated_time=std::stoul(set_value_name);
		}catch (...) { 
			data.updated_time=0;
			 }
			break;
		case 13:
		 try{
			data.updated_user=std::stoul(set_value_name);
		}catch (...) { 
			data.updated_user=0;
			 }
			break;
		case 14:
		 try{
			data.isvirtual=std::stoi(set_value_name);
		}catch (...) { 
			data.isvirtual=0;
			 }
			break;
		case 15:
		 try{
			data.linkdpid.append(set_value_name);
		}catch (...) { 
			data.linkdpid.clear();
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
			data.dpid=set_value_name;
		}catch (...) { 
			data.dpid=0;
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
			data.parentid=set_value_name;
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 3:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 4:
		 try{
			data.depart_code=std::to_string(set_value_name);
		}catch (...) { 
			data.depart_code.clear();
			 }
			break;
		case 5:
		 try{
			data.bianzhi_num=set_value_name;
		}catch (...) { 
			data.bianzhi_num=0;
			 }
			break;
		case 6:
		 try{
			data.real_num=set_value_name;
		}catch (...) { 
			data.real_num=0;
			 }
			break;
		case 7:
		 try{
			data.quan_weight=set_value_name;
		}catch (...) { 
			data.quan_weight=0;
			 }
			break;
		case 8:
		 try{
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 9:
		 try{
			data.memo=std::to_string(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 10:
		 try{
			data.created_time=set_value_name;
		}catch (...) { 
			data.created_time=0;
			 }
			break;
		case 11:
		 try{
			data.created_user=set_value_name;
		}catch (...) { 
			data.created_user=0;
			 }
			break;
		case 12:
		 try{
			data.updated_time=set_value_name;
		}catch (...) { 
			data.updated_time=0;
			 }
			break;
		case 13:
		 try{
			data.updated_user=set_value_name;
		}catch (...) { 
			data.updated_user=0;
			 }
			break;
		case 14:
		 try{
			data.isvirtual=set_value_name;
		}catch (...) { 
			data.isvirtual=0;
			 }
			break;
		case 15:
		 try{
			data.linkdpid=std::to_string(set_value_name);
		}catch (...) { 
			data.linkdpid.clear();
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
			data.dpid=(unsigned int)set_value_name;
		}catch (...) { 
			data.dpid=0;
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
			data.parentid=(unsigned int)set_value_name;
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 3:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 4:
		 try{
			data.depart_code=std::to_string(set_value_name);
		}catch (...) { 
			data.depart_code.clear();
			 }
			break;
		case 5:
		 try{
			data.bianzhi_num=(int)set_value_name;
		}catch (...) { 
			data.bianzhi_num=0;
			 }
			break;
		case 6:
		 try{
			data.real_num=(int)set_value_name;
		}catch (...) { 
			data.real_num=0;
			 }
			break;
		case 7:
		 try{
			data.quan_weight=(int)set_value_name;
		}catch (...) { 
			data.quan_weight=0;
			 }
			break;
		case 8:
		 try{
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 9:
		 try{
			data.memo=std::to_string(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 10:
		 try{
			data.created_time=(unsigned int)set_value_name;
		}catch (...) { 
			data.created_time=0;
			 }
			break;
		case 11:
		 try{
			data.created_user=(unsigned int)set_value_name;
		}catch (...) { 
			data.created_user=0;
			 }
			break;
		case 12:
		 try{
			data.updated_time=(unsigned int)set_value_name;
		}catch (...) { 
			data.updated_time=0;
			 }
			break;
		case 13:
		 try{
			data.updated_user=(unsigned int)set_value_name;
		}catch (...) { 
			data.updated_user=0;
			 }
			break;
		case 14:
		 try{
			data.isvirtual=(int)set_value_name;
		}catch (...) { 
			data.isvirtual=0;
			 }
			break;
		case 15:
		 try{
			data.linkdpid=std::to_string(set_value_name);
		}catch (...) { 
			data.linkdpid.clear();
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
        for(jj=0;jj<department_info::col_names.size();jj++){
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
if(record[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(record[n].dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(record[n].depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(record[n].bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(record[n].real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(record[n].quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(record[n].memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(record[n].created_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(record[n].created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(record[n].updated_time);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(record[n].updated_user);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(record[n].isvirtual);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(record[n].linkdpid)<<"\"";
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
   
   std::string to_json(std::function<bool(std::string&,department_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<department_info::col_names.size();jj++){
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
if(record[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(record[n].dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(record[n].depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(record[n].bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(record[n].real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(record[n].quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(record[n].memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(record[n].created_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(record[n].created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(record[n].updated_time);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(record[n].updated_user);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(record[n].isvirtual);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(record[n].linkdpid)<<"\"";
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
   long long getPK(){  return data.dpid; } 
 void setPK(long long val){  data.dpid=val;} 
 unsigned  int  getDpid(){  return data.dpid; } 
 void setDpid( unsigned  int  val){  data.dpid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getParentid(){  return data.parentid; } 
 void setParentid( unsigned  int  val){  data.parentid=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 std::string  getDepartCode(){  return data.depart_code; } 
 std::string & getRefDepartCode(){  return std::ref(data.depart_code); } 
 void setDepartCode( std::string  &val){  data.depart_code=val;} 
 void setDepartCode(std::string_view val){  data.depart_code=val;} 

 int  getBianzhiNum(){  return data.bianzhi_num; } 
 void setBianzhiNum( int  val){  data.bianzhi_num=val;} 

 int  getRealNum(){  return data.real_num; } 
 void setRealNum( int  val){  data.real_num=val;} 

 int  getQuanWeight(){  return data.quan_weight; } 
 void setQuanWeight( int  val){  data.quan_weight=val;} 

 char  getIsopen(){  return data.isopen; } 
 void setIsopen( char  val){  data.isopen=val;} 

 std::string  getMemo(){  return data.memo; } 
 std::string & getRefMemo(){  return std::ref(data.memo); } 
 void setMemo( std::string  &val){  data.memo=val;} 
 void setMemo(std::string_view val){  data.memo=val;} 

 unsigned  int  getCreatedTime(){  return data.created_time; } 
 void setCreatedTime( unsigned  int  val){  data.created_time=val;} 

 unsigned  int  getCreatedUser(){  return data.created_user; } 
 void setCreatedUser( unsigned  int  val){  data.created_user=val;} 

 unsigned  int  getUpdatedTime(){  return data.updated_time; } 
 void setUpdatedTime( unsigned  int  val){  data.updated_time=val;} 

 unsigned  int  getUpdatedUser(){  return data.updated_user; } 
 void setUpdatedUser( unsigned  int  val){  data.updated_user=val;} 

 char  getIsvirtual(){  return data.isvirtual; } 
 void setIsvirtual( char  val){  data.isvirtual=val;} 

 std::string  getLinkdpid(){  return data.linkdpid; } 
 std::string & getRefLinkdpid(){  return std::ref(data.linkdpid); } 
 void setLinkdpid( std::string  &val){  data.linkdpid=val;} 
 void setLinkdpid(std::string_view val){  data.linkdpid=val;} 

department_info::meta getnewData(){
 	 struct department_info::meta newdata;
	 return newdata; 
} 
department_info::meta getData(){
 	 return data; 
} 
std::vector<department_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<department_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<department_info::col_names.size();jj++){
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
if(tree_data[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(tree_data[n].dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(tree_data[n].depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(tree_data[n].bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(tree_data[n].real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(tree_data[n].quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(tree_data[n].memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(tree_data[n].created_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(tree_data[n].created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(tree_data[n].updated_time);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(tree_data[n].updated_user);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(tree_data[n].isvirtual);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(tree_data[n].linkdpid)<<"\"";
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
   
   std::string tree_tojson(const std::vector<department_info::meta_tree> &tree_data,std::function<bool(std::string&,const department_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<department_info::col_names.size();jj++){
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
if(tree_data[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(tree_data[n].dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(tree_data[n].depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(tree_data[n].bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(tree_data[n].real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(tree_data[n].quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(tree_data[n].memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(tree_data[n].created_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(tree_data[n].created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(tree_data[n].updated_time);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(tree_data[n].updated_user);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(tree_data[n].isvirtual);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(tree_data[n].linkdpid)<<"\"";
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
   
    template<department_info::cols KeyCol, department_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>()));
        using ValType = decltype(department_info::getField<ValCol>(std::declval<const department_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(department_info::getField<KeyCol>(iter), department_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<department_info::cols KeyCol, department_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>())), 
            decltype(department_info::getField<ValCol>(std::declval<const department_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>())), 
                    decltype(department_info::getField<ValCol>(std::declval<const department_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>()));
        using ValType = decltype(department_info::getField<ValCol>(std::declval<const department_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(department_info::getField<KeyCol>(iter), department_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(department_info::getField<KeyCol>(iter), department_info::getField<ValCol>(iter))) {
                    result.emplace(department_info::getField<KeyCol>(iter), department_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<department_info::cols KeyCol, department_info::cols ValCol> 
    auto get_vec_cols()
    {
        using KeyType = decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>()));
        using ValType = decltype(department_info::getField<ValCol>(std::declval<const department_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(department_info::getField<KeyCol>(iter), department_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<department_info::cols KeyCol, department_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>())), 
            decltype(department_info::getField<ValCol>(std::declval<const department_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>())), 
                    decltype(department_info::getField<ValCol>(std::declval<const department_info::meta&>()))>, bool>
    auto get_vec_cols(Callback&& callback)
    {
        using KeyType = decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>()));
        using ValType = decltype(department_info::getField<ValCol>(std::declval<const department_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(department_info::getField<KeyCol>(iter), department_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(department_info::getField<KeyCol>(iter), department_info::getField<ValCol>(iter))) {
                    result.emplace_back(department_info::getField<KeyCol>(iter), department_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<department_info::cols KeyCol>
    auto get_vec_col()
    {
        using KeyType = decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(department_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_col<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<department_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>()))>, bool>
    auto get_vec_col(Callback&& callback)
    {
        using KeyType = decltype(department_info::getField<KeyCol>(std::declval<const department_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(department_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(department_info::getField<KeyCol>(iter))) {
                    result.emplace_back(department_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<department_info::cols Col>
        requires requires(std::ostream& os, decltype(department_info::getField<Col>(std::declval<const department_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << department_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<department_info::cols Col>
        requires requires(std::ostream& os, decltype(department_info::getField<Col>(std::declval<const department_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << department_info::getField<Col>(iter); 
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
   