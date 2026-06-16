#ifndef ORM_CMS_PRODUCTBASEMATA_H
#define ORM_CMS_PRODUCTBASEMATA_H
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

namespace product_info
{
 
    enum class cols : unsigned char 
    {
		pid = 0,
		userid = 1,
		topicid = 2,
		bigid = 3,
		smallid = 4,
		brandid = 5,
		isview = 6,
		isstore = 7,
		ishome = 8,
		showtype = 9,
		sntype = 10,
		name = 11,
		keywords = 12,
		introduce = 13,
		listimg = 14,
		bigimg = 15,
		maincontent = 16,
		paracontent = 17,
		samepro = 18,
		attatchfiles = 19,
		price = 20,
		sortid = 21,
		adddate = 22,
		editdate = 23,

    };

    struct meta
    {
		 unsigned  int  pid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  topicid = 0; ///*栏目id*/
		 unsigned  int  bigid = 0; ///*大类*/
		 unsigned  int  smallid = 0; ///*小类*/
		 unsigned  int  brandid = 0; ///*品牌id*/
		 char  isview = 0; ///*是否显示*/
		 char  isstore = 0; ///*精品*/
		 unsigned  char  ishome = 0; ///*是否首页*/
		 char  showtype = 0; ///*page layout*/
		 std::string  sntype = ""; ///*型号*/
		 std::string  name = ""; ///*产品名称*/
		 std::string  keywords = ""; ///**/
		 std::string  introduce = ""; ///*介绍*/
		 std::string  listimg = ""; ///*列表图片*/
		 std::string  bigimg = ""; ///*主图*/
		 std::string  maincontent = ""; ///*产品主内容*/
		 std::string  paracontent = ""; ///*参数*/
		 std::string  samepro = ""; ///*类似*/
		 std::string  attatchfiles = ""; ///*附件*/
		 unsigned  int  price = 0; ///*价格*/
		 unsigned  int  sortid = 0; ///**/
		 std::string  adddate = ""; ///*产品添加日期*/
		 std::string  editdate = ""; ///*产品修改时间*/
	};
  
    struct meta_tree
    {
		 unsigned  int  pid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  topicid = 0; ///*栏目id*/
		 unsigned  int  bigid = 0; ///*大类*/
		 unsigned  int  smallid = 0; ///*小类*/
		 unsigned  int  brandid = 0; ///*品牌id*/
		 char  isview = 0; ///*是否显示*/
		 char  isstore = 0; ///*精品*/
		 unsigned  char  ishome = 0; ///*是否首页*/
		 char  showtype = 0; ///*page layout*/
		 std::string  sntype = ""; ///*型号*/
		 std::string  name = ""; ///*产品名称*/
		 std::string  keywords = ""; ///**/
		 std::string  introduce = ""; ///*介绍*/
		 std::string  listimg = ""; ///*列表图片*/
		 std::string  bigimg = ""; ///*主图*/
		 std::string  maincontent = ""; ///*产品主内容*/
		 std::string  paracontent = ""; ///*参数*/
		 std::string  samepro = ""; ///*类似*/
		 std::string  attatchfiles = ""; ///*附件*/
		 unsigned  int  price = 0; ///*价格*/
		 unsigned  int  sortid = 0; ///**/
		 std::string  adddate = ""; ///*产品添加日期*/
		 std::string  editdate = ""; ///*产品修改时间*/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  pid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 unsigned  int  topicid = 0; ///*栏目id*/
		 unsigned  int  bigid = 0; ///*大类*/
		 unsigned  int  smallid = 0; ///*小类*/
		 unsigned  int  brandid = 0; ///*品牌id*/
		 char  isview = 0; ///*是否显示*/
		 char  isstore = 0; ///*精品*/
		 unsigned  char  ishome = 0; ///*是否首页*/
		 char  showtype = 0; ///*page layout*/
		 std::string  sntype = ""; ///*型号*/
		 std::string  name = ""; ///*产品名称*/
		 std::string  keywords = ""; ///**/
		 std::string  introduce = ""; ///*介绍*/
		 std::string  listimg = ""; ///*列表图片*/
		 std::string  bigimg = ""; ///*主图*/
		 std::string  maincontent = ""; ///*产品主内容*/
		 std::string  paracontent = ""; ///*参数*/
		 std::string  samepro = ""; ///*类似*/
		 std::string  attatchfiles = ""; ///*附件*/
		 unsigned  int  price = 0; ///*价格*/
		 unsigned  int  sortid = 0; ///**/
		 std::string  adddate = ""; ///*产品添加日期*/
		 std::string  editdate = ""; ///*产品修改时间*/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::pid) { 
		 return m.pid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::topicid) { 
		 return m.topicid;
		} else if constexpr (Col == cols::bigid) { 
		 return m.bigid;
		} else if constexpr (Col == cols::smallid) { 
		 return m.smallid;
		} else if constexpr (Col == cols::brandid) { 
		 return m.brandid;
		} else if constexpr (Col == cols::isview) { 
		 return m.isview;
		} else if constexpr (Col == cols::isstore) { 
		 return m.isstore;
		} else if constexpr (Col == cols::ishome) { 
		 return m.ishome;
		} else if constexpr (Col == cols::showtype) { 
		 return m.showtype;
		} else if constexpr (Col == cols::sntype) { 
		 return m.sntype;
		} else if constexpr (Col == cols::name) { 
		 return m.name;
		} else if constexpr (Col == cols::keywords) { 
		 return m.keywords;
		} else if constexpr (Col == cols::introduce) { 
		 return m.introduce;
		} else if constexpr (Col == cols::listimg) { 
		 return m.listimg;
		} else if constexpr (Col == cols::bigimg) { 
		 return m.bigimg;
		} else if constexpr (Col == cols::maincontent) { 
		 return m.maincontent;
		} else if constexpr (Col == cols::paracontent) { 
		 return m.paracontent;
		} else if constexpr (Col == cols::samepro) { 
		 return m.samepro;
		} else if constexpr (Col == cols::attatchfiles) { 
		 return m.attatchfiles;
		} else if constexpr (Col == cols::price) { 
		 return m.price;
		} else if constexpr (Col == cols::sortid) { 
		 return m.sortid;
		} else if constexpr (Col == cols::adddate) { 
		 return m.adddate;
		} else if constexpr (Col == cols::editdate) { 
		 return m.editdate;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using pid =  unsigned  int ;
		using userid =  unsigned  int ;
		using topicid =  unsigned  int ;
		using bigid =  unsigned  int ;
		using smallid =  unsigned  int ;
		using brandid =  unsigned  int ;
		using isview =  char ;
		using isstore =  char ;
		using ishome =  unsigned  char ;
		using showtype =  char ;
		using sntype =  std::string ;
		using name =  std::string ;
		using keywords =  std::string ;
		using introduce =  std::string ;
		using listimg =  std::string ;
		using bigimg =  std::string ;
		using maincontent =  std::string ;
		using paracontent =  std::string ;
		using samepro =  std::string ;
		using attatchfiles =  std::string ;
		using price =  unsigned  int ;
		using sortid =  unsigned  int ;
		using adddate =  std::string ;
		using editdate =  std::string ;

    }

    
    #define ORM_CMS_PRODUCT_EXPAND(x) x 
    
    #define ORM_CMS_PRODUCT_META_FIELD_TYPE(col) \
         orm::cms::product_info::type::col 
    
    #define ORM_CMS_PRODUCT_PROJ_MEMBER(col) \
          ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_1(c1) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_1( c1)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c2))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c3))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c4))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c5))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c6))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c7))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c8))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c9))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c10))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c11))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c12))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c13))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c14))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c15))
         
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBER(c16))
         
    #define ORM_CMS_PRODUCT_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_PRODUCT_PROJ_MEMBERS(...) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_16, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_15, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_14, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_13, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_12, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_11, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_10, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_9, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_8, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_7, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_6, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_5, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_4, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_3, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_2, \
            ORM_CMS_PRODUCT_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_PRODUCT_COUNT(...) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_PRODUCT_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_PRODUCT_TO_JSON_1(c1) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_PRODUCT_TO_JSON_2(c1,c2) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_PRODUCT_TO_JSON_BODY(...) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_PRODUCT_TO_JSON_16,ORM_CMS_PRODUCT_TO_JSON_15,ORM_CMS_PRODUCT_TO_JSON_14,ORM_CMS_PRODUCT_TO_JSON_13,ORM_CMS_PRODUCT_TO_JSON_12,ORM_CMS_PRODUCT_TO_JSON_11,ORM_CMS_PRODUCT_TO_JSON_10,ORM_CMS_PRODUCT_TO_JSON_9,ORM_CMS_PRODUCT_TO_JSON_8,ORM_CMS_PRODUCT_TO_JSON_7,ORM_CMS_PRODUCT_TO_JSON_6,ORM_CMS_PRODUCT_TO_JSON_5,ORM_CMS_PRODUCT_TO_JSON_4,ORM_CMS_PRODUCT_TO_JSON_3,ORM_CMS_PRODUCT_TO_JSON_2,ORM_CMS_PRODUCT_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_PRODUCT_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_1(n1)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_1(n1)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_PRODUCT_CAT(a, b) ORM_CMS_PRODUCT_CAT_(a, b)
    #define ORM_CMS_PRODUCT_CAT_(a, b) a##b

    #define ORM_CMS_PRODUCT_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_PRODUCT_CAT(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_PRODUCT_TO_JSON_CUSTOM(...) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_PRODUCT_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_PRODUCT_SET_VAL_1(c1) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_PRODUCT_SET_VAL_2(c1,c2) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_1(c1)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_2(c1,c2)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_PRODUCT_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_PRODUCT_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_PRODUCT_CAT(ORM_CMS_PRODUCT_SET_VAL_, N)
    
    
    #define ORM_CMS_PRODUCT_SET_VAL_FIELDS(...) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_PRODUCT_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_FIELDS(ORM_CMS_PRODUCT_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_PRODUCT_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::product_info { \
            struct StructName { \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_PRODUCT_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::product_info { \
            struct StructName { \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM(ORM_CMS_PRODUCT_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_PRODUCT_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::product_info { \
            struct StructName { \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_PRODUCT_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::product_info { \
            struct StructName { \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_PRODUCT_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::product_info { \
            struct StructName { \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_TO_JSON_CUSTOM(ORM_CMS_PRODUCT_UNWRAP CustomNames));  \
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
                        ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_PRODUCT_EXPAND(ORM_CMS_PRODUCT_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,24> col_names={"pid","userid","topicid","bigid","smallid","brandid","isview","isstore","ishome","showtype","sntype","name","keywords","introduce","listimg","bigimg","maincontent","paracontent","samepro","attatchfiles","price","sortid","adddate","editdate"};
	static constexpr std::array<unsigned char,24> col_types={3,3,3,3,3,3,1,1,1,1,253,253,253,252,252,253,252,252,253,252,3,3,253,253};
	static constexpr std::array<unsigned char,24> col_length={0,0,0,0,0,0,0,0,0,0,128,253,0,0,0,254,0,0,254,0,0,0,20,20};
	static constexpr std::array<unsigned char,24> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct product_base
{
      product_info::meta data;
    std::vector<product_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<product_info::meta>::iterator begin(){     return record.begin(); }
std::vector<product_info::meta>::iterator end(){     return record.end(); }
std::vector<product_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<product_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="product";
static constexpr std::string_view org_tablename="product";
static constexpr std::string_view modelname="Product";

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
   	 return 22;
break;
case 12:
   	 return 19;
break;
 }
 break;
case 'b':
 switch(coln.size()){  
case 5:
   	 return 3;
break;
case 6:
   	 return 15;
break;
case 7:
   	 return 5;
break;
 }
 break;
case 'e':
   	 return 23;
break;
case 'i':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 8; }
 if(colpospppc=='w'){ return 6; }
   	 break;
case 7:
   	 return 7;
break;
case 9:
   	 return 13;
break;
 }
 break;
case 'k':
   	 return 12;
break;
case 'l':
   	 return 14;
break;
case 'm':
   	 return 16;
break;
case 'n':
   	 return 11;
break;
case 'p':
 switch(coln.size()){  
case 3:
   	 return 0;
break;
case 5:
   	 return 20;
break;
case 11:
   	 return 17;
break;
 }
 break;
case 's':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 21; }
 if(colpospppc=='e'){ return 10; }
   	 break;
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 4; }
 if(colpospppc=='o'){ return 18; }
   	 break;
case 8:
   	 return 9;
break;
 }
 break;
case 't':
   	 return 2;
break;
case 'u':
   	 return 1;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "pid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     product_info::meta metatemp;    
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
        for(;j<product_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<product_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.pid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.pid);
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
if(data.bigid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.bigid);
}
if(data.smallid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.smallid);
}
if(data.brandid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.brandid);
}
if(data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isview);
}
if(data.isstore==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isstore);
}
if(data.ishome==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.ishome);
}
if(data.showtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.showtype);
}
tempsql<<",'"<<stringaddslash(data.sntype)<<"'";
tempsql<<",'"<<stringaddslash(data.name)<<"'";
tempsql<<",'"<<stringaddslash(data.keywords)<<"'";
tempsql<<",'"<<stringaddslash(data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(data.listimg)<<"'";
tempsql<<",'"<<stringaddslash(data.bigimg)<<"'";
tempsql<<",'"<<stringaddslash(data.maincontent)<<"'";
tempsql<<",'"<<stringaddslash(data.paracontent)<<"'";
tempsql<<",'"<<stringaddslash(data.samepro)<<"'";
tempsql<<",'"<<stringaddslash(data.attatchfiles)<<"'";
if(data.price==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.price);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
tempsql<<",'"<<stringaddslash(data.adddate)<<"'";
tempsql<<",'"<<stringaddslash(data.editdate)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const product_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<product_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<product_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.pid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.pid);
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
if(insert_data.bigid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.bigid);
}
if(insert_data.smallid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.smallid);
}
if(insert_data.brandid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.brandid);
}
if(insert_data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isview);
}
if(insert_data.isstore==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isstore);
}
if(insert_data.ishome==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.ishome);
}
if(insert_data.showtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.showtype);
}
tempsql<<",'"<<stringaddslash(insert_data.sntype)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.keywords)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.listimg)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.bigimg)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.maincontent)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.paracontent)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.samepro)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.attatchfiles)<<"'";
if(insert_data.price==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.price);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
tempsql<<",'"<<stringaddslash(insert_data.adddate)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.editdate)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<product_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<product_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<product_info::col_names[j];
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


            	if(insert_data[i].pid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].pid);
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
	if(insert_data[i].bigid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].bigid);
	}
	if(insert_data[i].smallid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].smallid);
	}
	if(insert_data[i].brandid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].brandid);
	}
	if(insert_data[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isview);
	}
	if(insert_data[i].isstore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isstore);
	}
	if(insert_data[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].ishome);
	}
	if(insert_data[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].showtype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].sntype)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].keywords)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].introduce)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].listimg)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].bigimg)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].maincontent)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].paracontent)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].samepro)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].attatchfiles)<<"'";
	if(insert_data[i].price==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].price);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].adddate)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].editdate)<<"'";
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

        if(data.pid==0){
	tempsql<<"`pid`=0";
 }else{ 
	tempsql<<"`pid`="<<std::to_string(data.pid);
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
if(data.bigid==0){
	tempsql<<",`bigid`=0";
 }else{ 
	tempsql<<",`bigid`="<<std::to_string(data.bigid);
}
if(data.smallid==0){
	tempsql<<",`smallid`=0";
 }else{ 
	tempsql<<",`smallid`="<<std::to_string(data.smallid);
}
if(data.brandid==0){
	tempsql<<",`brandid`=0";
 }else{ 
	tempsql<<",`brandid`="<<std::to_string(data.brandid);
}
if(data.isview==0){
	tempsql<<",`isview`=0";
 }else{ 
	tempsql<<",`isview`="<<std::to_string(data.isview);
}
if(data.isstore==0){
	tempsql<<",`isstore`=0";
 }else{ 
	tempsql<<",`isstore`="<<std::to_string(data.isstore);
}
if(data.ishome==0){
	tempsql<<",`ishome`=0";
 }else{ 
	tempsql<<",`ishome`="<<std::to_string(data.ishome);
}
if(data.showtype==0){
	tempsql<<",`showtype`=0";
 }else{ 
	tempsql<<",`showtype`="<<std::to_string(data.showtype);
}
tempsql<<",`sntype`='"<<stringaddslash(data.sntype)<<"'";
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
tempsql<<",`keywords`='"<<stringaddslash(data.keywords)<<"'";
tempsql<<",`introduce`='"<<stringaddslash(data.introduce)<<"'";
tempsql<<",`listimg`='"<<stringaddslash(data.listimg)<<"'";
tempsql<<",`bigimg`='"<<stringaddslash(data.bigimg)<<"'";
tempsql<<",`maincontent`='"<<stringaddslash(data.maincontent)<<"'";
tempsql<<",`paracontent`='"<<stringaddslash(data.paracontent)<<"'";
tempsql<<",`samepro`='"<<stringaddslash(data.samepro)<<"'";
tempsql<<",`attatchfiles`='"<<stringaddslash(data.attatchfiles)<<"'";
if(data.price==0){
	tempsql<<",`price`=0";
 }else{ 
	tempsql<<",`price`="<<std::to_string(data.price);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
tempsql<<",`adddate`='"<<stringaddslash(data.adddate)<<"'";
tempsql<<",`editdate`='"<<stringaddslash(data.editdate)<<"'";
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
if(data.pid==0){
	tempsql<<"`pid`=0";
 }else{ 
	tempsql<<"`pid`="<<std::to_string(data.pid);
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
if(data.bigid==0){
	tempsql<<"`bigid`=0";
 }else{ 
	tempsql<<"`bigid`="<<std::to_string(data.bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.smallid==0){
	tempsql<<"`smallid`=0";
 }else{ 
	tempsql<<"`smallid`="<<std::to_string(data.smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.brandid==0){
	tempsql<<"`brandid`=0";
 }else{ 
	tempsql<<"`brandid`="<<std::to_string(data.brandid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isview==0){
	tempsql<<"`isview`=0";
 }else{ 
	tempsql<<"`isview`="<<std::to_string(data.isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.isstore==0){
	tempsql<<"`isstore`=0";
 }else{ 
	tempsql<<"`isstore`="<<std::to_string(data.isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.ishome==0){
	tempsql<<"`ishome`=0";
 }else{ 
	tempsql<<"`ishome`="<<std::to_string(data.ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.showtype==0){
	tempsql<<"`showtype`=0";
 }else{ 
	tempsql<<"`showtype`="<<std::to_string(data.showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sntype`='"<<stringaddslash(data.sntype)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`keywords`='"<<stringaddslash(data.keywords)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`introduce`='"<<stringaddslash(data.introduce)<<"'";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`listimg`='"<<stringaddslash(data.listimg)<<"'";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`bigimg`='"<<stringaddslash(data.bigimg)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`maincontent`='"<<stringaddslash(data.maincontent)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`paracontent`='"<<stringaddslash(data.paracontent)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`samepro`='"<<stringaddslash(data.samepro)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`attatchfiles`='"<<stringaddslash(data.attatchfiles)<<"'";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.price==0){
	tempsql<<"`price`=0";
 }else{ 
	tempsql<<"`price`="<<std::to_string(data.price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`adddate`='"<<stringaddslash(data.adddate)<<"'";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`editdate`='"<<stringaddslash(data.editdate)<<"'";
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
        for (; j < product_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << product_info::col_names[j];
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
            	if(record[i].pid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].pid);
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
	if(record[i].bigid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].bigid);
	}
	if(record[i].smallid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].smallid);
	}
	if(record[i].brandid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].brandid);
	}
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].isstore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isstore);
	}
	if(record[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ishome);
	}
	if(record[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].showtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sntype)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].keywords)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].listimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].bigimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].maincontent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].paracontent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].samepro)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].attatchfiles)<<"'";
	if(record[i].price==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].price);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].adddate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].editdate)<<"'";
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
        for (; j < product_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << product_info::col_names[j];
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
            	if(record[i].pid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].pid);
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
	if(record[i].bigid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].bigid);
	}
	if(record[i].smallid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].smallid);
	}
	if(record[i].brandid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].brandid);
	}
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].isstore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isstore);
	}
	if(record[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ishome);
	}
	if(record[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].showtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sntype)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].keywords)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].listimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].bigimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].maincontent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].paracontent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].samepro)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].attatchfiles)<<"'";
	if(record[i].price==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].price);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].adddate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].editdate)<<"'";
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
            for(jj=0;jj<product_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.pid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.pid));
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
if(data.bigid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.bigid));
}
 break;
 case 4:
if(data.smallid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.smallid));
}
 break;
 case 5:
if(data.brandid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.brandid));
}
 break;
 case 6:
if(data.isview==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isview));
}
 break;
 case 7:
if(data.isstore==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isstore));
}
 break;
 case 8:
if(data.ishome==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.ishome));
}
 break;
 case 9:
if(data.showtype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.showtype));
}
 break;
 case 10:
	temparray.push_back(data.sntype);
 break;
 case 11:
	temparray.push_back(data.name);
 break;
 case 12:
	temparray.push_back(data.keywords);
 break;
 case 13:
	temparray.push_back(data.introduce);
 break;
 case 14:
	temparray.push_back(data.listimg);
 break;
 case 15:
	temparray.push_back(data.bigimg);
 break;
 case 16:
	temparray.push_back(data.maincontent);
 break;
 case 17:
	temparray.push_back(data.paracontent);
 break;
 case 18:
	temparray.push_back(data.samepro);
 break;
 case 19:
	temparray.push_back(data.attatchfiles);
 break;
 case 20:
if(data.price==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.price));
}
 break;
 case 21:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 22:
	temparray.push_back(data.adddate);
 break;
 case 23:
	temparray.push_back(data.editdate);
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
            for(jj=0;jj<product_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.pid==0){
	tempsql.insert({"pid","0"});
 }else{ 
	tempsql.insert({"pid",std::to_string(data.pid)});
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
if(data.bigid==0){
	tempsql.insert({"bigid","0"});
 }else{ 
	tempsql.insert({"bigid",std::to_string(data.bigid)});
}
 break;
 case 4:
if(data.smallid==0){
	tempsql.insert({"smallid","0"});
 }else{ 
	tempsql.insert({"smallid",std::to_string(data.smallid)});
}
 break;
 case 5:
if(data.brandid==0){
	tempsql.insert({"brandid","0"});
 }else{ 
	tempsql.insert({"brandid",std::to_string(data.brandid)});
}
 break;
 case 6:
if(data.isview==0){
	tempsql.insert({"isview","0"});
 }else{ 
	tempsql.insert({"isview",std::to_string(data.isview)});
}
 break;
 case 7:
if(data.isstore==0){
	tempsql.insert({"isstore","0"});
 }else{ 
	tempsql.insert({"isstore",std::to_string(data.isstore)});
}
 break;
 case 8:
if(data.ishome==0){
	tempsql.insert({"ishome","0"});
 }else{ 
	tempsql.insert({"ishome",std::to_string(data.ishome)});
}
 break;
 case 9:
if(data.showtype==0){
	tempsql.insert({"showtype","0"});
 }else{ 
	tempsql.insert({"showtype",std::to_string(data.showtype)});
}
 break;
 case 10:
	tempsql.insert({"sntype",data.sntype});
 break;
 case 11:
	tempsql.insert({"name",data.name});
 break;
 case 12:
	tempsql.insert({"keywords",data.keywords});
 break;
 case 13:
	tempsql.insert({"introduce",data.introduce});
 break;
 case 14:
	tempsql.insert({"listimg",data.listimg});
 break;
 case 15:
	tempsql.insert({"bigimg",data.bigimg});
 break;
 case 16:
	tempsql.insert({"maincontent",data.maincontent});
 break;
 case 17:
	tempsql.insert({"paracontent",data.paracontent});
 break;
 case 18:
	tempsql.insert({"samepro",data.samepro});
 break;
 case 19:
	tempsql.insert({"attatchfiles",data.attatchfiles});
 break;
 case 20:
if(data.price==0){
	tempsql.insert({"price","0"});
 }else{ 
	tempsql.insert({"price",std::to_string(data.price)});
}
 break;
 case 21:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 22:
	tempsql.insert({"adddate",data.adddate});
 break;
 case 23:
	tempsql.insert({"editdate",data.editdate});
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
if(data.pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(data.pid);
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
if(data.bigid==0){
	tempsql<<",\"bigid\":0";
 }else{ 
	tempsql<<",\"bigid\":"<<std::to_string(data.bigid);
}
if(data.smallid==0){
	tempsql<<",\"smallid\":0";
 }else{ 
	tempsql<<",\"smallid\":"<<std::to_string(data.smallid);
}
if(data.brandid==0){
	tempsql<<",\"brandid\":0";
 }else{ 
	tempsql<<",\"brandid\":"<<std::to_string(data.brandid);
}
if(data.isview==0){
	tempsql<<",\"isview\":0";
 }else{ 
	tempsql<<",\"isview\":"<<std::to_string(data.isview);
}
if(data.isstore==0){
	tempsql<<",\"isstore\":0";
 }else{ 
	tempsql<<",\"isstore\":"<<std::to_string(data.isstore);
}
if(data.ishome==0){
	tempsql<<",\"ishome\":0";
 }else{ 
	tempsql<<",\"ishome\":"<<std::to_string(data.ishome);
}
if(data.showtype==0){
	tempsql<<",\"showtype\":0";
 }else{ 
	tempsql<<",\"showtype\":"<<std::to_string(data.showtype);
}
tempsql<<",\"sntype\":\""<<http::utf8_to_jsonstring(data.sntype);
tempsql<<"\"";
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
tempsql<<",\"keywords\":\""<<http::utf8_to_jsonstring(data.keywords);
tempsql<<"\"";
tempsql<<",\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce);
tempsql<<"\"";
tempsql<<",\"listimg\":\""<<http::utf8_to_jsonstring(data.listimg);
tempsql<<"\"";
tempsql<<",\"bigimg\":\""<<http::utf8_to_jsonstring(data.bigimg);
tempsql<<"\"";
tempsql<<",\"maincontent\":\""<<http::utf8_to_jsonstring(data.maincontent);
tempsql<<"\"";
tempsql<<",\"paracontent\":\""<<http::utf8_to_jsonstring(data.paracontent);
tempsql<<"\"";
tempsql<<",\"samepro\":\""<<http::utf8_to_jsonstring(data.samepro);
tempsql<<"\"";
tempsql<<",\"attatchfiles\":\""<<http::utf8_to_jsonstring(data.attatchfiles);
tempsql<<"\"";
if(data.price==0){
	tempsql<<",\"price\":0";
 }else{ 
	tempsql<<",\"price\":"<<std::to_string(data.price);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
tempsql<<",\"adddate\":\""<<http::utf8_to_jsonstring(data.adddate);
tempsql<<"\"";
tempsql<<",\"editdate\":\""<<http::utf8_to_jsonstring(data.editdate);
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
            for(jj=0;jj<product_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(data.pid);
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
if(data.bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(data.bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(data.smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(data.brandid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(data.isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(data.isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(data.ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(data.showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(data.sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(data.keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(data.listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(data.bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(data.maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(data.paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(data.samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(data.attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(data.price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(data.adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(data.editdate)<<"\"";
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
        product_info::meta metatemp; 
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
			data.pid=std::stoul(set_value_name);
		}catch (...) { 
			data.pid=0;
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
			data.bigid=std::stoul(set_value_name);
		}catch (...) { 
			data.bigid=0;
			 }
			break;
		case 4:
		 try{
			data.smallid=std::stoul(set_value_name);
		}catch (...) { 
			data.smallid=0;
			 }
			break;
		case 5:
		 try{
			data.brandid=std::stoul(set_value_name);
		}catch (...) { 
			data.brandid=0;
			 }
			break;
		case 6:
		 try{
			data.isview=std::stoi(set_value_name);
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 7:
		 try{
			data.isstore=std::stoi(set_value_name);
		}catch (...) { 
			data.isstore=0;
			 }
			break;
		case 8:
		 try{
			data.ishome=std::stoi(set_value_name);
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 9:
		 try{
			data.showtype=std::stoi(set_value_name);
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 10:
		 try{
			data.sntype.append(set_value_name);
		}catch (...) { 
			data.sntype.clear();
			 }
			break;
		case 11:
		 try{
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 12:
		 try{
			data.keywords.append(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 13:
		 try{
			data.introduce.append(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 14:
		 try{
			data.listimg.append(set_value_name);
		}catch (...) { 
			data.listimg.clear();
			 }
			break;
		case 15:
		 try{
			data.bigimg.append(set_value_name);
		}catch (...) { 
			data.bigimg.clear();
			 }
			break;
		case 16:
		 try{
			data.maincontent.append(set_value_name);
		}catch (...) { 
			data.maincontent.clear();
			 }
			break;
		case 17:
		 try{
			data.paracontent.append(set_value_name);
		}catch (...) { 
			data.paracontent.clear();
			 }
			break;
		case 18:
		 try{
			data.samepro.append(set_value_name);
		}catch (...) { 
			data.samepro.clear();
			 }
			break;
		case 19:
		 try{
			data.attatchfiles.append(set_value_name);
		}catch (...) { 
			data.attatchfiles.clear();
			 }
			break;
		case 20:
		 try{
			data.price=std::stoul(set_value_name);
		}catch (...) { 
			data.price=0;
			 }
			break;
		case 21:
		 try{
			data.sortid=std::stoul(set_value_name);
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 22:
		 try{
			data.adddate.append(set_value_name);
		}catch (...) { 
			data.adddate.clear();
			 }
			break;
		case 23:
		 try{
			data.editdate.append(set_value_name);
		}catch (...) { 
			data.editdate.clear();
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
			data.pid=set_value_name;
		}catch (...) { 
			data.pid=0;
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
			data.bigid=set_value_name;
		}catch (...) { 
			data.bigid=0;
			 }
			break;
		case 4:
		 try{
			data.smallid=set_value_name;
		}catch (...) { 
			data.smallid=0;
			 }
			break;
		case 5:
		 try{
			data.brandid=set_value_name;
		}catch (...) { 
			data.brandid=0;
			 }
			break;
		case 6:
		 try{
			data.isview=set_value_name;
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 7:
		 try{
			data.isstore=set_value_name;
		}catch (...) { 
			data.isstore=0;
			 }
			break;
		case 8:
		 try{
			data.ishome=set_value_name;
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 9:
		 try{
			data.showtype=set_value_name;
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 10:
		 try{
			data.sntype=std::to_string(set_value_name);
		}catch (...) { 
			data.sntype.clear();
			 }
			break;
		case 11:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 12:
		 try{
			data.keywords=std::to_string(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 13:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 14:
		 try{
			data.listimg=std::to_string(set_value_name);
		}catch (...) { 
			data.listimg.clear();
			 }
			break;
		case 15:
		 try{
			data.bigimg=std::to_string(set_value_name);
		}catch (...) { 
			data.bigimg.clear();
			 }
			break;
		case 16:
		 try{
			data.maincontent=std::to_string(set_value_name);
		}catch (...) { 
			data.maincontent.clear();
			 }
			break;
		case 17:
		 try{
			data.paracontent=std::to_string(set_value_name);
		}catch (...) { 
			data.paracontent.clear();
			 }
			break;
		case 18:
		 try{
			data.samepro=std::to_string(set_value_name);
		}catch (...) { 
			data.samepro.clear();
			 }
			break;
		case 19:
		 try{
			data.attatchfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.attatchfiles.clear();
			 }
			break;
		case 20:
		 try{
			data.price=set_value_name;
		}catch (...) { 
			data.price=0;
			 }
			break;
		case 21:
		 try{
			data.sortid=set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 22:
		 try{
			data.adddate=std::to_string(set_value_name);
		}catch (...) { 
			data.adddate.clear();
			 }
			break;
		case 23:
		 try{
			data.editdate=std::to_string(set_value_name);
		}catch (...) { 
			data.editdate.clear();
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
			data.pid=(unsigned int)set_value_name;
		}catch (...) { 
			data.pid=0;
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
			data.bigid=(unsigned int)set_value_name;
		}catch (...) { 
			data.bigid=0;
			 }
			break;
		case 4:
		 try{
			data.smallid=(unsigned int)set_value_name;
		}catch (...) { 
			data.smallid=0;
			 }
			break;
		case 5:
		 try{
			data.brandid=(unsigned int)set_value_name;
		}catch (...) { 
			data.brandid=0;
			 }
			break;
		case 6:
		 try{
			data.isview=(int)set_value_name;
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 7:
		 try{
			data.isstore=(int)set_value_name;
		}catch (...) { 
			data.isstore=0;
			 }
			break;
		case 8:
		 try{
			data.ishome=(int)set_value_name;
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 9:
		 try{
			data.showtype=(int)set_value_name;
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 10:
		 try{
			data.sntype=std::to_string(set_value_name);
		}catch (...) { 
			data.sntype.clear();
			 }
			break;
		case 11:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 12:
		 try{
			data.keywords=std::to_string(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 13:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 14:
		 try{
			data.listimg=std::to_string(set_value_name);
		}catch (...) { 
			data.listimg.clear();
			 }
			break;
		case 15:
		 try{
			data.bigimg=std::to_string(set_value_name);
		}catch (...) { 
			data.bigimg.clear();
			 }
			break;
		case 16:
		 try{
			data.maincontent=std::to_string(set_value_name);
		}catch (...) { 
			data.maincontent.clear();
			 }
			break;
		case 17:
		 try{
			data.paracontent=std::to_string(set_value_name);
		}catch (...) { 
			data.paracontent.clear();
			 }
			break;
		case 18:
		 try{
			data.samepro=std::to_string(set_value_name);
		}catch (...) { 
			data.samepro.clear();
			 }
			break;
		case 19:
		 try{
			data.attatchfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.attatchfiles.clear();
			 }
			break;
		case 20:
		 try{
			data.price=(unsigned int)set_value_name;
		}catch (...) { 
			data.price=0;
			 }
			break;
		case 21:
		 try{
			data.sortid=(unsigned int)set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 22:
		 try{
			data.adddate=std::to_string(set_value_name);
		}catch (...) { 
			data.adddate.clear();
			 }
			break;
		case 23:
		 try{
			data.editdate=std::to_string(set_value_name);
		}catch (...) { 
			data.editdate.clear();
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
        for(jj=0;jj<product_info::col_names.size();jj++){
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
if(record[n].pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(record[n].pid);
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
if(record[n].bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(record[n].bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(record[n].smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(record[n].brandid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(record[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(record[n].isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(record[n].ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(record[n].showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(record[n].sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(record[n].listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(record[n].bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(record[n].maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(record[n].paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(record[n].samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(record[n].attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(record[n].price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(record[n].adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(record[n].editdate)<<"\"";
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
   
   std::string to_json(std::function<bool(std::string&,product_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<product_info::col_names.size();jj++){
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
if(record[n].pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(record[n].pid);
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
if(record[n].bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(record[n].bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(record[n].smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(record[n].brandid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(record[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(record[n].isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(record[n].ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(record[n].showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(record[n].sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(record[n].listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(record[n].bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(record[n].maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(record[n].paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(record[n].samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(record[n].attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(record[n].price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(record[n].adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(record[n].editdate)<<"\"";
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
   long long getPK(){  return data.pid; } 
 void setPK(long long val){  data.pid=val;} 
 unsigned  int  getPid(){  return data.pid; } 
 void setPid( unsigned  int  val){  data.pid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getTopicid(){  return data.topicid; } 
 void setTopicid( unsigned  int  val){  data.topicid=val;} 

 unsigned  int  getBigid(){  return data.bigid; } 
 void setBigid( unsigned  int  val){  data.bigid=val;} 

 unsigned  int  getSmallid(){  return data.smallid; } 
 void setSmallid( unsigned  int  val){  data.smallid=val;} 

 unsigned  int  getBrandid(){  return data.brandid; } 
 void setBrandid( unsigned  int  val){  data.brandid=val;} 

 char  getIsview(){  return data.isview; } 
 void setIsview( char  val){  data.isview=val;} 

 char  getIsstore(){  return data.isstore; } 
 void setIsstore( char  val){  data.isstore=val;} 

 unsigned  char  getIshome(){  return data.ishome; } 
 void setIshome( unsigned  char  val){  data.ishome=val;} 

 char  getShowtype(){  return data.showtype; } 
 void setShowtype( char  val){  data.showtype=val;} 

 std::string  getSntype(){  return data.sntype; } 
 std::string & getRefSntype(){  return std::ref(data.sntype); } 
 void setSntype( std::string  &val){  data.sntype=val;} 
 void setSntype(std::string_view val){  data.sntype=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 std::string  getKeywords(){  return data.keywords; } 
 std::string & getRefKeywords(){  return std::ref(data.keywords); } 
 void setKeywords( std::string  &val){  data.keywords=val;} 
 void setKeywords(std::string_view val){  data.keywords=val;} 

 std::string  getIntroduce(){  return data.introduce; } 
 std::string & getRefIntroduce(){  return std::ref(data.introduce); } 
 void setIntroduce( std::string  &val){  data.introduce=val;} 
 void setIntroduce(std::string_view val){  data.introduce=val;} 

 std::string  getListimg(){  return data.listimg; } 
 std::string & getRefListimg(){  return std::ref(data.listimg); } 
 void setListimg( std::string  &val){  data.listimg=val;} 
 void setListimg(std::string_view val){  data.listimg=val;} 

 std::string  getBigimg(){  return data.bigimg; } 
 std::string & getRefBigimg(){  return std::ref(data.bigimg); } 
 void setBigimg( std::string  &val){  data.bigimg=val;} 
 void setBigimg(std::string_view val){  data.bigimg=val;} 

 std::string  getMaincontent(){  return data.maincontent; } 
 std::string & getRefMaincontent(){  return std::ref(data.maincontent); } 
 void setMaincontent( std::string  &val){  data.maincontent=val;} 
 void setMaincontent(std::string_view val){  data.maincontent=val;} 

 std::string  getParacontent(){  return data.paracontent; } 
 std::string & getRefParacontent(){  return std::ref(data.paracontent); } 
 void setParacontent( std::string  &val){  data.paracontent=val;} 
 void setParacontent(std::string_view val){  data.paracontent=val;} 

 std::string  getSamepro(){  return data.samepro; } 
 std::string & getRefSamepro(){  return std::ref(data.samepro); } 
 void setSamepro( std::string  &val){  data.samepro=val;} 
 void setSamepro(std::string_view val){  data.samepro=val;} 

 std::string  getAttatchfiles(){  return data.attatchfiles; } 
 std::string & getRefAttatchfiles(){  return std::ref(data.attatchfiles); } 
 void setAttatchfiles( std::string  &val){  data.attatchfiles=val;} 
 void setAttatchfiles(std::string_view val){  data.attatchfiles=val;} 

 unsigned  int  getPrice(){  return data.price; } 
 void setPrice( unsigned  int  val){  data.price=val;} 

 unsigned  int  getSortid(){  return data.sortid; } 
 void setSortid( unsigned  int  val){  data.sortid=val;} 

 std::string  getAdddate(){  return data.adddate; } 
 std::string & getRefAdddate(){  return std::ref(data.adddate); } 
 void setAdddate( std::string  &val){  data.adddate=val;} 
 void setAdddate(std::string_view val){  data.adddate=val;} 

 std::string  getEditdate(){  return data.editdate; } 
 std::string & getRefEditdate(){  return std::ref(data.editdate); } 
 void setEditdate( std::string  &val){  data.editdate=val;} 
 void setEditdate(std::string_view val){  data.editdate=val;} 

product_info::meta getnewData(){
 	 struct product_info::meta newdata;
	 return newdata; 
} 
product_info::meta getData(){
 	 return data; 
} 
std::vector<product_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<product_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<product_info::col_names.size();jj++){
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
if(tree_data[n].pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(tree_data[n].pid);
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
if(tree_data[n].bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(tree_data[n].bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(tree_data[n].smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(tree_data[n].brandid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(tree_data[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(tree_data[n].isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(tree_data[n].ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(tree_data[n].showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(tree_data[n].sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(tree_data[n].keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(tree_data[n].listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(tree_data[n].bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(tree_data[n].maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(tree_data[n].paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(tree_data[n].samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(tree_data[n].attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(tree_data[n].price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(tree_data[n].adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(tree_data[n].editdate)<<"\"";
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
   
   std::string tree_tojson(const std::vector<product_info::meta_tree> &tree_data,std::function<bool(std::string&,const product_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<product_info::col_names.size();jj++){
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
if(tree_data[n].pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(tree_data[n].pid);
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
if(tree_data[n].bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(tree_data[n].bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(tree_data[n].smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(tree_data[n].brandid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(tree_data[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(tree_data[n].isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(tree_data[n].ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(tree_data[n].showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(tree_data[n].sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(tree_data[n].keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(tree_data[n].listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(tree_data[n].bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(tree_data[n].maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(tree_data[n].paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(tree_data[n].samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(tree_data[n].attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(tree_data[n].price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(tree_data[n].adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(tree_data[n].editdate)<<"\"";
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
   
    template<product_info::cols KeyCol, product_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>()));
        using ValType = decltype(product_info::getField<ValCol>(std::declval<const product_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(product_info::getField<KeyCol>(iter), product_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<product_info::cols KeyCol, product_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>())), 
            decltype(product_info::getField<ValCol>(std::declval<const product_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>())), 
                    decltype(product_info::getField<ValCol>(std::declval<const product_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>()));
        using ValType = decltype(product_info::getField<ValCol>(std::declval<const product_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(product_info::getField<KeyCol>(iter), product_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(product_info::getField<KeyCol>(iter), product_info::getField<ValCol>(iter))) {
                    result.emplace(product_info::getField<KeyCol>(iter), product_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<product_info::cols KeyCol, product_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>()));
        using ValType = decltype(product_info::getField<ValCol>(std::declval<const product_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(product_info::getField<KeyCol>(iter), product_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<product_info::cols KeyCol, product_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>())), 
            decltype(product_info::getField<ValCol>(std::declval<const product_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>())), 
                    decltype(product_info::getField<ValCol>(std::declval<const product_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>()));
        using ValType = decltype(product_info::getField<ValCol>(std::declval<const product_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(product_info::getField<KeyCol>(iter), product_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(product_info::getField<KeyCol>(iter), product_info::getField<ValCol>(iter))) {
                    result.emplace_back(product_info::getField<KeyCol>(iter), product_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<product_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(product_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<product_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(product_info::getField<KeyCol>(std::declval<const product_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(product_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(product_info::getField<KeyCol>(iter))) {
                    result.emplace_back(product_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<product_info::cols Col>
        requires requires(std::ostream& os, decltype(product_info::getField<Col>(std::declval<const product_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << product_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<product_info::cols Col>
        requires requires(std::ostream& os, decltype(product_info::getField<Col>(std::declval<const product_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << product_info::getField<Col>(iter); 
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
   