#ifndef ORM_CMS_OAUTH_CODEBASEMATA_H
#define ORM_CMS_OAUTH_CODEBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Fri, 30 Oct 2026 07:29:13 GMT
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

namespace oauth_code_info
{
 
    static constexpr std::size_t col_count = 10;
    enum class cols : unsigned char 
    {
		id = 0,
		code_hash = 1,
		client_id = 2,
		user_id = 3,
		redirect_uri = 4,
		scope = 5,
		code_challenge = 6,
		expires_at = 7,
		used = 8,
		family_id = 9,

    };

    struct meta
    {
		 unsigned  int  id = 0; ///**/
		 std::string  code_hash = ""; ///*SHA-256(授权码) 小写hex，明文码不落库*/
		 std::string  client_id = ""; ///*签发目标客户端*/
		 long long  user_id = 0; ///*授权人=sysuser.adminid（不是 companyid）*/
		 std::string  redirect_uri = ""; ///*授权时登记的回调地址，换码时逐字节比对*/
		 std::string  scope = ""; ///*授权范围，空格分隔*/
		 std::string  code_challenge = ""; ///*PKCE S256 挑战值，只支持 S256*/
		 long long  expires_at = 0; ///*过期 Unix 秒*/
		 char  used = 0; ///*0=未消费 1=已消费，一次性*/
		 unsigned  int  family_id = 0; ///*换码成功时回填的 refresh 族根 id；0=还没换过码或回填失败*/
	};
  
    struct meta_tree
    {
		 unsigned  int  id = 0; ///**/
		 std::string  code_hash = ""; ///*SHA-256(授权码) 小写hex，明文码不落库*/
		 std::string  client_id = ""; ///*签发目标客户端*/
		 long long  user_id = 0; ///*授权人=sysuser.adminid（不是 companyid）*/
		 std::string  redirect_uri = ""; ///*授权时登记的回调地址，换码时逐字节比对*/
		 std::string  scope = ""; ///*授权范围，空格分隔*/
		 std::string  code_challenge = ""; ///*PKCE S256 挑战值，只支持 S256*/
		 long long  expires_at = 0; ///*过期 Unix 秒*/
		 char  used = 0; ///*0=未消费 1=已消费，一次性*/
		 unsigned  int  family_id = 0; ///*换码成功时回填的 refresh 族根 id；0=还没换过码或回填失败*/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  id = 0; ///**/
		 std::string  code_hash = ""; ///*SHA-256(授权码) 小写hex，明文码不落库*/
		 std::string  client_id = ""; ///*签发目标客户端*/
		 long long  user_id = 0; ///*授权人=sysuser.adminid（不是 companyid）*/
		 std::string  redirect_uri = ""; ///*授权时登记的回调地址，换码时逐字节比对*/
		 std::string  scope = ""; ///*授权范围，空格分隔*/
		 std::string  code_challenge = ""; ///*PKCE S256 挑战值，只支持 S256*/
		 long long  expires_at = 0; ///*过期 Unix 秒*/
		 char  used = 0; ///*0=未消费 1=已消费，一次性*/
		 unsigned  int  family_id = 0; ///*换码成功时回填的 refresh 族根 id；0=还没换过码或回填失败*/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::id) { 
		 return m.id;
		} else if constexpr (Col == cols::code_hash) { 
		 return m.code_hash;
		} else if constexpr (Col == cols::client_id) { 
		 return m.client_id;
		} else if constexpr (Col == cols::user_id) { 
		 return m.user_id;
		} else if constexpr (Col == cols::redirect_uri) { 
		 return m.redirect_uri;
		} else if constexpr (Col == cols::scope) { 
		 return m.scope;
		} else if constexpr (Col == cols::code_challenge) { 
		 return m.code_challenge;
		} else if constexpr (Col == cols::expires_at) { 
		 return m.expires_at;
		} else if constexpr (Col == cols::used) { 
		 return m.used;
		} else if constexpr (Col == cols::family_id) { 
		 return m.family_id;
		
        } else {
            //static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using id =  unsigned  int ;
		using code_hash =  std::string ;
		using client_id =  std::string ;
		using user_id =  long long ;
		using redirect_uri =  std::string ;
		using scope =  std::string ;
		using code_challenge =  std::string ;
		using expires_at =  long long ;
		using used =  char ;
		using family_id =  unsigned  int ;

    }

    
    #define ORM_CMS_OAUTH_CODE_EXPAND(x) x 
    
    #define ORM_CMS_OAUTH_CODE_META_FIELD_TYPE(col) \
         orm::cms::oauth_code_info::type::col 
    
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBER(col) \
          ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_META_FIELD_TYPE(col)) col{};
                 
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_1(c1) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c1)) 
     
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_2( c1, c2) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_1( c1)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c2))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_3( c1, c2, c3) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_2( c1, c2)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c3))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_4( c1, c2, c3, c4) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_3( c1, c2, c3)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c4))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_4( c1, c2, c3, c4)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c5))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_5( c1, c2, c3, c4, c5)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c6))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c7))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c8))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c9))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c10))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c11))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c12))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c13))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c14))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c15))
         
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBER(c16))
         
    #define ORM_CMS_OAUTH_CODE_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_OAUTH_CODE_PROJ_MEMBERS(...) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_16, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_15, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_14, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_13, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_12, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_11, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_10, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_9, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_8, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_7, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_6, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_5, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_4, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_3, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_2, \
            ORM_CMS_OAUTH_CODE_PROJ_MEMBERS_1, \
        )(__VA_ARGS__))

    
    #define ORM_CMS_OAUTH_CODE_COUNT(...) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_GET_MACRO(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
    
    
    #define ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c) \
        oss << "\"" #c "\":" << http::to_json_value(c)
    
    #define ORM_CMS_OAUTH_CODE_TO_JSON_1(c1) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c1))
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_2(c1,c2) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_1(c1)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c2)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_3(c1,c2,c3) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_2(c1,c2)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c3)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_4(c1,c2,c3,c4) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_3(c1,c2,c3)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c4)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_5(c1,c2,c3,c4,c5) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_4(c1,c2,c3,c4)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c5)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_6(c1,c2,c3,c4,c5,c6) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_5(c1,c2,c3,c4,c5)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c6)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_6(c1,c2,c3,c4,c5,c6)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c7)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_7(c1,c2,c3,c4,c5,c6,c7)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c8)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_8(c1,c2,c3,c4,c5,c6,c7,c8)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c9)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c10)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c11)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c12)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c13)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c14)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c15)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
         ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)); \
            oss << ','; \
            ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_ITEM(c16)) 
        
        
    #define ORM_CMS_OAUTH_CODE_TO_JSON_BODY(...) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_OAUTH_CODE_TO_JSON_16,ORM_CMS_OAUTH_CODE_TO_JSON_15,ORM_CMS_OAUTH_CODE_TO_JSON_14,ORM_CMS_OAUTH_CODE_TO_JSON_13,ORM_CMS_OAUTH_CODE_TO_JSON_12,ORM_CMS_OAUTH_CODE_TO_JSON_11,ORM_CMS_OAUTH_CODE_TO_JSON_10,ORM_CMS_OAUTH_CODE_TO_JSON_9,ORM_CMS_OAUTH_CODE_TO_JSON_8,ORM_CMS_OAUTH_CODE_TO_JSON_7,ORM_CMS_OAUTH_CODE_TO_JSON_6,ORM_CMS_OAUTH_CODE_TO_JSON_5,ORM_CMS_OAUTH_CODE_TO_JSON_4,ORM_CMS_OAUTH_CODE_TO_JSON_3,ORM_CMS_OAUTH_CODE_TO_JSON_2,ORM_CMS_OAUTH_CODE_TO_JSON_1 \
         )(__VA_ARGS__))
         
          
    #define ORM_CMS_OAUTH_CODE_UNWRAP(...) __VA_ARGS__  

    #define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(name) \
        oss << ",\"" #name "\":" << http::to_json_value(name);

    #define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_1(n1)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n1)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_2(n1,n2)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_1(n1)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n2)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_3(n1,n2,n3)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_2(n1,n2)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n3)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_4(n1,n2,n3,n4)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_3(n1,n2,n3)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n4)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_4(n1,n2,n3,n4)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n5)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_5(n1,n2,n3,n4,n5)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n6)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_6(n1,n2,n3,n4,n5,n6)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n7)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_7(n1,n2,n3,n4,n5,n6,n7)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n8)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_8(n1,n2,n3,n4,n5,n6,n7,n8)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n9)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_9(n1,n2,n3,n4,n5,n6,n7,n8,n9)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n10)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_10(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n11)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_11(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n12)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_12(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n13)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_13(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n14)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_14(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n15)) 

#define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_16(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16)  ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_15(n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15)) ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_ITEM(n16)) 


    #define ORM_CMS_OAUTH_CODE_CAT(a, b) ORM_CMS_OAUTH_CODE_CAT_(a, b)
    #define ORM_CMS_OAUTH_CODE_CAT_(a, b) a##b

    #define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) \
        ORM_CMS_OAUTH_CODE_CAT(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_, N)

    

    #define ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM(...) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM_N(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))


    #define ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(field) \
    if (http::str_colname_casecmp(_orm_name , #field)) { \
        http::try_set_val(field, _buf, _length, _field_type); \
        return; \
    }
    
    
    #define ORM_CMS_OAUTH_CODE_SET_VAL_1(c1) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c1)
    
    
    #define ORM_CMS_OAUTH_CODE_SET_VAL_2(c1,c2) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_1(c1)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c2)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_3(c1,c2,c3) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_2(c1,c2)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c3)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_4(c1,c2,c3,c4) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_3(c1,c2,c3)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c4)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_5(c1,c2,c3,c4,c5) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_4(c1,c2,c3,c4)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c5)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_6(c1,c2,c3,c4,c5,c6) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_5(c1,c2,c3,c4,c5)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c6)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_6(c1,c2,c3,c4,c5,c6)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c7)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_7(c1,c2,c3,c4,c5,c6,c7)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c8)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_8(c1,c2,c3,c4,c5,c6,c7,c8)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c9)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_9(c1,c2,c3,c4,c5,c6,c7,c8,c9)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c10)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c11)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c12)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c13)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c14)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c15)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)) \
        ORM_CMS_OAUTH_CODE_SET_VAL_FIELD(c16)
        
        
    #define ORM_CMS_OAUTH_CODE_SET_VAL_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) \
        ORM_CMS_OAUTH_CODE_CAT(ORM_CMS_OAUTH_CODE_SET_VAL_, N)
    
    
    #define ORM_CMS_OAUTH_CODE_SET_VAL_FIELDS(...) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_N(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)(__VA_ARGS__))
    
    
    #define ORM_CMS_OAUTH_CODE_SET_VAL_CUSTOM_FIELDS(...) \
        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_FIELDS(ORM_CMS_OAUTH_CODE_UNWRAP __VA_ARGS__))
    
    
    #define ORM_CMS_OAUTH_CODE_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::oauth_code_info { \
            struct StructName { \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS(__VA_ARGS__)) \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_BODY(__VA_ARGS__)); \
                oss << '}'; \
                return oss.str(); \
            } \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_OAUTH_CODE_SELF_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::oauth_code_info { \
            struct StructName { \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_BODY(__VA_ARGS__)); \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM(ORM_CMS_OAUTH_CODE_UNWRAP CustomNames));  \
                oss << '}'; \
                return oss.str(); \
            } \
            \
            void set_val(const std::string& _orm_name, \
                        const unsigned char* _buf,size_t _length,[[maybe_unused]] unsigned char _field_type) { \
                        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_FIELDS(__VA_ARGS__)) \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    
    #define ORM_CMS_OAUTH_CODE_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::oauth_code_info { \
            struct StructName { \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<StructName> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_OAUTH_CODE_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::oauth_code_info { \
            struct StructName { \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS(__VA_ARGS__)) \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_BODY(__VA_ARGS__)); \
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
                        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_FIELDS(__VA_ARGS__)) \
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
        
    
    #define ORM_CMS_OAUTH_CODE_CUST_STRUCT(StructName, CustomDecl, CustomNames, ...) \
        namespace orm::cms::oauth_code_info { \
            struct StructName { \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_PROJ_MEMBERS(__VA_ARGS__)) \
                CustomDecl \
                std::vector<std::unique_ptr<StructName>> children; \
                \
                std::string to_json() const { \
                std::ostringstream oss; \
                oss << '{'; \
                ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_BODY(__VA_ARGS__)); \
    ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_TO_JSON_CUSTOM(ORM_CMS_OAUTH_CODE_UNWRAP CustomNames));  \
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
                        ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_FIELDS(__VA_ARGS__)) \
                    ORM_CMS_OAUTH_CODE_EXPAND(ORM_CMS_OAUTH_CODE_SET_VAL_CUSTOM_FIELDS(CustomNames)) \
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
        
    inline constexpr std::array<std::string_view,10> col_names={"id","code_hash","client_id","user_id","redirect_uri","scope","code_challenge","expires_at","used","family_id"};
	static constexpr std::array<unsigned char,10> col_types={3,254,253,8,253,253,253,8,1,3};
	static constexpr std::array<unsigned int,10> col_length={0,64,64,0,512,256,128,0,0,0};
	static constexpr std::array<unsigned char,10> col_decimals={0,0,0,0,0,0,0,0,0,0};
	static constexpr std::array<bool,10> col_null={false,false,false,false,false,false,false,false,false,false};
	static constexpr std::array<bool,10> col_indexed={true,true,true,false,false,false,false,true,false,false};
	static constexpr std::string_view auto_pk_name ="id";
	static constexpr int auto_pk_index = 0;

}

struct oauth_code_base
{
    using cols = oauth_code_info::cols;
      oauth_code_info::meta data;
    std::vector<oauth_code_info::meta> record;
static constexpr std::string_view _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<oauth_code_info::meta>::iterator begin(){     return record.begin(); }
std::vector<oauth_code_info::meta>::iterator end(){     return record.end(); }
std::vector<oauth_code_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<oauth_code_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="oauth_code";
static constexpr std::string_view org_tablename="oauth_code";
static constexpr std::string_view modelname="OauthCode";
	static constexpr std::array<bool,10> col_need_quote={false,true,true,false,true,true,true,false,false,false};

            std::bitset<10> dirty_bits;
            void clear_dirty() noexcept {
                dirty_bits.reset();
            }

            void set_dirty(std::size_t idx) noexcept {
                if(idx < 10)
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
                        result.push_back(oauth_code_info::col_names[i]);
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

         case 'c':
 switch(coln.size()){  
case 9:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 2; }
 if(colpospppc=='h'){ return 1; }
   	 break;
case 14:
   	 return 6;
break;
 }
 break;
case 'e':
   	 return 7;
break;
case 'f':
   	 return 9;
break;
case 'i':
   	 return 0;
break;
case 'r':
   	 return 4;
break;
case 's':
   	 return 5;
break;
case 'u':
 switch(coln.size()){  
case 4:
   	 return 8;
break;
case 7:
   	 return 3;
break;
 }
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
     oauth_code_info::meta metatemp;    
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
        for(;j<oauth_code_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                   // tempsql<<"`";
                }
                tempsql<<oauth_code_info::col_names[j];
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
tempsql<<",'"<<stringaddslash(data.code_hash)<<"'";
tempsql<<",'"<<stringaddslash(data.client_id)<<"'";
if(data.user_id==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.user_id);
}
tempsql<<",'"<<stringaddslash(data.redirect_uri)<<"'";
tempsql<<",'"<<stringaddslash(data.scope)<<"'";
tempsql<<",'"<<stringaddslash(data.code_challenge)<<"'";
if(data.expires_at==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.expires_at);
}
if(data.used==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.used);
}
if(data.family_id==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.family_id);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string make_data_insert_sql(const oauth_code_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<oauth_code_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                    //tempsql<<"`";
                }
                tempsql<<oauth_code_info::col_names[j];
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
tempsql<<",'"<<stringaddslash(insert_data.code_hash)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.client_id)<<"'";
if(insert_data.user_id==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.user_id);
}
tempsql<<",'"<<stringaddslash(insert_data.redirect_uri)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.scope)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.code_challenge)<<"'";
if(insert_data.expires_at==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.expires_at);
}
if(insert_data.used==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.used);
}
if(insert_data.family_id==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.family_id);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string make_vector_insert_sql(const std::vector<oauth_code_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<oauth_code_info::col_names.size();j++){
                if(j>0){
                    tempsql<<",";
                }else{
                   // tempsql<<"`";
                }
                tempsql<<oauth_code_info::col_names[j];
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
		tempsql<<",'"<<stringaddslash(insert_data[i].code_hash)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].client_id)<<"'";
	if(insert_data[i].user_id==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].user_id);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].redirect_uri)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].scope)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].code_challenge)<<"'";
	if(insert_data[i].expires_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].expires_at);
	}
	if(insert_data[i].used==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].used);
	}
	if(insert_data[i].family_id==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].family_id);
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

        if(data.id==0){
	tempsql<<"id=0";
 }else{ 
	tempsql<<"id="<<std::to_string(data.id);
}
tempsql<<",code_hash='"<<stringaddslash(data.code_hash)<<"'";
tempsql<<",client_id='"<<stringaddslash(data.client_id)<<"'";
if(data.user_id==0){
	tempsql<<",user_id=0";
 }else{ 
	tempsql<<",user_id="<<std::to_string(data.user_id);
}
tempsql<<",redirect_uri='"<<stringaddslash(data.redirect_uri)<<"'";
tempsql<<",scope='"<<stringaddslash(data.scope)<<"'";
tempsql<<",code_challenge='"<<stringaddslash(data.code_challenge)<<"'";
if(data.expires_at==0){
	tempsql<<",expires_at=0";
 }else{ 
	tempsql<<",expires_at="<<std::to_string(data.expires_at);
}
if(data.used==0){
	tempsql<<",used=0";
 }else{ 
	tempsql<<",used="<<std::to_string(data.used);
}
if(data.family_id==0){
	tempsql<<",family_id=0";
 }else{ 
	tempsql<<",family_id="<<std::to_string(data.family_id);
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
if(data.id==0){
	tempsql<<"id=0";
 }else{ 
	tempsql<<"id="<<std::to_string(data.id);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"code_hash='"<<stringaddslash(data.code_hash)<<"'";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"client_id='"<<stringaddslash(data.client_id)<<"'";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.user_id==0){
	tempsql<<"user_id=0";
 }else{ 
	tempsql<<"user_id="<<std::to_string(data.user_id);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"redirect_uri='"<<stringaddslash(data.redirect_uri)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"scope='"<<stringaddslash(data.scope)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"code_challenge='"<<stringaddslash(data.code_challenge)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.expires_at==0){
	tempsql<<"expires_at=0";
 }else{ 
	tempsql<<"expires_at="<<std::to_string(data.expires_at);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.used==0){
	tempsql<<"used=0";
 }else{ 
	tempsql<<"used="<<std::to_string(data.used);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.family_id==0){
	tempsql<<"family_id=0";
 }else{ 
	tempsql<<"family_id="<<std::to_string(data.family_id);
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

    constexpr std::size_t total = oauth_code_info::col_names.size();

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
                        tempsql<<"code_hash='"<<stringaddslash(data.code_hash)<<"'";
                        break;
                    case 2:
                        tempsql<<"client_id='"<<stringaddslash(data.client_id)<<"'";
                        break;
                    case 3:
                        if(data.user_id==0){
                            tempsql<<"user_id=0";
                        }else{ 
                            tempsql<<"user_id="<<std::to_string(data.user_id);
                        }
                        break;
                    case 4:
                        tempsql<<"redirect_uri='"<<stringaddslash(data.redirect_uri)<<"'";
                        break;
                    case 5:
                        tempsql<<"scope='"<<stringaddslash(data.scope)<<"'";
                        break;
                    case 6:
                        tempsql<<"code_challenge='"<<stringaddslash(data.code_challenge)<<"'";
                        break;
                    case 7:
                        if(data.expires_at==0){
                            tempsql<<"expires_at=0";
                        }else{ 
                            tempsql<<"expires_at="<<std::to_string(data.expires_at);
                        }
                        break;
                    case 8:
                        if(data.used==0){
                            tempsql<<"used=0";
                        }else{ 
                            tempsql<<"used="<<std::to_string(data.used);
                        }
                        break;
                    case 9:
                        if(data.family_id==0){
                            tempsql<<"family_id=0";
                        }else{ 
                            tempsql<<"family_id="<<std::to_string(data.family_id);
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
        for (; j < oauth_code_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << ",";
            }
            else
            {
                tempsql << "";
            }
            tempsql << oauth_code_info::col_names[j];
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
	tempsql<<",'"<<stringaddslash(record[i].code_hash)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].client_id)<<"'";
	if(record[i].user_id==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].user_id);
	}
	tempsql<<",'"<<stringaddslash(record[i].redirect_uri)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].scope)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].code_challenge)<<"'";
	if(record[i].expires_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expires_at);
	}
	if(record[i].used==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].used);
	}
	if(record[i].family_id==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].family_id);
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
        for (; j < oauth_code_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << ",";
            }
            else
            {
                tempsql << "";
            }
            tempsql << oauth_code_info::col_names[j];
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
	tempsql<<",'"<<stringaddslash(record[i].code_hash)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].client_id)<<"'";
	if(record[i].user_id==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].user_id);
	}
	tempsql<<",'"<<stringaddslash(record[i].redirect_uri)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].scope)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].code_challenge)<<"'";
	if(record[i].expires_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expires_at);
	}
	if(record[i].used==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].used);
	}
	if(record[i].family_id==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].family_id);
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
            for(jj=0;jj<oauth_code_info::col_names.size();jj++){
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
	temparray.push_back(data.code_hash);
 break;
 case 2:
	temparray.push_back(data.client_id);
 break;
 case 3:
if(data.user_id==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.user_id));
}
 break;
 case 4:
	temparray.push_back(data.redirect_uri);
 break;
 case 5:
	temparray.push_back(data.scope);
 break;
 case 6:
	temparray.push_back(data.code_challenge);
 break;
 case 7:
if(data.expires_at==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expires_at));
}
 break;
 case 8:
if(data.used==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.used));
}
 break;
 case 9:
if(data.family_id==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.family_id));
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
            for(jj=0;jj<oauth_code_info::col_names.size();jj++){
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
	tempsql.insert({"code_hash",data.code_hash});
 break;
 case 2:
	tempsql.insert({"client_id",data.client_id});
 break;
 case 3:
if(data.user_id==0){
	tempsql.insert({"user_id","0"});
 }else{ 
	tempsql.insert({"user_id",std::to_string(data.user_id)});
}
 break;
 case 4:
	tempsql.insert({"redirect_uri",data.redirect_uri});
 break;
 case 5:
	tempsql.insert({"scope",data.scope});
 break;
 case 6:
	tempsql.insert({"code_challenge",data.code_challenge});
 break;
 case 7:
if(data.expires_at==0){
	tempsql.insert({"expires_at","0"});
 }else{ 
	tempsql.insert({"expires_at",std::to_string(data.expires_at)});
}
 break;
 case 8:
if(data.used==0){
	tempsql.insert({"used","0"});
 }else{ 
	tempsql.insert({"used",std::to_string(data.used)});
}
 break;
 case 9:
if(data.family_id==0){
	tempsql.insert({"family_id","0"});
 }else{ 
	tempsql.insert({"family_id",std::to_string(data.family_id)});
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
if(data.id==0){
	tempsql<<"\"id\":0";
 }else{ 
	tempsql<<"\"id\":"<<std::to_string(data.id);
}
tempsql<<",\"code_hash\":\""<<http::utf8_to_jsonstring(data.code_hash);
tempsql<<"\"";
tempsql<<",\"client_id\":\""<<http::utf8_to_jsonstring(data.client_id);
tempsql<<"\"";
if(data.user_id==0){
	tempsql<<",\"user_id\":0";
 }else{ 
	tempsql<<",\"user_id\":"<<std::to_string(data.user_id);
}
tempsql<<",\"redirect_uri\":\""<<http::utf8_to_jsonstring(data.redirect_uri);
tempsql<<"\"";
tempsql<<",\"scope\":\""<<http::utf8_to_jsonstring(data.scope);
tempsql<<"\"";
tempsql<<",\"code_challenge\":\""<<http::utf8_to_jsonstring(data.code_challenge);
tempsql<<"\"";
if(data.expires_at==0){
	tempsql<<",\"expires_at\":0";
 }else{ 
	tempsql<<",\"expires_at\":"<<std::to_string(data.expires_at);
}
if(data.used==0){
	tempsql<<",\"used\":0";
 }else{ 
	tempsql<<",\"used\":"<<std::to_string(data.used);
}
if(data.family_id==0){
	tempsql<<",\"family_id\":0";
 }else{ 
	tempsql<<",\"family_id\":"<<std::to_string(data.family_id);
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
            for(jj=0;jj<oauth_code_info::col_names.size();jj++){
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
tempsql<<"\"code_hash\":\""<<http::utf8_to_jsonstring(data.code_hash)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"client_id\":\""<<http::utf8_to_jsonstring(data.client_id)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.user_id==0){
	tempsql<<"\"user_id\":0";
 }else{ 
	tempsql<<"\"user_id\":"<<std::to_string(data.user_id);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"redirect_uri\":\""<<http::utf8_to_jsonstring(data.redirect_uri)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"scope\":\""<<http::utf8_to_jsonstring(data.scope)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"code_challenge\":\""<<http::utf8_to_jsonstring(data.code_challenge)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.expires_at==0){
	tempsql<<"\"expires_at\":0";
 }else{ 
	tempsql<<"\"expires_at\":"<<std::to_string(data.expires_at);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.used==0){
	tempsql<<"\"used\":0";
 }else{ 
	tempsql<<"\"used\":"<<std::to_string(data.used);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.family_id==0){
	tempsql<<"\"family_id\":0";
 }else{ 
	tempsql<<"\"family_id\":"<<std::to_string(data.family_id);
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
        oauth_code_info::meta metatemp; 
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
		  http::json_set_val(data.code_hash,set_value_name);
		 break;
		
		case 2:
		  http::json_set_val(data.client_id,set_value_name);
		 break;
		
		case 3:
		  http::json_set_val(data.user_id,set_value_name);
		 break;
		
		case 4:
		  http::json_set_val(data.redirect_uri,set_value_name);
		 break;
		
		case 5:
		  http::json_set_val(data.scope,set_value_name);
		 break;
		
		case 6:
		  http::json_set_val(data.code_challenge,set_value_name);
		 break;
		
		case 7:
		  http::json_set_val(data.expires_at,set_value_name);
		 break;
		
		case 8:
		  http::json_set_val(data.used,set_value_name);
		 break;
		
		case 9:
		  http::json_set_val(data.family_id,set_value_name);
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
        for(jj=0;jj<oauth_code_info::col_names.size();jj++){
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
tempsql<<"\"code_hash\":\""<<http::utf8_to_jsonstring(record[n].code_hash)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"client_id\":\""<<http::utf8_to_jsonstring(record[n].client_id)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].user_id==0){
	tempsql<<"\"user_id\":0";
 }else{ 
	tempsql<<"\"user_id\":"<<std::to_string(record[n].user_id);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"redirect_uri\":\""<<http::utf8_to_jsonstring(record[n].redirect_uri)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"scope\":\""<<http::utf8_to_jsonstring(record[n].scope)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"code_challenge\":\""<<http::utf8_to_jsonstring(record[n].code_challenge)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].expires_at==0){
	tempsql<<"\"expires_at\":0";
 }else{ 
	tempsql<<"\"expires_at\":"<<std::to_string(record[n].expires_at);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].used==0){
	tempsql<<"\"used\":0";
 }else{ 
	tempsql<<"\"used\":"<<std::to_string(record[n].used);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].family_id==0){
	tempsql<<"\"family_id\":0";
 }else{ 
	tempsql<<"\"family_id\":"<<std::to_string(record[n].family_id);
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
   
   std::string to_json(std::function<bool(std::string&,oauth_code_info::meta&)> func,std::string_view field=""){
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
            for(jj=0;jj<oauth_code_info::col_names.size();jj++){
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
tempsql<<"\"code_hash\":\""<<http::utf8_to_jsonstring(record[n].code_hash)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"client_id\":\""<<http::utf8_to_jsonstring(record[n].client_id)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].user_id==0){
	tempsql<<"\"user_id\":0";
 }else{ 
	tempsql<<"\"user_id\":"<<std::to_string(record[n].user_id);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"redirect_uri\":\""<<http::utf8_to_jsonstring(record[n].redirect_uri)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"scope\":\""<<http::utf8_to_jsonstring(record[n].scope)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"code_challenge\":\""<<http::utf8_to_jsonstring(record[n].code_challenge)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].expires_at==0){
	tempsql<<"\"expires_at\":0";
 }else{ 
	tempsql<<"\"expires_at\":"<<std::to_string(record[n].expires_at);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].used==0){
	tempsql<<"\"used\":0";
 }else{ 
	tempsql<<"\"used\":"<<std::to_string(record[n].used);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].family_id==0){
	tempsql<<"\"family_id\":0";
 }else{ 
	tempsql<<"\"family_id\":"<<std::to_string(record[n].family_id);
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
   long long getPK(){  return data.id; } 
 void setPK(long long val){  data.id=val;} 
 unsigned  int  getId(){  return data.id; } 
 void setId( unsigned  int  val){  data.id=val;} 

 std::string  getCodeHash(){  return data.code_hash; } 
 std::string & getRefCodeHash(){  return std::ref(data.code_hash); } 
 void setCodeHash( std::string  &val){  data.code_hash=val;
		 set_dirty(1);  }
 void setCodeHash(std::string_view val){  data.code_hash=val;
		 set_dirty(1);  }

 std::string  getClientId(){  return data.client_id; } 
 std::string & getRefClientId(){  return std::ref(data.client_id); } 
 void setClientId( std::string  &val){  data.client_id=val;
		 set_dirty(2);  }
 void setClientId(std::string_view val){  data.client_id=val;
		 set_dirty(2);  }

 long long  getUserId(){  return data.user_id; } 
 void setUserId( long long  val){  data.user_id=val;
		 set_dirty(3);  }

 std::string  getRedirectUri(){  return data.redirect_uri; } 
 std::string & getRefRedirectUri(){  return std::ref(data.redirect_uri); } 
 void setRedirectUri( std::string  &val){  data.redirect_uri=val;
		 set_dirty(4);  }
 void setRedirectUri(std::string_view val){  data.redirect_uri=val;
		 set_dirty(4);  }

 std::string  getScope(){  return data.scope; } 
 std::string & getRefScope(){  return std::ref(data.scope); } 
 void setScope( std::string  &val){  data.scope=val;
		 set_dirty(5);  }
 void setScope(std::string_view val){  data.scope=val;
		 set_dirty(5);  }

 std::string  getCodeChallenge(){  return data.code_challenge; } 
 std::string & getRefCodeChallenge(){  return std::ref(data.code_challenge); } 
 void setCodeChallenge( std::string  &val){  data.code_challenge=val;
		 set_dirty(6);  }
 void setCodeChallenge(std::string_view val){  data.code_challenge=val;
		 set_dirty(6);  }

 long long  getExpiresAt(){  return data.expires_at; } 
 void setExpiresAt( long long  val){  data.expires_at=val;
		 set_dirty(7);  }

 char  getUsed(){  return data.used; } 
 void setUsed( char  val){  data.used=val;
		 set_dirty(8);  }

 unsigned  int  getFamilyId(){  return data.family_id; } 
 void setFamilyId( unsigned  int  val){  data.family_id=val;
		 set_dirty(9);  }

oauth_code_info::meta getnewData(){
 	 struct oauth_code_info::meta newdata;
	 return newdata; 
} 
oauth_code_info::meta getData(){
 	 return data; 
} 
std::vector<oauth_code_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<oauth_code_info::meta_tree> &tree_data, std::string_view field=""){
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
            for(jj=0;jj<oauth_code_info::col_names.size();jj++){
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
tempsql<<"\"code_hash\":\""<<http::utf8_to_jsonstring(tree_data[n].code_hash)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"client_id\":\""<<http::utf8_to_jsonstring(tree_data[n].client_id)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].user_id==0){
	tempsql<<"\"user_id\":0";
 }else{ 
	tempsql<<"\"user_id\":"<<std::to_string(tree_data[n].user_id);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"redirect_uri\":\""<<http::utf8_to_jsonstring(tree_data[n].redirect_uri)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"scope\":\""<<http::utf8_to_jsonstring(tree_data[n].scope)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"code_challenge\":\""<<http::utf8_to_jsonstring(tree_data[n].code_challenge)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expires_at==0){
	tempsql<<"\"expires_at\":0";
 }else{ 
	tempsql<<"\"expires_at\":"<<std::to_string(tree_data[n].expires_at);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].used==0){
	tempsql<<"\"used\":0";
 }else{ 
	tempsql<<"\"used\":"<<std::to_string(tree_data[n].used);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].family_id==0){
	tempsql<<"\"family_id\":0";
 }else{ 
	tempsql<<"\"family_id\":"<<std::to_string(tree_data[n].family_id);
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
   
   std::string tree_tojson(const std::vector<oauth_code_info::meta_tree> &tree_data,std::function<bool(std::string&,const oauth_code_info::meta_tree&)> func,std::string_view field=""){
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
            for(jj=0;jj<oauth_code_info::col_names.size();jj++){
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
tempsql<<"\"code_hash\":\""<<http::utf8_to_jsonstring(tree_data[n].code_hash)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"client_id\":\""<<http::utf8_to_jsonstring(tree_data[n].client_id)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].user_id==0){
	tempsql<<"\"user_id\":0";
 }else{ 
	tempsql<<"\"user_id\":"<<std::to_string(tree_data[n].user_id);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"redirect_uri\":\""<<http::utf8_to_jsonstring(tree_data[n].redirect_uri)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"scope\":\""<<http::utf8_to_jsonstring(tree_data[n].scope)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"code_challenge\":\""<<http::utf8_to_jsonstring(tree_data[n].code_challenge)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expires_at==0){
	tempsql<<"\"expires_at\":0";
 }else{ 
	tempsql<<"\"expires_at\":"<<std::to_string(tree_data[n].expires_at);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].used==0){
	tempsql<<"\"used\":0";
 }else{ 
	tempsql<<"\"used\":"<<std::to_string(tree_data[n].used);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].family_id==0){
	tempsql<<"\"family_id\":0";
 }else{ 
	tempsql<<"\"family_id\":"<<std::to_string(tree_data[n].family_id);
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
   
    template<oauth_code_info::cols KeyCol, oauth_code_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>()));
        using ValType = decltype(oauth_code_info::getField<ValCol>(std::declval<const oauth_code_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(oauth_code_info::getField<KeyCol>(iter), oauth_code_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<oauth_code_info::cols KeyCol, oauth_code_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>())), 
            decltype(oauth_code_info::getField<ValCol>(std::declval<const oauth_code_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>())), 
                    decltype(oauth_code_info::getField<ValCol>(std::declval<const oauth_code_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>()));
        using ValType = decltype(oauth_code_info::getField<ValCol>(std::declval<const oauth_code_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(oauth_code_info::getField<KeyCol>(iter), oauth_code_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(oauth_code_info::getField<KeyCol>(iter), oauth_code_info::getField<ValCol>(iter))) {
                    result.emplace(oauth_code_info::getField<KeyCol>(iter), oauth_code_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<oauth_code_info::cols KeyCol, oauth_code_info::cols ValCol> 
    auto get_cols_vecs()
    {
        using KeyType = decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>()));
        using ValType = decltype(oauth_code_info::getField<ValCol>(std::declval<const oauth_code_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(oauth_code_info::getField<KeyCol>(iter), oauth_code_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vecs<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<oauth_code_info::cols KeyCol, oauth_code_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>())), 
            decltype(oauth_code_info::getField<ValCol>(std::declval<const oauth_code_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>())), 
                    decltype(oauth_code_info::getField<ValCol>(std::declval<const oauth_code_info::meta&>()))>, bool>
    auto get_cols_vecs(Callback&& callback)
    {
        using KeyType = decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>()));
        using ValType = decltype(oauth_code_info::getField<ValCol>(std::declval<const oauth_code_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(oauth_code_info::getField<KeyCol>(iter), oauth_code_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(oauth_code_info::getField<KeyCol>(iter), oauth_code_info::getField<ValCol>(iter))) {
                    result.emplace_back(oauth_code_info::getField<KeyCol>(iter), oauth_code_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<oauth_code_info::cols KeyCol>
    auto get_cols_vec()
    {
        using KeyType = decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(oauth_code_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols_vec<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<oauth_code_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>()))>, bool>
    auto get_cols_vec(Callback&& callback)
    {
        using KeyType = decltype(oauth_code_info::getField<KeyCol>(std::declval<const oauth_code_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(oauth_code_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(oauth_code_info::getField<KeyCol>(iter))) {
                    result.emplace_back(oauth_code_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<oauth_code_info::cols Col>
        requires requires(std::ostream& os, decltype(oauth_code_info::getField<Col>(std::declval<const oauth_code_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << oauth_code_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<oauth_code_info::cols Col>
        requires requires(std::ostream& os, decltype(oauth_code_info::getField<Col>(std::declval<const oauth_code_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << oauth_code_info::getField<Col>(iter); 
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
   