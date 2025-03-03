#ifndef ORM_CMS_SYSPERMSBASEMATA_H
#define ORM_CMS_SYSPERMSBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Mon, 03 Mar 2025 03:13:23 GMT
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

struct syspermsbase
{
    struct meta{
     unsigned  int  permsid = 0; ///**/
 unsigned  int  parentid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 std::string  urlpath = ""; ///**/
 std::string  urlname = ""; ///**/
 std::string  name = ""; ///**/
 unsigned  char  status = 0; ///**/
 unsigned  int  permscode = 0; ///**/
 unsigned  char  readonly = 0; ///**/
 unsigned  int  created_time = 0; ///**/
 unsigned  int  updated_time = 0; ///**/
 unsigned  int  created_user = 0; ///**/
 unsigned  int  updated_user = 0; ///**/
 } data;
  
        struct meta_tree{
         unsigned  int  permsid = 0; ///**/
 unsigned  int  parentid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 std::string  urlpath = ""; ///**/
 std::string  urlname = ""; ///**/
 std::string  name = ""; ///**/
 unsigned  char  status = 0; ///**/
 unsigned  int  permscode = 0; ///**/
 unsigned  char  readonly = 0; ///**/
 unsigned  int  created_time = 0; ///**/
 unsigned  int  updated_time = 0; ///**/
 unsigned  int  created_user = 0; ///**/
 unsigned  int  updated_user = 0; ///**/

	std::vector<meta_tree> children;
 };
 std::vector<syspermsbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<syspermsbase::meta>::iterator begin(){     return record.begin(); }
std::vector<syspermsbase::meta>::iterator end(){     return record.end(); }
std::vector<syspermsbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<syspermsbase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,13> col_names={"permsid","parentid","userid","urlpath","urlname","name","status","permscode","readonly","created_time","updated_time","created_user","updated_user"};
static constexpr std::array<unsigned char,13> col_types={3,3,3,253,253,253,1,3,1,3,3,3,3};
static constexpr std::array<unsigned char,13> col_length={0,0,0,120,60,60,0,0,0,0,0,0,0};
static constexpr std::array<unsigned char,13> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="sysperms";
static constexpr std::string_view modelname="Sysperms";

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


         case 'c':
 switch(coln.size()){  
case 12:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 9; }
 if(colpospppc=='r'){ return 11; }
   	 break;
 }
 break;
case 'n':
   	 return 5;
break;
case 'p':
 switch(coln.size()){  
case 7:
   	 return 0;
break;
case 8:
   	 return 1;
break;
case 9:
   	 return 7;
break;
 }
 break;
case 'r':
   	 return 8;
break;
case 's':
   	 return 6;
break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 2;
break;
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 4; }
 if(colpospppc=='h'){ return 3; }
   	 break;
case 12:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 10; }
 if(colpospppc=='r'){ return 12; }
   	 break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "permsid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     syspermsbase::meta metatemp;    
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

        if(data.permsid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.permsid);
}
if(data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.parentid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
tempsql<<",'"<<stringaddslash(data.urlpath)<<"'";
tempsql<<",'"<<stringaddslash(data.urlname)<<"'";
tempsql<<",'"<<stringaddslash(data.name)<<"'";
if(data.status==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.status);
}
if(data.permscode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.permscode);
}
if(data.readonly==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.readonly);
}
if(data.created_time==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.created_time);
}
if(data.updated_time==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.updated_time);
}
if(data.created_user==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.created_user);
}
if(data.updated_user==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.updated_user);
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

        if(insert_data.permsid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.permsid);
}
if(insert_data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.parentid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
tempsql<<",'"<<stringaddslash(insert_data.urlpath)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.urlname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
if(insert_data.status==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.status);
}
if(insert_data.permscode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.permscode);
}
if(insert_data.readonly==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.readonly);
}
if(insert_data.created_time==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.created_time);
}
if(insert_data.updated_time==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.updated_time);
}
if(insert_data.created_user==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.created_user);
}
if(insert_data.updated_user==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.updated_user);
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


            	if(insert_data[i].permsid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].permsid);
	}
	if(insert_data[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].parentid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].urlpath)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].urlname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
	if(insert_data[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].status);
	}
	if(insert_data[i].permscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].permscode);
	}
	if(insert_data[i].readonly==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].readonly);
	}
	if(insert_data[i].created_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].created_time);
	}
	if(insert_data[i].updated_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].updated_time);
	}
	if(insert_data[i].created_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].created_user);
	}
	if(insert_data[i].updated_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].updated_user);
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

        if(data.permsid==0){
	tempsql<<"`permsid`=0";
 }else{ 
	tempsql<<"`permsid`="<<std::to_string(data.permsid);
}
if(data.parentid==0){
	tempsql<<",`parentid`=0";
 }else{ 
	tempsql<<",`parentid`="<<std::to_string(data.parentid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
tempsql<<",`urlpath`='"<<stringaddslash(data.urlpath)<<"'";
tempsql<<",`urlname`='"<<stringaddslash(data.urlname)<<"'";
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
if(data.status==0){
	tempsql<<",`status`=0";
 }else{ 
	tempsql<<",`status`="<<std::to_string(data.status);
}
if(data.permscode==0){
	tempsql<<",`permscode`=0";
 }else{ 
	tempsql<<",`permscode`="<<std::to_string(data.permscode);
}
if(data.readonly==0){
	tempsql<<",`readonly`=0";
 }else{ 
	tempsql<<",`readonly`="<<std::to_string(data.readonly);
}
if(data.created_time==0){
	tempsql<<",`created_time`=0";
 }else{ 
	tempsql<<",`created_time`="<<std::to_string(data.created_time);
}
if(data.updated_time==0){
	tempsql<<",`updated_time`=0";
 }else{ 
	tempsql<<",`updated_time`="<<std::to_string(data.updated_time);
}
if(data.created_user==0){
	tempsql<<",`created_user`=0";
 }else{ 
	tempsql<<",`created_user`="<<std::to_string(data.created_user);
}
if(data.updated_user==0){
	tempsql<<",`updated_user`=0";
 }else{ 
	tempsql<<",`updated_user`="<<std::to_string(data.updated_user);
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
if(data.permsid==0){
	tempsql<<"`permsid`=0";
 }else{ 
	tempsql<<"`permsid`="<<std::to_string(data.permsid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.parentid==0){
	tempsql<<"`parentid`=0";
 }else{ 
	tempsql<<"`parentid`="<<std::to_string(data.parentid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"`userid`=0";
 }else{ 
	tempsql<<"`userid`="<<std::to_string(data.userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`urlpath`='"<<stringaddslash(data.urlpath)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`urlname`='"<<stringaddslash(data.urlname)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.status==0){
	tempsql<<"`status`=0";
 }else{ 
	tempsql<<"`status`="<<std::to_string(data.status);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.permscode==0){
	tempsql<<"`permscode`=0";
 }else{ 
	tempsql<<"`permscode`="<<std::to_string(data.permscode);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.readonly==0){
	tempsql<<"`readonly`=0";
 }else{ 
	tempsql<<"`readonly`="<<std::to_string(data.readonly);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.created_time==0){
	tempsql<<"`created_time`=0";
 }else{ 
	tempsql<<"`created_time`="<<std::to_string(data.created_time);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.updated_time==0){
	tempsql<<"`updated_time`=0";
 }else{ 
	tempsql<<"`updated_time`="<<std::to_string(data.updated_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.created_user==0){
	tempsql<<"`created_user`=0";
 }else{ 
	tempsql<<"`created_user`="<<std::to_string(data.created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.updated_user==0){
	tempsql<<"`updated_user`=0";
 }else{ 
	tempsql<<"`updated_user`="<<std::to_string(data.updated_user);
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
            	if(record[i].permsid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].permsid);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	tempsql<<",'"<<stringaddslash(record[i].urlpath)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].urlname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].status);
	}
	if(record[i].permscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].permscode);
	}
	if(record[i].readonly==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].readonly);
	}
	if(record[i].created_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_time);
	}
	if(record[i].updated_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updated_time);
	}
	if(record[i].created_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_user);
	}
	if(record[i].updated_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updated_user);
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
            	if(record[i].permsid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].permsid);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	tempsql<<",'"<<stringaddslash(record[i].urlpath)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].urlname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].status);
	}
	if(record[i].permscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].permscode);
	}
	if(record[i].readonly==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].readonly);
	}
	if(record[i].created_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_time);
	}
	if(record[i].updated_time==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updated_time);
	}
	if(record[i].created_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].created_user);
	}
	if(record[i].updated_user==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].updated_user);
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
if(data.permsid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.permsid));
}
 break;
 case 1:
if(data.parentid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.parentid));
}
 break;
 case 2:
if(data.userid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.userid));
}
 break;
 case 3:
	temparray.push_back(data.urlpath);
 break;
 case 4:
	temparray.push_back(data.urlname);
 break;
 case 5:
	temparray.push_back(data.name);
 break;
 case 6:
if(data.status==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.status));
}
 break;
 case 7:
if(data.permscode==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.permscode));
}
 break;
 case 8:
if(data.readonly==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.readonly));
}
 break;
 case 9:
if(data.created_time==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.created_time));
}
 break;
 case 10:
if(data.updated_time==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.updated_time));
}
 break;
 case 11:
if(data.created_user==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.created_user));
}
 break;
 case 12:
if(data.updated_user==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.updated_user));
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
if(data.permsid==0){
	tempsql.insert({"permsid","0"});
 }else{ 
	tempsql.insert({"permsid",std::to_string(data.permsid)});
}
 break;
 case 1:
if(data.parentid==0){
	tempsql.insert({"parentid","0"});
 }else{ 
	tempsql.insert({"parentid",std::to_string(data.parentid)});
}
 break;
 case 2:
if(data.userid==0){
	tempsql.insert({"userid","0"});
 }else{ 
	tempsql.insert({"userid",std::to_string(data.userid)});
}
 break;
 case 3:
	tempsql.insert({"urlpath",data.urlpath});
 break;
 case 4:
	tempsql.insert({"urlname",data.urlname});
 break;
 case 5:
	tempsql.insert({"name",data.name});
 break;
 case 6:
if(data.status==0){
	tempsql.insert({"status","0"});
 }else{ 
	tempsql.insert({"status",std::to_string(data.status)});
}
 break;
 case 7:
if(data.permscode==0){
	tempsql.insert({"permscode","0"});
 }else{ 
	tempsql.insert({"permscode",std::to_string(data.permscode)});
}
 break;
 case 8:
if(data.readonly==0){
	tempsql.insert({"readonly","0"});
 }else{ 
	tempsql.insert({"readonly",std::to_string(data.readonly)});
}
 break;
 case 9:
if(data.created_time==0){
	tempsql.insert({"created_time","0"});
 }else{ 
	tempsql.insert({"created_time",std::to_string(data.created_time)});
}
 break;
 case 10:
if(data.updated_time==0){
	tempsql.insert({"updated_time","0"});
 }else{ 
	tempsql.insert({"updated_time",std::to_string(data.updated_time)});
}
 break;
 case 11:
if(data.created_user==0){
	tempsql.insert({"created_user","0"});
 }else{ 
	tempsql.insert({"created_user",std::to_string(data.created_user)});
}
 break;
 case 12:
if(data.updated_user==0){
	tempsql.insert({"updated_user","0"});
 }else{ 
	tempsql.insert({"updated_user",std::to_string(data.updated_user)});
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
if(data.permsid==0){
	tempsql<<"\"permsid\":0";
 }else{ 
	tempsql<<"\"permsid\":"<<std::to_string(data.permsid);
}
if(data.parentid==0){
	tempsql<<",\"parentid\":0";
 }else{ 
	tempsql<<",\"parentid\":"<<std::to_string(data.parentid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
tempsql<<",\"urlpath\":\""<<http::utf8_to_jsonstring(data.urlpath);
tempsql<<"\"";
tempsql<<",\"urlname\":\""<<http::utf8_to_jsonstring(data.urlname);
tempsql<<"\"";
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
if(data.status==0){
	tempsql<<",\"status\":0";
 }else{ 
	tempsql<<",\"status\":"<<std::to_string(data.status);
}
if(data.permscode==0){
	tempsql<<",\"permscode\":0";
 }else{ 
	tempsql<<",\"permscode\":"<<std::to_string(data.permscode);
}
if(data.readonly==0){
	tempsql<<",\"readonly\":0";
 }else{ 
	tempsql<<",\"readonly\":"<<std::to_string(data.readonly);
}
if(data.created_time==0){
	tempsql<<",\"created_time\":0";
 }else{ 
	tempsql<<",\"created_time\":"<<std::to_string(data.created_time);
}
if(data.updated_time==0){
	tempsql<<",\"updated_time\":0";
 }else{ 
	tempsql<<",\"updated_time\":"<<std::to_string(data.updated_time);
}
if(data.created_user==0){
	tempsql<<",\"created_user\":0";
 }else{ 
	tempsql<<",\"created_user\":"<<std::to_string(data.created_user);
}
if(data.updated_user==0){
	tempsql<<",\"updated_user\":0";
 }else{ 
	tempsql<<",\"updated_user\":"<<std::to_string(data.updated_user);
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
if(data.permsid==0){
	tempsql<<"\"permsid\":0";
 }else{ 
	tempsql<<"\"permsid\":"<<std::to_string(data.permsid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(data.parentid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(data.userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(data.urlpath)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlname\":\""<<http::utf8_to_jsonstring(data.urlname)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(data.status);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.permscode==0){
	tempsql<<"\"permscode\":0";
 }else{ 
	tempsql<<"\"permscode\":"<<std::to_string(data.permscode);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.readonly==0){
	tempsql<<"\"readonly\":0";
 }else{ 
	tempsql<<"\"readonly\":"<<std::to_string(data.readonly);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(data.created_time);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(data.updated_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(data.created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(data.updated_user);
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
        syspermsbase::meta metatemp; 
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
			data.permsid=std::stoul(set_value_name);
		}catch (...) { 
			data.permsid=0;
			 }
			break;
		case 1:
		 try{
			data.parentid=std::stoul(set_value_name);
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 2:
		 try{
			data.userid=std::stoul(set_value_name);
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 3:
		 try{
			data.urlpath.append(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
			 }
			break;
		case 4:
		 try{
			data.urlname.append(set_value_name);
		}catch (...) { 
			data.urlname.clear();
			 }
			break;
		case 5:
		 try{
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 6:
		 try{
			data.status=std::stoi(set_value_name);
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 7:
		 try{
			data.permscode=std::stoul(set_value_name);
		}catch (...) { 
			data.permscode=0;
			 }
			break;
		case 8:
		 try{
			data.readonly=std::stoi(set_value_name);
		}catch (...) { 
			data.readonly=0;
			 }
			break;
		case 9:
		 try{
			data.created_time=std::stoul(set_value_name);
		}catch (...) { 
			data.created_time=0;
			 }
			break;
		case 10:
		 try{
			data.updated_time=std::stoul(set_value_name);
		}catch (...) { 
			data.updated_time=0;
			 }
			break;
		case 11:
		 try{
			data.created_user=std::stoul(set_value_name);
		}catch (...) { 
			data.created_user=0;
			 }
			break;
		case 12:
		 try{
			data.updated_user=std::stoul(set_value_name);
		}catch (...) { 
			data.updated_user=0;
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
			data.permsid=set_value_name;
		}catch (...) { 
			data.permsid=0;
			 }
			break;
		case 1:
		 try{
			data.parentid=set_value_name;
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 2:
		 try{
			data.userid=set_value_name;
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 3:
		 try{
			data.urlpath=std::to_string(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
			 }
			break;
		case 4:
		 try{
			data.urlname=std::to_string(set_value_name);
		}catch (...) { 
			data.urlname.clear();
			 }
			break;
		case 5:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 6:
		 try{
			data.status=set_value_name;
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 7:
		 try{
			data.permscode=set_value_name;
		}catch (...) { 
			data.permscode=0;
			 }
			break;
		case 8:
		 try{
			data.readonly=set_value_name;
		}catch (...) { 
			data.readonly=0;
			 }
			break;
		case 9:
		 try{
			data.created_time=set_value_name;
		}catch (...) { 
			data.created_time=0;
			 }
			break;
		case 10:
		 try{
			data.updated_time=set_value_name;
		}catch (...) { 
			data.updated_time=0;
			 }
			break;
		case 11:
		 try{
			data.created_user=set_value_name;
		}catch (...) { 
			data.created_user=0;
			 }
			break;
		case 12:
		 try{
			data.updated_user=set_value_name;
		}catch (...) { 
			data.updated_user=0;
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
			data.permsid=(unsigned int)set_value_name;
		}catch (...) { 
			data.permsid=0;
			 }
			break;
		case 1:
		 try{
			data.parentid=(unsigned int)set_value_name;
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 2:
		 try{
			data.userid=(unsigned int)set_value_name;
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 3:
		 try{
			data.urlpath=std::to_string(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
			 }
			break;
		case 4:
		 try{
			data.urlname=std::to_string(set_value_name);
		}catch (...) { 
			data.urlname.clear();
			 }
			break;
		case 5:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 6:
		 try{
			data.status=(int)set_value_name;
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 7:
		 try{
			data.permscode=(unsigned int)set_value_name;
		}catch (...) { 
			data.permscode=0;
			 }
			break;
		case 8:
		 try{
			data.readonly=(int)set_value_name;
		}catch (...) { 
			data.readonly=0;
			 }
			break;
		case 9:
		 try{
			data.created_time=(unsigned int)set_value_name;
		}catch (...) { 
			data.created_time=0;
			 }
			break;
		case 10:
		 try{
			data.updated_time=(unsigned int)set_value_name;
		}catch (...) { 
			data.updated_time=0;
			 }
			break;
		case 11:
		 try{
			data.created_user=(unsigned int)set_value_name;
		}catch (...) { 
			data.created_user=0;
			 }
			break;
		case 12:
		 try{
			data.updated_user=(unsigned int)set_value_name;
		}catch (...) { 
			data.updated_user=0;
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
if(record[n].permsid==0){
	tempsql<<"\"permsid\":0";
 }else{ 
	tempsql<<"\"permsid\":"<<std::to_string(record[n].permsid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(record[n].urlpath)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlname\":\""<<http::utf8_to_jsonstring(record[n].urlname)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(record[n].status);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].permscode==0){
	tempsql<<"\"permscode\":0";
 }else{ 
	tempsql<<"\"permscode\":"<<std::to_string(record[n].permscode);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].readonly==0){
	tempsql<<"\"readonly\":0";
 }else{ 
	tempsql<<"\"readonly\":"<<std::to_string(record[n].readonly);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(record[n].created_time);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(record[n].updated_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(record[n].created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(record[n].updated_user);
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
if(record[n].permsid==0){
	tempsql<<"\"permsid\":0";
 }else{ 
	tempsql<<"\"permsid\":"<<std::to_string(record[n].permsid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(record[n].urlpath)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlname\":\""<<http::utf8_to_jsonstring(record[n].urlname)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(record[n].status);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].permscode==0){
	tempsql<<"\"permscode\":0";
 }else{ 
	tempsql<<"\"permscode\":"<<std::to_string(record[n].permscode);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].readonly==0){
	tempsql<<"\"readonly\":0";
 }else{ 
	tempsql<<"\"readonly\":"<<std::to_string(record[n].readonly);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(record[n].created_time);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(record[n].updated_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(record[n].created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(record[n].updated_user);
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
   long long getPK(){  return data.permsid; } 
 void setPK(long long val){  data.permsid=val;} 
 unsigned  int  getPermsid(){  return data.permsid; } 
 void setPermsid( unsigned  int  val){  data.permsid=val;} 

 unsigned  int  getParentid(){  return data.parentid; } 
 void setParentid( unsigned  int  val){  data.parentid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 std::string  getUrlpath(){  return data.urlpath; } 
 std::string & getRefUrlpath(){  return std::ref(data.urlpath); } 
 void setUrlpath( std::string  &val){  data.urlpath=val;} 
 void setUrlpath(std::string_view val){  data.urlpath=val;} 

 std::string  getUrlname(){  return data.urlname; } 
 std::string & getRefUrlname(){  return std::ref(data.urlname); } 
 void setUrlname( std::string  &val){  data.urlname=val;} 
 void setUrlname(std::string_view val){  data.urlname=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 unsigned  char  getStatus(){  return data.status; } 
 void setStatus( unsigned  char  val){  data.status=val;} 

 unsigned  int  getPermscode(){  return data.permscode; } 
 void setPermscode( unsigned  int  val){  data.permscode=val;} 

 unsigned  char  getReadonly(){  return data.readonly; } 
 void setReadonly( unsigned  char  val){  data.readonly=val;} 

 unsigned  int  getCreatedTime(){  return data.created_time; } 
 void setCreatedTime( unsigned  int  val){  data.created_time=val;} 

 unsigned  int  getUpdatedTime(){  return data.updated_time; } 
 void setUpdatedTime( unsigned  int  val){  data.updated_time=val;} 

 unsigned  int  getCreatedUser(){  return data.created_user; } 
 void setCreatedUser( unsigned  int  val){  data.created_user=val;} 

 unsigned  int  getUpdatedUser(){  return data.updated_user; } 
 void setUpdatedUser( unsigned  int  val){  data.updated_user=val;} 

syspermsbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
syspermsbase::meta getData(){
 	 return data; 
} 
std::vector<syspermsbase::meta> getRecord(){
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
if(tree_data[n].permsid==0){
	tempsql<<"\"permsid\":0";
 }else{ 
	tempsql<<"\"permsid\":"<<std::to_string(tree_data[n].permsid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(tree_data[n].userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(tree_data[n].urlpath)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlname\":\""<<http::utf8_to_jsonstring(tree_data[n].urlname)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(tree_data[n].status);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].permscode==0){
	tempsql<<"\"permscode\":0";
 }else{ 
	tempsql<<"\"permscode\":"<<std::to_string(tree_data[n].permscode);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].readonly==0){
	tempsql<<"\"readonly\":0";
 }else{ 
	tempsql<<"\"readonly\":"<<std::to_string(tree_data[n].readonly);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(tree_data[n].created_time);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(tree_data[n].updated_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(tree_data[n].created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(tree_data[n].updated_user);
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
if(tree_data[n].permsid==0){
	tempsql<<"\"permsid\":0";
 }else{ 
	tempsql<<"\"permsid\":"<<std::to_string(tree_data[n].permsid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(tree_data[n].userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(tree_data[n].urlpath)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlname\":\""<<http::utf8_to_jsonstring(tree_data[n].urlname)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(tree_data[n].status);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].permscode==0){
	tempsql<<"\"permscode\":0";
 }else{ 
	tempsql<<"\"permscode\":"<<std::to_string(tree_data[n].permscode);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].readonly==0){
	tempsql<<"\"readonly\":0";
 }else{ 
	tempsql<<"\"readonly\":"<<std::to_string(tree_data[n].readonly);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_time==0){
	tempsql<<"\"created_time\":0";
 }else{ 
	tempsql<<"\"created_time\":"<<std::to_string(tree_data[n].created_time);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updated_time==0){
	tempsql<<"\"updated_time\":0";
 }else{ 
	tempsql<<"\"updated_time\":"<<std::to_string(tree_data[n].updated_time);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].created_user==0){
	tempsql<<"\"created_user\":0";
 }else{ 
	tempsql<<"\"created_user\":"<<std::to_string(tree_data[n].created_user);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].updated_user==0){
	tempsql<<"\"updated_user\":0";
 }else{ 
	tempsql<<"\"updated_user\":"<<std::to_string(tree_data[n].updated_user);
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
   

    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="urlpath")
		{
			return data.urlpath;
		}
		 if(key_name=="urlname")
		{
			return data.urlname;
		}
		 if(key_name=="name")
		{
			return data.name;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="permsid")
		{
			return data.permsid;
		}
		 if(key_name=="parentid")
		{
			return data.parentid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="status")
		{
			return data.status;
		}
		 if(key_name=="permscode")
		{
			return data.permscode;
		}
		 if(key_name=="readonly")
		{
			return data.readonly;
		}
		 if(key_name=="created_time")
		{
			return data.created_time;
		}
		 if(key_name=="updated_time")
		{
			return data.updated_time;
		}
		 if(key_name=="created_user")
		{
			return data.created_user;
		}
		 if(key_name=="updated_user")
		{
			return data.updated_user;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
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
 				 a.emplace_back(iter.permsid);
				 break;
			case 1: 
 				 a.emplace_back(iter.parentid);
				 break;
			case 2: 
 				 a.emplace_back(iter.userid);
				 break;
			case 6: 
 				 a.emplace_back(iter.status);
				 break;
			case 7: 
 				 a.emplace_back(iter.permscode);
				 break;
			case 8: 
 				 a.emplace_back(iter.readonly);
				 break;
			case 9: 
 				 a.emplace_back(iter.created_time);
				 break;
			case 10: 
 				 a.emplace_back(iter.updated_time);
				 break;
			case 11: 
 				 a.emplace_back(iter.created_user);
				 break;
			case 12: 
 				 a.emplace_back(iter.updated_user);
				 break;

                    }
                }
    
                return a;
            }
    
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >    
			std::vector<T> getCol([[maybe_unused]] std::string keyname)
			{
				std::vector<T> a;
				

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
 				 return data.permsid;
				 break;
			case 1: 
 				 return data.parentid;
				 break;
			case 2: 
 				 return data.userid;
				 break;
			case 6: 
 				 return data.status;
				 break;
			case 7: 
 				 return data.permscode;
				 break;
			case 8: 
 				 return data.readonly;
				 break;
			case 9: 
 				 return data.created_time;
				 break;
			case 10: 
 				 return data.updated_time;
				 break;
			case 11: 
 				 return data.created_user;
				 break;
			case 12: 
 				 return data.updated_user;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] syspermsbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.permsid;
				 break;
			case 1: 
 				 return iter.parentid;
				 break;
			case 2: 
 				 return iter.userid;
				 break;
			case 6: 
 				 return iter.status;
				 break;
			case 7: 
 				 return iter.permscode;
				 break;
			case 8: 
 				 return iter.readonly;
				 break;
			case 9: 
 				 return iter.created_time;
				 break;
			case 10: 
 				 return iter.updated_time;
				 break;
			case 11: 
 				 return iter.created_user;
				 break;
			case 12: 
 				 return iter.updated_user;
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

    
                    }
                   
    
                    return 0.0;
            }  
    
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
            T getVal([[maybe_unused]] syspermsbase::meta & iter,std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
                switch(kpos)
                {
   
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

   			case 3: 
 				 return data.urlpath;
				 break;
			case 4: 
 				 return data.urlname;
				 break;
			case 5: 
 				 return data.name;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] syspermsbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 3: 
 				 return iter.urlpath;
				 break;
			case 4: 
 				 return iter.urlname;
				 break;
			case 5: 
 				 return iter.name;
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

    			case 3: 
 				 a.emplace_back(iter.urlpath);
					 break;
			case 4: 
 				 a.emplace_back(iter.urlname);
					 break;
			case 5: 
 				 a.emplace_back(iter.name);
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
 				 a<<std::to_string(iter.permsid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.parentid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 3: 
 				 if(isyinhao){ a<<jsonaddslash(iter.urlpath); 
				 }else{
				 a<<iter.urlpath;
				 }
				 break;
			case 4: 
 				 if(isyinhao){ a<<jsonaddslash(iter.urlname); 
				 }else{
				 a<<iter.urlname;
				 }
				 break;
			case 5: 
 				 if(isyinhao){ a<<jsonaddslash(iter.name); 
				 }else{
				 a<<iter.name;
				 }
				 break;
			case 6: 
 				 a<<std::to_string(iter.status);
				 break;
			case 7: 
 				 a<<std::to_string(iter.permscode);
				 break;
			case 8: 
 				 a<<std::to_string(iter.readonly);
				 break;
			case 9: 
 				 a<<std::to_string(iter.created_time);
				 break;
			case 10: 
 				 a<<std::to_string(iter.updated_time);
				 break;
			case 11: 
 				 a<<std::to_string(iter.created_user);
				 break;
			case 12: 
 				 a<<std::to_string(iter.updated_user);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
				 } 
			switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
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
 
       			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			 } 
		 switch(vpos){

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
 	 ktemp=iter.permsid;
	 break;
case 1: 
 	 ktemp=iter.parentid;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 6: 
 	 ktemp=iter.status;
	 break;
case 7: 
 	 ktemp=iter.permscode;
	 break;
case 8: 
 	 ktemp=iter.readonly;
	 break;
case 9: 
 	 ktemp=iter.created_time;
	 break;
case 10: 
 	 ktemp=iter.updated_time;
	 break;
case 11: 
 	 ktemp=iter.created_user;
	 break;
case 12: 
 	 ktemp=iter.updated_user;
	 break;
	 } 
 		  switch(vpos){

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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }
 			switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
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
 				 a.emplace(iter.permsid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.parentid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 6: 
 				 a.emplace(iter.status,iter);
				 break;
			case 7: 
 				 a.emplace(iter.permscode,iter);
				 break;
			case 8: 
 				 a.emplace(iter.readonly,iter);
				 break;
			case 9: 
 				 a.emplace(iter.created_time,iter);
				 break;
			case 10: 
 				 a.emplace(iter.updated_time,iter);
				 break;
			case 11: 
 				 a.emplace(iter.created_user,iter);
				 break;
			case 12: 
 				 a.emplace(iter.updated_user,iter);
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

   			case 3: 
 				 a.emplace(iter.urlpath,iter);
			 break;
			case 4: 
 				 a.emplace(iter.urlname,iter);
			 break;
			case 5: 
 				 a.emplace(iter.name,iter);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
	 		 }
 			switch(vpos){

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
 				 ktemp=iter.permsid;
			 break;
			case 1: 
 				 ktemp=iter.parentid;
			 break;
			case 2: 
 				 ktemp=iter.userid;
			 break;
			case 6: 
 				 ktemp=iter.status;
			 break;
			case 7: 
 				 ktemp=iter.permscode;
			 break;
			case 8: 
 				 ktemp=iter.readonly;
			 break;
			case 9: 
 				 ktemp=iter.created_time;
			 break;
			case 10: 
 				 ktemp=iter.updated_time;
			 break;
			case 11: 
 				 ktemp=iter.created_user;
			 break;
			case 12: 
 				 ktemp=iter.updated_user;
			 break;
			  }
			 switch(vpos){

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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }
 			switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
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

   case 3: 
 	 ktemp=iter.urlpath;
	 break;
case 4: 
 	 ktemp=iter.urlname;
	 break;
case 5: 
 	 ktemp=iter.name;
	 break;
	  }
 switch(vpos){
case 3: 
 	 vtemp=iter.urlpath;
	 break;
case 4: 
 	 vtemp=iter.urlname;
	 break;
case 5: 
 	 vtemp=iter.name;
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
 	 a.emplace_back(iter.permsid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.parentid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.status,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.permscode,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.readonly,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.created_time,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.updated_time,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.created_user,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.updated_user,iter);
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

   case 3: 
 	 a.emplace_back(iter.urlpath,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.urlname,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.name,iter);
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
				 break;
			  }

			 switch(dpos){

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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.permsid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.status);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.permscode);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.readonly);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.created_time);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.updated_time);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.created_user);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.updated_user);
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
				 break;
			 }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.urlname);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
				 break;
			  }

			 switch(dpos){

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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.permsid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.status);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.permscode);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.readonly);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.created_time);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.updated_time);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.created_user);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.updated_user);
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
				 break;
			  }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.urlname);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
				 break;
			  }

			 switch(dpos){

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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.permsid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.status);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.permscode);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.readonly);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.created_time);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.updated_time);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.created_user);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.updated_user);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.permsid;
				 break;
			case 1: 
 				 vtemp=iter.parentid;
				 break;
			case 2: 
 				 vtemp=iter.userid;
				 break;
			case 6: 
 				 vtemp=iter.status;
				 break;
			case 7: 
 				 vtemp=iter.permscode;
				 break;
			case 8: 
 				 vtemp=iter.readonly;
				 break;
			case 9: 
 				 vtemp=iter.created_time;
				 break;
			case 10: 
 				 vtemp=iter.updated_time;
				 break;
			case 11: 
 				 vtemp=iter.created_user;
				 break;
			case 12: 
 				 vtemp=iter.updated_user;
				 break;
			 }

			switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.urlname);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
				 break;
			  }

			 switch(dpos){

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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.permsid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.status);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.permscode);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.readonly);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.created_time);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.updated_time);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.created_user);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.updated_user);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.urlpath;
				 break;
			case 4: 
 				 vtemp=iter.urlname;
				 break;
			case 5: 
 				 vtemp=iter.name;
				 break;
			  }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.urlname);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 a[ktemp].emplace_back(iter.urlpath);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.urlname);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.name);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			 }

			 switch(vpos){

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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.permsid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.status);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.permscode);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.readonly);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.created_time);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.updated_time);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.created_user);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.updated_user);
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 a[ktemp].emplace_back(iter.urlpath);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.urlname);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.name);
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }

			 switch(vpos){

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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.permsid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.status);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.permscode);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.readonly);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.created_time);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.updated_time);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.created_user);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.updated_user);
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
 				 a[iter.permsid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.parentid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.status].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.permscode].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.readonly].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.created_time].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.updated_time].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.created_user].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.updated_user].emplace_back(iter);
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

   			case 3: 
 				 a[iter.urlpath].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.urlname].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.name].emplace_back(iter);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 a[ktemp][iter.urlpath].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.urlname].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.name].emplace_back(iter);
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

   			case 3: 
 				 ktemp=iter.urlpath;
				 break;
			case 4: 
 				 ktemp=iter.urlname;
				 break;
			case 5: 
 				 ktemp=iter.name;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.permsid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.status].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.permscode].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.readonly].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.created_time].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.updated_time].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.created_user].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.updated_user].emplace_back(iter);
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.permsid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.status].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.permscode].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.readonly].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.created_time].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.updated_time].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.created_user].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.updated_user].emplace_back(iter);
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
 				 ktemp=iter.permsid;
				 break;
			case 1: 
 				 ktemp=iter.parentid;
				 break;
			case 2: 
 				 ktemp=iter.userid;
				 break;
			case 6: 
 				 ktemp=iter.status;
				 break;
			case 7: 
 				 ktemp=iter.permscode;
				 break;
			case 8: 
 				 ktemp=iter.readonly;
				 break;
			case 9: 
 				 ktemp=iter.created_time;
				 break;
			case 10: 
 				 ktemp=iter.updated_time;
				 break;
			case 11: 
 				 ktemp=iter.created_user;
				 break;
			case 12: 
 				 ktemp=iter.updated_user;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 a[ktemp][iter.urlpath].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.urlname].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   