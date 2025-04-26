#ifndef ORM_CMS_SUPERADMINBASEMATA_H
#define ORM_CMS_SUPERADMINBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sat, 26 Apr 2025 15:44:27 GMT
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

struct superadmin_base
{
    struct meta{
     unsigned  int  adminid = 0; ///**/
 std::string  name = ""; ///**/
 std::string  password = ""; ///**/
 std::string  nickname = ""; ///**/
 char  isopen = 0; ///**/
 unsigned  int  begindate = 0; ///**/
 unsigned  int  enddate = 0; ///**/
 std::string  regdate = ""; ///**/
 std::string  mobile = ""; ///**/
 std::string  email = ""; ///**/
 unsigned  int  loginnum = 0; ///**/
 unsigned  int  qrtemp = 0; ///**/
 std::string  wxuuid = ""; ///**/
 std::string  basesitepath = ""; ///**/
 } data;
 std::vector<superadmin_base::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<superadmin_base::meta>::iterator begin(){     return record.begin(); }
std::vector<superadmin_base::meta>::iterator end(){     return record.end(); }
std::vector<superadmin_base::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<superadmin_base::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,14> col_names={"adminid","name","password","nickname","isopen","begindate","enddate","regdate","mobile","email","loginnum","qrtemp","wxuuid","basesitepath"};
static constexpr std::array<unsigned char,14> col_types={3,253,253,253,1,3,3,253,253,253,3,3,253,253};
static constexpr std::array<unsigned char,14> col_length={0,40,40,30,0,0,0,30,40,40,0,0,40,0};
static constexpr std::array<unsigned char,14> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="superadmin";
static constexpr std::string_view modelname="Superadmin";

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
   	 return 0;
break;
case 'b':
 switch(coln.size()){  
case 9:
   	 return 5;
break;
case 12:
   	 return 13;
break;
 }
 break;
case 'e':
 switch(coln.size()){  
case 5:
   	 return 9;
break;
case 7:
   	 return 6;
break;
 }
 break;
case 'i':
   	 return 4;
break;
case 'l':
   	 return 10;
break;
case 'm':
   	 return 8;
break;
case 'n':
 switch(coln.size()){  
case 4:
   	 return 1;
break;
case 8:
   	 return 3;
break;
 }
 break;
case 'p':
   	 return 2;
break;
case 'q':
   	 return 11;
break;
case 'r':
   	 return 7;
break;
case 'w':
   	 return 12;
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
     superadmin_base::meta metatemp;    
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
tempsql<<",'"<<stringaddslash(data.nickname)<<"'";
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
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
tempsql<<",'"<<stringaddslash(data.regdate)<<"'";
tempsql<<",'"<<stringaddslash(data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(data.email)<<"'";
if(data.loginnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.loginnum);
}
if(data.qrtemp==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.qrtemp);
}
tempsql<<",'"<<stringaddslash(data.wxuuid)<<"'";
tempsql<<",'"<<stringaddslash(data.basesitepath)<<"'";
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
tempsql<<",'"<<stringaddslash(insert_data.nickname)<<"'";
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
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
tempsql<<",'"<<stringaddslash(insert_data.regdate)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.email)<<"'";
if(insert_data.loginnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.loginnum);
}
if(insert_data.qrtemp==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.qrtemp);
}
tempsql<<",'"<<stringaddslash(insert_data.wxuuid)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.basesitepath)<<"'";
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
		tempsql<<",'"<<stringaddslash(insert_data[i].nickname)<<"'";
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
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
		tempsql<<",'"<<stringaddslash(insert_data[i].regdate)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].mobile)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].email)<<"'";
	if(insert_data[i].loginnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].loginnum);
	}
	if(insert_data[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].qrtemp);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].wxuuid)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].basesitepath)<<"'";
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
tempsql<<",`nickname`='"<<stringaddslash(data.nickname)<<"'";
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
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
tempsql<<",`regdate`='"<<stringaddslash(data.regdate)<<"'";
tempsql<<",`mobile`='"<<stringaddslash(data.mobile)<<"'";
tempsql<<",`email`='"<<stringaddslash(data.email)<<"'";
if(data.loginnum==0){
	tempsql<<",`loginnum`=0";
 }else{ 
	tempsql<<",`loginnum`="<<std::to_string(data.loginnum);
}
if(data.qrtemp==0){
	tempsql<<",`qrtemp`=0";
 }else{ 
	tempsql<<",`qrtemp`="<<std::to_string(data.qrtemp);
}
tempsql<<",`wxuuid`='"<<stringaddslash(data.wxuuid)<<"'";
tempsql<<",`basesitepath`='"<<stringaddslash(data.basesitepath)<<"'";
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
tempsql<<"`nickname`='"<<stringaddslash(data.nickname)<<"'";
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
tempsql<<"`regdate`='"<<stringaddslash(data.regdate)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`mobile`='"<<stringaddslash(data.mobile)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`email`='"<<stringaddslash(data.email)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.loginnum==0){
	tempsql<<"`loginnum`=0";
 }else{ 
	tempsql<<"`loginnum`="<<std::to_string(data.loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.qrtemp==0){
	tempsql<<"`qrtemp`=0";
 }else{ 
	tempsql<<"`qrtemp`="<<std::to_string(data.qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`wxuuid`='"<<stringaddslash(data.wxuuid)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`basesitepath`='"<<stringaddslash(data.basesitepath)<<"'";
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
	tempsql<<",'"<<stringaddslash(record[i].nickname)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
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
	tempsql<<",'"<<stringaddslash(record[i].regdate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	if(record[i].loginnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].loginnum);
	}
	if(record[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].qrtemp);
	}
	tempsql<<",'"<<stringaddslash(record[i].wxuuid)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].basesitepath)<<"'";
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
	tempsql<<",'"<<stringaddslash(record[i].nickname)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
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
	tempsql<<",'"<<stringaddslash(record[i].regdate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	if(record[i].loginnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].loginnum);
	}
	if(record[i].qrtemp==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].qrtemp);
	}
	tempsql<<",'"<<stringaddslash(record[i].wxuuid)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].basesitepath)<<"'";
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
	temparray.push_back(data.nickname);
 break;
 case 4:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
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
	temparray.push_back(data.regdate);
 break;
 case 8:
	temparray.push_back(data.mobile);
 break;
 case 9:
	temparray.push_back(data.email);
 break;
 case 10:
if(data.loginnum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.loginnum));
}
 break;
 case 11:
if(data.qrtemp==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.qrtemp));
}
 break;
 case 12:
	temparray.push_back(data.wxuuid);
 break;
 case 13:
	temparray.push_back(data.basesitepath);
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
	tempsql.insert({"nickname",data.nickname});
 break;
 case 4:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
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
	tempsql.insert({"regdate",data.regdate});
 break;
 case 8:
	tempsql.insert({"mobile",data.mobile});
 break;
 case 9:
	tempsql.insert({"email",data.email});
 break;
 case 10:
if(data.loginnum==0){
	tempsql.insert({"loginnum","0"});
 }else{ 
	tempsql.insert({"loginnum",std::to_string(data.loginnum)});
}
 break;
 case 11:
if(data.qrtemp==0){
	tempsql.insert({"qrtemp","0"});
 }else{ 
	tempsql.insert({"qrtemp",std::to_string(data.qrtemp)});
}
 break;
 case 12:
	tempsql.insert({"wxuuid",data.wxuuid});
 break;
 case 13:
	tempsql.insert({"basesitepath",data.basesitepath});
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
tempsql<<",\"nickname\":\""<<http::utf8_to_jsonstring(data.nickname);
tempsql<<"\"";
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
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
tempsql<<",\"regdate\":\""<<http::utf8_to_jsonstring(data.regdate);
tempsql<<"\"";
tempsql<<",\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile);
tempsql<<"\"";
tempsql<<",\"email\":\""<<http::utf8_to_jsonstring(data.email);
tempsql<<"\"";
if(data.loginnum==0){
	tempsql<<",\"loginnum\":0";
 }else{ 
	tempsql<<",\"loginnum\":"<<std::to_string(data.loginnum);
}
if(data.qrtemp==0){
	tempsql<<",\"qrtemp\":0";
 }else{ 
	tempsql<<",\"qrtemp\":"<<std::to_string(data.qrtemp);
}
tempsql<<",\"wxuuid\":\""<<http::utf8_to_jsonstring(data.wxuuid);
tempsql<<"\"";
tempsql<<",\"basesitepath\":\""<<http::utf8_to_jsonstring(data.basesitepath);
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
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(data.nickname)<<"\"";
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
tempsql<<"\"regdate\":\""<<http::utf8_to_jsonstring(data.regdate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(data.email)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.loginnum==0){
	tempsql<<"\"loginnum\":0";
 }else{ 
	tempsql<<"\"loginnum\":"<<std::to_string(data.loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(data.qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(data.wxuuid)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"basesitepath\":\""<<http::utf8_to_jsonstring(data.basesitepath)<<"\"";
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
        superadmin_base::meta metatemp; 
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
			data.nickname.append(set_value_name);
		}catch (...) { 
			data.nickname.clear();
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
			data.regdate.append(set_value_name);
		}catch (...) { 
			data.regdate.clear();
			 }
			break;
		case 8:
		 try{
			data.mobile.append(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 9:
		 try{
			data.email.append(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 10:
		 try{
			data.loginnum=std::stoul(set_value_name);
		}catch (...) { 
			data.loginnum=0;
			 }
			break;
		case 11:
		 try{
			data.qrtemp=std::stoul(set_value_name);
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 12:
		 try{
			data.wxuuid.append(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
			 }
			break;
		case 13:
		 try{
			data.basesitepath.append(set_value_name);
		}catch (...) { 
			data.basesitepath.clear();
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
			data.nickname=std::to_string(set_value_name);
		}catch (...) { 
			data.nickname.clear();
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
			data.regdate=std::to_string(set_value_name);
		}catch (...) { 
			data.regdate.clear();
			 }
			break;
		case 8:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 9:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 10:
		 try{
			data.loginnum=set_value_name;
		}catch (...) { 
			data.loginnum=0;
			 }
			break;
		case 11:
		 try{
			data.qrtemp=set_value_name;
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 12:
		 try{
			data.wxuuid=std::to_string(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
			 }
			break;
		case 13:
		 try{
			data.basesitepath=std::to_string(set_value_name);
		}catch (...) { 
			data.basesitepath.clear();
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
			data.nickname=std::to_string(set_value_name);
		}catch (...) { 
			data.nickname.clear();
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
			data.regdate=std::to_string(set_value_name);
		}catch (...) { 
			data.regdate.clear();
			 }
			break;
		case 8:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 9:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 10:
		 try{
			data.loginnum=(unsigned int)set_value_name;
		}catch (...) { 
			data.loginnum=0;
			 }
			break;
		case 11:
		 try{
			data.qrtemp=(unsigned int)set_value_name;
		}catch (...) { 
			data.qrtemp=0;
			 }
			break;
		case 12:
		 try{
			data.wxuuid=std::to_string(set_value_name);
		}catch (...) { 
			data.wxuuid.clear();
			 }
			break;
		case 13:
		 try{
			data.basesitepath=std::to_string(set_value_name);
		}catch (...) { 
			data.basesitepath.clear();
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
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(record[n].nickname)<<"\"";
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
tempsql<<"\"regdate\":\""<<http::utf8_to_jsonstring(record[n].regdate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].loginnum==0){
	tempsql<<"\"loginnum\":0";
 }else{ 
	tempsql<<"\"loginnum\":"<<std::to_string(record[n].loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(record[n].qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(record[n].wxuuid)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"basesitepath\":\""<<http::utf8_to_jsonstring(record[n].basesitepath)<<"\"";
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
tempsql<<"\"nickname\":\""<<http::utf8_to_jsonstring(record[n].nickname)<<"\"";
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
tempsql<<"\"regdate\":\""<<http::utf8_to_jsonstring(record[n].regdate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].loginnum==0){
	tempsql<<"\"loginnum\":0";
 }else{ 
	tempsql<<"\"loginnum\":"<<std::to_string(record[n].loginnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].qrtemp==0){
	tempsql<<"\"qrtemp\":0";
 }else{ 
	tempsql<<"\"qrtemp\":"<<std::to_string(record[n].qrtemp);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"wxuuid\":\""<<http::utf8_to_jsonstring(record[n].wxuuid)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"basesitepath\":\""<<http::utf8_to_jsonstring(record[n].basesitepath)<<"\"";
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

 std::string  getNickname(){  return data.nickname; } 
 std::string & getRefNickname(){  return std::ref(data.nickname); } 
 void setNickname( std::string  &val){  data.nickname=val;} 
 void setNickname(std::string_view val){  data.nickname=val;} 

 char  getIsopen(){  return data.isopen; } 
 void setIsopen( char  val){  data.isopen=val;} 

 unsigned  int  getBegindate(){  return data.begindate; } 
 void setBegindate( unsigned  int  val){  data.begindate=val;} 

 unsigned  int  getEnddate(){  return data.enddate; } 
 void setEnddate( unsigned  int  val){  data.enddate=val;} 

 std::string  getRegdate(){  return data.regdate; } 
 std::string & getRefRegdate(){  return std::ref(data.regdate); } 
 void setRegdate( std::string  &val){  data.regdate=val;} 
 void setRegdate(std::string_view val){  data.regdate=val;} 

 std::string  getMobile(){  return data.mobile; } 
 std::string & getRefMobile(){  return std::ref(data.mobile); } 
 void setMobile( std::string  &val){  data.mobile=val;} 
 void setMobile(std::string_view val){  data.mobile=val;} 

 std::string  getEmail(){  return data.email; } 
 std::string & getRefEmail(){  return std::ref(data.email); } 
 void setEmail( std::string  &val){  data.email=val;} 
 void setEmail(std::string_view val){  data.email=val;} 

 unsigned  int  getLoginnum(){  return data.loginnum; } 
 void setLoginnum( unsigned  int  val){  data.loginnum=val;} 

 unsigned  int  getQrtemp(){  return data.qrtemp; } 
 void setQrtemp( unsigned  int  val){  data.qrtemp=val;} 

 std::string  getWxuuid(){  return data.wxuuid; } 
 std::string & getRefWxuuid(){  return std::ref(data.wxuuid); } 
 void setWxuuid( std::string  &val){  data.wxuuid=val;} 
 void setWxuuid(std::string_view val){  data.wxuuid=val;} 

 std::string  getBasesitepath(){  return data.basesitepath; } 
 std::string & getRefBasesitepath(){  return std::ref(data.basesitepath); } 
 void setBasesitepath( std::string  &val){  data.basesitepath=val;} 
 void setBasesitepath(std::string_view val){  data.basesitepath=val;} 

superadmin_base::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
superadmin_base::meta getData(){
 	 return data; 
} 
std::vector<superadmin_base::meta> getRecord(){
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
		 if(key_name=="nickname")
		{
			return data.nickname;
		}
		 if(key_name=="regdate")
		{
			return data.regdate;
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
		 if(key_name=="basesitepath")
		{
			return data.basesitepath;
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
		 if(key_name=="begindate")
		{
			return data.begindate;
		}
		 if(key_name=="enddate")
		{
			return data.enddate;
		}
		 if(key_name=="loginnum")
		{
			return data.loginnum;
		}
		 if(key_name=="qrtemp")
		{
			return data.qrtemp;
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
 				 a.emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a.emplace_back(iter.enddate);
				 break;
			case 10: 
 				 a.emplace_back(iter.loginnum);
				 break;
			case 11: 
 				 a.emplace_back(iter.qrtemp);
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
 				 return data.begindate;
				 break;
			case 6: 
 				 return data.enddate;
				 break;
			case 10: 
 				 return data.loginnum;
				 break;
			case 11: 
 				 return data.qrtemp;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] superadmin_base::meta & iter,[[maybe_unused]] std::string keyname)
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
 				 return iter.begindate;
				 break;
			case 6: 
 				 return iter.enddate;
				 break;
			case 10: 
 				 return iter.loginnum;
				 break;
			case 11: 
 				 return iter.qrtemp;
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
            T getVal([[maybe_unused]] superadmin_base::meta & iter,std::string keyname)
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
 				 return data.nickname;
				 break;
			case 7: 
 				 return data.regdate;
				 break;
			case 8: 
 				 return data.mobile;
				 break;
			case 9: 
 				 return data.email;
				 break;
			case 12: 
 				 return data.wxuuid;
				 break;
			case 13: 
 				 return data.basesitepath;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] superadmin_base::meta & iter,std::string keyname)
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
 				 return iter.nickname;
				 break;
			case 7: 
 				 return iter.regdate;
				 break;
			case 8: 
 				 return iter.mobile;
				 break;
			case 9: 
 				 return iter.email;
				 break;
			case 12: 
 				 return iter.wxuuid;
				 break;
			case 13: 
 				 return iter.basesitepath;
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
 				 a.emplace_back(iter.nickname);
					 break;
			case 7: 
 				 a.emplace_back(iter.regdate);
					 break;
			case 8: 
 				 a.emplace_back(iter.mobile);
					 break;
			case 9: 
 				 a.emplace_back(iter.email);
					 break;
			case 12: 
 				 a.emplace_back(iter.wxuuid);
					 break;
			case 13: 
 				 a.emplace_back(iter.basesitepath);
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
 				 if(isyinhao){ a<<jsonaddslash(iter.nickname); 
				 }else{
				 a<<iter.nickname;
				 }
				 break;
			case 4: 
 				 a<<std::to_string(iter.isopen);
				 break;
			case 5: 
 				 a<<std::to_string(iter.begindate);
				 break;
			case 6: 
 				 a<<std::to_string(iter.enddate);
				 break;
			case 7: 
 				 if(isyinhao){ a<<jsonaddslash(iter.regdate); 
				 }else{
				 a<<iter.regdate;
				 }
				 break;
			case 8: 
 				 if(isyinhao){ a<<jsonaddslash(iter.mobile); 
				 }else{
				 a<<iter.mobile;
				 }
				 break;
			case 9: 
 				 if(isyinhao){ a<<jsonaddslash(iter.email); 
				 }else{
				 a<<iter.email;
				 }
				 break;
			case 10: 
 				 a<<std::to_string(iter.loginnum);
				 break;
			case 11: 
 				 a<<std::to_string(iter.qrtemp);
				 break;
			case 12: 
 				 if(isyinhao){ a<<jsonaddslash(iter.wxuuid); 
				 }else{
				 a<<iter.wxuuid;
				 }
				 break;
			case 13: 
 				 if(isyinhao){ a<<jsonaddslash(iter.basesitepath); 
				 }else{
				 a<<iter.basesitepath;
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 	 ktemp=iter.begindate;
	 break;
case 6: 
 	 ktemp=iter.enddate;
	 break;
case 10: 
 	 ktemp=iter.loginnum;
	 break;
case 11: 
 	 ktemp=iter.qrtemp;
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 a.emplace(iter.begindate,iter);
				 break;
			case 6: 
 				 a.emplace(iter.enddate,iter);
				 break;
			case 10: 
 				 a.emplace(iter.loginnum,iter);
				 break;
			case 11: 
 				 a.emplace(iter.qrtemp,iter);
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
 				 a.emplace(iter.nickname,iter);
			 break;
			case 7: 
 				 a.emplace(iter.regdate,iter);
			 break;
			case 8: 
 				 a.emplace(iter.mobile,iter);
			 break;
			case 9: 
 				 a.emplace(iter.email,iter);
			 break;
			case 12: 
 				 a.emplace(iter.wxuuid,iter);
			 break;
			case 13: 
 				 a.emplace(iter.basesitepath,iter);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 ktemp=iter.begindate;
			 break;
			case 6: 
 				 ktemp=iter.enddate;
			 break;
			case 10: 
 				 ktemp=iter.loginnum;
			 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 	 ktemp=iter.nickname;
	 break;
case 7: 
 	 ktemp=iter.regdate;
	 break;
case 8: 
 	 ktemp=iter.mobile;
	 break;
case 9: 
 	 ktemp=iter.email;
	 break;
case 12: 
 	 ktemp=iter.wxuuid;
	 break;
case 13: 
 	 ktemp=iter.basesitepath;
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
 	 vtemp=iter.nickname;
	 break;
case 7: 
 	 vtemp=iter.regdate;
	 break;
case 8: 
 	 vtemp=iter.mobile;
	 break;
case 9: 
 	 vtemp=iter.email;
	 break;
case 12: 
 	 vtemp=iter.wxuuid;
	 break;
case 13: 
 	 vtemp=iter.basesitepath;
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
 	 a.emplace_back(iter.begindate,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.enddate,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.loginnum,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.qrtemp,iter);
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
 	 a.emplace_back(iter.nickname,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.regdate,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.mobile,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.email,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.wxuuid,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.basesitepath,iter);
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.loginnum);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.qrtemp);
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 a[ktemp][vtemp].emplace_back(iter.nickname);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.wxuuid);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.basesitepath);
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.loginnum);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.qrtemp);
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 a[ktemp][vtemp].emplace_back(iter.nickname);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.wxuuid);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.basesitepath);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.loginnum);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.qrtemp);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.begindate;
				 break;
			case 6: 
 				 vtemp=iter.enddate;
				 break;
			case 10: 
 				 vtemp=iter.loginnum;
				 break;
			case 11: 
 				 vtemp=iter.qrtemp;
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
 				 a[ktemp][vtemp].emplace_back(iter.nickname);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.wxuuid);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.basesitepath);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 a[ktemp][vtemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.loginnum);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.qrtemp);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 vtemp=iter.nickname;
				 break;
			case 7: 
 				 vtemp=iter.regdate;
				 break;
			case 8: 
 				 vtemp=iter.mobile;
				 break;
			case 9: 
 				 vtemp=iter.email;
				 break;
			case 12: 
 				 vtemp=iter.wxuuid;
				 break;
			case 13: 
 				 vtemp=iter.basesitepath;
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
 				 a[ktemp][vtemp].emplace_back(iter.nickname);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.wxuuid);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.basesitepath);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 a[ktemp].emplace_back(iter.nickname);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.mobile);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.email);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.wxuuid);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.basesitepath);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 a[ktemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.enddate);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.loginnum);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.qrtemp);
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 a[ktemp].emplace_back(iter.nickname);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.regdate);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.mobile);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.email);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.wxuuid);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.basesitepath);
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 a[ktemp].emplace_back(iter.begindate);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.enddate);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.loginnum);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.qrtemp);
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
 				 a[iter.begindate].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.enddate].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.loginnum].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.qrtemp].emplace_back(iter);
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
 				 a[iter.nickname].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.regdate].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.mobile].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.email].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.wxuuid].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.basesitepath].emplace_back(iter);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 a[ktemp][iter.nickname].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.regdate].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.mobile].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.email].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.wxuuid].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.basesitepath].emplace_back(iter);
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
 				 ktemp=iter.nickname;
				 break;
			case 7: 
 				 ktemp=iter.regdate;
				 break;
			case 8: 
 				 ktemp=iter.mobile;
				 break;
			case 9: 
 				 ktemp=iter.email;
				 break;
			case 12: 
 				 ktemp=iter.wxuuid;
				 break;
			case 13: 
 				 ktemp=iter.basesitepath;
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
 				 a[ktemp][iter.begindate].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.enddate].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.loginnum].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.qrtemp].emplace_back(iter);
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 a[ktemp][iter.begindate].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.enddate].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.loginnum].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.qrtemp].emplace_back(iter);
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
 				 ktemp=iter.begindate;
				 break;
			case 6: 
 				 ktemp=iter.enddate;
				 break;
			case 10: 
 				 ktemp=iter.loginnum;
				 break;
			case 11: 
 				 ktemp=iter.qrtemp;
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
 				 a[ktemp][iter.nickname].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.regdate].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.mobile].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.email].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.wxuuid].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.basesitepath].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   