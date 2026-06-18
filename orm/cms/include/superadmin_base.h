#ifndef ORM_CMS_SUPERADMINBASEMATA_H
#define ORM_CMS_SUPERADMINBASEMATA_H
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

namespace superadmin_info
{
 
    enum class cols : unsigned char 
    {
		adminid = 0,
		name = 1,
		password = 2,
		nickname = 3,
		isopen = 4,
		begindate = 5,
		enddate = 6,
		regdate = 7,
		mobile = 8,
		email = 9,
		loginnum = 10,
		qrtemp = 11,
		wxuuid = 12,
		basesitepath = 13,

    };

    struct meta
    {
		 unsigned  int  adminid = 0; ///**/
		 std::string  name = ""; ///**/
		 std::string  password = ""; ///**/
		 std::string  nickname = ""; ///**/
		 char  isopen = 0; ///**/
		 unsigned  int  begindate = 0; ///**/
		 unsigned  int  enddate = 0; ///**/
		 std::string  regdate = ""; ///**/
		 std::string  mobile = ""; ///**/
		 std::string  email = ""; ///**/
		 unsigned  int  loginnum = 0; ///**/
		 unsigned  int  qrtemp = 0; ///**/
		 std::string  wxuuid = ""; ///**/
		 std::string  basesitepath = ""; ///**/
	};
  
    struct meta_tree
    {
		 unsigned  int  adminid = 0; ///**/
		 std::string  name = ""; ///**/
		 std::string  password = ""; ///**/
		 std::string  nickname = ""; ///**/
		 char  isopen = 0; ///**/
		 unsigned  int  begindate = 0; ///**/
		 unsigned  int  enddate = 0; ///**/
		 std::string  regdate = ""; ///**/
		 std::string  mobile = ""; ///**/
		 std::string  email = ""; ///**/
		 unsigned  int  loginnum = 0; ///**/
		 unsigned  int  qrtemp = 0; ///**/
		 std::string  wxuuid = ""; ///**/
		 std::string  basesitepath = ""; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  adminid = 0; ///**/
		 std::string  name = ""; ///**/
		 std::string  password = ""; ///**/
		 std::string  nickname = ""; ///**/
		 char  isopen = 0; ///**/
		 unsigned  int  begindate = 0; ///**/
		 unsigned  int  enddate = 0; ///**/
		 std::string  regdate = ""; ///**/
		 std::string  mobile = ""; ///**/
		 std::string  email = ""; ///**/
		 unsigned  int  loginnum = 0; ///**/
		 unsigned  int  qrtemp = 0; ///**/
		 std::string  wxuuid = ""; ///**/
		 std::string  basesitepath = ""; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::adminid) { 
		 return m.adminid;
		} else if constexpr (Col == cols::name) { 
		 return m.name;
		} else if constexpr (Col == cols::password) { 
		 return m.password;
		} else if constexpr (Col == cols::nickname) { 
		 return m.nickname;
		} else if constexpr (Col == cols::isopen) { 
		 return m.isopen;
		} else if constexpr (Col == cols::begindate) { 
		 return m.begindate;
		} else if constexpr (Col == cols::enddate) { 
		 return m.enddate;
		} else if constexpr (Col == cols::regdate) { 
		 return m.regdate;
		} else if constexpr (Col == cols::mobile) { 
		 return m.mobile;
		} else if constexpr (Col == cols::email) { 
		 return m.email;
		} else if constexpr (Col == cols::loginnum) { 
		 return m.loginnum;
		} else if constexpr (Col == cols::qrtemp) { 
		 return m.qrtemp;
		} else if constexpr (Col == cols::wxuuid) { 
		 return m.wxuuid;
		} else if constexpr (Col == cols::basesitepath) { 
		 return m.basesitepath;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using adminid =  unsigned  int ;
		using name =  std::string ;
		using password =  std::string ;
		using nickname =  std::string ;
		using isopen =  char ;
		using begindate =  unsigned  int ;
		using enddate =  unsigned  int ;
		using regdate =  std::string ;
		using mobile =  std::string ;
		using email =  std::string ;
		using loginnum =  unsigned  int ;
		using qrtemp =  unsigned  int ;
		using wxuuid =  std::string ;
		using basesitepath =  std::string ;

    }

    
    #define ORM_CMS_SUPERADMIN_EXPAND(x) x 
    
    #define ORM_CMS_SUPERADMIN_META_FIELD_TYPE(col) \
         orm::cms::superadmin_info::type::col 
    
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBER(col) \
          ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_1(c1) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_1( c1)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c2))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c3))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c4))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c5))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c6))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c7))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c8))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c9))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c10))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c11))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c12))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c13))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c14))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c15))
         
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBER(c16))
         
    #define ORM_CMS_SUPERADMIN_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_SUPERADMIN_PROJ_MEMBERS(...) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_16, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_15, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_14, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_13, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_12, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_11, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_10, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_9, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_8, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_7, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_6, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_5, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_4, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_3, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_2, \
            ORM_CMS_SUPERADMIN_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_SUPERADMIN_COUNT(...) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_SUPERADMIN_TO_JSON_1(c1) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_2(c1,c2) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_SUPERADMIN_TO_JSON_BODY(...) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_SUPERADMIN_TO_JSON_16,ORM_CMS_SUPERADMIN_TO_JSON_15,ORM_CMS_SUPERADMIN_TO_JSON_14,ORM_CMS_SUPERADMIN_TO_JSON_13,ORM_CMS_SUPERADMIN_TO_JSON_12,ORM_CMS_SUPERADMIN_TO_JSON_11,ORM_CMS_SUPERADMIN_TO_JSON_10,ORM_CMS_SUPERADMIN_TO_JSON_9,ORM_CMS_SUPERADMIN_TO_JSON_8,ORM_CMS_SUPERADMIN_TO_JSON_7,ORM_CMS_SUPERADMIN_TO_JSON_6,ORM_CMS_SUPERADMIN_TO_JSON_5,ORM_CMS_SUPERADMIN_TO_JSON_4,ORM_CMS_SUPERADMIN_TO_JSON_3,ORM_CMS_SUPERADMIN_TO_JSON_2,ORM_CMS_SUPERADMIN_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_SUPERADMIN_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_1(n1)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_1(n1)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_SUPERADMIN_CAT(a, b) ORM_CMS_SUPERADMIN_CAT_(a, b)
    #define ORM_CMS_SUPERADMIN_CAT_(a, b) a##b

    #define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_SUPERADMIN_CAT(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM(...) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_SUPERADMIN_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_SUPERADMIN_SET_VAL_1(c1) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_SUPERADMIN_SET_VAL_2(c1,c2) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_1(c1)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_2(c1,c2)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_SUPERADMIN_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_SUPERADMIN_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_SUPERADMIN_CAT(ORM_CMS_SUPERADMIN_SET_VAL_, N)
    
    
    #define ORM_CMS_SUPERADMIN_SET_VAL_FIELDS(...) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_SUPERADMIN_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_FIELDS(ORM_CMS_SUPERADMIN_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_SUPERADMIN_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::superadmin_info { \
            struct StructName { \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SUPERADMIN_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::superadmin_info { \
            struct StructName { \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM(ORM_CMS_SUPERADMIN_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_SUPERADMIN_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::superadmin_info { \
            struct StructName { \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SUPERADMIN_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::superadmin_info { \
            struct StructName { \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SUPERADMIN_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::superadmin_info { \
            struct StructName { \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_TO_JSON_CUSTOM(ORM_CMS_SUPERADMIN_UNWRAP CustomNames));  \
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
                        ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_SUPERADMIN_EXPAND(ORM_CMS_SUPERADMIN_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,14> col_names={"adminid","name","password","nickname","isopen","begindate","enddate","regdate","mobile","email","loginnum","qrtemp","wxuuid","basesitepath"};
	static constexpr std::array<unsigned char,14> col_types={3,253,253,253,1,3,3,253,253,253,3,3,253,253};
	static constexpr std::array<unsigned char,14> col_length={0,40,40,30,0,0,0,30,40,40,0,0,40,0};
	static constexpr std::array<unsigned char,14> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct superadmin_base
{
      superadmin_info::meta data;
    std::vector<superadmin_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<superadmin_info::meta>::iterator begin(){     return record.begin(); }
std::vector<superadmin_info::meta>::iterator end(){     return record.end(); }
std::vector<superadmin_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<superadmin_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="superadmin";
static constexpr std::string_view org_tablename="superadmin";
static constexpr std::string_view modelname="Superadmin";

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
   	 return 0;
break;
case 'b':
 switch(coln.size()){  
case 9:
   	 return 5;
break;
case 12:
   	 return 13;
break;
 }
 break;
case 'e':
 switch(coln.size()){  
case 5:
   	 return 9;
break;
case 7:
   	 return 6;
break;
 }
 break;
case 'i':
   	 return 4;
break;
case 'l':
   	 return 10;
break;
case 'm':
   	 return 8;
break;
case 'n':
 switch(coln.size()){  
case 4:
   	 return 1;
break;
case 8:
   	 return 3;
break;
 }
 break;
case 'p':
   	 return 2;
break;
case 'q':
   	 return 11;
break;
case 'r':
   	 return 7;
break;
case 'w':
   	 return 12;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "adminid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     superadmin_info::meta metatemp;    
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
        for(;j<superadmin_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<superadmin_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.adminid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.adminid);
}
tempsql<<",'"<<stringaddslash(data.name)<<"'";
tempsql<<",'"<<stringaddslash(data.password)<<"'";
tempsql<<",'"<<stringaddslash(data.nickname)<<"'";
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
if(data.begindate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.begindate);
}
if(data.enddate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.enddate);
}
tempsql<<",'"<<stringaddslash(data.regdate)<<"'";
tempsql<<",'"<<stringaddslash(data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(data.email)<<"'";
if(data.loginnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.loginnum);
}
if(data.qrtemp==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.qrtemp);
}
tempsql<<",'"<<stringaddslash(data.wxuuid)<<"'";
tempsql<<",'"<<stringaddslash(data.basesitepath)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const superadmin_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<superadmin_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<superadmin_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.adminid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.adminid);
}
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.password)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.nickname)<<"'";
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
}
if(insert_data.begindate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.begindate);
}
if(insert_data.enddate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.enddate);
}
tempsql<<",'"<<stringaddslash(insert_data.regdate)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.email)<<"'";
if(insert_data.loginnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.loginnum);
}
if(insert_data.qrtemp==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.qrtemp);
}
tempsql<<",'"<<stringaddslash(insert_data.wxuuid)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.basesitepath)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<superadmin_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<superadmin_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<superadmin_info::col_names[j];
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


            	if(insert_data[i].adminid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].adminid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].password)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].nickname)<<"'";
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
	}
	if(insert_data[i].begindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].begindate);
	}
	if(insert_data[i].enddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].enddate);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].regdate)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].mobile)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].email)<<"'";
	if(insert_data[i].loginnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].loginnum);
	}
	if(insert_data[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].qrtemp);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].wxuuid)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].basesitepath)<<"'";
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

        if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
tempsql<<",`password`='"<<stringaddslash(data.password)<<"'";
tempsql<<",`nickname`='"<<stringaddslash(data.nickname)<<"'";
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
if(data.begindate==0){
	tempsql<<",`begindate`=0";
 }else{ 
	tempsql<<",`begindate`="<<std::to_string(data.begindate);
}
if(data.enddate==0){
	tempsql<<",`enddate`=0";
 }else{ 
	tempsql<<",`enddate`="<<std::to_string(data.enddate);
}
tempsql<<",`regdate`='"<<stringaddslash(data.regdate)<<"'";
tempsql<<",`mobile`='"<<stringaddslash(data.mobile)<<"'";
tempsql<<",`email`='"<<stringaddslash(data.email)<<"'";
if(data.loginnum==0){
	tempsql<<",`loginnum`=0";
 }else{ 
	tempsql<<",`loginnum`="<<std::to_string(data.loginnum);
}
if(data.qrtemp==0){
	tempsql<<",`qrtemp`=0";
 }else{ 
	tempsql<<",`qrtemp`="<<std::to_string(data.qrtemp);
}
tempsql<<",`wxuuid`='"<<stringaddslash(data.wxuuid)<<"'";
tempsql<<",`basesitepath`='"<<stringaddslash(data.basesitepath)<<"'";
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
if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`password`='"<<stringaddslash(data.password)<<"'";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`nickname`='"<<stringaddslash(data.nickname)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.begindate==0){
	tempsql<<"`begindate`=0";
 }else{ 
	tempsql<<"`begindate`="<<std::to_string(data.begindate);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.enddate==0){
	tempsql<<"`enddate`=0";
 }else{ 
	tempsql<<"`enddate`="<<std::to_string(data.enddate);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`regdate`='"<<stringaddslash(data.regdate)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`mobile`='"<<stringaddslash(data.mobile)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`email`='"<<stringaddslash(data.email)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.loginnum==0){
	tempsql<<"`loginnum`=0";
 }else{ 
	tempsql<<"`loginnum`="<<std::to_string(data.loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.qrtemp==0){
	tempsql<<"`qrtemp`=0";
 }else{ 
	tempsql<<"`qrtemp`="<<std::to_string(data.qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`wxuuid`='"<<stringaddslash(data.wxuuid)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`basesitepath`='"<<stringaddslash(data.basesitepath)<<"'";
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
        for (; j < superadmin_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << superadmin_info::col_names[j];
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
            	if(record[i].adminid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].adminid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].password)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].nickname)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].begindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].begindate);
	}
	if(record[i].enddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].enddate);
	}
	tempsql<<",'"<<stringaddslash(record[i].regdate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	if(record[i].loginnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].loginnum);
	}
	if(record[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].qrtemp);
	}
	tempsql<<",'"<<stringaddslash(record[i].wxuuid)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].basesitepath)<<"'";
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
        for (; j < superadmin_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << superadmin_info::col_names[j];
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
            	if(record[i].adminid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].adminid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].password)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].nickname)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].begindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].begindate);
	}
	if(record[i].enddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].enddate);
	}
	tempsql<<",'"<<stringaddslash(record[i].regdate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	if(record[i].loginnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].loginnum);
	}
	if(record[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].qrtemp);
	}
	tempsql<<",'"<<stringaddslash(record[i].wxuuid)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].basesitepath)<<"'";
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
            for(jj=0;jj<superadmin_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.adminid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.adminid));
}
 break;
 case 1:
	temparray.push_back(data.name);
 break;
 case 2:
	temparray.push_back(data.password);
 break;
 case 3:
	temparray.push_back(data.nickname);
 break;
 case 4:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 5:
if(data.begindate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.begindate));
}
 break;
 case 6:
if(data.enddate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.enddate));
}
 break;
 case 7:
	temparray.push_back(data.regdate);
 break;
 case 8:
	temparray.push_back(data.mobile);
 break;
 case 9:
	temparray.push_back(data.email);
 break;
 case 10:
if(data.loginnum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.loginnum));
}
 break;
 case 11:
if(data.qrtemp==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.qrtemp));
}
 break;
 case 12:
	temparray.push_back(data.wxuuid);
 break;
 case 13:
	temparray.push_back(data.basesitepath);
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
            for(jj=0;jj<superadmin_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.adminid==0){
	tempsql.insert({"adminid","0"});
 }else{ 
	tempsql.insert({"adminid",std::to_string(data.adminid)});
}
 break;
 case 1:
	tempsql.insert({"name",data.name});
 break;
 case 2:
	tempsql.insert({"password",data.password});
 break;
 case 3:
	tempsql.insert({"nickname",data.nickname});
 break;
 case 4:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 5:
if(data.begindate==0){
	tempsql.insert({"begindate","0"});
 }else{ 
	tempsql.insert({"begindate",std::to_string(data.begindate)});
}
 break;
 case 6:
if(data.enddate==0){
	tempsql.insert({"enddate","0"});
 }else{ 
	tempsql.insert({"enddate",std::to_string(data.enddate)});
}
 break;
 case 7:
	tempsql.insert({"regdate",data.regdate});
 break;
 case 8:
	tempsql.insert({"mobile",data.mobile});
 break;
 case 9:
	tempsql.insert({"email",data.email});
 break;
 case 10:
if(data.loginnum==0){
	tempsql.insert({"loginnum","0"});
 }else{ 
	tempsql.insert({"loginnum",std::to_string(data.loginnum)});
}
 break;
 case 11:
if(data.qrtemp==0){
	tempsql.insert({"qrtemp","0"});
 }else{ 
	tempsql.insert({"qrtemp",std::to_string(data.qrtemp)});
}
 break;
 case 12:
	tempsql.insert({"wxuuid",data.wxuuid});
 break;
 case 13:
	tempsql.insert({"basesitepath",data.basesitepath});
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
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
tempsql<<",\"password\":\""<<http::utf8_to_jsonstring(data.password);
tempsql<<"\"";
tempsql<<",\"nickname\":\""<<http::utf8_to_jsonstring(data.nickname);
tempsql<<"\"";
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
if(data.begindate==0){
	tempsql<<",\"begindate\":0";
 }else{ 
	tempsql<<",\"begindate\":"<<std::to_string(data.begindate);
}
if(data.enddate==0){
	tempsql<<",\"enddate\":0";
 }else{ 
	tempsql<<",\"enddate\":"<<std::to_string(data.enddate);
}
tempsql<<",\"regdate\":\""<<http::utf8_to_jsonstring(data.regdate);
tempsql<<"\"";
tempsql<<",\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile);
tempsql<<"\"";
tempsql<<",\"email\":\""<<http::utf8_to_jsonstring(data.email);
tempsql<<"\"";
if(data.loginnum==0){
	tempsql<<",\"loginnum\":0";
 }else{ 
	tempsql<<",\"loginnum\":"<<std::to_string(data.loginnum);
}
if(data.qrtemp==0){
	tempsql<<",\"qrtemp\":0";
 }else{ 
	tempsql<<",\"qrtemp\":"<<std::to_string(data.qrtemp);
}
tempsql<<",\"wxuuid\":\""<<http::utf8_to_jsonstring(data.wxuuid);
tempsql<<"\"";
tempsql<<",\"basesitepath\":\""<<http::utf8_to_jsonstring(data.basesitepath);
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
            for(jj=0;jj<superadmin_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"password\":\""<<http::utf8_to_jsonstring(data.password)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(data.nickname)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(data.begindate);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(data.enddate);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"regdate\":\""<<http::utf8_to_jsonstring(data.regdate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(data.email)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.loginnum==0){
	tempsql<<"\"loginnum\":0";
 }else{ 
	tempsql<<"\"loginnum\":"<<std::to_string(data.loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(data.qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(data.wxuuid)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"basesitepath\":\""<<http::utf8_to_jsonstring(data.basesitepath)<<"\"";
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
        superadmin_info::meta metatemp; 
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
			data.adminid=std::stoul(set_value_name);
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 1:
		 try{
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 2:
		 try{
			data.password.append(set_value_name);
		}catch (...) { 
			data.password.clear();
			 }
			break;
		case 3:
		 try{
			data.nickname.append(set_value_name);
		}catch (...) { 
			data.nickname.clear();
			 }
			break;
		case 4:
		 try{
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 5:
		 try{
			data.begindate=std::stoul(set_value_name);
		}catch (...) { 
			data.begindate=0;
			 }
			break;
		case 6:
		 try{
			data.enddate=std::stoul(set_value_name);
		}catch (...) { 
			data.enddate=0;
			 }
			break;
		case 7:
		 try{
			data.regdate.append(set_value_name);
		}catch (...) { 
			data.regdate.clear();
			 }
			break;
		case 8:
		 try{
			data.mobile.append(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 9:
		 try{
			data.email.append(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 10:
		 try{
			data.loginnum=std::stoul(set_value_name);
		}catch (...) { 
			data.loginnum=0;
			 }
			break;
		case 11:
		 try{
			data.qrtemp=std::stoul(set_value_name);
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 12:
		 try{
			data.wxuuid.append(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
			 }
			break;
		case 13:
		 try{
			data.basesitepath.append(set_value_name);
		}catch (...) { 
			data.basesitepath.clear();
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
			data.adminid=set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 1:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 2:
		 try{
			data.password=std::to_string(set_value_name);
		}catch (...) { 
			data.password.clear();
			 }
			break;
		case 3:
		 try{
			data.nickname=std::to_string(set_value_name);
		}catch (...) { 
			data.nickname.clear();
			 }
			break;
		case 4:
		 try{
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 5:
		 try{
			data.begindate=set_value_name;
		}catch (...) { 
			data.begindate=0;
			 }
			break;
		case 6:
		 try{
			data.enddate=set_value_name;
		}catch (...) { 
			data.enddate=0;
			 }
			break;
		case 7:
		 try{
			data.regdate=std::to_string(set_value_name);
		}catch (...) { 
			data.regdate.clear();
			 }
			break;
		case 8:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 9:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 10:
		 try{
			data.loginnum=set_value_name;
		}catch (...) { 
			data.loginnum=0;
			 }
			break;
		case 11:
		 try{
			data.qrtemp=set_value_name;
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 12:
		 try{
			data.wxuuid=std::to_string(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
			 }
			break;
		case 13:
		 try{
			data.basesitepath=std::to_string(set_value_name);
		}catch (...) { 
			data.basesitepath.clear();
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
			data.adminid=(unsigned int)set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 1:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 2:
		 try{
			data.password=std::to_string(set_value_name);
		}catch (...) { 
			data.password.clear();
			 }
			break;
		case 3:
		 try{
			data.nickname=std::to_string(set_value_name);
		}catch (...) { 
			data.nickname.clear();
			 }
			break;
		case 4:
		 try{
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 5:
		 try{
			data.begindate=(unsigned int)set_value_name;
		}catch (...) { 
			data.begindate=0;
			 }
			break;
		case 6:
		 try{
			data.enddate=(unsigned int)set_value_name;
		}catch (...) { 
			data.enddate=0;
			 }
			break;
		case 7:
		 try{
			data.regdate=std::to_string(set_value_name);
		}catch (...) { 
			data.regdate.clear();
			 }
			break;
		case 8:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 9:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 10:
		 try{
			data.loginnum=(unsigned int)set_value_name;
		}catch (...) { 
			data.loginnum=0;
			 }
			break;
		case 11:
		 try{
			data.qrtemp=(unsigned int)set_value_name;
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 12:
		 try{
			data.wxuuid=std::to_string(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
			 }
			break;
		case 13:
		 try{
			data.basesitepath=std::to_string(set_value_name);
		}catch (...) { 
			data.basesitepath.clear();
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
        for(jj=0;jj<superadmin_info::col_names.size();jj++){
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
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"password\":\""<<http::utf8_to_jsonstring(record[n].password)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(record[n].nickname)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(record[n].begindate);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(record[n].enddate);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"regdate\":\""<<http::utf8_to_jsonstring(record[n].regdate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].loginnum==0){
	tempsql<<"\"loginnum\":0";
 }else{ 
	tempsql<<"\"loginnum\":"<<std::to_string(record[n].loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(record[n].qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(record[n].wxuuid)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"basesitepath\":\""<<http::utf8_to_jsonstring(record[n].basesitepath)<<"\"";
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
   
   std::string to_json(std::function<bool(std::string&,superadmin_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<superadmin_info::col_names.size();jj++){
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
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"password\":\""<<http::utf8_to_jsonstring(record[n].password)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(record[n].nickname)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(record[n].begindate);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(record[n].enddate);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"regdate\":\""<<http::utf8_to_jsonstring(record[n].regdate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].loginnum==0){
	tempsql<<"\"loginnum\":0";
 }else{ 
	tempsql<<"\"loginnum\":"<<std::to_string(record[n].loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(record[n].qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(record[n].wxuuid)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"basesitepath\":\""<<http::utf8_to_jsonstring(record[n].basesitepath)<<"\"";
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
   long long getPK(){  return data.adminid; } 
 void setPK(long long val){  data.adminid=val;} 
 unsigned  int  getAdminid(){  return data.adminid; } 
 void setAdminid( unsigned  int  val){  data.adminid=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 std::string  getPassword(){  return data.password; } 
 std::string & getRefPassword(){  return std::ref(data.password); } 
 void setPassword( std::string  &val){  data.password=val;} 
 void setPassword(std::string_view val){  data.password=val;} 

 std::string  getNickname(){  return data.nickname; } 
 std::string & getRefNickname(){  return std::ref(data.nickname); } 
 void setNickname( std::string  &val){  data.nickname=val;} 
 void setNickname(std::string_view val){  data.nickname=val;} 

 char  getIsopen(){  return data.isopen; } 
 void setIsopen( char  val){  data.isopen=val;} 

 unsigned  int  getBegindate(){  return data.begindate; } 
 void setBegindate( unsigned  int  val){  data.begindate=val;} 

 unsigned  int  getEnddate(){  return data.enddate; } 
 void setEnddate( unsigned  int  val){  data.enddate=val;} 

 std::string  getRegdate(){  return data.regdate; } 
 std::string & getRefRegdate(){  return std::ref(data.regdate); } 
 void setRegdate( std::string  &val){  data.regdate=val;} 
 void setRegdate(std::string_view val){  data.regdate=val;} 

 std::string  getMobile(){  return data.mobile; } 
 std::string & getRefMobile(){  return std::ref(data.mobile); } 
 void setMobile( std::string  &val){  data.mobile=val;} 
 void setMobile(std::string_view val){  data.mobile=val;} 

 std::string  getEmail(){  return data.email; } 
 std::string & getRefEmail(){  return std::ref(data.email); } 
 void setEmail( std::string  &val){  data.email=val;} 
 void setEmail(std::string_view val){  data.email=val;} 

 unsigned  int  getLoginnum(){  return data.loginnum; } 
 void setLoginnum( unsigned  int  val){  data.loginnum=val;} 

 unsigned  int  getQrtemp(){  return data.qrtemp; } 
 void setQrtemp( unsigned  int  val){  data.qrtemp=val;} 

 std::string  getWxuuid(){  return data.wxuuid; } 
 std::string & getRefWxuuid(){  return std::ref(data.wxuuid); } 
 void setWxuuid( std::string  &val){  data.wxuuid=val;} 
 void setWxuuid(std::string_view val){  data.wxuuid=val;} 

 std::string  getBasesitepath(){  return data.basesitepath; } 
 std::string & getRefBasesitepath(){  return std::ref(data.basesitepath); } 
 void setBasesitepath( std::string  &val){  data.basesitepath=val;} 
 void setBasesitepath(std::string_view val){  data.basesitepath=val;} 

superadmin_info::meta getnewData(){
 	 struct superadmin_info::meta newdata;
	 return newdata; 
} 
superadmin_info::meta getData(){
 	 return data; 
} 
std::vector<superadmin_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<superadmin_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<superadmin_info::col_names.size();jj++){
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
if(tree_data[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(tree_data[n].adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"password\":\""<<http::utf8_to_jsonstring(tree_data[n].password)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(tree_data[n].nickname)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(tree_data[n].begindate);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(tree_data[n].enddate);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"regdate\":\""<<http::utf8_to_jsonstring(tree_data[n].regdate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(tree_data[n].mobile)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(tree_data[n].email)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].loginnum==0){
	tempsql<<"\"loginnum\":0";
 }else{ 
	tempsql<<"\"loginnum\":"<<std::to_string(tree_data[n].loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(tree_data[n].qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(tree_data[n].wxuuid)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"basesitepath\":\""<<http::utf8_to_jsonstring(tree_data[n].basesitepath)<<"\"";
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
   
   std::string tree_tojson(const std::vector<superadmin_info::meta_tree> &tree_data,std::function<bool(std::string&,const superadmin_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<superadmin_info::col_names.size();jj++){
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
if(tree_data[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(tree_data[n].adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"password\":\""<<http::utf8_to_jsonstring(tree_data[n].password)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(tree_data[n].nickname)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(tree_data[n].begindate);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(tree_data[n].enddate);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"regdate\":\""<<http::utf8_to_jsonstring(tree_data[n].regdate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(tree_data[n].mobile)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(tree_data[n].email)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].loginnum==0){
	tempsql<<"\"loginnum\":0";
 }else{ 
	tempsql<<"\"loginnum\":"<<std::to_string(tree_data[n].loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(tree_data[n].qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(tree_data[n].wxuuid)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"basesitepath\":\""<<http::utf8_to_jsonstring(tree_data[n].basesitepath)<<"\"";
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
   
    template<superadmin_info::cols KeyCol, superadmin_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>()));
        using ValType = decltype(superadmin_info::getField<ValCol>(std::declval<const superadmin_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(superadmin_info::getField<KeyCol>(iter), superadmin_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<superadmin_info::cols KeyCol, superadmin_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>())), 
            decltype(superadmin_info::getField<ValCol>(std::declval<const superadmin_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>())), 
                    decltype(superadmin_info::getField<ValCol>(std::declval<const superadmin_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>()));
        using ValType = decltype(superadmin_info::getField<ValCol>(std::declval<const superadmin_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(superadmin_info::getField<KeyCol>(iter), superadmin_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(superadmin_info::getField<KeyCol>(iter), superadmin_info::getField<ValCol>(iter))) {
                    result.emplace(superadmin_info::getField<KeyCol>(iter), superadmin_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<superadmin_info::cols KeyCol, superadmin_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>()));
        using ValType = decltype(superadmin_info::getField<ValCol>(std::declval<const superadmin_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(superadmin_info::getField<KeyCol>(iter), superadmin_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<superadmin_info::cols KeyCol, superadmin_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>())), 
            decltype(superadmin_info::getField<ValCol>(std::declval<const superadmin_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>())), 
                    decltype(superadmin_info::getField<ValCol>(std::declval<const superadmin_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>()));
        using ValType = decltype(superadmin_info::getField<ValCol>(std::declval<const superadmin_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(superadmin_info::getField<KeyCol>(iter), superadmin_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(superadmin_info::getField<KeyCol>(iter), superadmin_info::getField<ValCol>(iter))) {
                    result.emplace_back(superadmin_info::getField<KeyCol>(iter), superadmin_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<superadmin_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(superadmin_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<superadmin_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(superadmin_info::getField<KeyCol>(std::declval<const superadmin_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(superadmin_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(superadmin_info::getField<KeyCol>(iter))) {
                    result.emplace_back(superadmin_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<superadmin_info::cols Col>
        requires requires(std::ostream& os, decltype(superadmin_info::getField<Col>(std::declval<const superadmin_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << superadmin_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<superadmin_info::cols Col>
        requires requires(std::ostream& os, decltype(superadmin_info::getField<Col>(std::declval<const superadmin_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << superadmin_info::getField<Col>(iter); 
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
   