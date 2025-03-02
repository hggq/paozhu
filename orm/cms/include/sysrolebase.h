#ifndef ORM_CMS_SYSROLEBASEMATA_H
#define ORM_CMS_SYSROLEBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Sun, 02 Mar 2025 08:24:43 GMT
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

struct sysrolebase
{
    struct meta{
     unsigned  int  roleid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 std::string  name = ""; ///**/
 unsigned  char  status = 0; ///**/
 unsigned  long long  rolecode = 0; ///**/
 int  sortid = 0; ///**/
 unsigned  int  rolevalue = 0; ///**/
 } data;
 std::vector<sysrolebase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<sysrolebase::meta>::iterator begin(){     return record.begin(); }
std::vector<sysrolebase::meta>::iterator end(){     return record.end(); }
std::vector<sysrolebase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<sysrolebase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,7> col_names={"roleid","userid","name","status","rolecode","sortid","rolevalue"};
static constexpr std::array<unsigned char,7> col_types={3,3,253,1,8,3,3};
static constexpr std::array<unsigned char,7> col_length={0,0,60,0,0,0,0};
static constexpr std::array<unsigned char,7> col_decimals={0,0,0,0,0,0,0};
std::string tablename="sysrole";
static constexpr std::string_view modelname="Sysrole";

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


         case 'n':
   	 return 2;
break;
case 'r':
 switch(coln.size()){  
case 6:
   	 return 0;
break;
case 8:
   	 return 4;
break;
case 9:
   	 return 6;
break;
 }
 break;
case 's':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 5; }
 if(colpospppc=='s'){ return 3; }
   	 break;
 }
 break;
case 'u':
   	 return 1;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "roleid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     sysrolebase::meta metatemp;    
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

        if(data.roleid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.roleid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
tempsql<<",'"<<stringaddslash(data.name)<<"'";
if(data.status==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.status);
}
if(data.rolecode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.rolecode);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
if(data.rolevalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.rolevalue);
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

        if(insert_data.roleid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.roleid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
if(insert_data.status==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.status);
}
if(insert_data.rolecode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.rolecode);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
if(insert_data.rolevalue==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.rolevalue);
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


            	if(insert_data[i].roleid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].roleid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
	if(insert_data[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].status);
	}
	if(insert_data[i].rolecode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].rolecode);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
	if(insert_data[i].rolevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].rolevalue);
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

        if(data.roleid==0){
	tempsql<<"`roleid`=0";
 }else{ 
	tempsql<<"`roleid`="<<std::to_string(data.roleid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
if(data.status==0){
	tempsql<<",`status`=0";
 }else{ 
	tempsql<<",`status`="<<std::to_string(data.status);
}
if(data.rolecode==0){
	tempsql<<",`rolecode`=0";
 }else{ 
	tempsql<<",`rolecode`="<<std::to_string(data.rolecode);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
if(data.rolevalue==0){
	tempsql<<",`rolevalue`=0";
 }else{ 
	tempsql<<",`rolevalue`="<<std::to_string(data.rolevalue);
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
if(data.roleid==0){
	tempsql<<"`roleid`=0";
 }else{ 
	tempsql<<"`roleid`="<<std::to_string(data.roleid);
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
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.status==0){
	tempsql<<"`status`=0";
 }else{ 
	tempsql<<"`status`="<<std::to_string(data.status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.rolecode==0){
	tempsql<<"`rolecode`=0";
 }else{ 
	tempsql<<"`rolecode`="<<std::to_string(data.rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.rolevalue==0){
	tempsql<<"`rolevalue`=0";
 }else{ 
	tempsql<<"`rolevalue`="<<std::to_string(data.rolevalue);
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
            	if(record[i].roleid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].roleid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].status);
	}
	if(record[i].rolecode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rolecode);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	if(record[i].rolevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rolevalue);
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
            	if(record[i].roleid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].roleid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].status==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].status);
	}
	if(record[i].rolecode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rolecode);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	if(record[i].rolevalue==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].rolevalue);
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
if(data.roleid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.roleid));
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
	temparray.push_back(data.name);
 break;
 case 3:
if(data.status==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.status));
}
 break;
 case 4:
if(data.rolecode==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.rolecode));
}
 break;
 case 5:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 6:
if(data.rolevalue==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.rolevalue));
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
if(data.roleid==0){
	tempsql.insert({"roleid","0"});
 }else{ 
	tempsql.insert({"roleid",std::to_string(data.roleid)});
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
	tempsql.insert({"name",data.name});
 break;
 case 3:
if(data.status==0){
	tempsql.insert({"status","0"});
 }else{ 
	tempsql.insert({"status",std::to_string(data.status)});
}
 break;
 case 4:
if(data.rolecode==0){
	tempsql.insert({"rolecode","0"});
 }else{ 
	tempsql.insert({"rolecode",std::to_string(data.rolecode)});
}
 break;
 case 5:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 6:
if(data.rolevalue==0){
	tempsql.insert({"rolevalue","0"});
 }else{ 
	tempsql.insert({"rolevalue",std::to_string(data.rolevalue)});
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
if(data.roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(data.roleid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
if(data.status==0){
	tempsql<<",\"status\":0";
 }else{ 
	tempsql<<",\"status\":"<<std::to_string(data.status);
}
if(data.rolecode==0){
	tempsql<<",\"rolecode\":0";
 }else{ 
	tempsql<<",\"rolecode\":"<<std::to_string(data.rolecode);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
if(data.rolevalue==0){
	tempsql<<",\"rolevalue\":0";
 }else{ 
	tempsql<<",\"rolevalue\":"<<std::to_string(data.rolevalue);
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
if(data.roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(data.roleid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(data.status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.rolecode==0){
	tempsql<<"\"rolecode\":0";
 }else{ 
	tempsql<<"\"rolecode\":"<<std::to_string(data.rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.rolevalue==0){
	tempsql<<"\"rolevalue\":0";
 }else{ 
	tempsql<<"\"rolevalue\":"<<std::to_string(data.rolevalue);
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
        sysrolebase::meta metatemp; 
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
			data.roleid=std::stoul(set_value_name);
		}catch (...) { 
			data.roleid=0;
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
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.status=std::stoi(set_value_name);
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 4:
		 try{
			data.rolecode=std::stoull(set_value_name);
		}catch (...) { 
			data.rolecode=0;
			 }
			break;
		case 5:
		 try{
			data.sortid=std::stoi(set_value_name);
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 6:
		 try{
			data.rolevalue=std::stoul(set_value_name);
		}catch (...) { 
			data.rolevalue=0;
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
			data.roleid=set_value_name;
		}catch (...) { 
			data.roleid=0;
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
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.status=set_value_name;
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 4:
		 try{
			data.rolecode=set_value_name;
		}catch (...) { 
			data.rolecode=0;
			 }
			break;
		case 5:
		 try{
			data.sortid=set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 6:
		 try{
			data.rolevalue=set_value_name;
		}catch (...) { 
			data.rolevalue=0;
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
			data.roleid=(unsigned int)set_value_name;
		}catch (...) { 
			data.roleid=0;
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
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 3:
		 try{
			data.status=(int)set_value_name;
		}catch (...) { 
			data.status=0;
			 }
			break;
		case 4:
		 try{
			data.rolecode=(unsigned long long)set_value_name;
		}catch (...) { 
			data.rolecode=0;
			 }
			break;
		case 5:
		 try{
			data.sortid=(int)set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 6:
		 try{
			data.rolevalue=(unsigned int)set_value_name;
		}catch (...) { 
			data.rolevalue=0;
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
if(record[n].roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(record[n].roleid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(record[n].status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].rolecode==0){
	tempsql<<"\"rolecode\":0";
 }else{ 
	tempsql<<"\"rolecode\":"<<std::to_string(record[n].rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].rolevalue==0){
	tempsql<<"\"rolevalue\":0";
 }else{ 
	tempsql<<"\"rolevalue\":"<<std::to_string(record[n].rolevalue);
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
if(record[n].roleid==0){
	tempsql<<"\"roleid\":0";
 }else{ 
	tempsql<<"\"roleid\":"<<std::to_string(record[n].roleid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].status==0){
	tempsql<<"\"status\":0";
 }else{ 
	tempsql<<"\"status\":"<<std::to_string(record[n].status);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].rolecode==0){
	tempsql<<"\"rolecode\":0";
 }else{ 
	tempsql<<"\"rolecode\":"<<std::to_string(record[n].rolecode);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].rolevalue==0){
	tempsql<<"\"rolevalue\":0";
 }else{ 
	tempsql<<"\"rolevalue\":"<<std::to_string(record[n].rolevalue);
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
   long long getPK(){  return data.roleid; } 
 void setPK(long long val){  data.roleid=val;} 
 unsigned  int  getRoleid(){  return data.roleid; } 
 void setRoleid( unsigned  int  val){  data.roleid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 unsigned  char  getStatus(){  return data.status; } 
 void setStatus( unsigned  char  val){  data.status=val;} 

 unsigned  long long  getRolecode(){  return data.rolecode; } 
 void setRolecode( unsigned  long long  val){  data.rolecode=val;} 

 int  getSortid(){  return data.sortid; } 
 void setSortid( int  val){  data.sortid=val;} 

 unsigned  int  getRolevalue(){  return data.rolevalue; } 
 void setRolevalue( unsigned  int  val){  data.rolevalue=val;} 

sysrolebase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
sysrolebase::meta getData(){
 	 return data; 
} 
std::vector<sysrolebase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="name")
		{
			return data.name;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="roleid")
		{
			return data.roleid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="status")
		{
			return data.status;
		}
		 if(key_name=="rolecode")
		{
			return data.rolecode;
		}
		 if(key_name=="sortid")
		{
			return data.sortid;
		}
		 if(key_name=="rolevalue")
		{
			return data.rolevalue;
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
 				 a.emplace_back(iter.roleid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 3: 
 				 a.emplace_back(iter.status);
				 break;
			case 4: 
 				 a.emplace_back(iter.rolecode);
				 break;
			case 5: 
 				 a.emplace_back(iter.sortid);
				 break;
			case 6: 
 				 a.emplace_back(iter.rolevalue);
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
 				 return data.roleid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 3: 
 				 return data.status;
				 break;
			case 4: 
 				 return data.rolecode;
				 break;
			case 5: 
 				 return data.sortid;
				 break;
			case 6: 
 				 return data.rolevalue;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] sysrolebase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.roleid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 3: 
 				 return iter.status;
				 break;
			case 4: 
 				 return iter.rolecode;
				 break;
			case 5: 
 				 return iter.sortid;
				 break;
			case 6: 
 				 return iter.rolevalue;
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
            T getVal([[maybe_unused]] sysrolebase::meta & iter,std::string keyname)
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

   			case 2: 
 				 return data.name;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] sysrolebase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 2: 
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

    			case 2: 
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
 				 a<<std::to_string(iter.roleid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 if(isyinhao){ a<<jsonaddslash(iter.name); 
				 }else{
				 a<<iter.name;
				 }
				 break;
			case 3: 
 				 a<<std::to_string(iter.status);
				 break;
			case 4: 
 				 a<<std::to_string(iter.rolecode);
				 break;
			case 5: 
 				 a<<std::to_string(iter.sortid);
				 break;
			case 6: 
 				 a<<std::to_string(iter.rolevalue);
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
				 } 
			switch(vpos){
			case 2: 
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
 
       			case 2: 
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
 	 ktemp=iter.roleid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 3: 
 	 ktemp=iter.status;
	 break;
case 4: 
 	 ktemp=iter.rolecode;
	 break;
case 5: 
 	 ktemp=iter.sortid;
	 break;
case 6: 
 	 ktemp=iter.rolevalue;
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			  }
 			switch(vpos){
			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
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
 				 a.emplace(iter.roleid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.status,iter);
				 break;
			case 4: 
 				 a.emplace(iter.rolecode,iter);
				 break;
			case 5: 
 				 a.emplace(iter.sortid,iter);
				 break;
			case 6: 
 				 a.emplace(iter.rolevalue,iter);
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

   			case 2: 
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

   			case 2: 
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
 				 ktemp=iter.roleid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 3: 
 				 ktemp=iter.status;
			 break;
			case 4: 
 				 ktemp=iter.rolecode;
			 break;
			case 5: 
 				 ktemp=iter.sortid;
			 break;
			case 6: 
 				 ktemp=iter.rolevalue;
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			  }
 			switch(vpos){
			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
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

   case 2: 
 	 ktemp=iter.name;
	 break;
	  }
 switch(vpos){
case 2: 
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
 	 a.emplace_back(iter.roleid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.status,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.rolecode,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.sortid,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.rolevalue,iter);
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

   case 2: 
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.roleid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.status);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.rolecode);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.rolevalue);
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
				 break;
			 }

			 switch(dpos){
			case 2: 
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			 }

			 switch(vpos){
			case 2: 
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			 }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.name;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.roleid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.status);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.rolecode);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.rolevalue);
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			  }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.name;
				 break;
			  }

			 switch(dpos){
			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.roleid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.status);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.rolecode);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.rolevalue);
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.roleid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.status;
				 break;
			case 4: 
 				 vtemp=iter.rolecode;
				 break;
			case 5: 
 				 vtemp=iter.sortid;
				 break;
			case 6: 
 				 vtemp=iter.rolevalue;
				 break;
			 }

			switch(dpos){
			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			 }

			 switch(vpos){
			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.name;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.roleid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.status);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.rolecode);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.rolevalue);
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.name;
				 break;
			  }

			 switch(dpos){
			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 2: 
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

   			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.roleid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.status);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.rolecode);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.sortid);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.rolevalue);
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			 }

			 switch(vpos){
			case 2: 
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.roleid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.status);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.rolecode);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.sortid);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.rolevalue);
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
 				 a[iter.roleid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.status].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.rolecode].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.sortid].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.rolevalue].emplace_back(iter);
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

   			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
			 }

			 switch(vpos){
			case 2: 
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

   			case 2: 
 				 ktemp=iter.name;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.roleid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.status].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.rolecode].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.sortid].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.rolevalue].emplace_back(iter);
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.roleid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.status].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.rolecode].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.sortid].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.rolevalue].emplace_back(iter);
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
 				 ktemp=iter.roleid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.status;
				 break;
			case 4: 
 				 ktemp=iter.rolecode;
				 break;
			case 5: 
 				 ktemp=iter.sortid;
				 break;
			case 6: 
 				 ktemp=iter.rolevalue;
				 break;
			  }

			 switch(vpos){
			case 2: 
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
   