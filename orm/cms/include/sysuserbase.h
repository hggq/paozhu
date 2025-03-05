#ifndef ORM_CMS_SYSUSERBASEMATA_H
#define ORM_CMS_SYSUSERBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Wed, 05 Mar 2025 07:37:54 GMT
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

struct sysuserbase
{
    struct meta{
     unsigned  int  adminid = 0; ///**/
 std::string  name = ""; ///**/
 std::string  password = ""; ///**/
 std::string  textword = ""; ///**/
 char  isopen = 0; ///**/
 int  level = 0; ///**/
 unsigned  int  companyid = 0; ///*所属企业*/
 unsigned  int  dpid = 0; ///**/
 unsigned  int  jobid = 0; ///**/
 unsigned  int  roleid = 0; ///**/
 unsigned  int  postid = 0; ///**/
 unsigned  int  created_at = 0; ///**/
 unsigned  int  enddate = 0; ///**/
 unsigned  int  qrtemp = 0; ///**/
 unsigned  char  gender = 0; ///**/
 std::string  nickname = ""; ///**/
 std::string  realname = ""; ///**/
 std::string  avatar = ""; ///**/
 std::string  mobile = ""; ///**/
 std::string  email = ""; ///**/
 std::string  wxuuid = ""; ///**/
 } data;
 std::vector<sysuserbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<sysuserbase::meta>::iterator begin(){     return record.begin(); }
std::vector<sysuserbase::meta>::iterator end(){     return record.end(); }
std::vector<sysuserbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<sysuserbase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,21> col_names={"adminid","name","password","textword","isopen","level","companyid","dpid","jobid","roleid","postid","created_at","enddate","qrtemp","gender","nickname","realname","avatar","mobile","email","wxuuid"};
static constexpr std::array<unsigned char,21> col_types={3,253,253,253,1,3,3,3,3,3,3,3,3,3,1,253,253,253,253,253,253};
static constexpr std::array<unsigned char,21> col_length={0,40,40,40,1,0,0,0,0,0,0,0,0,0,0,60,40,0,40,60,40};
static constexpr std::array<unsigned char,21> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="sysuser";
static constexpr std::string_view modelname="Sysuser";

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


         case 'a':
 switch(coln.size()){  
case 6:
   	 return 17;
break;
case 7:
   	 return 0;
break;
 }
 break;
case 'c':
 switch(coln.size()){  
case 9:
   	 return 6;
break;
case 10:
   	 return 11;
break;
 }
 break;
case 'd':
   	 return 7;
break;
case 'e':
 switch(coln.size()){  
case 5:
   	 return 19;
break;
case 7:
   	 return 12;
break;
 }
 break;
case 'g':
   	 return 14;
break;
case 'i':
   	 return 4;
break;
case 'j':
   	 return 8;
break;
case 'l':
   	 return 5;
break;
case 'm':
   	 return 18;
break;
case 'n':
 switch(coln.size()){  
case 4:
   	 return 1;
break;
case 8:
   	 return 15;
break;
 }
 break;
case 'p':
 switch(coln.size()){  
case 6:
   	 return 10;
break;
case 8:
   	 return 2;
break;
 }
 break;
case 'q':
   	 return 13;
break;
case 'r':
 switch(coln.size()){  
case 6:
   	 return 9;
break;
case 8:
   	 return 16;
break;
 }
 break;
case 't':
   	 return 3;
break;
case 'w':
   	 return 20;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "adminid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     sysuserbase::meta metatemp;    
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

        if(data.adminid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.adminid);
}
tempsql<<",'"<<stringaddslash(data.name)<<"'";
tempsql<<",'"<<stringaddslash(data.password)<<"'";
tempsql<<",'"<<stringaddslash(data.textword)<<"'";
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
if(data.level==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.level);
}
if(data.companyid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.companyid);
}
if(data.dpid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.dpid);
}
if(data.jobid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.jobid);
}
if(data.roleid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.roleid);
}
if(data.postid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.postid);
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
if(data.qrtemp==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.qrtemp);
}
if(data.gender==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.gender);
}
tempsql<<",'"<<stringaddslash(data.nickname)<<"'";
tempsql<<",'"<<stringaddslash(data.realname)<<"'";
tempsql<<",'"<<stringaddslash(data.avatar)<<"'";
tempsql<<",'"<<stringaddslash(data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(data.email)<<"'";
tempsql<<",'"<<stringaddslash(data.wxuuid)<<"'";
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

        if(insert_data.adminid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.adminid);
}
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.password)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.textword)<<"'";
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
}
if(insert_data.level==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.level);
}
if(insert_data.companyid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.companyid);
}
if(insert_data.dpid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.dpid);
}
if(insert_data.jobid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.jobid);
}
if(insert_data.roleid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.roleid);
}
if(insert_data.postid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.postid);
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
if(insert_data.qrtemp==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.qrtemp);
}
if(insert_data.gender==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.gender);
}
tempsql<<",'"<<stringaddslash(insert_data.nickname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.realname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.avatar)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.email)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.wxuuid)<<"'";
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


            	if(insert_data[i].adminid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].adminid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].password)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].textword)<<"'";
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
	}
	if(insert_data[i].level==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].level);
	}
	if(insert_data[i].companyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].companyid);
	}
	if(insert_data[i].dpid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].dpid);
	}
	if(insert_data[i].jobid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].jobid);
	}
	if(insert_data[i].roleid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].roleid);
	}
	if(insert_data[i].postid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].postid);
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
	if(insert_data[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].qrtemp);
	}
	if(insert_data[i].gender==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].gender);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].nickname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].realname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].avatar)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].mobile)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].email)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].wxuuid)<<"'";
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

        if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
tempsql<<",`password`='"<<stringaddslash(data.password)<<"'";
tempsql<<",`textword`='"<<stringaddslash(data.textword)<<"'";
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
if(data.level==0){
	tempsql<<",`level`=0";
 }else{ 
	tempsql<<",`level`="<<std::to_string(data.level);
}
if(data.companyid==0){
	tempsql<<",`companyid`=0";
 }else{ 
	tempsql<<",`companyid`="<<std::to_string(data.companyid);
}
if(data.dpid==0){
	tempsql<<",`dpid`=0";
 }else{ 
	tempsql<<",`dpid`="<<std::to_string(data.dpid);
}
if(data.jobid==0){
	tempsql<<",`jobid`=0";
 }else{ 
	tempsql<<",`jobid`="<<std::to_string(data.jobid);
}
if(data.roleid==0){
	tempsql<<",`roleid`=0";
 }else{ 
	tempsql<<",`roleid`="<<std::to_string(data.roleid);
}
if(data.postid==0){
	tempsql<<",`postid`=0";
 }else{ 
	tempsql<<",`postid`="<<std::to_string(data.postid);
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
if(data.qrtemp==0){
	tempsql<<",`qrtemp`=0";
 }else{ 
	tempsql<<",`qrtemp`="<<std::to_string(data.qrtemp);
}
if(data.gender==0){
	tempsql<<",`gender`=0";
 }else{ 
	tempsql<<",`gender`="<<std::to_string(data.gender);
}
tempsql<<",`nickname`='"<<stringaddslash(data.nickname)<<"'";
tempsql<<",`realname`='"<<stringaddslash(data.realname)<<"'";
tempsql<<",`avatar`='"<<stringaddslash(data.avatar)<<"'";
tempsql<<",`mobile`='"<<stringaddslash(data.mobile)<<"'";
tempsql<<",`email`='"<<stringaddslash(data.email)<<"'";
tempsql<<",`wxuuid`='"<<stringaddslash(data.wxuuid)<<"'";
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
if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`password`='"<<stringaddslash(data.password)<<"'";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`textword`='"<<stringaddslash(data.textword)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.level==0){
	tempsql<<"`level`=0";
 }else{ 
	tempsql<<"`level`="<<std::to_string(data.level);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.companyid==0){
	tempsql<<"`companyid`=0";
 }else{ 
	tempsql<<"`companyid`="<<std::to_string(data.companyid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.dpid==0){
	tempsql<<"`dpid`=0";
 }else{ 
	tempsql<<"`dpid`="<<std::to_string(data.dpid);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.jobid==0){
	tempsql<<"`jobid`=0";
 }else{ 
	tempsql<<"`jobid`="<<std::to_string(data.jobid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.roleid==0){
	tempsql<<"`roleid`=0";
 }else{ 
	tempsql<<"`roleid`="<<std::to_string(data.roleid);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.postid==0){
	tempsql<<"`postid`=0";
 }else{ 
	tempsql<<"`postid`="<<std::to_string(data.postid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.created_at==0){
	tempsql<<"`created_at`=0";
 }else{ 
	tempsql<<"`created_at`="<<std::to_string(data.created_at);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.enddate==0){
	tempsql<<"`enddate`=0";
 }else{ 
	tempsql<<"`enddate`="<<std::to_string(data.enddate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.qrtemp==0){
	tempsql<<"`qrtemp`=0";
 }else{ 
	tempsql<<"`qrtemp`="<<std::to_string(data.qrtemp);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.gender==0){
	tempsql<<"`gender`=0";
 }else{ 
	tempsql<<"`gender`="<<std::to_string(data.gender);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`nickname`='"<<stringaddslash(data.nickname)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`realname`='"<<stringaddslash(data.realname)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`avatar`='"<<stringaddslash(data.avatar)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`mobile`='"<<stringaddslash(data.mobile)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`email`='"<<stringaddslash(data.email)<<"'";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`wxuuid`='"<<stringaddslash(data.wxuuid)<<"'";
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
            	if(record[i].adminid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].adminid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].password)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].textword)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].level==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].level);
	}
	if(record[i].companyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].companyid);
	}
	if(record[i].dpid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].dpid);
	}
	if(record[i].jobid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].jobid);
	}
	if(record[i].roleid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].roleid);
	}
	if(record[i].postid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].postid);
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
	if(record[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].qrtemp);
	}
	if(record[i].gender==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].gender);
	}
	tempsql<<",'"<<stringaddslash(record[i].nickname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].realname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].avatar)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].wxuuid)<<"'";
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
            	if(record[i].adminid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].adminid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].password)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].textword)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].level==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].level);
	}
	if(record[i].companyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].companyid);
	}
	if(record[i].dpid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].dpid);
	}
	if(record[i].jobid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].jobid);
	}
	if(record[i].roleid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].roleid);
	}
	if(record[i].postid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].postid);
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
	if(record[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].qrtemp);
	}
	if(record[i].gender==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].gender);
	}
	tempsql<<",'"<<stringaddslash(record[i].nickname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].realname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].avatar)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].wxuuid)<<"'";
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
if(data.adminid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.adminid));
}
 break;
 case 1:
	temparray.push_back(data.name);
 break;
 case 2:
	temparray.push_back(data.password);
 break;
 case 3:
	temparray.push_back(data.textword);
 break;
 case 4:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 5:
if(data.level==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.level));
}
 break;
 case 6:
if(data.companyid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.companyid));
}
 break;
 case 7:
if(data.dpid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.dpid));
}
 break;
 case 8:
if(data.jobid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.jobid));
}
 break;
 case 9:
if(data.roleid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.roleid));
}
 break;
 case 10:
if(data.postid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.postid));
}
 break;
 case 11:
if(data.created_at==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.created_at));
}
 break;
 case 12:
if(data.enddate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.enddate));
}
 break;
 case 13:
if(data.qrtemp==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.qrtemp));
}
 break;
 case 14:
if(data.gender==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.gender));
}
 break;
 case 15:
	temparray.push_back(data.nickname);
 break;
 case 16:
	temparray.push_back(data.realname);
 break;
 case 17:
	temparray.push_back(data.avatar);
 break;
 case 18:
	temparray.push_back(data.mobile);
 break;
 case 19:
	temparray.push_back(data.email);
 break;
 case 20:
	temparray.push_back(data.wxuuid);
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
if(data.adminid==0){
	tempsql.insert({"adminid","0"});
 }else{ 
	tempsql.insert({"adminid",std::to_string(data.adminid)});
}
 break;
 case 1:
	tempsql.insert({"name",data.name});
 break;
 case 2:
	tempsql.insert({"password",data.password});
 break;
 case 3:
	tempsql.insert({"textword",data.textword});
 break;
 case 4:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 5:
if(data.level==0){
	tempsql.insert({"level","0"});
 }else{ 
	tempsql.insert({"level",std::to_string(data.level)});
}
 break;
 case 6:
if(data.companyid==0){
	tempsql.insert({"companyid","0"});
 }else{ 
	tempsql.insert({"companyid",std::to_string(data.companyid)});
}
 break;
 case 7:
if(data.dpid==0){
	tempsql.insert({"dpid","0"});
 }else{ 
	tempsql.insert({"dpid",std::to_string(data.dpid)});
}
 break;
 case 8:
if(data.jobid==0){
	tempsql.insert({"jobid","0"});
 }else{ 
	tempsql.insert({"jobid",std::to_string(data.jobid)});
}
 break;
 case 9:
if(data.roleid==0){
	tempsql.insert({"roleid","0"});
 }else{ 
	tempsql.insert({"roleid",std::to_string(data.roleid)});
}
 break;
 case 10:
if(data.postid==0){
	tempsql.insert({"postid","0"});
 }else{ 
	tempsql.insert({"postid",std::to_string(data.postid)});
}
 break;
 case 11:
if(data.created_at==0){
	tempsql.insert({"created_at","0"});
 }else{ 
	tempsql.insert({"created_at",std::to_string(data.created_at)});
}
 break;
 case 12:
if(data.enddate==0){
	tempsql.insert({"enddate","0"});
 }else{ 
	tempsql.insert({"enddate",std::to_string(data.enddate)});
}
 break;
 case 13:
if(data.qrtemp==0){
	tempsql.insert({"qrtemp","0"});
 }else{ 
	tempsql.insert({"qrtemp",std::to_string(data.qrtemp)});
}
 break;
 case 14:
if(data.gender==0){
	tempsql.insert({"gender","0"});
 }else{ 
	tempsql.insert({"gender",std::to_string(data.gender)});
}
 break;
 case 15:
	tempsql.insert({"nickname",data.nickname});
 break;
 case 16:
	tempsql.insert({"realname",data.realname});
 break;
 case 17:
	tempsql.insert({"avatar",data.avatar});
 break;
 case 18:
	tempsql.insert({"mobile",data.mobile});
 break;
 case 19:
	tempsql.insert({"email",data.email});
 break;
 case 20:
	tempsql.insert({"wxuuid",data.wxuuid});
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
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
tempsql<<",\"password\":\""<<http::utf8_to_jsonstring(data.password);
tempsql<<"\"";
tempsql<<",\"textword\":\""<<http::utf8_to_jsonstring(data.textword);
tempsql<<"\"";
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
if(data.level==0){
	tempsql<<",\"level\":0";
 }else{ 
	tempsql<<",\"level\":"<<std::to_string(data.level);
}
if(data.companyid==0){
	tempsql<<",\"companyid\":0";
 }else{ 
	tempsql<<",\"companyid\":"<<std::to_string(data.companyid);
}
if(data.dpid==0){
	tempsql<<",\"dpid\":0";
 }else{ 
	tempsql<<",\"dpid\":"<<std::to_string(data.dpid);
}
if(data.jobid==0){
	tempsql<<",\"jobid\":0";
 }else{ 
	tempsql<<",\"jobid\":"<<std::to_string(data.jobid);
}
if(data.roleid==0){
	tempsql<<",\"roleid\":0";
 }else{ 
	tempsql<<",\"roleid\":"<<std::to_string(data.roleid);
}
if(data.postid==0){
	tempsql<<",\"postid\":0";
 }else{ 
	tempsql<<",\"postid\":"<<std::to_string(data.postid);
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
if(data.qrtemp==0){
	tempsql<<",\"qrtemp\":0";
 }else{ 
	tempsql<<",\"qrtemp\":"<<std::to_string(data.qrtemp);
}
if(data.gender==0){
	tempsql<<",\"gender\":0";
 }else{ 
	tempsql<<",\"gender\":"<<std::to_string(data.gender);
}
tempsql<<",\"nickname\":\""<<http::utf8_to_jsonstring(data.nickname);
tempsql<<"\"";
tempsql<<",\"realname\":\""<<http::utf8_to_jsonstring(data.realname);
tempsql<<"\"";
tempsql<<",\"avatar\":\""<<http::utf8_to_jsonstring(data.avatar);
tempsql<<"\"";
tempsql<<",\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile);
tempsql<<"\"";
tempsql<<",\"email\":\""<<http::utf8_to_jsonstring(data.email);
tempsql<<"\"";
tempsql<<",\"wxuuid\":\""<<http::utf8_to_jsonstring(data.wxuuid);
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
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"password\":\""<<http::utf8_to_jsonstring(data.password)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"textword\":\""<<http::utf8_to_jsonstring(data.textword)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(data.level);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.companyid==0){
	tempsql<<"\"companyid\":0";
 }else{ 
	tempsql<<"\"companyid\":"<<std::to_string(data.companyid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(data.dpid);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.jobid==0){
	tempsql<<"\"jobid\":0";
 }else{ 
	tempsql<<"\"jobid\":"<<std::to_string(data.jobid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(data.roleid);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.postid==0){
	tempsql<<"\"postid\":0";
 }else{ 
	tempsql<<"\"postid\":"<<std::to_string(data.postid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(data.created_at);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(data.enddate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(data.qrtemp);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.gender==0){
	tempsql<<"\"gender\":0";
 }else{ 
	tempsql<<"\"gender\":"<<std::to_string(data.gender);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(data.nickname)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"realname\":\""<<http::utf8_to_jsonstring(data.realname)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"avatar\":\""<<http::utf8_to_jsonstring(data.avatar)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(data.email)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(data.wxuuid)<<"\"";
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
        sysuserbase::meta metatemp; 
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
			data.adminid=std::stoul(set_value_name);
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 1:
		 try{
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 2:
		 try{
			data.password.append(set_value_name);
		}catch (...) { 
			data.password.clear();
			 }
			break;
		case 3:
		 try{
			data.textword.append(set_value_name);
		}catch (...) { 
			data.textword.clear();
			 }
			break;
		case 4:
		 try{
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 5:
		 try{
			data.level=std::stoi(set_value_name);
		}catch (...) { 
			data.level=0;
			 }
			break;
		case 6:
		 try{
			data.companyid=std::stoul(set_value_name);
		}catch (...) { 
			data.companyid=0;
			 }
			break;
		case 7:
		 try{
			data.dpid=std::stoul(set_value_name);
		}catch (...) { 
			data.dpid=0;
			 }
			break;
		case 8:
		 try{
			data.jobid=std::stoul(set_value_name);
		}catch (...) { 
			data.jobid=0;
			 }
			break;
		case 9:
		 try{
			data.roleid=std::stoul(set_value_name);
		}catch (...) { 
			data.roleid=0;
			 }
			break;
		case 10:
		 try{
			data.postid=std::stoul(set_value_name);
		}catch (...) { 
			data.postid=0;
			 }
			break;
		case 11:
		 try{
			data.created_at=std::stoul(set_value_name);
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 12:
		 try{
			data.enddate=std::stoul(set_value_name);
		}catch (...) { 
			data.enddate=0;
			 }
			break;
		case 13:
		 try{
			data.qrtemp=std::stoul(set_value_name);
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 14:
		 try{
			data.gender=std::stoi(set_value_name);
		}catch (...) { 
			data.gender=0;
			 }
			break;
		case 15:
		 try{
			data.nickname.append(set_value_name);
		}catch (...) { 
			data.nickname.clear();
			 }
			break;
		case 16:
		 try{
			data.realname.append(set_value_name);
		}catch (...) { 
			data.realname.clear();
			 }
			break;
		case 17:
		 try{
			data.avatar.append(set_value_name);
		}catch (...) { 
			data.avatar.clear();
			 }
			break;
		case 18:
		 try{
			data.mobile.append(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 19:
		 try{
			data.email.append(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 20:
		 try{
			data.wxuuid.append(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
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
			data.adminid=set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 1:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 2:
		 try{
			data.password=std::to_string(set_value_name);
		}catch (...) { 
			data.password.clear();
			 }
			break;
		case 3:
		 try{
			data.textword=std::to_string(set_value_name);
		}catch (...) { 
			data.textword.clear();
			 }
			break;
		case 4:
		 try{
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 5:
		 try{
			data.level=set_value_name;
		}catch (...) { 
			data.level=0;
			 }
			break;
		case 6:
		 try{
			data.companyid=set_value_name;
		}catch (...) { 
			data.companyid=0;
			 }
			break;
		case 7:
		 try{
			data.dpid=set_value_name;
		}catch (...) { 
			data.dpid=0;
			 }
			break;
		case 8:
		 try{
			data.jobid=set_value_name;
		}catch (...) { 
			data.jobid=0;
			 }
			break;
		case 9:
		 try{
			data.roleid=set_value_name;
		}catch (...) { 
			data.roleid=0;
			 }
			break;
		case 10:
		 try{
			data.postid=set_value_name;
		}catch (...) { 
			data.postid=0;
			 }
			break;
		case 11:
		 try{
			data.created_at=set_value_name;
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 12:
		 try{
			data.enddate=set_value_name;
		}catch (...) { 
			data.enddate=0;
			 }
			break;
		case 13:
		 try{
			data.qrtemp=set_value_name;
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 14:
		 try{
			data.gender=set_value_name;
		}catch (...) { 
			data.gender=0;
			 }
			break;
		case 15:
		 try{
			data.nickname=std::to_string(set_value_name);
		}catch (...) { 
			data.nickname.clear();
			 }
			break;
		case 16:
		 try{
			data.realname=std::to_string(set_value_name);
		}catch (...) { 
			data.realname.clear();
			 }
			break;
		case 17:
		 try{
			data.avatar=std::to_string(set_value_name);
		}catch (...) { 
			data.avatar.clear();
			 }
			break;
		case 18:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 19:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 20:
		 try{
			data.wxuuid=std::to_string(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
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
			data.adminid=(unsigned int)set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 1:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 2:
		 try{
			data.password=std::to_string(set_value_name);
		}catch (...) { 
			data.password.clear();
			 }
			break;
		case 3:
		 try{
			data.textword=std::to_string(set_value_name);
		}catch (...) { 
			data.textword.clear();
			 }
			break;
		case 4:
		 try{
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 5:
		 try{
			data.level=(int)set_value_name;
		}catch (...) { 
			data.level=0;
			 }
			break;
		case 6:
		 try{
			data.companyid=(unsigned int)set_value_name;
		}catch (...) { 
			data.companyid=0;
			 }
			break;
		case 7:
		 try{
			data.dpid=(unsigned int)set_value_name;
		}catch (...) { 
			data.dpid=0;
			 }
			break;
		case 8:
		 try{
			data.jobid=(unsigned int)set_value_name;
		}catch (...) { 
			data.jobid=0;
			 }
			break;
		case 9:
		 try{
			data.roleid=(unsigned int)set_value_name;
		}catch (...) { 
			data.roleid=0;
			 }
			break;
		case 10:
		 try{
			data.postid=(unsigned int)set_value_name;
		}catch (...) { 
			data.postid=0;
			 }
			break;
		case 11:
		 try{
			data.created_at=(unsigned int)set_value_name;
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 12:
		 try{
			data.enddate=(unsigned int)set_value_name;
		}catch (...) { 
			data.enddate=0;
			 }
			break;
		case 13:
		 try{
			data.qrtemp=(unsigned int)set_value_name;
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 14:
		 try{
			data.gender=(int)set_value_name;
		}catch (...) { 
			data.gender=0;
			 }
			break;
		case 15:
		 try{
			data.nickname=std::to_string(set_value_name);
		}catch (...) { 
			data.nickname.clear();
			 }
			break;
		case 16:
		 try{
			data.realname=std::to_string(set_value_name);
		}catch (...) { 
			data.realname.clear();
			 }
			break;
		case 17:
		 try{
			data.avatar=std::to_string(set_value_name);
		}catch (...) { 
			data.avatar.clear();
			 }
			break;
		case 18:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 19:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 20:
		 try{
			data.wxuuid=std::to_string(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
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
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"password\":\""<<http::utf8_to_jsonstring(record[n].password)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"textword\":\""<<http::utf8_to_jsonstring(record[n].textword)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(record[n].level);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].companyid==0){
	tempsql<<"\"companyid\":0";
 }else{ 
	tempsql<<"\"companyid\":"<<std::to_string(record[n].companyid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(record[n].dpid);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].jobid==0){
	tempsql<<"\"jobid\":0";
 }else{ 
	tempsql<<"\"jobid\":"<<std::to_string(record[n].jobid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(record[n].roleid);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].postid==0){
	tempsql<<"\"postid\":0";
 }else{ 
	tempsql<<"\"postid\":"<<std::to_string(record[n].postid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(record[n].created_at);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(record[n].enddate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(record[n].qrtemp);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].gender==0){
	tempsql<<"\"gender\":0";
 }else{ 
	tempsql<<"\"gender\":"<<std::to_string(record[n].gender);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(record[n].nickname)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"realname\":\""<<http::utf8_to_jsonstring(record[n].realname)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"avatar\":\""<<http::utf8_to_jsonstring(record[n].avatar)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(record[n].wxuuid)<<"\"";
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
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"password\":\""<<http::utf8_to_jsonstring(record[n].password)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"textword\":\""<<http::utf8_to_jsonstring(record[n].textword)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(record[n].level);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].companyid==0){
	tempsql<<"\"companyid\":0";
 }else{ 
	tempsql<<"\"companyid\":"<<std::to_string(record[n].companyid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(record[n].dpid);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].jobid==0){
	tempsql<<"\"jobid\":0";
 }else{ 
	tempsql<<"\"jobid\":"<<std::to_string(record[n].jobid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(record[n].roleid);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].postid==0){
	tempsql<<"\"postid\":0";
 }else{ 
	tempsql<<"\"postid\":"<<std::to_string(record[n].postid);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(record[n].created_at);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(record[n].enddate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(record[n].qrtemp);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].gender==0){
	tempsql<<"\"gender\":0";
 }else{ 
	tempsql<<"\"gender\":"<<std::to_string(record[n].gender);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(record[n].nickname)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"realname\":\""<<http::utf8_to_jsonstring(record[n].realname)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"avatar\":\""<<http::utf8_to_jsonstring(record[n].avatar)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(record[n].wxuuid)<<"\"";
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
   long long getPK(){  return data.adminid; } 
 void setPK(long long val){  data.adminid=val;} 
 unsigned  int  getAdminid(){  return data.adminid; } 
 void setAdminid( unsigned  int  val){  data.adminid=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 std::string  getPassword(){  return data.password; } 
 std::string & getRefPassword(){  return std::ref(data.password); } 
 void setPassword( std::string  &val){  data.password=val;} 
 void setPassword(std::string_view val){  data.password=val;} 

 std::string  getTextword(){  return data.textword; } 
 std::string & getRefTextword(){  return std::ref(data.textword); } 
 void setTextword( std::string  &val){  data.textword=val;} 
 void setTextword(std::string_view val){  data.textword=val;} 

 char  getIsopen(){  return data.isopen; } 
 void setIsopen( char  val){  data.isopen=val;} 

 int  getLevel(){  return data.level; } 
 void setLevel( int  val){  data.level=val;} 

 unsigned  int  getCompanyid(){  return data.companyid; } 
 void setCompanyid( unsigned  int  val){  data.companyid=val;} 

 unsigned  int  getDpid(){  return data.dpid; } 
 void setDpid( unsigned  int  val){  data.dpid=val;} 

 unsigned  int  getJobid(){  return data.jobid; } 
 void setJobid( unsigned  int  val){  data.jobid=val;} 

 unsigned  int  getRoleid(){  return data.roleid; } 
 void setRoleid( unsigned  int  val){  data.roleid=val;} 

 unsigned  int  getPostid(){  return data.postid; } 
 void setPostid( unsigned  int  val){  data.postid=val;} 

 unsigned  int  getCreatedAt(){  return data.created_at; } 
 void setCreatedAt( unsigned  int  val){  data.created_at=val;} 

 unsigned  int  getEnddate(){  return data.enddate; } 
 void setEnddate( unsigned  int  val){  data.enddate=val;} 

 unsigned  int  getQrtemp(){  return data.qrtemp; } 
 void setQrtemp( unsigned  int  val){  data.qrtemp=val;} 

 unsigned  char  getGender(){  return data.gender; } 
 void setGender( unsigned  char  val){  data.gender=val;} 

 std::string  getNickname(){  return data.nickname; } 
 std::string & getRefNickname(){  return std::ref(data.nickname); } 
 void setNickname( std::string  &val){  data.nickname=val;} 
 void setNickname(std::string_view val){  data.nickname=val;} 

 std::string  getRealname(){  return data.realname; } 
 std::string & getRefRealname(){  return std::ref(data.realname); } 
 void setRealname( std::string  &val){  data.realname=val;} 
 void setRealname(std::string_view val){  data.realname=val;} 

 std::string  getAvatar(){  return data.avatar; } 
 std::string & getRefAvatar(){  return std::ref(data.avatar); } 
 void setAvatar( std::string  &val){  data.avatar=val;} 
 void setAvatar(std::string_view val){  data.avatar=val;} 

 std::string  getMobile(){  return data.mobile; } 
 std::string & getRefMobile(){  return std::ref(data.mobile); } 
 void setMobile( std::string  &val){  data.mobile=val;} 
 void setMobile(std::string_view val){  data.mobile=val;} 

 std::string  getEmail(){  return data.email; } 
 std::string & getRefEmail(){  return std::ref(data.email); } 
 void setEmail( std::string  &val){  data.email=val;} 
 void setEmail(std::string_view val){  data.email=val;} 

 std::string  getWxuuid(){  return data.wxuuid; } 
 std::string & getRefWxuuid(){  return std::ref(data.wxuuid); } 
 void setWxuuid( std::string  &val){  data.wxuuid=val;} 
 void setWxuuid(std::string_view val){  data.wxuuid=val;} 

sysuserbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
sysuserbase::meta getData(){
 	 return data; 
} 
std::vector<sysuserbase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="name")
		{
			return data.name;
		}
		 if(key_name=="password")
		{
			return data.password;
		}
		 if(key_name=="textword")
		{
			return data.textword;
		}
		 if(key_name=="nickname")
		{
			return data.nickname;
		}
		 if(key_name=="realname")
		{
			return data.realname;
		}
		 if(key_name=="avatar")
		{
			return data.avatar;
		}
		 if(key_name=="mobile")
		{
			return data.mobile;
		}
		 if(key_name=="email")
		{
			return data.email;
		}
		 if(key_name=="wxuuid")
		{
			return data.wxuuid;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="adminid")
		{
			return data.adminid;
		}
		 if(key_name=="isopen")
		{
			return data.isopen;
		}
		 if(key_name=="level")
		{
			return data.level;
		}
		 if(key_name=="companyid")
		{
			return data.companyid;
		}
		 if(key_name=="dpid")
		{
			return data.dpid;
		}
		 if(key_name=="jobid")
		{
			return data.jobid;
		}
		 if(key_name=="roleid")
		{
			return data.roleid;
		}
		 if(key_name=="postid")
		{
			return data.postid;
		}
		 if(key_name=="created_at")
		{
			return data.created_at;
		}
		 if(key_name=="enddate")
		{
			return data.enddate;
		}
		 if(key_name=="qrtemp")
		{
			return data.qrtemp;
		}
		 if(key_name=="gender")
		{
			return data.gender;
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
 				 a.emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a.emplace_back(iter.isopen);
				 break;
			case 5: 
 				 a.emplace_back(iter.level);
				 break;
			case 6: 
 				 a.emplace_back(iter.companyid);
				 break;
			case 7: 
 				 a.emplace_back(iter.dpid);
				 break;
			case 8: 
 				 a.emplace_back(iter.jobid);
				 break;
			case 9: 
 				 a.emplace_back(iter.roleid);
				 break;
			case 10: 
 				 a.emplace_back(iter.postid);
				 break;
			case 11: 
 				 a.emplace_back(iter.created_at);
				 break;
			case 12: 
 				 a.emplace_back(iter.enddate);
				 break;
			case 13: 
 				 a.emplace_back(iter.qrtemp);
				 break;
			case 14: 
 				 a.emplace_back(iter.gender);
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
 				 return data.adminid;
				 break;
			case 4: 
 				 return data.isopen;
				 break;
			case 5: 
 				 return data.level;
				 break;
			case 6: 
 				 return data.companyid;
				 break;
			case 7: 
 				 return data.dpid;
				 break;
			case 8: 
 				 return data.jobid;
				 break;
			case 9: 
 				 return data.roleid;
				 break;
			case 10: 
 				 return data.postid;
				 break;
			case 11: 
 				 return data.created_at;
				 break;
			case 12: 
 				 return data.enddate;
				 break;
			case 13: 
 				 return data.qrtemp;
				 break;
			case 14: 
 				 return data.gender;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] sysuserbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.adminid;
				 break;
			case 4: 
 				 return iter.isopen;
				 break;
			case 5: 
 				 return iter.level;
				 break;
			case 6: 
 				 return iter.companyid;
				 break;
			case 7: 
 				 return iter.dpid;
				 break;
			case 8: 
 				 return iter.jobid;
				 break;
			case 9: 
 				 return iter.roleid;
				 break;
			case 10: 
 				 return iter.postid;
				 break;
			case 11: 
 				 return iter.created_at;
				 break;
			case 12: 
 				 return iter.enddate;
				 break;
			case 13: 
 				 return iter.qrtemp;
				 break;
			case 14: 
 				 return iter.gender;
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
            T getVal([[maybe_unused]] sysuserbase::meta & iter,std::string keyname)
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

   			case 1: 
 				 return data.name;
				 break;
			case 2: 
 				 return data.password;
				 break;
			case 3: 
 				 return data.textword;
				 break;
			case 15: 
 				 return data.nickname;
				 break;
			case 16: 
 				 return data.realname;
				 break;
			case 17: 
 				 return data.avatar;
				 break;
			case 18: 
 				 return data.mobile;
				 break;
			case 19: 
 				 return data.email;
				 break;
			case 20: 
 				 return data.wxuuid;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] sysuserbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 1: 
 				 return iter.name;
				 break;
			case 2: 
 				 return iter.password;
				 break;
			case 3: 
 				 return iter.textword;
				 break;
			case 15: 
 				 return iter.nickname;
				 break;
			case 16: 
 				 return iter.realname;
				 break;
			case 17: 
 				 return iter.avatar;
				 break;
			case 18: 
 				 return iter.mobile;
				 break;
			case 19: 
 				 return iter.email;
				 break;
			case 20: 
 				 return iter.wxuuid;
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

    			case 1: 
 				 a.emplace_back(iter.name);
					 break;
			case 2: 
 				 a.emplace_back(iter.password);
					 break;
			case 3: 
 				 a.emplace_back(iter.textword);
					 break;
			case 15: 
 				 a.emplace_back(iter.nickname);
					 break;
			case 16: 
 				 a.emplace_back(iter.realname);
					 break;
			case 17: 
 				 a.emplace_back(iter.avatar);
					 break;
			case 18: 
 				 a.emplace_back(iter.mobile);
					 break;
			case 19: 
 				 a.emplace_back(iter.email);
					 break;
			case 20: 
 				 a.emplace_back(iter.wxuuid);
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
 				 a<<std::to_string(iter.adminid);
				 break;
			case 1: 
 				 if(isyinhao){ a<<jsonaddslash(iter.name); 
				 }else{
				 a<<iter.name;
				 }
				 break;
			case 2: 
 				 if(isyinhao){ a<<jsonaddslash(iter.password); 
				 }else{
				 a<<iter.password;
				 }
				 break;
			case 3: 
 				 if(isyinhao){ a<<jsonaddslash(iter.textword); 
				 }else{
				 a<<iter.textword;
				 }
				 break;
			case 4: 
 				 a<<std::to_string(iter.isopen);
				 break;
			case 5: 
 				 a<<std::to_string(iter.level);
				 break;
			case 6: 
 				 a<<std::to_string(iter.companyid);
				 break;
			case 7: 
 				 a<<std::to_string(iter.dpid);
				 break;
			case 8: 
 				 a<<std::to_string(iter.jobid);
				 break;
			case 9: 
 				 a<<std::to_string(iter.roleid);
				 break;
			case 10: 
 				 a<<std::to_string(iter.postid);
				 break;
			case 11: 
 				 a<<std::to_string(iter.created_at);
				 break;
			case 12: 
 				 a<<std::to_string(iter.enddate);
				 break;
			case 13: 
 				 a<<std::to_string(iter.qrtemp);
				 break;
			case 14: 
 				 a<<std::to_string(iter.gender);
				 break;
			case 15: 
 				 if(isyinhao){ a<<jsonaddslash(iter.nickname); 
				 }else{
				 a<<iter.nickname;
				 }
				 break;
			case 16: 
 				 if(isyinhao){ a<<jsonaddslash(iter.realname); 
				 }else{
				 a<<iter.realname;
				 }
				 break;
			case 17: 
 				 if(isyinhao){ a<<jsonaddslash(iter.avatar); 
				 }else{
				 a<<iter.avatar;
				 }
				 break;
			case 18: 
 				 if(isyinhao){ a<<jsonaddslash(iter.mobile); 
				 }else{
				 a<<iter.mobile;
				 }
				 break;
			case 19: 
 				 if(isyinhao){ a<<jsonaddslash(iter.email); 
				 }else{
				 a<<iter.email;
				 }
				 break;
			case 20: 
 				 if(isyinhao){ a<<jsonaddslash(iter.wxuuid); 
				 }else{
				 a<<iter.wxuuid;
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
				 } 
			switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
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
 
       			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
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
 	 ktemp=iter.adminid;
	 break;
case 4: 
 	 ktemp=iter.isopen;
	 break;
case 5: 
 	 ktemp=iter.level;
	 break;
case 6: 
 	 ktemp=iter.companyid;
	 break;
case 7: 
 	 ktemp=iter.dpid;
	 break;
case 8: 
 	 ktemp=iter.jobid;
	 break;
case 9: 
 	 ktemp=iter.roleid;
	 break;
case 10: 
 	 ktemp=iter.postid;
	 break;
case 11: 
 	 ktemp=iter.created_at;
	 break;
case 12: 
 	 ktemp=iter.enddate;
	 break;
case 13: 
 	 ktemp=iter.qrtemp;
	 break;
case 14: 
 	 ktemp=iter.gender;
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			  }
 			switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
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
 				 a.emplace(iter.adminid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.isopen,iter);
				 break;
			case 5: 
 				 a.emplace(iter.level,iter);
				 break;
			case 6: 
 				 a.emplace(iter.companyid,iter);
				 break;
			case 7: 
 				 a.emplace(iter.dpid,iter);
				 break;
			case 8: 
 				 a.emplace(iter.jobid,iter);
				 break;
			case 9: 
 				 a.emplace(iter.roleid,iter);
				 break;
			case 10: 
 				 a.emplace(iter.postid,iter);
				 break;
			case 11: 
 				 a.emplace(iter.created_at,iter);
				 break;
			case 12: 
 				 a.emplace(iter.enddate,iter);
				 break;
			case 13: 
 				 a.emplace(iter.qrtemp,iter);
				 break;
			case 14: 
 				 a.emplace(iter.gender,iter);
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

   			case 1: 
 				 a.emplace(iter.name,iter);
			 break;
			case 2: 
 				 a.emplace(iter.password,iter);
			 break;
			case 3: 
 				 a.emplace(iter.textword,iter);
			 break;
			case 15: 
 				 a.emplace(iter.nickname,iter);
			 break;
			case 16: 
 				 a.emplace(iter.realname,iter);
			 break;
			case 17: 
 				 a.emplace(iter.avatar,iter);
			 break;
			case 18: 
 				 a.emplace(iter.mobile,iter);
			 break;
			case 19: 
 				 a.emplace(iter.email,iter);
			 break;
			case 20: 
 				 a.emplace(iter.wxuuid,iter);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
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
 				 ktemp=iter.adminid;
			 break;
			case 4: 
 				 ktemp=iter.isopen;
			 break;
			case 5: 
 				 ktemp=iter.level;
			 break;
			case 6: 
 				 ktemp=iter.companyid;
			 break;
			case 7: 
 				 ktemp=iter.dpid;
			 break;
			case 8: 
 				 ktemp=iter.jobid;
			 break;
			case 9: 
 				 ktemp=iter.roleid;
			 break;
			case 10: 
 				 ktemp=iter.postid;
			 break;
			case 11: 
 				 ktemp=iter.created_at;
			 break;
			case 12: 
 				 ktemp=iter.enddate;
			 break;
			case 13: 
 				 ktemp=iter.qrtemp;
			 break;
			case 14: 
 				 ktemp=iter.gender;
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			  }
 			switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
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

   case 1: 
 	 ktemp=iter.name;
	 break;
case 2: 
 	 ktemp=iter.password;
	 break;
case 3: 
 	 ktemp=iter.textword;
	 break;
case 15: 
 	 ktemp=iter.nickname;
	 break;
case 16: 
 	 ktemp=iter.realname;
	 break;
case 17: 
 	 ktemp=iter.avatar;
	 break;
case 18: 
 	 ktemp=iter.mobile;
	 break;
case 19: 
 	 ktemp=iter.email;
	 break;
case 20: 
 	 ktemp=iter.wxuuid;
	 break;
	  }
 switch(vpos){
case 1: 
 	 vtemp=iter.name;
	 break;
case 2: 
 	 vtemp=iter.password;
	 break;
case 3: 
 	 vtemp=iter.textword;
	 break;
case 15: 
 	 vtemp=iter.nickname;
	 break;
case 16: 
 	 vtemp=iter.realname;
	 break;
case 17: 
 	 vtemp=iter.avatar;
	 break;
case 18: 
 	 vtemp=iter.mobile;
	 break;
case 19: 
 	 vtemp=iter.email;
	 break;
case 20: 
 	 vtemp=iter.wxuuid;
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
 	 a.emplace_back(iter.adminid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.isopen,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.level,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.companyid,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.dpid,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.jobid,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.roleid,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.postid,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.created_at,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.enddate,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.qrtemp,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.gender,iter);
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

   case 1: 
 	 a.emplace_back(iter.name,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.password,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.textword,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.nickname,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.realname,iter);
	 break;
case 17: 
 	 a.emplace_back(iter.avatar,iter);
	 break;
case 18: 
 	 a.emplace_back(iter.mobile,iter);
	 break;
case 19: 
 	 a.emplace_back(iter.email,iter);
	 break;
case 20: 
 	 a.emplace_back(iter.wxuuid,iter);
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.companyid);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.jobid);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.roleid);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.postid);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.created_at);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.qrtemp);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.gender);
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
				 break;
			 }

			 switch(dpos){
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.password);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.textword);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.nickname);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.realname);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.avatar);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.wxuuid);
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			 }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			 }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.companyid);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.jobid);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.roleid);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.postid);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.created_at);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.qrtemp);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.gender);
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			  }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
				 break;
			  }

			 switch(dpos){
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.password);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.textword);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.nickname);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.realname);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.avatar);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.wxuuid);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.companyid);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.jobid);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.roleid);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.postid);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.created_at);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.qrtemp);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.gender);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.adminid;
				 break;
			case 4: 
 				 vtemp=iter.isopen;
				 break;
			case 5: 
 				 vtemp=iter.level;
				 break;
			case 6: 
 				 vtemp=iter.companyid;
				 break;
			case 7: 
 				 vtemp=iter.dpid;
				 break;
			case 8: 
 				 vtemp=iter.jobid;
				 break;
			case 9: 
 				 vtemp=iter.roleid;
				 break;
			case 10: 
 				 vtemp=iter.postid;
				 break;
			case 11: 
 				 vtemp=iter.created_at;
				 break;
			case 12: 
 				 vtemp=iter.enddate;
				 break;
			case 13: 
 				 vtemp=iter.qrtemp;
				 break;
			case 14: 
 				 vtemp=iter.gender;
				 break;
			 }

			switch(dpos){
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.password);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.textword);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.nickname);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.realname);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.avatar);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.wxuuid);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			 }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			  }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.companyid);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.jobid);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.roleid);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.postid);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.created_at);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.qrtemp);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.gender);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			  }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			case 3: 
 				 vtemp=iter.textword;
				 break;
			case 15: 
 				 vtemp=iter.nickname;
				 break;
			case 16: 
 				 vtemp=iter.realname;
				 break;
			case 17: 
 				 vtemp=iter.avatar;
				 break;
			case 18: 
 				 vtemp=iter.mobile;
				 break;
			case 19: 
 				 vtemp=iter.email;
				 break;
			case 20: 
 				 vtemp=iter.wxuuid;
				 break;
			  }

			 switch(dpos){
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.password);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.textword);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.nickname);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.realname);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.avatar);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.wxuuid);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			  }

			 switch(vpos){
			case 1: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.password);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.textword);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.nickname);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.realname);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.avatar);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.mobile);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.email);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.wxuuid);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.level);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.companyid);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.dpid);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.jobid);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.roleid);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.postid);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.created_at);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.enddate);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.qrtemp);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.gender);
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			 }

			 switch(vpos){
			case 1: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.password);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.textword);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.nickname);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.realname);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.avatar);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.mobile);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.email);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.wxuuid);
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.adminid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.level);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.companyid);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.dpid);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.jobid);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.roleid);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.postid);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.created_at);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.enddate);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.qrtemp);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.gender);
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
 				 a[iter.adminid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.isopen].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.level].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.companyid].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.dpid].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.jobid].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.roleid].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.postid].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.created_at].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.enddate].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.qrtemp].emplace_back(iter);
				 break;
			case 14: 
 				 a[iter.gender].emplace_back(iter);
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

   			case 1: 
 				 a[iter.name].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.password].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.textword].emplace_back(iter);
				 break;
			case 15: 
 				 a[iter.nickname].emplace_back(iter);
				 break;
			case 16: 
 				 a[iter.realname].emplace_back(iter);
				 break;
			case 17: 
 				 a[iter.avatar].emplace_back(iter);
				 break;
			case 18: 
 				 a[iter.mobile].emplace_back(iter);
				 break;
			case 19: 
 				 a[iter.email].emplace_back(iter);
				 break;
			case 20: 
 				 a[iter.wxuuid].emplace_back(iter);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
			 }

			 switch(vpos){
			case 1: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.password].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.textword].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.nickname].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.realname].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.avatar].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.mobile].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.email].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.wxuuid].emplace_back(iter);
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

   			case 1: 
 				 ktemp=iter.name;
				 break;
			case 2: 
 				 ktemp=iter.password;
				 break;
			case 3: 
 				 ktemp=iter.textword;
				 break;
			case 15: 
 				 ktemp=iter.nickname;
				 break;
			case 16: 
 				 ktemp=iter.realname;
				 break;
			case 17: 
 				 ktemp=iter.avatar;
				 break;
			case 18: 
 				 ktemp=iter.mobile;
				 break;
			case 19: 
 				 ktemp=iter.email;
				 break;
			case 20: 
 				 ktemp=iter.wxuuid;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.adminid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.level].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.companyid].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.dpid].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.jobid].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.roleid].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.postid].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.created_at].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.enddate].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.qrtemp].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.gender].emplace_back(iter);
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.adminid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.level].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.companyid].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.dpid].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.jobid].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.roleid].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.postid].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.created_at].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.enddate].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.qrtemp].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.gender].emplace_back(iter);
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
 				 ktemp=iter.adminid;
				 break;
			case 4: 
 				 ktemp=iter.isopen;
				 break;
			case 5: 
 				 ktemp=iter.level;
				 break;
			case 6: 
 				 ktemp=iter.companyid;
				 break;
			case 7: 
 				 ktemp=iter.dpid;
				 break;
			case 8: 
 				 ktemp=iter.jobid;
				 break;
			case 9: 
 				 ktemp=iter.roleid;
				 break;
			case 10: 
 				 ktemp=iter.postid;
				 break;
			case 11: 
 				 ktemp=iter.created_at;
				 break;
			case 12: 
 				 ktemp=iter.enddate;
				 break;
			case 13: 
 				 ktemp=iter.qrtemp;
				 break;
			case 14: 
 				 ktemp=iter.gender;
				 break;
			  }

			 switch(vpos){
			case 1: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.password].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.textword].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.nickname].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.realname].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.avatar].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.mobile].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.email].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.wxuuid].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   