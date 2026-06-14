#ifndef ORM_CMS_SITEINFOBASEMATA_H
#define ORM_CMS_SITEINFOBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sun, 14 Jun 2026 00:57:48 GMT
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

namespace siteinfo_info
{
 
    enum class cols : unsigned char 
    {
		sid = 0,
		userid = 1,
		agentid = 2,
		languagetype = 3,
		sitename = 4,
		sitedomain = 5,
		metakeys = 6,
		metadesc = 7,
		copyright = 8,
		beiansn = 9,
		footscript = 10,
		headscript = 11,
		introduce = 12,
		sitelogo = 13,
		sitebanner = 14,
		contactman = 15,
		phone = 16,
		mobile = 17,
		email = 18,
		bankname = 19,
		banksn = 20,
		address = 21,
		zipnum = 22,
		taxsn = 23,
		companyname = 24,
		linkname = 25,
		linkmobile = 26,
		linkaddress = 27,
		theme = 28,
		sitepath = 29,
		isopen = 30,
		created_at = 31,
		enddate = 32,

    };

    struct meta
    {
		 int  sid = 0; ///**/
		 int  userid = 0; ///**/
		 unsigned  int  agentid = 0; ///**/
		 unsigned  int  languagetype = 0; ///*语言类型*/
		 std::string  sitename = ""; ///**/
		 std::string  sitedomain = ""; ///**/
		 std::string  metakeys = ""; ///**/
		 std::string  metadesc = ""; ///**/
		 std::string  copyright = ""; ///**/
		 std::string  beiansn = ""; ///*备案号*/
		 std::string  footscript = ""; ///*底部脚本*/
		 std::string  headscript = ""; ///*头部script*/
		 std::string  introduce = ""; ///**/
		 std::string  sitelogo = ""; ///*logo url*/
		 std::string  sitebanner = ""; ///**/
		 std::string  contactman = ""; ///**/
		 std::string  phone = ""; ///**/
		 std::string  mobile = ""; ///**/
		 std::string  email = ""; ///**/
		 std::string  bankname = ""; ///**/
		 std::string  banksn = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  zipnum = ""; ///**/
		 std::string  taxsn = ""; ///**/
		 std::string  companyname = ""; ///**/
		 std::string  linkname = ""; ///**/
		 std::string  linkmobile = ""; ///**/
		 std::string  linkaddress = ""; ///**/
		 std::string  theme = ""; ///**/
		 std::string  sitepath = ""; ///**/
		 unsigned  char  isopen = 0; ///**/
		 unsigned  int  created_at = 0; ///**/
		 unsigned  int  enddate = 0; ///**/
	};
  
    struct meta_tree
    {
		 int  sid = 0; ///**/
		 int  userid = 0; ///**/
		 unsigned  int  agentid = 0; ///**/
		 unsigned  int  languagetype = 0; ///*语言类型*/
		 std::string  sitename = ""; ///**/
		 std::string  sitedomain = ""; ///**/
		 std::string  metakeys = ""; ///**/
		 std::string  metadesc = ""; ///**/
		 std::string  copyright = ""; ///**/
		 std::string  beiansn = ""; ///*备案号*/
		 std::string  footscript = ""; ///*底部脚本*/
		 std::string  headscript = ""; ///*头部script*/
		 std::string  introduce = ""; ///**/
		 std::string  sitelogo = ""; ///*logo url*/
		 std::string  sitebanner = ""; ///**/
		 std::string  contactman = ""; ///**/
		 std::string  phone = ""; ///**/
		 std::string  mobile = ""; ///**/
		 std::string  email = ""; ///**/
		 std::string  bankname = ""; ///**/
		 std::string  banksn = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  zipnum = ""; ///**/
		 std::string  taxsn = ""; ///**/
		 std::string  companyname = ""; ///**/
		 std::string  linkname = ""; ///**/
		 std::string  linkmobile = ""; ///**/
		 std::string  linkaddress = ""; ///**/
		 std::string  theme = ""; ///**/
		 std::string  sitepath = ""; ///**/
		 unsigned  char  isopen = 0; ///**/
		 unsigned  int  created_at = 0; ///**/
		 unsigned  int  enddate = 0; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 int  sid = 0; ///**/
		 int  userid = 0; ///**/
		 unsigned  int  agentid = 0; ///**/
		 unsigned  int  languagetype = 0; ///*语言类型*/
		 std::string  sitename = ""; ///**/
		 std::string  sitedomain = ""; ///**/
		 std::string  metakeys = ""; ///**/
		 std::string  metadesc = ""; ///**/
		 std::string  copyright = ""; ///**/
		 std::string  beiansn = ""; ///*备案号*/
		 std::string  footscript = ""; ///*底部脚本*/
		 std::string  headscript = ""; ///*头部script*/
		 std::string  introduce = ""; ///**/
		 std::string  sitelogo = ""; ///*logo url*/
		 std::string  sitebanner = ""; ///**/
		 std::string  contactman = ""; ///**/
		 std::string  phone = ""; ///**/
		 std::string  mobile = ""; ///**/
		 std::string  email = ""; ///**/
		 std::string  bankname = ""; ///**/
		 std::string  banksn = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  zipnum = ""; ///**/
		 std::string  taxsn = ""; ///**/
		 std::string  companyname = ""; ///**/
		 std::string  linkname = ""; ///**/
		 std::string  linkmobile = ""; ///**/
		 std::string  linkaddress = ""; ///**/
		 std::string  theme = ""; ///**/
		 std::string  sitepath = ""; ///**/
		 unsigned  char  isopen = 0; ///**/
		 unsigned  int  created_at = 0; ///**/
		 unsigned  int  enddate = 0; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::sid) { 
		 return m.sid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::agentid) { 
		 return m.agentid;
		} else if constexpr (Col == cols::languagetype) { 
		 return m.languagetype;
		} else if constexpr (Col == cols::sitename) { 
		 return m.sitename;
		} else if constexpr (Col == cols::sitedomain) { 
		 return m.sitedomain;
		} else if constexpr (Col == cols::metakeys) { 
		 return m.metakeys;
		} else if constexpr (Col == cols::metadesc) { 
		 return m.metadesc;
		} else if constexpr (Col == cols::copyright) { 
		 return m.copyright;
		} else if constexpr (Col == cols::beiansn) { 
		 return m.beiansn;
		} else if constexpr (Col == cols::footscript) { 
		 return m.footscript;
		} else if constexpr (Col == cols::headscript) { 
		 return m.headscript;
		} else if constexpr (Col == cols::introduce) { 
		 return m.introduce;
		} else if constexpr (Col == cols::sitelogo) { 
		 return m.sitelogo;
		} else if constexpr (Col == cols::sitebanner) { 
		 return m.sitebanner;
		} else if constexpr (Col == cols::contactman) { 
		 return m.contactman;
		} else if constexpr (Col == cols::phone) { 
		 return m.phone;
		} else if constexpr (Col == cols::mobile) { 
		 return m.mobile;
		} else if constexpr (Col == cols::email) { 
		 return m.email;
		} else if constexpr (Col == cols::bankname) { 
		 return m.bankname;
		} else if constexpr (Col == cols::banksn) { 
		 return m.banksn;
		} else if constexpr (Col == cols::address) { 
		 return m.address;
		} else if constexpr (Col == cols::zipnum) { 
		 return m.zipnum;
		} else if constexpr (Col == cols::taxsn) { 
		 return m.taxsn;
		} else if constexpr (Col == cols::companyname) { 
		 return m.companyname;
		} else if constexpr (Col == cols::linkname) { 
		 return m.linkname;
		} else if constexpr (Col == cols::linkmobile) { 
		 return m.linkmobile;
		} else if constexpr (Col == cols::linkaddress) { 
		 return m.linkaddress;
		} else if constexpr (Col == cols::theme) { 
		 return m.theme;
		} else if constexpr (Col == cols::sitepath) { 
		 return m.sitepath;
		} else if constexpr (Col == cols::isopen) { 
		 return m.isopen;
		} else if constexpr (Col == cols::created_at) { 
		 return m.created_at;
		} else if constexpr (Col == cols::enddate) { 
		 return m.enddate;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using sid =  int ;
		using userid =  int ;
		using agentid =  unsigned  int ;
		using languagetype =  unsigned  int ;
		using sitename =  std::string ;
		using sitedomain =  std::string ;
		using metakeys =  std::string ;
		using metadesc =  std::string ;
		using copyright =  std::string ;
		using beiansn =  std::string ;
		using footscript =  std::string ;
		using headscript =  std::string ;
		using introduce =  std::string ;
		using sitelogo =  std::string ;
		using sitebanner =  std::string ;
		using contactman =  std::string ;
		using phone =  std::string ;
		using mobile =  std::string ;
		using email =  std::string ;
		using bankname =  std::string ;
		using banksn =  std::string ;
		using address =  std::string ;
		using zipnum =  std::string ;
		using taxsn =  std::string ;
		using companyname =  std::string ;
		using linkname =  std::string ;
		using linkmobile =  std::string ;
		using linkaddress =  std::string ;
		using theme =  std::string ;
		using sitepath =  std::string ;
		using isopen =  unsigned  char ;
		using created_at =  unsigned  int ;
		using enddate =  unsigned  int ;

    }

    
    #define ORM_CMS_SITEINFO_EXPAND(x) x 
    
    #define ORM_CMS_SITEINFO_META_FIELD_TYPE(col) \
         orm::cms::siteinfo_info::type::col 
    
    #define ORM_CMS_SITEINFO_PROJ_MEMBER(col) \
          ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_1(c1) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_1( c1)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c2))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c3))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c4))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c5))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c6))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c7))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c8))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c9))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c10))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c11))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c12))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c13))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c14))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c15))
         
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBER(c16))
         
    #define ORM_CMS_SITEINFO_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_SITEINFO_PROJ_MEMBERS(...) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_16, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_15, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_14, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_13, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_12, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_11, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_10, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_9, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_8, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_7, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_6, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_5, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_4, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_3, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_2, \
            ORM_CMS_SITEINFO_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_SITEINFO_COUNT(...) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_SITEINFO_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_SITEINFO_TO_JSON_1(c1) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_SITEINFO_TO_JSON_2(c1,c2) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_SITEINFO_TO_JSON_BODY(...) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_SITEINFO_TO_JSON_16,ORM_CMS_SITEINFO_TO_JSON_15,ORM_CMS_SITEINFO_TO_JSON_14,ORM_CMS_SITEINFO_TO_JSON_13,ORM_CMS_SITEINFO_TO_JSON_12,ORM_CMS_SITEINFO_TO_JSON_11,ORM_CMS_SITEINFO_TO_JSON_10,ORM_CMS_SITEINFO_TO_JSON_9,ORM_CMS_SITEINFO_TO_JSON_8,ORM_CMS_SITEINFO_TO_JSON_7,ORM_CMS_SITEINFO_TO_JSON_6,ORM_CMS_SITEINFO_TO_JSON_5,ORM_CMS_SITEINFO_TO_JSON_4,ORM_CMS_SITEINFO_TO_JSON_3,ORM_CMS_SITEINFO_TO_JSON_2,ORM_CMS_SITEINFO_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_SITEINFO_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_1(n1)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_1(n1)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_SITEINFO_CAT(a, b) ORM_CMS_SITEINFO_CAT_(a, b)
    #define ORM_CMS_SITEINFO_CAT_(a, b) a##b

    #define ORM_CMS_SITEINFO_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_SITEINFO_CAT(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_SITEINFO_TO_JSON_CUSTOM(...) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_SITEINFO_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_SITEINFO_SET_VAL_1(c1) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_SITEINFO_SET_VAL_2(c1,c2) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_1(c1)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_2(c1,c2)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_SITEINFO_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_SITEINFO_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_SITEINFO_CAT(ORM_CMS_SITEINFO_SET_VAL_, N)
    
    
    #define ORM_CMS_SITEINFO_SET_VAL_FIELDS(...) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_SITEINFO_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_FIELDS(ORM_CMS_SITEINFO_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_SITEINFO_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::siteinfo_info { \
            struct StructName { \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SITEINFO_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::siteinfo_info { \
            struct StructName { \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM(ORM_CMS_SITEINFO_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_SITEINFO_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::siteinfo_info { \
            struct StructName { \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SITEINFO_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::siteinfo_info { \
            struct StructName { \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_SITEINFO_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::siteinfo_info { \
            struct StructName { \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_TO_JSON_CUSTOM(ORM_CMS_SITEINFO_UNWRAP CustomNames));  \
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
                        ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_SITEINFO_EXPAND(ORM_CMS_SITEINFO_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,33> col_names={"sid","userid","agentid","languagetype","sitename","sitedomain","metakeys","metadesc","copyright","beiansn","footscript","headscript","introduce","sitelogo","sitebanner","contactman","phone","mobile","email","bankname","banksn","address","zipnum","taxsn","companyname","linkname","linkmobile","linkaddress","theme","sitepath","isopen","created_at","enddate"};
	static constexpr std::array<unsigned char,33> col_types={3,3,3,3,253,253,252,252,252,253,252,252,252,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,1,3,3};
	static constexpr std::array<unsigned char,33> col_length={0,0,0,0,120,120,0,0,0,0,0,0,0,0,0,30,60,60,60,120,60,200,20,120,200,30,30,200,60,200,0,0,0};
	static constexpr std::array<unsigned char,33> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct siteinfo_base
{
      siteinfo_info::meta data;
    std::vector<siteinfo_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<siteinfo_info::meta>::iterator begin(){     return record.begin(); }
std::vector<siteinfo_info::meta>::iterator end(){     return record.end(); }
std::vector<siteinfo_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<siteinfo_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="siteinfo";
static constexpr std::string_view modelname="Siteinfo";

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
 if(colpospppc=='d'){ return 2; }
 if(colpospppc=='s'){ return 21; }
   	 break;
 }
 break;
case 'b':
 switch(coln.size()){  
case 6:
   	 return 20;
break;
case 7:
   	 return 9;
break;
case 8:
   	 return 19;
break;
 }
 break;
case 'c':
 switch(coln.size()){  
case 9:
   	 return 8;
break;
case 10:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='n'){ return 15; }
 if(colpospppc=='t'){ return 31; }
   	 break;
case 11:
   	 return 24;
break;
 }
 break;
case 'e':
 switch(coln.size()){  
case 5:
   	 return 18;
break;
case 7:
   	 return 32;
break;
 }
 break;
case 'f':
   	 return 10;
break;
case 'h':
   	 return 11;
break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 30;
break;
case 9:
   	 return 12;
break;
 }
 break;
case 'l':
 switch(coln.size()){  
case 8:
   	 return 25;
break;
case 10:
   	 return 26;
break;
case 11:
   	 return 27;
break;
case 12:
   	 return 3;
break;
 }
 break;
case 'm':
 switch(coln.size()){  
case 6:
   	 return 17;
break;
case 8:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='c'){ return 7; }
 if(colpospppc=='s'){ return 6; }
   	 break;
 }
 break;
case 'p':
   	 return 16;
break;
case 's':
 switch(coln.size()){  
case 3:
   	 return 0;
break;
case 8:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 4; }
 if(colpospppc=='h'){ return 29; }
 if(colpospppc=='o'){ return 13; }
   	 break;
case 10:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='n'){ return 5; }
 if(colpospppc=='r'){ return 14; }
   	 break;
 }
 break;
case 't':
 switch(coln.size()){  
case 5:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 28; }
 if(colpospppc=='n'){ return 23; }
   	 break;
 }
 break;
case 'u':
   	 return 1;
break;
case 'z':
   	 return 22;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "sid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     siteinfo_info::meta metatemp;    
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
        for(;j<siteinfo_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<siteinfo_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.sid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.sid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.agentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.agentid);
}
if(data.languagetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.languagetype);
}
tempsql<<",'"<<stringaddslash(data.sitename)<<"'";
tempsql<<",'"<<stringaddslash(data.sitedomain)<<"'";
tempsql<<",'"<<stringaddslash(data.metakeys)<<"'";
tempsql<<",'"<<stringaddslash(data.metadesc)<<"'";
tempsql<<",'"<<stringaddslash(data.copyright)<<"'";
tempsql<<",'"<<stringaddslash(data.beiansn)<<"'";
tempsql<<",'"<<stringaddslash(data.footscript)<<"'";
tempsql<<",'"<<stringaddslash(data.headscript)<<"'";
tempsql<<",'"<<stringaddslash(data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(data.sitelogo)<<"'";
tempsql<<",'"<<stringaddslash(data.sitebanner)<<"'";
tempsql<<",'"<<stringaddslash(data.contactman)<<"'";
tempsql<<",'"<<stringaddslash(data.phone)<<"'";
tempsql<<",'"<<stringaddslash(data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(data.email)<<"'";
tempsql<<",'"<<stringaddslash(data.bankname)<<"'";
tempsql<<",'"<<stringaddslash(data.banksn)<<"'";
tempsql<<",'"<<stringaddslash(data.address)<<"'";
tempsql<<",'"<<stringaddslash(data.zipnum)<<"'";
tempsql<<",'"<<stringaddslash(data.taxsn)<<"'";
tempsql<<",'"<<stringaddslash(data.companyname)<<"'";
tempsql<<",'"<<stringaddslash(data.linkname)<<"'";
tempsql<<",'"<<stringaddslash(data.linkmobile)<<"'";
tempsql<<",'"<<stringaddslash(data.linkaddress)<<"'";
tempsql<<",'"<<stringaddslash(data.theme)<<"'";
tempsql<<",'"<<stringaddslash(data.sitepath)<<"'";
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
if(data.created_at==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.created_at);
}
if(data.enddate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.enddate);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const siteinfo_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<siteinfo_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<siteinfo_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.sid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.sid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.agentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.agentid);
}
if(insert_data.languagetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.languagetype);
}
tempsql<<",'"<<stringaddslash(insert_data.sitename)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.sitedomain)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.metakeys)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.metadesc)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.copyright)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.beiansn)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.footscript)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.headscript)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.sitelogo)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.sitebanner)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.contactman)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.phone)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.email)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.bankname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.banksn)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.address)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.zipnum)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.taxsn)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.companyname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.linkname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.linkmobile)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.linkaddress)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.theme)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.sitepath)<<"'";
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
}
if(insert_data.created_at==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.created_at);
}
if(insert_data.enddate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.enddate);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<siteinfo_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<siteinfo_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<siteinfo_info::col_names[j];
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


            	if(insert_data[i].sid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].sid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].agentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].agentid);
	}
	if(insert_data[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].languagetype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].sitename)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].sitedomain)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].metakeys)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].metadesc)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].copyright)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].beiansn)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].footscript)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].headscript)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].introduce)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].sitelogo)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].sitebanner)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].contactman)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].phone)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].mobile)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].email)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].bankname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].banksn)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].address)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].zipnum)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].taxsn)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].companyname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].linkname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].linkmobile)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].linkaddress)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].theme)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].sitepath)<<"'";
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
	}
	if(insert_data[i].created_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].created_at);
	}
	if(insert_data[i].enddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].enddate);
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

        if(data.sid==0){
	tempsql<<"`sid`=0";
 }else{ 
	tempsql<<"`sid`="<<std::to_string(data.sid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.agentid==0){
	tempsql<<",`agentid`=0";
 }else{ 
	tempsql<<",`agentid`="<<std::to_string(data.agentid);
}
if(data.languagetype==0){
	tempsql<<",`languagetype`=0";
 }else{ 
	tempsql<<",`languagetype`="<<std::to_string(data.languagetype);
}
tempsql<<",`sitename`='"<<stringaddslash(data.sitename)<<"'";
tempsql<<",`sitedomain`='"<<stringaddslash(data.sitedomain)<<"'";
tempsql<<",`metakeys`='"<<stringaddslash(data.metakeys)<<"'";
tempsql<<",`metadesc`='"<<stringaddslash(data.metadesc)<<"'";
tempsql<<",`copyright`='"<<stringaddslash(data.copyright)<<"'";
tempsql<<",`beiansn`='"<<stringaddslash(data.beiansn)<<"'";
tempsql<<",`footscript`='"<<stringaddslash(data.footscript)<<"'";
tempsql<<",`headscript`='"<<stringaddslash(data.headscript)<<"'";
tempsql<<",`introduce`='"<<stringaddslash(data.introduce)<<"'";
tempsql<<",`sitelogo`='"<<stringaddslash(data.sitelogo)<<"'";
tempsql<<",`sitebanner`='"<<stringaddslash(data.sitebanner)<<"'";
tempsql<<",`contactman`='"<<stringaddslash(data.contactman)<<"'";
tempsql<<",`phone`='"<<stringaddslash(data.phone)<<"'";
tempsql<<",`mobile`='"<<stringaddslash(data.mobile)<<"'";
tempsql<<",`email`='"<<stringaddslash(data.email)<<"'";
tempsql<<",`bankname`='"<<stringaddslash(data.bankname)<<"'";
tempsql<<",`banksn`='"<<stringaddslash(data.banksn)<<"'";
tempsql<<",`address`='"<<stringaddslash(data.address)<<"'";
tempsql<<",`zipnum`='"<<stringaddslash(data.zipnum)<<"'";
tempsql<<",`taxsn`='"<<stringaddslash(data.taxsn)<<"'";
tempsql<<",`companyname`='"<<stringaddslash(data.companyname)<<"'";
tempsql<<",`linkname`='"<<stringaddslash(data.linkname)<<"'";
tempsql<<",`linkmobile`='"<<stringaddslash(data.linkmobile)<<"'";
tempsql<<",`linkaddress`='"<<stringaddslash(data.linkaddress)<<"'";
tempsql<<",`theme`='"<<stringaddslash(data.theme)<<"'";
tempsql<<",`sitepath`='"<<stringaddslash(data.sitepath)<<"'";
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
if(data.created_at==0){
	tempsql<<",`created_at`=0";
 }else{ 
	tempsql<<",`created_at`="<<std::to_string(data.created_at);
}
if(data.enddate==0){
	tempsql<<",`enddate`=0";
 }else{ 
	tempsql<<",`enddate`="<<std::to_string(data.enddate);
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
if(data.sid==0){
	tempsql<<"`sid`=0";
 }else{ 
	tempsql<<"`sid`="<<std::to_string(data.sid);
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
if(data.agentid==0){
	tempsql<<"`agentid`=0";
 }else{ 
	tempsql<<"`agentid`="<<std::to_string(data.agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.languagetype==0){
	tempsql<<"`languagetype`=0";
 }else{ 
	tempsql<<"`languagetype`="<<std::to_string(data.languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitename`='"<<stringaddslash(data.sitename)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitedomain`='"<<stringaddslash(data.sitedomain)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`metakeys`='"<<stringaddslash(data.metakeys)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`metadesc`='"<<stringaddslash(data.metadesc)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`copyright`='"<<stringaddslash(data.copyright)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`beiansn`='"<<stringaddslash(data.beiansn)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`footscript`='"<<stringaddslash(data.footscript)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`headscript`='"<<stringaddslash(data.headscript)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`introduce`='"<<stringaddslash(data.introduce)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitelogo`='"<<stringaddslash(data.sitelogo)<<"'";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitebanner`='"<<stringaddslash(data.sitebanner)<<"'";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`contactman`='"<<stringaddslash(data.contactman)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`phone`='"<<stringaddslash(data.phone)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`mobile`='"<<stringaddslash(data.mobile)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`email`='"<<stringaddslash(data.email)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`bankname`='"<<stringaddslash(data.bankname)<<"'";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`banksn`='"<<stringaddslash(data.banksn)<<"'";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`address`='"<<stringaddslash(data.address)<<"'";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`zipnum`='"<<stringaddslash(data.zipnum)<<"'";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`taxsn`='"<<stringaddslash(data.taxsn)<<"'";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`companyname`='"<<stringaddslash(data.companyname)<<"'";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`linkname`='"<<stringaddslash(data.linkname)<<"'";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`linkmobile`='"<<stringaddslash(data.linkmobile)<<"'";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`linkaddress`='"<<stringaddslash(data.linkaddress)<<"'";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`theme`='"<<stringaddslash(data.theme)<<"'";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitepath`='"<<stringaddslash(data.sitepath)<<"'";
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(data.created_at==0){
	tempsql<<"`created_at`=0";
 }else{ 
	tempsql<<"`created_at`="<<std::to_string(data.created_at);
}
 break;
 case 32:
 if(jj>0){ tempsql<<","; } 
if(data.enddate==0){
	tempsql<<"`enddate`=0";
 }else{ 
	tempsql<<"`enddate`="<<std::to_string(data.enddate);
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
        for (; j < siteinfo_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << siteinfo_info::col_names[j];
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
            	if(record[i].sid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].sid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].agentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].agentid);
	}
	if(record[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languagetype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sitename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitedomain)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].metakeys)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].metadesc)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].copyright)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].beiansn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].footscript)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].headscript)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitelogo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitebanner)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].contactman)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].phone)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].bankname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].banksn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].zipnum)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].taxsn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].companyname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkmobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkaddress)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].theme)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitepath)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].created_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_at);
	}
	if(record[i].enddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].enddate);
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
        for (; j < siteinfo_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << siteinfo_info::col_names[j];
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
            	if(record[i].sid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].sid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].agentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].agentid);
	}
	if(record[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languagetype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sitename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitedomain)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].metakeys)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].metadesc)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].copyright)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].beiansn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].footscript)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].headscript)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitelogo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitebanner)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].contactman)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].phone)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].bankname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].banksn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].zipnum)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].taxsn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].companyname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkmobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkaddress)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].theme)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitepath)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].created_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_at);
	}
	if(record[i].enddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].enddate);
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
            for(jj=0;jj<siteinfo_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.sid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sid));
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
if(data.agentid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.agentid));
}
 break;
 case 3:
if(data.languagetype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.languagetype));
}
 break;
 case 4:
	temparray.push_back(data.sitename);
 break;
 case 5:
	temparray.push_back(data.sitedomain);
 break;
 case 6:
	temparray.push_back(data.metakeys);
 break;
 case 7:
	temparray.push_back(data.metadesc);
 break;
 case 8:
	temparray.push_back(data.copyright);
 break;
 case 9:
	temparray.push_back(data.beiansn);
 break;
 case 10:
	temparray.push_back(data.footscript);
 break;
 case 11:
	temparray.push_back(data.headscript);
 break;
 case 12:
	temparray.push_back(data.introduce);
 break;
 case 13:
	temparray.push_back(data.sitelogo);
 break;
 case 14:
	temparray.push_back(data.sitebanner);
 break;
 case 15:
	temparray.push_back(data.contactman);
 break;
 case 16:
	temparray.push_back(data.phone);
 break;
 case 17:
	temparray.push_back(data.mobile);
 break;
 case 18:
	temparray.push_back(data.email);
 break;
 case 19:
	temparray.push_back(data.bankname);
 break;
 case 20:
	temparray.push_back(data.banksn);
 break;
 case 21:
	temparray.push_back(data.address);
 break;
 case 22:
	temparray.push_back(data.zipnum);
 break;
 case 23:
	temparray.push_back(data.taxsn);
 break;
 case 24:
	temparray.push_back(data.companyname);
 break;
 case 25:
	temparray.push_back(data.linkname);
 break;
 case 26:
	temparray.push_back(data.linkmobile);
 break;
 case 27:
	temparray.push_back(data.linkaddress);
 break;
 case 28:
	temparray.push_back(data.theme);
 break;
 case 29:
	temparray.push_back(data.sitepath);
 break;
 case 30:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 31:
if(data.created_at==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.created_at));
}
 break;
 case 32:
if(data.enddate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.enddate));
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
            for(jj=0;jj<siteinfo_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.sid==0){
	tempsql.insert({"sid","0"});
 }else{ 
	tempsql.insert({"sid",std::to_string(data.sid)});
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
if(data.agentid==0){
	tempsql.insert({"agentid","0"});
 }else{ 
	tempsql.insert({"agentid",std::to_string(data.agentid)});
}
 break;
 case 3:
if(data.languagetype==0){
	tempsql.insert({"languagetype","0"});
 }else{ 
	tempsql.insert({"languagetype",std::to_string(data.languagetype)});
}
 break;
 case 4:
	tempsql.insert({"sitename",data.sitename});
 break;
 case 5:
	tempsql.insert({"sitedomain",data.sitedomain});
 break;
 case 6:
	tempsql.insert({"metakeys",data.metakeys});
 break;
 case 7:
	tempsql.insert({"metadesc",data.metadesc});
 break;
 case 8:
	tempsql.insert({"copyright",data.copyright});
 break;
 case 9:
	tempsql.insert({"beiansn",data.beiansn});
 break;
 case 10:
	tempsql.insert({"footscript",data.footscript});
 break;
 case 11:
	tempsql.insert({"headscript",data.headscript});
 break;
 case 12:
	tempsql.insert({"introduce",data.introduce});
 break;
 case 13:
	tempsql.insert({"sitelogo",data.sitelogo});
 break;
 case 14:
	tempsql.insert({"sitebanner",data.sitebanner});
 break;
 case 15:
	tempsql.insert({"contactman",data.contactman});
 break;
 case 16:
	tempsql.insert({"phone",data.phone});
 break;
 case 17:
	tempsql.insert({"mobile",data.mobile});
 break;
 case 18:
	tempsql.insert({"email",data.email});
 break;
 case 19:
	tempsql.insert({"bankname",data.bankname});
 break;
 case 20:
	tempsql.insert({"banksn",data.banksn});
 break;
 case 21:
	tempsql.insert({"address",data.address});
 break;
 case 22:
	tempsql.insert({"zipnum",data.zipnum});
 break;
 case 23:
	tempsql.insert({"taxsn",data.taxsn});
 break;
 case 24:
	tempsql.insert({"companyname",data.companyname});
 break;
 case 25:
	tempsql.insert({"linkname",data.linkname});
 break;
 case 26:
	tempsql.insert({"linkmobile",data.linkmobile});
 break;
 case 27:
	tempsql.insert({"linkaddress",data.linkaddress});
 break;
 case 28:
	tempsql.insert({"theme",data.theme});
 break;
 case 29:
	tempsql.insert({"sitepath",data.sitepath});
 break;
 case 30:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 31:
if(data.created_at==0){
	tempsql.insert({"created_at","0"});
 }else{ 
	tempsql.insert({"created_at",std::to_string(data.created_at)});
}
 break;
 case 32:
if(data.enddate==0){
	tempsql.insert({"enddate","0"});
 }else{ 
	tempsql.insert({"enddate",std::to_string(data.enddate)});
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
if(data.sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(data.sid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.agentid==0){
	tempsql<<",\"agentid\":0";
 }else{ 
	tempsql<<",\"agentid\":"<<std::to_string(data.agentid);
}
if(data.languagetype==0){
	tempsql<<",\"languagetype\":0";
 }else{ 
	tempsql<<",\"languagetype\":"<<std::to_string(data.languagetype);
}
tempsql<<",\"sitename\":\""<<http::utf8_to_jsonstring(data.sitename);
tempsql<<"\"";
tempsql<<",\"sitedomain\":\""<<http::utf8_to_jsonstring(data.sitedomain);
tempsql<<"\"";
tempsql<<",\"metakeys\":\""<<http::utf8_to_jsonstring(data.metakeys);
tempsql<<"\"";
tempsql<<",\"metadesc\":\""<<http::utf8_to_jsonstring(data.metadesc);
tempsql<<"\"";
tempsql<<",\"copyright\":\""<<http::utf8_to_jsonstring(data.copyright);
tempsql<<"\"";
tempsql<<",\"beiansn\":\""<<http::utf8_to_jsonstring(data.beiansn);
tempsql<<"\"";
tempsql<<",\"footscript\":\""<<http::utf8_to_jsonstring(data.footscript);
tempsql<<"\"";
tempsql<<",\"headscript\":\""<<http::utf8_to_jsonstring(data.headscript);
tempsql<<"\"";
tempsql<<",\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce);
tempsql<<"\"";
tempsql<<",\"sitelogo\":\""<<http::utf8_to_jsonstring(data.sitelogo);
tempsql<<"\"";
tempsql<<",\"sitebanner\":\""<<http::utf8_to_jsonstring(data.sitebanner);
tempsql<<"\"";
tempsql<<",\"contactman\":\""<<http::utf8_to_jsonstring(data.contactman);
tempsql<<"\"";
tempsql<<",\"phone\":\""<<http::utf8_to_jsonstring(data.phone);
tempsql<<"\"";
tempsql<<",\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile);
tempsql<<"\"";
tempsql<<",\"email\":\""<<http::utf8_to_jsonstring(data.email);
tempsql<<"\"";
tempsql<<",\"bankname\":\""<<http::utf8_to_jsonstring(data.bankname);
tempsql<<"\"";
tempsql<<",\"banksn\":\""<<http::utf8_to_jsonstring(data.banksn);
tempsql<<"\"";
tempsql<<",\"address\":\""<<http::utf8_to_jsonstring(data.address);
tempsql<<"\"";
tempsql<<",\"zipnum\":\""<<http::utf8_to_jsonstring(data.zipnum);
tempsql<<"\"";
tempsql<<",\"taxsn\":\""<<http::utf8_to_jsonstring(data.taxsn);
tempsql<<"\"";
tempsql<<",\"companyname\":\""<<http::utf8_to_jsonstring(data.companyname);
tempsql<<"\"";
tempsql<<",\"linkname\":\""<<http::utf8_to_jsonstring(data.linkname);
tempsql<<"\"";
tempsql<<",\"linkmobile\":\""<<http::utf8_to_jsonstring(data.linkmobile);
tempsql<<"\"";
tempsql<<",\"linkaddress\":\""<<http::utf8_to_jsonstring(data.linkaddress);
tempsql<<"\"";
tempsql<<",\"theme\":\""<<http::utf8_to_jsonstring(data.theme);
tempsql<<"\"";
tempsql<<",\"sitepath\":\""<<http::utf8_to_jsonstring(data.sitepath);
tempsql<<"\"";
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
if(data.created_at==0){
	tempsql<<",\"created_at\":0";
 }else{ 
	tempsql<<",\"created_at\":"<<std::to_string(data.created_at);
}
if(data.enddate==0){
	tempsql<<",\"enddate\":0";
 }else{ 
	tempsql<<",\"enddate\":"<<std::to_string(data.enddate);
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
            for(jj=0;jj<siteinfo_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(data.sid);
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
if(data.agentid==0){
	tempsql<<"\"agentid\":0";
 }else{ 
	tempsql<<"\"agentid\":"<<std::to_string(data.agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(data.languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitename\":\""<<http::utf8_to_jsonstring(data.sitename)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitedomain\":\""<<http::utf8_to_jsonstring(data.sitedomain)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metakeys\":\""<<http::utf8_to_jsonstring(data.metakeys)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metadesc\":\""<<http::utf8_to_jsonstring(data.metadesc)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"copyright\":\""<<http::utf8_to_jsonstring(data.copyright)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"beiansn\":\""<<http::utf8_to_jsonstring(data.beiansn)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"footscript\":\""<<http::utf8_to_jsonstring(data.footscript)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"headscript\":\""<<http::utf8_to_jsonstring(data.headscript)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitelogo\":\""<<http::utf8_to_jsonstring(data.sitelogo)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitebanner\":\""<<http::utf8_to_jsonstring(data.sitebanner)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"contactman\":\""<<http::utf8_to_jsonstring(data.contactman)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"phone\":\""<<http::utf8_to_jsonstring(data.phone)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(data.email)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bankname\":\""<<http::utf8_to_jsonstring(data.bankname)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"banksn\":\""<<http::utf8_to_jsonstring(data.banksn)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(data.address)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zipnum\":\""<<http::utf8_to_jsonstring(data.zipnum)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"taxsn\":\""<<http::utf8_to_jsonstring(data.taxsn)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"companyname\":\""<<http::utf8_to_jsonstring(data.companyname)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkname\":\""<<http::utf8_to_jsonstring(data.linkname)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkmobile\":\""<<http::utf8_to_jsonstring(data.linkmobile)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkaddress\":\""<<http::utf8_to_jsonstring(data.linkaddress)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"theme\":\""<<http::utf8_to_jsonstring(data.theme)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitepath\":\""<<http::utf8_to_jsonstring(data.sitepath)<<"\"";
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(data.created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(data.created_at);
}
 break;
 case 32:
 if(jj>0){ tempsql<<","; } 
if(data.enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(data.enddate);
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
        siteinfo_info::meta metatemp; 
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
			data.sid=std::stoi(set_value_name);
		}catch (...) { 
			data.sid=0;
			 }
			break;
		case 1:
		 try{
			data.userid=std::stoi(set_value_name);
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 2:
		 try{
			data.agentid=std::stoul(set_value_name);
		}catch (...) { 
			data.agentid=0;
			 }
			break;
		case 3:
		 try{
			data.languagetype=std::stoul(set_value_name);
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 4:
		 try{
			data.sitename.append(set_value_name);
		}catch (...) { 
			data.sitename.clear();
			 }
			break;
		case 5:
		 try{
			data.sitedomain.append(set_value_name);
		}catch (...) { 
			data.sitedomain.clear();
			 }
			break;
		case 6:
		 try{
			data.metakeys.append(set_value_name);
		}catch (...) { 
			data.metakeys.clear();
			 }
			break;
		case 7:
		 try{
			data.metadesc.append(set_value_name);
		}catch (...) { 
			data.metadesc.clear();
			 }
			break;
		case 8:
		 try{
			data.copyright.append(set_value_name);
		}catch (...) { 
			data.copyright.clear();
			 }
			break;
		case 9:
		 try{
			data.beiansn.append(set_value_name);
		}catch (...) { 
			data.beiansn.clear();
			 }
			break;
		case 10:
		 try{
			data.footscript.append(set_value_name);
		}catch (...) { 
			data.footscript.clear();
			 }
			break;
		case 11:
		 try{
			data.headscript.append(set_value_name);
		}catch (...) { 
			data.headscript.clear();
			 }
			break;
		case 12:
		 try{
			data.introduce.append(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 13:
		 try{
			data.sitelogo.append(set_value_name);
		}catch (...) { 
			data.sitelogo.clear();
			 }
			break;
		case 14:
		 try{
			data.sitebanner.append(set_value_name);
		}catch (...) { 
			data.sitebanner.clear();
			 }
			break;
		case 15:
		 try{
			data.contactman.append(set_value_name);
		}catch (...) { 
			data.contactman.clear();
			 }
			break;
		case 16:
		 try{
			data.phone.append(set_value_name);
		}catch (...) { 
			data.phone.clear();
			 }
			break;
		case 17:
		 try{
			data.mobile.append(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 18:
		 try{
			data.email.append(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 19:
		 try{
			data.bankname.append(set_value_name);
		}catch (...) { 
			data.bankname.clear();
			 }
			break;
		case 20:
		 try{
			data.banksn.append(set_value_name);
		}catch (...) { 
			data.banksn.clear();
			 }
			break;
		case 21:
		 try{
			data.address.append(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 22:
		 try{
			data.zipnum.append(set_value_name);
		}catch (...) { 
			data.zipnum.clear();
			 }
			break;
		case 23:
		 try{
			data.taxsn.append(set_value_name);
		}catch (...) { 
			data.taxsn.clear();
			 }
			break;
		case 24:
		 try{
			data.companyname.append(set_value_name);
		}catch (...) { 
			data.companyname.clear();
			 }
			break;
		case 25:
		 try{
			data.linkname.append(set_value_name);
		}catch (...) { 
			data.linkname.clear();
			 }
			break;
		case 26:
		 try{
			data.linkmobile.append(set_value_name);
		}catch (...) { 
			data.linkmobile.clear();
			 }
			break;
		case 27:
		 try{
			data.linkaddress.append(set_value_name);
		}catch (...) { 
			data.linkaddress.clear();
			 }
			break;
		case 28:
		 try{
			data.theme.append(set_value_name);
		}catch (...) { 
			data.theme.clear();
			 }
			break;
		case 29:
		 try{
			data.sitepath.append(set_value_name);
		}catch (...) { 
			data.sitepath.clear();
			 }
			break;
		case 30:
		 try{
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 31:
		 try{
			data.created_at=std::stoul(set_value_name);
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 32:
		 try{
			data.enddate=std::stoul(set_value_name);
		}catch (...) { 
			data.enddate=0;
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
			data.sid=set_value_name;
		}catch (...) { 
			data.sid=0;
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
			data.agentid=set_value_name;
		}catch (...) { 
			data.agentid=0;
			 }
			break;
		case 3:
		 try{
			data.languagetype=set_value_name;
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 4:
		 try{
			data.sitename=std::to_string(set_value_name);
		}catch (...) { 
			data.sitename.clear();
			 }
			break;
		case 5:
		 try{
			data.sitedomain=std::to_string(set_value_name);
		}catch (...) { 
			data.sitedomain.clear();
			 }
			break;
		case 6:
		 try{
			data.metakeys=std::to_string(set_value_name);
		}catch (...) { 
			data.metakeys.clear();
			 }
			break;
		case 7:
		 try{
			data.metadesc=std::to_string(set_value_name);
		}catch (...) { 
			data.metadesc.clear();
			 }
			break;
		case 8:
		 try{
			data.copyright=std::to_string(set_value_name);
		}catch (...) { 
			data.copyright.clear();
			 }
			break;
		case 9:
		 try{
			data.beiansn=std::to_string(set_value_name);
		}catch (...) { 
			data.beiansn.clear();
			 }
			break;
		case 10:
		 try{
			data.footscript=std::to_string(set_value_name);
		}catch (...) { 
			data.footscript.clear();
			 }
			break;
		case 11:
		 try{
			data.headscript=std::to_string(set_value_name);
		}catch (...) { 
			data.headscript.clear();
			 }
			break;
		case 12:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 13:
		 try{
			data.sitelogo=std::to_string(set_value_name);
		}catch (...) { 
			data.sitelogo.clear();
			 }
			break;
		case 14:
		 try{
			data.sitebanner=std::to_string(set_value_name);
		}catch (...) { 
			data.sitebanner.clear();
			 }
			break;
		case 15:
		 try{
			data.contactman=std::to_string(set_value_name);
		}catch (...) { 
			data.contactman.clear();
			 }
			break;
		case 16:
		 try{
			data.phone=std::to_string(set_value_name);
		}catch (...) { 
			data.phone.clear();
			 }
			break;
		case 17:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 18:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 19:
		 try{
			data.bankname=std::to_string(set_value_name);
		}catch (...) { 
			data.bankname.clear();
			 }
			break;
		case 20:
		 try{
			data.banksn=std::to_string(set_value_name);
		}catch (...) { 
			data.banksn.clear();
			 }
			break;
		case 21:
		 try{
			data.address=std::to_string(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 22:
		 try{
			data.zipnum=std::to_string(set_value_name);
		}catch (...) { 
			data.zipnum.clear();
			 }
			break;
		case 23:
		 try{
			data.taxsn=std::to_string(set_value_name);
		}catch (...) { 
			data.taxsn.clear();
			 }
			break;
		case 24:
		 try{
			data.companyname=std::to_string(set_value_name);
		}catch (...) { 
			data.companyname.clear();
			 }
			break;
		case 25:
		 try{
			data.linkname=std::to_string(set_value_name);
		}catch (...) { 
			data.linkname.clear();
			 }
			break;
		case 26:
		 try{
			data.linkmobile=std::to_string(set_value_name);
		}catch (...) { 
			data.linkmobile.clear();
			 }
			break;
		case 27:
		 try{
			data.linkaddress=std::to_string(set_value_name);
		}catch (...) { 
			data.linkaddress.clear();
			 }
			break;
		case 28:
		 try{
			data.theme=std::to_string(set_value_name);
		}catch (...) { 
			data.theme.clear();
			 }
			break;
		case 29:
		 try{
			data.sitepath=std::to_string(set_value_name);
		}catch (...) { 
			data.sitepath.clear();
			 }
			break;
		case 30:
		 try{
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 31:
		 try{
			data.created_at=set_value_name;
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 32:
		 try{
			data.enddate=set_value_name;
		}catch (...) { 
			data.enddate=0;
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
			data.sid=(int)set_value_name;
		}catch (...) { 
			data.sid=0;
			 }
			break;
		case 1:
		 try{
			data.userid=(int)set_value_name;
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 2:
		 try{
			data.agentid=(unsigned int)set_value_name;
		}catch (...) { 
			data.agentid=0;
			 }
			break;
		case 3:
		 try{
			data.languagetype=(unsigned int)set_value_name;
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 4:
		 try{
			data.sitename=std::to_string(set_value_name);
		}catch (...) { 
			data.sitename.clear();
			 }
			break;
		case 5:
		 try{
			data.sitedomain=std::to_string(set_value_name);
		}catch (...) { 
			data.sitedomain.clear();
			 }
			break;
		case 6:
		 try{
			data.metakeys=std::to_string(set_value_name);
		}catch (...) { 
			data.metakeys.clear();
			 }
			break;
		case 7:
		 try{
			data.metadesc=std::to_string(set_value_name);
		}catch (...) { 
			data.metadesc.clear();
			 }
			break;
		case 8:
		 try{
			data.copyright=std::to_string(set_value_name);
		}catch (...) { 
			data.copyright.clear();
			 }
			break;
		case 9:
		 try{
			data.beiansn=std::to_string(set_value_name);
		}catch (...) { 
			data.beiansn.clear();
			 }
			break;
		case 10:
		 try{
			data.footscript=std::to_string(set_value_name);
		}catch (...) { 
			data.footscript.clear();
			 }
			break;
		case 11:
		 try{
			data.headscript=std::to_string(set_value_name);
		}catch (...) { 
			data.headscript.clear();
			 }
			break;
		case 12:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 13:
		 try{
			data.sitelogo=std::to_string(set_value_name);
		}catch (...) { 
			data.sitelogo.clear();
			 }
			break;
		case 14:
		 try{
			data.sitebanner=std::to_string(set_value_name);
		}catch (...) { 
			data.sitebanner.clear();
			 }
			break;
		case 15:
		 try{
			data.contactman=std::to_string(set_value_name);
		}catch (...) { 
			data.contactman.clear();
			 }
			break;
		case 16:
		 try{
			data.phone=std::to_string(set_value_name);
		}catch (...) { 
			data.phone.clear();
			 }
			break;
		case 17:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 18:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 19:
		 try{
			data.bankname=std::to_string(set_value_name);
		}catch (...) { 
			data.bankname.clear();
			 }
			break;
		case 20:
		 try{
			data.banksn=std::to_string(set_value_name);
		}catch (...) { 
			data.banksn.clear();
			 }
			break;
		case 21:
		 try{
			data.address=std::to_string(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 22:
		 try{
			data.zipnum=std::to_string(set_value_name);
		}catch (...) { 
			data.zipnum.clear();
			 }
			break;
		case 23:
		 try{
			data.taxsn=std::to_string(set_value_name);
		}catch (...) { 
			data.taxsn.clear();
			 }
			break;
		case 24:
		 try{
			data.companyname=std::to_string(set_value_name);
		}catch (...) { 
			data.companyname.clear();
			 }
			break;
		case 25:
		 try{
			data.linkname=std::to_string(set_value_name);
		}catch (...) { 
			data.linkname.clear();
			 }
			break;
		case 26:
		 try{
			data.linkmobile=std::to_string(set_value_name);
		}catch (...) { 
			data.linkmobile.clear();
			 }
			break;
		case 27:
		 try{
			data.linkaddress=std::to_string(set_value_name);
		}catch (...) { 
			data.linkaddress.clear();
			 }
			break;
		case 28:
		 try{
			data.theme=std::to_string(set_value_name);
		}catch (...) { 
			data.theme.clear();
			 }
			break;
		case 29:
		 try{
			data.sitepath=std::to_string(set_value_name);
		}catch (...) { 
			data.sitepath.clear();
			 }
			break;
		case 30:
		 try{
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 31:
		 try{
			data.created_at=(unsigned int)set_value_name;
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 32:
		 try{
			data.enddate=(unsigned int)set_value_name;
		}catch (...) { 
			data.enddate=0;
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
        for(jj=0;jj<siteinfo_info::col_names.size();jj++){
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
if(record[n].sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(record[n].sid);
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
if(record[n].agentid==0){
	tempsql<<"\"agentid\":0";
 }else{ 
	tempsql<<"\"agentid\":"<<std::to_string(record[n].agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(record[n].languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitename\":\""<<http::utf8_to_jsonstring(record[n].sitename)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitedomain\":\""<<http::utf8_to_jsonstring(record[n].sitedomain)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metakeys\":\""<<http::utf8_to_jsonstring(record[n].metakeys)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metadesc\":\""<<http::utf8_to_jsonstring(record[n].metadesc)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"copyright\":\""<<http::utf8_to_jsonstring(record[n].copyright)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"beiansn\":\""<<http::utf8_to_jsonstring(record[n].beiansn)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"footscript\":\""<<http::utf8_to_jsonstring(record[n].footscript)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"headscript\":\""<<http::utf8_to_jsonstring(record[n].headscript)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitelogo\":\""<<http::utf8_to_jsonstring(record[n].sitelogo)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitebanner\":\""<<http::utf8_to_jsonstring(record[n].sitebanner)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"contactman\":\""<<http::utf8_to_jsonstring(record[n].contactman)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"phone\":\""<<http::utf8_to_jsonstring(record[n].phone)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bankname\":\""<<http::utf8_to_jsonstring(record[n].bankname)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"banksn\":\""<<http::utf8_to_jsonstring(record[n].banksn)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zipnum\":\""<<http::utf8_to_jsonstring(record[n].zipnum)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"taxsn\":\""<<http::utf8_to_jsonstring(record[n].taxsn)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"companyname\":\""<<http::utf8_to_jsonstring(record[n].companyname)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkname\":\""<<http::utf8_to_jsonstring(record[n].linkname)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkmobile\":\""<<http::utf8_to_jsonstring(record[n].linkmobile)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkaddress\":\""<<http::utf8_to_jsonstring(record[n].linkaddress)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"theme\":\""<<http::utf8_to_jsonstring(record[n].theme)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitepath\":\""<<http::utf8_to_jsonstring(record[n].sitepath)<<"\"";
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(record[n].created_at);
}
 break;
 case 32:
 if(jj>0){ tempsql<<","; } 
if(record[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(record[n].enddate);
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
   
   std::string to_json(std::function<bool(std::string&,siteinfo_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<siteinfo_info::col_names.size();jj++){
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
if(record[n].sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(record[n].sid);
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
if(record[n].agentid==0){
	tempsql<<"\"agentid\":0";
 }else{ 
	tempsql<<"\"agentid\":"<<std::to_string(record[n].agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(record[n].languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitename\":\""<<http::utf8_to_jsonstring(record[n].sitename)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitedomain\":\""<<http::utf8_to_jsonstring(record[n].sitedomain)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metakeys\":\""<<http::utf8_to_jsonstring(record[n].metakeys)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metadesc\":\""<<http::utf8_to_jsonstring(record[n].metadesc)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"copyright\":\""<<http::utf8_to_jsonstring(record[n].copyright)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"beiansn\":\""<<http::utf8_to_jsonstring(record[n].beiansn)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"footscript\":\""<<http::utf8_to_jsonstring(record[n].footscript)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"headscript\":\""<<http::utf8_to_jsonstring(record[n].headscript)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitelogo\":\""<<http::utf8_to_jsonstring(record[n].sitelogo)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitebanner\":\""<<http::utf8_to_jsonstring(record[n].sitebanner)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"contactman\":\""<<http::utf8_to_jsonstring(record[n].contactman)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"phone\":\""<<http::utf8_to_jsonstring(record[n].phone)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bankname\":\""<<http::utf8_to_jsonstring(record[n].bankname)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"banksn\":\""<<http::utf8_to_jsonstring(record[n].banksn)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zipnum\":\""<<http::utf8_to_jsonstring(record[n].zipnum)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"taxsn\":\""<<http::utf8_to_jsonstring(record[n].taxsn)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"companyname\":\""<<http::utf8_to_jsonstring(record[n].companyname)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkname\":\""<<http::utf8_to_jsonstring(record[n].linkname)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkmobile\":\""<<http::utf8_to_jsonstring(record[n].linkmobile)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkaddress\":\""<<http::utf8_to_jsonstring(record[n].linkaddress)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"theme\":\""<<http::utf8_to_jsonstring(record[n].theme)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitepath\":\""<<http::utf8_to_jsonstring(record[n].sitepath)<<"\"";
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(record[n].created_at);
}
 break;
 case 32:
 if(jj>0){ tempsql<<","; } 
if(record[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(record[n].enddate);
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
   long long getPK(){  return data.sid; } 
 void setPK(long long val){  data.sid=val;} 
 int  getSid(){  return data.sid; } 
 void setSid( int  val){  data.sid=val;} 

 int  getUserid(){  return data.userid; } 
 void setUserid( int  val){  data.userid=val;} 

 unsigned  int  getAgentid(){  return data.agentid; } 
 void setAgentid( unsigned  int  val){  data.agentid=val;} 

 unsigned  int  getLanguagetype(){  return data.languagetype; } 
 void setLanguagetype( unsigned  int  val){  data.languagetype=val;} 

 std::string  getSitename(){  return data.sitename; } 
 std::string & getRefSitename(){  return std::ref(data.sitename); } 
 void setSitename( std::string  &val){  data.sitename=val;} 
 void setSitename(std::string_view val){  data.sitename=val;} 

 std::string  getSitedomain(){  return data.sitedomain; } 
 std::string & getRefSitedomain(){  return std::ref(data.sitedomain); } 
 void setSitedomain( std::string  &val){  data.sitedomain=val;} 
 void setSitedomain(std::string_view val){  data.sitedomain=val;} 

 std::string  getMetakeys(){  return data.metakeys; } 
 std::string & getRefMetakeys(){  return std::ref(data.metakeys); } 
 void setMetakeys( std::string  &val){  data.metakeys=val;} 
 void setMetakeys(std::string_view val){  data.metakeys=val;} 

 std::string  getMetadesc(){  return data.metadesc; } 
 std::string & getRefMetadesc(){  return std::ref(data.metadesc); } 
 void setMetadesc( std::string  &val){  data.metadesc=val;} 
 void setMetadesc(std::string_view val){  data.metadesc=val;} 

 std::string  getCopyright(){  return data.copyright; } 
 std::string & getRefCopyright(){  return std::ref(data.copyright); } 
 void setCopyright( std::string  &val){  data.copyright=val;} 
 void setCopyright(std::string_view val){  data.copyright=val;} 

 std::string  getBeiansn(){  return data.beiansn; } 
 std::string & getRefBeiansn(){  return std::ref(data.beiansn); } 
 void setBeiansn( std::string  &val){  data.beiansn=val;} 
 void setBeiansn(std::string_view val){  data.beiansn=val;} 

 std::string  getFootscript(){  return data.footscript; } 
 std::string & getRefFootscript(){  return std::ref(data.footscript); } 
 void setFootscript( std::string  &val){  data.footscript=val;} 
 void setFootscript(std::string_view val){  data.footscript=val;} 

 std::string  getHeadscript(){  return data.headscript; } 
 std::string & getRefHeadscript(){  return std::ref(data.headscript); } 
 void setHeadscript( std::string  &val){  data.headscript=val;} 
 void setHeadscript(std::string_view val){  data.headscript=val;} 

 std::string  getIntroduce(){  return data.introduce; } 
 std::string & getRefIntroduce(){  return std::ref(data.introduce); } 
 void setIntroduce( std::string  &val){  data.introduce=val;} 
 void setIntroduce(std::string_view val){  data.introduce=val;} 

 std::string  getSitelogo(){  return data.sitelogo; } 
 std::string & getRefSitelogo(){  return std::ref(data.sitelogo); } 
 void setSitelogo( std::string  &val){  data.sitelogo=val;} 
 void setSitelogo(std::string_view val){  data.sitelogo=val;} 

 std::string  getSitebanner(){  return data.sitebanner; } 
 std::string & getRefSitebanner(){  return std::ref(data.sitebanner); } 
 void setSitebanner( std::string  &val){  data.sitebanner=val;} 
 void setSitebanner(std::string_view val){  data.sitebanner=val;} 

 std::string  getContactman(){  return data.contactman; } 
 std::string & getRefContactman(){  return std::ref(data.contactman); } 
 void setContactman( std::string  &val){  data.contactman=val;} 
 void setContactman(std::string_view val){  data.contactman=val;} 

 std::string  getPhone(){  return data.phone; } 
 std::string & getRefPhone(){  return std::ref(data.phone); } 
 void setPhone( std::string  &val){  data.phone=val;} 
 void setPhone(std::string_view val){  data.phone=val;} 

 std::string  getMobile(){  return data.mobile; } 
 std::string & getRefMobile(){  return std::ref(data.mobile); } 
 void setMobile( std::string  &val){  data.mobile=val;} 
 void setMobile(std::string_view val){  data.mobile=val;} 

 std::string  getEmail(){  return data.email; } 
 std::string & getRefEmail(){  return std::ref(data.email); } 
 void setEmail( std::string  &val){  data.email=val;} 
 void setEmail(std::string_view val){  data.email=val;} 

 std::string  getBankname(){  return data.bankname; } 
 std::string & getRefBankname(){  return std::ref(data.bankname); } 
 void setBankname( std::string  &val){  data.bankname=val;} 
 void setBankname(std::string_view val){  data.bankname=val;} 

 std::string  getBanksn(){  return data.banksn; } 
 std::string & getRefBanksn(){  return std::ref(data.banksn); } 
 void setBanksn( std::string  &val){  data.banksn=val;} 
 void setBanksn(std::string_view val){  data.banksn=val;} 

 std::string  getAddress(){  return data.address; } 
 std::string & getRefAddress(){  return std::ref(data.address); } 
 void setAddress( std::string  &val){  data.address=val;} 
 void setAddress(std::string_view val){  data.address=val;} 

 std::string  getZipnum(){  return data.zipnum; } 
 std::string & getRefZipnum(){  return std::ref(data.zipnum); } 
 void setZipnum( std::string  &val){  data.zipnum=val;} 
 void setZipnum(std::string_view val){  data.zipnum=val;} 

 std::string  getTaxsn(){  return data.taxsn; } 
 std::string & getRefTaxsn(){  return std::ref(data.taxsn); } 
 void setTaxsn( std::string  &val){  data.taxsn=val;} 
 void setTaxsn(std::string_view val){  data.taxsn=val;} 

 std::string  getCompanyname(){  return data.companyname; } 
 std::string & getRefCompanyname(){  return std::ref(data.companyname); } 
 void setCompanyname( std::string  &val){  data.companyname=val;} 
 void setCompanyname(std::string_view val){  data.companyname=val;} 

 std::string  getLinkname(){  return data.linkname; } 
 std::string & getRefLinkname(){  return std::ref(data.linkname); } 
 void setLinkname( std::string  &val){  data.linkname=val;} 
 void setLinkname(std::string_view val){  data.linkname=val;} 

 std::string  getLinkmobile(){  return data.linkmobile; } 
 std::string & getRefLinkmobile(){  return std::ref(data.linkmobile); } 
 void setLinkmobile( std::string  &val){  data.linkmobile=val;} 
 void setLinkmobile(std::string_view val){  data.linkmobile=val;} 

 std::string  getLinkaddress(){  return data.linkaddress; } 
 std::string & getRefLinkaddress(){  return std::ref(data.linkaddress); } 
 void setLinkaddress( std::string  &val){  data.linkaddress=val;} 
 void setLinkaddress(std::string_view val){  data.linkaddress=val;} 

 std::string  getTheme(){  return data.theme; } 
 std::string & getRefTheme(){  return std::ref(data.theme); } 
 void setTheme( std::string  &val){  data.theme=val;} 
 void setTheme(std::string_view val){  data.theme=val;} 

 std::string  getSitepath(){  return data.sitepath; } 
 std::string & getRefSitepath(){  return std::ref(data.sitepath); } 
 void setSitepath( std::string  &val){  data.sitepath=val;} 
 void setSitepath(std::string_view val){  data.sitepath=val;} 

 unsigned  char  getIsopen(){  return data.isopen; } 
 void setIsopen( unsigned  char  val){  data.isopen=val;} 

 unsigned  int  getCreatedAt(){  return data.created_at; } 
 void setCreatedAt( unsigned  int  val){  data.created_at=val;} 

 unsigned  int  getEnddate(){  return data.enddate; } 
 void setEnddate( unsigned  int  val){  data.enddate=val;} 

siteinfo_info::meta getnewData(){
 	 struct siteinfo_info::meta newdata;
	 return newdata; 
} 
siteinfo_info::meta getData(){
 	 return data; 
} 
std::vector<siteinfo_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<siteinfo_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<siteinfo_info::col_names.size();jj++){
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
if(tree_data[n].sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(tree_data[n].sid);
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
if(tree_data[n].agentid==0){
	tempsql<<"\"agentid\":0";
 }else{ 
	tempsql<<"\"agentid\":"<<std::to_string(tree_data[n].agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(tree_data[n].languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitename\":\""<<http::utf8_to_jsonstring(tree_data[n].sitename)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitedomain\":\""<<http::utf8_to_jsonstring(tree_data[n].sitedomain)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metakeys\":\""<<http::utf8_to_jsonstring(tree_data[n].metakeys)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metadesc\":\""<<http::utf8_to_jsonstring(tree_data[n].metadesc)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"copyright\":\""<<http::utf8_to_jsonstring(tree_data[n].copyright)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"beiansn\":\""<<http::utf8_to_jsonstring(tree_data[n].beiansn)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"footscript\":\""<<http::utf8_to_jsonstring(tree_data[n].footscript)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"headscript\":\""<<http::utf8_to_jsonstring(tree_data[n].headscript)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitelogo\":\""<<http::utf8_to_jsonstring(tree_data[n].sitelogo)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitebanner\":\""<<http::utf8_to_jsonstring(tree_data[n].sitebanner)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"contactman\":\""<<http::utf8_to_jsonstring(tree_data[n].contactman)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"phone\":\""<<http::utf8_to_jsonstring(tree_data[n].phone)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(tree_data[n].mobile)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(tree_data[n].email)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bankname\":\""<<http::utf8_to_jsonstring(tree_data[n].bankname)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"banksn\":\""<<http::utf8_to_jsonstring(tree_data[n].banksn)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(tree_data[n].address)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zipnum\":\""<<http::utf8_to_jsonstring(tree_data[n].zipnum)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"taxsn\":\""<<http::utf8_to_jsonstring(tree_data[n].taxsn)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"companyname\":\""<<http::utf8_to_jsonstring(tree_data[n].companyname)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkname\":\""<<http::utf8_to_jsonstring(tree_data[n].linkname)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkmobile\":\""<<http::utf8_to_jsonstring(tree_data[n].linkmobile)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkaddress\":\""<<http::utf8_to_jsonstring(tree_data[n].linkaddress)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"theme\":\""<<http::utf8_to_jsonstring(tree_data[n].theme)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitepath\":\""<<http::utf8_to_jsonstring(tree_data[n].sitepath)<<"\"";
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(tree_data[n].created_at);
}
 break;
 case 32:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(tree_data[n].enddate);
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
   
   std::string tree_tojson(const std::vector<siteinfo_info::meta_tree> &tree_data,std::function<bool(std::string&,const siteinfo_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<siteinfo_info::col_names.size();jj++){
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
if(tree_data[n].sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(tree_data[n].sid);
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
if(tree_data[n].agentid==0){
	tempsql<<"\"agentid\":0";
 }else{ 
	tempsql<<"\"agentid\":"<<std::to_string(tree_data[n].agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(tree_data[n].languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitename\":\""<<http::utf8_to_jsonstring(tree_data[n].sitename)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitedomain\":\""<<http::utf8_to_jsonstring(tree_data[n].sitedomain)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metakeys\":\""<<http::utf8_to_jsonstring(tree_data[n].metakeys)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metadesc\":\""<<http::utf8_to_jsonstring(tree_data[n].metadesc)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"copyright\":\""<<http::utf8_to_jsonstring(tree_data[n].copyright)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"beiansn\":\""<<http::utf8_to_jsonstring(tree_data[n].beiansn)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"footscript\":\""<<http::utf8_to_jsonstring(tree_data[n].footscript)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"headscript\":\""<<http::utf8_to_jsonstring(tree_data[n].headscript)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitelogo\":\""<<http::utf8_to_jsonstring(tree_data[n].sitelogo)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitebanner\":\""<<http::utf8_to_jsonstring(tree_data[n].sitebanner)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"contactman\":\""<<http::utf8_to_jsonstring(tree_data[n].contactman)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"phone\":\""<<http::utf8_to_jsonstring(tree_data[n].phone)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(tree_data[n].mobile)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(tree_data[n].email)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bankname\":\""<<http::utf8_to_jsonstring(tree_data[n].bankname)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"banksn\":\""<<http::utf8_to_jsonstring(tree_data[n].banksn)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(tree_data[n].address)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zipnum\":\""<<http::utf8_to_jsonstring(tree_data[n].zipnum)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"taxsn\":\""<<http::utf8_to_jsonstring(tree_data[n].taxsn)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"companyname\":\""<<http::utf8_to_jsonstring(tree_data[n].companyname)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkname\":\""<<http::utf8_to_jsonstring(tree_data[n].linkname)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkmobile\":\""<<http::utf8_to_jsonstring(tree_data[n].linkmobile)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkaddress\":\""<<http::utf8_to_jsonstring(tree_data[n].linkaddress)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"theme\":\""<<http::utf8_to_jsonstring(tree_data[n].theme)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitepath\":\""<<http::utf8_to_jsonstring(tree_data[n].sitepath)<<"\"";
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(tree_data[n].created_at);
}
 break;
 case 32:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(tree_data[n].enddate);
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
   
    template<siteinfo_info::cols KeyCol, siteinfo_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>()));
        using ValType = decltype(siteinfo_info::getField<ValCol>(std::declval<const siteinfo_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(siteinfo_info::getField<KeyCol>(iter), siteinfo_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<siteinfo_info::cols KeyCol, siteinfo_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>())), 
            decltype(siteinfo_info::getField<ValCol>(std::declval<const siteinfo_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>())), 
                    decltype(siteinfo_info::getField<ValCol>(std::declval<const siteinfo_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>()));
        using ValType = decltype(siteinfo_info::getField<ValCol>(std::declval<const siteinfo_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(siteinfo_info::getField<KeyCol>(iter), siteinfo_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(siteinfo_info::getField<KeyCol>(iter), siteinfo_info::getField<ValCol>(iter))) {
                    result.emplace(siteinfo_info::getField<KeyCol>(iter), siteinfo_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<siteinfo_info::cols KeyCol, siteinfo_info::cols ValCol> 
    auto get_vec_cols()
    {
        using KeyType = decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>()));
        using ValType = decltype(siteinfo_info::getField<ValCol>(std::declval<const siteinfo_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(siteinfo_info::getField<KeyCol>(iter), siteinfo_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<siteinfo_info::cols KeyCol, siteinfo_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>())), 
            decltype(siteinfo_info::getField<ValCol>(std::declval<const siteinfo_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>())), 
                    decltype(siteinfo_info::getField<ValCol>(std::declval<const siteinfo_info::meta&>()))>, bool>
    auto get_vec_cols(Callback&& callback)
    {
        using KeyType = decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>()));
        using ValType = decltype(siteinfo_info::getField<ValCol>(std::declval<const siteinfo_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(siteinfo_info::getField<KeyCol>(iter), siteinfo_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(siteinfo_info::getField<KeyCol>(iter), siteinfo_info::getField<ValCol>(iter))) {
                    result.emplace_back(siteinfo_info::getField<KeyCol>(iter), siteinfo_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<siteinfo_info::cols KeyCol>
    auto get_vec_col()
    {
        using KeyType = decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(siteinfo_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_col<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<siteinfo_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>()))>, bool>
    auto get_vec_col(Callback&& callback)
    {
        using KeyType = decltype(siteinfo_info::getField<KeyCol>(std::declval<const siteinfo_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(siteinfo_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(siteinfo_info::getField<KeyCol>(iter))) {
                    result.emplace_back(siteinfo_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<siteinfo_info::cols Col>
        requires requires(std::ostream& os, decltype(siteinfo_info::getField<Col>(std::declval<const siteinfo_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << siteinfo_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<siteinfo_info::cols Col>
        requires requires(std::ostream& os, decltype(siteinfo_info::getField<Col>(std::declval<const siteinfo_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << siteinfo_info::getField<Col>(iter); 
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
   