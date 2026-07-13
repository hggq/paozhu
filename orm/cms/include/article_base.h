#ifndef ORM_CMS_ARTICLEBASEMATA_H
#define ORM_CMS_ARTICLEBASEMATA_H
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

namespace article_info
{
 
    enum class cols : unsigned char 
    {
		aid = 0,
		topicid = 1,
		classtype = 2,
		userid = 3,
		sortid = 4,
		topicname = 5,
		title = 6,
		keywords = 7,
		fromsource = 8,
		author = 9,
		addip = 10,
		createtime = 11,
		addtime = 12,
		readnum = 13,
		review = 14,
		icoimg = 15,
		content = 16,
		mdcontent = 17,
		isopen = 18,
		ishome = 19,
		iscomment = 20,
		showtype = 21,
		fromlocal = 22,
		texturl = 23,
		summary = 24,
		editauthor = 25,
		relatecontent = 26,

    };

    struct meta
    {
		 unsigned  int  aid = 0; ///**/
		 unsigned  int  topicid = 0; ///*栏目id*/
		 unsigned  int  classtype = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 int  sortid = 0; ///*排序*/
		 std::string  topicname = ""; ///**/
		 std::string  title = ""; ///**/
		 std::string  keywords = ""; ///*关键字*/
		 std::string  fromsource = ""; ///*文章来源*/
		 std::string  author = ""; ///*文章作者*/
		 std::string  addip = ""; ///**/
		 std::string  createtime = ""; ///*显示的创建时间*/
		 unsigned  long long  addtime = 0; ///*添加或修改时间*/
		 int  readnum = 0; ///**/
		 int  review = 0; ///**/
		 std::string  icoimg = ""; ///*列表图片*/
		 std::string  content = ""; ///**/
		 std::string  mdcontent = ""; ///*markdown content*/
		 char  isopen = 0; ///*是否开放*/
		 char  ishome = 0; ///*show homepage*/
		 char  iscomment = 0; ///*是否可以评论*/
		 char  showtype = 0; ///*art content display type*/
		 std::string  fromlocal = ""; ///*发表地址*/
		 std::string  texturl = ""; ///*url用英文代替*/
		 std::string  summary = ""; ///*文章摘要*/
		 std::string  editauthor = ""; ///*文章编辑*/
		 std::string  relatecontent = ""; ///*相关内容*/
	};
  
    struct meta_tree
    {
		 unsigned  int  aid = 0; ///**/
		 unsigned  int  topicid = 0; ///*栏目id*/
		 unsigned  int  classtype = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 int  sortid = 0; ///*排序*/
		 std::string  topicname = ""; ///**/
		 std::string  title = ""; ///**/
		 std::string  keywords = ""; ///*关键字*/
		 std::string  fromsource = ""; ///*文章来源*/
		 std::string  author = ""; ///*文章作者*/
		 std::string  addip = ""; ///**/
		 std::string  createtime = ""; ///*显示的创建时间*/
		 unsigned  long long  addtime = 0; ///*添加或修改时间*/
		 int  readnum = 0; ///**/
		 int  review = 0; ///**/
		 std::string  icoimg = ""; ///*列表图片*/
		 std::string  content = ""; ///**/
		 std::string  mdcontent = ""; ///*markdown content*/
		 char  isopen = 0; ///*是否开放*/
		 char  ishome = 0; ///*show homepage*/
		 char  iscomment = 0; ///*是否可以评论*/
		 char  showtype = 0; ///*art content display type*/
		 std::string  fromlocal = ""; ///*发表地址*/
		 std::string  texturl = ""; ///*url用英文代替*/
		 std::string  summary = ""; ///*文章摘要*/
		 std::string  editauthor = ""; ///*文章编辑*/
		 std::string  relatecontent = ""; ///*相关内容*/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  aid = 0; ///**/
		 unsigned  int  topicid = 0; ///*栏目id*/
		 unsigned  int  classtype = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 int  sortid = 0; ///*排序*/
		 std::string  topicname = ""; ///**/
		 std::string  title = ""; ///**/
		 std::string  keywords = ""; ///*关键字*/
		 std::string  fromsource = ""; ///*文章来源*/
		 std::string  author = ""; ///*文章作者*/
		 std::string  addip = ""; ///**/
		 std::string  createtime = ""; ///*显示的创建时间*/
		 unsigned  long long  addtime = 0; ///*添加或修改时间*/
		 int  readnum = 0; ///**/
		 int  review = 0; ///**/
		 std::string  icoimg = ""; ///*列表图片*/
		 std::string  content = ""; ///**/
		 std::string  mdcontent = ""; ///*markdown content*/
		 char  isopen = 0; ///*是否开放*/
		 char  ishome = 0; ///*show homepage*/
		 char  iscomment = 0; ///*是否可以评论*/
		 char  showtype = 0; ///*art content display type*/
		 std::string  fromlocal = ""; ///*发表地址*/
		 std::string  texturl = ""; ///*url用英文代替*/
		 std::string  summary = ""; ///*文章摘要*/
		 std::string  editauthor = ""; ///*文章编辑*/
		 std::string  relatecontent = ""; ///*相关内容*/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::aid) { 
		 return m.aid;
		} else if constexpr (Col == cols::topicid) { 
		 return m.topicid;
		} else if constexpr (Col == cols::classtype) { 
		 return m.classtype;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::sortid) { 
		 return m.sortid;
		} else if constexpr (Col == cols::topicname) { 
		 return m.topicname;
		} else if constexpr (Col == cols::title) { 
		 return m.title;
		} else if constexpr (Col == cols::keywords) { 
		 return m.keywords;
		} else if constexpr (Col == cols::fromsource) { 
		 return m.fromsource;
		} else if constexpr (Col == cols::author) { 
		 return m.author;
		} else if constexpr (Col == cols::addip) { 
		 return m.addip;
		} else if constexpr (Col == cols::createtime) { 
		 return m.createtime;
		} else if constexpr (Col == cols::addtime) { 
		 return m.addtime;
		} else if constexpr (Col == cols::readnum) { 
		 return m.readnum;
		} else if constexpr (Col == cols::review) { 
		 return m.review;
		} else if constexpr (Col == cols::icoimg) { 
		 return m.icoimg;
		} else if constexpr (Col == cols::content) { 
		 return m.content;
		} else if constexpr (Col == cols::mdcontent) { 
		 return m.mdcontent;
		} else if constexpr (Col == cols::isopen) { 
		 return m.isopen;
		} else if constexpr (Col == cols::ishome) { 
		 return m.ishome;
		} else if constexpr (Col == cols::iscomment) { 
		 return m.iscomment;
		} else if constexpr (Col == cols::showtype) { 
		 return m.showtype;
		} else if constexpr (Col == cols::fromlocal) { 
		 return m.fromlocal;
		} else if constexpr (Col == cols::texturl) { 
		 return m.texturl;
		} else if constexpr (Col == cols::summary) { 
		 return m.summary;
		} else if constexpr (Col == cols::editauthor) { 
		 return m.editauthor;
		} else if constexpr (Col == cols::relatecontent) { 
		 return m.relatecontent;
		
        } else {
            //static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using aid =  unsigned  int ;
		using topicid =  unsigned  int ;
		using classtype =  unsigned  int ;
		using userid =  unsigned  int ;
		using sortid =  int ;
		using topicname =  std::string ;
		using title =  std::string ;
		using keywords =  std::string ;
		using fromsource =  std::string ;
		using author =  std::string ;
		using addip =  std::string ;
		using createtime =  std::string ;
		using addtime =  unsigned  long long ;
		using readnum =  int ;
		using review =  int ;
		using icoimg =  std::string ;
		using content =  std::string ;
		using mdcontent =  std::string ;
		using isopen =  char ;
		using ishome =  char ;
		using iscomment =  char ;
		using showtype =  char ;
		using fromlocal =  std::string ;
		using texturl =  std::string ;
		using summary =  std::string ;
		using editauthor =  std::string ;
		using relatecontent =  std::string ;

    }

    
    #define ORM_CMS_ARTICLE_EXPAND(x) x 
    
    #define ORM_CMS_ARTICLE_META_FIELD_TYPE(col) \
         orm::cms::article_info::type::col 
    
    #define ORM_CMS_ARTICLE_PROJ_MEMBER(col) \
          ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_1(c1) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_1( c1)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c2))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c3))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c4))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c5))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c6))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c7))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c8))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c9))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c10))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c11))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c12))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c13))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c14))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c15))
         
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBER(c16))
         
    #define ORM_CMS_ARTICLE_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_ARTICLE_PROJ_MEMBERS(...) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_16, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_15, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_14, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_13, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_12, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_11, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_10, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_9, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_8, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_7, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_6, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_5, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_4, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_3, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_2, \
            ORM_CMS_ARTICLE_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_ARTICLE_COUNT(...) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_ARTICLE_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_ARTICLE_TO_JSON_1(c1) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_ARTICLE_TO_JSON_2(c1,c2) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_ARTICLE_TO_JSON_BODY(...) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_ARTICLE_TO_JSON_16,ORM_CMS_ARTICLE_TO_JSON_15,ORM_CMS_ARTICLE_TO_JSON_14,ORM_CMS_ARTICLE_TO_JSON_13,ORM_CMS_ARTICLE_TO_JSON_12,ORM_CMS_ARTICLE_TO_JSON_11,ORM_CMS_ARTICLE_TO_JSON_10,ORM_CMS_ARTICLE_TO_JSON_9,ORM_CMS_ARTICLE_TO_JSON_8,ORM_CMS_ARTICLE_TO_JSON_7,ORM_CMS_ARTICLE_TO_JSON_6,ORM_CMS_ARTICLE_TO_JSON_5,ORM_CMS_ARTICLE_TO_JSON_4,ORM_CMS_ARTICLE_TO_JSON_3,ORM_CMS_ARTICLE_TO_JSON_2,ORM_CMS_ARTICLE_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_ARTICLE_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_1(n1)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_1(n1)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_ARTICLE_CAT(a, b) ORM_CMS_ARTICLE_CAT_(a, b)
    #define ORM_CMS_ARTICLE_CAT_(a, b) a##b

    #define ORM_CMS_ARTICLE_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_ARTICLE_CAT(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_ARTICLE_TO_JSON_CUSTOM(...) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_ARTICLE_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_ARTICLE_SET_VAL_1(c1) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_ARTICLE_SET_VAL_2(c1,c2) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_1(c1)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_2(c1,c2)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_ARTICLE_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_ARTICLE_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_ARTICLE_CAT(ORM_CMS_ARTICLE_SET_VAL_, N)
    
    
    #define ORM_CMS_ARTICLE_SET_VAL_FIELDS(...) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_ARTICLE_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_FIELDS(ORM_CMS_ARTICLE_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_ARTICLE_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::article_info { \
            struct StructName { \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_ARTICLE_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::article_info { \
            struct StructName { \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM(ORM_CMS_ARTICLE_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_ARTICLE_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::article_info { \
            struct StructName { \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_ARTICLE_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::article_info { \
            struct StructName { \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_ARTICLE_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::article_info { \
            struct StructName { \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_TO_JSON_CUSTOM(ORM_CMS_ARTICLE_UNWRAP CustomNames));  \
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
                        ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_ARTICLE_EXPAND(ORM_CMS_ARTICLE_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    static constexpr std::array<std::string_view,27> col_names={"aid","topicid","classtype","userid","sortid","topicname","title","keywords","fromsource","author","addip","createtime","addtime","readnum","review","icoimg","content","mdcontent","isopen","ishome","iscomment","showtype","fromlocal","texturl","summary","editauthor","relatecontent"};
	static constexpr std::array<unsigned char,27> col_types={3,3,3,3,3,253,253,253,253,253,253,253,8,3,3,253,252,252,1,1,1,1,253,253,253,253,253};
	static constexpr std::array<unsigned char,27> col_length={0,0,0,0,0,140,128,128,60,40,20,20,0,0,0,255,0,0,0,0,0,0,60,255,255,40,0};
	static constexpr std::array<unsigned char,27> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

struct article_base
{
      article_info::meta data;
    std::vector<article_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<article_info::meta>::iterator begin(){     return record.begin(); }
std::vector<article_info::meta>::iterator end(){     return record.end(); }
std::vector<article_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<article_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="article";
static constexpr std::string_view org_tablename="article";
static constexpr std::string_view modelname="Article";

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
case 3:
   	 return 0;
break;
case 5:
   	 return 10;
break;
case 6:
   	 return 9;
break;
case 7:
   	 return 12;
break;
 }
 break;
case 'c':
 switch(coln.size()){  
case 7:
   	 return 16;
break;
case 9:
   	 return 2;
break;
case 10:
   	 return 11;
break;
 }
 break;
case 'e':
   	 return 25;
break;
case 'f':
 switch(coln.size()){  
case 9:
   	 return 22;
break;
case 10:
   	 return 8;
break;
 }
 break;
case 'i':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 19; }
 if(colpospppc=='g'){ return 15; }
 if(colpospppc=='n'){ return 18; }
   	 break;
case 9:
   	 return 20;
break;
 }
 break;
case 'k':
   	 return 7;
break;
case 'm':
   	 return 17;
break;
case 'r':
 switch(coln.size()){  
case 6:
   	 return 14;
break;
case 7:
   	 return 13;
break;
case 13:
   	 return 26;
break;
 }
 break;
case 's':
 switch(coln.size()){  
case 6:
   	 return 4;
break;
case 7:
   	 return 24;
break;
case 8:
   	 return 21;
break;
 }
 break;
case 't':
 switch(coln.size()){  
case 5:
   	 return 6;
break;
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 1; }
 if(colpospppc=='l'){ return 23; }
   	 break;
case 9:
   	 return 5;
break;
 }
 break;
case 'u':
   	 return 3;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "aid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     article_info::meta metatemp;    
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
        for(;j<article_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<article_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.aid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.aid);
}
if(data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.topicid);
}
if(data.classtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.classtype);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
tempsql<<",'"<<stringaddslash(data.topicname)<<"'";
tempsql<<",'"<<stringaddslash(data.title)<<"'";
tempsql<<",'"<<stringaddslash(data.keywords)<<"'";
tempsql<<",'"<<stringaddslash(data.fromsource)<<"'";
tempsql<<",'"<<stringaddslash(data.author)<<"'";
tempsql<<",'"<<stringaddslash(data.addip)<<"'";
tempsql<<",'"<<stringaddslash(data.createtime)<<"'";
if(data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.addtime);
}
if(data.readnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.readnum);
}
if(data.review==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.review);
}
tempsql<<",'"<<stringaddslash(data.icoimg)<<"'";
tempsql<<",'"<<stringaddslash(data.content)<<"'";
tempsql<<",'"<<stringaddslash(data.mdcontent)<<"'";
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
if(data.ishome==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.ishome);
}
if(data.iscomment==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.iscomment);
}
if(data.showtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.showtype);
}
tempsql<<",'"<<stringaddslash(data.fromlocal)<<"'";
tempsql<<",'"<<stringaddslash(data.texturl)<<"'";
tempsql<<",'"<<stringaddslash(data.summary)<<"'";
tempsql<<",'"<<stringaddslash(data.editauthor)<<"'";
tempsql<<",'"<<stringaddslash(data.relatecontent)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string make_data_insert_sql(const article_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<article_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<article_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.aid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.aid);
}
if(insert_data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.topicid);
}
if(insert_data.classtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.classtype);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
tempsql<<",'"<<stringaddslash(insert_data.topicname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.keywords)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.fromsource)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.author)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.addip)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.createtime)<<"'";
if(insert_data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.addtime);
}
if(insert_data.readnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.readnum);
}
if(insert_data.review==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.review);
}
tempsql<<",'"<<stringaddslash(insert_data.icoimg)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.content)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.mdcontent)<<"'";
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
}
if(insert_data.ishome==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.ishome);
}
if(insert_data.iscomment==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.iscomment);
}
if(insert_data.showtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.showtype);
}
tempsql<<",'"<<stringaddslash(insert_data.fromlocal)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.texturl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.summary)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.editauthor)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.relatecontent)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string make_vector_insert_sql(const std::vector<article_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<article_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<article_info::col_names[j];
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


            	if(insert_data[i].aid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].aid);
	}
	if(insert_data[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].topicid);
	}
	if(insert_data[i].classtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].classtype);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].topicname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].keywords)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].fromsource)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].author)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].addip)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].createtime)<<"'";
	if(insert_data[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].addtime);
	}
	if(insert_data[i].readnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].readnum);
	}
	if(insert_data[i].review==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].review);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].icoimg)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].content)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].mdcontent)<<"'";
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
	}
	if(insert_data[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].ishome);
	}
	if(insert_data[i].iscomment==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].iscomment);
	}
	if(insert_data[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].showtype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].fromlocal)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].texturl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].summary)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].editauthor)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].relatecontent)<<"'";
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

        if(data.aid==0){
	tempsql<<"`aid`=0";
 }else{ 
	tempsql<<"`aid`="<<std::to_string(data.aid);
}
if(data.topicid==0){
	tempsql<<",`topicid`=0";
 }else{ 
	tempsql<<",`topicid`="<<std::to_string(data.topicid);
}
if(data.classtype==0){
	tempsql<<",`classtype`=0";
 }else{ 
	tempsql<<",`classtype`="<<std::to_string(data.classtype);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
tempsql<<",`topicname`='"<<stringaddslash(data.topicname)<<"'";
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
tempsql<<",`keywords`='"<<stringaddslash(data.keywords)<<"'";
tempsql<<",`fromsource`='"<<stringaddslash(data.fromsource)<<"'";
tempsql<<",`author`='"<<stringaddslash(data.author)<<"'";
tempsql<<",`addip`='"<<stringaddslash(data.addip)<<"'";
tempsql<<",`createtime`='"<<stringaddslash(data.createtime)<<"'";
if(data.addtime==0){
	tempsql<<",`addtime`=0";
 }else{ 
	tempsql<<",`addtime`="<<std::to_string(data.addtime);
}
if(data.readnum==0){
	tempsql<<",`readnum`=0";
 }else{ 
	tempsql<<",`readnum`="<<std::to_string(data.readnum);
}
if(data.review==0){
	tempsql<<",`review`=0";
 }else{ 
	tempsql<<",`review`="<<std::to_string(data.review);
}
tempsql<<",`icoimg`='"<<stringaddslash(data.icoimg)<<"'";
tempsql<<",`content`='"<<stringaddslash(data.content)<<"'";
tempsql<<",`mdcontent`='"<<stringaddslash(data.mdcontent)<<"'";
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
if(data.ishome==0){
	tempsql<<",`ishome`=0";
 }else{ 
	tempsql<<",`ishome`="<<std::to_string(data.ishome);
}
if(data.iscomment==0){
	tempsql<<",`iscomment`=0";
 }else{ 
	tempsql<<",`iscomment`="<<std::to_string(data.iscomment);
}
if(data.showtype==0){
	tempsql<<",`showtype`=0";
 }else{ 
	tempsql<<",`showtype`="<<std::to_string(data.showtype);
}
tempsql<<",`fromlocal`='"<<stringaddslash(data.fromlocal)<<"'";
tempsql<<",`texturl`='"<<stringaddslash(data.texturl)<<"'";
tempsql<<",`summary`='"<<stringaddslash(data.summary)<<"'";
tempsql<<",`editauthor`='"<<stringaddslash(data.editauthor)<<"'";
tempsql<<",`relatecontent`='"<<stringaddslash(data.relatecontent)<<"'";
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
if(data.aid==0){
	tempsql<<"`aid`=0";
 }else{ 
	tempsql<<"`aid`="<<std::to_string(data.aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.classtype==0){
	tempsql<<"`classtype`=0";
 }else{ 
	tempsql<<"`classtype`="<<std::to_string(data.classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"`userid`=0";
 }else{ 
	tempsql<<"`userid`="<<std::to_string(data.userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`topicname`='"<<stringaddslash(data.topicname)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`keywords`='"<<stringaddslash(data.keywords)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`fromsource`='"<<stringaddslash(data.fromsource)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`author`='"<<stringaddslash(data.author)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`addip`='"<<stringaddslash(data.addip)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`createtime`='"<<stringaddslash(data.createtime)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"`addtime`=0";
 }else{ 
	tempsql<<"`addtime`="<<std::to_string(data.addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.readnum==0){
	tempsql<<"`readnum`=0";
 }else{ 
	tempsql<<"`readnum`="<<std::to_string(data.readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.review==0){
	tempsql<<"`review`=0";
 }else{ 
	tempsql<<"`review`="<<std::to_string(data.review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`icoimg`='"<<stringaddslash(data.icoimg)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`content`='"<<stringaddslash(data.content)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`mdcontent`='"<<stringaddslash(data.mdcontent)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(data.ishome==0){
	tempsql<<"`ishome`=0";
 }else{ 
	tempsql<<"`ishome`="<<std::to_string(data.ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.iscomment==0){
	tempsql<<"`iscomment`=0";
 }else{ 
	tempsql<<"`iscomment`="<<std::to_string(data.iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.showtype==0){
	tempsql<<"`showtype`=0";
 }else{ 
	tempsql<<"`showtype`="<<std::to_string(data.showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`fromlocal`='"<<stringaddslash(data.fromlocal)<<"'";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`texturl`='"<<stringaddslash(data.texturl)<<"'";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`summary`='"<<stringaddslash(data.summary)<<"'";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`editauthor`='"<<stringaddslash(data.editauthor)<<"'";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`relatecontent`='"<<stringaddslash(data.relatecontent)<<"'";
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
        for (; j < article_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << article_info::col_names[j];
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
            	if(record[i].aid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].aid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].classtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].classtype);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].topicname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].keywords)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].fromsource)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].author)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addip)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].createtime)<<"'";
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].readnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].readnum);
	}
	if(record[i].review==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].review);
	}
	tempsql<<",'"<<stringaddslash(record[i].icoimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mdcontent)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ishome);
	}
	if(record[i].iscomment==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].iscomment);
	}
	if(record[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].showtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].fromlocal)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].texturl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].summary)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].editauthor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].relatecontent)<<"'";
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
        for (; j < article_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << article_info::col_names[j];
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
            	if(record[i].aid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].aid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].classtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].classtype);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].topicname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].keywords)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].fromsource)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].author)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addip)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].createtime)<<"'";
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].readnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].readnum);
	}
	if(record[i].review==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].review);
	}
	tempsql<<",'"<<stringaddslash(record[i].icoimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mdcontent)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ishome);
	}
	if(record[i].iscomment==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].iscomment);
	}
	if(record[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].showtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].fromlocal)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].texturl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].summary)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].editauthor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].relatecontent)<<"'";
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
            for(jj=0;jj<article_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.aid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.aid));
}
 break;
 case 1:
if(data.topicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicid));
}
 break;
 case 2:
if(data.classtype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.classtype));
}
 break;
 case 3:
if(data.userid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.userid));
}
 break;
 case 4:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 5:
	temparray.push_back(data.topicname);
 break;
 case 6:
	temparray.push_back(data.title);
 break;
 case 7:
	temparray.push_back(data.keywords);
 break;
 case 8:
	temparray.push_back(data.fromsource);
 break;
 case 9:
	temparray.push_back(data.author);
 break;
 case 10:
	temparray.push_back(data.addip);
 break;
 case 11:
	temparray.push_back(data.createtime);
 break;
 case 12:
if(data.addtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.addtime));
}
 break;
 case 13:
if(data.readnum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.readnum));
}
 break;
 case 14:
if(data.review==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.review));
}
 break;
 case 15:
	temparray.push_back(data.icoimg);
 break;
 case 16:
	temparray.push_back(data.content);
 break;
 case 17:
	temparray.push_back(data.mdcontent);
 break;
 case 18:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 19:
if(data.ishome==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.ishome));
}
 break;
 case 20:
if(data.iscomment==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.iscomment));
}
 break;
 case 21:
if(data.showtype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.showtype));
}
 break;
 case 22:
	temparray.push_back(data.fromlocal);
 break;
 case 23:
	temparray.push_back(data.texturl);
 break;
 case 24:
	temparray.push_back(data.summary);
 break;
 case 25:
	temparray.push_back(data.editauthor);
 break;
 case 26:
	temparray.push_back(data.relatecontent);
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
            for(jj=0;jj<article_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.aid==0){
	tempsql.insert({"aid","0"});
 }else{ 
	tempsql.insert({"aid",std::to_string(data.aid)});
}
 break;
 case 1:
if(data.topicid==0){
	tempsql.insert({"topicid","0"});
 }else{ 
	tempsql.insert({"topicid",std::to_string(data.topicid)});
}
 break;
 case 2:
if(data.classtype==0){
	tempsql.insert({"classtype","0"});
 }else{ 
	tempsql.insert({"classtype",std::to_string(data.classtype)});
}
 break;
 case 3:
if(data.userid==0){
	tempsql.insert({"userid","0"});
 }else{ 
	tempsql.insert({"userid",std::to_string(data.userid)});
}
 break;
 case 4:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 5:
	tempsql.insert({"topicname",data.topicname});
 break;
 case 6:
	tempsql.insert({"title",data.title});
 break;
 case 7:
	tempsql.insert({"keywords",data.keywords});
 break;
 case 8:
	tempsql.insert({"fromsource",data.fromsource});
 break;
 case 9:
	tempsql.insert({"author",data.author});
 break;
 case 10:
	tempsql.insert({"addip",data.addip});
 break;
 case 11:
	tempsql.insert({"createtime",data.createtime});
 break;
 case 12:
if(data.addtime==0){
	tempsql.insert({"addtime","0"});
 }else{ 
	tempsql.insert({"addtime",std::to_string(data.addtime)});
}
 break;
 case 13:
if(data.readnum==0){
	tempsql.insert({"readnum","0"});
 }else{ 
	tempsql.insert({"readnum",std::to_string(data.readnum)});
}
 break;
 case 14:
if(data.review==0){
	tempsql.insert({"review","0"});
 }else{ 
	tempsql.insert({"review",std::to_string(data.review)});
}
 break;
 case 15:
	tempsql.insert({"icoimg",data.icoimg});
 break;
 case 16:
	tempsql.insert({"content",data.content});
 break;
 case 17:
	tempsql.insert({"mdcontent",data.mdcontent});
 break;
 case 18:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 19:
if(data.ishome==0){
	tempsql.insert({"ishome","0"});
 }else{ 
	tempsql.insert({"ishome",std::to_string(data.ishome)});
}
 break;
 case 20:
if(data.iscomment==0){
	tempsql.insert({"iscomment","0"});
 }else{ 
	tempsql.insert({"iscomment",std::to_string(data.iscomment)});
}
 break;
 case 21:
if(data.showtype==0){
	tempsql.insert({"showtype","0"});
 }else{ 
	tempsql.insert({"showtype",std::to_string(data.showtype)});
}
 break;
 case 22:
	tempsql.insert({"fromlocal",data.fromlocal});
 break;
 case 23:
	tempsql.insert({"texturl",data.texturl});
 break;
 case 24:
	tempsql.insert({"summary",data.summary});
 break;
 case 25:
	tempsql.insert({"editauthor",data.editauthor});
 break;
 case 26:
	tempsql.insert({"relatecontent",data.relatecontent});
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
if(data.aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(data.aid);
}
if(data.topicid==0){
	tempsql<<",\"topicid\":0";
 }else{ 
	tempsql<<",\"topicid\":"<<std::to_string(data.topicid);
}
if(data.classtype==0){
	tempsql<<",\"classtype\":0";
 }else{ 
	tempsql<<",\"classtype\":"<<std::to_string(data.classtype);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
tempsql<<",\"topicname\":\""<<http::utf8_to_jsonstring(data.topicname);
tempsql<<"\"";
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"keywords\":\""<<http::utf8_to_jsonstring(data.keywords);
tempsql<<"\"";
tempsql<<",\"fromsource\":\""<<http::utf8_to_jsonstring(data.fromsource);
tempsql<<"\"";
tempsql<<",\"author\":\""<<http::utf8_to_jsonstring(data.author);
tempsql<<"\"";
tempsql<<",\"addip\":\""<<http::utf8_to_jsonstring(data.addip);
tempsql<<"\"";
tempsql<<",\"createtime\":\""<<http::utf8_to_jsonstring(data.createtime);
tempsql<<"\"";
if(data.addtime==0){
	tempsql<<",\"addtime\":0";
 }else{ 
	tempsql<<",\"addtime\":"<<std::to_string(data.addtime);
}
if(data.readnum==0){
	tempsql<<",\"readnum\":0";
 }else{ 
	tempsql<<",\"readnum\":"<<std::to_string(data.readnum);
}
if(data.review==0){
	tempsql<<",\"review\":0";
 }else{ 
	tempsql<<",\"review\":"<<std::to_string(data.review);
}
tempsql<<",\"icoimg\":\""<<http::utf8_to_jsonstring(data.icoimg);
tempsql<<"\"";
tempsql<<",\"content\":\""<<http::utf8_to_jsonstring(data.content);
tempsql<<"\"";
tempsql<<",\"mdcontent\":\""<<http::utf8_to_jsonstring(data.mdcontent);
tempsql<<"\"";
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
if(data.ishome==0){
	tempsql<<",\"ishome\":0";
 }else{ 
	tempsql<<",\"ishome\":"<<std::to_string(data.ishome);
}
if(data.iscomment==0){
	tempsql<<",\"iscomment\":0";
 }else{ 
	tempsql<<",\"iscomment\":"<<std::to_string(data.iscomment);
}
if(data.showtype==0){
	tempsql<<",\"showtype\":0";
 }else{ 
	tempsql<<",\"showtype\":"<<std::to_string(data.showtype);
}
tempsql<<",\"fromlocal\":\""<<http::utf8_to_jsonstring(data.fromlocal);
tempsql<<"\"";
tempsql<<",\"texturl\":\""<<http::utf8_to_jsonstring(data.texturl);
tempsql<<"\"";
tempsql<<",\"summary\":\""<<http::utf8_to_jsonstring(data.summary);
tempsql<<"\"";
tempsql<<",\"editauthor\":\""<<http::utf8_to_jsonstring(data.editauthor);
tempsql<<"\"";
tempsql<<",\"relatecontent\":\""<<http::utf8_to_jsonstring(data.relatecontent);
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
            for(jj=0;jj<article_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(data.aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(data.classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(data.userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topicname\":\""<<http::utf8_to_jsonstring(data.topicname)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(data.keywords)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<http::utf8_to_jsonstring(data.fromsource)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<http::utf8_to_jsonstring(data.author)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(data.addip)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<http::utf8_to_jsonstring(data.createtime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(data.addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(data.readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(data.review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<http::utf8_to_jsonstring(data.icoimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mdcontent\":\""<<http::utf8_to_jsonstring(data.mdcontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(data.ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(data.ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(data.iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(data.showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<http::utf8_to_jsonstring(data.fromlocal)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<http::utf8_to_jsonstring(data.texturl)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<http::utf8_to_jsonstring(data.summary)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<http::utf8_to_jsonstring(data.editauthor)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"relatecontent\":\""<<http::utf8_to_jsonstring(data.relatecontent)<<"\"";
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
        article_info::meta metatemp; 
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
		  http::json_set_val(data.aid,set_value_name);
		 break;
		
		case 1:
		  http::json_set_val(data.topicid,set_value_name);
		 break;
		
		case 2:
		  http::json_set_val(data.classtype,set_value_name);
		 break;
		
		case 3:
		  http::json_set_val(data.userid,set_value_name);
		 break;
		
		case 4:
		  http::json_set_val(data.sortid,set_value_name);
		 break;
		
		case 5:
		  http::json_set_val(data.topicname,set_value_name);
		 break;
		
		case 6:
		  http::json_set_val(data.title,set_value_name);
		 break;
		
		case 7:
		  http::json_set_val(data.keywords,set_value_name);
		 break;
		
		case 8:
		  http::json_set_val(data.fromsource,set_value_name);
		 break;
		
		case 9:
		  http::json_set_val(data.author,set_value_name);
		 break;
		
		case 10:
		  http::json_set_val(data.addip,set_value_name);
		 break;
		
		case 11:
		  http::json_set_val(data.createtime,set_value_name);
		 break;
		
		case 12:
		  http::json_set_val(data.addtime,set_value_name);
		 break;
		
		case 13:
		  http::json_set_val(data.readnum,set_value_name);
		 break;
		
		case 14:
		  http::json_set_val(data.review,set_value_name);
		 break;
		
		case 15:
		  http::json_set_val(data.icoimg,set_value_name);
		 break;
		
		case 16:
		  http::json_set_val(data.content,set_value_name);
		 break;
		
		case 17:
		  http::json_set_val(data.mdcontent,set_value_name);
		 break;
		
		case 18:
		  http::json_set_val(data.isopen,set_value_name);
		 break;
		
		case 19:
		  http::json_set_val(data.ishome,set_value_name);
		 break;
		
		case 20:
		  http::json_set_val(data.iscomment,set_value_name);
		 break;
		
		case 21:
		  http::json_set_val(data.showtype,set_value_name);
		 break;
		
		case 22:
		  http::json_set_val(data.fromlocal,set_value_name);
		 break;
		
		case 23:
		  http::json_set_val(data.texturl,set_value_name);
		 break;
		
		case 24:
		  http::json_set_val(data.summary,set_value_name);
		 break;
		
		case 25:
		  http::json_set_val(data.editauthor,set_value_name);
		 break;
		
		case 26:
		  http::json_set_val(data.relatecontent,set_value_name);
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
        for(jj=0;jj<article_info::col_names.size();jj++){
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
if(record[n].aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(record[n].aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(record[n].classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topicname\":\""<<http::utf8_to_jsonstring(record[n].topicname)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<http::utf8_to_jsonstring(record[n].fromsource)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<http::utf8_to_jsonstring(record[n].author)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(record[n].addip)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<http::utf8_to_jsonstring(record[n].createtime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(record[n].readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(record[n].review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<http::utf8_to_jsonstring(record[n].icoimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mdcontent\":\""<<http::utf8_to_jsonstring(record[n].mdcontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(record[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(record[n].ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(record[n].iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(record[n].showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<http::utf8_to_jsonstring(record[n].fromlocal)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<http::utf8_to_jsonstring(record[n].texturl)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<http::utf8_to_jsonstring(record[n].summary)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<http::utf8_to_jsonstring(record[n].editauthor)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"relatecontent\":\""<<http::utf8_to_jsonstring(record[n].relatecontent)<<"\"";
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
   
   std::string to_json(std::function<bool(std::string&,article_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<article_info::col_names.size();jj++){
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
if(record[n].aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(record[n].aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(record[n].classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topicname\":\""<<http::utf8_to_jsonstring(record[n].topicname)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<http::utf8_to_jsonstring(record[n].fromsource)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<http::utf8_to_jsonstring(record[n].author)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(record[n].addip)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<http::utf8_to_jsonstring(record[n].createtime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(record[n].readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(record[n].review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<http::utf8_to_jsonstring(record[n].icoimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mdcontent\":\""<<http::utf8_to_jsonstring(record[n].mdcontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(record[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(record[n].ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(record[n].iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(record[n].showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<http::utf8_to_jsonstring(record[n].fromlocal)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<http::utf8_to_jsonstring(record[n].texturl)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<http::utf8_to_jsonstring(record[n].summary)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<http::utf8_to_jsonstring(record[n].editauthor)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"relatecontent\":\""<<http::utf8_to_jsonstring(record[n].relatecontent)<<"\"";
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
   long long getPK(){  return data.aid; } 
 void setPK(long long val){  data.aid=val;} 
 unsigned  int  getAid(){  return data.aid; } 
 void setAid( unsigned  int  val){  data.aid=val;} 

 unsigned  int  getTopicid(){  return data.topicid; } 
 void setTopicid( unsigned  int  val){  data.topicid=val;} 

 unsigned  int  getClasstype(){  return data.classtype; } 
 void setClasstype( unsigned  int  val){  data.classtype=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 int  getSortid(){  return data.sortid; } 
 void setSortid( int  val){  data.sortid=val;} 

 std::string  getTopicname(){  return data.topicname; } 
 std::string & getRefTopicname(){  return std::ref(data.topicname); } 
 void setTopicname( std::string  &val){  data.topicname=val;} 
 void setTopicname(std::string_view val){  data.topicname=val;} 

 std::string  getTitle(){  return data.title; } 
 std::string & getRefTitle(){  return std::ref(data.title); } 
 void setTitle( std::string  &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

 std::string  getKeywords(){  return data.keywords; } 
 std::string & getRefKeywords(){  return std::ref(data.keywords); } 
 void setKeywords( std::string  &val){  data.keywords=val;} 
 void setKeywords(std::string_view val){  data.keywords=val;} 

 std::string  getFromsource(){  return data.fromsource; } 
 std::string & getRefFromsource(){  return std::ref(data.fromsource); } 
 void setFromsource( std::string  &val){  data.fromsource=val;} 
 void setFromsource(std::string_view val){  data.fromsource=val;} 

 std::string  getAuthor(){  return data.author; } 
 std::string & getRefAuthor(){  return std::ref(data.author); } 
 void setAuthor( std::string  &val){  data.author=val;} 
 void setAuthor(std::string_view val){  data.author=val;} 

 std::string  getAddip(){  return data.addip; } 
 std::string & getRefAddip(){  return std::ref(data.addip); } 
 void setAddip( std::string  &val){  data.addip=val;} 
 void setAddip(std::string_view val){  data.addip=val;} 

 std::string  getCreatetime(){  return data.createtime; } 
 std::string & getRefCreatetime(){  return std::ref(data.createtime); } 
 void setCreatetime( std::string  &val){  data.createtime=val;} 
 void setCreatetime(std::string_view val){  data.createtime=val;} 

 unsigned  long long  getAddtime(){  return data.addtime; } 
 void setAddtime( unsigned  long long  val){  data.addtime=val;} 

 int  getReadnum(){  return data.readnum; } 
 void setReadnum( int  val){  data.readnum=val;} 

 int  getReview(){  return data.review; } 
 void setReview( int  val){  data.review=val;} 

 std::string  getIcoimg(){  return data.icoimg; } 
 std::string & getRefIcoimg(){  return std::ref(data.icoimg); } 
 void setIcoimg( std::string  &val){  data.icoimg=val;} 
 void setIcoimg(std::string_view val){  data.icoimg=val;} 

 std::string  getContent(){  return data.content; } 
 std::string & getRefContent(){  return std::ref(data.content); } 
 void setContent( std::string  &val){  data.content=val;} 
 void setContent(std::string_view val){  data.content=val;} 

 std::string  getMdcontent(){  return data.mdcontent; } 
 std::string & getRefMdcontent(){  return std::ref(data.mdcontent); } 
 void setMdcontent( std::string  &val){  data.mdcontent=val;} 
 void setMdcontent(std::string_view val){  data.mdcontent=val;} 

 char  getIsopen(){  return data.isopen; } 
 void setIsopen( char  val){  data.isopen=val;} 

 char  getIshome(){  return data.ishome; } 
 void setIshome( char  val){  data.ishome=val;} 

 char  getIscomment(){  return data.iscomment; } 
 void setIscomment( char  val){  data.iscomment=val;} 

 char  getShowtype(){  return data.showtype; } 
 void setShowtype( char  val){  data.showtype=val;} 

 std::string  getFromlocal(){  return data.fromlocal; } 
 std::string & getRefFromlocal(){  return std::ref(data.fromlocal); } 
 void setFromlocal( std::string  &val){  data.fromlocal=val;} 
 void setFromlocal(std::string_view val){  data.fromlocal=val;} 

 std::string  getTexturl(){  return data.texturl; } 
 std::string & getRefTexturl(){  return std::ref(data.texturl); } 
 void setTexturl( std::string  &val){  data.texturl=val;} 
 void setTexturl(std::string_view val){  data.texturl=val;} 

 std::string  getSummary(){  return data.summary; } 
 std::string & getRefSummary(){  return std::ref(data.summary); } 
 void setSummary( std::string  &val){  data.summary=val;} 
 void setSummary(std::string_view val){  data.summary=val;} 

 std::string  getEditauthor(){  return data.editauthor; } 
 std::string & getRefEditauthor(){  return std::ref(data.editauthor); } 
 void setEditauthor( std::string  &val){  data.editauthor=val;} 
 void setEditauthor(std::string_view val){  data.editauthor=val;} 

 std::string  getRelatecontent(){  return data.relatecontent; } 
 std::string & getRefRelatecontent(){  return std::ref(data.relatecontent); } 
 void setRelatecontent( std::string  &val){  data.relatecontent=val;} 
 void setRelatecontent(std::string_view val){  data.relatecontent=val;} 

article_info::meta getnewData(){
 	 struct article_info::meta newdata;
	 return newdata; 
} 
article_info::meta getData(){
 	 return data; 
} 
std::vector<article_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<article_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<article_info::col_names.size();jj++){
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
if(tree_data[n].aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(tree_data[n].aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(tree_data[n].classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(tree_data[n].userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topicname\":\""<<http::utf8_to_jsonstring(tree_data[n].topicname)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(tree_data[n].keywords)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<http::utf8_to_jsonstring(tree_data[n].fromsource)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<http::utf8_to_jsonstring(tree_data[n].author)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(tree_data[n].addip)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<http::utf8_to_jsonstring(tree_data[n].createtime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(tree_data[n].addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(tree_data[n].readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(tree_data[n].review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<http::utf8_to_jsonstring(tree_data[n].icoimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mdcontent\":\""<<http::utf8_to_jsonstring(tree_data[n].mdcontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(tree_data[n].ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(tree_data[n].iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(tree_data[n].showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<http::utf8_to_jsonstring(tree_data[n].fromlocal)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<http::utf8_to_jsonstring(tree_data[n].texturl)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<http::utf8_to_jsonstring(tree_data[n].summary)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<http::utf8_to_jsonstring(tree_data[n].editauthor)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"relatecontent\":\""<<http::utf8_to_jsonstring(tree_data[n].relatecontent)<<"\"";
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
   
   std::string tree_tojson(const std::vector<article_info::meta_tree> &tree_data,std::function<bool(std::string&,const article_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<article_info::col_names.size();jj++){
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
if(tree_data[n].aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(tree_data[n].aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(tree_data[n].classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(tree_data[n].userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topicname\":\""<<http::utf8_to_jsonstring(tree_data[n].topicname)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(tree_data[n].keywords)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<http::utf8_to_jsonstring(tree_data[n].fromsource)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<http::utf8_to_jsonstring(tree_data[n].author)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(tree_data[n].addip)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<http::utf8_to_jsonstring(tree_data[n].createtime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(tree_data[n].addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(tree_data[n].readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(tree_data[n].review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<http::utf8_to_jsonstring(tree_data[n].icoimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mdcontent\":\""<<http::utf8_to_jsonstring(tree_data[n].mdcontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(tree_data[n].ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(tree_data[n].iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(tree_data[n].showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<http::utf8_to_jsonstring(tree_data[n].fromlocal)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<http::utf8_to_jsonstring(tree_data[n].texturl)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<http::utf8_to_jsonstring(tree_data[n].summary)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<http::utf8_to_jsonstring(tree_data[n].editauthor)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"relatecontent\":\""<<http::utf8_to_jsonstring(tree_data[n].relatecontent)<<"\"";
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
   
    template<article_info::cols KeyCol, article_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>()));
        using ValType = decltype(article_info::getField<ValCol>(std::declval<const article_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(article_info::getField<KeyCol>(iter), article_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<article_info::cols KeyCol, article_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>())), 
            decltype(article_info::getField<ValCol>(std::declval<const article_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>())), 
                    decltype(article_info::getField<ValCol>(std::declval<const article_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>()));
        using ValType = decltype(article_info::getField<ValCol>(std::declval<const article_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(article_info::getField<KeyCol>(iter), article_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(article_info::getField<KeyCol>(iter), article_info::getField<ValCol>(iter))) {
                    result.emplace(article_info::getField<KeyCol>(iter), article_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<article_info::cols KeyCol, article_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>()));
        using ValType = decltype(article_info::getField<ValCol>(std::declval<const article_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(article_info::getField<KeyCol>(iter), article_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<article_info::cols KeyCol, article_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>())), 
            decltype(article_info::getField<ValCol>(std::declval<const article_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>())), 
                    decltype(article_info::getField<ValCol>(std::declval<const article_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>()));
        using ValType = decltype(article_info::getField<ValCol>(std::declval<const article_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(article_info::getField<KeyCol>(iter), article_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(article_info::getField<KeyCol>(iter), article_info::getField<ValCol>(iter))) {
                    result.emplace_back(article_info::getField<KeyCol>(iter), article_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<article_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(article_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<article_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(article_info::getField<KeyCol>(std::declval<const article_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(article_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(article_info::getField<KeyCol>(iter))) {
                    result.emplace_back(article_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<article_info::cols Col>
        requires requires(std::ostream& os, decltype(article_info::getField<Col>(std::declval<const article_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << article_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<article_info::cols Col>
        requires requires(std::ostream& os, decltype(article_info::getField<Col>(std::declval<const article_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << article_info::getField<Col>(iter); 
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
   