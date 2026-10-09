#ifndef ORM_CMS_ORDERLISTBASEMATA_H
#define ORM_CMS_ORDERLISTBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Thu, 08 Oct 2026 06:25:06 GMT
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
   
     namespace cms { 

namespace orderlist_info
{
 
    static constexpr std::size_t col_count = 31;
    enum class cols : unsigned char 
    {
		orderid = 0,
		userid = 1,
		wxid = 2,
		product_trade = 3,
		paytype = 4,
		openid = 5,
		wxorder = 6,
		storeorder = 7,
		totalnum = 8,
		addtime = 9,
		paytime = 10,
		payip = 11,
		username = 12,
		address = 13,
		mobile = 14,
		payprice = 15,
		shipprice = 16,
		isrefund = 17,
		isship = 18,
		isfinsh = 19,
		isremove = 20,
		refundnum = 21,
		paytitle = 22,
		content = 23,
		liuyan = 24,
		excomany = 25,
		exnumber = 26,
		expaddtime = 27,
		goodsid = 28,
		status = 29,
		jifen = 30,

    };

    struct meta
    {
		 int  orderid = 0; ///**/
		 int  userid = 0; ///**/
		 int  wxid = 0; ///**/
		 std::string  product_trade = ""; ///*product id*/
		 unsigned  char  paytype = 0; ///*支付渠道 0=未标记 1=alipay 2=wxpayv2 3=wxpayv3 4=weixinxcx*/
		 std::string  openid = ""; ///**/
		 std::string  wxorder = ""; ///*out order*/
		 std::string  storeorder = ""; ///*in order*/
		 unsigned  int  totalnum = 0; ///**/
		 unsigned  int  addtime = 0; ///**/
		 unsigned  int  paytime = 0; ///**/
		 std::string  payip = ""; ///**/
		 std::string  username = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  mobile = ""; ///**/
		 unsigned  int  payprice = 0; ///*cents*/
		 unsigned  int  shipprice = 0; ///*cents*/
		 char  isrefund = 0; ///**/
		 char  isship = 0; ///**/
		 char  isfinsh = 0; ///**/
		 unsigned  char  isremove = 0; ///*用户删除，后台不删除*/
		 unsigned  int  refundnum = 0; ///**/
		 std::string  paytitle = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  liuyan = ""; ///**/
		 std::string  excomany = ""; ///*快递公司*/
		 std::string  exnumber = ""; ///*快递单号*/
		 unsigned  int  expaddtime = 0; ///*快递添加时间*/
		 unsigned  int  goodsid = 0; ///*product id*/
		 int  status = 0; ///*流转状态*/
		 unsigned  int  jifen = 0; ///**/
	};
  
    struct meta_tree
    {
		 int  orderid = 0; ///**/
		 int  userid = 0; ///**/
		 int  wxid = 0; ///**/
		 std::string  product_trade = ""; ///*product id*/
		 unsigned  char  paytype = 0; ///*支付渠道 0=未标记 1=alipay 2=wxpayv2 3=wxpayv3 4=weixinxcx*/
		 std::string  openid = ""; ///**/
		 std::string  wxorder = ""; ///*out order*/
		 std::string  storeorder = ""; ///*in order*/
		 unsigned  int  totalnum = 0; ///**/
		 unsigned  int  addtime = 0; ///**/
		 unsigned  int  paytime = 0; ///**/
		 std::string  payip = ""; ///**/
		 std::string  username = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  mobile = ""; ///**/
		 unsigned  int  payprice = 0; ///*cents*/
		 unsigned  int  shipprice = 0; ///*cents*/
		 char  isrefund = 0; ///**/
		 char  isship = 0; ///**/
		 char  isfinsh = 0; ///**/
		 unsigned  char  isremove = 0; ///*用户删除，后台不删除*/
		 unsigned  int  refundnum = 0; ///**/
		 std::string  paytitle = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  liuyan = ""; ///**/
		 std::string  excomany = ""; ///*快递公司*/
		 std::string  exnumber = ""; ///*快递单号*/
		 unsigned  int  expaddtime = 0; ///*快递添加时间*/
		 unsigned  int  goodsid = 0; ///*product id*/
		 int  status = 0; ///*流转状态*/
		 unsigned  int  jifen = 0; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 int  orderid = 0; ///**/
		 int  userid = 0; ///**/
		 int  wxid = 0; ///**/
		 std::string  product_trade = ""; ///*product id*/
		 unsigned  char  paytype = 0; ///*支付渠道 0=未标记 1=alipay 2=wxpayv2 3=wxpayv3 4=weixinxcx*/
		 std::string  openid = ""; ///**/
		 std::string  wxorder = ""; ///*out order*/
		 std::string  storeorder = ""; ///*in order*/
		 unsigned  int  totalnum = 0; ///**/
		 unsigned  int  addtime = 0; ///**/
		 unsigned  int  paytime = 0; ///**/
		 std::string  payip = ""; ///**/
		 std::string  username = ""; ///**/
		 std::string  address = ""; ///**/
		 std::string  mobile = ""; ///**/
		 unsigned  int  payprice = 0; ///*cents*/
		 unsigned  int  shipprice = 0; ///*cents*/
		 char  isrefund = 0; ///**/
		 char  isship = 0; ///**/
		 char  isfinsh = 0; ///**/
		 unsigned  char  isremove = 0; ///*用户删除，后台不删除*/
		 unsigned  int  refundnum = 0; ///**/
		 std::string  paytitle = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  liuyan = ""; ///**/
		 std::string  excomany = ""; ///*快递公司*/
		 std::string  exnumber = ""; ///*快递单号*/
		 unsigned  int  expaddtime = 0; ///*快递添加时间*/
		 unsigned  int  goodsid = 0; ///*product id*/
		 int  status = 0; ///*流转状态*/
		 unsigned  int  jifen = 0; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::orderid) { 
		 return m.orderid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::wxid) { 
		 return m.wxid;
		} else if constexpr (Col == cols::product_trade) { 
		 return m.product_trade;
		} else if constexpr (Col == cols::paytype) { 
		 return m.paytype;
		} else if constexpr (Col == cols::openid) { 
		 return m.openid;
		} else if constexpr (Col == cols::wxorder) { 
		 return m.wxorder;
		} else if constexpr (Col == cols::storeorder) { 
		 return m.storeorder;
		} else if constexpr (Col == cols::totalnum) { 
		 return m.totalnum;
		} else if constexpr (Col == cols::addtime) { 
		 return m.addtime;
		} else if constexpr (Col == cols::paytime) { 
		 return m.paytime;
		} else if constexpr (Col == cols::payip) { 
		 return m.payip;
		} else if constexpr (Col == cols::username) { 
		 return m.username;
		} else if constexpr (Col == cols::address) { 
		 return m.address;
		} else if constexpr (Col == cols::mobile) { 
		 return m.mobile;
		} else if constexpr (Col == cols::payprice) { 
		 return m.payprice;
		} else if constexpr (Col == cols::shipprice) { 
		 return m.shipprice;
		} else if constexpr (Col == cols::isrefund) { 
		 return m.isrefund;
		} else if constexpr (Col == cols::isship) { 
		 return m.isship;
		} else if constexpr (Col == cols::isfinsh) { 
		 return m.isfinsh;
		} else if constexpr (Col == cols::isremove) { 
		 return m.isremove;
		} else if constexpr (Col == cols::refundnum) { 
		 return m.refundnum;
		} else if constexpr (Col == cols::paytitle) { 
		 return m.paytitle;
		} else if constexpr (Col == cols::content) { 
		 return m.content;
		} else if constexpr (Col == cols::liuyan) { 
		 return m.liuyan;
		} else if constexpr (Col == cols::excomany) { 
		 return m.excomany;
		} else if constexpr (Col == cols::exnumber) { 
		 return m.exnumber;
		} else if constexpr (Col == cols::expaddtime) { 
		 return m.expaddtime;
		} else if constexpr (Col == cols::goodsid) { 
		 return m.goodsid;
		} else if constexpr (Col == cols::status) { 
		 return m.status;
		} else if constexpr (Col == cols::jifen) { 
		 return m.jifen;
		
        } else {
            //static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using orderid =  int ;
		using userid =  int ;
		using wxid =  int ;
		using product_trade =  std::string ;
		using paytype =  unsigned  char ;
		using openid =  std::string ;
		using wxorder =  std::string ;
		using storeorder =  std::string ;
		using totalnum =  unsigned  int ;
		using addtime =  unsigned  int ;
		using paytime =  unsigned  int ;
		using payip =  std::string ;
		using username =  std::string ;
		using address =  std::string ;
		using mobile =  std::string ;
		using payprice =  unsigned  int ;
		using shipprice =  unsigned  int ;
		using isrefund =  char ;
		using isship =  char ;
		using isfinsh =  char ;
		using isremove =  unsigned  char ;
		using refundnum =  unsigned  int ;
		using paytitle =  std::string ;
		using content =  std::string ;
		using liuyan =  std::string ;
		using excomany =  std::string ;
		using exnumber =  std::string ;
		using expaddtime =  unsigned  int ;
		using goodsid =  unsigned  int ;
		using status =  int ;
		using jifen =  unsigned  int ;

    }

    
    #define ORM_CMS_ORDERLIST_EXPAND(x) x 
    
    #define ORM_CMS_ORDERLIST_META_FIELD_TYPE(col) \
         orm::cms::orderlist_info::type::col 
    
    #define ORM_CMS_ORDERLIST_PROJ_MEMBER(col) \
          ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_1(c1) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_1( c1)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c2))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c3))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c4))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c5))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c6))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c7))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c8))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c9))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c10))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c11))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c12))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c13))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c14))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c15))
         
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBER(c16))
         
    #define ORM_CMS_ORDERLIST_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_ORDERLIST_PROJ_MEMBERS(...) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_16, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_15, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_14, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_13, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_12, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_11, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_10, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_9, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_8, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_7, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_6, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_5, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_4, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_3, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_2, \
            ORM_CMS_ORDERLIST_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_ORDERLIST_COUNT(...) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_ORDERLIST_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_ORDERLIST_TO_JSON_1(c1) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_ORDERLIST_TO_JSON_2(c1,c2) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_ORDERLIST_TO_JSON_BODY(...) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_ORDERLIST_TO_JSON_16,ORM_CMS_ORDERLIST_TO_JSON_15,ORM_CMS_ORDERLIST_TO_JSON_14,ORM_CMS_ORDERLIST_TO_JSON_13,ORM_CMS_ORDERLIST_TO_JSON_12,ORM_CMS_ORDERLIST_TO_JSON_11,ORM_CMS_ORDERLIST_TO_JSON_10,ORM_CMS_ORDERLIST_TO_JSON_9,ORM_CMS_ORDERLIST_TO_JSON_8,ORM_CMS_ORDERLIST_TO_JSON_7,ORM_CMS_ORDERLIST_TO_JSON_6,ORM_CMS_ORDERLIST_TO_JSON_5,ORM_CMS_ORDERLIST_TO_JSON_4,ORM_CMS_ORDERLIST_TO_JSON_3,ORM_CMS_ORDERLIST_TO_JSON_2,ORM_CMS_ORDERLIST_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_ORDERLIST_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_1(n1)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_1(n1)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_ORDERLIST_CAT(a, b) ORM_CMS_ORDERLIST_CAT_(a, b)
    #define ORM_CMS_ORDERLIST_CAT_(a, b) a##b

    #define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_ORDERLIST_CAT(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_ORDERLIST_TO_JSON_CUSTOM(...) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_ORDERLIST_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_ORDERLIST_SET_VAL_1(c1) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_ORDERLIST_SET_VAL_2(c1,c2) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_1(c1)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_2(c1,c2)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_ORDERLIST_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_ORDERLIST_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_ORDERLIST_CAT(ORM_CMS_ORDERLIST_SET_VAL_, N)
    
    
    #define ORM_CMS_ORDERLIST_SET_VAL_FIELDS(...) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_ORDERLIST_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_FIELDS(ORM_CMS_ORDERLIST_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_ORDERLIST_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::orderlist_info { \
            struct StructName { \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_ORDERLIST_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::orderlist_info { \
            struct StructName { \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM(ORM_CMS_ORDERLIST_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_ORDERLIST_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::orderlist_info { \
            struct StructName { \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_ORDERLIST_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::orderlist_info { \
            struct StructName { \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_ORDERLIST_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::orderlist_info { \
            struct StructName { \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_TO_JSON_CUSTOM(ORM_CMS_ORDERLIST_UNWRAP CustomNames));  \
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
                        ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_ORDERLIST_EXPAND(ORM_CMS_ORDERLIST_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    inline constexpr std::array<std::string_view,31> col_names={"orderid","userid","wxid","product_trade","paytype","openid","wxorder","storeorder","totalnum","addtime","paytime","payip","username","address","mobile","payprice","shipprice","isrefund","isship","isfinsh","isremove","refundnum","paytitle","content","liuyan","excomany","exnumber","expaddtime","goodsid","status","jifen"};
	static constexpr std::array<unsigned char,31> col_types={3,3,3,253,1,253,253,253,3,3,3,253,253,253,253,3,3,1,1,1,1,3,253,252,252,253,253,3,3,3,3};
	static constexpr std::array<unsigned short,31> col_length={0,0,0,64,0,64,64,64,0,0,0,60,60,120,60,0,0,0,0,0,0,0,255,0,0,60,40,0,0,0,0};
	static constexpr std::array<unsigned char,31> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static constexpr std::array<bool,31> col_null={false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
	static constexpr std::array<bool,31> col_indexed={true,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
	static constexpr std::string_view auto_pk_name ="orderid";
	static constexpr int auto_pk_index = 0;

}

struct orderlist_base
{
    using cols = orderlist_info::cols;
      orderlist_info::meta data;
    std::vector<orderlist_info::meta> record;
static constexpr std::string_view _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<orderlist_info::meta>::iterator begin(){     return record.begin(); }
std::vector<orderlist_info::meta>::iterator end(){     return record.end(); }
std::vector<orderlist_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<orderlist_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="orderlist";
static constexpr std::string_view org_tablename="orderlist";
static constexpr std::string_view modelname="Orderlist";
	static constexpr std::array<bool,31> col_need_quote={false,false,false,true,false,true,true,true,false,false,false,true,true,true,true,false,false,false,false,false,false,false,true,true,true,true,true,false,false,false,false};

            std::bitset<31> dirty_bits;
            void clear_dirty() noexcept {
                dirty_bits.reset();
            }

            void set_dirty(std::size_t idx) noexcept {
                if(idx < 31)
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
                        result.push_back(orderlist_info::col_names[i]);
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
         char colpospppc;

	         if(bi<91&&bi>64){
	         bi+=32;
	         }
	         switch(bi){

         case 'a':
 switch(coln.size()){  
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 9; }
 if(colpospppc=='s'){ return 13; }
   	 break;
 }
 break;
case 'c':
   	 return 23;
break;
case 'e':
 switch(coln.size()){  
case 8:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='r'){ return 26; }
 if(colpospppc=='y'){ return 25; }
   	 break;
case 10:
   	 return 27;
break;
 }
 break;
case 'g':
   	 return 28;
break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 18;
break;
case 7:
   	 return 19;
break;
case 8:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 17; }
 if(colpospppc=='e'){ return 20; }
   	 break;
 }
 break;
case 'j':
   	 return 30;
break;
case 'l':
   	 return 24;
break;
case 'm':
   	 return 14;
break;
case 'o':
 switch(coln.size()){  
case 6:
   	 return 5;
break;
case 7:
   	 return 0;
break;
 }
 break;
case 'p':
 switch(coln.size()){  
case 5:
   	 return 11;
break;
case 7:
 if(coln.size()>4&&(coln[4]=='i'||coln[4]=='I')){ return 10; }
 if(coln.size()>4&&(coln[4]=='y'||coln[4]=='Y')){ return 4; }
   	 break;
case 8:
 if(coln.size()>3&&(coln[3]=='p'||coln[3]=='P')){ return 15; }
 if(coln.size()>3&&(coln[3]=='t'||coln[3]=='T')){ return 22; }
   	 break;
case 13:
   	 return 3;
break;
 }
 break;
case 'r':
   	 return 21;
break;
case 's':
 switch(coln.size()){  
case 6:
   	 return 29;
break;
case 9:
   	 return 16;
break;
case 10:
   	 return 7;
break;
 }
 break;
case 't':
   	 return 8;
break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 1;
break;
case 8:
   	 return 12;
break;
 }
 break;
case 'w':
 switch(coln.size()){  
case 4:
   	 return 2;
break;
case 7:
   	 return 6;
break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "orderid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     orderlist_info::meta metatemp;    
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
        for(;j<orderlist_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                   // tempsql<<"`";
                }
                tempsql<<orderlist_info::col_names[j];
        }
        if(j>0){
            //tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.orderid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.orderid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.wxid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.wxid);
}
tempsql<<",'"<<stringaddslash(data.product_trade)<<"'";
if(data.paytype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.paytype);
}
tempsql<<",'"<<stringaddslash(data.openid)<<"'";
tempsql<<",'"<<stringaddslash(data.wxorder)<<"'";
tempsql<<",'"<<stringaddslash(data.storeorder)<<"'";
if(data.totalnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.totalnum);
}
if(data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.addtime);
}
if(data.paytime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.paytime);
}
tempsql<<",'"<<stringaddslash(data.payip)<<"'";
tempsql<<",'"<<stringaddslash(data.username)<<"'";
tempsql<<",'"<<stringaddslash(data.address)<<"'";
tempsql<<",'"<<stringaddslash(data.mobile)<<"'";
if(data.payprice==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.payprice);
}
if(data.shipprice==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.shipprice);
}
if(data.isrefund==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isrefund);
}
if(data.isship==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isship);
}
if(data.isfinsh==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isfinsh);
}
if(data.isremove==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isremove);
}
if(data.refundnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.refundnum);
}
tempsql<<",'"<<stringaddslash(data.paytitle)<<"'";
tempsql<<",'"<<stringaddslash(data.content)<<"'";
tempsql<<",'"<<stringaddslash(data.liuyan)<<"'";
tempsql<<",'"<<stringaddslash(data.excomany)<<"'";
tempsql<<",'"<<stringaddslash(data.exnumber)<<"'";
if(data.expaddtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.expaddtime);
}
if(data.goodsid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.goodsid);
}
if(data.status==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.status);
}
if(data.jifen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.jifen);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string make_data_insert_sql(const orderlist_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<orderlist_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                    //tempsql<<"`";
                }
                tempsql<<orderlist_info::col_names[j];
        }
        if(j>0){
           // tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.orderid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.orderid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.wxid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.wxid);
}
tempsql<<",'"<<stringaddslash(insert_data.product_trade)<<"'";
if(insert_data.paytype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.paytype);
}
tempsql<<",'"<<stringaddslash(insert_data.openid)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.wxorder)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.storeorder)<<"'";
if(insert_data.totalnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.totalnum);
}
if(insert_data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.addtime);
}
if(insert_data.paytime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.paytime);
}
tempsql<<",'"<<stringaddslash(insert_data.payip)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.username)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.address)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.mobile)<<"'";
if(insert_data.payprice==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.payprice);
}
if(insert_data.shipprice==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.shipprice);
}
if(insert_data.isrefund==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isrefund);
}
if(insert_data.isship==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isship);
}
if(insert_data.isfinsh==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isfinsh);
}
if(insert_data.isremove==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isremove);
}
if(insert_data.refundnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.refundnum);
}
tempsql<<",'"<<stringaddslash(insert_data.paytitle)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.content)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.liuyan)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.excomany)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.exnumber)<<"'";
if(insert_data.expaddtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.expaddtime);
}
if(insert_data.goodsid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.goodsid);
}
if(insert_data.status==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.status);
}
if(insert_data.jifen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.jifen);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string make_vector_insert_sql(const std::vector<orderlist_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<orderlist_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                   // tempsql<<"`";
                }
                tempsql<<orderlist_info::col_names[j];
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

            	if(insert_data[i].orderid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].orderid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].wxid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].wxid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].product_trade)<<"'";
	if(insert_data[i].paytype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].paytype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].openid)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].wxorder)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].storeorder)<<"'";
	if(insert_data[i].totalnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].totalnum);
	}
	if(insert_data[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].addtime);
	}
	if(insert_data[i].paytime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].paytime);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].payip)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].username)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].address)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].mobile)<<"'";
	if(insert_data[i].payprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].payprice);
	}
	if(insert_data[i].shipprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].shipprice);
	}
	if(insert_data[i].isrefund==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isrefund);
	}
	if(insert_data[i].isship==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isship);
	}
	if(insert_data[i].isfinsh==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isfinsh);
	}
	if(insert_data[i].isremove==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isremove);
	}
	if(insert_data[i].refundnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].refundnum);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].paytitle)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].content)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].liuyan)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].excomany)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].exnumber)<<"'";
	if(insert_data[i].expaddtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].expaddtime);
	}
	if(insert_data[i].goodsid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].goodsid);
	}
	if(insert_data[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].status);
	}
	if(insert_data[i].jifen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].jifen);
	}
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

        if(data.orderid==0){
	tempsql<<"orderid=0";
 }else{ 
	tempsql<<"orderid="<<std::to_string(data.orderid);
}
if(data.userid==0){
	tempsql<<",userid=0";
 }else{ 
	tempsql<<",userid="<<std::to_string(data.userid);
}
if(data.wxid==0){
	tempsql<<",wxid=0";
 }else{ 
	tempsql<<",wxid="<<std::to_string(data.wxid);
}
tempsql<<",product_trade='"<<stringaddslash(data.product_trade)<<"'";
if(data.paytype==0){
	tempsql<<",paytype=0";
 }else{ 
	tempsql<<",paytype="<<std::to_string(data.paytype);
}
tempsql<<",openid='"<<stringaddslash(data.openid)<<"'";
tempsql<<",wxorder='"<<stringaddslash(data.wxorder)<<"'";
tempsql<<",storeorder='"<<stringaddslash(data.storeorder)<<"'";
if(data.totalnum==0){
	tempsql<<",totalnum=0";
 }else{ 
	tempsql<<",totalnum="<<std::to_string(data.totalnum);
}
if(data.addtime==0){
	tempsql<<",addtime=0";
 }else{ 
	tempsql<<",addtime="<<std::to_string(data.addtime);
}
if(data.paytime==0){
	tempsql<<",paytime=0";
 }else{ 
	tempsql<<",paytime="<<std::to_string(data.paytime);
}
tempsql<<",payip='"<<stringaddslash(data.payip)<<"'";
tempsql<<",username='"<<stringaddslash(data.username)<<"'";
tempsql<<",address='"<<stringaddslash(data.address)<<"'";
tempsql<<",mobile='"<<stringaddslash(data.mobile)<<"'";
if(data.payprice==0){
	tempsql<<",payprice=0";
 }else{ 
	tempsql<<",payprice="<<std::to_string(data.payprice);
}
if(data.shipprice==0){
	tempsql<<",shipprice=0";
 }else{ 
	tempsql<<",shipprice="<<std::to_string(data.shipprice);
}
if(data.isrefund==0){
	tempsql<<",isrefund=0";
 }else{ 
	tempsql<<",isrefund="<<std::to_string(data.isrefund);
}
if(data.isship==0){
	tempsql<<",isship=0";
 }else{ 
	tempsql<<",isship="<<std::to_string(data.isship);
}
if(data.isfinsh==0){
	tempsql<<",isfinsh=0";
 }else{ 
	tempsql<<",isfinsh="<<std::to_string(data.isfinsh);
}
if(data.isremove==0){
	tempsql<<",isremove=0";
 }else{ 
	tempsql<<",isremove="<<std::to_string(data.isremove);
}
if(data.refundnum==0){
	tempsql<<",refundnum=0";
 }else{ 
	tempsql<<",refundnum="<<std::to_string(data.refundnum);
}
tempsql<<",paytitle='"<<stringaddslash(data.paytitle)<<"'";
tempsql<<",content='"<<stringaddslash(data.content)<<"'";
tempsql<<",liuyan='"<<stringaddslash(data.liuyan)<<"'";
tempsql<<",excomany='"<<stringaddslash(data.excomany)<<"'";
tempsql<<",exnumber='"<<stringaddslash(data.exnumber)<<"'";
if(data.expaddtime==0){
	tempsql<<",expaddtime=0";
 }else{ 
	tempsql<<",expaddtime="<<std::to_string(data.expaddtime);
}
if(data.goodsid==0){
	tempsql<<",goodsid=0";
 }else{ 
	tempsql<<",goodsid="<<std::to_string(data.goodsid);
}
if(data.status==0){
	tempsql<<",status=0";
 }else{ 
	tempsql<<",status="<<std::to_string(data.status);
}
if(data.jifen==0){
	tempsql<<",jifen=0";
 }else{ 
	tempsql<<",jifen="<<std::to_string(data.jifen);
}
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
if(data.orderid==0){
	tempsql<<"orderid=0";
 }else{ 
	tempsql<<"orderid="<<std::to_string(data.orderid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"userid=0";
 }else{ 
	tempsql<<"userid="<<std::to_string(data.userid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.wxid==0){
	tempsql<<"wxid=0";
 }else{ 
	tempsql<<"wxid="<<std::to_string(data.wxid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"product_trade='"<<stringaddslash(data.product_trade)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.paytype==0){
	tempsql<<"paytype=0";
 }else{ 
	tempsql<<"paytype="<<std::to_string(data.paytype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"openid='"<<stringaddslash(data.openid)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"wxorder='"<<stringaddslash(data.wxorder)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"storeorder='"<<stringaddslash(data.storeorder)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.totalnum==0){
	tempsql<<"totalnum=0";
 }else{ 
	tempsql<<"totalnum="<<std::to_string(data.totalnum);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"addtime=0";
 }else{ 
	tempsql<<"addtime="<<std::to_string(data.addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.paytime==0){
	tempsql<<"paytime=0";
 }else{ 
	tempsql<<"paytime="<<std::to_string(data.paytime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"payip='"<<stringaddslash(data.payip)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"username='"<<stringaddslash(data.username)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"address='"<<stringaddslash(data.address)<<"'";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"mobile='"<<stringaddslash(data.mobile)<<"'";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(data.payprice==0){
	tempsql<<"payprice=0";
 }else{ 
	tempsql<<"payprice="<<std::to_string(data.payprice);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(data.shipprice==0){
	tempsql<<"shipprice=0";
 }else{ 
	tempsql<<"shipprice="<<std::to_string(data.shipprice);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(data.isrefund==0){
	tempsql<<"isrefund=0";
 }else{ 
	tempsql<<"isrefund="<<std::to_string(data.isrefund);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(data.isship==0){
	tempsql<<"isship=0";
 }else{ 
	tempsql<<"isship="<<std::to_string(data.isship);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(data.isfinsh==0){
	tempsql<<"isfinsh=0";
 }else{ 
	tempsql<<"isfinsh="<<std::to_string(data.isfinsh);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.isremove==0){
	tempsql<<"isremove=0";
 }else{ 
	tempsql<<"isremove="<<std::to_string(data.isremove);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.refundnum==0){
	tempsql<<"refundnum=0";
 }else{ 
	tempsql<<"refundnum="<<std::to_string(data.refundnum);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"paytitle='"<<stringaddslash(data.paytitle)<<"'";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"content='"<<stringaddslash(data.content)<<"'";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"liuyan='"<<stringaddslash(data.liuyan)<<"'";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"excomany='"<<stringaddslash(data.excomany)<<"'";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"exnumber='"<<stringaddslash(data.exnumber)<<"'";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
if(data.expaddtime==0){
	tempsql<<"expaddtime=0";
 }else{ 
	tempsql<<"expaddtime="<<std::to_string(data.expaddtime);
}
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
if(data.goodsid==0){
	tempsql<<"goodsid=0";
 }else{ 
	tempsql<<"goodsid="<<std::to_string(data.goodsid);
}
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(data.status==0){
	tempsql<<"status=0";
 }else{ 
	tempsql<<"status="<<std::to_string(data.status);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(data.jifen==0){
	tempsql<<"jifen=0";
 }else{ 
	tempsql<<"jifen="<<std::to_string(data.jifen);
}
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

    constexpr std::size_t total = orderlist_info::col_names.size();

    bool first = true;
    for (std::size_t idx = 0; idx < total; ++idx) {
        if (dirty_bits.test(idx)) {
            if (idx < total) {
                if (!first) tempsql << ",";
                switch (idx) {
                    case 0:
                        if(data.orderid==0){
                            tempsql<<"orderid=0";
                        }else{ 
                            tempsql<<"orderid="<<std::to_string(data.orderid);
                        }
                        break;
                    case 1:
                        if(data.userid==0){
                            tempsql<<"userid=0";
                        }else{ 
                            tempsql<<"userid="<<std::to_string(data.userid);
                        }
                        break;
                    case 2:
                        if(data.wxid==0){
                            tempsql<<"wxid=0";
                        }else{ 
                            tempsql<<"wxid="<<std::to_string(data.wxid);
                        }
                        break;
                    case 3:
                        tempsql<<"product_trade='"<<stringaddslash(data.product_trade)<<"'";
                        break;
                    case 4:
                        if(data.paytype==0){
                            tempsql<<"paytype=0";
                        }else{ 
                            tempsql<<"paytype="<<std::to_string(data.paytype);
                        }
                        break;
                    case 5:
                        tempsql<<"openid='"<<stringaddslash(data.openid)<<"'";
                        break;
                    case 6:
                        tempsql<<"wxorder='"<<stringaddslash(data.wxorder)<<"'";
                        break;
                    case 7:
                        tempsql<<"storeorder='"<<stringaddslash(data.storeorder)<<"'";
                        break;
                    case 8:
                        if(data.totalnum==0){
                            tempsql<<"totalnum=0";
                        }else{ 
                            tempsql<<"totalnum="<<std::to_string(data.totalnum);
                        }
                        break;
                    case 9:
                        if(data.addtime==0){
                            tempsql<<"addtime=0";
                        }else{ 
                            tempsql<<"addtime="<<std::to_string(data.addtime);
                        }
                        break;
                    case 10:
                        if(data.paytime==0){
                            tempsql<<"paytime=0";
                        }else{ 
                            tempsql<<"paytime="<<std::to_string(data.paytime);
                        }
                        break;
                    case 11:
                        tempsql<<"payip='"<<stringaddslash(data.payip)<<"'";
                        break;
                    case 12:
                        tempsql<<"username='"<<stringaddslash(data.username)<<"'";
                        break;
                    case 13:
                        tempsql<<"address='"<<stringaddslash(data.address)<<"'";
                        break;
                    case 14:
                        tempsql<<"mobile='"<<stringaddslash(data.mobile)<<"'";
                        break;
                    case 15:
                        if(data.payprice==0){
                            tempsql<<"payprice=0";
                        }else{ 
                            tempsql<<"payprice="<<std::to_string(data.payprice);
                        }
                        break;
                    case 16:
                        if(data.shipprice==0){
                            tempsql<<"shipprice=0";
                        }else{ 
                            tempsql<<"shipprice="<<std::to_string(data.shipprice);
                        }
                        break;
                    case 17:
                        if(data.isrefund==0){
                            tempsql<<"isrefund=0";
                        }else{ 
                            tempsql<<"isrefund="<<std::to_string(data.isrefund);
                        }
                        break;
                    case 18:
                        if(data.isship==0){
                            tempsql<<"isship=0";
                        }else{ 
                            tempsql<<"isship="<<std::to_string(data.isship);
                        }
                        break;
                    case 19:
                        if(data.isfinsh==0){
                            tempsql<<"isfinsh=0";
                        }else{ 
                            tempsql<<"isfinsh="<<std::to_string(data.isfinsh);
                        }
                        break;
                    case 20:
                        if(data.isremove==0){
                            tempsql<<"isremove=0";
                        }else{ 
                            tempsql<<"isremove="<<std::to_string(data.isremove);
                        }
                        break;
                    case 21:
                        if(data.refundnum==0){
                            tempsql<<"refundnum=0";
                        }else{ 
                            tempsql<<"refundnum="<<std::to_string(data.refundnum);
                        }
                        break;
                    case 22:
                        tempsql<<"paytitle='"<<stringaddslash(data.paytitle)<<"'";
                        break;
                    case 23:
                        tempsql<<"content='"<<stringaddslash(data.content)<<"'";
                        break;
                    case 24:
                        tempsql<<"liuyan='"<<stringaddslash(data.liuyan)<<"'";
                        break;
                    case 25:
                        tempsql<<"excomany='"<<stringaddslash(data.excomany)<<"'";
                        break;
                    case 26:
                        tempsql<<"exnumber='"<<stringaddslash(data.exnumber)<<"'";
                        break;
                    case 27:
                        if(data.expaddtime==0){
                            tempsql<<"expaddtime=0";
                        }else{ 
                            tempsql<<"expaddtime="<<std::to_string(data.expaddtime);
                        }
                        break;
                    case 28:
                        if(data.goodsid==0){
                            tempsql<<"goodsid=0";
                        }else{ 
                            tempsql<<"goodsid="<<std::to_string(data.goodsid);
                        }
                        break;
                    case 29:
                        if(data.status==0){
                            tempsql<<"status=0";
                        }else{ 
                            tempsql<<"status="<<std::to_string(data.status);
                        }
                        break;
                    case 30:
                        if(data.jifen==0){
                            tempsql<<"jifen=0";
                        }else{ 
                            tempsql<<"jifen="<<std::to_string(data.jifen);
                        }
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
        for (; j < orderlist_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << ",";
            }
            else
            {
                tempsql << "";
            }
            tempsql << orderlist_info::col_names[j];
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
            	if(record[i].orderid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].orderid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].wxid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].wxid);
	}
	tempsql<<",'"<<stringaddslash(record[i].product_trade)<<"'";
	if(record[i].paytype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].paytype);
	}
	tempsql<<",'"<<stringaddslash(record[i].openid)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].wxorder)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].storeorder)<<"'";
	if(record[i].totalnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].totalnum);
	}
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].paytime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].paytime);
	}
	tempsql<<",'"<<stringaddslash(record[i].payip)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].username)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	if(record[i].payprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].payprice);
	}
	if(record[i].shipprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].shipprice);
	}
	if(record[i].isrefund==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isrefund);
	}
	if(record[i].isship==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isship);
	}
	if(record[i].isfinsh==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isfinsh);
	}
	if(record[i].isremove==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isremove);
	}
	if(record[i].refundnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].refundnum);
	}
	tempsql<<",'"<<stringaddslash(record[i].paytitle)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].liuyan)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].excomany)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].exnumber)<<"'";
	if(record[i].expaddtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expaddtime);
	}
	if(record[i].goodsid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].goodsid);
	}
	if(record[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].status);
	}
	if(record[i].jifen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].jifen);
	}
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
        for (; j < orderlist_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << ",";
            }
            else
            {
                tempsql << "";
            }
            tempsql << orderlist_info::col_names[j];
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
            	if(record[i].orderid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].orderid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].wxid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].wxid);
	}
	tempsql<<",'"<<stringaddslash(record[i].product_trade)<<"'";
	if(record[i].paytype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].paytype);
	}
	tempsql<<",'"<<stringaddslash(record[i].openid)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].wxorder)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].storeorder)<<"'";
	if(record[i].totalnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].totalnum);
	}
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].paytime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].paytime);
	}
	tempsql<<",'"<<stringaddslash(record[i].payip)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].username)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	if(record[i].payprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].payprice);
	}
	if(record[i].shipprice==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].shipprice);
	}
	if(record[i].isrefund==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isrefund);
	}
	if(record[i].isship==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isship);
	}
	if(record[i].isfinsh==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isfinsh);
	}
	if(record[i].isremove==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isremove);
	}
	if(record[i].refundnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].refundnum);
	}
	tempsql<<",'"<<stringaddslash(record[i].paytitle)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].liuyan)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].excomany)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].exnumber)<<"'";
	if(record[i].expaddtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expaddtime);
	}
	if(record[i].goodsid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].goodsid);
	}
	if(record[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].status);
	}
	if(record[i].jifen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].jifen);
	}
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
            for(jj=0;jj<orderlist_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.orderid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.orderid));
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
if(data.wxid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.wxid));
}
 break;
 case 3:
	temparray.push_back(data.product_trade);
 break;
 case 4:
if(data.paytype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.paytype));
}
 break;
 case 5:
	temparray.push_back(data.openid);
 break;
 case 6:
	temparray.push_back(data.wxorder);
 break;
 case 7:
	temparray.push_back(data.storeorder);
 break;
 case 8:
if(data.totalnum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.totalnum));
}
 break;
 case 9:
if(data.addtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.addtime));
}
 break;
 case 10:
if(data.paytime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.paytime));
}
 break;
 case 11:
	temparray.push_back(data.payip);
 break;
 case 12:
	temparray.push_back(data.username);
 break;
 case 13:
	temparray.push_back(data.address);
 break;
 case 14:
	temparray.push_back(data.mobile);
 break;
 case 15:
if(data.payprice==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.payprice));
}
 break;
 case 16:
if(data.shipprice==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.shipprice));
}
 break;
 case 17:
if(data.isrefund==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isrefund));
}
 break;
 case 18:
if(data.isship==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isship));
}
 break;
 case 19:
if(data.isfinsh==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isfinsh));
}
 break;
 case 20:
if(data.isremove==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isremove));
}
 break;
 case 21:
if(data.refundnum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.refundnum));
}
 break;
 case 22:
	temparray.push_back(data.paytitle);
 break;
 case 23:
	temparray.push_back(data.content);
 break;
 case 24:
	temparray.push_back(data.liuyan);
 break;
 case 25:
	temparray.push_back(data.excomany);
 break;
 case 26:
	temparray.push_back(data.exnumber);
 break;
 case 27:
if(data.expaddtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expaddtime));
}
 break;
 case 28:
if(data.goodsid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.goodsid));
}
 break;
 case 29:
if(data.status==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.status));
}
 break;
 case 30:
if(data.jifen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.jifen));
}
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
            for(jj=0;jj<orderlist_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.orderid==0){
	tempsql.insert({"orderid","0"});
 }else{ 
	tempsql.insert({"orderid",std::to_string(data.orderid)});
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
if(data.wxid==0){
	tempsql.insert({"wxid","0"});
 }else{ 
	tempsql.insert({"wxid",std::to_string(data.wxid)});
}
 break;
 case 3:
	tempsql.insert({"product_trade",data.product_trade});
 break;
 case 4:
if(data.paytype==0){
	tempsql.insert({"paytype","0"});
 }else{ 
	tempsql.insert({"paytype",std::to_string(data.paytype)});
}
 break;
 case 5:
	tempsql.insert({"openid",data.openid});
 break;
 case 6:
	tempsql.insert({"wxorder",data.wxorder});
 break;
 case 7:
	tempsql.insert({"storeorder",data.storeorder});
 break;
 case 8:
if(data.totalnum==0){
	tempsql.insert({"totalnum","0"});
 }else{ 
	tempsql.insert({"totalnum",std::to_string(data.totalnum)});
}
 break;
 case 9:
if(data.addtime==0){
	tempsql.insert({"addtime","0"});
 }else{ 
	tempsql.insert({"addtime",std::to_string(data.addtime)});
}
 break;
 case 10:
if(data.paytime==0){
	tempsql.insert({"paytime","0"});
 }else{ 
	tempsql.insert({"paytime",std::to_string(data.paytime)});
}
 break;
 case 11:
	tempsql.insert({"payip",data.payip});
 break;
 case 12:
	tempsql.insert({"username",data.username});
 break;
 case 13:
	tempsql.insert({"address",data.address});
 break;
 case 14:
	tempsql.insert({"mobile",data.mobile});
 break;
 case 15:
if(data.payprice==0){
	tempsql.insert({"payprice","0"});
 }else{ 
	tempsql.insert({"payprice",std::to_string(data.payprice)});
}
 break;
 case 16:
if(data.shipprice==0){
	tempsql.insert({"shipprice","0"});
 }else{ 
	tempsql.insert({"shipprice",std::to_string(data.shipprice)});
}
 break;
 case 17:
if(data.isrefund==0){
	tempsql.insert({"isrefund","0"});
 }else{ 
	tempsql.insert({"isrefund",std::to_string(data.isrefund)});
}
 break;
 case 18:
if(data.isship==0){
	tempsql.insert({"isship","0"});
 }else{ 
	tempsql.insert({"isship",std::to_string(data.isship)});
}
 break;
 case 19:
if(data.isfinsh==0){
	tempsql.insert({"isfinsh","0"});
 }else{ 
	tempsql.insert({"isfinsh",std::to_string(data.isfinsh)});
}
 break;
 case 20:
if(data.isremove==0){
	tempsql.insert({"isremove","0"});
 }else{ 
	tempsql.insert({"isremove",std::to_string(data.isremove)});
}
 break;
 case 21:
if(data.refundnum==0){
	tempsql.insert({"refundnum","0"});
 }else{ 
	tempsql.insert({"refundnum",std::to_string(data.refundnum)});
}
 break;
 case 22:
	tempsql.insert({"paytitle",data.paytitle});
 break;
 case 23:
	tempsql.insert({"content",data.content});
 break;
 case 24:
	tempsql.insert({"liuyan",data.liuyan});
 break;
 case 25:
	tempsql.insert({"excomany",data.excomany});
 break;
 case 26:
	tempsql.insert({"exnumber",data.exnumber});
 break;
 case 27:
if(data.expaddtime==0){
	tempsql.insert({"expaddtime","0"});
 }else{ 
	tempsql.insert({"expaddtime",std::to_string(data.expaddtime)});
}
 break;
 case 28:
if(data.goodsid==0){
	tempsql.insert({"goodsid","0"});
 }else{ 
	tempsql.insert({"goodsid",std::to_string(data.goodsid)});
}
 break;
 case 29:
if(data.status==0){
	tempsql.insert({"status","0"});
 }else{ 
	tempsql.insert({"status",std::to_string(data.status)});
}
 break;
 case 30:
if(data.jifen==0){
	tempsql.insert({"jifen","0"});
 }else{ 
	tempsql.insert({"jifen",std::to_string(data.jifen)});
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
if(data.orderid==0){
	tempsql<<"\"orderid\":0";
 }else{ 
	tempsql<<"\"orderid\":"<<std::to_string(data.orderid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.wxid==0){
	tempsql<<",\"wxid\":0";
 }else{ 
	tempsql<<",\"wxid\":"<<std::to_string(data.wxid);
}
tempsql<<",\"product_trade\":\""<<http::utf8_to_jsonstring(data.product_trade);
tempsql<<"\"";
if(data.paytype==0){
	tempsql<<",\"paytype\":0";
 }else{ 
	tempsql<<",\"paytype\":"<<std::to_string(data.paytype);
}
tempsql<<",\"openid\":\""<<http::utf8_to_jsonstring(data.openid);
tempsql<<"\"";
tempsql<<",\"wxorder\":\""<<http::utf8_to_jsonstring(data.wxorder);
tempsql<<"\"";
tempsql<<",\"storeorder\":\""<<http::utf8_to_jsonstring(data.storeorder);
tempsql<<"\"";
if(data.totalnum==0){
	tempsql<<",\"totalnum\":0";
 }else{ 
	tempsql<<",\"totalnum\":"<<std::to_string(data.totalnum);
}
if(data.addtime==0){
	tempsql<<",\"addtime\":0";
 }else{ 
	tempsql<<",\"addtime\":"<<std::to_string(data.addtime);
}
if(data.paytime==0){
	tempsql<<",\"paytime\":0";
 }else{ 
	tempsql<<",\"paytime\":"<<std::to_string(data.paytime);
}
tempsql<<",\"payip\":\""<<http::utf8_to_jsonstring(data.payip);
tempsql<<"\"";
tempsql<<",\"username\":\""<<http::utf8_to_jsonstring(data.username);
tempsql<<"\"";
tempsql<<",\"address\":\""<<http::utf8_to_jsonstring(data.address);
tempsql<<"\"";
tempsql<<",\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile);
tempsql<<"\"";
if(data.payprice==0){
	tempsql<<",\"payprice\":0";
 }else{ 
	tempsql<<",\"payprice\":"<<std::to_string(data.payprice);
}
if(data.shipprice==0){
	tempsql<<",\"shipprice\":0";
 }else{ 
	tempsql<<",\"shipprice\":"<<std::to_string(data.shipprice);
}
if(data.isrefund==0){
	tempsql<<",\"isrefund\":0";
 }else{ 
	tempsql<<",\"isrefund\":"<<std::to_string(data.isrefund);
}
if(data.isship==0){
	tempsql<<",\"isship\":0";
 }else{ 
	tempsql<<",\"isship\":"<<std::to_string(data.isship);
}
if(data.isfinsh==0){
	tempsql<<",\"isfinsh\":0";
 }else{ 
	tempsql<<",\"isfinsh\":"<<std::to_string(data.isfinsh);
}
if(data.isremove==0){
	tempsql<<",\"isremove\":0";
 }else{ 
	tempsql<<",\"isremove\":"<<std::to_string(data.isremove);
}
if(data.refundnum==0){
	tempsql<<",\"refundnum\":0";
 }else{ 
	tempsql<<",\"refundnum\":"<<std::to_string(data.refundnum);
}
tempsql<<",\"paytitle\":\""<<http::utf8_to_jsonstring(data.paytitle);
tempsql<<"\"";
tempsql<<",\"content\":\""<<http::utf8_to_jsonstring(data.content);
tempsql<<"\"";
tempsql<<",\"liuyan\":\""<<http::utf8_to_jsonstring(data.liuyan);
tempsql<<"\"";
tempsql<<",\"excomany\":\""<<http::utf8_to_jsonstring(data.excomany);
tempsql<<"\"";
tempsql<<",\"exnumber\":\""<<http::utf8_to_jsonstring(data.exnumber);
tempsql<<"\"";
if(data.expaddtime==0){
	tempsql<<",\"expaddtime\":0";
 }else{ 
	tempsql<<",\"expaddtime\":"<<std::to_string(data.expaddtime);
}
if(data.goodsid==0){
	tempsql<<",\"goodsid\":0";
 }else{ 
	tempsql<<",\"goodsid\":"<<std::to_string(data.goodsid);
}
if(data.status==0){
	tempsql<<",\"status\":0";
 }else{ 
	tempsql<<",\"status\":"<<std::to_string(data.status);
}
if(data.jifen==0){
	tempsql<<",\"jifen\":0";
 }else{ 
	tempsql<<",\"jifen\":"<<std::to_string(data.jifen);
}
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
            for(jj=0;jj<orderlist_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.orderid==0){
	tempsql<<"\"orderid\":0";
 }else{ 
	tempsql<<"\"orderid\":"<<std::to_string(data.orderid);
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
if(data.wxid==0){
	tempsql<<"\"wxid\":0";
 }else{ 
	tempsql<<"\"wxid\":"<<std::to_string(data.wxid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"product_trade\":\""<<http::utf8_to_jsonstring(data.product_trade)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.paytype==0){
	tempsql<<"\"paytype\":0";
 }else{ 
	tempsql<<"\"paytype\":"<<std::to_string(data.paytype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"openid\":\""<<http::utf8_to_jsonstring(data.openid)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxorder\":\""<<http::utf8_to_jsonstring(data.wxorder)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"storeorder\":\""<<http::utf8_to_jsonstring(data.storeorder)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(data.totalnum);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(data.addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.paytime==0){
	tempsql<<"\"paytime\":0";
 }else{ 
	tempsql<<"\"paytime\":"<<std::to_string(data.paytime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"payip\":\""<<http::utf8_to_jsonstring(data.payip)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"username\":\""<<http::utf8_to_jsonstring(data.username)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(data.address)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(data.payprice==0){
	tempsql<<"\"payprice\":0";
 }else{ 
	tempsql<<"\"payprice\":"<<std::to_string(data.payprice);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(data.shipprice==0){
	tempsql<<"\"shipprice\":0";
 }else{ 
	tempsql<<"\"shipprice\":"<<std::to_string(data.shipprice);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(data.isrefund==0){
	tempsql<<"\"isrefund\":0";
 }else{ 
	tempsql<<"\"isrefund\":"<<std::to_string(data.isrefund);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(data.isship==0){
	tempsql<<"\"isship\":0";
 }else{ 
	tempsql<<"\"isship\":"<<std::to_string(data.isship);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(data.isfinsh==0){
	tempsql<<"\"isfinsh\":0";
 }else{ 
	tempsql<<"\"isfinsh\":"<<std::to_string(data.isfinsh);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.isremove==0){
	tempsql<<"\"isremove\":0";
 }else{ 
	tempsql<<"\"isremove\":"<<std::to_string(data.isremove);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.refundnum==0){
	tempsql<<"\"refundnum\":0";
 }else{ 
	tempsql<<"\"refundnum\":"<<std::to_string(data.refundnum);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paytitle\":\""<<http::utf8_to_jsonstring(data.paytitle)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"liuyan\":\""<<http::utf8_to_jsonstring(data.liuyan)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"excomany\":\""<<http::utf8_to_jsonstring(data.excomany)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"exnumber\":\""<<http::utf8_to_jsonstring(data.exnumber)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
if(data.expaddtime==0){
	tempsql<<"\"expaddtime\":0";
 }else{ 
	tempsql<<"\"expaddtime\":"<<std::to_string(data.expaddtime);
}
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
if(data.goodsid==0){
	tempsql<<"\"goodsid\":0";
 }else{ 
	tempsql<<"\"goodsid\":"<<std::to_string(data.goodsid);
}
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(data.status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(data.status);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(data.jifen==0){
	tempsql<<"\"jifen\":0";
 }else{ 
	tempsql<<"\"jifen\":"<<std::to_string(data.jifen);
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
        orderlist_info::meta metatemp; 
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
		  http::json_set_val(data.orderid,set_value_name);
		 break;
		
		case 1:
		  http::json_set_val(data.userid,set_value_name);
		 break;
		
		case 2:
		  http::json_set_val(data.wxid,set_value_name);
		 break;
		
		case 3:
		  http::json_set_val(data.product_trade,set_value_name);
		 break;
		
		case 4:
		  http::json_set_val(data.paytype,set_value_name);
		 break;
		
		case 5:
		  http::json_set_val(data.openid,set_value_name);
		 break;
		
		case 6:
		  http::json_set_val(data.wxorder,set_value_name);
		 break;
		
		case 7:
		  http::json_set_val(data.storeorder,set_value_name);
		 break;
		
		case 8:
		  http::json_set_val(data.totalnum,set_value_name);
		 break;
		
		case 9:
		  http::json_set_val(data.addtime,set_value_name);
		 break;
		
		case 10:
		  http::json_set_val(data.paytime,set_value_name);
		 break;
		
		case 11:
		  http::json_set_val(data.payip,set_value_name);
		 break;
		
		case 12:
		  http::json_set_val(data.username,set_value_name);
		 break;
		
		case 13:
		  http::json_set_val(data.address,set_value_name);
		 break;
		
		case 14:
		  http::json_set_val(data.mobile,set_value_name);
		 break;
		
		case 15:
		  http::json_set_val(data.payprice,set_value_name);
		 break;
		
		case 16:
		  http::json_set_val(data.shipprice,set_value_name);
		 break;
		
		case 17:
		  http::json_set_val(data.isrefund,set_value_name);
		 break;
		
		case 18:
		  http::json_set_val(data.isship,set_value_name);
		 break;
		
		case 19:
		  http::json_set_val(data.isfinsh,set_value_name);
		 break;
		
		case 20:
		  http::json_set_val(data.isremove,set_value_name);
		 break;
		
		case 21:
		  http::json_set_val(data.refundnum,set_value_name);
		 break;
		
		case 22:
		  http::json_set_val(data.paytitle,set_value_name);
		 break;
		
		case 23:
		  http::json_set_val(data.content,set_value_name);
		 break;
		
		case 24:
		  http::json_set_val(data.liuyan,set_value_name);
		 break;
		
		case 25:
		  http::json_set_val(data.excomany,set_value_name);
		 break;
		
		case 26:
		  http::json_set_val(data.exnumber,set_value_name);
		 break;
		
		case 27:
		  http::json_set_val(data.expaddtime,set_value_name);
		 break;
		
		case 28:
		  http::json_set_val(data.goodsid,set_value_name);
		 break;
		
		case 29:
		  http::json_set_val(data.status,set_value_name);
		 break;
		
		case 30:
		  http::json_set_val(data.jifen,set_value_name);
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
        for(jj=0;jj<orderlist_info::col_names.size();jj++){
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
if(record[n].orderid==0){
	tempsql<<"\"orderid\":0";
 }else{ 
	tempsql<<"\"orderid\":"<<std::to_string(record[n].orderid);
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
if(record[n].wxid==0){
	tempsql<<"\"wxid\":0";
 }else{ 
	tempsql<<"\"wxid\":"<<std::to_string(record[n].wxid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"product_trade\":\""<<http::utf8_to_jsonstring(record[n].product_trade)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].paytype==0){
	tempsql<<"\"paytype\":0";
 }else{ 
	tempsql<<"\"paytype\":"<<std::to_string(record[n].paytype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"openid\":\""<<http::utf8_to_jsonstring(record[n].openid)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxorder\":\""<<http::utf8_to_jsonstring(record[n].wxorder)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"storeorder\":\""<<http::utf8_to_jsonstring(record[n].storeorder)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(record[n].totalnum);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].paytime==0){
	tempsql<<"\"paytime\":0";
 }else{ 
	tempsql<<"\"paytime\":"<<std::to_string(record[n].paytime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"payip\":\""<<http::utf8_to_jsonstring(record[n].payip)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"username\":\""<<http::utf8_to_jsonstring(record[n].username)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(record[n].payprice==0){
	tempsql<<"\"payprice\":0";
 }else{ 
	tempsql<<"\"payprice\":"<<std::to_string(record[n].payprice);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(record[n].shipprice==0){
	tempsql<<"\"shipprice\":0";
 }else{ 
	tempsql<<"\"shipprice\":"<<std::to_string(record[n].shipprice);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(record[n].isrefund==0){
	tempsql<<"\"isrefund\":0";
 }else{ 
	tempsql<<"\"isrefund\":"<<std::to_string(record[n].isrefund);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(record[n].isship==0){
	tempsql<<"\"isship\":0";
 }else{ 
	tempsql<<"\"isship\":"<<std::to_string(record[n].isship);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(record[n].isfinsh==0){
	tempsql<<"\"isfinsh\":0";
 }else{ 
	tempsql<<"\"isfinsh\":"<<std::to_string(record[n].isfinsh);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].isremove==0){
	tempsql<<"\"isremove\":0";
 }else{ 
	tempsql<<"\"isremove\":"<<std::to_string(record[n].isremove);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].refundnum==0){
	tempsql<<"\"refundnum\":0";
 }else{ 
	tempsql<<"\"refundnum\":"<<std::to_string(record[n].refundnum);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paytitle\":\""<<http::utf8_to_jsonstring(record[n].paytitle)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"liuyan\":\""<<http::utf8_to_jsonstring(record[n].liuyan)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"excomany\":\""<<http::utf8_to_jsonstring(record[n].excomany)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"exnumber\":\""<<http::utf8_to_jsonstring(record[n].exnumber)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
if(record[n].expaddtime==0){
	tempsql<<"\"expaddtime\":0";
 }else{ 
	tempsql<<"\"expaddtime\":"<<std::to_string(record[n].expaddtime);
}
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
if(record[n].goodsid==0){
	tempsql<<"\"goodsid\":0";
 }else{ 
	tempsql<<"\"goodsid\":"<<std::to_string(record[n].goodsid);
}
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(record[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(record[n].status);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(record[n].jifen==0){
	tempsql<<"\"jifen\":0";
 }else{ 
	tempsql<<"\"jifen\":"<<std::to_string(record[n].jifen);
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
   
   std::string to_json(std::function<bool(std::string&,orderlist_info::meta&)> func,std::string_view field=""){
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
            for(jj=0;jj<orderlist_info::col_names.size();jj++){
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
if(record[n].orderid==0){
	tempsql<<"\"orderid\":0";
 }else{ 
	tempsql<<"\"orderid\":"<<std::to_string(record[n].orderid);
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
if(record[n].wxid==0){
	tempsql<<"\"wxid\":0";
 }else{ 
	tempsql<<"\"wxid\":"<<std::to_string(record[n].wxid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"product_trade\":\""<<http::utf8_to_jsonstring(record[n].product_trade)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].paytype==0){
	tempsql<<"\"paytype\":0";
 }else{ 
	tempsql<<"\"paytype\":"<<std::to_string(record[n].paytype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"openid\":\""<<http::utf8_to_jsonstring(record[n].openid)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxorder\":\""<<http::utf8_to_jsonstring(record[n].wxorder)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"storeorder\":\""<<http::utf8_to_jsonstring(record[n].storeorder)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(record[n].totalnum);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].paytime==0){
	tempsql<<"\"paytime\":0";
 }else{ 
	tempsql<<"\"paytime\":"<<std::to_string(record[n].paytime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"payip\":\""<<http::utf8_to_jsonstring(record[n].payip)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"username\":\""<<http::utf8_to_jsonstring(record[n].username)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(record[n].payprice==0){
	tempsql<<"\"payprice\":0";
 }else{ 
	tempsql<<"\"payprice\":"<<std::to_string(record[n].payprice);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(record[n].shipprice==0){
	tempsql<<"\"shipprice\":0";
 }else{ 
	tempsql<<"\"shipprice\":"<<std::to_string(record[n].shipprice);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(record[n].isrefund==0){
	tempsql<<"\"isrefund\":0";
 }else{ 
	tempsql<<"\"isrefund\":"<<std::to_string(record[n].isrefund);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(record[n].isship==0){
	tempsql<<"\"isship\":0";
 }else{ 
	tempsql<<"\"isship\":"<<std::to_string(record[n].isship);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(record[n].isfinsh==0){
	tempsql<<"\"isfinsh\":0";
 }else{ 
	tempsql<<"\"isfinsh\":"<<std::to_string(record[n].isfinsh);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].isremove==0){
	tempsql<<"\"isremove\":0";
 }else{ 
	tempsql<<"\"isremove\":"<<std::to_string(record[n].isremove);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].refundnum==0){
	tempsql<<"\"refundnum\":0";
 }else{ 
	tempsql<<"\"refundnum\":"<<std::to_string(record[n].refundnum);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paytitle\":\""<<http::utf8_to_jsonstring(record[n].paytitle)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"liuyan\":\""<<http::utf8_to_jsonstring(record[n].liuyan)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"excomany\":\""<<http::utf8_to_jsonstring(record[n].excomany)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"exnumber\":\""<<http::utf8_to_jsonstring(record[n].exnumber)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
if(record[n].expaddtime==0){
	tempsql<<"\"expaddtime\":0";
 }else{ 
	tempsql<<"\"expaddtime\":"<<std::to_string(record[n].expaddtime);
}
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
if(record[n].goodsid==0){
	tempsql<<"\"goodsid\":0";
 }else{ 
	tempsql<<"\"goodsid\":"<<std::to_string(record[n].goodsid);
}
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(record[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(record[n].status);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(record[n].jifen==0){
	tempsql<<"\"jifen\":0";
 }else{ 
	tempsql<<"\"jifen\":"<<std::to_string(record[n].jifen);
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
   long long getPK(){  return data.orderid; } 
 void setPK(long long val){  data.orderid=val;} 
 int  getOrderid(){  return data.orderid; } 
 void setOrderid( int  val){  data.orderid=val;} 

 int  getUserid(){  return data.userid; } 
 void setUserid( int  val){  data.userid=val;
		 set_dirty(1);  }

 int  getWxid(){  return data.wxid; } 
 void setWxid( int  val){  data.wxid=val;
		 set_dirty(2);  }

 std::string  getProductTrade(){  return data.product_trade; } 
 std::string & getRefProductTrade(){  return std::ref(data.product_trade); } 
 void setProductTrade( std::string  &val){  data.product_trade=val;
		 set_dirty(3);  }
 void setProductTrade(std::string_view val){  data.product_trade=val;
		 set_dirty(3);  }

 unsigned  char  getPaytype(){  return data.paytype; } 
 void setPaytype( unsigned  char  val){  data.paytype=val;
		 set_dirty(4);  }

 std::string  getOpenid(){  return data.openid; } 
 std::string & getRefOpenid(){  return std::ref(data.openid); } 
 void setOpenid( std::string  &val){  data.openid=val;
		 set_dirty(5);  }
 void setOpenid(std::string_view val){  data.openid=val;
		 set_dirty(5);  }

 std::string  getWxorder(){  return data.wxorder; } 
 std::string & getRefWxorder(){  return std::ref(data.wxorder); } 
 void setWxorder( std::string  &val){  data.wxorder=val;
		 set_dirty(6);  }
 void setWxorder(std::string_view val){  data.wxorder=val;
		 set_dirty(6);  }

 std::string  getStoreorder(){  return data.storeorder; } 
 std::string & getRefStoreorder(){  return std::ref(data.storeorder); } 
 void setStoreorder( std::string  &val){  data.storeorder=val;
		 set_dirty(7);  }
 void setStoreorder(std::string_view val){  data.storeorder=val;
		 set_dirty(7);  }

 unsigned  int  getTotalnum(){  return data.totalnum; } 
 void setTotalnum( unsigned  int  val){  data.totalnum=val;
		 set_dirty(8);  }

 unsigned  int  getAddtime(){  return data.addtime; } 
 void setAddtime( unsigned  int  val){  data.addtime=val;
		 set_dirty(9);  }

 unsigned  int  getPaytime(){  return data.paytime; } 
 void setPaytime( unsigned  int  val){  data.paytime=val;
		 set_dirty(10);  }

 std::string  getPayip(){  return data.payip; } 
 std::string & getRefPayip(){  return std::ref(data.payip); } 
 void setPayip( std::string  &val){  data.payip=val;
		 set_dirty(11);  }
 void setPayip(std::string_view val){  data.payip=val;
		 set_dirty(11);  }

 std::string  getUsername(){  return data.username; } 
 std::string & getRefUsername(){  return std::ref(data.username); } 
 void setUsername( std::string  &val){  data.username=val;
		 set_dirty(12);  }
 void setUsername(std::string_view val){  data.username=val;
		 set_dirty(12);  }

 std::string  getAddress(){  return data.address; } 
 std::string & getRefAddress(){  return std::ref(data.address); } 
 void setAddress( std::string  &val){  data.address=val;
		 set_dirty(13);  }
 void setAddress(std::string_view val){  data.address=val;
		 set_dirty(13);  }

 std::string  getMobile(){  return data.mobile; } 
 std::string & getRefMobile(){  return std::ref(data.mobile); } 
 void setMobile( std::string  &val){  data.mobile=val;
		 set_dirty(14);  }
 void setMobile(std::string_view val){  data.mobile=val;
		 set_dirty(14);  }

 unsigned  int  getPayprice(){  return data.payprice; } 
 void setPayprice( unsigned  int  val){  data.payprice=val;
		 set_dirty(15);  }

 unsigned  int  getShipprice(){  return data.shipprice; } 
 void setShipprice( unsigned  int  val){  data.shipprice=val;
		 set_dirty(16);  }

 char  getIsrefund(){  return data.isrefund; } 
 void setIsrefund( char  val){  data.isrefund=val;
		 set_dirty(17);  }

 char  getIsship(){  return data.isship; } 
 void setIsship( char  val){  data.isship=val;
		 set_dirty(18);  }

 char  getIsfinsh(){  return data.isfinsh; } 
 void setIsfinsh( char  val){  data.isfinsh=val;
		 set_dirty(19);  }

 unsigned  char  getIsremove(){  return data.isremove; } 
 void setIsremove( unsigned  char  val){  data.isremove=val;
		 set_dirty(20);  }

 unsigned  int  getRefundnum(){  return data.refundnum; } 
 void setRefundnum( unsigned  int  val){  data.refundnum=val;
		 set_dirty(21);  }

 std::string  getPaytitle(){  return data.paytitle; } 
 std::string & getRefPaytitle(){  return std::ref(data.paytitle); } 
 void setPaytitle( std::string  &val){  data.paytitle=val;
		 set_dirty(22);  }
 void setPaytitle(std::string_view val){  data.paytitle=val;
		 set_dirty(22);  }

 std::string  getContent(){  return data.content; } 
 std::string & getRefContent(){  return std::ref(data.content); } 
 void setContent( std::string  &val){  data.content=val;
		 set_dirty(23);  }
 void setContent(std::string_view val){  data.content=val;
		 set_dirty(23);  }

 std::string  getLiuyan(){  return data.liuyan; } 
 std::string & getRefLiuyan(){  return std::ref(data.liuyan); } 
 void setLiuyan( std::string  &val){  data.liuyan=val;
		 set_dirty(24);  }
 void setLiuyan(std::string_view val){  data.liuyan=val;
		 set_dirty(24);  }

 std::string  getExcomany(){  return data.excomany; } 
 std::string & getRefExcomany(){  return std::ref(data.excomany); } 
 void setExcomany( std::string  &val){  data.excomany=val;
		 set_dirty(25);  }
 void setExcomany(std::string_view val){  data.excomany=val;
		 set_dirty(25);  }

 std::string  getExnumber(){  return data.exnumber; } 
 std::string & getRefExnumber(){  return std::ref(data.exnumber); } 
 void setExnumber( std::string  &val){  data.exnumber=val;
		 set_dirty(26);  }
 void setExnumber(std::string_view val){  data.exnumber=val;
		 set_dirty(26);  }

 unsigned  int  getExpaddtime(){  return data.expaddtime; } 
 void setExpaddtime( unsigned  int  val){  data.expaddtime=val;
		 set_dirty(27);  }

 unsigned  int  getGoodsid(){  return data.goodsid; } 
 void setGoodsid( unsigned  int  val){  data.goodsid=val;
		 set_dirty(28);  }

 int  getStatus(){  return data.status; } 
 void setStatus( int  val){  data.status=val;
		 set_dirty(29);  }

 unsigned  int  getJifen(){  return data.jifen; } 
 void setJifen( unsigned  int  val){  data.jifen=val;
		 set_dirty(30);  }

orderlist_info::meta getnewData(){
 	 struct orderlist_info::meta newdata;
	 return newdata; 
} 
orderlist_info::meta getData(){
 	 return data; 
} 
std::vector<orderlist_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<orderlist_info::meta_tree> &tree_data, std::string_view field=""){
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
            for(jj=0;jj<orderlist_info::col_names.size();jj++){
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
if(tree_data[n].orderid==0){
	tempsql<<"\"orderid\":0";
 }else{ 
	tempsql<<"\"orderid\":"<<std::to_string(tree_data[n].orderid);
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
if(tree_data[n].wxid==0){
	tempsql<<"\"wxid\":0";
 }else{ 
	tempsql<<"\"wxid\":"<<std::to_string(tree_data[n].wxid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"product_trade\":\""<<http::utf8_to_jsonstring(tree_data[n].product_trade)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].paytype==0){
	tempsql<<"\"paytype\":0";
 }else{ 
	tempsql<<"\"paytype\":"<<std::to_string(tree_data[n].paytype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"openid\":\""<<http::utf8_to_jsonstring(tree_data[n].openid)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxorder\":\""<<http::utf8_to_jsonstring(tree_data[n].wxorder)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"storeorder\":\""<<http::utf8_to_jsonstring(tree_data[n].storeorder)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(tree_data[n].totalnum);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(tree_data[n].addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].paytime==0){
	tempsql<<"\"paytime\":0";
 }else{ 
	tempsql<<"\"paytime\":"<<std::to_string(tree_data[n].paytime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"payip\":\""<<http::utf8_to_jsonstring(tree_data[n].payip)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"username\":\""<<http::utf8_to_jsonstring(tree_data[n].username)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(tree_data[n].address)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(tree_data[n].mobile)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].payprice==0){
	tempsql<<"\"payprice\":0";
 }else{ 
	tempsql<<"\"payprice\":"<<std::to_string(tree_data[n].payprice);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].shipprice==0){
	tempsql<<"\"shipprice\":0";
 }else{ 
	tempsql<<"\"shipprice\":"<<std::to_string(tree_data[n].shipprice);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isrefund==0){
	tempsql<<"\"isrefund\":0";
 }else{ 
	tempsql<<"\"isrefund\":"<<std::to_string(tree_data[n].isrefund);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isship==0){
	tempsql<<"\"isship\":0";
 }else{ 
	tempsql<<"\"isship\":"<<std::to_string(tree_data[n].isship);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isfinsh==0){
	tempsql<<"\"isfinsh\":0";
 }else{ 
	tempsql<<"\"isfinsh\":"<<std::to_string(tree_data[n].isfinsh);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isremove==0){
	tempsql<<"\"isremove\":0";
 }else{ 
	tempsql<<"\"isremove\":"<<std::to_string(tree_data[n].isremove);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].refundnum==0){
	tempsql<<"\"refundnum\":0";
 }else{ 
	tempsql<<"\"refundnum\":"<<std::to_string(tree_data[n].refundnum);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paytitle\":\""<<http::utf8_to_jsonstring(tree_data[n].paytitle)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"liuyan\":\""<<http::utf8_to_jsonstring(tree_data[n].liuyan)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"excomany\":\""<<http::utf8_to_jsonstring(tree_data[n].excomany)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"exnumber\":\""<<http::utf8_to_jsonstring(tree_data[n].exnumber)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expaddtime==0){
	tempsql<<"\"expaddtime\":0";
 }else{ 
	tempsql<<"\"expaddtime\":"<<std::to_string(tree_data[n].expaddtime);
}
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].goodsid==0){
	tempsql<<"\"goodsid\":0";
 }else{ 
	tempsql<<"\"goodsid\":"<<std::to_string(tree_data[n].goodsid);
}
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(tree_data[n].status);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].jifen==0){
	tempsql<<"\"jifen\":0";
 }else{ 
	tempsql<<"\"jifen\":"<<std::to_string(tree_data[n].jifen);
}
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
   
   std::string tree_tojson(const std::vector<orderlist_info::meta_tree> &tree_data,std::function<bool(std::string&,const orderlist_info::meta_tree&)> func,std::string_view field=""){
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
            for(jj=0;jj<orderlist_info::col_names.size();jj++){
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
if(tree_data[n].orderid==0){
	tempsql<<"\"orderid\":0";
 }else{ 
	tempsql<<"\"orderid\":"<<std::to_string(tree_data[n].orderid);
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
if(tree_data[n].wxid==0){
	tempsql<<"\"wxid\":0";
 }else{ 
	tempsql<<"\"wxid\":"<<std::to_string(tree_data[n].wxid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"product_trade\":\""<<http::utf8_to_jsonstring(tree_data[n].product_trade)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].paytype==0){
	tempsql<<"\"paytype\":0";
 }else{ 
	tempsql<<"\"paytype\":"<<std::to_string(tree_data[n].paytype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"openid\":\""<<http::utf8_to_jsonstring(tree_data[n].openid)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxorder\":\""<<http::utf8_to_jsonstring(tree_data[n].wxorder)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"storeorder\":\""<<http::utf8_to_jsonstring(tree_data[n].storeorder)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].totalnum==0){
	tempsql<<"\"totalnum\":0";
 }else{ 
	tempsql<<"\"totalnum\":"<<std::to_string(tree_data[n].totalnum);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(tree_data[n].addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].paytime==0){
	tempsql<<"\"paytime\":0";
 }else{ 
	tempsql<<"\"paytime\":"<<std::to_string(tree_data[n].paytime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"payip\":\""<<http::utf8_to_jsonstring(tree_data[n].payip)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"username\":\""<<http::utf8_to_jsonstring(tree_data[n].username)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(tree_data[n].address)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(tree_data[n].mobile)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].payprice==0){
	tempsql<<"\"payprice\":0";
 }else{ 
	tempsql<<"\"payprice\":"<<std::to_string(tree_data[n].payprice);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].shipprice==0){
	tempsql<<"\"shipprice\":0";
 }else{ 
	tempsql<<"\"shipprice\":"<<std::to_string(tree_data[n].shipprice);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isrefund==0){
	tempsql<<"\"isrefund\":0";
 }else{ 
	tempsql<<"\"isrefund\":"<<std::to_string(tree_data[n].isrefund);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isship==0){
	tempsql<<"\"isship\":0";
 }else{ 
	tempsql<<"\"isship\":"<<std::to_string(tree_data[n].isship);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isfinsh==0){
	tempsql<<"\"isfinsh\":0";
 }else{ 
	tempsql<<"\"isfinsh\":"<<std::to_string(tree_data[n].isfinsh);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isremove==0){
	tempsql<<"\"isremove\":0";
 }else{ 
	tempsql<<"\"isremove\":"<<std::to_string(tree_data[n].isremove);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].refundnum==0){
	tempsql<<"\"refundnum\":0";
 }else{ 
	tempsql<<"\"refundnum\":"<<std::to_string(tree_data[n].refundnum);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paytitle\":\""<<http::utf8_to_jsonstring(tree_data[n].paytitle)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"liuyan\":\""<<http::utf8_to_jsonstring(tree_data[n].liuyan)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"excomany\":\""<<http::utf8_to_jsonstring(tree_data[n].excomany)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"exnumber\":\""<<http::utf8_to_jsonstring(tree_data[n].exnumber)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expaddtime==0){
	tempsql<<"\"expaddtime\":0";
 }else{ 
	tempsql<<"\"expaddtime\":"<<std::to_string(tree_data[n].expaddtime);
}
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].goodsid==0){
	tempsql<<"\"goodsid\":0";
 }else{ 
	tempsql<<"\"goodsid\":"<<std::to_string(tree_data[n].goodsid);
}
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(tree_data[n].status);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].jifen==0){
	tempsql<<"\"jifen\":0";
 }else{ 
	tempsql<<"\"jifen\":"<<std::to_string(tree_data[n].jifen);
}
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
   
    template<orderlist_info::cols KeyCol, orderlist_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>()));
        using ValType = decltype(orderlist_info::getField<ValCol>(std::declval<const orderlist_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(orderlist_info::getField<KeyCol>(iter), orderlist_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<orderlist_info::cols KeyCol, orderlist_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>())), 
            decltype(orderlist_info::getField<ValCol>(std::declval<const orderlist_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>())), 
                    decltype(orderlist_info::getField<ValCol>(std::declval<const orderlist_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>()));
        using ValType = decltype(orderlist_info::getField<ValCol>(std::declval<const orderlist_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(orderlist_info::getField<KeyCol>(iter), orderlist_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(orderlist_info::getField<KeyCol>(iter), orderlist_info::getField<ValCol>(iter))) {
                    result.emplace(orderlist_info::getField<KeyCol>(iter), orderlist_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<orderlist_info::cols KeyCol, orderlist_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>()));
        using ValType = decltype(orderlist_info::getField<ValCol>(std::declval<const orderlist_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(orderlist_info::getField<KeyCol>(iter), orderlist_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<orderlist_info::cols KeyCol, orderlist_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>())), 
            decltype(orderlist_info::getField<ValCol>(std::declval<const orderlist_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>())), 
                    decltype(orderlist_info::getField<ValCol>(std::declval<const orderlist_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>()));
        using ValType = decltype(orderlist_info::getField<ValCol>(std::declval<const orderlist_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(orderlist_info::getField<KeyCol>(iter), orderlist_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(orderlist_info::getField<KeyCol>(iter), orderlist_info::getField<ValCol>(iter))) {
                    result.emplace_back(orderlist_info::getField<KeyCol>(iter), orderlist_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<orderlist_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(orderlist_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<orderlist_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(orderlist_info::getField<KeyCol>(std::declval<const orderlist_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(orderlist_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(orderlist_info::getField<KeyCol>(iter))) {
                    result.emplace_back(orderlist_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<orderlist_info::cols Col>
        requires requires(std::ostream& os, decltype(orderlist_info::getField<Col>(std::declval<const orderlist_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << orderlist_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<orderlist_info::cols Col>
        requires requires(std::ostream& os, decltype(orderlist_info::getField<Col>(std::declval<const orderlist_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << orderlist_info::getField<Col>(iter); 
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
   