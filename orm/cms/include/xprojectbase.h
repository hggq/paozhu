#ifndef ORM_CMS_XPROJECTBASEMATA_H
#define ORM_CMS_XPROJECTBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Fri, 11 Apr 2025 14:33:49 GMT
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

struct xprojectbase
{
    struct meta{
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
 } data;
 std::vector<xprojectbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<xprojectbase::meta>::iterator begin(){     return record.begin(); }
std::vector<xprojectbase::meta>::iterator end(){     return record.end(); }
std::vector<xprojectbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<xprojectbase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,25> col_names={"xpjid","userid","prexpjid","dpid","grouptype","title","adminuserid","regdate","begindate","expiredate","isopen","clientid","totalnum","referdocverion","xtheme","xlogo","introduce","giturl","gitname","gitpwd","xcolor","fupan","totalvalue","expectday","realday"};
static constexpr std::array<unsigned char,25> col_types={3,3,3,3,3,253,3,3,3,3,1,3,3,3,3,253,252,253,252,253,253,252,5,4,4};
static constexpr std::array<unsigned char,25> col_length={0,0,0,0,0,120,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,0};
static constexpr std::array<unsigned char,25> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="xproject";
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
     xprojectbase::meta metatemp;    
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
            for(jj=0;jj<col_names.size();jj++){
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
        xprojectbase::meta metatemp; 
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
			data.xpjid=std::stoul(set_value_name);
		}catch (...) { 
			data.xpjid=0;
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
			data.prexpjid=std::stoul(set_value_name);
		}catch (...) { 
			data.prexpjid=0;
			 }
			break;
		case 3:
		 try{
			data.dpid=std::stoul(set_value_name);
		}catch (...) { 
			data.dpid=0;
			 }
			break;
		case 4:
		 try{
			data.grouptype=std::stoul(set_value_name);
		}catch (...) { 
			data.grouptype=0;
			 }
			break;
		case 5:
		 try{
			data.title.append(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.adminuserid=std::stoul(set_value_name);
		}catch (...) { 
			data.adminuserid=0;
			 }
			break;
		case 7:
		 try{
			data.regdate=std::stoul(set_value_name);
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 8:
		 try{
			data.begindate=std::stoul(set_value_name);
		}catch (...) { 
			data.begindate=0;
			 }
			break;
		case 9:
		 try{
			data.expiredate=std::stoul(set_value_name);
		}catch (...) { 
			data.expiredate=0;
			 }
			break;
		case 10:
		 try{
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 11:
		 try{
			data.clientid=std::stoul(set_value_name);
		}catch (...) { 
			data.clientid=0;
			 }
			break;
		case 12:
		 try{
			data.totalnum=std::stoul(set_value_name);
		}catch (...) { 
			data.totalnum=0;
			 }
			break;
		case 13:
		 try{
			data.referdocverion=std::stoul(set_value_name);
		}catch (...) { 
			data.referdocverion=0;
			 }
			break;
		case 14:
		 try{
			data.xtheme=std::stoul(set_value_name);
		}catch (...) { 
			data.xtheme=0;
			 }
			break;
		case 15:
		 try{
			data.xlogo.append(set_value_name);
		}catch (...) { 
			data.xlogo.clear();
			 }
			break;
		case 16:
		 try{
			data.introduce.append(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 17:
		 try{
			data.giturl.append(set_value_name);
		}catch (...) { 
			data.giturl.clear();
			 }
			break;
		case 18:
		 try{
			data.gitname.append(set_value_name);
		}catch (...) { 
			data.gitname.clear();
			 }
			break;
		case 19:
		 try{
			data.gitpwd.append(set_value_name);
		}catch (...) { 
			data.gitpwd.clear();
			 }
			break;
		case 20:
		 try{
			data.xcolor.append(set_value_name);
		}catch (...) { 
			data.xcolor.clear();
			 }
			break;
		case 21:
		 try{
			data.fupan.append(set_value_name);
		}catch (...) { 
			data.fupan.clear();
			 }
			break;
		case 22:
		 try{
			data.totalvalue=std::stod(set_value_name);
		}catch (...) { 
			data.totalvalue=0.0;
			 }
			break;
		case 23:
		 try{
			data.expectday=std::stof(set_value_name);
		}catch (...) { 
			data.expectday=0.0;
			 }
			break;
		case 24:
		 try{
			data.realday=std::stof(set_value_name);
		}catch (...) { 
			data.realday=0.0;
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
			data.xpjid=set_value_name;
		}catch (...) { 
			data.xpjid=0;
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
			data.prexpjid=set_value_name;
		}catch (...) { 
			data.prexpjid=0;
			 }
			break;
		case 3:
		 try{
			data.dpid=set_value_name;
		}catch (...) { 
			data.dpid=0;
			 }
			break;
		case 4:
		 try{
			data.grouptype=set_value_name;
		}catch (...) { 
			data.grouptype=0;
			 }
			break;
		case 5:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.adminuserid=set_value_name;
		}catch (...) { 
			data.adminuserid=0;
			 }
			break;
		case 7:
		 try{
			data.regdate=set_value_name;
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 8:
		 try{
			data.begindate=set_value_name;
		}catch (...) { 
			data.begindate=0;
			 }
			break;
		case 9:
		 try{
			data.expiredate=set_value_name;
		}catch (...) { 
			data.expiredate=0;
			 }
			break;
		case 10:
		 try{
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 11:
		 try{
			data.clientid=set_value_name;
		}catch (...) { 
			data.clientid=0;
			 }
			break;
		case 12:
		 try{
			data.totalnum=set_value_name;
		}catch (...) { 
			data.totalnum=0;
			 }
			break;
		case 13:
		 try{
			data.referdocverion=set_value_name;
		}catch (...) { 
			data.referdocverion=0;
			 }
			break;
		case 14:
		 try{
			data.xtheme=set_value_name;
		}catch (...) { 
			data.xtheme=0;
			 }
			break;
		case 15:
		 try{
			data.xlogo=std::to_string(set_value_name);
		}catch (...) { 
			data.xlogo.clear();
			 }
			break;
		case 16:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 17:
		 try{
			data.giturl=std::to_string(set_value_name);
		}catch (...) { 
			data.giturl.clear();
			 }
			break;
		case 18:
		 try{
			data.gitname=std::to_string(set_value_name);
		}catch (...) { 
			data.gitname.clear();
			 }
			break;
		case 19:
		 try{
			data.gitpwd=std::to_string(set_value_name);
		}catch (...) { 
			data.gitpwd.clear();
			 }
			break;
		case 20:
		 try{
			data.xcolor=std::to_string(set_value_name);
		}catch (...) { 
			data.xcolor.clear();
			 }
			break;
		case 21:
		 try{
			data.fupan=std::to_string(set_value_name);
		}catch (...) { 
			data.fupan.clear();
			 }
			break;
		case 22:
		 try{
			data.totalvalue=(double)set_value_name;
		}catch (...) { 
			data.totalvalue=0.0;
			 }
			break;
		case 23:
		 try{
			data.expectday=set_value_name;
		}catch (...) { 
			data.expectday=0.0;
			 }
			break;
		case 24:
		 try{
			data.realday=set_value_name;
		}catch (...) { 
			data.realday=0.0;
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
			data.xpjid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xpjid=0;
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
			data.prexpjid=(unsigned int)set_value_name;
		}catch (...) { 
			data.prexpjid=0;
			 }
			break;
		case 3:
		 try{
			data.dpid=(unsigned int)set_value_name;
		}catch (...) { 
			data.dpid=0;
			 }
			break;
		case 4:
		 try{
			data.grouptype=(unsigned int)set_value_name;
		}catch (...) { 
			data.grouptype=0;
			 }
			break;
		case 5:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.adminuserid=(unsigned int)set_value_name;
		}catch (...) { 
			data.adminuserid=0;
			 }
			break;
		case 7:
		 try{
			data.regdate=(unsigned int)set_value_name;
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 8:
		 try{
			data.begindate=(unsigned int)set_value_name;
		}catch (...) { 
			data.begindate=0;
			 }
			break;
		case 9:
		 try{
			data.expiredate=(unsigned int)set_value_name;
		}catch (...) { 
			data.expiredate=0;
			 }
			break;
		case 10:
		 try{
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 11:
		 try{
			data.clientid=(unsigned int)set_value_name;
		}catch (...) { 
			data.clientid=0;
			 }
			break;
		case 12:
		 try{
			data.totalnum=(unsigned int)set_value_name;
		}catch (...) { 
			data.totalnum=0;
			 }
			break;
		case 13:
		 try{
			data.referdocverion=(unsigned int)set_value_name;
		}catch (...) { 
			data.referdocverion=0;
			 }
			break;
		case 14:
		 try{
			data.xtheme=(unsigned int)set_value_name;
		}catch (...) { 
			data.xtheme=0;
			 }
			break;
		case 15:
		 try{
			data.xlogo=std::to_string(set_value_name);
		}catch (...) { 
			data.xlogo.clear();
			 }
			break;
		case 16:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 17:
		 try{
			data.giturl=std::to_string(set_value_name);
		}catch (...) { 
			data.giturl.clear();
			 }
			break;
		case 18:
		 try{
			data.gitname=std::to_string(set_value_name);
		}catch (...) { 
			data.gitname.clear();
			 }
			break;
		case 19:
		 try{
			data.gitpwd=std::to_string(set_value_name);
		}catch (...) { 
			data.gitpwd.clear();
			 }
			break;
		case 20:
		 try{
			data.xcolor=std::to_string(set_value_name);
		}catch (...) { 
			data.xcolor.clear();
			 }
			break;
		case 21:
		 try{
			data.fupan=std::to_string(set_value_name);
		}catch (...) { 
			data.fupan.clear();
			 }
			break;
		case 22:
		 try{
			data.totalvalue=set_value_name;
		}catch (...) { 
			data.totalvalue=0.0;
			 }
			break;
		case 23:
		 try{
			data.expectday=(float)set_value_name;
		}catch (...) { 
			data.expectday=0.0;
			 }
			break;
		case 24:
		 try{
			data.realday=(float)set_value_name;
		}catch (...) { 
			data.realday=0.0;
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

xprojectbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
xprojectbase::meta getData(){
 	 return data; 
} 
std::vector<xprojectbase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="title")
		{
			return data.title;
		}
		 if(key_name=="xlogo")
		{
			return data.xlogo;
		}
		 if(key_name=="introduce")
		{
			return data.introduce;
		}
		 if(key_name=="giturl")
		{
			return data.giturl;
		}
		 if(key_name=="gitname")
		{
			return data.gitname;
		}
		 if(key_name=="gitpwd")
		{
			return data.gitpwd;
		}
		 if(key_name=="xcolor")
		{
			return data.xcolor;
		}
		 if(key_name=="fupan")
		{
			return data.fupan;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="xpjid")
		{
			return data.xpjid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="prexpjid")
		{
			return data.prexpjid;
		}
		 if(key_name=="dpid")
		{
			return data.dpid;
		}
		 if(key_name=="grouptype")
		{
			return data.grouptype;
		}
		 if(key_name=="adminuserid")
		{
			return data.adminuserid;
		}
		 if(key_name=="regdate")
		{
			return data.regdate;
		}
		 if(key_name=="begindate")
		{
			return data.begindate;
		}
		 if(key_name=="expiredate")
		{
			return data.expiredate;
		}
		 if(key_name=="isopen")
		{
			return data.isopen;
		}
		 if(key_name=="clientid")
		{
			return data.clientid;
		}
		 if(key_name=="totalnum")
		{
			return data.totalnum;
		}
		 if(key_name=="referdocverion")
		{
			return data.referdocverion;
		}
		 if(key_name=="xtheme")
		{
			return data.xtheme;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="totalvalue")
		{
			return data.totalvalue;
		}
		 if(key_name=="expectday")
		{
			return data.expectday;
		}
		 if(key_name=="realday")
		{
			return data.realday;
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
 				 a.emplace_back(iter.xpjid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.prexpjid);
				 break;
			case 3: 
 				 a.emplace_back(iter.dpid);
				 break;
			case 4: 
 				 a.emplace_back(iter.grouptype);
				 break;
			case 6: 
 				 a.emplace_back(iter.adminuserid);
				 break;
			case 7: 
 				 a.emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a.emplace_back(iter.begindate);
				 break;
			case 9: 
 				 a.emplace_back(iter.expiredate);
				 break;
			case 10: 
 				 a.emplace_back(iter.isopen);
				 break;
			case 11: 
 				 a.emplace_back(iter.clientid);
				 break;
			case 12: 
 				 a.emplace_back(iter.totalnum);
				 break;
			case 13: 
 				 a.emplace_back(iter.referdocverion);
				 break;
			case 14: 
 				 a.emplace_back(iter.xtheme);
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

			case 22: 
 					 a.emplace_back(iter.totalvalue);
			 break;
			case 23: 
 					 a.emplace_back(iter.expectday);
			 break;
			case 24: 
 					 a.emplace_back(iter.realday);
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
 				 return data.xpjid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.prexpjid;
				 break;
			case 3: 
 				 return data.dpid;
				 break;
			case 4: 
 				 return data.grouptype;
				 break;
			case 6: 
 				 return data.adminuserid;
				 break;
			case 7: 
 				 return data.regdate;
				 break;
			case 8: 
 				 return data.begindate;
				 break;
			case 9: 
 				 return data.expiredate;
				 break;
			case 10: 
 				 return data.isopen;
				 break;
			case 11: 
 				 return data.clientid;
				 break;
			case 12: 
 				 return data.totalnum;
				 break;
			case 13: 
 				 return data.referdocverion;
				 break;
			case 14: 
 				 return data.xtheme;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] xprojectbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.xpjid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.prexpjid;
				 break;
			case 3: 
 				 return iter.dpid;
				 break;
			case 4: 
 				 return iter.grouptype;
				 break;
			case 6: 
 				 return iter.adminuserid;
				 break;
			case 7: 
 				 return iter.regdate;
				 break;
			case 8: 
 				 return iter.begindate;
				 break;
			case 9: 
 				 return iter.expiredate;
				 break;
			case 10: 
 				 return iter.isopen;
				 break;
			case 11: 
 				 return iter.clientid;
				 break;
			case 12: 
 				 return iter.totalnum;
				 break;
			case 13: 
 				 return iter.referdocverion;
				 break;
			case 14: 
 				 return iter.xtheme;
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

    			case 22: 
 				 return data.totalvalue;
				 break;
			case 23: 
 				 return data.expectday;
				 break;
			case 24: 
 				 return data.realday;
				 break;

                    }
                   
    
                    return 0.0;
            }  
    
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
            T getVal([[maybe_unused]] xprojectbase::meta & iter,std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
                switch(kpos)
                {
   			case 22: 
 				 return iter.totalvalue;
				 break;
			case 23: 
 				 return iter.expectday;
				 break;
			case 24: 
 				 return iter.realday;
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

   			case 5: 
 				 return data.title;
				 break;
			case 15: 
 				 return data.xlogo;
				 break;
			case 16: 
 				 return data.introduce;
				 break;
			case 17: 
 				 return data.giturl;
				 break;
			case 18: 
 				 return data.gitname;
				 break;
			case 19: 
 				 return data.gitpwd;
				 break;
			case 20: 
 				 return data.xcolor;
				 break;
			case 21: 
 				 return data.fupan;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] xprojectbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 5: 
 				 return iter.title;
				 break;
			case 15: 
 				 return iter.xlogo;
				 break;
			case 16: 
 				 return iter.introduce;
				 break;
			case 17: 
 				 return iter.giturl;
				 break;
			case 18: 
 				 return iter.gitname;
				 break;
			case 19: 
 				 return iter.gitpwd;
				 break;
			case 20: 
 				 return iter.xcolor;
				 break;
			case 21: 
 				 return iter.fupan;
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

    			case 5: 
 				 a.emplace_back(iter.title);
					 break;
			case 15: 
 				 a.emplace_back(iter.xlogo);
					 break;
			case 16: 
 				 a.emplace_back(iter.introduce);
					 break;
			case 17: 
 				 a.emplace_back(iter.giturl);
					 break;
			case 18: 
 				 a.emplace_back(iter.gitname);
					 break;
			case 19: 
 				 a.emplace_back(iter.gitpwd);
					 break;
			case 20: 
 				 a.emplace_back(iter.xcolor);
					 break;
			case 21: 
 				 a.emplace_back(iter.fupan);
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
 				 a<<std::to_string(iter.xpjid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.prexpjid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.dpid);
				 break;
			case 4: 
 				 a<<std::to_string(iter.grouptype);
				 break;
			case 5: 
 				 if(isyinhao){ a<<jsonaddslash(iter.title); 
				 }else{
				 a<<iter.title;
				 }
				 break;
			case 6: 
 				 a<<std::to_string(iter.adminuserid);
				 break;
			case 7: 
 				 a<<std::to_string(iter.regdate);
				 break;
			case 8: 
 				 a<<std::to_string(iter.begindate);
				 break;
			case 9: 
 				 a<<std::to_string(iter.expiredate);
				 break;
			case 10: 
 				 a<<std::to_string(iter.isopen);
				 break;
			case 11: 
 				 a<<std::to_string(iter.clientid);
				 break;
			case 12: 
 				 a<<std::to_string(iter.totalnum);
				 break;
			case 13: 
 				 a<<std::to_string(iter.referdocverion);
				 break;
			case 14: 
 				 a<<std::to_string(iter.xtheme);
				 break;
			case 15: 
 				 if(isyinhao){ a<<jsonaddslash(iter.xlogo); 
				 }else{
				 a<<iter.xlogo;
				 }
				 break;
			case 16: 
 				 if(isyinhao){ a<<jsonaddslash(iter.introduce); 
				 }else{
				 a<<iter.introduce;
				 }
				 break;
			case 17: 
 				 if(isyinhao){ a<<jsonaddslash(iter.giturl); 
				 }else{
				 a<<iter.giturl;
				 }
				 break;
			case 18: 
 				 if(isyinhao){ a<<jsonaddslash(iter.gitname); 
				 }else{
				 a<<iter.gitname;
				 }
				 break;
			case 19: 
 				 if(isyinhao){ a<<jsonaddslash(iter.gitpwd); 
				 }else{
				 a<<iter.gitpwd;
				 }
				 break;
			case 20: 
 				 if(isyinhao){ a<<jsonaddslash(iter.xcolor); 
				 }else{
				 a<<iter.xcolor;
				 }
				 break;
			case 21: 
 				 if(isyinhao){ a<<jsonaddslash(iter.fupan); 
				 }else{
				 a<<iter.fupan;
				 }
				 break;
			case 22: 
 				 a<<std::to_string(iter.totalvalue);
				 break;
			case 23: 
 				 a<<std::to_string(iter.expectday);
				 break;
			case 24: 
 				 a<<std::to_string(iter.realday);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
				 } 
			switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
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
 
       			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			 } 
		 switch(vpos){
			case 22: 
 				 vtemp=iter.totalvalue;
				 break;
			case 23: 
 				 vtemp=iter.expectday;
				 break;
			case 24: 
 				 vtemp=iter.realday;
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
 	 ktemp=iter.xpjid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.prexpjid;
	 break;
case 3: 
 	 ktemp=iter.dpid;
	 break;
case 4: 
 	 ktemp=iter.grouptype;
	 break;
case 6: 
 	 ktemp=iter.adminuserid;
	 break;
case 7: 
 	 ktemp=iter.regdate;
	 break;
case 8: 
 	 ktemp=iter.begindate;
	 break;
case 9: 
 	 ktemp=iter.expiredate;
	 break;
case 10: 
 	 ktemp=iter.isopen;
	 break;
case 11: 
 	 ktemp=iter.clientid;
	 break;
case 12: 
 	 ktemp=iter.totalnum;
	 break;
case 13: 
 	 ktemp=iter.referdocverion;
	 break;
case 14: 
 	 ktemp=iter.xtheme;
	 break;
	 } 
 		  switch(vpos){
case 22: 
 	 vtemp=iter.totalvalue;
	 break;
case 23: 
 	 vtemp=iter.expectday;
	 break;
case 24: 
 	 vtemp=iter.realday;
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
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
 				 a.emplace(iter.xpjid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.prexpjid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.dpid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.grouptype,iter);
				 break;
			case 6: 
 				 a.emplace(iter.adminuserid,iter);
				 break;
			case 7: 
 				 a.emplace(iter.regdate,iter);
				 break;
			case 8: 
 				 a.emplace(iter.begindate,iter);
				 break;
			case 9: 
 				 a.emplace(iter.expiredate,iter);
				 break;
			case 10: 
 				 a.emplace(iter.isopen,iter);
				 break;
			case 11: 
 				 a.emplace(iter.clientid,iter);
				 break;
			case 12: 
 				 a.emplace(iter.totalnum,iter);
				 break;
			case 13: 
 				 a.emplace(iter.referdocverion,iter);
				 break;
			case 14: 
 				 a.emplace(iter.xtheme,iter);
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

   			case 5: 
 				 a.emplace(iter.title,iter);
			 break;
			case 15: 
 				 a.emplace(iter.xlogo,iter);
			 break;
			case 16: 
 				 a.emplace(iter.introduce,iter);
			 break;
			case 17: 
 				 a.emplace(iter.giturl,iter);
			 break;
			case 18: 
 				 a.emplace(iter.gitname,iter);
			 break;
			case 19: 
 				 a.emplace(iter.gitpwd,iter);
			 break;
			case 20: 
 				 a.emplace(iter.xcolor,iter);
			 break;
			case 21: 
 				 a.emplace(iter.fupan,iter);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
	 		 }
 			switch(vpos){
			case 22: 
 				 vtemp=iter.totalvalue;
				 break;
			case 23: 
 				 vtemp=iter.expectday;
				 break;
			case 24: 
 				 vtemp=iter.realday;
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
 				 ktemp=iter.xpjid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.prexpjid;
			 break;
			case 3: 
 				 ktemp=iter.dpid;
			 break;
			case 4: 
 				 ktemp=iter.grouptype;
			 break;
			case 6: 
 				 ktemp=iter.adminuserid;
			 break;
			case 7: 
 				 ktemp=iter.regdate;
			 break;
			case 8: 
 				 ktemp=iter.begindate;
			 break;
			case 9: 
 				 ktemp=iter.expiredate;
			 break;
			case 10: 
 				 ktemp=iter.isopen;
			 break;
			case 11: 
 				 ktemp=iter.clientid;
			 break;
			case 12: 
 				 ktemp=iter.totalnum;
			 break;
			case 13: 
 				 ktemp=iter.referdocverion;
			 break;
			case 14: 
 				 ktemp=iter.xtheme;
			 break;
			  }
			 switch(vpos){
			case 22: 
 				 vtemp=iter.totalvalue;
				 break;
			case 23: 
 				 vtemp=iter.expectday;
				 break;
			case 24: 
 				 vtemp=iter.realday;
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
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

   case 5: 
 	 ktemp=iter.title;
	 break;
case 15: 
 	 ktemp=iter.xlogo;
	 break;
case 16: 
 	 ktemp=iter.introduce;
	 break;
case 17: 
 	 ktemp=iter.giturl;
	 break;
case 18: 
 	 ktemp=iter.gitname;
	 break;
case 19: 
 	 ktemp=iter.gitpwd;
	 break;
case 20: 
 	 ktemp=iter.xcolor;
	 break;
case 21: 
 	 ktemp=iter.fupan;
	 break;
	  }
 switch(vpos){
case 5: 
 	 vtemp=iter.title;
	 break;
case 15: 
 	 vtemp=iter.xlogo;
	 break;
case 16: 
 	 vtemp=iter.introduce;
	 break;
case 17: 
 	 vtemp=iter.giturl;
	 break;
case 18: 
 	 vtemp=iter.gitname;
	 break;
case 19: 
 	 vtemp=iter.gitpwd;
	 break;
case 20: 
 	 vtemp=iter.xcolor;
	 break;
case 21: 
 	 vtemp=iter.fupan;
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
 	 a.emplace_back(iter.xpjid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.prexpjid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.dpid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.grouptype,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.adminuserid,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.regdate,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.begindate,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.expiredate,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.isopen,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.clientid,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.totalnum,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.referdocverion,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.xtheme,iter);
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

   case 5: 
 	 a.emplace_back(iter.title,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.xlogo,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.introduce,iter);
	 break;
case 17: 
 	 a.emplace_back(iter.giturl,iter);
	 break;
case 18: 
 	 a.emplace_back(iter.gitname,iter);
	 break;
case 19: 
 	 a.emplace_back(iter.gitpwd,iter);
	 break;
case 20: 
 	 a.emplace_back(iter.xcolor,iter);
	 break;
case 21: 
 	 a.emplace_back(iter.fupan,iter);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
				 break;
			  }

			 switch(dpos){
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.totalvalue);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.expectday);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.realday);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.prexpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.grouptype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.adminuserid);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.expiredate);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.clientid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.totalnum);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.referdocverion);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.xtheme);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
				 break;
			 }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.xlogo);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.giturl);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.gitname);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.gitpwd);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.xcolor);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.fupan);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
				 break;
			  }

			 switch(dpos){
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.totalvalue);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.expectday);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.realday);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.prexpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.grouptype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.adminuserid);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.expiredate);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.clientid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.totalnum);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.referdocverion);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.xtheme);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.xlogo);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.giturl);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.gitname);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.gitpwd);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.xcolor);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.fupan);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
				 break;
			  }

			 switch(dpos){
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.totalvalue);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.expectday);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.realday);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.prexpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.grouptype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.adminuserid);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.expiredate);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.clientid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.totalnum);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.referdocverion);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.xtheme);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.xpjid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.prexpjid;
				 break;
			case 3: 
 				 vtemp=iter.dpid;
				 break;
			case 4: 
 				 vtemp=iter.grouptype;
				 break;
			case 6: 
 				 vtemp=iter.adminuserid;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.begindate;
				 break;
			case 9: 
 				 vtemp=iter.expiredate;
				 break;
			case 10: 
 				 vtemp=iter.isopen;
				 break;
			case 11: 
 				 vtemp=iter.clientid;
				 break;
			case 12: 
 				 vtemp=iter.totalnum;
				 break;
			case 13: 
 				 vtemp=iter.referdocverion;
				 break;
			case 14: 
 				 vtemp=iter.xtheme;
				 break;
			 }

			switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.xlogo);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.giturl);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.gitname);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.gitpwd);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.xcolor);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.fupan);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
				 break;
			  }

			 switch(dpos){
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.totalvalue);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.expectday);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.realday);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.prexpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.grouptype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.adminuserid);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.expiredate);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.clientid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.totalnum);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.referdocverion);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.xtheme);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 15: 
 				 vtemp=iter.xlogo;
				 break;
			case 16: 
 				 vtemp=iter.introduce;
				 break;
			case 17: 
 				 vtemp=iter.giturl;
				 break;
			case 18: 
 				 vtemp=iter.gitname;
				 break;
			case 19: 
 				 vtemp=iter.gitpwd;
				 break;
			case 20: 
 				 vtemp=iter.xcolor;
				 break;
			case 21: 
 				 vtemp=iter.fupan;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.xlogo);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.giturl);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.gitname);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.gitpwd);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.xcolor);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.fupan);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.xlogo);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.introduce);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.giturl);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.gitname);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.gitpwd);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.xcolor);
				 break;
			case 21: 
 				 a[ktemp].emplace_back(iter.fupan);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			 }

			 switch(vpos){
			case 22: 
 				 a[ktemp].emplace_back(iter.totalvalue);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.expectday);
				 break;
			case 24: 
 				 a[ktemp].emplace_back(iter.realday);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.xpjid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.prexpjid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.dpid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.grouptype);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.adminuserid);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.begindate);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.expiredate);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.clientid);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.totalnum);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.referdocverion);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.xtheme);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.xlogo);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.introduce);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.giturl);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.gitname);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.gitpwd);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.xcolor);
				 break;
			case 21: 
 				 a[ktemp].emplace_back(iter.fupan);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }

			 switch(vpos){
			case 22: 
 				 a[ktemp].emplace_back(iter.totalvalue);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.expectday);
				 break;
			case 24: 
 				 a[ktemp].emplace_back(iter.realday);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.xpjid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.prexpjid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.dpid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.grouptype);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.adminuserid);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.begindate);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.expiredate);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.clientid);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.totalnum);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.referdocverion);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.xtheme);
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
 				 a[iter.xpjid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.prexpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.dpid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.grouptype].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.adminuserid].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.regdate].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.begindate].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.expiredate].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.isopen].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.clientid].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.totalnum].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.referdocverion].emplace_back(iter);
				 break;
			case 14: 
 				 a[iter.xtheme].emplace_back(iter);
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

   			case 5: 
 				 a[iter.title].emplace_back(iter);
				 break;
			case 15: 
 				 a[iter.xlogo].emplace_back(iter);
				 break;
			case 16: 
 				 a[iter.introduce].emplace_back(iter);
				 break;
			case 17: 
 				 a[iter.giturl].emplace_back(iter);
				 break;
			case 18: 
 				 a[iter.gitname].emplace_back(iter);
				 break;
			case 19: 
 				 a[iter.gitpwd].emplace_back(iter);
				 break;
			case 20: 
 				 a[iter.xcolor].emplace_back(iter);
				 break;
			case 21: 
 				 a[iter.fupan].emplace_back(iter);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.xlogo].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.introduce].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.giturl].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.gitname].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.gitpwd].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.xcolor].emplace_back(iter);
				 break;
			case 21: 
 				 a[ktemp][iter.fupan].emplace_back(iter);
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

   			case 5: 
 				 ktemp=iter.title;
				 break;
			case 15: 
 				 ktemp=iter.xlogo;
				 break;
			case 16: 
 				 ktemp=iter.introduce;
				 break;
			case 17: 
 				 ktemp=iter.giturl;
				 break;
			case 18: 
 				 ktemp=iter.gitname;
				 break;
			case 19: 
 				 ktemp=iter.gitpwd;
				 break;
			case 20: 
 				 ktemp=iter.xcolor;
				 break;
			case 21: 
 				 ktemp=iter.fupan;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.xpjid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.prexpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.dpid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.grouptype].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.adminuserid].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.regdate].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.begindate].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.expiredate].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.clientid].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.totalnum].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.referdocverion].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.xtheme].emplace_back(iter);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.xpjid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.prexpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.dpid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.grouptype].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.adminuserid].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.regdate].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.begindate].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.expiredate].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.clientid].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.totalnum].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.referdocverion].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.xtheme].emplace_back(iter);
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
 				 ktemp=iter.xpjid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.prexpjid;
				 break;
			case 3: 
 				 ktemp=iter.dpid;
				 break;
			case 4: 
 				 ktemp=iter.grouptype;
				 break;
			case 6: 
 				 ktemp=iter.adminuserid;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.begindate;
				 break;
			case 9: 
 				 ktemp=iter.expiredate;
				 break;
			case 10: 
 				 ktemp=iter.isopen;
				 break;
			case 11: 
 				 ktemp=iter.clientid;
				 break;
			case 12: 
 				 ktemp=iter.totalnum;
				 break;
			case 13: 
 				 ktemp=iter.referdocverion;
				 break;
			case 14: 
 				 ktemp=iter.xtheme;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.xlogo].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.introduce].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.giturl].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.gitname].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.gitpwd].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.xcolor].emplace_back(iter);
				 break;
			case 21: 
 				 a[ktemp][iter.fupan].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   