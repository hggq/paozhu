#ifndef ORM_CMS_XTASKBASEMATA_H
#define ORM_CMS_XTASKBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Thu, 07 Aug 2025 02:38:21 GMT
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
#include "unicode.h"

namespace orm { 
   
     namespace cms { 

struct xtask_base
{
    struct meta{
     unsigned  int  xtaskid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  xpjid = 0; ///**/
 unsigned  int  adminid = 0; ///**/
 unsigned  int  parentid = 0; ///**/
 unsigned  int  begindate = 0; ///**/
 unsigned  int  enddate = 0; ///**/
 unsigned  int  expectbegindate = 0; ///**/
 unsigned  int  expectenddate = 0; ///**/
 unsigned  int  milestone = 0; ///**/
 unsigned  int  subxpjid = 0; ///**/
 unsigned  int  depxtaskid = 0; ///**/
 unsigned  int  referdocid = 0; ///**/
 unsigned  char  isfinish = 0; ///**/
 unsigned  int  updatedate = 0; ///**/
 unsigned  int  finishdate = 0; ///**/
 unsigned  int  iscore = 0; ///**/
 double  xvalue = 0; ///**/
 float  expectday = 0; ///**/
 float  realday = 0; ///**/
 float  pricevalue = 0; ///**/
 std::string  title = ""; ///**/
 std::string  introduce = ""; ///**/
 std::string  xlogo = ""; ///**/
 std::string  xcolor = ""; ///**/
 std::string  pullurl = ""; ///**/
 std::string  pulltitle = ""; ///**/
 std::string  pullauthor = ""; ///**/
 std::string  note = ""; ///**/
 std::string  itemnote = ""; ///**/
 } data;
  
        struct meta_tree{
         unsigned  int  xtaskid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  xpjid = 0; ///**/
 unsigned  int  adminid = 0; ///**/
 unsigned  int  parentid = 0; ///**/
 unsigned  int  begindate = 0; ///**/
 unsigned  int  enddate = 0; ///**/
 unsigned  int  expectbegindate = 0; ///**/
 unsigned  int  expectenddate = 0; ///**/
 unsigned  int  milestone = 0; ///**/
 unsigned  int  subxpjid = 0; ///**/
 unsigned  int  depxtaskid = 0; ///**/
 unsigned  int  referdocid = 0; ///**/
 unsigned  char  isfinish = 0; ///**/
 unsigned  int  updatedate = 0; ///**/
 unsigned  int  finishdate = 0; ///**/
 unsigned  int  iscore = 0; ///**/
 double  xvalue = 0; ///**/
 float  expectday = 0; ///**/
 float  realday = 0; ///**/
 float  pricevalue = 0; ///**/
 std::string  title = ""; ///**/
 std::string  introduce = ""; ///**/
 std::string  xlogo = ""; ///**/
 std::string  xcolor = ""; ///**/
 std::string  pullurl = ""; ///**/
 std::string  pulltitle = ""; ///**/
 std::string  pullauthor = ""; ///**/
 std::string  note = ""; ///**/
 std::string  itemnote = ""; ///**/

	std::vector<meta_tree> children;
 };
 std::vector<xtask_base::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<xtask_base::meta>::iterator begin(){     return record.begin(); }
std::vector<xtask_base::meta>::iterator end(){     return record.end(); }
std::vector<xtask_base::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<xtask_base::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,30> col_names={"xtaskid","userid","xpjid","adminid","parentid","begindate","enddate","expectbegindate","expectenddate","milestone","subxpjid","depxtaskid","referdocid","isfinish","updatedate","finishdate","iscore","xvalue","expectday","realday","pricevalue","title","introduce","xlogo","xcolor","pullurl","pulltitle","pullauthor","note","itemnote"};
static constexpr std::array<unsigned char,30> col_types={3,3,3,3,3,3,3,3,3,3,3,3,3,1,3,3,3,5,4,4,4,253,252,253,253,253,253,253,252,252};
static constexpr std::array<unsigned char,30> col_length={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,60,0,0};
static constexpr std::array<unsigned char,30> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="xtask";
static constexpr std::string_view modelname="Xtask";

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
   	 return 3;
break;
case 'b':
   	 return 5;
break;
case 'd':
   	 return 11;
break;
case 'e':
 switch(coln.size()){  
case 7:
   	 return 6;
break;
case 9:
   	 return 18;
break;
case 13:
   	 return 8;
break;
case 15:
   	 return 7;
break;
 }
 break;
case 'f':
   	 return 15;
break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 16;
break;
case 8:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 29; }
 if(colpospppc=='h'){ return 13; }
   	 break;
case 9:
   	 return 22;
break;
 }
 break;
case 'm':
   	 return 9;
break;
case 'n':
   	 return 28;
break;
case 'p':
 switch(coln.size()){  
case 7:
   	 return 25;
break;
case 8:
   	 return 4;
break;
case 9:
   	 return 26;
break;
case 10:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 20; }
 if(colpospppc=='r'){ return 27; }
   	 break;
 }
 break;
case 'r':
 switch(coln.size()){  
case 7:
   	 return 19;
break;
case 10:
   	 return 12;
break;
 }
 break;
case 's':
   	 return 10;
break;
case 't':
   	 return 21;
break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 1;
break;
case 10:
   	 return 14;
break;
 }
 break;
case 'x':
 switch(coln.size()){  
case 5:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 2; }
 if(colpospppc=='o'){ return 23; }
   	 break;
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 17; }
 if(colpospppc=='r'){ return 24; }
   	 break;
case 7:
   	 return 0;
break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "xtaskid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     xtask_base::meta metatemp;    
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
        for(;j<col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(data.xtaskid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.xtaskid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.xpjid);
}
if(data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.adminid);
}
if(data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.parentid);
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
if(data.expectbegindate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.expectbegindate);
}
if(data.expectenddate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.expectenddate);
}
if(data.milestone==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.milestone);
}
if(data.subxpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.subxpjid);
}
if(data.depxtaskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.depxtaskid);
}
if(data.referdocid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.referdocid);
}
if(data.isfinish==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isfinish);
}
if(data.updatedate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.updatedate);
}
if(data.finishdate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.finishdate);
}
if(data.iscore==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.iscore);
}
if(data.xvalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.xvalue);
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
if(data.pricevalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.pricevalue);
}
tempsql<<",'"<<stringaddslash(data.title)<<"'";
tempsql<<",'"<<stringaddslash(data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(data.xlogo)<<"'";
tempsql<<",'"<<stringaddslash(data.xcolor)<<"'";
tempsql<<",'"<<stringaddslash(data.pullurl)<<"'";
tempsql<<",'"<<stringaddslash(data.pulltitle)<<"'";
tempsql<<",'"<<stringaddslash(data.pullauthor)<<"'";
tempsql<<",'"<<stringaddslash(data.note)<<"'";
tempsql<<",'"<<stringaddslash(data.itemnote)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql(const meta &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<col_names[j];
        }
        if(j>0){
            tempsql<<"`";
        }
        tempsql<<") VALUES (";

        if(insert_data.xtaskid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.xtaskid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.xpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.xpjid);
}
if(insert_data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.adminid);
}
if(insert_data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.parentid);
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
if(insert_data.expectbegindate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.expectbegindate);
}
if(insert_data.expectenddate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.expectenddate);
}
if(insert_data.milestone==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.milestone);
}
if(insert_data.subxpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.subxpjid);
}
if(insert_data.depxtaskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.depxtaskid);
}
if(insert_data.referdocid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.referdocid);
}
if(insert_data.isfinish==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isfinish);
}
if(insert_data.updatedate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.updatedate);
}
if(insert_data.finishdate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.finishdate);
}
if(insert_data.iscore==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.iscore);
}
if(insert_data.xvalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.xvalue);
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
if(insert_data.pricevalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.pricevalue);
}
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.xlogo)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.xcolor)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.pullurl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.pulltitle)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.pullauthor)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.note)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.itemnote)<<"'";
tempsql<<")";

     
       return tempsql.str();
   } 
       
    std::string _makerecordinsertsql(const std::vector<meta> &insert_data){
        unsigned int j=0;
        std::ostringstream tempsql;
        tempsql<<"INSERT INTO ";
        tempsql<<tablename;
        tempsql<<" (";
        for(;j<col_names.size();j++){
                if(j>0){
                    tempsql<<"`,`";
                }else{
                    tempsql<<"`";
                }
                tempsql<<col_names[j];
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


            	if(insert_data[i].xtaskid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].xtaskid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].xpjid);
	}
	if(insert_data[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].adminid);
	}
	if(insert_data[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].parentid);
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
	if(insert_data[i].expectbegindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].expectbegindate);
	}
	if(insert_data[i].expectenddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].expectenddate);
	}
	if(insert_data[i].milestone==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].milestone);
	}
	if(insert_data[i].subxpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].subxpjid);
	}
	if(insert_data[i].depxtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].depxtaskid);
	}
	if(insert_data[i].referdocid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].referdocid);
	}
	if(insert_data[i].isfinish==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isfinish);
	}
	if(insert_data[i].updatedate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].updatedate);
	}
	if(insert_data[i].finishdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].finishdate);
	}
	if(insert_data[i].iscore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].iscore);
	}
	if(insert_data[i].xvalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].xvalue);
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
	if(insert_data[i].pricevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].pricevalue);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].introduce)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].xlogo)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].xcolor)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].pullurl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].pulltitle)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].pullauthor)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].note)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].itemnote)<<"'";
		tempsql<<")";
	 } 

     
       return tempsql.str();
   } 
       
    std::string _makeupdatesql(const std::string &fileld){
        std::ostringstream tempsql;
        tempsql<<"UPDATE ";
        tempsql<<tablename;
        tempsql<<" SET ";

        bool isall=false;
        if(fileld.empty()){
            isall=true;
        }
        if(isall){

        if(data.xtaskid==0){
	tempsql<<"`xtaskid`=0";
 }else{ 
	tempsql<<"`xtaskid`="<<std::to_string(data.xtaskid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",`xpjid`=0";
 }else{ 
	tempsql<<",`xpjid`="<<std::to_string(data.xpjid);
}
if(data.adminid==0){
	tempsql<<",`adminid`=0";
 }else{ 
	tempsql<<",`adminid`="<<std::to_string(data.adminid);
}
if(data.parentid==0){
	tempsql<<",`parentid`=0";
 }else{ 
	tempsql<<",`parentid`="<<std::to_string(data.parentid);
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
if(data.expectbegindate==0){
	tempsql<<",`expectbegindate`=0";
 }else{ 
	tempsql<<",`expectbegindate`="<<std::to_string(data.expectbegindate);
}
if(data.expectenddate==0){
	tempsql<<",`expectenddate`=0";
 }else{ 
	tempsql<<",`expectenddate`="<<std::to_string(data.expectenddate);
}
if(data.milestone==0){
	tempsql<<",`milestone`=0";
 }else{ 
	tempsql<<",`milestone`="<<std::to_string(data.milestone);
}
if(data.subxpjid==0){
	tempsql<<",`subxpjid`=0";
 }else{ 
	tempsql<<",`subxpjid`="<<std::to_string(data.subxpjid);
}
if(data.depxtaskid==0){
	tempsql<<",`depxtaskid`=0";
 }else{ 
	tempsql<<",`depxtaskid`="<<std::to_string(data.depxtaskid);
}
if(data.referdocid==0){
	tempsql<<",`referdocid`=0";
 }else{ 
	tempsql<<",`referdocid`="<<std::to_string(data.referdocid);
}
if(data.isfinish==0){
	tempsql<<",`isfinish`=0";
 }else{ 
	tempsql<<",`isfinish`="<<std::to_string(data.isfinish);
}
if(data.updatedate==0){
	tempsql<<",`updatedate`=0";
 }else{ 
	tempsql<<",`updatedate`="<<std::to_string(data.updatedate);
}
if(data.finishdate==0){
	tempsql<<",`finishdate`=0";
 }else{ 
	tempsql<<",`finishdate`="<<std::to_string(data.finishdate);
}
if(data.iscore==0){
	tempsql<<",`iscore`=0";
 }else{ 
	tempsql<<",`iscore`="<<std::to_string(data.iscore);
}
if(data.xvalue==0){
	tempsql<<",`xvalue`=0";
 }else{ 
	tempsql<<",`xvalue`="<<std::to_string(data.xvalue);
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
if(data.pricevalue==0){
	tempsql<<",`pricevalue`=0";
 }else{ 
	tempsql<<",`pricevalue`="<<std::to_string(data.pricevalue);
}
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
tempsql<<",`introduce`='"<<stringaddslash(data.introduce)<<"'";
tempsql<<",`xlogo`='"<<stringaddslash(data.xlogo)<<"'";
tempsql<<",`xcolor`='"<<stringaddslash(data.xcolor)<<"'";
tempsql<<",`pullurl`='"<<stringaddslash(data.pullurl)<<"'";
tempsql<<",`pulltitle`='"<<stringaddslash(data.pulltitle)<<"'";
tempsql<<",`pullauthor`='"<<stringaddslash(data.pullauthor)<<"'";
tempsql<<",`note`='"<<stringaddslash(data.note)<<"'";
tempsql<<",`itemnote`='"<<stringaddslash(data.itemnote)<<"'";
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
if(data.xtaskid==0){
	tempsql<<"`xtaskid`=0";
 }else{ 
	tempsql<<"`xtaskid`="<<std::to_string(data.xtaskid);
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
if(data.xpjid==0){
	tempsql<<"`xpjid`=0";
 }else{ 
	tempsql<<"`xpjid`="<<std::to_string(data.xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.parentid==0){
	tempsql<<"`parentid`=0";
 }else{ 
	tempsql<<"`parentid`="<<std::to_string(data.parentid);
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
if(data.expectbegindate==0){
	tempsql<<"`expectbegindate`=0";
 }else{ 
	tempsql<<"`expectbegindate`="<<std::to_string(data.expectbegindate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.expectenddate==0){
	tempsql<<"`expectenddate`=0";
 }else{ 
	tempsql<<"`expectenddate`="<<std::to_string(data.expectenddate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.milestone==0){
	tempsql<<"`milestone`=0";
 }else{ 
	tempsql<<"`milestone`="<<std::to_string(data.milestone);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.subxpjid==0){
	tempsql<<"`subxpjid`=0";
 }else{ 
	tempsql<<"`subxpjid`="<<std::to_string(data.subxpjid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.depxtaskid==0){
	tempsql<<"`depxtaskid`=0";
 }else{ 
	tempsql<<"`depxtaskid`="<<std::to_string(data.depxtaskid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.referdocid==0){
	tempsql<<"`referdocid`=0";
 }else{ 
	tempsql<<"`referdocid`="<<std::to_string(data.referdocid);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.isfinish==0){
	tempsql<<"`isfinish`=0";
 }else{ 
	tempsql<<"`isfinish`="<<std::to_string(data.isfinish);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.updatedate==0){
	tempsql<<"`updatedate`=0";
 }else{ 
	tempsql<<"`updatedate`="<<std::to_string(data.updatedate);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(data.finishdate==0){
	tempsql<<"`finishdate`=0";
 }else{ 
	tempsql<<"`finishdate`="<<std::to_string(data.finishdate);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(data.iscore==0){
	tempsql<<"`iscore`=0";
 }else{ 
	tempsql<<"`iscore`="<<std::to_string(data.iscore);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(data.xvalue==0){
	tempsql<<"`xvalue`=0";
 }else{ 
	tempsql<<"`xvalue`="<<std::to_string(data.xvalue);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(data.expectday==0){
	tempsql<<"`expectday`=0";
 }else{ 
	tempsql<<"`expectday`="<<std::to_string(data.expectday);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(data.realday==0){
	tempsql<<"`realday`=0";
 }else{ 
	tempsql<<"`realday`="<<std::to_string(data.realday);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.pricevalue==0){
	tempsql<<"`pricevalue`=0";
 }else{ 
	tempsql<<"`pricevalue`="<<std::to_string(data.pricevalue);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`introduce`='"<<stringaddslash(data.introduce)<<"'";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`xlogo`='"<<stringaddslash(data.xlogo)<<"'";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`xcolor`='"<<stringaddslash(data.xcolor)<<"'";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`pullurl`='"<<stringaddslash(data.pullurl)<<"'";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`pulltitle`='"<<stringaddslash(data.pulltitle)<<"'";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`pullauthor`='"<<stringaddslash(data.pullauthor)<<"'";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`note`='"<<stringaddslash(data.note)<<"'";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`itemnote`='"<<stringaddslash(data.itemnote)<<"'";
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
        for (; j < col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << col_names[j];
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
            	if(record[i].xtaskid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].xtaskid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xpjid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
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
	if(record[i].expectbegindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expectbegindate);
	}
	if(record[i].expectenddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expectenddate);
	}
	if(record[i].milestone==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].milestone);
	}
	if(record[i].subxpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].subxpjid);
	}
	if(record[i].depxtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].depxtaskid);
	}
	if(record[i].referdocid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].referdocid);
	}
	if(record[i].isfinish==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isfinish);
	}
	if(record[i].updatedate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updatedate);
	}
	if(record[i].finishdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].finishdate);
	}
	if(record[i].iscore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].iscore);
	}
	if(record[i].xvalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xvalue);
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
	if(record[i].pricevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].pricevalue);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].xlogo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].xcolor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].pullurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].pulltitle)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].pullauthor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].note)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].itemnote)<<"'";
	tempsql<<")";

 }
 return tempsql.str();
}

    std::string _make_insert_into_sql(const std::string &fileld)
    {
        unsigned int j = 0;
        std::ostringstream tempsql;
        tempsql << "INSERT INTO ";
        tempsql << tablename;
        tempsql << " (";
        for (; j < col_names.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << col_names[j];
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
            	if(record[i].xtaskid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].xtaskid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xpjid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
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
	if(record[i].expectbegindate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expectbegindate);
	}
	if(record[i].expectenddate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expectenddate);
	}
	if(record[i].milestone==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].milestone);
	}
	if(record[i].subxpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].subxpjid);
	}
	if(record[i].depxtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].depxtaskid);
	}
	if(record[i].referdocid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].referdocid);
	}
	if(record[i].isfinish==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isfinish);
	}
	if(record[i].updatedate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updatedate);
	}
	if(record[i].finishdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].finishdate);
	}
	if(record[i].iscore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].iscore);
	}
	if(record[i].xvalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xvalue);
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
	if(record[i].pricevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].pricevalue);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].xlogo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].xcolor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].pullurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].pulltitle)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].pullauthor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].note)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].itemnote)<<"'";
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

   std::vector<std::string> data_toarray(std::string fileld=""){
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
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
            for(jj=0;jj<keypos.size();jj++){
                switch(keypos[jj]){
         case 0:
if(data.xtaskid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xtaskid));
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
if(data.xpjid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xpjid));
}
 break;
 case 3:
if(data.adminid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.adminid));
}
 break;
 case 4:
if(data.parentid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.parentid));
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
if(data.expectbegindate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expectbegindate));
}
 break;
 case 8:
if(data.expectenddate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expectenddate));
}
 break;
 case 9:
if(data.milestone==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.milestone));
}
 break;
 case 10:
if(data.subxpjid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.subxpjid));
}
 break;
 case 11:
if(data.depxtaskid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.depxtaskid));
}
 break;
 case 12:
if(data.referdocid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.referdocid));
}
 break;
 case 13:
if(data.isfinish==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isfinish));
}
 break;
 case 14:
if(data.updatedate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.updatedate));
}
 break;
 case 15:
if(data.finishdate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.finishdate));
}
 break;
 case 16:
if(data.iscore==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.iscore));
}
 break;
 case 17:
if(data.xvalue==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xvalue));
}
 break;
 case 18:
if(data.expectday==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expectday));
}
 break;
 case 19:
if(data.realday==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.realday));
}
 break;
 case 20:
if(data.pricevalue==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.pricevalue));
}
 break;
 case 21:
	temparray.push_back(data.title);
 break;
 case 22:
	temparray.push_back(data.introduce);
 break;
 case 23:
	temparray.push_back(data.xlogo);
 break;
 case 24:
	temparray.push_back(data.xcolor);
 break;
 case 25:
	temparray.push_back(data.pullurl);
 break;
 case 26:
	temparray.push_back(data.pulltitle);
 break;
 case 27:
	temparray.push_back(data.pullauthor);
 break;
 case 28:
	temparray.push_back(data.note);
 break;
 case 29:
	temparray.push_back(data.itemnote);
 break;

                             default:
                                ;
                     }
                 }   
   
     return temparray;             
   }   
   
   std::map<std::string,std::string> data_tomap(std::string fileld=""){
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
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
    
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
if(data.xtaskid==0){
	tempsql.insert({"xtaskid","0"});
 }else{ 
	tempsql.insert({"xtaskid",std::to_string(data.xtaskid)});
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
if(data.xpjid==0){
	tempsql.insert({"xpjid","0"});
 }else{ 
	tempsql.insert({"xpjid",std::to_string(data.xpjid)});
}
 break;
 case 3:
if(data.adminid==0){
	tempsql.insert({"adminid","0"});
 }else{ 
	tempsql.insert({"adminid",std::to_string(data.adminid)});
}
 break;
 case 4:
if(data.parentid==0){
	tempsql.insert({"parentid","0"});
 }else{ 
	tempsql.insert({"parentid",std::to_string(data.parentid)});
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
if(data.expectbegindate==0){
	tempsql.insert({"expectbegindate","0"});
 }else{ 
	tempsql.insert({"expectbegindate",std::to_string(data.expectbegindate)});
}
 break;
 case 8:
if(data.expectenddate==0){
	tempsql.insert({"expectenddate","0"});
 }else{ 
	tempsql.insert({"expectenddate",std::to_string(data.expectenddate)});
}
 break;
 case 9:
if(data.milestone==0){
	tempsql.insert({"milestone","0"});
 }else{ 
	tempsql.insert({"milestone",std::to_string(data.milestone)});
}
 break;
 case 10:
if(data.subxpjid==0){
	tempsql.insert({"subxpjid","0"});
 }else{ 
	tempsql.insert({"subxpjid",std::to_string(data.subxpjid)});
}
 break;
 case 11:
if(data.depxtaskid==0){
	tempsql.insert({"depxtaskid","0"});
 }else{ 
	tempsql.insert({"depxtaskid",std::to_string(data.depxtaskid)});
}
 break;
 case 12:
if(data.referdocid==0){
	tempsql.insert({"referdocid","0"});
 }else{ 
	tempsql.insert({"referdocid",std::to_string(data.referdocid)});
}
 break;
 case 13:
if(data.isfinish==0){
	tempsql.insert({"isfinish","0"});
 }else{ 
	tempsql.insert({"isfinish",std::to_string(data.isfinish)});
}
 break;
 case 14:
if(data.updatedate==0){
	tempsql.insert({"updatedate","0"});
 }else{ 
	tempsql.insert({"updatedate",std::to_string(data.updatedate)});
}
 break;
 case 15:
if(data.finishdate==0){
	tempsql.insert({"finishdate","0"});
 }else{ 
	tempsql.insert({"finishdate",std::to_string(data.finishdate)});
}
 break;
 case 16:
if(data.iscore==0){
	tempsql.insert({"iscore","0"});
 }else{ 
	tempsql.insert({"iscore",std::to_string(data.iscore)});
}
 break;
 case 17:
if(data.xvalue==0){
	tempsql.insert({"xvalue","0"});
 }else{ 
	tempsql.insert({"xvalue",std::to_string(data.xvalue)});
}
 break;
 case 18:
if(data.expectday==0){
	tempsql.insert({"expectday","0"});
 }else{ 
	tempsql.insert({"expectday",std::to_string(data.expectday)});
}
 break;
 case 19:
if(data.realday==0){
	tempsql.insert({"realday","0"});
 }else{ 
	tempsql.insert({"realday",std::to_string(data.realday)});
}
 break;
 case 20:
if(data.pricevalue==0){
	tempsql.insert({"pricevalue","0"});
 }else{ 
	tempsql.insert({"pricevalue",std::to_string(data.pricevalue)});
}
 break;
 case 21:
	tempsql.insert({"title",data.title});
 break;
 case 22:
	tempsql.insert({"introduce",data.introduce});
 break;
 case 23:
	tempsql.insert({"xlogo",data.xlogo});
 break;
 case 24:
	tempsql.insert({"xcolor",data.xcolor});
 break;
 case 25:
	tempsql.insert({"pullurl",data.pullurl});
 break;
 case 26:
	tempsql.insert({"pulltitle",data.pulltitle});
 break;
 case 27:
	tempsql.insert({"pullauthor",data.pullauthor});
 break;
 case 28:
	tempsql.insert({"note",data.note});
 break;
 case 29:
	tempsql.insert({"itemnote",data.itemnote});
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
if(data.xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(data.xtaskid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",\"xpjid\":0";
 }else{ 
	tempsql<<",\"xpjid\":"<<std::to_string(data.xpjid);
}
if(data.adminid==0){
	tempsql<<",\"adminid\":0";
 }else{ 
	tempsql<<",\"adminid\":"<<std::to_string(data.adminid);
}
if(data.parentid==0){
	tempsql<<",\"parentid\":0";
 }else{ 
	tempsql<<",\"parentid\":"<<std::to_string(data.parentid);
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
if(data.expectbegindate==0){
	tempsql<<",\"expectbegindate\":0";
 }else{ 
	tempsql<<",\"expectbegindate\":"<<std::to_string(data.expectbegindate);
}
if(data.expectenddate==0){
	tempsql<<",\"expectenddate\":0";
 }else{ 
	tempsql<<",\"expectenddate\":"<<std::to_string(data.expectenddate);
}
if(data.milestone==0){
	tempsql<<",\"milestone\":0";
 }else{ 
	tempsql<<",\"milestone\":"<<std::to_string(data.milestone);
}
if(data.subxpjid==0){
	tempsql<<",\"subxpjid\":0";
 }else{ 
	tempsql<<",\"subxpjid\":"<<std::to_string(data.subxpjid);
}
if(data.depxtaskid==0){
	tempsql<<",\"depxtaskid\":0";
 }else{ 
	tempsql<<",\"depxtaskid\":"<<std::to_string(data.depxtaskid);
}
if(data.referdocid==0){
	tempsql<<",\"referdocid\":0";
 }else{ 
	tempsql<<",\"referdocid\":"<<std::to_string(data.referdocid);
}
if(data.isfinish==0){
	tempsql<<",\"isfinish\":0";
 }else{ 
	tempsql<<",\"isfinish\":"<<std::to_string(data.isfinish);
}
if(data.updatedate==0){
	tempsql<<",\"updatedate\":0";
 }else{ 
	tempsql<<",\"updatedate\":"<<std::to_string(data.updatedate);
}
if(data.finishdate==0){
	tempsql<<",\"finishdate\":0";
 }else{ 
	tempsql<<",\"finishdate\":"<<std::to_string(data.finishdate);
}
if(data.iscore==0){
	tempsql<<",\"iscore\":0";
 }else{ 
	tempsql<<",\"iscore\":"<<std::to_string(data.iscore);
}
if(data.xvalue==0){
	tempsql<<",\"xvalue\":0";
 }else{ 
	tempsql<<",\"xvalue\":"<<std::to_string(data.xvalue);
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
if(data.pricevalue==0){
	tempsql<<",\"pricevalue\":0";
 }else{ 
	tempsql<<",\"pricevalue\":"<<std::to_string(data.pricevalue);
}
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce);
tempsql<<"\"";
tempsql<<",\"xlogo\":\""<<http::utf8_to_jsonstring(data.xlogo);
tempsql<<"\"";
tempsql<<",\"xcolor\":\""<<http::utf8_to_jsonstring(data.xcolor);
tempsql<<"\"";
tempsql<<",\"pullurl\":\""<<http::utf8_to_jsonstring(data.pullurl);
tempsql<<"\"";
tempsql<<",\"pulltitle\":\""<<http::utf8_to_jsonstring(data.pulltitle);
tempsql<<"\"";
tempsql<<",\"pullauthor\":\""<<http::utf8_to_jsonstring(data.pullauthor);
tempsql<<"\"";
tempsql<<",\"note\":\""<<http::utf8_to_jsonstring(data.note);
tempsql<<"\"";
tempsql<<",\"itemnote\":\""<<http::utf8_to_jsonstring(data.itemnote);
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
            for(jj=0;jj<col_names.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
        tempsql<<"{";
        for(jj=0;jj<keypos.size();jj++){
            switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(data.xtaskid);
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
if(data.xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(data.xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(data.parentid);
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
if(data.expectbegindate==0){
	tempsql<<"\"expectbegindate\":0";
 }else{ 
	tempsql<<"\"expectbegindate\":"<<std::to_string(data.expectbegindate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.expectenddate==0){
	tempsql<<"\"expectenddate\":0";
 }else{ 
	tempsql<<"\"expectenddate\":"<<std::to_string(data.expectenddate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.milestone==0){
	tempsql<<"\"milestone\":0";
 }else{ 
	tempsql<<"\"milestone\":"<<std::to_string(data.milestone);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.subxpjid==0){
	tempsql<<"\"subxpjid\":0";
 }else{ 
	tempsql<<"\"subxpjid\":"<<std::to_string(data.subxpjid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.depxtaskid==0){
	tempsql<<"\"depxtaskid\":0";
 }else{ 
	tempsql<<"\"depxtaskid\":"<<std::to_string(data.depxtaskid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.referdocid==0){
	tempsql<<"\"referdocid\":0";
 }else{ 
	tempsql<<"\"referdocid\":"<<std::to_string(data.referdocid);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.isfinish==0){
	tempsql<<"\"isfinish\":0";
 }else{ 
	tempsql<<"\"isfinish\":"<<std::to_string(data.isfinish);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.updatedate==0){
	tempsql<<"\"updatedate\":0";
 }else{ 
	tempsql<<"\"updatedate\":"<<std::to_string(data.updatedate);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(data.finishdate==0){
	tempsql<<"\"finishdate\":0";
 }else{ 
	tempsql<<"\"finishdate\":"<<std::to_string(data.finishdate);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(data.iscore==0){
	tempsql<<"\"iscore\":0";
 }else{ 
	tempsql<<"\"iscore\":"<<std::to_string(data.iscore);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(data.xvalue==0){
	tempsql<<"\"xvalue\":0";
 }else{ 
	tempsql<<"\"xvalue\":"<<std::to_string(data.xvalue);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(data.expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(data.expectday);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(data.realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(data.realday);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.pricevalue==0){
	tempsql<<"\"pricevalue\":0";
 }else{ 
	tempsql<<"\"pricevalue\":"<<std::to_string(data.pricevalue);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(data.xlogo)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(data.xcolor)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullurl\":\""<<http::utf8_to_jsonstring(data.pullurl)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pulltitle\":\""<<http::utf8_to_jsonstring(data.pulltitle)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullauthor\":\""<<http::utf8_to_jsonstring(data.pullauthor)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"note\":\""<<http::utf8_to_jsonstring(data.note)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"itemnote\":\""<<http::utf8_to_jsonstring(data.itemnote)<<"\"";
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
        xtask_base::meta metatemp; 
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
			data.xtaskid=std::stoul(set_value_name);
		}catch (...) { 
			data.xtaskid=0;
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
			data.xpjid=std::stoul(set_value_name);
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.adminid=std::stoul(set_value_name);
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 4:
		 try{
			data.parentid=std::stoul(set_value_name);
		}catch (...) { 
			data.parentid=0;
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
			data.expectbegindate=std::stoul(set_value_name);
		}catch (...) { 
			data.expectbegindate=0;
			 }
			break;
		case 8:
		 try{
			data.expectenddate=std::stoul(set_value_name);
		}catch (...) { 
			data.expectenddate=0;
			 }
			break;
		case 9:
		 try{
			data.milestone=std::stoul(set_value_name);
		}catch (...) { 
			data.milestone=0;
			 }
			break;
		case 10:
		 try{
			data.subxpjid=std::stoul(set_value_name);
		}catch (...) { 
			data.subxpjid=0;
			 }
			break;
		case 11:
		 try{
			data.depxtaskid=std::stoul(set_value_name);
		}catch (...) { 
			data.depxtaskid=0;
			 }
			break;
		case 12:
		 try{
			data.referdocid=std::stoul(set_value_name);
		}catch (...) { 
			data.referdocid=0;
			 }
			break;
		case 13:
		 try{
			data.isfinish=std::stoi(set_value_name);
		}catch (...) { 
			data.isfinish=0;
			 }
			break;
		case 14:
		 try{
			data.updatedate=std::stoul(set_value_name);
		}catch (...) { 
			data.updatedate=0;
			 }
			break;
		case 15:
		 try{
			data.finishdate=std::stoul(set_value_name);
		}catch (...) { 
			data.finishdate=0;
			 }
			break;
		case 16:
		 try{
			data.iscore=std::stoul(set_value_name);
		}catch (...) { 
			data.iscore=0;
			 }
			break;
		case 17:
		 try{
			data.xvalue=std::stod(set_value_name);
		}catch (...) { 
			data.xvalue=0.0;
			 }
			break;
		case 18:
		 try{
			data.expectday=std::stof(set_value_name);
		}catch (...) { 
			data.expectday=0.0;
			 }
			break;
		case 19:
		 try{
			data.realday=std::stof(set_value_name);
		}catch (...) { 
			data.realday=0.0;
			 }
			break;
		case 20:
		 try{
			data.pricevalue=std::stof(set_value_name);
		}catch (...) { 
			data.pricevalue=0.0;
			 }
			break;
		case 21:
		 try{
			data.title.append(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 22:
		 try{
			data.introduce.append(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 23:
		 try{
			data.xlogo.append(set_value_name);
		}catch (...) { 
			data.xlogo.clear();
			 }
			break;
		case 24:
		 try{
			data.xcolor.append(set_value_name);
		}catch (...) { 
			data.xcolor.clear();
			 }
			break;
		case 25:
		 try{
			data.pullurl.append(set_value_name);
		}catch (...) { 
			data.pullurl.clear();
			 }
			break;
		case 26:
		 try{
			data.pulltitle.append(set_value_name);
		}catch (...) { 
			data.pulltitle.clear();
			 }
			break;
		case 27:
		 try{
			data.pullauthor.append(set_value_name);
		}catch (...) { 
			data.pullauthor.clear();
			 }
			break;
		case 28:
		 try{
			data.note.append(set_value_name);
		}catch (...) { 
			data.note.clear();
			 }
			break;
		case 29:
		 try{
			data.itemnote.append(set_value_name);
		}catch (...) { 
			data.itemnote.clear();
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
			data.xtaskid=set_value_name;
		}catch (...) { 
			data.xtaskid=0;
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
			data.xpjid=set_value_name;
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.adminid=set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 4:
		 try{
			data.parentid=set_value_name;
		}catch (...) { 
			data.parentid=0;
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
			data.expectbegindate=set_value_name;
		}catch (...) { 
			data.expectbegindate=0;
			 }
			break;
		case 8:
		 try{
			data.expectenddate=set_value_name;
		}catch (...) { 
			data.expectenddate=0;
			 }
			break;
		case 9:
		 try{
			data.milestone=set_value_name;
		}catch (...) { 
			data.milestone=0;
			 }
			break;
		case 10:
		 try{
			data.subxpjid=set_value_name;
		}catch (...) { 
			data.subxpjid=0;
			 }
			break;
		case 11:
		 try{
			data.depxtaskid=set_value_name;
		}catch (...) { 
			data.depxtaskid=0;
			 }
			break;
		case 12:
		 try{
			data.referdocid=set_value_name;
		}catch (...) { 
			data.referdocid=0;
			 }
			break;
		case 13:
		 try{
			data.isfinish=set_value_name;
		}catch (...) { 
			data.isfinish=0;
			 }
			break;
		case 14:
		 try{
			data.updatedate=set_value_name;
		}catch (...) { 
			data.updatedate=0;
			 }
			break;
		case 15:
		 try{
			data.finishdate=set_value_name;
		}catch (...) { 
			data.finishdate=0;
			 }
			break;
		case 16:
		 try{
			data.iscore=set_value_name;
		}catch (...) { 
			data.iscore=0;
			 }
			break;
		case 17:
		 try{
			data.xvalue=(double)set_value_name;
		}catch (...) { 
			data.xvalue=0.0;
			 }
			break;
		case 18:
		 try{
			data.expectday=set_value_name;
		}catch (...) { 
			data.expectday=0.0;
			 }
			break;
		case 19:
		 try{
			data.realday=set_value_name;
		}catch (...) { 
			data.realday=0.0;
			 }
			break;
		case 20:
		 try{
			data.pricevalue=set_value_name;
		}catch (...) { 
			data.pricevalue=0.0;
			 }
			break;
		case 21:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 22:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 23:
		 try{
			data.xlogo=std::to_string(set_value_name);
		}catch (...) { 
			data.xlogo.clear();
			 }
			break;
		case 24:
		 try{
			data.xcolor=std::to_string(set_value_name);
		}catch (...) { 
			data.xcolor.clear();
			 }
			break;
		case 25:
		 try{
			data.pullurl=std::to_string(set_value_name);
		}catch (...) { 
			data.pullurl.clear();
			 }
			break;
		case 26:
		 try{
			data.pulltitle=std::to_string(set_value_name);
		}catch (...) { 
			data.pulltitle.clear();
			 }
			break;
		case 27:
		 try{
			data.pullauthor=std::to_string(set_value_name);
		}catch (...) { 
			data.pullauthor.clear();
			 }
			break;
		case 28:
		 try{
			data.note=std::to_string(set_value_name);
		}catch (...) { 
			data.note.clear();
			 }
			break;
		case 29:
		 try{
			data.itemnote=std::to_string(set_value_name);
		}catch (...) { 
			data.itemnote.clear();
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
			data.xtaskid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xtaskid=0;
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
			data.xpjid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.adminid=(unsigned int)set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 4:
		 try{
			data.parentid=(unsigned int)set_value_name;
		}catch (...) { 
			data.parentid=0;
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
			data.expectbegindate=(unsigned int)set_value_name;
		}catch (...) { 
			data.expectbegindate=0;
			 }
			break;
		case 8:
		 try{
			data.expectenddate=(unsigned int)set_value_name;
		}catch (...) { 
			data.expectenddate=0;
			 }
			break;
		case 9:
		 try{
			data.milestone=(unsigned int)set_value_name;
		}catch (...) { 
			data.milestone=0;
			 }
			break;
		case 10:
		 try{
			data.subxpjid=(unsigned int)set_value_name;
		}catch (...) { 
			data.subxpjid=0;
			 }
			break;
		case 11:
		 try{
			data.depxtaskid=(unsigned int)set_value_name;
		}catch (...) { 
			data.depxtaskid=0;
			 }
			break;
		case 12:
		 try{
			data.referdocid=(unsigned int)set_value_name;
		}catch (...) { 
			data.referdocid=0;
			 }
			break;
		case 13:
		 try{
			data.isfinish=(int)set_value_name;
		}catch (...) { 
			data.isfinish=0;
			 }
			break;
		case 14:
		 try{
			data.updatedate=(unsigned int)set_value_name;
		}catch (...) { 
			data.updatedate=0;
			 }
			break;
		case 15:
		 try{
			data.finishdate=(unsigned int)set_value_name;
		}catch (...) { 
			data.finishdate=0;
			 }
			break;
		case 16:
		 try{
			data.iscore=(unsigned int)set_value_name;
		}catch (...) { 
			data.iscore=0;
			 }
			break;
		case 17:
		 try{
			data.xvalue=set_value_name;
		}catch (...) { 
			data.xvalue=0.0;
			 }
			break;
		case 18:
		 try{
			data.expectday=(float)set_value_name;
		}catch (...) { 
			data.expectday=0.0;
			 }
			break;
		case 19:
		 try{
			data.realday=(float)set_value_name;
		}catch (...) { 
			data.realday=0.0;
			 }
			break;
		case 20:
		 try{
			data.pricevalue=(float)set_value_name;
		}catch (...) { 
			data.pricevalue=0.0;
			 }
			break;
		case 21:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 22:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 23:
		 try{
			data.xlogo=std::to_string(set_value_name);
		}catch (...) { 
			data.xlogo.clear();
			 }
			break;
		case 24:
		 try{
			data.xcolor=std::to_string(set_value_name);
		}catch (...) { 
			data.xcolor.clear();
			 }
			break;
		case 25:
		 try{
			data.pullurl=std::to_string(set_value_name);
		}catch (...) { 
			data.pullurl.clear();
			 }
			break;
		case 26:
		 try{
			data.pulltitle=std::to_string(set_value_name);
		}catch (...) { 
			data.pulltitle.clear();
			 }
			break;
		case 27:
		 try{
			data.pullauthor=std::to_string(set_value_name);
		}catch (...) { 
			data.pullauthor.clear();
			 }
			break;
		case 28:
		 try{
			data.note=std::to_string(set_value_name);
		}catch (...) { 
			data.note.clear();
			 }
			break;
		case 29:
		 try{
			data.itemnote=std::to_string(set_value_name);
		}catch (...) { 
			data.itemnote.clear();
			 }
			break;
	default:
		 { }
			


        }
   } 
    
   std::string to_json(std::string fileld=""){
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
        for(jj=0;jj<col_names.size();jj++){
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
if(record[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(record[n].xtaskid);
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
if(record[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(record[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
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
if(record[n].expectbegindate==0){
	tempsql<<"\"expectbegindate\":0";
 }else{ 
	tempsql<<"\"expectbegindate\":"<<std::to_string(record[n].expectbegindate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].expectenddate==0){
	tempsql<<"\"expectenddate\":0";
 }else{ 
	tempsql<<"\"expectenddate\":"<<std::to_string(record[n].expectenddate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].milestone==0){
	tempsql<<"\"milestone\":0";
 }else{ 
	tempsql<<"\"milestone\":"<<std::to_string(record[n].milestone);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].subxpjid==0){
	tempsql<<"\"subxpjid\":0";
 }else{ 
	tempsql<<"\"subxpjid\":"<<std::to_string(record[n].subxpjid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].depxtaskid==0){
	tempsql<<"\"depxtaskid\":0";
 }else{ 
	tempsql<<"\"depxtaskid\":"<<std::to_string(record[n].depxtaskid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].referdocid==0){
	tempsql<<"\"referdocid\":0";
 }else{ 
	tempsql<<"\"referdocid\":"<<std::to_string(record[n].referdocid);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].isfinish==0){
	tempsql<<"\"isfinish\":0";
 }else{ 
	tempsql<<"\"isfinish\":"<<std::to_string(record[n].isfinish);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].updatedate==0){
	tempsql<<"\"updatedate\":0";
 }else{ 
	tempsql<<"\"updatedate\":"<<std::to_string(record[n].updatedate);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(record[n].finishdate==0){
	tempsql<<"\"finishdate\":0";
 }else{ 
	tempsql<<"\"finishdate\":"<<std::to_string(record[n].finishdate);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(record[n].iscore==0){
	tempsql<<"\"iscore\":0";
 }else{ 
	tempsql<<"\"iscore\":"<<std::to_string(record[n].iscore);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(record[n].xvalue==0){
	tempsql<<"\"xvalue\":0";
 }else{ 
	tempsql<<"\"xvalue\":"<<std::to_string(record[n].xvalue);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(record[n].expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(record[n].expectday);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(record[n].realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(record[n].realday);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].pricevalue==0){
	tempsql<<"\"pricevalue\":0";
 }else{ 
	tempsql<<"\"pricevalue\":"<<std::to_string(record[n].pricevalue);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(record[n].xlogo)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(record[n].xcolor)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullurl\":\""<<http::utf8_to_jsonstring(record[n].pullurl)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pulltitle\":\""<<http::utf8_to_jsonstring(record[n].pulltitle)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullauthor\":\""<<http::utf8_to_jsonstring(record[n].pullauthor)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"note\":\""<<http::utf8_to_jsonstring(record[n].note)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"itemnote\":\""<<http::utf8_to_jsonstring(record[n].itemnote)<<"\"";
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
   
   std::string to_json(std::function<bool(std::string&,meta&)> func,std::string fileld=""){
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
            for(jj=0;jj<col_names.size();jj++){
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
if(record[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(record[n].xtaskid);
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
if(record[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(record[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
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
if(record[n].expectbegindate==0){
	tempsql<<"\"expectbegindate\":0";
 }else{ 
	tempsql<<"\"expectbegindate\":"<<std::to_string(record[n].expectbegindate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].expectenddate==0){
	tempsql<<"\"expectenddate\":0";
 }else{ 
	tempsql<<"\"expectenddate\":"<<std::to_string(record[n].expectenddate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].milestone==0){
	tempsql<<"\"milestone\":0";
 }else{ 
	tempsql<<"\"milestone\":"<<std::to_string(record[n].milestone);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].subxpjid==0){
	tempsql<<"\"subxpjid\":0";
 }else{ 
	tempsql<<"\"subxpjid\":"<<std::to_string(record[n].subxpjid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].depxtaskid==0){
	tempsql<<"\"depxtaskid\":0";
 }else{ 
	tempsql<<"\"depxtaskid\":"<<std::to_string(record[n].depxtaskid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].referdocid==0){
	tempsql<<"\"referdocid\":0";
 }else{ 
	tempsql<<"\"referdocid\":"<<std::to_string(record[n].referdocid);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].isfinish==0){
	tempsql<<"\"isfinish\":0";
 }else{ 
	tempsql<<"\"isfinish\":"<<std::to_string(record[n].isfinish);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].updatedate==0){
	tempsql<<"\"updatedate\":0";
 }else{ 
	tempsql<<"\"updatedate\":"<<std::to_string(record[n].updatedate);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(record[n].finishdate==0){
	tempsql<<"\"finishdate\":0";
 }else{ 
	tempsql<<"\"finishdate\":"<<std::to_string(record[n].finishdate);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(record[n].iscore==0){
	tempsql<<"\"iscore\":0";
 }else{ 
	tempsql<<"\"iscore\":"<<std::to_string(record[n].iscore);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(record[n].xvalue==0){
	tempsql<<"\"xvalue\":0";
 }else{ 
	tempsql<<"\"xvalue\":"<<std::to_string(record[n].xvalue);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(record[n].expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(record[n].expectday);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(record[n].realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(record[n].realday);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].pricevalue==0){
	tempsql<<"\"pricevalue\":0";
 }else{ 
	tempsql<<"\"pricevalue\":"<<std::to_string(record[n].pricevalue);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(record[n].xlogo)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(record[n].xcolor)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullurl\":\""<<http::utf8_to_jsonstring(record[n].pullurl)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pulltitle\":\""<<http::utf8_to_jsonstring(record[n].pulltitle)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullauthor\":\""<<http::utf8_to_jsonstring(record[n].pullauthor)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"note\":\""<<http::utf8_to_jsonstring(record[n].note)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"itemnote\":\""<<http::utf8_to_jsonstring(record[n].itemnote)<<"\"";
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
   long long getPK(){  return data.xtaskid; } 
 void setPK(long long val){  data.xtaskid=val;} 
 unsigned  int  getXtaskid(){  return data.xtaskid; } 
 void setXtaskid( unsigned  int  val){  data.xtaskid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getXpjid(){  return data.xpjid; } 
 void setXpjid( unsigned  int  val){  data.xpjid=val;} 

 unsigned  int  getAdminid(){  return data.adminid; } 
 void setAdminid( unsigned  int  val){  data.adminid=val;} 

 unsigned  int  getParentid(){  return data.parentid; } 
 void setParentid( unsigned  int  val){  data.parentid=val;} 

 unsigned  int  getBegindate(){  return data.begindate; } 
 void setBegindate( unsigned  int  val){  data.begindate=val;} 

 unsigned  int  getEnddate(){  return data.enddate; } 
 void setEnddate( unsigned  int  val){  data.enddate=val;} 

 unsigned  int  getExpectbegindate(){  return data.expectbegindate; } 
 void setExpectbegindate( unsigned  int  val){  data.expectbegindate=val;} 

 unsigned  int  getExpectenddate(){  return data.expectenddate; } 
 void setExpectenddate( unsigned  int  val){  data.expectenddate=val;} 

 unsigned  int  getMilestone(){  return data.milestone; } 
 void setMilestone( unsigned  int  val){  data.milestone=val;} 

 unsigned  int  getSubxpjid(){  return data.subxpjid; } 
 void setSubxpjid( unsigned  int  val){  data.subxpjid=val;} 

 unsigned  int  getDepxtaskid(){  return data.depxtaskid; } 
 void setDepxtaskid( unsigned  int  val){  data.depxtaskid=val;} 

 unsigned  int  getReferdocid(){  return data.referdocid; } 
 void setReferdocid( unsigned  int  val){  data.referdocid=val;} 

 unsigned  char  getIsfinish(){  return data.isfinish; } 
 void setIsfinish( unsigned  char  val){  data.isfinish=val;} 

 unsigned  int  getUpdatedate(){  return data.updatedate; } 
 void setUpdatedate( unsigned  int  val){  data.updatedate=val;} 

 unsigned  int  getFinishdate(){  return data.finishdate; } 
 void setFinishdate( unsigned  int  val){  data.finishdate=val;} 

 unsigned  int  getIscore(){  return data.iscore; } 
 void setIscore( unsigned  int  val){  data.iscore=val;} 

 double  getXvalue(){  return data.xvalue; } 
 void setXvalue( double  val){  data.xvalue=val;} 

 float  getExpectday(){  return data.expectday; } 
 void setExpectday( float  val){  data.expectday=val;} 

 float  getRealday(){  return data.realday; } 
 void setRealday( float  val){  data.realday=val;} 

 float  getPricevalue(){  return data.pricevalue; } 
 void setPricevalue( float  val){  data.pricevalue=val;} 

 std::string  getTitle(){  return data.title; } 
 std::string & getRefTitle(){  return std::ref(data.title); } 
 void setTitle( std::string  &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

 std::string  getIntroduce(){  return data.introduce; } 
 std::string & getRefIntroduce(){  return std::ref(data.introduce); } 
 void setIntroduce( std::string  &val){  data.introduce=val;} 
 void setIntroduce(std::string_view val){  data.introduce=val;} 

 std::string  getXlogo(){  return data.xlogo; } 
 std::string & getRefXlogo(){  return std::ref(data.xlogo); } 
 void setXlogo( std::string  &val){  data.xlogo=val;} 
 void setXlogo(std::string_view val){  data.xlogo=val;} 

 std::string  getXcolor(){  return data.xcolor; } 
 std::string & getRefXcolor(){  return std::ref(data.xcolor); } 
 void setXcolor( std::string  &val){  data.xcolor=val;} 
 void setXcolor(std::string_view val){  data.xcolor=val;} 

 std::string  getPullurl(){  return data.pullurl; } 
 std::string & getRefPullurl(){  return std::ref(data.pullurl); } 
 void setPullurl( std::string  &val){  data.pullurl=val;} 
 void setPullurl(std::string_view val){  data.pullurl=val;} 

 std::string  getPulltitle(){  return data.pulltitle; } 
 std::string & getRefPulltitle(){  return std::ref(data.pulltitle); } 
 void setPulltitle( std::string  &val){  data.pulltitle=val;} 
 void setPulltitle(std::string_view val){  data.pulltitle=val;} 

 std::string  getPullauthor(){  return data.pullauthor; } 
 std::string & getRefPullauthor(){  return std::ref(data.pullauthor); } 
 void setPullauthor( std::string  &val){  data.pullauthor=val;} 
 void setPullauthor(std::string_view val){  data.pullauthor=val;} 

 std::string  getNote(){  return data.note; } 
 std::string & getRefNote(){  return std::ref(data.note); } 
 void setNote( std::string  &val){  data.note=val;} 
 void setNote(std::string_view val){  data.note=val;} 

 std::string  getItemnote(){  return data.itemnote; } 
 std::string & getRefItemnote(){  return std::ref(data.itemnote); } 
 void setItemnote( std::string  &val){  data.itemnote=val;} 
 void setItemnote(std::string_view val){  data.itemnote=val;} 

xtask_base::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
xtask_base::meta getData(){
 	 return data; 
} 
std::vector<xtask_base::meta> getRecord(){
 	 return record; 
} 

   std::string tree_tojson(const std::vector<meta_tree> &tree_data, std::string fileld=""){
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
            for(jj=0;jj<col_names.size();jj++){
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
if(tree_data[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(tree_data[n].xtaskid);
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
if(tree_data[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(tree_data[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(tree_data[n].adminid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
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
if(tree_data[n].expectbegindate==0){
	tempsql<<"\"expectbegindate\":0";
 }else{ 
	tempsql<<"\"expectbegindate\":"<<std::to_string(tree_data[n].expectbegindate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expectenddate==0){
	tempsql<<"\"expectenddate\":0";
 }else{ 
	tempsql<<"\"expectenddate\":"<<std::to_string(tree_data[n].expectenddate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].milestone==0){
	tempsql<<"\"milestone\":0";
 }else{ 
	tempsql<<"\"milestone\":"<<std::to_string(tree_data[n].milestone);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].subxpjid==0){
	tempsql<<"\"subxpjid\":0";
 }else{ 
	tempsql<<"\"subxpjid\":"<<std::to_string(tree_data[n].subxpjid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].depxtaskid==0){
	tempsql<<"\"depxtaskid\":0";
 }else{ 
	tempsql<<"\"depxtaskid\":"<<std::to_string(tree_data[n].depxtaskid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].referdocid==0){
	tempsql<<"\"referdocid\":0";
 }else{ 
	tempsql<<"\"referdocid\":"<<std::to_string(tree_data[n].referdocid);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isfinish==0){
	tempsql<<"\"isfinish\":0";
 }else{ 
	tempsql<<"\"isfinish\":"<<std::to_string(tree_data[n].isfinish);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updatedate==0){
	tempsql<<"\"updatedate\":0";
 }else{ 
	tempsql<<"\"updatedate\":"<<std::to_string(tree_data[n].updatedate);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].finishdate==0){
	tempsql<<"\"finishdate\":0";
 }else{ 
	tempsql<<"\"finishdate\":"<<std::to_string(tree_data[n].finishdate);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].iscore==0){
	tempsql<<"\"iscore\":0";
 }else{ 
	tempsql<<"\"iscore\":"<<std::to_string(tree_data[n].iscore);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].xvalue==0){
	tempsql<<"\"xvalue\":0";
 }else{ 
	tempsql<<"\"xvalue\":"<<std::to_string(tree_data[n].xvalue);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(tree_data[n].expectday);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(tree_data[n].realday);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].pricevalue==0){
	tempsql<<"\"pricevalue\":0";
 }else{ 
	tempsql<<"\"pricevalue\":"<<std::to_string(tree_data[n].pricevalue);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(tree_data[n].xlogo)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(tree_data[n].xcolor)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullurl\":\""<<http::utf8_to_jsonstring(tree_data[n].pullurl)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pulltitle\":\""<<http::utf8_to_jsonstring(tree_data[n].pulltitle)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullauthor\":\""<<http::utf8_to_jsonstring(tree_data[n].pullauthor)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"note\":\""<<http::utf8_to_jsonstring(tree_data[n].note)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"itemnote\":\""<<http::utf8_to_jsonstring(tree_data[n].itemnote)<<"\"";
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
   
   std::string tree_tojson(const std::vector<meta_tree> &tree_data,std::function<bool(std::string&,const meta_tree&)> func,std::string fileld=""){
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
            for(jj=0;jj<col_names.size();jj++){
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
if(tree_data[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(tree_data[n].xtaskid);
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
if(tree_data[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(tree_data[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(tree_data[n].adminid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
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
if(tree_data[n].expectbegindate==0){
	tempsql<<"\"expectbegindate\":0";
 }else{ 
	tempsql<<"\"expectbegindate\":"<<std::to_string(tree_data[n].expectbegindate);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expectenddate==0){
	tempsql<<"\"expectenddate\":0";
 }else{ 
	tempsql<<"\"expectenddate\":"<<std::to_string(tree_data[n].expectenddate);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].milestone==0){
	tempsql<<"\"milestone\":0";
 }else{ 
	tempsql<<"\"milestone\":"<<std::to_string(tree_data[n].milestone);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].subxpjid==0){
	tempsql<<"\"subxpjid\":0";
 }else{ 
	tempsql<<"\"subxpjid\":"<<std::to_string(tree_data[n].subxpjid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].depxtaskid==0){
	tempsql<<"\"depxtaskid\":0";
 }else{ 
	tempsql<<"\"depxtaskid\":"<<std::to_string(tree_data[n].depxtaskid);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].referdocid==0){
	tempsql<<"\"referdocid\":0";
 }else{ 
	tempsql<<"\"referdocid\":"<<std::to_string(tree_data[n].referdocid);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isfinish==0){
	tempsql<<"\"isfinish\":0";
 }else{ 
	tempsql<<"\"isfinish\":"<<std::to_string(tree_data[n].isfinish);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updatedate==0){
	tempsql<<"\"updatedate\":0";
 }else{ 
	tempsql<<"\"updatedate\":"<<std::to_string(tree_data[n].updatedate);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].finishdate==0){
	tempsql<<"\"finishdate\":0";
 }else{ 
	tempsql<<"\"finishdate\":"<<std::to_string(tree_data[n].finishdate);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].iscore==0){
	tempsql<<"\"iscore\":0";
 }else{ 
	tempsql<<"\"iscore\":"<<std::to_string(tree_data[n].iscore);
}
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].xvalue==0){
	tempsql<<"\"xvalue\":0";
 }else{ 
	tempsql<<"\"xvalue\":"<<std::to_string(tree_data[n].xvalue);
}
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].expectday==0){
	tempsql<<"\"expectday\":0";
 }else{ 
	tempsql<<"\"expectday\":"<<std::to_string(tree_data[n].expectday);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].realday==0){
	tempsql<<"\"realday\":0";
 }else{ 
	tempsql<<"\"realday\":"<<std::to_string(tree_data[n].realday);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].pricevalue==0){
	tempsql<<"\"pricevalue\":0";
 }else{ 
	tempsql<<"\"pricevalue\":"<<std::to_string(tree_data[n].pricevalue);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xlogo\":\""<<http::utf8_to_jsonstring(tree_data[n].xlogo)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"xcolor\":\""<<http::utf8_to_jsonstring(tree_data[n].xcolor)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullurl\":\""<<http::utf8_to_jsonstring(tree_data[n].pullurl)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pulltitle\":\""<<http::utf8_to_jsonstring(tree_data[n].pulltitle)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"pullauthor\":\""<<http::utf8_to_jsonstring(tree_data[n].pullauthor)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"note\":\""<<http::utf8_to_jsonstring(tree_data[n].note)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"itemnote\":\""<<http::utf8_to_jsonstring(tree_data[n].itemnote)<<"\"";
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
   

    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="title")
		{
			return data.title;
		}
		 if(key_name=="introduce")
		{
			return data.introduce;
		}
		 if(key_name=="xlogo")
		{
			return data.xlogo;
		}
		 if(key_name=="xcolor")
		{
			return data.xcolor;
		}
		 if(key_name=="pullurl")
		{
			return data.pullurl;
		}
		 if(key_name=="pulltitle")
		{
			return data.pulltitle;
		}
		 if(key_name=="pullauthor")
		{
			return data.pullauthor;
		}
		 if(key_name=="note")
		{
			return data.note;
		}
		 if(key_name=="itemnote")
		{
			return data.itemnote;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="xtaskid")
		{
			return data.xtaskid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="xpjid")
		{
			return data.xpjid;
		}
		 if(key_name=="adminid")
		{
			return data.adminid;
		}
		 if(key_name=="parentid")
		{
			return data.parentid;
		}
		 if(key_name=="begindate")
		{
			return data.begindate;
		}
		 if(key_name=="enddate")
		{
			return data.enddate;
		}
		 if(key_name=="expectbegindate")
		{
			return data.expectbegindate;
		}
		 if(key_name=="expectenddate")
		{
			return data.expectenddate;
		}
		 if(key_name=="milestone")
		{
			return data.milestone;
		}
		 if(key_name=="subxpjid")
		{
			return data.subxpjid;
		}
		 if(key_name=="depxtaskid")
		{
			return data.depxtaskid;
		}
		 if(key_name=="referdocid")
		{
			return data.referdocid;
		}
		 if(key_name=="isfinish")
		{
			return data.isfinish;
		}
		 if(key_name=="updatedate")
		{
			return data.updatedate;
		}
		 if(key_name=="finishdate")
		{
			return data.finishdate;
		}
		 if(key_name=="iscore")
		{
			return data.iscore;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="xvalue")
		{
			return data.xvalue;
		}
		 if(key_name=="expectday")
		{
			return data.expectday;
		}
		 if(key_name=="realday")
		{
			return data.realday;
		}
		 if(key_name=="pricevalue")
		{
			return data.pricevalue;
		}
		return nullptr; 
	}

            template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >  
            std::vector<T> getCol([[maybe_unused]] std::string keyname)
            {
                std::vector<T> a;
                
   
                unsigned char kpos;
                kpos=findcolpos(keyname);               
                for(auto &iter:record)
                {
                    switch(kpos)
                    {
   			case 0: 
 				 a.emplace_back(iter.xtaskid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a.emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a.emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a.emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a.emplace_back(iter.enddate);
				 break;
			case 7: 
 				 a.emplace_back(iter.expectbegindate);
				 break;
			case 8: 
 				 a.emplace_back(iter.expectenddate);
				 break;
			case 9: 
 				 a.emplace_back(iter.milestone);
				 break;
			case 10: 
 				 a.emplace_back(iter.subxpjid);
				 break;
			case 11: 
 				 a.emplace_back(iter.depxtaskid);
				 break;
			case 12: 
 				 a.emplace_back(iter.referdocid);
				 break;
			case 13: 
 				 a.emplace_back(iter.isfinish);
				 break;
			case 14: 
 				 a.emplace_back(iter.updatedate);
				 break;
			case 15: 
 				 a.emplace_back(iter.finishdate);
				 break;
			case 16: 
 				 a.emplace_back(iter.iscore);
				 break;

                    }
                }
    
                return a;
            }
    
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >    
			std::vector<T> getCol([[maybe_unused]] std::string keyname)
			{
				std::vector<T> a;
				

                unsigned char kpos;
				kpos = findcolpos(keyname);  
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

			case 17: 
 					 a.emplace_back(iter.xvalue);
			 break;
			case 18: 
 					 a.emplace_back(iter.expectday);
			 break;
			case 19: 
 					 a.emplace_back(iter.realday);
			 break;
			case 20: 
 					 a.emplace_back(iter.pricevalue);
			 break;

                    }
                }
    
                return a;
            }
    
            template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
            T getVal([[maybe_unused]] std::string keyname)
            {
   
                    unsigned char kpos;
                    kpos=findcolpos(keyname);                   
                    switch(kpos)
                    {

   			case 0: 
 				 return data.xtaskid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.xpjid;
				 break;
			case 3: 
 				 return data.adminid;
				 break;
			case 4: 
 				 return data.parentid;
				 break;
			case 5: 
 				 return data.begindate;
				 break;
			case 6: 
 				 return data.enddate;
				 break;
			case 7: 
 				 return data.expectbegindate;
				 break;
			case 8: 
 				 return data.expectenddate;
				 break;
			case 9: 
 				 return data.milestone;
				 break;
			case 10: 
 				 return data.subxpjid;
				 break;
			case 11: 
 				 return data.depxtaskid;
				 break;
			case 12: 
 				 return data.referdocid;
				 break;
			case 13: 
 				 return data.isfinish;
				 break;
			case 14: 
 				 return data.updatedate;
				 break;
			case 15: 
 				 return data.finishdate;
				 break;
			case 16: 
 				 return data.iscore;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] xtask_base::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.xtaskid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.xpjid;
				 break;
			case 3: 
 				 return iter.adminid;
				 break;
			case 4: 
 				 return iter.parentid;
				 break;
			case 5: 
 				 return iter.begindate;
				 break;
			case 6: 
 				 return iter.enddate;
				 break;
			case 7: 
 				 return iter.expectbegindate;
				 break;
			case 8: 
 				 return iter.expectenddate;
				 break;
			case 9: 
 				 return iter.milestone;
				 break;
			case 10: 
 				 return iter.subxpjid;
				 break;
			case 11: 
 				 return iter.depxtaskid;
				 break;
			case 12: 
 				 return iter.referdocid;
				 break;
			case 13: 
 				 return iter.isfinish;
				 break;
			case 14: 
 				 return iter.updatedate;
				 break;
			case 15: 
 				 return iter.finishdate;
				 break;
			case 16: 
 				 return iter.iscore;
				 break;

			}

            return 0;
        }  
    
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
            T getVal(std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
            
                switch(kpos)
                {

    			case 17: 
 				 return data.xvalue;
				 break;
			case 18: 
 				 return data.expectday;
				 break;
			case 19: 
 				 return data.realday;
				 break;
			case 20: 
 				 return data.pricevalue;
				 break;

                    }
                   
    
                    return 0.0;
            }  
    
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
            T getVal([[maybe_unused]] xtask_base::meta & iter,std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
                switch(kpos)
                {
   			case 17: 
 				 return iter.xvalue;
				 break;
			case 18: 
 				 return iter.expectday;
				 break;
			case 19: 
 				 return iter.realday;
				 break;
			case 20: 
 				 return iter.pricevalue;
				 break;

                }
                 
    
            
                return 0.0;
            }  
    
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal(std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
        
                switch(kpos)
                {

   			case 21: 
 				 return data.title;
				 break;
			case 22: 
 				 return data.introduce;
				 break;
			case 23: 
 				 return data.xlogo;
				 break;
			case 24: 
 				 return data.xcolor;
				 break;
			case 25: 
 				 return data.pullurl;
				 break;
			case 26: 
 				 return data.pulltitle;
				 break;
			case 27: 
 				 return data.pullauthor;
				 break;
			case 28: 
 				 return data.note;
				 break;
			case 29: 
 				 return data.itemnote;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] xtask_base::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 21: 
 				 return iter.title;
				 break;
			case 22: 
 				 return iter.introduce;
				 break;
			case 23: 
 				 return iter.xlogo;
				 break;
			case 24: 
 				 return iter.xcolor;
				 break;
			case 25: 
 				 return iter.pullurl;
				 break;
			case 26: 
 				 return iter.pulltitle;
				 break;
			case 27: 
 				 return iter.pullauthor;
				 break;
			case 28: 
 				 return iter.note;
				 break;
			case 29: 
 				 return iter.itemnote;
				 break;

                }
                
    
                 
                return "";
            }  
     
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >   
            std::vector<std::string> getCol([[maybe_unused]] std::string keyname)
            {
                std::vector<std::string> a;

           
                unsigned char kpos;
                kpos=findcolpos(keyname);                    
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

    			case 21: 
 				 a.emplace_back(iter.title);
					 break;
			case 22: 
 				 a.emplace_back(iter.introduce);
					 break;
			case 23: 
 				 a.emplace_back(iter.xlogo);
					 break;
			case 24: 
 				 a.emplace_back(iter.xcolor);
					 break;
			case 25: 
 				 a.emplace_back(iter.pullurl);
					 break;
			case 26: 
 				 a.emplace_back(iter.pulltitle);
					 break;
			case 27: 
 				 a.emplace_back(iter.pullauthor);
					 break;
			case 28: 
 				 a.emplace_back(iter.note);
					 break;
			case 29: 
 				 a.emplace_back(iter.itemnote);
					 break;
					}
				}

        return a;
    }
     
        std::string getstrCol(std::string keyname,[[maybe_unused]] bool isyinhao=false)
        {
            std::ostringstream a;
    
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            int j=0;
            if(isyinhao&&record.size()>0)
            {
                a<<'"';
            }
            for(auto &iter:record)
            {
                    if(j>0)
                    {
                        if(isyinhao)
                        {
                            a<<"\",\"";
                        }else{
                            a<<',';    
                        }
                    }
                    switch(kpos)
                    {

   			case 0: 
 				 a<<std::to_string(iter.xtaskid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.xpjid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.adminid);
				 break;
			case 4: 
 				 a<<std::to_string(iter.parentid);
				 break;
			case 5: 
 				 a<<std::to_string(iter.begindate);
				 break;
			case 6: 
 				 a<<std::to_string(iter.enddate);
				 break;
			case 7: 
 				 a<<std::to_string(iter.expectbegindate);
				 break;
			case 8: 
 				 a<<std::to_string(iter.expectenddate);
				 break;
			case 9: 
 				 a<<std::to_string(iter.milestone);
				 break;
			case 10: 
 				 a<<std::to_string(iter.subxpjid);
				 break;
			case 11: 
 				 a<<std::to_string(iter.depxtaskid);
				 break;
			case 12: 
 				 a<<std::to_string(iter.referdocid);
				 break;
			case 13: 
 				 a<<std::to_string(iter.isfinish);
				 break;
			case 14: 
 				 a<<std::to_string(iter.updatedate);
				 break;
			case 15: 
 				 a<<std::to_string(iter.finishdate);
				 break;
			case 16: 
 				 a<<std::to_string(iter.iscore);
				 break;
			case 17: 
 				 a<<std::to_string(iter.xvalue);
				 break;
			case 18: 
 				 a<<std::to_string(iter.expectday);
				 break;
			case 19: 
 				 a<<std::to_string(iter.realday);
				 break;
			case 20: 
 				 a<<std::to_string(iter.pricevalue);
				 break;
			case 21: 
 				 if(isyinhao){ a<<jsonaddslash(iter.title); 
				 }else{
				 a<<iter.title;
				 }
				 break;
			case 22: 
 				 if(isyinhao){ a<<jsonaddslash(iter.introduce); 
				 }else{
				 a<<iter.introduce;
				 }
				 break;
			case 23: 
 				 if(isyinhao){ a<<jsonaddslash(iter.xlogo); 
				 }else{
				 a<<iter.xlogo;
				 }
				 break;
			case 24: 
 				 if(isyinhao){ a<<jsonaddslash(iter.xcolor); 
				 }else{
				 a<<iter.xcolor;
				 }
				 break;
			case 25: 
 				 if(isyinhao){ a<<jsonaddslash(iter.pullurl); 
				 }else{
				 a<<iter.pullurl;
				 }
				 break;
			case 26: 
 				 if(isyinhao){ a<<jsonaddslash(iter.pulltitle); 
				 }else{
				 a<<iter.pulltitle;
				 }
				 break;
			case 27: 
 				 if(isyinhao){ a<<jsonaddslash(iter.pullauthor); 
				 }else{
				 a<<iter.pullauthor;
				 }
				 break;
			case 28: 
 				 if(isyinhao){ a<<jsonaddslash(iter.note); 
				 }else{
				 a<<iter.note;
				 }
				 break;
			case 29: 
 				 if(isyinhao){ a<<jsonaddslash(iter.itemnote); 
				 }else{
				 a<<iter.itemnote;
				 }
				 break;

                    }
                    j++;
            } 
            if(isyinhao&&j>0){
                a<<'"';
            }      
    
                return a.str();
        }
    
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>     
    std::map<std::string,std::string> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
    {
        std::map<std::string,std::string> a;
    
        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);        
         std::string ktemp,vtemp;
         for(auto &iter:record)
         {
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
				 } 
			switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;

                }
                if(ktemp.size()>0)
                {
                    a.emplace(ktemp,vtemp);
                }
            }       

        
            return a;
        } 
    

        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<std::string,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::map<std::string,U> a;
      
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);            
                std::string ktemp;
                U vtemp;
                for(auto &iter:record)
                {    
                    switch(kpos)
                    {
 
       			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			 } 
		 switch(vpos){
			case 17: 
 				 vtemp=iter.xvalue;
				 break;
			case 18: 
 				 vtemp=iter.expectday;
				 break;
			case 19: 
 				 vtemp=iter.realday;
				 break;
			case 20: 
 				 vtemp=iter.pricevalue;
				 break;

                    }
                    if(ktemp.size()>0)
                    {
                        a.emplace(ktemp,vtemp);
                    }
                }       
        
            return a;
        } 
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>       
        std::map<T,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,U> a;
       
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);        
            T ktemp;
            U vtemp;
            for(auto &iter:record)
            {
                switch(kpos)
                {
 
       case 0: 
 	 ktemp=iter.xtaskid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.xpjid;
	 break;
case 3: 
 	 ktemp=iter.adminid;
	 break;
case 4: 
 	 ktemp=iter.parentid;
	 break;
case 5: 
 	 ktemp=iter.begindate;
	 break;
case 6: 
 	 ktemp=iter.enddate;
	 break;
case 7: 
 	 ktemp=iter.expectbegindate;
	 break;
case 8: 
 	 ktemp=iter.expectenddate;
	 break;
case 9: 
 	 ktemp=iter.milestone;
	 break;
case 10: 
 	 ktemp=iter.subxpjid;
	 break;
case 11: 
 	 ktemp=iter.depxtaskid;
	 break;
case 12: 
 	 ktemp=iter.referdocid;
	 break;
case 13: 
 	 ktemp=iter.isfinish;
	 break;
case 14: 
 	 ktemp=iter.updatedate;
	 break;
case 15: 
 	 ktemp=iter.finishdate;
	 break;
case 16: 
 	 ktemp=iter.iscore;
	 break;
	 } 
 		  switch(vpos){
case 17: 
 	 vtemp=iter.xvalue;
	 break;
case 18: 
 	 vtemp=iter.expectday;
	 break;
case 19: 
 	 vtemp=iter.realday;
	 break;
case 20: 
 	 vtemp=iter.pricevalue;
	 break;

                }
                if(ktemp.size()>0)
                {
                    a.emplace(ktemp,vtemp);
                }
            }       
     
        return a;
    }  
            template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>      
            std::map<T,std::string> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
            {
                std::map<T,std::string> a;
   
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);         
                T ktemp;
                std::string vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }
 			switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;

                    }
                    if(ktemp.size()>0)
                    {
                        a.emplace(ktemp,vtemp);
                    }
                } 
         
                return a;
            }     
        
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>       
        std::map<std::string,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<std::string,U> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);            
            std::string  ktemp;
            U  vtemp;
            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;

                }
                if(ktemp.size()>0)
                {
                    a.emplace(ktemp,vtemp);
                }
            }       
      
        return a;
    }  
    
        template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>   
        std::map<T,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,U> a;
    
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);            
            T ktemp;
            U vtemp;
            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;

                }
                if(ktemp.size()>0)
                {
                    a.emplace(ktemp,vtemp);
                }
            }       
    
            return a;
        }   
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >         
        std::map<T,meta> getmapRows([[maybe_unused]] std::string keyname)
        {
            std::map<T,meta> a;
    
            unsigned char kpos;
            kpos=findcolpos(keyname);                        
            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 0: 
 				 a.emplace(iter.xtaskid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.xpjid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.adminid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.parentid,iter);
				 break;
			case 5: 
 				 a.emplace(iter.begindate,iter);
				 break;
			case 6: 
 				 a.emplace(iter.enddate,iter);
				 break;
			case 7: 
 				 a.emplace(iter.expectbegindate,iter);
				 break;
			case 8: 
 				 a.emplace(iter.expectenddate,iter);
				 break;
			case 9: 
 				 a.emplace(iter.milestone,iter);
				 break;
			case 10: 
 				 a.emplace(iter.subxpjid,iter);
				 break;
			case 11: 
 				 a.emplace(iter.depxtaskid,iter);
				 break;
			case 12: 
 				 a.emplace(iter.referdocid,iter);
				 break;
			case 13: 
 				 a.emplace(iter.isfinish,iter);
				 break;
			case 14: 
 				 a.emplace(iter.updatedate,iter);
				 break;
			case 15: 
 				 a.emplace(iter.finishdate,iter);
				 break;
			case 16: 
 				 a.emplace(iter.iscore,iter);
				 break;

                }
            }       
     
            return a;
        }     
    
        template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >    
        std::map<std::string,meta> getmapRows([[maybe_unused]] std::string keyname)
        {
            std::map<std::string,meta> a;

    
            unsigned char kpos;
            kpos=findcolpos(keyname);            
            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 21: 
 				 a.emplace(iter.title,iter);
			 break;
			case 22: 
 				 a.emplace(iter.introduce,iter);
			 break;
			case 23: 
 				 a.emplace(iter.xlogo,iter);
			 break;
			case 24: 
 				 a.emplace(iter.xcolor,iter);
			 break;
			case 25: 
 				 a.emplace(iter.pullurl,iter);
			 break;
			case 26: 
 				 a.emplace(iter.pulltitle,iter);
			 break;
			case 27: 
 				 a.emplace(iter.pullauthor,iter);
			 break;
			case 28: 
 				 a.emplace(iter.note,iter);
			 break;
			case 29: 
 				 a.emplace(iter.itemnote,iter);
			 break;

                }
                //a.emplace(ktemp,iter);
            }       
    

        return a;
    }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>  
        std::vector<std::pair<std::string,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::vector<std::pair<std::string,U>> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);                   
            std::string ktemp;
            U vtemp;
            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
	 		 }
 			switch(vpos){
			case 17: 
 				 vtemp=iter.xvalue;
				 break;
			case 18: 
 				 vtemp=iter.expectday;
				 break;
			case 19: 
 				 vtemp=iter.realday;
				 break;
			case 20: 
 				 vtemp=iter.pricevalue;
				 break;

                   }

                 a.emplace_back(ktemp,vtemp);
            }       

     

            return a;
        }   
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;
   
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   			case 0: 
 				 ktemp=iter.xtaskid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.xpjid;
			 break;
			case 3: 
 				 ktemp=iter.adminid;
			 break;
			case 4: 
 				 ktemp=iter.parentid;
			 break;
			case 5: 
 				 ktemp=iter.begindate;
			 break;
			case 6: 
 				 ktemp=iter.enddate;
			 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
			 break;
			case 8: 
 				 ktemp=iter.expectenddate;
			 break;
			case 9: 
 				 ktemp=iter.milestone;
			 break;
			case 10: 
 				 ktemp=iter.subxpjid;
			 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
			 break;
			case 12: 
 				 ktemp=iter.referdocid;
			 break;
			case 13: 
 				 ktemp=iter.isfinish;
			 break;
			case 14: 
 				 ktemp=iter.updatedate;
			 break;
			case 15: 
 				 ktemp=iter.finishdate;
			 break;
			case 16: 
 				 ktemp=iter.iscore;
			 break;
			  }
			 switch(vpos){
			case 17: 
 				 vtemp=iter.xvalue;
				 break;
			case 18: 
 				 vtemp=iter.expectday;
				 break;
			case 19: 
 				 vtemp=iter.realday;
				 break;
			case 20: 
 				 vtemp=iter.pricevalue;
				 break;

                   }

                   a.emplace_back(ktemp,vtemp);
                }       

    

            return a;
        }   
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;

   
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }
 			switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;

                   }

                    a.emplace_back(ktemp,vtemp);
                }       
    
            return a;
        }  
    
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>     
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;
   
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);                
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    
                    switch(kpos)
                    {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;

                   }
                    a.emplace_back(ktemp,vtemp);
                }       
    
            return a;
        }  
    
        template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;
   
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;

                   }
                    a.emplace_back(ktemp,vtemp);
                }       
      
            return a;
        }   
    
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>     
        std::vector<std::pair<T,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::vector<std::pair<T,U>> a;
   
                unsigned char kpos,vpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                T ktemp;
                U vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   case 21: 
 	 ktemp=iter.title;
	 break;
case 22: 
 	 ktemp=iter.introduce;
	 break;
case 23: 
 	 ktemp=iter.xlogo;
	 break;
case 24: 
 	 ktemp=iter.xcolor;
	 break;
case 25: 
 	 ktemp=iter.pullurl;
	 break;
case 26: 
 	 ktemp=iter.pulltitle;
	 break;
case 27: 
 	 ktemp=iter.pullauthor;
	 break;
case 28: 
 	 ktemp=iter.note;
	 break;
case 29: 
 	 ktemp=iter.itemnote;
	 break;
	  }
 switch(vpos){
case 21: 
 	 vtemp=iter.title;
	 break;
case 22: 
 	 vtemp=iter.introduce;
	 break;
case 23: 
 	 vtemp=iter.xlogo;
	 break;
case 24: 
 	 vtemp=iter.xcolor;
	 break;
case 25: 
 	 vtemp=iter.pullurl;
	 break;
case 26: 
 	 vtemp=iter.pulltitle;
	 break;
case 27: 
 	 vtemp=iter.pullauthor;
	 break;
case 28: 
 	 vtemp=iter.note;
	 break;
case 29: 
 	 vtemp=iter.itemnote;
	 break;

                   }

                    a.emplace_back(ktemp,vtemp);
                }       
    
            return a;
        }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
        std::vector<std::pair<T,meta>> getvecRows([[maybe_unused]] std::string keyname)
        {
            std::vector<std::pair<T,meta>> a;
     
            unsigned char kpos;
            kpos=findcolpos(keyname);                  
            for(auto &iter:record)
            { 
                switch(kpos)
                {

   case 0: 
 	 a.emplace_back(iter.xtaskid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.xpjid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.adminid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.parentid,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.begindate,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.enddate,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.expectbegindate,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.expectenddate,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.milestone,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.subxpjid,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.depxtaskid,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.referdocid,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.isfinish,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.updatedate,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.finishdate,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.iscore,iter);
	 break;

                }
            }       
    
        return a;
    }
        template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >  
        std::vector<std::pair<std::string,meta>> getvecRows([[maybe_unused]] std::string keyname)
        {
            std::vector<std::pair<std::string,meta>> a;
      
            unsigned char kpos;
            kpos=findcolpos(keyname);                     
            for(auto &iter:record)
            {
                switch(kpos)
                {

   case 21: 
 	 a.emplace_back(iter.title,iter);
	 break;
case 22: 
 	 a.emplace_back(iter.introduce,iter);
	 break;
case 23: 
 	 a.emplace_back(iter.xlogo,iter);
	 break;
case 24: 
 	 a.emplace_back(iter.xcolor,iter);
	 break;
case 25: 
 	 a.emplace_back(iter.pullurl,iter);
	 break;
case 26: 
 	 a.emplace_back(iter.pulltitle,iter);
	 break;
case 27: 
 	 a.emplace_back(iter.pullauthor,iter);
	 break;
case 28: 
 	 a.emplace_back(iter.note,iter);
	 break;
case 29: 
 	 a.emplace_back(iter.itemnote,iter);
	 break;

                }
            }       
    

        return a;
    }
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
    
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);      
            T ktemp;
            U vtemp;
            for(auto &iter:record)
            { 
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;
			  }

			 switch(dpos){
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.xvalue);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.expectday);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.realday);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.pricevalue);
				 break;

                }
            }       

    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
    
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);          
            T ktemp;
            U vtemp;
            //D vtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xtaskid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.expectbegindate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.expectenddate);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.milestone);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.subxpjid);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.depxtaskid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.referdocid);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.isfinish);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.updatedate);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.finishdate);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.iscore);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);       
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;
			 }

			 switch(dpos){
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.xlogo);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.xcolor);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.pullurl);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.pulltitle);
				 break;
			case 27: 
 				 a[ktemp][vtemp].emplace_back(iter.pullauthor);
				 break;
			case 28: 
 				 a[ktemp][vtemp].emplace_back(iter.note);
				 break;
			case 29: 
 				 a[ktemp][vtemp].emplace_back(iter.itemnote);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
                std::map<T,std::map<U,std::vector<D>>> a;
   
                unsigned char kpos,vpos,dpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                dpos=findcolpos(dataname);
                T ktemp;
                U vtemp;
            // D dtemp;

                for(auto &iter:record)
                {
                    
                    switch(kpos)
                    {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			 }

			 switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;
			  }

			 switch(dpos){
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.xvalue);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.expectday);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.realday);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.pricevalue);
				 break;

                   }
                }       
    
            return a;
        }
    
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
    std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
    {
        std::map<T,std::map<U,std::vector<D>>> a;

   
        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);             
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record)
         {
            switch(kpos)
            {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			 }

			 switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xtaskid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.expectbegindate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.expectenddate);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.milestone);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.subxpjid);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.depxtaskid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.referdocid);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.isfinish);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.updatedate);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.finishdate);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.iscore);
				 break;

            }
         }       
    
        return a;
    }
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            { 
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }

			 switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;
			  }

			 switch(dpos){
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.xlogo);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.xcolor);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.pullurl);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.pulltitle);
				 break;
			case 27: 
 				 a[ktemp][vtemp].emplace_back(iter.pullauthor);
				 break;
			case 28: 
 				 a[ktemp][vtemp].emplace_back(iter.note);
				 break;
			case 29: 
 				 a[ktemp][vtemp].emplace_back(iter.itemnote);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
                std::map<T,std::map<U,std::vector<D>>> a;
   
                unsigned char kpos,vpos,dpos;
                kpos=findcolpos(keyname);
                vpos=findcolpos(valname);
                dpos=findcolpos(dataname);               
                T ktemp;
                U vtemp;
                //D vtemp;
                for(auto &iter:record)
                {
                    switch(kpos)
                    {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;
			  }

			 switch(dpos){
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.xvalue);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.expectday);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.realday);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.pricevalue);
				 break;

                    }
                }       
    

            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);            
            T ktemp;
            U vtemp;
            //D vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xtaskid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.expectbegindate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.expectenddate);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.milestone);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.subxpjid);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.depxtaskid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.referdocid);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.isfinish);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.updatedate);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.finishdate);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.iscore);
				 break;

                }
            }       
    
            return a;
        }
    
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
    std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
    {
        std::map<T,std::map<U,std::vector<D>>> a;

   
        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
        // D dtemp;

         for(auto &iter:record)
         {
             
            switch(kpos)
            {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.xtaskid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 7: 
 				 vtemp=iter.expectbegindate;
				 break;
			case 8: 
 				 vtemp=iter.expectenddate;
				 break;
			case 9: 
 				 vtemp=iter.milestone;
				 break;
			case 10: 
 				 vtemp=iter.subxpjid;
				 break;
			case 11: 
 				 vtemp=iter.depxtaskid;
				 break;
			case 12: 
 				 vtemp=iter.referdocid;
				 break;
			case 13: 
 				 vtemp=iter.isfinish;
				 break;
			case 14: 
 				 vtemp=iter.updatedate;
				 break;
			case 15: 
 				 vtemp=iter.finishdate;
				 break;
			case 16: 
 				 vtemp=iter.iscore;
				 break;
			 }

			switch(dpos){
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.xlogo);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.xcolor);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.pullurl);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.pulltitle);
				 break;
			case 27: 
 				 a[ktemp][vtemp].emplace_back(iter.pullauthor);
				 break;
			case 28: 
 				 a[ktemp][vtemp].emplace_back(iter.note);
				 break;
			case 29: 
 				 a[ktemp][vtemp].emplace_back(iter.itemnote);
				 break;

            }
         }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);        
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			 }

			 switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;
			  }

			 switch(dpos){
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.xvalue);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.expectday);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.realday);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.pricevalue);
				 break;

                }
            }       

    
            return a;
        }
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			  }

			 switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xtaskid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.expectbegindate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.expectenddate);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.milestone);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.subxpjid);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.depxtaskid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.referdocid);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.isfinish);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.updatedate);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.finishdate);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.iscore);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   
            unsigned char kpos,vpos,dpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            dpos=findcolpos(dataname);        
            T ktemp;
            U vtemp;
            // D dtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			  }

			 switch(vpos){
			case 21: 
 				 vtemp=iter.title;
				 break;
			case 22: 
 				 vtemp=iter.introduce;
				 break;
			case 23: 
 				 vtemp=iter.xlogo;
				 break;
			case 24: 
 				 vtemp=iter.xcolor;
				 break;
			case 25: 
 				 vtemp=iter.pullurl;
				 break;
			case 26: 
 				 vtemp=iter.pulltitle;
				 break;
			case 27: 
 				 vtemp=iter.pullauthor;
				 break;
			case 28: 
 				 vtemp=iter.note;
				 break;
			case 29: 
 				 vtemp=iter.itemnote;
				 break;
			  }

			 switch(dpos){
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.xlogo);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.xcolor);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.pullurl);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.pulltitle);
				 break;
			case 27: 
 				 a[ktemp][vtemp].emplace_back(iter.pullauthor);
				 break;
			case 28: 
 				 a[ktemp][vtemp].emplace_back(iter.note);
				 break;
			case 29: 
 				 a[ktemp][vtemp].emplace_back(iter.itemnote);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;

   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			  }

			 switch(vpos){
			case 21: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 22: 
 				 a[ktemp].emplace_back(iter.introduce);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.xlogo);
				 break;
			case 24: 
 				 a[ktemp].emplace_back(iter.xcolor);
				 break;
			case 25: 
 				 a[ktemp].emplace_back(iter.pullurl);
				 break;
			case 26: 
 				 a[ktemp].emplace_back(iter.pulltitle);
				 break;
			case 27: 
 				 a[ktemp].emplace_back(iter.pullauthor);
				 break;
			case 28: 
 				 a[ktemp].emplace_back(iter.note);
				 break;
			case 29: 
 				 a[ktemp].emplace_back(iter.itemnote);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			 }

			 switch(vpos){
			case 17: 
 				 a[ktemp].emplace_back(iter.xvalue);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.expectday);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.realday);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.pricevalue);
				 break;

                }
            }       

    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.xtaskid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.enddate);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.expectbegindate);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.expectenddate);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.milestone);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.subxpjid);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.depxtaskid);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.referdocid);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.isfinish);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.updatedate);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.finishdate);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.iscore);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			 }

			 switch(vpos){
			case 21: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 22: 
 				 a[ktemp].emplace_back(iter.introduce);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.xlogo);
				 break;
			case 24: 
 				 a[ktemp].emplace_back(iter.xcolor);
				 break;
			case 25: 
 				 a[ktemp].emplace_back(iter.pullurl);
				 break;
			case 26: 
 				 a[ktemp].emplace_back(iter.pulltitle);
				 break;
			case 27: 
 				 a[ktemp].emplace_back(iter.pullauthor);
				 break;
			case 28: 
 				 a[ktemp].emplace_back(iter.note);
				 break;
			case 29: 
 				 a[ktemp].emplace_back(iter.itemnote);
				 break;

                }
            }       

    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
               
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }

			 switch(vpos){
			case 17: 
 				 a[ktemp].emplace_back(iter.xvalue);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.expectday);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.realday);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.pricevalue);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.xtaskid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.enddate);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.expectbegindate);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.expectenddate);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.milestone);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.subxpjid);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.depxtaskid);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.referdocid);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.isfinish);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.updatedate);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.finishdate);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.iscore);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename std::enable_if<std::is_integral_v<T>,bool>::type = true>    
        std::map<T,std::vector<meta>> getgroupRows([[maybe_unused]] std::string keyname)
        {
            std::map<T,std::vector<meta>> a;
   
            unsigned char kpos;
            kpos=findcolpos(keyname);

            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 0: 
 				 a[iter.xtaskid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.xpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.adminid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.begindate].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.enddate].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.expectbegindate].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.expectenddate].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.milestone].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.subxpjid].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.depxtaskid].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.referdocid].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.isfinish].emplace_back(iter);
				 break;
			case 14: 
 				 a[iter.updatedate].emplace_back(iter);
				 break;
			case 15: 
 				 a[iter.finishdate].emplace_back(iter);
				 break;
			case 16: 
 				 a[iter.iscore].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<meta>> getgroupRows([[maybe_unused]] std::string keyname)
        {
            std::map<T,std::vector<meta>> a;
   
            unsigned char kpos;
            kpos=findcolpos(keyname);

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 21: 
 				 a[iter.title].emplace_back(iter);
				 break;
			case 22: 
 				 a[iter.introduce].emplace_back(iter);
				 break;
			case 23: 
 				 a[iter.xlogo].emplace_back(iter);
				 break;
			case 24: 
 				 a[iter.xcolor].emplace_back(iter);
				 break;
			case 25: 
 				 a[iter.pullurl].emplace_back(iter);
				 break;
			case 26: 
 				 a[iter.pulltitle].emplace_back(iter);
				 break;
			case 27: 
 				 a[iter.pullauthor].emplace_back(iter);
				 break;
			case 28: 
 				 a[iter.note].emplace_back(iter);
				 break;
			case 29: 
 				 a[iter.itemnote].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);
            T ktemp;

            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
			 }

			 switch(vpos){
			case 21: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 22: 
 				 a[ktemp][iter.introduce].emplace_back(iter);
				 break;
			case 23: 
 				 a[ktemp][iter.xlogo].emplace_back(iter);
				 break;
			case 24: 
 				 a[ktemp][iter.xcolor].emplace_back(iter);
				 break;
			case 25: 
 				 a[ktemp][iter.pullurl].emplace_back(iter);
				 break;
			case 26: 
 				 a[ktemp][iter.pulltitle].emplace_back(iter);
				 break;
			case 27: 
 				 a[ktemp][iter.pullauthor].emplace_back(iter);
				 break;
			case 28: 
 				 a[ktemp][iter.note].emplace_back(iter);
				 break;
			case 29: 
 				 a[ktemp][iter.itemnote].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);            
            T ktemp;
            
            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 21: 
 				 ktemp=iter.title;
				 break;
			case 22: 
 				 ktemp=iter.introduce;
				 break;
			case 23: 
 				 ktemp=iter.xlogo;
				 break;
			case 24: 
 				 ktemp=iter.xcolor;
				 break;
			case 25: 
 				 ktemp=iter.pullurl;
				 break;
			case 26: 
 				 ktemp=iter.pulltitle;
				 break;
			case 27: 
 				 ktemp=iter.pullauthor;
				 break;
			case 28: 
 				 ktemp=iter.note;
				 break;
			case 29: 
 				 ktemp=iter.itemnote;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.xtaskid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.xpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.adminid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.begindate].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.enddate].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.expectbegindate].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.expectenddate].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.milestone].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.subxpjid].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.depxtaskid].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.referdocid].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.isfinish].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.updatedate].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.finishdate].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.iscore].emplace_back(iter);
				 break;

                }
            }       

    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<U>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;
   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);        
        T ktemp;
        
            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.xtaskid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.xpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.adminid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.begindate].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.enddate].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.expectbegindate].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.expectenddate].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.milestone].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.subxpjid].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.depxtaskid].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.referdocid].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.isfinish].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.updatedate].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.finishdate].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.iscore].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;

   
            unsigned char kpos,vpos;
            kpos=findcolpos(keyname);
            vpos=findcolpos(valname);            
            T ktemp;
            
            for(auto &iter:record)
            {
                
                switch(kpos)
                {

   			case 0: 
 				 ktemp=iter.xtaskid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 7: 
 				 ktemp=iter.expectbegindate;
				 break;
			case 8: 
 				 ktemp=iter.expectenddate;
				 break;
			case 9: 
 				 ktemp=iter.milestone;
				 break;
			case 10: 
 				 ktemp=iter.subxpjid;
				 break;
			case 11: 
 				 ktemp=iter.depxtaskid;
				 break;
			case 12: 
 				 ktemp=iter.referdocid;
				 break;
			case 13: 
 				 ktemp=iter.isfinish;
				 break;
			case 14: 
 				 ktemp=iter.updatedate;
				 break;
			case 15: 
 				 ktemp=iter.finishdate;
				 break;
			case 16: 
 				 ktemp=iter.iscore;
				 break;
			  }

			 switch(vpos){
			case 21: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 22: 
 				 a[ktemp][iter.introduce].emplace_back(iter);
				 break;
			case 23: 
 				 a[ktemp][iter.xlogo].emplace_back(iter);
				 break;
			case 24: 
 				 a[ktemp][iter.xcolor].emplace_back(iter);
				 break;
			case 25: 
 				 a[ktemp][iter.pullurl].emplace_back(iter);
				 break;
			case 26: 
 				 a[ktemp][iter.pulltitle].emplace_back(iter);
				 break;
			case 27: 
 				 a[ktemp][iter.pullauthor].emplace_back(iter);
				 break;
			case 28: 
 				 a[ktemp][iter.note].emplace_back(iter);
				 break;
			case 29: 
 				 a[ktemp][iter.itemnote].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   