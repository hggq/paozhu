#ifndef ORM_CMS_HOMEBLOCKBASEMATA_H
#define ORM_CMS_HOMEBLOCKBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sat, 13 Jun 2026 01:24:53 GMT
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

namespace homeblock_info
{
 
    enum class cols : unsigned char 
    {
		hbid = 0,
		userid = 1,
		title = 2,
		content = 3,
		jsonconfig = 4,
		viewtype = 5,
		gettype = 6,
		rownum = 7,
		width = 8,
		height = 9,
		strlength = 10,
		sortid = 11,

    };

    struct meta
    {
		 unsigned  int  hbid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 std::string  title = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  jsonconfig = ""; ///**/
		 unsigned  char  viewtype = 0; ///*display type sample or list*/
		 unsigned  char  gettype = 0; ///**/
		 unsigned  int  rownum = 0; ///**/
		 unsigned  int  width = 0; ///**/
		 unsigned  int  height = 0; ///**/
		 unsigned  int  strlength = 0; ///**/
		 int  sortid = 0; ///**/
	};
  
    struct meta_tree
    {
		 unsigned  int  hbid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 std::string  title = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  jsonconfig = ""; ///**/
		 unsigned  char  viewtype = 0; ///*display type sample or list*/
		 unsigned  char  gettype = 0; ///**/
		 unsigned  int  rownum = 0; ///**/
		 unsigned  int  width = 0; ///**/
		 unsigned  int  height = 0; ///**/
		 unsigned  int  strlength = 0; ///**/
		 int  sortid = 0; ///**/

	 std::vector<meta_tree> children;
 };
  
    struct meta_tree_ptr
    {
		 unsigned  int  hbid = 0; ///**/
		 unsigned  int  userid = 0; ///**/
		 std::string  title = ""; ///**/
		 std::string  content = ""; ///**/
		 std::string  jsonconfig = ""; ///**/
		 unsigned  char  viewtype = 0; ///*display type sample or list*/
		 unsigned  char  gettype = 0; ///**/
		 unsigned  int  rownum = 0; ///**/
		 unsigned  int  width = 0; ///**/
		 unsigned  int  height = 0; ///**/
		 unsigned  int  strlength = 0; ///**/
		 int  sortid = 0; ///**/

	 std::vector<std::unique_ptr<meta_tree>> children;
 };
 
    template<cols Col>
    auto getField(const meta& m) 
    {
    	if constexpr (Col == cols::hbid) { 
		 return m.hbid;
		} else if constexpr (Col == cols::userid) { 
		 return m.userid;
		} else if constexpr (Col == cols::title) { 
		 return m.title;
		} else if constexpr (Col == cols::content) { 
		 return m.content;
		} else if constexpr (Col == cols::jsonconfig) { 
		 return m.jsonconfig;
		} else if constexpr (Col == cols::viewtype) { 
		 return m.viewtype;
		} else if constexpr (Col == cols::gettype) { 
		 return m.gettype;
		} else if constexpr (Col == cols::rownum) { 
		 return m.rownum;
		} else if constexpr (Col == cols::width) { 
		 return m.width;
		} else if constexpr (Col == cols::height) { 
		 return m.height;
		} else if constexpr (Col == cols::strlength) { 
		 return m.strlength;
		} else if constexpr (Col == cols::sortid) { 
		 return m.sortid;
		
        } else {
            static_assert(false, "Unsupported column type");
        }
    }
    
    namespace type {
		using hbid = decltype(std::declval<const meta>().hbid);
		using userid = decltype(std::declval<const meta>().userid);
		using title = decltype(std::declval<const meta>().title);
		using content = decltype(std::declval<const meta>().content);
		using jsonconfig = decltype(std::declval<const meta>().jsonconfig);
		using viewtype = decltype(std::declval<const meta>().viewtype);
		using gettype = decltype(std::declval<const meta>().gettype);
		using rownum = decltype(std::declval<const meta>().rownum);
		using width = decltype(std::declval<const meta>().width);
		using height = decltype(std::declval<const meta>().height);
		using strlength = decltype(std::declval<const meta>().strlength);
		using sortid = decltype(std::declval<const meta>().sortid);

    }

    
    #define ORM_CMS_HOMEBLOCK_META_FIELD_TYPE(col) \
        decltype(orm::cms::homeblock_info::getField<orm::cms::homeblock_info::cols::col>(std::declval<const orm::cms::homeblock_info::meta&>()))

    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBER(col) \
          ORM_CMS_HOMEBLOCK_META_FIELD_TYPE(col) col;
                 
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_1(c1) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c1) 
     
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_2( c1, c2) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_1( c1) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c2)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_3( c1, c2, c3) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_2( c1, c2) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c3)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_4( c1, c2, c3, c4) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_3( c1, c2, c3) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c4)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_4( c1, c2, c3, c4) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c5)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_5( c1, c2, c3, c4, c5) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c6)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_6( c1, c2, c3, c4, c5, c6) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c7)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_7( c1, c2, c3, c4, c5, c6, c7) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c8)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_8( c1, c2, c3, c4, c5, c6, c7, c8) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c9)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_9( c1, c2, c3, c4, c5, c6, c7, c8, c9) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c10)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_10( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c11)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_11( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c12)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_12( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c13)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_13( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c14)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_14( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c15)
         
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_16( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) \
        ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_15( c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) ORM_CMS_HOMEBLOCK_PROJ_MEMBER(c16)
         
    #define ORM_CMS_HOMEBLOCK_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME 
    
     
    #define ORM_CMS_HOMEBLOCK_PROJ_MEMBERS(...) \
        ORM_CMS_HOMEBLOCK_GET_MACRO(__VA_ARGS__, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_16, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_15, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_14, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_13, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_12, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_11, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_10, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_9, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_8, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_7, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_6, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_5, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_4, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_3, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_2, \
            ORM_CMS_HOMEBLOCK_PROJ_MEMBERS_1, \
        )(__VA_ARGS__)

    
    #define ORM_CMS_HOMEBLOCK_DEFINE_STRUCT(StructName, ...) \
        namespace orm::cms::homeblock_info { \
            struct StructName { \
                ORM_CMS_HOMEBLOCK_PROJ_MEMBERS(__VA_ARGS__) \
            }; \
       }
        
    
    #define ORM_CMS_HOMEBLOCK_TREE_STRUCT(StructName, ...) \
        namespace orm::cms::homeblock_info { \
            struct StructName##_tree { \
                ORM_CMS_HOMEBLOCK_PROJ_MEMBERS(__VA_ARGS__) \
                std::vector<StructName##_tree> children; \
            }; \
       }
        
    
    #define ORM_CMS_HOMEBLOCK_TREE_PTR_STRUCT(StructName, ...) \
        namespace orm::cms::homeblock_info { \
            struct StructName##_tree_ptr { \
                ORM_CMS_HOMEBLOCK_PROJ_MEMBERS(__VA_ARGS__) \
                std::vector<std::unique_ptr<StructName##_tree_ptr>> children; \
            }; \
       }
        
    static constexpr std::array<std::string_view,12> col_names={"hbid","userid","title","content","jsonconfig","viewtype","gettype","rownum","width","height","strlength","sortid"};
	static constexpr std::array<unsigned char,12> col_types={3,3,253,252,252,1,1,3,3,3,3,3};
	static constexpr std::array<unsigned char,12> col_length={0,0,120,0,0,0,0,0,0,0,0,0};
	static constexpr std::array<unsigned char,12> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0};

}

struct homeblock_base
{
      homeblock_info::meta data;
    std::vector<homeblock_info::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<homeblock_info::meta>::iterator begin(){     return record.begin(); }
std::vector<homeblock_info::meta>::iterator end(){     return record.end(); }
std::vector<homeblock_info::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<homeblock_info::meta>::const_iterator end() const{     return record.end(); }
std::string tablename="homeblock";
static constexpr std::string_view modelname="Homeblock";

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


         case 'c':
   	 return 3;
break;
case 'g':
   	 return 6;
break;
case 'h':
 switch(coln.size()){  
case 4:
   	 return 0;
break;
case 6:
   	 return 9;
break;
 }
 break;
case 'j':
   	 return 4;
break;
case 'r':
   	 return 7;
break;
case 's':
 switch(coln.size()){  
case 6:
   	 return 11;
break;
case 9:
   	 return 10;
break;
 }
 break;
case 't':
   	 return 2;
break;
case 'u':
   	 return 1;
break;
case 'v':
   	 return 5;
break;
case 'w':
   	 return 8;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "hbid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     homeblock_info::meta metatemp;    
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
        for(;j<homeblock_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<homeblock_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.hbid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.hbid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
tempsql<<",'"<<stringaddslash(data.title)<<"'";
tempsql<<",'"<<stringaddslash(data.content)<<"'";
tempsql<<",'"<<stringaddslash(data.jsonconfig)<<"'";
if(data.viewtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.viewtype);
}
if(data.gettype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.gettype);
}
if(data.rownum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.rownum);
}
if(data.width==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.width);
}
if(data.height==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.height);
}
if(data.strlength==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.strlength);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const homeblock_info::meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<homeblock_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<homeblock_info::col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.hbid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.hbid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.content)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.jsonconfig)<<"'";
if(insert_data.viewtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.viewtype);
}
if(insert_data.gettype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.gettype);
}
if(insert_data.rownum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.rownum);
}
if(insert_data.width==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.width);
}
if(insert_data.height==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.height);
}
if(insert_data.strlength==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.strlength);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<homeblock_info::meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<homeblock_info::col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<homeblock_info::col_names[j];
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


            	if(insert_data[i].hbid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].hbid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].content)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].jsonconfig)<<"'";
	if(insert_data[i].viewtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].viewtype);
	}
	if(insert_data[i].gettype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].gettype);
	}
	if(insert_data[i].rownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].rownum);
	}
	if(insert_data[i].width==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].width);
	}
	if(insert_data[i].height==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].height);
	}
	if(insert_data[i].strlength==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].strlength);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
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

        if(data.hbid==0){
	tempsql<<"`hbid`=0";
 }else{ 
	tempsql<<"`hbid`="<<std::to_string(data.hbid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
tempsql<<",`content`='"<<stringaddslash(data.content)<<"'";
tempsql<<",`jsonconfig`='"<<stringaddslash(data.jsonconfig)<<"'";
if(data.viewtype==0){
	tempsql<<",`viewtype`=0";
 }else{ 
	tempsql<<",`viewtype`="<<std::to_string(data.viewtype);
}
if(data.gettype==0){
	tempsql<<",`gettype`=0";
 }else{ 
	tempsql<<",`gettype`="<<std::to_string(data.gettype);
}
if(data.rownum==0){
	tempsql<<",`rownum`=0";
 }else{ 
	tempsql<<",`rownum`="<<std::to_string(data.rownum);
}
if(data.width==0){
	tempsql<<",`width`=0";
 }else{ 
	tempsql<<",`width`="<<std::to_string(data.width);
}
if(data.height==0){
	tempsql<<",`height`=0";
 }else{ 
	tempsql<<",`height`="<<std::to_string(data.height);
}
if(data.strlength==0){
	tempsql<<",`strlength`=0";
 }else{ 
	tempsql<<",`strlength`="<<std::to_string(data.strlength);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
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
if(data.hbid==0){
	tempsql<<"`hbid`=0";
 }else{ 
	tempsql<<"`hbid`="<<std::to_string(data.hbid);
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
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`content`='"<<stringaddslash(data.content)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`jsonconfig`='"<<stringaddslash(data.jsonconfig)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.viewtype==0){
	tempsql<<"`viewtype`=0";
 }else{ 
	tempsql<<"`viewtype`="<<std::to_string(data.viewtype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.gettype==0){
	tempsql<<"`gettype`=0";
 }else{ 
	tempsql<<"`gettype`="<<std::to_string(data.gettype);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.rownum==0){
	tempsql<<"`rownum`=0";
 }else{ 
	tempsql<<"`rownum`="<<std::to_string(data.rownum);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.width==0){
	tempsql<<"`width`=0";
 }else{ 
	tempsql<<"`width`="<<std::to_string(data.width);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.height==0){
	tempsql<<"`height`=0";
 }else{ 
	tempsql<<"`height`="<<std::to_string(data.height);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.strlength==0){
	tempsql<<"`strlength`=0";
 }else{ 
	tempsql<<"`strlength`="<<std::to_string(data.strlength);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
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
        for (; j < homeblock_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << homeblock_info::col_names[j];
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
            	if(record[i].hbid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].hbid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jsonconfig)<<"'";
	if(record[i].viewtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].viewtype);
	}
	if(record[i].gettype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].gettype);
	}
	if(record[i].rownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rownum);
	}
	if(record[i].width==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].width);
	}
	if(record[i].height==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].height);
	}
	if(record[i].strlength==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].strlength);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
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
        for (; j < homeblock_info::col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << homeblock_info::col_names[j];
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
            	if(record[i].hbid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].hbid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jsonconfig)<<"'";
	if(record[i].viewtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].viewtype);
	}
	if(record[i].gettype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].gettype);
	}
	if(record[i].rownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rownum);
	}
	if(record[i].width==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].width);
	}
	if(record[i].height==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].height);
	}
	if(record[i].strlength==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].strlength);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
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
            for(jj=0;jj<homeblock_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.hbid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.hbid));
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
	temparray.push_back(data.title);
 break;
 case 3:
	temparray.push_back(data.content);
 break;
 case 4:
	temparray.push_back(data.jsonconfig);
 break;
 case 5:
if(data.viewtype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.viewtype));
}
 break;
 case 6:
if(data.gettype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.gettype));
}
 break;
 case 7:
if(data.rownum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.rownum));
}
 break;
 case 8:
if(data.width==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.width));
}
 break;
 case 9:
if(data.height==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.height));
}
 break;
 case 10:
if(data.strlength==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.strlength));
}
 break;
 case 11:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
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
            for(jj=0;jj<homeblock_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.hbid==0){
	tempsql.insert({"hbid","0"});
 }else{ 
	tempsql.insert({"hbid",std::to_string(data.hbid)});
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
	tempsql.insert({"title",data.title});
 break;
 case 3:
	tempsql.insert({"content",data.content});
 break;
 case 4:
	tempsql.insert({"jsonconfig",data.jsonconfig});
 break;
 case 5:
if(data.viewtype==0){
	tempsql.insert({"viewtype","0"});
 }else{ 
	tempsql.insert({"viewtype",std::to_string(data.viewtype)});
}
 break;
 case 6:
if(data.gettype==0){
	tempsql.insert({"gettype","0"});
 }else{ 
	tempsql.insert({"gettype",std::to_string(data.gettype)});
}
 break;
 case 7:
if(data.rownum==0){
	tempsql.insert({"rownum","0"});
 }else{ 
	tempsql.insert({"rownum",std::to_string(data.rownum)});
}
 break;
 case 8:
if(data.width==0){
	tempsql.insert({"width","0"});
 }else{ 
	tempsql.insert({"width",std::to_string(data.width)});
}
 break;
 case 9:
if(data.height==0){
	tempsql.insert({"height","0"});
 }else{ 
	tempsql.insert({"height",std::to_string(data.height)});
}
 break;
 case 10:
if(data.strlength==0){
	tempsql.insert({"strlength","0"});
 }else{ 
	tempsql.insert({"strlength",std::to_string(data.strlength)});
}
 break;
 case 11:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
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
if(data.hbid==0){
	tempsql<<"\"hbid\":0";
 }else{ 
	tempsql<<"\"hbid\":"<<std::to_string(data.hbid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"content\":\""<<http::utf8_to_jsonstring(data.content);
tempsql<<"\"";
tempsql<<",\"jsonconfig\":\""<<http::utf8_to_jsonstring(data.jsonconfig);
tempsql<<"\"";
if(data.viewtype==0){
	tempsql<<",\"viewtype\":0";
 }else{ 
	tempsql<<",\"viewtype\":"<<std::to_string(data.viewtype);
}
if(data.gettype==0){
	tempsql<<",\"gettype\":0";
 }else{ 
	tempsql<<",\"gettype\":"<<std::to_string(data.gettype);
}
if(data.rownum==0){
	tempsql<<",\"rownum\":0";
 }else{ 
	tempsql<<",\"rownum\":"<<std::to_string(data.rownum);
}
if(data.width==0){
	tempsql<<",\"width\":0";
 }else{ 
	tempsql<<",\"width\":"<<std::to_string(data.width);
}
if(data.height==0){
	tempsql<<",\"height\":0";
 }else{ 
	tempsql<<",\"height\":"<<std::to_string(data.height);
}
if(data.strlength==0){
	tempsql<<",\"strlength\":0";
 }else{ 
	tempsql<<",\"strlength\":"<<std::to_string(data.strlength);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
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
            for(jj=0;jj<homeblock_info::col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.hbid==0){
	tempsql<<"\"hbid\":0";
 }else{ 
	tempsql<<"\"hbid\":"<<std::to_string(data.hbid);
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
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jsonconfig\":\""<<http::utf8_to_jsonstring(data.jsonconfig)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.viewtype==0){
	tempsql<<"\"viewtype\":0";
 }else{ 
	tempsql<<"\"viewtype\":"<<std::to_string(data.viewtype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.gettype==0){
	tempsql<<"\"gettype\":0";
 }else{ 
	tempsql<<"\"gettype\":"<<std::to_string(data.gettype);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.rownum==0){
	tempsql<<"\"rownum\":0";
 }else{ 
	tempsql<<"\"rownum\":"<<std::to_string(data.rownum);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.width==0){
	tempsql<<"\"width\":0";
 }else{ 
	tempsql<<"\"width\":"<<std::to_string(data.width);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.height==0){
	tempsql<<"\"height\":0";
 }else{ 
	tempsql<<"\"height\":"<<std::to_string(data.height);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.strlength==0){
	tempsql<<"\"strlength\":0";
 }else{ 
	tempsql<<"\"strlength\":"<<std::to_string(data.strlength);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
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
        homeblock_info::meta metatemp; 
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
			data.hbid=std::stoul(set_value_name);
		}catch (...) { 
			data.hbid=0;
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
			data.title.append(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 3:
		 try{
			data.content.append(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 4:
		 try{
			data.jsonconfig.append(set_value_name);
		}catch (...) { 
			data.jsonconfig.clear();
			 }
			break;
		case 5:
		 try{
			data.viewtype=std::stoi(set_value_name);
		}catch (...) { 
			data.viewtype=0;
			 }
			break;
		case 6:
		 try{
			data.gettype=std::stoi(set_value_name);
		}catch (...) { 
			data.gettype=0;
			 }
			break;
		case 7:
		 try{
			data.rownum=std::stoul(set_value_name);
		}catch (...) { 
			data.rownum=0;
			 }
			break;
		case 8:
		 try{
			data.width=std::stoul(set_value_name);
		}catch (...) { 
			data.width=0;
			 }
			break;
		case 9:
		 try{
			data.height=std::stoul(set_value_name);
		}catch (...) { 
			data.height=0;
			 }
			break;
		case 10:
		 try{
			data.strlength=std::stoul(set_value_name);
		}catch (...) { 
			data.strlength=0;
			 }
			break;
		case 11:
		 try{
			data.sortid=std::stoi(set_value_name);
		}catch (...) { 
			data.sortid=0;
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
			data.hbid=set_value_name;
		}catch (...) { 
			data.hbid=0;
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
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 3:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 4:
		 try{
			data.jsonconfig=std::to_string(set_value_name);
		}catch (...) { 
			data.jsonconfig.clear();
			 }
			break;
		case 5:
		 try{
			data.viewtype=set_value_name;
		}catch (...) { 
			data.viewtype=0;
			 }
			break;
		case 6:
		 try{
			data.gettype=set_value_name;
		}catch (...) { 
			data.gettype=0;
			 }
			break;
		case 7:
		 try{
			data.rownum=set_value_name;
		}catch (...) { 
			data.rownum=0;
			 }
			break;
		case 8:
		 try{
			data.width=set_value_name;
		}catch (...) { 
			data.width=0;
			 }
			break;
		case 9:
		 try{
			data.height=set_value_name;
		}catch (...) { 
			data.height=0;
			 }
			break;
		case 10:
		 try{
			data.strlength=set_value_name;
		}catch (...) { 
			data.strlength=0;
			 }
			break;
		case 11:
		 try{
			data.sortid=set_value_name;
		}catch (...) { 
			data.sortid=0;
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
			data.hbid=(unsigned int)set_value_name;
		}catch (...) { 
			data.hbid=0;
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
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 3:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 4:
		 try{
			data.jsonconfig=std::to_string(set_value_name);
		}catch (...) { 
			data.jsonconfig.clear();
			 }
			break;
		case 5:
		 try{
			data.viewtype=(int)set_value_name;
		}catch (...) { 
			data.viewtype=0;
			 }
			break;
		case 6:
		 try{
			data.gettype=(int)set_value_name;
		}catch (...) { 
			data.gettype=0;
			 }
			break;
		case 7:
		 try{
			data.rownum=(unsigned int)set_value_name;
		}catch (...) { 
			data.rownum=0;
			 }
			break;
		case 8:
		 try{
			data.width=(unsigned int)set_value_name;
		}catch (...) { 
			data.width=0;
			 }
			break;
		case 9:
		 try{
			data.height=(unsigned int)set_value_name;
		}catch (...) { 
			data.height=0;
			 }
			break;
		case 10:
		 try{
			data.strlength=(unsigned int)set_value_name;
		}catch (...) { 
			data.strlength=0;
			 }
			break;
		case 11:
		 try{
			data.sortid=(int)set_value_name;
		}catch (...) { 
			data.sortid=0;
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
        for(jj=0;jj<homeblock_info::col_names.size();jj++){
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
if(record[n].hbid==0){
	tempsql<<"\"hbid\":0";
 }else{ 
	tempsql<<"\"hbid\":"<<std::to_string(record[n].hbid);
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
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jsonconfig\":\""<<http::utf8_to_jsonstring(record[n].jsonconfig)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].viewtype==0){
	tempsql<<"\"viewtype\":0";
 }else{ 
	tempsql<<"\"viewtype\":"<<std::to_string(record[n].viewtype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].gettype==0){
	tempsql<<"\"gettype\":0";
 }else{ 
	tempsql<<"\"gettype\":"<<std::to_string(record[n].gettype);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].rownum==0){
	tempsql<<"\"rownum\":0";
 }else{ 
	tempsql<<"\"rownum\":"<<std::to_string(record[n].rownum);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].width==0){
	tempsql<<"\"width\":0";
 }else{ 
	tempsql<<"\"width\":"<<std::to_string(record[n].width);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].height==0){
	tempsql<<"\"height\":0";
 }else{ 
	tempsql<<"\"height\":"<<std::to_string(record[n].height);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].strlength==0){
	tempsql<<"\"strlength\":0";
 }else{ 
	tempsql<<"\"strlength\":"<<std::to_string(record[n].strlength);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
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
   
   std::string to_json(std::function<bool(std::string&,homeblock_info::meta&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<homeblock_info::col_names.size();jj++){
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
if(record[n].hbid==0){
	tempsql<<"\"hbid\":0";
 }else{ 
	tempsql<<"\"hbid\":"<<std::to_string(record[n].hbid);
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
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jsonconfig\":\""<<http::utf8_to_jsonstring(record[n].jsonconfig)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].viewtype==0){
	tempsql<<"\"viewtype\":0";
 }else{ 
	tempsql<<"\"viewtype\":"<<std::to_string(record[n].viewtype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].gettype==0){
	tempsql<<"\"gettype\":0";
 }else{ 
	tempsql<<"\"gettype\":"<<std::to_string(record[n].gettype);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].rownum==0){
	tempsql<<"\"rownum\":0";
 }else{ 
	tempsql<<"\"rownum\":"<<std::to_string(record[n].rownum);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].width==0){
	tempsql<<"\"width\":0";
 }else{ 
	tempsql<<"\"width\":"<<std::to_string(record[n].width);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].height==0){
	tempsql<<"\"height\":0";
 }else{ 
	tempsql<<"\"height\":"<<std::to_string(record[n].height);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].strlength==0){
	tempsql<<"\"strlength\":0";
 }else{ 
	tempsql<<"\"strlength\":"<<std::to_string(record[n].strlength);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
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
   long long getPK(){  return data.hbid; } 
 void setPK(long long val){  data.hbid=val;} 
 unsigned  int  getHbid(){  return data.hbid; } 
 void setHbid( unsigned  int  val){  data.hbid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 std::string  getTitle(){  return data.title; } 
 std::string & getRefTitle(){  return std::ref(data.title); } 
 void setTitle( std::string  &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

 std::string  getContent(){  return data.content; } 
 std::string & getRefContent(){  return std::ref(data.content); } 
 void setContent( std::string  &val){  data.content=val;} 
 void setContent(std::string_view val){  data.content=val;} 

 std::string  getJsonconfig(){  return data.jsonconfig; } 
 std::string & getRefJsonconfig(){  return std::ref(data.jsonconfig); } 
 void setJsonconfig( std::string  &val){  data.jsonconfig=val;} 
 void setJsonconfig(std::string_view val){  data.jsonconfig=val;} 

 unsigned  char  getViewtype(){  return data.viewtype; } 
 void setViewtype( unsigned  char  val){  data.viewtype=val;} 

 unsigned  char  getGettype(){  return data.gettype; } 
 void setGettype( unsigned  char  val){  data.gettype=val;} 

 unsigned  int  getRownum(){  return data.rownum; } 
 void setRownum( unsigned  int  val){  data.rownum=val;} 

 unsigned  int  getWidth(){  return data.width; } 
 void setWidth( unsigned  int  val){  data.width=val;} 

 unsigned  int  getHeight(){  return data.height; } 
 void setHeight( unsigned  int  val){  data.height=val;} 

 unsigned  int  getStrlength(){  return data.strlength; } 
 void setStrlength( unsigned  int  val){  data.strlength=val;} 

 int  getSortid(){  return data.sortid; } 
 void setSortid( int  val){  data.sortid=val;} 

homeblock_info::meta getnewData(){
 	 struct homeblock_info::meta newdata;
	 return newdata; 
} 
homeblock_info::meta getData(){
 	 return data; 
} 
std::vector<homeblock_info::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<homeblock_info::meta_tree> &tree_data, std::string_view fileld=""){
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
            for(jj=0;jj<homeblock_info::col_names.size();jj++){
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
if(tree_data[n].hbid==0){
	tempsql<<"\"hbid\":0";
 }else{ 
	tempsql<<"\"hbid\":"<<std::to_string(tree_data[n].hbid);
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
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jsonconfig\":\""<<http::utf8_to_jsonstring(tree_data[n].jsonconfig)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].viewtype==0){
	tempsql<<"\"viewtype\":0";
 }else{ 
	tempsql<<"\"viewtype\":"<<std::to_string(tree_data[n].viewtype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].gettype==0){
	tempsql<<"\"gettype\":0";
 }else{ 
	tempsql<<"\"gettype\":"<<std::to_string(tree_data[n].gettype);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].rownum==0){
	tempsql<<"\"rownum\":0";
 }else{ 
	tempsql<<"\"rownum\":"<<std::to_string(tree_data[n].rownum);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].width==0){
	tempsql<<"\"width\":0";
 }else{ 
	tempsql<<"\"width\":"<<std::to_string(tree_data[n].width);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].height==0){
	tempsql<<"\"height\":0";
 }else{ 
	tempsql<<"\"height\":"<<std::to_string(tree_data[n].height);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].strlength==0){
	tempsql<<"\"strlength\":0";
 }else{ 
	tempsql<<"\"strlength\":"<<std::to_string(tree_data[n].strlength);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
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
   
   std::string tree_tojson(const std::vector<homeblock_info::meta_tree> &tree_data,std::function<bool(std::string&,const homeblock_info::meta_tree&)> func,std::string_view fileld=""){
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
            for(jj=0;jj<homeblock_info::col_names.size();jj++){
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
if(tree_data[n].hbid==0){
	tempsql<<"\"hbid\":0";
 }else{ 
	tempsql<<"\"hbid\":"<<std::to_string(tree_data[n].hbid);
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
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(tree_data[n].content)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jsonconfig\":\""<<http::utf8_to_jsonstring(tree_data[n].jsonconfig)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].viewtype==0){
	tempsql<<"\"viewtype\":0";
 }else{ 
	tempsql<<"\"viewtype\":"<<std::to_string(tree_data[n].viewtype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].gettype==0){
	tempsql<<"\"gettype\":0";
 }else{ 
	tempsql<<"\"gettype\":"<<std::to_string(tree_data[n].gettype);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].rownum==0){
	tempsql<<"\"rownum\":0";
 }else{ 
	tempsql<<"\"rownum\":"<<std::to_string(tree_data[n].rownum);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].width==0){
	tempsql<<"\"width\":0";
 }else{ 
	tempsql<<"\"width\":"<<std::to_string(tree_data[n].width);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].height==0){
	tempsql<<"\"height\":0";
 }else{ 
	tempsql<<"\"height\":"<<std::to_string(tree_data[n].height);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].strlength==0){
	tempsql<<"\"strlength\":0";
 }else{ 
	tempsql<<"\"strlength\":"<<std::to_string(tree_data[n].strlength);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
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
   
    template<homeblock_info::cols KeyCol, homeblock_info::cols ValCol> 
    auto get_cols()
    {
        using KeyType = decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>()));
        using ValType = decltype(homeblock_info::getField<ValCol>(std::declval<const homeblock_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) {
            result.emplace(homeblock_info::getField<KeyCol>(iter), homeblock_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<homeblock_info::cols KeyCol, homeblock_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>())), 
            decltype(homeblock_info::getField<ValCol>(std::declval<const homeblock_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>())), 
                    decltype(homeblock_info::getField<ValCol>(std::declval<const homeblock_info::meta&>()))>, bool>
    auto get_cols(Callback&& callback)
    {
        using KeyType = decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>()));
        using ValType = decltype(homeblock_info::getField<ValCol>(std::declval<const homeblock_info::meta&>()));

        std::map<KeyType, ValType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace(homeblock_info::getField<KeyCol>(iter), homeblock_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(homeblock_info::getField<KeyCol>(iter), homeblock_info::getField<ValCol>(iter))) {
                    result.emplace(homeblock_info::getField<KeyCol>(iter), homeblock_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<homeblock_info::cols KeyCol, homeblock_info::cols ValCol> 
    auto get_vec_cols()
    {
        using KeyType = decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>()));
        using ValType = decltype(homeblock_info::getField<ValCol>(std::declval<const homeblock_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) {
            result.emplace_back(homeblock_info::getField<KeyCol>(iter), homeblock_info::getField<ValCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_cols<..,..>([](const auto& key, const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<homeblock_info::cols KeyCol, homeblock_info::cols ValCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>())), 
            decltype(homeblock_info::getField<ValCol>(std::declval<const homeblock_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>())), 
                    decltype(homeblock_info::getField<ValCol>(std::declval<const homeblock_info::meta&>()))>, bool>
    auto get_vec_cols(Callback&& callback)
    {
        using KeyType = decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>()));
        using ValType = decltype(homeblock_info::getField<ValCol>(std::declval<const homeblock_info::meta&>()));

        std::vector<std::pair<KeyType, ValType>> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(homeblock_info::getField<KeyCol>(iter), homeblock_info::getField<ValCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(homeblock_info::getField<KeyCol>(iter), homeblock_info::getField<ValCol>(iter))) {
                    result.emplace_back(homeblock_info::getField<KeyCol>(iter), homeblock_info::getField<ValCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<homeblock_info::cols KeyCol>
    auto get_vec_col()
    {
        using KeyType = decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>()));

        std::vector<KeyType> result;
        for (const auto& iter : record) {
            result.emplace_back(homeblock_info::getField<KeyCol>(iter));
        }
 
        return result;
    }
    
    /* 
    get_vec_col<..,..>([](const auto& value) -> bool {
            return value > 150; 
        })
    */
    template<homeblock_info::cols KeyCol, typename Callback> 
    requires std::invocable<Callback, 
            decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>()))> &&
            std::convertible_to<
                std::invoke_result_t<Callback&, 
                    decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>()))>, bool>
    auto get_vec_col(Callback&& callback)
    {
        using KeyType = decltype(homeblock_info::getField<KeyCol>(std::declval<const homeblock_info::meta&>()));
        std::vector<KeyType> result;
        for (const auto& iter : record) 
        {
            if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>) 
            {
                result.emplace_back(homeblock_info::getField<KeyCol>(iter));
            } else {
                if (std::forward<Callback>(callback)(homeblock_info::getField<KeyCol>(iter))) {
                    result.emplace_back(homeblock_info::getField<KeyCol>(iter));
                }
            }
        }
 
        return result;
    }
    
    template<homeblock_info::cols Col>
        requires requires(std::ostream& os, decltype(homeblock_info::getField<Col>(std::declval<const homeblock_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_strs() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << "\"";
            oss << homeblock_info::getField<Col>(iter); 
            oss << "\",";
        }
        std::string temp=oss.str();
        if(!temp.empty())
        {
            temp.pop_back();
        }
        return temp;
    }
    
    template<homeblock_info::cols Col>
        requires requires(std::ostream& os, decltype(homeblock_info::getField<Col>(std::declval<const homeblock_info::meta&>())) t) {
            { os << t } -> std::same_as<std::ostream&>;
        }
    std::string get_cols_to_str() 
    {
        std::ostringstream oss;

        for (const auto& iter : record) {
            oss << homeblock_info::getField<Col>(iter); 
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
   