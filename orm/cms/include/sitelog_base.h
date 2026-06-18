#ifndef ORM_CMS_SITELOGBASEMATA_H
#define ORM_CMS_SITELOGBASEMATA_H
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

namespace sitelog_info
{
 
    enum class cols : unsigned char 
    {
		logid = 0,
		userid = 1,
		memberid = 2,
		ipport = 3,
		httpv = 4,
		ipaddress = 5,
		visittime = 6,
		useragent = 7,
		referer = 8,
		cururl = 9,
		address = 10,
		hostname = 11,
		derefererurl = 12,
		deurl = 13,

    };

    struct meta
    {
		 unsigned  int  logid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  memberid = 0; ///**/
		 unsigned  int  ipport = 0; ///**/
		 unsigned  char  httpv = 0; ///*https http2*/
		 std::string  ipaddress = ""; ///**/
		 std::string  visittime = ""; ///**/
		 std::string  useragent = ""; ///**/
		 std::string  referer = ""; ///**/
		 std::string  cururl = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  hostname = ""; ///**/
		 std::string  derefererurl = ""; ///**/
		 std::string  deurl = ""; ///**/
	};
  
    struct meta_tree
    {
		 unsigned  int  logid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  memberid = 0; ///**/
		 unsigned  int  ipport = 0; ///**/
		 unsigned  char  httpv = 0; ///*https http2*/
		 std::string  ipaddress = ""; ///**/
		 std::string  visittime = ""; ///**/
		 std::string  useragent = ""; ///**/
		 std::string  referer = ""; ///**/
		 std::string  cururl = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  hostname = ""; ///**/
		 std::string  derefererurl = ""; ///**/
		 std::string  deurl = ""; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  logid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  memberid = 0; ///**/
		 unsigned  int  ipport = 0; ///**/
		 unsigned  char  httpv = 0; ///*https http2*/
		 std::string  ipaddress = ""; ///**/
		 std::string  visittime = ""; ///**/
		 std::string  useragent = ""; ///**/
		 std::string  referer = ""; ///**/
		 std::string  cururl = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  hostname = ""; ///**/
		 std::string  derefererurl = ""; ///**/
		 std::string  deurl = ""; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::logid) { 
		 return m.logid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::memberid) { 
		 return m.memberid;
		} else if constexpr (Col == cols::ipport) { 
		 return m.ipport;
		} else if constexpr (Col == cols::httpv) { 
		 return m.httpv;
		} else if constexpr (Col == cols::ipaddress) { 
		 return m.ipaddress;
		} else if constexpr (Col == cols::visittime) { 
		 return m.visittime;
		} else if constexpr (Col == cols::useragent) { 
		 return m.useragent;
		} else if constexpr (Col == cols::referer) { 
		 return m.referer;
		} else if constexpr (Col == cols::cururl) { 
		 return m.cururl;
		} else if constexpr (Col == cols::address) { 
		 return m.address;
		} else if constexpr (Col == cols::hostname) { 
		 return m.hostname;
		} else if constexpr (Col == cols::derefererurl) { 
		 return m.derefererurl;
		} else if constexpr (Col == cols::deurl) { 
		 return m.deurl;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using logid =  unsigned  int ;
		using userid =  unsigned  int ;
		using memberid =  unsigned  int ;
		using ipport =  unsigned  int ;
		using httpv =  unsigned  char ;
		using ipaddress =  std::string ;
		using visittime =  std::string ;
		using useragent =  std::string ;
		using referer =  std::string ;
		using cururl =  std::string ;
		using address =  std::string ;
		using hostname =  std::string ;
		using derefererurl =  std::string ;
		using deurl =  std::string ;

    }

    
    #define ORM_CMS_SITELOG_EXPAND(x) x 
    
    #define ORM_CMS_SITELOG_META_FIELD_TYPE(col) \
         orm::cms::sitelog_info::type::col 
    
    #define ORM_CMS_SITELOG_PROJ_MEMBER(col) \
          ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_1(c1) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_1( c1)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c2))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c3))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c4))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c5))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c6))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c7))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c8))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c9))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c10))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c11))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c12))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c13))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c14))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c15))
         
    #define ORM_CMS_SITELOG_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBER(c16))
         
    #define ORM_CMS_SITELOG_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_SITELOG_PROJ_MEMBERS(...) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_16, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_15, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_14, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_13, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_12, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_11, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_10, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_9, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_8, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_7, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_6, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_5, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_4, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_3, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_2, \
            ORM_CMS_SITELOG_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_SITELOG_COUNT(...) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_SITELOG_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_SITELOG_TO_JSON_1(c1) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_SITELOG_TO_JSON_2(c1,c2) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_SITELOG_TO_JSON_BODY(...) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_SITELOG_TO_JSON_16,ORM_CMS_SITELOG_TO_JSON_15,ORM_CMS_SITELOG_TO_JSON_14,ORM_CMS_SITELOG_TO_JSON_13,ORM_CMS_SITELOG_TO_JSON_12,ORM_CMS_SITELOG_TO_JSON_11,ORM_CMS_SITELOG_TO_JSON_10,ORM_CMS_SITELOG_TO_JSON_9,ORM_CMS_SITELOG_TO_JSON_8,ORM_CMS_SITELOG_TO_JSON_7,ORM_CMS_SITELOG_TO_JSON_6,ORM_CMS_SITELOG_TO_JSON_5,ORM_CMS_SITELOG_TO_JSON_4,ORM_CMS_SITELOG_TO_JSON_3,ORM_CMS_SITELOG_TO_JSON_2,ORM_CMS_SITELOG_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_SITELOG_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_SITELOG_TO_JSON_CUSTOM_1(n1)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_1(n1)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_SITELOG_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_SITELOG_CAT(a, b) ORM_CMS_SITELOG_CAT_(a, b)
    #define ORM_CMS_SITELOG_CAT_(a, b) a##b

    #define ORM_CMS_SITELOG_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_SITELOG_CAT(ORM_CMS_SITELOG_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_SITELOG_TO_JSON_CUSTOM(...) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_SITELOG_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_SITELOG_SET_VAL_1(c1) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_SITELOG_SET_VAL_2(c1,c2) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_1(c1)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_2(c1,c2)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_SITELOG_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_SITELOG_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_SITELOG_CAT(ORM_CMS_SITELOG_SET_VAL_, N)
    
    
    #define ORM_CMS_SITELOG_SET_VAL_FIELDS(...) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_SITELOG_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_FIELDS(ORM_CMS_SITELOG_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_SITELOG_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::sitelog_info { \
            struct StructName { \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SITELOG_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::sitelog_info { \
            struct StructName { \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM(ORM_CMS_SITELOG_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_SITELOG_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::sitelog_info { \
            struct StructName { \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SITELOG_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::sitelog_info { \
            struct StructName { \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SITELOG_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::sitelog_info { \
            struct StructName { \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_TO_JSON_CUSTOM(ORM_CMS_SITELOG_UNWRAP CustomNames));  \
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
                        ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_SITELOG_EXPAND(ORM_CMS_SITELOG_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,14> col_names={"logid","userid","memberid","ipport","httpv","ipaddress","visittime","useragent","referer","cururl","address","hostname","derefererurl","deurl"};
	static constexpr std::array<unsigned char,14> col_types={3,3,3,3,1,253,253,253,253,253,253,253,253,253};
	static constexpr std::array<unsigned char,14> col_length={0,0,0,0,0,60,30,0,0,0,120,60,0,0};
	static constexpr std::array<unsigned char,14> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct sitelog_base
{
      sitelog_info::meta data;
    std::vector<sitelog_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<sitelog_info::meta>::iterator begin(){     return record.begin(); }
std::vector<sitelog_info::meta>::iterator end(){     return record.end(); }
std::vector<sitelog_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<sitelog_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="sitelog";
static constexpr std::string_view org_tablename="sitelog";
static constexpr std::string_view modelname="Sitelog";

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
   	 return 10;
break;
case 'c':
   	 return 9;
break;
case 'd':
 switch(coln.size()){  
case 5:
   	 return 13;
break;
case 12:
   	 return 12;
break;
 }
 break;
case 'h':
 switch(coln.size()){  
case 5:
   	 return 4;
break;
case 8:
   	 return 11;
break;
 }
 break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 3;
break;
case 9:
   	 return 5;
break;
 }
 break;
case 'l':
   	 return 0;
break;
case 'm':
   	 return 2;
break;
case 'r':
   	 return 8;
break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 1;
break;
case 9:
   	 return 7;
break;
 }
 break;
case 'v':
   	 return 6;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "logid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     sitelog_info::meta metatemp;    
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
        for(;j<sitelog_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<sitelog_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.logid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.logid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.memberid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.memberid);
}
if(data.ipport==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.ipport);
}
if(data.httpv==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.httpv);
}
tempsql<<",'"<<stringaddslash(data.ipaddress)<<"'";
tempsql<<",'"<<stringaddslash(data.visittime)<<"'";
tempsql<<",'"<<stringaddslash(data.useragent)<<"'";
tempsql<<",'"<<stringaddslash(data.referer)<<"'";
tempsql<<",'"<<stringaddslash(data.cururl)<<"'";
tempsql<<",'"<<stringaddslash(data.address)<<"'";
tempsql<<",'"<<stringaddslash(data.hostname)<<"'";
tempsql<<",'"<<stringaddslash(data.derefererurl)<<"'";
tempsql<<",'"<<stringaddslash(data.deurl)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const sitelog_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<sitelog_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<sitelog_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.logid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.logid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.memberid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.memberid);
}
if(insert_data.ipport==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.ipport);
}
if(insert_data.httpv==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.httpv);
}
tempsql<<",'"<<stringaddslash(insert_data.ipaddress)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.visittime)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.useragent)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.referer)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.cururl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.address)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.hostname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.derefererurl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.deurl)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<sitelog_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<sitelog_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<sitelog_info::col_names[j];
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


            	if(insert_data[i].logid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].logid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].memberid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].memberid);
	}
	if(insert_data[i].ipport==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].ipport);
	}
	if(insert_data[i].httpv==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].httpv);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].ipaddress)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].visittime)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].useragent)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].referer)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].cururl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].address)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].hostname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].derefererurl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].deurl)<<"'";
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

        if(data.logid==0){
	tempsql<<"`logid`=0";
 }else{ 
	tempsql<<"`logid`="<<std::to_string(data.logid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.memberid==0){
	tempsql<<",`memberid`=0";
 }else{ 
	tempsql<<",`memberid`="<<std::to_string(data.memberid);
}
if(data.ipport==0){
	tempsql<<",`ipport`=0";
 }else{ 
	tempsql<<",`ipport`="<<std::to_string(data.ipport);
}
if(data.httpv==0){
	tempsql<<",`httpv`=0";
 }else{ 
	tempsql<<",`httpv`="<<std::to_string(data.httpv);
}
tempsql<<",`ipaddress`='"<<stringaddslash(data.ipaddress)<<"'";
tempsql<<",`visittime`='"<<stringaddslash(data.visittime)<<"'";
tempsql<<",`useragent`='"<<stringaddslash(data.useragent)<<"'";
tempsql<<",`referer`='"<<stringaddslash(data.referer)<<"'";
tempsql<<",`cururl`='"<<stringaddslash(data.cururl)<<"'";
tempsql<<",`address`='"<<stringaddslash(data.address)<<"'";
tempsql<<",`hostname`='"<<stringaddslash(data.hostname)<<"'";
tempsql<<",`derefererurl`='"<<stringaddslash(data.derefererurl)<<"'";
tempsql<<",`deurl`='"<<stringaddslash(data.deurl)<<"'";
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
if(data.logid==0){
	tempsql<<"`logid`=0";
 }else{ 
	tempsql<<"`logid`="<<std::to_string(data.logid);
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
if(data.memberid==0){
	tempsql<<"`memberid`=0";
 }else{ 
	tempsql<<"`memberid`="<<std::to_string(data.memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.ipport==0){
	tempsql<<"`ipport`=0";
 }else{ 
	tempsql<<"`ipport`="<<std::to_string(data.ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.httpv==0){
	tempsql<<"`httpv`=0";
 }else{ 
	tempsql<<"`httpv`="<<std::to_string(data.httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`ipaddress`='"<<stringaddslash(data.ipaddress)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`visittime`='"<<stringaddslash(data.visittime)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`useragent`='"<<stringaddslash(data.useragent)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`referer`='"<<stringaddslash(data.referer)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`cururl`='"<<stringaddslash(data.cururl)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`address`='"<<stringaddslash(data.address)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`hostname`='"<<stringaddslash(data.hostname)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`derefererurl`='"<<stringaddslash(data.derefererurl)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`deurl`='"<<stringaddslash(data.deurl)<<"'";
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
        for (; j < sitelog_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << sitelog_info::col_names[j];
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
            	if(record[i].logid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].logid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].memberid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].memberid);
	}
	if(record[i].ipport==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ipport);
	}
	if(record[i].httpv==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].httpv);
	}
	tempsql<<",'"<<stringaddslash(record[i].ipaddress)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].visittime)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].useragent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].referer)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].cururl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].hostname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].derefererurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].deurl)<<"'";
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
        for (; j < sitelog_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << sitelog_info::col_names[j];
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
            	if(record[i].logid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].logid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].memberid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].memberid);
	}
	if(record[i].ipport==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ipport);
	}
	if(record[i].httpv==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].httpv);
	}
	tempsql<<",'"<<stringaddslash(record[i].ipaddress)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].visittime)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].useragent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].referer)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].cururl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].hostname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].derefererurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].deurl)<<"'";
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
            for(jj=0;jj<sitelog_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.logid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.logid));
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
if(data.memberid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.memberid));
}
 break;
 case 3:
if(data.ipport==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.ipport));
}
 break;
 case 4:
if(data.httpv==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.httpv));
}
 break;
 case 5:
	temparray.push_back(data.ipaddress);
 break;
 case 6:
	temparray.push_back(data.visittime);
 break;
 case 7:
	temparray.push_back(data.useragent);
 break;
 case 8:
	temparray.push_back(data.referer);
 break;
 case 9:
	temparray.push_back(data.cururl);
 break;
 case 10:
	temparray.push_back(data.address);
 break;
 case 11:
	temparray.push_back(data.hostname);
 break;
 case 12:
	temparray.push_back(data.derefererurl);
 break;
 case 13:
	temparray.push_back(data.deurl);
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
            for(jj=0;jj<sitelog_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.logid==0){
	tempsql.insert({"logid","0"});
 }else{ 
	tempsql.insert({"logid",std::to_string(data.logid)});
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
if(data.memberid==0){
	tempsql.insert({"memberid","0"});
 }else{ 
	tempsql.insert({"memberid",std::to_string(data.memberid)});
}
 break;
 case 3:
if(data.ipport==0){
	tempsql.insert({"ipport","0"});
 }else{ 
	tempsql.insert({"ipport",std::to_string(data.ipport)});
}
 break;
 case 4:
if(data.httpv==0){
	tempsql.insert({"httpv","0"});
 }else{ 
	tempsql.insert({"httpv",std::to_string(data.httpv)});
}
 break;
 case 5:
	tempsql.insert({"ipaddress",data.ipaddress});
 break;
 case 6:
	tempsql.insert({"visittime",data.visittime});
 break;
 case 7:
	tempsql.insert({"useragent",data.useragent});
 break;
 case 8:
	tempsql.insert({"referer",data.referer});
 break;
 case 9:
	tempsql.insert({"cururl",data.cururl});
 break;
 case 10:
	tempsql.insert({"address",data.address});
 break;
 case 11:
	tempsql.insert({"hostname",data.hostname});
 break;
 case 12:
	tempsql.insert({"derefererurl",data.derefererurl});
 break;
 case 13:
	tempsql.insert({"deurl",data.deurl});
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
if(data.logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(data.logid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.memberid==0){
	tempsql<<",\"memberid\":0";
 }else{ 
	tempsql<<",\"memberid\":"<<std::to_string(data.memberid);
}
if(data.ipport==0){
	tempsql<<",\"ipport\":0";
 }else{ 
	tempsql<<",\"ipport\":"<<std::to_string(data.ipport);
}
if(data.httpv==0){
	tempsql<<",\"httpv\":0";
 }else{ 
	tempsql<<",\"httpv\":"<<std::to_string(data.httpv);
}
tempsql<<",\"ipaddress\":\""<<http::utf8_to_jsonstring(data.ipaddress);
tempsql<<"\"";
tempsql<<",\"visittime\":\""<<http::utf8_to_jsonstring(data.visittime);
tempsql<<"\"";
tempsql<<",\"useragent\":\""<<http::utf8_to_jsonstring(data.useragent);
tempsql<<"\"";
tempsql<<",\"referer\":\""<<http::utf8_to_jsonstring(data.referer);
tempsql<<"\"";
tempsql<<",\"cururl\":\""<<http::utf8_to_jsonstring(data.cururl);
tempsql<<"\"";
tempsql<<",\"address\":\""<<http::utf8_to_jsonstring(data.address);
tempsql<<"\"";
tempsql<<",\"hostname\":\""<<http::utf8_to_jsonstring(data.hostname);
tempsql<<"\"";
tempsql<<",\"derefererurl\":\""<<http::utf8_to_jsonstring(data.derefererurl);
tempsql<<"\"";
tempsql<<",\"deurl\":\""<<http::utf8_to_jsonstring(data.deurl);
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
            for(jj=0;jj<sitelog_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(data.logid);
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
if(data.memberid==0){
	tempsql<<"\"memberid\":0";
 }else{ 
	tempsql<<"\"memberid\":"<<std::to_string(data.memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.ipport==0){
	tempsql<<"\"ipport\":0";
 }else{ 
	tempsql<<"\"ipport\":"<<std::to_string(data.ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.httpv==0){
	tempsql<<"\"httpv\":0";
 }else{ 
	tempsql<<"\"httpv\":"<<std::to_string(data.httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"ipaddress\":\""<<http::utf8_to_jsonstring(data.ipaddress)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"visittime\":\""<<http::utf8_to_jsonstring(data.visittime)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"useragent\":\""<<http::utf8_to_jsonstring(data.useragent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"referer\":\""<<http::utf8_to_jsonstring(data.referer)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"cururl\":\""<<http::utf8_to_jsonstring(data.cururl)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(data.address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"hostname\":\""<<http::utf8_to_jsonstring(data.hostname)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"derefererurl\":\""<<http::utf8_to_jsonstring(data.derefererurl)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"deurl\":\""<<http::utf8_to_jsonstring(data.deurl)<<"\"";
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
        sitelog_info::meta metatemp; 
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
			data.logid=std::stoul(set_value_name);
		}catch (...) { 
			data.logid=0;
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
			data.memberid=std::stoul(set_value_name);
		}catch (...) { 
			data.memberid=0;
			 }
			break;
		case 3:
		 try{
			data.ipport=std::stoul(set_value_name);
		}catch (...) { 
			data.ipport=0;
			 }
			break;
		case 4:
		 try{
			data.httpv=std::stoi(set_value_name);
		}catch (...) { 
			data.httpv=0;
			 }
			break;
		case 5:
		 try{
			data.ipaddress.append(set_value_name);
		}catch (...) { 
			data.ipaddress.clear();
			 }
			break;
		case 6:
		 try{
			data.visittime.append(set_value_name);
		}catch (...) { 
			data.visittime.clear();
			 }
			break;
		case 7:
		 try{
			data.useragent.append(set_value_name);
		}catch (...) { 
			data.useragent.clear();
			 }
			break;
		case 8:
		 try{
			data.referer.append(set_value_name);
		}catch (...) { 
			data.referer.clear();
			 }
			break;
		case 9:
		 try{
			data.cururl.append(set_value_name);
		}catch (...) { 
			data.cururl.clear();
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
			data.hostname.append(set_value_name);
		}catch (...) { 
			data.hostname.clear();
			 }
			break;
		case 12:
		 try{
			data.derefererurl.append(set_value_name);
		}catch (...) { 
			data.derefererurl.clear();
			 }
			break;
		case 13:
		 try{
			data.deurl.append(set_value_name);
		}catch (...) { 
			data.deurl.clear();
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
			data.logid=set_value_name;
		}catch (...) { 
			data.logid=0;
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
			data.memberid=set_value_name;
		}catch (...) { 
			data.memberid=0;
			 }
			break;
		case 3:
		 try{
			data.ipport=set_value_name;
		}catch (...) { 
			data.ipport=0;
			 }
			break;
		case 4:
		 try{
			data.httpv=set_value_name;
		}catch (...) { 
			data.httpv=0;
			 }
			break;
		case 5:
		 try{
			data.ipaddress=std::to_string(set_value_name);
		}catch (...) { 
			data.ipaddress.clear();
			 }
			break;
		case 6:
		 try{
			data.visittime=std::to_string(set_value_name);
		}catch (...) { 
			data.visittime.clear();
			 }
			break;
		case 7:
		 try{
			data.useragent=std::to_string(set_value_name);
		}catch (...) { 
			data.useragent.clear();
			 }
			break;
		case 8:
		 try{
			data.referer=std::to_string(set_value_name);
		}catch (...) { 
			data.referer.clear();
			 }
			break;
		case 9:
		 try{
			data.cururl=std::to_string(set_value_name);
		}catch (...) { 
			data.cururl.clear();
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
			data.hostname=std::to_string(set_value_name);
		}catch (...) { 
			data.hostname.clear();
			 }
			break;
		case 12:
		 try{
			data.derefererurl=std::to_string(set_value_name);
		}catch (...) { 
			data.derefererurl.clear();
			 }
			break;
		case 13:
		 try{
			data.deurl=std::to_string(set_value_name);
		}catch (...) { 
			data.deurl.clear();
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
			data.logid=(unsigned int)set_value_name;
		}catch (...) { 
			data.logid=0;
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
			data.memberid=(unsigned int)set_value_name;
		}catch (...) { 
			data.memberid=0;
			 }
			break;
		case 3:
		 try{
			data.ipport=(unsigned int)set_value_name;
		}catch (...) { 
			data.ipport=0;
			 }
			break;
		case 4:
		 try{
			data.httpv=(int)set_value_name;
		}catch (...) { 
			data.httpv=0;
			 }
			break;
		case 5:
		 try{
			data.ipaddress=std::to_string(set_value_name);
		}catch (...) { 
			data.ipaddress.clear();
			 }
			break;
		case 6:
		 try{
			data.visittime=std::to_string(set_value_name);
		}catch (...) { 
			data.visittime.clear();
			 }
			break;
		case 7:
		 try{
			data.useragent=std::to_string(set_value_name);
		}catch (...) { 
			data.useragent.clear();
			 }
			break;
		case 8:
		 try{
			data.referer=std::to_string(set_value_name);
		}catch (...) { 
			data.referer.clear();
			 }
			break;
		case 9:
		 try{
			data.cururl=std::to_string(set_value_name);
		}catch (...) { 
			data.cururl.clear();
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
			data.hostname=std::to_string(set_value_name);
		}catch (...) { 
			data.hostname.clear();
			 }
			break;
		case 12:
		 try{
			data.derefererurl=std::to_string(set_value_name);
		}catch (...) { 
			data.derefererurl.clear();
			 }
			break;
		case 13:
		 try{
			data.deurl=std::to_string(set_value_name);
		}catch (...) { 
			data.deurl.clear();
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
        for(jj=0;jj<sitelog_info::col_names.size();jj++){
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
if(record[n].logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(record[n].logid);
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
if(record[n].memberid==0){
	tempsql<<"\"memberid\":0";
 }else{ 
	tempsql<<"\"memberid\":"<<std::to_string(record[n].memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].ipport==0){
	tempsql<<"\"ipport\":0";
 }else{ 
	tempsql<<"\"ipport\":"<<std::to_string(record[n].ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].httpv==0){
	tempsql<<"\"httpv\":0";
 }else{ 
	tempsql<<"\"httpv\":"<<std::to_string(record[n].httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"ipaddress\":\""<<http::utf8_to_jsonstring(record[n].ipaddress)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"visittime\":\""<<http::utf8_to_jsonstring(record[n].visittime)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"useragent\":\""<<http::utf8_to_jsonstring(record[n].useragent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"referer\":\""<<http::utf8_to_jsonstring(record[n].referer)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"cururl\":\""<<http::utf8_to_jsonstring(record[n].cururl)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"hostname\":\""<<http::utf8_to_jsonstring(record[n].hostname)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"derefererurl\":\""<<http::utf8_to_jsonstring(record[n].derefererurl)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"deurl\":\""<<http::utf8_to_jsonstring(record[n].deurl)<<"\"";
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
   
   std::string to_json(std::function<bool(std::string&,sitelog_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<sitelog_info::col_names.size();jj++){
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
if(record[n].logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(record[n].logid);
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
if(record[n].memberid==0){
	tempsql<<"\"memberid\":0";
 }else{ 
	tempsql<<"\"memberid\":"<<std::to_string(record[n].memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].ipport==0){
	tempsql<<"\"ipport\":0";
 }else{ 
	tempsql<<"\"ipport\":"<<std::to_string(record[n].ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].httpv==0){
	tempsql<<"\"httpv\":0";
 }else{ 
	tempsql<<"\"httpv\":"<<std::to_string(record[n].httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"ipaddress\":\""<<http::utf8_to_jsonstring(record[n].ipaddress)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"visittime\":\""<<http::utf8_to_jsonstring(record[n].visittime)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"useragent\":\""<<http::utf8_to_jsonstring(record[n].useragent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"referer\":\""<<http::utf8_to_jsonstring(record[n].referer)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"cururl\":\""<<http::utf8_to_jsonstring(record[n].cururl)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"hostname\":\""<<http::utf8_to_jsonstring(record[n].hostname)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"derefererurl\":\""<<http::utf8_to_jsonstring(record[n].derefererurl)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"deurl\":\""<<http::utf8_to_jsonstring(record[n].deurl)<<"\"";
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
   long long getPK(){  return data.logid; } 
 void setPK(long long val){  data.logid=val;} 
 unsigned  int  getLogid(){  return data.logid; } 
 void setLogid( unsigned  int  val){  data.logid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getMemberid(){  return data.memberid; } 
 void setMemberid( unsigned  int  val){  data.memberid=val;} 

 unsigned  int  getIpport(){  return data.ipport; } 
 void setIpport( unsigned  int  val){  data.ipport=val;} 

 unsigned  char  getHttpv(){  return data.httpv; } 
 void setHttpv( unsigned  char  val){  data.httpv=val;} 

 std::string  getIpaddress(){  return data.ipaddress; } 
 std::string & getRefIpaddress(){  return std::ref(data.ipaddress); } 
 void setIpaddress( std::string  &val){  data.ipaddress=val;} 
 void setIpaddress(std::string_view val){  data.ipaddress=val;} 

 std::string  getVisittime(){  return data.visittime; } 
 std::string & getRefVisittime(){  return std::ref(data.visittime); } 
 void setVisittime( std::string  &val){  data.visittime=val;} 
 void setVisittime(std::string_view val){  data.visittime=val;} 

 std::string  getUseragent(){  return data.useragent; } 
 std::string & getRefUseragent(){  return std::ref(data.useragent); } 
 void setUseragent( std::string  &val){  data.useragent=val;} 
 void setUseragent(std::string_view val){  data.useragent=val;} 

 std::string  getReferer(){  return data.referer; } 
 std::string & getRefReferer(){  return std::ref(data.referer); } 
 void setReferer( std::string  &val){  data.referer=val;} 
 void setReferer(std::string_view val){  data.referer=val;} 

 std::string  getCururl(){  return data.cururl; } 
 std::string & getRefCururl(){  return std::ref(data.cururl); } 
 void setCururl( std::string  &val){  data.cururl=val;} 
 void setCururl(std::string_view val){  data.cururl=val;} 

 std::string  getAddress(){  return data.address; } 
 std::string & getRefAddress(){  return std::ref(data.address); } 
 void setAddress( std::string  &val){  data.address=val;} 
 void setAddress(std::string_view val){  data.address=val;} 

 std::string  getHostname(){  return data.hostname; } 
 std::string & getRefHostname(){  return std::ref(data.hostname); } 
 void setHostname( std::string  &val){  data.hostname=val;} 
 void setHostname(std::string_view val){  data.hostname=val;} 

 std::string  getDerefererurl(){  return data.derefererurl; } 
 std::string & getRefDerefererurl(){  return std::ref(data.derefererurl); } 
 void setDerefererurl( std::string  &val){  data.derefererurl=val;} 
 void setDerefererurl(std::string_view val){  data.derefererurl=val;} 

 std::string  getDeurl(){  return data.deurl; } 
 std::string & getRefDeurl(){  return std::ref(data.deurl); } 
 void setDeurl( std::string  &val){  data.deurl=val;} 
 void setDeurl(std::string_view val){  data.deurl=val;} 

sitelog_info::meta getnewData(){
 	 struct sitelog_info::meta newdata;
	 return newdata; 
} 
sitelog_info::meta getData(){
 	 return data; 
} 
std::vector<sitelog_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<sitelog_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<sitelog_info::col_names.size();jj++){
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
if(tree_data[n].logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(tree_data[n].logid);
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
if(tree_data[n].memberid==0){
	tempsql<<"\"memberid\":0";
 }else{ 
	tempsql<<"\"memberid\":"<<std::to_string(tree_data[n].memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].ipport==0){
	tempsql<<"\"ipport\":0";
 }else{ 
	tempsql<<"\"ipport\":"<<std::to_string(tree_data[n].ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].httpv==0){
	tempsql<<"\"httpv\":0";
 }else{ 
	tempsql<<"\"httpv\":"<<std::to_string(tree_data[n].httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"ipaddress\":\""<<http::utf8_to_jsonstring(tree_data[n].ipaddress)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"visittime\":\""<<http::utf8_to_jsonstring(tree_data[n].visittime)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"useragent\":\""<<http::utf8_to_jsonstring(tree_data[n].useragent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"referer\":\""<<http::utf8_to_jsonstring(tree_data[n].referer)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"cururl\":\""<<http::utf8_to_jsonstring(tree_data[n].cururl)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(tree_data[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"hostname\":\""<<http::utf8_to_jsonstring(tree_data[n].hostname)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"derefererurl\":\""<<http::utf8_to_jsonstring(tree_data[n].derefererurl)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"deurl\":\""<<http::utf8_to_jsonstring(tree_data[n].deurl)<<"\"";
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
   
   std::string tree_tojson(const std::vector<sitelog_info::meta_tree> &tree_data,std::function<bool(std::string&,const sitelog_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<sitelog_info::col_names.size();jj++){
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
if(tree_data[n].logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(tree_data[n].logid);
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
if(tree_data[n].memberid==0){
	tempsql<<"\"memberid\":0";
 }else{ 
	tempsql<<"\"memberid\":"<<std::to_string(tree_data[n].memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].ipport==0){
	tempsql<<"\"ipport\":0";
 }else{ 
	tempsql<<"\"ipport\":"<<std::to_string(tree_data[n].ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].httpv==0){
	tempsql<<"\"httpv\":0";
 }else{ 
	tempsql<<"\"httpv\":"<<std::to_string(tree_data[n].httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"ipaddress\":\""<<http::utf8_to_jsonstring(tree_data[n].ipaddress)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"visittime\":\""<<http::utf8_to_jsonstring(tree_data[n].visittime)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"useragent\":\""<<http::utf8_to_jsonstring(tree_data[n].useragent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"referer\":\""<<http::utf8_to_jsonstring(tree_data[n].referer)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"cururl\":\""<<http::utf8_to_jsonstring(tree_data[n].cururl)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(tree_data[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"hostname\":\""<<http::utf8_to_jsonstring(tree_data[n].hostname)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"derefererurl\":\""<<http::utf8_to_jsonstring(tree_data[n].derefererurl)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"deurl\":\""<<http::utf8_to_jsonstring(tree_data[n].deurl)<<"\"";
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
   
    template<sitelog_info::cols KeyCol, sitelog_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>()));
        using ValType = decltype(sitelog_info::getField<ValCol>(std::declval<const sitelog_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(sitelog_info::getField<KeyCol>(iter), sitelog_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<sitelog_info::cols KeyCol, sitelog_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>())), 
            decltype(sitelog_info::getField<ValCol>(std::declval<const sitelog_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>())), 
                    decltype(sitelog_info::getField<ValCol>(std::declval<const sitelog_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>()));
        using ValType = decltype(sitelog_info::getField<ValCol>(std::declval<const sitelog_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(sitelog_info::getField<KeyCol>(iter), sitelog_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(sitelog_info::getField<KeyCol>(iter), sitelog_info::getField<ValCol>(iter))) {
                    result.emplace(sitelog_info::getField<KeyCol>(iter), sitelog_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<sitelog_info::cols KeyCol, sitelog_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>()));
        using ValType = decltype(sitelog_info::getField<ValCol>(std::declval<const sitelog_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(sitelog_info::getField<KeyCol>(iter), sitelog_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<sitelog_info::cols KeyCol, sitelog_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>())), 
            decltype(sitelog_info::getField<ValCol>(std::declval<const sitelog_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>())), 
                    decltype(sitelog_info::getField<ValCol>(std::declval<const sitelog_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>()));
        using ValType = decltype(sitelog_info::getField<ValCol>(std::declval<const sitelog_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(sitelog_info::getField<KeyCol>(iter), sitelog_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(sitelog_info::getField<KeyCol>(iter), sitelog_info::getField<ValCol>(iter))) {
                    result.emplace_back(sitelog_info::getField<KeyCol>(iter), sitelog_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<sitelog_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(sitelog_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<sitelog_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(sitelog_info::getField<KeyCol>(std::declval<const sitelog_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(sitelog_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(sitelog_info::getField<KeyCol>(iter))) {
                    result.emplace_back(sitelog_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<sitelog_info::cols Col>
        requires requires(std::ostream& os, decltype(sitelog_info::getField<Col>(std::declval<const sitelog_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << sitelog_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<sitelog_info::cols Col>
        requires requires(std::ostream& os, decltype(sitelog_info::getField<Col>(std::declval<const sitelog_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << sitelog_info::getField<Col>(iter); 
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
   