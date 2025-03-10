#ifndef ORM_CMS_LOGININFOBASEMATA_H
#define ORM_CMS_LOGININFOBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Mon, 10 Mar 2025 03:03:53 GMT
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

struct logininfobase
{
    struct meta{
     unsigned  int  lgid = 0; ///**/
 unsigned  int  userid = 0; ///*会员id*/
 unsigned  char  logtype = 0; ///**/
 std::string  username = ""; ///*会员名字*/
 std::string  addtime = ""; ///*登录时间*/
 std::string  addip = ""; ///*登录ip*/
 std::string  addregion = ""; ///*登录地区*/
 std::string  loginstate = ""; ///*登录状态*/
 std::string  agent = ""; ///**/
 std::string  urlpath = ""; ///**/
 } data;
 std::vector<logininfobase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<logininfobase::meta>::iterator begin(){     return record.begin(); }
std::vector<logininfobase::meta>::iterator end(){     return record.end(); }
std::vector<logininfobase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<logininfobase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,10> col_names={"lgid","userid","logtype","username","addtime","addip","addregion","loginstate","agent","urlpath"};
static constexpr std::array<unsigned char,10> col_types={3,3,1,253,253,253,253,253,253,253};
static constexpr std::array<unsigned char,10> col_length={0,0,0,40,20,70,70,20,120,120};
static constexpr std::array<unsigned char,10> col_decimals={0,0,0,0,0,0,0,0,0,0};
std::string tablename="logininfo";
static constexpr std::string_view modelname="Logininfo";

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
case 5:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='p'){ return 5; }
 if(colpospppc=='t'){ return 8; }
   	 break;
case 7:
   	 return 4;
break;
case 9:
   	 return 6;
break;
 }
 break;
case 'l':
 switch(coln.size()){  
case 4:
   	 return 0;
break;
case 7:
   	 return 2;
break;
case 10:
   	 return 7;
break;
 }
 break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 1;
break;
case 7:
   	 return 9;
break;
case 8:
   	 return 3;
break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "lgid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     logininfobase::meta metatemp;    
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

        if(data.lgid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.lgid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.logtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.logtype);
}
tempsql<<",'"<<stringaddslash(data.username)<<"'";
tempsql<<",'"<<stringaddslash(data.addtime)<<"'";
tempsql<<",'"<<stringaddslash(data.addip)<<"'";
tempsql<<",'"<<stringaddslash(data.addregion)<<"'";
tempsql<<",'"<<stringaddslash(data.loginstate)<<"'";
tempsql<<",'"<<stringaddslash(data.agent)<<"'";
tempsql<<",'"<<stringaddslash(data.urlpath)<<"'";
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

        if(insert_data.lgid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.lgid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.logtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.logtype);
}
tempsql<<",'"<<stringaddslash(insert_data.username)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.addtime)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.addip)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.addregion)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.loginstate)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.agent)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.urlpath)<<"'";
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


            	if(insert_data[i].lgid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].lgid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].logtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].logtype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].username)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].addtime)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].addip)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].addregion)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].loginstate)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].agent)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].urlpath)<<"'";
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

        if(data.lgid==0){
	tempsql<<"`lgid`=0";
 }else{ 
	tempsql<<"`lgid`="<<std::to_string(data.lgid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.logtype==0){
	tempsql<<",`logtype`=0";
 }else{ 
	tempsql<<",`logtype`="<<std::to_string(data.logtype);
}
tempsql<<",`username`='"<<stringaddslash(data.username)<<"'";
tempsql<<",`addtime`='"<<stringaddslash(data.addtime)<<"'";
tempsql<<",`addip`='"<<stringaddslash(data.addip)<<"'";
tempsql<<",`addregion`='"<<stringaddslash(data.addregion)<<"'";
tempsql<<",`loginstate`='"<<stringaddslash(data.loginstate)<<"'";
tempsql<<",`agent`='"<<stringaddslash(data.agent)<<"'";
tempsql<<",`urlpath`='"<<stringaddslash(data.urlpath)<<"'";
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
if(data.lgid==0){
	tempsql<<"`lgid`=0";
 }else{ 
	tempsql<<"`lgid`="<<std::to_string(data.lgid);
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
if(data.logtype==0){
	tempsql<<"`logtype`=0";
 }else{ 
	tempsql<<"`logtype`="<<std::to_string(data.logtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`username`='"<<stringaddslash(data.username)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`addtime`='"<<stringaddslash(data.addtime)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`addip`='"<<stringaddslash(data.addip)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`addregion`='"<<stringaddslash(data.addregion)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`loginstate`='"<<stringaddslash(data.loginstate)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`agent`='"<<stringaddslash(data.agent)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`urlpath`='"<<stringaddslash(data.urlpath)<<"'";
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
            	if(record[i].lgid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].lgid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].logtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].logtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].username)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addtime)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addip)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addregion)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].loginstate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].agent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].urlpath)<<"'";
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
            	if(record[i].lgid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].lgid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].logtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].logtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].username)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addtime)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addip)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addregion)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].loginstate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].agent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].urlpath)<<"'";
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
if(data.lgid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.lgid));
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
if(data.logtype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.logtype));
}
 break;
 case 3:
	temparray.push_back(data.username);
 break;
 case 4:
	temparray.push_back(data.addtime);
 break;
 case 5:
	temparray.push_back(data.addip);
 break;
 case 6:
	temparray.push_back(data.addregion);
 break;
 case 7:
	temparray.push_back(data.loginstate);
 break;
 case 8:
	temparray.push_back(data.agent);
 break;
 case 9:
	temparray.push_back(data.urlpath);
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
if(data.lgid==0){
	tempsql.insert({"lgid","0"});
 }else{ 
	tempsql.insert({"lgid",std::to_string(data.lgid)});
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
if(data.logtype==0){
	tempsql.insert({"logtype","0"});
 }else{ 
	tempsql.insert({"logtype",std::to_string(data.logtype)});
}
 break;
 case 3:
	tempsql.insert({"username",data.username});
 break;
 case 4:
	tempsql.insert({"addtime",data.addtime});
 break;
 case 5:
	tempsql.insert({"addip",data.addip});
 break;
 case 6:
	tempsql.insert({"addregion",data.addregion});
 break;
 case 7:
	tempsql.insert({"loginstate",data.loginstate});
 break;
 case 8:
	tempsql.insert({"agent",data.agent});
 break;
 case 9:
	tempsql.insert({"urlpath",data.urlpath});
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
if(data.lgid==0){
	tempsql<<"\"lgid\":0";
 }else{ 
	tempsql<<"\"lgid\":"<<std::to_string(data.lgid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.logtype==0){
	tempsql<<",\"logtype\":0";
 }else{ 
	tempsql<<",\"logtype\":"<<std::to_string(data.logtype);
}
tempsql<<",\"username\":\""<<http::utf8_to_jsonstring(data.username);
tempsql<<"\"";
tempsql<<",\"addtime\":\""<<http::utf8_to_jsonstring(data.addtime);
tempsql<<"\"";
tempsql<<",\"addip\":\""<<http::utf8_to_jsonstring(data.addip);
tempsql<<"\"";
tempsql<<",\"addregion\":\""<<http::utf8_to_jsonstring(data.addregion);
tempsql<<"\"";
tempsql<<",\"loginstate\":\""<<http::utf8_to_jsonstring(data.loginstate);
tempsql<<"\"";
tempsql<<",\"agent\":\""<<http::utf8_to_jsonstring(data.agent);
tempsql<<"\"";
tempsql<<",\"urlpath\":\""<<http::utf8_to_jsonstring(data.urlpath);
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
if(data.lgid==0){
	tempsql<<"\"lgid\":0";
 }else{ 
	tempsql<<"\"lgid\":"<<std::to_string(data.lgid);
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
if(data.logtype==0){
	tempsql<<"\"logtype\":0";
 }else{ 
	tempsql<<"\"logtype\":"<<std::to_string(data.logtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"username\":\""<<http::utf8_to_jsonstring(data.username)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addtime\":\""<<http::utf8_to_jsonstring(data.addtime)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(data.addip)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addregion\":\""<<http::utf8_to_jsonstring(data.addregion)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"loginstate\":\""<<http::utf8_to_jsonstring(data.loginstate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"agent\":\""<<http::utf8_to_jsonstring(data.agent)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(data.urlpath)<<"\"";
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
        logininfobase::meta metatemp; 
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
			data.lgid=std::stoul(set_value_name);
		}catch (...) { 
			data.lgid=0;
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
			data.logtype=std::stoi(set_value_name);
		}catch (...) { 
			data.logtype=0;
			 }
			break;
		case 3:
		 try{
			data.username.append(set_value_name);
		}catch (...) { 
			data.username.clear();
			 }
			break;
		case 4:
		 try{
			data.addtime.append(set_value_name);
		}catch (...) { 
			data.addtime.clear();
			 }
			break;
		case 5:
		 try{
			data.addip.append(set_value_name);
		}catch (...) { 
			data.addip.clear();
			 }
			break;
		case 6:
		 try{
			data.addregion.append(set_value_name);
		}catch (...) { 
			data.addregion.clear();
			 }
			break;
		case 7:
		 try{
			data.loginstate.append(set_value_name);
		}catch (...) { 
			data.loginstate.clear();
			 }
			break;
		case 8:
		 try{
			data.agent.append(set_value_name);
		}catch (...) { 
			data.agent.clear();
			 }
			break;
		case 9:
		 try{
			data.urlpath.append(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
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
			data.lgid=set_value_name;
		}catch (...) { 
			data.lgid=0;
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
			data.logtype=set_value_name;
		}catch (...) { 
			data.logtype=0;
			 }
			break;
		case 3:
		 try{
			data.username=std::to_string(set_value_name);
		}catch (...) { 
			data.username.clear();
			 }
			break;
		case 4:
		 try{
			data.addtime=std::to_string(set_value_name);
		}catch (...) { 
			data.addtime.clear();
			 }
			break;
		case 5:
		 try{
			data.addip=std::to_string(set_value_name);
		}catch (...) { 
			data.addip.clear();
			 }
			break;
		case 6:
		 try{
			data.addregion=std::to_string(set_value_name);
		}catch (...) { 
			data.addregion.clear();
			 }
			break;
		case 7:
		 try{
			data.loginstate=std::to_string(set_value_name);
		}catch (...) { 
			data.loginstate.clear();
			 }
			break;
		case 8:
		 try{
			data.agent=std::to_string(set_value_name);
		}catch (...) { 
			data.agent.clear();
			 }
			break;
		case 9:
		 try{
			data.urlpath=std::to_string(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
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
			data.lgid=(unsigned int)set_value_name;
		}catch (...) { 
			data.lgid=0;
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
			data.logtype=(int)set_value_name;
		}catch (...) { 
			data.logtype=0;
			 }
			break;
		case 3:
		 try{
			data.username=std::to_string(set_value_name);
		}catch (...) { 
			data.username.clear();
			 }
			break;
		case 4:
		 try{
			data.addtime=std::to_string(set_value_name);
		}catch (...) { 
			data.addtime.clear();
			 }
			break;
		case 5:
		 try{
			data.addip=std::to_string(set_value_name);
		}catch (...) { 
			data.addip.clear();
			 }
			break;
		case 6:
		 try{
			data.addregion=std::to_string(set_value_name);
		}catch (...) { 
			data.addregion.clear();
			 }
			break;
		case 7:
		 try{
			data.loginstate=std::to_string(set_value_name);
		}catch (...) { 
			data.loginstate.clear();
			 }
			break;
		case 8:
		 try{
			data.agent=std::to_string(set_value_name);
		}catch (...) { 
			data.agent.clear();
			 }
			break;
		case 9:
		 try{
			data.urlpath=std::to_string(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
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
if(record[n].lgid==0){
	tempsql<<"\"lgid\":0";
 }else{ 
	tempsql<<"\"lgid\":"<<std::to_string(record[n].lgid);
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
if(record[n].logtype==0){
	tempsql<<"\"logtype\":0";
 }else{ 
	tempsql<<"\"logtype\":"<<std::to_string(record[n].logtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"username\":\""<<http::utf8_to_jsonstring(record[n].username)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addtime\":\""<<http::utf8_to_jsonstring(record[n].addtime)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(record[n].addip)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addregion\":\""<<http::utf8_to_jsonstring(record[n].addregion)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"loginstate\":\""<<http::utf8_to_jsonstring(record[n].loginstate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"agent\":\""<<http::utf8_to_jsonstring(record[n].agent)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(record[n].urlpath)<<"\"";
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
if(record[n].lgid==0){
	tempsql<<"\"lgid\":0";
 }else{ 
	tempsql<<"\"lgid\":"<<std::to_string(record[n].lgid);
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
if(record[n].logtype==0){
	tempsql<<"\"logtype\":0";
 }else{ 
	tempsql<<"\"logtype\":"<<std::to_string(record[n].logtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"username\":\""<<http::utf8_to_jsonstring(record[n].username)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addtime\":\""<<http::utf8_to_jsonstring(record[n].addtime)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(record[n].addip)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addregion\":\""<<http::utf8_to_jsonstring(record[n].addregion)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"loginstate\":\""<<http::utf8_to_jsonstring(record[n].loginstate)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"agent\":\""<<http::utf8_to_jsonstring(record[n].agent)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(record[n].urlpath)<<"\"";
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
   long long getPK(){  return data.lgid; } 
 void setPK(long long val){  data.lgid=val;} 
 unsigned  int  getLgid(){  return data.lgid; } 
 void setLgid( unsigned  int  val){  data.lgid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  char  getLogtype(){  return data.logtype; } 
 void setLogtype( unsigned  char  val){  data.logtype=val;} 

 std::string  getUsername(){  return data.username; } 
 std::string & getRefUsername(){  return std::ref(data.username); } 
 void setUsername( std::string  &val){  data.username=val;} 
 void setUsername(std::string_view val){  data.username=val;} 

 std::string  getAddtime(){  return data.addtime; } 
 std::string & getRefAddtime(){  return std::ref(data.addtime); } 
 void setAddtime( std::string  &val){  data.addtime=val;} 
 void setAddtime(std::string_view val){  data.addtime=val;} 

 std::string  getAddip(){  return data.addip; } 
 std::string & getRefAddip(){  return std::ref(data.addip); } 
 void setAddip( std::string  &val){  data.addip=val;} 
 void setAddip(std::string_view val){  data.addip=val;} 

 std::string  getAddregion(){  return data.addregion; } 
 std::string & getRefAddregion(){  return std::ref(data.addregion); } 
 void setAddregion( std::string  &val){  data.addregion=val;} 
 void setAddregion(std::string_view val){  data.addregion=val;} 

 std::string  getLoginstate(){  return data.loginstate; } 
 std::string & getRefLoginstate(){  return std::ref(data.loginstate); } 
 void setLoginstate( std::string  &val){  data.loginstate=val;} 
 void setLoginstate(std::string_view val){  data.loginstate=val;} 

 std::string  getAgent(){  return data.agent; } 
 std::string & getRefAgent(){  return std::ref(data.agent); } 
 void setAgent( std::string  &val){  data.agent=val;} 
 void setAgent(std::string_view val){  data.agent=val;} 

 std::string  getUrlpath(){  return data.urlpath; } 
 std::string & getRefUrlpath(){  return std::ref(data.urlpath); } 
 void setUrlpath( std::string  &val){  data.urlpath=val;} 
 void setUrlpath(std::string_view val){  data.urlpath=val;} 

logininfobase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
logininfobase::meta getData(){
 	 return data; 
} 
std::vector<logininfobase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="username")
		{
			return data.username;
		}
		 if(key_name=="addtime")
		{
			return data.addtime;
		}
		 if(key_name=="addip")
		{
			return data.addip;
		}
		 if(key_name=="addregion")
		{
			return data.addregion;
		}
		 if(key_name=="loginstate")
		{
			return data.loginstate;
		}
		 if(key_name=="agent")
		{
			return data.agent;
		}
		 if(key_name=="urlpath")
		{
			return data.urlpath;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="lgid")
		{
			return data.lgid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="logtype")
		{
			return data.logtype;
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
 				 a.emplace_back(iter.lgid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.logtype);
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
 				 return data.lgid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.logtype;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] logininfobase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.lgid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.logtype;
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
            T getVal([[maybe_unused]] logininfobase::meta & iter,std::string keyname)
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
 				 return data.username;
				 break;
			case 4: 
 				 return data.addtime;
				 break;
			case 5: 
 				 return data.addip;
				 break;
			case 6: 
 				 return data.addregion;
				 break;
			case 7: 
 				 return data.loginstate;
				 break;
			case 8: 
 				 return data.agent;
				 break;
			case 9: 
 				 return data.urlpath;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] logininfobase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 3: 
 				 return iter.username;
				 break;
			case 4: 
 				 return iter.addtime;
				 break;
			case 5: 
 				 return iter.addip;
				 break;
			case 6: 
 				 return iter.addregion;
				 break;
			case 7: 
 				 return iter.loginstate;
				 break;
			case 8: 
 				 return iter.agent;
				 break;
			case 9: 
 				 return iter.urlpath;
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
 				 a.emplace_back(iter.username);
					 break;
			case 4: 
 				 a.emplace_back(iter.addtime);
					 break;
			case 5: 
 				 a.emplace_back(iter.addip);
					 break;
			case 6: 
 				 a.emplace_back(iter.addregion);
					 break;
			case 7: 
 				 a.emplace_back(iter.loginstate);
					 break;
			case 8: 
 				 a.emplace_back(iter.agent);
					 break;
			case 9: 
 				 a.emplace_back(iter.urlpath);
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
 				 a<<std::to_string(iter.lgid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.logtype);
				 break;
			case 3: 
 				 if(isyinhao){ a<<jsonaddslash(iter.username); 
				 }else{
				 a<<iter.username;
				 }
				 break;
			case 4: 
 				 if(isyinhao){ a<<jsonaddslash(iter.addtime); 
				 }else{
				 a<<iter.addtime;
				 }
				 break;
			case 5: 
 				 if(isyinhao){ a<<jsonaddslash(iter.addip); 
				 }else{
				 a<<iter.addip;
				 }
				 break;
			case 6: 
 				 if(isyinhao){ a<<jsonaddslash(iter.addregion); 
				 }else{
				 a<<iter.addregion;
				 }
				 break;
			case 7: 
 				 if(isyinhao){ a<<jsonaddslash(iter.loginstate); 
				 }else{
				 a<<iter.loginstate;
				 }
				 break;
			case 8: 
 				 if(isyinhao){ a<<jsonaddslash(iter.agent); 
				 }else{
				 a<<iter.agent;
				 }
				 break;
			case 9: 
 				 if(isyinhao){ a<<jsonaddslash(iter.urlpath); 
				 }else{
				 a<<iter.urlpath;
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

   			case 3: 
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
				 } 
			switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
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
 	 ktemp=iter.lgid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.logtype;
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			  }
 			switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
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
 				 a.emplace(iter.lgid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.logtype,iter);
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
 				 a.emplace(iter.username,iter);
			 break;
			case 4: 
 				 a.emplace(iter.addtime,iter);
			 break;
			case 5: 
 				 a.emplace(iter.addip,iter);
			 break;
			case 6: 
 				 a.emplace(iter.addregion,iter);
			 break;
			case 7: 
 				 a.emplace(iter.loginstate,iter);
			 break;
			case 8: 
 				 a.emplace(iter.agent,iter);
			 break;
			case 9: 
 				 a.emplace(iter.urlpath,iter);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
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
 				 ktemp=iter.lgid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.logtype;
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			  }
 			switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
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
 	 ktemp=iter.username;
	 break;
case 4: 
 	 ktemp=iter.addtime;
	 break;
case 5: 
 	 ktemp=iter.addip;
	 break;
case 6: 
 	 ktemp=iter.addregion;
	 break;
case 7: 
 	 ktemp=iter.loginstate;
	 break;
case 8: 
 	 ktemp=iter.agent;
	 break;
case 9: 
 	 ktemp=iter.urlpath;
	 break;
	  }
 switch(vpos){
case 3: 
 	 vtemp=iter.username;
	 break;
case 4: 
 	 vtemp=iter.addtime;
	 break;
case 5: 
 	 vtemp=iter.addip;
	 break;
case 6: 
 	 vtemp=iter.addregion;
	 break;
case 7: 
 	 vtemp=iter.loginstate;
	 break;
case 8: 
 	 vtemp=iter.agent;
	 break;
case 9: 
 	 vtemp=iter.urlpath;
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
 	 a.emplace_back(iter.lgid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.logtype,iter);
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
 	 a.emplace_back(iter.username,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.addtime,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.addip,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.addregion,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.loginstate,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.agent,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.urlpath,iter);
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.lgid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.logtype);
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
				 break;
			 }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.username);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.addip);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.addregion);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.loginstate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.agent);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.lgid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.logtype);
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
				 break;
			  }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.username);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.addip);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.addregion);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.loginstate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.agent);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.lgid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.logtype);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.lgid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.logtype;
				 break;
			 }

			switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.username);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.addip);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.addregion);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.loginstate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.agent);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.lgid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.logtype);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.username;
				 break;
			case 4: 
 				 vtemp=iter.addtime;
				 break;
			case 5: 
 				 vtemp=iter.addip;
				 break;
			case 6: 
 				 vtemp=iter.addregion;
				 break;
			case 7: 
 				 vtemp=iter.loginstate;
				 break;
			case 8: 
 				 vtemp=iter.agent;
				 break;
			case 9: 
 				 vtemp=iter.urlpath;
				 break;
			  }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.username);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.addip);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.addregion);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.loginstate);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.agent);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 a[ktemp].emplace_back(iter.username);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.addtime);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.addip);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.addregion);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.loginstate);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.agent);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.urlpath);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.lgid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.logtype);
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 a[ktemp].emplace_back(iter.username);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.addtime);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.addip);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.addregion);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.loginstate);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.agent);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.urlpath);
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.lgid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.logtype);
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
 				 a[iter.lgid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.logtype].emplace_back(iter);
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
 				 a[iter.username].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.addtime].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.addip].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.addregion].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.loginstate].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.agent].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.urlpath].emplace_back(iter);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 a[ktemp][iter.username].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.addtime].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.addip].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.addregion].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.loginstate].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.agent].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.urlpath].emplace_back(iter);
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
 				 ktemp=iter.username;
				 break;
			case 4: 
 				 ktemp=iter.addtime;
				 break;
			case 5: 
 				 ktemp=iter.addip;
				 break;
			case 6: 
 				 ktemp=iter.addregion;
				 break;
			case 7: 
 				 ktemp=iter.loginstate;
				 break;
			case 8: 
 				 ktemp=iter.agent;
				 break;
			case 9: 
 				 ktemp=iter.urlpath;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.lgid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.logtype].emplace_back(iter);
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.lgid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.logtype].emplace_back(iter);
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
 				 ktemp=iter.lgid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.logtype;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 a[ktemp][iter.username].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.addtime].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.addip].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.addregion].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.loginstate].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.agent].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.urlpath].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   