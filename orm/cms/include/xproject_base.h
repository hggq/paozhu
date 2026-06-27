#ifndef ORM_CMS_XPROJECTBASEMATA_H
#define ORM_CMS_XPROJECTBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sat, 27 Jun 2026 04:40:19 GMT
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

namespace xproject_info
{
 
    enum class cols : unsigned char 
    {
		xpjid = 0,
		userid = 1,
		prexpjid = 2,
		dpid = 3,
		grouptype = 4,
		title = 5,
		adminuserid = 6,
		regdate = 7,
		begindate = 8,
		expiredate = 9,
		isopen = 10,
		clientid = 11,
		totalnum = 12,
		referdocverion = 13,
		xtheme = 14,
		xlogo = 15,
		introduce = 16,
		giturl = 17,
		gitname = 18,
		gitpwd = 19,
		xcolor = 20,
		fupan = 21,
		totalvalue = 22,
		expectday = 23,
		realday = 24,

    };

    struct meta
    {
		 unsigned  int  xpjid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  prexpjid = 0; ///**/
		 unsigned  int  dpid = 0; ///**/
		 unsigned  int  grouptype = 0; ///**/
		 std::string  title = ""; ///**/
		 unsigned  int  adminuserid = 0; ///**/
		 unsigned  int  regdate = 0; ///**/
		 unsigned  int  begindate = 0; ///**/
		 unsigned  int  expiredate = 0; ///**/
		 char  isopen = 0; ///**/
		 unsigned  int  clientid = 0; ///**/
		 unsigned  int  totalnum = 0; ///**/
		 unsigned  int  referdocverion = 0; ///**/
		 unsigned  int  xtheme = 0; ///**/
		 std::string  xlogo = ""; ///**/
		 std::string  introduce = ""; ///**/
		 std::string  giturl = ""; ///**/
		 std::string  gitname = ""; ///**/
		 std::string  gitpwd = ""; ///**/
		 std::string  xcolor = ""; ///**/
		 std::string  fupan = ""; ///**/
		 double  totalvalue = 0; ///**/
		 float  expectday = 0; ///**/
		 float  realday = 0; ///**/
	};
  
    struct meta_tree
    {
		 unsigned  int  xpjid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  prexpjid = 0; ///**/
		 unsigned  int  dpid = 0; ///**/
		 unsigned  int  grouptype = 0; ///**/
		 std::string  title = ""; ///**/
		 unsigned  int  adminuserid = 0; ///**/
		 unsigned  int  regdate = 0; ///**/
		 unsigned  int  begindate = 0; ///**/
		 unsigned  int  expiredate = 0; ///**/
		 char  isopen = 0; ///**/
		 unsigned  int  clientid = 0; ///**/
		 unsigned  int  totalnum = 0; ///**/
		 unsigned  int  referdocverion = 0; ///**/
		 unsigned  int  xtheme = 0; ///**/
		 std::string  xlogo = ""; ///**/
		 std::string  introduce = ""; ///**/
		 std::string  giturl = ""; ///**/
		 std::string  gitname = ""; ///**/
		 std::string  gitpwd = ""; ///**/
		 std::string  xcolor = ""; ///**/
		 std::string  fupan = ""; ///**/
		 double  totalvalue = 0; ///**/
		 float  expectday = 0; ///**/
		 float  realday = 0; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  xpjid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  prexpjid = 0; ///**/
		 unsigned  int  dpid = 0; ///**/
		 unsigned  int  grouptype = 0; ///**/
		 std::string  title = ""; ///**/
		 unsigned  int  adminuserid = 0; ///**/
		 unsigned  int  regdate = 0; ///**/
		 unsigned  int  begindate = 0; ///**/
		 unsigned  int  expiredate = 0; ///**/
		 char  isopen = 0; ///**/
		 unsigned  int  clientid = 0; ///**/
		 unsigned  int  totalnum = 0; ///**/
		 unsigned  int  referdocverion = 0; ///**/
		 unsigned  int  xtheme = 0; ///**/
		 std::string  xlogo = ""; ///**/
		 std::string  introduce = ""; ///**/
		 std::string  giturl = ""; ///**/
		 std::string  gitname = ""; ///**/
		 std::string  gitpwd = ""; ///**/
		 std::string  xcolor = ""; ///**/
		 std::string  fupan = ""; ///**/
		 double  totalvalue = 0; ///**/
		 float  expectday = 0; ///**/
		 float  realday = 0; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::xpjid) { 
		 return m.xpjid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::prexpjid) { 
		 return m.prexpjid;
		} else if constexpr (Col == cols::dpid) { 
		 return m.dpid;
		} else if constexpr (Col == cols::grouptype) { 
		 return m.grouptype;
		} else if constexpr (Col == cols::title) { 
		 return m.title;
		} else if constexpr (Col == cols::adminuserid) { 
		 return m.adminuserid;
		} else if constexpr (Col == cols::regdate) { 
		 return m.regdate;
		} else if constexpr (Col == cols::begindate) { 
		 return m.begindate;
		} else if constexpr (Col == cols::expiredate) { 
		 return m.expiredate;
		} else if constexpr (Col == cols::isopen) { 
		 return m.isopen;
		} else if constexpr (Col == cols::clientid) { 
		 return m.clientid;
		} else if constexpr (Col == cols::totalnum) { 
		 return m.totalnum;
		} else if constexpr (Col == cols::referdocverion) { 
		 return m.referdocverion;
		} else if constexpr (Col == cols::xtheme) { 
		 return m.xtheme;
		} else if constexpr (Col == cols::xlogo) { 
		 return m.xlogo;
		} else if constexpr (Col == cols::introduce) { 
		 return m.introduce;
		} else if constexpr (Col == cols::giturl) { 
		 return m.giturl;
		} else if constexpr (Col == cols::gitname) { 
		 return m.gitname;
		} else if constexpr (Col == cols::gitpwd) { 
		 return m.gitpwd;
		} else if constexpr (Col == cols::xcolor) { 
		 return m.xcolor;
		} else if constexpr (Col == cols::fupan) { 
		 return m.fupan;
		} else if constexpr (Col == cols::totalvalue) { 
		 return m.totalvalue;
		} else if constexpr (Col == cols::expectday) { 
		 return m.expectday;
		} else if constexpr (Col == cols::realday) { 
		 return m.realday;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using xpjid =  unsigned  int ;
		using userid =  unsigned  int ;
		using prexpjid =  unsigned  int ;
		using dpid =  unsigned  int ;
		using grouptype =  unsigned  int ;
		using title =  std::string ;
		using adminuserid =  unsigned  int ;
		using regdate =  unsigned  int ;
		using begindate =  unsigned  int ;
		using expiredate =  unsigned  int ;
		using isopen =  char ;
		using clientid =  unsigned  int ;
		using totalnum =  unsigned  int ;
		using referdocverion =  unsigned  int ;
		using xtheme =  unsigned  int ;
		using xlogo =  std::string ;
		using introduce =  std::string ;
		using giturl =  std::string ;
		using gitname =  std::string ;
		using gitpwd =  std::string ;
		using xcolor =  std::string ;
		using fupan =  std::string ;
		using totalvalue =  double ;
		using expectday =  float ;
		using realday =  float ;

    }

    
    #define ORM_CMS_XPROJECT_EXPAND(x) x 
    
    #define ORM_CMS_XPROJECT_META_FIELD_TYPE(col) \
         orm::cms::xproject_info::type::col 
    
    #define ORM_CMS_XPROJECT_PROJ_MEMBER(col) \
          ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_1(c1) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_1( c1)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c2))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c3))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c4))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c5))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c6))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c7))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c8))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c9))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c10))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c11))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c12))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c13))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c14))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c15))
         
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBER(c16))
         
    #define ORM_CMS_XPROJECT_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_XPROJECT_PROJ_MEMBERS(...) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_16, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_15, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_14, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_13, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_12, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_11, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_10, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_9, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_8, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_7, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_6, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_5, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_4, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_3, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_2, \
            ORM_CMS_XPROJECT_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_XPROJECT_COUNT(...) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_XPROJECT_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_XPROJECT_TO_JSON_1(c1) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_XPROJECT_TO_JSON_2(c1,c2) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_XPROJECT_TO_JSON_BODY(...) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_XPROJECT_TO_JSON_16,ORM_CMS_XPROJECT_TO_JSON_15,ORM_CMS_XPROJECT_TO_JSON_14,ORM_CMS_XPROJECT_TO_JSON_13,ORM_CMS_XPROJECT_TO_JSON_12,ORM_CMS_XPROJECT_TO_JSON_11,ORM_CMS_XPROJECT_TO_JSON_10,ORM_CMS_XPROJECT_TO_JSON_9,ORM_CMS_XPROJECT_TO_JSON_8,ORM_CMS_XPROJECT_TO_JSON_7,ORM_CMS_XPROJECT_TO_JSON_6,ORM_CMS_XPROJECT_TO_JSON_5,ORM_CMS_XPROJECT_TO_JSON_4,ORM_CMS_XPROJECT_TO_JSON_3,ORM_CMS_XPROJECT_TO_JSON_2,ORM_CMS_XPROJECT_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_XPROJECT_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_1(n1)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_1(n1)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_XPROJECT_CAT(a, b) ORM_CMS_XPROJECT_CAT_(a, b)
    #define ORM_CMS_XPROJECT_CAT_(a, b) a##b

    #define ORM_CMS_XPROJECT_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_XPROJECT_CAT(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_XPROJECT_TO_JSON_CUSTOM(...) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_XPROJECT_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_XPROJECT_SET_VAL_1(c1) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_XPROJECT_SET_VAL_2(c1,c2) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_1(c1)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_2(c1,c2)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_XPROJECT_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_XPROJECT_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_XPROJECT_CAT(ORM_CMS_XPROJECT_SET_VAL_, N)
    
    
    #define ORM_CMS_XPROJECT_SET_VAL_FIELDS(...) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_XPROJECT_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_FIELDS(ORM_CMS_XPROJECT_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_XPROJECT_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::xproject_info { \
            struct StructName { \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_XPROJECT_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::xproject_info { \
            struct StructName { \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM(ORM_CMS_XPROJECT_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_XPROJECT_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::xproject_info { \
            struct StructName { \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_XPROJECT_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::xproject_info { \
            struct StructName { \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_XPROJECT_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::xproject_info { \
            struct StructName { \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_TO_JSON_CUSTOM(ORM_CMS_XPROJECT_UNWRAP CustomNames));  \
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
                        ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_XPROJECT_EXPAND(ORM_CMS_XPROJECT_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,25> col_names={"xpjid","userid","prexpjid","dpid","grouptype","title","adminuserid","regdate","begindate","expiredate","isopen","clientid","totalnum","referdocverion","xtheme","xlogo","introduce","giturl","gitname","gitpwd","xcolor","fupan","totalvalue","expectday","realday"};
	static constexpr std::array<unsigned char,25> col_types={3,3,3,3,3,253,3,3,3,3,1,3,3,3,3,253,252,253,252,253,253,252,5,4,4};
	static constexpr std::array<unsigned char,25> col_length={0,0,0,0,0,120,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,0};
	static constexpr std::array<unsigned char,25> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct xproject_base
{
      xproject_info::meta data;
    std::vector<xproject_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<xproject_info::meta>::iterator begin(){     return record.begin(); }
std::vector<xproject_info::meta>::iterator end(){     return record.end(); }
std::vector<xproject_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<xproject_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="xproject";
static constexpr std::string_view org_tablename="xproject";
static constexpr std::string_view modelname="Xproject";

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
   	 return 6;
break;
case 'b':
   	 return 8;
break;
case 'c':
   	 return 11;
break;
case 'd':
   	 return 3;
break;
case 'e':
 switch(coln.size()){  
case 9:
   	 return 23;
break;
case 10:
   	 return 9;
break;
 }
 break;
case 'f':
   	 return 21;
break;
case 'g':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 19; }
 if(colpospppc=='l'){ return 17; }
   	 break;
case 7:
   	 return 18;
break;
case 9:
   	 return 4;
break;
 }
 break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 10;
break;
case 9:
   	 return 16;
break;
 }
 break;
case 'p':
   	 return 2;
break;
case 'r':
 switch(coln.size()){  
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 7; }
 if(colpospppc=='y'){ return 24; }
   	 break;
case 14:
   	 return 13;
break;
 }
 break;
case 't':
 switch(coln.size()){  
case 5:
   	 return 5;
break;
case 8:
   	 return 12;
break;
case 10:
   	 return 22;
break;
 }
 break;
case 'u':
   	 return 1;
break;
case 'x':
 switch(coln.size()){  
case 5:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 0; }
 if(colpospppc=='o'){ return 15; }
   	 break;
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 14; }
 if(colpospppc=='r'){ return 20; }
   	 break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "xpjid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     xproject_info::meta metatemp;    
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
        for(;j<xproject_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xproject_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.xpjid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.xpjid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.prexpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.prexpjid);
}
if(data.dpid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.dpid);
}
if(data.grouptype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.grouptype);
}
tempsql<<",'"<<stringaddslash(data.title)<<"'";
if(data.adminuserid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.adminuserid);
}
if(data.regdate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.regdate);
}
if(data.begindate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.begindate);
}
if(data.expiredate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.expiredate);
}
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
if(data.clientid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.clientid);
}
if(data.totalnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.totalnum);
}
if(data.referdocverion==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.referdocverion);
}
if(data.xtheme==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.xtheme);
}
tempsql<<",'"<<stringaddslash(data.xlogo)<<"'";
tempsql<<",'"<<stringaddslash(data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(data.giturl)<<"'";
tempsql<<",'"<<stringaddslash(data.gitname)<<"'";
tempsql<<",'"<<stringaddslash(data.gitpwd)<<"'";
tempsql<<",'"<<stringaddslash(data.xcolor)<<"'";
tempsql<<",'"<<stringaddslash(data.fupan)<<"'";
if(data.totalvalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.totalvalue);
}
if(data.expectday==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.expectday);
}
if(data.realday==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.realday);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string make_data_insert_sql(const xproject_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<xproject_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xproject_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.xpjid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.xpjid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.prexpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.prexpjid);
}
if(insert_data.dpid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.dpid);
}
if(insert_data.grouptype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.grouptype);
}
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
if(insert_data.adminuserid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.adminuserid);
}
if(insert_data.regdate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.regdate);
}
if(insert_data.begindate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.begindate);
}
if(insert_data.expiredate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.expiredate);
}
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
}
if(insert_data.clientid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.clientid);
}
if(insert_data.totalnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.totalnum);
}
if(insert_data.referdocverion==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.referdocverion);
}
if(insert_data.xtheme==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.xtheme);
}
tempsql<<",'"<<stringaddslash(insert_data.xlogo)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.giturl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.gitname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.gitpwd)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.xcolor)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.fupan)<<"'";
if(insert_data.totalvalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.totalvalue);
}
if(insert_data.expectday==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.expectday);
}
if(insert_data.realday==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.realday);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string make_vector_insert_sql(const std::vector<xproject_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<xproject_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<xproject_info::col_names[j];
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


            	if(insert_data[i].xpjid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].xpjid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].prexpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].prexpjid);
	}
	if(insert_data[i].dpid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].dpid);
	}
	if(insert_data[i].grouptype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].grouptype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
	if(insert_data[i].adminuserid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].adminuserid);
	}
	if(insert_data[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].regdate);
	}
	if(insert_data[i].begindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].begindate);
	}
	if(insert_data[i].expiredate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].expiredate);
	}
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
	}
	if(insert_data[i].clientid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].clientid);
	}
	if(insert_data[i].totalnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].totalnum);
	}
	if(insert_data[i].referdocverion==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].referdocverion);
	}
	if(insert_data[i].xtheme==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].xtheme);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].xlogo)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].introduce)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].giturl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].gitname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].gitpwd)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].xcolor)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].fupan)<<"'";
	if(insert_data[i].totalvalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].totalvalue);
	}
	if(insert_data[i].expectday==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].expectday);
	}
	if(insert_data[i].realday==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].realday);
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

        if(data.xpjid==0){
	tempsql<<"`xpjid`=0";
 }else{ 
	tempsql<<"`xpjid`="<<std::to_string(data.xpjid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.prexpjid==0){
	tempsql<<",`prexpjid`=0";
 }else{ 
	tempsql<<",`prexpjid`="<<std::to_string(data.prexpjid);
}
if(data.dpid==0){
	tempsql<<",`dpid`=0";
 }else{ 
	tempsql<<",`dpid`="<<std::to_string(data.dpid);
}
if(data.grouptype==0){
	tempsql<<",`grouptype`=0";
 }else{ 
	tempsql<<",`grouptype`="<<std::to_string(data.grouptype);
}
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
if(data.adminuserid==0){
	tempsql<<",`adminuserid`=0";
 }else{ 
	tempsql<<",`adminuserid`="<<std::to_string(data.adminuserid);
}
if(data.regdate==0){
	tempsql<<",`regdate`=0";
 }else{ 
	tempsql<<",`regdate`="<<std::to_string(data.regdate);
}
if(data.begindate==0){
	tempsql<<",`begindate`=0";
 }else{ 
	tempsql<<",`begindate`="<<std::to_string(data.begindate);
}
if(data.expiredate==0){
	tempsql<<",`expiredate`=0";
 }else{ 
	tempsql<<",`expiredate`="<<std::to_string(data.expiredate);
}
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
if(data.clientid==0){
	tempsql<<",`clientid`=0";
 }else{ 
	tempsql<<",`clientid`="<<std::to_string(data.clientid);
}
if(data.totalnum==0){
	tempsql<<",`totalnum`=0";
 }else{ 
	tempsql<<",`totalnum`="<<std::to_string(data.totalnum);
}
if(data.referdocverion==0){
	tempsql<<",`referdocverion`=0";
 }else{ 
	tempsql<<",`referdocverion`="<<std::to_string(data.referdocverion);
}
if(data.xtheme==0){
	tempsql<<",`xtheme`=0";
 }else{ 
	tempsql<<",`xtheme`="<<std::to_string(data.xtheme);
}
tempsql<<",`xlogo`='"<<stringaddslash(data.xlogo)<<"'";
tempsql<<",`introduce`='"<<stringaddslash(data.introduce)<<"'";
tempsql<<",`giturl`='"<<stringaddslash(data.giturl)<<"'";
tempsql<<",`gitname`='"<<stringaddslash(data.gitname)<<"'";
tempsql<<",`gitpwd`='"<<stringaddslash(data.gitpwd)<<"'";
tempsql<<",`xcolor`='"<<stringaddslash(data.xcolor)<<"'";
tempsql<<",`fupan`='"<<stringaddslash(data.fupan)<<"'";
if(data.totalvalue==0){
	tempsql<<",`totalvalue`=0";
 }else{ 
	tempsql<<",`totalvalue`="<<std::to_string(data.totalvalue);
}
if(data.expectday==0){
	tempsql<<",`expectday`=0";
 }else{ 
	tempsql<<",`expectday`="<<std::to_string(data.expectday);
}
if(data.realday==0){
	tempsql<<",`realday`=0";
 }else{ 
	tempsql<<",`realday`="<<std::to_string(data.realday);
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
if(data.xpjid==0){
	tempsql<<"`xpjid`=0";
 }else{ 
	tempsql<<"`xpjid`="<<std::to_string(data.xpjid);
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
if(data.prexpjid==0){
	tempsql<<"`prexpjid`=0";
 }else{ 
	tempsql<<"`prexpjid`="<<std::to_string(data.prexpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.dpid==0){
	tempsql<<"`dpid`=0";
 }else{ 
	tempsql<<"`dpid`="<<std::to_string(data.dpid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.grouptype==0){
	tempsql<<"`grouptype`=0";
 }else{ 
	tempsql<<"`grouptype`="<<std::to_string(data.grouptype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.adminuserid==0){
	tempsql<<"`adminuserid`=0";
 }else{ 
	tempsql<<"`adminuserid`="<<std::to_string(data.adminuserid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.regdate==0){
	tempsql<<"`regdate`=0";
 }else{ 
	tempsql<<"`regdate`="<<std::to_string(data.regdate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.begindate==0){
	tempsql<<"`begindate`=0";
 }else{ 
	tempsql<<"`begindate`="<<std::to_string(data.begindate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.expiredate==0){
	tempsql<<"`expiredate`=0";
 }else{ 
	tempsql<<"`expiredate`="<<std::to_string(data.expiredate);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.clientid==0){
	tempsql<<"`clientid`=0";
 }else{ 
	tempsql<<"`clientid`="<<std::to_string(data.clientid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.totalnum==0){
	tempsql<<"`totalnum`=0";
 }else{ 
	tempsql<<"`totalnum`="<<std::to_string(data.totalnum);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.referdocverion==0){
	tempsql<<"`referdocverion`=0";
 }else{ 
	tempsql<<"`referdocverion`="<<std::to_string(data.referdocverion);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.xtheme==0){
	tempsql<<"`xtheme`=0";
 }else{ 
	tempsql<<"`xtheme`="<<std::to_string(data.xtheme);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`xlogo`='"<<stringaddslash(data.xlogo)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`introduce`='"<<stringaddslash(data.introduce)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`giturl`='"<<stringaddslash(data.giturl)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`gitname`='"<<stringaddslash(data.gitname)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`gitpwd`='"<<stringaddslash(data.gitpwd)<<"'";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`xcolor`='"<<stringaddslash(data.xcolor)<<"'";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`fupan`='"<<stringaddslash(data.fupan)<<"'";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
if(data.totalvalue==0){
	tempsql<<"`totalvalue`=0";
 }else{ 
	tempsql<<"`totalvalue`="<<std::to_string(data.totalvalue);
}
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
if(data.expectday==0){
	tempsql<<"`expectday`=0";
 }else{ 
	tempsql<<"`expectday`="<<std::to_string(data.expectday);
}
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
if(data.realday==0){
	tempsql<<"`realday`=0";
 }else{ 
	tempsql<<"`realday`="<<std::to_string(data.realday);
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
        for (; j < xproject_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << xproject_info::col_names[j];
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
            	if(record[i].xpjid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].xpjid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].prexpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].prexpjid);
	}
	if(record[i].dpid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].dpid);
	}
	if(record[i].grouptype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].grouptype);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	if(record[i].adminuserid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminuserid);
	}
	if(record[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].regdate);
	}
	if(record[i].begindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].begindate);
	}
	if(record[i].expiredate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expiredate);
	}
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].clientid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].clientid);
	}
	if(record[i].totalnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].totalnum);
	}
	if(record[i].referdocverion==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].referdocverion);
	}
	if(record[i].xtheme==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xtheme);
	}
	tempsql<<",'"<<stringaddslash(record[i].xlogo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].giturl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].gitname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].gitpwd)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].xcolor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].fupan)<<"'";
	if(record[i].totalvalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].totalvalue);
	}
	if(record[i].expectday==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expectday);
	}
	if(record[i].realday==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].realday);
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
        for (; j < xproject_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << xproject_info::col_names[j];
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
            	if(record[i].xpjid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].xpjid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].prexpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].prexpjid);
	}
	if(record[i].dpid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].dpid);
	}
	if(record[i].grouptype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].grouptype);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	if(record[i].adminuserid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminuserid);
	}
	if(record[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].regdate);
	}
	if(record[i].begindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].begindate);
	}
	if(record[i].expiredate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expiredate);
	}
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].clientid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].clientid);
	}
	if(record[i].totalnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].totalnum);
	}
	if(record[i].referdocverion==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].referdocverion);
	}
	if(record[i].xtheme==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xtheme);
	}
	tempsql<<",'"<<stringaddslash(record[i].xlogo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].giturl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].gitname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].gitpwd)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].xcolor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].fupan)<<"'";
	if(record[i].totalvalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].totalvalue);
	}
	if(record[i].expectday==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expectday);
	}
	if(record[i].realday==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].realday);
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
            for(jj=0;jj<xproject_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.xpjid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xpjid));
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
if(data.prexpjid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.prexpjid));
}
 break;
 case 3:
if(data.dpid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.dpid));
}
 break;
 case 4:
if(data.grouptype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.grouptype));
}
 break;
 case 5:
	temparray.push_back(data.title);
 break;
 case 6:
if(data.adminuserid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.adminuserid));
}
 break;
 case 7:
if(data.regdate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.regdate));
}
 break;
 case 8:
if(data.begindate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.begindate));
}
 break;
 case 9:
if(data.expiredate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expiredate));
}
 break;
 case 10:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 11:
if(data.clientid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.clientid));
}
 break;
 case 12:
if(data.totalnum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.totalnum));
}
 break;
 case 13:
if(data.referdocverion==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.referdocverion));
}
 break;
 case 14:
if(data.xtheme==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xtheme));
}
 break;
 case 15:
	temparray.push_back(data.xlogo);
 break;
 case 16:
	temparray.push_back(data.introduce);
 break;
 case 17:
	temparray.push_back(data.giturl);
 break;
 case 18:
	temparray.push_back(data.gitname);
 break;
 case 19:
	temparray.push_back(data.gitpwd);
 break;
 case 20:
	temparray.push_back(data.xcolor);
 break;
 case 21:
	temparray.push_back(data.fupan);
 break;
 case 22:
if(data.totalvalue==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.totalvalue));
}
 break;
 case 23:
if(data.expectday==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expectday));
}
 break;
 case 24:
if(data.realday==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.realday));
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
            for(jj=0;jj<xproject_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.xpjid==0){
	tempsql.insert({"xpjid","0"});
 }else{ 
	tempsql.insert({"xpjid",std::to_string(data.xpjid)});
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
if(data.prexpjid==0){
	tempsql.insert({"prexpjid","0"});
 }else{ 
	tempsql.insert({"prexpjid",std::to_string(data.prexpjid)});
}
 break;
 case 3:
if(data.dpid==0){
	tempsql.insert({"dpid","0"});
 }else{ 
	tempsql.insert({"dpid",std::to_string(data.dpid)});
}
 break;
 case 4:
if(data.grouptype==0){
	tempsql.insert({"grouptype","0"});
 }else{ 
	tempsql.insert({"grouptype",std::to_string(data.grouptype)});
}
 break;
 case 5:
	tempsql.insert({"title",data.title});
 break;
 case 6:
if(data.adminuserid==0){
	tempsql.insert({"adminuserid","0"});
 }else{ 
	tempsql.insert({"adminuserid",std::to_string(data.adminuserid)});
}
 break;
 case 7:
if(data.regdate==0){
	tempsql.insert({"regdate","0"});
 }else{ 
	tempsql.insert({"regdate",std::to_string(data.regdate)});
}
 break;
 case 8:
if(data.begindate==0){
	tempsql.insert({"begindate","0"});
 }else{ 
	tempsql.insert({"begindate",std::to_string(data.begindate)});
}
 break;
 case 9:
if(data.expiredate==0){
	tempsql.insert({"expiredate","0"});
 }else{ 
	tempsql.insert({"expiredate",std::to_string(data.expiredate)});
}
 break;
 case 10:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 11:
if(data.clientid==0){
	tempsql.insert({"clientid","0"});
 }else{ 
	tempsql.insert({"clientid",std::to_string(data.clientid)});
}
 break;
 case 12:
if(data.totalnum==0){
	tempsql.insert({"totalnum","0"});
 }else{ 
	tempsql.insert({"totalnum",std::to_string(data.totalnum)});
}
 break;
 case 13:
if(data.referdocverion==0){
	tempsql.insert({"referdocverion","0"});
 }else{ 
	tempsql.insert({"referdocverion",std::to_string(data.referdocverion)});
}
 break;
 case 14:
if(data.xtheme==0){
	tempsql.insert({"xtheme","0"});
 }else{ 
	tempsql.insert({"xtheme",std::to_string(data.xtheme)});
}
 break;
 case 15:
	tempsql.insert({"xlogo",data.xlogo});
 break;
 case 16:
	tempsql.insert({"introduce",data.introduce});
 break;
 case 17:
	tempsql.insert({"giturl",data.giturl});
 break;
 case 18:
	tempsql.insert({"gitname",data.gitname});
 break;
 case 19:
	tempsql.insert({"gitpwd",data.gitpwd});
 break;
 case 20:
	tempsql.insert({"xcolor",data.xcolor});
 break;
 case 21:
	tempsql.insert({"fupan",data.fupan});
 break;
 case 22:
if(data.totalvalue==0){
	tempsql.insert({"totalvalue","0"});
 }else{ 
	tempsql.insert({"totalvalue",std::to_string(data.totalvalue)});
}
 break;
 case 23:
if(data.expectday==0){
	tempsql.insert({"expectday","0"});
 }else{ 
	tempsql.insert({"expectday",std::to_string(data.expectday)});
}
 break;
 case 24:
if(data.realday==0){
	tempsql.insert({"realday","0"});
 }else{ 
	tempsql.insert({"realday",std::to_string(data.realday)});
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
if(data.xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(data.xpjid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.prexpjid==0){
	tempsql<<",\"prexpjid\":0";
 }else{ 
	tempsql<<",\"prexpjid\":"<<std::to_string(data.prexpjid);
}
if(data.dpid==0){
	tempsql<<",\"dpid\":0";
 }else{ 
	tempsql<<",\"dpid\":"<<std::to_string(data.dpid);
}
if(data.grouptype==0){
	tempsql<<",\"grouptype\":0";
 }else{ 
	tempsql<<",\"grouptype\":"<<std::to_string(data.grouptype);
}
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
if(data.adminuserid==0){
	tempsql<<",\"adminuserid\":0";
 }else{ 
	tempsql<<",\"adminuserid\":"<<std::to_string(data.adminuserid);
}
if(data.regdate==0){
	tempsql<<",\"regdate\":0";
 }else{ 
	tempsql<<",\"regdate\":"<<std::to_string(data.regdate);
}
if(data.begindate==0){
	tempsql<<",\"begindate\":0";
 }else{ 
	tempsql<<",\"begindate\":"<<std::to_string(data.begindate);
}
if(data.expiredate==0){
	tempsql<<",\"expiredate\":0";
 }else{ 
	tempsql<<",\"expiredate\":"<<std::to_string(data.expiredate);
}
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
if(data.clientid==0){
	tempsql<<",\"clientid\":0";
 }else{ 
	tempsql<<",\"clientid\":"<<std::to_string(data.clientid);
}
if(data.totalnum==0){
	tempsql<<",\"totalnum\":0";
 }else{ 
	tempsql<<",\"totalnum\":"<<std::to_string(data.totalnum);
}
if(data.referdocverion==0){
	tempsql<<",\"referdocverion\":0";
 }else{ 
	tempsql<<",\"referdocverion\":"<<std::to_string(data.referdocverion);
}
if(data.xtheme==0){
	tempsql<<",\"xtheme\":0";
 }else{ 
	tempsql<<",\"xtheme\":"<<std::to_string(data.xtheme);
}
tempsql<<",\"xlogo\":\""<<http::utf8_to_jsonstring(data.xlogo);
tempsql<<"\"";
tempsql<<",\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce);
tempsql<<"\"";
tempsql<<",\"giturl\":\""<<http::utf8_to_jsonstring(data.giturl);
tempsql<<"\"";
tempsql<<",\"gitname\":\""<<http::utf8_to_jsonstring(data.gitname);
tempsql<<"\"";
tempsql<<",\"gitpwd\":\""<<http::utf8_to_jsonstring(data.gitpwd);
tempsql<<"\"";
tempsql<<",\"xcolor\":\""<<http::utf8_to_jsonstring(data.xcolor);
tempsql<<"\"";
tempsql<<",\"fupan\":\""<<http::utf8_to_jsonstring(data.fupan);
tempsql<<"\"";
if(data.totalvalue==0){
	tempsql<<",\"totalvalue\":0";
 }else{ 
	tempsql<<",\"totalvalue\":"<<std::to_string(data.totalvalue);
}
if(data.expectday==0){
	tempsql<<",\"expectday\":0";
 }else{ 
	tempsql<<",\"expectday\":"<<std::to_string(data.expectday);
}
if(data.realday==0){
	tempsql<<",\"realday\":0";
 }else{ 
	tempsql<<",\"realday\":"<<std::to_string(data.realday);
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
            for(jj=0;jj<xproject_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(data.xpjid);
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
if(data.prexpjid==0){
	tempsql<<"\"prexpjid\":0";
 }else{ 
	tempsql<<"\"prexpjid\":"<<std::to_string(data.prexpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(data.dpid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.grouptype==0){
	tempsql<<"\"grouptype\":0";
 }else{ 
	tempsql<<"\"grouptype\":"<<std::to_string(data.grouptype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.adminuserid==0){
	tempsql<<"\"adminuserid\":0";
 }else{ 
	tempsql<<"\"adminuserid\":"<<std::to_string(data.adminuserid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(data.regdate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(data.begindate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.expiredate==0){
	tempsql<<"\"expiredate\":0";
 }else{ 
	tempsql<<"\"expiredate\":"<<std::to_string(data.expiredate);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.clientid==0){
	tempsql<<"\"clientid\":0";
 }else{ 
	tempsql<<"\"clientid\":"<<std::to_string(data.clientid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(data.totalnum);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.referdocverion==0){
	tempsql<<"\"referdocverion\":0";
 }else{ 
	tempsql<<"\"referdocverion\":"<<std::to_string(data.referdocverion);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.xtheme==0){
	tempsql<<"\"xtheme\":0";
 }else{ 
	tempsql<<"\"xtheme\":"<<std::to_string(data.xtheme);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(data.xlogo)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"giturl\":\""<<http::utf8_to_jsonstring(data.giturl)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitname\":\""<<http::utf8_to_jsonstring(data.gitname)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitpwd\":\""<<http::utf8_to_jsonstring(data.gitpwd)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(data.xcolor)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fupan\":\""<<http::utf8_to_jsonstring(data.fupan)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
if(data.totalvalue==0){
	tempsql<<"\"totalvalue\":0";
 }else{ 
	tempsql<<"\"totalvalue\":"<<std::to_string(data.totalvalue);
}
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
if(data.expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(data.expectday);
}
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
if(data.realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(data.realday);
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
        xproject_info::meta metatemp; 
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
		  http::json_set_val(data.xpjid,set_value_name);
		 break;
		
		case 1:
		  http::json_set_val(data.userid,set_value_name);
		 break;
		
		case 2:
		  http::json_set_val(data.prexpjid,set_value_name);
		 break;
		
		case 3:
		  http::json_set_val(data.dpid,set_value_name);
		 break;
		
		case 4:
		  http::json_set_val(data.grouptype,set_value_name);
		 break;
		
		case 5:
		  http::json_set_val(data.title,set_value_name);
		 break;
		
		case 6:
		  http::json_set_val(data.adminuserid,set_value_name);
		 break;
		
		case 7:
		  http::json_set_val(data.regdate,set_value_name);
		 break;
		
		case 8:
		  http::json_set_val(data.begindate,set_value_name);
		 break;
		
		case 9:
		  http::json_set_val(data.expiredate,set_value_name);
		 break;
		
		case 10:
		  http::json_set_val(data.isopen,set_value_name);
		 break;
		
		case 11:
		  http::json_set_val(data.clientid,set_value_name);
		 break;
		
		case 12:
		  http::json_set_val(data.totalnum,set_value_name);
		 break;
		
		case 13:
		  http::json_set_val(data.referdocverion,set_value_name);
		 break;
		
		case 14:
		  http::json_set_val(data.xtheme,set_value_name);
		 break;
		
		case 15:
		  http::json_set_val(data.xlogo,set_value_name);
		 break;
		
		case 16:
		  http::json_set_val(data.introduce,set_value_name);
		 break;
		
		case 17:
		  http::json_set_val(data.giturl,set_value_name);
		 break;
		
		case 18:
		  http::json_set_val(data.gitname,set_value_name);
		 break;
		
		case 19:
		  http::json_set_val(data.gitpwd,set_value_name);
		 break;
		
		case 20:
		  http::json_set_val(data.xcolor,set_value_name);
		 break;
		
		case 21:
		  http::json_set_val(data.fupan,set_value_name);
		 break;
		
		case 22:
		  http::json_set_val(data.totalvalue,set_value_name);
		 break;
		
		case 23:
		  http::json_set_val(data.expectday,set_value_name);
		 break;
		
		case 24:
		  http::json_set_val(data.realday,set_value_name);
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
        for(jj=0;jj<xproject_info::col_names.size();jj++){
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
if(record[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(record[n].xpjid);
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
if(record[n].prexpjid==0){
	tempsql<<"\"prexpjid\":0";
 }else{ 
	tempsql<<"\"prexpjid\":"<<std::to_string(record[n].prexpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(record[n].dpid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].grouptype==0){
	tempsql<<"\"grouptype\":0";
 }else{ 
	tempsql<<"\"grouptype\":"<<std::to_string(record[n].grouptype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].adminuserid==0){
	tempsql<<"\"adminuserid\":0";
 }else{ 
	tempsql<<"\"adminuserid\":"<<std::to_string(record[n].adminuserid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(record[n].regdate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(record[n].begindate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].expiredate==0){
	tempsql<<"\"expiredate\":0";
 }else{ 
	tempsql<<"\"expiredate\":"<<std::to_string(record[n].expiredate);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].clientid==0){
	tempsql<<"\"clientid\":0";
 }else{ 
	tempsql<<"\"clientid\":"<<std::to_string(record[n].clientid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(record[n].totalnum);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].referdocverion==0){
	tempsql<<"\"referdocverion\":0";
 }else{ 
	tempsql<<"\"referdocverion\":"<<std::to_string(record[n].referdocverion);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].xtheme==0){
	tempsql<<"\"xtheme\":0";
 }else{ 
	tempsql<<"\"xtheme\":"<<std::to_string(record[n].xtheme);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(record[n].xlogo)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"giturl\":\""<<http::utf8_to_jsonstring(record[n].giturl)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitname\":\""<<http::utf8_to_jsonstring(record[n].gitname)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitpwd\":\""<<http::utf8_to_jsonstring(record[n].gitpwd)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(record[n].xcolor)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fupan\":\""<<http::utf8_to_jsonstring(record[n].fupan)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
if(record[n].totalvalue==0){
	tempsql<<"\"totalvalue\":0";
 }else{ 
	tempsql<<"\"totalvalue\":"<<std::to_string(record[n].totalvalue);
}
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
if(record[n].expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(record[n].expectday);
}
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
if(record[n].realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(record[n].realday);
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
   
   std::string to_json(std::function<bool(std::string&,xproject_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<xproject_info::col_names.size();jj++){
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
if(record[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(record[n].xpjid);
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
if(record[n].prexpjid==0){
	tempsql<<"\"prexpjid\":0";
 }else{ 
	tempsql<<"\"prexpjid\":"<<std::to_string(record[n].prexpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(record[n].dpid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].grouptype==0){
	tempsql<<"\"grouptype\":0";
 }else{ 
	tempsql<<"\"grouptype\":"<<std::to_string(record[n].grouptype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].adminuserid==0){
	tempsql<<"\"adminuserid\":0";
 }else{ 
	tempsql<<"\"adminuserid\":"<<std::to_string(record[n].adminuserid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(record[n].regdate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(record[n].begindate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].expiredate==0){
	tempsql<<"\"expiredate\":0";
 }else{ 
	tempsql<<"\"expiredate\":"<<std::to_string(record[n].expiredate);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].clientid==0){
	tempsql<<"\"clientid\":0";
 }else{ 
	tempsql<<"\"clientid\":"<<std::to_string(record[n].clientid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(record[n].totalnum);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].referdocverion==0){
	tempsql<<"\"referdocverion\":0";
 }else{ 
	tempsql<<"\"referdocverion\":"<<std::to_string(record[n].referdocverion);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].xtheme==0){
	tempsql<<"\"xtheme\":0";
 }else{ 
	tempsql<<"\"xtheme\":"<<std::to_string(record[n].xtheme);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(record[n].xlogo)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"giturl\":\""<<http::utf8_to_jsonstring(record[n].giturl)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitname\":\""<<http::utf8_to_jsonstring(record[n].gitname)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitpwd\":\""<<http::utf8_to_jsonstring(record[n].gitpwd)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(record[n].xcolor)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fupan\":\""<<http::utf8_to_jsonstring(record[n].fupan)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
if(record[n].totalvalue==0){
	tempsql<<"\"totalvalue\":0";
 }else{ 
	tempsql<<"\"totalvalue\":"<<std::to_string(record[n].totalvalue);
}
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
if(record[n].expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(record[n].expectday);
}
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
if(record[n].realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(record[n].realday);
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
   long long getPK(){  return data.xpjid; } 
 void setPK(long long val){  data.xpjid=val;} 
 unsigned  int  getXpjid(){  return data.xpjid; } 
 void setXpjid( unsigned  int  val){  data.xpjid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getPrexpjid(){  return data.prexpjid; } 
 void setPrexpjid( unsigned  int  val){  data.prexpjid=val;} 

 unsigned  int  getDpid(){  return data.dpid; } 
 void setDpid( unsigned  int  val){  data.dpid=val;} 

 unsigned  int  getGrouptype(){  return data.grouptype; } 
 void setGrouptype( unsigned  int  val){  data.grouptype=val;} 

 std::string  getTitle(){  return data.title; } 
 std::string & getRefTitle(){  return std::ref(data.title); } 
 void setTitle( std::string  &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

 unsigned  int  getAdminuserid(){  return data.adminuserid; } 
 void setAdminuserid( unsigned  int  val){  data.adminuserid=val;} 

 unsigned  int  getRegdate(){  return data.regdate; } 
 void setRegdate( unsigned  int  val){  data.regdate=val;} 

 unsigned  int  getBegindate(){  return data.begindate; } 
 void setBegindate( unsigned  int  val){  data.begindate=val;} 

 unsigned  int  getExpiredate(){  return data.expiredate; } 
 void setExpiredate( unsigned  int  val){  data.expiredate=val;} 

 char  getIsopen(){  return data.isopen; } 
 void setIsopen( char  val){  data.isopen=val;} 

 unsigned  int  getClientid(){  return data.clientid; } 
 void setClientid( unsigned  int  val){  data.clientid=val;} 

 unsigned  int  getTotalnum(){  return data.totalnum; } 
 void setTotalnum( unsigned  int  val){  data.totalnum=val;} 

 unsigned  int  getReferdocverion(){  return data.referdocverion; } 
 void setReferdocverion( unsigned  int  val){  data.referdocverion=val;} 

 unsigned  int  getXtheme(){  return data.xtheme; } 
 void setXtheme( unsigned  int  val){  data.xtheme=val;} 

 std::string  getXlogo(){  return data.xlogo; } 
 std::string & getRefXlogo(){  return std::ref(data.xlogo); } 
 void setXlogo( std::string  &val){  data.xlogo=val;} 
 void setXlogo(std::string_view val){  data.xlogo=val;} 

 std::string  getIntroduce(){  return data.introduce; } 
 std::string & getRefIntroduce(){  return std::ref(data.introduce); } 
 void setIntroduce( std::string  &val){  data.introduce=val;} 
 void setIntroduce(std::string_view val){  data.introduce=val;} 

 std::string  getGiturl(){  return data.giturl; } 
 std::string & getRefGiturl(){  return std::ref(data.giturl); } 
 void setGiturl( std::string  &val){  data.giturl=val;} 
 void setGiturl(std::string_view val){  data.giturl=val;} 

 std::string  getGitname(){  return data.gitname; } 
 std::string & getRefGitname(){  return std::ref(data.gitname); } 
 void setGitname( std::string  &val){  data.gitname=val;} 
 void setGitname(std::string_view val){  data.gitname=val;} 

 std::string  getGitpwd(){  return data.gitpwd; } 
 std::string & getRefGitpwd(){  return std::ref(data.gitpwd); } 
 void setGitpwd( std::string  &val){  data.gitpwd=val;} 
 void setGitpwd(std::string_view val){  data.gitpwd=val;} 

 std::string  getXcolor(){  return data.xcolor; } 
 std::string & getRefXcolor(){  return std::ref(data.xcolor); } 
 void setXcolor( std::string  &val){  data.xcolor=val;} 
 void setXcolor(std::string_view val){  data.xcolor=val;} 

 std::string  getFupan(){  return data.fupan; } 
 std::string & getRefFupan(){  return std::ref(data.fupan); } 
 void setFupan( std::string  &val){  data.fupan=val;} 
 void setFupan(std::string_view val){  data.fupan=val;} 

 double  getTotalvalue(){  return data.totalvalue; } 
 void setTotalvalue( double  val){  data.totalvalue=val;} 

 float  getExpectday(){  return data.expectday; } 
 void setExpectday( float  val){  data.expectday=val;} 

 float  getRealday(){  return data.realday; } 
 void setRealday( float  val){  data.realday=val;} 

xproject_info::meta getnewData(){
 	 struct xproject_info::meta newdata;
	 return newdata; 
} 
xproject_info::meta getData(){
 	 return data; 
} 
std::vector<xproject_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<xproject_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<xproject_info::col_names.size();jj++){
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
if(tree_data[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(tree_data[n].xpjid);
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
if(tree_data[n].prexpjid==0){
	tempsql<<"\"prexpjid\":0";
 }else{ 
	tempsql<<"\"prexpjid\":"<<std::to_string(tree_data[n].prexpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(tree_data[n].dpid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].grouptype==0){
	tempsql<<"\"grouptype\":0";
 }else{ 
	tempsql<<"\"grouptype\":"<<std::to_string(tree_data[n].grouptype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].adminuserid==0){
	tempsql<<"\"adminuserid\":0";
 }else{ 
	tempsql<<"\"adminuserid\":"<<std::to_string(tree_data[n].adminuserid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(tree_data[n].regdate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(tree_data[n].begindate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expiredate==0){
	tempsql<<"\"expiredate\":0";
 }else{ 
	tempsql<<"\"expiredate\":"<<std::to_string(tree_data[n].expiredate);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].clientid==0){
	tempsql<<"\"clientid\":0";
 }else{ 
	tempsql<<"\"clientid\":"<<std::to_string(tree_data[n].clientid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(tree_data[n].totalnum);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].referdocverion==0){
	tempsql<<"\"referdocverion\":0";
 }else{ 
	tempsql<<"\"referdocverion\":"<<std::to_string(tree_data[n].referdocverion);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].xtheme==0){
	tempsql<<"\"xtheme\":0";
 }else{ 
	tempsql<<"\"xtheme\":"<<std::to_string(tree_data[n].xtheme);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(tree_data[n].xlogo)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"giturl\":\""<<http::utf8_to_jsonstring(tree_data[n].giturl)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitname\":\""<<http::utf8_to_jsonstring(tree_data[n].gitname)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitpwd\":\""<<http::utf8_to_jsonstring(tree_data[n].gitpwd)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(tree_data[n].xcolor)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fupan\":\""<<http::utf8_to_jsonstring(tree_data[n].fupan)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].totalvalue==0){
	tempsql<<"\"totalvalue\":0";
 }else{ 
	tempsql<<"\"totalvalue\":"<<std::to_string(tree_data[n].totalvalue);
}
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(tree_data[n].expectday);
}
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(tree_data[n].realday);
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
   
   std::string tree_tojson(const std::vector<xproject_info::meta_tree> &tree_data,std::function<bool(std::string&,const xproject_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<xproject_info::col_names.size();jj++){
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
if(tree_data[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(tree_data[n].xpjid);
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
if(tree_data[n].prexpjid==0){
	tempsql<<"\"prexpjid\":0";
 }else{ 
	tempsql<<"\"prexpjid\":"<<std::to_string(tree_data[n].prexpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(tree_data[n].dpid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].grouptype==0){
	tempsql<<"\"grouptype\":0";
 }else{ 
	tempsql<<"\"grouptype\":"<<std::to_string(tree_data[n].grouptype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].adminuserid==0){
	tempsql<<"\"adminuserid\":0";
 }else{ 
	tempsql<<"\"adminuserid\":"<<std::to_string(tree_data[n].adminuserid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(tree_data[n].regdate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].begindate==0){
	tempsql<<"\"begindate\":0";
 }else{ 
	tempsql<<"\"begindate\":"<<std::to_string(tree_data[n].begindate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expiredate==0){
	tempsql<<"\"expiredate\":0";
 }else{ 
	tempsql<<"\"expiredate\":"<<std::to_string(tree_data[n].expiredate);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].clientid==0){
	tempsql<<"\"clientid\":0";
 }else{ 
	tempsql<<"\"clientid\":"<<std::to_string(tree_data[n].clientid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(tree_data[n].totalnum);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].referdocverion==0){
	tempsql<<"\"referdocverion\":0";
 }else{ 
	tempsql<<"\"referdocverion\":"<<std::to_string(tree_data[n].referdocverion);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].xtheme==0){
	tempsql<<"\"xtheme\":0";
 }else{ 
	tempsql<<"\"xtheme\":"<<std::to_string(tree_data[n].xtheme);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(tree_data[n].xlogo)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"giturl\":\""<<http::utf8_to_jsonstring(tree_data[n].giturl)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitname\":\""<<http::utf8_to_jsonstring(tree_data[n].gitname)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"gitpwd\":\""<<http::utf8_to_jsonstring(tree_data[n].gitpwd)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(tree_data[n].xcolor)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fupan\":\""<<http::utf8_to_jsonstring(tree_data[n].fupan)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].totalvalue==0){
	tempsql<<"\"totalvalue\":0";
 }else{ 
	tempsql<<"\"totalvalue\":"<<std::to_string(tree_data[n].totalvalue);
}
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(tree_data[n].expectday);
}
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(tree_data[n].realday);
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
   
    template<xproject_info::cols KeyCol, xproject_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>()));
        using ValType = decltype(xproject_info::getField<ValCol>(std::declval<const xproject_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(xproject_info::getField<KeyCol>(iter), xproject_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xproject_info::cols KeyCol, xproject_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>())), 
            decltype(xproject_info::getField<ValCol>(std::declval<const xproject_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>())), 
                    decltype(xproject_info::getField<ValCol>(std::declval<const xproject_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>()));
        using ValType = decltype(xproject_info::getField<ValCol>(std::declval<const xproject_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(xproject_info::getField<KeyCol>(iter), xproject_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xproject_info::getField<KeyCol>(iter), xproject_info::getField<ValCol>(iter))) {
                    result.emplace(xproject_info::getField<KeyCol>(iter), xproject_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xproject_info::cols KeyCol, xproject_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>()));
        using ValType = decltype(xproject_info::getField<ValCol>(std::declval<const xproject_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(xproject_info::getField<KeyCol>(iter), xproject_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xproject_info::cols KeyCol, xproject_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>())), 
            decltype(xproject_info::getField<ValCol>(std::declval<const xproject_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>())), 
                    decltype(xproject_info::getField<ValCol>(std::declval<const xproject_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>()));
        using ValType = decltype(xproject_info::getField<ValCol>(std::declval<const xproject_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(xproject_info::getField<KeyCol>(iter), xproject_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xproject_info::getField<KeyCol>(iter), xproject_info::getField<ValCol>(iter))) {
                    result.emplace_back(xproject_info::getField<KeyCol>(iter), xproject_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xproject_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(xproject_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<xproject_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(xproject_info::getField<KeyCol>(std::declval<const xproject_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(xproject_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(xproject_info::getField<KeyCol>(iter))) {
                    result.emplace_back(xproject_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<xproject_info::cols Col>
        requires requires(std::ostream& os, decltype(xproject_info::getField<Col>(std::declval<const xproject_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << xproject_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<xproject_info::cols Col>
        requires requires(std::ostream& os, decltype(xproject_info::getField<Col>(std::declval<const xproject_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << xproject_info::getField<Col>(iter); 
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
   